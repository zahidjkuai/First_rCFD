/******************************************************************************
 * @file        main.c
 * @brief       Test application for transport-based rCFD lib.
 *
 * Run tests for a list of rCFD library modules.
 *
 * @copyright   (C) 2024
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "rCFD_library.h"

/*
 * Hexagonal prism cell
 *
 *     y 7
 *      /
 *     +--------> z
 *     |    ____
 *     |   / /  \
 *     |   \_\__/
 *   x V
 */
#define MY_POLY_SIZE                 0.01
#define MY_POLY_GRID_HORIZ_SPACING   (sqrt(3.0) * MY_POLY_SIZE)
#define MY_POLY_GRID_VERT_SPACING    (1.5 * MY_POLY_SIZE)
#define MY_POLY_CELL_HEIGHT          0.01
#define MY_POLY_FACE_AREA            (MY_POLY_GRID_HORIZ_SPACING * MY_POLY_GRID_VERT_SPACING)
#define MY_POLY_CELL_VOLUME          (MY_POLY_FACE_AREA * MY_POLY_CELL_HEIGHT)
#define MY_POLY_INT_CELL_COUNT       2
#define MY_POLY_INT_FACE_COUNT       1
#define MY_HEX_NODE_COUNT            6
#define MY_POLY_NODE_COUNT           (MY_HEX_NODE_COUNT * (MY_POLY_INT_CELL_COUNT + 1))

void pointy_hexagon_corner(const double *center, double size, int i, double *corner)
{
    double angle_deg = 60.0 * i - 30.0;
    double angle_rad = angle_deg * M_PI / 180.0 ;
    corner[0] = center[0] + size * cos(angle_rad);
    corner[1] = center[1] + size * sin(angle_rad);
}
void test_poly_topo_set_file_dict(void *handle)
{
    rcfd_set_dict_property_s(handle, "File_Dict", -1, -1, -1, "working_dir", "./results/modules");
}
void test_poly_topo_set_cell_dict(void *handle, const short i_layer)
{
    if(i_layer == 0){
        rcfd_set_dict_property_i(handle, "Cell_Dict", i_layer, -1, -1, "number_of_int_cells", MY_POLY_INT_CELL_COUNT);
        rcfd_set_dict_property_i(handle, "Cell_Dict", i_layer, -1, -1, "number_of_ext_cells", 0);
    }
}
void test_poly_topo_set_face_dict(void *handle, const short i_layer)
{
    if(i_layer == 0){
        rcfd_set_dict_property_i(handle, "Face_Dict", i_layer, -1, -1, "number_of_int_faces", MY_POLY_INT_FACE_COUNT);
        rcfd_set_dict_property_i(handle, "Face_Dict", i_layer, -1, -1, "number_of_ext_faces", 0);
    }
}
void test_poly_topo_set_node_dict(void *handle, const short i_layer)
{
    if(i_layer == 0){
        rcfd_set_dict_property_i(handle, "Node_Dict", i_layer, -1, -1, "number_of_nodes", MY_POLY_NODE_COUNT);
    }
}
void test_poly_topo_init_cells(void *handle, const short i_layer)
{
    if(i_layer == 0){
        int     myid;
        int     i_cell;
        double  **cell_x    = rcfd_get_cell_center(handle, i_layer);
        double  *cell_volume= rcfd_get_cell_volume(handle, i_layer);

        MPI_Comm_rank(MPI_COMM_WORLD, &myid);

        for(i_cell = 0; i_cell < MY_POLY_INT_CELL_COUNT; ++i_cell){
            cell_x[i_cell][0]   = 0.0;
            cell_x[i_cell][1]   = 0.0;
            cell_x[i_cell][2]   = MY_POLY_CELL_HEIGHT*(i_cell + MY_POLY_INT_CELL_COUNT*myid);
            cell_volume[i_cell] = MY_POLY_CELL_VOLUME;
        }
    }
}
void test_poly_topo_init_faces(void *handle, const short i_layer)
{
    if(i_layer == 0){
        int     i_face;
        int     *face_c0    = rcfd_get_face_c0  (handle, i_layer); // [i_face]
        int     *face_c1    = rcfd_get_face_c1  (handle, i_layer); // [i_face]
        double  **face_area = rcfd_get_face_area(handle, i_layer); // [i_face][i_dim]

        // internal faces with normal in z-direction
        for(i_face = 0; i_face < MY_POLY_INT_FACE_COUNT; ++i_face){
            face_c0[i_face] = i_face;
            face_c1[i_face] = i_face+1;
            face_area[i_face][0] = 0.0;
            face_area[i_face][1] = 0.0;
            face_area[i_face][2] = MY_POLY_FACE_AREA;
        }
    }
}
void test_poly_topo_init_cell_hosts(void *handle, const short i_layer)
{
    if(i_layer == 0){
        int     myid;
        int     i_cell;
        int     *cell_host = rcfd_get_cell_host(handle, i_layer);

        MPI_Comm_rank(MPI_COMM_WORLD, &myid);

        for(i_cell = 0; i_cell < MY_POLY_INT_CELL_COUNT; ++i_cell){
            cell_host[i_cell] = myid;
        }
    }
}
void test_poly_topo_init_nodes(void *handle, const short i_layer)
{
    if(i_layer == 0){
        int     myid;
        int     i_cell, i_node, i_node_depth, i_side;
        int     cell_node_ids[37] = {0};
        int     poly_face_nodes[9] = {8, 6, 6, 4, 4, 4, 4, 4, 4};
        double  node_z = 0.0;
        double  **node_x  = rcfd_get_node(handle, i_layer);
        double hexcenter[2] = {0.0};
        double hexcorners[MY_HEX_NODE_COUNT][2];

        MPI_Comm_rank(MPI_COMM_WORLD, &myid);

        if(!node_x){
            printf("[%d] rCFD has not allocated memory for cell nodes\n", myid);
            return;
        }

        for(i_node = 0; i_node < MY_HEX_NODE_COUNT; ++i_node){
            pointy_hexagon_corner(hexcenter, MY_POLY_SIZE, i_node, &hexcorners[i_node][0]);
        }

        // node positions
        for(i_node_depth = 0; i_node_depth < (MY_POLY_INT_CELL_COUNT + 1); ++i_node_depth) {
            for(i_node = 0; i_node < MY_HEX_NODE_COUNT; ++i_node){
                node_x[i_node + i_node_depth*MY_HEX_NODE_COUNT][0] = hexcorners[i_node][0];
                node_x[i_node + i_node_depth*MY_HEX_NODE_COUNT][1] = hexcorners[i_node][1];
                node_x[i_node + i_node_depth*MY_HEX_NODE_COUNT][2] = node_z + myid*MY_POLY_CELL_HEIGHT*MY_POLY_INT_CELL_COUNT;
            }
            node_z += MY_POLY_CELL_HEIGHT;
        }

        cell_node_ids[0] = 36;

        // face node indices
        for(i_cell = 0; i_cell < MY_POLY_INT_CELL_COUNT; ++i_cell){
            // bottom, top
            for(i_node_depth = 0; i_node_depth < 2; ++i_node_depth) {
                for(i_node = 0; i_node < MY_HEX_NODE_COUNT; ++i_node){
                    cell_node_ids[1 + i_node + i_node_depth*MY_HEX_NODE_COUNT] = i_node + i_node_depth*MY_HEX_NODE_COUNT + i_cell*MY_HEX_NODE_COUNT;
                }
            }
            // sides
            for(i_side = 0; i_side < MY_HEX_NODE_COUNT; ++i_side){
                cell_node_ids[1 + 2*MY_HEX_NODE_COUNT + 4*i_side + 0] = i_cell*MY_HEX_NODE_COUNT + i_side + 1;
                cell_node_ids[1 + 2*MY_HEX_NODE_COUNT + 4*i_side + 1] = i_cell*MY_HEX_NODE_COUNT + i_side;
                cell_node_ids[1 + 2*MY_HEX_NODE_COUNT + 4*i_side + 2] = i_cell*MY_HEX_NODE_COUNT + i_side + MY_HEX_NODE_COUNT;
                cell_node_ids[1 + 2*MY_HEX_NODE_COUNT + 4*i_side + 3] = i_cell*MY_HEX_NODE_COUNT + i_side + MY_HEX_NODE_COUNT + 1;
            }
            cell_node_ids[1 + 2*MY_HEX_NODE_COUNT + 4*(MY_HEX_NODE_COUNT-1) + 0] = i_cell*MY_HEX_NODE_COUNT + 0;
            cell_node_ids[1 + 2*MY_HEX_NODE_COUNT + 4*(MY_HEX_NODE_COUNT-1) + 3] = i_cell*MY_HEX_NODE_COUNT + 0 + MY_HEX_NODE_COUNT;

            rcfd_set_cell_shape(handle, i_layer, i_cell, 42, cell_node_ids, poly_face_nodes);
        }
    }
}

int main(int argc, char *argv[])
{
    int myid = -1; // MPI rank
    int compute_node_count = 0;
    void *myrcfd;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &compute_node_count);

    rcfd_open(argc, argv, MPI_COMM_WORLD, &myrcfd);

    rcfd_register_user_function(myrcfd, "user_set_file_dict",   (void(*)(void))test_poly_topo_set_file_dict);
    rcfd_register_user_function(myrcfd, "user_set_cell_dict",   (void(*)(void))test_poly_topo_set_cell_dict);
    rcfd_register_user_function(myrcfd, "user_set_face_dict",   (void(*)(void))test_poly_topo_set_face_dict);
    rcfd_register_user_function(myrcfd, "user_set_node_dict",   (void(*)(void))test_poly_topo_set_node_dict);
    rcfd_register_user_function(myrcfd, "user_init_cells",      (void(*)(void))test_poly_topo_init_cells);
    rcfd_register_user_function(myrcfd, "user_init_faces",      (void(*)(void))test_poly_topo_init_faces);
    rcfd_register_user_function(myrcfd, "user_init_cell_hosts", (void(*)(void))test_poly_topo_init_cell_hosts);
    rcfd_register_user_function(myrcfd, "user_init_nodes",      (void(*)(void))test_poly_topo_init_nodes);

    rcfd_init(myrcfd, 1);

    rcfd_write_topo(myrcfd);

    int pass_test = 0;

    if(myid == 0) printf("\n");
    MPI_Barrier(MPI_COMM_WORLD);
    pass_test = rcfd_test_module(myrcfd, "parallel"); fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    {
        if(pass_test != 0){
            printf("[%d] rCFD failed test 'parallel' failed with code %d\n", myid, pass_test);
        }
        else{
            printf("[%d] rCFD passed test 'parallel'\n", myid);
        }
        fflush(stdout);
    }

    if(myid == 0) printf("\n");
    MPI_Barrier(MPI_COMM_WORLD);
    pass_test = rcfd_test_module(myrcfd, "report"); fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    {
        if(pass_test != 0){
            printf("[%d] rCFD failed test 'report' failed with code %d\n", myid, pass_test);
        }
        else{
            printf("[%d] rCFD passed test 'report'\n", myid);
        }
        fflush(stdout);
    }

    if(myid == 0) printf("\n");
    MPI_Barrier(MPI_COMM_WORLD);
    pass_test = rcfd_test_module(myrcfd, "memory"); fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    {
        if(pass_test != 0){
            printf("[%d] rCFD failed test 'memory' failed with code %d\n", myid, pass_test);
        }
        else{
            printf("[%d] rCFD passed test 'memory'\n", myid);
        }
        fflush(stdout);
    }

    if(myid == 0) printf("\n");
    MPI_Barrier(MPI_COMM_WORLD);
    pass_test = rcfd_test_module(myrcfd, "topo"); fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    {
        if(pass_test != 0){
            printf("[%d] rCFD failed test 'topo' failed with code %d\n", myid, pass_test);
        }
        else{
            printf("[%d] rCFD passed test 'topo'\n", myid);
        }
        fflush(stdout);
    }

    if(myid == 0) printf("\n");
    MPI_Barrier(MPI_COMM_WORLD);
    pass_test = rcfd_test_module(myrcfd, "dump"); fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    {
        if(pass_test != 0){
            printf("[%d] rCFD failed test 'dump' failed with code %d\n", myid, pass_test);
        }
        else{
            printf("[%d] rCFD passed test 'dump'\n", myid);
        }
        fflush(stdout);
    }

    rcfd_close(myrcfd);

    MPI_Finalize();

    return 0;
}


