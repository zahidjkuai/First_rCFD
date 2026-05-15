/******************************************************************************
 * @internal
 * @file        rCFD_dict_read_toml.h
 * @copyright   (C) 2025
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_DICT_READ_TOML_H
#define RCFD_DICT_READ_TOML_H

#include "tomlc17.h"

static const char* file_dict_property_s[] =
{
    "working_dir",
    "data_dir",
    "tmp_sub_dir",
    "vof_sub_dir",
    "topo_sub_dir",
    "local_diff_sub_dir",
    "c2c_sub_dir",
    "rec_sub_dir",
    "user_sub_dir",
    "post_dir",
    "metadata_filename",
    "tracer_start_position_filename",
    "Prep_Transscript_filename",
    "Run_Transscript_filename",
    "C2C_filename",
    "Norm_filename",
    "vof_filename",
    "local_diff_filename",
    "rec_user_filename",
    "Jump_filename",
    "Matrix_filename",
    "Balance_filename",
    "Rec_Frames_filename",
    "Topo_filename",
    "Dump_filename",
    "Probe_filename"
};

static const char* solver_dict_property_i_or_s[] =
{
    "report_priority",
    "local_diff_format"
};

static const char* solver_dict_property_i[] =
{
    "profiling_on",
    "multifile_output_on",
    "write_config_to_file_on",
    "number_of_frames",
    "number_of_states",
    "number_of_phases",
    "number_of_layers",
    "number_of_runs",
    "recurrence_process_on",
    "data_convection_on",
    "face_swap_diffusion_on",
    "local_diff_on",
    "parallel_diffusion_on",
    "data_binarization_on",
    "data_drifting_on",
    "balance_correction_on",
    "interpolation_on",
    "shifts_from_displacement_fields_on",
    "moving_geometry_on",
    "grid_interface_shifts_on",
    "max_number_of_cells_per_time_step",
    "time_steps_per_monitoring_interval",
    "single_phase_vof",
    "single_phase_vof_id",
    "min_number_of_cells_per_layer",
    "max_number_of_faces_per_cell",
    "max_number_of_children",
    "number_redist_loops_per_layer",
    "max_fill_loops",
    "face_swap_max_loops",
    "number_of_drift_loops",
    "balance_correction_update",
    "control_conc_sum_on"
};

static const char* solver_dict_property_r[] =
{
    "start_time_for_monitoring",
    "vof_cutoff_limit",
    "global_time_step",
    "face_swap_max_per_loop",
    "grid_interface_shift_weight"
};

#if RP_NODE
static const char* phase_dict_property_i[] =
{
    "number_of_data",
    "hindered_drift_on",
    "local_drift_on",
    "drift_control_conc_sum_on",
    "number_of_user_vars"
};

static const char* phase_dict_property_r[] =
{
    "time_step",
    "density",
    "heat_capacity",
    "reference_temperature",
    "vof_max",
    "d32_diam"
};

static const char* prep_dict_property_i_or_s[] =
{
    "tracer_format",
    "interpolation_format"
};

static const char* prep_dict_property_i[] =
{
    "tracers_per_cell_count",
    "tracer_coarse_graining",
    "tracer_reverse_direction_on",
    "tracer_splitting_on",
    "tracer_region_of_interest_exists",
    //"tracer_random_walk_on", // int array, special treatment
    "interpolation_max_number_of_aligned_neigh_cells",
    "interpolation_pruning_on",
    "interpolation_min_shifts_after_pruning",
    "interpolation_multiply_shift_weights_with_vof_on"
};

static const char* prep_dict_property_r[] =
{
    "tracer_ROI_x_min",
    "tracer_ROI_x_max",
    "tracer_ROI_y_min",
    "tracer_ROI_y_max",
    "tracer_ROI_z_min",
    "tracer_ROI_z_max",
    //"tracer_random_walk_velocity_ratio" // real array, special treatment
    "interpolation_pruning_weight"
};

static const char* norm_dict_property_i_or_s[] =
{
    "format"
};

static const char* norm_dict_property_i[] =
{
    "coarse_graining",
    "larger_than_mean_norm_only"
};
#endif

static const char* rec_dict_property_i_or_s[] =
{
    "format"
};

static const char* rec_dict_property_i[] =
{
    "monitor_rec_frames_on",
    "adapt_vof_stitching_on",
    "min_seq_length",
    "max_seq_length",
    "off_diagonal_band_width",
    "number_of_adapt_vof_loops",
    "seed"
};

#if RP_NODE
static const char* data_dict_property_i_or_s[] =
{
    "type"
};

static const char* data_dict_property_i[] =
{
    "drifting_data",
    "number_of_user_vars"
};

static const char* data_dict_property_r[] =
{
    "face_swap_diffusion",
    "binarization_art_diff",
    "d1_diam"
};

static const char* data_dict_property_r_3d[] =
{
    "drift_velocity"
};

static const char* balance_dict_property_i_or_s[] =
{
    "type"
};

static const char* balance_dict_property_i[] =
{
    "max_correction_loops",
    "write_balance_to_file",
    "write_balance_to_file_interval"
};

static const char* balance_dict_property_r[] =
{
    "accuracy_level"
};

static const char* cell_dict_property_i[] =
{
    "number_of_user_vars",
    "number_of_rec_user_vars"
};

static const char* cell_dict_property_i_part[] =
{
    // arrays with values for each compute node
    "number_of_int_cells",
    "number_of_ext_cells"
};

static const char* cell_dict_property_r[] =
{
    "next_neigh_cell_skin_factor"
};

static const char* face_dict_property_i_part[] =
{
    // arrays with values for each compute node
    "number_of_int_faces",
    "number_of_ext_faces"
};

static const char* node_dict_property_i_part[] =
{
    // arrays with values for each compute node
    "number_of_nodes"
};

static const char* dump_dict_property_i[] =
{
    "dump_interval",
    "overwrite_interval",
    "dump_file_type",
    "multifile_output_on",
    "dump_vof",
    "probe_interval"
};

static const char* dump_dict_property_i_arr[] =
{
    // arrays with on/off values for each data
    "dump_user_data",
    "dump_rec_user_data"
};

// NOTE: special treatment for dump_cell_data flags and dump dict probes,
// cf. read_dump_dict_toml()
#endif

/**
 * @brief parse and store complete toml config file
 * @param rcfd  pointer to rCFD instance
 * @param filename name of tom config file
 * @retval -1 Error
 * @retval 0 OK
 */
int read_config_toml(rCFD_type *rcfd, const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if(!fp){

        report_info0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                     "Failed to open config file: %s", filename);
        return -1;
    }

    toml_result_t result = toml_parse_file(fp);
    fclose(fp);

    if(!result.ok){

        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "Failed to parse config file: %s", result.errmsg);
        return -1;
    }

    rcfd->config = malloc(sizeof(result));
    *(toml_result_t*)rcfd->config = result;

    return 0;
}

/**
 * @brief free toml config structure
 * @param rcfd  pointer to rCFD instance
 */
void free_config_toml(rCFD_type *rcfd)
{
    if(rcfd && rcfd->config){

        toml_free(*(toml_result_t*)rcfd->config);
        free(rcfd->config);
        rcfd->config = NULL;
    }
}

/**
 * @brief read file dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_file_dict_toml(rCFD_type *rcfd)
{
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "files";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t files = toml_get(config->toptab, dict);

    if(files.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict %s in config file",
                      dict,
                      (files.type == TOML_UNKNOWN)
                      ? "does not exist"
                      : "is not a table");
        return;
    }

    int property_count = sizeof(file_dict_property_s)/sizeof(file_dict_property_s[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(files, file_dict_property_s[i_property]);

        if(property.type == TOML_STRING){

            if(set_file_dict_property_s(rcfd, file_dict_property_s[i_property], property.u.str.ptr) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set string property '%s'"
                              " in %s dict from config file",
                              solver_dict_property_i_or_s[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist or is not a string in config file",
                          dict, file_dict_property_s[i_property]);
        }
    }
}

/**
 * @brief read solver dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_solver_dict_toml(rCFD_type *rcfd)
{
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "solver";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t solver = toml_get(config->toptab, dict);

    if(solver.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist or is not a table in config file",
                      dict);
        return;
    }

    // integer properties
    int property_count = sizeof(solver_dict_property_i)/sizeof(solver_dict_property_i[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(solver, solver_dict_property_i[i_property]);

        if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

            if(set_solver_dict_property_i(rcfd, solver_dict_property_i[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              solver_dict_property_i[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Config file: '%s' does not exist in %s dict"
                          " or is neither an integer nor a boolean",
                          solver_dict_property_i[i_property], dict);
        }
    }

    // integer or string properties
    property_count = sizeof(solver_dict_property_i_or_s)/sizeof(solver_dict_property_i_or_s[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(solver, solver_dict_property_i_or_s[i_property]);

        if(property.type == TOML_INT64){

            if(set_solver_dict_property_i(rcfd, solver_dict_property_i_or_s[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              solver_dict_property_i_or_s[i_property], dict);
            }
        }
        else if(property.type == TOML_STRING){

            if(set_solver_dict_property_s(rcfd, solver_dict_property_i_or_s[i_property], property.u.str.ptr) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set string property '%s'"
                              " in %s dict from config file",
                              solver_dict_property_i_or_s[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is neither an integer nor a string in config file",
                          dict, solver_dict_property_i_or_s[i_property]);
        }
    }

    // real properties
    property_count = sizeof(solver_dict_property_r)/sizeof(solver_dict_property_r[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(solver, solver_dict_property_r[i_property]);

        if(property.type == TOML_INT64){

            if(set_solver_dict_property_r(rcfd, solver_dict_property_r[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set real property '%s'"
                              " in %s dict from config file",
                              solver_dict_property_r[i_property], dict);
            }
        }
        else if(property.type == TOML_FP64){

            if(set_solver_dict_property_r(rcfd, solver_dict_property_r[i_property], property.u.fp64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set real property '%s'"
                              " in %s dict from config file",
                              solver_dict_property_r[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is not a real in config file",
                          dict, solver_dict_property_r[i_property]);
        }
    }
}

/**
 * @brief read phase dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_phase_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "phases";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t phases = toml_get(config->toptab, dict);

    if(phases.type != TOML_ARRAY){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist"
                      " or is not a (table) array in config file",
                      dict);
        return;
    }

    int phase_count = rcfd->Solver_Dict->number_of_phases;
    int phase_count_toml = phases.u.arr.size;

    if(phase_count > phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "Only %d of %d phases in %s (table) array"
                        " are defined in config file",
                        phase_count_toml, phase_count, dict);
        phase_count = phase_count_toml;
    }
    else if(phase_count < phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "More than %d phases are defined in %s (table) array in config file"
                        " - ignoring %d phase definitions",
                        phase_count, dict, phase_count_toml - phase_count);
    }

    for(int i_phase = 0; i_phase < phase_count; ++i_phase){

        toml_datum_t phase = phases.u.arr.elem[i_phase];

        if(phase.type != TOML_TABLE){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Element %d of '%s' is not a table in config file",
                          i_phase, dict);
            continue;
        }

        // integer properties
        int property_count = sizeof(phase_dict_property_i)/sizeof(phase_dict_property_i[0]);

        for(int i_property = 0; i_property < property_count; ++i_property){

            toml_datum_t property = toml_get(phase, phase_dict_property_i[i_property]);

            if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

                if(set_phase_dict_property_i(rcfd, i_phase, phase_dict_property_i[i_property], property.u.int64) != 0){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Failed to set integer property '%s'"
                                  " in %s dict from config file",
                                  phase_dict_property_i[i_property], dict);
                }
            }
            else{

                report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Config file: '%s' does not exist in %s dict"
                              " or is neither an integer nor a boolean",
                              phase_dict_property_i[i_property], dict);
            }
        }

        // real properties
        property_count = sizeof(phase_dict_property_r)/sizeof(phase_dict_property_r[0]);

        for(int i_property = 0; i_property < property_count; ++i_property){

            toml_datum_t property = toml_get(phase, phase_dict_property_r[i_property]);

            if(property.type == TOML_INT64){

                if(set_phase_dict_property_r(rcfd, i_phase, phase_dict_property_r[i_property], property.u.int64) != 0){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Failed to set real property '%s'"
                                  " in %s dict from config file",
                                  phase_dict_property_r[i_property], dict);
                }
            }
            else if(property.type == TOML_FP64){

                if(set_phase_dict_property_r(rcfd, i_phase, phase_dict_property_r[i_property], property.u.fp64) != 0){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Failed to set real property '%s'"
                                  " in %s dict from config file",
                                  phase_dict_property_r[i_property], dict);
                }
            }
            else{

                report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "'%s.%s' does not exist or is not a real in config file",
                              dict, phase_dict_property_r[i_property]);
            }
        }
    }
#endif
}

/**
 * @brief read prep dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_prep_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "prep";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t prep = toml_get(config->toptab, dict);

    if(prep.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist or is not a table in config file",
                      dict);
        return;
    }

    // integer properties
    int property_count = sizeof(prep_dict_property_i)/sizeof(prep_dict_property_i[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(prep, prep_dict_property_i[i_property]);

        if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

            if(set_prep_dict_property_i(rcfd, -1, prep_dict_property_i[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              prep_dict_property_i[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Config file: '%s' does not exist in %s dict"
                          " or is neither an integer nor a boolean",
                          prep_dict_property_i[i_property], dict);
        }
    }

    // integer or string properties
    property_count = sizeof(prep_dict_property_i_or_s)/sizeof(prep_dict_property_i_or_s[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(prep, prep_dict_property_i_or_s[i_property]);

        if(property.type == TOML_INT64){

            if(set_prep_dict_property_i(rcfd, -1, prep_dict_property_i_or_s[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              prep_dict_property_i_or_s[i_property], dict);
            }
        }
        else if(property.type == TOML_STRING){

            if(set_prep_dict_property_s(rcfd, prep_dict_property_i_or_s[i_property], property.u.str.ptr) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set string property '%s'"
                              " in %s dict from config file",
                              prep_dict_property_i_or_s[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is neither an integer nor a string in config file",
                          dict, prep_dict_property_i_or_s[i_property]);
        }
    }

    // real properties
    property_count = sizeof(prep_dict_property_r)/sizeof(prep_dict_property_r[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(prep, prep_dict_property_r[i_property]);

        if(property.type == TOML_INT64){

            if(set_prep_dict_property_r(rcfd, -1, prep_dict_property_r[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set real property '%s'"
                              " in %s dict from config file",
                              prep_dict_property_r[i_property], dict);
            }
        }
        else if(property.type == TOML_FP64){

            if(set_prep_dict_property_r(rcfd, -1, prep_dict_property_r[i_property], property.u.fp64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set real property '%s'"
                              " in %s dict from config file",
                              prep_dict_property_r[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist or is not a real in config file",
                          dict, prep_dict_property_r[i_property]);
        }
    }

    // per phase integer/real properties
    if(rcfd->Prep_Dict->tracer_random_walk_on){

        toml_datum_t property = toml_get(prep, "tracer_random_walk_on");

        if(property.type != TOML_ARRAY){

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "%s.tracer_random_walk_on does not exist"
                          " or is not an array in config file",
                          dict);
        }
        else{

            int phase_count = rcfd->Solver_Dict->number_of_phases;
            int phase_count_toml = property.u.arr.size;

            if(phase_count > phase_count_toml){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                "Only %d values for %d phases are defined"
                                " for %s.random_walk in config file",
                                phase_count_toml, phase_count, dict);
                phase_count = phase_count_toml;
            }
            else if(phase_count < phase_count_toml){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                "More values than phases are defined"
                                " for %s.random_walk in config file"
                                " - ignoring %d elements",
                                dict, phase_count_toml - phase_count, dict);
            }

            for(int i_phase = 0; i_phase < phase_count; ++i_phase){

                toml_datum_t phase_property = property.u.arr.elem[i_phase];

                if(phase_property.type == TOML_INT64 || phase_property.type == TOML_BOOLEAN){

                    if(set_prep_dict_property_i(rcfd, i_phase, "tracer_random_walk_on", phase_property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set integer property 'tracer_random_walk_on'"
                                      " in %s dict for phase %d from config file",
                                      dict, i_phase);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "'%s.tracer_random_walk_on[%d]' is not an integer/boolean in config file",
                                  dict, i_phase);
                }
            }
        }
    }

    if(rcfd->Prep_Dict->tracer_random_walk_velocity_ratio){

        toml_datum_t property = toml_get(prep, "tracer_random_walk_velocity_ratio");

        if(property.type != TOML_ARRAY){

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "%s.tracer_random_walk_velocity_ratio does not exist"
                          " or is not an array in config file",
                          dict);
        }
        else{

            int phase_count = rcfd->Solver_Dict->number_of_phases;
            int phase_count_toml = property.u.arr.size;

            if(phase_count > phase_count_toml){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                "Only %d values for %d phases are defined"
                                " for %s.tracer_random_walk_velocity_ratio in config file",
                                phase_count_toml, phase_count, dict);
                phase_count = phase_count_toml;
            }
            else if(phase_count < phase_count_toml){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                "More values than phases are defined"
                                " for %s.tracer_random_walk_velocity_ratio in config file"
                                " - ignoring %d elements",
                                dict, phase_count_toml - phase_count, dict);
            }

            for(int i_phase = 0; i_phase < phase_count; ++i_phase){

                toml_datum_t phase_property = property.u.arr.elem[i_phase];

                if(phase_property.type == TOML_INT64){

                    if(set_prep_dict_property_r(rcfd, i_phase, "tracer_random_walk_velocity_ratio", phase_property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set real property 'tracer_random_walk_velocity_ratio'"
                                      " in %s dict for phase %d from config file",
                                      dict, i_phase);
                    }
                }
                else if(phase_property.type == TOML_FP64){

                    if(set_prep_dict_property_r(rcfd, i_phase, "tracer_random_walk_velocity_ratio", phase_property.u.fp64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set real property 'tracer_random_walk_velocity_ratio'"
                                      " in %s dict for phase %d from config file",
                                      dict, i_phase);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "'%s.tracer_random_walk_velocity_ratio[%d]'"
                                  " is not a real in config file",
                                  dict, i_phase);
                }
            }
        }
    }
#endif
}

/**
 * @brief read norm dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_norm_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "norm";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t norm = toml_get(config->toptab, dict);

    if(norm.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist"
                      " or is not a table in config file",
                      dict);
        return;
    }

    // integer properties
    int property_count = sizeof(norm_dict_property_i)/sizeof(norm_dict_property_i[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(norm, norm_dict_property_i[i_property]);

        if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

            if(set_norm_dict_property_i(rcfd, norm_dict_property_i[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              norm_dict_property_i[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Config file: '%s' does not exist in %s dict"
                          " or is neither an integer nor a boolean",
                          norm_dict_property_i[i_property], dict);
        }
    }

    // integer or string properties
    property_count = sizeof(norm_dict_property_i_or_s)/sizeof(norm_dict_property_i_or_s[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(norm, norm_dict_property_i_or_s[i_property]);

        if(property.type == TOML_INT64){

            if(set_norm_dict_property_i(rcfd, norm_dict_property_i_or_s[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              norm_dict_property_i_or_s[i_property], dict);
            }
        }
        else if(property.type == TOML_STRING){

            if(set_norm_dict_property_s(rcfd, norm_dict_property_i_or_s[i_property], property.u.str.ptr) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set string property '%s'"
                              " in %s dict from config file",
                              norm_dict_property_i_or_s[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is neither an integer nor a string in config file",
                          dict, norm_dict_property_i_or_s[i_property]);
        }
    }
#endif
}

/**
 * @brief read rec dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_rec_dict_toml(rCFD_type *rcfd)
{
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "rec";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t rec = toml_get(config->toptab, dict);

    if(rec.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist"
                      " or is not a table in config file",
                      dict);
        return;
    }

    // integer properties
    int property_count = sizeof(rec_dict_property_i)/sizeof(rec_dict_property_i[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(rec, rec_dict_property_i[i_property]);

        if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

            if(set_rec_dict_property_i(rcfd, rec_dict_property_i[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              rec_dict_property_i[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is neither an integer nor a boolean",
                          dict, rec_dict_property_i[i_property]);
        }
    }

    // integer or string properties
    property_count = sizeof(rec_dict_property_i_or_s)/sizeof(rec_dict_property_i_or_s[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(rec, rec_dict_property_i_or_s[i_property]);

        if(property.type == TOML_INT64){

            if(set_rec_dict_property_i(rcfd, rec_dict_property_i_or_s[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              rec_dict_property_i_or_s[i_property], dict);
            }
        }
        else if(property.type == TOML_STRING){

            if(set_rec_dict_property_s(rcfd, rec_dict_property_i_or_s[i_property], property.u.str.ptr) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set string property '%s'"
                              " in %s dict from config file",
                              rec_dict_property_i_or_s[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is neither an integer nor a string in config file",
                          dict, rec_dict_property_i_or_s[i_property]);
        }
    }
}

/**
 * @brief read data dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_data_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "data";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t data = toml_get(config->toptab, dict);

    if(data.type != TOML_ARRAY){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist or is not a (table) array in config file",
                      dict);
        return;
    }

    int phase_count = rcfd->Solver_Dict->number_of_phases;
    int phase_count_toml = data.u.arr.size;

    if(phase_count > phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "Only %d of %d phases are defined"
                        " for %s (table) array in config file",
                        phase_count_toml, phase_count, dict);
        phase_count = phase_count_toml;
    }
    else if(phase_count < phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "More than %d phases are defined"
                        " for %s (table) array in config file"
                        " - ignoring %d phase definitions",
                        phase_count, dict, phase_count_toml - phase_count);
    }

    for(int i_phase = 0; i_phase < phase_count; ++i_phase){

        toml_datum_t phase_data = toml_get(data.u.arr.elem[i_phase], "items");

        if(phase_data.type != TOML_ARRAY){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Element %d of '%s' is not a (table) array in config file",
                          i_phase, dict);
            continue;
        }

        int data_count = rcfd->Phase_Dict[i_phase].number_of_data;
        int data_count_toml = phase_data.u.arr.size;

        if(data_count > data_count_toml){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "Only %d of %d %s items are defined"
                            " for phase %d in config file",
                            data_count_toml, data_count, dict, i_phase);
            data_count = data_count_toml;
        }
        else if(data_count < data_count_toml){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "More than %d %s items are defined"
                            " for phase %d in config file"
                            " - ignoring %d item definition(s)",
                            data_count, dict, i_phase, data_count_toml - data_count);
        }

        for(int i_data = 0; i_data < data_count; ++i_data){

            toml_datum_t phase_data_item = phase_data.u.arr.elem[i_data];

            if(phase_data_item.type != TOML_TABLE){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Element %d of '%s.items' for phase %d"
                              " is not a table in config file",
                              i_data, dict, i_phase);
                continue;
            }

            // integer properties
            int property_count = sizeof(data_dict_property_i)/sizeof(data_dict_property_i[0]);

            for(int i_property = 0; i_property < property_count; ++i_property){

                toml_datum_t property = toml_get(phase_data_item, data_dict_property_i[i_property]);

                if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

                    if(set_data_dict_property_i(rcfd, i_phase, i_data, data_dict_property_i[i_property], property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set integer property '%s'"
                                      " in %s dict for phase %d, item %d from config file",
                                      data_dict_property_i[i_property], dict, i_phase, i_data);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Config file: '%s' does not exist"
                                  " in %s dict for phase %d, item %d"
                                  " or is neither an integer nor a boolean",
                                  data_dict_property_i[i_property], dict, i_phase, i_data);
                }
            }

            // integer or string properties
            property_count = sizeof(data_dict_property_i_or_s)/sizeof(data_dict_property_i_or_s[0]);

            for(int i_property = 0; i_property < property_count; ++i_property){

                toml_datum_t property = toml_get(phase_data_item, data_dict_property_i_or_s[i_property]);

                if(property.type == TOML_INT64){

                    if(set_data_dict_property_i(rcfd, i_phase, i_data, data_dict_property_i_or_s[i_property], property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set integer property '%s'"
                                      " in %s dict for phase %d, item %d from config file",
                                      data_dict_property_i_or_s[i_property], dict, i_phase, i_data);
                    }
                }
                else if(property.type == TOML_STRING){

                    if(set_data_dict_property_s(rcfd, i_phase, i_data, data_dict_property_i_or_s[i_property], property.u.str.ptr) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set string property '%s'"
                                      " in %s dict for phase %d, item %d from config file",
                                      data_dict_property_i_or_s[i_property], dict, i_phase, i_data);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "'%s.items.%s' does not exist"
                                  " for phase %d, item %d"
                                  " or is neither an integer nor a string in config file",
                                  dict, data_dict_property_i_or_s[i_property], i_phase, i_data);
                }
            }

            // real properties
            property_count = sizeof(data_dict_property_r)/sizeof(data_dict_property_r[0]);

            for(int i_property = 0; i_property < property_count; ++i_property){

                toml_datum_t property = toml_get(phase_data_item, data_dict_property_r[i_property]);

                if(property.type == TOML_INT64){

                    if(set_data_dict_property_r(rcfd, i_phase, i_data, data_dict_property_r[i_property], property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set real property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      data_dict_property_r[i_property], dict, i_phase, i_data);
                    }
                }
                else if(property.type == TOML_FP64){

                    if(set_data_dict_property_r(rcfd, i_phase, i_data, data_dict_property_r[i_property], property.u.fp64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set real property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      data_dict_property_r[i_property], dict, i_phase, i_data);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "'%s.items.%s' does not exist"
                                  " for phase %d, item %d"
                                  " or is not a real in config file",
                                  dict, data_dict_property_r[i_property], i_phase, i_data);
                }
            }

            // real vector properties
            property_count = sizeof(data_dict_property_r_3d)/sizeof(data_dict_property_r_3d[0]);

            for(int i_property = 0; i_property < property_count; ++i_property){

                toml_datum_t property = toml_get(phase_data_item, data_dict_property_r_3d[i_property]);

                if(property.type == TOML_ARRAY && property.u.arr.size == 3){

                    int i_dim;
                    double vector[3] = {0};

                    for(i_dim = 0; i_dim < 3; ++i_dim){

                        toml_datum_t x = property.u.arr.elem[i_dim];
                        if(x.type == TOML_INT64){

                            vector[i_dim] = x.u.int64;
                        }
                        else if(x.type == TOML_FP64){

                            vector[i_dim] = x.u.fp64;
                        }
                        else{

                            report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                          "%s of element %d of '%s.items'"
                                          " has invalid component type in config file",
                                          data_dict_property_r_3d[i_property], i_property, dict);
                            break;
                        }
                    }

                    if(i_dim != 3)
                        continue;

                    if(set_data_dict_property_r_3d(rcfd, i_phase, i_data, data_dict_property_r_3d[i_property], vector) != 0){
                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set vector property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      data_dict_property_r_3d[i_property], dict, i_phase, i_data);

                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "'%s.items.%s' does not exist"
                                  " for phase %d, item %d"
                                  " or is not a 3D array in config file",
                                  dict, data_dict_property_r_3d[i_property], i_phase, i_data);
                }
            }
        }
    }
#endif
}

/**
 * @brief read balance dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_balance_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "balances";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t balances = toml_get(config->toptab, dict);

    if(balances.type != TOML_ARRAY){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist"
                      " or is not a (table) array in config file",
                      dict);
        return;
    }

    int phase_count = rcfd->Solver_Dict->number_of_phases;
    int phase_count_toml = balances.u.arr.size;

    // check if table array only consists of any empty table
    if(phase_count_toml == 1){

        toml_datum_t table0 = balances.u.arr.elem[0];

        if(table0.u.tab.size == 0){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s' dict is an empty (table) array",
                            dict);
            return;
        }
    }

    if(phase_count > phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "Only %d of %d phases are defined"
                        " for %s (table) array in config file",
                        phase_count_toml, phase_count, dict);
        phase_count = phase_count_toml;
    }
    else if(phase_count < phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "More than %d phases are defined"
                        " for %s (table) array in config file"
                        " - ignoring %d phase definitions",
                        phase_count, dict, phase_count_toml - phase_count);
    }

    for(int i_phase = 0; i_phase < phase_count; ++i_phase){

        toml_datum_t phase_balances = toml_get(balances.u.arr.elem[i_phase], "items");

        if(phase_balances.type != TOML_ARRAY){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Element %d of '%s' is not a (table) array in config file",
                          i_phase, dict);
            continue;
        }

        int data_count = rcfd->Phase_Dict[i_phase].number_of_data;
        int data_count_toml = phase_balances.u.arr.size;

        if(data_count > data_count_toml){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "Only %d of %d %s items are defined"
                            " for phase %d in config file",
                            data_count_toml, data_count, dict, i_phase);
            data_count = data_count_toml;
        }
        else if(data_count < data_count_toml){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "More than %d %s items are defined"
                            " for phase %d in config file"
                            " - ignoring %d item definition(s)",
                            data_count, dict, i_phase, data_count_toml - data_count);
        }

        for(int i_data = 0; i_data < data_count; ++i_data){

            toml_datum_t phase_balances_item = phase_balances.u.arr.elem[i_data];

            if(phase_balances_item.type != TOML_TABLE){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Element %d of '%s.items' for phase %d"
                              " is not a table in config file",
                              i_data, dict, i_phase);
                continue;
            }

            // integer properties
            int property_count = sizeof(balance_dict_property_i)/sizeof(balance_dict_property_i[0]);

            for(int i_property = 0; i_property < property_count; ++i_property){

                toml_datum_t property = toml_get(phase_balances_item, balance_dict_property_i[i_property]);

                if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

                    if(set_balance_dict_property_i(rcfd, i_phase, i_data, balance_dict_property_i[i_property], property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set integer property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      balance_dict_property_i[i_property], dict, i_phase, i_data);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Config file: '%s' does not exist"
                                  " in %s dict for phase %d, item %d"
                                  " or is neither an integer nor a boolean",
                                  balance_dict_property_i[i_property], dict, i_phase, i_data);
                }
            }

            // integer or string properties
            property_count = sizeof(balance_dict_property_i_or_s)/sizeof(balance_dict_property_i_or_s[0]);

            for(int i_property = 0; i_property < property_count; ++i_property){

                toml_datum_t property = toml_get(phase_balances_item, balance_dict_property_i_or_s[i_property]);

                if(property.type == TOML_INT64){

                    if(set_balance_dict_property_i(rcfd, i_phase, i_data, balance_dict_property_i_or_s[i_property], property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set integer property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      balance_dict_property_i_or_s[i_property], dict, i_phase, i_data);
                    }
                }
                else if(property.type == TOML_STRING){

                    if(set_balance_dict_property_s(rcfd, i_phase, i_data, balance_dict_property_i_or_s[i_property], property.u.str.ptr) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set string property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      balance_dict_property_i_or_s[i_property], dict, i_phase, i_data);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "'%s.items.%s' does not exist"
                                  " for phase %d, item %d"
                                  " or is neither an integer nor a string"
                                  " in config file",
                                  dict, balance_dict_property_i_or_s[i_property], i_phase, i_data);
                }
            }

            // real properties
            property_count = sizeof(balance_dict_property_r)/sizeof(balance_dict_property_r[0]);

            for(int i_property = 0; i_property < property_count; ++i_property){

                toml_datum_t property = toml_get(phase_balances_item, balance_dict_property_r[i_property]);

                if(property.type == TOML_INT64){

                    if(set_balance_dict_property_r(rcfd, i_phase, i_data, balance_dict_property_r[i_property], property.u.int64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set real property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      balance_dict_property_r[i_property], dict, i_phase, i_data);
                    }
                }
                else if(property.type == TOML_FP64){

                    if(set_balance_dict_property_r(rcfd, i_phase, i_data, balance_dict_property_r[i_property], property.u.fp64) != 0){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Failed to set real property '%s'"
                                      " in %s dict for phase %d, item %d"
                                      " from config file",
                                      balance_dict_property_r[i_property], dict, i_phase, i_data);
                    }
                }
                else{

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "'%s.items.%s' does not exist"
                                  " for phase %d, item %d"
                                  " or is not a real in config file",
                                  dict, balance_dict_property_r[i_property], i_phase, i_data);
                }
            }
        }
    }
#endif
}

/**
 * @brief read cell dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_cell_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "cells";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t cells = toml_get(config->toptab, dict);

    if(cells.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist or is not a table in config file",
                      dict);
        return;
    }

    // integer properties
    int property_count = sizeof(cell_dict_property_i)/sizeof(cell_dict_property_i[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(cells, cell_dict_property_i[i_property]);

        if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

            if(set_cell_dict_property_i(rcfd, 0, cell_dict_property_i[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              cell_dict_property_i[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Config file: '%s' does not exist in %s dict"
                          " or is neither an integer nor a boolean",
                          cell_dict_property_i[i_property], dict);
        }
    }

    // integer array properties (per compute node)
    property_count = sizeof(cell_dict_property_i_part)/sizeof(cell_dict_property_i_part[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(cells, cell_dict_property_i_part[i_property]);

        if(property.type != TOML_ARRAY){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.%s' is not an integer array in config file",
                            dict, cell_dict_property_i_part[i_property]);
            continue;
        }

        int compute_node_count_toml = property.u.arr.size;

        if(compute_node_count != compute_node_count_toml){

            report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.%s' must have size %d in config file",
                            dict, cell_dict_property_i_part[i_property], compute_node_count);
            continue;
        }

        toml_datum_t partition_property = property.u.arr.elem[myid];

        if(partition_property.type == TOML_INT64 ){

            if(set_cell_dict_property_i(rcfd, 0, cell_dict_property_i_part[i_property], partition_property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict for node %d from config file",
                              cell_dict_property_i_part[i_property], dict, myid);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Config file: '%s' does not exist in %s dict"
                          " or is not an integer array",
                          cell_dict_property_i_part[i_property], dict);
        }
    }

    // real properties
    property_count = sizeof(cell_dict_property_r)/sizeof(cell_dict_property_r[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(cells, cell_dict_property_r[i_property]);

        if(property.type == TOML_INT64){

            if(set_cell_dict_property_r(rcfd, 0, cell_dict_property_r[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set real property '%s'"
                              " in %s dict from config file",
                              cell_dict_property_r[i_property], dict);
            }
        }
        else if(property.type == TOML_FP64){

            if(set_cell_dict_property_r(rcfd, 0, cell_dict_property_r[i_property], property.u.fp64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set real property '%s'"
                              " in %s dict from config file",
                              cell_dict_property_r[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is not a real in config file",
                          dict, cell_dict_property_r[i_property]);
        }
    }
#endif
}

/**
 * @brief read face dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_face_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "faces";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t faces = toml_get(config->toptab, dict);

    if(faces.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist or is not a table in config file",
                      dict);
        return;
    }

    int property_count = sizeof(face_dict_property_i_part)/sizeof(face_dict_property_i_part[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(faces, face_dict_property_i_part[i_property]);

        if(property.type != TOML_ARRAY){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.%s' is not an integer array in config file",
                            dict, face_dict_property_i_part[i_property]);
            continue;
        }

        int compute_node_count_toml = property.u.arr.size;

        if(compute_node_count != compute_node_count_toml){

            report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.%s' must have size %d in config file",
                            dict, face_dict_property_i_part[i_property], compute_node_count);
            continue;
        }

        toml_datum_t partition_property = property.u.arr.elem[myid];

        if(partition_property.type == TOML_INT64 ){

            if(set_face_dict_property_i(rcfd, 0, face_dict_property_i_part[i_property], partition_property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict for node %d from config file",
                              face_dict_property_i_part[i_property], dict, myid);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Config file: '%s' does not exist in %s dict"
                          " or is not an integer array",
                          face_dict_property_i_part[i_property], dict);
        }
    }
#endif
}

/**
 * @brief read node dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_node_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "nodes";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t nodes = toml_get(config->toptab, dict);

    if(nodes.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist"
                      " or is not a table in config file",
                      dict);
        return;
    }

    int property_count = sizeof(node_dict_property_i_part)/sizeof(node_dict_property_i_part[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(nodes, node_dict_property_i_part[i_property]);

        if(property.type != TOML_ARRAY){

            report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.%s' is not an integer array in config file",
                            dict, node_dict_property_i_part[i_property]);
            continue;
        }

        int compute_node_count_toml = property.u.arr.size;

        if(compute_node_count != compute_node_count_toml){

            report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.%s' must have size %d in config file",
                            dict, node_dict_property_i_part[i_property], compute_node_count);
            continue;
        }

        toml_datum_t partition_property = property.u.arr.elem[myid];

        if(partition_property.type == TOML_INT64 ){

            if(set_node_dict_property_i(rcfd, 0, node_dict_property_i_part[i_property], partition_property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict for node %d from config file",
                              node_dict_property_i_part[i_property], dict, myid);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "Config file: '%s' does not exist in %s dict"
                          " or is not an integer array",
                          node_dict_property_i_part[i_property], dict);
        }
    }
#endif
}

/**
 * @brief read dump dict properties from toml structure into rCFD dictionary
 * @param rcfd  pointer to rCFD instance
 */
void read_dump_dict_toml(rCFD_type *rcfd)
{
#if RP_NODE
    if(!rcfd || !rcfd->config)
        return;

    const char* dict = "dump";
    toml_result_t* config = (toml_result_t*)rcfd->config;
    toml_datum_t dump = toml_get(config->toptab, dict);

    if(dump.type != TOML_TABLE){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s' dict does not exist"
                      " or is not a table in config file",
                      dict);
        return;
    }

    // integer properties
    int property_count = sizeof(dump_dict_property_i)/sizeof(dump_dict_property_i[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(dump, dump_dict_property_i[i_property]);

        if(property.type == TOML_INT64 || property.type == TOML_BOOLEAN){

            if(set_dump_dict_property_i(rcfd, -1, -1, dump_dict_property_i[i_property], property.u.int64) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set integer property '%s'"
                              " in %s dict from config file",
                              dump_dict_property_i[i_property], dict);
            }
        }
        else{

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist"
                          " or is neither an integer nor a boolean",
                          dict, dump_dict_property_i[i_property]);
        }
    }

    // integer array properties (per data item)
    property_count = sizeof(dump_dict_property_i_arr)/sizeof(dump_dict_property_i_arr[0]);

    for(int i_property = 0; i_property < property_count; ++i_property){

        toml_datum_t property = toml_get(dump, dump_dict_property_i_arr[i_property]);

        if(property.type != TOML_ARRAY){

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                          "'%s.%s' does not exist or"
                          " is not an integer/boolean array in config file",
                          dict, dump_dict_property_i_arr[i_property]);
            continue;
        }
        else{

            // read flags for each data item
            for(int i_data = 0; i_data < property.u.arr.size; ++i_data){

                toml_datum_t data_on = property.u.arr.elem[i_data];

                if(set_dump_dict_property_i(rcfd, -1, i_data, dump_dict_property_i_arr[i_property], data_on.u.int64) != 0){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Failed to set integer property '%s'"
                                  " in %s dict for data %d from config file",
                                  dump_dict_property_i_arr[i_property], dict, i_data);

                }
            }
        }
    }

    // phase independent probes: [[dump.probes]]
    toml_datum_t dump_probes = toml_seek(dump, "probes");

    if(dump_probes.type != TOML_ARRAY){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s.probes' does not exist"
                      " or is not a (table) array in config file",
                      dict);
    }
    else
    {
        int probe_count = dump_probes.u.arr.size;

        for(int i_probe = 0; i_probe < probe_count; ++i_probe){

            toml_datum_t dump_probe = dump_probes.u.arr.elem[i_probe];
            if(dump_probe.type != TOML_TABLE){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Element %d of '%s.probes' is not a table in config file",
                              i_probe, dict);
                continue;
            }

            int i_dim;
            double probe_position[3] = {0};

            toml_datum_t property = toml_get(dump_probe, "probe_position");

            if(property.type != TOML_ARRAY || property.u.arr.size != 3){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Probe position of element %d of '%s.probes'"
                              " is not a 3D array in config file",
                              i_probe, dict);
                continue;
            }

            for(i_dim = 0; i_dim < 3; ++i_dim){

                toml_datum_t x = property.u.arr.elem[i_dim];
                if(x.type == TOML_INT64){

                    probe_position[i_dim] = x.u.int64;
                }
                else if(x.type == TOML_FP64){

                    probe_position[i_dim] = x.u.fp64;
                }
                else{

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Probe position of element %d of '%s.probes'"
                                  " has invalid component type in config file",
                                  i_probe, dict);
                    break;
                }
            }

            if(i_dim != 3)
                continue;

            char probe_type[16] = "probe_";

            property = toml_get(dump_probe, "probe_type");

            if(property.type == TOML_INT64){

                if(property.u.int64 != RCFD_PROBE_USER && property.u.int64 != RCFD_PROBE_REC_USER){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Probe types other than 'user' and 'rec_user'"
                                  " must be placed in phase-specific section"
                                  " '%s.phases.probes' in config file",
                                  dict);
                    continue;
                }

                strcpy(probe_type, (property.u.int64 == RCFD_PROBE_USER) ? "probe_user" : "probe_rec_user");
            }
            else if(property.type == TOML_STRING){

                if(    strcmp(property.u.str.ptr, "user") != 0
                    && strcmp(property.u.str.ptr, "rec_user") != 0){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Probe types other than 'user' and 'rec_user'"
                                  " must be placed in phase-specific section"
                                  " '%s.phases.probes' in config file",
                                  dict);
                    continue;
                }

                strcat(probe_type, property.u.str.ptr);
            }
            else{

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Probe type of element %d of '%s.probes' does not exist"
                              " or is neither an integer nor a string in config file",
                              i_probe, dict);
                continue;
            }

            int i_data = -1;
            property = toml_get(dump_probe, "probe_data");

            if(property.type == TOML_INT64){

                i_data = property.u.int64;
            }
            else{

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Probe data of element %d of '%s.probes' does not exist"
                              " or is not an integer in config file",
                              i_probe, dict);
                continue;
            }

            if(set_dump_dict_property_r_3d(rcfd, 0, -1, i_data, probe_type, probe_position) != 0){

                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Failed to set %s of '%s.probes' from config file",
                              probe_type, dict);
            }
        }
    }


    // phase dependent flag arrays: dump.phases
    toml_datum_t dump_phases = toml_seek(dump, "phases");
    if(dump_phases.type != TOML_ARRAY){

        report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                      "'%s.phases' does not exist"
                      " or is not a (table) array in config file",
                      dict);
        return;
    }

    int phase_count = rcfd->Solver_Dict->number_of_phases;
    int phase_count_toml = dump_phases.u.arr.size;

    if(phase_count > phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "Only %d of %d phases are defined"
                        " for %s.phases (table) array in config file",
                        phase_count_toml, phase_count, dict);
        phase_count = phase_count_toml;
    }
    else if(phase_count < phase_count_toml){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "More than %d phases are defined"
                        " for %s.phases (table) array in config file"
                        " - ignoring %d phase definitions",
                        phase_count, dict, phase_count_toml - phase_count);
    }


    for(int i_phase = 0; i_phase < phase_count; ++i_phase){

        // integer array property "dump_cell_data" (per data item)
        toml_datum_t property = toml_get(dump_phases.u.arr.elem[i_phase], "dump_cell_data");

        if(property.type != TOML_ARRAY){

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.phases.%s' does not exist"
                            " or is not an integer/boolean array"
                            " for phase %d in config file",
                            dict, "dump_cell_data", i_phase);
        }
        else{

            // read flags for each data item of this phase
            for(int i_data = 0; i_data < property.u.arr.size; ++i_data){

                toml_datum_t data_on = property.u.arr.elem[i_data];

                if(set_dump_dict_property_i(rcfd, i_phase, i_data, "dump_cell_data", data_on.u.int64) != 0){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Failed to set integer property '%s'"
                                  " in %s.phases dict for phase %d, data %d"
                                  " from config file",
                                  "dump_cell_data", dict, i_phase, i_data);

                }
            }
        }

        // phase-specific probes
        toml_datum_t dump_phase_probes = toml_get(dump_phases.u.arr.elem[i_phase], "probes");

        if(dump_phase_probes.type != TOML_ARRAY){

            report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                            "'%s.phases.probes' does not exist"
                            " or is not a (table) array for phase %d"
                            " in config file",
                            dict, i_phase);
            continue;
        }
        else
        {
            int probe_count = dump_phase_probes.u.arr.size;

            for(int i_probe = 0; i_probe < probe_count; ++i_probe){

                toml_datum_t dump_phase_probe = dump_phase_probes.u.arr.elem[i_probe];
                if(dump_phase_probe.type != TOML_TABLE){

                    report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Element %d of '%s.phases.probes'"
                                  " is not a table in config file",
                                  i_probe, dict);
                    continue;
                }

                int i_dim;
                double probe_position[3] = {0};

                toml_datum_t property = toml_get(dump_phase_probe, "probe_position");

                if(property.type != TOML_ARRAY || property.u.arr.size != 3){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Probe position of element %d of '%s.probes'"
                                  " is not a 3D array in config file",
                                  i_probe, dict);
                    continue;
                }

                for(i_dim = 0; i_dim < 3; ++i_dim){

                    toml_datum_t x = property.u.arr.elem[i_dim];
                    if(x.type == TOML_INT64){

                        probe_position[i_dim] = x.u.int64;
                    }
                    else if(x.type == TOML_FP64){

                        probe_position[i_dim] = x.u.fp64;
                    }
                    else{

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Probe position of element %d of '%s.probes'"
                                      " has invalid component type in config file",
                                      i_probe, dict);
                        break;
                    }
                }

                if(i_dim != 3)
                    continue;

                char probe_type[16] = {0};//"probe_";

                property = toml_get(dump_phase_probe, "probe_type");

                // Note: user and rec_user probes are not phase-specific and
                // should be defined under [[dump.probes]] instead, but for now
                // we allow them to be defined here as well.
                if(property.type == TOML_INT64){

                    if(property.u.int64 < 0 || property.u.int64 >= RCFD_PROBE_MAX){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Invalid probe type %d in '%s.phases.probes'"
                                      " for phase %d in config file",
                                      property.u.int64, dict, i_phase);
                        continue;
                    }

                    switch(property.u.int64){

                    case RCFD_PROBE_CELL:
                        strcpy(probe_type, "probe_cell_data");
                        break;
                    case RCFD_PROBE_VOF:
                        strcpy(probe_type, "probe_vof");
                        break;
                    case RCFD_PROBE_USER:
                        strcpy(probe_type, "probe_user");
                        break;
                    case RCFD_PROBE_REC_USER:
                        strcpy(probe_type, "probe_rec_user");
                        break;
                    default:
                        break;
                    }
                }
                else if(property.type == TOML_STRING){

                    if(    strcmp(property.u.str.ptr, "cell") == 0
                        || strcmp(property.u.str.ptr, "cell_data") == 0){

                        strcpy(probe_type, "probe_cell_data");
                    }
                    else if(strcmp(property.u.str.ptr, "vof") == 0){

                        strcpy(probe_type, "probe_vof");
                    }
                    else if(strcmp(property.u.str.ptr, "user") == 0){

                        strcpy(probe_type, "probe_user");
                    }
                    else if(strcmp(property.u.str.ptr, "rec_user") == 0){

                        strcpy(probe_type, "probe_rec_user");
                    }
                    else{

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                      "Invalid probe type %s in '%s.phases.probes'"
                                      " for phase %d in config file",
                                      property.u.str.ptr, dict, i_phase);
                        continue;
                    }
                }
                else{

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Probe type of element %d of '%s.phases.probes'"
                                  " for phase %d does not exist"
                                  " or is neither an integer nor a string"
                                  " in config file",
                                  i_probe, dict, i_phase);
                    continue;
                }

                int i_data = -1;
                property = toml_get(dump_phase_probe, "probe_data");

                if(property.type == TOML_INT64){

                    i_data = property.u.int64;
                }
                else if(strcmp(probe_type, "probe_vof") != 0){ // vof does not need data index

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Probe data of element %d of '%s.phases.probes'"
                                  " for phase %d does not exist"
                                  " or is not an integer in config file",
                                  i_probe, dict, i_phase);
                    continue;
                }

                if(set_dump_dict_property_r_3d(rcfd, 0, i_phase, i_data, probe_type, probe_position) != 0){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "Failed to set %s of '%s.phases.probes'"
                                  " for phase %d, data %d from config file",
                                  probe_type, dict, i_phase, i_data);
                }
            }
        }
    }
#endif
}

#endif // RCFD_DICT_READ_TOML_H
