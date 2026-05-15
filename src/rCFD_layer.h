/******************************************************************************
 * @internal
 * @file        rCFD_layer.h
 * @copyright   (C) 2022
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_LAYERS
#define RCFD_LAYERS

#include <math.h>
#include "rCFD_macros.h"
#include "rCFD_types.h"

/**
 * @brief set all pointers of the Cell struct of the given layer to NULL
 *
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   cell layer to operate on
 */
void layer_default(rCFD_type* rcfd, const short i_layer)
{
#if RP_NODE
    _C(i_layer).x                           = NULL;
    _C(i_layer).volume                      = NULL;
    _C(i_layer).grid_spacing                = NULL;

    _C(i_layer).number_of_neigh_cells       = NULL;
    _C(i_layer).number_of_next_neigh_cells  = NULL;
    _C(i_layer).neigh_cell                  = NULL;
    _C(i_layer).next_neigh_cell             = NULL;

    _C(i_layer).average_velocity            = NULL;
    _C(i_layer).crossing_time               = NULL;

    _C(i_layer).hit_by_other_cell           = NULL;

    _C(i_layer).weight_after_shift          = NULL;
    _C(i_layer).weight_after_swap           = NULL;

    _C(i_layer).vof                         = NULL;
    _C(i_layer).vof_changed                 = NULL;

    _C(i_layer).data                        = NULL;
    _C(i_layer).data_shift                  = NULL;
    _C(i_layer).data_swap                   = NULL;
    _C(i_layer).data_shift_updated          = NULL;

    _C(i_layer).local_diff                  = NULL;

    _C(i_layer).drift_exchange              = NULL;

    _C(i_layer).cfd_displ                   = NULL;
    _C(i_layer).cfd_displ_diff              = NULL;

    _C(i_layer).tmp                         = NULL;

    _C(i_layer).user                        = NULL;
    _C(i_layer).rec_user                    = NULL;

    _C(i_layer).marked                      = NULL;
    _C(i_layer).parent_cell                 = NULL;
    _C(i_layer).number_of_children          = NULL;
    _C(i_layer).child_index                 = NULL;

    _C(i_layer).type                        = NULL;
    _C(i_layer).number_of_nodes             = NULL;
    _C(i_layer).nodes                       = NULL;

    _C(i_layer).number_of_polyhedra         = 0;
    _C(i_layer).number_of_poly_faces        = NULL;
    _C(i_layer).number_of_poly_face_nodes   = NULL;
    _C(i_layer).poly_nodes                  = NULL;

#endif
}

void layer_allocate(rCFD_type* rcfd, const short i_layer)
{
#if RP_NODE

    int i_phase;

    int number_of_vof_frames;

    _C(i_layer).x = malloc_r_2d(_CELL_DICT(i_layer).number_of_cells, 3);

    _C(i_layer).volume = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));
    _C(i_layer).grid_spacing = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));

    _C(i_layer).average_velocity = malloc_r_2d(rcfd->Solver_Dict->number_of_phases, _CELL_DICT(i_layer).number_of_cells);
    _C(i_layer).crossing_time    = malloc_r_2d(rcfd->Solver_Dict->number_of_phases, _CELL_DICT(i_layer).number_of_cells);

    _C(i_layer).hit_by_other_cell =   (short*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(short));

    _C(i_layer).weight_after_shift =  (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));
    _C(i_layer).weight_after_swap =   (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));

    if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

        number_of_vof_frames = 1;
    }
    else{

        number_of_vof_frames = rcfd->Solver_Dict->number_of_frames;
    }

    _C(i_layer).vof = malloc_r_3d(number_of_vof_frames, _CELL_DICT(i_layer).number_of_cells, rcfd->Solver_Dict->number_of_phases);

    if(rcfd->Rec_Dict->adapt_vof_stitching_on){

        _C(i_layer).vof_changed = malloc_r_2d(rcfd->Solver_Dict->number_of_phases, _CELL_DICT(i_layer).number_of_cells);
    }
    else{

        _C(i_layer).vof_changed = NULL;
    }

    _C(i_layer).data = (double***)malloc(rcfd->Solver_Dict->number_of_phases * sizeof(double**));

    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

        _C(i_layer).data[i_phase] = malloc_r_2d(_CELL_DICT(i_layer).number_of_cells, rcfd->Phase_Dict[i_phase].number_of_data);
    }

    _C(i_layer).data_shift = malloc_r_2d(_CELL_DICT(i_layer).number_of_cells, rcfd->Solver_Dict->max_phase_data_count);
    _C(i_layer).data_swap  = malloc_r_2d(_CELL_DICT(i_layer).number_of_cells, rcfd->Solver_Dict->max_phase_data_count);

    _C(i_layer).data_shift_updated = malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(short));

    if(rcfd->Solver_Dict->local_diff_on){

        _C(i_layer).local_diff = malloc_r_3d(rcfd->Solver_Dict->number_of_frames, rcfd->Solver_Dict->number_of_phases, _CELL_DICT(i_layer).number_of_cells);
    }
    else{

        _C(i_layer).local_diff = NULL;
    }

    if(rcfd->Solver_Dict->data_drifting_on){

        _C(i_layer).drift_exchange = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));
    }
    else{

        _C(i_layer).drift_exchange = NULL;
    }

    if((rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION) && rcfd->Solver_Dict->shifts_from_displacement_fields_on){

        _C(i_layer).cfd_displ = malloc_r_3d(rcfd->Solver_Dict->number_of_phases, _CELL_DICT(i_layer).number_of_cells, 3);

        if(rcfd->Solver_Dict->local_diff_on && (rcfd->Solver_Dict->local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED)){

            _C(i_layer).cfd_displ_diff  = malloc_r_3d(rcfd->Solver_Dict->number_of_phases, _CELL_DICT(i_layer).number_of_cells, 6);
        }
        else{

            _C(i_layer).cfd_displ_diff  = NULL;
        }
    }
    else{

        _C(i_layer).cfd_displ = NULL;
        _C(i_layer).cfd_displ_diff  = NULL;
    }

    _C(i_layer).tmp = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));

    if(_CELL_DICT(i_layer).number_of_user_vars > 0){

        _C(i_layer).user = malloc_r_2d(_CELL_DICT(i_layer).number_of_cells, _CELL_DICT(i_layer).number_of_user_vars);
    }
    else{

        _C(i_layer).user = NULL;
    }

    if(_CELL_DICT(i_layer).number_of_rec_user_vars > 0){

        if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

            _C(i_layer).rec_user = malloc_r_3d(1, _CELL_DICT(i_layer).number_of_cells, _CELL_DICT(i_layer).number_of_rec_user_vars);
        }
        else{

            _C(i_layer).rec_user = malloc_r_3d(rcfd->Solver_Dict->number_of_frames, _CELL_DICT(i_layer).number_of_cells, _CELL_DICT(i_layer).number_of_rec_user_vars);
        }
    }
    else{

        _C(i_layer).rec_user = NULL;
    }

    /* multi-layer vars */

    _C(i_layer).marked =                 (short*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(short));

    if((i_layer + 1) < rcfd->Solver_Dict->number_of_layers){

        _C(i_layer).parent_cell =        (int*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int));
    }
    else{

        _C(i_layer).parent_cell =        NULL;
    }

    if(i_layer > 0){

        _C(i_layer).number_of_children =    (short*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(short));
        _C(i_layer).child_index =           (int**) malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int*));      /* child_index[i_cell] will be allocated later */
    }
    else{

        _C(i_layer).number_of_children =    NULL;
        _C(i_layer).child_index =           NULL;
    }

    /* nodes */
    if(_NODE_DICT(i_layer).number_of_nodes > 0){

        _C(i_layer).type            = (int*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int));
        _C(i_layer).number_of_nodes = (int*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int));
        _C(i_layer).nodes           = malloc_i_2d(_CELL_DICT(i_layer).number_of_cells, RCFD_MAX_NODES_PER_CELL);
        memset(_C(i_layer).number_of_nodes, 0, _CELL_DICT(i_layer).number_of_cells * sizeof(int));
    }
    else{

        _C(i_layer).type            = NULL;
        _C(i_layer).number_of_nodes = NULL;
        _C(i_layer).nodes           = NULL;
    }

    /* memory for polyhedron nodes gets allocated upon topo set/read if needed */
    _C(i_layer).number_of_polyhedra         = 0;
    _C(i_layer).number_of_poly_faces        = NULL;
    _C(i_layer).number_of_poly_face_nodes   = NULL;
    _C(i_layer).number_of_poly_nodes        = 0;
    _C(i_layer).poly_nodes                  = NULL;
#endif
}

void layer_map_parent(rCFD_type* rcfd, const short i_layer)
{
#if RP_NODE
    short debug_this_code = 0;

    if((i_layer + 1) >= rcfd->Solver_Dict->number_of_layers){

        report_warning(rcfd, RCFD_REPORT_CATEGORY_NOTSET,
                       "[%d] Tried to map to non-existing parent of layer %d", myid, i_layer);
        return;
    }

    int i_cell, i_data, i_phase;

    int upper_cell;

    /* initialize upper-layer data */
    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                _C(i_layer + 1).data[i_phase][i_cell][i_data] = 0.0;
            }
        }
    }

    /* accumulate upper-layer data */
    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            upper_cell = _C(i_layer).parent_cell[i_cell];

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                _C(i_layer + 1).data[i_phase][upper_cell][i_data] += _C(i_layer).data[i_phase][i_cell][i_data];
            }
        }
    }

    /* average upper-layer data */
    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                _C(i_layer + 1).data[i_phase][i_cell][i_data] /= (double) _C(i_layer + 1).number_of_children[i_cell];
            }
        }
    }

    if(debug_this_code){

        i_data = 0;
        i_phase = 0;

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer + 1).number_of_cells; i_cell++){

            _C(i_layer + 1).data[i_phase][i_cell][i_data] = 0.0;
        }

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            upper_cell = _C(i_layer).parent_cell[i_cell];

            _C(i_layer + 1).data[i_phase][upper_cell][i_data] += 1.0;
        }
    }
#endif
}

void layer_map_children(rCFD_type* rcfd, const short i_layer)
{
#if RP_NODE
    short debug_this_code = 0;

    if(i_layer < 1){

        report_warning(rcfd, RCFD_REPORT_CATEGORY_NOTSET,
                       "[%d] Tried to map to non-existing children of layer %d", myid, i_layer);
        return;
    }

    int i_cell, i_data, i_phase, i_child;

    int lower_cell;

    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                for(i_child = 0; i_child < _C(i_layer).number_of_children[i_cell]; i_child++){

                    lower_cell = _C(i_layer).child_index[i_cell][i_child];

                    _C(i_layer - 1).data[i_phase][lower_cell][i_data] = _C(i_layer).data[i_phase][i_cell][i_data];
                }
            }
        }
    }

    if(debug_this_code){

        i_data = 0;
        i_phase = 0;

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer - 1).number_of_cells; i_cell++){

            _C(i_layer - 1).data[i_phase][i_cell][i_data] = 0.0;
        }

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            for(i_child = 0; i_child < _C(i_layer).number_of_children[i_cell]; i_child++){

                lower_cell = _C(i_layer).child_index[i_cell][i_child];

                _C(i_layer - 1).data[i_phase][lower_cell][i_data] = _C(i_layer).data[i_phase][i_cell][i_data];
            }
        }
    }
#endif
}

void layer_map_from_to(rCFD_type* rcfd, const short from_layer, const short to_layer)
{
#if RP_NODE

    if((to_layer >= rcfd->Solver_Dict->number_of_layers) || (to_layer < 0)){

        report_warning(rcfd, RCFD_REPORT_CATEGORY_NOTSET,
                       "[%d] Tried to map to non-existing layer %d", myid, to_layer);
        return;
    }

    if((from_layer >= rcfd->Solver_Dict->number_of_layers) || (from_layer < 0)){

        report_warning(rcfd, RCFD_REPORT_CATEGORY_NOTSET,
                       "[%d] Unreasonable mapping layer %d", myid, from_layer);
        return;
    }

    int i_layer;

    if(from_layer > to_layer){

        i_layer = from_layer;

        while(i_layer > to_layer){

            layer_map_children(rcfd, i_layer);

            i_layer--;
        }
    }
    else if(from_layer < to_layer){

        i_layer = from_layer;

        while(i_layer < to_layer){

            layer_map_parent(rcfd, i_layer);

            i_layer++;
        }
    }
#endif
}

void layer_map_data_to_new_vof(rCFD_type* rcfd)
{
#if RP_NODE

#define LOCAL_DEBUG_ON  0

#if 1   // local vars
    int     i_layer, i_phase, i_cell, i_frame, i_frame_prev, i_data, i_adapt, i_face;
    int     c0, c1;
    double  data_before_stitching, data_after_stitching, available_data_change, relax_adaption;
    double  *data_min_of_neighbors = NULL, *data_max_of_neighbors = NULL;
#endif

    if((rcfd->Solver->global_run_counter > 0) && ((rcfd->Rec->frame_in_sequence == 0) || (rcfd->Rec->jumped_at_last_frame))){

        i_layer = rcfd->Solver->current_layer;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            i_frame = rcfd->Solver->current_frame;
            i_frame_prev = rcfd->Solver->prev_frame;

            /* calc. _C(i_layer).vof_changed */
            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){     // all cells, because of subsequent face loop

                _C(i_layer).vof_changed[i_phase][i_cell] = _C(i_layer).vof[i_frame][i_cell][i_phase] - _C(i_layer).vof[i_frame_prev][i_cell][i_phase];
            }

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                /* sum up data before/after stitching */
                {
                    data_before_stitching = 0.0;
                    data_after_stitching = 0.0;

                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                        data_before_stitching +=  _C(i_layer).data[i_phase][i_cell][i_data]
                                                * _C(i_layer).vof[i_frame_prev][i_cell][i_phase]
                                                * _C(i_layer).volume[i_cell]
                                                * rcfd->Phase_Dict[i_phase].density;

                        data_after_stitching +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                * _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                * _C(i_layer).volume[i_cell]
                                                * rcfd->Phase_Dict[i_phase].density;
                    }

                    data_before_stitching = PRF_GRSUM1(data_before_stitching);
                    data_after_stitching = PRF_GRSUM1(data_after_stitching);

                    if(LOCAL_DEBUG_ON){

                        report_debug0(rcfd, RCFD_REPORT_CATEGORY_NOTSET,
                                      "sequence stitching data before/after initial vof patch: %e/%e\n",
                                      data_before_stitching, data_after_stitching);
                    }
                }

                /* if data gain, reduce data in vof_gaining cells */
                if(data_after_stitching > data_before_stitching){

                    /* calc available data change */
                    {
                        available_data_change = 0.0;

                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                            if(_C(i_layer).vof_changed[i_phase][i_cell] > 0.0){

                                available_data_change +=  _C(i_layer).data[i_phase][i_cell][i_data]
                                                        * _C(i_layer).vof_changed[i_phase][i_cell]
                                                        * _C(i_layer).volume[i_cell]
                                                        * rcfd->Phase_Dict[i_phase].density;
                            }
                        }

                        available_data_change = PRF_GRSUM1(available_data_change);
                    }

                    /* adapt data */
                    {
                        if(available_data_change > 0.0){

                            relax_adaption =  (data_after_stitching - data_before_stitching) / available_data_change;

                            if(relax_adaption > 1.0){

                                relax_adaption = 1.0;
                            }

                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                                if(_C(i_layer).vof_changed[i_phase][i_cell] > 0.0){

                                    _C(i_layer).data[i_phase][i_cell][i_data] *=  1.0
                                                                                + relax_adaption
                                                                                * ((_C(i_layer).vof[i_frame][i_cell][i_phase] - _C(i_layer).vof_changed[i_phase][i_cell])
                                                                                   / _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                                                   - 1.0);
                                }
                            }
                        }
                    }

                    /* calc. mixing_after_adaption */
                    {
                        data_after_stitching = 0.0;

                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                            data_after_stitching +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                    * _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                    * _C(i_layer).volume[i_cell]
                                                    * rcfd->Phase_Dict[i_phase].density;
                        }

                        data_after_stitching = PRF_GRSUM1(data_after_stitching);
                    }
                }
                /* if data loss, increase data in vof_gaining cells by neighboring data values */
                else if(data_after_stitching < (1.0 - rcfd->Balance_Dict[i_phase][i_data].accuracy_level) * data_before_stitching){

                    /* allocate  min/max of neighbors */
                    {
                        data_min_of_neighbors = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));
                        data_max_of_neighbors = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));
                    }

                    i_adapt = 0;

                    while(   (i_adapt < rcfd->Rec_Dict->number_of_adapt_vof_loops)
                          && (fabs(data_after_stitching - data_before_stitching)
                              > rcfd->Balance_Dict[i_phase][i_data].accuracy_level * data_before_stitching)){

                        /* calc. min/max of neighbors */
                        {
                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                                data_min_of_neighbors[i_cell] = 1.0e10;
                                data_max_of_neighbors[i_cell] = -1.0e10;
                            }

                            for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                                c0 = _F(i_layer).c0[i_face];
                                c1 = _F(i_layer).c1[i_face];

                                if(_C(i_layer).data[i_phase][c1][i_data] > _C(i_layer).data[i_phase][c0][i_data]){

                                    if(_C(i_layer).data[i_phase][c1][i_data] > data_max_of_neighbors[c0]){

                                        data_max_of_neighbors[c0] = _C(i_layer).data[i_phase][c1][i_data];
                                    }

                                    if(_C(i_layer).data[i_phase][c0][i_data] < data_min_of_neighbors[c1]){

                                        data_min_of_neighbors[c1] = _C(i_layer).data[i_phase][c0][i_data];
                                    }
                                }
                                else if(_C(i_layer).data[i_phase][c1][i_data] < _C(i_layer).data[i_phase][c0][i_data]){

                                    if(_C(i_layer).data[i_phase][c0][i_data] > data_max_of_neighbors[c1]){

                                        data_max_of_neighbors[c1] = _C(i_layer).data[i_phase][c0][i_data];
                                    }

                                    if(_C(i_layer).data[i_phase][c1][i_data] < data_min_of_neighbors[c0]){

                                        data_min_of_neighbors[c0] = _C(i_layer).data[i_phase][c1][i_data];
                                    }
                                }
                                else{

                                    if(_C(i_layer).data[i_phase][c0][i_data] > data_max_of_neighbors[c1]){

                                        data_max_of_neighbors[c1] = _C(i_layer).data[i_phase][c0][i_data];
                                    }
                                    else if(_C(i_layer).data[i_phase][c0][i_data] < data_min_of_neighbors[c1]){

                                        data_min_of_neighbors[c1] = _C(i_layer).data[i_phase][c0][i_data];
                                    }

                                    if(_C(i_layer).data[i_phase][c1][i_data] > data_max_of_neighbors[c0]){

                                        data_max_of_neighbors[c0] = _C(i_layer).data[i_phase][c1][i_data];
                                    }
                                    else if(_C(i_layer).data[i_phase][c1][i_data] < data_min_of_neighbors[c0]){

                                        data_min_of_neighbors[c0] = _C(i_layer).data[i_phase][c1][i_data];
                                    }
                                }
                            }
                        }

                        /* calc available change */
                        {
                            available_data_change = 0.0;

                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                                if(_C(i_layer).vof_changed[i_phase][i_cell] > 0.0){

                                    if(data_after_stitching < data_before_stitching){

                                        available_data_change +=  (data_max_of_neighbors[i_cell] - _C(i_layer).data[i_phase][i_cell][i_data])
                                                                * _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                                * _C(i_layer).volume[i_cell]
                                                                * rcfd->Phase_Dict[i_phase].density;
                                    }
                                }
                            }

                            available_data_change = PRF_GRSUM1(available_data_change);
                        }

                        /* adapt data */
                        {
                            if(available_data_change > 0.0){

                                relax_adaption =  fabs(data_before_stitching - data_after_stitching) / available_data_change;

                                if(relax_adaption > 1.0){

                                    relax_adaption = 1.0;
                                }

                                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                                    if(_C(i_layer).vof_changed[i_phase][i_cell] > 0.0){

                                        if(data_after_stitching < data_before_stitching){

                                            _C(i_layer).data[i_phase][i_cell][i_data] =   _C(i_layer).data[i_phase][i_cell][i_data]
                                                                                        + relax_adaption
                                                                                        * (data_max_of_neighbors[i_cell]
                                                                                           - _C(i_layer).data[i_phase][i_cell][i_data]);
                                        }
                                    }
                                }
                            }
                        }

                        /* calc. mixing_after_stitching */
                        {
                            data_after_stitching = 0.0;

                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                                data_after_stitching +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                        * _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                        * _C(i_layer).volume[i_cell]
                                                        * rcfd->Phase_Dict[i_phase].density;
                            }

                            data_after_stitching = PRF_GRSUM1(data_after_stitching);
                        }

                        i_adapt++;
                    }

                    /* free local vars */
                    {
                        free(data_min_of_neighbors);
                        free(data_max_of_neighbors);
                    }
                }

                /* node-0 checks for warnings & message */
                if(myid == 0){

                    if( fabs(data_after_stitching - data_before_stitching)
                        > rcfd->Balance_Dict[i_phase][i_data].accuracy_level * data_before_stitching){

                        //Solver.warnings[code_run_update_frame]++; // to be implemented
                    }
                }
            }
        }
    }

#undef LOCAL_DEBUG_ON

#endif
}

#endif
