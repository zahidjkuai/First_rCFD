/******************************************************************************
 * @internal
 * @file        rCFD_parallel.h
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_PARALLEL
#define RCFD_PARALLEL

#include "rCFD.h"
#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_shift.h"
#include "rCFD_free.h"

void parallel_gatherv0_shifts(rCFD_type *rcfd,
                              C2C_Shift_type *source_shifts,
                              int source_shifts_count,
                              C2C_Shift_type *dest_shifts,
                              int shift_type);
void parallel_bcast0_shifts  (rCFD_type *rcfd,
                              C2C_Shift_type *shifts,
                              int shifts_count,
                              int shift_type);

void parallel_init                 (rCFD_type *rcfd, MPI_Comm communicator);
void parallel_init_topology        (rCFD_type *rcfd, int i_layer);
void parallel_expand_initial_fields(rCFD_type *rcfd, int i_layer);
void parallel_shift_init           (rCFD_type *rcfd);
void init_upper_layer_parallel_c2c (rCFD_type* rcfd);

void parallel_communicate         (rCFD_type *rcfd, double *cell_data, int i_layer, short comm_format);
void parallel_communicate_data    (rCFD_type *rcfd, int i_layer, int i_phase, int i_data, short comm_format);
void parallel_shift_data          (rCFD_type *rcfd, int i_state, int i_phase, int i_frame, int i_layer);
int  parallel_map_phase_data_shift(rCFD_type *rcfd, int i_phase);
int  parallel_map_phase_data      (rCFD_type *rcfd, int i_phase);


#if 1 // parallel utilities

#define TAG_GATHERV 101
#if defined(RCFD_STANDALONE)
#if defined(RCFD_LIB_MPI)
    int     prf_gisum1  (void *handle, int a)     { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_INT,       MPI_SUM, rcfd->Parallel->world); return a; }
    int     prf_gihigh1 (void *handle, int a)     { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_INT,       MPI_MAX, rcfd->Parallel->world); return a; }
    int     prf_gilow1  (void *handle, int a)     { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_INT,       MPI_MIN, rcfd->Parallel->world); return a; }
    double  prf_grsum1  (void *handle, double a)  { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_DOUBLE,    MPI_SUM, rcfd->Parallel->world); return a; }
    double  prf_grhigh1 (void *handle, double a)  { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_DOUBLE,    MPI_MAX, rcfd->Parallel->world); return a; }
    double  prf_grlow1  (void *handle, double a)  { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_DOUBLE,    MPI_MIN, rcfd->Parallel->world); return a; }
    int64_t prf_gi64sum1(void *handle, int64_t a) { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_INT64_T,   MPI_SUM, rcfd->Parallel->world); return a; }
    int64_t prf_gi64high1(void *handle,int64_t a) { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_INT64_T,   MPI_MAX, rcfd->Parallel->world); return a; }
    int64_t prf_gi64low1(void *handle, int64_t a) { rCFD_type *rcfd = (rCFD_type*)handle; MPI_Allreduce(MPI_IN_PLACE, &a, 1, MPI_INT64_T,   MPI_MIN, rcfd->Parallel->world); return a; }
#endif // RCFD_LIB_MPI
#else
    /**
     * @brief Gather varying amounts of integer data from all processes to the root process
     * @param sarray        starting address of send buffer
     * @param scount        number of elements in send buffer
     * @param rarray        address of receive buffer (significant only at root)
     * @param recvcounts    array (of length compute_node_count) containing the number of elements
     *                      that are received from each process (significant only at root)
     * @param displs        array (of length compute_node_count) containing the displacements
     *                      relative to rarray at which to place the incoming data from each process
     *                      (significant only at root)
     * @param root          rank of receiving process
     */
    void prf_gatherv_int(int *sarray, int scount, int *rarray, const int recvcounts[], const int displs[], int root)
    {
#if RP_NODE
        int i_proc;

        if(myid != root){
            if(scount > 0){
                PRF_CSEND_INT(root, sarray, scount, TAG_GATHERV);
            }
            return;
        }

        for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
            if(recvcounts[i_proc] > 0){
                if(i_proc == root){
                    if(scount > 0){
                        memcpy(&rarray[displs[i_proc]], sarray, recvcounts[i_proc]*sizeof(int));
                    }
                }
                else{
                    PRF_CRECV_INT(i_proc, &rarray[displs[i_proc]], recvcounts[i_proc], TAG_GATHERV);
                }
            }
        }
#endif // RP_NODE
    }

    /**
     * @brief Gather varying amounts of floating point data from all processes to the root process
     * @param sarray        starting address of send buffer
     * @param scount        number of elements in send buffer
     * @param rarray        address of receive buffer (significant only at root)
     * @param recvcounts    array (of length compute_node_count) containing the number of elements
     *                      that are received from each process (significant only at root)
     * @param displs        array (of length compute_node_count) containing the displacements
     *                      relative to rarray at which to place the incoming data from each process
     *                      (significant only at root)
     * @param root          rank of receiving process
     */
    void prf_gatherv_real(double *sarray, int scount, double *rarray, const int recvcounts[], const int displs[], int root)
    {
#if RP_NODE
        int i_proc;

        if(myid != root){
            if(scount > 0){
                PRF_CSEND_REAL(root, sarray, scount, TAG_GATHERV);
            }
            return;
        }

        for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
            if(recvcounts[i_proc] > 0){
                if(i_proc == root){
                    if(scount > 0){
                        memcpy(&rarray[displs[i_proc]], sarray, recvcounts[i_proc] * sizeof(double));
                    }
                }
                else{
                    PRF_CRECV_REAL(i_proc, &rarray[displs[i_proc]], recvcounts[i_proc], TAG_GATHERV);
                }
            }
        }
#endif // RP_NODE
    }

    /**
     * @brief Gather varying amounts of char data from all processes to the root process
     * @param sarray        starting address of send buffer
     * @param scount        number of elements in send buffer
     * @param rarray        address of receive buffer (significant only at root)
     * @param recvcounts    array (of length compute_node_count) containing the number of elements
     *                      that are received from each process (significant only at root)
     * @param displs        array (of length compute_node_count) containing the displacements
     *                      relative to rarray at which to place the incoming data from each process
     *                      (significant only at root)
     * @param root          rank of receiving process
     */
    void prf_gatherv_char(char *sarray, int scount, char *rarray, const int recvcounts[], const int displs[], int root)
    {
#if RP_NODE
        int i_proc;

        if(myid != root){
            if(scount > 0){
                PRF_CSEND_CHAR(root, sarray, scount, TAG_GATHERV);
            }
            return;
        }

        for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
            if(recvcounts[i_proc] > 0){
                if(i_proc == root){
                    if(scount > 0){
                        memcpy(&rarray[displs[i_proc]], sarray, recvcounts[i_proc] * sizeof(char));
                    }
                }
                else{
                    PRF_CRECV_CHAR(i_proc, &rarray[displs[i_proc]], recvcounts[i_proc], TAG_GATHERV);
                }
            }
        }
#endif // RP_NODE
    }

#endif // RCFD_STANDALONE

// parallel gather, broadcast shifts

/**
 * @brief Gathers source lists of shifts from all nodes into global list of node-zero
 *        (global list exists on node-zero)
 *
 * @param rcfd                pointer to previously created rCFD instance
 * @param source_shifts       list of shifts to be gathered
 * @param source_shifts_count length of source_shift lists
 * @param dest_shifts         global list of gathered shifts of node-zero
 */
void parallel_gatherv0_shifts(rCFD_type *rcfd,
                             C2C_Shift_type *source_shifts,
                             int  source_shifts_count,
                             C2C_Shift_type *dest_shifts,
                             int shift_type)
{
#if RP_NODE

    int dest_shifts_count = PRF_GISUM1(source_shifts_count);

    if(dest_shifts_count < 1){

        return;
    }

    if(shift_type == RCFD_SHIFT_TYPE_PREPARATION){

        int i_node;
        int *recvcounts = NULL, *displs = NULL;

        if(source_shifts_count == 0){

            shift_initialize_lists(rcfd, source_shifts);    // make sure all pointer members are NULL
        }

        recvcounts = malloc(sizeof(int) * compute_node_count);
        displs =     malloc(sizeof(int) * compute_node_count);

        PRF_GATHER_INT(recvcounts, &source_shifts_count, 1, 0);

        displs[0] = 0;

        for(i_node = 1; i_node < compute_node_count; ++i_node){

            displs[i_node] = displs[i_node-1] + recvcounts[i_node-1];
        }

        PRF_GATHERV_INT (dest_shifts->c0,       source_shifts->c0,      recvcounts, source_shifts_count, displs, 0);
        PRF_GATHERV_INT (dest_shifts->node0,    source_shifts->node0,   recvcounts, source_shifts_count, displs, 0);
        PRF_GATHERV_INT (dest_shifts->c1,       source_shifts->c1,      recvcounts, source_shifts_count, displs, 0);
        PRF_GATHERV_INT (dest_shifts->node1,    source_shifts->node1,   recvcounts, source_shifts_count, displs, 0);
        PRF_GATHERV_REAL(dest_shifts->w0,       source_shifts->w0,      recvcounts, source_shifts_count, displs, 0);
        PRF_GATHERV_REAL(dest_shifts->dist_sq,  source_shifts->dist_sq, recvcounts, source_shifts_count, displs, 0);

        // adapt source_shifts_count, displ and recvcounts for 2-dim arrays
        {
            source_shifts_count *= 3;

            for(i_node = 0; i_node < compute_node_count; ++i_node){

                recvcounts[i_node] *=  3;
                displs[i_node] *=      3;
            }
        }

        if(source_shifts_count > 0){

            PRF_GATHERV_REAL(dest_shifts->c0_coords[0], source_shifts->c0_coords[0], recvcounts, source_shifts_count, displs, 0);
            PRF_GATHERV_REAL(dest_shifts->displ[0],     source_shifts->displ[0],     recvcounts, source_shifts_count, displs, 0);
        }
        else{

            PRF_GATHERV_REAL(dest_shifts->c0_coords[0], NULL, recvcounts, source_shifts_count, displs, 0);
            PRF_GATHERV_REAL(dest_shifts->displ[0],     NULL, recvcounts, source_shifts_count, displs, 0);
        }

        free(recvcounts);
        free(displs);
    }
    else if(shift_type == RCFD_SHIFT_TYPE_INTER_NODE){

        int i_node;
        int *recvcounts = NULL, *displs = NULL;

        recvcounts = malloc(sizeof(int) * compute_node_count);
        displs =     malloc(sizeof(int) * compute_node_count);

        PRF_GATHER_INT(recvcounts, &source_shifts_count, 1, 0);

        displs[0] = 0;

        for(i_node = 1; i_node < compute_node_count; ++i_node){

            displs[i_node] = displs[i_node-1] + recvcounts[i_node-1];
        }

        PRF_GATHERV_INT(dest_shifts->c0, source_shifts->c0, recvcounts, source_shifts_count, displs, 0);
        PRF_GATHERV_INT(dest_shifts->c1, source_shifts->c1, recvcounts, source_shifts_count, displs, 0);

        int shift_format = rcfd->Solver->c2c_shift_format;

        if( (shift_format == RCFD_SHIFT_C0_N0_C1_N1_W0)  ||
            (shift_format == RCFD_SHIFT_C0_N0_C1_W0)     ||
            (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR)){

            PRF_GATHERV_INT(dest_shifts->node0, source_shifts->node0, recvcounts, source_shifts_count, displs, 0);
        }
        else{

            PRF_GATHERV_CHAR((char*)dest_shifts->node0char, (char*)source_shifts->node0char, recvcounts, source_shifts_count, displs, 0);
        }

        if( (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR)     ||
            (shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            PRF_GATHERV_CHAR((char*)dest_shifts->w0char, (char*)source_shifts->w0char, recvcounts, source_shifts_count, displs, 0);
        }
        else{

            PRF_GATHERV_REAL(dest_shifts->w0, source_shifts->w0, recvcounts, source_shifts_count, displs, 0);
        }

        free(recvcounts);
        free(displs);
    }
    else{   // other shift_types

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_RUN,
            "parallel_gatherv0_shift: not implemented for shift_type %d",
            shift_type);
    }
#endif
}

/**
 * @brief Broadcasts node-zero source lists of shifts to all nodes
 *        (lists exist on all nodes)
 *
 * @param rcfd        pointer to previously created rCFD instance
 * @param shifts      pointer to shifts to be broadcasted
 * @param shift count length of source lists
 */
void parallel_bcast0_shifts(rCFD_type *rcfd, C2C_Shift_type *shifts, int shifts_count, int shift_type)
{
#if RP_NODE

    const int source_node = 0;

    if(shifts_count < 1){

        return;
    }

    if(shift_type == RCFD_SHIFT_TYPE_PREPARATION){

        PRF_BCAST_INT (shifts->c0,      shifts_count, source_node);
        PRF_BCAST_INT (shifts->node0,   shifts_count, source_node);
        PRF_BCAST_INT (shifts->c1,      shifts_count, source_node);
        PRF_BCAST_INT (shifts->node1,   shifts_count, source_node);
        PRF_BCAST_REAL(shifts->w0,      shifts_count, source_node);
        PRF_BCAST_REAL(shifts->dist_sq, shifts_count, source_node);

        PRF_BCAST_REAL(shifts->c0_coords[0], 3 * shifts_count, source_node);
        PRF_BCAST_REAL(shifts->displ[0],     3 * shifts_count, source_node);
    }
    else if(shift_type == RCFD_SHIFT_TYPE_INTER_NODE){

        PRF_BCAST_INT (shifts->c0, shifts_count, source_node);
        PRF_BCAST_INT (shifts->c1, shifts_count, source_node);

        int shift_format = rcfd->Solver->c2c_shift_format;

        if( (shift_format == RCFD_SHIFT_C0_N0_C1_N1_W0)  ||
            (shift_format == RCFD_SHIFT_C0_N0_C1_W0)     ||
            (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR)){

            PRF_BCAST_INT (shifts->node0, shifts_count, source_node);
        }
        else{

            PRF_BCAST_CHAR((char*)shifts->node0char, shifts_count, source_node);
        }

        if( (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR)     ||
            (shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            PRF_BCAST_CHAR((char*)shifts->w0char, shifts_count, source_node);
        }
        else{

            PRF_BCAST_REAL(shifts->w0, shifts_count, source_node);
        }
    }
    else{   // other shift_types

        report_warning0(rcfd, RCFD_REPORT_CATEGORY_RUN,
            "parallel_bcast0_shifts: not implemented for shift_type %d",
            shift_type);
    }
#endif
}

#endif // parallel utilities

#if 1 /* init */

void parallel_init(rCFD_type *rcfd, MPI_Comm communicator)
{
#if RP_HOST
    rcfd->Parallel->MPI_Topo    = NULL;
#endif
#if RP_NODE
    rcfd->Parallel->MPI_Topo    = (MPI_topo_type*)malloc(sizeof(MPI_topo_type));
    rcfd->Parallel->MPI_Topo->MPI_Cell = NULL;
#endif
    rcfd->Parallel->finalize = 0;
    rcfd->Parallel->world = communicator;
    MPI_Comm_rank(communicator, &rcfd->Parallel->rank);
    MPI_Comm_size(communicator, &rcfd->Parallel->size);
}

void parallel_read_hosts(rCFD_type *rcfd, int i_layer)
{
#if RP_NODE
    FILE    *f_topo = NULL;

    if(i_layer > 0){

        return;
    }

    {
        char    filename[80];

        snprintf(filename, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);

        f_topo = fopen(filename, "r");

        if(f_topo == NULL){

            report_info(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "[%d] parallel_read_hosts: failed to open '%s'. "
                        "Expecting topology to be set via user functions",
                        myid, filename);
            return;
        }
    }

    // read cell data
    {
        int     i_cell;
        int     number_of_cells = 0;
        int     number_of_faces = 0;
        int     number_of_nodes = 0;
        double  x[3];
        double  volume;

        fscanf(f_topo, "%d %d %d \n", &number_of_cells, &number_of_faces, &number_of_nodes);

        if(_CELL_DICT(i_layer).number_of_cells > 0 && _CELL_DICT(i_layer).number_of_cells != number_of_cells){

            report_info(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                        "[%d] parallel_read_hosts: mismatch of cell count in Cell_Dict (%d) and Topo file (%d)."
                        "Expecting topology to be set via user functions",
                        myid, _CELL_DICT(i_layer).number_of_cells, number_of_cells);

            fclose(f_topo);

            return;
        }

        //loop_cells
        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            fscanf(f_topo, "%le %le %le %le %d \n", &x[0], &x[1], &x[2], &volume, &_MPI_Cell.host_of_cell[i_cell]);
        }
    }
#endif
}

void parallel_set_host_of_cell(rCFD_type *rcfd, int i_layer)
{
#if RP_NODE
    parallel_read_hosts(rcfd, i_layer);

    rcfd->rCFD_UDF->_user_init_cell_hosts(rcfd, i_layer);
#endif
}

void parallel_init_topology(rCFD_type *rcfd, int i_layer)
{
#if RP_NODE
#if 1   // local vars
        int     i_node, i_cell, i_list, i_host;
        int     size;
        short   local_debug_on = 0;
#endif

        // only implemented for base layer !
        if(i_layer > 0){

            _MPI_Cell.total_number_of_ext_cells     = 0;
            _MPI_Cell.number_of_ext_cells_per_node  = NULL;
            _MPI_Cell.number_of_host_cells_per_node = NULL;
            _MPI_Cell.host_of_cell                  = NULL;
            _MPI_Cell.host_of_ext_cells             = NULL;
            _MPI_Cell.hosting_cell_index            = NULL;
            _MPI_Cell.host2ext_index                = NULL;
            _MPI_Cell.data                          = NULL;

            return;
        }

        /* P.1  allocate MPI_topo and _MPI_Cell  */
        {

            rcfd->Parallel->MPI_Topo->MPI_Cell = (MPI_cells_type*)malloc(rcfd->Solver_Dict->number_of_layers * sizeof(MPI_cells_type));

            _MPI_Cell.host_of_cell = (int*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int));
            _MPI_Cell.hosting_cell_index = NULL;

            if(myid > 0){

                PRF_CSEND_INT(node_zero, &_CELL_DICT(i_layer).number_of_ext_cells, 1, myid);

                _MPI_Cell.total_number_of_ext_cells     = -1;
                _MPI_Cell.number_of_ext_cells_per_node  = NULL;
                _MPI_Cell.number_of_host_cells_per_node = NULL;
                _MPI_Cell.host_of_ext_cells             = NULL;
                _MPI_Cell.host2ext_index                = NULL;
                _MPI_Cell.data                          = NULL;
            }

            if(myid == 0){

                // per node and total number corona cells

                _MPI_Cell.number_of_ext_cells_per_node = (int*)malloc(compute_node_count * sizeof(int));
                _MPI_Cell.number_of_ext_cells_per_node[0] = _CELL_DICT(i_layer).number_of_ext_cells;
                _MPI_Cell.total_number_of_ext_cells = _CELL_DICT(i_layer).number_of_ext_cells;

                for(i_node = 1; i_node < compute_node_count; i_node++){

                    PRF_CRECV_INT(i_node, &_MPI_Cell.number_of_ext_cells_per_node[i_node], 1, i_node);
                    _MPI_Cell.total_number_of_ext_cells += _MPI_Cell.number_of_ext_cells_per_node[i_node];
                }

                // other node-0 MPI vars allocated, but not set yet

                _MPI_Cell.number_of_host_cells_per_node = (int*)malloc(compute_node_count * sizeof(int));
                _MPI_Cell.host_of_ext_cells = (int*)malloc(_MPI_Cell.total_number_of_ext_cells * sizeof(int));
                _MPI_Cell.host2ext_index = (int*)malloc(_MPI_Cell.total_number_of_ext_cells * sizeof(int));
                _MPI_Cell.data = (double*)malloc(_MPI_Cell.total_number_of_ext_cells * sizeof(double));
            }
        }

        /* P.2  all nodes set _MPI_Cell.host_of_cell */
        {
            parallel_set_host_of_cell(rcfd, i_layer);
        }

        /* P.3  set up MPI_cell communication */
        {
            double *tmp_x;
            double *list_of_double = NULL;

            /* P3.1 node-0 fills _MPI_Cell.host_of_ext_cells, tmp_x */
            {
                int int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
                int ext_cell_count = _CELL_DICT(i_layer).number_of_ext_cells;

                if(myid > 0){

                    PRF_CSEND_INT(node_zero, &_MPI_Cell.host_of_cell[int_cell_count], ext_cell_count, myid);
                    PRF_CSEND_REAL(node_zero, &_C(i_layer).x[int_cell_count][0], 3 * ext_cell_count, myid);
                }

                if(myid == 0){

                    tmp_x = (double*)malloc(3 * _MPI_Cell.total_number_of_ext_cells * sizeof(double));

                    i_list = 0;

                    // get ext cell info from node 0
                    memcpy(&_MPI_Cell.host_of_ext_cells[i_list], &_MPI_Cell.host_of_cell[int_cell_count], sizeof(int)    * ext_cell_count);
                    memcpy(&tmp_x[(3 * i_list)],                 &_C(i_layer).x[int_cell_count][0],       sizeof(double) * ext_cell_count * 3);
                    i_list += ext_cell_count;

                    // get ext cell info from other nodes
                    for(i_node = 1; i_node < compute_node_count; i_node++){

                        int ext_cell_count_node = _MPI_Cell.number_of_ext_cells_per_node[i_node];

                        PRF_CRECV_INT(i_node, &_MPI_Cell.host_of_ext_cells[i_list], ext_cell_count_node, i_node);
                        PRF_CRECV_REAL(i_node, &tmp_x[(3 * i_list)], (3 * ext_cell_count_node), i_node);
                        i_list += ext_cell_count_node;
                    }
                }
            }

            /* P3.2 analyze _MPI_Cell.host_of_ext_cells, set _MPI_Cell.host2ext_index */
            {
                if(myid == 0){

                    memset(_MPI_Cell.number_of_host_cells_per_node, 0, sizeof(int) * compute_node_count);

                    // set number of host cells per node
                    for(i_cell = 0; i_cell < _MPI_Cell.total_number_of_ext_cells; i_cell++){

                        i_host = _MPI_Cell.host_of_ext_cells[i_cell];

                        _MPI_Cell.number_of_host_cells_per_node[i_host]++;
                    }

                    int *host_start_index = malloc(sizeof(int) * compute_node_count);
                    int *host_index = malloc(sizeof(int) * compute_node_count);

                    host_start_index[0] = 0; host_index[0] = 0;

                    for(i_host = 1; i_host < compute_node_count; i_host++){

                        host_start_index[i_host] = host_start_index[(i_host-1)] + _MPI_Cell.number_of_host_cells_per_node[(i_host-1)];

                        host_index[i_host] = 0;
                    }

                    for(i_cell = 0; i_cell < _MPI_Cell.total_number_of_ext_cells; i_cell++){

                        i_host = _MPI_Cell.host_of_ext_cells[i_cell];

                        _MPI_Cell.host2ext_index[(host_start_index[i_host] + host_index[i_host])] = i_cell;

                        host_index[i_host]++;
                    }

                    free(host_start_index);
                    free(host_index);
                }
            }

            /* P3.3 set _MPI_Cell.hosting_cell_index */
            {
                if(myid == 0){

                    _MPI_Cell.number_of_host_cells = _MPI_Cell.number_of_host_cells_per_node[0];

                    list_of_double = (double*)malloc(3 * _MPI_Cell.number_of_host_cells_per_node[0] * sizeof(double));

                    i_list = 0;

                    for(i_cell = 0; i_cell < _MPI_Cell.total_number_of_ext_cells; i_cell++){

                        i_host = _MPI_Cell.host_of_ext_cells[i_cell];

                        if(i_host == 0){

                            list_of_double[(3 * i_list)] =      tmp_x[(3 * i_cell)];
                            list_of_double[(3 * i_list + 1)] =  tmp_x[(3 * i_cell + 1)];
                            list_of_double[(3 * i_list + 2)] =  tmp_x[(3 * i_cell + 2)];

                            i_list++;
                        }
                    }

                    _MPI_Cell.hosting_cell_index = (int*)malloc(_MPI_Cell.number_of_host_cells * sizeof(int));

                    for(i_host = 0; i_host < _MPI_Cell.number_of_host_cells; i_host++){

                        _MPI_Cell.hosting_cell_index[i_host] = -1;
                    }

                    for(i_list = 0; i_list < _MPI_Cell.number_of_host_cells_per_node[0]; i_list++){

                        //loop_int_cells
                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                            //i_cell = _F(i_layer).c0[i_face];     // potential host cell

                            if( (_C(i_layer).x[i_cell][0] == list_of_double[(3 * i_list + 0)]) &&
                                (_C(i_layer).x[i_cell][1] == list_of_double[(3 * i_list + 1)]) &&
                                (_C(i_layer).x[i_cell][2] == list_of_double[(3 * i_list + 2)])){

                                _MPI_Cell.hosting_cell_index[i_list] = i_cell;
                            }
                        }
                    }

                    free(list_of_double);

                    /* send tmp_x to Nodes */
                    for(i_node = 1; i_node < compute_node_count; i_node++){

                        PRF_CSEND_INT(i_node, &_MPI_Cell.number_of_host_cells_per_node[i_node], 1, node_zero);

                        list_of_double = (double*)malloc( 3 * _MPI_Cell.number_of_host_cells_per_node[i_node] * sizeof(double));

                        i_list = 0;

                        for(i_cell = 0; i_cell < _MPI_Cell.total_number_of_ext_cells; i_cell++){

                            i_host = _MPI_Cell.host_of_ext_cells[i_cell];

                            if(i_host == i_node){

                                list_of_double[(3 * i_list)] =      tmp_x[(3 * i_cell)];
                                list_of_double[(3 * i_list + 1)] =  tmp_x[(3 * i_cell + 1)];
                                list_of_double[(3 * i_list + 2)] =  tmp_x[(3 * i_cell + 2)];

                                i_list++;
                            }
                        }

                        PRF_CSEND_REAL(i_node, list_of_double, (3 * _MPI_Cell.number_of_host_cells_per_node[i_node]), node_zero);

                        free(list_of_double);
                    }

                    free(tmp_x);
                }

                if(myid > 0){

                    PRF_CRECV_INT(node_zero, &size, 1, node_zero);  /* size = number of host cells for this node */

                    _MPI_Cell.number_of_host_cells = size;
                    _MPI_Cell.hosting_cell_index = (int*)malloc(size * sizeof(int));

                    for(i_host = 0; i_host < _MPI_Cell.number_of_host_cells; i_host++){

                        _MPI_Cell.hosting_cell_index[i_host] = -1;
                    }

                    list_of_double = (double*)malloc(3 * size * sizeof(double));

                    PRF_CRECV_REAL(node_zero, list_of_double, 3 * size, node_zero);

                    for(i_list = 0; i_list < size; i_list++){

                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                            if( (_C(i_layer).x[i_cell][0] == list_of_double[(3 * i_list + 0)]) &&
                                (_C(i_layer).x[i_cell][1] == list_of_double[(3 * i_list + 1)]) &&
                                (_C(i_layer).x[i_cell][2] == list_of_double[(3 * i_list + 2)])){

                                _MPI_Cell.hosting_cell_index[i_list] = i_cell;
                            }
                        }
                    }

                    free(list_of_double);
                }

                if(local_debug_on){

                    for(i_host = 0; i_host < _MPI_Cell.number_of_host_cells; i_host++){

                        if((_MPI_Cell.hosting_cell_index[i_host] < 0)||(_MPI_Cell.hosting_cell_index[i_host] > _CELL_DICT(i_layer).number_of_int_cells)){

                            report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                         "[%d] _MPI_Cell.hosting_cell_index[%d] = %d",
                                         myid, i_host, _MPI_Cell.hosting_cell_index[i_host]);
                        }
                    }
                }
            }
        }

        if(local_debug_on){

            if(myid == 0){

                report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                              "total_number_of_ext_cells = %d",
                              _MPI_Cell.total_number_of_ext_cells);

                for(i_node = 0; i_node < compute_node_count; i_node++){

                    report_debug0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                  "number_of_ext/host_cells_per_node[%d] = %d / %d",
                                  i_node,
                                  _MPI_Cell.number_of_ext_cells_per_node[i_node],
                                  _MPI_Cell.number_of_host_cells_per_node[i_node]);
                }
            }
        }

#endif
    }

void parallel_expand_initial_fields(rCFD_type *rcfd, int i_layer)
{
#if RP_NODE
    int     i_rec_user, i_frame, i_cell;
    double  *tmp = NULL;

    if(_C(i_layer).rec_user != NULL){

        // expand rec_user values to corona cells
        {
            tmp = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));

            //loop_frames
            for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                for(i_rec_user = 0; i_rec_user < _CELL_DICT(i_layer).number_of_rec_user_vars; i_rec_user++){

                    //loop_cells
                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        tmp[i_cell] = _C(i_layer).rec_user[i_frame][i_cell][i_rec_user];
                    }

                    parallel_communicate(rcfd, tmp, i_layer, RCFD_PARALLEL_MAP_HOST_FORMAT);

                    //loop_ext_cells
                    for(i_cell = _CELL_DICT(i_layer).number_of_int_cells; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        _C(i_layer).rec_user[i_frame][i_cell][i_rec_user] = tmp[i_cell];
                    }
                }
            }

            free(tmp);
        }
    }

    if(_C(i_layer).local_diff != NULL){

        int i_phase;

        tmp = (double*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(double));

        for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

            for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    tmp[i_cell] = _C(i_layer).local_diff[i_frame][i_phase][i_cell];
                }

                parallel_communicate(rcfd, tmp, i_layer, RCFD_PARALLEL_MAP_HOST_FORMAT);

                for(i_cell = _CELL_DICT(i_layer).number_of_int_cells; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    _C(i_layer).local_diff[i_frame][i_phase][i_cell] = tmp[i_cell];
                }
            }
        }

        free(tmp);
    }
#endif
}

void parallel_shift_init(rCFD_type *rcfd)
{
#if RP_NODE

    int i_state, i_phase, i_frame, i_layer, i_node, i_shift, i_shift_out, i_global_list;

    int shifts_in_count, shifts_out_count, max_transfer_shifts_count, transfer_shifts_count;

    int node0;

    C2C_Shift_type global_shifts;

    // A. node-zero allocates global_shifts (based on max_transfer_shifts_count)
    {
        max_transfer_shifts_count = 0;

        for(i_state = 0; i_state < rcfd->Solver_Dict->number_of_states; i_state++){

            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                    for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                        shifts_in_count = PRF_GISUM1(rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.shifts_count);

                        if(shifts_in_count > max_transfer_shifts_count){

                            max_transfer_shifts_count = shifts_in_count;
                        }
                    }
                }
            }
        }

        shift_allocate_lists(rcfd, &global_shifts, max_transfer_shifts_count, RCFD_SHIFT_TYPE_INCOMING);
    }

    // B. sets C2Cs.shift_out, C2Cs.in2out, C2Cs.shifts_to_node_count
    {
        for(i_state = 0; i_state < rcfd->Solver_Dict->number_of_states; i_state++){

            for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                     for(i_layer = 0; i_layer < rcfd->Solver_Dict->number_of_layers; i_layer++){

                        // B.1 per loop initializations
                        {
                            shifts_in_count = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.shifts_count;

                            transfer_shifts_count = PRF_GISUM1(shifts_in_count);

                            global_shifts.valid_shifts_count = transfer_shifts_count;

                            PRF_GATHER_INT(rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_to_node_count,
                                           &shifts_in_count, 1, 0);

                            parallel_gatherv0_shifts(rcfd,
                                                    &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in,
                                                    shifts_in_count,
                                                    &global_shifts,
                                                    RCFD_SHIFT_TYPE_INCOMING);

                            parallel_bcast0_shifts(rcfd, &global_shifts, transfer_shifts_count, RCFD_SHIFT_TYPE_INCOMING);

                        }

                        // B.2: set shifts_out for each Node
                        {
                            shifts_out_count = 0;

                            for(i_shift = 0; i_shift < transfer_shifts_count; i_shift++){

                                node0 = shifts_get_node0(rcfd, &global_shifts, i_shift);

                                if((node0 > -1) && (node0 == myid)){

                                    shifts_out_count++;
                                }
                            }

                            shift_allocate_lists(rcfd,
                                                &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_out,
                                                 shifts_out_count,
                                                 RCFD_SHIFT_TYPE_OUTGOING);

                            i_shift_out = 0;

                            for(i_shift = 0; i_shift < transfer_shifts_count; i_shift++){

                                node0 = shifts_get_node0(rcfd, &global_shifts, i_shift);

                                if((node0 > -1) && (node0 == myid)){

                                    shift_copy_one_in_list(rcfd,
                                                           &rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_out,
                                                           i_shift_out,
                                                           &global_shifts, i_shift,
                                                           RCFD_SHIFT_TYPE_OUTGOING);

                                    i_shift_out++;
                                }
                            }
                        }

                        /* B.3: Node-0 link outgoing data to list of incoming data */
                        if(myid == 0){

                            rcfd->C2C[i_state][i_phase][i_frame][i_layer].out2in = malloc(transfer_shifts_count * sizeof(int));

                            i_global_list = 0;

                            for(i_node = 0; i_node < compute_node_count; i_node++){

                                i_shift_out = 0;

                                for(i_shift = 0; i_shift < transfer_shifts_count; i_shift++){

                                    node0 = shifts_get_node0(rcfd, &global_shifts, i_shift);

                                    if((node0 > -1) && (node0 == i_node)){

                                        rcfd->C2C[i_state][i_phase][i_frame][i_layer].out2in[i_global_list + i_shift_out] = i_shift;

                                        i_shift_out++;
                                    }
                                }

                                i_global_list += i_shift_out;
                            }
                        }
                    }
                }
            }
        }
    }

    // C. Allocate Parallel->transfer_data
    if(myid == 0){

        int  max_phase_data_count = rcfd->Solver_Dict->max_phase_data_count;

        rcfd->Parallel->transfer_data_in =  malloc( max_transfer_shifts_count * max_phase_data_count * sizeof(double));
        rcfd->Parallel->transfer_data_out = malloc( max_transfer_shifts_count * max_phase_data_count * sizeof(double));
    }

    // D. free local vars
    {
        free_lists_of_shift(rcfd, &global_shifts);
    }
#endif
}

void init_upper_layer_parallel_c2c(rCFD_type* rcfd)
{
#if RP_NODE
#if MULTI_LAYER_IMPLEMENTATION_COMPLETE
    short   debug_this_code = 0;

    if(rcfd->Solver_Dict->number_of_layers > 1){

#if 1       /* local vars */

        int     i_state, i_phase, i_frame, i_layer, i_node, i_cell, i_shift, i_tmp;

        int     number_of_node_cells = 0, number_of_shifts, number_of_shifts_in_upper_layer, number_of_shifts_from_node = 0;

        int     node0, c0, c1, upper_c0, upper_c1;

        int     **tmp_parent_ids = NULL, *list_of_c0, *list_of_node0, *list_of_upper_c0;

        double  w0;
#endif

        i_layer = 0;

        i_tmp = 0;

        while((i_layer + 1) < rcfd->Solver_Dict->number_of_layers){

            /* E.1 Node-0 collects parent_id's */
            {
                if(myid == 0){

                    tmp_parent_ids = (int**)malloc(compute_node_count * sizeof(int*));

                    tmp_parent_ids[0] = (int*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int));

                    //loop_cells
                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        tmp_parent_ids[0][i_cell] = _C(i_layer).parent_cell[i_cell];
                    }

                    for(i_node = 1; i_node < compute_node_count; i_node++){

                        PRF_CRECV_INT(i_node, &number_of_node_cells, 1, i_node);

                        tmp_parent_ids[i_node] = (int*)malloc(number_of_node_cells * sizeof(int));

                        list_of_upper_c0 = (int*)malloc(number_of_node_cells * sizeof(int));

                        PRF_CRECV_INT(i_node, list_of_upper_c0, number_of_node_cells, i_node);

                        for(i_cell = 0; i_cell < number_of_node_cells; i_cell++){

                            tmp_parent_ids[i_node][i_cell] = list_of_upper_c0[i_cell];
                        }

                        free(list_of_upper_c0);
                    }
                }

                if(myid > 0){

                    PRF_CSEND_INT(node_zero, &_CELL_DICT(i_layer).number_of_cells, 1, myid);

                    PRF_CSEND_INT(node_zero, _C(i_layer).parent_cell, _CELL_DICT(i_layer).number_of_cells, myid);
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] E.1", myid);
                }
            }

            /* E.2 set (i_layer + 1) C2Cs */
            /* TODO remove doublets */
            //loop_states
            for(i_state = 0; i_state < rcfd->Solver_Dict->number_of_states; i_state++){

                //loop_phases
                for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                    //loop_frames
                    for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                        number_of_shifts = rcfd->C2Cs[i_state][i_phase][i_frame].number_of_shifts_in[i_layer];

                        number_of_shifts_in_upper_layer = number_of_shifts;

                        rcfd->C2Cs[i_state][i_phase][i_frame].number_of_shifts_in[(i_layer + 1)] = number_of_shifts;

                        rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)] = (C2C_shift_type*)malloc(number_of_shifts_in_upper_layer * sizeof(C2C_shift_type));

                        if(myid == 0){

                            /* set (i_layer + 1) shifts_in for node-0 */
                            //loop_shifts
                            for(i_shift = 0; i_shift < number_of_shifts; i_shift++){

                                node0 = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].node0;

                                c0 = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].c0;

                                upper_c0 = tmp_parent_ids[node0][c0];

                                c1 =  rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].c1;

                                upper_c1 = _C(i_layer).parent_cell[c1];

                                w0 = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].w0;

                                if(w0 == 0.0){

                                    i_tmp++;
                                }

                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].node0 = node0;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].c0 =    upper_c0;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].node1 = myid;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].c1 =    upper_c1;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].w0 =    w0;
                            }

                            /* send list_of_upper_c0 to nodes */
                            for(i_node = 1; i_node < compute_node_count; i_node++){

                                PRF_CRECV_INT(i_node, &number_of_shifts_from_node, 1, i_node);

                                list_of_node0 = (int*)malloc(number_of_shifts_from_node * sizeof(int));

                                list_of_c0 = (int*)malloc(number_of_shifts_from_node * sizeof(int));

                                PRF_CRECV_INT(i_node, list_of_node0, number_of_shifts_from_node, i_node);

                                PRF_CRECV_INT(i_node, list_of_c0, number_of_shifts_from_node, i_node);

                                list_of_upper_c0 = (int*)malloc(number_of_shifts_from_node * sizeof(int));

                                for(i_shift = 0; i_shift < number_of_shifts_from_node; i_shift++){

                                    node0 = list_of_node0[i_shift];

                                    c0 = list_of_c0[i_shift];

                                    list_of_upper_c0[i_shift] = tmp_parent_ids[node0][c0];
                                }

                                PRF_CSEND_INT(i_node, list_of_upper_c0, number_of_shifts_from_node, node_zero);

                                free(list_of_node0);

                                free(list_of_c0);

                                free(list_of_upper_c0);
                            }
                        }

                        if(myid>0){

                            PRF_CSEND_INT(node_zero, &number_of_shifts, 1, myid);

                            list_of_node0 = (int*)malloc(number_of_shifts * sizeof(int));

                            list_of_c0 = (int*)malloc(number_of_shifts * sizeof(int));

                            //loop_shifts
                            for(i_shift = 0; i_shift < number_of_shifts; i_shift++){

                                list_of_node0[i_shift] = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].node0;

                                list_of_c0[i_shift] = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].c0;
                            }

                            PRF_CSEND_INT(node_zero, list_of_node0, number_of_shifts, myid);

                            PRF_CSEND_INT(node_zero, list_of_c0, number_of_shifts, myid);

                            free(list_of_node0);

                            free(list_of_c0);

                            list_of_upper_c0 = (int*)malloc(number_of_shifts * sizeof(int));

                            PRF_CRECV_INT(node_zero, list_of_upper_c0, number_of_shifts, node_zero);

                            //loop_shifts
                            for(i_shift = 0; i_shift < number_of_shifts; i_shift++){

                                node0 = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].node0;

                                upper_c0 = list_of_upper_c0[i_shift];

                                c1 = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].c1;

                                upper_c1 = _C(i_layer).parent_cell[c1];

                                w0 = rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].w0;

                                if(w0 == 0.0){

                                    i_tmp++;
                                }


                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].node0 = node0;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].c0 =    upper_c0;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].node1 = myid;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].c1 =    upper_c1;
                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[(i_layer + 1)][i_shift].w0 =    w0;
                            }

                            free(list_of_upper_c0);
                        }

                        if((debug_this_code)&&(i_frame == 0)){

                            report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                         "[%d] E.2: i_layer %d, number_of_shifts %d/%d w0==0 %d",
                                         myid, i_layer, number_of_shifts, number_of_shifts_in_upper_layer, i_tmp);

                        }
                    }
                }
            }

            /* E.3 free tmp_parent_ids */
            {
                if(tmp_parent_ids != NULL){

                    for(i_node = 0; i_node < compute_node_count; i_node++){

                        if(tmp_parent_ids[i_node] != NULL){

                            free(tmp_parent_ids[i_node]);
                        }
                    }

                    free(tmp_parent_ids);
                }

                if(debug_this_code){

                    report_debug(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE, "[%d] E.3", myid);
                }
            }

            i_layer++;
        }

        if(debug_this_code){

            char    filename[40];
            FILE    *fi = NULL;

            sprintf(filename,"%s_%d", "tmp_parallel_L1_shifts", myid);

            fi = fopen(filename,"w");

            i_layer = 1;

            //loop_states
            for(i_state = 0; i_state < rcfd->Solver_Dict->number_of_states; i_state++){

                //loop_phases
                for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

                    //loop_frames
                    for(i_frame = 0; i_frame < rcfd->Solver_Dict->number_of_frames; i_frame++){

                        number_of_shifts = rcfd->C2Cs[i_state][i_phase][i_frame].number_of_shifts_in[i_layer];

                        fprintf(fi, "%d\n", number_of_shifts);

                        //loop_shifts
                        for(i_shift = 0; i_shift < number_of_shifts; i_shift++){

                            fprintf(fi, "%d %d %d %d %e\n", rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].node0, rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].c0,

                                rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].node1, rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].c1, rcfd->C2Cs[i_state][i_phase][i_frame].shifts_in[i_layer][i_shift].w0);
                        }
                    }
                }
            }

            fclose(fi);
        }

    }
#endif // MULTI_LAYER_IMPLEMENTATION_COMPLETE
#endif // RP_NODE
}

#endif

#if 1 /* communication */

void parallel_communicate(rCFD_type *rcfd, double *cell_data, int i_layer, short comm_format)
{
#if RP_NODE
    int     i_cell, i_list, i_node;
    int     size = 0;
    int     int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
    int     ext_cell_count = _CELL_DICT(i_layer).number_of_ext_cells;

    double  *list_of_double;

    /* MPI.1 Node-0 fills _MPI_Cell.data with corona cell values */
    {
        if(myid > 0){

            PRF_CSEND_INT(node_zero, &ext_cell_count, 1, myid);
            PRF_CSEND_REAL(node_zero, &cell_data[int_cell_count], ext_cell_count, myid);
        }

        if(myid == 0){

            i_list = 0;
            memcpy(&_MPI_Cell.data[i_list], &cell_data[int_cell_count], sizeof(double) * ext_cell_count);
            i_list += ext_cell_count;

            for(i_node = 1; i_node < compute_node_count; i_node++){

                PRF_CRECV_INT(i_node, &size, 1, i_node);
                PRF_CRECV_REAL(i_node, &_MPI_Cell.data[i_list], size, i_node);
                i_list += size;
            }
        }
    }

    /* MPI.2 Node-0 sends _MPI_Cell.data to host-Nodes */
    {
        if(myid == 0){

            i_list = _MPI_Cell.number_of_host_cells_per_node[0];

            for(i_node = 1; i_node < compute_node_count; i_node++){

                PRF_CSEND_INT(i_node, &_MPI_Cell.number_of_host_cells_per_node[i_node], 1, node_zero);

                list_of_double = (double*)malloc(_MPI_Cell.number_of_host_cells_per_node[i_node] * sizeof(double));

                for(i_cell = 0; i_cell < _MPI_Cell.number_of_host_cells_per_node[i_node]; i_cell++){

                    list_of_double[i_cell] = _MPI_Cell.data[_MPI_Cell.host2ext_index[i_list]];

                    i_list++;
                }

                PRF_CSEND_REAL(i_node, list_of_double, _MPI_Cell.number_of_host_cells_per_node[i_node], node_zero);

                free(list_of_double);
            }
        }

    }

    /* MPI.3 all nodes update data based on hosting inner cells; Node-0 collects updated data into _MPI_Cell.data */
    {

        /* other nodes receive corona cells, eventually update their host cells and send them back to node-0 */
        if(myid > 0){

            PRF_CRECV_INT(node_zero, &size, 1, node_zero);

            list_of_double = (double*)malloc(size * sizeof(double));

            PRF_CRECV_REAL(node_zero, list_of_double, size, node_zero);

            // eventually update host_cell data by corona cell values
            if(comm_format == RCFD_PARALLEL_SUM_CELLS_FORMAT){

                for(i_list = 0; i_list < size; i_list++){

                    cell_data[_MPI_Cell.hosting_cell_index[i_list]] += list_of_double[i_list];
                }
            }

            // send host cell values back to node-0
            for(i_list = 0; i_list < size; i_list++){

                list_of_double[i_list] = cell_data[_MPI_Cell.hosting_cell_index[i_list]];
            }

            PRF_CSEND_INT(node_zero, &size, 1, myid);

            PRF_CSEND_REAL(node_zero, list_of_double, size, myid);

            free(list_of_double);
        }


        if(myid == 0){

            /* Node-0 fills his own _MPI_Cell.data */

            size = _MPI_Cell.number_of_host_cells_per_node[0];

            list_of_double = (double*)malloc(size * sizeof(double));

            // fill list of host cells of node-0
            for(i_cell = 0; i_cell < size; i_cell++){

                list_of_double[i_cell] = _MPI_Cell.data[_MPI_Cell.host2ext_index[i_cell]];
            }

            //
            if(comm_format == RCFD_PARALLEL_SUM_CELLS_FORMAT){

                for(i_list = 0; i_list < size; i_list++){

                    cell_data[_MPI_Cell.hosting_cell_index[i_list]] += list_of_double[i_list];
                }
            }

            for(i_list = 0; i_list < size; i_list++){

                list_of_double[i_list] = cell_data[_MPI_Cell.hosting_cell_index[i_list]];
            }

            for(i_cell = 0; i_cell < size; i_cell++){

                _MPI_Cell.data[_MPI_Cell.host2ext_index[i_cell]] = list_of_double[i_cell];
            }

            free(list_of_double);

            /* Node-0 collects _MPI_Cell.data from other nodes */

            i_list = _MPI_Cell.number_of_host_cells_per_node[0];

            for(i_node = 1; i_node < compute_node_count; i_node++){

                PRF_CRECV_INT(i_node, &size, 1, i_node);

                list_of_double = (double*)malloc(size * sizeof(double));

                PRF_CRECV_REAL(i_node, list_of_double, size, i_node);

                for(i_cell = 0; i_cell < size; i_cell++){

                    _MPI_Cell.data[_MPI_Cell.host2ext_index[i_list]] = list_of_double[i_cell];

                    i_list++;
                }

                free(list_of_double);
            }
        }

    }

    /* MPI.4 Node-0 sends updated data to nodes */
    {
        if(myid == 0){

            i_list = 0;
            memcpy(&cell_data[int_cell_count],  &_MPI_Cell.data[i_list], sizeof(double) * ext_cell_count);
            i_list += ext_cell_count;

            for(i_node = 1; i_node < compute_node_count; i_node++){

                int ext_cell_count_node = _MPI_Cell.number_of_ext_cells_per_node[i_node];

                PRF_CSEND_INT(i_node, &ext_cell_count_node, 1, node_zero);
                PRF_CSEND_REAL(i_node, &_MPI_Cell.data[i_list], ext_cell_count_node, node_zero);
                i_list += ext_cell_count_node;
            }

        }

        if(myid > 0){

            PRF_CRECV_INT(node_zero, &size, 1, node_zero);
            PRF_CRECV_REAL(node_zero, &cell_data[int_cell_count], size, node_zero);
        }
    }
#endif
}

void parallel_communicate_data(rCFD_type *rcfd, int i_layer, int i_phase, int i_data, short comm_format)
{
#if RP_NODE
    int     i_cell;

    // fill _.tmp field with host and corona values
    //loop_cells
    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

        _C(i_layer).tmp[i_cell] = _C(i_layer).data[i_phase][i_cell][i_data];
    }

    parallel_communicate(rcfd, _C(i_layer).tmp, i_layer, comm_format);

    // re-fill ext cells with updated corona values
    //loop_ext_cells
    for(i_cell = _CELL_DICT(i_layer).number_of_int_cells; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

        _C(i_layer).data[i_phase][i_cell][i_data] = _C(i_layer).tmp[i_cell];
    }
#endif
}

void parallel_shift_data(rCFD_type *rcfd, int i_state, int i_phase, int i_frame, int i_layer)
{
#if RP_NODE

#if 1   // local vars

    int     i_shift, i_node, i_global_list, i_data, i_data_shift_in, i_data_shift_out;

    int     c0, c1;

    int     data_count =       rcfd->Phase_Dict[i_phase].number_of_data;
    int     shifts_out_count = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_out.shifts_count;
    int     shifts_in_count =  rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in. shifts_count;

    int     data_from_shifts_out_count;
    int     transfer_data_count;
    int     transfer_shifts_count;
    int     shifts_to_my_node_count;

    int     shift_format = rcfd->Solver->c2c_shift_format;

    int     invalid_shifts_exist     = rcfd->C2C[i_state][i_phase][i_frame][i_layer].invalid_shifts_exist;
    int     shift_weights_add_to_one = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shift_weights_add_to_one;

    int     *shifts_to_node_count = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_to_node_count;
    int     *out2in =               rcfd->C2C[i_state][i_phase][i_frame][i_layer].out2in;

    double  w0, data0, w0_old, data0_old;

    double  *data_from_shifts_out = NULL, *data_to_my_node = NULL;;

#endif

    // event. set w0
    {
        if((shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR) ||
           (shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            if(shifts_in_count > 0){

                rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0 = malloc(shifts_in_count * sizeof(double));

                for(i_shift = 0; i_shift < shifts_in_count; i_shift++){

                    rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0[i_shift] =
                        (double)rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0char[i_shift] / 255.0;
                }
            }
            else{
                rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0 = NULL;
            }
        }
    }

    // all nodes collect data_from_shifts and gather it into node-0's rcfd->Parallel->transfer_data_out
    {
        data_from_shifts_out_count = shifts_out_count * data_count;

        if(data_from_shifts_out_count > 0){

            data_from_shifts_out = malloc(data_from_shifts_out_count * sizeof(double));

            for(i_shift = 0; i_shift < shifts_out_count; i_shift++){

                c0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_out.c0[i_shift];

                for(i_data = 0; i_data < data_count; i_data++){

                    data_from_shifts_out[i_shift * data_count + i_data] = _C(i_layer).data[i_phase][c0][i_data];
                }
            }
        }

        transfer_data_count = PRF_GISUM1(data_from_shifts_out_count);

        int *recvcounts = malloc(sizeof(int) * compute_node_count);
        int *displs =     malloc(sizeof(int) * compute_node_count);

        PRF_GATHER_INT(recvcounts, &data_from_shifts_out_count, 1, 0);

        displs[0] = 0;

        for(i_node = 1; i_node < compute_node_count; ++i_node){

            displs[i_node] = displs[i_node-1] + recvcounts[i_node-1];
        }

        PRF_GATHERV_REAL(rcfd->Parallel->transfer_data_out, data_from_shifts_out, recvcounts, data_from_shifts_out_count, displs, 0);

        free(recvcounts);
        free(displs);
    }

    // node-0 reorders trasfer_data and scatters data_to_my_node to nodes
    {
        if(myid == 0){

            transfer_shifts_count = transfer_data_count / data_count;

            // translate transfer_data_out to transfer_data_in
            for(i_shift = 0; i_shift < transfer_shifts_count; i_shift++){

                i_data_shift_in  = out2in[i_shift] * data_count;
                i_data_shift_out = i_shift * data_count;

                for(i_data = 0; i_data < data_count; i_data++){

                    rcfd->Parallel->transfer_data_in[i_data_shift_in + i_data] =

                        rcfd->Parallel->transfer_data_out[i_data_shift_out + i_data];
                }
            }

            i_global_list = shifts_to_node_count[0] * data_count;

            for(i_node = 1; i_node < compute_node_count; i_node++){

                PRF_CSEND_INT (i_node, &shifts_to_node_count[i_node], 1, node_zero);

                if(shifts_to_node_count[i_node] > 0){

                    PRF_CSEND_REAL(i_node,
                                   &rcfd->Parallel->transfer_data_in[i_global_list],
                                   shifts_to_node_count[i_node] * data_count,
                                   node_zero);

                    i_global_list += shifts_to_node_count[i_node] * data_count;
                }
            }

            data_to_my_node = &rcfd->Parallel->transfer_data_in[0];

            shifts_to_my_node_count = shifts_to_node_count[0];
        }

        if(myid > 0){

            PRF_CRECV_INT(node_zero, &shifts_to_my_node_count, 1, node_zero);

            if(shifts_to_my_node_count > 0){

                data_to_my_node = malloc(shifts_to_my_node_count * data_count *sizeof(double));

                PRF_CRECV_REAL(node_zero, data_to_my_node, shifts_to_my_node_count * data_count, node_zero);
            }
        }
    }

    // all nodes shift data depending on shift characteristics
    {
        if(shift_weights_add_to_one && (!invalid_shifts_exist)){  // sum(w0(c1)) = 1 && w0 > 0

            for(i_shift = 0; i_shift < shifts_to_my_node_count; i_shift++){

                c1 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.c1[i_shift];
                w0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0[i_shift];

                for(i_data = 0; i_data < data_count; i_data++){

                    data0 = data_to_my_node[i_shift * data_count + i_data];

                    _C(i_layer).data_shift[c1][i_data] += w0 * data0;
                }

                _C(i_layer).weight_after_shift[c1] = 1.0;
                _C(i_layer).data_shift_updated[c1] = 1;
            }
        }
        else if(!invalid_shifts_exist){ // w0 > 0

            for(i_shift = 0; i_shift < shifts_to_my_node_count; i_shift++){

                c1 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.c1[i_shift];
                w0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0[i_shift];

                for(i_data = 0; i_data < data_count; i_data++){

                    data0 = data_to_my_node[i_shift * data_count + i_data];

                    w0_old =    _C(i_layer).weight_after_shift[c1];
                    data0_old = _C(i_layer).data_shift[c1][i_data];

                    _C(i_layer).data_shift[c1][i_data] = (w0 * data0 + w0_old * data0_old) / (w0 + w0_old);
                }

                _C(i_layer).weight_after_shift[c1] += w0;
                _C(i_layer).data_shift_updated[c1] = 1;
            }
        }
        else{

            for(i_shift = 0; i_shift < shifts_to_my_node_count; i_shift++){

                w0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0[i_shift];

                if(w0 > 0){

                    c1 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.c1[i_shift];

                    for(i_data = 0; i_data < data_count; i_data++){

                        data0 = data_to_my_node[i_shift * data_count + i_data];

                        w0_old =    _C(i_layer).weight_after_shift[c1];
                        data0_old = _C(i_layer).data_shift[c1][i_data];

                        _C(i_layer).data_shift[c1][i_data] = (w0 * data0 + w0_old * data0_old) / (w0 + w0_old);
                    }

                    _C(i_layer).weight_after_shift[c1] += w0;
                    _C(i_layer).data_shift_updated[c1] = 1;
                }
            }
        }
    }

    // free local vars
    {
        free(data_from_shifts_out);

        if(myid > 0){

            free(data_to_my_node);
        }

        if( (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR) ||
            (shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            free(rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0);
            rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_in.w0 = NULL;
        }
    }

#endif
}

/**
 * @brief map phase cell data_shift from host to corona

 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   index of phase
 * @return 0 or -1 if error
 */
int parallel_map_phase_data_shift(rCFD_type *rcfd, int i_phase)
{
#if RP_NODE
    if(rcfd){

        if(rcfd->Solver_Dict->mode != RCFD_MODE_RUN){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN,
                         "[%d] parallel_map_phase_data_shift: only available in run mode", myid);
            return -1;
        }

        int i_layer = rcfd->Solver->current_layer;

        if((i_layer < 0) || (i_layer >= rcfd->Solver_Dict->number_of_layers)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN,
                         "[%d] parallel_map_phase_data_shift: invalid i_layer %d", myid, i_layer);
            return -1;
        }

        int i_data, i_cell;

        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            for(i_cell = 0;
                i_cell < _CELL_DICT(i_layer).number_of_cells;
                i_cell++){

                _C(i_layer).tmp[i_cell] = _C(i_layer).data_shift[i_cell][i_data];
            }

            parallel_communicate(rcfd, _C(i_layer).tmp, i_layer, RCFD_PARALLEL_MAP_HOST_FORMAT);

            for(i_cell = _CELL_DICT(i_layer).number_of_int_cells;
                i_cell < _CELL_DICT(i_layer).number_of_cells;
                i_cell++){

                _C(i_layer).data_shift[i_cell][i_data] = _C(i_layer).tmp[i_cell];
            }
        }

        return 0;
    }
#endif
    return -1;
}

/**
 * @brief map phase cell data from host to corona

 * @param handle    pointer to previously created rCFD instance
 * @param i_phase   index of phase
 * @return 0 or -1 if error
 */
int parallel_map_phase_data(rCFD_type *rcfd, int i_phase)
{
#if RP_NODE
    if(rcfd){

        if(rcfd->Solver_Dict->mode != RCFD_MODE_RUN){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN,
                         "[%d] parallel_map_phase_data: only available in run mode", myid);
            return -1;
        }

        int i_layer = rcfd->Solver->current_layer;

        if((i_layer < 0) || (i_layer >= rcfd->Solver_Dict->number_of_layers)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN,
                         "[%d] parallel_map_phase_data: invalid i_layer %d", myid, i_layer);
            return -1;
        }

        int i_data, i_cell;

        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            for(i_cell = 0;
                i_cell < _CELL_DICT(i_layer).number_of_cells;
                i_cell++){

                _C(i_layer).tmp[i_cell] = _C(i_layer).data[i_phase][i_cell][i_data];
            }

            parallel_communicate(rcfd, _C(i_layer).tmp, i_layer, RCFD_PARALLEL_MAP_HOST_FORMAT);

            for(i_cell = _CELL_DICT(i_layer).number_of_int_cells;
                i_cell < _CELL_DICT(i_layer).number_of_cells;
                i_cell++){

                _C(i_layer).data[i_phase][i_cell][i_data] = _C(i_layer).tmp[i_cell];
            }
        }

        return 0;
    }
#endif
    return -1;
}

#endif

/*****************************************************************************/

#if defined(RCFD_LIB_TEST_MODULES)

int test_parallel(rCFD_type *rcfd)
{
    int success = 0;
#if RP_NODE
    int sarray_i[3] = {0};
    int *rarray0_i = NULL;
    double sarray_r[3] = {0.};
    double *rarray0_r = NULL;

    sarray_i[0] = 3 * (myid) + 0;
    sarray_i[1] = 3 * (myid) + 1;
    sarray_i[2] = 3 * (myid) + 2;

    sarray_r[0] = 3.0 * (myid) + 0 + 0.5;
    sarray_r[1] = 3.0 * (myid) + 1 + 0.5;
    sarray_r[2] = 3.0 * (myid) + 2 + 0.5;

    PRF_GSYNC();

    Message0("\nStarting rCFD test 'parallel'\n");

    {
        int i_proc;
        int root = 0;
        int count = 2;
        int total_count = 0;
        int *srcounts   = (int*)malloc(sizeof(int) * compute_node_count);
        int *displs     = (int*)malloc(sizeof(int) * compute_node_count);

        // test integer versions of gather functions
        if(myid == root){
            rarray0_i = (int*)malloc(sizeof(int) * compute_node_count * count);
        }
        PRF_GATHER_INT(rarray0_i, sarray_i, count, root);
        if(myid == root){
            int chars = 3;
            char *message = (char*)malloc(sizeof(char) * compute_node_count * count * chars + 1);
            int *expected_rarray = (int*)malloc(sizeof(int) * compute_node_count * count);
            int i_recv;
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = i_proc * count + i_recv;
                    expected_rarray[index] = 3 * (i_proc) + i_recv;
                    sprintf(&message[index*chars], "%2d ", expected_rarray[index]);
                    if(expected_rarray[index] != rarray0_i[index]){
                        success |= 1;
                    }
                }
            }
            Message("Test PRF_GATHER_INT from proc %d expected [%s]\n", myid, message);
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = i_proc * count + i_recv;
                    sprintf(&message[index*chars], "%2d ", rarray0_i[index]);
                }
            }
            Message("Test PRF_GATHER_INT from proc %d received [%s]\n", myid, message);

            free(message);
            free(expected_rarray);
        }
        free(rarray0_i); rarray0_i = NULL;
        PRF_GSYNC();


        total_count = 0;
        count = 3;
        for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
            srcounts[i_proc] = count;
            displs[i_proc] = 3 * i_proc;
            total_count += srcounts[i_proc];
        }
        if(myid == root){
            rarray0_i = (int*)malloc(sizeof(int) * compute_node_count * 3);
        }
        PRF_GATHERV_INT(rarray0_i, sarray_i, srcounts, srcounts[myid], displs, root);
        if(myid == root){
            int chars = 3;
            char *message = (char*)malloc(sizeof(char) * compute_node_count * count * chars + 1);
            int *expected_rarray = (int*)malloc(sizeof(int) * compute_node_count * count);
            int i_recv;
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = displs[i_proc] + i_recv;
                    expected_rarray[index] = 3 * (i_proc) + i_recv;
                    sprintf(&message[index*chars], "%2d ", expected_rarray[index]);
                    if(expected_rarray[index] != rarray0_i[index]){
                        success |= 2;
                    }
                }
            }
            Message("Test PRF_GATHERV_INT from proc %d expected [%s]\n", myid, message);
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = displs[i_proc] + i_recv;
                    sprintf(&message[index*chars], "%2d ", rarray0_i[index]);
                }
            }
            Message("Test PRF_GATHERV_INT from proc %d received [%s]\n", myid, message);

            free(message);
            free(expected_rarray);
        }
        free(rarray0_i); rarray0_i = NULL;
        PRF_GSYNC();


        // test floating point versions of gather functions
        count = 2;
        total_count = 0;

        if(myid == root){
            rarray0_r = (double*)malloc(sizeof(double) * compute_node_count * count);
        }
        PRF_GATHER_REAL(rarray0_r, sarray_r, count, root);
        if(myid == root){
            int chars = 6;
            char *message = (char*)malloc(sizeof(char) * compute_node_count * count * chars + 1);
            double *expected_rarray = (double*)malloc(sizeof(double) * compute_node_count * count);
            int i_recv;
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = i_proc * count + i_recv;
                    expected_rarray[index] = 3.0 * (i_proc) + i_recv + 0.5;
                    sprintf(&message[index*chars], "%5.2f ", expected_rarray[index]);
                    if(expected_rarray[index] != rarray0_r[index]){
                        success |= 1;
                    }
                }
            }
            Message("Test PRF_GATHER_REAL from proc %d expected [%s]\n", myid, message);
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = i_proc * count + i_recv;
                    sprintf(&message[index*chars], "%5.2f ", rarray0_r[index]);
                }
            }
            Message("Test PRF_GATHER_REAL from proc %d received [%s]\n", myid, message);

            free(message);
            free(expected_rarray);
        }
        free(rarray0_r); rarray0_r = NULL;
        PRF_GSYNC();


        total_count = 0;
        count = 3;
        for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
            srcounts[i_proc] = count;
            displs[i_proc] = 3 * i_proc;
            total_count += srcounts[i_proc];
        }
        if(myid == root){
            rarray0_r = (double*)malloc(sizeof(double) * compute_node_count * 3);
        }
        PRF_GATHERV_REAL(rarray0_r, sarray_r, srcounts, srcounts[myid], displs, root);
        if(myid == root){
            int chars = 6;
            char *message = (char*)malloc(sizeof(char) * compute_node_count * count * chars + 1);
            double *expected_rarray = (double*)malloc(sizeof(double) * compute_node_count * count);
            int i_recv;
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = displs[i_proc] + i_recv;
                    expected_rarray[index] = 3.0 * (i_proc) + i_recv + 0.5;
                    sprintf(&message[index*chars], "%5.2f ", expected_rarray[index]);
                    if(expected_rarray[index] != rarray0_r[index]){
                        success |= 2;
                    }
                }
            }
            Message("Test PRF_GATHERV_REAL from proc %d expected [%s]\n", myid, message);
            for(i_proc = 0; i_proc < compute_node_count; ++i_proc){
                for(i_recv = 0; i_recv < count; ++i_recv){
                    int index = displs[i_proc] + i_recv;
                    sprintf(&message[index*chars], "%5.2f ", rarray0_r[index]);
                }
            }
            Message("Test PRF_GATHERV_REAL from proc %d received [%s]\n", myid, message);

            free(message);
            free(expected_rarray);
        }
        free(rarray0_r); rarray0_r = NULL;
        PRF_GSYNC();


        free(srcounts);
        free(displs);
    }
#endif //  RP_NODE
    return success;
}

#endif // RCFD_LIB_TEST_MODULES

#endif // RCFD_PARALLEL
