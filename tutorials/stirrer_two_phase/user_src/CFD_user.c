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
static int gas_monitor_initialized = 0;
static int BC_gas_initialized = 0;
static int CFD_oxy_initialized = 0;

#define BC_GAS_ID    12     // Fluent patch id

#define GAS_PHASE_ID 1      // Fluent phase ids
#define LIQ_PHASE_ID 0

#define SINK_COEF    2.0e0

#define OXY_INIT              8e-6
#define OXY_EQ               10e-6
#define VOF_CUTOFF          1.0e-3
#define BUBBLE_DIAM         1.0e-2
#define REACT_TRANS_COEFF   1.0e3       // (1/m2.s)

enum{
    BC_GAS,
    OXY_REACT_RATE,
    OXY_REACT_AREA,
    OXY_BUBBLE_VOF,
    UDMI_COUNT
};

enum{
    OXY,
    USDI_COUNT
};

#endif

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_energy)
/*****************************************************************************/
{
#if RP_NODE
    int       i_cell;
    double    resolved_energy, unresolved_energy, vel_mag;
    double    vel[3];

    Domain    *d = Get_Domain(1);
    Thread    *t_mix = NULL, *t_liq = NULL;

    FILE      *f_out = NULL;
    char      filename[80] = "CFD_start_energy.out";

    // calc total kinetic energy (resolved and unresolved)
    {
        resolved_energy =   0.0;
        unresolved_energy = 0.0;

        thread_loop_c(t_mix,d){

            t_liq = THREAD_SUB_THREAD(t_mix, LIQ_PHASE_ID);

            begin_c_loop_int(i_cell, t_mix){

                NV_D(vel, =, C_U(i_cell, t_liq), C_V(i_cell, t_liq), C_W(i_cell, t_liq));
                vel_mag = NV_MAG(vel);

                resolved_energy   += C_VOLUME(i_cell, t_mix) * C_R(i_cell, t_liq) * vel_mag * vel_mag  / 2.0;
                unresolved_energy += C_VOLUME(i_cell, t_mix) * C_R(i_cell, t_mix) * C_K(i_cell, t_mix) / 2.0;

            }end_c_loop_int(i_cell, t_mix)
        }

        resolved_energy =   PRF_GRSUM1(resolved_energy);
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

    Message0("\nCFD_write_energy\n");
#endif
}

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_gas_vol)
/*****************************************************************************/
{
#if RP_NODE
    int     i_cell;
    double  vol_gas, vol_gas_global;

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL, *tg = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "CFD_start_gas_volume.out";

    // calc gas vol
    {
        vol_gas = 0.0;

        thread_loop_c(t,d){

            tg = THREAD_SUB_THREAD(t, GAS_PHASE_ID);

            begin_c_loop_int(i_cell,t){

                vol_gas += C_VOLUME(i_cell, t) * C_VOF(i_cell, tg);

            }end_c_loop_int(i_cell,t)
        }

        vol_gas_global = PRF_GRSUM1(vol_gas);
    }

    // node-0 writes gas_vol_global
    if(myid == 0){

        // open file
        {
            if(gas_monitor_initialized == 0){

                f_out = fopen(filename, "w");
                gas_monitor_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                Message0("\nERROR: Failed to open %s", filename);
                return;
            }
        }

        fprintf(f_out, "%e %e\n", CURRENT_TIME, vol_gas_global);
        fclose(f_out);
    }

    Message0("\nCFD_write_gas_vol\n");

#endif
}

/*****************************************************************************/
DEFINE_ON_DEMAND(CFD_define_BC_gas)
/*****************************************************************************/
{
#if RP_NODE
    int     i_cell, i_face;
    int     c0;

    Domain  *d = Get_Domain(1);
    Thread  *tf = NULL, *tc = NULL;

    if(BC_gas_initialized == 0){

        thread_loop_c(tc, d){begin_c_loop(i_cell, tc){

            C_UDMI(i_cell, tc, BC_GAS) = 0.0;

        }end_c_loop(i_cell, tc)}

        thread_loop_f(tf, d){

            if(THREAD_ID(tf) == BC_GAS_ID){

                begin_f_loop(i_face, tf){

                    c0 = F_C0       (i_face, tf);
                    tc = F_C0_THREAD(i_face, tf);

                    C_UDMI(c0, tc, BC_GAS) = 1.0;

                }end_f_loop(i_face, tf)
            }
        }

        BC_gas_initialized = 1;
    }
#endif
}

/*****************************************************************************/
DEFINE_SOURCE(CFD_gas_sink, i_cell, t_gas, dS, eqn)
/*****************************************************************************/
{
#if RP_NODE

    if(BC_gas_initialized == 0) return 0.0;

    if(!t_gas) return 0.0;

    Thread *t_mix = THREAD_SUPER_THREAD(t_gas);

    if(C_UDMI(i_cell, t_mix, BC_GAS) > 0.0){

        dS[eqn] = - SINK_COEF * C_R(i_cell, t_gas) / CURRENT_TIMESTEP;

        return dS[eqn] * C_VOF(i_cell, t_gas);   // (kg/m3.s)
    }

#endif
    return 0.0;
}

/*****************************************************************************/
DEFINE_ON_DEMAND(CFD_init_Oxy)
/*****************************************************************************/
{
#if RP_NODE
    int     i_cell;

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL, *t_liq = NULL;

    thread_loop_c(t, d){

        t_liq = THREAD_SUB_THREAD(t, LIQ_PHASE_ID);

        if(THREAD_STORAGE(t_liq, SV_UDS_I(OXY))!= NULL){

            begin_c_loop(i_cell, t){

                C_UDSI(i_cell, t_liq, OXY) = OXY_INIT;

            }end_c_loop(i_cell, t)

            Message0("\nCFD_init_Oxy with %e", OXY_INIT);
        }
        else{

            Message0("\nCFD_init_Oxy failed");
        }
    }

#endif
}

/*****************************************************************************/
DEFINE_SOURCE(CFD_Oxy_source, i_cell, t_liq, dS, eqn)
/*****************************************************************************/
{
#if RP_NODE
    double vof_water  = C_VOF(i_cell, t_liq);
    double vof_bubble = 1.0 - vof_water;

    double dmdtOxy =   0.0;
    double cell_area = 0.0;

    Thread *t_mix = THREAD_SUPER_THREAD(t_liq);

    // sanity checks
    if(C_UDSI(i_cell, t_liq, OXY) < OXY_INIT) C_UDSI(i_cell, t_liq, OXY) = OXY_INIT;
    if(C_UDSI(i_cell, t_liq, OXY) > OXY_EQ)   C_UDSI(i_cell, t_liq, OXY) = OXY_EQ;

    dS[eqn] = 0.0;

    if((vof_bubble > VOF_CUTOFF) &&
       (vof_water  > VOF_CUTOFF)){

        cell_area = 6.0 * vof_bubble * C_VOLUME(i_cell, t_mix) / BUBBLE_DIAM;     // disperse sphere approx.

        dmdtOxy = REACT_TRANS_COEFF * cell_area * vof_water * C_R(i_cell, t_liq) * (OXY_EQ - C_UDSI(i_cell, t_liq, OXY)); // (kg/m3.s)

        dS[eqn] = - REACT_TRANS_COEFF * cell_area * vof_water * C_R(i_cell, t_liq);
    }

    C_UDMI(i_cell, t_mix, OXY_REACT_RATE) = dmdtOxy / C_VOLUME(i_cell, t_mix);
    C_UDMI(i_cell, t_mix, OXY_REACT_AREA) = cell_area;
    C_UDMI(i_cell, t_mix, OXY_BUBBLE_VOF) = vof_bubble;

    return dmdtOxy; // (kg/m3.s)
#endif
    return 0.0;
}

/*****************************************************************************/
DEFINE_EXECUTE_AT_END(CFD_write_Oxy_mass)
/*****************************************************************************/
{
#if RP_NODE
    int     i_cell;
    double  oxy_mass, oxy_mass_global;

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL, *t_liq = NULL;

    FILE    *f_out = NULL;
    char    filename[80] = "./CFD_Oxy.out";

    // calc gas vol
    {
        oxy_mass = 0.0;

        thread_loop_c(t,d){

            t_liq = THREAD_SUB_THREAD(t, LIQ_PHASE_ID);

            if((THREAD_STORAGE(t_liq, SV_VOF)       != NULL) &&
               (THREAD_STORAGE(t_liq, SV_UDS_I(OXY))!= NULL)){

                begin_c_loop_int(i_cell, t){

                    oxy_mass += C_UDSI(i_cell, t_liq, OXY) * C_VOF(i_cell, t_liq) * C_VOLUME(i_cell, t) * C_R(i_cell, t_liq);

                }end_c_loop_int(i_cell, t)
            }
        }

        oxy_mass_global = PRF_GRSUM1(oxy_mass);
    }

    // node-0 writes gas_vol_global
    if(myid == 0){

        // open file
        {
            if(CFD_oxy_initialized == 0){

                f_out = fopen(filename, "w");
                CFD_oxy_initialized = 1;
            }
            else{

                f_out = fopen(filename, "a");
            }

            if(f_out == NULL){

                Message0("\nERROR: Failed to open %s", filename);
                return;
            }
        }

        fprintf(f_out, "%e %e\n", CURRENT_TIME, oxy_mass_global);
        fclose(f_out);
    }

    Message0("\nCFD_write_Oxy_mass\n");
#endif
}
