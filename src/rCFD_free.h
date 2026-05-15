/******************************************************************************
 * @internal
 * @file        rCFD_free.h
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_FREE
#define RCFD_FREE

#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_shift.h"

void free_report(rCFD_type *rcfd);
void free_parallel(rCFD_type *rcfd);
void free_user(rCFD_type *rcfd);

void free_rcfd(rCFD_type *rcfd)
{
    free_report(rcfd);
    free_parallel(rcfd);
    free_user(rcfd);

    free(rcfd);
}

void free_lists_of_shift(rCFD_type *rcfd, C2C_Shift_type *shifts)
{
#if RP_NODE

    if(shifts != NULL){

        free(shifts->c0);
        free(shifts->node0);
        free(shifts->node0char);
        free(shifts->c1);
        free(shifts->node1);
        free(shifts->w0);
        free(shifts->w0char);
        free(shifts->dist_sq);

        free_r_2d(shifts->c0_coords);
        free_r_2d(shifts->displ);
    }

#endif
}

void free_c2c_prep(rCFD_type *rcfd)
{
#if RP_NODE

    if(rcfd->C2C_Prep != NULL){

        int i_phase;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            free_lists_of_shift(rcfd, &rcfd->C2C_Prep[i_phase]);
        }

        free(rcfd->C2C_Prep);

        rcfd->C2C_Prep = NULL;
    }
#endif
}

void free_c2c_interface(rCFD_type *rcfd)
{
#if RP_NODE

    if(rcfd->C2C_Interface != NULL){

        free_lists_of_shift(rcfd, rcfd->C2C_Interface);

        free(rcfd->C2C_Interface);

        rcfd->C2C_Interface = NULL;
    }
#endif
}

void free_c2c(rCFD_type *rcfd)
{
#if RP_NODE

    if(rcfd->C2C != NULL){

        int i_state, i_phase, i_frame, i_layer;

        int state_count =  rcfd->Solver_Dict->number_of_states;
        int phase_count =  rcfd->Solver_Dict->number_of_phases;
        int frame_count =  rcfd->Solver_Dict->number_of_frames;
        int layer_count =  rcfd->Solver_Dict->number_of_layers;

        for(i_state = 0; i_state < state_count; i_state++){

            if(rcfd->C2C[i_state] != NULL){

                for(i_phase = 0; i_phase < phase_count; i_phase++){

                    if(rcfd->C2C[i_state][i_phase] != NULL){

                        for(i_frame = 0; i_frame < frame_count; i_frame++){

                            if(rcfd->C2C[i_state][i_phase][i_frame] != NULL){

                                for(i_layer = 0; i_layer < layer_count; i_layer++){

                                    free_lists_of_shift(rcfd, &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts);
                                    free_lists_of_shift(rcfd, &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in);
                                    free_lists_of_shift(rcfd, &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_out);

                                    free(rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_to_node_count);
                                    free(rcfd->C2C[i_state][i_phase][i_frame][i_layer].out2in);
                                }

                                free(rcfd->C2C[i_state][i_phase][i_frame]);
                            }
                        }

                        free(rcfd->C2C[i_state][i_phase]);
                    }
                }

                free(rcfd->C2C[i_state]);
            }
        }

        free(rcfd->C2C);
        rcfd->C2C = NULL;
    }
#endif
}

void free_parallel_comm(rCFD_type *rcfd)
{
#if RP_NODE

    int i_layer;

    if(rcfd->Parallel->MPI_Topo->MPI_Cell != NULL){

        for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

            if(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].number_of_ext_cells_per_node != NULL){

                free(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].number_of_ext_cells_per_node);
            }

            if(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].number_of_host_cells_per_node != NULL){

                free(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].number_of_host_cells_per_node);
            }

            if(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].host_of_cell != NULL){

                free(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].host_of_cell);
            }

            if(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].host_of_ext_cells != NULL){

                free(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].host_of_ext_cells);
            }

            if(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].hosting_cell_index != NULL){

                free(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].hosting_cell_index);
            }

            if(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].host2ext_index != NULL){

                free(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].host2ext_index);
            }

            if(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].data != NULL){

                free(rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer].data);
            }
        }

        free(rcfd->Parallel->MPI_Topo->MPI_Cell);
    }

    free(rcfd->Parallel->MPI_Topo);

    free(rcfd->Parallel->transfer_data_in );
    free(rcfd->Parallel->transfer_data_out);
#endif
}

void free_parallel(rCFD_type *rcfd)
{
#if RP_NODE
    free(rcfd->Parallel);
    rcfd->Parallel = NULL;
#endif
}

void free_balance(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Balance != NULL){

        int i_phase;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            if(rcfd->Balance[i_phase] != NULL){

                free(rcfd->Balance[i_phase]);
            }
        }

        free(rcfd->Balance);
        rcfd->Balance = NULL;
    }
#endif
}

void free_norms(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Norms != NULL){

        free(rcfd->Norms->norm);

        free(rcfd->Norms);
        rcfd->Norms = NULL;
    }
#endif
}

void free_rec(rCFD_type *rcfd)
{
    if(rcfd->Rec != NULL){

        free_i_3d(rcfd->Rec->jumps);

        free(rcfd->Rec);
        rcfd->Rec = NULL;
    }
}

void free_topo(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Topo != NULL){

        int i_phase, i_cell, i_layer;

        if(rcfd->Topo->Cell != NULL){

            for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                free_r_2d(_C(i_layer).x);

                free(_C(i_layer).volume);

                free(_C(i_layer).grid_spacing);

                free(_C(i_layer).number_of_neigh_cells);
                free(_C(i_layer).number_of_next_neigh_cells);

                if(_C(i_layer).neigh_cell != NULL){

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        free(_C(i_layer).neigh_cell[i_cell]);
                    }
                }

                if(_C(i_layer).next_neigh_cell != NULL){

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        free(_C(i_layer).next_neigh_cell[i_cell]);
                    }
                }

                free(_C(i_layer).neigh_cell);
                free(_C(i_layer).next_neigh_cell);

                free_r_2d(_C(i_layer).average_velocity);

                free_r_2d(_C(i_layer).crossing_time);

                free(_C(i_layer).hit_by_other_cell);

                free(_C(i_layer).weight_after_shift);
                free(_C(i_layer).weight_after_swap);

                free_r_3d(_C(i_layer).vof);

                free_r_2d(_C(i_layer).vof_changed);

                if(_C(i_layer).data != NULL){

                    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                        free_r_2d(_C(i_layer).data[i_phase]);
                    }

                    free(_C(i_layer).data);
                }

                free_r_2d(_C(i_layer).data_shift);
                free_r_2d(_C(i_layer).data_swap);

                free(_C(i_layer).data_shift_updated);

                free_r_3d(_C(i_layer).local_diff);

                free(_C(i_layer).drift_exchange);

                free_r_3d(_C(i_layer).cfd_displ);
                free_r_3d(_C(i_layer).cfd_displ_diff);

                free(_C(i_layer).tmp);

                free_r_2d(_C(i_layer).user);

                free_r_3d(_C(i_layer).rec_user);

                free(_C(i_layer).marked);

                free(_C(i_layer).parent_cell);

                free(_C(i_layer).number_of_children);

                if(_C(i_layer).child_index != NULL){

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        free(_C(i_layer).child_index[i_cell]);
                    }

                    free(_C(i_layer).child_index);
                }

                free(_C(i_layer).type);

                free(_C(i_layer).number_of_nodes);

                free_i_2d(_C(i_layer).nodes);

                free(_C(i_layer).number_of_poly_faces);

                free_i_2d(_C(i_layer).number_of_poly_face_nodes);

                free(_C(i_layer).poly_nodes);
            }

            free(rcfd->Topo->Cell);
        }

        if(rcfd->Topo->Face != NULL){

            for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                free(_F(i_layer).c0);

                free(_F(i_layer).c1);

                free_r_2d(_F(i_layer).area);

                if(rcfd->Solver_Dict->data_drifting_on){

                    free_r_2d(_F(i_layer).local_drift_adaption);
                }
            }

            free(rcfd->Topo->Face);
        }

        if(rcfd->Topo->Node != NULL){

            for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                free_r_2d(_N(i_layer).x);
            }
        }

        free(rcfd->Topo);
        rcfd->Topo = NULL;
    }
#endif
}

void free_dump_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Dump_Dict != NULL){

        if(rcfd->Dump_Dict->dump_cell_data){

            int i_phase;

            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                free(rcfd->Dump_Dict->dump_cell_data[i_phase]);
            }

            free(rcfd->Dump_Dict->dump_cell_data);
        }

        free(rcfd->Dump_Dict->dump_user_data);
        free(rcfd->Dump_Dict->dump_rec_user_data);

        free(rcfd->Dump_Dict->probe_layer);
        free(rcfd->Dump_Dict->probe_cell);
        free(rcfd->Dump_Dict->probe_type);
        free(rcfd->Dump_Dict->probe_phase);
        free(rcfd->Dump_Dict->probe_data);
        free(rcfd->Dump_Dict->probe_value);
        free_r_2d(rcfd->Dump_Dict->probe_x);

        free(rcfd->Dump_Dict);
        rcfd->Dump_Dict = NULL;
    }
#endif
}

void free_topo_dict(rCFD_type *rcfd)
{
    if(rcfd->Topo_Dict != NULL){

#if RP_NODE
        free(rcfd->Topo_Dict->Cell_Dict);

        free(rcfd->Topo_Dict->Face_Dict);

        free(rcfd->Topo_Dict->Node_Dict);
#endif
        free(rcfd->Topo_Dict);
        rcfd->Topo_Dict = NULL;
    }
}

void free_balance_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Balance_Dict != NULL){

        int i_phase;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            free(rcfd->Balance_Dict[i_phase]);
        }

        free(rcfd->Balance_Dict);
        rcfd->Balance_Dict = NULL;
    }
#endif
}

void free_data_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Data_Dict != NULL){

        int i_phase, i_data;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            if(rcfd->Data_Dict[i_phase] != NULL){

                for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                    free(rcfd->Data_Dict[i_phase][i_data].drift_velocity);

                    free(rcfd->Data_Dict[i_phase][i_data].user);
                }

                free(rcfd->Data_Dict[i_phase]);
            }
        }

        free(rcfd->Data_Dict);
        rcfd->Data_Dict = NULL;
    }
#endif
}

void free_rec_dict(rCFD_type *rcfd)
{
    free(rcfd->Rec_Dict);
    rcfd->Rec_Dict = NULL;
}

void free_norm_dict(rCFD_type *rcfd)
{
#if RP_NODE
    free(rcfd->Norm_Dict);
    rcfd->Norm_Dict = NULL;
#endif
}

void free_prep_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Prep_Dict != NULL){

        free(rcfd->Prep_Dict->tracer_random_walk_on);
        free(rcfd->Prep_Dict->tracer_random_walk_velocity_ratio);

        free(rcfd->Prep_Dict);
        rcfd->Prep_Dict = NULL;
    }
#endif
}

void free_phase_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Phase_Dict != NULL){

        int i_phase;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            free(rcfd->Phase_Dict[i_phase].user);
        }

        free(rcfd->Phase_Dict);
        rcfd->Phase_Dict    = NULL;
    }
#endif
}

void free_solver(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Solver != NULL){

        free(rcfd->Solver->timestep_width);

        free(rcfd->Solver);
        rcfd->Solver = NULL;
    }
#endif
}

void free_file_dict(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->File_Dict != NULL){

        free(rcfd->File_Dict->working_dir);
        free(rcfd->File_Dict->data_dir);
        free(rcfd->File_Dict->tmp_sub_dir);
        free(rcfd->File_Dict->vof_sub_dir);
        free(rcfd->File_Dict->topo_sub_dir);
        free(rcfd->File_Dict->local_diff_sub_dir);
        free(rcfd->File_Dict->c2c_sub_dir);
        free(rcfd->File_Dict->rec_sub_dir);
        free(rcfd->File_Dict->user_sub_dir);
        free(rcfd->File_Dict->post_dir);

        free(rcfd->File_Dict->metadata_filename);
        free(rcfd->File_Dict->tracer_start_position_filename);
        free(rcfd->File_Dict->C2C_filename);
        free(rcfd->File_Dict->Prep_Transscript_filename);
        free(rcfd->File_Dict->Run_Transscript_filename);
        free(rcfd->File_Dict->Norm_filename);
        free(rcfd->File_Dict->vof_filename);
        free(rcfd->File_Dict->local_diff_filename);
        free(rcfd->File_Dict->rec_user_filename);
        free(rcfd->File_Dict->Jump_filename);
        free(rcfd->File_Dict->Matrix_filename);
        free(rcfd->File_Dict->Balance_filename);
        free(rcfd->File_Dict->Rec_Frames_filename);
        free(rcfd->File_Dict->Topo_filename);
        free(rcfd->File_Dict->Dump_filename);
        free(rcfd->File_Dict->Probe_filename);

        free(rcfd->File_Dict);
        rcfd->File_Dict = NULL;
    }
#endif
}

void free_solver_dict(rCFD_type *rcfd)
{
    free(rcfd->Solver_Dict);
    rcfd->Solver_Dict = NULL;
}

void free_report(rCFD_type *rcfd)
{
    if(rcfd->Report != NULL){

        int i;

        if(rcfd->Report->file_is_open) {

            fclose(rcfd->Report->report_file);
            rcfd->Report->report_file = NULL;
            rcfd->Report->file_is_open = 0;
        }

        if(rcfd->Report->file0_is_open) {

#if defined(RCFD_LIB_MPI)
            MPI_File_close(&rcfd->Report->report_file0);
#else
            fclose(rcfd->Report->report_file0);
            rcfd->Report->report_file0 = NULL;
#endif
            rcfd->Report->file0_is_open = 0;
        }

        free_i_2d(rcfd->Report->number_of_reports);

        for(i = 0; i < RCFD_TIMER_MAX; ++i){

            unit_timer_destroy(rcfd->Report->timers[i]);
        }

        free(rcfd->Report);
        rcfd->Report = NULL;
    }
}

void free_user(rCFD_type *rcfd)
{
    free(rcfd->rCFD_UDF);
    rcfd->rCFD_UDF = NULL;
}

void free_all(rCFD_type *rcfd)
{
    rcfd->initialized   = 0;

    free_c2c(rcfd);

    free_parallel_comm(rcfd);

    free_balance(rcfd);

    free_norms(rcfd);

    free_rec(rcfd);

    free_c2c_interface(rcfd);

    free_c2c_prep(rcfd);

    free_dump_dict(rcfd);

    free_topo(rcfd);

    free_topo_dict(rcfd);

    free_balance_dict(rcfd);

    free_data_dict(rcfd);

    free_rec_dict(rcfd);

    free_norm_dict(rcfd);

    free_prep_dict(rcfd);

    free_phase_dict(rcfd);

    free_solver(rcfd);

    free_solver_dict(rcfd);

    free_file_dict(rcfd);
}

#endif
