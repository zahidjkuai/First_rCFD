/******************************************************************************
 * @internal
 * @file        rCFD_library.c
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#include "rCFD.h"

#include <stdio.h>
#include <string.h> // memset, strcmp
#include <time.h>   // clock

#include "rCFD_library.h"
#include "rCFD_types.h"
#include "rCFD_memory.h"
#include "rCFD_macros.h"
#include "rCFD_report.h"
#include "rCFD_parallel.h"
#include "rCFD_dict.h"
#include "rCFD_dict_read_toml.h"
#include "rCFD_dict_write_toml.h"
#include "rCFD_dump.h"
#include "rCFD_defaults.h"
#include "rCFD_init.h"
#include "rCFD_layer.h"
#include "rCFD_topo.h"
#include "rCFD_run.h"
#include "rCFD_shift.h"
#include "rCFD_prepare.h"
#include "rCFD_swap.h"
#include "rCFD_balance.h"
#include "rCFD_post.h"
#include "rCFD_free.h"
#include "rCFD_user_defaults.h"


/**
 * @brief get rCFD library version as a string of the form "rCFD major.minor.patch"
 *
 * @param major get major version
 * @param minor get minor version
 * @param patch get patch level
 * @return rCFD library version string
 */
const char* rcfd_version(int *major, int *minor, int *patch)
{
    if(major) *major = RCFD_VERSION_MAJOR;
    if(minor) *minor = RCFD_VERSION_MINOR;
    if(patch) *patch = RCFD_VERSION_PATCH;

    return RCFD_VERSION_STRING;
}

/**
 * @brief parse command line args for configuration filename (-c filename)
 * @param argc  number of command-line arguments
 * @param argv  list of command-line arguments
 * @return configuration filename or NULL if config option is not found
 */
char* get_config_filename_from_args(int argc, char **argv)
{
    int count;

    for(count = 0; count < argc-1; ++count){

        if(strcmp(argv[count], "--") == 0)
            break;

        if(    (strcmp(argv[count], "-c") == 0)
            || (strcmp(argv[count], "/c") == 0)
            || (strcmp(argv[count], "-config") == 0)
            || (strcmp(argv[count], "/config") == 0)
            || (strcmp(argv[count], "--config") == 0)){

            return argv[count+1];
        }
    }

    return NULL;
}

/**
 * @brief create an instance of rCFD and return a pointer to it;
 *        pass in command-line args and MPI communicator to run on
 *
 * @param argc  number of command-line arguments
 * @param argv  list of command-line arguments
 * @param comm  MPI communicator
 * @param ptr   pointer to a void pointer variable which serves as a handle
 */
void rcfd_open(int argc, char **argv, MPI_Comm comm, void **ptr)
{
    rCFD_type *rcfd = init_rcfd(comm, get_config_filename_from_args(argc, argv));

    if(ptr) *ptr = (void*)rcfd;
}

/**
 * @brief create an instance of rCFD and return pointer to it;
 *        caller doesn't know MPI communicator, so use MPI_COMM_WORLD;
 *        intialize MPI if needed
 *
 * @param argc  number of command-line arguments
 * @param argv  list of command-line arguments
 * @param ptr   pointer to a void pointer variable which serves as a handle
 */
void rcfd_open_no_mpi(int argc, char **argv, void **ptr)
{
#if defined(RCFD_LIB_MPI)
    int flag = 0;

    MPI_Initialized(&flag);

    if(!flag){

        int argc = 0;
        char **argv = NULL;

        MPI_Init(&argc,&argv);
    }
#endif

    rcfd_open(argc, argv, MPI_COMM_WORLD, ptr);

#if defined(RCFD_LIB_MPI)
    if(!flag){

        rCFD_type *rcfd = (rCFD_type*)(*ptr);

        rcfd->Parallel->finalize = 1;
    }
#endif
}

/**
 * @brief initialize all data structures for specified mode
 *
 * @param handle    pointer to previously created rCFD instance
 * @param mode      preparation (0) or run (1) mode
 */
void rcfd_init(void *handle, int mode)
{
    rCFD_type *rcfd = (rCFD_type*)handle;

    report_start_timer(rcfd, RCFD_TIMER_GLOBAL);

    switch(mode){

    case RCFD_MODE_PREPARATION:
        init_all(rcfd, mode);
        break;
    case RCFD_MODE_RUN:
        report_start_timer(rcfd, RCFD_TIMER_INIT);
        init_all(rcfd, mode);
        post_init_all(rcfd);
        break;
    }
}

/**
 * @brief get initialization status of rCFD instance
 *
 * @param handle    pointer to previously created rCFD instance
 * @return initialization status of rCFD instance
 */
int rcfd_is_initialized(void *handle)
{
    rCFD_type *rcfd = (rCFD_type*)handle;

    return rcfd->initialized;
}

/**
 * @brief read rCFD database (cell-to-cell shift pattern)
 *
 * @param handle    pointer to previously created rCFD instance
 */
void rcfd_read_db(void *handle)
{
    rCFD_type *rcfd = (rCFD_type*)handle;

    report_start_timer(rcfd, RCFD_TIMER_READ_DB);

    report_start_timer(rcfd, RCFD_TIMER_READ_SHIFT);
    shift_read(rcfd);
    post_read_shift(rcfd);

#if MULTI_LAYER_IMPLEMENTATION_COMPLETE
    shift_create_shifts_for_upper_layers(rcfd);
#endif

    parallel_shift_init(rcfd);

    post_read(rcfd);
}

/**
 * @brief run rCFD timestep cycles
 *
 * @param handle    pointer to previously created rCFD instance
 */
void rcfd_run(void *handle)
{
#if RP_NODE
    int i_run, i_phase;

    rCFD_type *rcfd = (rCFD_type*)handle;

    report_start_timer(rcfd, RCFD_TIMER_RUN);

    for(i_run = 0; i_run < rcfd->Solver_Dict->number_of_runs; i_run++){

        /* Update state, layer and frame */
        {
            run_update_state(rcfd);

#if MULTI_LAYER_IMPLEMENTATION_COMPLETE
            run_update_layer(rcfd);
#endif

            report_start_timer(rcfd, RCFD_TIMER_RUN_UPDATE);
            run_update_frame(rcfd);
            post_run_update_frame(rcfd);

            run_update_balance(rcfd);
        }

        /* User access before phase loop */
        {
            report_start_timer(rcfd, RCFD_TIMER_USER);
            rcfd->rCFD_UDF->_user_access_data_before_phase_loop(handle);
            report_stop_timer(rcfd, RCFD_TIMER_USER);
        }

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            /* User access before shift */
            {
                report_start_timer(rcfd, RCFD_TIMER_USER);
                rcfd->rCFD_UDF->_user_access_data_before_shift(handle, i_phase);
                report_stop_timer(rcfd, RCFD_TIMER_USER);
            }

            /* Cell shifts */
            {

                if(rcfd->Solver_Dict->data_convection_on){

                    report_start_timer(rcfd, RCFD_TIMER_RUN_SHIFT);

                    shift_data(rcfd, i_phase);

                    shift_fill(rcfd, i_phase);

                    post_run_shift(rcfd, i_phase);
                }
            }

            /* Face swaps */
            {
                report_start_timer(rcfd, RCFD_TIMER_RUN_SWAP);

                if(rcfd->Solver_Dict->face_swap_diffusion_on && rcfd->Solver->current_layer == 0){

                    swap_data(rcfd, i_phase);
                }

                if(rcfd->Solver_Dict->data_drifting_on && rcfd->Solver->current_layer == 0){

                    swap_drift_data(rcfd, i_phase);
                }

                if(rcfd->Solver_Dict->data_binarization_on && rcfd->Solver->current_layer == 0){

                    swap_binarize_data(rcfd, i_phase);
                }

                post_run_swap(rcfd, i_phase);
            }

            /* User access after swap */
            {
                report_start_timer(rcfd, RCFD_TIMER_USER);
                rcfd->rCFD_UDF->_user_access_data_after_swap(handle, i_phase);
                report_stop_timer(rcfd, RCFD_TIMER_USER);
            }

            /* Balance correction */
            {
                report_start_timer(rcfd, RCFD_TIMER_RUN_BALANCE);

                balance_update(rcfd, i_phase);

                balance_correct(rcfd, i_phase);

                balance_control_conc_sum(rcfd, i_phase);

                post_run_balance(rcfd, i_phase);
            }

        }

        /* User access after phase loop */
        {
            report_start_timer(rcfd, RCFD_TIMER_USER);
            rcfd->rCFD_UDF->_user_post(handle);
            report_stop_timer(rcfd, RCFD_TIMER_USER);
        }

        dump_data(rcfd);

        rcfd->Solver->global_run_counter++;

        rcfd->Solver->global_time += rcfd->Solver->timestep_width[rcfd->Solver->current_layer];
    }

    post_run(rcfd);
#endif
}

/**
 * @brief delete rCFD instance created by rcfd_open() or its variants
 *
 * @param handle    pointer to previously created rCFD instance
 */
void rcfd_close(void *handle)
{
    if(!handle)
        return;

    rCFD_type *rcfd = (rCFD_type*)handle;

    if(rcfd->initialized){

        int mode =  rcfd->Solver_Dict->mode;

        if(mode == RCFD_MODE_PREPARATION){

            prep_write_metadata(rcfd);
        }

        free_all(rcfd);

        post_free(rcfd, mode);
    }

#if defined(RCFD_LIB_MPI)
    if(rcfd->Parallel->finalize){

        int flag = 0;
        MPI_Initialized(&flag);

        if(flag){

            MPI_Finalized(&flag);
            if(!flag){

                free_report(rcfd);
                MPI_Barrier(MPI_COMM_WORLD);
                MPI_Finalize();
            }
        }
    }
#endif

    free_rcfd(rcfd);
}

/**
 * @brief set displacement and diffusion fields
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the layer
 * @param i_phase   specifies the phase
 * @param cfd_displ (flattened) displacement field (x, y, z values for each cell)
 * @param cfd_displ_diff  (flattened) diffusion field (xx, yy, zz, xy, yz, zx values for each cell)
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_displacement_fields(void *handle, int i_layer, int i_phase, double *cfd_displ, double *cfd_displ_diff)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%d] rcfd_set_displacement_fields:"
                         " setting displacement fields only possible in preparation mode", myid);
            return -1;
        }

        if(    (i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            || (i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%d] rcfd_set_displacement_fields: invalid args", myid);
            return -1;
        }

        if(cfd_displ && _C(i_layer).cfd_displ){

            memcpy(&_C(i_layer).cfd_displ[i_phase][0][0], cfd_displ, sizeof(double) * _CELL_DICT(i_layer).number_of_cells * 3);
        }

        if(cfd_displ_diff && _C(i_layer).cfd_displ_diff){

            memcpy(&_C(i_layer).cfd_displ_diff[i_phase][0][0], cfd_displ_diff, sizeof(double) * _CELL_DICT(i_layer).number_of_cells * 6);
        }

        return 0;
    }
#endif // RP_NODE

    return -1;
}

/**
 * @brief generate primary cell-to-cell shifts from continuous displacement fields,
 *        additional shifts may be generated after this function has succeeded by
 *        calling rcfd_interpolate_shifts()
 *
 * @param handle    pointer to previously created rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_create_shifts_from_displacement_fields(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%d] rcfd_create_shifts_from_displacement_fields:"
                         " generation of shifts only possible in preparation mode", myid);
            return -1;
        }

        int i_layer = rcfd->Solver->current_layer;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers){

            report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%d] rcfd_create_shifts_from_displacement_fields: invalid layer %d", myid, i_layer);
            return -1;
        }

        // generate primary shifts for all phases
        if(rcfd->Solver_Dict->shifts_from_displacement_fields_on){

            int i_phase;
            int phase_count = rcfd->Solver_Dict->number_of_phases;
            int primary_shift_count, global_primary_shift_count;

            for(i_phase = 0; i_phase < phase_count; i_phase++){

                primary_shift_count = prep_shift_from_displacement_fields(rcfd, i_layer, i_phase);
                global_primary_shift_count = PRF_GISUM1(primary_shift_count);

                report_info0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                             "rcfd_create_shifts_from_displacement_fields (layer %d, phase %d): number of primary shifts: %d",
                             i_layer, i_phase, global_primary_shift_count);
            }
        }
        else{

            report_error0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                          "rcfd_create_shifts_from_displacement_fields:"
                          " shifts_from_displacement_fields_on is turned off");
            return -1;
        }

        return 0;
    }
#endif // RP_NODE

    return -1;
}

/**
 * @brief create cell-to-cell shifts for grid interfaces (in order to prevent information locks)
 *
 * @param handle                    pointer to previously created rCFD instance
 * @param interface_face_centers    (flattened) array of (process-local) coordinates for interface face centers
 * @param interface_face_count      (process-local) number of interface face centers
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_create_shifts_from_interfaces(void   *handle,
                                       double *interface_face_centers,
                                       int     interface_face_count)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%d] rcfd_create_shifts_from_interfaces:"
                         " creation of shifts only possible in preparation mode", myid);
            return -1;
        }

        if(!rcfd->Solver_Dict->grid_interface_shifts_on){

            report_error0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                          "rcfd_create_shifts_from_interfaces:"
                          " grid_interface_shifts_on is turned off");
            return -1;
        }

        int interface_shift_count = prep_shift_from_interfaces(rcfd,
                                                               interface_face_centers,
                                                               interface_face_count);

        int error_flag = PRF_GILOW1(interface_shift_count);

        if(error_flag < 0){

            report_error0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "rcfd_create_shifts_from_interfaces: failed to create interface shifts");
        }
        else{

            int global_interface_shift_count = PRF_GISUM1(interface_shift_count);

            if(global_interface_shift_count > 0){

                report_info0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                             "rcfd_create_shifts_from_interfaces: created %d interface shifts",
                             global_interface_shift_count);
            }
        }

        return 0;
    }
#endif // RP_NODE

    return -1;
}

/**
 * @brief Generate additional cell-to-cell shifts via interpolation for more
 *        accurate shift patterns; only available in preparation mode
 *
 * @param handle    pointer to previously created rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_interpolate_shifts(void *handle)
{
#if RP_NODE

    /* known limitations:
        ->  single layer
    */

    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%d] rcfd_interpolate_shifts: interpolation only possible in preparation mode", myid);
            return -1;
        }

        return prep_interpolate_shifts(rcfd);
    }
#endif

    return -1;
}

/**
 * @brief sets vof field variable
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   current grid layer
 * @param cell_vof  two-dim array of cell vof values [i_cell][i_phase]
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_vof(void *handle, int i_layer, double** cell_vof)
{
#if RP_NODE

    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%d] rcfd_set_vof: only available in preparation mode", myid);
            return -1;
        }

        int i_cell, i_phase, i_frame = 0;

        int int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
        int cell_count =     _CELL_DICT(i_layer).number_of_cells;
        int phase_count =    rcfd->Solver_Dict->number_of_phases;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            for(i_phase = 0; i_phase < phase_count; i_phase++){

                _C(i_layer).vof[i_frame][i_cell][i_phase] = cell_vof[i_cell][i_phase];
            }
        }

        double *tmp = malloc(cell_count * sizeof(double));
        memset(tmp, 0, cell_count * sizeof(double));

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                tmp[i_cell] = cell_vof[i_cell][i_phase];
            }

            parallel_communicate(rcfd, tmp, i_layer, RCFD_PARALLEL_MAP_HOST_FORMAT);

            for(i_cell = int_cell_count; i_cell < cell_count; i_cell++){

                _C(i_layer).vof[i_frame][i_cell][i_phase] = tmp[i_cell];
            }
        }

        free(tmp);

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief write cell-to-cell shifts to files; only available in preparation mode
 *
 * @param handle    pointer to previously created rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_write_shifts(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;
        int i_phase, i_state, i_frame, i_layer;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_SHIFT,
                         "[%d] rcfd_write_shifts: only possible in preparation mode", myid);
            return -1;
        }

        i_frame = rcfd->Solver->current_frame;

        if(i_frame >= rcfd->Solver_Dict->number_of_frames){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_SHIFT,
                         "[%d] rcfd_write_shifts: reached max. number of frames", myid);
            return -1;
        }

        {
            int stored_C2C_count;

            i_state = rcfd->Solver->current_state;
            i_layer = rcfd->Solver->current_layer;

            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                stored_C2C_count = shift_write(rcfd, i_state, i_frame, i_phase, i_layer);
                stored_C2C_count = PRF_GISUM1(stored_C2C_count);

                report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_SHIFT,
                             "rcfd_write_shifts: total number of shifts %d (i_state %d i_frame %d, i_phase %d i_layer %d)",
                             stored_C2C_count, i_state, i_frame, i_phase, i_layer);
            }

            rcfd->C2C_Interface->valid_shifts_count = 0;

            rcfd->Solver->current_frame++;
        }

        return 0;
    }
#endif // RP_NODE

    return -1;
}

/**
 * @brief write the recurrence matrix, recurrence path and recurrence path
 *        differences to files; only available in preparation mode
 * @param handle    pointer to previously created rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_write_recurrence(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_write_recurrence: only possible in preparation mode", myid);
            return -1;
        }

        return prep_write_recurrence(rcfd);
    }

    return -1;
}

/**
 * @brief Write cell, face and node data to (process-specific) topology files
 * @param handle    pointer to previously created rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_write_topo(void *handle)
{
#if RP_NODE

    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!rcfd->initialized){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_TOPOLOGY,
                         "[%d] rcfd_write_topo: rcfd is not initialized", myid);
            return -1;
        }

        return topo_write(rcfd);
    }
#endif

    return -1;
}

/**
 * @brief set the norms, vof, local diffusion and user-defined recurrence fields for the current frame;
 *          only available in preparation mode
 * @param handle    pointer to previously created rCFD instance
 * @param norm      array of cell norms; may be NULL if set directly or via user defined function (i.e. norms format is different from 'standard')
 * @param vof       vof field for each phase ([i_cell][i_phase]); may be NULL in case of single-phase or if set directly
 * @param local_diffusion local diffusion field ([i_phase][i_cell]); may be NULL if local diffusion is turned off or if set directly
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_fields(void *handle, double *norm, double **vof, double **local_diffusion)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_set_fields: only possible in preparation mode", myid);
            return -1;
        }

        return prep_set_fields(rcfd, norm, vof, local_diffusion);
    }
#endif

    return -1;
}

/**
 * @brief write a database entry for norms, vof, local diffusion and user-defined recurrence fields of the current frame;
 *          only available in preparation mode
 * @param handle    pointer to previously created rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_write_fields(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_write_fields: only possible in preparation mode", myid);
            return -1;
        }

        return prep_write_fields(rcfd);
    }
#endif

    return -1;
}

/**
 * @brief register user defined functions
 *
 * @param handle    pointer to previously created rCFD instance
 * @param udf       name of user defined function to register to
 * @param callback  user function to be registered
 * @retval -1 Error (invalid udf string)
 * @retval 0 OK
 */
int rcfd_register_user_function(void *handle, const char *udf, void (*callback)(void))
{
    rCFD_type *rcfd = (rCFD_type*)handle;

    if(strcmp(udf, "user_set_solver_dict") == 0){

        rcfd->rCFD_UDF->_user_set_solver_dict   = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_file_dict") == 0){

        rcfd->rCFD_UDF->_user_set_file_dict     = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_phase_dict") == 0){

        rcfd->rCFD_UDF->_user_set_phase_dict    = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_prep_dict") == 0){

        rcfd->rCFD_UDF->_user_set_prep_dict     = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_norm_dict") == 0){

        rcfd->rCFD_UDF->_user_set_norm_dict     = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_rec_dict") == 0){

        rcfd->rCFD_UDF->_user_set_rec_dict      = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_data_dict") == 0){

        rcfd->rCFD_UDF->_user_set_data_dict     = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_balance_dict") == 0){

        rcfd->rCFD_UDF->_user_set_balance_dict  = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_topo_dict") == 0){

        rcfd->rCFD_UDF->_user_set_topo_dict     = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_cell_dict") == 0){

        rcfd->rCFD_UDF->_user_set_cell_dict     = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_set_face_dict") == 0){

        rcfd->rCFD_UDF->_user_set_face_dict     = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_set_node_dict") == 0){

        rcfd->rCFD_UDF->_user_set_node_dict     = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_set_dump_dict") == 0){

        rcfd->rCFD_UDF->_user_set_dump_dict     = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_pre_proc") == 0){

        rcfd->rCFD_UDF->_user_pre_proc          = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_init_cells") == 0){

        rcfd->rCFD_UDF->_user_init_cells        = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_init_faces") == 0){

        rcfd->rCFD_UDF->_user_init_faces        = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_init_cell_hosts") == 0){

        rcfd->rCFD_UDF->_user_init_cell_hosts   = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_init_nodes") == 0){

        rcfd->rCFD_UDF->_user_init_nodes        = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_init_data") == 0){

        rcfd->rCFD_UDF->_user_init_data         = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_set_layer") == 0){

        rcfd->rCFD_UDF->_user_set_layer         = (short(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_access_data_before_phase_loop") == 0){

        rcfd->rCFD_UDF->_user_access_data_before_phase_loop = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_access_data_before_shift") == 0){

        rcfd->rCFD_UDF->_user_access_data_before_shift  = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_access_data_after_swap") == 0){

        rcfd->rCFD_UDF->_user_access_data_after_swap    = (void(*)(void*,const short))callback;
    }
    else if(strcmp(udf, "user_post") == 0){

        rcfd->rCFD_UDF->_user_post              = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_norm") == 0){

        rcfd->rCFD_UDF->_user_set_norm          = (void(*)(void*))callback;
    }
    else if(strcmp(udf, "user_set_rec_vars") == 0){

        rcfd->rCFD_UDF->_user_set_rec_vars = (void(*)(void*, const short))callback;
    }
    else{

        return -1;
    }

    return 0;
}

/**
 * @brief Write a message to the node0 report output stream, depending on the message priority
 *
 * @param handle    pointer to previously created rCFD instance
 * @param priority  message priority determining visibility in output
 * @param format    string that contains the text to be written; it may contain
 *                  format specifiers that are replaced by the values specified
 *                  in subsequent additional arguments and formatted as requested
 */
void rcfd_report0(void* handle, int priority, const char *format, ...)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(priority < 0 || priority >= RCFD_REPORT_MAX){

            priority = RCFD_REPORT_UNKNOWN;
        }

        rcfd->Report->number_of_reports[RCFD_REPORT_CATEGORY_USER][priority]++;

        if(rcfd->Report->priority > priority)
            return;

        va_list args;
        va_start(args, format);
        report_write_line0(rcfd, priority, vform(format, args));
        va_end(args);
    }
}

/**
 * @brief set integer dict property
 *
 * @param handle    pointer to previously created rCFD instance
 * @param dict      name of the dictonary holding the property
 * @param i_layer   specifies the layer if the property is per-layer
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_dict_property_i(void *handle, const char *dict, int i_layer, int i_phase, int i_data, const char *property, int value)
{
    if(handle){

        if(strcmp(dict, "File_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "integer");
            return -1;
        }
        else if(strcmp(dict, "Solver_Dict") == 0){

            return set_solver_dict_property_i(handle, property, value);
        }
        else if(strcmp(dict, "Phase_Dict") == 0){

            return set_phase_dict_property_i(handle, i_phase, property, value);
        }
        else if(strcmp(dict, "Prep_Dict") == 0){

            return set_prep_dict_property_i(handle, i_phase, property, value);
        }
        else if(strcmp(dict, "Norm_Dict") == 0){

            return set_norm_dict_property_i(handle, property, value);
        }
        else if(strcmp(dict, "Rec_Dict") == 0){

            return set_rec_dict_property_i(handle, property, value);
        }
        else if(strcmp(dict, "Data_Dict") == 0){

            return set_data_dict_property_i(handle, i_phase, i_data, property, value);
        }
        else if(strcmp(dict, "Balance_Dict") == 0){

            return set_balance_dict_property_i(handle, i_phase, i_data, property, value);
        }
        else if(strcmp(dict, "Topo_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "integer");
            return -1;
        }
        else if(strcmp(dict, "Cell_Dict") == 0){

            return set_cell_dict_property_i(handle, i_layer, property, value);
        }
        else if(strcmp(dict, "Face_Dict") == 0){

            return set_face_dict_property_i(handle, i_layer, property, value);
        }
        else if(strcmp(dict, "Node_Dict") == 0){

            return set_node_dict_property_i(handle, i_layer, property, value);
        }
        else if(strcmp(dict, "Dump_Dict") == 0){

            return set_dump_dict_property_i(handle, i_phase, i_data, property, value);
        }
        else{

            report_dict_invalid((rCFD_type*)handle, dict, property);
            return -1;
        }
    }

    return -1;
}

/**
 * @brief set floating point dict property
 *
 * @param handle    pointer to previously created rCFD instance
 * @param dict      name of the dictonary holding the property
 * @param i_layer   specifies the layer if the property is per-layer
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     floating point value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_dict_property_r(void *handle, const char *dict, int i_layer, int i_phase, int i_data, const char *property, double value)
{
    if(handle){

        if(strcmp(dict, "File_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "real");
            return -1;
        }
        else if(strcmp(dict, "Solver_Dict") == 0){

            return set_solver_dict_property_r(handle, property, value);
        }
        else if(strcmp(dict, "Phase_Dict") == 0){

            return set_phase_dict_property_r(handle, i_phase, property, value);
        }
        else if(strcmp(dict, "Prep_Dict") == 0){

            return set_prep_dict_property_r(handle, i_phase, property, value);
        }
        else if(strcmp(dict, "Norm_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "real");
            return -1;
        }
        else if(strcmp(dict, "Rec_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "real");
            return -1;
        }
        else if(strcmp(dict, "Data_Dict") == 0){

            return set_data_dict_property_r(handle, i_phase, i_data, property, value);
        }
        else if(strcmp(dict, "Balance_Dict") == 0){

            return set_balance_dict_property_r(handle, i_phase, i_data, property, value);
        }
        else if(strcmp(dict, "Topo_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "real");
            return -1;
        }
        else if(strcmp(dict, "Cell_Dict") == 0){

            return set_cell_dict_property_r(handle, i_layer, property, value);
        }
        else if(strcmp(dict, "Face_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "real");
            return -1;
        }
        else if(strcmp(dict, "Node_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "real");
            return -1;
        }
        else if(strcmp(dict, "Dump_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "real");
            return -1;
        }
        else{

            report_dict_invalid((rCFD_type*)handle, dict, property);
            return -1;
        }
    }

    return -1;
}

/**
 * @brief set 3d vector dict property
 *
 * @param handle    pointer to previously created rCFD instance
 * @param dict      name of the dictonary holding the property
 * @param i_layer   specifies the layer if the property is per-layer
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     3d array of double values to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_dict_property_r_3d(void *handle, const char *dict, int i_layer, int i_phase, int i_data, const char *property, double *value)
{
    if(handle){

        if(strcmp(dict, "Data_Dict") == 0){

            return set_data_dict_property_r_3d(handle, i_phase, i_data, property, value);
        }
        else if(strcmp(dict, "Dump_Dict") == 0){

            return set_dump_dict_property_r_3d(handle, i_layer, i_phase, i_data, property, value);
        }
        else{

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "vector");
            return -1;
        }
    }

    return -1;
}

/**
 * @brief set string dict property
 *
 * @param handle    pointer to previously created rCFD instance
 * @param dict      name of the dictonary holding the property
 * @param i_layer   specifies the layer if the property is per-layer
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_dict_property_s(void *handle, const char *dict, int i_layer, int i_phase, int i_data, const char *property, const char *value)
{
    if(handle){

        if(strcmp(dict, "File_Dict") == 0){

            return set_file_dict_property_s(handle, property, value);
        }
        else if(strcmp(dict, "Solver_Dict") == 0){

            return set_solver_dict_property_s(handle, property, value);
        }
        else if(strcmp(dict, "Phase_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "string");
            return -1;
        }
        else if(strcmp(dict, "Prep_Dict") == 0){

            return set_prep_dict_property_s(handle, property, value);
        }
        else if(strcmp(dict, "Norm_Dict") == 0){

            return set_norm_dict_property_s(handle, property, value);
        }
        else if(strcmp(dict, "Rec_Dict") == 0){

            return set_rec_dict_property_s(handle, property, value);
        }
        else if(strcmp(dict, "Data_Dict") == 0){

            return set_data_dict_property_s(handle, i_phase, i_data, property, value);
        }
        else if(strcmp(dict, "Balance_Dict") == 0){

            return set_balance_dict_property_s(handle, i_phase, i_data, property, value);
        }
        else if(strcmp(dict, "Topo_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "string");
            return -1;
        }
        else if(strcmp(dict, "Cell_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "string");
            return -1;
        }
        else if(strcmp(dict, "Face_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "string");
            return -1;
        }
        else if(strcmp(dict, "Node_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "string");
            return -1;
        }
        else if(strcmp(dict, "Dump_Dict") == 0){

            report_dict_invalid_property_type((rCFD_type*)handle, dict, property, "string");
            return -1;
        }
        else{

            report_dict_invalid((rCFD_type*)handle, dict, property);
            return -1;
        }
    }

    return -1;
}

// getter methods
// from Solver_Dict_type
/**
 * @brief get global time step
 *
 * @param handle    pointer to previously created rCFD instance
 * @return global time step or 0 if handle is invalid
 */
double rcfd_get_global_timestep(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->global_time_step;
    }

    return 0.0;
}

/**
 * @brief get vof cutoff limit
 *
 * @param handle    pointer to previously created rCFD instance
 * @return vof cutoff limit or 0 if handle is invalid
 */
double  rcfd_get_vof_cutoff_limit(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->vof_cutoff_limit;
    }

    return 0.0;
}

/**
 * @brief get number of frames
 *
 * @param handle    pointer to previously created rCFD instance
 * @return number of frames or -1 if handle is invalid
 */
int rcfd_get_frame_count(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->number_of_frames;
    }

    return -1;
}

/**
 * @brief get number of phases
 *
 * @param handle    pointer to previously created rCFD instance
 * @return number of phases or -1 if handle is invalid
 */
int rcfd_get_phase_count(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->number_of_phases;
    }

    return -1;
}

/**
 * @brief get local diffusion format
 *
 * @param handle    pointer to previously created rCFD instance
 * @return diffusion format or -1 if handle is invalid
 */
int rcfd_get_local_diffusion_format(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->local_diff_format;
    }

    return -1;
}

/**
 * @brief get number of timesteps per monitoring interval
 *
 * @param handle    pointer to previously created rCFD instance
 * @return monitoring interval size or -1 if handle is invalid
 */
int rcfd_get_monitoring_interval(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->time_steps_per_monitoring_interval;
    }

    return -1;
}

/**
 * @brief check if vof is used in simulation
 *
 * @param handle    pointer to previously created rCFD instance
 * @retval 1 in case of multiple phases or single-phase vof
 * @retval 0 in case of single phase (vof is equal 1)
 */
int rcfd_is_vof_on(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return (rcfd->Solver_Dict->number_of_phases > 1 || rcfd->Solver_Dict->single_phase_vof);
    }

    return 0;
}

/**
 * @brief check if cell-to-cell shifts are created based on displacement fields
 *
 * @param handle    pointer to previously created rCFD instance
 * @return on/off status of setting or 0 if handle is invalid
 */
int rcfd_is_shifts_from_displacement_fields_on(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->shifts_from_displacement_fields_on;
    }

    return 0;
}

/**
 * @brief check if geometry is moving
 *
 * @param handle    pointer to previously created rCFD instance
 * @return on/off status of setting or 0 if handle is invalid
 */
int rcfd_is_moving_geometry_on(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->moving_geometry_on;
    }

    return 0;
}

/**
 * @brief check if local diffusion is turned on
 *
 * @param handle    pointer to previously created rCFD instance
 * @return on/off status of setting or 0 if handle is invalid
 */
int rcfd_is_local_diffusion_on(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver_Dict->local_diff_on;
    }

    return 0;
}

// from Cell_Dict_type
/**
 * @brief get number of (process-local) cells in specified layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return number of cells or -1 if handle or i_layer is invalid
 */
int rcfd_get_cell_count(void *handle, int i_layer)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1;

        return _CELL_DICT(i_layer).number_of_cells;
    }

    return -1;
}

/**
 * @brief get number of (process-local) interal cells in specified layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return number of internal cells or -1 if handle or i_layer is invalid
 */
int rcfd_get_int_cell_count(void *handle, int i_layer)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1;

        return _CELL_DICT(i_layer).number_of_int_cells;
    }

    return -1;
}

/**
 * @brief get number of user variables in specified layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return number of user variables or -1 if handle or i_layer is invalid
 */
int rcfd_get_cell_user_count(void *handle, int i_layer)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1;

        return _CELL_DICT(i_layer).number_of_user_vars;
    }

    return -1;
}

/**
 * @brief get number of recurrent user variables in specified layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return number of rec. user variables or -1 if handle or i_layer is invalid
 */
int rcfd_get_cell_rec_user_count(void *handle, int i_layer)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1;

        return _CELL_DICT(i_layer).number_of_rec_user_vars;
    }

    return -1;
}

// from Face_Dict_type
/**
 * @brief get number of (process-local) faces in specified layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return number of faces or -1 if handle or i_layer is invalid
 */
int rcfd_get_face_count(void *handle, int i_layer)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1;

        return _FACE_DICT(i_layer).number_of_faces;
    }

    return -1;
}

/**
 * @brief get number of (process-local) internal faces in specified layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return number of internal faces or -1 if handle or i_layer is invalid
 */
int rcfd_get_int_face_count(void *handle, int i_layer)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1;

        return _FACE_DICT(i_layer).number_of_int_faces;
    }

    return -1;
}

// from Phase_Dict_type
/**
 * @brief get number of data for the specified phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase
 * @return number of data or -1 if handle or i_phase is invalid
 */
int rcfd_get_phase_data_count(void *handle, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return -1;

        return rcfd->Phase_Dict[i_phase].number_of_data;
    }
#endif // RP_NODE

    return -1;
}

/**
 * @brief get density for the specified phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase
 * @return density or 0 if handle or i_phase is invalid
 */
double rcfd_get_phase_density(void *handle, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return 0.0;

        return rcfd->Phase_Dict[i_phase].density;
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get heat capacity for the specified phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase
 * @return heat capacity or 0 if handle or i_phase is invalid
 */
double  rcfd_get_phase_heat_capacity(void *handle, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return 0.0;

        return rcfd->Phase_Dict[i_phase].heat_capacity;
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get reference temperature for the specified phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase
 * @return reference temperature or 0 if handle or i_phase is invalid
 */
double  rcfd_get_phase_reference_temperature(void *handle, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return 0.0;

        return rcfd->Phase_Dict[i_phase].reference_temperature;
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get number of user variables for the specified phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase
 * @return number of user variables or -1 if handle or i_phase is invalid or not RP_NODE
 */
int rcfd_get_phase_user_count(void *handle, int i_phase)
{
#if RP_NODE

    if(handle == NULL) return -1;

    rCFD_type *rcfd = (rCFD_type*)handle;

    if((i_phase < 0) || (i_phase >= rcfd->Solver_Dict->number_of_phases))
        return -1;

    return rcfd->Phase_Dict[i_phase].number_of_user_vars;

#endif // RP_NODE

    return -1;
}

/**
 * @brief get user vars for the specified phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase
 * @return pointer to array of user_vars or NULL if handle, i_phase is invalid or not RP_NODE
 */

double* rcfd_get_phase_user(void *handle, int i_phase)
{
#if RP_NODE

    if(handle == NULL) return NULL;

    rCFD_type *rcfd = (rCFD_type*)handle;

    if((i_phase < 0) || (i_phase >= rcfd->Solver_Dict->number_of_phases))
        return NULL;

    return rcfd->Phase_Dict[i_phase].user;

#endif // RP_NODE

    return NULL;
}

// from Solver_type
/**
 * @brief get current state
 *
 * @param handle    pointer to previously created rCFD instance
 * @return current state or -1 if handle is invalid
 */
int rcfd_get_current_state(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver->current_state;
    }

    return -1;
}

/**
 * @brief get upcoming state
 *
 * @param handle    pointer to previously created rCFD instance
 * @return upcoming state or -1 if handle is invalid
 */
int rcfd_get_next_state(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver->upcoming_state;
    }

    return -1;
}

/**
 * @brief get current layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @return current layer or -1 if handle is invalid
 */
int rcfd_get_current_layer(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver->current_layer;
    }

    return -1;
}

/**
 * @brief get number of cell-to-cell shift frames (i.e. database entries) already written

 * @param handle    pointer to previously created rCFD instance
 * @return number of already written cell-to-cell shift frames or -1 if handle is invalid or not in preparation mode
 */
int rcfd_get_current_frame(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver->current_frame;
    }
#endif

    return -1;
}

/**
 * @brief get global run counter
 *
 * @param handle    pointer to previously created rCFD instance
 * @return global run counter or -1 if handle is invalid
 */
int rcfd_get_run_counter(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver->global_run_counter;
    }

    return -1;
}

/**
 * @brief get global time
 *
 * @param handle    pointer to previously created rCFD instance
 * @return global time or 0 if handle is invalid
 */
double rcfd_get_time(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Solver->global_time;
    }

    return 0.0;
}

/**
 * @brief get timestep width for the specified layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return timestep width or 0 if handle or i_layer is invalid
 */
double rcfd_get_timestep(void *handle, int i_layer)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return 0.0;

        return rcfd->Solver->timestep_width[i_layer];
    }

    return 0.0;
}

// dealing with multiple cell layers
/**
 * @brief map cell data from first specified cell layer to second specified cell layer
 *
 * @param handle    pointer to previously created rCFD instance
 * @param from      layer to map from
 * @param to        layer to map to
 */
void rcfd_map_layer(void *handle, int from, int to)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

#if MULTI_LAYER_IMPLEMENTATION_COMPLETE
        layer_map_from_to(rcfd, from, to);
#else
        report_error0(rcfd, RCFD_REPORT_CATEGORY_NOTSET,
                      "Implementation of multiple cell layers is incomplete.");
#endif // MULTI_LAYER_IMPLEMENTATION_COMPLETE
    }
#endif // RP_NODE
}


// access methods

// indirect access to Cell_type arrays
/**
 * @brief set cell shape type and nodes of the given cell; cell shapes must be set in consecutive ascending order of cell indices;
 *          cell shapes must not be reset;
 * @param handle            pointer to previously created rCFD instance
 * @param i_layer           specifies the cell layer
 * @param i_cell            specifies the cell to define
 * @param cell_type         specifies cell shape type (one of CELL_TYPE as in VTK)
 * @param cell_node_ids     array containing the number of cell nodes followed by the node indices
 * @param poly_face_nodes   in case of polyhedral cells, array containing the number of polyfaces followed by the the node count of each face
 *                          otherwise might be NULL
 * @retval -1 Error
 * @retval 0 OK
 */
int rcfd_set_cell_shape(void *handle, int i_layer, int i_cell, int cell_type, const int *cell_node_ids, const int *poly_face_nodes)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1;

        if(i_cell < 0 || i_cell >= _CELL_DICT(i_layer).number_of_cells)
            return -1;


        return topo_set_cell_nodes(rcfd, i_layer, i_cell, cell_type, cell_node_ids, poly_face_nodes);
    }
#endif // RP_NODE

    return -1;
}

// direct access to MPI_Cell_type arrays
/**
 * @brief get (process-local) cell host array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to cell hosts or NULL if handle or i_layer is invalid
 */
int* rcfd_get_cell_host(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _MPI_Cell.host_of_cell;
    }
#endif // RP_NODE

    return NULL;
}

// direct access to Cell_type arrays
/**
 * @brief get (process-local) cell center array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to cell centers or NULL if handle or i_layer is invalid
 */
double** rcfd_get_cell_center(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _C(i_layer).x;
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) cell volume array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to cell volumes or NULL if handle or i_layer is invalid
 */
double* rcfd_get_cell_volume(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _C(i_layer).volume;
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) cell data array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_phase   specifies the desired phase
 * @return pointer to cell data or NULL if handle, i_layer or i_phase is invalid
 */
double** rcfd_get_cell_data(void *handle, int i_layer, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return NULL;

        return _C(i_layer).data[i_phase]; //[i_cell][i_data]
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) cell vof array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_frame   specifies the rec frame
 * @return pointer to cell vof or NULL if handle, i_layer or i_frame is invalid
 */
double** rcfd_get_cell_vof(void *handle, int i_layer, int i_frame)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        if(i_frame < 0 || i_frame >= rcfd->Solver_Dict->number_of_frames)
            return NULL;

        return _C(i_layer).vof[i_frame]; //[i_cell][i_phase]
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) cell user data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to cell user data or NULL if handle or i_layer is invalid
 */
double** rcfd_get_cell_user(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _C(i_layer).user; //[i_cell][i_user] (number_of_user_vars)
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) cell recurrent user data of a given frame
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_frame   specifies the frame
 * @return pointer to cell rec. user data or NULL if handle or i_layer or i_frame is invalid
 */
double** rcfd_get_cell_rec_user(void *handle, int i_layer, int i_frame)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        if(i_frame < 0 || i_frame >= rcfd->Solver_Dict->number_of_frames)
            return NULL;

        if((rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION) && (i_frame != 0))
            return NULL;

        return _C(i_layer).rec_user[i_frame]; //[i_cell][i_rec_user] (number_of_rec_user_vars)
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get the value for a given probe (collective function, must be called from all processes)
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_probe   specifies the probe to query
 * @return probe value or zero if handle or i_probe is invalid
 */
double rcfd_get_probe_value(void *handle, int i_probe)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_probe < 0 || i_probe >= rcfd->Dump_Dict->probe_count)
            return 0.0;

        return dump_get_probe_value(rcfd, i_probe);
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get distance between a cell center and given point coordinates
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_cell    specifies the cell
 * @param point_x   specifies the point coordinates
 * @return distance or -1.0 if handle, i_layer or i_cell is invalid
 */
double rcfd_cell2point_distance(void *handle, int i_layer, int i_cell, double* point_x)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return -1.0;

        if(i_cell < 0 || i_cell >= _CELL_DICT(i_layer).number_of_cells)
            return -1.0;

        if(!point_x)
            return -1.0;

        return cell2point_distance(rcfd, i_layer, i_cell, point_x);
    }
#endif // RP_NODE

    return -1.0;
}

/**
 * @brief get (process-local) cfd diffusion data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_phase   specifies the desired phase
 * @return pointer to cfd diffusion data or NULL if handle or i_layer or i_phase is invalid
 */
double** rcfd_get_cell_cfd_displ_diff(void *handle, int i_layer, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_get_cell_cfd_displ_diff:"
                         " access to cfd diffusion only possible in preparation mode", myid);
            return NULL;
        }

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return NULL;

        return _C(i_layer).cfd_displ_diff[i_phase]; // [i_cell][i_dim]
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) local diffusion data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_phase   specifies the desired phase
 * @return pointer to local diffusion data or NULL if handle or i_layer or i_phase is invalid
 */
double* rcfd_get_cell_local_diffusion(void *handle, int i_layer, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_get_cell_local_diffusion:"
                         " access to local diffusion only possible in preparation mode", myid);
            return NULL;
        }

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return NULL;

        return _C(i_layer).local_diff[0][i_phase]; // [i_cell]
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) cell crossing time array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_phase   specifies the desired phase
 * @return pointer to cell crossing times or NULL if handle, i_layer or i_phase is invalid
 */
double* rcfd_get_cell_crossing_time(void *handle, int i_layer, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return NULL;

        return _C(i_layer).crossing_time[i_phase]; // [i_cell]
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) cell average velocity array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_phase   specifies the desired phase
 * @return pointer to cell average velocities or NULL if handle, i_layer or i_phase is invalid
 */
double* rcfd_get_cell_ave_velocity(void *handle, int i_layer, int i_phase)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return NULL;

        return _C(i_layer).average_velocity[i_phase]; // [i_cell]
    }
#endif // RP_NODE

    return NULL;
}

// direct access to Face_type arrays
/**
 * @brief get (process-local) face c0 array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to face c0 or NULL if handle or i_layer is invalid
 */
int* rcfd_get_face_c0(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _F(i_layer).c0; //[i_face]
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) face c1 array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to face c1 or NULL if handle or i_layer is invalid
 */
int* rcfd_get_face_c1(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _F(i_layer).c1; //[i_face]
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief get (process-local) face area-vector array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to face area-vectors or NULL if handle or i_layer is invalid
 */
double** rcfd_get_face_area(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _F(i_layer).area; //[i_face][i_dim]
    }
#endif // RP_NODE

    return NULL;
}

// direct access to Node_type arrays
/**
 * @brief get (process-local) node position array
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @return pointer to node positions or NULL if handle or i_layer is invalid
 */
double** rcfd_get_node(void *handle, int i_layer)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers)
            return NULL;

        return _N(i_layer).x; //[i_node][i_dim]
    }
#endif // RP_NODE

    return NULL;
}

// direct access to Rec_type arrays
/**
 * @brief get jump state
 *
 * @param handle    pointer to previously created rCFD instance
 * @retval -1 Error
 * @retval 0 no jump
 * @retval 1 jumped
 */
int rcfd_get_rec_jumped_at_last_frame(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Rec->jumped_at_last_frame;
    }

    return -1;
}

/**
 * @brief get current sequence length
 *
 * @param handle    pointer to previously created rCFD instance
 * @return current sequence length or -1 if handle is invalid
 */
int rcfd_get_rec_sequence_length(void *handle)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        return rcfd->Rec->sequence_length;
    }

    return -1;
}

// indirect access to Balance_type members
/**
 * @brief set the mass source for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @param ms        mass source value
 */
void rcfd_set_balance_mass_source_one(void *handle, int i_phase, int i_data, double ms)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return;

        rcfd->Balance[i_phase][i_data].mass_source = ms;
    }
#endif // RP_NODE
}

/**
 * @brief set the mass source for all data of the given phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param ms        mass source value
 */
void rcfd_set_balance_mass_source_all(void *handle, int i_phase, double ms)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        {
            int i_data;

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Balance[i_phase][i_data].mass_source = ms;
            }
        }
    }
#endif // RP_NODE
}

/**
 * @brief set the mass source for each data of the given phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param ms        mass source values for each data
 */
void rcfd_set_balance_mass_source_each(void *handle, int i_phase, double *ms)
{
#if RP_NODE
    if(handle && ms){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        {
            int i_data;

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Balance[i_phase][i_data].mass_source = ms[i_data];
            }
        }
    }
#endif // RP_NODE
}

/**
 * @brief modify the mass source for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @param dms       increment value to add to specified mass source
 */
void rcfd_modify_balance_mass_source_one(void *handle, int i_phase, int i_data, double dms)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return;

        rcfd->Balance[i_phase][i_data].mass_source += dms;
    }
#endif // RP_NODE
}

/**
 * @brief modify all mass sources for the given phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param dms       increment value to add to all data mass sources
 */
void rcfd_modify_balance_mass_source_all(void *handle, int i_phase, double dms)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        {
            int i_data;

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Balance[i_phase][i_data].mass_source += dms;
            }
        }
    }
#endif // RP_NODE
}

/**
 * @brief modify each mass source for the given phase
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param dms       increment values to add to each data mass source
 */
void rcfd_modify_balance_mass_source_each(void *handle, int i_phase, double *dms)
{
#if RP_NODE
    if(handle && dms){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        {
            int i_data;

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Balance[i_phase][i_data].mass_source += dms[i_data];
            }
        }
    }
#endif // RP_NODE
}

/**
 * @brief set the (process-local) mass integral for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @param mi        (process-local) mass integral value
 */
void rcfd_set_balance_mass_integral(void *handle, int i_phase, int i_data, double mi)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return;

        rcfd->Balance[i_phase][i_data].mass_integral = mi;
    }
#endif // RP_NODE
}

/**
 * @brief set the (process-local) mass integral target for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @param mit       (process-local) mass integral target value
 */
void rcfd_set_balance_mass_integral_target(void *handle, int i_phase, int i_data, double mit)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return;

        rcfd->Balance[i_phase][i_data].mass_integral_target = mit;
    }
#endif // RP_NODE
}

/**
 * @brief set the global mass integral for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @param mig       global mass integral value
 */
void rcfd_set_balance_mass_integral_global(void *handle, int i_phase, int i_data, double mig)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return;

        rcfd->Balance[i_phase][i_data].mass_integral_global = mig;
    }
#endif // RP_NODE
}

/**
 * @brief set the global mass integral target for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @param mitg      global mass integral target value
 */
void rcfd_set_balance_mass_integral_target_global(void *handle, int i_phase, int i_data, double mitg)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return;

        rcfd->Balance[i_phase][i_data].mass_integral_target_global = mitg;
    }
#endif // RP_NODE
}

/**
 * @brief get the (process-local) mass integral for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @return (process-local) mass integral value or 0 if handle is invalid
 */
double rcfd_get_balance_mass_integral(void *handle, int i_phase, int i_data)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return 0.0;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return 0.0;

        return rcfd->Balance[i_phase][i_data].mass_integral;
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get the (process-local) mass integral target for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @return (process-local) mass integral target value or 0 if handle is invalid
 */
double rcfd_get_balance_mass_integral_target(void *handle, int i_phase, int i_data)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return 0.0;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return 0.0;

        return rcfd->Balance[i_phase][i_data].mass_integral_target;
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get the global mass integral for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @return global mass integral value or 0 if handle is invalid
 */
double rcfd_get_balance_mass_integral_global(void *handle, int i_phase, int i_data)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return 0.0;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return 0.0;

        return rcfd->Balance[i_phase][i_data].mass_integral_global;
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get the global mass integral target for the given phase and data
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the desired phase
 * @param i_data    specifies the desired data
 * @return global mass integral target value or 0 if handle is invalid
 */
double rcfd_get_balance_mass_integral_target_global(void *handle, int i_phase, int i_data)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases)
            return 0.0;

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data)
            return 0.0;

        return rcfd->Balance[i_phase][i_data].mass_integral_target_global;
    }
#endif // RP_NODE

    return 0.0;
}

/**
 * @brief get coarsening factor for norms, i.e. the reciprocal of the fraction
 *          of cells used for the calculation of the recurrence norm
 *
 * @param handle    pointer to previously created rCFD instance
 * @return norm coarsening factor or -1 if handle is invalid
 */
int rcfd_get_norm_coarsening(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_get_norm_coarsening:"
                         " access to norms only possible in preparation mode", myid);
            return -1;
        }

        return rcfd->Norm_Dict->coarse_graining;
    }
#endif

    return -1;
}

/**
 * @brief get the norm format
 * @param handle    pointer to previously created rCFD instance
 * @return the norm format or -1 if handle is invalid or not in preparation mode
 */
int rcfd_get_norm_format(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_get_norm_format:"
                         " access to norms only possible in preparation mode", myid);
            return -1;
        }

        return rcfd->Norm_Dict->format;
    }
#endif

    return -1;
}

/**
 * @brief get the (process-local) number of norms per frame; less than or equal to the
 *          (process-local) number of internal cells, depending on the norm coarsening;
 *          only available in preparation mode
 *
 * @param handle    pointer to previously created rCFD instance
 * @return the number of norms per frame or -1 if handle is invalid or not in preparation mode
 */
int rcfd_get_norm_count(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_get_norm_count:"
                         " access to norms only possible in preparation mode", myid);
            return -1;
        }

        return rcfd->Norms->number_of_norms;
    }
#endif

    return -1;
}

/**
 * @brief get (process-local) norm array, the length of the array can be obtained via
 *          rcfd_get_norm_count(); only available in preparation mode
 *
 * @param handle    pointer to previously created rCFD instance
 * @return pointer to norms or NULL if handle is invalid or not in preparation mode
 */
double *rcfd_get_norm(void *handle)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(rcfd->Solver_Dict->mode != RCFD_MODE_PREPARATION){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] rcfd_get_norm:"
                         " access to norms only possible in preparation mode", myid);
            return NULL;
        }

        return rcfd->Norms->norm;
    }
#endif

    return NULL;
}


#if defined(RCFD_LIB_TEST_MODULES)
/**
 * @brief run basic tests for the given module
 *
 * @param handle    pointer to previously created rCFD instance
 * @param module    name of module to test
 * @return 0 on success, else non-zero error code
 * @retval -1 invalid handle or module name
 */
int rcfd_test_module(void *handle, const char *module)
{
    int success = 0;
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(strcmp(module, "all") == 0){

            success |= test_memory(rcfd);
            success |= test_parallel(rcfd);
            success |= test_report(rcfd);
        }
        else if(strcmp(module, "memory") == 0){

            success |= test_memory(rcfd);
        }
        else if(strcmp(module, "parallel") == 0){

            success |= test_parallel(rcfd);
        }
        else if(strcmp(module, "report") == 0){

            success |= test_report(rcfd);
        }
        else if(strcmp(module, "topo") == 0){

            success |= test_topo(rcfd);
        }
        else if(strcmp(module, "dump") == 0){

            success |= test_dump(rcfd);
        }
        else{

            return -1;
        }
    }
    else{

        return -1;
    }
#endif // RP_NODE
    return success;
}
#endif // RCFD_LIB_TEST_MODULES

