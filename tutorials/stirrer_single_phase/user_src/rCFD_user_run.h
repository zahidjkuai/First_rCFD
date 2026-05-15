/******************************************************************************
 * @copyright   (C) 2026
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_USER
#define RCFD_USER

#include "rCFD_library.h"

/*****************************************************************************/
/* user names, definitions, global vars & auxiliary functions                */
/*****************************************************************************/

// names of phases
enum{
    WATER,
    PHASE_COUNT
};

// names of data
enum{
    MIX_Z,
    MIX_R,
    DATA_COUNT
};

#if RP_NODE
static int mix_ind_initialized = 0;
static int dmean_initialized =   0;

static double dmean_mix_z0 = 0.0;
static double dmean_mix_r0 = 0.0;
#endif

/*****************************************************************************/
/* user dictionaries                                                         */
/*****************************************************************************/

void user_set_solver_dict(void *handle)
{
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "report_priority", RCFD_REPORT_INFO);
    rcfd_set_dict_property_r(handle, "Solver_Dict", -1, -1, -1, "global_time_step", 0.05);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "time_steps_per_monitoring_interval", 5);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_frames", 200);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "local_diff_on", 0);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "control_conc_sum_on", 0);  // since mix fields do not sum up to one
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "moving_geometry_on", 1);
}

void user_set_phase_dict(void *handle)
{
#if RP_NODE
    rcfd_set_dict_property_i(handle, "Phase_Dict", -1, WATER, -1, "number_of_data", 2);
    rcfd_set_dict_property_r(handle, "Phase_Dict", -1, WATER, -1, "density", 1000.0);
#endif
}

void user_set_rec_dict(void *handle)
{
    rcfd_set_dict_property_s(handle, "Rec_Dict", -1, -1, -1, "format", "replay");
}

void user_set_data_dict(void *handle)
{
#if RP_NODE
    rcfd_set_dict_property_s(handle, "Data_Dict", -1, WATER, MIX_Z, "type", "concentration");
    rcfd_set_dict_property_r(handle, "Data_Dict", -1, WATER, MIX_Z, "face_swap_diffusion", 0.2);

    rcfd_set_dict_property_s(handle, "Data_Dict", -1, WATER, MIX_R, "type", "concentration");
    rcfd_set_dict_property_r(handle, "Data_Dict", -1, WATER, MIX_R, "face_swap_diffusion", 0.2);
#endif
}

/*****************************************************************************/
/* user initialization                                                       */
/*****************************************************************************/

void user_init_data(void *handle, const short i_layer)
{
#if RP_NODE
    int     i_cell;
    int     int_cell_count;
    double  radius_sq;
    double  **cell_center = NULL, **cell_data = NULL;

    // access data
    {
        int_cell_count  = rcfd_get_int_cell_count(handle, i_layer);
        cell_center     = rcfd_get_cell_center(handle, i_layer);
        cell_data       = rcfd_get_cell_data(handle, i_layer, WATER);

        if((cell_data == NULL) || (cell_center == NULL)){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_init_data: var == NULL");
            return;
        }
    }

    // init data
    {
        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            if(cell_center[i_cell][2] < 0.3) cell_data[i_cell][MIX_Z] = 1.0;
            else                             cell_data[i_cell][MIX_Z] = 0.0;

            radius_sq = (cell_center[i_cell][0] * cell_center[i_cell][0] +
                         cell_center[i_cell][1] * cell_center[i_cell][1]);

            if(radius_sq < 0.25 * 0.25) cell_data[i_cell][MIX_R] = 1.0;
            else                        cell_data[i_cell][MIX_R] = 0.0;

        }
    }

    rcfd_report0(handle, RCFD_REPORT_INFO, "user_init_data: initialized mix fields");
#endif
}

/*****************************************************************************/
/* user data access                                                          */
/*****************************************************************************/

void user_post(void *handle)
{
#if RP_NODE
    int     i_layer, i_cell, i_fluent_cell;
    int     int_cell_count;

    double  cell_mass, density, mass;
    double  mass_mix_z, mass_mix_r;
    double  mean_mix_z, mean_mix_r;
    double  dmean_mix_z, dmean_mix_r;
    double  mix_ind_z, mix_ind_r;
    double  current_time;

    double  *cell_volume = NULL;
    double  **cell_data = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "rCFD_mix_ind.out";

    // access data
    {
        i_layer         = rcfd_get_current_layer(handle);
        int_cell_count  = rcfd_get_int_cell_count(handle, i_layer);
        density         = rcfd_get_phase_density(handle, WATER);
        current_time    = rcfd_get_time(handle);
        cell_volume     = rcfd_get_cell_volume(handle, i_layer);
        cell_data       = rcfd_get_cell_data(handle, i_layer, WATER);

        if(cell_data == NULL){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_post: var == NULL");
            return;
        }
    }

    // calc mean concs
    {
        mass       = 0.0;
        mass_mix_z = 0.0;       mass_mix_r = 0.0;
        mean_mix_z = 0.0;       mean_mix_r = 0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_mass = cell_volume[i_cell] * density;

            mass       += cell_mass;
            mass_mix_z += cell_mass * cell_data[i_cell][MIX_Z];
            mass_mix_r += cell_mass * cell_data[i_cell][MIX_R];
        }

        mass       = PRF_GRSUM1(mass);
        mass_mix_z = PRF_GRSUM1(mass_mix_z);
        mass_mix_r = PRF_GRSUM1(mass_mix_r);

        if(mass > 0.0){

            mean_mix_z = mass_mix_z / mass;
            mean_mix_r = mass_mix_r / mass;
        }
    }

    // calc dmean concs
    {
        mass_mix_z  = 0.0;      mass_mix_r  = 0.0;
        dmean_mix_z = 0.0;      dmean_mix_r = 0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_mass = cell_volume[i_cell] * density;

            mass_mix_z += cell_mass * fabs(cell_data[i_cell][MIX_Z] - mean_mix_z);
            mass_mix_r += cell_mass * fabs(cell_data[i_cell][MIX_R] - mean_mix_r);
        }

        mass_mix_z = PRF_GRSUM1(mass_mix_z);
        mass_mix_r = PRF_GRSUM1(mass_mix_r);

        if(mass > 0.0){

            dmean_mix_z = mass_mix_z / mass;
            dmean_mix_r = mass_mix_r / mass;
        }

        if(dmean_initialized == 0){

            dmean_mix_z0 = dmean_mix_z;
            dmean_mix_r0 = dmean_mix_r;

            dmean_initialized = 1;
        }
    }

    // node-0 writes mixing indices
    if(myid == 0){

        // open file
        {
            if(mix_ind_initialized == 0){

                f_out = fopen(filename, "w");
                rcfd_report0(handle, RCFD_REPORT_INFO, "user_post: initialized %s", filename);
                mix_ind_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                rcfd_report0(handle, RCFD_REPORT_ERROR, "Failed to open %s", filename);
                return;
            }
        }

        mix_ind_z = 0.0;    mix_ind_r = 0.0;

        if(dmean_mix_z0 > 0.0) mix_ind_z = 1.0 - dmean_mix_z/dmean_mix_z0;
        if(dmean_mix_r0 > 0.0) mix_ind_r = 1.0 - dmean_mix_r/dmean_mix_r0;

        fprintf(f_out, "%e %e %e\n", current_time, mix_ind_z, mix_ind_r);
        fclose(f_out);
    }

    // nodes write data to UDMI
    {
        Domain  *d = Get_Domain(1);
        Thread  *t;

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){begin_c_loop_int(i_fluent_cell, t){

            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

            if((i_cell > 0) && (i_cell < int_cell_count)){

                C_UDMI(i_fluent_cell, t, 0) = cell_data[i_cell][MIX_Z];
                C_UDMI(i_fluent_cell, t, 1) = cell_data[i_cell][MIX_R];
            }

        }end_c_loop_int(i_fluent_cell, t)}}

        rcfd_report0(handle, RCFD_REPORT_INFO, "user_post: set Fluent UDMI fields");
    }

#endif
}

/*****************************************************************************/
/* register user functions                                                   */
/*****************************************************************************/

void user_register_functions(void *handle)
{
    REGISTER_RCFD_UDF( user_set_solver_dict );
    REGISTER_RCFD_UDF( user_set_phase_dict );
    REGISTER_RCFD_UDF( user_set_rec_dict );
    REGISTER_RCFD_UDF( user_set_data_dict );

    REGISTER_RCFD_UDF( user_init_data );
    REGISTER_RCFD_UDF( user_post );
}

#endif
