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

/******************************************************************************
 * user enums, definitions, global vars & auxiliary functions
 *****************************************************************************/

// enum for phases
enum{
    GAS,
    SOLID,
    PHASE_COUNT
};

// enum for gas data
enum{
    GAS_MIX,
    GAS_DATA_COUNT
};

// enum for solid data
enum{
    SOLID_MIX,
    SOLID_DATA_COUNT
};

// enum for Ansys Fluent UDMIs (postproc)
enum{
    UDMI_solid_conc_vof,
    UDMI_sec_gas_conc,
    UDMI_solid_vof,
    UDMI_COUNT
};

#define SEC_GAS_MASSFLOW    7.43e-5 // (kg/s)
#define OUTLET_GAS_MASSFLOW 3.49e-3

#if RP_NODE
static int sec_gas_mass_initialized = 0;
static int mix_ind_initialized = 0;

static int    dmean_solid_mix_initialized = 0;
static double dmean_solid_mix_0 = 0.0;
#endif

/******************************************************************************
 * user dictionaries
 *****************************************************************************/

void user_set_solver_dict(void *handle)
{
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "report_priority", RCFD_REPORT_INFO);
    rcfd_set_dict_property_r(handle, "Solver_Dict", -1, -1, -1, "global_time_step", 0.01);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_frames", 500);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_phases", PHASE_COUNT);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "local_diff_on", 0);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "control_conc_sum_on", 0);  // since mix fields do not sum up to one
}

void user_set_phase_dict(void *handle)
{
#if RP_NODE
    rcfd_set_dict_property_i(handle, "Phase_Dict", -1, GAS,   -1, "number_of_data", 1);
    rcfd_set_dict_property_i(handle, "Phase_Dict", -1, SOLID, -1, "number_of_data", 1);

    rcfd_set_dict_property_r(handle, "Phase_Dict", -1, GAS,   -1, "density", 1.225);
    rcfd_set_dict_property_r(handle, "Phase_Dict", -1, SOLID, -1, "density", 1400.0);
#endif
}

void user_set_rec_dict(void *handle)
{
    rcfd_set_dict_property_s(handle, "Rec_Dict", -1, -1, -1, "format", "replay");
}

void user_set_data_dict(void *handle)
{
#if RP_NODE
    rcfd_set_dict_property_s(handle, "Data_Dict", -1, GAS,   GAS_MIX,   "type", "concentration");
    rcfd_set_dict_property_r(handle, "Data_Dict", -1, GAS,   GAS_MIX,   "face_swap_diffusion", 1.5);

    rcfd_set_dict_property_s(handle, "Data_Dict", -1, SOLID, SOLID_MIX, "type", "concentration");
    rcfd_set_dict_property_r(handle, "Data_Dict", -1, SOLID, SOLID_MIX, "face_swap_diffusion", 0.1);
#endif
}

/******************************************************************************
 * user data initialization
 *****************************************************************************/

void user_init_data(void *handle, const short i_layer)
{
#if RP_NODE
    int     i_cell;
    int     int_cell_count;
    double  **cell_center = NULL;
    double  **data_solid = NULL, **data_gas = NULL;

    // access data
    {
        int_cell_count  = rcfd_get_int_cell_count(handle, i_layer);
        cell_center     = rcfd_get_cell_center   (handle, i_layer);
        data_gas        = rcfd_get_cell_data     (handle, i_layer, GAS);
        data_solid      = rcfd_get_cell_data     (handle, i_layer, SOLID);

        if(!data_gas || !data_solid || !cell_center){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_init_data: var == NULL");
            return;
        }
    }

    // init data
    {
        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            // cell_data_gas
            if(cell_center[i_cell][1] < -0.075){

                data_gas[i_cell][GAS_MIX] = 1.0;
            }
            else{

                data_gas[i_cell][GAS_MIX] = 0.0;
            }

            // cell_data_solid
            if(cell_center[i_cell][1] > 0.0){

                data_solid[i_cell][SOLID_MIX] = 1.0;
            }
            else{

                data_solid[i_cell][SOLID_MIX] = 0.0;
            }
        }
    }

    rcfd_report0(handle, RCFD_REPORT_INFO, "user_init_data");
#endif
}

/******************************************************************************
 * user data access
 *****************************************************************************/

void user_access_data_before_shift(void *handle, const short i_phase)
{
#if RP_NODE
    if(i_phase == SOLID) return;   // no need to call this fct for solids

    int     i_layer, i_frame, i_cell;
    int     int_cell_count;

    double  per_node_sec_inlet_volume, per_node_outlet_volume;
    double  per_node_outlet_gas_mass, per_node_outlet_sec_gas_mass, cell_gas_mass, density_gas;
    double  sec_inlet_volume_share, global_sec_inlet_volume, balance_source, global_timestep;
    double  outlet_volume_share, outlet_sec_gas_mean, global_outlet_volume, global_outlet_gas_mass, global_outlet_sec_gas_mass;

    double  *cell_volume = NULL;
    double  **data_gas = NULL, **cell_center = NULL, **cell_vof = NULL;

    // access rCFD vars, init vars, sanity check
    {
        i_layer         = rcfd_get_current_layer(handle);
        i_frame         = rcfd_get_current_frame(handle);
        int_cell_count  = rcfd_get_int_cell_count(handle, i_layer);
        global_timestep = rcfd_get_timestep(handle, i_layer);
        density_gas     = rcfd_get_phase_density(handle, GAS);
        cell_volume     = rcfd_get_cell_volume(handle, i_layer);
        cell_center     = rcfd_get_cell_center(handle, i_layer);
        data_gas        = rcfd_get_cell_data(handle, i_layer, GAS);
        cell_vof        = rcfd_get_cell_vof(handle, i_layer, i_frame);

        if(!cell_volume || !data_gas || !cell_vof){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_access_data_before_shift: var == NULL");
            return;
        }
    }

    // set BC's
    {
        per_node_sec_inlet_volume    = 0.0;
        per_node_outlet_volume       = 0.0;
        per_node_outlet_gas_mass     = 0.0;
        per_node_outlet_sec_gas_mass = 0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            if(cell_center[i_cell][1] < -0.075){        // sec inlet

                data_gas[i_cell][GAS_MIX] = 1.0;
                per_node_sec_inlet_volume += cell_volume[i_cell];
            }
            else if(cell_center[i_cell][2] > 0.4){      // outlet

                cell_gas_mass = cell_volume[i_cell] * cell_vof[i_cell][GAS] * density_gas;

                per_node_outlet_gas_mass     += cell_gas_mass;
                per_node_outlet_sec_gas_mass += cell_gas_mass * data_gas[i_cell][GAS_MIX];
                per_node_outlet_volume       += cell_volume[i_cell];
            }
        }
    }

    // balances
    {
        // sec gas inlet
        {
            sec_inlet_volume_share = 0.0;

            global_sec_inlet_volume = PRF_GRSUM1(per_node_sec_inlet_volume);

            if(global_sec_inlet_volume > 0){

                sec_inlet_volume_share = per_node_sec_inlet_volume / global_sec_inlet_volume;
            }

            balance_source = SEC_GAS_MASSFLOW * sec_inlet_volume_share *  global_timestep;

            rcfd_modify_balance_mass_source_one(handle, GAS, GAS_MIX, balance_source);
        }

        // gas outlet
        {
            outlet_volume_share = 0.0;
            outlet_sec_gas_mean = 0.0;

            global_outlet_volume        = PRF_GRSUM1(per_node_outlet_volume);
            global_outlet_gas_mass      = PRF_GRSUM1(per_node_outlet_gas_mass);
            global_outlet_sec_gas_mass  = PRF_GRSUM1(per_node_outlet_sec_gas_mass);

            if(global_outlet_volume > 0){

                outlet_volume_share = per_node_outlet_volume / global_outlet_volume;
            }

            if(global_outlet_sec_gas_mass > 0){

                outlet_sec_gas_mean = global_outlet_sec_gas_mass / global_outlet_gas_mass;
            }

            balance_source = -1.0 * OUTLET_GAS_MASSFLOW * outlet_sec_gas_mean * outlet_volume_share *  global_timestep;

            rcfd_modify_balance_mass_source_one(handle, GAS, GAS_MIX, balance_source);
        }
    }
#endif
}

void user_post(void *handle)
{
#if RP_NODE
    int     i_layer, i_frame, i_cell;
    int     int_cell_count;

    double  density_solid, density_gas;
    double  mass_solid, mass_solid_mix, mass_sec_gas, cell_mass_gas, cell_mass_solid, mean_solid_mix;
    double  dmean_solid_mix, solid_mix_ind;
    double  current_time;

    double  *cell_volume = NULL;
    double  **cell_vof = NULL;
    double  **data_gas = NULL, **data_solid = NULL;

    FILE    *f_out = NULL;
    char    filename_mix [80] = "rCFD_mix_ind.out";
    char    filename_mass[80] = "rCFD_mass.out";

    // access data
    {
        i_layer = rcfd_get_current_layer(handle);
        i_frame = rcfd_get_current_frame(handle);

        int_cell_count = rcfd_get_int_cell_count(handle, i_layer);

        density_solid = rcfd_get_phase_density(handle, SOLID);
        density_gas   = rcfd_get_phase_density(handle, GAS);

        current_time = rcfd_get_time(handle);

        cell_volume = rcfd_get_cell_volume(handle, i_layer);

        data_gas   = rcfd_get_cell_data(handle, i_layer, GAS);
        data_solid = rcfd_get_cell_data(handle, i_layer, SOLID);

        cell_vof = rcfd_get_cell_vof(handle, i_layer, i_frame);

        if((data_gas == NULL)||(data_solid == NULL)||(cell_volume == NULL)||(cell_vof == NULL)){

            rcfd_report0(handle, RCFD_REPORT_ERROR, "user_post: var == NULL");

            return;
        }
    }

    // calc mean solid mix, mass sec gas
    {
        mass_solid =     0.0;
        mass_solid_mix = 0.0;
        mass_sec_gas =   0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_mass_gas   = cell_volume[i_cell] * cell_vof[i_cell][GAS]   * density_gas;
            cell_mass_solid = cell_volume[i_cell] * cell_vof[i_cell][SOLID] * density_solid;

            mass_solid +=     cell_mass_solid;
            mass_solid_mix += cell_mass_solid * data_solid[i_cell][SOLID_MIX];

            mass_sec_gas += cell_mass_gas * data_gas[i_cell][GAS_MIX];
        }

        mass_solid     = PRF_GRSUM1(mass_solid);
        mass_solid_mix = PRF_GRSUM1(mass_solid_mix);
        mass_sec_gas   = PRF_GRSUM1(mass_sec_gas);

        mean_solid_mix = 0.0;

        if(mass_solid > 0.0){

            mean_solid_mix = mass_solid_mix / mass_solid;
        }

    }

    // calc dmean solid mix
    {
        mass_solid_mix = 0.0;

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_mass_solid = cell_volume[i_cell] * cell_vof[i_cell][SOLID] * density_solid;
            mass_solid_mix += cell_mass_solid * fabs(data_solid[i_cell][SOLID_MIX] - mean_solid_mix);
        }

        mass_solid_mix = PRF_GRSUM1(mass_solid_mix);
        dmean_solid_mix = 0.0;

        if(mass_solid > 0.0){

            dmean_solid_mix = mass_solid_mix / mass_solid;
        }

        if(dmean_solid_mix_initialized == 0){

            dmean_solid_mix_0 = dmean_solid_mix;
            dmean_solid_mix_initialized = 1;
        }
    }

    // node-0 writes mixing indice, sec mass
    if(myid == 0){

        // open file for mixing index
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

        solid_mix_ind = 0.0;

        if(dmean_solid_mix_0 > 0.0){

            solid_mix_ind = 1.0 - dmean_solid_mix / dmean_solid_mix_0;
        }

        fprintf(f_out, "%e %e\n", current_time, solid_mix_ind);
        fclose(f_out);

        // open file for sec mass
        {
            if(sec_gas_mass_initialized == 0){

                f_out = fopen(filename_mass, "w");
                rcfd_report0(handle, RCFD_REPORT_INFO, "user_post: initialized %s", filename_mass);
                sec_gas_mass_initialized = 1;
            }
            else{

                f_out = fopen(filename_mass, "a");
            }

            if(f_out == NULL){

                rcfd_report0(handle, RCFD_REPORT_ERROR, "Failed to open %s", filename_mass);
                return;
            }
        }

        fprintf(f_out, "%e %e\n", current_time, mass_sec_gas);
        fclose(f_out);
    }

    // nodes write data to UDMI
    {
        int     i_fluent_cell;
        Domain  *d = Get_Domain(1);
        Thread  *t;

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){begin_c_loop_int(i_fluent_cell, t){

            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

            if((i_cell > 0) && (i_cell < int_cell_count)){

                C_UDMI(i_fluent_cell, t, UDMI_solid_conc_vof) = data_solid[i_cell][SOLID_MIX] * cell_vof[i_cell][SOLID];
                C_UDMI(i_fluent_cell, t, UDMI_sec_gas_conc)   = data_gas  [i_cell][GAS_MIX];
                C_UDMI(i_fluent_cell, t, UDMI_solid_vof)      = cell_vof  [i_cell][SOLID];
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
    REGISTER_RCFD_UDF( user_access_data_before_shift );
    REGISTER_RCFD_UDF( user_post );
}

#endif
