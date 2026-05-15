/******************************************************************************
 * @internal
 * @file        rCFD_dict_write_toml.h
 * @copyright   (C) 2025
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_DICT_WRITE_TOML_H
#define RCFD_DICT_WRITE_TOML_H

#include <stdio.h>

#define WRITE_DICT_PROPERTY_S(file, dict, property) write_property_s(file, #property, rcfd->dict->property)
#define WRITE_DICT_PROPERTY_I(file, dict, property) write_property_i(file, #property, rcfd->dict->property)
#define WRITE_DICT_PROPERTY_B(file, dict, property) write_property_b(file, #property, rcfd->dict->property)
#define WRITE_DICT_PROPERTY_R(file, dict, property) write_property_r(file, #property, rcfd->dict->property)
#define WRITE_ARRAY_DICT_PROPERTY_S(file, dict, property) write_property_s(file, #property, rcfd->dict.property)
#define WRITE_ARRAY_DICT_PROPERTY_I(file, dict, property) write_property_i(file, #property, rcfd->dict.property)
#define WRITE_ARRAY_DICT_PROPERTY_B(file, dict, property) write_property_b(file, #property, rcfd->dict.property)
#define WRITE_ARRAY_DICT_PROPERTY_R(file, dict, property) write_property_r(file, #property, rcfd->dict.property)

void write_property_s(FILE* out, const char *property_name, const char *property_value)
{
    fprintf(out, "%s = \"%s\"\n", property_name, property_value);
}
void write_property_i(FILE* out, const char *property_name, int property_value)
{
    fprintf(out, "%s = %d\n", property_name, property_value);
}
void write_property_b(FILE* out, const char *property_name, int property_value)
{
    fprintf(out, "%s = %s\n", property_name, property_value ? "true" : "false");
}
void write_property_r(FILE* out, const char *property_name, double property_value)
{
    fprintf(out, "%s = %g\n", property_name, property_value);
}

#if RP_NODE
/**
 * @brief write file dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_file_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0){

        fprintf(out, "[files]\n");
        WRITE_DICT_PROPERTY_S(out, File_Dict, working_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, data_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, tmp_sub_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, vof_sub_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, topo_sub_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, local_diff_sub_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, c2c_sub_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, rec_sub_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, user_sub_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, post_dir);
        WRITE_DICT_PROPERTY_S(out, File_Dict, metadata_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, tracer_start_position_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Prep_Transscript_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Run_Transscript_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, C2C_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Norm_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, vof_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, local_diff_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, rec_user_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Jump_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Matrix_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Balance_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Rec_Frames_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Topo_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Dump_filename);
        WRITE_DICT_PROPERTY_S(out, File_Dict, Probe_filename);
        fprintf(out, "\n");
    }
}

/**
 * @brief write solver dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_solver_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0){

        fprintf(out, "[solver]\n");
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, report_priority);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, profiling_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, multifile_output_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, write_config_to_file_on);

        WRITE_DICT_PROPERTY_I(out, Solver_Dict, number_of_frames);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, number_of_states);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, number_of_phases);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, number_of_layers);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, number_of_runs);

        WRITE_DICT_PROPERTY_B(out, Solver_Dict, recurrence_process_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, data_convection_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, face_swap_diffusion_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, local_diff_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, parallel_diffusion_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, data_binarization_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, data_drifting_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, balance_correction_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, interpolation_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, shifts_from_displacement_fields_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, moving_geometry_on);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, grid_interface_shifts_on);

        WRITE_DICT_PROPERTY_I(out, Solver_Dict, max_number_of_cells_per_time_step);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, time_steps_per_monitoring_interval);
        WRITE_DICT_PROPERTY_R(out, Solver_Dict, start_time_for_monitoring);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, single_phase_vof);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, single_phase_vof_id);
        WRITE_DICT_PROPERTY_R(out, Solver_Dict, vof_cutoff_limit);

        WRITE_DICT_PROPERTY_I(out, Solver_Dict, min_number_of_cells_per_layer);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, max_number_of_faces_per_cell);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, max_number_of_children);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, number_redist_loops_per_layer);

        WRITE_DICT_PROPERTY_R(out, Solver_Dict, global_time_step);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, max_fill_loops);
        WRITE_DICT_PROPERTY_R(out, Solver_Dict, face_swap_max_per_loop);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, face_swap_max_loops);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, local_diff_format);
        WRITE_DICT_PROPERTY_R(out, Solver_Dict, grid_interface_shift_weight);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, number_of_drift_loops);
        WRITE_DICT_PROPERTY_I(out, Solver_Dict, balance_correction_update);
        WRITE_DICT_PROPERTY_B(out, Solver_Dict, control_conc_sum_on);
        fprintf(out, "\n");
    }
}

/**
 * @brief write phase dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_phase_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0){

        int i_phase, i_user;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

            // write as array of tables
            fprintf(out, "[[phases]]\n");
            WRITE_ARRAY_DICT_PROPERTY_I(out, Phase_Dict[i_phase], number_of_data);
            WRITE_ARRAY_DICT_PROPERTY_R(out, Phase_Dict[i_phase], time_step);
            WRITE_ARRAY_DICT_PROPERTY_R(out, Phase_Dict[i_phase], density);
            WRITE_ARRAY_DICT_PROPERTY_R(out, Phase_Dict[i_phase], heat_capacity);
            WRITE_ARRAY_DICT_PROPERTY_R(out, Phase_Dict[i_phase], reference_temperature);
            WRITE_ARRAY_DICT_PROPERTY_R(out, Phase_Dict[i_phase], vof_max);
            WRITE_ARRAY_DICT_PROPERTY_B(out, Phase_Dict[i_phase], hindered_drift_on);
            WRITE_ARRAY_DICT_PROPERTY_B(out, Phase_Dict[i_phase], local_drift_on);
            WRITE_ARRAY_DICT_PROPERTY_B(out, Phase_Dict[i_phase], drift_control_conc_sum_on);
            WRITE_ARRAY_DICT_PROPERTY_R(out, Phase_Dict[i_phase], d32_diam);
            WRITE_ARRAY_DICT_PROPERTY_I(out, Phase_Dict[i_phase], number_of_user_vars);

            // write user vars?
            if(rcfd->Phase_Dict[i_phase].number_of_user_vars > 0){

                fprintf(out, "user = [");
                for(i_user = 0; i_user < rcfd->Phase_Dict[i_phase].number_of_user_vars; ++i_user){

                    fprintf
                    (
                        out,
                        "%g, ",
                        rcfd->Phase_Dict[i_phase].user[i_user]
                    );
                }
                fprintf(out, "]\n");
            }
        }
        fprintf(out, "\n");
    }
}

/**
 * @brief write prep dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_prep_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0){

        int i_phase;

        fprintf(out, "[prep]\n");
        WRITE_DICT_PROPERTY_I(out, Prep_Dict, tracer_format);
        WRITE_DICT_PROPERTY_I(out, Prep_Dict, tracers_per_cell_count);
        WRITE_DICT_PROPERTY_I(out, Prep_Dict, tracer_coarse_graining);

        WRITE_DICT_PROPERTY_B(out, Prep_Dict, tracer_reverse_direction_on);
        WRITE_DICT_PROPERTY_B(out, Prep_Dict, tracer_splitting_on);

        WRITE_DICT_PROPERTY_I(out, Prep_Dict, tracer_region_of_interest_exists);
        WRITE_DICT_PROPERTY_R(out, Prep_Dict, tracer_ROI_x_min);
        WRITE_DICT_PROPERTY_R(out, Prep_Dict, tracer_ROI_x_max);
        WRITE_DICT_PROPERTY_R(out, Prep_Dict, tracer_ROI_y_min);
        WRITE_DICT_PROPERTY_R(out, Prep_Dict, tracer_ROI_y_max);
        WRITE_DICT_PROPERTY_R(out, Prep_Dict, tracer_ROI_z_min);
        WRITE_DICT_PROPERTY_R(out, Prep_Dict, tracer_ROI_z_max);

        fprintf(out, "tracer_random_walk_on = [");
        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

            fprintf
            (
                out,
                "%d, ",
                rcfd->Prep_Dict->tracer_random_walk_on ? rcfd->Prep_Dict->tracer_random_walk_on[i_phase] : 0
            );
        }
        fprintf(out, "]\n");

        fprintf(out, "tracer_random_walk_velocity_ratio = [");
        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

            fprintf
            (
                out,
                "%g, ",
                rcfd->Prep_Dict->tracer_random_walk_velocity_ratio ? rcfd->Prep_Dict->tracer_random_walk_velocity_ratio[i_phase] : 0.1
            );
        }
        fprintf(out, "]\n");

        WRITE_DICT_PROPERTY_I(out, Prep_Dict, interpolation_format);
        WRITE_DICT_PROPERTY_I(out, Prep_Dict, interpolation_max_number_of_aligned_neigh_cells);
        WRITE_DICT_PROPERTY_B(out, Prep_Dict, interpolation_pruning_on);
        WRITE_DICT_PROPERTY_I(out, Prep_Dict, interpolation_min_shifts_after_pruning);
        WRITE_DICT_PROPERTY_R(out, Prep_Dict, interpolation_pruning_weight);
        WRITE_DICT_PROPERTY_B(out, Prep_Dict, interpolation_multiply_shift_weights_with_vof_on);

        fprintf(out, "\n");
    }
}

/**
 * @brief write norm dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_norm_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0){

        fprintf(out, "[norm]\n");
        WRITE_DICT_PROPERTY_I(out, Norm_Dict, format);
        WRITE_DICT_PROPERTY_I(out, Norm_Dict, coarse_graining);
        WRITE_DICT_PROPERTY_B(out, Norm_Dict, larger_than_mean_norm_only);
        fprintf(out, "\n");
    }
}

/**
 * @brief write rec dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_rec_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0){

        fprintf(out, "[rec]\n");
        WRITE_DICT_PROPERTY_I(out, Rec_Dict, format);
        WRITE_DICT_PROPERTY_B(out, Rec_Dict, monitor_rec_frames_on);
        WRITE_DICT_PROPERTY_B(out, Rec_Dict, adapt_vof_stitching_on);
        WRITE_DICT_PROPERTY_I(out, Rec_Dict, min_seq_length);
        WRITE_DICT_PROPERTY_I(out, Rec_Dict, max_seq_length);
        WRITE_DICT_PROPERTY_I(out, Rec_Dict, off_diagonal_band_width);
        WRITE_DICT_PROPERTY_I(out, Rec_Dict, number_of_adapt_vof_loops);
        WRITE_DICT_PROPERTY_I(out, Rec_Dict, seed);
        fprintf(out, "\n");
    }
}

/**
 * @brief write data dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_data_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0 && rcfd->Data_Dict){

        int i_phase, i_data, i_user;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

            // write as array of tables
            fprintf(out, "[[data]]\n");
            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; ++i_data){

                fprintf(out, "[[data.items]]\n");
                WRITE_ARRAY_DICT_PROPERTY_I(out, Data_Dict[i_phase][i_data], type);
                WRITE_ARRAY_DICT_PROPERTY_R(out, Data_Dict[i_phase][i_data], face_swap_diffusion);
                WRITE_ARRAY_DICT_PROPERTY_R(out, Data_Dict[i_phase][i_data], binarization_art_diff);
                WRITE_ARRAY_DICT_PROPERTY_B(out, Data_Dict[i_phase][i_data], drifting_data);
                WRITE_ARRAY_DICT_PROPERTY_R(out, Data_Dict[i_phase][i_data], d1_diam);
                WRITE_ARRAY_DICT_PROPERTY_I(out, Data_Dict[i_phase][i_data], number_of_user_vars);

                if(rcfd->Solver_Dict->data_drifting_on){

                    fprintf
                    (
                        out,
                        "drift_velocity = [%g, %g, %g]\n",
                        rcfd->Data_Dict[i_phase][i_data].drift_velocity[0],
                        rcfd->Data_Dict[i_phase][i_data].drift_velocity[1],
                        rcfd->Data_Dict[i_phase][i_data].drift_velocity[2]
                    );
                }

                if(rcfd->Data_Dict[i_phase][i_data].number_of_user_vars > 0){

                    fprintf(out, "user = [");
                    for(i_user = 0; i_user < rcfd->Data_Dict[i_phase][i_data].number_of_user_vars; ++i_user){

                        fprintf
                        (
                            out,
                            "%g, ",
                            rcfd->Data_Dict[i_phase][i_data].user[i_user]
                        );
                    }
                    fprintf(out, "]\n");
                }
            }
        }
        fprintf(out, "\n");
    }
}

/**
 * @brief write balance dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_balance_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0 && rcfd->Balance_Dict){

        int i_phase, i_data;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

            // write as array of tables
            fprintf(out, "[[balances]]\n");
            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                fprintf(out, "[[balances.items]]\n");
                WRITE_ARRAY_DICT_PROPERTY_I(out, Balance_Dict[i_phase][i_data], type);
                WRITE_ARRAY_DICT_PROPERTY_I(out, Balance_Dict[i_phase][i_data], max_correction_loops);
                WRITE_ARRAY_DICT_PROPERTY_R(out, Balance_Dict[i_phase][i_data], accuracy_level);
                WRITE_ARRAY_DICT_PROPERTY_B(out, Balance_Dict[i_phase][i_data], write_balance_to_file);
                WRITE_ARRAY_DICT_PROPERTY_I(out, Balance_Dict[i_phase][i_data], write_balance_to_file_interval);
            }
        }
        fprintf(out, "\n");
    }
}

/**
 * @brief write cell dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_cell_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd){

        int *recv_number_of_int_cells = NULL;
        int *recv_number_of_ext_cells = NULL;

        if(myid == 0){

            recv_number_of_int_cells = (int*)malloc(sizeof(int) * compute_node_count);
            recv_number_of_ext_cells = (int*)malloc(sizeof(int) * compute_node_count);
        }

        PRF_GATHER_INT(recv_number_of_int_cells, &_CELL_DICT(0).number_of_int_cells, 1, 0);
        PRF_GATHER_INT(recv_number_of_ext_cells, &_CELL_DICT(0).number_of_ext_cells, 1, 0);

        if(out && myid == 0){

            int i_proc;

            fprintf(out, "[cells]\n");

            fprintf(out, "number_of_int_cells = [");
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){

                fprintf
                (
                    out,
                    "%d, ",
                    recv_number_of_int_cells[i_proc]
                );
            }
            fprintf(out, "]\n");

            fprintf(out, "number_of_ext_cells = [");
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){

                fprintf
                (
                    out,
                    "%d, ",
                    recv_number_of_ext_cells[i_proc]
                );
            }
            fprintf(out, "]\n");

            WRITE_ARRAY_DICT_PROPERTY_I(out, Topo_Dict->Cell_Dict[0], number_of_user_vars);
            WRITE_ARRAY_DICT_PROPERTY_I(out, Topo_Dict->Cell_Dict[0], number_of_rec_user_vars);
            WRITE_ARRAY_DICT_PROPERTY_R(out, Topo_Dict->Cell_Dict[0], next_neigh_cell_skin_factor);

            // write user and rec user values?

            fprintf(out, "\n");
        }

        free(recv_number_of_int_cells);
        free(recv_number_of_ext_cells);
    }
}

/**
 * @brief write face dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_face_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd){

        int *recv_number_of_int_faces = NULL;
        int *recv_number_of_ext_faces = NULL;

        if(myid == 0){

            recv_number_of_int_faces = (int*)malloc(sizeof(int) * compute_node_count);
            recv_number_of_ext_faces = (int*)malloc(sizeof(int) * compute_node_count);
        }

        PRF_GATHER_INT(recv_number_of_int_faces, &_FACE_DICT(0).number_of_int_faces, 1, 0);
        PRF_GATHER_INT(recv_number_of_ext_faces, &_FACE_DICT(0).number_of_ext_faces, 1, 0);

        if(out && myid == 0){

            int i_proc;

            fprintf(out, "[faces]\n");

            fprintf(out, "number_of_int_faces = [");
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){

                fprintf
                (
                    out,
                    "%d, ",
                    recv_number_of_int_faces[i_proc]
                );
            }
            fprintf(out, "]\n");

            fprintf(out, "number_of_ext_faces = [");
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){

                fprintf
                (
                    out,
                    "%d, ",
                    recv_number_of_ext_faces[i_proc]
                );
            }
            fprintf(out, "]\n");

            fprintf(out, "\n");
        }

        free(recv_number_of_int_faces);
        free(recv_number_of_ext_faces);
    }
}

/**
 * @brief write node dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_node_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd){

        int *recv_number_of_nodes = NULL;

        if(myid == 0){

            recv_number_of_nodes = (int*)malloc(sizeof(int) * compute_node_count);
        }

        PRF_GATHER_INT(recv_number_of_nodes, &_NODE_DICT(0).number_of_nodes, 1, 0);

        if(out && myid == 0){

            int i_proc;

            fprintf(out, "[nodes]\n");

            fprintf(out, "number_of_nodes = [");
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){

                fprintf
                (
                    out,
                    "%d, ",
                    recv_number_of_nodes[i_proc]
                );
            }
            fprintf(out, "]\n");

            fprintf(out, "\n");
        }

        free(recv_number_of_nodes);
    }
}

/**
 * @brief write dump dict settings to given FILE or stdout
 * @param rcfd  pointer to rCFD instance
 * @param out   pointer to output FILE
 */
void write_dump_dict_toml(rCFD_type *rcfd, FILE *out)
{
    if(rcfd && out && myid == 0){

        int i_phase, i_data, i_user, i_probe;

        fprintf(out, "[dump]\n");
        WRITE_DICT_PROPERTY_I(out, Dump_Dict, dump_interval);
        WRITE_DICT_PROPERTY_I(out, Dump_Dict, overwrite_interval);
        WRITE_DICT_PROPERTY_I(out, Dump_Dict, dump_file_type);
        WRITE_DICT_PROPERTY_B(out, Dump_Dict, multifile_output_on);
        WRITE_DICT_PROPERTY_B(out, Dump_Dict, dump_vof);
        WRITE_DICT_PROPERTY_I(out, Dump_Dict, probe_interval);

        if(_CELL_DICT(0).number_of_user_vars > 0){

            fprintf(out, "dump_user_data = [");
            for(i_user = 0; i_user < _CELL_DICT(0).number_of_user_vars; ++i_user){

                fprintf
                (
                    out,
                    "%s, ",
                    rcfd->Dump_Dict->dump_user_data[i_user] ? "true" : "false"
                );
            }
            fprintf(out, "]\n");
        }

        if(_CELL_DICT(0).number_of_rec_user_vars > 0){

            fprintf(out, "dump_rec_user_data = [");
            for(i_user = 0; i_user < _CELL_DICT(0).number_of_rec_user_vars; ++i_user){

                fprintf
                (
                    out,
                    "%s, ",
                    rcfd->Dump_Dict->dump_rec_user_data[i_user] ? "true" : "false"
                );
            }
            fprintf(out, "]\n");
        }
        for(i_probe = 0; i_probe < rcfd->Dump_Dict->probe_count; ++i_probe){

            if(    rcfd->Dump_Dict->probe_type[i_probe] != RCFD_PROBE_USER
                && rcfd->Dump_Dict->probe_type[i_probe] != RCFD_PROBE_REC_USER)
                continue;

            fprintf(out, "[[dump.probes]]\n");
            switch(rcfd->Dump_Dict->probe_type[i_probe]){

            case RCFD_PROBE_USER:
                fprintf(out, "probe_type = \"user\"\n");
                break;
            case RCFD_PROBE_VOF:
                fprintf(out, "probe_type = \"rec_user\"\n");
                break;
            default:
                break;
            }

            fprintf
            (
                out,
                "probe_data = %d\n"
                "probe_position = [%g, %g, %g]\n",
                rcfd->Dump_Dict->probe_data[i_probe],
                rcfd->Dump_Dict->probe_x[i_probe][0],
                rcfd->Dump_Dict->probe_x[i_probe][1],
                rcfd->Dump_Dict->probe_x[i_probe][2]
            );
        }

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

            fprintf(out, "[[dump.phases]]\n");
            fprintf(out, "dump_cell_data = [");
            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; ++i_data){

                fprintf
                (
                    out,
                    "%s, ",
                    rcfd->Dump_Dict->dump_cell_data[i_phase][i_data] ? "true" : "false"
                );
            }
            fprintf(out, "]\n");

            if(rcfd->Dump_Dict->probe_count > 0){

                for(i_probe = 0; i_probe < rcfd->Dump_Dict->probe_count; ++i_probe){

                    if(    rcfd->Dump_Dict->probe_type[i_probe] == RCFD_PROBE_USER
                        || rcfd->Dump_Dict->probe_type[i_probe] == RCFD_PROBE_REC_USER)
                        continue;

                    if(rcfd->Dump_Dict->probe_phase[i_probe] != i_phase)
                        continue;

                    fprintf(out, "[[dump.phases.probes]]\n");

                    switch(rcfd->Dump_Dict->probe_type[i_probe]){

                    case RCFD_PROBE_CELL:
                        fprintf(out, "probe_type = \"cell\"\n");
                        fprintf(out, "probe_data = %d\n",  rcfd->Dump_Dict->probe_data[i_probe]);
                        break;
                    case RCFD_PROBE_VOF:
                        fprintf(out, "probe_type = \"vof\"\n");
                        break;
                    default:
                        break;
                    }

                    fprintf
                    (
                        out,
                        "probe_position = [%g, %g, %g]\n",
                        rcfd->Dump_Dict->probe_x[i_probe][0],
                        rcfd->Dump_Dict->probe_x[i_probe][1],
                        rcfd->Dump_Dict->probe_x[i_probe][2]
                    );
                }
            }
        }

        fprintf(out, "\n");
    }
}
#endif

/**
 * @brief write current config_toml
 * @param rcfd  pointer to rCFD instance
 * @param filename
 * @return
 */
int write_config_toml(rCFD_type *rcfd, const char *filename)
{
#if RP_NODE
    FILE *out = NULL;
    int file_is_open = 0;
    int mode = rcfd->Solver_Dict->mode;

    if(myid == 0){

        if(filename){

            out = fopen(filename, "wt");
        }
        else{

            out = stdout;
        }

        if(out){

            time_t  current_time = time(NULL);
            char    *c_time_string = ctime(&current_time);

            file_is_open = 1;
            fprintf(out, "# %s\n", RCFD_VERSION_STRING);
            fprintf(out, "# auto generated by rCFD_%s at %s\n",
                    (mode == RCFD_MODE_RUN) ? "run" : "prep", c_time_string);
        }
    }

    file_is_open = PRF_GISUM1(file_is_open);

    if(!file_is_open)
        return -1;

    write_file_dict_toml(rcfd, out);
    write_solver_dict_toml(rcfd, out);
    write_phase_dict_toml(rcfd, out);
    write_prep_dict_toml(rcfd, out);
    write_norm_dict_toml(rcfd, out);
    write_rec_dict_toml(rcfd, out);
    write_data_dict_toml(rcfd, out);
    write_balance_dict_toml(rcfd, out);
    write_cell_dict_toml(rcfd, out);
    write_face_dict_toml(rcfd, out);
    write_node_dict_toml(rcfd, out);
    write_dump_dict_toml(rcfd, out);

    if(out){

        fclose(out);
    }

#endif

    return  0;
}

#endif // RCFD_DICT_WRITE_TOML_H
