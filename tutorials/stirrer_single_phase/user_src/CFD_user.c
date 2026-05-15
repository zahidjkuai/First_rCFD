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
static int startup_energy_monitor_initialized = 0;
static int mix_ind_initialized = 0;
static int dmean_initialized = 0;

static double dmean_mix_z0 = 0.0;
static double dmean_mix_r0 = 0.0;
#endif

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_startup_energy)
/*****************************************************************************/
{
#if RP_NODE

    int     i_cell;

    double  resolved_energy, unresolved_energy, vel_mag;
    double  vel[3];

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "CFD_startup_energy.out";

    // calc total kinetic energy (resolved and unresolved)
    {
        resolved_energy =   0.0;
        unresolved_energy = 0.0;

        thread_loop_c(t,d){ begin_c_loop_int(i_cell, t){

            NV_D(vel, =, C_U(i_cell, t), C_V(i_cell, t), C_W(i_cell, t));
            vel_mag = NV_MAG(vel);

            resolved_energy   += C_VOLUME(i_cell, t) * C_R(i_cell, t) * vel_mag * vel_mag / 2.0;
            unresolved_energy += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_K(i_cell, t)    / 2.0;

        }end_c_loop_int(i_cell, t)}

        resolved_energy =   PRF_GRSUM1(resolved_energy  );
        unresolved_energy = PRF_GRSUM1(unresolved_energy);
    }


    // node-0 writes mixing indices
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

        fprintf(f_out, "%e %e %e\n", CURRENT_TIME, resolved_energy, unresolved_energy);

        fclose(f_out);
    }

    Message0("\nCFD_write_startup_energy\n");

#endif
}


/*************************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_mixing_indices)
/*************************************************************************************/
{
#if RP_NODE

    int     i_cell;

    double  mass, mass_mix_z, mass_mix_r;
    double  mean_mix_z, mean_mix_r;
    double  dmean_mix_z, dmean_mix_r;
    double  mix_ind_z, mix_ind_r;

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "CFD_mix_ind.out";

    // calc mean concs
    {
        mass =       0.0;
        mass_mix_z = 0.0;       mass_mix_r = 0.0;
        mean_mix_z = 0.0;       mean_mix_r = 0.0;

        thread_loop_c(t,d){ begin_c_loop_int(i_cell,t){

            mass +=       C_VOLUME(i_cell, t) * C_R(i_cell, t);
            mass_mix_z += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 0);
            mass_mix_r += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 1);

        }end_c_loop_int(i_cell,t)}

        mass =       PRF_GRSUM1(mass);
        mass_mix_z = PRF_GRSUM1(mass_mix_z);
        mass_mix_r = PRF_GRSUM1(mass_mix_r);

        if(mass > 0.0){

            mean_mix_z = mass_mix_z / mass;
            mean_mix_r = mass_mix_r / mass;
        }
    }

    // calc dmean concs
    {
        mass_mix_z =  0.0;      mass_mix_r =  0.0;
        dmean_mix_z = 0.0;      dmean_mix_r = 0.0;

        thread_loop_c(t,d){ begin_c_loop_int(i_cell,t){

            mass_mix_z += C_VOLUME(i_cell, t) * C_R(i_cell, t) * fabs(C_UDSI(i_cell, t, 0) - mean_mix_z);
            mass_mix_r += C_VOLUME(i_cell, t) * C_R(i_cell, t) * fabs(C_UDSI(i_cell, t, 1) - mean_mix_r);

        }end_c_loop_int(i_cell,t)}

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
                mix_ind_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                Message0("\nERROR: Failed to open %s", filename);
                return;
            }
        }

        mix_ind_z = 1.0 - dmean_mix_z/dmean_mix_z0;
        mix_ind_r = 1.0 - dmean_mix_r/dmean_mix_r0;

        fprintf(f_out, "%e %e %e\n", CURRENT_TIME, mix_ind_z, mix_ind_r);

        fclose(f_out);
    }

    Message0("\nCFD_write_mixing_indices\n");

#endif
}
