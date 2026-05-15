/******************************************************************************
 * @copyright   (C) 2026
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_USER
#define RCFD_USER

#include "rCFD_library.h"

/*****************************************************************************/
/* user names, definitions, global vars & auxiliary functions                */
/*****************************************************************************/

// names of phases
enum{
    WATER,
    PHASE_COUNT
};

/*****************************************************************************/
/* user dictionaries                                                         */
/*****************************************************************************/

void user_set_solver_dict(void *handle)
{
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "report_priority", RCFD_REPORT_INFO);
    rcfd_set_dict_property_r(handle, "Solver_Dict", -1, -1, -1, "global_time_step", 0.05);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "time_steps_per_monitoring_interval", 20);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_frames", 200);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "shifts_from_displacement_fields_on", 1);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "interpolation_on", 1);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "local_diff_on", 0);
    rcfd_set_dict_property_s(handle, "Solver_Dict", -1, -1, -1, "local_diff_format", "field_based");
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "moving_geometry_on", 1);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "single_phase_vof", 1);
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "single_phase_vof_id", 0);
}

void user_set_phase_dict(void *handle)
{
#if RP_NODE
    rcfd_set_dict_property_r(handle, "Phase_Dict", -1, WATER, -1, "density", 1000.0);
#endif
}

void user_set_prep_dict(void *handle)
{
#if RP_NODE
    rcfd_set_dict_property_i(handle, "Prep_Dict", -1, -1, -1, "interpolation_max_number_of_aligned_neigh_cells", 200);
    rcfd_set_dict_property_i(handle, "Prep_Dict", -1, -1, -1, "interpolation_multiply_shift_weights_with_vof_on", 1);
#endif
}

void user_set_rec_dict(void *handle)
{
    rcfd_set_dict_property_s(handle, "Rec_Dict", -1, -1, -1, "format", "replay");
}

void user_set_cell_dict(void *handle, const short i_layer)
{
#if RP_NODE
    fluent_set_cell_dict(handle, i_layer);

    rcfd_set_dict_property_i(handle, "Cell_Dict", i_layer, -1, -1, "number_of_user_vars", 5);
    rcfd_set_dict_property_r(handle, "Cell_Dict", i_layer, -1, -1, "next_neigh_cell_skin_factor", 1.5);
#endif
}

void user_set_face_dict(void *handle, const short i_layer)
{
#if RP_NODE
    fluent_set_face_dict(handle, i_layer);
#endif
}

/*****************************************************************************/
/* register user functions                                                   */
/*****************************************************************************/

void user_register_functions(void *handle)
{
    REGISTER_RCFD_UDF( user_set_solver_dict );
    REGISTER_RCFD_UDF( user_set_phase_dict );
    REGISTER_RCFD_UDF( user_set_prep_dict );
    REGISTER_RCFD_UDF( user_set_rec_dict );
    REGISTER_RCFD_UDF( user_set_cell_dict );
    REGISTER_RCFD_UDF( user_set_face_dict );
}

#endif
