/******************************************************************************
 * @internal
 * @file        rCFD_dump.h
 * @copyright   (C) 2024
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_DUMP_H
#define RCFD_DUMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rCFD_types.h"
#include "rCFD_macros.h"

//#define UNSTRUCTURED_GRID_VTK // enable to use unstructured grid also if no nodes are defined
#define DOUBLE_PRECISION 0 // enable to write binary file type with double precision


/**
 * @brief Swap byte order of 4 byte data type Little Endian - Big Endian
 * @param data bytes to swap
 */
void swap4(char *data)
{
    const uint32_t ref32 = *((uint32_t*)data);
    *((uint32_t*)data) =
            (ref32 >> 24) | (ref32 << 24) | ((ref32 & 0x00ff0000) >> 8) | ((ref32 & 0x0000ff00) << 8);
}

/**
 * @brief Swap byte order of 8 byte data type Little Endian - Big Endian
 * @param data bytes to swap
 */
void swap8(char *data)
{
    const uint64_t ref64 = *((uint64_t*)data);
    *((uint64_t*)data) = (ref64 >> 56) | (ref64 << 56) |
            ((ref64 & 0x00ff000000000000) >> 40) | ((ref64 & 0x000000000000ff00) << 40) |
            ((ref64 & 0x0000ff0000000000) >> 24) | ((ref64 & 0x0000000000ff0000) << 24) |
            ((ref64 & 0x000000ff00000000) >> 8) | ((ref64 & 0x00000000ff000000) << 8);
}

/**
 * @brief Write the VTK file header, dataset type, simulation time and timestep to the given file
 * @param fout              pointer to open dump file
 * @param file_type         dump file type (ascii or binary)
 * @param dataset           type of data set (UNSTRUCTURED_GRID or POLYDATA)
 * @param simulation_time   time simulated so far
 * @param simulation_cycle  time steps simulated so far
 */
void dump_vtk_header(FILE *fout, int file_type, const char *dataset, double simulation_time, int simulation_cycle)
{
#if RP_NODE
    time_t current_time = time(NULL);
    char buffer[256];

    // use VisIt convention to store time information in legacy vtk files
    snprintf(buffer, 256,  "# vtk DataFile Version 3.0\n"
                            "rCFD data generated on %s"
                            "%s\n"
                            "DATASET %s\n"
                            "FIELD FieldData 2\n"
                            "TIME 1 1 double\n",
                            ctime(&current_time),
                            (file_type == RCFD_FILE_ASCII)?"ASCII":"BINARY",
                            dataset);

    if(file_type == RCFD_FILE_ASCII){

        fprintf(fout, "%s", buffer);
        fprintf(fout, "%f\n", simulation_time);
        fprintf(fout, "CYCLE 1 1 int\n");
        fprintf(fout, "%d\n", simulation_cycle);
    }
    else{

        fwrite(buffer, sizeof(char), strlen(buffer), fout);

        swap8((char*)&simulation_time);
        fwrite(&simulation_time, sizeof(double), 1, fout);

        snprintf(buffer, 256, "\nCYCLE 1 1 int\n");
        fwrite(buffer, sizeof(char), strlen(buffer), fout);

        swap4((char*)&simulation_cycle);
        fwrite(&simulation_cycle, sizeof(int), 1, fout);
        buffer[0] = '\n';
        fwrite(buffer, sizeof(char), 1, fout);
    }
#endif
}

/**
 * @brief Write the header for the current (legacy) VTK dump files
 * @param rcfd  pointer to rCFD instance
 * @param fout  pointer to open dump file
 */
void dump_vtk_header_multi(rCFD_type *rcfd, FILE *fout)
{
#if RP_NODE
    int i_layer = rcfd->Solver->current_layer;
    int n_nodes = _NODE_DICT(i_layer).number_of_nodes;

    {
#ifdef UNSTRUCTURED_GRID_VTK
        dump_vtk_header(fout, rcfd->Dump_Dict->dump_file_type, "UNSTRUCTURED_GRID", rcfd->Solver->global_time, rcfd->Solver->global_run_counter);
#else
        if(n_nodes > 0){

            dump_vtk_header(fout, rcfd->Dump_Dict->dump_file_type, "UNSTRUCTURED_GRID", rcfd->Solver->global_time, rcfd->Solver->global_run_counter);
        }
        else{

            dump_vtk_header(fout, rcfd->Dump_Dict->dump_file_type, "POLYDATA", rcfd->Solver->global_time, rcfd->Solver->global_run_counter);
        }
#endif
    }
#endif
}

/**
 * @brief Write the header for the current (legacy) VTK dump file
 * @param rcfd  pointer to rCFD instance
 * @param fout  pointer to open dump file on node 0
 */
void dump_vtk_header0(rCFD_type *rcfd, FILE *fout)
{
#if RP_NODE
    int i_layer = rcfd->Solver->current_layer;
    int n_nodes = PRF_GISUM1(_NODE_DICT(i_layer).number_of_nodes);

    if(myid == 0 && fout){
#ifdef UNSTRUCTURED_GRID_VTK
        dump_vtk_header(fout, rcfd->Dump_Dict->dump_file_type, "UNSTRUCTURED_GRID", rcfd->Solver->global_time, rcfd->Solver->global_run_counter);
#else
        if(n_nodes > 0){

            dump_vtk_header(fout, rcfd->Dump_Dict->dump_file_type, "UNSTRUCTURED_GRID", rcfd->Solver->global_time, rcfd->Solver->global_run_counter);
        }
        else{

            dump_vtk_header(fout, rcfd->Dump_Dict->dump_file_type, "POLYDATA", rcfd->Solver->global_time, rcfd->Solver->global_run_counter);
        }
#endif
    }
#endif
}

/**
 * @brief Write geometry points to the given file (3 points per line)
 * @param fout              pointer to open dump file
 * @param file_type         dump file type (ascii or binary)
 * @param x                 array of point coordinates
 * @param n_points  total   number of points to write
 * @param n_points_carry    number of points to fill up previous line
 */
void dump_vtk_topo_points(FILE *fout, int file_type, double **x, int n_points, int n_points_carry)
{
    int i, i_point = 0;

    if(file_type == RCFD_FILE_ASCII){

        int i_triple;

        // fill up last line from prev process if necessary
        if(n_points_carry > 0){

            for(i_point = 0; i_point < n_points_carry && i_point < n_points; ++i_point){

                fprintf(fout, "%f %f %f ", x[i_point][0], x[i_point][1], x[i_point][2]);
            }
            fprintf(fout, "\n");
        }
        // write 3 points per line (i.e. 9 values)
        // cell nodes if cell types are available, else cell centers
        for(i_triple = 0; i_triple < (n_points-n_points_carry)/3; ++i_triple){

            for(i = 0; i < 3; ++i){

                i_point = i_triple * 3 + i + n_points_carry;
                fprintf(fout, "%f %f %f ", x[i_point][0], x[i_point][1], x[i_point][2]);
            }
            fprintf(fout, "\n");
        }
        // write remaining points
        for(++i_point; i_point < n_points; ++i_point){

            fprintf(fout, "%f %f %f ", x[i_point][0], x[i_point][1], x[i_point][2]);
        }
    }
    else{

#if DOUBLE_PRECISION
        double BE_x;
#else
        float BE_x;
#endif // DOUBLE_PRECISION

        for(i_point = 0; i_point < n_points; ++i_point){

            for(i = 0; i < 3; ++i){

                BE_x =  x[i_point][i];
#if DOUBLE_PRECISION
                swap8((char*)&BE_x);
                fwrite(&BE_x, sizeof(double), 1, fout);
#else
                swap4((char*)&BE_x);
                fwrite(&BE_x, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
            }
        }
    }
}

/**
 * @brief Write cell definitions to the given file
 * @param fout              pointer to open dump file
 * @param file_type         dump file type (ascii or binary)
 * @param nodes             array of node indices for each cell
 * @param nodes_per_cell    array of node count for each cell
 * @param n_cells           total number of cells
 * @param point_offset      offset to be added to the given node indices
 */
void dump_vtk_topo_cells(FILE *fout, int file_type, int **nodes, int *nodes_per_cell, int n_cells,
                         int *type, int *number_of_poly_faces, int **number_of_poly_face_nodes, int *poly_nodes,
                         int point_offset)
{
    int i_cell, i_node, i_polyhedron = 0, i_poly_node = 0;

    if(file_type == RCFD_FILE_ASCII){

        for(i_cell = 0; i_cell < n_cells; ++i_cell){

            int n_nodes = nodes_per_cell[i_cell];
            int cell_type = type[i_cell];

            if(cell_type == RCFD_CELL_POLYHEDRON){

                int i_face;
                int n_faces = number_of_poly_faces[i_polyhedron];

                fprintf(fout, "%d %d ", 1 + n_faces + n_nodes, n_faces);

                for(i_face = 0; i_face < n_faces; ++i_face){

                    int n_face_nodes = number_of_poly_face_nodes[i_polyhedron][i_face];
                    fprintf(fout, "%d ", n_face_nodes);

                    for(i_node = 0; i_node < n_face_nodes; ++i_node, ++i_poly_node){

                        fprintf(fout, "%d ", poly_nodes[i_poly_node] + point_offset);
                    }
                }
                ++i_polyhedron;
            }
            else{

                fprintf(fout, "%d ", n_nodes);

                for(i_node = 0; i_node < n_nodes; ++i_node){

                    fprintf(fout, "%d ", nodes[i_cell][i_node] + point_offset);
                }
            }
            fprintf(fout, "\n");
        }
    }
    else{

        for(i_cell = 0; i_cell < n_cells; ++i_cell){

            int n_nodes = nodes_per_cell[i_cell];
            int cell_type = type[i_cell];

            if(cell_type == RCFD_CELL_POLYHEDRON){

                int i_face;
                int n_faces = number_of_poly_faces[i_polyhedron];
                int list_size =  1 + n_faces + n_nodes;
                int list_index = 0;
                int *faces_and_nodes_with_offset = malloc(sizeof(int) * list_size);

                faces_and_nodes_with_offset[list_index] = n_faces;
                swap4((char*)&faces_and_nodes_with_offset[list_index]);
                ++list_index;

                for(i_face = 0; i_face < n_faces; ++i_face){

                    int n_face_nodes = number_of_poly_face_nodes[i_polyhedron][i_face];
                    faces_and_nodes_with_offset[list_index] = n_face_nodes;
                    swap4((char*)&faces_and_nodes_with_offset[list_index]);
                    ++list_index;

                    for(i_node = 0; i_node < n_face_nodes; ++i_node, ++i_poly_node, ++list_index){

                        faces_and_nodes_with_offset[list_index] = poly_nodes[i_poly_node] + point_offset;
                        swap4((char*)&faces_and_nodes_with_offset[list_index]);
                    }
                }

                swap4((char*)&list_size);
                fwrite(&list_size, sizeof(int), 1, fout);
                fwrite(faces_and_nodes_with_offset, sizeof(int), 1 + n_faces + n_nodes, fout);

                free(faces_and_nodes_with_offset);

                ++i_polyhedron;
            }
            else{

                int nodes_with_offset[RCFD_MAX_NODES_PER_CELL];

                for(i_node = 0; i_node < n_nodes; ++i_node){

                    nodes_with_offset[i_node] = nodes[i_cell][i_node] + point_offset;
                    swap4((char*)&nodes_with_offset[i_node]);
                }

                swap4((char*)&n_nodes);
                fwrite(&n_nodes, sizeof(int), 1, fout);
                fwrite(nodes_with_offset, sizeof(int), nodes_per_cell[i_cell], fout);
            }
        }
    }
}

/**
 * @brief Write the topology information for the current VTK dump file
 * @param rcfd  pointer to rCFD instance
 * @param fout  pointer to open dump file
 */
void dump_vtk_topo_multi(rCFD_type *rcfd, FILE *fout)
{
#if RP_NODE
    int i_layer = rcfd->Solver->current_layer;
    int i_cell = 0;
    int n_local_cells = _CELL_DICT(i_layer).number_of_int_cells;
    int n_local_nodes = _NODE_DICT(i_layer).number_of_nodes;
    int n_local_points = 0;
    double **x = NULL;

    if(n_local_nodes > 0){

        n_local_points = n_local_nodes;
        x = _N(i_layer).x;
    }
    else{

        n_local_points = n_local_cells;
        x = _C(i_layer).x;
    }

    // write points
    if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

        fprintf(fout, "POINTS %d float\n", n_local_points);

        dump_vtk_topo_points(fout, rcfd->Dump_Dict->dump_file_type, x, n_local_points, 0);

        fprintf(fout, "\n");
    }
    else{

        char buffer[64];
        snprintf(buffer, 64, "POINTS %d %s\n", n_local_points, DOUBLE_PRECISION?"double":"float");
        fwrite(buffer, sizeof(char), strlen(buffer), fout);

        dump_vtk_topo_points(fout, rcfd->Dump_Dict->dump_file_type, x, n_local_points, 0);

        buffer[0] = '\n';
        fwrite(buffer, sizeof(char), 1, fout);
    }

    // write cell definitions
    if(n_local_nodes > 0){

        int *type = _C(i_layer).type;
        int n_node_ref = _NODE_DICT(i_layer).number_of_int_node_references;

        // write cell node IDs
        if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

            fprintf(fout, "CELLS %d %d\n", n_local_cells, n_local_cells + n_node_ref);

            dump_vtk_topo_cells(fout, rcfd->Dump_Dict->dump_file_type, _C(i_layer).nodes, _C(i_layer).number_of_nodes, n_local_cells,
                                type, _C(i_layer).number_of_poly_faces, _C(i_layer).number_of_poly_face_nodes, _C(i_layer).poly_nodes, 0);

            // write cell types
            fprintf(fout, "CELL_TYPES %d\n", n_local_cells);

            for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                fprintf(fout, "%d\n", type[i_cell]);
            }
        }
        else{ // FT_BINARY

            char buffer[64];
            snprintf(buffer, 64, "\nCELLS %d %d\n", n_local_cells, n_local_cells + n_node_ref);
            fwrite(buffer, sizeof(char), strlen(buffer), fout);

            dump_vtk_topo_cells(fout, rcfd->Dump_Dict->dump_file_type, _C(i_layer).nodes, _C(i_layer).number_of_nodes, n_local_cells,
                                type, _C(i_layer).number_of_poly_faces, _C(i_layer).number_of_poly_face_nodes, _C(i_layer).poly_nodes, 0);

            // write cell types
            snprintf(buffer, 64, "\nCELL_TYPES %d\n", n_local_cells);
            fwrite(buffer, sizeof(char), strlen(buffer), fout);

            for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                int BE_type = type[i_cell];
                swap4((char*)&BE_type);
                fwrite(&BE_type, sizeof(int), 1, fout);
            }
        }
    }
    else{ // no cell node info

        if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

#ifdef UNSTRUCTURED_GRID_VTK
            fprintf(fout, "CELLS %d %d\n", n_local_cells, 2*n_local_cells);
#else
            fprintf(fout, "VERTICES %d %d\n", n_local_cells, 2*n_local_cells);
#endif

            for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                fprintf(fout, "1 %d\n", i_cell); // number of point IDs, point ID
            }

#ifdef UNSTRUCTURED_GRID_VTK
            // write cell types
            fprintf(fout, "CELL_TYPES %d\n", n_local_cells);

            for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                fprintf(fout, "%d\n", RCFD_CELL_VERTEX);
            }
#endif
        }
        else{ // FT_BINARY

            char buffer[64];
            int pointid[2] = {1, 0};

#ifdef UNSTRUCTURED_GRID_VTK
            snprintf(buffer, 64, "CELLS %d %d\n", n_local_cells, 2*n_local_cells);
#else
            snprintf(buffer, 64, "VERTICES %d %d\n", n_local_cells, 2*n_local_cells);
#endif
            fwrite(buffer, sizeof(char), strlen(buffer), fout);

            swap4((char*)&pointid[0]);

            for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                pointid[1] = i_cell;
                swap4((char*)&pointid[1]);
                fwrite(pointid, sizeof(int), 2, fout);
            }

#ifdef UNSTRUCTURED_GRID_VTK
            // write cell types
            snprintf(buffer, 64, "CELL_TYPES %d\n", n_local_cells);
            fwrite(buffer, sizeof(char), strlen(buffer), fout);
            {
                int type = RCFD_CELL_VERTEX;
                swap4((char*)&type);

                for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                    fwrite(&type, sizeof(int), 1, fout);
                }
            }
#endif // UNSTRUCTURED_GRID_VTK
        }
    }
#endif
}

/**
 * @brief Write the topology information for the current VTK dump file
 * @param rcfd  pointer to rCFD instance
 * @param fout  pointer to open dump file on node 0
 */
void dump_vtk_topo0(rCFD_type *rcfd, FILE *fout)
{
#if RP_NODE
    int i_layer = rcfd->Solver->current_layer;
    int i_cell = 0;
    int i_compute_node = 0;
    int n_local_cells = _CELL_DICT(i_layer).number_of_int_cells;
    int n_local_nodes = _NODE_DICT(i_layer).number_of_nodes;
    int n_local_points = 0;
    int n_cells = PRF_GISUM1(n_local_cells);
    int n_nodes = PRF_GISUM1(n_local_nodes);
    int n_points = 0;
    int *n_points_all = NULL;
    double **x = NULL;

    if(n_nodes > 0){

        n_local_points = n_local_nodes;
        n_points = n_nodes;
        x = _N(i_layer).x;
    }
    else{

        n_local_points = n_local_cells;
        n_points = n_cells;
        x = _C(i_layer).x;
    }

    // gather number of points from all compute nodes
    {
        n_points_all = (int*)malloc(sizeof(int) * compute_node_count);

        for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

            n_points_all[i_compute_node] = 0;
        }

        n_points_all[myid] = n_local_points;

        for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

            n_points_all[i_compute_node] = PRF_GIHIGH1(n_points_all[i_compute_node]);
        }
    }

    // write points
    if(myid == 0){

        int n_points_written = 0;

        if(fout){

            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                fprintf(fout, "POINTS %d float\n", n_points);
            }
            else{

                char buffer[64];
                snprintf(buffer, 64, "POINTS %d %s\n", n_points, DOUBLE_PRECISION?"double":"float");
                fwrite(buffer, sizeof(char), strlen(buffer), fout);
            }

            // proc 0
            dump_vtk_topo_points(fout, rcfd->Dump_Dict->dump_file_type, x, n_points_all[0], 0);
        }

        n_points_written = n_points_all[0];

        // proc > 0
        for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

            if(n_points_all[i_compute_node] > 0){

                int n_points_carry = 3 - (n_points_written % 3);

                x = malloc_r_2d(n_points_all[i_compute_node], 3);

                PRF_CRECV_REAL(i_compute_node, x[0], n_points_all[i_compute_node] * 3, i_compute_node);

                if(fout){

                    dump_vtk_topo_points(fout, rcfd->Dump_Dict->dump_file_type, x, n_points_all[i_compute_node], n_points_carry);
                }

                free_r_2d(x);

                n_points_written += n_points_all[i_compute_node];
            }
        }

        if(fout){

            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                fprintf(fout, "\n");
            }
            else{

                char buffer[] = "\n";
                fwrite(buffer, sizeof(char), 1, fout);
            }
        }
    }
    else{

        if(n_local_points > 0){

            PRF_CSEND_REAL(node_zero, x[0], n_local_points * 3, myid);
        }
    }

    // write cell definitions
    if(n_nodes > 0){

        int number_of_polyhedra = _C(i_layer).number_of_polyhedra;
        int *number_of_poly_faces = _C(i_layer).number_of_poly_faces;
        int **number_of_poly_face_nodes = _C(i_layer).number_of_poly_face_nodes;
        int number_of_poly_nodes = _C(i_layer).number_of_poly_nodes;
        int *poly_nodes = _C(i_layer).poly_nodes;
        int *type = _C(i_layer).type;
        int **type_all = NULL;
        int *number_of_nodes = _C(i_layer).number_of_nodes;
        int **nodes = _C(i_layer).nodes;
        int *n_cells_all = NULL;
        int n_node_ref = PRF_GISUM1(_NODE_DICT(i_layer).number_of_int_node_references);

        // gather number of cells from all compute nodes
        {
            n_cells_all = (int*)malloc(sizeof(int) * compute_node_count);

            for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

                n_cells_all[i_compute_node] = 0;
            }

            n_cells_all[myid] = n_local_cells;

            for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

                n_cells_all[i_compute_node] = PRF_GIHIGH1(n_cells_all[i_compute_node]);
            }

            if(myid == 0){

                type_all = (int**)malloc(sizeof(int*) * compute_node_count);
                type_all[0] = type;
            }
        }

        if(myid == 0){

            int point_offset = 0;

            // write cell node IDs
            if(fout){

                if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                    fprintf(fout, "CELLS %d %d\n", n_cells, n_cells + n_node_ref);
                }
                else{

                    char buffer[64];
                    snprintf(buffer, 64, "\nCELLS %d %d\n", n_cells, n_cells + n_node_ref);
                    fwrite(buffer, sizeof(char), strlen(buffer), fout);
                }

                // proc 0
                dump_vtk_topo_cells(fout, rcfd->Dump_Dict->dump_file_type, nodes, number_of_nodes, n_cells_all[0],
                                    type, number_of_poly_faces, number_of_poly_face_nodes, poly_nodes, point_offset);
            }

            // procs > 0
            for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                point_offset += n_points_all[i_compute_node-1];

                if(n_cells_all[i_compute_node] > 0){

                    number_of_nodes = (int*)malloc(n_cells_all[i_compute_node] * sizeof(int));
                    nodes = malloc_i_2d(n_cells_all[i_compute_node], RCFD_MAX_NODES_PER_CELL);
                    type_all[i_compute_node] = (int*)malloc(n_cells_all[i_compute_node] * sizeof(int));

                    PRF_CRECV_INT(i_compute_node, number_of_nodes,  n_cells_all[i_compute_node], i_compute_node*8+0);
                    PRF_CRECV_INT(i_compute_node, nodes[0],         n_cells_all[i_compute_node] * RCFD_MAX_NODES_PER_CELL, i_compute_node*8+1);
                    PRF_CRECV_INT(i_compute_node, type_all[i_compute_node], n_cells_all[i_compute_node], i_compute_node*8+2);

                    number_of_poly_faces = NULL;
                    number_of_poly_face_nodes= NULL;
                    poly_nodes = NULL;
                    PRF_CRECV_INT(i_compute_node, &number_of_polyhedra, 1, i_compute_node*8+3);

                    if(number_of_polyhedra > 0){

                        number_of_poly_faces = malloc(sizeof(int) * number_of_polyhedra);
                        number_of_poly_face_nodes = malloc_i_2d(number_of_polyhedra, RCFD_MAX_FACES_PER_POLY_CELL);

                        PRF_CRECV_INT(i_compute_node, number_of_poly_faces, number_of_polyhedra, i_compute_node*8+4);
                        PRF_CRECV_INT(i_compute_node, number_of_poly_face_nodes[0], number_of_polyhedra * RCFD_MAX_FACES_PER_POLY_CELL, i_compute_node*8+5);
                        PRF_CRECV_INT(i_compute_node, &number_of_poly_nodes, 1, i_compute_node*8+6);
                        poly_nodes = malloc(sizeof(int) * number_of_poly_nodes);
                        PRF_CRECV_INT(i_compute_node, poly_nodes, number_of_poly_nodes, i_compute_node*8+7);
                    }

                    if(fout){

                        dump_vtk_topo_cells(fout, rcfd->Dump_Dict->dump_file_type, nodes, number_of_nodes, n_cells_all[i_compute_node],
                                            type_all[i_compute_node], number_of_poly_faces, number_of_poly_face_nodes, poly_nodes, point_offset);
                    }

                    free(number_of_nodes);
                    free_i_2d(nodes);
                    free(number_of_poly_faces);
                    free_i_2d(number_of_poly_face_nodes);
                    free(poly_nodes);
                }
            }

            // write cell types
            if(fout){

                if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                    fprintf(fout, "CELL_TYPES %d\n", n_cells);
                }
                else{

                    char buffer[64];
                    snprintf(buffer, 64, "\nCELL_TYPES %d\n", n_cells);
                    fwrite(buffer, sizeof(char), strlen(buffer), fout);
                }

                // proc 0
                if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                    for(i_cell = 0; i_cell < n_cells_all[0]; ++i_cell){

                        fprintf(fout, "%d\n", type[i_cell]);
                    }
                }
                else{

                    for(i_cell = 0; i_cell < n_cells_all[0]; ++i_cell){

                        int BE_type = type[i_cell];
                        swap4((char*)&BE_type);
                        fwrite(&BE_type, sizeof(int), 1, fout);
                    }
                }
            }

            // procs > 0
            for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                if(n_cells_all[i_compute_node] > 0){

                    type = type_all[i_compute_node];

                    if(fout){

                        if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                            for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){

                                fprintf(fout, "%d\n", type[i_cell]);
                            }
                        }
                        else{

                            for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){

                                swap4((char*)&type[i_cell]);
                                fwrite(&type[i_cell], sizeof(int), 1, fout);
                            }
                        }
                    }

                    free(type);
                }
            }

            free(type_all);
        }
        else{

            if(n_local_cells > 0){

                PRF_CSEND_INT(node_zero, number_of_nodes,   n_local_cells, myid*8+0);
                PRF_CSEND_INT(node_zero, nodes[0],          n_local_cells * RCFD_MAX_NODES_PER_CELL, myid*8+1);
                PRF_CSEND_INT(node_zero, type,              n_local_cells, myid*8+2);
                PRF_CSEND_INT(node_zero, &number_of_polyhedra, 1, myid*8+3);

                if(number_of_polyhedra > 0){

                    PRF_CSEND_INT(node_zero, number_of_poly_faces, number_of_polyhedra, myid*8+4);
                    PRF_CSEND_INT(node_zero, number_of_poly_face_nodes[0], number_of_polyhedra * RCFD_MAX_FACES_PER_POLY_CELL, myid*8+5);
                    PRF_CSEND_INT(node_zero, &number_of_poly_nodes, 1, myid*8+6);
                    PRF_CSEND_INT(node_zero, poly_nodes, number_of_poly_nodes, myid*8+7);
                }
            }
        }

        free(n_cells_all);
    }
    else{ // no cell node info

        if(myid == 0 && fout){

#ifdef UNSTRUCTURED_GRID_VTK
            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                fprintf(fout, "CELLS %d %d\n", n_cells, 2*n_cells);
            }
            else{

                char buffer[64];
                snprintf(buffer, 64, "CELLS %d %d\n", n_cells, 2*n_cells);
                fwrite(buffer, sizeof(char), strlen(buffer), fout);
            }
#else
            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                fprintf(fout, "VERTICES %d %d\n", n_cells, 2*n_cells);
            }
            else{

                char buffer[64];
                snprintf(buffer, 64, "VERTICES %d %d\n", n_cells, 2*n_cells);
                fwrite(buffer, sizeof(char), strlen(buffer), fout);
            }
#endif

            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                for(i_cell = 0; i_cell < n_cells; ++i_cell){

                    fprintf(fout, "1 %d\n", i_cell); // number of point IDs, point ID
                }
            }
            else{

                int pointid[2] = {1, 0};
                swap4((char*)&pointid[0]);

                for(i_cell = 0; i_cell < n_cells; ++i_cell){

                    pointid[1] = i_cell;
                    swap4((char*)&pointid[1]);
                    fwrite(pointid, sizeof(int), 2, fout);
                }
            }

#ifdef UNSTRUCTURED_GRID_VTK
            // write cell types
            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                fprintf(fout, "CELL_TYPES %d\n", n_cells);

                for(i_cell = 0; i_cell < n_cells; ++i_cell){

                    fprintf(fout, "%d\n", RCFD_CELL_VERTEX);
                }
            }
            else{

                char buffer[64];
                snprintf(buffer, 64, "CELL_TYPES %d\n", n_cells);
                fwrite(buffer, sizeof(char), strlen(buffer), fout);
                {
                    int type = RCFD_CELL_VERTEX;
                    swap4((char*)&type);

                    for(i_cell = 0; i_cell < n_cells; ++i_cell){

                        fwrite(&type, sizeof(int), 1, fout);
                    }
                }
            }
#endif
        }
    }

    free(n_points_all);
#endif
}


/**
 * @brief Write the cell data for the current VTK dump file
 * @param rcfd  pointer to rCFD instance
 * @param fout  pointer to open dump file
 */
void dump_vtk_data_multi(rCFD_type *rcfd, FILE *fout)
{
#if RP_NODE
    int i_layer = rcfd->Solver->current_layer;
    int i_frame = rcfd->Solver->current_frame;
    int i_cell  = 0;
    int i_phase = 0;
    int phase_count = rcfd->Solver_Dict->number_of_phases;
    int i_data  = 0;
    int n_local_cells = _CELL_DICT(i_layer).number_of_int_cells;
    int n_local_nodes = _NODE_DICT(i_layer).number_of_nodes;

    if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

        if(n_local_nodes > 0){

            fprintf(fout, "CELL_DATA %d\n", n_local_cells);
        }
        else{ // no cell type info

            fprintf(fout, "POINT_DATA %d\n", n_local_cells);
        }

        // write cell IDs
        fprintf(fout, "SCALARS cell_id int 1\n");
        fprintf(fout, "LOOKUP_TABLE default\n");

        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

            fprintf(fout, "%d\n", i_cell);
        }
    }
    else{

        char buffer[64];

        if(n_local_nodes > 0){

            snprintf(buffer, 64, "CELL_DATA %d\n", n_local_cells);
        }
        else{ // no cell type info

            snprintf(buffer, 64, "POINT_DATA %d\n", n_local_cells);
        }

        fwrite(buffer, sizeof(char), strlen(buffer), fout);

        // write cell IDs
        snprintf(buffer, 64, "SCALARS cell_id int 1\n"
                             "LOOKUP_TABLE default\n");
        fwrite(buffer, sizeof(char), strlen(buffer), fout);

        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

            int cellid = i_cell;
            swap4((char*)&cellid);
            fwrite(&cellid, sizeof(int), 1, fout);
        }
    }

    // write vof data
    if(rcfd->Dump_Dict->dump_vof && _C(i_layer).vof){

        for(i_phase = 0; i_phase < phase_count; ++i_phase){

            double **vof_data   = _C(i_layer).vof[i_frame];

            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                fprintf(fout, "SCALARS vof_phase%d double 1\n", i_phase); // TODO: custom data label
                fprintf(fout, "LOOKUP_TABLE default\n");

                for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                    fprintf(fout, "%f\n", vof_data[i_cell][i_phase]);
                }
            }
            else{

                char buffer[64];
                snprintf(buffer, 64, "SCALARS vof_phase%d %s 1\n"
                                     "LOOKUP_TABLE default\n", i_phase, DOUBLE_PRECISION?"double":"float");
                fwrite(buffer, sizeof(char), strlen(buffer), fout);

                for(i_cell = 0; i_cell < n_local_cells; ++i_cell){
#if DOUBLE_PRECISION
                    double vof = vof_data[i_cell][i_phase];
                    swap8((char*)&vof);
                    fwrite(&vof, sizeof(double), 1, fout);
#else
                    float vof = vof_data[i_cell][i_phase];
                    swap4((char*)&vof);
                    fwrite(&vof, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                }
            }
        }
    }

    // write cell data for each phase
    for(i_phase = 0; i_phase < phase_count; ++i_phase){

        int phase_data_count= rcfd->Phase_Dict[i_phase].number_of_data;

        if(phase_data_count > 0 && _C(i_layer).data){

            double **cell_data  = _C(i_layer).data[i_phase];

            for(i_data = 0; i_data < phase_data_count; ++i_data){

                if(rcfd->Dump_Dict->dump_cell_data[i_phase][i_data]){

                    if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                        fprintf(fout, "SCALARS phase%d_data%d double 1\n", i_phase, i_data); // TODO: custom data label
                        fprintf(fout, "LOOKUP_TABLE default\n");

                        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                            fprintf(fout, "%f\n", cell_data[i_cell][i_data]);
                        }
                    }
                    else{

                        char buffer[80];
                        snprintf(buffer, 80, "SCALARS phase%d_data%d %s 1\n"
                                             "LOOKUP_TABLE default\n", i_phase, i_data, DOUBLE_PRECISION?"double":"float");
                        fwrite(buffer, sizeof(char), strlen(buffer), fout);

                        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){
#if DOUBLE_PRECISION
                            double cdata = cell_data[i_cell][i_data];
                            swap8((char*)&cdata);
                            fwrite(&cdata, sizeof(double), 1, fout);
#else
                            float cdata = cell_data[i_cell][i_data];
                            swap4((char*)&cdata);
                            fwrite(&cdata, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                        }
                    }
                }
            }
        }
    }

    // write user data
    {
        int user_data_count = _CELL_DICT(i_layer).number_of_user_vars;

        if(user_data_count > 0 && _C(i_layer).user){

            double **cell_user_data = _C(i_layer).user;

            for(i_data = 0; i_data < user_data_count; ++i_data){

                if(rcfd->Dump_Dict->dump_user_data[i_data]){

                    if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                        fprintf(fout, "SCALARS user_data%d double 1\n", i_data); // TODO: custom data label
                        fprintf(fout, "LOOKUP_TABLE default\n");

                        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                            fprintf(fout, "%f\n", cell_user_data[i_cell][i_data]);
                        }
                    }
                    else{

                        char buffer[80];
                        snprintf(buffer, 80, "SCALARS user_data%d %s 1\n"
                                             "LOOKUP_TABLE default\n", i_data, DOUBLE_PRECISION?"double":"float");
                        fwrite(buffer, sizeof(char), strlen(buffer), fout);

                        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){
#if DOUBLE_PRECISION
                            double udata = cell_user_data[i_cell][i_data];
                            swap8((char*)&udata);
                            fwrite(&udata, sizeof(double), 1, fout);
#else
                            float udata = cell_user_data[i_cell][i_data];
                            swap4((char*)&udata);
                            fwrite(&udata, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                        }
                    }
                }
            }
        }
    }

    // write rec user data
    {
        int rec_user_data_count = _CELL_DICT(i_layer).number_of_rec_user_vars;

        if(rec_user_data_count > 0 && _C(i_layer).rec_user){

            int i_frame = rcfd->Solver->current_frame;
            double **cell_rec_user_data = _C(i_layer).rec_user[i_frame];

            for(i_data = 0; i_data < rec_user_data_count; ++i_data){

                if(rcfd->Dump_Dict->dump_rec_user_data[i_data]){

                    if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                        fprintf(fout, "SCALARS rec_user_data%d double 1\n", i_data); // TODO: custom data label
                        fprintf(fout, "LOOKUP_TABLE default\n");

                        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){

                            fprintf(fout, "%f\n", cell_rec_user_data[i_cell][i_data]);
                        }
                    }
                    else{

                        char buffer[80];
                        snprintf(buffer, 80, "SCALARS rec_user_data%d %s 1\n"
                                             "LOOKUP_TABLE default\n", i_data, DOUBLE_PRECISION?"double":"float");
                        fwrite(buffer, sizeof(char), strlen(buffer), fout);

                        for(i_cell = 0; i_cell < n_local_cells; ++i_cell){
#if DOUBLE_PRECISION
                            double rudata = cell_rec_user_data[i_cell][i_data];
                            swap8((char*)&rudata);
                            fwrite(&rudata, sizeof(double), 1, fout);
#else
                            float rudata = cell_rec_user_data[i_cell][i_data];
                            swap4((char*)&rudata);
                            fwrite(&rudata, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                        }
                    }
                }
            }
        }
    }
#endif
}

/**
 * @brief Write the cell data for the current VTK dump file
 * @param rcfd  pointer to rCFD instance
 * @param fout  pointer to open dump file on node 0
 */
void dump_vtk_data0(rCFD_type *rcfd, FILE *fout)
{
#if RP_NODE
    int i_layer = rcfd->Solver->current_layer;
    int i_frame = rcfd->Solver->current_frame;
    int i_cell  = 0;
    int i_phase = 0;
    int phase_count = rcfd->Solver_Dict->number_of_phases;
    int i_data  = 0;
    int n_local_cells = _CELL_DICT(i_layer).number_of_int_cells;
    int n_local_nodes = _NODE_DICT(i_layer).number_of_nodes;
    int n_cells = PRF_GISUM1(n_local_cells);
    int n_nodes = PRF_GISUM1(n_local_nodes);
    int *n_cells_all = NULL;
    int i_compute_node;
    double **vof_data = _C(i_layer).vof ? _C(i_layer).vof[i_frame] : NULL;
    double ***vof_data_all = NULL;

    n_cells_all = (int*)malloc(sizeof(int) * compute_node_count);

    for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

        n_cells_all[i_compute_node] = 0;
    }

    n_cells_all[myid] = n_local_cells;

    for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

        n_cells_all[i_compute_node] = PRF_GIHIGH1(n_cells_all[i_compute_node]);
    }

    if(myid == 0 && fout){

        if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

            if(n_nodes > 0){

                fprintf(fout, "CELL_DATA %d\n", n_cells);
            }
            else{ // no cell type info

                fprintf(fout, "POINT_DATA %d\n", n_cells);
            }

            // write cell IDs
            fprintf(fout, "SCALARS cell_id int 1\n");
            fprintf(fout, "LOOKUP_TABLE default\n");

            for(i_cell = 0; i_cell < n_cells; ++i_cell){

                fprintf(fout, "%d\n", i_cell);
            }
        }
        else{

            char buffer[64];

            if(n_nodes > 0){

                snprintf(buffer, 64, "CELL_DATA %d\n", n_cells);
            }
            else{ // no cell type info

                snprintf(buffer, 64, "POINT_DATA %d\n", n_cells);
            }

            fwrite(buffer, sizeof(char), strlen(buffer), fout);

            // write cell IDs
            snprintf(buffer, 64, "SCALARS cell_id int 1\n"
                                 "LOOKUP_TABLE default\n");
            fwrite(buffer, sizeof(char), strlen(buffer), fout);

            for(i_cell = 0; i_cell < n_cells; ++i_cell){

                int cellid = i_cell;
                swap4((char*)&cellid);
                fwrite(&cellid, sizeof(int), 1, fout);
            }
        }
    }

    if(rcfd->Dump_Dict->dump_vof && vof_data){

        // gather vof data
        if(myid == 0){

            vof_data_all = (double***)malloc(compute_node_count * sizeof(double***));
            vof_data_all[0] = vof_data;

            for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                if(n_cells_all[i_compute_node] > 0){

                    vof_data_all[i_compute_node] = malloc_r_2d(n_cells_all[i_compute_node], phase_count);
                    PRF_CRECV_REAL(i_compute_node, vof_data_all[i_compute_node][0], n_cells_all[i_compute_node] * phase_count, i_compute_node);
                }
                else{

                    vof_data_all[i_compute_node] = NULL;
                }
            }
        }
        else{

            if(n_cells_all[myid] > 0){

                PRF_CSEND_REAL(node_zero, vof_data[0], n_cells_all[myid] * phase_count, myid);
            }
        }

        // write vof data
        if(myid == 0 && fout){

            for(i_phase = 0; i_phase < phase_count; ++i_phase){

                if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                    fprintf(fout, "SCALARS vof_phase%d double 1\n", i_phase); // TODO: custom data label
                    fprintf(fout, "LOOKUP_TABLE default\n");
                }
                else{

                    char buffer[64];
                    snprintf(buffer, 64, "SCALARS vof_phase%d %s 1\n"
                                         "LOOKUP_TABLE default\n", i_phase, DOUBLE_PRECISION?"double":"float");
                    fwrite(buffer, sizeof(char), strlen(buffer), fout);
                }

                for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

                    if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                        for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){

                            fprintf(fout, "%f\n", vof_data_all[i_compute_node][i_cell][i_phase]);
                        }
                    }
                    else{

                        for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){
#if DOUBLE_PRECISION
                            double vof = vof_data_all[i_compute_node][i_cell][i_phase];
                            swap8((char*)&vof);
                            fwrite(&vof, sizeof(double), 1, fout);
#else
                            float vof = vof_data_all[i_compute_node][i_cell][i_phase];
                            swap4((char*)&vof);
                            fwrite(&vof, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                        }
                    }
                }
            }
        }

        // free vof data
        if(myid == 0){

            for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                free_r_2d(vof_data_all[i_compute_node]);
            }
            free(vof_data_all);
        }
    }

    // write cell data of each phase
    for(i_phase = 0; i_phase < phase_count; ++i_phase){

        // write cell data of current phase
        int phase_data_count = rcfd->Phase_Dict[i_phase].number_of_data;

        if(phase_data_count > 0 && _C(i_layer).data){

            double **cell_data = _C(i_layer).data[i_phase];

            if(myid == 0){

                // gather cell data
                double ***cell_data_all = (double***)malloc(compute_node_count * sizeof(double***));
                cell_data_all[0] = cell_data;

                for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                    if(n_cells_all[i_compute_node] > 0){

                        cell_data_all[i_compute_node] = malloc_r_2d(n_cells_all[i_compute_node], phase_data_count);
                        PRF_CRECV_REAL(i_compute_node, cell_data_all[i_compute_node][0], n_cells_all[i_compute_node] * phase_data_count, i_compute_node);
                    }
                    else{

                        cell_data_all[i_compute_node] = NULL;
                    }
                }

                // write cell data
                if(fout){

                    for(i_data = 0; i_data < phase_data_count; ++i_data){

                        if(rcfd->Dump_Dict->dump_cell_data[i_phase][i_data]){

                            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                                fprintf(fout, "SCALARS phase%d_data%d double 1\n", i_phase, i_data); // TODO: custom data label
                                fprintf(fout, "LOOKUP_TABLE default\n");
                            }
                            else{

                                char buffer[80];
                                snprintf(buffer, 80, "SCALARS phase%d_data%d %s 1\n"
                                                     "LOOKUP_TABLE default\n", i_phase, i_data, DOUBLE_PRECISION?"double":"float");
                                fwrite(buffer, sizeof(char), strlen(buffer), fout);
                            }

                            for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

                                if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                                    for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){

                                        fprintf(fout, "%f\n", cell_data_all[i_compute_node][i_cell][i_data]);
                                    }
                                }
                                else{

                                    for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){
#if DOUBLE_PRECISION
                                        double cdata = cell_data_all[i_compute_node][i_cell][i_data];
                                        swap8((char*)&cdata);
                                        fwrite(&cdata, sizeof(double), 1, fout);
#else
                                        float cdata = cell_data_all[i_compute_node][i_cell][i_data];
                                        swap4((char*)&cdata);
                                        fwrite(&cdata, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                                    }
                                }
                            }
                        }
                    }
                }

                // free cell data
                for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                    free_r_2d(cell_data_all[i_compute_node]);
                }
                free(cell_data_all);
            }
            else{

                if(n_cells_all[myid] > 0){

                    PRF_CSEND_REAL(node_zero, cell_data[0], n_cells_all[myid] * phase_data_count, myid);
                }
            }
        }
    }

    // write user data
    {
        int user_data_count = _CELL_DICT(i_layer).number_of_user_vars;

        if(user_data_count > 0 && _C(i_layer).user){

            double **cell_user_data = _C(i_layer).user;
            double ***cell_user_data_all = NULL;

            if(myid == 0){

                // gather user data
                cell_user_data_all = (double***)malloc(compute_node_count * sizeof(double***));
                cell_user_data_all[0] = cell_user_data;

                for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                    if(n_cells_all[i_compute_node] > 0){

                        cell_user_data_all[i_compute_node] = malloc_r_2d(n_cells_all[i_compute_node], user_data_count);
                        PRF_CRECV_REAL(i_compute_node, cell_user_data_all[i_compute_node][0], n_cells_all[i_compute_node] * user_data_count, i_compute_node);
                    }
                    else{

                        cell_user_data_all[i_compute_node] = NULL;
                    }
                }

                // write user data
                if(fout){

                    for(i_data = 0; i_data < user_data_count; ++i_data){

                        if(rcfd->Dump_Dict->dump_user_data[i_data]){

                            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                                fprintf(fout, "SCALARS user_data%d double 1\n", i_data); // TODO: custom data label
                                fprintf(fout, "LOOKUP_TABLE default\n");
                            }
                            else{

                                char buffer[80];
                                snprintf(buffer, 80, "SCALARS user_data%d %s 1\n"
                                                     "LOOKUP_TABLE default\n", i_data, DOUBLE_PRECISION?"double":"float");
                                fwrite(buffer, sizeof(char), strlen(buffer), fout);
                            }

                            for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

                                if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                                    for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){

                                        fprintf(fout, "%f\n", cell_user_data[i_cell][i_data]);
                                    }
                                }
                                else{

                                    for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){
#if DOUBLE_PRECISION
                                        double udata = cell_user_data[i_cell][i_data];
                                        swap8((char*)&udata);
                                        fwrite(&udata, sizeof(double), 1, fout);
#else
                                        float udata = cell_user_data[i_cell][i_data];
                                        swap4((char*)&udata);
                                        fwrite(&udata, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                                    }
                                }
                            }
                        }
                    }
                }

                // free user data
                for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                    free_r_2d(cell_user_data_all[i_compute_node]);
                }
                free(cell_user_data_all);
            }
            else{

                if(n_cells_all[myid] > 0){

                    PRF_CSEND_REAL(node_zero, cell_user_data[0], n_cells_all[myid] * user_data_count, myid);
                }
            }
        }
    }

    // write rec user data
    {
        int rec_user_data_count = _CELL_DICT(i_layer).number_of_rec_user_vars;

        if(rec_user_data_count > 0 && _C(i_layer).rec_user){

            int i_frame = rcfd->Solver->current_frame;
            double **cell_rec_user_data = _C(i_layer).rec_user[i_frame];
            double ***cell_rec_user_data_all = NULL;

            if(myid == 0){

                // gather user data
                cell_rec_user_data_all = (double***)malloc(compute_node_count * sizeof(double***));
                cell_rec_user_data_all[0] = cell_rec_user_data;

                for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                    if(n_cells_all[i_compute_node] > 0){

                        cell_rec_user_data_all[i_compute_node] = malloc_r_2d(n_cells_all[i_compute_node], rec_user_data_count);
                        PRF_CRECV_REAL(i_compute_node, cell_rec_user_data_all[i_compute_node][0], n_cells_all[i_compute_node] * rec_user_data_count, i_compute_node);
                    }
                    else{

                        cell_rec_user_data_all[i_compute_node] = NULL;
                    }
                }

                // write user data
                if(fout){

                    for(i_data = 0; i_data < rec_user_data_count; ++i_data){

                        if(rcfd->Dump_Dict->dump_rec_user_data[i_data]){

                            if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                                fprintf(fout, "SCALARS rec_user_data%d double 1\n", i_data); // TODO: custom data label
                                fprintf(fout, "LOOKUP_TABLE default\n");
                            }
                            else{

                                char buffer[80];
                                snprintf(buffer, 80, "SCALARS rec_user_data%d %s 1\n"
                                                     "LOOKUP_TABLE default\n", i_data, DOUBLE_PRECISION?"double":"float");
                                fwrite(buffer, sizeof(char), strlen(buffer), fout);
                            }

                            for(i_compute_node = 0; i_compute_node < compute_node_count; ++i_compute_node){

                                if(rcfd->Dump_Dict->dump_file_type == RCFD_FILE_ASCII){

                                    for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){

                                        fprintf(fout, "%f\n", cell_rec_user_data[i_cell][i_data]);
                                    }
                                }
                                else{

                                    for(i_cell = 0; i_cell < n_cells_all[i_compute_node]; ++i_cell){
#if DOUBLE_PRECISION
                                        double rudata = cell_rec_user_data[i_cell][i_data];
                                        swap8((char*)&rudata);
                                        fwrite(&rudata, sizeof(double), 1, fout);
#else
                                        float rudata = cell_rec_user_data[i_cell][i_data];
                                        swap4((char*)&rudata);
                                        fwrite(&rudata, sizeof(float), 1, fout);
#endif // DOUBLE_PRECISION
                                    }
                                }
                            }
                        }
                    }
                }

                // free user data
                for(i_compute_node = 1; i_compute_node < compute_node_count; ++i_compute_node){

                    free_r_2d(cell_rec_user_data_all[i_compute_node]);
                }
                free(cell_rec_user_data_all);
            }
            else{

                if(n_cells_all[myid] > 0){

                    PRF_CSEND_REAL(node_zero, cell_rec_user_data[0], n_cells_all[myid] * rec_user_data_count, myid);
                }
            }
        }
    }
#endif
}

/**
 * @brief rename temporary file to vtk file by replacing the "tmp" file extension by "vtk"
 * @param tmp_filename  file to be renamed
 * @return zero if file was renamed successfully, non-zero on failure
 */
int dump_rename_tmp_to_vtk(const char *tmp_filename)
{
    int success = 0;

#if RP_NODE
    char *final_filename = NULL;
    size_t filename_length = strlen(tmp_filename) + 1;

    final_filename = (char*)malloc(filename_length * sizeof(char));
    strcpy(final_filename, tmp_filename);

    final_filename[filename_length-4] = 'v';
    final_filename[filename_length-3] = 't';
    final_filename[filename_length-2] = 'k';

    // move current_filename to final_filename
    success = rename(tmp_filename, final_filename);

    free(final_filename);
#endif

    return success;
}

/**
 * @brief Write the current data to a (legacy) VTK dump file
 * @param rcfd  pointer to rCFD instance
 */
void dump_vtk(rCFD_type *rcfd)
{
#if RP_NODE
    int ntimestep = rcfd->Solver->global_run_counter;

    if(rcfd->Dump_Dict->dump_interval > 0 && (ntimestep % rcfd->Dump_Dict->dump_interval == 0)){

        char *current_filename = NULL;
        size_t filename_length = 0;
        FILE *fout = NULL;

        if(rcfd->Dump_Dict->multifile_output_on){

            if(rcfd->Dump_Dict->overwrite_interval > 0){

                if(rcfd->Dump_Dict->overwrite_interval > 1){

                    filename_length = strlen(rcfd->File_Dict->Dump_filename) + 1 + 4 + 1 + 6 + 4 + 1;
                    current_filename = (char*)malloc(filename_length * sizeof(char));
                    sprintf(current_filename, "%s_%04d_%06d.tmp", rcfd->File_Dict->Dump_filename, myid, ntimestep % rcfd->Dump_Dict->overwrite_interval);
                }
                else{

                    filename_length = strlen(rcfd->File_Dict->Dump_filename) + 1 + 4 + 4 + 1;
                    current_filename = (char*)malloc(filename_length * sizeof(char));
                    sprintf(current_filename, "%s_%04d.tmp", rcfd->File_Dict->Dump_filename, myid);
                }
            }
            else{

                filename_length = strlen(rcfd->File_Dict->Dump_filename) + 1 + 4 + 1 + 6 + 4 + 1;
                current_filename = (char*)malloc(filename_length * sizeof(char));
                sprintf(current_filename, "%s_%04d_%06d.vtk", rcfd->File_Dict->Dump_filename, myid, ntimestep);
            }

            fout = fopen(current_filename, (rcfd->Dump_Dict->dump_file_type)?"wb":"wt");
            if(fout){

                dump_vtk_header_multi(rcfd, fout);
                dump_vtk_topo_multi(rcfd, fout);
                dump_vtk_data_multi(rcfd, fout);
                fclose(fout);

                if(rcfd->Dump_Dict->overwrite_interval > 0){

                    if(dump_rename_tmp_to_vtk(current_filename) != 0){

                        report_error(rcfd, RCFD_REPORT_CATEGORY_POSTPROCESS, "failed to rename temporary file %s", current_filename);
                    }
                }
            }
        }
        else{

            if(myid == 0){

                if(rcfd->Dump_Dict->overwrite_interval > 0){

                    if(rcfd->Dump_Dict->overwrite_interval > 1){

                        filename_length = strlen(rcfd->File_Dict->Dump_filename) + 1 + 6 + 4 + 1;
                        current_filename = (char*)malloc(filename_length * sizeof(char));
                        sprintf(current_filename, "%s_%06d.tmp", rcfd->File_Dict->Dump_filename, ntimestep % rcfd->Dump_Dict->overwrite_interval);
                    }
                    else{

                        filename_length = strlen(rcfd->File_Dict->Dump_filename) + 4 + 1;
                        current_filename = (char*)malloc(filename_length * sizeof(char));
                        sprintf(current_filename, "%s.vtk", rcfd->File_Dict->Dump_filename);
                    }
                }
                else{

                    filename_length = strlen(rcfd->File_Dict->Dump_filename) + 1 + 6 + 4 + 1;
                    current_filename = (char*)malloc(filename_length * sizeof(char));
                    sprintf(current_filename, "%s_%06d.vtk", rcfd->File_Dict->Dump_filename, ntimestep);
                }

                fout = fopen(current_filename, (rcfd->Dump_Dict->dump_file_type)?"wb":"wt");
            }

            dump_vtk_header0(rcfd, fout);
            dump_vtk_topo0(rcfd, fout);
            dump_vtk_data0(rcfd, fout);

            if(fout){

                fclose(fout);

                if(rcfd->Dump_Dict->overwrite_interval > 0){

                    if(dump_rename_tmp_to_vtk(current_filename) != 0){

                        report_error(rcfd, RCFD_REPORT_CATEGORY_POSTPROCESS, "failed to rename temporary file %s", current_filename);
                    }
                }
            }
        }

        free(current_filename);
    }
#endif
}

/**
 * @brief get (process-local) probe value
 * @param rcfd      pointer to rCFD instance
 * @param i_probe   the probe to query
 * @return probe value if probe is on process else zero
 */
double dump_get_probe_value_local(rCFD_type *rcfd, int i_probe)
{
#if RP_NODE
    int i_cell = rcfd->Dump_Dict->probe_cell[i_probe];

    if(i_cell < 0){

        return 0.0;
    }
    else{

        int i_frame = rcfd->Solver->current_frame;
        int i_layer = rcfd->Dump_Dict->probe_layer[i_probe];
        int i_data = rcfd->Dump_Dict->probe_data[i_probe];
        int i_phase = rcfd->Dump_Dict->probe_phase[i_probe];

        switch(rcfd->Dump_Dict->probe_type[i_probe]){

        case RCFD_PROBE_CELL:
            return _C(i_layer).data[i_phase][i_cell][i_data];
        case RCFD_PROBE_VOF:
            return _C(i_layer).vof[i_frame][i_cell][i_phase];
        case RCFD_PROBE_USER:
            return _C(i_layer).user[i_cell][i_data];
        case RCFD_PROBE_REC_USER:
            return _C(i_layer).rec_user[i_frame][i_cell][i_data];
        default:
            return 0.0;
        }
    }
#else
    return 0.0;
#endif
}

/**
 * @brief get probe value
 * @param rcfd      pointer to rCFD instance
 * @param i_probe   the probe to query
 * @return probe value
 */
double dump_get_probe_value(rCFD_type *rcfd, int i_probe)
{
#if RP_NODE
    double value = dump_get_probe_value_local(rcfd, i_probe);
    return PRF_GRSUM1(value);
#else
    return 0.0;
#endif
}

/**
 * @brief Write current probe values to a csv file
 * @param rcfd  pointer to rCFD instance
 */
void dump_probes(rCFD_type *rcfd)
{
#if RP_NODE
    if(rcfd->Dump_Dict->probe_count > 0){

        int ntimestep = rcfd->Solver->global_run_counter;

        if(rcfd->Dump_Dict->probe_interval > 0 && (ntimestep % rcfd->Dump_Dict->probe_interval == 0)){

            int i_probe;
            FILE *fout = NULL;

            if(myid == 0){

                fout = fopen(rcfd->File_Dict->Probe_filename, "at");
            }

            {
                double *work = (double*)malloc(sizeof(double) * rcfd->Dump_Dict->probe_count);

                for(i_probe = 0; i_probe < rcfd->Dump_Dict->probe_count; ++i_probe){

                    rcfd->Dump_Dict->probe_value[i_probe] = dump_get_probe_value_local(rcfd, i_probe);
                }

                PRF_GRSUM(rcfd->Dump_Dict->probe_value, rcfd->Dump_Dict->probe_count, work);
                free(work);
            }

            // writes
            // t probe1 probe2 ...
            if(fout){

                fprintf(fout, "%f ", rcfd->Solver->global_time);

                for(i_probe = 0; i_probe < rcfd->Dump_Dict->probe_count; ++i_probe){

                    fprintf(fout, "%f ", rcfd->Dump_Dict->probe_value[i_probe]);
                }

                fprintf(fout, "\n");

                fclose(fout);
            }
        }
    }
#endif // RP_NODE
}

/**
 * @brief Write the current data to a dump file or stream
 * @param rcfd  pointer to rCFD instance
 */
void dump_data(rCFD_type *rcfd)
{
    {
        dump_probes(rcfd);
    }

    if(1){ // TODO: add other dump options
        dump_vtk(rcfd);
    }
}



/*****************************************************************************/

#if defined(RCFD_LIB_TEST_MODULES)

/**
 * @brief test dumping current topology and data to VTK file in single-file
 *          and multi-file mode
 * @param rcfd  pointer to rCFD instance
 * @return 0 on success, else non-zero error code
 */
int test_dump(rCFD_type* rcfd)
{
    int success = 0;
#if RP_NODE
    // store original configuration
    int orig_dump_interval = rcfd->Dump_Dict->dump_interval;
    int orig_dump_file_type = rcfd->Dump_Dict->dump_file_type;
    int orig_multifile_output_on = rcfd->Dump_Dict->multifile_output_on;

    PRF_GSYNC();

    Message0("\nStarting rCFD test 'dump'\n");

    rcfd->Dump_Dict->dump_interval = 1;
    rcfd->Dump_Dict->dump_file_type = RCFD_FILE_ASCII;

    rcfd->Dump_Dict->multifile_output_on = 0;
    dump_vtk(rcfd);

    rcfd->Dump_Dict->multifile_output_on = 1;
    dump_vtk(rcfd);

    // reset to original configuration
    rcfd->Dump_Dict->dump_interval = orig_dump_interval;
    rcfd->Dump_Dict->dump_file_type = orig_dump_file_type;
    rcfd->Dump_Dict->multifile_output_on = orig_multifile_output_on;

#endif // RP_NODE

    return success;
}
#endif // RCFD_LIB_TEST_MODULES

#endif // RCFD_DUMP_H
