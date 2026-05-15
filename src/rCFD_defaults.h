/******************************************************************************
 * @internal
 * @file        rCFD_defaults.h
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_DEFAULTS
#define RCFD_DEFAULTS

#include "rCFD.h"
#include "rCFD_macros.h"
#include "rCFD_types.h"
#include "rCFD_shift.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>


    void default_Report(rCFD_type* rcfd)
    {
        rcfd->Report->timing_on         = 1;
        rcfd->Report->file_output_on    = 1;
        rcfd->Report->console_output_on = 1;
        rcfd->Report->fflush_on         = 1;
        rcfd->Report->file_is_open      = 0;
        rcfd->Report->file0_is_open     = 0;
        rcfd->Report->priority          = RCFD_REPORT_INFO;
        rcfd->Report->report_file       = NULL;
#if !defined(RCFD_LIB_MPI)
        rcfd->Report->report_file0      = NULL;
#endif
        rcfd->Report->time_unit         = RCFD_TIMER_SEC;
        rcfd->Report->ticks_2_time_unit = 1.0/CLOCKS_PER_SEC;
        rcfd->Report->number_of_reports = malloc_i_2d(RCFD_REPORT_CATEGORY_MAX, RCFD_REPORT_MAX);
        memset(rcfd->Report->number_of_reports[0], 0, RCFD_REPORT_CATEGORY_MAX * RCFD_REPORT_MAX * sizeof(int));
        memset(rcfd->Report->timers, 0, RCFD_TIMER_MAX * sizeof(Timer_type*));
    }

    /* A. default Dicts */

    void default_Solver_Dict(rCFD_type* rcfd)
    {
        rcfd->Solver_Dict->report_priority =  RCFD_REPORT_WARNING;

        rcfd->Solver_Dict->profiling_on             = 1;
        rcfd->Solver_Dict->multifile_output_on      = 0;
        rcfd->Solver_Dict->write_config_to_file_on  = 0;

        rcfd->Solver_Dict->mode = RCFD_MODE_RUN;

        rcfd->Solver_Dict->number_of_frames =     1;
        rcfd->Solver_Dict->number_of_states =     1;
        rcfd->Solver_Dict->number_of_phases =     1;
        rcfd->Solver_Dict->number_of_layers =     1;
        rcfd->Solver_Dict->number_of_runs =       1;

        rcfd->Solver_Dict->recurrence_process_on =     1;
        rcfd->Solver_Dict->data_convection_on =        1;
        rcfd->Solver_Dict->face_swap_diffusion_on =    1;
        rcfd->Solver_Dict->local_diff_on =             0;
        rcfd->Solver_Dict->parallel_diffusion_on =     1;
        rcfd->Solver_Dict->data_binarization_on =      0;
        rcfd->Solver_Dict->data_drifting_on =          0;
        rcfd->Solver_Dict->balance_correction_on =     1;
        rcfd->Solver_Dict->interpolation_on =          0;
        rcfd->Solver_Dict->shifts_from_displacement_fields_on = 0;
        rcfd->Solver_Dict->moving_geometry_on =        0;
        rcfd->Solver_Dict->grid_interface_shifts_on =  1;

        rcfd->Solver_Dict->max_number_of_cells_per_time_step = 10;

        rcfd->Solver_Dict->time_steps_per_monitoring_interval =   10;
        rcfd->Solver_Dict->start_time_for_monitoring =            0.0;

        rcfd->Solver_Dict->single_phase_vof     = 0;
        rcfd->Solver_Dict->single_phase_vof_id  = 0;
        rcfd->Solver_Dict->vof_cutoff_limit     = 1.0e-4;

        rcfd->Solver_Dict->min_number_of_cells_per_layer =     100;
        rcfd->Solver_Dict->max_number_of_faces_per_cell =      100;
        rcfd->Solver_Dict->max_number_of_children =            100;
        rcfd->Solver_Dict->number_redist_loops_per_layer =     5;

        rcfd->Solver_Dict->global_time_step = 1.0;

        rcfd->Solver_Dict->max_fill_loops =   10;

        rcfd->Solver_Dict->face_swap_max_per_loop =      (1./8.);
        rcfd->Solver_Dict->face_swap_max_loops =         4;
        rcfd->Solver_Dict->local_diff_format =           RCFD_LOCAL_DIFF_VISCOSITY_BASED;
        rcfd->Solver_Dict->grid_interface_shift_weight = 0.5;

        rcfd->Solver_Dict->number_of_drift_loops =    1;

        rcfd->Solver_Dict->balance_correction_update = 1;
        rcfd->Solver_Dict->control_conc_sum_on = 1;

        rcfd->Solver_Dict->max_phase_data_count = 1;
    }

    void default_File_Dict(rCFD_type* rcfd)
    {
        rcfd->File_Dict->working_dir        = malloc(sizeof(char)*16);
        rcfd->File_Dict->data_dir           = malloc(sizeof(char)*16);
        rcfd->File_Dict->tmp_sub_dir        = malloc(sizeof(char)*16);
        rcfd->File_Dict->vof_sub_dir        = malloc(sizeof(char)*16);
        rcfd->File_Dict->topo_sub_dir       = malloc(sizeof(char)*16);
        rcfd->File_Dict->local_diff_sub_dir = malloc(sizeof(char)*16);
        rcfd->File_Dict->c2c_sub_dir        = malloc(sizeof(char)*16);
        rcfd->File_Dict->rec_sub_dir        = malloc(sizeof(char)*16);
        rcfd->File_Dict->user_sub_dir       = malloc(sizeof(char)*16);
        rcfd->File_Dict->post_dir           = malloc(sizeof(char)*16);

        strcpy(rcfd->File_Dict->working_dir,        ".");
        strcpy(rcfd->File_Dict->data_dir,           "data");
        strcpy(rcfd->File_Dict->tmp_sub_dir,        "tmp");
        strcpy(rcfd->File_Dict->vof_sub_dir,        "vof");
        strcpy(rcfd->File_Dict->topo_sub_dir,       "topo");
        strcpy(rcfd->File_Dict->local_diff_sub_dir, "local_diff");
        strcpy(rcfd->File_Dict->c2c_sub_dir,        "c2c");
        strcpy(rcfd->File_Dict->rec_sub_dir,        "rec");
        strcpy(rcfd->File_Dict->user_sub_dir,       "user");
        strcpy(rcfd->File_Dict->post_dir,           "post");

        rcfd->File_Dict->metadata_filename              = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->tracer_start_position_filename = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->C2C_filename                   = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Prep_Transscript_filename      = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Run_Transscript_filename       = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Norm_filename                  = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->vof_filename                   = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->local_diff_filename            = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->rec_user_filename              = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Jump_filename                  = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Matrix_filename                = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Balance_filename               = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Rec_Frames_filename            = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Topo_filename                  = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Dump_filename                  = (char*)malloc(32*sizeof(char));
        rcfd->File_Dict->Probe_filename                 = (char*)malloc(32*sizeof(char));

        strcpy(rcfd->File_Dict->metadata_filename,              "./data/rcfd_db.meta.toml");
        strcpy(rcfd->File_Dict->tracer_start_position_filename, "./data/tmp/tracer_start_pos.inj");
        strcpy(rcfd->File_Dict->C2C_filename,                   "./data/c2c/c2c");
        strcpy(rcfd->File_Dict->Prep_Transscript_filename,      "./rCFD_prep.trn");
        strcpy(rcfd->File_Dict->Run_Transscript_filename,       "./rCFD_run.trn");
        strcpy(rcfd->File_Dict->Norm_filename,                  "./data/tmp/norm");
        strcpy(rcfd->File_Dict->vof_filename,                   "./data/vof/vof");
        strcpy(rcfd->File_Dict->local_diff_filename,            "./data/local_diff/local_diff");
        strcpy(rcfd->File_Dict->rec_user_filename,              "./data/user/rec_user");
        strcpy(rcfd->File_Dict->Jump_filename,                  "./data/rec/jump");
        strcpy(rcfd->File_Dict->Matrix_filename,                "./data/rec/matrix");
        strcpy(rcfd->File_Dict->Balance_filename,               "./rCFD_balance.out");
        strcpy(rcfd->File_Dict->Rec_Frames_filename,            "./post/rec_frames_monitor.out");
        strcpy(rcfd->File_Dict->Topo_filename,                  "./data/topo/topo");
        strcpy(rcfd->File_Dict->Dump_filename,                  "./post/dump");
        strcpy(rcfd->File_Dict->Probe_filename,                 "./post/probes");
    }

    void default_Phase_Dict(rCFD_type* rcfd)
    {
#if RP_NODE
        int i_phase;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            rcfd->Phase_Dict[i_phase].number_of_data = 1;

            rcfd->Phase_Dict[i_phase].number_of_concentration_data = 1;

            rcfd->Phase_Dict[i_phase].time_step = rcfd->Solver_Dict->global_time_step;

            rcfd->Phase_Dict[i_phase].density = 1.0;

            rcfd->Phase_Dict[i_phase].heat_capacity = 1.0;

            rcfd->Phase_Dict[i_phase].reference_temperature = 300.0;

            rcfd->Phase_Dict[i_phase].vof_max = 1.0;

            rcfd->Phase_Dict[i_phase].hindered_drift_on = 0;

            rcfd->Phase_Dict[i_phase].local_drift_on = 0;

            rcfd->Phase_Dict[i_phase].drift_control_conc_sum_on = 1;

            rcfd->Phase_Dict[i_phase].d32_diam = 1.0;

            rcfd->Phase_Dict[i_phase].number_of_user_vars = 0;

            rcfd->Phase_Dict[i_phase].user = NULL;
        }
#endif
    }

    void default_Prep_Dict(rCFD_type* rcfd)
    {
#if RP_NODE

        // start tracers
        rcfd->Prep_Dict->tracer_format = RCFD_TRACER_FORCE_GUIDED;

        rcfd->Prep_Dict->tracers_per_cell_count = 1;

        rcfd->Prep_Dict->tracer_region_of_interest_exists = 0;

        rcfd->Prep_Dict->tracer_ROI_x_min = 0.0;
        rcfd->Prep_Dict->tracer_ROI_x_max = 0.0;
        rcfd->Prep_Dict->tracer_ROI_y_min = 0.0;
        rcfd->Prep_Dict->tracer_ROI_y_max = 0.0;
        rcfd->Prep_Dict->tracer_ROI_z_min = 0.0;
        rcfd->Prep_Dict->tracer_ROI_z_max = 0.0;

        rcfd->Prep_Dict->tracer_coarse_graining = 1;

        if(rcfd->Solver_Dict->shifts_from_displacement_fields_on == 0){

            int i_phase;
            int phase_count = rcfd->Solver_Dict->number_of_phases;

            memset(rcfd->Prep_Dict->tracer_random_walk_on, 0, phase_count * sizeof(int));

            for(i_phase = 0; i_phase < phase_count; i_phase++){

                rcfd->Prep_Dict->tracer_random_walk_velocity_ratio[i_phase] = 0.1;
            }
        }

        rcfd->Prep_Dict->tracer_reverse_direction_on = 0;
        rcfd->Prep_Dict->tracer_splitting_on = 1;
        // ... end tracers

        // start interpolation_on
        rcfd->Prep_Dict->interpolation_format = RCFD_INTERP_NEAREST_ALIGNED_NEIGHBOR_CELLS;

        rcfd->Prep_Dict->interpolation_max_number_of_aligned_neigh_cells = 100;

        rcfd->Prep_Dict->interpolation_pruning_on =               0;
        rcfd->Prep_Dict->interpolation_min_shifts_after_pruning = 2;
        rcfd->Prep_Dict->interpolation_pruning_weight =           0.05;

        rcfd->Prep_Dict->interpolation_multiply_shift_weights_with_vof_on = 1;
        // ... end interpolation

#endif
    }

    void default_Norm_Dict(rCFD_type* rcfd)
    {
#if RP_NODE
        rcfd->Norm_Dict->format = RCFD_NORM_DEFAULT;

        rcfd->Norm_Dict->coarse_graining = 1;

        rcfd->Norm_Dict->larger_than_mean_norm_only = 0;
#endif
    }

    void default_Rec_Dict(rCFD_type* rcfd)
    {
        rcfd->Rec_Dict->format = RCFD_REC_QUARTER_JUMPS;

        rcfd->Rec_Dict->monitor_rec_frames_on = 0;

        rcfd->Rec_Dict->seed = -1;

        // rcfd->Rec_Dict->adapt_vof_stitching_on
        {
            if((rcfd->Solver_Dict->number_of_phases > 1) || (rcfd->Solver_Dict->single_phase_vof)){

                rcfd->Rec_Dict->adapt_vof_stitching_on = 1;
            }
            else{

                rcfd->Rec_Dict->adapt_vof_stitching_on = 0;
            }

            rcfd->Rec_Dict->number_of_adapt_vof_loops = 1;
        }

        // rcfd->Rec_Dict->min/max_seq_length
        {
            rcfd->Rec_Dict->min_seq_length = (int)((double)rcfd->Solver_Dict->number_of_frames/25.);

            if(rcfd->Rec_Dict->min_seq_length < 1){

                rcfd->Rec_Dict->min_seq_length = 1;
            }

            rcfd->Rec_Dict->max_seq_length = (int)((double)rcfd->Solver_Dict->number_of_frames/4.);

            if(rcfd->Rec_Dict->max_seq_length < 1){

                rcfd->Rec_Dict->max_seq_length = 1;
            }
        }

        // rcfd->Rec_Dict->off_diagonal_band_width
        {
            rcfd->Rec_Dict->off_diagonal_band_width = (int)((double)rcfd->Solver_Dict->number_of_frames/10.);

            if(rcfd->Rec_Dict->off_diagonal_band_width < 1){

                rcfd->Rec_Dict->off_diagonal_band_width = 1;
            }
        }
    }

    void default_Data_Dict(rCFD_type* rcfd)
    {
#if RP_NODE
        int i_phase, i_data, i_dim;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Data_Dict[i_phase][i_data].type = RCFD_DATA_GENERIC;

                rcfd->Data_Dict[i_phase][i_data].face_swap_diffusion = 1./8.;

                rcfd->Data_Dict[i_phase][i_data].binarization_art_diff = 1./8.;

                rcfd->Data_Dict[i_phase][i_data].drifting_data = 0;

                if(rcfd->Solver_Dict->data_drifting_on){

                    rcfd->Data_Dict[i_phase][i_data].drift_velocity = (double*)malloc(3 * sizeof(double));

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        rcfd->Data_Dict[i_phase][i_data].drift_velocity[i_dim] = 0.0;
                    }
                }
                else{

                    rcfd->Data_Dict[i_phase][i_data].drift_velocity = NULL;
                }

                rcfd->Data_Dict[i_phase][i_data].d1_diam = 1.0;

                rcfd->Data_Dict[i_phase][i_data].d2_diam = 1.0;

                rcfd->Data_Dict[i_phase][i_data].d3_diam = 1.0;

                rcfd->Data_Dict[i_phase][i_data].number_of_user_vars = 0;

                rcfd->Data_Dict[i_phase][i_data].user = NULL;
            }
        }
#endif
    }

    void default_Balance_Dict(rCFD_type* rcfd)
    {
#if RP_NODE
        int i_phase, i_data;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Balance_Dict[i_phase][i_data].type = RCFD_BALANCE_GLOBAL;

                rcfd->Balance_Dict[i_phase][i_data].max_correction_loops = 10;

                rcfd->Balance_Dict[i_phase][i_data].accuracy_level = 0.01;

                rcfd->Balance_Dict[i_phase][i_data].write_balance_to_file = 1;

                rcfd->Balance_Dict[i_phase][i_data].write_balance_to_file_interval = 1;
            }
        }
#endif
    }

    void default_Topo_Dict(rCFD_type* rcfd)
    {
        rcfd->Topo_Dict->Cell_Dict = NULL;
        rcfd->Topo_Dict->Face_Dict = NULL;
        rcfd->Topo_Dict->Node_Dict = NULL;
    }

    void default_Cell_Dict(rCFD_type* rcfd)
    {
        int i_layer;

        // set Cell_Dict Vars
        for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

            _CELL_DICT(i_layer).number_of_int_cells = 0;
            _CELL_DICT(i_layer).number_of_ext_cells = 0;
            _CELL_DICT(i_layer).number_of_cells = 0;

            _CELL_DICT(i_layer).number_of_user_vars = 0;
            _CELL_DICT(i_layer).number_of_rec_user_vars = 0;

            _CELL_DICT(i_layer).next_neigh_cell_skin_factor = 2.0;
        }

        // Note: must be set by user via user_set_cell_dict(rcfd, i_layer)
    }

    void default_Face_Dict(rCFD_type* rcfd)
    {
        int i_layer;

        // set Face_Dict vars
        for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

            _FACE_DICT(i_layer).number_of_int_faces = 0;
            _FACE_DICT(i_layer).number_of_ext_faces = 0;
            _FACE_DICT(i_layer).number_of_faces = 0;
        }

        // Note: must be set by user via user_set_face_dict(rcfd, i_layer)
    }

    void default_Node_Dict(rCFD_type* rcfd)
    {
        int i_layer;

        // set Node_Dict vars
        for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

            _NODE_DICT(i_layer).number_of_nodes = 0;
            _NODE_DICT(i_layer).number_of_int_node_references = 0;
            _NODE_DICT(i_layer).number_of_ext_node_references = 0;
        }
    }

    void default_Dump_Dict(rCFD_type* rcfd)
    {
#if RP_NODE
        int i_phase, i_data, i_user, i_layer = 0;

        rcfd->Dump_Dict->dump_interval = 0;

        rcfd->Dump_Dict->overwrite_interval = 0;

        rcfd->Dump_Dict->multifile_output_on = 0;

        rcfd->Dump_Dict->dump_file_type = RCFD_FILE_BINARY;

        rcfd->Dump_Dict->dump_vof = 0;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                rcfd->Dump_Dict->dump_cell_data[i_phase][i_data] = 0;
            }
        }

        for(i_user = 0; i_user < _CELL_DICT(i_layer).number_of_user_vars; i_user++){

            rcfd->Dump_Dict->dump_user_data[i_user] = 0;
        }

        for(i_user = 0; i_user < _CELL_DICT(i_layer).number_of_rec_user_vars; i_user++){

            rcfd->Dump_Dict->dump_rec_user_data[i_user] = 0;
        }

        rcfd->Dump_Dict->probe_count = 0;
        rcfd->Dump_Dict->probe_interval = 0;
        rcfd->Dump_Dict->probe_layer = NULL;
        rcfd->Dump_Dict->probe_cell = NULL;
        rcfd->Dump_Dict->probe_type = NULL;
        rcfd->Dump_Dict->probe_phase = NULL;
        rcfd->Dump_Dict->probe_data = NULL;
        rcfd->Dump_Dict->probe_value = NULL;
        rcfd->Dump_Dict->probe_x = NULL;
#endif
    }

    /* B. default vars settings */

    void default_Solver(rCFD_type* rcfd)
    {
#if RP_NODE
        rcfd->Solver->current_state =  0;
        rcfd->Solver->upcoming_state = 0;
        rcfd->Solver->current_layer =  0;
        rcfd->Solver->current_frame =  0;
        rcfd->Solver->prev_frame =     0;

        rcfd->Solver->analyse_CFD_counter =       0;
        rcfd->Solver->shifts_ready2write =        0;
        rcfd->Solver->tracer_monitoring_started = 0;
        rcfd->Solver->displ_reset_counter =       0;

        rcfd->Solver->c2c_shift_format = (compute_node_count > 256) ? RCFD_SHIFT_C0_N0_C1_W0CHAR : RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR;
        rcfd->Solver->c2c_file_type =  RCFD_FILE_BINARY;
        rcfd->Solver->c2c_shift_type_count = 2;
        rcfd->Solver->total_shift_read_count = 0;

        rcfd->Solver->global_run_counter = 0;
        rcfd->Solver->global_time = 0.0;

        if(rcfd->Solver->timestep_width != NULL){

            int i_layer;

            for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                rcfd->Solver->timestep_width[i_layer] = rcfd->Solver_Dict->global_time_step * (1 << i_layer); //pow(2.0, (double) i_layer);
            }
        }

        rcfd->Solver->balance_file_opened = 0;
        rcfd->Solver->rec_frames_monitor_file_opened = 0;
#endif
    }

    void default_Topo(rCFD_type* rcfd)
    {
#if RP_NODE
        rcfd->Topo->Cell = NULL;
        rcfd->Topo->Face = NULL;
        rcfd->Topo->Node = NULL;
#endif
    }

    void default_Cell_L0(rCFD_type* rcfd)
    {
#if RP_NODE

#if 1   // local vars

        int     i_phase, i_cell, i_frame, i_rec_user, i_tmp;

        int     i_layer = 0;

        int     cell_count =     _CELL_DICT(i_layer).number_of_cells;
        int     int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
        int     phase_count = rcfd->Solver_Dict->number_of_phases;
        int     data_count;
        int     max_phase_data_count = rcfd->Solver_Dict->max_phase_data_count;
        int     user_count =     _CELL_DICT(i_layer).number_of_user_vars;
        int     rec_user_count = _CELL_DICT(i_layer).number_of_rec_user_vars;
        int     frame_count = rcfd->Solver_Dict->number_of_frames;
#endif

        // Note: _C(i_layer).x, _C(i_layer).volume must have been set already
        // Note: neigh_cells will be set later

        for(i_cell = 0; i_cell < cell_count; i_cell++){

            _C(i_layer).grid_spacing[i_cell] = pow(_C(i_layer).volume[i_cell], (1.0/3.0));
        }

        memset(_C(i_layer).average_velocity[0], 0, phase_count * cell_count * sizeof(double));
        memset(_C(i_layer).crossing_time   [0], 0, phase_count * cell_count * sizeof(double));

        memset(_C(i_layer).hit_by_other_cell, 0, cell_count * sizeof(short));

        memset(_C(i_layer).weight_after_shift, 0, cell_count * sizeof(double));
        memset(_C(i_layer).weight_after_swap,  0, cell_count * sizeof(double));

        if(_C(i_layer).vof != NULL){

            if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

                i_frame = 0;
                i_phase = 0;

                for(i_cell = 0; i_cell < cell_count; i_cell++){

                    _C(i_layer).vof[i_frame][i_cell][i_phase] = 1.0;
                }

                for(i_phase = 1; i_phase < phase_count; i_phase++){

                    for(i_cell = 0; i_cell < cell_count; i_cell++){

                        _C(i_layer).vof[i_frame][i_cell][i_phase] = 0.0;
                    }
                }
            }
            else{ // run mode

                if((rcfd->Solver_Dict->number_of_phases == 1) && (rcfd->Solver_Dict->single_phase_vof == 0)){

                    i_phase = 0;

                    for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                        for(i_cell = 0; i_cell < cell_count; i_cell++){

                            _C(i_layer).vof[i_frame][i_cell][i_phase] = 1.0;
                        }
                    }
                }
                else{   // multi-phase or single-phase with vof

                    int     i_state;
                    short   vof_file_exists = 1;
                    FILE    *f_in = NULL;
                    char    file_name[80];

                    i_state = 0;    // TODO multiple DBs

                    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                        sprintf(file_name,"%s_%d_%d_%d", rcfd->File_Dict->vof_filename, i_state, i_phase, myid);

                        f_in = fopen(file_name, "r");

                        if(f_in == NULL){

                            for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                                for(i_cell = 0; i_cell < cell_count; i_cell++){

                                    if(i_phase == 0){

                                        _C(i_layer).vof[i_frame][i_cell][i_phase] = 1.0;
                                    }
                                    else{

                                        _C(i_layer).vof[i_frame][i_cell][i_phase] = 0.0;
                                    }
                                }
                            }

                            vof_file_exists = 0;
                        }
                        // read vof field
                        else{

                            for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                                fscanf(f_in,"%d\n", &i_tmp);

                                if(i_tmp != cell_count){

                                    report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                                 "[%d] default_Cell: _C(i_layer).vof: i_tmp != cell_count ...", myid);

                                    fclose(f_in);

                                    return;
                                }

                                for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                                    fscanf(f_in,"%le\n", &_C(i_layer).vof[i_frame][i_cell][i_phase]);

                                    if((_C(i_layer).vof[i_frame][i_cell][i_phase] < 0.0) || (_C(i_layer).vof[i_frame][i_cell][i_phase] > 1.0)){

                                        report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                                     "[%d] i_frame %d i_cell %d i_phase %d vof %e",
                                                     myid, i_frame, i_cell, i_phase, _C(i_layer).vof[i_frame][i_cell][i_phase]);
                                        return;
                                    }
                                }
                            }

                            fclose(f_in);
                        }
                    }

                    vof_file_exists = PRF_GILOW1(vof_file_exists);

                    if(!vof_file_exists){

                        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                        "default_Cell_L0: at least one vof file doesn't exist");
                    }
                }
            }
        }

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            data_count = rcfd->Phase_Dict[i_phase].number_of_data;

            memset(_C(i_layer).data[i_phase][0], 0, cell_count * data_count * sizeof(double));
        }

        memset(_C(i_layer).data_shift[0], 0, cell_count * max_phase_data_count * sizeof(double));
        memset(_C(i_layer).data_swap [0], 0, cell_count * max_phase_data_count * sizeof(double));

        memset(_C(i_layer).data_shift_updated, 0, cell_count * sizeof(short));

        if(rcfd->Solver_Dict->local_diff_on){

            if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

                memset(_C(i_layer).local_diff[0][0], 0, phase_count * cell_count * sizeof(double));
            }
            else{   // run mode

                int     i_state;
                char    local_diff_filename[80];
                FILE    *f_local_diff = NULL;

                // read _C(i_layer).local_diff
                for(i_phase = 0; i_phase < phase_count; i_phase++){

                    // open & check file
                    {
                        i_state = 0;    // TODO: How do we incorporate changes in operating conditions ?

                        sprintf(local_diff_filename,"%s_%d_%d_%d", rcfd->File_Dict->local_diff_filename, i_state, i_phase, myid);

                        f_local_diff = fopen(local_diff_filename, "r");

                        if(!f_local_diff){

                            report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                         "[%d] default_Cell_L0: could not open %s",
                                         myid, local_diff_filename);

                            return;
                        }
                    }

                    for(i_frame = 0; i_frame < frame_count; i_frame++){

                        fscanf(f_local_diff, "%d \n", &i_tmp);

                        if(i_tmp != int_cell_count){

                            report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                         "[%d] default_Cell_L0: number of cells in %s does not match _Cell.Dict (%d %d - %d)",
                                         myid, local_diff_filename, i_tmp, int_cell_count, i_frame);

                            fclose(f_local_diff);

                            return;
                        }

                        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                            fscanf(f_local_diff, "%le \n", &_C(i_layer).local_diff[i_frame][i_phase][i_cell]);

                            if(_C(i_layer).local_diff[i_frame][i_phase][i_cell] < 0.0){

                                report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                             "[%d] default_Cell_L0: _C(i_layer).local_diff < 0.0", myid);
                                return;
                            }
                        }
                    }

                    fclose( f_local_diff);
                }
            }
        }

        if(_C(i_layer).drift_exchange != NULL){

            memset(_C(i_layer).drift_exchange, 0, cell_count * sizeof(double));
        }

        if(_C(i_layer).cfd_displ != NULL){

            memset(_C(i_layer).cfd_displ[0][0], 0, 3 * phase_count * cell_count * sizeof(double));
        }

        if(_C(i_layer).cfd_displ_diff != NULL){

            memset(_C(i_layer).cfd_displ_diff[0][0], 0, 6 * phase_count * cell_count * sizeof(double));
        }

        memset(_C(i_layer).tmp, 0, cell_count * sizeof(double));

        if(_C(i_layer).user != NULL){

            memset(_C(i_layer).user[0], 0, cell_count * user_count * sizeof(double));
        }

        if(_C(i_layer).rec_user != NULL){

            if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

                memset(_C(i_layer).rec_user[0][0], 0, cell_count * rec_user_count * sizeof(double));
            }
            else{   // run mode

                FILE    *f_in = NULL;
                char    file_name[80];

                // first initialization
                memset(_C(i_layer).rec_user[0][0], 0, frame_count * cell_count * rec_user_count * sizeof(double));

                // open file
                {
                    sprintf(file_name,"%s_%d_%d", rcfd->File_Dict->rec_user_filename, rcfd->Solver->current_state, myid);

                    f_in = fopen(file_name, "r");

                    if(f_in == NULL){

                        report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                     "[%d] default_Cell_L0: failed to open file %s", myid, file_name);
                        return;
                    }
                }

                // read rec_user field
                {
                    for(i_frame = 0; i_frame < frame_count; i_frame++){

                        fscanf(f_in, "%d\n", &i_tmp);

                        if(i_tmp != int_cell_count){

                            report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                         "[%d] default_Cell_L0: number of rec_user variables != int_cell_count", myid);

                            fclose(f_in);

                            return;
                        }

                        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                            for(i_rec_user = 0; i_rec_user < rec_user_count; i_rec_user++){

                                fscanf(f_in, "%lf ", &_C(i_layer).rec_user[i_frame][i_cell][i_rec_user]);
                            }

                            fscanf(f_in, "\n");
                        }
                    }
                }
            }
        }
#endif
    }

    void default_Face_L0(rCFD_type* rcfd)
    {
#if RP_NODE

#if 1   // local vars

        int     i_layer, i_phase, i_data, i_face;

#endif

        i_layer = 0;

        // Note: _F(i_layer).c0/c1 and _F(i_layer).area must be set by user

        // eventually, set _F(i_layer).local_drift_adaption
        if(rcfd->Solver_Dict->data_drifting_on){

            for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){    // in order to reach max of i_data;

                    for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                        _F(i_layer).local_drift_adaption[i_face][i_data] = 1.0;
                    }
                }
            }
        }
#endif
    }

    void default_c2c_prep(rCFD_type* rcfd)
    {
#if RP_NODE
        int i_phase, i_shift, i_dim;
        int shifts_count;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            shifts_count = rcfd->C2C_Prep[i_phase].shifts_count; // already set by shift_allocate_lists

            rcfd->C2C_Prep[i_phase].valid_shifts_count = 0;

            for(i_shift = 0; i_shift < shifts_count; i_shift++){

                rcfd->C2C_Prep[i_phase].c0      [i_shift] = -1;
                rcfd->C2C_Prep[i_phase].node0   [i_shift] = -1;
                rcfd->C2C_Prep[i_phase].c1      [i_shift] = -1;
                rcfd->C2C_Prep[i_phase].node1   [i_shift] = -1;
                rcfd->C2C_Prep[i_phase].w0      [i_shift] = 0.0;
                rcfd->C2C_Prep[i_phase].dist_sq [i_shift] = 0.0;

                for(i_dim = 0; i_dim < 3; i_dim++){

                    rcfd->C2C_Prep[i_phase].c0_coords[i_shift][i_dim] = 0.0;
                    rcfd->C2C_Prep[i_phase].displ    [i_shift][i_dim] = 0.0;
                }
            }
        }
#endif
    }

    void default_c2c(rCFD_type* rcfd)
    {
#if RP_NODE
        int i_state, i_phase, i_frame, i_layer;

        for(i_state = 0; i_state < rcfd->Solver_Dict->number_of_states; i_state++){

            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                    for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                        rcfd->C2C[i_state][i_phase][i_frame][i_layer].waterproof_shifts =        0;
                        rcfd->C2C[i_state][i_phase][i_frame][i_layer].invalid_shifts_exist =     1;
                        rcfd->C2C[i_state][i_phase][i_frame][i_layer].shift_weights_add_to_one = 0;
                        rcfd->C2C[i_state][i_phase][i_frame][i_layer].c1_is_interior_cell =      1;

                        shift_initialize_lists(rcfd, &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts    );
                        shift_initialize_lists(rcfd, &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in );
                        shift_initialize_lists(rcfd, &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_out);

                        memset(rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_to_node_count, 0, compute_node_count * sizeof(int));

                        rcfd->C2C[i_state][i_phase][i_frame][i_layer].out2in = NULL;
                    }
                }
            }
        }
#endif
    }

    void default_Norms(rCFD_type* rcfd)
    {
#if RP_NODE
        rcfd->Norms->frame_counter = 0;
#endif
    }

    void default_Rec(rCFD_type* rcfd)
    {
#if RP_NODE
        int i_state, i_state2, i_frame;

        int jump_file_count = 0;
        int state_count = rcfd->Solver_Dict->number_of_states;
        int frame_count = rcfd->Solver_Dict->number_of_frames;
        int current_frame;

        char    filename[80];
        FILE    *fi = NULL;

        rcfd->Rec->frame_in_sequence =    0;
        rcfd->Rec->sequence_length =      rcfd->Rec_Dict->min_seq_length;
        rcfd->Rec->jumped_at_last_frame = 0;

        for(i_state = 0; i_state < state_count; i_state++){

            for(i_state2 = 0; i_state2 < state_count; i_state2++){

                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    rcfd->Rec->jumps[i_state][i_state2][i_frame] = -1;
                }

                if(myid == 0){

                    sprintf(filename,"%s_%d_%d_%d", rcfd->File_Dict->Jump_filename, i_state, i_state2, 0);

                    fi = fopen(filename, "r");

                    if(fi != NULL){

                        for(i_frame = 0; i_frame < frame_count; i_frame++){

                            fscanf(fi, "%d \n", &rcfd->Rec->jumps[i_state][i_state2][i_frame]);
                        }

                        fclose(fi);

                        for(i_frame = 0; i_frame < frame_count; i_frame++){

                            current_frame = rcfd->Rec->jumps[i_state][i_state2][i_frame];

                            if((current_frame < 0) ||
                               (current_frame >= frame_count)){

                                report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                    "[%d] default_rec: jump file %s contains invalid frame %d",
                                    myid, rcfd->File_Dict->Jump_filename, current_frame);
                            }
                        }

                        jump_file_count++;
                    }
                }

                PRF_BCAST_INT(&rcfd->Rec->jumps[i_state][i_state2][0], frame_count, 0);
            }
        }

        report_info0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                     "default_rec: read %d jump files from %s",
                     jump_file_count, rcfd->File_Dict->Jump_filename);
#endif
    }

#endif
