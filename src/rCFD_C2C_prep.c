/******************************************************************************
 * @internal
 * @file        rCFD_C2C_prep.c
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#include <stdio.h>

#include "udf.h"
#include "rCFD_library.h"
#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_fluent.h"
#include "rCFD_user.h"

#if RP_NODE
static int g_uds_offset = 0;
static int g_start_timestep = 0;
#endif

// case-specific udf extensions
#if 1

    // #include "case-specific include files"

#if RP_NODE

    // Fluent Particle Vars
    #define p_just_started      (0 + p_vars_used_by_case)
    #define p_start_time        (1 + p_vars_used_by_case)
    #define p_invalid           (2 + p_vars_used_by_case)
    #define p_phase_id          (3 + p_vars_used_by_case)
    #define p_phase_fraction    (4 + p_vars_used_by_case)
    #define p_c0                (5 + p_vars_used_by_case)
    #define p_node0             (6 + p_vars_used_by_case)
    #define p_w0                (7 + p_vars_used_by_case)
    #define p_c_old             (8 + p_vars_used_by_case)
    #define p_time_ratio        (9 + p_vars_used_by_case)
    #define p_u_rwm             (10 + p_vars_used_by_case)
    #define p_v_rwm             (11 + p_vars_used_by_case)
    #define p_w_rwm             (12 + p_vars_used_by_case)
    #define p_vel_rwm_old       (13 + p_vars_used_by_case)

    static int p_vars_used_by_case = 0;

    void case_specific_scalar_update(int i_fluent_cell, Thread *t, int initialize, Tracked_Particle *p){

        return;
    }

    double case_specific_body_forces(Tracked_Particle *p, int i){

        return 0.0;
    }

#endif // RP_NODE
#endif

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_init_all)
/*************************************************************************************/
{
#if RP_NODE
    g_start_timestep = N_TIME + 1;
#endif
    fluent_init_topo();

    rcfd_open_no_mpi(0, NULL, &myrcfd);

    rcfd_register_user_function(myrcfd, "user_set_node_dict",   (void(*)(void))fluent_set_node_dict);
    rcfd_register_user_function(myrcfd, "user_init_cells",      (void(*)(void))fluent_init_cells);
    rcfd_register_user_function(myrcfd, "user_init_faces",      (void(*)(void))fluent_init_faces);
    rcfd_register_user_function(myrcfd, "user_init_cell_hosts", (void(*)(void))fluent_init_cell_hosts);
    rcfd_register_user_function(myrcfd, "user_init_nodes",      (void(*)(void))fluent_init_nodes);

    user_register_functions(myrcfd);

    rcfd_init(myrcfd, RCFD_MODE_PREPARATION);

    rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                 "rCFD_init_all: rcfd initialized for preparation");
}

/*************************************************************************************/
DEFINE_EXECUTE_AT_END(rCFD_analyse_CFD)
/*************************************************************************************/
{
#if RP_NODE

#if 1   /* local vars */
    int     i_layer, i_phase, i_cell, i_fluent_cell;
    int     phase_count = rcfd_get_phase_count(myrcfd);
    int     time_steps_per_monitoring_interval, number_of_monitoring_steps;
    double  v_mag, dt_cross_min_global;
    double  *dt_cross_min, *dt_cross_max;

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL, *t_phase = NULL;
#endif

    rCFD_type *rcfd = (rCFD_type*)myrcfd;
    i_layer = 0;

    /* 1. allocate & initialize dt_cross */
    {
        dt_cross_min = (double*)malloc(phase_count * sizeof(double));
        dt_cross_max = (double*)malloc(phase_count * sizeof(double));

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            dt_cross_min[i_phase] = 1.e10;
            dt_cross_max[i_phase] = -1.e10;
        }
    }

    /* 2. set C.average_velocity, C.crossing_time, dt_cross_min/max */
    thread_loop_c(t,d){ begin_c_loop_int(i_fluent_cell, t){

        i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

        if(i_cell > -1){

            for(i_phase = 0; i_phase < phase_count; i_phase++){

                double *cell_crossing_time = rcfd_get_cell_crossing_time(myrcfd,i_layer,i_phase);

                /* 2.1. set t_phase */
                {
                    if(phase_count == 1){

                        t_phase = t;
                    }
                    else if(THREAD_SUB_THREAD(t,i_phase) != NULL) {

                        t_phase = THREAD_SUB_THREAD(t,i_phase);
                    }
                    else{

                        rcfd_report0(myrcfd, RCFD_REPORT_ERROR,
                                     "rCFD_analyse_CFD: THREAD_SUB_THREAD(t,i_phase) == NULL");
                        return;
                    }
                }

                /* 2.2. set C.average_velocity, C.crossing_time */
                {
                    double *cell_ave_velocity =  rcfd_get_cell_ave_velocity(myrcfd,i_layer,i_phase);

                    v_mag = sqrt( C_U(i_fluent_cell, t_phase) * C_U(i_fluent_cell, t_phase) +
                                  C_V(i_fluent_cell, t_phase) * C_V(i_fluent_cell, t_phase) +
                                  C_W(i_fluent_cell, t_phase) * C_W(i_fluent_cell, t_phase) );

                    number_of_monitoring_steps = rcfd->Solver->analyse_CFD_counter;

                    cell_ave_velocity[i_cell] = ((double)number_of_monitoring_steps * cell_ave_velocity[i_cell] + v_mag) /
                        (double)(number_of_monitoring_steps + 1);

                    if(cell_ave_velocity[i_cell] > 0.0){

                        cell_crossing_time[i_cell] = _C(i_layer).grid_spacing[i_cell]/cell_ave_velocity[i_cell];
                    }
                    else{

                        cell_crossing_time[i_cell] = 1.0e10; /* something large */
                    }
                }

                /* 2.3 set dt_cross_min/max */
                {
                    if((dt_cross_max[i_phase] < cell_crossing_time[i_cell]) && (cell_crossing_time[i_cell] < 1.e10)){

                        dt_cross_max[i_phase] = cell_crossing_time[i_cell];
                    }

                    if(dt_cross_min[i_phase] > cell_crossing_time[i_cell]){

                        dt_cross_min[i_phase] = cell_crossing_time[i_cell];
                    }
                }
            }
        }
    }end_c_loop_int(i_fluent_cell, t)}


    /* 3. recommend recurrence time-step width */
    {
        dt_cross_min_global = 1.e10;

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            dt_cross_max[i_phase] = PRF_GRHIGH1(dt_cross_max[i_phase]);
            dt_cross_min[i_phase] = PRF_GRLOW1(dt_cross_min[i_phase]);

            if(dt_cross_min[i_phase] < dt_cross_min_global){

                dt_cross_min_global = dt_cross_min[i_phase];
            }

            rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                         "rCFD_analyse_CFD: cell_crossing_time for phase %d (based on %d time-steps): [%e, %e]",
                         i_phase, (rcfd->Solver->analyse_CFD_counter + 1), dt_cross_min[i_phase], dt_cross_max[i_phase]);
        }

        time_steps_per_monitoring_interval = (int)(dt_cross_min_global * (double)rcfd->Solver_Dict->max_number_of_cells_per_time_step / CURRENT_TIMESTEP);

        if(time_steps_per_monitoring_interval < 1){

            time_steps_per_monitoring_interval = 1;

            rcfd_report0(myrcfd, RCFD_REPORT_WARNING,
                         "rCFD_analyse_CFD: set monitoring interval = 1");
        }

        rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                     "rCFD_analyse_CFD: recommend %d time_steps_per_monitoring_interval (based on %d time-steps)",
                     time_steps_per_monitoring_interval, (rcfd->Solver->analyse_CFD_counter + 1));
    }

    /* 4. free local vars */
    {
        free(dt_cross_max);
        free(dt_cross_min);
    }

    rcfd->Solver->analyse_CFD_counter++;

    rcfd->rCFD_UDF->_user_pre_proc(myrcfd);

#endif // RP_NODE
}

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_write_Tracer_Positions)
/*************************************************************************************/
{
#if RP_NODE
    Domain  *d=Get_Domain(1);
    Thread  *t;
    cell_t  c;

    int     start_position_count, start_position_coords_count, global_start_postions_count;
    int     phase_count = rcfd_get_phase_count(myrcfd);
    double  *start_position_coords = NULL, x[3];
    rCFD_type *rcfd = (rCFD_type*)myrcfd;

    /* A: set Tracer_Dict, determine number of start_positions & allocate storage */
    {
        start_position_count = 0;

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){begin_c_loop_int(c,t){

            C_CENTROID(x,c,t);

            if(NO_ROI_DEFINED || CELL_IN_ROI){

                start_position_count++;
            }
        }end_c_loop_int(c,t)}}

        start_position_count *= phase_count * rcfd->Prep_Dict->tracers_per_cell_count;
        global_start_postions_count = PRF_GISUM1(start_position_count);
        start_position_coords_count = 3. * start_position_count;
        start_position_coords = (double*)malloc( start_position_coords_count * sizeof(double));

        rcfd_report0(myrcfd, RCFD_REPORT_DEBUG,
                     "rCFD_write_Tracer_Positions: start_position_count %d",
                     start_position_count);
    }

    /* B: determine position in cell, fill local start_position_coords */
    {
        int i_arr, i_node, i_tracer, i_dim;
        double rand_real;
        int nodes_in_cell, n1_index, n2_index;
        double  x[3],n1[3],n2[3],x_n1[3],x_n2[3],p1[3],p2[3],p1_p2[3],p_start[3];
        Node *n;

        i_arr = 0;

        thread_loop_c(t,d){ if(FLUID_CELL_THREAD_P(t)){ begin_c_loop_int(c,t){

            C_CENTROID(x,c,t);

            if(NO_ROI_DEFINED || CELL_IN_ROI){

                for(i_tracer = 0; i_tracer < phase_count * rcfd->Prep_Dict->tracers_per_cell_count; i_tracer++){

                    /* P1: Chose two nodes */
                    {
                        nodes_in_cell = 0;
                        c_node_loop(c,t,i_node) nodes_in_cell++;

                        rand_real = (double)rand()/(double)RAND_MAX;            /* [0..1] */
                        n1_index = (int)((double)(nodes_in_cell+1)*rand_real);  /* [0..nodes_in_cell] */
                        n2_index = n1_index;

                        while(n2_index == n1_index){

                            rand_real = (double)rand()/(double)RAND_MAX;
                            n2_index = (int)((double)(nodes_in_cell+1)*rand_real);
                        }

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            n1[i_dim]=0.; n2[i_dim]=0.;
                        }

                        c_node_loop(c,t,i_node){

                            if(i_node==n1_index) {n=C_NODE(c,t,i_node);n1[0]=NODE_X(n);n1[1]=NODE_Y(n);n1[2]=NODE_Z(n);}
                            if(i_node==n2_index) {n=C_NODE(c,t,i_node);n2[0]=NODE_X(n);n2[1]=NODE_Y(n);n2[2]=NODE_Z(n);}
                        }
                    }

                    /* P2: Chose two Points between Cell Center and Nodes (closer to nodes) */
                    {
                        NV_VV(x_n1,=,n1,-,x);
                        rand_real=(double)rand()/(double)RAND_MAX;
                        NV_V_VS(p1,=,x,+,x_n1,*,(1.-rand_real*rand_real));
                        NV_VV(x_n2,=,n2,-,x);
                        rand_real=(double)rand()/(double)RAND_MAX;
                        NV_V_VS(p2,=,x,+,x_n2,*,(1.-rand_real*rand_real));
                    }

                    /* P3: Chose final Point between P1 and P2 */
                    {
                        NV_VV(p1_p2,=,p2,-,p1);
                        rand_real=(double)rand()/(double)RAND_MAX;
                        NV_V_VS(p_start,=,p1,+,p1_p2,*,rand_real);

                        if(rcfd->Prep_Dict->tracer_reverse_direction_on){

                            start_position_coords[i_arr] = x[0]; i_arr++;
                            start_position_coords[i_arr] = x[1]; i_arr++;
                            start_position_coords[i_arr] = x[2]; i_arr++;
                        }
                        else{

                            start_position_coords[i_arr] = p_start[0]; i_arr++;
                            start_position_coords[i_arr] = p_start[1]; i_arr++;
                            start_position_coords[i_arr] = p_start[2]; i_arr++;
                        }
                    }
                }
            }
        }end_c_loop_int(c,t)}}
    }

    /* C: communicate to Node-0, which writes to file */
    {
        if(myid > 0){

            PRF_CSEND_INT(node_zero, &start_position_coords_count, 1, myid);
            PRF_CSEND_REAL(node_zero, start_position_coords, start_position_coords_count, myid);

            free(start_position_coords);
        }

        if(myid == 0){

            int     i_node, number_of_lines, i_line;
            FILE    *f_out = fopen(rcfd->File_Dict->tracer_start_position_filename, "w");

            if(f_out == NULL){

                rcfd_report0(myrcfd, RCFD_REPORT_ERROR,
                             "rCFD_write_Tracer_Positions: failed to open file %s",
                             rcfd->File_Dict->tracer_start_position_filename);

                return;
            }

            number_of_lines = start_position_coords_count/3;

            for(i_line = 0; i_line < number_of_lines; i_line++){

                fprintf(f_out,"(( ");
                fprintf(f_out,"%f %f %f", start_position_coords[i_line*3], start_position_coords[i_line*3+1], start_position_coords[i_line*3+2]);
                fprintf(f_out," 0. 0. 0. 1.e-6 273. 1.e-10 ))\n");
            }

            free(start_position_coords);

            compute_node_loop_not_zero(i_node){

                PRF_CRECV_INT(i_node, &start_position_coords_count, 1, i_node);

                start_position_coords = (double*)malloc(start_position_coords_count * sizeof(double));

                PRF_CRECV_REAL(i_node, start_position_coords, start_position_coords_count, i_node);

                number_of_lines = start_position_coords_count/3;

                for(i_line = 0; i_line < number_of_lines; i_line++){

                    fprintf(f_out,"(( ");
                    fprintf(f_out,"%f %f %f", start_position_coords[i_line*3  ],
                                              start_position_coords[i_line*3+1],
                                              start_position_coords[i_line*3+2]);
                    fprintf(f_out," 0. 0. 0. 1.e-6 273. 1.e-10 ))\n");
                }

                free(start_position_coords);
            }

            fclose(f_out);

            rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                         "rCFD_write_Tracer_Positions: wrote %d positions to file %s",
                         global_start_postions_count, rcfd->File_Dict->tracer_start_position_filename);
        }
    }
#endif // RP_NODE
}

/*************************************************************************************/
DEFINE_DPM_INJECTION_INIT(rcfd_init_tracers,I)
/*************************************************************************************/
{
#if RP_NODE

#if 1   /* local vars */
    rCFD_type   *rcfd = (rCFD_type*)myrcfd;

    int         n_time = N_TIME - g_start_timestep;
    int         i_phase, i_phase_fluent, i_cell, i_fluent_cell;
    int         phase_count = rcfd_get_phase_count(myrcfd);
    int         initialized_tracer_count = 0;

    int         *valid_tracer_count = NULL, *invalid_tracer_count = NULL;

    double      rand_real;

    cell_t      c;
    Particle    *p;
    Thread      *t = NULL, *t_phase = NULL;

#endif

    if(!rcfd_is_initialized(myrcfd)){

        rcfd_report0(myrcfd, RCFD_REPORT_ERROR,
                     "rcfd_init_tracers: rcfd is not yet initialized");
        return;
    }

    /* 1. initialize particle vars */
    {

        valid_tracer_count =   (int*)malloc(phase_count * sizeof(int));
        invalid_tracer_count = (int*)malloc(phase_count * sizeof(int));

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            valid_tracer_count  [i_phase] = 0;
            invalid_tracer_count[i_phase] = 0;
        }

        loop(p, I->p_init){

            PP_USER_REAL(p, p_just_started) = 1.0;
            PP_USER_REAL(p, p_start_time)   = CURRENT_TIME - CURRENT_TIMESTEP;
            PP_USER_REAL(p, p_invalid)      = 0.0;

            t = P_CELL_THREAD(p);
            i_fluent_cell = P_CELL(p);

            if(C_PART(i_fluent_cell, t) != myid){

                rcfd_report0(myrcfd, RCFD_REPORT_WARNING,
                     "rcfd_init_tracers: exterior start cell");
            }

            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

            PP_USER_REAL(p, p_c0) = (double)i_cell;
            PP_USER_REAL(p, p_node0) = (double)myid;

            /* set phase_id, phase_vof, in case vof==0 set tracer invalid */
            {
                i_phase = (initialized_tracer_count % phase_count);

                PP_USER_REAL(p, p_phase_id) = (double)i_phase;
                PP_RHO(p) = rcfd_get_phase_density(myrcfd, i_phase);

                if(rcfd_is_vof_on(myrcfd)){

                    c = P_CELL(p);
                    t = P_CELL_THREAD(p);

                    i_phase_fluent = fluent_get_phase_id(rcfd, i_phase);
                    t_phase = THREAD_SUB_THREAD(t, i_phase_fluent);

                    if(C_VOF(c, t_phase) > rcfd_get_vof_cutoff_limit(myrcfd)){

                        PP_USER_REAL(p, p_phase_fraction) = C_VOF(c, t_phase);
                    }
                    else{

                        PP_USER_REAL(p, p_phase_fraction) = 0.0;
                        PP_USER_REAL(p, p_invalid) = 1.0;
                    }
                }
                else{

                    PP_USER_REAL(p, p_phase_fraction) = 1.0;
                }
            }

            /* in some cases (e.g. coarse graining, full database, monitoring hasn't started yet, ...),
               set tracers invalid */
            {
                rand_real = (double)rand()/(double)RAND_MAX;

                if( ((n_time % rcfd_get_monitoring_interval(myrcfd)) != 0) ||               // not tracer start interval
                    (CURRENT_TIME < rcfd->Solver_Dict->start_time_for_monitoring) ||        // too early to start tracers
                    (rcfd_get_current_frame(myrcfd) >= rcfd_get_frame_count(myrcfd)) ||     // shifts database full
                    (rand_real > (1./(double)rcfd->Prep_Dict->tracer_coarse_graining))){    // kill tracer due to coarse graining

                    PP_USER_REAL(p, p_invalid) = 1.0;
                    invalid_tracer_count[i_phase]++;
                }
                else{

                    valid_tracer_count[i_phase]++;
                }
            }

            initialized_tracer_count++;
        }
    }

    /* 2. Transcript */
    {
        rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                     "rCFD_init_tracers at timestep %ld", N_TIME);

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            valid_tracer_count  [i_phase] = PRF_GISUM1(valid_tracer_count  [i_phase]);
            invalid_tracer_count[i_phase] = PRF_GISUM1(invalid_tracer_count[i_phase]);

            rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                         "rCFD_init_tracers: i_phase = %d: initialized %d valid and %d invalid tracers",
                         i_phase, valid_tracer_count[i_phase], invalid_tracer_count[i_phase]);
        }

        if(CURRENT_TIMESTEP > rcfd_get_global_timestep(myrcfd)){

            rcfd_report0(myrcfd, RCFD_REPORT_WARNING,
                         "rCFD_init_tracers: CURRENT_TIMESTEP (%e) > global_time_step (%e)",
                          CURRENT_TIMESTEP, rcfd_get_global_timestep(myrcfd));
        }
    }

    /* 3. free local vars */
    {
        free(valid_tracer_count);
        free(invalid_tracer_count);
    }

#endif // RP_NODE
}

/*************************************************************************************/
DEFINE_DPM_SCALAR_UPDATE(rCFD_update_Tracers,i_fluent_cell,t,initialize,p)
/*************************************************************************************/
{
#if RP_NODE

    // rCFD tracers
    if(strcmp(p->injection->name, "rcfd_tracer") == 0){

#if 1  /*local vars */
        int     i_cell, i_phase, i_phase_fluent, i_shift, i_layer = 0;
        double  rand_real = 0., time_ratio, random_walk_velocity, v_mag = -1.0;

        Thread  *t_phase = NULL;
        rCFD_type *rcfd = (rCFD_type*)myrcfd;
#endif

        /* 1. Remove invalid tracers & exit */
        if(p->user[p_invalid] > 0.0){ // check invalid tracer flag

            MARK_TP(p, P_FL_REMOVED);
            return;
        }

        /* 2. Multiphase settings, Fluent to rCFD cell id */
        {
            if(!rcfd_is_vof_on(myrcfd)){

                i_phase = 0;
                t_phase = t;
            }
            else{

                i_phase = (int)p->user[p_phase_id];
                i_phase_fluent = fluent_get_phase_id(rcfd, i_phase);
                t_phase = THREAD_SUB_THREAD(t, i_phase_fluent);
            }

            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));
        }

        /* 3. Initialize Tracers */
        if(p->user[p_just_started] > 0.0){ // check tracer just started flag

            rcfd->Solver->tracer_monitoring_started = 1;

            // standard initialization
            {
                p->user[p_w0]       = C_VOLUME(i_fluent_cell,t) * p->user[p_phase_fraction];
                p->user[p_c_old]    = (double)i_cell;

                p->state.V[0] = C_U(i_fluent_cell,t_phase);
                p->state.V[1] = C_V(i_fluent_cell,t_phase);
                p->state.V[2] = C_W(i_fluent_cell,t_phase);

                p->user[p_time_ratio] = 1.0;
                p->user[p_just_started] = 0.;
            }

            /* tracer_splitting in slow cells */
            if(rcfd->Prep_Dict->tracer_splitting_on){

                double *cell_crossing_time = rcfd_get_cell_crossing_time(myrcfd,i_layer,i_phase);

                v_mag = sqrt( C_U(i_fluent_cell, t_phase) * C_U(i_fluent_cell, t_phase) +
                              C_V(i_fluent_cell, t_phase) * C_V(i_fluent_cell, t_phase) +
                              C_W(i_fluent_cell, t_phase) * C_W(i_fluent_cell, t_phase) );

                if(v_mag < _C(i_layer).grid_spacing[i_cell] / (100. * rcfd_get_global_timestep(myrcfd))){

                    cell_crossing_time[i_cell] = 100. * rcfd_get_global_timestep(myrcfd);
                }
                else{

                    cell_crossing_time[i_cell] = _C(i_layer).grid_spacing[i_cell] / v_mag;
                }

                if(EXCESS_TRACER_CELL_CROSSING_TIME){

                    time_ratio = cell_crossing_time[i_cell] / (2. * rcfd->Phase_Dict[i_phase].time_step);  /* (time_ratio > 1.) */

                    p->user[p_time_ratio] = time_ratio;

                    /* store c0_c0_tracer */
                    if((TRACER_DATABASE_NOT_FULL) && (p->user[p_w0] > 0.0)){

                        i_shift = rcfd->C2C_Prep[i_phase].valid_shifts_count;

                        if(i_shift < (rcfd->C2C_Prep[i_phase].shifts_count - 1)){

                            rcfd->C2C_Prep[i_phase].c0   [i_shift] =  i_cell;
                            rcfd->C2C_Prep[i_phase].node0[i_shift] =  myid;
                            rcfd->C2C_Prep[i_phase].c1   [i_shift] =  i_cell;
                            rcfd->C2C_Prep[i_phase].node1[i_shift] =  myid;
                            rcfd->C2C_Prep[i_phase].w0   [i_shift] =  (1.-1./time_ratio) * p->user[p_w0];

                            rcfd->C2C_Prep[i_phase].valid_shifts_count++;
                        }
                    }

                    /* init c0_c1_tracer */
                    {
                        p->user[p_w0] = (1./time_ratio) * p->user[p_w0];

                        p->state.V[0] = time_ratio * C_U(i_fluent_cell,t_phase);
                        p->state.V[1] = time_ratio * C_V(i_fluent_cell,t_phase);
                        p->state.V[2] = time_ratio * C_W(i_fluent_cell,t_phase);
                    }
                }
            }

            // random walk initialization
            {
                if(rcfd->Prep_Dict->tracer_random_walk_on[i_phase]){

                    if(v_mag < 0.0){

                        v_mag = sqrt( C_U(i_fluent_cell, t_phase) * C_U(i_fluent_cell, t_phase) +
                                      C_V(i_fluent_cell, t_phase) * C_V(i_fluent_cell, t_phase) +
                                      C_W(i_fluent_cell, t_phase) * C_W(i_fluent_cell, t_phase) );
                    }

                    random_walk_velocity = rcfd->Prep_Dict->tracer_random_walk_velocity_ratio[i_phase] * v_mag;

                    rand_real =         2.*((double)rand()/(double)RAND_MAX-0.5);
                    p->user[p_u_rwm] =  rand_real * random_walk_velocity;

                    rand_real =         2.*((double)rand()/(double)RAND_MAX-0.5);
                    p->user[p_v_rwm] =  rand_real * random_walk_velocity;

                    rand_real =         2.*((double)rand()/(double)RAND_MAX-0.5);
                    p->user[p_w_rwm] =  rand_real * random_walk_velocity;

                    p->state.V[0] += p->user[p_u_rwm];
                    p->state.V[1] += p->user[p_v_rwm];
                    p->state.V[2] += p->user[p_w_rwm];

                    p->user[p_vel_rwm_old] = random_walk_velocity;
                }
                else{

                    p->user[p_u_rwm] = 0.0;
                    p->user[p_v_rwm] = 0.0;
                    p->user[p_w_rwm] = 0.0;
                }
            }

            if(rcfd->Prep_Dict->tracer_reverse_direction_on){

                p->user[p_time_ratio] *= -1.0;
            }
        }

        /* 4. Consider (i) RWM changes across cells  (ii) fast Tracer from slow cells */
        if(TRACER_HAS_CROSSED_CELL_BORDER){

            if(rcfd->Prep_Dict->tracer_random_walk_on[i_phase]){

                v_mag = sqrt( C_U(i_fluent_cell, t_phase) * C_U(i_fluent_cell, t_phase) +
                              C_V(i_fluent_cell, t_phase) * C_V(i_fluent_cell, t_phase) +
                              C_W(i_fluent_cell, t_phase) * C_W(i_fluent_cell, t_phase) );

                random_walk_velocity = rcfd->Prep_Dict->tracer_random_walk_velocity_ratio[i_phase] * v_mag;

                // this keeps the direction of fluctuation, just scales its magnitude,
                // most probably, this results in large diffusion
                p->user[p_u_rwm] *= random_walk_velocity / p->user[p_vel_rwm_old];
                p->user[p_v_rwm] *= random_walk_velocity / p->user[p_vel_rwm_old];
                p->user[p_w_rwm] *= random_walk_velocity / p->user[p_vel_rwm_old];

                p->user[p_vel_rwm_old] = random_walk_velocity;
            }

            if((rcfd->Prep_Dict->tracer_splitting_on) && (TRACER_FROM_SLOW_CELL) && (TRACER_DATABASE_NOT_FULL) && (TRACER_NOT_STORED_YET)){

                i_shift = rcfd->C2C_Prep[i_phase].valid_shifts_count;

                if((i_shift < (rcfd->C2C_Prep[i_phase].shifts_count - 1)) && (p->user[p_w0] > 0.0)){

                    rcfd->C2C_Prep[i_phase].c0   [i_shift] =  (int)p->user[p_c0];
                    rcfd->C2C_Prep[i_phase].node0[i_shift] =  (int)p->user[p_node0];
                    rcfd->C2C_Prep[i_phase].c1   [i_shift] =  i_cell;
                    rcfd->C2C_Prep[i_phase].node1[i_shift] =  myid;
                    rcfd->C2C_Prep[i_phase].w0   [i_shift] =  p->user[p_w0];

                    rcfd->C2C_Prep[i_phase].valid_shifts_count++;
                }

                p->user[p_invalid] = 1.;
                p->state.mass = 0.;
                p->type = 2;

                MARK_TP(p, P_FL_REMOVED);
            }

            p->user[p_c_old] = (double)i_cell;
        }

        /* 5. store and kill tracers after lifetime */
        if((TRACER_LIFETIME_EXPIRED) && (TRACER_DATABASE_NOT_FULL)){

            i_shift = rcfd->C2C_Prep[i_phase].valid_shifts_count;

            if((i_shift < (rcfd->C2C_Prep[i_phase].shifts_count - 1)) && (p->user[p_w0] > 0.0)){

                rcfd->C2C_Prep[i_phase].c0   [i_shift] =  (int)p->user[p_c0];
                rcfd->C2C_Prep[i_phase].node0[i_shift] =  (int)p->user[p_node0];
                rcfd->C2C_Prep[i_phase].c1   [i_shift] =  i_cell;
                rcfd->C2C_Prep[i_phase].node1[i_shift] =  C_PART(i_fluent_cell, t);
                rcfd->C2C_Prep[i_phase].w0   [i_shift] =  p->user[p_w0];

                rcfd->C2C_Prep[i_phase].valid_shifts_count++;
                rcfd->Solver->shifts_ready2write = 1;
            }

            p->user[p_invalid] = 2.;
            p->state.mass = 0.;
            p->type = 2;

            MARK_TP(p, P_FL_REMOVED);
        }
    }
    // case-specific user input
    else{

        case_specific_scalar_update(i_fluent_cell,t,initialize,p);
    }

    return;

#endif // RP_NODE
}

/*************************************************************************************/
DEFINE_DPM_DRAG(rcfd_no_standard_drag,p,i)
/*************************************************************************************/
{
    return 0.;
}

/*************************************************************************************/
DEFINE_DPM_BODY_FORCE(rCFD_guide_Tracers,p,i)
/*************************************************************************************/
{
#if RP_NODE

    // rCFD tracers
    if(strcmp(p->injection->name, "rcfd_tracer") == 0){

        int phase_count = rcfd_get_phase_count(myrcfd);
        rCFD_type *rcfd = (rCFD_type*)myrcfd;

        /* in case particle is declared dead: V[i], f[i] = 0 */
        if(p->user[p_invalid] > 0){

            p->state.V[i] = 0.0;
            return 0.0;
        }

        switch (rcfd->Prep_Dict->tracer_format){

            case RCFD_TRACER_FORCE_GUIDED:
            {
                Thread* t = P_CELL_THREAD(p);
                cell_t  c = P_CELL(p);

                if(phase_count == 1){

                    if(i==0)    return p->user[p_time_ratio] * (C_U(c,t) + p->user[p_u_rwm]) - p->state.V[0];
                    if(i==1)    return p->user[p_time_ratio] * (C_V(c,t) + p->user[p_v_rwm]) - p->state.V[1];
                    if(i==2)    return p->user[p_time_ratio] * (C_W(c,t) + p->user[p_w_rwm]) - p->state.V[2];
                }
                else{
                    Thread  *t_phase = NULL;
                    int i_phase = (int)p->user[p_phase_id];
                    t_phase = THREAD_SUB_THREAD(t, i_phase);

                    if(i==0)    return p->user[p_time_ratio] * (C_U(c,t_phase) + p->user[p_u_rwm]) - p->state.V[0];
                    if(i==1)    return p->user[p_time_ratio] * (C_V(c,t_phase) + p->user[p_v_rwm]) - p->state.V[1];
                    if(i==2)    return p->user[p_time_ratio] * (C_W(c,t_phase) + p->user[p_w_rwm]) - p->state.V[2];
                }

                break;
            }
            case RCFD_TRACER_VELOCITY_GUIDED:
            {
                Thread* t = P_CELL_THREAD(p);
                cell_t  c = P_CELL(p);

                if(phase_count == 1){

                    if(i==0)    p->state.V[i] = p->user[p_time_ratio] * (C_U(c,t) + p->user[p_u_rwm]);
                    if(i==1)    p->state.V[i] = p->user[p_time_ratio] * (C_V(c,t) + p->user[p_v_rwm]);
                    if(i==2)    p->state.V[i] = p->user[p_time_ratio] * (C_W(c,t) + p->user[p_w_rwm]);
                }
                else{

                    Thread  *t_phase = NULL;
                    int i_phase = (int)p->user[p_phase_id];
                    t_phase = THREAD_SUB_THREAD(t, i_phase);

                    if(i==0)    p->state.V[i] = p->user[p_time_ratio] * (C_U(c,t_phase) + p->user[p_u_rwm]);
                    if(i==1)    p->state.V[i] = p->user[p_time_ratio] * (C_V(c,t_phase) + p->user[p_v_rwm]);
                    if(i==2)    p->state.V[i] = p->user[p_time_ratio] * (C_W(c,t_phase) + p->user[p_w_rwm]);
                }

                break;
            }
        }
    }
    // case-specific user input
    else{

        return case_specific_body_forces(p, i);
    }

#endif // RP_NODE

    return 0.0;
}

/**
 * @brief Set (i.e. transfer) the current CFD displacement fields to the rCFD data structure
 * @param rcfd  pointer to previously created rCFD instance
 */
void rCFD_set_displacement_fields(rCFD_type *rcfd)
{
#if RP_NODE
    // read cfd field and store in _C(i_layer).cfd_displ[i_phase][i_cell][i_dim]
    //                      and in _C(i_layer).cfd_displ_diff [i_phase][i_cell][i_dim]
    if((rcfd->Solver->displ_reset_counter > 0)){

#if 1   // local vars
        int     i_dim, i_cell, i_fluent_cell, i_phase, i_phase_fluent, udsi_per_phase;
        int     uds_offset  = g_uds_offset;
        int     i_layer = 0;   // only base-layer allowed in prep mode
        int     i_frame =  0;   // only current frame considered in prep mode
        int     cell_count  = rcfd_get_cell_count(myrcfd, i_layer);
        int     phase_count = rcfd_get_phase_count(myrcfd);
        int     local_diff_on = rcfd_is_local_diffusion_on(myrcfd);
        int     local_diff_format = rcfd_get_local_diffusion_format(myrcfd);
        double  **cell_vof  = rcfd_get_cell_vof(myrcfd, i_layer, i_frame);
        double  *cfd_displ = malloc(sizeof(double) * cell_count * 3);
        double  *cfd_displ_diff = NULL;

        Domain  *d = Get_Domain(1);
        Thread  *t = NULL, *t_phase = NULL;
#endif

        if((local_diff_on) &&
           (local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED)){

            udsi_per_phase = 9;
            cfd_displ_diff = malloc(sizeof(double) * cell_count * 6);
        }
        else{

            udsi_per_phase = 3;
        }

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            thread_loop_c(t,d){

                // set Fluent phase pointer (t_phase)
                {
                    t_phase = t;    // default for single-phase

                    if(rcfd_is_vof_on(myrcfd)){

                        i_phase_fluent = fluent_get_phase_id(rcfd, i_phase);
                        t_phase = THREAD_SUB_THREAD(t, i_phase_fluent);
                    }
                }

                begin_c_loop_int(i_fluent_cell, t){

                    i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

                    if(i_cell > -1){

                        // valid vof cell
                        if(cell_vof[i_cell][i_phase] > rcfd_get_vof_cutoff_limit(myrcfd)){

                            for(i_dim = 0; i_dim < 3; i_dim++){

                                cfd_displ[i_cell*3 + i_dim] = C_UDSI(i_fluent_cell, t_phase,
                                                                           ((udsi_per_phase * i_phase) + i_dim + uds_offset));

                                if(rcfd_is_local_diffusion_on(myrcfd) && (local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED)){

                                    cfd_displ_diff[i_cell*6 + i_dim] =
                                            C_UDSI(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 3));
                                    cfd_displ_diff[i_cell*6 + i_dim + 3] =
                                            C_UDSI(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 6));
                                }
    #if DEBUG_INTERPOLATION
                                C_UDMI(i_fluent_cell, t, (u_cal_x + i_dim)) = cfd_displ[i_cell*3 + i_dim];

                                if(rcfd_is_local_diffusion_on(myrcfd) && (local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED)){

                                    C_UDMI(i_fluent_cell, t, (u_cal_dx + i_dim)) = cfd_displ_diff[i_cell*6 + i_dim];
                                }
    #endif // DEBUG_INTERPOLATION
                            }
                        }
                        // invalid vof cell
                        else{

                            for(i_dim = 0; i_dim < 3; i_dim++){

                                cfd_displ[i_cell*3 + i_dim] = 0.0;

                                if(rcfd_is_local_diffusion_on(myrcfd) && (local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED)){

                                    cfd_displ_diff[i_cell*6 + i_dim] = 0.0;
                                    cfd_displ_diff[i_cell*6 + i_dim + 3] = 0.0;
                                }
    #if DEBUG_INTERPOLATION
                                C_UDMI(i_fluent_cell, t, (u_cal_x + i_dim)) = 0.0;
                                C_UDMI(i_fluent_cell, t, (u_cal_dx + i_dim)) = 0.0;
    #endif // DEBUG_INTERPOLATION
                            }
                        }

                    }

                }end_c_loop_int(i_fluent_cell, t)

            }

            rcfd_set_displacement_fields(myrcfd, i_layer, i_phase, cfd_displ, cfd_displ_diff);

            rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                         "rCFD_set_displacement_fields: set displacement field for i_phase %d", i_phase);
        }

        free(cfd_displ);
        free(cfd_displ_diff);
    }
#endif // RP_NODE
}

/**
 * @brief Reset the CFD displacement fields to initial values
 * @param rcfd  pointer to previously created rCFD instance
 */
void rCFD_reset_displacement_fields(rCFD_type *rcfd)
{
#if RP_NODE

#if 1   // local vars
    int     i_dim, i_cell, i_fluent_cell, i_phase, i_phase_fluent, udsi_per_phase;
    int     i_layer = 0;
    int     uds_offset = g_uds_offset;
    int     phase_count = rcfd_get_phase_count(myrcfd);
    int     local_diff_on = rcfd_is_local_diffusion_on(myrcfd);
    int     local_diff_format = rcfd_get_local_diffusion_format(myrcfd);
    double  x[3];
    double  **cell_x = rcfd_get_cell_center(myrcfd, i_layer);

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL, *t_phase = NULL;
#endif

    // in case of moving geom update rCFD cell coords
    if(rcfd_is_moving_geometry_on(myrcfd)){

        thread_loop_c(t,d){begin_c_loop(i_fluent_cell, t){

            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

            if(i_cell > -1){

                C_CENTROID(x, i_fluent_cell, t);

                for(i_dim = 0; i_dim < 3; i_dim++){

                    cell_x[i_cell][i_dim] = x[i_dim];
                }
            }
        }end_c_loop(i_fluent_cell, t)}
    }

    if((local_diff_on) &&
       (local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED)){

        udsi_per_phase = 9;
    }
    else{

        udsi_per_phase = 3;
    }

    // reset displacement patterns
    for(i_phase = 0; i_phase < phase_count; i_phase++){

        thread_loop_c(t,d){

            // set Fluent phase pointer (t_phase)
            {
                t_phase = t;

                if(rcfd_is_vof_on(myrcfd)){

                    i_phase_fluent = fluent_get_phase_id(rcfd, i_phase);
                    t_phase = THREAD_SUB_THREAD(t, i_phase_fluent);
                }
            }

            begin_c_loop(i_fluent_cell, t){

                C_CENTROID(x, i_fluent_cell, t);

                for(i_dim = 0; i_dim < 3; i_dim++){

                    // current time step
                    C_UDSI(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset    )) = x[i_dim];

                    // previous time step
                    if(THREAD_STORAGE(t_phase,SV_UDSI_M1(i_dim)) != NULL){

                        C_UDSI_M1(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset    )) = x[i_dim];
                    }

                    // second previous time step
                    if(THREAD_STORAGE(t_phase,SV_UDSI_M2(i_dim)) != NULL){

                        C_UDSI_M2(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset    )) = x[i_dim];
                    }

                    if((local_diff_on) &&
                       (local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED)){

                        // current time step
                        C_UDSI(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 3)) = x[i_dim];
                        C_UDSI(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 6)) = x[i_dim] * x[i_dim];

                        // previous time step
                        if(THREAD_STORAGE(t_phase,SV_UDSI_M1(i_dim)) != NULL){

                            C_UDSI_M1(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 3)) = x[i_dim];
                            C_UDSI_M1(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 6)) = x[i_dim] * x[i_dim];
                        }

                        // second previous time step
                        if(THREAD_STORAGE(t_phase,SV_UDSI_M2(i_dim)) != NULL){

                            C_UDSI_M2(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 3)) = x[i_dim];
                            C_UDSI_M2(i_fluent_cell, t_phase, ((udsi_per_phase * i_phase) + i_dim + uds_offset + 6)) = x[i_dim] * x[i_dim];
                        }
                    }
                }
            }end_c_loop(i_fluent_cell, t)
        }

        rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                     "rCFD_reset_displacement_fields: reset displacement patterns for i_phase %d, reset_counter %d",
                     i_phase, rcfd->Solver->displ_reset_counter);
    }

    rcfd->Solver->displ_reset_counter++;

#endif // RP_NODE
}

/**
 * @brief Set (i.e. transfer) the CFD displacement fields to the rCFD data structure;
 *          then reset the fields to initial values
 * @param rcfd  pointer to previously created rCFD instance
 */
void rCFD_update_displacement_fields(rCFD_type *rcfd)
{
#if RP_NODE
    rCFD_set_displacement_fields(rcfd);

    rCFD_reset_displacement_fields(rcfd);
#endif // RP_NODE
}

/**
 * @brief darns grid interfaces by connecting interfaces cells by cell-to-cell shifts
 * @param rcfd  pointer to previously created rCFD instance
 */
void rCFD_create_shifts_from_grid_interfaces()
{
#if RP_NODE
#define DEBUG_LOCAL 0

    int interface_face_count = fluent_get_interface_face_count();
    int global_interface_face_count = PRF_GISUM1(interface_face_count);

    if(global_interface_face_count == 0)
        return;

    double  *interface_face_centers = NULL;

    if(interface_face_count > 0){

        interface_face_centers = malloc(3 * interface_face_count * sizeof(double));
    }

    fluent_get_interface_face_centers(interface_face_centers, interface_face_count);

    if(DEBUG_LOCAL){

        Message("\n[%d] rCFD_create_shifts_from_grid_interfaces: interface face node/global %d/%d",
            myid, interface_face_count, global_interface_face_count);
    }

    rcfd_create_shifts_from_interfaces(myrcfd,
                                       interface_face_centers,
                                       interface_face_count);

    free(interface_face_centers);

#undef DEBUG_LOCAL
#endif
}

/**
 * @brief Define the boundary condition as a fixed gradient by using the boundary
 *          cell C0 for each boundary face.
 */
/*************************************************************************************/
DEFINE_PROFILE(rCFD_wall_bc, t, i_uds)
/*************************************************************************************/
{
#if RP_NODE
    int     i_face = 0, i_off, udsi_per_phase;
    int     local_diff_format = rcfd_get_local_diffusion_format(myrcfd);
    int     phase_count = rcfd_get_phase_count(myrcfd);
    int     n_time = N_TIME - g_start_timestep;

    // position-vectors for face- and cell-center
    double x_f[3], x_c[3];
    double val;

    // The UDS in Fluent for rCFD do not necessarily need to start at ID 0
    i_off = i_uds - g_uds_offset;

    if(local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED){

        udsi_per_phase = 9;
    }
    else{

        udsi_per_phase = 3;
    }

    // send error and abort udf if the boundary profile of the wrong UDS gets altered.
    if(i_off < 0 || i_off >= udsi_per_phase * phase_count){

        rcfd_report0(myrcfd, RCFD_REPORT_ERROR,
                     "rCFD_wall_bc: boundary condition set for wrong UDS");
        return;
    }
    i_off %= udsi_per_phase;

    if(THREAD_STORAGE(F_C0_THREAD(i_face, t), SV_UDS_I(i_uds)) != NULL
        && ((n_time % rcfd_get_monitoring_interval(myrcfd)) != 0)){

        // calculate the boundary value for the fixed gradient BC and the neighboring cell center
        begin_f_loop_int(i_face, t){

            // get centroid positions
            C_CENTROID(x_c, F_C0(i_face, t), F_C0_THREAD(i_face, t));
            F_CENTROID(x_f, i_face, t);

            val = C_UDSI(F_C0(i_face, t), F_C0_THREAD(i_face, t), i_uds);

            switch(i_off){

                case 0: F_PROFILE(i_face, t, i_uds) = val + (x_f[0] - x_c[0]); break;  // x -> x0 + dx
                case 1: F_PROFILE(i_face, t, i_uds) = val + (x_f[1] - x_c[1]); break;  // y -> y0 + dy
                case 2: F_PROFILE(i_face, t, i_uds) = val + (x_f[2] - x_c[2]); break;  // z -> z0 + dz
                case 3: F_PROFILE(i_face, t, i_uds) = val + (x_f[0] - x_c[0]); break;  // x -> x0 + dx
                case 4: F_PROFILE(i_face, t, i_uds) = val + (x_f[1] - x_c[1]); break;  // y -> y0 + dy
                case 5: F_PROFILE(i_face, t, i_uds) = val + (x_f[2] - x_c[2]); break;  // z -> z0 + dz
                case 6: F_PROFILE(i_face, t, i_uds) = val + x_c[0] * (x_f[0] - x_c[0]) * 2; break;  // xx -> 2*x*dx
                case 7: F_PROFILE(i_face, t, i_uds) = val + x_c[1] * (x_f[1] - x_c[1]) * 2; break;  // yy -> 2*y*dy
                case 8: F_PROFILE(i_face, t, i_uds) = val + x_c[2] * (x_f[2] - x_c[2]) * 2; break;  // zz -> 2*z*dz
            }
        }end_f_loop_int(i_face, t)
    }
    else{

        // If the UDSI storage isn't allocated yet, or if it's then init the profile with standard values.
        begin_f_loop_int(i_face, t){

            F_CENTROID(x_f, i_face, t);

            switch(i_off){

                case 0: F_PROFILE(i_face, t, i_uds) = x_f[0]; break;  // x
                case 1: F_PROFILE(i_face, t, i_uds) = x_f[1]; break;  // y
                case 2: F_PROFILE(i_face, t, i_uds) = x_f[2]; break;  // z
                case 3: F_PROFILE(i_face, t, i_uds) = x_f[0]; break;  // x
                case 4: F_PROFILE(i_face, t, i_uds) = x_f[1]; break;  // y
                case 5: F_PROFILE(i_face, t, i_uds) = x_f[2]; break;  // z
                case 6: F_PROFILE(i_face, t, i_uds) = x_f[0] * x_f[0]; break;  // xx
                case 7: F_PROFILE(i_face, t, i_uds) = x_f[1] * x_f[1]; break;  // yy
                case 8: F_PROFILE(i_face, t, i_uds) = x_f[2] * x_f[2]; break;  // zz
            }
        }end_f_loop_int(i_face, t)
    }

#endif // RP_NODE
}

/*************************************************************************************/
DEFINE_PROFILE(rCFD_inlet_bc, t, i_uds)
/*************************************************************************************/
{
#if RP_NODE
    int     i_face, i_off, udsi_per_phase;
    int     local_diff_format = rcfd_get_local_diffusion_format(myrcfd);
    int     phase_count = rcfd_get_phase_count(myrcfd);
    double  x[3];

    i_off = i_uds - g_uds_offset;

    if(local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED){

        udsi_per_phase = 9;
    }
    else{

        udsi_per_phase = 3;
    }

    // send error and abort udf if the boundary profile of the wrong UDS gets altered.
    if(i_off < 0 || i_off >= udsi_per_phase * phase_count){

        rcfd_report0(myrcfd, RCFD_REPORT_ERROR,
                     "rCFD_inlet_bc: boundary condition set for wrong UDS");
        return;
    }

    // map from phase-specific dim index to basic dim index (all phases get the same values)
    i_off %= udsi_per_phase;

    begin_f_loop_int(i_face, t){

        F_CENTROID(x, i_face, t);

        if(i_off < 6){

            // Profiles depending on i_off = {0,1,2,3,4,5}: {x, y, z, x, y, z}
            F_PROFILE(i_face, t, i_uds) = x[i_off % 3];
        }
        else if(i_off < 9){

            // Profiles depending on i_off = {6,7,8}: {x*x, y*y, z*z}
            F_PROFILE(i_face, t, i_uds) = x[i_off % 3] * x[i_off % 3];
        }
    }end_f_loop_int(i_face, t)

#endif // RP_NODE
}

/**
 * @brief Write the cell-to-cell shifts of the current monitoring interval to the database
 * @param rcfd  pointer to previously created rCFD instance
 */
void rCFD_write_C2Cs(rCFD_type *rcfd)
{
#if RP_NODE

    int write_C2C_now = (rcfd_get_current_frame(myrcfd) < rcfd_get_frame_count(myrcfd));

    // decide whether to write C2C shifts at this function call
    {
        // tracer based generation of cell-to-cell shifts (old version)
        if(rcfd_is_shifts_from_displacement_fields_on(myrcfd) == 0){

            int shifts_ready2write = PRF_GIHIGH1(rcfd->Solver->shifts_ready2write);
            write_C2C_now = write_C2C_now && shifts_ready2write;
        }
        // displacement field based generation of cell-to-cell shifts
        else{

            write_C2C_now = write_C2C_now && (rcfd->Solver->displ_reset_counter > 1);
        }
    }

    if(write_C2C_now){

        rcfd_write_shifts(myrcfd);
        rcfd->Solver->shifts_ready2write = 0;
    }
    else{

       rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                    "rCFD_write_C2Cs: no shifts written; already wrote %d frames",
                    rcfd_get_current_frame(myrcfd));
    }
#endif // RP_NODE
}

/**
 * @brief Calculate the recurrence norms for the current monitoring interval
 * @param rcfd  pointer to previously created rCFD instance
 * @return pointer to an allocated array containing the updated norm values
 */
double *rCFD_update_norms(rCFD_type* rcfd)
{
#if RP_NODE
    if(rcfd_get_norm_format(myrcfd) == RCFD_NORM_DEFAULT){

        int     i_norm = 0;
        int     i_fluent_cell;
        int     phase_count, norm_count;
        int     norm_coarsening;
        double  *norms;

        Domain  *d = Get_Domain(1);
        Thread  *t = NULL, *t_phase = NULL;

        phase_count     = rcfd_get_phase_count(myrcfd);
        norm_count      = rcfd_get_norm_count(myrcfd);
        norm_coarsening = rcfd_get_norm_coarsening(myrcfd);

        norms = malloc(sizeof(double) * norm_count);

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){begin_c_loop_int(i_fluent_cell,t){

            if((i_fluent_cell % norm_coarsening) == 0){

                if(phase_count == 1){

                    norms[i_norm] = sqrt(   C_U(i_fluent_cell,t) * C_U(i_fluent_cell,t)
                                          + C_V(i_fluent_cell,t) * C_V(i_fluent_cell,t)
                                          + C_W(i_fluent_cell,t) * C_W(i_fluent_cell,t) );
                }
                else{

                    t_phase = THREAD_SUB_THREAD(t, 0);

                    if((t_phase != NULL) &&
                       (THREAD_STORAGE( t_phase, SV_VOF) != NULL)) {    // needed for sliding mesh

                        norms[i_norm] = C_VOF(i_fluent_cell, t_phase);
                    }
                }

                i_norm++;
            }

        }end_c_loop_int(i_fluent_cell,t)}}

        return norms;
    }
#endif // RP_NODE

    return NULL;
}

/**
 * @brief Set the CFD-solver_based local diffusion field for the current monitoring interval
 * @param rcfd  pointer to previously created rCFD instance
 * @return pointer to an allocated 2D array ([i_phase][i_cell]) containing the updated local diffusion values for each phase
 *     or NULL if not set by CFD-solver
 */
double **rCFD_update_local_diffusion(rCFD_type* rcfd)
{
#if RP_NODE
    if(rcfd_is_local_diffusion_on(myrcfd)){

        int     i_phase, i_cell, i_fluent_cell;
        int     i_layer = 0;
        int     int_cell_count  = rcfd_get_int_cell_count(myrcfd, i_layer);
        int     phase_count = rcfd_get_phase_count(myrcfd);
        int     local_diff_format = rcfd_get_local_diffusion_format(myrcfd);
        double  **diffusion = NULL;

        Domain  *d = Get_Domain(1);
        Thread  *t = NULL, *t_phase = NULL;

        // transfer fluent C_MU_EFF to temporal _C_local_diff field (sub-grid momentum diffusion)
        if(local_diff_format == RCFD_LOCAL_DIFF_VISCOSITY_BASED){

            diffusion = malloc(phase_count * sizeof(double*));

            for(i_phase = 0; i_phase < phase_count; i_phase++){

                diffusion[i_phase] = malloc(int_cell_count * sizeof(double*));

                thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){

                    t_phase = (phase_count > 1) ? THREAD_SUB_THREAD(t, i_phase) : t;

                    if((t_phase != NULL) &&
                       (THREAD_STORAGE( t_phase, SV_VOF) != NULL)) {    // needed for sliding mesh

                        begin_c_loop_int(i_fluent_cell, t){

                            i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

                            if(i_cell > -1){

                                diffusion[i_phase][i_cell] = C_MU_EFF(i_fluent_cell, t_phase) / C_R(i_fluent_cell, t_phase);
                            }

                        }end_c_loop_int(i_fluent_cell, t)
                    }
                }}
            }
        }
        else{ // RCFD_LOCAL_DIFF_FIELD_BASED

            diffusion = NULL; // diff set in rCFD_prep.h
        }

        return diffusion;
    }
#endif

    return NULL;
}

/**
 * @brief Extend the rCFD database with updated field data by one frame for the current monitoring interval
 * @param rcfd  pointer to previously created rCFD instance
 */
void rCFD_write_fields(rCFD_type* rcfd)
{
#if RP_NODE

    // decide whether to write fields at this function call
    int write_fields_now = (rcfd_get_current_frame(myrcfd) < rcfd_get_frame_count(myrcfd));

    {
        if(rcfd_is_shifts_from_displacement_fields_on(myrcfd) == 0){

             // fields should be in sync with shifts
            int shifts_ready2write = PRF_GIHIGH1(rcfd->Solver->shifts_ready2write);
            write_fields_now = write_fields_now && shifts_ready2write;
        }
        else{

            write_fields_now = write_fields_now && (rcfd->Solver->displ_reset_counter > 1);
        }
    }

    if(write_fields_now){

        int     i_cell, i_phase, i_layer = 0;
        int     cell_count = rcfd_get_cell_count(myrcfd, i_layer);
        int     phase_count = rcfd_get_phase_count(myrcfd);
        double  *norms = NULL;
        double  **vof = NULL;
        double  **diffusion = NULL;

        // update /set fields
        {
            /* calculate norms */
            norms = rCFD_update_norms(rcfd);

            /* set vof */
            vof = NULL;  // already updated by fluent_set_vof(rcfd);

            /* calculate local diffusion */
            diffusion = rCFD_update_local_diffusion(rcfd);
        }

        // transfer fields to library (if not set directly),
        // call user defined functions for setting fields
        rcfd_set_fields(myrcfd, norms, vof, diffusion);

        // free memory
        {
            free(norms);

            if(vof){
                for(i_cell = 0; i_cell < cell_count; ++i_cell){
                    free(vof[i_cell]);
                }
                free(vof);
            }

            if(diffusion){
                for(i_phase = 0; i_phase < phase_count; ++i_phase){
                    free(diffusion[i_phase]);
                }
                free(diffusion);
            }
        }

        // write fields to file
        rcfd_write_fields(myrcfd);
    }
    else{

        rcfd_report0(myrcfd, RCFD_REPORT_INFO,
                     "rCFD_write_fields: no field frame written; already wrote %d frames",
                     rcfd_get_current_frame(myrcfd));
    }
#endif // RP_NODE
}

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_write_Rec)
/*************************************************************************************/
{
    // typically at the end of preparation
    rcfd_write_recurrence(myrcfd);
}

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_write_Topo)
/*************************************************************************************/
{
#if RP_NODE
    rcfd_write_topo(myrcfd);
#endif // RP_NODE
}

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_free_all)
/*************************************************************************************/
{
    rcfd_close(myrcfd);
    fluent_free_topo();
}

/**
 * @brief calculate cell-to-cell shifts, update fields and write to database
 */
/*************************************************************************************/
DEFINE_EXECUTE_AT_END(rCFD_execute_at_end)
/*************************************************************************************/
{
#if RP_NODE
    int n_time = N_TIME - g_start_timestep;
    int current_frame = rcfd_get_current_frame(myrcfd);
    int frame_count =   rcfd_get_frame_count  (myrcfd);
    rCFD_type *rcfd = (rCFD_type*)myrcfd;

    if(((n_time % rcfd_get_monitoring_interval(myrcfd)) == 0) &&
       (current_frame < frame_count)){

        if(rcfd_is_vof_on(myrcfd)){

            fluent_set_vof(rcfd);
        }

        if(rcfd_is_shifts_from_displacement_fields_on(myrcfd)){

            rCFD_update_displacement_fields(rcfd);

            if(rcfd->Solver->displ_reset_counter > 1){

                rcfd_create_shifts_from_displacement_fields(myrcfd);

                rcfd_interpolate_shifts(myrcfd);

                rCFD_create_shifts_from_grid_interfaces();
            }
        }
        // else tracer based shifts ...

        rCFD_write_fields(rcfd);

        rCFD_write_C2Cs(rcfd);
    }
#endif
}
