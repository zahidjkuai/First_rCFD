/******************************************************************************
 * @internal
 * @file        rCFD_init.h
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_INIT
#define RCFD_INIT

#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define CREATE_DIR(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#define CREATE_DIR(path, mode) mkdir(path, mode)
#endif
#include <errno.h>
#define DIR_SEPARATOR "/"
#define DIR_SEPARATOR_CHAR '/'

#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_parallel.h"
#include "rCFD_defaults.h"
#include "rCFD_user_defaults.h"
#include "rCFD_layer.h"
#include "rCFD_report.h"
#include "rCFD_topo.h"
#include "rCFD_shift.h"

void init_user(rCFD_type *rcfd);
void init_parallel(rCFD_type *rcfd, MPI_Comm comm);
void init_report(rCFD_type *rcfd);
void init_config(rCFD_type *rcfd, const char *config_filename);

void test_layer_mapping(rCFD_type *rcfd);


rCFD_type* init_rcfd(MPI_Comm comm, const char *config_filename)
{
    rCFD_type *rcfd     = (rCFD_type*)malloc(sizeof(rCFD_type));

    rcfd->dict_write_permission_mask = 0;
    rcfd->initialized   = 0;
    rcfd->config        = NULL;

    rcfd->Report        = NULL;
    rcfd->File_Dict     = NULL;
    rcfd->Solver_Dict   = NULL;
    rcfd->Solver        = NULL;
    rcfd->Rec_Dict      = NULL;
    rcfd->Topo_Dict     = NULL;
    rcfd->Rec           = NULL;
    rcfd->rCFD_UDF      = NULL;
    rcfd->Parallel      = NULL;

#if RP_NODE
    rcfd->Phase_Dict    = NULL;
    rcfd->Norm_Dict     = NULL;
    rcfd->Data_Dict     = NULL;
    rcfd->Balance_Dict  = NULL;
    rcfd->Dump_Dict     = NULL;
    rcfd->Prep_Dict     = NULL;
    rcfd->C2C_Prep      = NULL;
    rcfd->C2C_Interface = NULL;
    rcfd->C2C           = NULL;
    rcfd->Topo          = NULL;
    rcfd->Norms         = NULL;
    rcfd->Balance       = NULL;
#endif

    init_user(rcfd);
    init_parallel(rcfd, comm);
    init_report(rcfd);
    init_config(rcfd, config_filename);

    return rcfd;
}

void init_user(rCFD_type *rcfd)
{
    if(rcfd->rCFD_UDF == NULL)
        rcfd->rCFD_UDF = (rCFD_UDF_type*)malloc(sizeof(rCFD_UDF_type));

    user_register_default_functions(rcfd);
}

void init_parallel(rCFD_type *rcfd, MPI_Comm comm)
{
    if(rcfd->Parallel == NULL)
        rcfd->Parallel = (Parallel_type*)malloc(sizeof(Parallel_type));

    parallel_init(rcfd, comm);
}

void init_report(rCFD_type *rcfd)
{
    if(rcfd->Report == NULL)
        rcfd->Report = (Report_type*)malloc(sizeof(Report_type));

    default_Report(rcfd);
}

void init_config(rCFD_type *rcfd, const char *config_filename)
{
    if(config_filename && strlen(config_filename) < RCFD_FILENAME_MAX){

        char *pch = strrchr(config_filename, '.');

        if(pch && strcmp(pch, ".toml") == 0){

            if(read_config_toml(rcfd, config_filename) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "init_config: failed to read config file '%s'",
                              config_filename);
            }
        }
        else{

            // potentially add other config file formats, e.g. json
            report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "init_config: '%s' is not a '.toml' file",
                          config_filename);
        }

        return;
    }

    // if no config filename is specified try default config filename
    read_config_toml(rcfd, RCFD_DEFAULT_CONFIG_FILENAME);
}

char* create_dir_name(const char *working_dir, const char *base_dir)
{
    char *dir_name = malloc (sizeof(char)
                             * (  strlen(working_dir) + 1
                                + strlen(base_dir) + 1
                               )
                            );
    strcpy(dir_name, working_dir);
    strcat(dir_name, DIR_SEPARATOR);
    strcat(dir_name, base_dir);

    return dir_name;
}

char* create_sub_dir_name(const char *working_dir, const char *base_dir, const char *sub_dir)
{
    char *dir_name = malloc (sizeof(char)
                             * (  strlen(working_dir) + 1
                                + strlen(base_dir) + 1
                                + strlen(sub_dir) + 1
                               )
                            );
    strcpy(dir_name, working_dir);
    strcat(dir_name, DIR_SEPARATOR);
    strcat(dir_name, base_dir);
    strcat(dir_name, DIR_SEPARATOR);
    strcat(dir_name, sub_dir);

    return dir_name;
}

int create_directory(rCFD_type *rcfd, const char *dir_name, unsigned int mode)
{
    int success = CREATE_DIR(dir_name, mode);

    if(success != 0){

        if(errno == EEXIST){

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "create_directory: failed to create directory %s - %s",
                          dir_name, strerror(errno));
            return 0;
        }
        else{

            report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "create_directory: failed to create directory %s - %s",
                          dir_name, strerror(errno));
            return -1;
        }
    }

    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                  "create_directory: created directory %s",
                  dir_name);

    return 0;
}

char* parent_path(const char *path)
{
    size_t len = strlen(path);

    if(len < 1)
        return NULL;

    char *sep = strrchr(path, DIR_SEPARATOR_CHAR);

    if(sep == NULL){

        return NULL;
    }
    else{

        int parent_len = (sep - path) + 1;
        char *parent = malloc(sizeof(char) * parent_len);
        strncpy(parent, path, parent_len-1);
        parent[parent_len-1] = 0;
        return parent;
    }
}

int create_directories(rCFD_type *rcfd, const char *path, unsigned int mode)
{
    if(create_directory(rcfd, path, mode) == 0)
        return 0;

    char* parent = parent_path(path);

    if(parent){

        int ret = create_directories(rcfd, parent, mode);

        free(parent);

        if(ret != 0)
            return -1;
    }

    return create_directory(rcfd, path, mode);
}

void create_folder_structure(rCFD_type *rcfd)
{
    /* Node-0 creates folder structure */
    if(myid == 0){

        int ret = 0;
#if defined(_WIN32) || defined(_WIN64)
        unsigned int permission_mode = 0777; // unused
#else
        unsigned int permission_mode = S_IRWXU | S_IRWXG | S_IRWXO;
#endif

        // strip trailing '/'
        size_t len = strlen(rcfd->File_Dict->working_dir);

        if(len > 0 && rcfd->File_Dict->working_dir[len-1] == DIR_SEPARATOR_CHAR)
            rcfd->File_Dict->working_dir[len-1] = 0;

        ret = create_directories(rcfd, rcfd->File_Dict->working_dir, permission_mode);

        if(ret != 0){

            report_error0(rcfd,
                          RCFD_REPORT_CATEGORY_INITIALIZE,
                          "failed to create working directory %s",
                          rcfd->File_Dict->working_dir);
            return;
        }

        char *dir_name = create_dir_name(rcfd->File_Dict->working_dir,
                                         rcfd->File_Dict->data_dir);
        ret = create_directory(rcfd, dir_name, permission_mode);
        free(dir_name);

        if(ret == 0){

            dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                           rcfd->File_Dict->data_dir,
                                           rcfd->File_Dict->tmp_sub_dir);
            create_directory(rcfd, dir_name, permission_mode);
            free(dir_name);

            dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                           rcfd->File_Dict->data_dir,
                                           rcfd->File_Dict->vof_sub_dir);
            create_directory(rcfd, dir_name, permission_mode);
            free(dir_name);

            dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                           rcfd->File_Dict->data_dir,
                                           rcfd->File_Dict->topo_sub_dir);
            create_directory(rcfd, dir_name, permission_mode);
            free(dir_name);

            dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                           rcfd->File_Dict->data_dir,
                                           rcfd->File_Dict->local_diff_sub_dir);
            create_directory(rcfd, dir_name, permission_mode);
            free(dir_name);

            dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                           rcfd->File_Dict->data_dir,
                                           rcfd->File_Dict->c2c_sub_dir);
            create_directory(rcfd, dir_name, permission_mode);
            free(dir_name);

            dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                           rcfd->File_Dict->data_dir,
                                           rcfd->File_Dict->rec_sub_dir);
            create_directory(rcfd, dir_name, permission_mode);
            free(dir_name);

            dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                           rcfd->File_Dict->data_dir,
                                           rcfd->File_Dict->user_sub_dir);
            create_directory(rcfd, dir_name, permission_mode);
            free(dir_name);
        }

        dir_name = create_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->post_dir);
        create_directory(rcfd, dir_name, permission_mode);
        free(dir_name);
    }
}

void recreate_filename(char *dir_name, char **filename)
{
    char *last_slash = strrchr(*filename, DIR_SEPARATOR_CHAR);
    char *base_name = last_slash ? last_slash + 1 : *filename;
    dir_name = s_realloc(dir_name, sizeof(char) * (strlen(dir_name) + 1 + strlen(base_name) + 1));

    if(dir_name){

        strcat(dir_name, DIR_SEPARATOR);
        strcat(dir_name, base_name);
        free(*filename);
        *filename = dir_name;
    }
}

void update_filenames(rCFD_type *rcfd)
{
    char *dir_name;

    dir_name = create_dir_name(rcfd->File_Dict->working_dir, rcfd->File_Dict->data_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->metadata_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->tmp_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->tracer_start_position_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->tmp_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->Norm_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->c2c_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->C2C_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->vof_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->vof_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->topo_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->Topo_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->local_diff_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->local_diff_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->user_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->rec_user_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->rec_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->Jump_filename);

    dir_name = create_sub_dir_name(rcfd->File_Dict->working_dir,
                                   rcfd->File_Dict->data_dir,
                                   rcfd->File_Dict->rec_sub_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->Matrix_filename);

    dir_name = create_dir_name(rcfd->File_Dict->working_dir, rcfd->File_Dict->post_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->Rec_Frames_filename);

    dir_name = create_dir_name(rcfd->File_Dict->working_dir, rcfd->File_Dict->post_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->Dump_filename);

    dir_name = create_dir_name(rcfd->File_Dict->working_dir, rcfd->File_Dict->post_dir);
    recreate_filename(dir_name, &rcfd->File_Dict->Probe_filename);
}

void init_file_dict(rCFD_type *rcfd, int mode)
{
    if(rcfd->File_Dict == NULL)
        rcfd->File_Dict = (File_Dict_type*)malloc(sizeof(File_Dict_type));

    default_File_Dict(rcfd);

    rcfd->dict_write_permission_mask = WRITE_FILE_DICT;
    read_file_dict_toml(rcfd);
    rcfd->rCFD_UDF->_user_set_file_dict(rcfd);
    rcfd->dict_write_permission_mask = 0;

    {
        char *report_filename =
                (mode == RCFD_MODE_RUN) ?
                    rcfd->File_Dict->Run_Transscript_filename :
                    rcfd->File_Dict->Prep_Transscript_filename;

        if(myid == 0)
            remove(report_filename);
        PRF_GSYNC();
        report_set_file0(rcfd, report_filename, "a");
    }

    report_print0(rcfd, RCFD_VERSION_STRING);

    if(myid == 0){

        time_t  current_time = time(NULL);
        char    *c_time_string = ctime(&current_time);

        report_print0(rcfd, "\n...rCFD_%s started at %s",
                      (mode == RCFD_MODE_RUN) ? "run" : "prep", c_time_string);
    }

    create_folder_structure(rcfd);
    update_filenames(rcfd);

    PRF_GSYNC();
}

void init_solver_dict(rCFD_type *rcfd, int mode)
{
    if(rcfd->Solver_Dict == NULL)
        rcfd->Solver_Dict = (Solver_Dict_type*)malloc(sizeof(Solver_Dict_type));

    default_Solver_Dict(rcfd);

    rcfd->Solver_Dict->mode = mode;

    rcfd->dict_write_permission_mask = WRITE_SOLVER_DICT;
    read_solver_dict_toml(rcfd);
    rcfd->rCFD_UDF->_user_set_solver_dict(rcfd);
    rcfd->dict_write_permission_mask = 0;

    report_set_priority(rcfd, rcfd->Solver_Dict->report_priority);
    report_toggle_timing(rcfd, rcfd->Solver_Dict->profiling_on);

    if(compute_node_count > 1 && rcfd->Solver_Dict->multifile_output_on){
        report_set_file(rcfd,
            (mode == RCFD_MODE_RUN) ?
                rcfd->File_Dict->Run_Transscript_filename :
                rcfd->File_Dict->Prep_Transscript_filename,
             "w");
    }

    if(mode == RCFD_MODE_PREPARATION){

        /*  TODO this might be changed in future,
            but at the moment we restrict ourselves to use
            just the base cell layer for the generation of c2c patterns */

        const short number_of_layers_for_preparation = 1;

        if(rcfd->Solver_Dict->number_of_layers > number_of_layers_for_preparation){

            rcfd->Solver_Dict->number_of_layers = number_of_layers_for_preparation;

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "reducing number of layers to %d for preparation step",
                            number_of_layers_for_preparation);
        }

        /* never use data drifting in preparation mode */
        if(rcfd->Solver_Dict->data_drifting_on != 0){

            rcfd->Solver_Dict->data_drifting_on = 0;

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "switching off data drifting for preparation step");
        }
    }
}

void init_solver(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Solver == NULL)
        rcfd->Solver = (Solver_type*)malloc(sizeof(Solver_type));

    rcfd->Solver->timestep_width = NULL;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        rcfd->Solver->timestep_width = malloc(rcfd->Solver_Dict->number_of_layers * sizeof(double));
    }

    default_Solver(rcfd);
#endif
}

void init_phase_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Phase_Dict == NULL)
        rcfd->Phase_Dict = (Phase_Dict_type*)malloc(rcfd->Solver_Dict->number_of_phases * sizeof(Phase_Dict_type));

    default_Phase_Dict(rcfd);

    rcfd->dict_write_permission_mask = WRITE_PHASE_DICT;
    read_phase_dict_toml(rcfd);
    rcfd->rCFD_UDF->_user_set_phase_dict(rcfd);
    rcfd->dict_write_permission_mask = 0;

    // has to be after user_set_phase_dict
    int i_phase, max_phase_data_count = 0;

    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

        if(max_phase_data_count < rcfd->Phase_Dict[i_phase].number_of_data){

            max_phase_data_count = rcfd->Phase_Dict[i_phase].number_of_data;
        }
    }

    rcfd->Solver_Dict->max_phase_data_count = max_phase_data_count;
#endif
}

void init_prep_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Prep_Dict == NULL)
        rcfd->Prep_Dict = (Prep_Dict_type*)malloc(sizeof(Prep_Dict_type));

    rcfd->Prep_Dict->tracer_random_walk_on = NULL;
    rcfd->Prep_Dict->tracer_random_walk_velocity_ratio = NULL;

    if(rcfd->Solver_Dict->shifts_from_displacement_fields_on == 0){

        int phase_count = rcfd->Solver_Dict->number_of_phases;

        rcfd->Prep_Dict->tracer_random_walk_on =             malloc(phase_count * sizeof(short));
        rcfd->Prep_Dict->tracer_random_walk_velocity_ratio = malloc(phase_count * sizeof(double));
    }

    default_Prep_Dict(rcfd);

    rcfd->dict_write_permission_mask = WRITE_PREP_DICT;
    read_prep_dict_toml(rcfd);
    rcfd->rCFD_UDF->_user_set_prep_dict(rcfd);
    rcfd->dict_write_permission_mask = 0;
#endif
}

void init_norm_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Norm_Dict == NULL)
        rcfd->Norm_Dict = (Norm_Dict_type*)malloc(sizeof(Norm_Dict_type));

    default_Norm_Dict(rcfd);

    if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

        rcfd->dict_write_permission_mask = WRITE_NORM_DICT;
        read_norm_dict_toml(rcfd);
        rcfd->rCFD_UDF->_user_set_norm_dict(rcfd);
        rcfd->dict_write_permission_mask = 0;
    }
#endif
}

void init_rec_dict(rCFD_type *rcfd)
{
    if(rcfd->Rec_Dict == NULL)
        rcfd->Rec_Dict = (Rec_Dict_type*)malloc(sizeof(Rec_Dict_type));

    default_Rec_Dict(rcfd);

    rcfd->dict_write_permission_mask = WRITE_REC_DICT;
    read_rec_dict_toml(rcfd);
    rcfd->rCFD_UDF->_user_set_rec_dict(rcfd);
    rcfd->dict_write_permission_mask = 0;
}

void init_data_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        int i_phase, i_data;

        rcfd->Data_Dict = (Data_Dict_type**)malloc(rcfd->Solver_Dict->number_of_phases * sizeof(Data_Dict_type*));

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            rcfd->Data_Dict[i_phase] = (Data_Dict_type*)malloc(rcfd->Phase_Dict[i_phase].number_of_data * sizeof(Data_Dict_type));
        }

        default_Data_Dict(rcfd);

        rcfd->dict_write_permission_mask = WRITE_DATA_DICT;
        read_data_dict_toml(rcfd);
        rcfd->rCFD_UDF->_user_set_data_dict(rcfd);
        rcfd->dict_write_permission_mask = 0;

        /* adapt Phase_Dict because of user Data_Dict input */
        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            rcfd->Phase_Dict[i_phase].number_of_concentration_data = 0;

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                if(rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_CONCENTRATION){

                    rcfd->Phase_Dict[i_phase].number_of_concentration_data++;
                }
            }
        }

        /* add Data_Dict.d2_diam, d3_diam because of Data_Dict.d1_diam */
        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Data_Dict[i_phase][i_data].d2_diam =    rcfd->Data_Dict[i_phase][i_data].d1_diam
                                                            * rcfd->Data_Dict[i_phase][i_data].d1_diam;

                rcfd->Data_Dict[i_phase][i_data].d3_diam =    rcfd->Data_Dict[i_phase][i_data].d2_diam
                                                            * rcfd->Data_Dict[i_phase][i_data].d1_diam;
            }
        }
    }
#endif
}

void init_balance_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        int i_phase;

        rcfd->Balance_Dict = (Balance_Dict_type**)malloc(rcfd->Solver_Dict->number_of_phases * sizeof(Balance_Dict_type*));

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            rcfd->Balance_Dict[i_phase] = (Balance_Dict_type*)malloc(rcfd->Phase_Dict[i_phase].number_of_data * sizeof(Balance_Dict_type));
        }

        default_Balance_Dict(rcfd);

        rcfd->dict_write_permission_mask = WRITE_BALANCE_DICT;
        read_balance_dict_toml(rcfd);
        rcfd->rCFD_UDF->_user_set_balance_dict(rcfd);
        rcfd->dict_write_permission_mask = 0;
    }
#endif
}

void init_topo_dict(rCFD_type *rcfd)
{
    if(rcfd->Topo_Dict == NULL)
        rcfd->Topo_Dict = (Topo_Dict_type*)malloc(sizeof(Topo_Dict_type));

    default_Topo_Dict(rcfd);

    rcfd->dict_write_permission_mask = WRITE_TOPO_DICT;
    rcfd->rCFD_UDF->_user_set_topo_dict(rcfd);
    rcfd->dict_write_permission_mask = 0;

#if RP_NODE

    // cells
    rcfd->Topo_Dict->Cell_Dict = (Cell_Dict_type*)malloc(rcfd->Solver_Dict->number_of_layers * sizeof(Cell_Dict_type));

    default_Cell_Dict(rcfd);

    int i_layer = 0;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN) _CELL_DICT(i_layer).number_of_cells = topo_read_cell_count(rcfd);

    rcfd->dict_write_permission_mask = WRITE_CELL_DICT;
    read_cell_dict_toml(rcfd);
    for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

        rcfd->rCFD_UDF->_user_set_cell_dict(rcfd, i_layer);
    }
    rcfd->dict_write_permission_mask = 0;

    // faces
    i_layer = 0;

    rcfd->Topo_Dict->Face_Dict = (Face_Dict_type*)malloc(rcfd->Solver_Dict->number_of_layers * sizeof(Face_Dict_type));

    default_Face_Dict(rcfd);

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN) _FACE_DICT(i_layer).number_of_faces = topo_read_face_count(rcfd);

    rcfd->dict_write_permission_mask = WRITE_FACE_DICT;
    read_face_dict_toml(rcfd);
    for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

        rcfd->rCFD_UDF->_user_set_face_dict(rcfd, i_layer);
    }
    rcfd->dict_write_permission_mask = 0;

    // nodes
    i_layer = 0;

    rcfd->Topo_Dict->Node_Dict = (Node_Dict_type*)malloc(rcfd->Solver_Dict->number_of_layers * sizeof(Node_Dict_type));

    default_Node_Dict(rcfd);

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN) _NODE_DICT(i_layer).number_of_nodes = topo_read_node_count(rcfd);

    rcfd->dict_write_permission_mask = WRITE_NODE_DICT;
    read_node_dict_toml(rcfd);
    for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

        rcfd->rCFD_UDF->_user_set_node_dict(rcfd, i_layer);
    }
    rcfd->dict_write_permission_mask = 0;

#endif
}

void init_dump_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Dump_Dict == NULL)
        rcfd->Dump_Dict = (Dump_Dict_type*)malloc(sizeof(Dump_Dict_type));

    {
        int i_phase;
        int i_layer = 0;

        rcfd->Dump_Dict->dump_cell_data = (short**)malloc(rcfd->Solver_Dict->number_of_phases * sizeof(short**));

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            if(rcfd->Phase_Dict[i_phase].number_of_data > 0){

                rcfd->Dump_Dict->dump_cell_data[i_phase] = (short*)malloc(rcfd->Phase_Dict[i_phase].number_of_data * sizeof(short*));
            }
            else{

                rcfd->Dump_Dict->dump_cell_data[i_phase] = NULL;
            }
        }

        if(_CELL_DICT(i_layer).number_of_user_vars > 0){

            rcfd->Dump_Dict->dump_user_data = (short*)malloc(_CELL_DICT(i_layer).number_of_user_vars * sizeof(short*));
        }
        else{

            rcfd->Dump_Dict->dump_user_data = NULL;
        }

        if(_CELL_DICT(i_layer).number_of_rec_user_vars > 0){

            rcfd->Dump_Dict->dump_rec_user_data = (short*)malloc(_CELL_DICT(i_layer).number_of_rec_user_vars * sizeof(short*));
        }
        else{

            rcfd->Dump_Dict->dump_rec_user_data = NULL;
        }

        default_Dump_Dict(rcfd);

        rcfd->dict_write_permission_mask = WRITE_DUMP_DICT;
        read_dump_dict_toml(rcfd);
        rcfd->rCFD_UDF->_user_set_dump_dict(rcfd);
        rcfd->dict_write_permission_mask = 0;

        if(rcfd->Dump_Dict->probe_count > 0){

            if(myid == 0){

                remove(rcfd->File_Dict->Probe_filename);
            }
        }
    }
#endif
}

void init_topo(rCFD_type *rcfd)
{
#if RP_NODE
    int i_layer;

    if(rcfd->Topo == NULL)
        rcfd->Topo = (Topo_type*)malloc(sizeof(Topo_type));

    default_Topo(rcfd);

    rcfd->Topo->Cell = (Cell_type*)malloc(rcfd->Solver_Dict->number_of_layers * sizeof(Cell_type));
    rcfd->Topo->Face = (Face_type*)malloc(rcfd->Solver_Dict->number_of_layers * sizeof(Face_type));
    rcfd->Topo->Node = (Node_type*)malloc(rcfd->Solver_Dict->number_of_layers * sizeof(Node_type));

    i_layer = 0;

    /* allocate cells for L0 */
    if(_CELL_DICT(i_layer).number_of_cells > 0){

        layer_allocate(rcfd, i_layer);
    }
    else{

        layer_default(rcfd, i_layer);

        report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                     "[%d] init_topo: _CELL_DICT(%d).number_of_cells < 1 ...", myid, i_layer);
    }

    /* allocate faces for L0 */
    if(_FACE_DICT(i_layer).number_of_faces > 0){

        _F(i_layer).c0 = (int*)malloc(_FACE_DICT(i_layer).number_of_faces * sizeof(int));
        _F(i_layer).c1 = (int*)malloc(_FACE_DICT(i_layer).number_of_faces * sizeof(int));
        _F(i_layer).area = malloc_r_2d(_FACE_DICT(i_layer).number_of_faces, 3);

        if(rcfd->Solver_Dict->data_drifting_on){ // only ever on in run mode

            int i_phase;
            int max_number_of_data = 0;

            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                if(max_number_of_data < rcfd->Phase_Dict[i_phase].number_of_data){

                    max_number_of_data = rcfd->Phase_Dict[i_phase].number_of_data;
                }
            }

            _F(i_layer).local_drift_adaption = malloc_r_2d(_FACE_DICT(i_layer).number_of_faces, max_number_of_data);
        }
        else{

            _F(i_layer).local_drift_adaption = NULL;
        }
    }
    else{

        _F(i_layer).c0 = NULL;
        _F(i_layer).c1 = NULL;
        _F(i_layer).area = NULL;
        _F(i_layer).local_drift_adaption = NULL;

        report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                     "[%d] init_topo: _FACE_DICT(%d).number_of_faces < 1 ...", myid, i_layer);
    }


    /* allocate nodes for L0 */
    {
        if(_NODE_DICT(i_layer).number_of_nodes > 0){

            _N(i_layer).x = malloc_r_2d(_NODE_DICT(i_layer).number_of_nodes, 3);
        }
        else{

            _N(i_layer).x = NULL;
        }
    }

    /* read cell, face and node topology from topo file (if present) */
    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        topo_read(rcfd);
    }

    /* set cell default values for L0 */
    {
        rcfd->rCFD_UDF->_user_init_cells(rcfd, i_layer);

        default_Cell_L0(rcfd);
    }

    /* set face default values for L0 */
    {
        rcfd->rCFD_UDF->_user_init_faces(rcfd, i_layer);

        default_Face_L0(rcfd);
    }

    /* set node default values for L0 */
    {
        rcfd->rCFD_UDF->_user_init_nodes(rcfd, i_layer);
    }

    /* 6. set nb cells and bounding box coords for L0 */
    {
        topo_init_neigh_cells(rcfd, i_layer);

        topo_init_bounding_box(rcfd, i_layer);
    }

    /* 7. set pointers of upper grid layers to NULL */
    if(rcfd->Solver_Dict->number_of_layers > 1){ // only ever > 1 in run mode

        for(i_layer = 1; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

            layer_default(rcfd, i_layer);

            _F(i_layer).c0                          = NULL;
            _F(i_layer).c1                          = NULL;

            _F(i_layer).area                        = NULL;

            _F(i_layer).local_drift_adaption        = NULL;

            _N(i_layer).x                           = NULL;
        }
    }
#endif
}

void init_c2c_prep(rCFD_type *rcfd)
{
#if RP_NODE
    rcfd->C2C_Prep = NULL;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

        int i_phase;
        int i_prep_layer = rcfd->Solver->current_layer;

        int phase_count = rcfd->Solver_Dict->number_of_phases;
        int int_cell_count = _CELL_DICT(i_prep_layer).number_of_int_cells;
        int tracer_per_cell_count = rcfd->Prep_Dict->tracers_per_cell_count;
        int shifts_count;

        rcfd->C2C_Prep = malloc(phase_count * sizeof(C2C_Shift_type));

        if(rcfd->Solver_Dict->shifts_from_displacement_fields_on){

            shifts_count = 4 * int_cell_count;
        }
        else{ // tracer_based shifts

            shifts_count = 2 * compute_node_count * int_cell_count * tracer_per_cell_count;
        }

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            shift_allocate_lists(rcfd, &rcfd->C2C_Prep[i_phase], shifts_count, RCFD_SHIFT_TYPE_PREPARATION);
        }

        default_c2c_prep(rcfd);
    }

#endif
}

void init_c2c_interface(rCFD_type *rcfd)
{
#if RP_NODE
    rcfd->C2C_Interface = NULL;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

        rcfd->C2C_Interface = malloc(sizeof(C2C_Shift_type));

        shift_initialize_lists(rcfd, rcfd->C2C_Interface);
    }

#endif
}

void init_rec(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Rec == NULL)
        rcfd->Rec = (Rec_type*)malloc(sizeof(Rec_type));

    rcfd->Rec->jumps = NULL;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        int state_count = rcfd->Solver_Dict->number_of_states;
        int frame_count = rcfd->Solver_Dict->number_of_frames;

        rcfd->Rec->jumps = malloc_i_3d(state_count, state_count, frame_count);

        default_Rec(rcfd);
    }
#endif
}

void init_norms(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Norms == NULL)
        rcfd->Norms = (Norm_type*)malloc(sizeof(Norm_type));

    rcfd->Norms->norm = NULL;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

#ifndef RCFD_STANDALONE
        Domain  *d=Get_Domain(1);
        Thread  *t;
        cell_t  i_cell;

        int     number_of_norms = 0;

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){begin_c_loop_int(i_cell,t){

            if((i_cell % rcfd->Norm_Dict->coarse_graining) == 0){

                number_of_norms++;
            }

        }end_c_loop_int(i_cell,t)}}
#else
        int i_layer = 0;

        int number_of_norms = _CELL_DICT(i_layer).number_of_int_cells/rcfd->Norm_Dict->coarse_graining;
#endif

        rcfd->Norms->number_of_norms = number_of_norms;

        rcfd->Norms->norm = (double*)malloc(number_of_norms * sizeof(double));

        default_Norms(rcfd);
    }

#endif
}

void init_balance(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        int i_phase, i_data;

        rcfd->Balance = (Balance_type**)malloc(rcfd->Solver_Dict->number_of_phases * sizeof(Balance_type*));

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            rcfd->Balance[i_phase] = (Balance_type*)malloc(rcfd->Phase_Dict[i_phase].number_of_data * sizeof(Balance_type));

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Balance[i_phase][i_data].mass_integral = 0.0;

                rcfd->Balance[i_phase][i_data].mass_integral_target = rcfd->Balance[i_phase][i_data].mass_integral;

                rcfd->Balance[i_phase][i_data].mass_integral_global = PRF_GRSUM1(rcfd->Balance[i_phase][i_data].mass_integral);

                rcfd->Balance[i_phase][i_data].mass_integral_target_global = rcfd->Balance[i_phase][i_data].mass_integral_global;

                rcfd->Balance[i_phase][i_data].mass_source = 0.0;

                rcfd->Balance[i_phase][i_data].mass_source_global = 0.0;

                rcfd->Balance[i_phase][i_data].mass_error = 0.0;

                rcfd->Balance[i_phase][i_data].mass_error_global = 0.0;
            }
        }
    }
#endif
}

void init_data(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        int i_layer, i_frame, i_phase, i_data, i_cell;

        i_layer = 0;    /* user initialization just for base layer of grid, might be changed to loop_layers in future */

        rcfd->rCFD_UDF->_user_init_data(rcfd, i_layer);

        i_frame = 0;

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

             for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                    switch (rcfd->Data_Dict[i_phase][i_data].type){

                        case RCFD_DATA_GENERIC:
                            rcfd->Balance[i_phase][i_data].mass_integral += _C(i_layer).data[i_phase][i_cell][i_data];
                            break;
                        case RCFD_DATA_CONCENTRATION:
                            rcfd->Balance[i_phase][i_data].mass_integral +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                                            * rcfd->Phase_Dict[i_phase].density
                                                                            * _C(i_layer).volume[i_cell]
                                                                            * _C(i_layer).vof[i_frame][i_cell][i_phase];
                            break;
                        case RCFD_DATA_TEMPERATURE:
                            rcfd->Balance[i_phase][i_data].mass_integral +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                                            * rcfd->Phase_Dict[i_phase].heat_capacity
                                                                            * rcfd->Phase_Dict[i_phase].density
                                                                            * _C(i_layer).volume[i_cell]
                                                                            * _C(i_layer).vof[i_frame][i_cell][i_phase];
                            break;
                        default:
                            break;
                    }
                }
            }
        }


        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Balance[i_phase][i_data].mass_integral_target = rcfd->Balance[i_phase][i_data].mass_integral;

                rcfd->Balance[i_phase][i_data].mass_integral_global = PRF_GRSUM1(rcfd->Balance[i_phase][i_data].mass_integral);

                rcfd->Balance[i_phase][i_data].mass_integral_target_global = rcfd->Balance[i_phase][i_data].mass_integral_global;
            }
        }

        rcfd->rCFD_UDF->_user_post(rcfd);    /* post-process initialization */
    }
#endif
}

void init_coarse_grid_layers(rCFD_type *rcfd)
{
    if(rcfd->Solver_Dict->number_of_layers > 1){
#if RP_NODE

#if 1   /* local vars */

        short   debug_this_code = 0;
        int     i_layer, i_cell, i_face, i_cell_face, i_face2, i_cell_face2, i_child, i_dim;
        int     i_tmp, i_tmp2, i_parent_face, i_redist, i_phase, i_frame;
        int     cell_index_in_upper_layer, number_of_cells_in_upper_layer, upper_cell, c0, c1, c1_c0, c1_c1;
        int     min_dist_parent_cell, max_number_of_faces_per_cell, max_number_of_children, min_number_of_children;
        int     number_of_unassigned_parents, number_of_unassigned_children, number_of_negative_child_indices;
        double  dist, min_dist, x_cluster[3];
        int     **tmp_faces_of_cell = NULL;     /* [i_cell (i_layer)][i_cell_face] */
        int     parent_face_count, upper_c0, upper_c1;
        int     *tmp_parent_face_index = NULL;  /* [i_face] */
        int     **tmp_parent_cell_cell_index = NULL;    /* [i_cell ((i_layer + 1))][i_cell_face] */
        int     **tmp_parent_cell_face_index = NULL;
        short   *tmp_cluster_already_visited = NULL;

        enum{
            not_yet_visited,
            center_of_cluster,
            member_of_cluster,
            neighbor_of_cluster
        };
#endif

        i_layer = 0;

        while((i_layer + 1) < rcfd->Solver_Dict->number_of_layers)
        {
            /* 1 create temporary faces_of_cell structure (tmp_faces_of_cell) */
            {
                tmp_faces_of_cell = malloc_i_2d(_CELL_DICT(i_layer).number_of_cells, rcfd->Solver_Dict->max_number_of_faces_per_cell);

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    for(i_cell_face = 0; i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell; i_cell_face++){

                        tmp_faces_of_cell[i_cell][i_cell_face] = -1;
                    }
                }

                max_number_of_faces_per_cell = 0;

                for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                    c0 = _F(i_layer).c0[i_face];
                    c1 = _F(i_layer).c1[i_face];

                    i_cell_face = 0;

                    while(i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell){

                        if(tmp_faces_of_cell[c0][i_cell_face] < 0){

                            tmp_faces_of_cell[c0][i_cell_face] = i_face;

                            if((i_cell_face + 1) > max_number_of_faces_per_cell){

                                max_number_of_faces_per_cell = (i_cell_face + 1);
                            }

                            i_cell_face = 2 * rcfd->Solver_Dict->max_number_of_faces_per_cell;
                        }
                        else{

                            i_cell_face++;
                        }
                    }

                    if(i_cell_face == rcfd->Solver_Dict->max_number_of_faces_per_cell){

                        report_warning(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                       "[%d] init layers, L1: i_layer %d c0 %d i_cell_face %d",
                                       myid, i_layer, c0, i_cell_face);
                    }


                    i_cell_face = 0;

                    while(i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell){

                        if(tmp_faces_of_cell[c1][i_cell_face] < 0){

                            tmp_faces_of_cell[c1][i_cell_face] = i_face;

                            if((i_cell_face + 1) > max_number_of_faces_per_cell){

                                max_number_of_faces_per_cell = (i_cell_face + 1);
                            }

                            i_cell_face = 2 * rcfd->Solver_Dict->max_number_of_faces_per_cell;
                        }
                        else{

                            i_cell_face++;
                        }
                    }

                    if(i_cell_face == rcfd->Solver_Dict->max_number_of_faces_per_cell){

                        report_warning(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                       "[%d] init layers, L1: i_layer %d c1 %d i_cell_face %d",
                                       myid, i_layer, c1, i_cell_face);
                    }

                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] L.1 i_layer %d max_number_of_faces_per_cell %d",
                                 myid, i_layer, max_number_of_faces_per_cell);
                }
            }

            /* 2 create cell clusters, set number of upper layer cells */
            {
                /* initialize _C(i_layer).marked and _C(i_layer).parent_cell */
                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    _C(i_layer).marked[i_cell] = not_yet_visited;
                    _C(i_layer).parent_cell[i_cell] = -1;
                }

                cell_index_in_upper_layer = 0;

                /* mark cell clusters, get number of upper layer cells */
                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    if(_C(i_layer).marked[i_cell] == not_yet_visited){

                        _C(i_layer).marked[i_cell] = center_of_cluster;

                        _C(i_layer).parent_cell[i_cell] = cell_index_in_upper_layer;

                        /* mark neighbors as members_of_cluster */
                        {
                            i_cell_face = 0;

                            while((tmp_faces_of_cell[i_cell][i_cell_face] > -1)
                                  && (i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell)){

                                i_face = tmp_faces_of_cell[i_cell][i_cell_face];

                                c0 = _F(i_layer).c0[i_face];
                                c1 = _F(i_layer).c1[i_face];

                                if(_C(i_layer).marked[c1] == center_of_cluster){

                                    i_tmp = c1; c1 = c0; c0 = i_tmp;
                                }

                                _C(i_layer).marked[c1] = member_of_cluster;

                                _C(i_layer).parent_cell[c1] = cell_index_in_upper_layer;

                                /* mark neighbors of neighbor as neighbors_of_cluster */
                                {
                                    i_cell_face2 = 0;

                                    while((tmp_faces_of_cell[c1][i_cell_face2] > -1)
                                          && (i_cell_face2 < rcfd->Solver_Dict->max_number_of_faces_per_cell)){

                                        i_face2 = tmp_faces_of_cell[c1][i_cell_face2];

                                        c1_c0 = _F(i_layer).c0[i_face2];
                                        c1_c1 = _F(i_layer).c1[i_face2];

                                        if(_C(i_layer).marked[c1_c0] == not_yet_visited){

                                            _C(i_layer).marked[c1_c0] = neighbor_of_cluster;
                                        }
                                        else if(_C(i_layer).marked[c1_c1] == not_yet_visited){

                                            _C(i_layer).marked[c1_c1] = neighbor_of_cluster;
                                        }

                                        i_cell_face2++;
                                    }
                                }

                                i_cell_face++;
                            }
                        }

                        cell_index_in_upper_layer++;
                    }
                }

                number_of_cells_in_upper_layer = cell_index_in_upper_layer;

                _CELL_DICT(i_layer + 1).number_of_cells     = number_of_cells_in_upper_layer;
                _CELL_DICT(i_layer + 1).number_of_int_cells = number_of_cells_in_upper_layer;
                _CELL_DICT(i_layer + 1).number_of_ext_cells = 0;

                if(debug_this_code){

                    i_tmp = 0;

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        if(_C(i_layer).marked[i_cell] == neighbor_of_cluster){

                            i_tmp++;
                        }
                    }

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] L.2 Topo_Dict.Cell_Dict[%d].number_of_cells %d; unused neighbors %d",
                                 myid, (i_layer + 1), _CELL_DICT(i_layer + 1).number_of_cells, i_tmp);
                }
            }

            /* 3 allocate and initialize upper layer grid vars */
            {
                layer_allocate(rcfd, (i_layer + 1));

                /* initialize multi-layer vars */
                for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        _C(i_layer + 1).x[i_cell][i_dim] = 0.0;
                    }

                    _C(i_layer + 1).volume[i_cell] = 0.0;

                    _C(i_layer + 1).number_of_children[i_cell] = 0;
                }

                for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                            _C(i_layer + 1).vof[i_frame][i_cell][i_phase] = 1.0;
                        }
                    }
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] L.3 i_layer %d", myid, i_layer);
                }
            }

            /* 4 fill upper layer grid vars (by first set of clusters) */
            {
                max_number_of_children = 0;
                min_number_of_children = 9999;

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    if((_C(i_layer).marked[i_cell] == center_of_cluster) || (_C(i_layer).marked[i_cell] == member_of_cluster)){

                        upper_cell = _C(i_layer).parent_cell[i_cell];

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            _C(i_layer + 1).x[upper_cell][i_dim] += _C(i_layer).x[i_cell][i_dim];
                        }

                        _C(i_layer + 1).volume[upper_cell] += _C(i_layer).volume[i_cell];

                        _C(i_layer + 1).number_of_children[upper_cell] += 1;

                        if(_C(i_layer + 1).number_of_children[upper_cell] > max_number_of_children){

                            max_number_of_children = _C(i_layer + 1).number_of_children[upper_cell];
                        }

                        if(_C(i_layer + 1).number_of_children[upper_cell] < min_number_of_children){

                            min_number_of_children = _C(i_layer + 1).number_of_children[upper_cell];
                        }
                    }
                }

                /* average coordinates */
                for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                    if(_C(i_layer + 1).number_of_children[i_cell] > 0){

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            _C(i_layer + 1).x[i_cell][i_dim] /= (double)_C(i_layer + 1).number_of_children[i_cell];
                        }
                    }
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] L.4 i_layer %d, max/min_number_of_children %d/%d",
                                 myid, i_layer, max_number_of_children, min_number_of_children);
                }
            }

            /* 5 add still unassigned cells */
            {
                i_tmp2 = 0;

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    if(_C(i_layer).marked[i_cell] == neighbor_of_cluster){

                        i_cell_face = 0;

                        min_dist = 1.0e10;

                        min_dist_parent_cell = 0;

                        while((tmp_faces_of_cell[i_cell][i_cell_face] > -1)
                              && (i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell)){

                            i_face = tmp_faces_of_cell[i_cell][i_cell_face];

                            c0 = _F(i_layer).c0[i_face];
                            c1 = _F(i_layer).c1[i_face];

                            if(c1 == i_cell){

                                i_tmp = c1; c1 = c0; c0 = i_tmp;
                            }

                            if(    (_C(i_layer).marked[c1] == center_of_cluster)
                                || (_C(i_layer).marked[c1] == member_of_cluster)){

                                /* calc min distance to nearest cluster midpoint */

                                upper_c1 = _C(i_layer).parent_cell[c1];

                                for(i_dim = 0; i_dim < 3; i_dim++){

                                    x_cluster[i_dim] = _C(i_layer + 1).x[upper_c1][i_dim];
                                }

                                dist = (x_cluster[0] - _C(i_layer).x[c1][0])*(x_cluster[0] - _C(i_layer).x[c1][0]) +
                                       (x_cluster[1] - _C(i_layer).x[c1][1])*(x_cluster[1] - _C(i_layer).x[c1][1]) +
                                       (x_cluster[2] - _C(i_layer).x[c1][2])*(x_cluster[2] - _C(i_layer).x[c1][2]);

                                if(dist < min_dist){

                                    min_dist = dist;

                                    min_dist_parent_cell = _C(i_layer).parent_cell[c1];
                                }
                            }

                            i_cell_face++;
                        }

                        upper_cell = min_dist_parent_cell;

                        _C(i_layer).marked[i_cell] = member_of_cluster;

                        _C(i_layer).parent_cell[i_cell] = upper_cell;

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            _C(i_layer + 1).x[upper_cell][i_dim] =  (
                                                                        _C(i_layer).x[i_cell][0]
                                                                      + (double)_C(i_layer + 1).number_of_children[upper_cell]
                                                                      * _C(i_layer + 1).x[upper_cell][i_dim]
                                                                    )
                                                                    /
                                                                    (
                                                                        (double)_C(i_layer + 1).number_of_children[upper_cell] + 1.0
                                                                    );
                        }

                        _C(i_layer + 1).volume[upper_cell] += _C(i_layer).volume[i_cell];

                        _C(i_layer + 1).number_of_children[upper_cell] += 1;

                        if(_C(i_layer + 1).number_of_children[upper_cell] > max_number_of_children){

                            max_number_of_children = _C(i_layer + 1).number_of_children[upper_cell];
                        }

                        i_tmp2++;
                    }
                }

                /* Warning, if unassigned cells exist */
                {
                    i_tmp = 0;

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        if(_C(i_layer).marked[i_cell] == neighbor_of_cluster){

                            i_tmp++;
                        }
                    }

                    if(i_tmp > 0){

                        report_warning(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                       "[%d] Initialize layers, %d unassigned cells exist in layer %d\n",
                                       myid, i_tmp, i_layer);
                    }
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] L.5 i_layer %d, added cells %d unassigned cells %d max_number_of_children %d",
                                 myid, i_layer, i_tmp2, i_tmp, max_number_of_children);
                }

            }

            /* 5.a fill upper layer vof */
            {
                /* init vof((i_layer + 1)) */
                for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        if(_C(i_layer).parent_cell[i_cell] > -1){

                            upper_cell = _C(i_layer).parent_cell[i_cell];

                            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                                _C(i_layer + 1).vof[i_frame][upper_cell][i_phase] = 0.0;
                            }
                        }
                    }
                }

                /* accu. map vof((i_layer + 1)) */
                for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        if(_C(i_layer).parent_cell[i_cell] > -1){

                            upper_cell = _C(i_layer).parent_cell[i_cell];

                            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                                _C(i_layer + 1).vof[i_frame][upper_cell][i_phase] +=  _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                                                    * _C(i_layer).volume[i_cell];
                            }
                        }
                    }
                }

                /* average vof((i_layer + 1)) */
                for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                        if(_C(i_layer + 1).volume[i_cell] > 0){

                            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                                _C(i_layer + 1).vof[i_frame][i_cell][i_phase] /= _C(i_layer + 1).volume[i_cell];
                            }
                        }
                    }
                }
            }

            /* 6 fill upper layer list of child_index */
            {
                /* allocate upper layer list of child_index */
                for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                    _C(i_layer + 1).child_index[i_cell] = (int*)malloc(_C(i_layer + 1).number_of_children[i_cell] * sizeof(int));

                    for(i_child = 0; i_child < _C(i_layer + 1).number_of_children[i_cell]; i_child++){

                        _C(i_layer + 1).child_index[i_cell][i_child] = -1;
                    }
                }

                /* fill upper layer list of child_index */

                number_of_unassigned_parents =      0;
                number_of_unassigned_children =     0;
                number_of_negative_child_indices =  0;

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    if(_C(i_layer).parent_cell[i_cell] > -1){

                        upper_cell = _C(i_layer).parent_cell[i_cell];

                        i_child = 0;

                        while(i_child < _C(i_layer + 1).number_of_children[upper_cell]){

                            if(_C(i_layer + 1).child_index[upper_cell][i_child] < 0){

                                _C(i_layer + 1).child_index[upper_cell][i_child] = i_cell;

                                i_child =  2 * _C(i_layer + 1).number_of_children[upper_cell];
                            }
                            else{

                                i_child++;
                            }
                        }

                        if(i_child == _C(i_layer + 1).number_of_children[upper_cell]){

                            number_of_unassigned_children++;
                        }
                    }
                    else{

                        number_of_unassigned_parents++;
                    }
                }

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                    for(i_child = 0; i_child < _C(i_layer + 1).number_of_children[i_cell]; i_child++){

                        if(_C(i_layer + 1).child_index[i_cell][i_child] == -1){

                            number_of_negative_child_indices++;
                        }
                    }
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] L.6 i_layer %d number_of_unassigned_parents %d, "
                                 "number_of_unassigned_children %d neg_child_index %d",
                                 myid, i_layer, number_of_unassigned_parents,
                                 number_of_unassigned_children, number_of_negative_child_indices);

                    if(myid == 0){

                        FILE *f_out = fopen("tmp", "w");

                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                            fprintf(f_out, "%d :", _C(i_layer + 1).number_of_children[i_cell]);

                            for(i_child = 0; i_child < _C(i_layer + 1).number_of_children[i_cell]; i_child++){

                                fprintf(f_out, " %d", _C(i_layer + 1).child_index[i_cell][i_child]);
                            }

                            fprintf(f_out, "\n");
                        }

                        fclose(f_out);
                    }
                }
            }

            /* 6.a re-distribute upper-layer's children */
            {
                i_redist = 0;

                while(   ((max_number_of_children - min_number_of_children) > 2)
                      && (i_redist < rcfd->Solver_Dict->number_redist_loops_per_layer)){

                    /* adapt min clusters */
                    for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                        c0 = _F(i_layer).c0[i_face];
                        c1 = _F(i_layer).c1[i_face];

                        upper_c0 = _C(i_layer).parent_cell[c0];
                        upper_c1 = _C(i_layer).parent_cell[c1];

                        if((_C(i_layer + 1).number_of_children[upper_c1] == min_number_of_children) &&
                           (_C(i_layer + 1).number_of_children[upper_c0] > (min_number_of_children + 1))){

                            i_tmp = c0; c0 = c1; c1 = i_tmp; i_tmp = upper_c0; upper_c0 = upper_c1; upper_c1 = i_tmp;
                        }

                        if((_C(i_layer + 1).number_of_children[upper_c0] == min_number_of_children) &&
                           (_C(i_layer + 1).number_of_children[upper_c1] > (min_number_of_children + 1))){

                            _C(i_layer).parent_cell[c1] = _C(i_layer).parent_cell[c0];

                            _C(i_layer + 1).number_of_children[upper_c0] += 1;
                            _C(i_layer + 1).number_of_children[upper_c1] -= 1;
                        }
                    }

                    /* adapt max clusters */
                    for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                        c0 = _F(i_layer).c0[i_face];
                        c1 = _F(i_layer).c1[i_face];

                        upper_c0 = _C(i_layer).parent_cell[c0];
                        upper_c1 = _C(i_layer).parent_cell[c1];

                        if((_C(i_layer + 1).number_of_children[upper_c1] == max_number_of_children) &&
                           (_C(i_layer + 1).number_of_children[upper_c0] < (max_number_of_children - 1))){

                            i_tmp = c0; c0 = c1; c1 = i_tmp; i_tmp = upper_c0; upper_c0 = upper_c1; upper_c1 = i_tmp;
                        }

                        if((_C(i_layer + 1).number_of_children[upper_c0] == max_number_of_children) &&
                           (_C(i_layer + 1).number_of_children[upper_c1] < (max_number_of_children - 1))){

                            _C(i_layer).parent_cell[c0] = _C(i_layer).parent_cell[c1];

                            _C(i_layer + 1).number_of_children[upper_c0] -= 1;
                            _C(i_layer + 1).number_of_children[upper_c1] += 1;
                        }
                    }

                    /* adapt clusters in between min and max */
                    {
                        tmp_cluster_already_visited = (short*)malloc(_CELL_DICT(i_layer + 1).number_of_cells * sizeof(short));

                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                            tmp_cluster_already_visited[i_cell] = 0;
                        }

                        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                            c0 = _F(i_layer).c0[i_face];
                            c1 = _F(i_layer).c1[i_face];

                            upper_c0 = _C(i_layer).parent_cell[c0];
                            upper_c1 = _C(i_layer).parent_cell[c1];

                            if((tmp_cluster_already_visited[upper_c0] + tmp_cluster_already_visited[upper_c1]) == 0){

                                if(_C(i_layer + 1).number_of_children[upper_c0] > (_C(i_layer + 1).number_of_children[upper_c1] + 1)){

                                    i_tmp = c0; c0 = c1; c1 = i_tmp; i_tmp = upper_c0; upper_c0 = upper_c1; upper_c1 = i_tmp;
                                }

                                if(_C(i_layer + 1).number_of_children[upper_c0] < (_C(i_layer + 1).number_of_children[upper_c1] - 1)){

                                    _C(i_layer).parent_cell[c1] = _C(i_layer).parent_cell[c0];

                                    _C(i_layer + 1).number_of_children[upper_c0] += 1;
                                    _C(i_layer + 1).number_of_children[upper_c1] -= 1;

                                    tmp_cluster_already_visited[upper_c0] = 1;
                                    tmp_cluster_already_visited[upper_c1] = 1;
                                }
                            }
                        }

                        free(tmp_cluster_already_visited);
                    }

                    /* re-allocate (i_layer + 1)_children, re-initialize upper layer vars */
                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                        free(_C(i_layer + 1).child_index[i_cell]);

                        _C(i_layer + 1).child_index[i_cell] = (int*)malloc(_C(i_layer + 1).number_of_children[i_cell] * sizeof(int));

                        for(i_child = 0; i_child < _C(i_layer + 1).number_of_children[i_cell]; i_child++){

                            _C(i_layer + 1).child_index[i_cell][i_child] = -1;
                        }

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            _C(i_layer + 1).x[i_cell][i_dim] = 0.0;
                        }

                        _C(i_layer + 1).volume[i_cell] = 0.0;
                    }

                    /* set (i_layer + 1) grid vars */
                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        upper_cell = _C(i_layer).parent_cell[i_cell];

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            _C(i_layer + 1).x[upper_cell][i_dim] += _C(i_layer).x[i_cell][i_dim];
                        }

                        _C(i_layer + 1).volume[upper_cell] += _C(i_layer).volume[i_cell];

                        /* add child_index */
                        if(upper_cell > -1){

                            i_child = 0;

                            while(i_child < _C(i_layer + 1).number_of_children[upper_cell]){

                                if(_C(i_layer + 1).child_index[upper_cell][i_child] < 0){

                                    _C(i_layer + 1).child_index[upper_cell][i_child] = i_cell;

                                    i_child =  2 * _C(i_layer + 1).number_of_children[upper_cell];
                                }
                                else{

                                    i_child++;
                                }
                            }
                        }
                    }

                    /* average upper-layer coords, get new min/max number_of_children */
                    {
                        max_number_of_children = 0;
                        min_number_of_children = 9999;

                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                            for(i_dim = 0; i_dim < 3; i_dim++){

                                _C(i_layer + 1).x[i_cell][i_dim] /= (double)_C(i_layer + 1).number_of_children[i_cell];
                            }

                            if(_C(i_layer + 1).number_of_children[i_cell] > max_number_of_children){

                                max_number_of_children = _C(i_layer + 1).number_of_children[i_cell];
                            }

                            if(_C(i_layer + 1).number_of_children[i_cell] < min_number_of_children){

                                min_number_of_children = _C(i_layer + 1).number_of_children[i_cell];
                            }
                        }
                    }

                    if(debug_this_code){

                        report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                     "[%d] L.6.a: i_redist %d, min/max number_of_children %d/%d",
                                     myid, i_redist, min_number_of_children, max_number_of_children);
                    }

                    i_redist++;
                }
            }

            /* 7 create tmp_parent_xy vars for (i_layer + 1) faces */
            {
                tmp_parent_face_index = (int*)malloc(_FACE_DICT(i_layer).number_of_faces * sizeof(int));

                for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                    tmp_parent_face_index[i_face] = -1;
                }

                tmp_parent_cell_cell_index = malloc_i_2d(_CELL_DICT(i_layer + 1).number_of_cells, rcfd->Solver_Dict->max_number_of_faces_per_cell);

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                    for(i_cell_face = 0; i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell; i_cell_face++){

                        tmp_parent_cell_cell_index[i_cell][i_cell_face] = -1;
                    }
                }

                tmp_parent_cell_face_index = malloc_i_2d(_CELL_DICT(i_layer + 1).number_of_cells, rcfd->Solver_Dict->max_number_of_faces_per_cell);

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

                    for(i_cell_face = 0; i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell; i_cell_face++){

                        tmp_parent_cell_face_index[i_cell][i_cell_face] = -1;
                    }
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] L.7 i_layer %d", myid, i_layer);
                }

            }

            /* 8 fill tmp_parent_xy vars */
            {
                parent_face_count = 0;

                for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                    c0 = _F(i_layer).c0[i_face];
                    c1 = _F(i_layer).c1[i_face];

                    upper_c0 = _C(i_layer).parent_cell[c0];
                    upper_c1 = _C(i_layer).parent_cell[c1];

                    if(upper_c0 == upper_c1){

                        tmp_parent_face_index[i_face] = -1;
                    }
                    else{

                        i_cell_face = 0;

                        while(i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell){

                            /* parent face already exists */
                            if(tmp_parent_cell_cell_index[upper_c0][i_cell_face] == upper_c1){

                                tmp_parent_face_index[i_face] = tmp_parent_cell_face_index[upper_c0][i_cell_face];

                                i_cell_face = rcfd->Solver_Dict->max_number_of_faces_per_cell;
                            }

                            /* add new parent face to upper_c0, upper_c1 */
                            if(tmp_parent_cell_cell_index[upper_c0][i_cell_face] < 0){

                                tmp_parent_cell_cell_index[upper_c0][i_cell_face] = upper_c1;

                                tmp_parent_cell_face_index[upper_c0][i_cell_face] = parent_face_count;

                                /* also add this parent face to the list of upper_c1 */
                                {
                                    i_cell_face2 = 0;

                                    while((tmp_parent_cell_cell_index[upper_c1][i_cell_face2] >= 0) &&
                                        (i_cell_face < rcfd->Solver_Dict->max_number_of_faces_per_cell)){

                                        i_cell_face2++;
                                    }

                                    if(i_cell_face2 < rcfd->Solver_Dict->max_number_of_faces_per_cell){

                                        tmp_parent_cell_cell_index[upper_c1][i_cell_face2] = upper_c0;
                                        tmp_parent_cell_face_index[upper_c1][i_cell_face2] = parent_face_count;
                                    }
                                }

                                tmp_parent_face_index[i_face] = parent_face_count;

                                parent_face_count++;

                                i_cell_face = rcfd->Solver_Dict->max_number_of_faces_per_cell;
                            }

                            i_cell_face++;
                        }
                    }
                }

                _FACE_DICT(i_layer + 1).number_of_faces = parent_face_count;

                /* TODO account for upper-layer parallel grid communication */
                _FACE_DICT(i_layer + 1).number_of_int_faces = parent_face_count;
                _FACE_DICT(i_layer + 1).number_of_ext_faces = 0;

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] L.8 upper_layer %d, number of faces in upper layer %d",
                                 myid, (i_layer + 1), parent_face_count);
                }
            }

            /* 9 create and fill upper faces */
            {
                _F(i_layer + 1).c0 = (int*)malloc(_FACE_DICT(i_layer + 1).number_of_faces * sizeof(int));
                _F(i_layer + 1).c1 = (int*)malloc(_FACE_DICT(i_layer + 1).number_of_faces * sizeof(int));

                _F(i_layer + 1).area = malloc_r_2d(_FACE_DICT(i_layer + 1).number_of_faces, 3);

                for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                    if(tmp_parent_face_index[i_face] >= 0){

                        c0 = _F(i_layer).c0[i_face];
                        c1 = _F(i_layer).c1[i_face];

                        upper_c0 = _C(i_layer).parent_cell[c0];
                        upper_c1 = _C(i_layer).parent_cell[c1];

                        i_parent_face = tmp_parent_face_index[i_face];

                        _F(i_layer + 1).c0[i_parent_face] = upper_c0;
                        _F(i_layer + 1).c1[i_parent_face] = upper_c1;

                        _F(i_layer + 1).area[i_parent_face][0] += _F(i_layer).area[i_face][0];
                        _F(i_layer + 1).area[i_parent_face][1] += _F(i_layer).area[i_face][1];
                        _F(i_layer + 1).area[i_parent_face][2] += _F(i_layer).area[i_face][2];
                    }
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] L.9 i_layer %d", myid, i_layer);
                }

            }

            /* 10 consistency checks */
            {
                if(_CELL_DICT(i_layer + 1).number_of_cells < rcfd->Solver_Dict->min_number_of_cells_per_layer){

                    report_warning(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                   "[%d] layer-%d number_of_cells (%d) is smaller than Solver_Dict.min_number_of_cells_per_layer (%d)",
                                   myid, (i_layer + 1), _CELL_DICT(i_layer + 1).number_of_cells, rcfd->Solver_Dict->min_number_of_cells_per_layer);
                }

                if(max_number_of_children > rcfd->Solver_Dict->max_number_of_children){

                    report_warning(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                   "[%d] layer-%d number_of_children (%d) is larger than Solver_Dict.max_number_of_children (%d)",
                                   myid, (i_layer + 1), max_number_of_children, rcfd->Solver_Dict->max_number_of_children);
                }

                if((number_of_unassigned_children + number_of_unassigned_parents + number_of_negative_child_indices) > 0){

                    report_warning(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                   "[%d] Inconsistency between layer %d and %d: "
                                   "unassigned_children (%d) unassigned_parents (%d) negative_child_indices (%d)",
                                   myid, i_layer, (i_layer + 1),
                                   number_of_unassigned_children, number_of_unassigned_parents, number_of_negative_child_indices);
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] L.10 i_layer %d", myid, i_layer);
                }
            }

            /* 11 free local vars */
            {
                free_i_2d(tmp_faces_of_cell);

                free(tmp_parent_face_index);

                free_i_2d(tmp_parent_cell_cell_index);

                free_i_2d(tmp_parent_cell_face_index);


                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] L.11 i_layer %d ", myid, i_layer);
                }

            }

            i_layer++;
        }

        if(rcfd->Solver_Dict->report_priority <= RCFD_REPORT_INFO){

            report_info(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] multi-layer grid:", myid);

            for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                report_info(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "[%d]   Layer %d: Cells %d Faces %d\n",
                            myid, i_layer, _CELL_DICT(i_layer).number_of_cells, _FACE_DICT(i_layer).number_of_faces);
            }
        }

        /* Test mapping */
        if(0==1){
            test_layer_mapping(rcfd);
        }
#endif
    }
}

void init_parallel_comm(rCFD_type *rcfd)
{
#if RP_NODE

    short   i_layer = 0;

    while(i_layer < rcfd->Solver_Dict->number_of_layers)
    {

        parallel_init_topology(rcfd, i_layer);

        if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

            parallel_expand_initial_fields(rcfd, i_layer);
        }

        /* not implemented yet

        if(_upper_layer < Solver_Dict->number_of_layers){

            create_parent_MPI_topology(i_layer);
        }*/

        i_layer++;
    }

    report_info0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "created MPI-grid topology");

    rcfd->Parallel->transfer_data_in  = NULL;
    rcfd->Parallel->transfer_data_out = NULL;

#endif
}

void init_c2c(rCFD_type *rcfd)
{
#if RP_NODE

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        int error = shift_allocate_run(rcfd);

        if(!error){

            default_c2c(rcfd);
        }
        else{

            report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                         "[%d] init_c2c: shift_allocate_run failed",
                         myid);
        }
    }
#endif
}

void init_all(rCFD_type *rcfd, int mode)
{
    /* Create directories and define file names */
    init_file_dict(rcfd, mode);

    /* Solver_Dict */
    init_solver_dict(rcfd, mode);

    /* Solver */
    init_solver(rcfd);

    /* Phase_Dict */
    init_phase_dict(rcfd);

    /* Prep_Dict */
    /* only used in preparation mode */
    init_prep_dict(rcfd);

    /* Norm_Dict */
    /* only used in preparation mode */
    init_norm_dict(rcfd);

    /* Rec_Dict */
    init_rec_dict(rcfd);

    /* Data_Dict */
    /* only used in run mode */
    init_data_dict(rcfd);

    /* Balance_Dict */
    /* only used in run mode */
    init_balance_dict(rcfd);

    /* Topo_Dict */
    init_topo_dict(rcfd);

    /* Topo */
    init_topo(rcfd);

    /* Dump_Dict */
    init_dump_dict(rcfd);

    /* C2Cs */
    /* only used in preparation mode */
    init_c2c_prep(rcfd);
    init_c2c_interface(rcfd);

    /* Rec */
    init_rec(rcfd);

    /* Norms */
    init_norms(rcfd);

    /* Balance (first initialization) */
    /* only used in run mode */
    init_balance(rcfd);

    /* Initialize data fields and update balances */
    /* only used in run mode */
    init_data(rcfd);

    /* Initialize layers */
#if MULTI_LAYER_IMPLEMENTATION_COMPLETE
    /* only used in run mode (preparation mode uses only base layer) */
    init_coarse_grid_layers(rcfd);
#endif

    /* Parallel grid communication */
    init_parallel_comm(rcfd);

    /* Initialize C2C shifts */
    /* only used in run mode */
    init_c2c(rcfd);

    free_config_toml(rcfd);

    if(rcfd->Solver_Dict->write_config_to_file_on){

        write_config_toml(rcfd, "config_out.toml");
    }

    rcfd->initialized   = 1;
}

void test_layer_mapping(rCFD_type *rcfd)
{
    int i_layer = 0;

    layer_map_parent(rcfd, i_layer);
    /* layer-1 should have number of layer-0 children in data[0] */

    i_layer = 1;

    layer_map_parent(rcfd, i_layer);
    /* layer-2 should have number of their layer-0 children in data[0] */
#if 0
    i_layer = 2;

    layer_map_parent(rcfd, i_layer);
    /* layer-3 cells should have their layer-2 siblings */

    i_layer = 3;

    layer_map_children(rcfd, i_layer);
    /* layer-1 cells should have their layer-2 siblings */
#endif
    i_layer = 2;

    layer_map_children(rcfd, i_layer);
    /* layer-0 cells should have their layer-2 siblings */

    i_layer = 1;

    layer_map_children(rcfd, i_layer);
    /* layer-0 cells should have their layer-2 siblings */

    i_layer = 0;

    rcfd->rCFD_UDF->_user_post(rcfd);
    /* data[0] should visualize clusters */
}

#endif
