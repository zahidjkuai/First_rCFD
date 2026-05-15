/******************************************************************************
 * @internal
 * @file        rCFD_macros.h
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_MACROS
#define RCFD_MACROS

#include "rCFD.h"


#if 1   /* global definitions */

    #define     STR_HELPER(x) #x
    #define     STR(x) STR_HELPER(x)

    #define     RCFD_VERSION_MAJOR  26
    #define     RCFD_VERSION_MINOR  03
    #define     RCFD_VERSION_PATCH  0
    #define     RCFD_VERSION_STRING "rCFD " STR(RCFD_VERSION_MAJOR) "." STR(RCFD_VERSION_MINOR) "." STR(RCFD_VERSION_PATCH)

    /*
     * Note: File version numbers describe reader compatibility.
     * if an older reader cannot correctly read newer formats (breaks backward compatibility)
     *  -> increase major version
     * if an older reader can safely interpret newer formats (e.g. by ignore additional content)
     *  -> increase minor version
     */
    #define     RCFD_METADATA_FILE_VERSION_MAJOR    1
    #define     RCFD_METADATA_FILE_VERSION_MINOR    0

    #define     RCFD_C2C_FILE_VERSION_MAJOR     1
    #define     RCFD_C2C_FILE_VERSION_MINOR     0

    #define     CLOCKTIME                       clock
    #define     REPORT_BUFFER_SIZE              1024
    #define     RCFD_FILENAME_MAX               1024
    #define     RCFD_DEFAULT_CONFIG_FILENAME    "config.toml"
    #define     READ_BUFFER_SIZE                256

    // NOTE: implementation of multiple cell layers is currently incomplete
    #define     MULTI_LAYER_IMPLEMENTATION_COMPLETE 0
    #define     VOF_STITCHING_IMPLEMENTATION_COMPLETE 0

#endif

#if 1   /* indices */

    #define     _CELL_DICT(i_layer)             rcfd->Topo_Dict->Cell_Dict[i_layer]
    #define     _FACE_DICT(i_layer)             rcfd->Topo_Dict->Face_Dict[i_layer]
    #define     _NODE_DICT(i_layer)             rcfd->Topo_Dict->Node_Dict[i_layer]

    #define     _C(i_layer)                     rcfd->Topo->Cell[i_layer]
    #define     _F(i_layer)                     rcfd->Topo->Face[i_layer]
    #define     _N(i_layer)                     rcfd->Topo->Node[i_layer]

    #define     _MPI_Cell                       rcfd->Parallel->MPI_Topo->MPI_Cell[i_layer]

#endif

#if 1   /* macros for rCFD_prep (mostly Fluent - specific) */

    /* rCFD_write_Tracer_Positions */

    #define     NO_ROI_DEFINED                  (rcfd->Prep_Dict->tracer_region_of_interest_exists == 0)

    #define     X_IN_ROI                        ((x[0] > rcfd->Prep_Dict->tracer_ROI_x_min) && (x[0] < rcfd->Prep_Dict->tracer_ROI_x_max))
    #define     Y_IN_ROI                        ((x[1] > rcfd->Prep_Dict->tracer_ROI_y_min) && (x[1] < rcfd->Prep_Dict->tracer_ROI_y_max))
    #define     Z_IN_ROI                        ((x[2] > rcfd->Prep_Dict->tracer_ROI_z_min) && (x[2] < rcfd->Prep_Dict->tracer_ROI_z_max))

    #define     CELL_IN_ROI                     ((X_IN_ROI) && (Y_IN_ROI) && (Z_IN_ROI))

    /* rCFD_update_Tracers */

    #define     TRACER_DATABASE_FULL                (rcfd->Solver->current_frame >= rcfd->Solver_Dict->number_of_frames)
    #define     TRACER_DATABASE_NOT_FULL            !(TRACER_DATABASE_FULL)
    #define     EXCESS_TRACER_CELL_CROSSING_TIME    (_C(i_layer).crossing_time[i_phase][i_cell] > (2. * rcfd->Solver_Dict->global_time_step))
    #define     TRACER_HAS_CROSSED_CELL_BORDER      ((int)p->user[p_c_old] != i_cell)
    #define     TRACER_FROM_SLOW_CELL               (p->user[p_time_ratio] > 1.0)
    #define     TRACER_NOT_STORED_YET               (p->user[p_invalid] < 1.0)
    #define     TRACER_LIFETIME_EXPIRED             ((CURRENT_TIME - p->user[p_start_time]) >= rcfd->Solver_Dict->global_time_step)

#endif

    #define     REGISTER_RCFD_UDF_DEFAULT(funcName) rcfd->rCFD_UDF->_##funcName = funcName##_default;

    #define     WRITE_FILE_DICT         (1<<0)
    #define     WRITE_SOLVER_DICT       (1<<1)
    #define     WRITE_PHASE_DICT        (1<<2)
    #define     WRITE_PREP_DICT         (1<<3)
    #define     WRITE_NORM_DICT         (1<<4)
    #define     WRITE_REC_DICT          (1<<5)
    #define     WRITE_DATA_DICT         (1<<6)
    #define     WRITE_BALANCE_DICT      (1<<7)
    #define     WRITE_CELL_DICT         (1<<8)
    #define     WRITE_FACE_DICT         (1<<9)
    #define     WRITE_NODE_DICT         (1<<10)
    #define     WRITE_TOPO_DICT         (1<<11)
    #define     WRITE_DUMP_DICT         (1<<12)

#endif
