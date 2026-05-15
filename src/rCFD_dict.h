/******************************************************************************
 * @internal
 * @file        rCFD_dict.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_DICT_H
#define RCFD_DICT_H

#include <stdlib.h>
#include <string.h>
#include "rCFD_macros.h"
#include "rCFD_types.h"
#include "rCFD_utilities.h"

/**
 * @brief set integer property in solver dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_solver_dict_property_i(void *handle, const char *property, int value)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_SOLVER_DICT)){

            report_dict_access_denied(rcfd, "solver", property, "integer");
            return -1;
        }

        if(strcmp(property, "report_priority") == 0){

            if(value < 0 || value >= RCFD_REPORT_MAX){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->report_priority = value;
        }
        else if(strcmp(property, "profiling_on") == 0){

            rcfd->Solver_Dict->profiling_on = value;
        }
        else if(strcmp(property, "multifile_output_on") == 0){

            rcfd->Solver_Dict->multifile_output_on = value;
        }
        else if(strcmp(property, "write_config_to_file_on") == 0){

            rcfd->Solver_Dict->write_config_to_file_on = value;
        }
        else if(strcmp(property, "number_of_frames") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->number_of_frames = value;
        }
        else if(strcmp(property, "number_of_states") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->number_of_states = value;
        }
        else if(strcmp(property, "number_of_phases") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->number_of_phases = value;
        }
        else if(strcmp(property, "number_of_layers") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

#if !MULTI_LAYER_IMPLEMENTATION_COMPLETE
            if(value > 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                report_error0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "Implementation of multiple cell layers is incomplete.");
                return -1;
            }
#endif // MULTI_LAYER_IMPLEMENTATION_COMPLETE

            rcfd->Solver_Dict->number_of_layers = value;
        }
        else if(strcmp(property, "number_of_runs") == 0){

            rcfd->Solver_Dict->number_of_runs = value;
        }
        else if(strcmp(property, "recurrence_process_on") == 0){

            rcfd->Solver_Dict->recurrence_process_on = value;
        }
        else if(strcmp(property, "data_convection_on") == 0){

            rcfd->Solver_Dict->data_convection_on = value;
        }
        else if(strcmp(property, "face_swap_diffusion_on") == 0){

            rcfd->Solver_Dict->face_swap_diffusion_on = value;
        }
        else if(strcmp(property, "local_diff_on") == 0){

            rcfd->Solver_Dict->local_diff_on = value;
        }
        else if(strcmp(property, "parallel_diffusion_on") == 0){

            rcfd->Solver_Dict->parallel_diffusion_on = value;
        }
        else if(strcmp(property, "data_binarization_on") == 0){

            rcfd->Solver_Dict->data_binarization_on = value;
        }
        else if(strcmp(property, "data_drifting_on") == 0){

            rcfd->Solver_Dict->data_drifting_on = value;
        }
        else if(strcmp(property, "balance_correction_on") == 0){

            rcfd->Solver_Dict->balance_correction_on = value;
        }
        else if(strcmp(property, "interpolation_on") == 0){

            rcfd->Solver_Dict->interpolation_on = value;
        }
        else if(strcmp(property, "shifts_from_displacement_fields_on") == 0){

            rcfd->Solver_Dict->shifts_from_displacement_fields_on = value;
        }
        else if(strcmp(property, "moving_geometry_on") == 0){

            rcfd->Solver_Dict->moving_geometry_on = value;
        }
        else if(strcmp(property, "grid_interface_shifts_on") == 0){

            rcfd->Solver_Dict->grid_interface_shifts_on = value;
        }
        else if(strcmp(property, "max_number_of_cells_per_time_step") == 0){

            rcfd->Solver_Dict->max_number_of_cells_per_time_step = value;
        }
        else if(strcmp(property, "time_steps_per_monitoring_interval") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->time_steps_per_monitoring_interval = value;
        }
        else if(strcmp(property, "single_phase_vof") == 0){

            rcfd->Solver_Dict->single_phase_vof = value;
        }
        else if(strcmp(property, "single_phase_vof_id") == 0){

            rcfd->Solver_Dict->single_phase_vof_id = value;
        }
        else if(strcmp(property, "min_number_of_cells_per_layer") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->min_number_of_cells_per_layer = value;
        }
        else if(strcmp(property, "max_number_of_faces_per_cell") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->max_number_of_faces_per_cell = value;
        }
        else if(strcmp(property, "max_number_of_children") == 0){

            rcfd->Solver_Dict->max_number_of_children = value;
        }
        else if(strcmp(property, "number_redist_loops_per_layer") == 0){

            rcfd->Solver_Dict->number_redist_loops_per_layer = value;
        }
        else if(strcmp(property, "max_fill_loops") == 0){

            rcfd->Solver_Dict->max_fill_loops = value;
        }
        else if(strcmp(property, "face_swap_max_loops") == 0){

            rcfd->Solver_Dict->face_swap_max_loops = value;
        }
        else if(strcmp(property, "local_diff_format") == 0){

            if(value < 0 || value >= RCFD_LOCAL_DIFF_MAX){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->local_diff_format = value;
        }
        else if(strcmp(property, "number_of_drift_loops") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->number_of_drift_loops = value;
        }
        else if(strcmp(property, "balance_correction_update") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->balance_correction_update = value;
        }
        else if(strcmp(property, "control_conc_sum_on") == 0){

            rcfd->Solver_Dict->control_conc_sum_on = value;
        }
        else{

            report_dict_unknown_property(rcfd, "solver", property, "integer");
            return -1;
        }

        return 0;
    }

    return -1;
}

/**
 * @brief set integer property in phase dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_phase_dict_property_i(void *handle, int i_phase, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_PHASE_DICT)){

            report_dict_access_denied(rcfd, "phase", property, "integer");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "phase", property, "integer", i_phase);
            return -1;
        }

        if(strcmp(property, "number_of_data") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "phase", property, value);
                return -1;
            }

            rcfd->Phase_Dict[i_phase].number_of_data = value;
        }
        else if(strcmp(property, "hindered_drift_on") == 0){

            rcfd->Phase_Dict[i_phase].hindered_drift_on = value;
        }
        else if(strcmp(property, "local_drift_on") == 0){

            rcfd->Phase_Dict[i_phase].local_drift_on = value;
        }
        else if(strcmp(property, "drift_control_conc_sum_on") == 0){

            rcfd->Phase_Dict[i_phase].drift_control_conc_sum_on = value;
        }
        else if(strcmp(property, "number_of_user_vars") == 0){

            if(rcfd->Phase_Dict[i_phase].number_of_user_vars == value)
                return 0;

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "phase", property, value);
                return - 1;
            }

            rcfd->Phase_Dict[i_phase].user = (double*)realloc(rcfd->Phase_Dict[i_phase].user, value * sizeof(double));
            rcfd->Phase_Dict[i_phase].number_of_user_vars = value;
        }
        else{

            report_dict_unknown_property(rcfd, "phase", property, "integer");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set integer property in tracer dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_prep_dict_property_i(void *handle, int i_phase, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_PREP_DICT)){

            report_dict_access_denied(rcfd, "prep", property, "integer");
            return -1;
        }

        if(strcmp(property, "tracer_format") == 0){

            if(value < 0 || value >= RCFD_TRACER_MAX){

                report_dict_invalid_value_i(rcfd, "prep", property, value);
                return -1;
            }

            rcfd->Prep_Dict->tracer_format = value;
        }
        else if(strcmp(property, "tracers_per_cell_count") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "prep", property, value);
                return -1;
            }

            rcfd->Prep_Dict->tracers_per_cell_count = value;
        }
        else if(strcmp(property, "tracer_region_of_interest_exists") == 0){

            rcfd->Prep_Dict->tracer_region_of_interest_exists = value;
        }
        else if(strcmp(property, "tracer_coarse_graining") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "prep", property, value);
                return -1;
            }

            rcfd->Prep_Dict->tracer_coarse_graining = value;
        }
        else if(strcmp(property, "tracer_random_walk_on") == 0){

            if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

                report_dict_invalid_phase(rcfd, "prep", property, "integer", i_phase);
                return -1;
            }

            if(!rcfd->Prep_Dict->tracer_random_walk_on){

                report_dict_invalid_memory(rcfd, "prep", property, "integer");
                return -1;
            }

            rcfd->Prep_Dict->tracer_random_walk_on[i_phase] = value;
        }
        else if(strcmp(property, "tracer_splitting_on") == 0){

            rcfd->Prep_Dict->tracer_splitting_on = value;
        }
        else if(strcmp(property, "tracer_reverse_direction_on") == 0){

            rcfd->Prep_Dict->tracer_reverse_direction_on = value;
        }
        else if(strcmp(property, "interpolation_format") == 0){

            if(value < 0 || value >= RCFD_INTERP_MAX){

                report_dict_invalid_value_i(rcfd, "prep", property, value);
                return -1;
            }

            rcfd->Prep_Dict->interpolation_format = value;
        }
        else if(strcmp(property, "interpolation_max_number_of_aligned_neigh_cells") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "prep", property, value);
                return -1;
            }

            rcfd->Prep_Dict->interpolation_max_number_of_aligned_neigh_cells = value;
        }
        else if(strcmp(property, "interpolation_pruning_on") == 0){

            rcfd->Prep_Dict->interpolation_pruning_on = value;
        }
        else if(strcmp(property, "interpolation_min_shifts_after_pruning") == 0){

            rcfd->Prep_Dict->interpolation_min_shifts_after_pruning = value;
        }
        else if(strcmp(property, "interpolation_multiply_shift_weights_with_vof_on") == 0){

            rcfd->Prep_Dict->interpolation_multiply_shift_weights_with_vof_on = value;
        }
        else{

            report_dict_unknown_property(rcfd, "prep", property, "integer");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set integer property in norm dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_norm_dict_property_i(void *handle, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_NORM_DICT)){

            report_dict_access_denied(rcfd, "norm", property, "integer");
            return -1;
        }

        if(strcmp(property, "format") == 0){

            if(value < 0 || value >= RCFD_NORM_MAX){

                report_dict_invalid_value_i(rcfd, "norm", property, value);
                return -1;
            }

            rcfd->Norm_Dict->format = value;
        }
        else if(strcmp(property, "coarse_graining") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "norm", property, value);
                return -1;
            }

            rcfd->Norm_Dict->coarse_graining = value;
        }
        else if(strcmp(property, "larger_than_mean_norm_only") == 0){

            rcfd->Norm_Dict->larger_than_mean_norm_only = value;
        }
        else{

            report_dict_unknown_property(rcfd, "norm", property, "integer");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set integer property in rec dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_rec_dict_property_i(void *handle, const char *property, int value)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_REC_DICT)){

            report_dict_access_denied(rcfd, "rec", property, "integer");
            return -1;
        }

        if(strcmp(property, "format") == 0){

            if(value < 0 || value >= RCFD_REC_MAX){

                report_dict_invalid_value_i(rcfd, "rec", property, value);
                return -1;
            }

            rcfd->Rec_Dict->format = value;
        }
        else if(strcmp(property, "monitor_rec_frames_on") == 0){

            rcfd->Rec_Dict->monitor_rec_frames_on = value;
        }
        else if(strcmp(property, "adapt_vof_stitching_on") == 0){

            rcfd->Rec_Dict->adapt_vof_stitching_on = value;
        }
        else if(strcmp(property, "min_seq_length") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "rec", property, value);
                return -1;
            }

            rcfd->Rec_Dict->min_seq_length = value;
        }
        else if(strcmp(property, "max_seq_length") == 0){

            rcfd->Rec_Dict->max_seq_length = value;
        }
        else if(strcmp(property, "off_diagonal_band_width") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "rec", property, value);
                return -1;
            }

            rcfd->Rec_Dict->off_diagonal_band_width = value;
        }
        else if(strcmp(property, "number_of_adapt_vof_loops") == 0){

            rcfd->Rec_Dict->number_of_adapt_vof_loops = value;
        }
        else if(strcmp(property, "seed") == 0){

            if(value < -1){

                report_dict_invalid_value_i(rcfd, "rec", property, value);
                return -1;
            }

            rcfd->Rec_Dict->seed = value;
        }
        else{

            report_dict_unknown_property(rcfd, "rec", property, "integer");
            return -1;
        }

        return 0;
    }

    return -1;
}

/**
 * @brief set integer property in data dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_data_dict_property_i(void *handle, int i_phase, int i_data, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_DATA_DICT)){

            report_dict_access_denied(rcfd, "data", property, "integer");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "data", property, "integer", i_phase);
            return -1;
        }

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

            report_dict_invalid_data(rcfd, "data", property, "integer", i_data);
            return -1;
        }


        if(strcmp(property, "type") == 0){

            if(value < 0 || value >= RCFD_DATA_MAX){

                report_dict_invalid_value_i(rcfd, "data", property, value);
                return -1;
            }

            rcfd->Data_Dict[i_phase][i_data].type = value;
        }
        else if(strcmp(property, "drifting_data") == 0){

            rcfd->Data_Dict[i_phase][i_data].drifting_data = value;
        }
        else if(strcmp(property, "number_of_user_vars") == 0){

            if(rcfd->Data_Dict[i_phase][i_data].number_of_user_vars == value)
                return 0;

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "data", property, value);
                return - 1;
            }

            rcfd->Data_Dict[i_phase][i_data].user = (double*)realloc(rcfd->Data_Dict[i_phase][i_data].user, value * sizeof(double));
            rcfd->Data_Dict[i_phase][i_data].number_of_user_vars = value;
        }
        else{

            report_dict_unknown_property(rcfd, "data", property, "integer");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set integer property in balance dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_balance_dict_property_i(void *handle, int i_phase, int i_data, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_BALANCE_DICT)){

            report_dict_access_denied(rcfd, "balance", property, "integer");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "balance", property, "integer", i_phase);
            return -1;
        }

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

            report_dict_invalid_data(rcfd, "balance", property, "integer", i_data);
            return -1;
        }


        if(strcmp(property, "type") == 0){

            if(value < 0 || value >= RCFD_BALANCE_MAX){

                report_dict_invalid_value_i(rcfd, "balance", property, value);
                return -1;
            }

            rcfd->Balance_Dict[i_phase][i_data].type = value;
        }
        else if(strcmp(property, "max_correction_loops") == 0){

            rcfd->Balance_Dict[i_phase][i_data].max_correction_loops = value;
        }
        else if(strcmp(property, "write_balance_to_file") == 0){

            rcfd->Balance_Dict[i_phase][i_data].write_balance_to_file = value;
        }
        else if(strcmp(property, "write_balance_to_file_interval") == 0){

            if(value < 1){

                report_dict_invalid_value_i(rcfd, "balance", property, value);
                return -1;
            }

            rcfd->Balance_Dict[i_phase][i_data].write_balance_to_file_interval = value;
        }
        else{

            report_dict_unknown_property(rcfd, "balance", property, "integer");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set integer property in cell dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the layer if the property is per-layer
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_cell_dict_property_i(void *handle, int i_layer, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_CELL_DICT)){

            report_dict_access_denied(rcfd, "cell", property, "integer");
            return -1;
        }

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers){

            report_dict_invalid_layer(rcfd, "cell", property, "integer", i_layer);
            return -1;
        }

        if(strcmp(property, "number_of_int_cells") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "cell", property, value);
                return -1;
            }

            _CELL_DICT(i_layer).number_of_int_cells = value;
            _CELL_DICT(i_layer).number_of_cells = _CELL_DICT(i_layer).number_of_int_cells + _CELL_DICT(i_layer).number_of_ext_cells;
        }
        else if(strcmp(property, "number_of_ext_cells") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "cell", property, value);
                return -1;
            }

            _CELL_DICT(i_layer).number_of_ext_cells = value;
            _CELL_DICT(i_layer).number_of_cells = _CELL_DICT(i_layer).number_of_int_cells + _CELL_DICT(i_layer).number_of_ext_cells;
        }
        else if(strcmp(property, "number_of_user_vars") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "cell", property, value);
                return -1;
            }

            _CELL_DICT(i_layer).number_of_user_vars = value;
        }
        else if(strcmp(property, "number_of_rec_user_vars") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "cell", property, value);
                return -1;
            }

            _CELL_DICT(i_layer).number_of_rec_user_vars = value;
        }
        else{

            report_dict_unknown_property(rcfd, "cell", property, "integer");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set integer property in face dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the layer if the property is per-layer
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_face_dict_property_i(void *handle, int i_layer, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_FACE_DICT)){

            report_dict_access_denied(rcfd, "face", property, "integer");
            return -1;
        }

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers){

            report_dict_invalid_layer(rcfd, "face", property, "integer", i_layer);
            return -1;
        }

        if(strcmp(property, "number_of_int_faces") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "face", property, value);
                return -1;
            }

            _FACE_DICT(i_layer).number_of_int_faces = value;
            _FACE_DICT(i_layer).number_of_faces = _FACE_DICT(i_layer).number_of_int_faces + _FACE_DICT(i_layer).number_of_ext_faces;
        }
        else if(strcmp(property, "number_of_ext_faces") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "face", property, value);
                return -1;
            }

            _FACE_DICT(i_layer).number_of_ext_faces = value;
            _FACE_DICT(i_layer).number_of_faces = _FACE_DICT(i_layer).number_of_int_faces + _FACE_DICT(i_layer).number_of_ext_faces;
        }
        else{

            report_dict_unknown_property(rcfd, "face", property, "integer");
            return -1;
        }

        return 0;
    }
#endif
    return -1;
}

/**
 * @brief set integer property in node dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the layer if the property is per-layer
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_node_dict_property_i(void *handle, int i_layer, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_NODE_DICT)){

            report_dict_access_denied(rcfd, "node", property, "integer");
            return -1;
        }

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers){

            report_dict_invalid_layer(rcfd, "node", property, "integer", i_layer);
            return -1;
        }

        if(strcmp(property, "number_of_nodes") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "node", property, value);
                return -1;
            }

            _NODE_DICT(i_layer).number_of_nodes = value;
        }
        else{

            report_dict_unknown_property(rcfd, "node", property, "integer");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set integer property in dump dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     integer value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_dump_dict_property_i(void *handle, int i_phase, int i_data, const char *property, int value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_DUMP_DICT)){

            report_dict_access_denied(rcfd, "dump", property, "integer");
            return -1;
        }

        if(strcmp(property, "dump_interval") == 0){

            if(value < 0){

                report_dict_invalid_value_i(rcfd, "dump", property, value);
                return -1;
            }

            rcfd->Dump_Dict->dump_interval = value;
        }
        else if(strcmp(property, "overwrite_interval") == 0){

            rcfd->Dump_Dict->overwrite_interval = value;
        }
        else if(strcmp(property, "dump_file_type") == 0){

            if(value < 0 || value >= RCFD_FILE_MAX){

                report_dict_invalid_value_i(rcfd, "dump", property, value);
                return -1;
            }

            rcfd->Dump_Dict->dump_file_type = value;
        }
        else if(strcmp(property, "multifile_output_on") == 0){

            rcfd->Dump_Dict->multifile_output_on = value;
        }
        else if(strcmp(property, "dump_vof") == 0){

            rcfd->Dump_Dict->dump_vof = value;
        }
        else if(strcmp(property, "dump_cell_data") == 0){

            if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

                report_dict_invalid_phase(rcfd, "dump", property, "integer", i_phase);
                return -1;
            }

            if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

                report_dict_invalid_data(rcfd, "dump", property, "integer", i_data);
                return -1;
            }

            rcfd->Dump_Dict->dump_cell_data[i_phase][i_data] = value;
        }
        else if(strcmp(property, "dump_user_data") == 0){

            if(i_data < 0 || i_data >= _CELL_DICT(0).number_of_user_vars){

                report_dict_invalid_data(rcfd, "dump", property, "integer", i_data);
                return -1;
            }

            rcfd->Dump_Dict->dump_user_data[i_data] = value;
        }
        else if(strcmp(property, "dump_rec_user_data") == 0){

            if(i_data < 0 || i_data >= _CELL_DICT(0).number_of_rec_user_vars){

                report_dict_invalid_data(rcfd, "dump", property, "integer", i_data);
                return -1;
            }

            rcfd->Dump_Dict->dump_rec_user_data[i_data] = value;
        }
        else if(strcmp(property, "probe_interval") == 0){

            rcfd->Dump_Dict->probe_interval = value;
        }
        else{

            report_dict_unknown_property(rcfd, "dump", property, "integer");
            return -1;
        }

        return 0;
    }
#endif
    return -1;
}

/**
 * @brief set floating point property in solver dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     floating point value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_solver_dict_property_r(void *handle, const char *property, double value)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_SOLVER_DICT)){

            report_dict_access_denied(rcfd, "solver", property, "real");
            return -1;
        }

        if(strcmp(property, "start_time_for_monitoring") == 0){

            rcfd->Solver_Dict->start_time_for_monitoring = value;
        }
        else if(strcmp(property, "vof_cutoff_limit") == 0){

            rcfd->Solver_Dict->vof_cutoff_limit = value;
        }
        else if(strcmp(property, "global_time_step") == 0){

            rcfd->Solver_Dict->global_time_step = value;
        }
        else if(strcmp(property, "face_swap_max_per_loop") == 0){

            rcfd->Solver_Dict->face_swap_max_per_loop = value;
        }
        else if(strcmp(property, "grid_interface_shift_weight") == 0){

            if((value <= 0.0) || (value > 1.0)){

                report_dict_invalid_value_r(rcfd, "solver", property, value);
                return -1;
            }

            rcfd->Solver_Dict->grid_interface_shift_weight = value;
        }
        else{

            report_dict_unknown_property(rcfd, "solver", property, "real");
            return -1;
        }

        return 0;
    }

    return -1;
}

/**
 * @brief set floating point property in phase dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     floating point value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_phase_dict_property_r(void *handle, int i_phase, const char *property, double value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_PHASE_DICT)){

            report_dict_access_denied(rcfd, "phase", property, "real");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "phase", property, "real", i_phase);
            return -1;
        }

        if(strcmp(property, "time_step") == 0){

            rcfd->Phase_Dict[i_phase].time_step = value;
        }
        else if(strcmp(property, "density") == 0){

            rcfd->Phase_Dict[i_phase].density = value;
        }
        else if(strcmp(property, "heat_capacity") == 0){

            rcfd->Phase_Dict[i_phase].heat_capacity = value;
        }
        else if(strcmp(property, "reference_temperature") == 0){

            rcfd->Phase_Dict[i_phase].reference_temperature = value;
        }
        else if(strcmp(property, "vof_max") == 0){

            rcfd->Phase_Dict[i_phase].vof_max = value;
        }
        else if(strcmp(property, "d32_diam") == 0){

            rcfd->Phase_Dict[i_phase].d32_diam = value;
        }
        else{

            report_dict_unknown_property(rcfd, "phase", property, "real");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set floating point property in tracer dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param property  name of the property to be set
 * @param value     floating point value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_prep_dict_property_r(void *handle, int i_phase, const char *property, double value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_PREP_DICT)){

            report_dict_access_denied(rcfd, "prep", property, "real");
            return -1;
        }

        if(strcmp(property, "tracer_ROI_x_min") == 0){

            rcfd->Prep_Dict->tracer_ROI_x_min = value;
        }
        else if(strcmp(property, "tracer_ROI_x_max") == 0){

            rcfd->Prep_Dict->tracer_ROI_x_max = value;
        }
        else if(strcmp(property, "tracer_ROI_y_min") == 0){

            rcfd->Prep_Dict->tracer_ROI_y_min = value;
        }
        else if(strcmp(property, "tracer_ROI_y_max") == 0){

            rcfd->Prep_Dict->tracer_ROI_y_max = value;
        }
        else if(strcmp(property, "tracer_ROI_z_min") == 0){

            rcfd->Prep_Dict->tracer_ROI_z_min = value;
        }
        else if(strcmp(property, "tracer_ROI_z_max") == 0){

            rcfd->Prep_Dict->tracer_ROI_z_max = value;
        }
        else if(strcmp(property, "tracer_random_walk_velocity_ratio") == 0){

            if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

                report_dict_invalid_phase(rcfd, "prep", property, "real", i_phase);
                return -1;
            }

            if(!rcfd->Prep_Dict->tracer_random_walk_velocity_ratio){

                report_dict_invalid_memory(rcfd, "prep", property, "real");
                return -1;
            }

            rcfd->Prep_Dict->tracer_random_walk_velocity_ratio[i_phase] = value;
        }
        else if(strcmp(property, "interpolation_pruning_weight") == 0){

            if(value < 0.0 || value > 1.0){

                report_dict_invalid_value_r(rcfd, "prep", property, value);
                return -1;
            }

            rcfd->Prep_Dict->interpolation_pruning_weight = value;
        }
        else{

            report_dict_unknown_property(rcfd, "prep", property, "real");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set floating point property in data dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     floating point value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_data_dict_property_r(void *handle, int i_phase, int i_data, const char *property, double value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_DATA_DICT)){

            report_dict_access_denied(rcfd, "data", property, "real");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "data", property, "real", i_phase);
            return -1;
        }

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

            report_dict_invalid_data(rcfd, "data", property, "real", i_data);
            return -1;
        }


        if(strcmp(property, "face_swap_diffusion") == 0){

            rcfd->Data_Dict[i_phase][i_data].face_swap_diffusion = value;
        }
        else if(strcmp(property, "binarization_art_diff") == 0){

            rcfd->Data_Dict[i_phase][i_data].binarization_art_diff = value;
        }
        else if(strcmp(property, "d1_diam") == 0){

            rcfd->Data_Dict[i_phase][i_data].d1_diam = value;
        }
        else{

            report_dict_unknown_property(rcfd, "data", property, "real");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set floating point property in balance dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     floating point value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_balance_dict_property_r(void *handle, int i_phase, int i_data, const char *property, double value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_BALANCE_DICT)){

            report_dict_access_denied(rcfd, "balance", property, "real");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "balance", property, "real", i_phase);
            return -1;
        }

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

            report_dict_invalid_data(rcfd, "balance", property, "real", i_data);
            return -1;
        }


        if(strcmp(property, "accuracy_level") == 0){

            rcfd->Balance_Dict[i_phase][i_data].accuracy_level = value;
        }
        else{

            report_dict_unknown_property(rcfd, "balance", property, "real");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set floating point property in cell dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the layer if the property is per-layer
 * @param property  name of the property to be set
 * @param value     floating point value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_cell_dict_property_r(void *handle, int i_layer, const char *property, double value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_CELL_DICT)){

            report_dict_access_denied(rcfd, "cell", property, "real");
            return -1;
        }

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers){

            report_dict_invalid_layer(rcfd, "cell", property, "real", i_layer);
            return -1;
        }

        if(strcmp(property, "next_neigh_cell_skin_factor") == 0){

            _CELL_DICT(i_layer).next_neigh_cell_skin_factor = value;
        }
        else{

            report_dict_unknown_property(rcfd, "cell", property, "real");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set 3d floating point property in data dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     3d array of double values to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_data_dict_property_r_3d(void *handle, int i_phase, int i_data, const char *property, double *value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_DATA_DICT)){

            report_dict_access_denied(rcfd, "data", property, "vector");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "data", property, "vector", i_phase);
            return -1;
        }

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

            report_dict_invalid_data(rcfd, "data", property, "vector", i_data);
            return -1;
        }

        if(strcmp(property, "drift_velocity") == 0){

            if(rcfd->Data_Dict[i_phase][i_data].drift_velocity == NULL){

                report_dict_invalid_memory(rcfd, "data", property, "vector");
                return -1;
            }

            rcfd->Data_Dict[i_phase][i_data].drift_velocity[0] = value[0];
            rcfd->Data_Dict[i_phase][i_data].drift_velocity[1] = value[1];
            rcfd->Data_Dict[i_phase][i_data].drift_velocity[2] = value[2];
        }
        else{

            report_dict_unknown_property(rcfd, "data", property, "vector");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief Add a new probe
 * @param rcfd          pointer to previously created rCFD instance
 * @param probe_type    which type of data to probe: cell data, vof data, user data, rec user data
 * @param i_layer       specifies the cell layer to search for probe cell
 * @param i_phase       specifies the phase for a phase-specific probe
 * @param i_data        specifies the data index
 * @param x             specifies the probe position
 */
void dump_dict_add_probe(rCFD_type *rcfd, int probe_type, int i_layer, int i_phase, int i_data, double *x)
{
#if RP_NODE
    int probe_node = 0;
    int i_probe = rcfd->Dump_Dict->probe_count;

    // find closest local cell center
    double dist_sq = 0.;
    int i_cell = find_closest_cell_to_point(rcfd, i_layer, x, &dist_sq);

    double *dist_sq_all = (double*)malloc(sizeof(double) * compute_node_count);

    // find closest global cell center
    PRF_GATHER_REAL(dist_sq_all, &dist_sq, 1, 0);

    if(myid == 0){

        int i_node;
        double min_dist_sq = dist_sq;

        for(i_node = 1; i_node < compute_node_count; ++i_node){

            if(dist_sq_all[i_node] < min_dist_sq){

                min_dist_sq = dist_sq_all[i_node];
                probe_node = i_node;
            }
        }
    }

    free(dist_sq_all);

    PRF_BCAST_INT(&probe_node, 1, 0);

    if(probe_node != myid){

        i_cell = -1; // nearest cell center not on this compute node
    }

    rcfd->Dump_Dict->probe_count++;

    rcfd->Dump_Dict->probe_layer= (int*)realloc(rcfd->Dump_Dict->probe_layer,sizeof(int) * rcfd->Dump_Dict->probe_count);
    rcfd->Dump_Dict->probe_cell = (int*)realloc(rcfd->Dump_Dict->probe_cell, sizeof(int) * rcfd->Dump_Dict->probe_count);
    rcfd->Dump_Dict->probe_type = (int*)realloc(rcfd->Dump_Dict->probe_type, sizeof(int) * rcfd->Dump_Dict->probe_count);
    rcfd->Dump_Dict->probe_phase= (int*)realloc(rcfd->Dump_Dict->probe_phase,sizeof(int) * rcfd->Dump_Dict->probe_count);
    rcfd->Dump_Dict->probe_data = (int*)realloc(rcfd->Dump_Dict->probe_data, sizeof(int) * rcfd->Dump_Dict->probe_count);
    rcfd->Dump_Dict->probe_value= (double*)realloc(rcfd->Dump_Dict->probe_value, sizeof(double) * rcfd->Dump_Dict->probe_count);
    rcfd->Dump_Dict->probe_x    = realloc_r_2d(rcfd->Dump_Dict->probe_x, rcfd->Dump_Dict->probe_count, 3);

    rcfd->Dump_Dict->probe_layer[i_probe]   = i_layer;
    rcfd->Dump_Dict->probe_cell[i_probe]    = i_cell;
    rcfd->Dump_Dict->probe_type[i_probe]    = probe_type;
    rcfd->Dump_Dict->probe_phase[i_probe]   = i_phase;
    rcfd->Dump_Dict->probe_data[i_probe]    = i_data;
    rcfd->Dump_Dict->probe_value[i_probe]   = 0.0;
    rcfd->Dump_Dict->probe_x[i_probe][0]    = x[0];
    rcfd->Dump_Dict->probe_x[i_probe][1]    = x[1];
    rcfd->Dump_Dict->probe_x[i_probe][2]    = x[2];
#endif
}

/**
 * @brief set integer property in dump dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the layer if the property is per-layer
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     3d array of double values to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_dump_dict_property_r_3d(void *handle, int i_layer, int i_phase, int i_data, const char *property, double *value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_DUMP_DICT)){

            report_dict_access_denied(rcfd, "dump", property, "vector");
            return -1;
        }

        if(i_layer < 0 || i_layer >= rcfd->Solver_Dict->number_of_layers){

            report_dict_invalid_layer(rcfd, "dump", property, "vector", i_layer);
            return -1;
        }

        if(strcmp(property, "probe_cell_data") == 0){

            if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

                report_dict_invalid_phase(rcfd, "dump", property, "vector", i_phase);
                return -1;
            }

            if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

                report_dict_invalid_data(rcfd, "dump", property, "vector", i_data);
                return -1;
            }

            dump_dict_add_probe(rcfd, RCFD_PROBE_CELL, i_layer, i_phase, i_data, value);
        }
        else if(strcmp(property, "probe_vof") == 0){

            if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

                report_dict_invalid_phase(rcfd, "dump", property, "vector", i_phase);
                return -1;
            }

            dump_dict_add_probe(rcfd, RCFD_PROBE_VOF, i_layer, i_phase, i_data, value);
        }
        else if(strcmp(property, "probe_user") == 0){

            if(i_data < 0 || i_data >= _CELL_DICT(i_layer).number_of_user_vars){

                report_dict_invalid_data(rcfd, "dump", property, "vector", i_data);
                return -1;
            }

            dump_dict_add_probe(rcfd, RCFD_PROBE_USER, i_layer, i_phase, i_data, value);
        }
        else if(strcmp(property, "probe_rec_user") == 0){

            if(i_data < 0 || i_data >= _CELL_DICT(i_layer).number_of_rec_user_vars){

                report_dict_invalid_data(rcfd, "dump", property, "vector", i_data);
                return -1;
            }

            dump_dict_add_probe(rcfd, RCFD_PROBE_REC_USER, i_layer, i_phase, i_data, value);
        }
        else{

            report_dict_unknown_property(rcfd, "dump", property, "vector");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set string property in solver dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_solver_dict_property_s(void *handle, const char *property, const char *value)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_SOLVER_DICT)){

            report_dict_access_denied(rcfd, "solver", property, "string");
            return -1;
        }

        if(strcmp(property, "report_priority") == 0){

            if(strcmp(value, "trace") == 0){

                rcfd->Solver_Dict->report_priority = RCFD_REPORT_TRACE;
            }
            else if(strcmp(value, "debug") == 0){

                rcfd->Solver_Dict->report_priority = RCFD_REPORT_DEBUG;
            }
            else if(strcmp(value, "info") == 0){

                rcfd->Solver_Dict->report_priority = RCFD_REPORT_INFO;
            }
            else if(strcmp(value, "warning") == 0){

                rcfd->Solver_Dict->report_priority = RCFD_REPORT_WARNING;
            }
            else if(strcmp(value, "error") == 0){

                rcfd->Solver_Dict->report_priority = RCFD_REPORT_ERROR;
            }
            else if(strcmp(value, "fatal") == 0){

                rcfd->Solver_Dict->report_priority = RCFD_REPORT_FATAL;
            }
            else if(strcmp(value, "unknown") == 0){

                rcfd->Solver_Dict->report_priority = RCFD_REPORT_UNKNOWN;
            }
            else{

                report_dict_invalid_value_s(rcfd, "solver", property, value);
                return -1;
            }
        }
        else if(strcmp(property, "local_diff_format") == 0){

            if(strcmp(value, "viscosity_based") == 0){

                rcfd->Solver_Dict->local_diff_format = RCFD_LOCAL_DIFF_VISCOSITY_BASED;
            }
            else if(strcmp(value, "field_based") == 0){

                rcfd->Solver_Dict->local_diff_format = RCFD_LOCAL_DIFF_FIELD_BASED;
            }
            else{

                report_dict_invalid_value_s(rcfd, "solver", property, value);
                return -1;
            }
        }
        else{

            report_dict_unknown_property(rcfd, "solver", property, "string");
            return -1;
        }

        return 0;
    }

    return -1;
}

/**
 * @brief set string property in file dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_file_dict_property_s(void *handle, const char *property, const char *value)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_FILE_DICT)){

            report_dict_access_denied(rcfd, "file", property, "string");
            return -1;
        }

        if(strcmp(property, "working_dir") == 0){

            free(rcfd->File_Dict->working_dir);
            rcfd->File_Dict->working_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->working_dir, value);
        }
        else if(strcmp(property, "data_dir") == 0){

            free(rcfd->File_Dict->data_dir);
            rcfd->File_Dict->data_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->data_dir, value);
        }
        else if(strcmp(property, "tmp_sub_dir") == 0){

            free(rcfd->File_Dict->tmp_sub_dir);
            rcfd->File_Dict->tmp_sub_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->tmp_sub_dir, value);
        }
        else if(strcmp(property, "vof_sub_dir") == 0){

            free(rcfd->File_Dict->vof_sub_dir);
            rcfd->File_Dict->vof_sub_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->vof_sub_dir, value);
        }
        else if(strcmp(property, "topo_sub_dir") == 0){

            free(rcfd->File_Dict->topo_sub_dir);
            rcfd->File_Dict->topo_sub_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->topo_sub_dir, value);
        }
        else if(strcmp(property, "local_diff_sub_dir") == 0){

            free(rcfd->File_Dict->local_diff_sub_dir);
            rcfd->File_Dict->local_diff_sub_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->local_diff_sub_dir, value);
        }
        else if(strcmp(property, "c2c_sub_dir") == 0){

            free(rcfd->File_Dict->c2c_sub_dir);
            rcfd->File_Dict->c2c_sub_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->c2c_sub_dir, value);
        }
        else if(strcmp(property, "rec_sub_dir") == 0){

            free(rcfd->File_Dict->rec_sub_dir);
            rcfd->File_Dict->rec_sub_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->rec_sub_dir, value);
        }
        else if(strcmp(property, "user_sub_dir") == 0){

            free(rcfd->File_Dict->user_sub_dir);
            rcfd->File_Dict->user_sub_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->user_sub_dir, value);
        }
        else if(strcmp(property, "post_dir") == 0){

            free(rcfd->File_Dict->post_dir);
            rcfd->File_Dict->post_dir = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->post_dir, value);
        }
        else if(strcmp(property, "metadata_filename") == 0){

            free(rcfd->File_Dict->metadata_filename);
            rcfd->File_Dict->metadata_filename              = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->metadata_filename, value);
        }
        else if(strcmp(property, "tracer_start_position_filename") == 0){

            free(rcfd->File_Dict->tracer_start_position_filename);
            rcfd->File_Dict->tracer_start_position_filename = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->tracer_start_position_filename, value);
        }
        else if(strcmp(property, "C2C_filename") == 0){

            free(rcfd->File_Dict->C2C_filename);
            rcfd->File_Dict->C2C_filename                   = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->C2C_filename, value);
        }
        else if(strcmp(property, "Prep_Transscript_filename") == 0){

            free(rcfd->File_Dict->Prep_Transscript_filename);
            rcfd->File_Dict->Prep_Transscript_filename      = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Prep_Transscript_filename, value);
        }
        else if(strcmp(property, "Run_Transscript_filename") == 0){

            free(rcfd->File_Dict->Run_Transscript_filename);
            rcfd->File_Dict->Run_Transscript_filename       = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Run_Transscript_filename, value);
        }
        else if(strcmp(property, "Norm_filename") == 0){

            free(rcfd->File_Dict->Norm_filename);
            rcfd->File_Dict->Norm_filename                  = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Norm_filename, value);
        }
        else if(strcmp(property, "vof_filename") == 0){

            free(rcfd->File_Dict->vof_filename);
            rcfd->File_Dict->vof_filename                   = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->vof_filename, value);
        }
        else if(strcmp(property, "local_diff_filename") == 0){

            free(rcfd->File_Dict->local_diff_filename);
            rcfd->File_Dict->local_diff_filename            = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->local_diff_filename, value);
        }
        else if(strcmp(property, "rec_user_filename") == 0){

            free(rcfd->File_Dict->rec_user_filename);
            rcfd->File_Dict->rec_user_filename            = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->rec_user_filename, value);
        }
        else if(strcmp(property, "Jump_filename") == 0){

            free(rcfd->File_Dict->Jump_filename);
            rcfd->File_Dict->Jump_filename                  = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Jump_filename, value);
        }
        else if(strcmp(property, "Matrix_filename") == 0){

            free(rcfd->File_Dict->Matrix_filename);
            rcfd->File_Dict->Matrix_filename                = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Matrix_filename, value);
        }
        else if(strcmp(property, "Balance_filename") == 0){

            free(rcfd->File_Dict->Balance_filename);
            rcfd->File_Dict->Balance_filename               = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Balance_filename, value);
        }
        else if(strcmp(property, "Rec_Frames_filename") == 0){

            free(rcfd->File_Dict->Rec_Frames_filename);
            rcfd->File_Dict->Rec_Frames_filename            = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Rec_Frames_filename, value);
        }
        else if(strcmp(property, "Topo_filename") == 0){

            free(rcfd->File_Dict->Topo_filename);
            rcfd->File_Dict->Topo_filename                  = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Topo_filename, value);
        }
        else if(strcmp(property, "Dump_filename") == 0){

            free(rcfd->File_Dict->Dump_filename);
            rcfd->File_Dict->Dump_filename                  = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Dump_filename, value);
        }
        else if(strcmp(property, "Probe_filename") == 0){

            free(rcfd->File_Dict->Probe_filename);
            rcfd->File_Dict->Probe_filename                 = (char*)malloc((strlen(value)+1)*sizeof(char));
            strcpy(rcfd->File_Dict->Probe_filename, value);
        }
        else{

            report_dict_unknown_property(rcfd, "file", property, "string");
            return -1;
        }

        return 0;
    }

    return -1;
}

/**
 * @brief set string property in prep dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_prep_dict_property_s(void *handle, const char *property, const char *value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_PREP_DICT)){

            report_dict_access_denied(rcfd, "prep", property, "string");
            return -1;
        }

        if(strcmp(property, "tracer_format") == 0){

            if(strcmp(value, "force_guided") == 0){

                rcfd->Prep_Dict->tracer_format = RCFD_TRACER_FORCE_GUIDED;
            }
            else if(strcmp(value, "velocity_guided") == 0){

                rcfd->Prep_Dict->tracer_format = RCFD_TRACER_VELOCITY_GUIDED;
            }
            else{

                report_dict_invalid_value_s(rcfd, "prep", property, value);
                return -1;
            }

            return 0;
        }
        else if(strcmp(property, "interpolation_format") == 0){

            if(strcmp(value, "interpolation_nearest_aligned_neighbor_cells") == 0){

                rcfd->Prep_Dict->interpolation_format = RCFD_INTERP_NEAREST_ALIGNED_NEIGHBOR_CELLS;
            }
            else{

                report_dict_invalid_value_s(rcfd, "prep", property, value);
                return -1;
            }

            return 0;
        }
        else{

            report_dict_unknown_property(rcfd, "prep", property, "string");
            return -1;
        }
    }
#endif

    return -1;
}

/**
 * @brief set string property in norm dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_norm_dict_property_s(void *handle, const char *property, const char *value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_NORM_DICT)){

            report_dict_access_denied(rcfd, "norm", property, "string");
            return -1;
        }

        if(strcmp(property, "format") == 0){

            if(strcmp(value, "default") == 0){

                rcfd->Norm_Dict->format = RCFD_NORM_DEFAULT;
            }
            else if(strcmp(value, "user") == 0){

                rcfd->Norm_Dict->format = RCFD_NORM_USER;
            }
            else{

                report_dict_invalid_value_s(rcfd, "norm", property, value);
                return -1;
            }
        }
        else{

            report_dict_unknown_property(rcfd, "norm", property, "string");
            return -1;
        }

        return 0;
    }
#endif

    return -1;
}

/**
 * @brief set string property in rec dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_rec_dict_property_s(void *handle, const char *property, const char *value)
{
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_REC_DICT)){

            report_dict_access_denied(rcfd, "rec", property, "string");
            return -1;
        }

        if(strcmp(property, "format") == 0){

            if(strcmp(value, "quarter_jumps") == 0){

                rcfd->Rec_Dict->format = RCFD_REC_QUARTER_JUMPS;
            }
            else if(strcmp(value, "off_diagonal_band") == 0){

                rcfd->Rec_Dict->format = RCFD_REC_OFF_DIAGONAL_BAND;
            }
            else if(strcmp(value, "replay") == 0){

                rcfd->Rec_Dict->format = RCFD_REC_REPLAY;
            }
            else{

                report_dict_invalid_value_s(rcfd, "rec", property, value);
                return -1;
            }

            return 0;
        }
        else{

            report_dict_unknown_property(rcfd, "rec", property, "string");
            return -1;
        }
    }

    return -1;
}

/**
 * @brief set string property in data dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_data_dict_property_s(void *handle, int i_phase, int i_data, const char *property, const char *value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_DATA_DICT)){

            report_dict_access_denied(rcfd, "data", property, "string");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "data", property, "string", i_phase);
            return -1;
        }

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

            report_dict_invalid_data(rcfd, "data", property, "string", i_data);
            return -1;
        }


        if(strcmp(property, "type") == 0){

            if(strcmp(value, "generic") == 0){

                rcfd->Data_Dict[i_phase][i_data].type = RCFD_DATA_GENERIC;
            }
            else if(strcmp(value, "concentration") == 0){

                rcfd->Data_Dict[i_phase][i_data].type = RCFD_DATA_CONCENTRATION;
            }
            else if(strcmp(value, "binary") == 0){

                rcfd->Data_Dict[i_phase][i_data].type = RCFD_DATA_BINARY;
            }
            else if(strcmp(value, "temperature") == 0){

                rcfd->Data_Dict[i_phase][i_data].type = RCFD_DATA_TEMPERATURE;
            }
            else{

                report_dict_invalid_value_s(rcfd, "data", property, value);
                return -1;
            }

            return 0;
        }
        else{

            report_dict_unknown_property(rcfd, "data", property, "string");
            return -1;
        }
    }
#endif

    return -1;
}

/**
 * @brief set string property in balance dict
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   specifies the phase if the property is per-phase
 * @param i_data    specifies the data index if the property is per-data
 * @param property  name of the property to be set
 * @param value     string value to assign to the property
 * @retval -1 Error
 * @retval 0 OK
 */
int set_balance_dict_property_s(void *handle, int i_phase, int i_data, const char *property, const char *value)
{
#if RP_NODE
    if(handle){

        rCFD_type *rcfd = (rCFD_type*)handle;

        if(!(rcfd->dict_write_permission_mask & WRITE_BALANCE_DICT)){

            report_dict_access_denied(rcfd, "balance", property, "string");
            return -1;
        }

        if(i_phase < 0 || i_phase >= rcfd->Solver_Dict->number_of_phases){

            report_dict_invalid_phase(rcfd, "balance", property, "string", i_phase);
            return -1;
        }

        if(i_data < 0 || i_data >= rcfd->Phase_Dict[i_phase].number_of_data){

            report_dict_invalid_data(rcfd, "balance", property, "string", i_data);
            return -1;
        }


        if(strcmp(property, "type") == 0){

            if(strcmp(value, "off") == 0){

                rcfd->Balance_Dict[i_phase][i_data].type = RCFD_BALANCE_OFF;
            }
            else if(strcmp(value, "global") == 0){

                rcfd->Balance_Dict[i_phase][i_data].type = RCFD_BALANCE_GLOBAL;
            }
            else if(strcmp(value, "local") == 0){

                rcfd->Balance_Dict[i_phase][i_data].type = RCFD_BALANCE_LOCAL;
            }
            else{

                report_dict_invalid_value_s(rcfd, "balance", property, value);
                return -1;
            }

            return 0;
        }
        else{

            report_dict_unknown_property(rcfd, "balance", property, "string");
            return -1;
        }
    }
#endif

    return -1;
}

#endif // RCFD_DICT_H
