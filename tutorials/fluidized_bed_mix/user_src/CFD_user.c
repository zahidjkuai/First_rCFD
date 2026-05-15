/******************************************************************************
 * @copyright   (C) 2026
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#include "udf.h"
#include <stdio.h>

#if RP_NODE

enum{
    GAS,
    SOLID,
    PHASE_COUNT
};

enum{
    MIX_SOLID,
    SEC_GAS,
    UDSI_COUNT
};

enum{
    MIX_SOLID_VOF,
    UDMI_COUNT
};

static int startup_energy_monitor_initialized = 0;
static int mix_ind_initialized = 0;
static int mass_monitor_initialized = 0;

static int    dmean_initialized = 0;
static double dmean_mix_0 = 0.0;

#endif

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_energy)
/*****************************************************************************/
{
#if RP_NODE
    int       i_cell;

    double    vel_mag;
    double    resolved_energy[2], vel[3];

    Domain    *d = Get_Domain(1);
    Thread    *t = NULL, *t_gas = NULL, *t_solid = NULL;

    FILE      *f_out = NULL;
    char      filename[80] = "CFD_start_energy.out";

    // calc total kinetic energy (resolved and unresolved)
    {
        resolved_energy[GAS]   = 0.0;
        resolved_energy[SOLID] = 0.0;

        thread_loop_c(t,d){

            t_gas =   THREAD_SUB_THREAD(t, GAS);
            t_solid = THREAD_SUB_THREAD(t, SOLID);

            begin_c_loop_int(i_cell, t){

                NV_D(vel, =, C_U(i_cell, t_gas), C_V(i_cell, t_gas), C_W(i_cell, t_gas));
                vel_mag = NV_MAG(vel);

                resolved_energy[GAS] +=   C_VOLUME(i_cell, t)
                                        * C_R(i_cell, t_gas)
                                        * C_VOF(i_cell, t_gas)
                                        * vel_mag * vel_mag / 2.0;

                NV_D(vel, =, C_U(i_cell, t_solid), C_V(i_cell, t_solid), C_W(i_cell, t_solid));
                vel_mag = NV_MAG(vel);

                resolved_energy[SOLID] += C_VOLUME(i_cell, t)
                                        * C_R(i_cell, t_solid)
                                        * C_VOF(i_cell, t_gas)
                                        * vel_mag * vel_mag / 2.0;

            }end_c_loop_int(i_cell, t)
        }

        resolved_energy[GAS]    = PRF_GRSUM1(resolved_energy[GAS]);
        resolved_energy[SOLID]  = PRF_GRSUM1(resolved_energy[SOLID]);
    }


    // node-0 writes energies
    if(myid == 0){

        // open file
        {
            if(startup_energy_monitor_initialized == 0){

                f_out = fopen(filename, "w");
                startup_energy_monitor_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                Message0("\nERROR: Failed to open %s", filename);
                return;
            }
        }

        fprintf(f_out, "%e %e %e\n", CURRENT_TIME, resolved_energy[GAS], resolved_energy[SOLID]);

        fclose(f_out);
    }

    Message0("\nCFD_write_energy\n");

#endif
}

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_mix_ind)
/*****************************************************************************/
{
#if RP_NODE
    int       i_cell;

    double    mass;
    double    mass_mix, mean_mix, dmean_mix, mix_ind;

    Domain    *d = Get_Domain(1);
    Thread    *t = NULL, *t_solid = NULL;

    FILE      *f_out = NULL;
    char      filename[80] = "CFD_mix_ind.out";

    // calc mean concs
    {
        mass =     0.0;
        mass_mix = 0.0;
        mean_mix = 0.0;

        thread_loop_c(t,d){

            t_solid = THREAD_SUB_THREAD(t, SOLID);

            begin_c_loop_int(i_cell,t){

                mass     += C_VOLUME(i_cell, t) * C_VOF(i_cell, t_solid) * C_R(i_cell, t_solid);
                mass_mix += C_VOLUME(i_cell, t) * C_VOF(i_cell, t_solid) * C_R(i_cell, t_solid) * C_UDSI(i_cell, t_solid, MIX_SOLID);

            }end_c_loop_int(i_cell,t)
        }

        mass =     PRF_GRSUM1(mass);
        mass_mix = PRF_GRSUM1(mass_mix);

        if(mass > 0.0){

            mean_mix = mass_mix / mass;
        }
    }

    // calc dmean concs
    {
        mass_mix =  0.0;
        dmean_mix = 0.0;

        thread_loop_c(t,d){

            t_solid = THREAD_SUB_THREAD(t, SOLID);

            begin_c_loop_int(i_cell,t){

                mass_mix +=   C_VOLUME(i_cell, t)
                            * C_VOF(i_cell, t_solid)
                            * C_R(i_cell, t_solid)
                            * fabs(C_UDSI(i_cell, t_solid, MIX_SOLID) - mean_mix);

            }end_c_loop_int(i_cell,t)
        }

        mass_mix = PRF_GRSUM1(mass_mix);

        if(mass > 0.0){

            dmean_mix = mass_mix / mass;
        }

        if(dmean_initialized == 0){

            dmean_mix_0 = dmean_mix;
            dmean_initialized = 1;
        }
    }

    // node-0 writes mixing indices
    if(myid == 0){

        // open file
        {
            if(mix_ind_initialized == 0){

                f_out = fopen(filename, "w");
                mix_ind_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                Message0("\nERROR: Could not open %s", filename);
                return;
            }
        }

        mix_ind = 1.0 - dmean_mix/dmean_mix_0;

        fprintf(f_out, "%e %e\n", CURRENT_TIME, mix_ind);

        fclose(f_out);
    }

    Message0("\nCFD_write_mix_ind\n");

#endif
}

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_mass)
/*****************************************************************************/
{
#if RP_NODE
    int       i_cell;

    double    sec_gas_mass;

    Domain    *d = Get_Domain(1);
    Thread    *t = NULL, *t_gas = NULL;

    FILE      *f_out = NULL;
    char      filename[80] = "CFD_mass.out";

    // calc sec_gas_mass
    {
        sec_gas_mass = 0.0;

        thread_loop_c(t,d){

            t_gas = THREAD_SUB_THREAD(t, GAS);

            begin_c_loop_int(i_cell,t){

                sec_gas_mass +=   C_VOLUME(i_cell, t)
                                * C_VOF(i_cell, t_gas)
                                * C_R(i_cell, t_gas)
                                * C_UDSI(i_cell, t_gas, SEC_GAS);

            }end_c_loop_int(i_cell,t)
        }

        sec_gas_mass = PRF_GRSUM1(sec_gas_mass);
    }

    // node-0 writes CoGs
    if(myid == 0){

        // open file
        {
            if(mass_monitor_initialized == 0){

                f_out = fopen(filename, "w");
                mass_monitor_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                Message0("\nERROR: Could not open %s", filename);
                return;
            }
        }

        fprintf(f_out, "%e %e\n", CURRENT_TIME, sec_gas_mass);

        fclose(f_out);
    }

    Message0("\nCFD_write_mass\n");

#endif
}


/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_set_solid_mix_UDMI)
/*****************************************************************************/
{
#if RP_NODE
    int       i_cell;

    Domain    *d = Get_Domain(1);
    Thread    *t = NULL, *t_solid = NULL;

    // set UDMI field for MIX_SOLID * vof
    {
        thread_loop_c(t,d){

            t_solid = THREAD_SUB_THREAD(t, SOLID);

            begin_c_loop_int(i_cell,t){

                C_UDMI(i_cell, t, MIX_SOLID_VOF) = C_VOF(i_cell, t_solid) * C_UDSI(i_cell, t_solid, MIX_SOLID);

            }end_c_loop_int(i_cell,t)
        }
    }

    Message0("\nCFD_set_solid_mix_UDMI\n");

#endif
}
