/******************************************************************************
 * @internal
 * @file        rCFD_fluent.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_FLUENT_H
#define RCFD_FLUENT_H

#include "udf.h"
#include "rCFD_library.h"
#include "rCFD_globals.h"
#include "rCFD_types.h"

#ifndef REGISTER_RCFD_UDF
#define REGISTER_RCFD_UDF(funcName) \
        rcfd_register_user_function(handle, #funcName, (void(*)(void))funcName)
#endif

#define DEBUG_INTERPOLATION 0

#if DEBUG_INTERPOLATION

// in case of DEBUG_INTERPOLATION intermediate fields (displacement, errors) are written to UDMIs

enum{               // rCFD_user_vars

    u_cal_x,        // 0
    u_cal_y,
    u_cal_z,
    u_cal_dx,
    u_cal_dy,
    u_cal_dz,

    u_c2c0_x,       // 6
    u_c2c0_y,
    u_c2c0_z,
    u_c2c0_dx,
    u_c2c0_dy,
    u_c2c0_dz,

    u_c2c1_x,       // 12
    u_c2c1_y,
    u_c2c1_z,
    u_c2c1_dx,
    u_c2c1_dy,
    u_c2c1_dz,

    number_of_rCFD_user_vars
};

#endif // DEBUG_INTERPOLATION

void fluent_init_topo();
void fluent_free_topo();

void fluent_set_cell_dict(void *handle, const short i_layer);
void fluent_set_face_dict(void *handle, const short i_layer);
void fluent_set_node_dict(void *handle, const short i_layer);

void fluent_init_cells     (void *handle, const short i_layer);
void fluent_init_faces     (void *handle, const short i_layer);
void fluent_init_nodes     (void *handle, const short i_layer);
void fluent_init_cell_hosts(void *handle, const short i_layer);

int fluent_get_partition_cell_id(int fluent_cell_id, int fluent_thread_id);

int fluent_to_vtk_cell_type(int fluent_cell_type, int *cell_node_ids);

short fluent_get_phase_id(rCFD_type *rcfd, const short i_phase);
void  fluent_set_vof     (rCFD_type *rcfd);

int  fluent_get_interface_face_count();
void fluent_get_interface_face_centers(double *interface_face_centers,
                                       int     interface_face_count);

void fluent_debug_test_displacement(double **test_displ, int post);

enum {
    NODE_VSITED = 0,
    NODE_LOCAL_ID
};

typedef struct  Fluent_Topo_struct
{
    int     number_of_int_cells, number_of_ext_cells;

    int     number_of_int_faces, number_of_ext_faces;

    int     number_of_nodes;

    int     max_fluent_cell_thread_id, max_fluent_cell_id;

    int     *rcfd_cell_thread_id;

    int     **rcfd_cell_id;

} Fluent_Topo_type;

// write debug information to UDMI
void fluent_debug_test_displacement(double **test_displ, int post)
{
#if DEBUG_INTERPOLATION
    rCFD_type *rcfd = (rCFD_type*)myrcfd;
    int i_dim, i_cell, i_fluent_cell, i_thread;
    int i_layer = rcfd_get_current_layer(myrcfd);
    double  **cell_x = rcfd_get_cell_center(myrcfd, i_layer);

    Domain  *d = Get_Domain(1);
    Thread  *t;

    thread_loop_c(t,d){ begin_c_loop(i_fluent_cell, t){

        i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(t)];
        i_cell = Fluent_Topo->rcfd_cell_id[i_thread][i_fluent_cell];

        for(i_dim = 0; i_dim < 3; i_dim++){

            if(_C(i_layer).weight_after_shift[i_cell] > 0.0){

                C_UDMI(i_fluent_cell, t, ((post? u_c2c1_x  : u_c2c0_x ) + i_dim))  = test_displ[i_cell][i_dim];
                C_UDMI(i_fluent_cell, t, ((post? u_c2c1_dx : u_c2c0_dx) + i_dim))  = cell_x[i_cell][i_dim];
                C_UDMI(i_fluent_cell, t, ((post? u_c2c1_dx : u_c2c0_dx) + i_dim)) -= test_displ[i_cell][i_dim];
            }
            else{

                C_UDMI(i_fluent_cell, t, ((post? u_c2c1_x  : u_c2c0_x ) + i_dim)) = 0.0;
                C_UDMI(i_fluent_cell, t, ((post? u_c2c1_dx : u_c2c0_dx) + i_dim)) = 0.0;
            }
        }
    }end_c_loop(i_fluent_cell, t)}
#else
    (void)(test_displ);
    (void)(post);
#endif // DEBUG_INTERPOLATION
}

/**
 * @brief allocate and initialize the global Fluent_Topo structure with the
 *        ANSYS Fluent mesh information, used to translate Fluent grid cells to
 *        rCFD grid cells (used in prep and run mode) by
 *        rCFD_cell_id = Fluent_Topo->rcfd_cell_id[fluent_cell_thread_id][fluent_cell_id]
 *        - multiple Fluent cell threads are collected into one global rCFD cell thread with
 *          continuous counting [0, ..., (cell_count - 1)]
 *        - interface ghost cells are omitted (in that case
 *          global_cell_id[fluent_cell_thread_id][fluent_cell_id] = -1)
 */
void fluent_init_topo()
{
#if RP_NODE

#if 1   // local vars

    int     i_thread, i_fluent_cell, i_cell, i_cell_int, i_cell_ext, i_node;

    int     max_fluent_cell_thread_id, number_of_fluent_cell_threads, max_fluent_cell_id;

    int     c0, c1, i_fluent_face, i_thread_ct0, i_thread_ct1;

    int     **cell_valid = NULL;

    Domain  *d = Get_Domain(1);

    Thread  *t = NULL, *ct0 = NULL, *ct1 = NULL;

    Node    *node;
#endif

    Fluent_Topo = (Fluent_Topo_type*)malloc(sizeof(Fluent_Topo_type));

    /* strategy:

        (1) create data struct Fluent_Topo->rcfd_cell_id[fluent_cell_thread_id][fluent_cell_id]
            and cell_valid[fluent_cell_thread_id][fluent_cell_id]
        (2) fill cell_valid by face loop
        (3) correct rcfd_cell_id
    */

    // get max_fluent_cell_thread_id, set Fluent_Topo->rcfd_cell_thread_id
    {
        max_fluent_cell_thread_id = -1;

        number_of_fluent_cell_threads = 0;

        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            if(max_fluent_cell_thread_id < THREAD_ID(t)){

                max_fluent_cell_thread_id = THREAD_ID(t);
            }

            number_of_fluent_cell_threads++;

        }}

        Fluent_Topo->max_fluent_cell_thread_id = max_fluent_cell_thread_id;

        Fluent_Topo->rcfd_cell_thread_id = malloc((max_fluent_cell_thread_id + 1) * sizeof(int));

        i_thread = 0;

        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(t)] = i_thread;

            i_thread++;

        }}
    }

    // get number of int. cells and max. of i_fluent_cell, set Fluent_Topo->rcfd_cell_id
    {
        max_fluent_cell_id = -1;

        // get max_fluent_cell_id
        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            begin_c_loop_all(i_fluent_cell, t){

                if(max_fluent_cell_id < i_fluent_cell){

                    max_fluent_cell_id = i_fluent_cell;
                }

            }end_c_loop_all(i_fluent_cell, t)

        }}

        // alloc and init Fluent_Topo->rcfd_cell_id and cell_valid
        {
            Fluent_Topo->max_fluent_cell_id = max_fluent_cell_id;

            int n = 0;

            int *data  = malloc(sizeof(int) * number_of_fluent_cell_threads * (max_fluent_cell_id + 1));
            int *data2 = malloc(sizeof(int) * number_of_fluent_cell_threads * (max_fluent_cell_id + 1));

            Fluent_Topo->rcfd_cell_id = malloc(sizeof(int*) * number_of_fluent_cell_threads);
            cell_valid =                malloc(sizeof(int*) * number_of_fluent_cell_threads);

            for(i_thread = 0; i_thread < number_of_fluent_cell_threads; ++i_thread){

                Fluent_Topo->rcfd_cell_id[i_thread] = &data [n];
                cell_valid[i_thread] =                &data2[n];

                n += (max_fluent_cell_id + 1);
            }

            for(i_thread = 0; i_thread < number_of_fluent_cell_threads; i_thread++){

                for(i_cell = 0; i_cell < (max_fluent_cell_id + 1); i_cell++){

                    Fluent_Topo->rcfd_cell_id[i_thread][i_cell] = -1;
                    cell_valid[i_thread][i_cell] =                 0;
                }
            }
        }

    }

    // get number of int. faces and ext. faces, fill cell_valid
    {
        Fluent_Topo->number_of_int_faces = 0;
        Fluent_Topo->number_of_ext_faces = 0;

        thread_loop_f(t,d){if(THREAD_TYPE(t)==THREAD_F_INTERIOR){begin_f_loop(i_fluent_face, t){

            c0 = (int)F_C0(i_fluent_face, t);
            c1 = (int)F_C1(i_fluent_face, t);

            ct0 = F_C0_THREAD(i_fluent_face, t);
            ct1 = F_C1_THREAD(i_fluent_face, t);

            if((c0 >= 0)&&(c1 >= 0)&&
               (THREAD_ID(ct0) == THREAD_ID(ct1))){ // otherwise interface face

                if((C_PART(c0, ct0) == myid) && (C_PART(c1, ct1) == myid)){

                    Fluent_Topo->number_of_int_faces++;
                }
                else if((C_PART(c0, ct0) == myid) || (C_PART(c1, ct1) == myid)){

                    Fluent_Topo->number_of_ext_faces++;
                }

                i_thread_ct0 = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(ct0)];
                i_thread_ct1 = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(ct1)];

                cell_valid[i_thread_ct0][c0] = 1;
                cell_valid[i_thread_ct1][c1] = 1;
            }
        }end_f_loop(i_fluent_face, t)}}
    }

    // set Fluent_Topo->rcfd_cell_id (avoid interface ghost cells)
    {
        i_cell_int = 0;

        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(t)];

            begin_c_loop_all(i_fluent_cell, t){

                if(cell_valid[i_thread][i_fluent_cell]){

                    if(C_PART(i_fluent_cell, t) == myid){

                        Fluent_Topo->rcfd_cell_id[i_thread][i_fluent_cell] = i_cell_int;

                        i_cell_int++;
                    }
                }
                else{
                    Fluent_Topo->rcfd_cell_id[i_thread][i_fluent_cell] = -1; // interface ghost cell
                }

            }end_c_loop_all(i_fluent_cell, t)
        }}

        Fluent_Topo->number_of_int_cells = i_cell_int;

        i_cell_ext = 0;

        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(t)];

            begin_c_loop_all(i_fluent_cell, t){

                if(cell_valid[i_thread][i_fluent_cell]){

                    if(C_PART(i_fluent_cell, t) != myid){

                        Fluent_Topo->rcfd_cell_id[i_thread][i_fluent_cell] = Fluent_Topo->number_of_int_cells + i_cell_ext;

                        i_cell_ext++;
                    }
                }

            }end_c_loop_all(i_fluent_cell, t)
        }}

        Fluent_Topo->number_of_ext_cells = i_cell_ext;
    }

    // set number_of_nodes
    {
        Fluent_Topo->number_of_nodes = 0;

        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            begin_c_loop_all(i_fluent_cell, t){

                c_node_loop(i_fluent_cell, t, i_node){

                    node = C_NODE(i_fluent_cell, t, i_node);

                    N_UDMI(node, NODE_VSITED) = 0.0; // not yet visited
                }
            }end_c_loop_all(i_fluent_cell, t)
        }}

        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(t)];

            begin_c_loop_all(i_fluent_cell, t){

                if(cell_valid[i_thread][i_fluent_cell]){

                    c_node_loop(i_fluent_cell, t, i_node){

                        node = C_NODE(i_fluent_cell, t, i_node);

                        if(N_UDMI(node, NODE_VSITED) == 0.0){

                            N_UDMI(node, NODE_VSITED) = 1.0; // visited

                            Fluent_Topo->number_of_nodes++;
                        }
                    }
                }

            }end_c_loop_all(i_fluent_cell, t)
        }}
    }

    // free local vars
    {
        free(cell_valid[0]);
        free(cell_valid   );
    }
#endif // RP_NODE
}

/**
 * @brief free the global Fluent_Topo structure
 */
void fluent_free_topo()
{
#if RP_NODE
    if(Fluent_Topo){

        free(Fluent_Topo->rcfd_cell_thread_id);

        if(Fluent_Topo->rcfd_cell_id){

            free(Fluent_Topo->rcfd_cell_id[0]);
            free(Fluent_Topo->rcfd_cell_id);
        }

        free(Fluent_Topo);
    }
#endif
}

/**
 * @brief extract the number of internal and external cells from the ANSYS
 *        Fluent mesh and set those parameters in the rCFD Cell_Dict. Note
 *        that this overwrites the parameters that may have been read from
 *        rCFD topo files
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 */
void fluent_set_cell_dict(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer > 0) return;

    if(!Fluent_Topo){

        rcfd_report0(handle, RCFD_REPORT_ERROR,
                     "fluent_set_cell_dict: Fluent_Topo == NULL");
        return;
    }

    rcfd_set_dict_property_i(handle, "Cell_Dict", i_layer, -1, -1, "number_of_int_cells", Fluent_Topo->number_of_int_cells);
    rcfd_set_dict_property_i(handle, "Cell_Dict", i_layer, -1, -1, "number_of_ext_cells", Fluent_Topo->number_of_ext_cells);

#endif // RP_NODE
}

/**
 * @brief extract the number of internal and external faces from the ANSYS
 *        Fluent mesh and set those parameters in the rCFD Face_Dict. Note
 *        that this overwrites the parameters that may have been read from
 *        rCFD topo files
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 */
void fluent_set_face_dict(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer > 0) return;

    if(!Fluent_Topo){

        rcfd_report0(handle, RCFD_REPORT_ERROR,
                     "fluent_set_face_dict: Fluent_Topo == NULL");
        return;
    }

    rcfd_set_dict_property_i(handle, "Face_Dict", i_layer, -1, -1, "number_of_int_faces", Fluent_Topo->number_of_int_faces);
    rcfd_set_dict_property_i(handle, "Face_Dict", i_layer, -1, -1, "number_of_ext_faces", Fluent_Topo->number_of_ext_faces);

#endif // RP_NODE
}

/**
 * @brief set the number of cell nodes extracted from from the ANSYS Fluent
 *        mesh in the rCFD Node_Dict. Note that this overwrites the
 *        parameter that may have been read from rCFD topo files
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 */
void fluent_set_node_dict(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer > 0) return;

    if(!Fluent_Topo){

        rcfd_report0(handle, RCFD_REPORT_ERROR,
                     "fluent_set_node_dict: Fluent_Topo == NULL");
        return;
    }

    rcfd_set_dict_property_i(handle, "Node_Dict", i_layer, -1, -1, "number_of_nodes", Fluent_Topo->number_of_nodes);
#endif // RP_NODE
}

/**
 * @brief extract cell centers and cell volumes from the ANSYS Fluent mesh
 *        to initialize the corresponding rCFD cell data structures. Note
 *        that this overwrites the values that may have been read from rCFD
 *        topo files
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 */
void fluent_init_cells(void *handle, const short i_layer)
{
#if RP_NODE

    if(i_layer == 0){

#if 1   // local vars

        int     i_cell, i_fluent_cell, i_dim;

        double  x[3];

        double  **cell_x    = rcfd_get_cell_center(handle, i_layer);

        double  *cell_volume= rcfd_get_cell_volume(handle, i_layer);

        Domain  *d = Get_Domain(1);

        Thread  *t;
#endif

        // set Fluent based cell values (x, vol, dx)
        {
            thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

                begin_c_loop_all(i_fluent_cell, t){

                    // set _C(i_layer).x, _C(i_layer).volume
                    {
                        i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

                        if(i_cell > -1){

                            C_CENTROID(x, i_fluent_cell, t);

                            for(i_dim = 0; i_dim < 3; ++i_dim){

                                cell_x[i_cell][i_dim] = x[i_dim];
                            }

                            cell_volume[i_cell] = C_VOLUME(i_fluent_cell, t);
                        }
                    }

                }end_c_loop_all(i_fluent_cell,t)
            }}
        }
    }
#endif // RP_NODE
}

/**
 * @brief extract face area vectors and the cells on either side of the
 *        faces from the ANSYS Fluent mesh to initialize the corresponding
 *        rCFD face data structures. Note that this overwrites the values
 *        that may have been read from rCFD topo files
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 */
void fluent_init_faces(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){

#if 1   // local vars

        int     i_face_int, i_face_ext, i_dim,i_face, i_thread, i_cell, i_fluent_face;

        int     c0, c1;

        double  A[3];

        int     int_face_count = rcfd_get_int_face_count(handle, i_layer);

        int     *face_c0    = rcfd_get_face_c0  (handle, i_layer); // [i_face]

        int     *face_c1    = rcfd_get_face_c1  (handle, i_layer); // [i_face]

        double  **face_area = rcfd_get_face_area(handle, i_layer); // [i_face][i_dim]

        Domain  *d = Get_Domain(1);

        Thread  *t = NULL, *ct0 = NULL, *ct1 = NULL;
#endif

        // set _F(i_layer).c0/c1 and _F(i_layer).A
        {
            i_face_int = 0;
            i_face_ext = 0;

            thread_loop_f(t,d){if(THREAD_TYPE(t)==THREAD_F_INTERIOR){begin_f_loop(i_fluent_face, t){

                c0 = (int)F_C0(i_fluent_face, t);
                c1 = (int)F_C1(i_fluent_face, t);

                ct0 = F_C0_THREAD(i_fluent_face, t);
                ct1 = F_C1_THREAD(i_fluent_face, t);

                if((c0 >= 0) && (c1 >= 0) && (THREAD_ID(ct0) == THREAD_ID(ct1))){

                    if((C_PART(c0, ct0) == myid) && (C_PART(c1, ct1) == myid)){

                        i_face = i_face_int;

                        i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(ct0)]; // same for ct1

                        i_cell = Fluent_Topo->rcfd_cell_id[i_thread][c0];

                        face_c0[i_face] = i_cell;

                        i_cell = Fluent_Topo->rcfd_cell_id[i_thread][c1];

                        face_c1[i_face] = i_cell;

                        F_AREA(A, i_fluent_face, t);

                        for(i_dim = 0; i_dim < 3; ++i_dim){

                            face_area[i_face][i_dim] = A[i_dim];
                        }

                        i_face_int++;

                    }
                    else if((C_PART(c0, ct0) == myid) || (C_PART(c1, ct1) == myid)){

                        i_face = int_face_count + i_face_ext;

                        i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(ct0)];

                        i_cell = Fluent_Topo->rcfd_cell_id[i_thread][c0];

                        face_c0[i_face] = i_cell;

                        i_cell = Fluent_Topo->rcfd_cell_id[i_thread][c1];

                        face_c1[i_face] = i_cell;

                        // eventually swap c0/c1 such that c1 is always corona cell
                        if(C_PART(c0, ct0) != myid){

                            i_cell = face_c0[i_face];

                            face_c0[i_face] = face_c1[i_face];

                            face_c1[i_face] = i_cell;
                        }

                        F_AREA(A, i_fluent_face, t);

                        for(i_dim = 0; i_dim < 3; ++i_dim){

                            face_area[i_face][i_dim] = A[i_dim];
                        }

                        i_face_ext++;
                    }
                }

            }end_f_loop(i_fluent_face, t)}}
        }

    }
#endif // RP_NODE
}

/**
 * @brief extract the cell host information from the ANSYS Fluent mesh to
 *        initialize the corresponding rCFD cell data structures. Note that
 *        this overwrites the values that may have been read from rCFD topo
 *        files
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 */
void fluent_init_cell_hosts(void *handle, const short i_layer)
{
    if(i_layer == 0){    // only implemented for base layer

#if 1   // local vars

        int     i_cell, i_fluent_cell;

        int     *cell_host = rcfd_get_cell_host(handle, i_layer);

        Domain  *d = Get_Domain(1);

        Thread  *t;
#endif

        thread_loop_c(t,d){if(FLUID_CELL_THREAD_P(t)){

            begin_c_loop_all(i_fluent_cell,t){

                i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

                if(i_cell > -1){

                    cell_host[i_cell] = C_PART(i_fluent_cell,t);
                }

            }end_c_loop_all(i_fluent_cell,t)

        }}
    }
}

/**
 * @brief Convert ANSYS Fluent cell type and order of node indices to VTK definition
 * @param fluent_cell_type  Ansys Fluent cell type
 * @param cell_node_ids     Ansys Fluent cell node ids to be reordered to VTK definition
 * @return corresponding VTK cell type or vertex type if unsupported
 */
int fluent_to_vtk_cell_type(int fluent_cell_type, int *cell_node_ids)
{
    int tmp_node_id;
    switch(fluent_cell_type){

    case TRI_CELL:
        // Fluent = VTK
        return RCFD_CELL_TRIANGLE;
#if RP_2D
    case QUAD_CELL:
        // Fluent = VTK
        return RCFD_CELL_QUADRILATERAL;
#endif
    case TET_CELL:
        // Fluent =/= VTK
        tmp_node_id = cell_node_ids[2];
        cell_node_ids[2] = cell_node_ids[3];
        cell_node_ids[3] = tmp_node_id;
        return RCFD_CELL_TETRAHEDRON;
    case PYRAMID_CELL:
        // Fluent =/= VTK
        tmp_node_id = cell_node_ids[2];
        cell_node_ids[2] = cell_node_ids[4];
        cell_node_ids[4] = tmp_node_id;
        return RCFD_CELL_PYRAMID;
    case WEDGE_CELL:
        // Fluent = VTK
        return RCFD_CELL_WEDGE;
    case HEX_CELL:
        tmp_node_id = cell_node_ids[2];
        cell_node_ids[2] = cell_node_ids[4];
        cell_node_ids[4] = tmp_node_id;
        tmp_node_id = cell_node_ids[6];
        cell_node_ids[6] = cell_node_ids[8];
        cell_node_ids[8] = tmp_node_id;
        return RCFD_CELL_HEXAHEDRON;
    case POLY_CELL:
        return RCFD_CELL_POLYHEDRON;
    default:
        // store any unsupported cell type as vertex
        Message("\n... WARNING: fluent_to_vtk_cell_type: unsupported cell type %d", fluent_cell_type);
        return RCFD_CELL_VERTEX;
    }
}

/**
 * @brief extract the cell type and cell nodes from the ANSYS Fluent mesh
 *        to initialize the corresponding rCFD cell/node data structures.
 *
 * @param handle    pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 */
void fluent_init_nodes(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){

        int i_node, i_fluent_cell, i_thread;
        int node_local_id = 0;
        double  **node_x  = rcfd_get_node(handle, i_layer);
        Node    *node;
        Domain  *d = Get_Domain(1);
        Thread  *t = NULL;

        if(!node_x){

            Message("\n... WARNING: [%d] rCFD has not allocated memory for cell nodes\n", myid);
            return;
        }

        // init node properties for all nodes
        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            begin_c_loop_all(i_fluent_cell, t){

                c_node_loop(i_fluent_cell,t,i_node){

                    node = C_NODE(i_fluent_cell,t,i_node);

                    N_UDMI(node, NODE_VSITED)   =  0.0; // not yet visited
                    N_UDMI(node, NODE_LOCAL_ID) = -1.0; // not yet set
                }
            }end_c_loop_all(i_fluent_cell,t)
        }}

        // set node positions and local node IDs
        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(t)];

            begin_c_loop_all(i_fluent_cell, t){

                if(fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t)) > -1){

                    c_node_loop(i_fluent_cell,t,i_node){

                        node = C_NODE(i_fluent_cell,t,i_node);

                        if(N_UDMI(node, NODE_VSITED) == 0.0){

                            node_x[node_local_id][0] = NODE_X(node);
                            node_x[node_local_id][1] = NODE_Y(node);
                            node_x[node_local_id][2] = NODE_Z(node);

                            N_UDMI(node, NODE_VSITED)   = 1.0;
                            N_UDMI(node, NODE_LOCAL_ID) = node_local_id;
                            node_local_id++;
                        }
                    }
                }
            }end_c_loop_all(i_fluent_cell,t)
        }}

        // set cell types and node indices
        thread_loop_c(t, d){if(FLUID_CELL_THREAD_P(t)){

            int i_cell, i_face, cell_type;
            int number_of_cell_faces;

            i_thread = Fluent_Topo->rcfd_cell_thread_id[THREAD_ID(t)];

            begin_c_loop_all(i_fluent_cell, t){

                if(fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t)) > -1){

                    int *poly_face_nodes = NULL; // contains number of poly faces followed by number of face nodes
                    int *cell_node_ids = NULL; // contains number of nodes followed by node IDs

                    i_cell = Fluent_Topo->rcfd_cell_id[i_thread][i_fluent_cell];

                    cell_type = C_TYPE(i_fluent_cell, t);

                    number_of_cell_faces = C_NFACES(i_fluent_cell, t); // cell face count

                    if(number_of_cell_faces > RCFD_MAX_FACES_PER_POLY_CELL){

                        Message("\n... WARNING: [%d] rCFD supports max. %d faces per polyhedron, %d were detected in mesh\n",
                                myid, RCFD_MAX_FACES_PER_POLY_CELL, number_of_cell_faces);
                    }

                    if(cell_type == POLY_CELL){

                        poly_face_nodes = malloc(sizeof(int) * (1 + number_of_cell_faces));
                        poly_face_nodes[0] = number_of_cell_faces;
                        // Note that C_NNODES(i_fluent_cell, t) is different from the sum of all face nodes F_NNODES(i_fluent_face, tf)
                        cell_node_ids = (int*)malloc(sizeof(int) * 1);
                        cell_node_ids[0] = 0;

                        c_face_loop(i_fluent_cell,t,i_face){

                            int i_fluent_face = C_FACE(i_fluent_cell, t, i_face);
                            Thread *tf = C_FACE_THREAD(i_fluent_cell, t, i_face);
                            int number_of_face_nodes = F_NNODES(i_fluent_face, tf);

                            poly_face_nodes[i_face+1] = number_of_face_nodes;
                            cell_node_ids = realloc(cell_node_ids, sizeof(int) * (1 + cell_node_ids[0] + number_of_face_nodes));

                            f_node_loop(i_fluent_face, tf, i_node){

                                node = F_NODE(i_fluent_face, tf, i_node);
                                cell_node_ids[1+cell_node_ids[0]+i_node] = N_UDMI(node, NODE_LOCAL_ID);
                            }
                            cell_node_ids[0] += number_of_face_nodes;
                        }
                    }
                    else{

                        cell_node_ids = malloc(sizeof(int) * (1 + C_NNODES(i_fluent_cell, t)));
                        cell_node_ids[0] = C_NNODES(i_fluent_cell, t); // cell node count

                        c_node_loop(i_fluent_cell,t,i_node){

                            if(i_node == RCFD_MAX_NODES_PER_CELL) break;

                            node = C_NODE(i_fluent_cell,t,i_node);

                            cell_node_ids[i_node+1] = N_UDMI(node, NODE_LOCAL_ID);
                        }
                    }

                    cell_type = fluent_to_vtk_cell_type(cell_type, cell_node_ids);

                    rcfd_set_cell_shape(handle, i_layer, i_cell, cell_type, cell_node_ids, poly_face_nodes);

                    free(poly_face_nodes);
                    free(cell_node_ids);

                }end_c_loop_all(i_fluent_cell,t)
            }
        }}
    }

#endif // RP_NODE
}

/**
 * @brief get the ANSYS Fluent phase ID from the given rCFD phase index
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param i_phase   specifies the rCFD phase index
 * @return the ANSYS Fluent phase ID
 */
short fluent_get_phase_id(rCFD_type *rcfd, const short i_phase)
{
#if RP_NODE
    if(rcfd->Solver_Dict->single_phase_vof){

        return rcfd->Solver_Dict->single_phase_vof_id;
    }
    else{

        return i_phase;
    }
#endif
    return 0;
}

/**
 * @brief get the rCFD partition cell id
 *
 * @param fluent_cell_id     ANSYS Fluent cell id
 * @param fluent_thread_id   ANSYS Fluent cell thread id
 * @return the rCFD partition cell id or -1 for host or error
 */
int fluent_get_partition_cell_id(int fluent_cell_id, int fluent_thread_id)
{
#if RP_NODE
    if((fluent_thread_id < 0) || (fluent_thread_id > Fluent_Topo->max_fluent_cell_thread_id))
        return -2;

    if((fluent_cell_id < 0) || (fluent_cell_id > Fluent_Topo->max_fluent_cell_id))
        return -3;

    int i_thread    = Fluent_Topo->rcfd_cell_thread_id[fluent_thread_id];
    int i_cell      = Fluent_Topo->rcfd_cell_id[i_thread][fluent_cell_id];

    return i_cell;
#endif
    return -1;
}

/**
 * @brief extract the cell VOF information from the ANSYS Fluent mesh to
 *        initialize the corresponding rCFD VOF data structures in
 *        preparation mode
 *
 * @param rcfd      pointer to previously created rCFD instance
 */
void fluent_set_vof(rCFD_type *rcfd)
{
#if RP_NODE

#if 1
    int     i_cell, i_fluent_cell, i_phase, i_phase_fluent;
    int     i_layer = 0;
    int     int_cell_count = rcfd_get_int_cell_count(myrcfd, i_layer);
    int     phase_count =    rcfd_get_phase_count   (myrcfd);
    double  **cell_vof = NULL;

    Domain  *d = Get_Domain(1);
    Thread  *t = NULL, *t_phase = NULL;
#endif

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        rcfd_report0(myrcfd, RCFD_REPORT_WARNING,
                     "fluent_set_vof: only sets first frame's vof (not designed for run mode)");
        return;
    }

    // alloc cell_vof
    {
        cell_vof = malloc(int_cell_count * sizeof(double*));

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_vof[i_cell] = malloc(phase_count * sizeof(double));
        }
    }

    if( !rcfd_is_vof_on(myrcfd)) return;

    thread_loop_c( t, d){ if(FLUID_CELL_THREAD_P( t)){

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            i_phase_fluent = fluent_get_phase_id(rcfd, i_phase);

            t_phase = THREAD_SUB_THREAD( t, i_phase_fluent);

            if((t_phase != NULL) &&
               (THREAD_STORAGE( t_phase, SV_VOF) != NULL)) {    // needed for sliding mesh

                begin_c_loop_int(i_fluent_cell, t){             // int bcs of invalid ext cells with sliding mesh

                    i_cell = fluent_get_partition_cell_id(i_fluent_cell, THREAD_ID(t));

                    if(i_cell > -1){

                        cell_vof[i_cell][i_phase] = C_VOF(i_fluent_cell, t_phase);
                    }

                }end_c_loop_int(i_fluent_cell, t)
            }

            t_phase = NULL;
        }
    }}

    rcfd_set_vof(myrcfd, i_layer, cell_vof);

    //free local vars
    {
        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            free(cell_vof[i_cell]);
        }

        free(cell_vof);
    }
#endif
}

/**
 * @brief gets number of (per partition) fluent grid interface faces (each one connecting a pair of interface cells)
 *
 * @return the number of interface faces
 */
int fluent_get_interface_face_count()
{
    int interface_face_count = 0;
#if RP_NODE
    int i_fluent_face;

    Domain *d = Get_Domain(1);
    Thread *t = NULL, *ct0 = NULL, *ct1 = NULL;

    thread_loop_f(t,d){if(THREAD_TYPE(t)==THREAD_F_INTERIOR){begin_f_loop_int(i_fluent_face, t){

        ct0 = F_C0_THREAD(i_fluent_face, t);
        ct1 = F_C1_THREAD(i_fluent_face, t);

        if(THREAD_ID(ct0) != THREAD_ID(ct1)){ // also true for interior face patches

            interface_face_count++;
        }

    }end_f_loop_int(i_fluent_face, t)}}
#endif
    return interface_face_count;
}

/**
 * @brief fills array of (process-local) interface face center coordinates
 *
 * @param interface_face_centers    (flattened) array of interface (process-local) face coords to be filled
 * @param interface_face_count      (process-local) number of interface faces
 */
void fluent_get_interface_face_centers(double *interface_face_centers,
                                       int     interface_face_count)
{
#if RP_NODE
    if(interface_face_count <= 0) return;

    int    i_fluent_face, i_interface_face = 0, i_dim;

    double xf[3];

    Domain *d = Get_Domain(1);
    Thread *t = NULL, *ct0 = NULL, *ct1 = NULL;

    thread_loop_f(t,d){if(THREAD_TYPE(t)==THREAD_F_INTERIOR){begin_f_loop_int(i_fluent_face, t){

        ct0 = F_C0_THREAD(i_fluent_face, t);
        ct1 = F_C1_THREAD(i_fluent_face, t);

        if(THREAD_ID(ct0) != THREAD_ID(ct1)){

            F_CENTROID(xf, i_fluent_face, t);

            for(i_dim = 0; i_dim < 3; i_dim++){

                interface_face_centers[3 * i_interface_face + i_dim] = xf[i_dim];
            }

            i_interface_face++;
        }

    }end_f_loop_int(i_fluent_face, t)}}
#endif
}
#endif // RCFD_FLUENT_H
