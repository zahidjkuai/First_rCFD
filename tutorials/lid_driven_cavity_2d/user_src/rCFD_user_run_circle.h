/******************************************************************************
 * @copyright   (C) 2026
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *
 * @file        rCFD_user_run_circle.h
 * @brief       rCFD user run file -- circular scalar injection
 *              CX=0.035, CY=0.035, CR=0.005, CZ=0.005
 *****************************************************************************/

#ifndef RCFD_USER
#define RCFD_USER

#include "rCFD_library.h"

/******************************************************************************
 * circle injection parameters
 *****************************************************************************/

#define CX  0.035   /* circle centre x [m] */
#define CY  0.035   /* circle centre y [m] */
#define CR  0.005   /* circle radius   [m] */
#define CZ  0.005   /* max depth z     [m] */

/******************************************************************************
 * user enums, definitions, global vars & auxiliary functions
 *****************************************************************************/

enum{
    WATER,
    PHASE_COUNT
};

enum{
    MIX1,
    MIX2,
    DATA_COUNT
};

#if RP_NODE
static int CoG_initialized = 0;
static int mix_ind_initialized = 0;

static int    dmean_initialized = 0;
static double dmean_mix1_0 = 0.0;
static double dmean_mix2_0 = 0.0;
#endif

/******************************************************************************
 * user dictionaries
 *****************************************************************************/

void user_set_solver_dict(void *handle)
{
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "report_priority", RCFD_REPORT_INFO);
    rcfd_set_dict_property_r(handle, "Solver_Dict", -1, -1, -1, "global_time_step", 0.025);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "time_steps_per_monitoring_interval", 5);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_frames", 200);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "local_diff_on", 0);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "control_conc_sum_on", 0);
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
    rcfd_set_dict_property_s(handle, "Data_Dict", -1, WATER, MIX1, "type", "concentration");
    rcfd_set_dict_property_r(handle, "Data_Dict", -1, WATER, MIX1, "face_swap_diffusion", 0.0001);

    rcfd_set_dict_property_s(handle, "Data_Dict", -1, WATER, MIX2, "type", "concentration");
    rcfd_set_dict_property_r(handle, "Data_Dict", -1, WATER, MIX2, "face_swap_diffusion", 0.4);
#endif
}

/******************************************************************************
 * user data initialization  --  CIRCLE at (CX, CY), radius CR, depth <= CZ
 *****************************************************************************/

void user_init_data(void *handle, const short i_layer)
{
#if RP_NODE
    int     i_cell;
    int     int_cell_count;
    double  dx, dy;
    double  **cell_center = NULL, **cell_data = NULL;

    // access data
    {
        int_cell_count = rcfd_get_int_cell_count(handle, i_layer);
        cell_center    = rcfd_get_cell_center(handle, i_layer);
        cell_data      = rcfd_get_cell_data(handle, i_layer, WATER);

        if((cell_data == NULL) || (cell_center == NULL)){
            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_init_data: var == NULL");
            return;
        }
    }

    // init data -- circle condition
    {
        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            dx = cell_center[i_cell][0] - CX;
            dy = cell_center[i_cell][1] - CY;

            if( (dx*dx + dy*dy) <= (CR*CR) &&
                (cell_center[i_cell][2] <= CZ) ){

                cell_data[i_cell][MIX1] = 1.0;
                cell_data[i_cell][MIX2] = 1.0;
            }
            else{

                cell_data[i_cell][MIX1] = 0.0;
                cell_data[i_cell][MIX2] = 0.0;
            }
        }
    }

    rcfd_report0(handle, RCFD_REPORT_INFO,
        "user_init_data: circle at (%.4f, %.4f) r=%.4f z<=%.4f", CX, CY, CR, CZ);
#endif
}

/******************************************************************************
 * user_post  --  unchanged from original
 *****************************************************************************/

void user_post(void *handle)
{
#if RP_NODE
    int     i_layer, i_cell;
    int     int_cell_count;

    double  cell_mass, density, mass, mass_mix1, mass_mix2;
    double  mean_mix1, mean_mix2, dmean_mix1, dmean_mix2, mix_ind1, mix_ind2;
    double  mass_CoG_x1, mass_CoG_y1, mass_CoG_z1, CoG_x1, CoG_y1, CoG_z1;
    double  mass_CoG_x2, mass_CoG_y2, mass_CoG_z2, CoG_x2, CoG_y2, CoG_z2;
    double  current_time;

    double  *cell_volume = NULL;
    double  **cell_center = NULL, **cell_data = NULL;

    FILE    *f_out = NULL;
    char    filename_mix[80] = "rCFD_mix_ind.out";
    char    filename_CoG[80] = "rCFD_CoG.out";

    // access data
    {
        i_layer        = rcfd_get_current_layer(handle);
        int_cell_count = rcfd_get_int_cell_count(handle, i_layer);
        density        = rcfd_get_phase_density(handle, WATER);
        current_time   = rcfd_get_time(handle);
        cell_volume    = rcfd_get_cell_volume(handle, i_layer);
        cell_data      = rcfd_get_cell_data(handle, i_layer, WATER);
        cell_center    = rcfd_get_cell_center(handle, i_layer);

        if(cell_data == NULL){
            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_post: var == NULL");
            return;
        }
    }

    // calc mean concs
    {
        mass = 0.0;

        mass_mix1   = mean_mix1   = 0.0;
        mass_CoG_x1 = mass_CoG_y1 = mass_CoG_z1 = 0.0;
        CoG_x1      = CoG_y1      = CoG_z1      = 0.0;

        mass_mix2   = mean_mix2   = 0.0;
        mass_CoG_x2 = mass_CoG_y2 = mass_CoG_z2 = 0.0;
        CoG_x2      = CoG_y2      = CoG_z2      = 0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_mass = cell_volume[i_cell] * density;

            mass        += cell_mass;
            mass_mix1   += cell_mass * cell_data[i_cell][MIX1];
            mass_CoG_x1 += cell_mass * cell_data[i_cell][MIX1] * cell_center[i_cell][0];
            mass_CoG_y1 += cell_mass * cell_data[i_cell][MIX1] * cell_center[i_cell][1];
            mass_CoG_z1 += cell_mass * cell_data[i_cell][MIX1] * cell_center[i_cell][2];
            mass_mix2   += cell_mass * cell_data[i_cell][MIX2];
            mass_CoG_x2 += cell_mass * cell_data[i_cell][MIX2] * cell_center[i_cell][0];
            mass_CoG_y2 += cell_mass * cell_data[i_cell][MIX2] * cell_center[i_cell][1];
            mass_CoG_z2 += cell_mass * cell_data[i_cell][MIX2] * cell_center[i_cell][2];
        }

        mass        = PRF_GRSUM1(mass);
        mass_mix1   = PRF_GRSUM1(mass_mix1);
        mass_CoG_x1 = PRF_GRSUM1(mass_CoG_x1);
        mass_CoG_y1 = PRF_GRSUM1(mass_CoG_y1);
        mass_CoG_z1 = PRF_GRSUM1(mass_CoG_z1);
        mass_mix2   = PRF_GRSUM1(mass_mix2);
        mass_CoG_x2 = PRF_GRSUM1(mass_CoG_x2);
        mass_CoG_y2 = PRF_GRSUM1(mass_CoG_y2);
        mass_CoG_z2 = PRF_GRSUM1(mass_CoG_z2);

        if(mass > 0.0){
            mean_mix1 = mass_mix1 / mass;
            CoG_x1    = mass_CoG_x1 / mass_mix1;
            CoG_y1    = mass_CoG_y1 / mass_mix1;
            CoG_z1    = mass_CoG_z1 / mass_mix1;
            mean_mix2 = mass_mix2 / mass;
            CoG_x2    = mass_CoG_x2 / mass_mix2;
            CoG_y2    = mass_CoG_y2 / mass_mix2;
            CoG_z2    = mass_CoG_z2 / mass_mix2;
        }
    }

    // calc dmean concs
    {
        mass_mix1  = 0.0;
        dmean_mix1 = 0.0;
        mass_mix2  = 0.0;
        dmean_mix2 = 0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_mass = cell_volume[i_cell] * density;

            mass_mix1 += cell_mass * fabs(cell_data[i_cell][MIX1] - mean_mix1);
            mass_mix2 += cell_mass * fabs(cell_data[i_cell][MIX2] - mean_mix2);
        }

        mass_mix1 = PRF_GRSUM1(mass_mix1);
        mass_mix2 = PRF_GRSUM1(mass_mix2);

        if(mass > 0.0){
            dmean_mix1 = mass_mix1 / mass;
            dmean_mix2 = mass_mix2 / mass;
        }

        if(dmean_initialized == 0){
            dmean_mix1_0 = dmean_mix1;
            dmean_mix2_0 = dmean_mix2;
            dmean_initialized = 1;
        }
    }

    // node-0 writes mixing indices
    if(myid == 0){

        {
            if(mix_ind_initialized == 0){
                f_out = fopen(filename_mix, "w");
                rcfd_report0(handle, RCFD_REPORT_INFO, "user_post: initialized %s", filename_mix);
                mix_ind_initialized = 1;
            }
            else{
                f_out = fopen(filename_mix, "a");
            }

            if(f_out == NULL){
                rcfd_report0(handle, RCFD_REPORT_ERROR, "Failed to open %s", filename_mix);
                return;
            }
        }

        mix_ind1 = 0.0;
        mix_ind2 = 0.0;

        if(dmean_mix1_0 > 0.0) mix_ind1 = 1.0 - dmean_mix1/dmean_mix1_0;
        if(dmean_mix2_0 > 0.0) mix_ind2 = 1.0 - dmean_mix2/dmean_mix2_0;

        fprintf(f_out, "%e %e %e\n", current_time, mix_ind1, mix_ind2);
        fclose(f_out);

        {
            if(CoG_initialized == 0){
                f_out = fopen(filename_CoG, "w");
                rcfd_report0(handle, RCFD_REPORT_INFO, "user_post: initialized %s", filename_CoG);
                CoG_initialized = 1;
            }
            else{
                f_out = fopen(filename_CoG, "a");
            }

            if(f_out == NULL){
                rcfd_report0(handle, RCFD_REPORT_ERROR, "Failed to open %s", filename_CoG);
                return;
            }
        }

        fprintf(f_out, "%e %e %e %e %e %e %e\n",
                current_time,
                CoG_x1, CoG_y1, CoG_z1,
                CoG_x2, CoG_y2, CoG_z2);
        fclose(f_out);
    }

    // nodes write data to UDMI
    {
        int      i_fluent_cell;
        Domain  *d = Get_Domain(1);
        Thread  *t;

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){begin_c_loop_int(i_fluent_cell, t){

            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

            if((i_cell > 0) && (i_cell < int_cell_count)){
                C_UDMI(i_fluent_cell, t, 0) = cell_data[i_cell][MIX1];
                C_UDMI(i_fluent_cell, t, 1) = cell_data[i_cell][MIX2];
            }
        }end_c_loop_int(i_fluent_cell, t)}}

        rcfd_report0(handle, RCFD_REPORT_INFO, "user_post: set Fluent UDMI fields");
    }
#endif
}

/******************************************************************************
 * register user functions
 *****************************************************************************/

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
