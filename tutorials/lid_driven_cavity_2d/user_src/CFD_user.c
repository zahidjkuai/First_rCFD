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
static int CoG_monitor_initialized = 0;

static int    dmean_initialized = 0;
static double dmean_mix1_0 = 0.0;
static double dmean_mix2_0 = 0.0;
#endif

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_energy)
/*****************************************************************************/
{
#if RP_NODE
    int     i_cell;

    double  resolved_energy, unresolved_energy, vel_mag;
    double  vel[3];

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "CFD_start_energy.out";

    // calc total kinetic energy (resolved and unresolved)
    {
        resolved_energy =   0.0;
        unresolved_energy = 0.0;

        thread_loop_c(t,d){ begin_c_loop_int(i_cell, t){

            NV_D(vel, =, C_U(i_cell, t), C_V(i_cell, t), C_W(i_cell, t));
            vel_mag = NV_MAG(vel);

            resolved_energy   += C_VOLUME(i_cell, t) * C_R(i_cell, t) * vel_mag * vel_mag / 2.0;
            //unresolved_energy += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_K(i_cell, t);

        }end_c_loop_int(i_cell, t)}

        resolved_energy =   PRF_GRSUM1(resolved_energy  );
        unresolved_energy = PRF_GRSUM1(unresolved_energy);
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

                Message0("\nERROR: Could not open %s", filename);
                return;
            }
        }

        fprintf(f_out, "%e %e %e\n", CURRENT_TIME, resolved_energy, unresolved_energy);
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
    int     i_cell;

    double  mass;
    double  mass_mix1, mean_mix1, dmean_mix1, mix_ind1;
    double  mass_mix2, mean_mix2, dmean_mix2, mix_ind2;

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "CFD_mix_ind.out";

    // calc mean concs
    {
        mass      = 0.0;
        mass_mix1 = 0.0; mass_mix2 = 0.0;
        mean_mix1 = 0.0; mean_mix2 = 0.0;

        thread_loop_c(t,d){ begin_c_loop_int(i_cell,t){

            mass +=      C_VOLUME(i_cell, t) * C_R(i_cell, t);
            mass_mix1 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 0);
            mass_mix2 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 1);

        }end_c_loop_int(i_cell,t)}

        mass =      PRF_GRSUM1(mass);
        mass_mix1 = PRF_GRSUM1(mass_mix1);
        mass_mix2 = PRF_GRSUM1(mass_mix2);

        if(mass > 0.0){

            mean_mix1 = mass_mix1 / mass;
            mean_mix2 = mass_mix2 / mass;
        }
    }

    // calc dmean concs
    {
        mass_mix1 =  0.0; mass_mix2 =  0.0;
        dmean_mix1 = 0.0; dmean_mix2 = 0.0;

        thread_loop_c(t,d){ begin_c_loop_int(i_cell,t){

            mass_mix1 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * fabs(C_UDSI(i_cell, t, 0) - mean_mix1);
            mass_mix2 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * fabs(C_UDSI(i_cell, t, 1) - mean_mix2);

        }end_c_loop_int(i_cell,t)}

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

        mix_ind1 = 1.0 - dmean_mix1/dmean_mix1_0;
        mix_ind2 = 1.0 - dmean_mix2/dmean_mix2_0;

        fprintf(f_out, "%e %e %e\n", CURRENT_TIME, mix_ind1, mix_ind2);

        fclose(f_out);
    }

    Message0("\nCFD_write_mix_ind\n");
#endif
}

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_CoG)
/*****************************************************************************/
{
#if RP_NODE
    int     i_cell;

    double  mass;
    double  mass_x1, mass_y1, mass_z1, CoG_x1, CoG_y1, CoG_z1;
    double  mass_x2, mass_y2, mass_z2, CoG_x2, CoG_y2, CoG_z2;
    double  x[3];

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "CFD_CoG.out";

    // calc mean CoGs
    {
        mass    = 0.0;
        mass_x1 = 0.0;      CoG_x1 = 0.0;
        mass_y1 = 0.0;      CoG_y1 = 0.0;
        mass_z1 = 0.0;      CoG_z1 = 0.0;
        mass_x2 = 0.0;      CoG_x2 = 0.0;
        mass_y2 = 0.0;      CoG_y2 = 0.0;
        mass_z2 = 0.0;      CoG_z2 = 0.0;

        thread_loop_c(t,d){ begin_c_loop_int(i_cell,t){

            C_CENTROID(x, i_cell, t);

            mass    += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 0);
            mass_x1 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 0) * x[0];
            mass_y1 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 0) * x[1];
            mass_z1 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 0) * x[2];
            mass_x2 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 1) * x[0];
            mass_y2 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 1) * x[1];
            mass_z2 += C_VOLUME(i_cell, t) * C_R(i_cell, t) * C_UDSI(i_cell, t, 1) * x[2];

        }end_c_loop_int(i_cell,t)}

        mass    = PRF_GRSUM1(mass  );
        mass_x1 = PRF_GRSUM1(mass_x1);
        mass_y1 = PRF_GRSUM1(mass_y1);
        mass_z1 = PRF_GRSUM1(mass_z1);
        mass_x2 = PRF_GRSUM1(mass_x2);
        mass_y2 = PRF_GRSUM1(mass_y2);
        mass_z2 = PRF_GRSUM1(mass_z2);

        if(mass > 0.0){

            CoG_x1 = mass_x1 / mass;
            CoG_y1 = mass_y1 / mass;
            CoG_z1 = mass_z1 / mass;
            CoG_x2 = mass_x2 / mass;
            CoG_y2 = mass_y2 / mass;
            CoG_z2 = mass_z2 / mass;
        }
    }

    // node-0 writes CoGs
    if(myid == 0){

        // open file
        {
            if(CoG_monitor_initialized == 0){

                f_out = fopen(filename, "w");
                CoG_monitor_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                Message0("\nERROR: Could not open %s", filename);
                return;
            }
        }

        fprintf(f_out, "%e %e %e %e %e %e %e\n",
                CURRENT_TIME,
                CoG_x1, CoG_y1, CoG_z1,
                CoG_x2, CoG_y2, CoG_z2);
        fclose(f_out);
    }

    Message0("\nCFD_write_CoG\n");
#endif
}
