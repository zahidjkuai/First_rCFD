/******************************************************************************
 * @internal
 * @file        rCFD_prepare.h
 * @copyright   (C) 2025
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 * @authors     Daniel Queteschiner
 * @authors     Hannes Lumetzberger
 *****************************************************************************/

#ifndef RCFD_PREPARE_H
#define RCFD_PREPARE_H

#include <float.h>
#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_parallel.h"
#include "rCFD_free.h"

#if !defined(RCFD_STANDALONE)
void fluent_debug_test_displacement(double **test_displ, int post);
#endif

/**
 * @brief adds single cell-to-cell shift to global C2C shift list (helper fct for prep_shift_from_displacement_fields)
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   current layer index
 * @param i_phase   current phase index
 * @param c0, noce0, c1, node1, w0, c0_coords entities of shift (which should be added)
 * @return 0 if success or -1 if error
 */
int prep_shift_add_to_list(rCFD_type* rcfd, const int i_layer, const int i_phase, int c0, int node0, double w0, int c1, int node1, double *c0_coords)
{
#if RP_NODE

    int     i_dim;

    int     i_shift =          rcfd->C2C_Prep[i_phase].valid_shifts_count;
    int     max_shifts_count = rcfd->C2C_Prep[i_phase].shifts_count;

    if((i_shift >= 0) && (i_shift < max_shifts_count)){

        rcfd->C2C_Prep[i_phase].c0   [i_shift] = c0;
        rcfd->C2C_Prep[i_phase].node0[i_shift] = node0;
        rcfd->C2C_Prep[i_phase].w0   [i_shift] = w0;
        rcfd->C2C_Prep[i_phase].c1   [i_shift] = c1;
        rcfd->C2C_Prep[i_phase].node1[i_shift] = node1;

        for(i_dim = 0; i_dim < 3; i_dim++){

            rcfd->C2C_Prep[i_phase].c0_coords[i_shift][i_dim] = c0_coords[i_dim];
        }

        rcfd->C2C_Prep[i_phase].valid_shifts_count = i_shift + 1;

        return 0;
    }
    else{

        report_warning(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                       "[%d] prep_shift_add_to_list: current_shift_index %d of phase %d out of range",
                       myid, i_shift, i_phase);
    }
#endif
    return -1;
}

/**
 * @brief adds single cell-to-cell shift to global C2C_Interface list (helper fct for prep_shift_from_interfaces)
 *
 * @param rcfd    pointer to previously created rCFD instance
 * @param c0, noce0, c1, node1, w0 entities of shift (which should be added)
 * @return 0 if success or -1 if error
 */
int prep_shift_add_to_interface_list(rCFD_type* rcfd, int c0, int node0, int c1, int node1, double w0)
{
#if RP_NODE

    int     i_shift =          rcfd->C2C_Interface->valid_shifts_count;
    int     max_shifts_count = rcfd->C2C_Interface->shifts_count;

    if((i_shift >= 0) && (i_shift < max_shifts_count)){

        rcfd->C2C_Interface->c0     [i_shift] = c0;
        rcfd->C2C_Interface->node0  [i_shift] = node0;
        rcfd->C2C_Interface->c1     [i_shift] = c1;
        rcfd->C2C_Interface->node1  [i_shift] = node1;
        rcfd->C2C_Interface->w0     [i_shift] = w0;

        rcfd->C2C_Interface->valid_shifts_count = i_shift + 1;

        return 0;
    }
    else{

        report_warning(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                       "[%d] prep_shift_add_to_interface_list: current_shift_index %d out of range",
                       myid, i_shift);
    }
#endif
    return -1;
}

/**
 * @brief creates per-node lists of cell-to-cell shifts across grid interfaces (only RCFD_MODE_PREPARATION)
 *
 * @param handle                    pointer to previously created rCFD instance
 * @param interface_face_centers    (flattened) array of (process-local) interface face center coordinates
 * @param interface_face_count      number of (process-local) interface faces
 * @return number of shifts created or -1 if called by host or error
 */
int prep_shift_from_interfaces(rCFD_type* rcfd,
                               double *interface_face_centers,
                               int     interface_face_count)
{
#if RP_NODE
#define DEBUG_LOCAL 0

#if 1
    int     i_dim, i_face, i_cell, i_node, i_shift, i_layer = 0;

    int     moving_geometry_on = rcfd->Solver_Dict->moving_geometry_on;

    int     shifts_on_node_count, shifts_from_oher_node_count, global_shifts_count;
    int     int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;

    int     c0, node0, c1, node1, c0_min_dist;

    int     *list_of_c0 = NULL;

    double  dist_sq, min_dist_sq, w0;
    double  interface_face_swap_diffusivity = rcfd->Solver_Dict->grid_interface_shift_weight;
    double  face_coords[3], c0_coords[3];

    double  *list_of_dist_sq = NULL;

    C2C_Shift_type shifts_on_node, global_shifts;

#endif

    // allocate and init local vars, set valid_shifts_count to zero
    {
        shift_initialize_lists(rcfd, &shifts_on_node);
        shift_initialize_lists(rcfd, &global_shifts );

        if(interface_face_count > 0){

            shift_allocate_lists(rcfd, &shifts_on_node, interface_face_count, RCFD_SHIFT_TYPE_PREPARATION);
        }

        shifts_on_node_count = 0;

        if(DEBUG_LOCAL){

            report_info0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                         "[%2d] prep_shift_from_interfaces - global_interface_face_count %d",
                         myid, PRF_GISUM1(interface_face_count));
        }
    }

    // look for interface cells c1/c0 on myid
    for(i_face = 0; i_face < interface_face_count; i_face++){

        // find c1 cell (i.e. nearest cell to interface face coords)
        {
            for(i_dim = 0; i_dim < 3; i_dim++){

                face_coords[i_dim] = interface_face_centers[3 * i_face + i_dim];
            }

            min_dist_sq = 1e10;
            c1 = -1;

            for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                dist_sq = cell2point_distance_sq(rcfd, i_layer, i_cell, face_coords);

                if(dist_sq < min_dist_sq){

                    min_dist_sq = dist_sq;
                    c1 = i_cell;
                }
            }
        }

        // adapt c1_coords to (ghost cell's) c0_coords
        {
            for(i_dim = 0; i_dim < 3; i_dim++){

                c0_coords[i_dim] = face_coords[i_dim] + (face_coords[i_dim] - _C(i_layer).x[c1][i_dim]);
            }
        }

        // look for c0 by checking all int cells of myid, if no succes fill shifts_on_node
        {
            min_dist_sq = 1e10;
            c0_min_dist = -1;

            for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                if(i_cell != c1){

                    dist_sq = cell2point_distance_sq(rcfd, i_layer, i_cell, c0_coords);

                    if(dist_sq < min_dist_sq){

                        min_dist_sq = dist_sq;
                        c0_min_dist = i_cell;
                    }
                }
            }

            if(c0_min_dist != -1){

                i_shift = shifts_on_node_count;

                shifts_on_node.c0     [i_shift] = c0_min_dist;
                shifts_on_node.node0  [i_shift] = myid;
                shifts_on_node.c1     [i_shift] = c1;
                shifts_on_node.node1  [i_shift] = myid;
                shifts_on_node.dist_sq[i_shift] = min_dist_sq;

                for(i_dim = 0; i_dim < 3; i_dim++){

                    shifts_on_node.c0_coords[i_shift][i_dim] = c0_coords[i_dim];
                }

                shifts_on_node_count++;
            }
            else{

                report_warning(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                             "[%2d] prep_shift_from_interfaces - no valid c0 cell found on partition for i_face %d",
                             myid, i_face);
            }
        }
    }

    if(DEBUG_LOCAL){

        report_info(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                    "[%2d] prep_shift_from_interfaces - shifts_on_node_count %d",
                    myid, shifts_on_node_count);
    }

    global_shifts_count = PRF_GISUM1(shifts_on_node_count);

    // try to find remaining prim shifts on other nodes
    if(global_shifts_count > 0){

        // provide global_shifts lists for all nodes
        {
            shift_allocate_lists(rcfd,
                                &global_shifts,
                                 global_shifts_count,
                                 RCFD_SHIFT_TYPE_PREPARATION);

            parallel_gatherv0_shifts(rcfd,
                                    &shifts_on_node,
                                     shifts_on_node_count,
                                    &global_shifts,
                                     RCFD_SHIFT_TYPE_PREPARATION);

            parallel_bcast0_shifts(rcfd,
                                  &global_shifts,
                                   global_shifts_count,
                                   RCFD_SHIFT_TYPE_PREPARATION);
        }

        // loop all cells and try to find min_dist
        {
            shifts_on_node_count = 0;

            for(i_shift = 0; i_shift < global_shifts_count; i_shift++){

                if(global_shifts.node1[i_shift] != myid){   // inter-node shift

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        c0_coords[i_dim] = global_shifts.c0_coords[i_shift][i_dim];
                    }

                    if((moving_geometry_on) ||
                       (is_point_inside_box(c0_coords, _C(i_layer).x_min_node, _C(i_layer).x_max_node))){

                        min_dist_sq = global_shifts.dist_sq[i_shift];

                        c0_min_dist = -1;

                        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                            dist_sq = cell2point_distance_sq(rcfd, i_layer, i_cell, c0_coords);

                            if(dist_sq < min_dist_sq){

                                min_dist_sq = dist_sq;
                                c0_min_dist = i_cell;
                            }
                        }

                        if(c0_min_dist != -1){

                            global_shifts.c0     [i_shift] = c0_min_dist;
                            global_shifts.dist_sq[i_shift] = min_dist_sq;

                            shifts_on_node_count++;
                        }
                    }
                }
            }
        }

        // node-zero adapts global list and broadcasts it to all nodes
        {
            if(myid == 0){

                list_of_c0 =      (int*)malloc   (global_shifts_count * sizeof(int));
                list_of_dist_sq = (double*)malloc(global_shifts_count * sizeof(double));

                shifts_from_oher_node_count = 0;

                for(i_node = 1; i_node < compute_node_count; i_node++){

                    PRF_CRECV_INT (i_node, list_of_c0,      global_shifts_count, i_node);
                    PRF_CRECV_REAL(i_node, list_of_dist_sq, global_shifts_count, i_node);

                    for(i_shift = 0; i_shift < global_shifts_count; i_shift++){

                        if(list_of_dist_sq[i_shift] < global_shifts.dist_sq[i_shift]){

                            global_shifts.c0     [i_shift] = list_of_c0[i_shift];
                            global_shifts.node0  [i_shift] = i_node;
                            global_shifts.dist_sq[i_shift] = list_of_dist_sq[i_shift];

                            shifts_from_oher_node_count++;
                        }
                    }
                }

                free(list_of_c0     );
                free(list_of_dist_sq);

                if(DEBUG_LOCAL){

                    report_info0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                                 "[%2d] prep_shift_from_interfaces - global_shifts_count %d, shifts_from_oher_node_count %d",
                                 myid, global_shifts_count, shifts_from_oher_node_count);
                }
            }

            if(myid> 0){

                PRF_CSEND_INT (node_zero, global_shifts.c0,      global_shifts_count, myid);
                PRF_CSEND_REAL(node_zero, global_shifts.dist_sq, global_shifts_count, myid);
            }

            parallel_bcast0_shifts(rcfd,
                                  &global_shifts,
                                   global_shifts_count,
                                   RCFD_SHIFT_TYPE_PREPARATION);
        }

        // add shifts
        {
            if(global_shifts_count > 0){

                free_lists_of_shift  (rcfd, rcfd->C2C_Interface);

                shift_allocate_lists (rcfd,
                                      rcfd->C2C_Interface,
                                      global_shifts_count,
                                      RCFD_SHIFT_TYPE_PREPARATION);

                for(i_shift = 0; i_shift < global_shifts_count; i_shift++){

                    c0 =      global_shifts.c0     [i_shift];
                    node0 =   global_shifts.node0  [i_shift];
                    c1 =      global_shifts.c1     [i_shift];
                    node1 =   global_shifts.node1  [i_shift];
                    w0    =   interface_face_swap_diffusivity;

                    if(node1 == myid){

                        prep_shift_add_to_interface_list(rcfd, c0, node0, c1, node1, w0);
                    }
                }
            }
        }
    }

    // free local vars
    {
        free_lists_of_shift(rcfd, &shifts_on_node);
        free_lists_of_shift(rcfd, &global_shifts );
    }

    return rcfd->C2C_Interface->valid_shifts_count;

#undef DEBUG_LOCAL
#endif

    return -1;
}

/**
 * @brief creates per-node lists of primary cell-to-cell shifts from displacement fields (only RCFD_MODE_PREPARATION)
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   current layer index
 * @param i_phase   current phase index
 * @return number of shifts created or -1 if called by host or error
 */
int prep_shift_from_displacement_fields(rCFD_type* rcfd, int i_layer, int i_phase)
{
#if RP_NODE

#define LOCAL_DEBUG 0

#if 1
    int     i_dim, i_cell, i_cell2, i_nnb, i_node, i_shift;

    int     int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
    int     moving_geometry_on = rcfd->Solver_Dict->moving_geometry_on;
    int     prim_shifts_next_neigh_cell_count, prim_shifts_all_cell_count;
    int     prim_shifts_not_found_count, global_prim_shifts_not_found_count, prim_shifts_from_other_nodes_count;

    int     prim_shift_found, c0, node0, c1, node1, c0_nnb_min_dist, c0_nnb, c0_min_dist;

    int     *list_of_c0 = NULL;

    double  dist_sq, min_dist_sq, w0;
    double  x_displ[3];

    double  *cell_length = NULL, *c0_coords = NULL;
    double  *list_of_c0_coords = NULL, *list_of_dist = NULL;

    C2C_Shift_type prim_shifts_not_found, global_prim_shifts_not_found;

#endif

    // allocate and init local vars, set valid_shifts_count to zero, event. update bounding box
    {
        shift_initialize_lists(rcfd, &prim_shifts_not_found);
        shift_initialize_lists(rcfd, &global_prim_shifts_not_found);

        shift_allocate_lists(rcfd, &prim_shifts_not_found, int_cell_count, RCFD_SHIFT_TYPE_PREPARATION);

        prim_shifts_next_neigh_cell_count =  0;
        prim_shifts_all_cell_count =         0;
        prim_shifts_not_found_count =        0;
        prim_shifts_from_other_nodes_count = 0;

        cell_length = malloc(int_cell_count * sizeof(double));

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_length[i_cell] = pow((3.0 / (4.0 * 3.14) * _C(i_layer).volume[i_cell]), (1./3.));   // radius of equivalent sphere
        }

        c0_coords = malloc(3 * sizeof(double));

        w0 = 1.0; // weight of prim shifts is always one

        rcfd->C2C_Prep[i_phase].valid_shifts_count = 0; // list should be empty before we start creating shifts

        if(rcfd->Solver_Dict->moving_geometry_on){

            topo_init_bounding_box(rcfd, i_layer);
        }
    }

    // look for primary shift (i.e. nearest {c0 <> x_displ(c1)} pairing) on myid
    for(i_cell = 0; i_cell < int_cell_count; i_cell++){

        if(_C(i_layer).vof[0][i_cell][i_phase] > rcfd->Solver_Dict->vof_cutoff_limit){

            // init loop vars
            {
                prim_shift_found = 0;

                for(i_dim = 0; i_dim < 3; i_dim++){

                    x_displ[i_dim] = _C(i_layer).cfd_displ[i_phase][i_cell][i_dim];
                }

                c1 = i_cell;
            }

            // check nnb cells of _C(i_layer).cal_cell_id
            if(!prim_shift_found){

                c0_nnb_min_dist = c1;

                min_dist_sq = cell2point_distance_sq(rcfd, i_layer, c0_nnb_min_dist, x_displ);

                for(i_nnb = 0; i_nnb < _C(i_layer).number_of_next_neigh_cells[c1]; i_nnb++){

                    c0_nnb = _C(i_layer).next_neigh_cell[c1][i_nnb];

                    if(_C(i_layer).vof[0][c0_nnb][i_phase] > rcfd->Solver_Dict->vof_cutoff_limit){

                        dist_sq = cell2point_distance_sq(rcfd, i_layer, c0_nnb, x_displ);

                        if((c0_nnb < int_cell_count) && (dist_sq < min_dist_sq)){

                            c0_nnb_min_dist = c0_nnb;
                            min_dist_sq =     dist_sq;
                        }
                    }
                }

                if(min_dist_sq < cell_length[c0_nnb_min_dist] * cell_length[c0_nnb_min_dist]){

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        c0_coords[i_dim] = _C(i_layer).x[c0_nnb_min_dist][i_dim];
                    }

                    prep_shift_add_to_list(rcfd, i_layer, i_phase, c0_nnb_min_dist, myid, w0, c1, myid, c0_coords);

                    prim_shift_found = 1;

                    prim_shifts_next_neigh_cell_count++;
                }
            }

            // check all cells of myid
            if(!prim_shift_found){

                min_dist_sq = DBL_MAX;

                c0_min_dist = -1;

                for(i_cell2 = 0; i_cell2 < int_cell_count; i_cell2++){

                    if(_C(i_layer).vof[0][i_cell2][i_phase] > rcfd->Solver_Dict->vof_cutoff_limit){

                        dist_sq = cell2point_distance_sq(rcfd, i_layer, i_cell2, x_displ);

                        if(dist_sq < min_dist_sq){

                            min_dist_sq = dist_sq;
                            c0_min_dist = i_cell2;
                        }
                    }
                }

                if((c0_min_dist != -1) && (min_dist_sq < cell_length[c0_min_dist] * cell_length[c0_min_dist])){

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        c0_coords[i_dim] = _C(i_layer).x[c0_nnb_min_dist][i_dim];
                    }

                    prep_shift_add_to_list(rcfd, i_layer, i_phase, c0_min_dist, myid, w0, c1, myid, c0_coords);

                    prim_shift_found = 1;

                    prim_shifts_all_cell_count++;
                }
            }

            // write all other shifts into lists of other nodes
            if(!prim_shift_found){

                prim_shifts_not_found.c0     [prim_shifts_not_found_count] = c0_min_dist;
                prim_shifts_not_found.node0  [prim_shifts_not_found_count] = myid;
                prim_shifts_not_found.c1     [prim_shifts_not_found_count] = c1;
                prim_shifts_not_found.node1  [prim_shifts_not_found_count] = myid;
                prim_shifts_not_found.dist_sq[prim_shifts_not_found_count] = min_dist_sq;

                for(i_dim = 0; i_dim < 3; i_dim++){

                    prim_shifts_not_found.displ    [prim_shifts_not_found_count][i_dim] = x_displ[i_dim];
                    prim_shifts_not_found.c0_coords[prim_shifts_not_found_count][i_dim] = _C(i_layer).x[c0_min_dist][i_dim];
                }

                prim_shifts_not_found_count++;
            }

        }
    }

    global_prim_shifts_not_found_count = PRF_GISUM1(prim_shifts_not_found_count);

    // try to find remaining prim shifts on other nodes
    if(global_prim_shifts_not_found_count > 0){

        // provide global_prim_shifts_not_found lists for all nodes
        {
            shift_allocate_lists(rcfd,
                                &global_prim_shifts_not_found,
                                 global_prim_shifts_not_found_count,
                                 RCFD_SHIFT_TYPE_PREPARATION);

            parallel_gatherv0_shifts(rcfd,
                                    &prim_shifts_not_found,
                                     prim_shifts_not_found_count,
                                    &global_prim_shifts_not_found,
                                     RCFD_SHIFT_TYPE_PREPARATION);

            parallel_bcast0_shifts(rcfd,
                                  &global_prim_shifts_not_found,
                                   global_prim_shifts_not_found_count,
                                   RCFD_SHIFT_TYPE_PREPARATION);
        }

        // loop all cells and try to find min_dist
        {
            for(i_shift = 0; i_shift < global_prim_shifts_not_found_count; i_shift++){

                if(global_prim_shifts_not_found.node1[i_shift] != myid){   // inter-node shift

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        x_displ[i_dim] = global_prim_shifts_not_found.displ[i_shift][i_dim];
                    }

                    if((moving_geometry_on) ||
                       (is_point_inside_box(x_displ, _C(i_layer).x_min_node, _C(i_layer).x_max_node))){

                        min_dist_sq = global_prim_shifts_not_found.dist_sq[i_shift];

                        c0_min_dist = -1;

                        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                            if(_C(i_layer).vof[0][i_cell][i_phase] > rcfd->Solver_Dict->vof_cutoff_limit){

                                dist_sq = cell2point_distance_sq(rcfd, i_layer, i_cell, x_displ);

                                if(dist_sq < min_dist_sq){

                                    min_dist_sq = dist_sq;
                                    c0_min_dist = i_cell;
                                }
                            }
                        }

                        if(c0_min_dist != -1){

                            global_prim_shifts_not_found.c0     [i_shift] = c0_min_dist;
                            global_prim_shifts_not_found.node0  [i_shift] = myid;
                            global_prim_shifts_not_found.dist_sq[i_shift] = min_dist_sq;

                            for(i_dim = 0; i_dim < 3; i_dim++){

                                global_prim_shifts_not_found.c0_coords[i_shift][i_dim] = _C(i_layer).x[c0_min_dist][i_dim];
                            }

                            prim_shifts_from_other_nodes_count++;
                        }
                    }
                }
            }
        }

        // node-zero adapts global list and broadcasts it to all nodes
        {

            if(myid == 0){

                list_of_c0 =        (int*)malloc   (    global_prim_shifts_not_found_count * sizeof(int));
                list_of_dist =      (double*)malloc(    global_prim_shifts_not_found_count * sizeof(double));
                list_of_c0_coords = (double*)malloc(3 * global_prim_shifts_not_found_count * sizeof(double));

                for(i_node = 1; i_node < compute_node_count; i_node++){

                    PRF_CRECV_INT (i_node, list_of_c0,            global_prim_shifts_not_found_count, i_node);
                    PRF_CRECV_REAL(i_node, list_of_dist,          global_prim_shifts_not_found_count, i_node);
                    PRF_CRECV_REAL(i_node, list_of_c0_coords, 3 * global_prim_shifts_not_found_count, i_node);

                    for(i_shift = 0; i_shift < global_prim_shifts_not_found_count; i_shift++){

                        if(list_of_dist[i_shift] < global_prim_shifts_not_found.dist_sq[i_shift]){

                            global_prim_shifts_not_found.c0     [i_shift] = list_of_c0[i_shift];
                            global_prim_shifts_not_found.node0  [i_shift] = i_node;
                            global_prim_shifts_not_found.dist_sq[i_shift] = list_of_dist[i_shift];

                            for(i_dim = 0; i_dim < 3; i_dim++){

                                global_prim_shifts_not_found.c0_coords[i_shift][i_dim] = list_of_c0_coords[(3 * i_shift + i_dim)];
                            }
                        }
                    }
                }

                free(list_of_c0);
                free(list_of_dist);
                free(list_of_c0_coords);
            }

            if(myid> 0){

                PRF_CSEND_INT (node_zero, global_prim_shifts_not_found.c0,               global_prim_shifts_not_found_count, myid);
                PRF_CSEND_REAL(node_zero, global_prim_shifts_not_found.dist_sq,          global_prim_shifts_not_found_count, myid);
                PRF_CSEND_REAL(node_zero, global_prim_shifts_not_found.c0_coords[0], 3 * global_prim_shifts_not_found_count, myid);
            }

            parallel_bcast0_shifts(rcfd,
                                  &global_prim_shifts_not_found,
                                   global_prim_shifts_not_found_count,
                                   RCFD_SHIFT_TYPE_PREPARATION);

        }

        // add shifts
        {
            for(i_shift = 0; i_shift < global_prim_shifts_not_found_count; i_shift++){

                if(global_prim_shifts_not_found.node1[i_shift] == myid){

                    c0 =    global_prim_shifts_not_found.c0   [i_shift];
                    node0 = global_prim_shifts_not_found.node0[i_shift];
                    c1 =    global_prim_shifts_not_found.c1   [i_shift];
                    node1 = myid;

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        c0_coords[i_dim] = global_prim_shifts_not_found.c0_coords[i_shift][i_dim];
                    }

                    prep_shift_add_to_list(rcfd, i_layer, i_phase, c0, node0, w0, c1, node1, c0_coords);
                }
            }
        }

    }

    // free local vars
    {
        free(cell_length);
        free(c0_coords);

        free_lists_of_shift(rcfd, &prim_shifts_not_found);
        free_lists_of_shift(rcfd, &global_prim_shifts_not_found);
    }

    if(LOCAL_DEBUG){

        if(shift_is_valid(rcfd, &rcfd->C2C_Prep[i_phase], RCFD_SHIFT_TYPE_PREPARATION)){

            report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                "[%d] prep_shift_from_displacement_fields: rcfd->C2C_Prep[%d] - all shifts are valid",
                myid, i_phase);
        }
        else{

            report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                "[%d] prep_shift_from_displacement_fields: rcfd->C2C_Prep[%d] - found invalid shifts",
                myid, i_phase);
        }

        report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                     "[%d] shift_from_displacement_fields: next_neigh %d, all_cell %d, not_found %d, found_on_other_node %d",
                     myid,
                     prim_shifts_next_neigh_cell_count,
                     prim_shifts_all_cell_count,
                     prim_shifts_not_found_count,
                     prim_shifts_from_other_nodes_count);
    }

    return rcfd->C2C_Prep[i_phase].valid_shifts_count;

#undef LOCAL_DEBUG

#endif

    return -1;
}

/**
 * @brief Check the error between the displacement calculated based on
 *        cell-to-cell shifts and the displacement calculated via CFD
 *        (helper fct for prep_interpolate_shifts)
 *
 * @param rcfd      pointer to rCFD instance
 * @param i_phase   specifies the phase for which the displacement is tested
 * @param report_format      flag to indicate whether function is called pre (0) or post (1)
 *                           interpolation of cell-to-cell shifts
 **/
void prep_test_displacement(rCFD_type *rcfd, int i_phase, int report_format)
{
#if RP_NODE

#if 1   // local var declarations
    int     i_dim, i_cell, i_shift;
    int     c0, node0, c1, node1;
    int     i_layer = 0;
    int     shift_count = rcfd->C2C_Prep[i_phase].valid_shifts_count;
    int     cell_count = _CELL_DICT(i_layer).number_of_cells;
    double  w0;
    double  total_error, total_volume;
    double  **test_displ = NULL;
#endif

    // alloc test_displ, initializations
    {
        test_displ = malloc_r_2d(cell_count, 3);
        memset(test_displ[0], 0, (3 * cell_count));

        memset(_C(i_layer).weight_after_shift, 0, cell_count * sizeof(double));
    }

    // loop C2Cs and set displacement field
    {
        for(i_shift = 0; i_shift < shift_count; i_shift++){

            c0 =    rcfd->C2C_Prep[i_phase].c0   [i_shift];
            node0 = rcfd->C2C_Prep[i_phase].node0[i_shift];
            c1 =    rcfd->C2C_Prep[i_phase].c1   [i_shift];
            node1 = rcfd->C2C_Prep[i_phase].node1[i_shift];
            w0 =    rcfd->C2C_Prep[i_phase].w0   [i_shift];

            if(node0 == node1){

                if(w0 > 0.0){

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        test_displ[c1][i_dim] =
                            (w0 * _C(i_layer).x[c0][i_dim] + _C(i_layer).weight_after_shift[c1] * test_displ[c1][i_dim]) /
                            (w0 + _C(i_layer).weight_after_shift[c1]);
                    }

                    _C(i_layer).weight_after_shift[c1] += w0;
                }
            }
        }
    }

    // analyze global error, set new displacement field
    {
        total_volume = 0.0;
        total_error =  0.0;

        for(i_cell = 0; i_cell < cell_count; i_cell++){

            if((_C(i_layer).vof[0][i_cell][i_phase] > rcfd->Solver_Dict->vof_cutoff_limit) &&
               (_C(i_layer).weight_after_shift[i_cell] > 0.0)) {

                total_volume += _C(i_layer).volume[i_cell];

                for(i_dim = 0; i_dim < 3; i_dim++){

                    total_error += fabs(_C(i_layer).volume[i_cell]
                                        * (_C(i_layer).cfd_displ[i_phase][i_cell][i_dim] - test_displ[i_cell][i_dim])
                                        / (_C(i_layer).x_max_global[i_dim] - _C(i_layer).x_min_global[i_dim])); //error independent of bounding-box-size
                }
            }
        }

        total_volume = PRF_GRSUM1(total_volume);
        total_error =  PRF_GRSUM1(total_error);

        if(total_volume > 0.0) total_error /= total_volume;
    }

#if !defined(RCFD_STANDALONE)
    // write debug information to UDMI
    fluent_debug_test_displacement(test_displ, report_format);
#endif // RCFD_STANDALONE

    // free local vars
    {
        free_r_2d(test_displ);
    }

    report_info0(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                 "prep_test_displacement (layer %d phase %d):"
                 " %s interpolation error (intra-process approx.): %e (m/m3)",
                 i_layer, i_phase, report_format?"post":"pre ", total_error);
#endif // RP_NODE
}

#if RP_NODE // helper fcts for prep_interpolate_shifts_inside_node
/**
 * @brief Find the closest neighbor of c0 to a point.
 *
 * @param rcfd          the rcfd struct
 * @param c0            the starting point of the search for the closest cell
 * @param x_cal         the point for which the closest cell should be found.
 * @param i_layer       the current layer
 * @param i_phase       the current phase (necessary for vof_cutoff_limit)
 * @param inclued_c0    flag sets if c0 should also be considered to obtain the closest cell to x_cal
 * @return the cell id of the closest neighbor to x_cal
 */
int find_closest_neighbor(rCFD_type *rcfd, int c0, const double *x_cal, int i_layer, int i_phase, int include_c0)
{
    double dist_sq, min_dist_sq;
    int i_nnb, c0_nnb, min_dist_id;

    // initialize the minimal distance with c0 or the first neighbor
    if(include_c0){

        min_dist_sq = cell2point_distance_sq(rcfd, i_layer, c0, x_cal);
        min_dist_id = c0;
        i_nnb = 0;
    }
    else{

        min_dist_sq = cell2point_distance_sq(rcfd, i_layer, _C(i_layer).next_neigh_cell[c0][0], x_cal);
        min_dist_id = _C(i_layer).next_neigh_cell[c0][0];
        i_nnb = 1;
    }

    // loop over all neighbors of c0 to find the one closest to x_cal
    for(; i_nnb < _C(i_layer).number_of_next_neigh_cells[c0]; i_nnb++){

        c0_nnb = _C(i_layer).next_neigh_cell[c0][i_nnb];

        if(_C(i_layer).vof[0][c0_nnb][i_phase] > rcfd->Solver_Dict->vof_cutoff_limit){

            dist_sq = cell2point_distance_sq(rcfd, i_layer, c0_nnb, x_cal);

            if(dist_sq < min_dist_sq){

                min_dist_sq = dist_sq;
                min_dist_id = c0_nnb;
            }
        }
    }

    return min_dist_id;
}

/**
 * @brief Find the closest cell to a point by iteratively looking through
 *        c0 and its neighboring cells.
 *
 * @param rcfd      the rcfd struct
 * @param c0        the starting cell of the search for the closest cell
 * @param x_cal     the point for which the closest cell should be found.
 * @param i_layer   the current layer
 * @param i_phase   the current phase (necessary for vof_cutoff_limit)
 * @return the cell id of the closest cell to x_cal
 */
int find_closest_cell(rCFD_type *rcfd, int i_layer, int i_phase, int c0, const double *x_cal)
{
    int min_dist_id;
    int c0_old = -1;

    // as long as a a neighboring cell has been found that is closer, check the neighbors of the
    // new closest cell for even closer neighbors.
    while(c0 != c0_old){

        c0_old = c0;

        min_dist_id = find_closest_neighbor(rcfd, c0, x_cal, i_layer, i_phase, 1);

        // if interior cell, adapt c0
        if(min_dist_id < _CELL_DICT(i_layer).number_of_int_cells){

            c0 = min_dist_id;
        }
    }

    return c0;
}

/**
 * @brief Finds all neighbors that could possibly used for interpolation of vec_c0_cal.
 *        It does so by checking if the scalar product of vec_c0_cal and the calclulated
 *        C2C vectors is positive.
 *
 * @param c0                    origin cell from which the neighbors are iterated
 * @param vec_c0_cal            the vector that should be interpolated via C2C vectors
 * @param cell_centers          used to calculate C2C vectors
 * @param next_neigh_cells      The neighboring cells to be iterated.
 * @param len_next_neigh_cells  length of next_neigh_cells
 * @param aligned_neigh_list    list to be filled with aligned neighbors
 * @param len_aligned_neigh_list length of aligned_neigh_list
 * @param cell_vofs             contains vof of all cells (necessary for vof_cutoff)
 * @param vof_cutoff            used to ignore shifts from cells with too little vof's
 * @param current_phase               the current phase (necessary for vof_cutoff)
 * @return number of aligned neighbor cells or -1 if there were more than len_aligned_neigh_list
 */
int find_aligned_neighbors(int c0, const double *vec_c0_cal, double **cell_centers,
                           const int *next_neigh_cells, int len_next_neigh_cells,
                           int *aligned_neigh_list, int len_aligned_neigh_list,
                           double **cell_vofs, int vof_cutoff,
                           int current_phase)
{
    int n_align_neigh_cells = 0;
    int i_nb, i_nnb, i_dim, c0_nb;

    double vec_c0_nb[3];
    double vec_align;

    // init aligned_neigh_list with -1 == no aligned neighbors
    for(i_nb = 0; i_nb < len_aligned_neigh_list; i_nb++){

        aligned_neigh_list[i_nb] = -1;
    }

    // iterate through all neighbors of c0...
    for(i_nnb = 0; i_nnb < len_next_neigh_cells; i_nnb++){

        c0_nb = next_neigh_cells[i_nnb];

        if(cell_vofs[c0_nb][current_phase] > vof_cutoff){

            // ...calculate the C2C vector from c0 to c0_nb via the cell-centers...
            for(i_dim = 0; i_dim < 3; i_dim++){

                vec_c0_nb[i_dim] = cell_centers[c0_nb][i_dim] - cell_centers[c0][i_dim];
            }

            // ... and check if the scalar projection on vec_c0_cal is positive.
            // (if it is not, you immediately know that the neighbor cell can only
            // extrapolate vec_c0_cal and not interpolate as it is pointing away
            // from x_cal.)
            vec_align = scalar_projection(vec_c0_nb, vec_c0_cal);

            if(vec_align > 0.0){

                // if it is, add it to the list of possible aligned cells.
                if(n_align_neigh_cells < len_aligned_neigh_list){

                    aligned_neigh_list[n_align_neigh_cells] = c0_nb;

                    n_align_neigh_cells++;
                }
                else{

                    return -1;
                }
            }
        }
    }

    return n_align_neigh_cells;
}

/**
 * @brief Calculate the C2C shift vectors between new_shift[0] and
 *        new_shift[1,2,3] and writes them to M
 *
 * @param new_shifts    The list of shifts from which the C2C shift vectors are calculated.
 * @param M             The shift matrix to be set up.
 * @param cell_centers The cell positions for the calculation of the C2C shift vectors.
 */
void setup_shift_matrix_3d(const C2C_Shift_type *new_shifts, double **M, double **cell_centers)
{
    int i_dim;

    for(i_dim = 0; i_dim < 3; i_dim++){

        M[i_dim][0] = cell_centers[new_shifts->c0[1]][i_dim] - cell_centers[new_shifts->c0[0]][i_dim];
        M[i_dim][1] = cell_centers[new_shifts->c0[2]][i_dim] - cell_centers[new_shifts->c0[0]][i_dim];
        M[i_dim][2] = cell_centers[new_shifts->c0[3]][i_dim] - cell_centers[new_shifts->c0[0]][i_dim];
    }
}

/**
 * @brief Calculate the C2C shift vectors between new_shift[0] and
 *        new_shift[1,2] and the cross product of new_shift[1,2] and writes them to M.
 *
 * @param new_shifts    The list of shifts from which the C2C shift vectors are calculated.
 * @param M             The shift matrix to be set up.
 * @param cell_centers The cell positions for the calculation of the C2C shift vectors.
 */
void setup_shift_matrix_2d(const C2C_Shift_type *new_shifts, double **M, double **cell_centers)
{
    int i_dim;
    double a[3], b[3], axb[3];

    for(i_dim = 0; i_dim < 3; i_dim++){

        a[i_dim] = cell_centers[new_shifts->c0[1]][i_dim] - cell_centers[new_shifts->c0[0]][i_dim];
        b[i_dim] = cell_centers[new_shifts->c0[2]][i_dim] - cell_centers[new_shifts->c0[0]][i_dim];
    }

    vector_cross_product(a, b, axb);

    for(i_dim = 0; i_dim < 3; i_dim++){

        M[i_dim][0] =   a[i_dim];
        M[i_dim][1] =   b[i_dim];
        M[i_dim][2] = axb[i_dim];
    }
}

/**
 * @brief Advance a set of 3 indices so that all possible combinations of
 *        indices will be reached for a given length of a list.
 *
 * @param indices   the indices to be advanced.
 * @param len_list  the length of the list the indices point at.
 */
void advance_indices(int *indices, int len_list)
{
    if(indices[2] < (len_list - 2)){

        indices[2]++;
    }
    else if(indices[1] < (len_list - 3)){

        indices[1]++;
        indices[2] = indices[1] + 1;
    }
    else{

        indices[0]++;
        indices[1] = indices[0] + 1;
        indices[2] = indices[0] + 2;
    }
}

void set_c0_coords(C2C_Shift_type *shifts, int shifts_count, double **cell_centers)
{
#if RP_NODE
    int i_dim, i_shift;

    int c0;

    for(i_shift = 0; i_shift < shifts_count; i_shift++){

        c0 = shifts->c0[i_shift];

        if(c0 >= 0){

            for(i_dim = 0; i_dim < 3; i_dim++){

                shifts->c0_coords[i_shift][i_dim] = cell_centers[c0][i_dim];
            }
        }
    }
#endif
}

/**
 * @brief Try to interpolate the vector vec_c0_cal by using a combination of cell centers.
 *
 * @param vec_c0_cal             The vector to be interpolated.
 * @param new_shifts             The C2C shifts that should be filled with correct shifts and weights
 * @param aligned_neigh_list     The list of cells that can be used for the interpolation.
 * @param len_aligned_neigh_list The length of aligned_neigh_list.
 * @param cell_centers           The cell positions for the calculation of the C2C shift vectors.
 * @return 1 if interpolation is successfull and 0 if it fails.
 */
int interpolate_vec_c0_displ(const double *vec_c0_cal, C2C_Shift_type *new_shifts,
                             const int *aligned_neigh_list, int len_aligned_neigh_list,
                             double **cell_centers)
{
    // allocate and init M, M_inv
    int nbytes = 3 * 3 * sizeof(double);
    int new_shifts_count = 4;

    int i_neighbors[3]= {0,1,2}; // indices to iterate through all combinations of neighbors
    double det;

    double **M       = malloc_r_2d(3, 3);
    double **M_inv   = malloc_r_2d(3, 3);

    memset(M[0],    0, nbytes);
    memset(M_inv[0],0, nbytes);

    // loop over indices of aligned nb cells
    while(i_neighbors[2] < (len_aligned_neigh_list - 3)){

        // set vectors to triple nb cells
        new_shifts->c0[1] = aligned_neigh_list[i_neighbors[0]];
        new_shifts->c0[2] = aligned_neigh_list[i_neighbors[1]];
        new_shifts->c0[3] = aligned_neigh_list[i_neighbors[2]];

        setup_shift_matrix_3d(new_shifts, M, cell_centers);

        // calculate individual weights
        det = inverse_3x3(M, M_inv);

        if(det != 0.0){

            new_shifts->w0[1] = M_inv[0][0]*vec_c0_cal[0] + M_inv[0][1]*vec_c0_cal[1] + M_inv[0][2]*vec_c0_cal[2];
            new_shifts->w0[2] = M_inv[1][0]*vec_c0_cal[0] + M_inv[1][1]*vec_c0_cal[1] + M_inv[1][2]*vec_c0_cal[2];
            new_shifts->w0[3] = M_inv[2][0]*vec_c0_cal[0] + M_inv[2][1]*vec_c0_cal[1] + M_inv[2][2]*vec_c0_cal[2];

            new_shifts->w0[0] = 1.0 - (new_shifts->w0[1] + new_shifts->w0[2] + new_shifts->w0[3]);

            set_c0_coords(new_shifts, new_shifts_count, cell_centers);
        }
        else{

            new_shifts->w0[0] = -1.0;
            new_shifts->w0[1] = -1.0;
            new_shifts->w0[2] = -1.0;
            new_shifts->w0[3] = -1.0;
        }

        // check if all pos - return sucessfully if true
        if( (new_shifts->w0[0] > 0.0) &&
            (new_shifts->w0[1] > 0.0) &&
            (new_shifts->w0[2] > 0.0) &&
            (new_shifts->w0[3] > 0.0)){

            free_r_2d(M);
            free_r_2d(M_inv);

            return 1;
        }

        // advance indices i_neighbors
        advance_indices(i_neighbors, len_aligned_neigh_list);
    }

    // free M, M_inv and return fail
    free_r_2d(M);
    free_r_2d(M_inv);

    new_shifts->w0[0] = -1.0;
    new_shifts->w0[1] = -1.0;
    new_shifts->w0[2] = -1.0;
    new_shifts->w0[3] = -1.0;

    return 0;
}

double calc_projection_accuracy(const double *vec_c0_cal,
                                const C2C_Shift_type *new_shifts, int new_shifts_count,
                                double **cell_centers)
{
    int i_dim, i_shift;
    double displ_pos[3] = {cell_centers[new_shifts->c0[0]][0] + vec_c0_cal[0],
                           cell_centers[new_shifts->c0[0]][1] + vec_c0_cal[1],
                           cell_centers[new_shifts->c0[0]][2] + vec_c0_cal[2]};
    double proj_pos[3] = {0.0, 0.0, 0.0};
    double weight;

    // assume the weights sum up to 1, otherwise we would have to divide by weight
    for(i_shift = 0; i_shift < new_shifts_count; i_shift++){

        for(i_dim = 0; i_dim < 3; i_dim++){

            proj_pos[i_dim] += cell_centers[new_shifts->c0[i_shift]][i_dim] * (new_shifts->w0[i_shift]);
        }
    }

    weight =  (displ_pos[0] - proj_pos[0])*(displ_pos[0] - proj_pos[0])
            + (displ_pos[1] - proj_pos[1])*(displ_pos[1] - proj_pos[1])
            + (displ_pos[2] - proj_pos[2])*(displ_pos[2] - proj_pos[2]);

    return weight;
}

/**
* @brief Projects vec_c0_cal onto a 2D plane, 1D line or 0D point - whichever
 *        gives the shortest projection distance. Never Fails, because you can
 *        always project onto a 0D point.
 *
 * @param vec_c0_cal             The vector to be interpolated.
 * @param min_new_shifts         The C2C shifts that should be filled with correct shifts and weights.
 * @param aligned_neigh_list     The list of cells that can be used for the interpolation.
 * @param len_aligned_neigh_list The length of aligned_neigh_list.
 * @param cell_centers           The cell positions for the calculation of the C2C shift vectors.
 */
void project_vec_c0_displ(const double *vec_c0_cal, C2C_Shift_type *min_new_shifts,
                          const int *aligned_neigh_list, int len_aligned_neigh_list,
                          double **cell_centers)
{
    // in order to increase efficiency, simple cases are handled immediately
    // before a more complex algorithm is used
    if(len_aligned_neigh_list == 0){

        min_new_shifts->w0[0] = 1.0;
        min_new_shifts->w0[1] = -1.0;
        min_new_shifts->w0[2] = -1.0;

        set_c0_coords(min_new_shifts, 1, cell_centers);

        return;
    }

    if(len_aligned_neigh_list == 1){

        int i_dim;
        int c0_0 = min_new_shifts->c0[0];
        int c0_1 = aligned_neigh_list[0];

        double c0_1_displ[3];

        for(i_dim = 0; i_dim < 3; i_dim++){

            c0_1_displ[i_dim] = cell_centers[c0_0][i_dim] + vec_c0_cal[i_dim] - cell_centers[c0_1][i_dim];
        }

        double dist_c0_0_displ = vector_magnitude(vec_c0_cal);
        double dist_c0_1_displ = vector_magnitude(c0_1_displ);

        min_new_shifts->w0[0] = dist_c0_1_displ / (dist_c0_0_displ + dist_c0_1_displ);
        min_new_shifts->w0[1] = dist_c0_0_displ / (dist_c0_0_displ + dist_c0_1_displ);
        min_new_shifts->w0[2] = -1.0;

        min_new_shifts->c0[1] = c0_1;

        set_c0_coords(min_new_shifts, 2, cell_centers);

        return;
    }

    int nbytes = 3 * 3 * sizeof(double);

    int i_neighbors[3]= {0,1,len_aligned_neigh_list}; // indices to iterate through all tuple-combinations of neighbors
    double det;

    double **M       = malloc_r_2d(3, 3);
    double **M_inv   = malloc_r_2d(3, 3);

    memset(M[0],    0, nbytes);
    memset(M_inv[0],0, nbytes);

    double min_proj_dist = -1.0, current_proj_dist;

    C2C_Shift_type new_shifts;

    new_shifts.c0 = malloc(3 * sizeof(int));    // cannot use shift_allocate_lists because *rcfd is not available
    new_shifts.w0 = malloc(3 * sizeof(double));

    new_shifts.c0[0] = min_new_shifts->c0[0];

    // FIRST: Try 2D-Interpolation (only use i_neighbors[0,1] and their cross-product for interpolation)
    // loop over indices of aligned nb cells

    while(i_neighbors[2] < (len_aligned_neigh_list + 1) && len_aligned_neigh_list > 1){

        // set vectors to tuple nb cells
        new_shifts.c0[1] = aligned_neigh_list[i_neighbors[0]];
        new_shifts.c0[2] = aligned_neigh_list[i_neighbors[1]];

        setup_shift_matrix_2d(&new_shifts, M, cell_centers);

        // calculate individual weights
        det = inverse_3x3(M, M_inv);

        if(det != 0.0){

            new_shifts.w0[1] = M_inv[0][0]*vec_c0_cal[0] + M_inv[0][1]*vec_c0_cal[1] + M_inv[0][2]*vec_c0_cal[2];
            new_shifts.w0[2] = M_inv[1][0]*vec_c0_cal[0] + M_inv[1][1]*vec_c0_cal[1] + M_inv[1][2]*vec_c0_cal[2];

            new_shifts.w0[0] = 1.0 - (new_shifts.w0[1] + new_shifts.w0[2]);
        }
        else{

            new_shifts.w0[0] = -1.0;
            new_shifts.w0[1] = -1.0;
            new_shifts.w0[2] = -1.0;
        }

        // check if all weights pos and if projection distance is smaller - if it is:
        // reset min_new_shifts to better projection
        if( (new_shifts.w0[0] > 0.0) &&
            (new_shifts.w0[1] > 0.0) &&
            (new_shifts.w0[2] > 0.0)){

            current_proj_dist = calc_projection_accuracy(vec_c0_cal, &new_shifts, 3, cell_centers);

            if(current_proj_dist < min_proj_dist || min_proj_dist < 0.0){

                min_proj_dist = current_proj_dist;

                min_new_shifts->w0[0] = new_shifts.w0[0];
                min_new_shifts->w0[1] = new_shifts.w0[1];
                min_new_shifts->w0[2] = new_shifts.w0[2];

                min_new_shifts->c0[1] = new_shifts.c0[1];
                min_new_shifts->c0[2] = new_shifts.c0[2];

                set_c0_coords(min_new_shifts, 3, cell_centers);
            }
        }

        // advance indices i_neighbors
        advance_indices(i_neighbors, 0);
    }

    // SECOND: Try 1D Interpolation (by using the closest cell and i_neighbors[0])

    int i_dim;
    // vector between cell centers of closest cell and i_neighbors[0]
    double c2c_vector[3], c2c_vec_length;

    new_shifts.w0[2] = -1.0;

    for(i_neighbors[0] = 0; i_neighbors[0] < len_aligned_neigh_list; i_neighbors[0]++){

        new_shifts.c0[1] = aligned_neigh_list[i_neighbors[0]];
        c2c_vec_length = 0.0;

        //calc c2c_vector and it's length
        for(i_dim = 0; i_dim < 3; i_dim++){
            c2c_vector[i_dim] =   cell_centers[new_shifts.c0[1]][i_dim]
                                - cell_centers[new_shifts.c0[0]][i_dim];
            c2c_vec_length += c2c_vector[i_dim] * c2c_vector[i_dim];
        }
        c2c_vec_length = sqrt(c2c_vec_length);

        // weight is calculated by:
        // c2c_vector . vec_c0_cal / |c2c_vector|^2 = cos(theta) * |vec_c0_cal| / |c2c_vector|
        // which can be seen as the length of vec_c0_cal when you project it onto c2c_vector,
        // diviced by the length of c2c_vector itself.
        // In other words: It is the length ratio between the projected vec_c0_cal and c2c_vector.
        new_shifts.w0[1] = scalar_projection(vec_c0_cal, c2c_vector) / c2c_vec_length;

        // check if all weights pos and if projection distance is smaller - if it is:
        // reset min_new_shifts to better projection
        if(new_shifts.w0[1] <= 1.0 && new_shifts.w0[1] >= 0.0){

            new_shifts.w0[0] = 1.0 - new_shifts.w0[1];

            current_proj_dist = calc_projection_accuracy(vec_c0_cal, &new_shifts, 2, cell_centers);

            if(current_proj_dist < min_proj_dist || min_proj_dist < 0.0){

                min_proj_dist = current_proj_dist;

                min_new_shifts->w0[0] = new_shifts.w0[0];
                min_new_shifts->w0[1] = new_shifts.w0[1];
                min_new_shifts->w0[2] = new_shifts.w0[2];

                min_new_shifts->c0[1] = new_shifts.c0[1];
                min_new_shifts->c0[2] = new_shifts.c0[2];

                set_c0_coords(min_new_shifts, 3, cell_centers);
            }
        }
    }

    new_shifts.w0[0] = 1.0;
    current_proj_dist = calc_projection_accuracy(vec_c0_cal, &new_shifts, 1, cell_centers);

    // if 2D and 1D interpolation failed, fall back to 0D interpolation
    // by using closest cell_center
    if(current_proj_dist < min_proj_dist || min_proj_dist < 0.0){

        min_new_shifts->w0[0] = 1.0;
        min_new_shifts->w0[1] = -1.0;
        min_new_shifts->w0[2] = -1.0;

        set_c0_coords(min_new_shifts, 1, cell_centers);
    }

    // free M, M_inv
    free_r_2d(M);
    free_r_2d(M_inv);
    free(new_shifts.c0);
    free(new_shifts.w0);
}

/**
 * @brief Multiply the weights of the given shifts with the vof of the
 *        corresponding c0 cell and re-normalize the resulting weigths
 *
 * @param new_shifts        list of C2C shifts to be re-weighted
 * @param new_shifts_count    length of new_shifts
 * @param vofs              list of vofs to be used as factors for the new weight.
 * @param current_phase           the phase for which the vof should be used.
 */
void shift_multiply_weights_with_vof(C2C_Shift_type *new_shifts, int new_shifts_count, double **vofs, int current_phase)
{
    int i_shift;
    double sum_of_weights = 0.0;

    for(i_shift = 0; i_shift < new_shifts_count; i_shift++){

        if(new_shifts->w0[i_shift] > 0.0){

            new_shifts->w0[i_shift] *= vofs[new_shifts->c0[i_shift]][current_phase];
            sum_of_weights += new_shifts->w0[i_shift];
        }
    }

    if(sum_of_weights > 0.0){

        double inv_sum_of_weights = 1.0/sum_of_weights;

        for(i_shift = 0; i_shift < new_shifts_count; i_shift++){

            if(new_shifts->w0[i_shift] > 0.0){

                new_shifts->w0[i_shift] *= inv_sum_of_weights;
            }
        }
    }
}

/**
 * @brief prune new_shifts.
 *
 * @param new_shifts        list of new shifts
 * @param new_shifts_count  number of new shifts
 * @param min_shifts_after_pruning sets how many shifts should remain at the very least
 * @param pruning_weight    sets the minimal weight fraction for pruning shifts
 */
void prune_shifts(C2C_Shift_type *new_shifts, int new_shifts_count,
                 int min_shifts_after_pruning, double pruning_weight)
{
    double sum_of_weights = 0.0, new_sum_of_weights;
    double *weights = (double*)malloc(new_shifts_count * sizeof(double));

    int *sorted_ids;
    int i;

    // calc sum_of_weights for comparison with pruning_weight and weights for argsort
    for(i = 0; i < new_shifts_count; i++){

        if(new_shifts->w0[i] > 0.0){

            sum_of_weights += new_shifts->w0[i];
        }

        weights[i] = new_shifts->w0[i];
    }

    //sort shifts with their weights.
    sorted_ids = selectionArgsort(weights, new_shifts_count, 0);

    free(weights);

    for(i = 0; i < new_shifts_count; i++){

        // mark pruned shifts
        if( (i >= min_shifts_after_pruning) &&
            (new_shifts->w0[sorted_ids[i]] / sum_of_weights) < pruning_weight){

            new_shifts->w0[sorted_ids[i]] = -1.0;
        }
    }

    new_sum_of_weights = 0.0;

    for(i = 0; i < new_shifts_count; i++){

        if(new_shifts->w0[i] > 0.0){

            new_sum_of_weights += new_shifts->w0[i];
        }
    }

    if(new_sum_of_weights > 0.0){

        for(i = 0; i < new_shifts_count; i++){

            if(new_shifts->w0[i] > 0.0){

                new_shifts->w0[i] *= sum_of_weights / new_sum_of_weights;
            }
        }
    }

    free(sorted_ids);
}

/**
 * @brief add valid new_shifts to global list
 *
 * @param shifts_to_be_interpolated    global list of shifts (primary and interpolated)
 * @param i_prim_shift                 current primary shift index
 * @param i_interpolated_shift         current interpolated shift index
 * @param new_shifts                   list of new shifts
 * @param new_shifts_count             number of new shifts
 * @param added_shifts_count           number of added shifts (< new_shifts_count)
 * @return either 0 or -1 if error
 */
int add_valid_new_shifts(rCFD_type *rcfd,
                         C2C_Shift_type *shifts_to_be_interpolated,
                         int i_prim_shift,
                         int i_interpolated_shift,
                         C2C_Shift_type *new_shifts,
                         int new_shifts_count,
                         int *added_shifts_count)
{
#if RP_NODE
    int i_new_shift, i_valid_new_shift = 0, i_dim;
    int i_layer = rcfd->Solver->current_layer;
    int c0;

    i_valid_new_shift = 0;

    for(i_new_shift = 0; i_new_shift < new_shifts_count; i_new_shift++){

        if(!shift_is_index_valid(rcfd, new_shifts, i_new_shift, RCFD_SHIFT_TYPE_PREPARATION)){

            new_shifts->w0[i_new_shift] = -1.0;
        }
        else{

            i_valid_new_shift++;

            c0 = new_shifts->c0[i_new_shift];

            if(i_valid_new_shift == 1){

                shift_copy_one_in_list(rcfd,
                                       shifts_to_be_interpolated, i_prim_shift,
                                       new_shifts, i_new_shift,
                                       RCFD_SHIFT_TYPE_PREPARATION);

                for(i_dim = 1; i_dim < 3; i_dim++){

                    shifts_to_be_interpolated->c0_coords[i_prim_shift][i_dim] = _C(i_layer).x[c0][i_dim];
                }
            }
            else{

                shift_copy_one_in_list(rcfd,
                                       shifts_to_be_interpolated, i_interpolated_shift,
                                       new_shifts, i_new_shift,
                                       RCFD_SHIFT_TYPE_PREPARATION);

                for(i_dim = 1; i_dim < 3; i_dim++){

                    shifts_to_be_interpolated->c0_coords[i_interpolated_shift][i_dim] = _C(i_layer).x[c0][i_dim];
                }

                i_interpolated_shift++;
            }
        }
    }

    if(i_valid_new_shift < 1){

        *added_shifts_count = 0;    // original primary shifts just stays in place, no new shift added
    }
    else{

        *added_shifts_count = i_valid_new_shift - 1;  // first i_valid just adapts primary shift, rest is added
    }

    return 0;
#endif
    return -1;
}


/**
 * @brief Count the number of valid shifts (with a weight larger than zero)
 *        contained in the given list of shifts.
 *
 * @param new_shifts list of C2C shifts
 * @param new_shifts_count length of new_shifts
 * @return Number of valid shifts (-1 as one shift is old)
 */
int valid_new_shifts(C2C_Shift_type *new_shifts, int new_shifts_count)
{
    int i, valid_shifts = 0;

    for(i = 0; i < new_shifts_count; i++){

        if(new_shifts->w0[i] > 0.0){

            valid_shifts++;
        }
    }

    // one shift was old
    return valid_shifts - 1;
}

#endif // helper fcts

int prep_interpolate_shifts_inside_node(rCFD_type *rcfd,
                                        int i_layer, int i_phase,
                                        C2C_Shift_type *shifts_inside_node,
                                        int *shifts_inside_node_count,
                                        double **displ)
{
#if RP_NODE
    /* known limitations
        if format == RCFD_INTERP_NEAREST_ALIGNED_NEIGHBOR_CELLS:
            in some cases three c0-nb cells are not enough to reach cal_x (we might need to use more nb cells)
    */
#define DEBUG_LOCAL 0

#if 1
    int     i_shift, i_dim, i_interpolate, n_align_neigh_cells, i_warning = 0;
    int     success_interpolation, unaligned_count, aligned_wo_success_count, aligned_w_success_count, pruning_count;
    int     valid_new_shifts_count_before_pruning, valid_new_shifts_count_after_pruning, added_shifts_count, total_added_shifts_count;
    int     *aligned_neigh_list = NULL;
    double  vec_c0_displ[3];
    int     new_shifts_count;
    int     shifts_count;

    C2C_Shift_type  new_shifts;
#endif

    shifts_count = *shifts_inside_node_count;
    if(shifts_count <  0) return -1;
    if(shifts_count == 0) return  0;

    // init global counters, allocate aligned_neigh_list, new_shifts
    {
        aligned_neigh_list = malloc(rcfd->Prep_Dict->interpolation_max_number_of_aligned_neigh_cells * sizeof(int));

        unaligned_count =          0;
        aligned_wo_success_count = 0;
        aligned_w_success_count =  0;
        pruning_count =            0;

        new_shifts_count = 4; // interpolation will sum up to 4 shifts per original shift
        shift_allocate_lists(rcfd, &new_shifts, new_shifts_count, RCFD_SHIFT_TYPE_PREPARATION);
        new_shifts.valid_shifts_count = new_shifts_count;

        total_added_shifts_count = 0;
    }

    // loop primary shifts and add new shifts by interpolation
    for(i_shift = 0; i_shift < shifts_count; i_shift++){

        success_interpolation = 0;

        for(i_interpolate = 0; i_interpolate < new_shifts_count; i_interpolate++){  // new_shifts_count = 4

            shift_copy_one_in_list(rcfd,
                                   &new_shifts, i_interpolate,
                                   shifts_inside_node, i_shift,
                                   RCFD_SHIFT_TYPE_PREPARATION);

            if(i_interpolate > 0){

                new_shifts.c0[i_interpolate] = -1;
                new_shifts.w0[i_interpolate] = -1.0;
            }
        }

        // vec x_c0 to displ[i_shift]
        for(i_dim = 0; i_dim < 3; i_dim++){

            vec_c0_displ[i_dim] = displ[i_shift][i_dim] - _C(i_layer).x[new_shifts.c0[0]][i_dim];
        }

        // create list of aligned nb cells
        // aligned cells are cells that can be used to interpolate displ[i_shift],
        // unaligned cells can only extrapolate displ[i_shift].
        n_align_neigh_cells = find_aligned_neighbors(new_shifts.c0[0],
                                                     vec_c0_displ,
                                                     _C(i_layer).x,
                                                     _C(i_layer).next_neigh_cell[new_shifts.c0[0]],
                                                     _C(i_layer).number_of_next_neigh_cells[new_shifts.c0[0]],
                                                     aligned_neigh_list,
                                                     rcfd->Prep_Dict->interpolation_max_number_of_aligned_neigh_cells,
                                                     _C(i_layer).vof[0],
                                                     rcfd->Solver_Dict->vof_cutoff_limit,
                                                     i_phase);

        // warning if aligned neighbour list is full
        if(n_align_neigh_cells == -1){

            i_warning++;
            n_align_neigh_cells = rcfd->Prep_Dict->interpolation_max_number_of_aligned_neigh_cells;
        }

        // if three or more aligned cells have been found, try to interpolate.
        if(n_align_neigh_cells >= 3){

            success_interpolation = interpolate_vec_c0_displ(vec_c0_displ,
                                                             &new_shifts,
                                                             aligned_neigh_list,
                                                             n_align_neigh_cells,
                                                             _C(i_layer).x);

            if(success_interpolation)   aligned_w_success_count++;
            else                        aligned_wo_success_count++;
        }
        else{

            unaligned_count++;
        }

        if(!success_interpolation){ // 2D or 1D projection - whichever gives the smallest projection error

            project_vec_c0_displ(vec_c0_displ,
                                 &new_shifts,
                                 aligned_neigh_list,
                                 n_align_neigh_cells,
                                 _C(i_layer).x);
        }

        // adapt the weights of all valid shifts with their vof's
        if(((rcfd->Solver_Dict->number_of_phases > 1) || (rcfd->Solver_Dict->single_phase_vof)) &&
           (rcfd->Prep_Dict->interpolation_multiply_shift_weights_with_vof_on)){

            shift_multiply_weights_with_vof(&new_shifts,
                                            new_shifts_count,
                                            _C(i_layer).vof[0],
                                            i_phase);
        }

        // prune new shifts
        if(rcfd->Prep_Dict->interpolation_pruning_on){

            valid_new_shifts_count_before_pruning = valid_new_shifts(&new_shifts, new_shifts_count);

            prune_shifts(&new_shifts,
                         new_shifts_count,
                         rcfd->Prep_Dict->interpolation_min_shifts_after_pruning,
                         rcfd->Prep_Dict->interpolation_pruning_weight);

            valid_new_shifts_count_after_pruning = valid_new_shifts(&new_shifts, new_shifts_count);

            pruning_count += (valid_new_shifts_count_before_pruning - valid_new_shifts_count_after_pruning);
        }

        // add valid new shifts
        {
            added_shifts_count = 0;

            add_valid_new_shifts(rcfd,
                                 shifts_inside_node,
                                 i_shift,
                                 shifts_count + total_added_shifts_count,   // index for newly to be added shifts
                                 &new_shifts,
                                 new_shifts_count,
                                 &added_shifts_count);

            total_added_shifts_count += added_shifts_count;
        }

    } // loop shifts_inside_node

    // event. report warnings
    if(i_warning > 0){

        report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                       "[%d] prep_interpolate_shifts_inside_node: find_aligned_neighbors: list is full for %d cells",
                       myid, i_warning);
    }

    // free local var
    {
        free_lists_of_shift(rcfd, &new_shifts);
        free(aligned_neigh_list);
    }

    *shifts_inside_node_count = shifts_count + total_added_shifts_count;

    if(DEBUG_LOCAL){

        report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                     "[%d] prep_interpolate_shifts_inside_node: total added shifts %d "
                     "(unaligned %d, aligned_w/o_sucess %d, aligned_success %d, pruning_count %d)",
                     myid, total_added_shifts_count,
                     unaligned_count, aligned_wo_success_count, aligned_w_success_count, pruning_count);
    }

    return 0;

#undef DEBUG_LOCAL

#endif // RP_NODE

    return -1;
}

/**
 * @brief Interpolate cross-partition cell-to-cell shifts
 *
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_phase   specifies the phase
 * @param shifts_to_my_node        list of shifts to this compute node
 * @param shifts_to_my_node_count  number of shifts to this compute node
 * @return 0 if success or -1 if error
 */
int prep_interpolate_shifts_to_my_node(rCFD_type *rcfd, int i_layer, int i_phase,
                                       C2C_Shift_type *shifts_to_my_node, int *shifts_to_my_node_count,
                                       double** shifts_to_my_node_coords)
{
#if RP_NODE

#if 1 // local vars
    int error;

    int i_node, i_shift, i_dim, i_shift_from_my_node, i_shift_to_my_node;

    int my_shifts_count;
    int global_shifts_count;
    int shifts_to_my_node_coords_count;
    int shifts_from_my_node_count;

    int *recvcounts = NULL, *displs = NULL;

    double *source_double_ptr = NULL;

    double **global_shifts_coords =       NULL;
    double **shifts_from_my_node_coords = NULL;

    C2C_Shift_type global_shifts;
    C2C_Shift_type shifts_from_my_node;
#endif

    my_shifts_count = *shifts_to_my_node_count;
    global_shifts_count = PRF_GISUM1(my_shifts_count);
    if(global_shifts_count <  0) return -1;
    if(global_shifts_count == 0) return  0;

    // gather shifts_to_my_node on node-zero, bcast global list to all nodes
    {
        shift_allocate_lists(rcfd, &global_shifts, 4 * global_shifts_count, RCFD_SHIFT_TYPE_PREPARATION);

        parallel_gatherv0_shifts(rcfd,
                                 shifts_to_my_node,
                                 my_shifts_count,
                                 &global_shifts,
                                 RCFD_SHIFT_TYPE_PREPARATION);

        parallel_bcast0_shifts(rcfd, &global_shifts, global_shifts_count, RCFD_SHIFT_TYPE_PREPARATION);

        // gather and bcast global_shifts_coords
        {
            recvcounts = malloc(sizeof(int) * compute_node_count);
            displs =     malloc(sizeof(int) * compute_node_count);

            shifts_to_my_node_coords_count = 3 * my_shifts_count;

            PRF_GATHER_INT(recvcounts, &shifts_to_my_node_coords_count, 1, 0);

            displs[0] = 0;

            for(i_node = 1; i_node < compute_node_count; ++i_node){

                displs[i_node] = displs[i_node-1] + recvcounts[i_node-1];
            }

            global_shifts_coords = malloc_r_2d(global_shifts_count, 3);
            memset(global_shifts_coords[0], 0, 3 * global_shifts_count * sizeof(double));

            if(shifts_to_my_node_coords_count > 0){

                source_double_ptr = shifts_to_my_node_coords[0];
            }
            else{

                source_double_ptr = NULL;
            }

            PRF_GATHERV_REAL(global_shifts_coords[0],
                         source_double_ptr,
                         recvcounts, shifts_to_my_node_coords_count, displs, 0);

            free(recvcounts);
            free(displs);

            PRF_BCAST_REAL(global_shifts_coords[0], 3 * global_shifts_count, 0);

        }
    }

    // all nodes collect shifts_from_my_node and produce new shifts by interpolation
    {
        // loop global list and fill shifts_from_my_node(_coords)
        shifts_from_my_node_count = 0;

        for(i_shift = 0; i_shift < global_shifts_count; i_shift++){

            if(global_shifts.node0[i_shift] == myid){

                shifts_from_my_node_count++;
            }
        }

        if(shifts_from_my_node_count > 0){

            shift_allocate_lists(rcfd, &shifts_from_my_node, 4 * shifts_from_my_node_count, RCFD_SHIFT_TYPE_PREPARATION);

            shifts_from_my_node_coords = malloc_r_2d(shifts_from_my_node_count, 3);
            memset(shifts_from_my_node_coords[0], 0, 3 * shifts_from_my_node_count * sizeof(double));

            i_shift_from_my_node = 0;

            for(i_shift = 0; i_shift < global_shifts_count; i_shift++){

                if(global_shifts.node0[i_shift] == myid){

                    shift_copy_one_in_list(rcfd,
                                           &shifts_from_my_node, i_shift_from_my_node,
                                           &global_shifts,       i_shift,
                                           RCFD_SHIFT_TYPE_PREPARATION);

                    for(i_dim = 0; i_dim < 3; i_dim++){

                        shifts_from_my_node_coords[i_shift_from_my_node][i_dim] = global_shifts_coords[i_shift][i_dim];
                    }

                    i_shift_from_my_node++;
                }
            }

            error = prep_interpolate_shifts_inside_node(rcfd,
                        i_layer, i_phase,
                        &shifts_from_my_node, &shifts_from_my_node_count,
                        shifts_from_my_node_coords);

            if(error){

                report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                    "prep_interpolate_shifts_to_my_node: interpolate_shifts_inside_node failed with error %d",
                    error);
            }
        }
    }

    // gather and bcast new shifts
    {
        free_lists_of_shift(rcfd, &global_shifts);

        global_shifts_count = PRF_GISUM1(shifts_from_my_node_count);

        shift_allocate_lists(rcfd, &global_shifts, global_shifts_count, RCFD_SHIFT_TYPE_PREPARATION);

        parallel_gatherv0_shifts(rcfd,
                                 &shifts_from_my_node,
                                 shifts_from_my_node_count,
                                 &global_shifts,
                                 RCFD_SHIFT_TYPE_PREPARATION);

        parallel_bcast0_shifts(rcfd, &global_shifts, global_shifts_count, RCFD_SHIFT_TYPE_PREPARATION);
    }

    // fill shifts_to_my_node, adjust shifts_to_my_node_count
    {
        i_shift_to_my_node = 0;

        for(i_shift = 0; i_shift < global_shifts_count; i_shift++){

            if(global_shifts.node1[i_shift] == myid){

                shift_copy_one_in_list(rcfd,
                                       shifts_to_my_node, i_shift_to_my_node,
                                       &global_shifts,    i_shift,
                                       RCFD_SHIFT_TYPE_PREPARATION);

                i_shift_to_my_node++;
            }
        }

        *shifts_to_my_node_count = i_shift_to_my_node;

        report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
             "[%d] prep_interpolate_shifts_to_my_node: global_shifts_count %d, i_shift_to_my_node %d",
             myid, global_shifts_count, i_shift_to_my_node);
    }

    // free local vars
    {
        free_lists_of_shift(rcfd, &global_shifts);

        free_r_2d(global_shifts_coords);

        if(shifts_from_my_node_count > 0){

            free_lists_of_shift(rcfd, &shifts_from_my_node);

            free_r_2d(shifts_from_my_node_coords);
        }
    }

    return 0;

#endif // RP_NODE
    return -1;
}

/**
 * @brief Generate additional cell-to-cell shifts via interpolation for more
 *        accurate shift patterns.
 *
 * @param rcfd  pointer to rCFD instance
 * @return 0 if success or -1 if error
 */
int prep_interpolate_shifts(rCFD_type *rcfd)
{
#if RP_NODE

#define DEBUG_LOCAL 0

    if((!rcfd->Solver_Dict->interpolation_on) ||
       (!rcfd->Solver_Dict->shifts_from_displacement_fields_on)) return -1;

#if 1
    enum arg_flags{
        pre_interpolation,
        post_interpolation
    };

    int     error = 0;
    int     i_phase, i_dim, i_shift, i_shift_inside_node, i_shift_to_my_node;
    int     i_layer = 0;
    int     node0, node1, c1;
    int     phase_count = rcfd->Solver_Dict->number_of_phases;
    int     shifts_before_interpolation_count;
    int     shifts_inside_node_count;
    int     shifts_to_my_node_count;

    double  **shifts_inside_node_coord = NULL;
    double  **shifts_to_my_node_coord =  NULL;

    C2C_Shift_type  shifts_inside_node, shifts_to_my_node;
#endif

    // generate interpolated cell-to-cell shifts for all phases
    for(i_phase = 0; i_phase < phase_count; i_phase++){

        // propagate information to get initial displacement pattern (only intra-process)
        prep_test_displacement(rcfd, i_phase, pre_interpolation);

        // create new c2c shifts by interpolation
        {
            // count  shifts_inside_node, shifts_to_my_node
            {
                i_shift_inside_node = 0;
                i_shift_to_my_node =  0;

                shifts_before_interpolation_count = rcfd->C2C_Prep[i_phase].valid_shifts_count;

                for(i_shift = 0; i_shift < shifts_before_interpolation_count; i_shift++){

                    node0 = rcfd->C2C_Prep[i_phase].node0[i_shift];
                    node1 = rcfd->C2C_Prep[i_phase].node1[i_shift];

                    if(node0 == node1){

                        i_shift_inside_node++;
                    }
                    else if(node1 == myid){

                        i_shift_to_my_node++;
                    }
                }

                shifts_inside_node_count = i_shift_inside_node;
                shifts_to_my_node_count =  i_shift_to_my_node;
            }

            // alloc  shifts_inside_node(_coord), shifts_to_my_node(_coord)
            {
                // note that one primary shift will result in 4 shifts after interpolation
                // if shifts_inside_node_count == 0, this will just initialize shifts_inside_node_count
                shift_allocate_lists(rcfd, &shifts_inside_node, 4 * shifts_inside_node_count, RCFD_SHIFT_TYPE_PREPARATION);

                if(shifts_inside_node_count > 0){

                    shifts_inside_node_coord = malloc_r_2d(shifts_inside_node_count, 3);
                }

                // note that one primary shift will result in 4 shifts after interpolation
                // if shifts_to_my_node_count == 0, this will just initialize shifts_to_my_node
                shift_allocate_lists(rcfd, &shifts_to_my_node, 4 * shifts_to_my_node_count, RCFD_SHIFT_TYPE_PREPARATION);

                if(shifts_to_my_node_count > 0){

                    shifts_to_my_node_coord = malloc_r_2d(shifts_to_my_node_count, 3);
                }
            }

            // fill  shifts_inside_node(_coord), shifts_to_my_node(_coord)
            {
                i_shift_inside_node = 0;
                i_shift_to_my_node =  0;

                for(i_shift = 0; i_shift < shifts_before_interpolation_count; i_shift++){

                    node0 = rcfd->C2C_Prep[i_phase].node0[i_shift];
                    node1 = rcfd->C2C_Prep[i_phase].node1[i_shift];

                    if(node0 == node1){

                        shift_copy_one_in_list(rcfd,
                                              &shifts_inside_node, i_shift_inside_node,
                                              &rcfd->C2C_Prep[i_phase], i_shift,
                                               RCFD_SHIFT_TYPE_PREPARATION);

                        c1 = shifts_inside_node.c1[i_shift_inside_node];

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            shifts_inside_node_coord[i_shift_inside_node][i_dim] = _C(i_layer).cfd_displ[i_phase][c1][i_dim];
                        }

                        i_shift_inside_node++;
                    }
                    else if(node1 == myid){

                        shift_copy_one_in_list(rcfd,
                                              &shifts_to_my_node, i_shift_to_my_node,
                                              &rcfd->C2C_Prep[i_phase], i_shift,
                                               RCFD_SHIFT_TYPE_PREPARATION);

                        c1 = shifts_to_my_node.c1[i_shift_to_my_node];

                        for(i_dim = 0; i_dim < 3; i_dim++){

                            shifts_to_my_node_coord[i_shift_to_my_node][i_dim] = _C(i_layer).cfd_displ[i_phase][c1][i_dim];
                        }

                        i_shift_to_my_node++;
                    }
                }
            }

            // interpolate  shifts_inside_node, shifts_to_my_node
            {
                error = prep_interpolate_shifts_inside_node(rcfd,
                                                            i_layer,
                                                            i_phase,
                                                            &shifts_inside_node,
                                                            &shifts_inside_node_count,
                                                            shifts_inside_node_coord);
                if(error){

                    report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                                 "shift_interpolate: interpolate_shifts_inside_node failed with error %d", error);
                }

                error = prep_interpolate_shifts_to_my_node(rcfd,
                                                           i_layer,
                                                           i_phase,
                                                           &shifts_to_my_node,
                                                           &shifts_to_my_node_count,
                                                           shifts_to_my_node_coord);

                if(error){

                    report_error(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                                 "shift_interpolate: interpolate_shifts_to_my_node failed with error %d", error);
                }

            }

            // refill rcfd->C2C_Prep[i_phase]
            {
                shift_copy_sequence_in_list(rcfd,
                                            &rcfd->C2C_Prep[i_phase], 0,
                                            &shifts_inside_node,      0, shifts_inside_node_count,
                                            RCFD_SHIFT_TYPE_PREPARATION);

                shift_copy_sequence_in_list(rcfd,
                                            &rcfd->C2C_Prep[i_phase], shifts_inside_node_count,
                                            &shifts_to_my_node,       0, shifts_to_my_node_count,
                                            RCFD_SHIFT_TYPE_PREPARATION);

                rcfd->C2C_Prep[i_phase].valid_shifts_count = shifts_inside_node_count + shifts_to_my_node_count;
            }

            // free shifts_inside_node(_coord), shifts_to_my_node(_coord)
            {
                free_lists_of_shift(rcfd, &shifts_inside_node);
                free_r_2d(shifts_inside_node_coord);
                shifts_inside_node_coord = NULL;

                free_lists_of_shift(rcfd, &shifts_to_my_node);
                free_r_2d(shifts_to_my_node_coord);
                shifts_to_my_node_coord = NULL;
            }

            // check shifts
            if(DEBUG_LOCAL){

                if(shift_is_valid(rcfd, &rcfd->C2C_Prep[i_phase], RCFD_SHIFT_TYPE_PREPARATION)){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                        "[%d] prep_interpolate_shifts: rcfd->C2C_Prep[%d] - all shifts are valid",
                        myid, i_phase);
                }
                else{

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_CREATE_SHIFT,
                        "[%d] prep_interpolate_shifts: rcfd->C2C_Prep[%d] - found invalid shifts",
                        myid, i_phase);
                }
            }
        }

        // propagate information to get final displacement pattern (only intra-process)
        prep_test_displacement(rcfd, i_phase, post_interpolation);

    }  // phase loop
#undef DEBUG_LOCAL

#endif // RP_NODE

    return 0;
}

/**
 * @brief set local diffusion field to diffusion stemming from interpolation
 *
 * @param rcfd  pointer to rCFD instance
 * @param local_diff  (to be set) cell diffusion vector
 * @param shifts  array of cell-to-cell shifts
 * @param timestep_size the time that passed for the first and second moment
 * @return 0 or -1 if error
 */
int prep_set_artificial_diffusivity(rCFD_type *rcfd, double *local_diff, C2C_Shift_type *shifts, double timestep_size)
{
#if RP_NODE

#if 1   // local vars init
    int     i_shift, i_dim, i_cell;

    int     i_layer = rcfd->Solver->current_layer;

    int     shifts_count = shifts->valid_shifts_count;
    int     cell_count = _CELL_DICT(i_layer).number_of_cells;

    int     c1;

    int     *shifts_to_c1_count = malloc(cell_count * sizeof(int));

    double  *c0_coord_diff_sqr =  malloc(cell_count * sizeof(double));
    double  *c1_w0_sum =          malloc(cell_count * sizeof(double));

    double  **c0_coord_mean =     malloc_r_2d(cell_count, 3);
#endif

    // loop shifts and set shifts_to_c1_count, accumulate c0_coords_mean
    {
        memset(shifts_to_c1_count, 0, cell_count * sizeof(int));

        memset(c0_coord_mean[0], 0, 3 * cell_count * sizeof(double));
        memset(c1_w0_sum,        0,     cell_count * sizeof(double));

        for(i_shift = 0; i_shift < shifts_count; i_shift++){

            if(shifts->w0[i_shift] > 0.0){

                c1 = shifts->c1[i_shift];

                shifts_to_c1_count[c1]++;

                for(i_dim = 0; i_dim < 3; i_dim++){

                    c0_coord_mean[c1][i_dim] += shifts->w0[i_shift] * shifts->c0_coords[i_shift][i_dim];
                }

                c1_w0_sum[c1] += shifts->w0[i_shift];
            }
        }
    }

    // loop cells and get c0_coords_mean
    {
        for(i_cell = 0; i_cell < cell_count; i_cell++){

            if(c1_w0_sum[i_cell] > 0.0){

                for(i_dim = 0; i_dim < 3; i_dim++){

                    c0_coord_mean[i_cell][i_dim] /= c1_w0_sum[i_cell];
                }
            }
        }
    }

    // loop shifts and accumulate c0_coord_diff_sqr
    {
        memset(c0_coord_diff_sqr, 0, cell_count * sizeof(double));

        for(i_shift = 0; i_shift < shifts_count; i_shift++){

            if(shifts->w0[i_shift] > 0.0){

                c1 = shifts->c1[i_shift];

                for(i_dim = 0; i_dim < 3; i_dim++){

                    c0_coord_diff_sqr[c1] += shifts->w0[i_shift]
                        * (shifts->c0_coords[i_shift][i_dim] - c0_coord_mean[c1][i_dim])
                        * (shifts->c0_coords[i_shift][i_dim] - c0_coord_mean[c1][i_dim]);
                }
            }
        }
    }

    // loop cells, get mean c0_coord_diff_sqr, set local_diff
    {
        for(i_cell = 0; i_cell < cell_count; i_cell++){

            if(c1_w0_sum[i_cell] > 0.0){

                c0_coord_diff_sqr[i_cell] /= c1_w0_sum[i_cell];
            }

            if(shifts_to_c1_count[i_cell] > 1){

                local_diff[i_cell] = c0_coord_diff_sqr[i_cell]
                    * (double)(shifts_to_c1_count[i_cell]) / (double)(shifts_to_c1_count[i_cell] - 1)
                    / (2.0 * timestep_size);
            }
            else{

                local_diff[i_cell] = 0.0;
            }
        }
    }

    // free local vars
    {
        free(shifts_to_c1_count);
        free(c0_coord_diff_sqr);
        free(c1_w0_sum);

        free_r_2d(c0_coord_mean);
    }

    return 0;
#endif
    return -1;
}

/**
 * @brief Uses the first and second moment of the propagator of a field to calculate the cell-wise diffusion constant
 *        Uses the following approximation: diff_const = tr(diff_ij - displ_i * displ_j) / (2 * timestep_size)
 *
 * @param rcfd  pointer to rCFD instance
 * @param local_diff  (to be set) cell diffusion vector
 * @param cfd_displ_diff  6D vector of the displacement (first moment of the propagator) and
 *                        the second moment of the propagator (xx, yy and zz components) for all cells
 * @param timestep_size the time that passed for the first and second moment
 * @return 0 or -1 if error
 */
int prep_set_lagrangian_diffusivity(rCFD_type *rcfd, double *local_diff, double **cfd_displ_diff, double timestep_size)
{
#if RP_NODE
    int    i_dim, i_cell;

    int    i_layer = rcfd->Solver->current_layer;

    int    cell_count = _CELL_DICT(i_layer).number_of_cells;

    double diff_const;

    for(i_cell = 0; i_cell < cell_count; i_cell++){

        diff_const = 0.0;

        for(i_dim = 0; i_dim < 3; i_dim++){

            diff_const += cfd_displ_diff[i_cell][i_dim + 3]
                - cfd_displ_diff[i_cell][i_dim] * cfd_displ_diff[i_cell][i_dim];
        }

        local_diff[i_cell] = diff_const / (2.0 * timestep_size);
    }

    return 0;
#endif
    return -1;
}

/**
 * @brief Generate local diffusivity field based on displacement patterns.
 *
 * @param rcfd  pointer to rCFD instance
 * @retval 0 or -1 if error
 */
int prep_set_local_diffusivity(rCFD_type *rcfd)
{
#if RP_NODE

#if 1
    int i_phase, i_cell;

    int i_layer = 0;

    int phase_count = rcfd->Solver_Dict->number_of_phases;
    int cell_count = _CELL_DICT(i_layer).number_of_cells;

    double timestep_size = rcfd->Solver_Dict->global_time_step;

    double diff_local;

    double *diff_lagrangian_phase = NULL;
    double *diff_artificial_phase = NULL;

    double **cfd_displ_diff = NULL;
#endif

    if(!rcfd->Solver_Dict->local_diff_on){

        return -1;
    }

    diff_lagrangian_phase = malloc(cell_count * sizeof(double));
    diff_artificial_phase = malloc(cell_count * sizeof(double));

    for(i_phase = 0; i_phase < phase_count; i_phase++){

        // A . set Lagrangian diffusion based on convection of x and xx
        {
            cfd_displ_diff = _C(i_layer).cfd_displ_diff[i_phase];

            memset(diff_lagrangian_phase, 0, cell_count * sizeof(double));

            prep_set_lagrangian_diffusivity(rcfd,
                                            diff_lagrangian_phase,
                                            cfd_displ_diff,
                                            timestep_size);
        }

        // B. estimate artificial (numerical) diffusion based on variance of sending cells' coordinates
        {
            memset(diff_artificial_phase, 0, cell_count * sizeof(double));

            prep_set_artificial_diffusivity(rcfd,
                                            diff_artificial_phase,
                                            &rcfd->C2C_Prep[i_phase],
                                            timestep_size);
        }

        // C. set local diffusion = lag_diff - num_diff > 0
        {
            memset(_C(i_layer).local_diff[0][i_phase], 0, sizeof(double) * cell_count);

            for(i_cell = 0; i_cell < cell_count; i_cell++){

                diff_local = diff_lagrangian_phase[i_cell] - diff_artificial_phase[i_cell];

                if(diff_local > 0.0){

                    _C(i_layer).local_diff[0][i_phase][i_cell] = diff_local;
                }
            }
        }
    }

    free(diff_lagrangian_phase);
    free(diff_artificial_phase);

    return 0;
#endif
    return -1;
}

/**
 * @brief write the recurrence matrix, recurrence path and recurrence path
 *        differences to files
 *
 * @param rcfd  pointer to rCFD instance
 * @return 0 if OK or -1 if error or called by host
 */
int prep_write_recurrence(rCFD_type *rcfd)
{
#if RP_NODE

#if 1 // local vars
    int     error = 0;

    int     i_norm, i_tmp, i_frame, i_frame2 = 0;

    int     i_state = rcfd->Solver->current_state;
    int     rec_format = rcfd->Rec_Dict->format;

    int     frame_count = rcfd->Solver->current_frame;
    int     norm_count =  rcfd->Norms->number_of_norms;

    int     *rec_jump = NULL;

    double  norm, diff, mean_norm, sum_of_all_norms;
    double  *rec_jump_diff = NULL;
    double  **rec_matrix = NULL, **norm_database = NULL;

    char    file_name[80];
    FILE    *f_in = NULL, *f_out = NULL;
#endif

    if(frame_count == 0){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                        "[%d] prep_write_recurrence: No current frame calculations found."
                        " Assuming a posteriori calculation of recurrence properties"
                        " for %d databse entries.",
                        myid, rcfd->Solver_Dict->number_of_frames);
        frame_count = rcfd->Solver_Dict->number_of_frames;
    }
    else if(frame_count != rcfd->Solver_Dict->number_of_frames){

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                        "[%d] prep_write_recurrence: Mismatch between desired frames (%d)"
                        " and calculated frames (%d) in database.",
                        myid, rcfd->Solver_Dict->number_of_frames, frame_count);
    }

    // read norm values into norm_database
    {
        norm_database = malloc_r_2d(frame_count, norm_count);

        sprintf(file_name,"%s_%d_%d", rcfd->File_Dict->Norm_filename, i_state, myid);

        f_in = fopen(file_name,"r");

        if(f_in == NULL){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                         "[%d] prep_write_recurrence: failed to open: '%s'.",
                         myid, file_name);

            error = -1;
        }
        else{

            for(i_frame = 0; i_frame < frame_count; i_frame++){

                if(fscanf(f_in, "%d \n", &i_tmp) != 1){ // read norm_count

                    report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                                 "[%d] prep_write_recurrence: failed to read norm for frame %d",
                                 myid, i_frame);
                    error = -1;
                    break;
                }

                for(i_norm = 0; i_norm < norm_count; i_norm++){

                    if(fscanf(f_in, "%le \n", &norm_database[i_frame][i_norm]) != 1){

                        report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                                     "[%d] prep_write_recurrence: failed to read norm %d for frame %d",
                                     myid, i_norm, i_frame);
                        error = -1;
                        i_frame = frame_count;
                        break;
                    }
                }
            }

            fclose(f_in);
        }

        error = PRF_GILOW1(error);

        if(error){

            free_r_2d(norm_database);

            return error;
        }
    }

    // calc rec_matrix
    {
        if(myid == 0){

            rec_matrix = malloc_r_2d(frame_count, frame_count);
        }

        sum_of_all_norms = 0.0;

        for(i_frame = 0; i_frame < frame_count; i_frame++){

            for(i_norm = 0; i_norm < norm_count; i_norm++){

                sum_of_all_norms += norm_database[i_frame][i_norm];
            }
        }

        mean_norm = PRF_GRSUM1(sum_of_all_norms) /
            (double) PRF_GISUM1(norm_count) /
            (double) frame_count;

        for(i_frame = 0; i_frame < frame_count; i_frame++){

            for(i_frame2 = 0; i_frame2 < frame_count; i_frame2++){

                norm = 0.0;

                for(i_norm = 0; i_norm < norm_count; i_norm++){

                    diff = fabs(norm_database[i_frame][i_norm] - norm_database[i_frame2][i_norm]);

                    if(rcfd->Norm_Dict->larger_than_mean_norm_only){

                        if(diff > mean_norm){

                            norm += diff;
                        }
                    }
                    else{

                        norm += diff;
                    }
                }

                norm = PRF_GRSUM1(norm);

                if(myid == 0){

                    rec_matrix[i_frame][i_frame2] = norm;
                    rec_matrix[i_frame2][i_frame] = norm;
                }
            }
        }
    }

    // calc jumps
    if(myid == 0){

        rec_jump =      malloc(frame_count * sizeof(int));
        rec_jump_diff = malloc(frame_count * sizeof(double));

        switch(rec_format){

            case RCFD_REC_QUARTER_JUMPS:
            {
                int N1_4=(int)floor((double)frame_count/4.);
                int N2_4=(int)floor((double)frame_count/2.);
                int N3_4=(int)floor((double)frame_count*3./4.);

                int j_frame, j_frame_min = 0;

                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    diff = 1e10; /* something large */

                    if(i_frame <= N1_4){

                        for(j_frame = N2_4; j_frame < N3_4; j_frame++){

                            if(rec_matrix[i_frame][j_frame] < diff){

                                diff = rec_matrix[i_frame][j_frame];
                                j_frame_min = j_frame;
                            }
                        }
                    }
                    else if(i_frame <= N2_4){

                        for(j_frame = N3_4; j_frame < frame_count; j_frame++){

                            if(rec_matrix[i_frame][j_frame] < diff){

                                diff = rec_matrix[i_frame][j_frame];
                                j_frame_min = j_frame;
                            }
                        }
                    }
                    else if(i_frame <= N3_4){

                        for(j_frame = 0; j_frame < N1_4; j_frame++){

                            if(rec_matrix[i_frame][j_frame] < diff){

                                diff = rec_matrix[i_frame][j_frame];
                                j_frame_min = j_frame;
                            }
                        }
                    }
                    else{

                        for(j_frame = N1_4; j_frame < N2_4; j_frame++){

                            if(rec_matrix[i_frame][j_frame] < diff){

                                diff = rec_matrix[i_frame][j_frame];
                                j_frame_min = j_frame;
                            }
                        }
                    }

                    rec_jump[i_frame] = j_frame_min;
                    rec_jump_diff[i_frame] = diff;
                }

                break;
            }
            case RCFD_REC_OFF_DIAGONAL_BAND:
            {
                int j_frame, j_frame_min = 0;

                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    diff = 1e10;

                    if(i_frame > rcfd->Rec_Dict->off_diagonal_band_width){

                        for(j_frame = 0; j_frame < (i_frame - rcfd->Rec_Dict->off_diagonal_band_width); j_frame++){

                            if(rec_matrix[i_frame][j_frame] < diff){

                                diff = rec_matrix[i_frame][j_frame];
                                j_frame_min = j_frame;
                            }
                        }
                    }

                    if(i_frame < (frame_count - rcfd->Rec_Dict->off_diagonal_band_width)){

                        for(j_frame = (i_frame + rcfd->Rec_Dict->off_diagonal_band_width);
                            j_frame < frame_count;
                            j_frame++){

                            if(rec_matrix[i_frame][j_frame] < diff){

                                diff = rec_matrix[i_frame][j_frame];
                                j_frame_min = j_frame;
                            }
                        }
                    }

                    rec_jump[i_frame] = j_frame_min;
                    rec_jump_diff[i_frame] = diff;
                }

                break;
            }
            case RCFD_REC_REPLAY:
            {
                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    if(i_frame < (frame_count - 1)){

                        rec_jump[i_frame] = i_frame + 1;
                        rec_jump_diff[i_frame] = rec_matrix[i_frame][(i_frame + 1)];
                    }
                    else{

                        rec_jump[i_frame] = 0;
                        rec_jump_diff[i_frame] = rec_matrix[i_frame][0];
                    }
                }
                break;
            }

            default: break;
        }
    }

    // write matrix, jumps
    if(myid == 0){

        // rec matrix
        {
            sprintf(file_name, "%s_%d_%d_%d", rcfd->File_Dict->Matrix_filename, i_state, i_state, 0);

            f_out = fopen(file_name, "w");

            if(f_out == NULL){

                report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                             "prep_write_recurrence: failed to open: '%s'.", file_name);
                error = -1;
            }
            else{

                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    for(i_frame2 = 0; i_frame2 < frame_count; i_frame2++){

                        fprintf(f_out, "%e ", rec_matrix[i_frame][i_frame2]);
                    }

                    fprintf(f_out, "\n");
                }

                report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                             "prep_write_recurrence: wrote %dx%d recurrence matrix to '%s'",
                             i_frame, i_frame2, file_name);

                fclose(f_out);
            }
        }

        // rec path
        {
            sprintf(file_name, "%s_%d_%d_%d", rcfd->File_Dict->Jump_filename, i_state, i_state, 0);

            f_out = fopen(file_name, "w");

            if(f_out == NULL){

                report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                             "prep_write_recurrence: failed to open: '%s'.",
                             file_name);

                error = -1;
            }
            else{

                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    fprintf(f_out, "%d \n", rec_jump[i_frame]);
                }

                report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                             "prep_write_recurrence: wrote %d path entries to '%s'",
                             i_frame, file_name);

                fclose(f_out);
            }
        }

        // rec path diff
        {
            sprintf(file_name, "%s_diff_%d_%d_%d", rcfd->File_Dict->Jump_filename, i_state, i_state, 0);

            f_out = fopen(file_name, "w");

            if(f_out == NULL){

                report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                             "prep_write_recurrence: failed to open: '%s'.",
                             file_name);

                error = -1;
            }
            else{

                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    fprintf(f_out, "%e \n", rec_jump_diff[i_frame]);
                }

                report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                             "prep_write_recurrence: wrote %d path diff entries to '%s'",
                             i_frame, file_name);

                fclose(f_out);
            }
        }

    }

    // free local vars
    {
        free_r_2d(norm_database);
        free_r_2d(rec_matrix);
        free(rec_jump);
        free(rec_jump_diff);
    }

    error = PRF_GILOW1(error);

    return error;
#endif
    return -1;
}

/**
 * @brief set the norms, vof, local diffusion and user-defined recurrence fields for the current frame
 * @param rcfd  pointer to rCFD instance
 * @param norm  array of cell norms; may be NULL if set directly or via user defined function
 * @param vof   vof field for each phase ([i_cell][i_phase]); may be NULL in case of single-phase or if set directly
 * @param local_diffusion  local diffusion field ([i_phase][i_cell]); may be NULL if local diffusion is turned off or if set directly
 * @retval -1 Error
 * @retval 0 OK
 */
int prep_set_fields(rCFD_type *rcfd, double *norm, double **vof, double **local_diffusion)
{
#if RP_NODE
    int i_cell, i_phase, i_layer = 0;

    if(rcfd->Norm_Dict->format == RCFD_NORM_DEFAULT){

        if(norm){

            memcpy(rcfd->Norms->norm, norm, sizeof(double) * rcfd->Norms->number_of_norms);
        }
    }
    else{ // RCFD_NORM_USER

        rcfd->rCFD_UDF->_user_set_norm(rcfd);
    }

    if((rcfd->Solver_Dict->number_of_phases > 1) || (rcfd->Solver_Dict->single_phase_vof)){

        if(vof){

            // must not assume that input array is contiguous memory so loop instead of memcpy
            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; ++i_cell){

                for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

                    _C(i_layer).vof[0][i_cell][i_phase] = vof[i_cell][i_phase];
                }
            }
        }
    }

    if(rcfd->Solver_Dict->local_diff_on){

        if(local_diffusion){

            // must not assume that input array is contiguous memory so loop instead of memcpy
            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; ++i_phase){

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; ++i_cell){

                    _C(i_layer).local_diff[0][i_phase][i_cell] = local_diffusion[i_phase][i_cell];
                }
            }
        }
        else if(rcfd->Solver_Dict->shifts_from_displacement_fields_on &&
                rcfd->Solver_Dict->local_diff_format == RCFD_LOCAL_DIFF_FIELD_BASED){

            prep_set_local_diffusivity(rcfd);
        }
    }

    rcfd->rCFD_UDF->_user_set_rec_vars(rcfd, 0);

#endif // RP_NODE

    return 0;
}

/**
 * @brief write a database entry for the norms of the current frame
 * @param rcfd  pointer to rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int prep_write_norm(rCFD_type *rcfd)
{
#if RP_NODE
    int i_norm;
    int i_layer = 0;
    int i_frame = rcfd->Solver->current_frame;
    int i_state = rcfd->Solver->current_state;
    int total_number_of_norms;
    int int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
    int norm_count = int_cell_count;
    char file_name[80];
    FILE *f_out = NULL;

    sprintf(file_name,"%s_%d_%d", rcfd->File_Dict->Norm_filename, i_state, myid);

    f_out = fopen(file_name, (i_frame == 0) ? "w" : "a");

    if(f_out == NULL){

        report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS, "[%d] prep_write_norm: failed to open file %s", myid, file_name);
        return -1;
    }

    fprintf(f_out, "%d \n", norm_count);

    for(i_norm = 0; i_norm < norm_count; i_norm++){

        fprintf(f_out,"%e \n", rcfd->Norms->norm[i_norm]);
    }

    fclose(f_out);

    total_number_of_norms = PRF_GISUM1(norm_count);

    report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS, "prep_write_norm: wrote norm frame # %d with %d values", i_frame, total_number_of_norms);

#endif // RP_NODE

    return 0;
}

/**
 * @brief write a database entry for the vof field(s) of the current frame
 * @param rcfd  pointer to rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int prep_write_vof(rCFD_type *rcfd)
{
#if RP_NODE
    if((rcfd->Solver_Dict->number_of_phases > 1) || (rcfd->Solver_Dict->single_phase_vof)){

        int i_phase, i_cell;
        int i_layer = 0;
        int i_frame = rcfd->Solver->current_frame;
        int int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
        int total_number_of_cells;
        int i_state = rcfd->Solver->current_state;
        char file_name[80];
        FILE *f_out = NULL;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            sprintf(file_name, "%s_%d_%d_%d", rcfd->File_Dict->vof_filename, i_state, i_phase, myid);

            f_out = fopen(file_name, (i_frame == 0) ? "w" : "a");

            if(f_out == NULL){

                report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                             "[%d] prep_write_vof: failed to open file %s", myid, file_name);
                return -1;
            }

            fprintf(f_out,"%d \n", _CELL_DICT(i_layer).number_of_cells);

            for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                fprintf(f_out,"%e \n", _C(i_layer).vof[0][i_cell][i_phase]);
            }

            fclose(f_out);

            total_number_of_cells = PRF_GISUM1(int_cell_count);

            report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "prep_write_vof: wrote vof frame # %d of phase %d with %d values",
                         i_frame, i_phase, total_number_of_cells);
        }
    }
#endif // RP_NODE

    return 0;
}

/**
 * @brief write a database entry for the local diffusion field of the current frame
 * @param rcfd  pointer to rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int prep_write_local_diffusion(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Solver_Dict->local_diff_on){

        int i_phase, i_cell;
        int total_number_of_cells;
        int i_layer = 0;
        int i_frame = rcfd->Solver->current_frame;
        int int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
        int i_state = rcfd->Solver->current_state;
        char file_name[80];
        FILE *f_out = NULL;

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            sprintf(file_name,"%s_%d_%d_%d", rcfd->File_Dict->local_diff_filename, i_state, i_phase, myid);

            f_out = fopen(file_name, (i_frame == 0) ? "w" : "a");

            if(f_out == NULL){

                report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                             "[%d] prep_write_local_diffusion: failed to open file %s", myid, file_name);
                return -1;
            }

            fprintf(f_out, "%d \n", int_cell_count);

            for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                    fprintf(f_out, "%e \n", _C(i_layer).local_diff[0][i_phase][i_cell]);
            }

            fclose(f_out);

            total_number_of_cells = PRF_GISUM1(int_cell_count);

            report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "prep_write_local_diffusion: wrote local_diff frame # %d of phase %d with %d values",
                         i_frame, i_phase, total_number_of_cells);
        }
    }
#endif // RP_NODE

    return 0;
}

/**
 * @brief write a database entry for the user-defined recurrence field(s) of the current frame
 * @param rcfd  pointer to rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int prep_write_rec_user(rCFD_type *rcfd)
{
#if RP_NODE
    int i_layer = 0;
    int rec_user_count = _CELL_DICT(i_layer).number_of_rec_user_vars;

    if(rec_user_count > 0){

        int i_rec_user, i_cell;
        int i_state = rcfd->Solver->current_state;
        int i_frame = rcfd->Solver->current_frame;
        int int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
        char file_name[80];
        FILE *f_out = NULL;

        sprintf(file_name, "%s_%d_%d", rcfd->File_Dict->rec_user_filename, i_state, myid);

        f_out = fopen(file_name, (i_frame == 0) ? "w" : "a");

        if(f_out == NULL){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                         "[%d] prep_write_rec_user: failed to open file %s", myid, file_name);
            return -1;
        }

        fprintf(f_out, "%d \n", int_cell_count);

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            for(i_rec_user = 0; i_rec_user < rec_user_count; i_rec_user++){

                fprintf(f_out, "%e ", _C(i_layer).rec_user[0][i_cell][i_rec_user]);
            }

            fprintf(f_out, "\n");
        }

        fclose(f_out);

        int_cell_count = PRF_GISUM1(int_cell_count);

        report_info0(rcfd, RCFD_REPORT_CATEGORY_WRITE_FIELDS,
                     "prep_write_rec_user: wrote rec_user frame # %d with %d rec user vars and %d values each",
                     i_frame, rec_user_count, int_cell_count);
    }
#endif // RP_NODE

    return 0;
}

/**
 * @brief write a database entry for norms, vof, local diffusion and user-defined recurrence fields of the current frame
 * @param rcfd  pointer to rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int prep_write_fields(rCFD_type *rcfd)
{
    int success = 0;

#if RP_NODE

    success |= prep_write_norm(rcfd);
    success |= prep_write_vof(rcfd);
    success |= prep_write_local_diffusion(rcfd);
    success |= prep_write_rec_user(rcfd);

#endif // RP_NODE

    return success;
}

/**
 * @brief write metadata file for generated database; the metadata file is a
 * TOML file structured in the following way
@verbatim
metadata_version = 1.0

[general]
source = "rCFD 26.02.0"
created_at = 2026-01-01T10:00:00

[[database]]
partitions = 8
frames = 100
[database.files.c2c]
file_version = 1.0
file_type = "binary"
shift_format = 0x00010414
shift_types = 2
@endverbatim
 * @param rcfd  pointer to rCFD instance
 * @retval -1 Error
 * @retval 0 OK
 */
int prep_write_metadata(rCFD_type *rcfd)
{
#if RP_NODE

    if(myid == 0){

        FILE *fmeta = NULL;
        const char *filename = rcfd->File_Dict->metadata_filename;

        fmeta = fopen(filename, "w");

        if(fmeta == NULL){

            report_error0(rcfd, RCFD_REPORT_CATEGORY_NOTSET,
                         "prep_write_metadata: failed to open file %s", filename);
            return -1;
        }

        // metadata file format version
        fprintf(fmeta, "metadata_version = %d.%d\n",
                RCFD_METADATA_FILE_VERSION_MAJOR, RCFD_METADATA_FILE_VERSION_MINOR);
        fprintf(fmeta, "\n");

        // general section
        {
            // get timestamp in iso format
            time_t now;
            time(&now);
            char ldt[sizeof "2026-01-01T10:00:00.123456Z"];
            //strftime(buf, sizeof buf, "%FT%T", gmtime(&now)); // UTC
            strftime(ldt, sizeof ldt, "%FT%T", localtime(&now));

            fprintf(fmeta, "[general]\n");
            fprintf(fmeta, "source = \"%s\"\n", RCFD_VERSION_STRING);
            fprintf(fmeta, "created_at = %s\n", ldt);
            fprintf(fmeta, "\n");
        }

        // files section
        {
            if(rcfd->Solver->current_frame != rcfd->Solver_Dict->number_of_frames){

                report_warning(rcfd, RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
                               "[%d] prep_write_metadata: Mismatch between desired frames (%d)"
                               " and calculated frames (%d) in database.",
                               myid, rcfd->Solver_Dict->number_of_frames, rcfd->Solver->current_frame);
            }

            fprintf(fmeta, "[[database]]\n");
            fprintf(fmeta, "partitions = %d\n", compute_node_count);
            fprintf(fmeta, "frames = %d\n", rcfd->Solver->current_frame);
            // c2c file description
            fprintf(fmeta, "[database.files.c2c]\n");
            fprintf(fmeta, "file_version = %d.%d\n",
                    RCFD_C2C_FILE_VERSION_MAJOR, RCFD_C2C_FILE_VERSION_MINOR);
            fprintf(fmeta, "file_type = \"%s\"\n",
                    (rcfd->Solver->c2c_file_type == RCFD_FILE_ASCII) ? "ascii" : "binary");
            fprintf(fmeta, "shift_format = %#010x\n", rcfd->Solver->c2c_shift_format);
            fprintf(fmeta, "shift_types = %d\n", rcfd->Solver->c2c_shift_type_count);
            // add further file descriptions as needed ...
            fprintf(fmeta, "\n");
        }

        fclose(fmeta);
    }

#endif // RP_NODE

    return 0;
}

#endif // RCFD_PREPARE_H
