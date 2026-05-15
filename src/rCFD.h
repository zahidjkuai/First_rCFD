/******************************************************************************
 * @internal
 * @file        rCFD.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_H
#define RCFD_H


#if defined(RCFD_STANDALONE)

#include <math.h>
#include <stdint.h>

#define RP_NODE 1
#define RP_HOST 1

// Fluent functions
#define Message  printf
#define Message0 if(myid == 0) printf

// compute node-0 to the host (host == node-0 here)
#define node_to_host_int_1(a)
#define node_to_host_real_1(a)

#if defined(RCFD_LIB_MPI)

#include <mpi.h>

#define myid                (rcfd->Parallel->rank)
#define node_zero           (0)
#define compute_node_count  (rcfd->Parallel->size)
#define node_last           (compute_node_count - 1)

#define PRF_CSEND_CHAR(to, buffer, nelem, tag) \
    MPI_Send(buffer, nelem, MPI_CHAR, to, tag, rcfd->Parallel->world)

#define PRF_CRECV_CHAR(from, buffer, nelem, tag) \
    MPI_Recv(buffer, nelem, MPI_CHAR, from, tag, rcfd->Parallel->world, MPI_STATUS_IGNORE)

#define PRF_CSEND_INT(to, buffer, nelem, tag) \
    MPI_Send(buffer, nelem, MPI_INT, to, tag, rcfd->Parallel->world)

#define PRF_CRECV_INT(from, buffer, nelem, tag) \
    MPI_Recv(buffer, nelem, MPI_INT, from, tag, rcfd->Parallel->world, MPI_STATUS_IGNORE)

#define PRF_CSEND_REAL(to, buffer, nelem, tag) \
    MPI_Send(buffer, nelem, MPI_DOUBLE, to, tag, rcfd->Parallel->world)

#define PRF_CRECV_REAL(from, buffer, nelem, tag) \
    MPI_Recv(buffer, nelem, MPI_DOUBLE, from, tag, rcfd->Parallel->world, MPI_STATUS_IGNORE)

#define PRF_ISEND_INT(to, buffer, nelem, tag, request) \
    MPI_Isend(buffer, nelem, MPI_INT, to, tag, rcfd->Parallel->world, request)

#define PRF_IRECV_INT(from, buffer, nelem, tag, request) \
    MPI_Irecv(buffer, nelem, MPI_INT, from, tag, rcfd->Parallel->world, request)

#define PRF_ISEND_REAL(to, buffer, nelem, tag, request) \
    MPI_Isend(buffer, nelem, MPI_DOUBLE, to, tag, rcfd->Parallel->world, request)

#define PRF_IRECV_REAL(from, buffer, nelem, tag, request) \
    MPI_Irecv(buffer, nelem, MPI_DOUBLE, from, tag, rcfd->Parallel->world, request)

#define PRF_GISUM(a,b,c) \
    MPI_Allreduce(MPI_IN_PLACE, a, b, MPI_INT, MPI_SUM, rcfd->Parallel->world);

#define PRF_GRSUM(a,b,c) \
    MPI_Allreduce(MPI_IN_PLACE, a, b, MPI_DOUBLE, MPI_SUM, rcfd->Parallel->world);

#define PRF_GSYNC() \
    MPI_Barrier(rcfd->Parallel->world)

#define PRF_WAIT_ALL(count, array_of_requests) \
    MPI_Waitall(count, array_of_requests, MPI_STATUSES_IGNORE)

int prf_gisum1(void *handle, int a);
int prf_gihigh1(void *handle, int a);
int prf_gilow1(void *handle, int a);
double prf_grsum1(void *handle, double a);
double prf_grhigh1(void *handle, double a);
double prf_grlow1(void *handle, double a);
int64_t prf_gi64sum1(void *handle, int64_t a);
int64_t prf_gi64high1(void *handle, int64_t a);
int64_t prf_gi64low1(void *handle, int64_t a);

#define PRF_GISUM1(a)    prf_gisum1(rcfd, a)
#define PRF_GIHIGH1(a)   prf_gihigh1(rcfd, a)
#define PRF_GILOW1(a)    prf_gilow1(rcfd, a)
#define PRF_GRSUM1(a)    prf_grsum1(rcfd, a)
#define PRF_GRHIGH1(a)   prf_grhigh1(rcfd, a)
#define PRF_GRLOW1(a)    prf_grlow1(rcfd, a)
#define PRF_GI64SUM1(a)  prf_gi64sum1(rcfd, a)
#define PRF_GI64HIGH1(a) prf_gi64high1(rcfd, a)
#define PRF_GI64LOW1(a)  prf_gi64low1(rcfd, a)

#define PRF_GATHER_INT(rarray, sarray, count, root) \
    MPI_Gather(sarray, count, MPI_INT, rarray, count, MPI_INT, root, rcfd->Parallel->world)

#define PRF_GATHER_REAL(rarray, sarray, count, root) \
    MPI_Gather(sarray, count, MPI_DOUBLE, rarray, count, MPI_DOUBLE, root, rcfd->Parallel->world)

#define PRF_GATHERV_CHAR(rarray, sarray, rcounts, scount, displs, root) \
    MPI_Gatherv(sarray, scount, MPI_CHAR, rarray, rcounts, displs, MPI_CHAR, root, rcfd->Parallel->world);

#define PRF_GATHERV_INT(rarray, sarray, rcounts, scount, displs, root) \
    MPI_Gatherv(sarray, scount, MPI_INT, rarray, rcounts, displs, MPI_INT, root, rcfd->Parallel->world);

#define PRF_GATHERV_REAL(rarray, sarray, rcounts, scount, displs, root) \
    MPI_Gatherv(sarray, scount, MPI_DOUBLE, rarray, rcounts, displs, MPI_DOUBLE, root, rcfd->Parallel->world);

#define PRF_BCAST_CHAR(array, count, root) \
    MPI_Bcast(array, count, MPI_CHAR, root, rcfd->Parallel->world)

#define PRF_BCAST_INT(array, count, root) \
    MPI_Bcast(array, count, MPI_INT, root, rcfd->Parallel->world)

#define PRF_BCAST_REAL(array, count, root) \
    MPI_Bcast(array, count, MPI_DOUBLE, root, rcfd->Parallel->world)

#else // RCFD_SERIAL

#define MPI_Comm            int
#define MPI_COMM_WORLD      0
#define MPI_Request         int
#define MPI_REQUEST_NULL    0
#define MPI_Comm_rank(world, rankptr)   if(rankptr) *(rankptr) = myid
#define MPI_Comm_size(world, sizeptr)   if(sizeptr) *(sizeptr) = compute_node_count

#define myid                (0)
#define node_zero           (0)
#define compute_node_count  (1)
#define node_last           (0)

#define PRF_CSEND_CHAR(to, buffer, nelem, tag)
#define PRF_CRECV_CHAR(from, buffer, nelem, tag)
#define PRF_CSEND_INT(to, buffer, nelem, tag)
#define PRF_CRECV_INT(from, buffer, nelem, tag)
#define PRF_CSEND_REAL(to, buffer, nelem, tag)
#define PRF_CRECV_REAL(from, buffer, nelem, tag)
#define PRF_ISEND_INT(to, buffer, nelem, tag, request)
#define PRF_IRECV_INT(from, buffer, nelem, tag, request)
#define PRF_ISEND_REAL(to, buffer, nelem, tag, request)
#define PRF_IRECV_REAL(from, buffer, nelem, tag, request)
#define PRF_GSYNC()
#define PRF_WAIT_ALL(count, array_of_requests)

#define PRF_GISUM1(a)    (a)
#define PRF_GIHIGH1(a)   (a)
#define PRF_GILOW1(a)    (a)
#define PRF_GRSUM1(a)    (a)
#define PRF_GRHIGH1(a)   (a)
#define PRF_GRLOW1(a)    (a)
#define PRF_GI64SUM1(a)  (a)
#define PRF_GI64HIGH1(a) (a)
#define PRF_GI64LOW1(a)  (a)

#define PRF_GISUM(a,b,c) memcpy(c, a, b*sizeof(int))
#define PRF_GRSUM(a,b,c) memcpy(c, a, b*sizeof(double))

#define PRF_GATHER_INT(rarray, sarray, count, root)
#define PRF_GATHER_REAL(rarray, sarray, count, root)
#define PRF_GATHERV_CHAR(rarray, sarray, rcounts, scount, displs, root)
#define PRF_GATHERV_INT(rarray, sarray, rcounts, scount, displs, root)
#define PRF_GATHERV_REAL(rarray, sarray, rcounts, scount, displs, root)
#define PRF_BCAST_CHAR(array, count, root)
#define PRF_BCAST_INT(array, count, root)
#define PRF_BCAST_REAL(array, count, root)

#endif // RCFD_LIB_MPI

#else

#include "udf.h"

#ifndef MPI_Comm
#define MPI_Comm int
#endif
#define MPI_COMM_WORLD                  0
#define MPI_Initialized(flagptr)        if(flagptr) *(flagptr) = 1
#define MPI_Init(x, y)
#define MPI_Comm_rank(world, rankptr)   if(rankptr) *(rankptr) = myid
#define MPI_Comm_size(world, sizeptr)   if(sizeptr) *(sizeptr) = compute_node_count

void prf_gatherv_int (int    *sarray, int scount, int    *rarray, const int recvcounts[], const int displs[], int root);
void prf_gatherv_real(double *sarray, int scount, double *rarray, const int recvcounts[], const int displs[], int root);
void prf_gatherv_char(char   *sarray, int scount, char   *rarray, const int recvcounts[], const int displs[], int root);

#define PRF_GATHER_INT(rarray, sarray, count, root)   prf_gather_int_even(rarray, sarray, count, root)
#define PRF_GATHER_REAL(rarray, sarray, count, root)  prf_gather_real_even(rarray, sarray, count, root)
#define PRF_GATHERV_INT(rarray, sarray, rcounts, scount, displs, root)  prf_gatherv_int(sarray, scount, rarray, rcounts, displs, root)
#define PRF_GATHERV_REAL(rarray, sarray, rcounts, scount, displs, root) prf_gatherv_real(sarray, scount, rarray, rcounts, displs, root)
#define PRF_GATHERV_CHAR(rarray, sarray, rcounts, scount, displs, root) prf_gatherv_char(sarray, scount, rarray, rcounts, displs, root)

#endif

#endif // RCFD_H
