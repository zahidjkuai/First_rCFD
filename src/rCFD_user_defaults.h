/******************************************************************************
 * @internal
 * @file        rCFD_user_defaults.h
 * @copyright   (C) 2022
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_USER_DEFAULT_H
#define RCFD_USER_DEFAULT_H

#include "rCFD_macros.h"
#include "rCFD_types.h"


    void user_set_solver_dict_default(void *handle){}

    void user_set_file_dict_default(void *handle){}

    void user_set_phase_dict_default(void *handle){}

    void user_set_prep_dict_default(void *handle){}

    void user_set_norm_dict_default(void *handle){}

    void user_set_rec_dict_default(void *handle){}

    void user_set_data_dict_default(void *handle){}

    void user_set_balance_dict_default(void *handle){}

    void user_set_dump_dict_default(void *handle){}

    void user_set_topo_dict_default(void *handle){}

    void user_set_cell_dict_default(void *handle, const short i_layer){}

    void user_set_face_dict_default(void *handle, const short i_layer){}

    void user_set_node_dict_default(void *handle, const short i_layer){}

    void user_pre_proc_default(void *handle){}

    void user_init_cells_default(void *handle, const short i_layer){}

    void user_init_faces_default(void *handle, const short i_layer){}

    void user_init_cell_hosts_default(void *handle, const short i_layer){}

    void user_init_nodes_default(void *handle, const short i_layer){}

    void user_init_data_default(void *handle, const short i_layer){}

    short user_set_layer_default(void *handle, const short current_layer){return 0;}

    void user_access_data_before_phase_loop_default(void *handle){}

    void user_access_data_before_shift_default(void *handle, const short i_phase){}

    void user_access_data_after_swap_default(void *handle, const short i_phase){}

    void user_post_default(void *handle){}

    void user_set_norm_default(void *handle){}

    void user_set_rec_vars_default(void *handle, const short i_layer){}


    void user_register_default_functions(rCFD_type* rcfd)
    {
        REGISTER_RCFD_UDF_DEFAULT( user_set_solver_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_file_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_phase_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_prep_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_norm_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_rec_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_data_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_balance_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_dump_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_topo_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_cell_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_face_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_set_node_dict );
        REGISTER_RCFD_UDF_DEFAULT( user_pre_proc );
        REGISTER_RCFD_UDF_DEFAULT( user_init_cells );
        REGISTER_RCFD_UDF_DEFAULT( user_init_faces );
        REGISTER_RCFD_UDF_DEFAULT( user_init_cell_hosts );
        REGISTER_RCFD_UDF_DEFAULT( user_init_nodes );
        REGISTER_RCFD_UDF_DEFAULT( user_init_data );
        REGISTER_RCFD_UDF_DEFAULT( user_set_layer );
        REGISTER_RCFD_UDF_DEFAULT( user_access_data_before_phase_loop );
        REGISTER_RCFD_UDF_DEFAULT( user_access_data_before_shift );
        REGISTER_RCFD_UDF_DEFAULT( user_access_data_after_swap );
        REGISTER_RCFD_UDF_DEFAULT( user_post );
        REGISTER_RCFD_UDF_DEFAULT( user_set_norm );
        REGISTER_RCFD_UDF_DEFAULT( user_set_rec_vars );
    }


#endif

