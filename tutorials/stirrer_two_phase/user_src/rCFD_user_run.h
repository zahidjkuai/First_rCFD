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
    OXY,
    DATA_COUNT
};

#if RP_NODE
static int oxy_monitor_initialized = 0;

#define     OXY_INIT              8e-6
#define     OXY_EQ               10e-6      // i.e. 10 mg/L max. Löslichkeit von O2 in Wasser
#define     BUBBLE_DIAM         1.0e-2
#define     REACT_TRANS_COEFF   1.0e3       // (1/m2.s)
#endif

/*****************************************************************************/
/* user dictionaries                                                         */
/*****************************************************************************/

void user_set_solver_dict(void *handle)
{
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "report_priority", RCFD_REPORT_INFO);
    rcfd_set_dict_property_r(handle, "Solver_Dict", -1, -1, -1, "global_time_step", 0.05);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "time_steps_per_monitoring_interval", 20);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_frames", 200);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "local_diff_on", 0);
    rcfd_set_dict_property_s(handle, "Solver_Dict", -1, -1, -1, "local_diff_format", "field_based");
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "control_conc_sum_on", 0);  // since mix fields do not sum up to one
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "moving_geometry_on", 1);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "single_phase_vof", 1);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "single_phase_vof_id", 0);
    rcfd_set_dict_property_r(handle, "Solver_Dict", -1, -1, -1, "vof_cutoff_limit", 1.0e-3);
}

void user_set_phase_dict(void *handle)
{
#if RP_NODE
    rcfd_set_dict_property_i(handle, "Phase_Dict", -1, WATER, -1, "number_of_data", 1);
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
    rcfd_set_dict_property_s(handle, "Data_Dict", -1, WATER, OXY, "type", "concentration");
    rcfd_set_dict_property_r(handle, "Data_Dict", -1, WATER, OXY, "face_swap_diffusion", 0.01);
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
    double  **cell_data = NULL;

    // access data
    {
        int_cell_count  = rcfd_get_int_cell_count(handle, i_layer);
        cell_data       = rcfd_get_cell_data(handle, i_layer, WATER);

        if(cell_data == NULL){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_init_data: var == NULL");
            return;
        }
    }

    // init data
    {
        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_data[i_cell][OXY] = OXY_INIT;
        }
    }

    rcfd_report0(handle, RCFD_REPORT_INFO, "user_init_data: initialized Oxy with %e", OXY_INIT);
#endif
}

/*****************************************************************************/
/* user data access                                                          */
/*****************************************************************************/

void user_access_data_before_shift(void *handle, const short i_phase)
{
#if RP_NODE
    int     i_layer, i_frame, i_cell;
    int     int_cell_count;

    double  global_timestep, cell_mass, water_density;
    double  vof_bubble, vof_water, vof_cutoff;
    double  cell_area, dmOxy, dmOxy_acc = 0.0;

    double  *cell_volume = NULL;
    double  **cell_conc = NULL, **cell_vof = NULL;

    // access rCFD vars, sanity check
    {
        i_layer         = rcfd_get_current_layer(handle);
        i_frame         = rcfd_get_current_frame(handle);
        int_cell_count  = rcfd_get_int_cell_count(handle, i_layer);
        global_timestep = rcfd_get_timestep(handle, i_layer);
        water_density   = rcfd_get_phase_density(handle, WATER);
        vof_cutoff      = rcfd_get_vof_cutoff_limit(handle);
        cell_volume     = rcfd_get_cell_volume(handle, i_layer);
        cell_vof        = rcfd_get_cell_vof(handle, i_layer, i_frame);
        cell_conc       = rcfd_get_cell_data(handle, i_layer, WATER);

        if(!cell_volume || !cell_conc || !cell_vof){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_access_data_before_shift: var == NULL");
            return;
        }
    }

    // calculate per cell reaction turnover
    {
        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            vof_water  = cell_vof[i_cell][WATER];
            vof_bubble = 1.0 - vof_water;

            if((vof_bubble > vof_cutoff) &&
               (vof_water  > vof_cutoff)){

                cell_area = 6.0 * vof_bubble * cell_volume[i_cell] / BUBBLE_DIAM;   // disperse sphere approx.

                dmOxy = REACT_TRANS_COEFF * cell_area * (OXY_EQ - cell_conc[i_cell][OXY])
                      * vof_water * cell_volume[i_cell] * water_density * global_timestep;   // (kg)

                cell_mass = vof_water * cell_volume[i_cell] * water_density;        // > vof_cutoff * V_cell * water_density

                cell_conc[i_cell][OXY] = (cell_conc[i_cell][OXY] * cell_mass + dmOxy) / cell_mass;

                if(cell_conc[i_cell][OXY] < 0.0) cell_conc[i_cell][OXY] = 0.0;
                if(cell_conc[i_cell][OXY] > 1.0) cell_conc[i_cell][OXY] = 1.0;

                dmOxy_acc += dmOxy;
            }
        }
    }

    // adjust global (per node) balance
    {
        rcfd_set_balance_mass_source_one(handle, i_phase, OXY, dmOxy_acc);
    }

#endif
}

void user_post(void *handle)
{
#if RP_NODE
    int     i_layer, i_frame, i_cell;
    int     int_cell_count;

    double  water_density, oxy_mass, oxy_mass_global, global_time;
    double  *cell_volume = NULL;
    double  **cell_conc = NULL, **cell_vof = NULL;

    char    filename[80] = "./rCFD_Oxy.out";
    FILE    *f_out = NULL;

    // access rCFD vars
    {
        i_layer         = rcfd_get_current_layer(handle);
        i_frame         = rcfd_get_current_frame(handle);
        int_cell_count  = rcfd_get_int_cell_count(handle, i_layer);
        global_time     = rcfd_get_time(handle);
        water_density   = rcfd_get_phase_density(handle, WATER);
        cell_volume     = rcfd_get_cell_volume(handle, i_layer);
        cell_conc       = rcfd_get_cell_data(handle, i_layer, WATER);
        cell_vof        = rcfd_get_cell_vof(handle, i_layer, i_frame);

        if((!cell_volume) || (!cell_conc) || (!cell_vof)){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_post: var == NULL");
            return;
        }
    }

    // calc global Oxy mass
    {
        oxy_mass = 0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            oxy_mass += cell_conc[i_cell][OXY] * cell_vof[i_cell][WATER] * cell_volume[i_cell] * water_density;
        }

        oxy_mass_global = PRF_GRSUM1(oxy_mass);
    }

    // node-0 writes global Oxy mass
    if(myid == 0){

        // open file
        {
            if(oxy_monitor_initialized == 0){

                f_out = fopen(filename, "w");
                oxy_monitor_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                rcfd_report0(handle, RCFD_REPORT_ERROR, "\nERROR: Could not open %s", filename);
                return;
            }
        }

        fprintf(f_out, "%e %e\n", global_time, oxy_mass_global);
        fclose(f_out);

        rcfd_report0(handle, RCFD_REPORT_INFO, "user_post: wrote Oxy to %s", filename);
    }

    // nodes write data to UDMI
    {
        int     udmi_offset = 4, i_fluent_cell;
        Domain  *d = Get_Domain(1);
        Thread  *t;

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){begin_c_loop_int(i_fluent_cell, t){

            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

            if((i_cell > 0) && (i_cell < int_cell_count)){

                C_UDMI(i_fluent_cell, t, (udmi_offset + 0)) = (1.0 - cell_vof [i_cell][WATER]);
                C_UDMI(i_fluent_cell, t, (udmi_offset + 1)) = cell_conc[i_cell][OXY];
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
    REGISTER_RCFD_UDF( user_access_data_before_shift );
    REGISTER_RCFD_UDF( user_post );
}

#endif
