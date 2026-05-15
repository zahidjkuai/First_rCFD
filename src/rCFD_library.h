/******************************************************************************
 * @file        rCFD_library.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_LIBRARY
#define RCFD_LIBRARY

/*
   C-style library interface to rCFD
*/

#if defined(RCFD_LIB_MPI)
#include <mpi.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

const char* rcfd_version(int *major, int *minor, int *patch);

// main flow control methods

/**
 * Library modes
 */
typedef enum {
    RCFD_MODE_PREPARATION,  /**< mode for preparing database */
    RCFD_MODE_RUN           /**< mode for running rCFD simulation */
} rcfd_library_mode;

#if defined(RCFD_LIB_MPI)
void rcfd_open(int argc, char **argv, MPI_Comm comm, void **ptr);
#endif
void rcfd_open_no_mpi(int argc, char **argv, void **ptr);
int  rcfd_register_user_function(void *handle, const char *udf, void (*callback)(void));
void rcfd_init(void *handle, int mode);
int  rcfd_is_initialized(void *handle);
void rcfd_read_db(void *handle);
void rcfd_run(void *handle);
void rcfd_close(void *handle);

/**
 * Log priorities for reporting module in increasing order of severity
 */
typedef enum {
    RCFD_REPORT_TRACE = 0,  /**< most detailed information */
    RCFD_REPORT_DEBUG,      /**< for detailed information on the flow through the system */
    RCFD_REPORT_INFO,       /**< important runtime events (startup, shutdown, ...) */
    RCFD_REPORT_WARNING,    /**< use of deprecated functions, poor use of functions, 'almost' errors,
                             *   other runtime situations that are undesirable or unexpected,
                             *   but not necessarily 'wrong' */
    RCFD_REPORT_ERROR,      /**< runtime errors or unexpected conditions from which the program may recover */
    RCFD_REPORT_FATAL,      /**< severe errors that cause premature program termination */
    RCFD_REPORT_UNKNOWN,    /**< messages of unknown severity */
    RCFD_REPORT_MAX
} rcfd_report_priority;

void rcfd_report0(void* handle, int priority, const char *format, ...);

// setter methods

/**
 * Cell shape types, cf. VTK
 */
typedef enum {
    RCFD_CELL_VERTEX        = 1,
    RCFD_CELL_TRIANGLE      = 5,
    RCFD_CELL_QUADRILATERAL = 9,
    RCFD_CELL_TETRAHEDRON   = 10,
    RCFD_CELL_HEXAHEDRON    = 12,
    RCFD_CELL_WEDGE         = 13,
    RCFD_CELL_PYRAMID       = 14,
    RCFD_CELL_POLYHEDRON    = 42
} rcfd_cell_type;

#define RCFD_MAX_NODES_PER_CELL         8
#define RCFD_MAX_FACES_PER_POLY_CELL    256

int rcfd_set_cell_shape(void *handle, int i_layer, int i_cell, int cell_type, const int *cell_node_ids, const int *poly_face_nodes);

/**
 * Tracer guiding formats for tracer-based cell-to-cell shift generation
 */
typedef enum {
    RCFD_TRACER_FORCE_GUIDED,       /**< use drag force to advance tracer particle in time */
    RCFD_TRACER_VELOCITY_GUIDED,    /**< use fluid velocity to advance tracer particle in time */
    RCFD_TRACER_MAX
} rcfd_tracer_format;

/**
 * Local diffusion formats
 */
typedef enum {
    RCFD_LOCAL_DIFF_VISCOSITY_BASED,    /**< local diffusion based on viscosity */
    RCFD_LOCAL_DIFF_FIELD_BASED,        /**< local diffusion based on field */
    RCFD_LOCAL_DIFF_MAX
} rcfd_local_diffusion_format;

/**
 * Recurrence norm format
 */
typedef enum {
    RCFD_NORM_DEFAULT,  /**< velocity-based norm in single-phase flows, vof-based norm in multi-phase flows */
    RCFD_NORM_USER,     /**< invoke user function to calculate custom norm */
    RCFD_NORM_MAX
} rcfd_norm_format;

/**
 * Recurrence path formats for navigating through the database
 */
typedef enum {
    RCFD_REC_QUARTER_JUMPS,     /**< jump to the next quarter of frames of the recurrence matrix to search for the next sequence start */
    RCFD_REC_OFF_DIAGONAL_BAND, /**< use an off-diagonal band of the recurrence matrix to search for the next sequence start */
    RCFD_REC_REPLAY,            /**< replay the frames in the same order as recorded */
    RCFD_REC_MAX
} rcfd_rec_format;

/**
 * Transported data types
 */
typedef enum {
    RCFD_DATA_GENERIC,          /**< generic data */
    RCFD_DATA_CONCENTRATION,    /**< concentration data */
    RCFD_DATA_BINARY,           /**< binary data */
    RCFD_DATA_TEMPERATURE,      /**< temperature data */
    RCFD_DATA_MAX
} rcfd_data_type;

/**
 * Balancing modes for mass conservation
 */
typedef enum {
    RCFD_BALANCE_OFF,       /**< no balancing */
    RCFD_BALANCE_GLOBAL,    /**< global balancing */
    RCFD_BALANCE_LOCAL,     /**< process-local (i.e. per-node) balancing */
    RCFD_BALANCE_MAX
} rcfd_balance_mode;

/**
 * Dump file types
 */
typedef enum {
    RCFD_FILE_ASCII,    /**< ascii file type */
    RCFD_FILE_BINARY,   /**< binary file type */
    RCFD_FILE_MAX
} rcfd_file_type;

// for _Dict_type
// layer, phase, data parameters are ignored if not applicable
int rcfd_set_dict_property_i    (void *handle, const char *dict, int layer, int phase, int data, const char *key, int value);
int rcfd_set_dict_property_r    (void *handle, const char *dict, int layer, int phase, int data, const char *key, double value);
int rcfd_set_dict_property_r_3d (void *handle, const char *dict, int layer, int phase, int data, const char *key, double *value);
int rcfd_set_dict_property_s    (void *handle, const char *dict, int layer, int phase, int data, const char *key, const char *value);

// preparation methods
int rcfd_set_displacement_fields(void *handle, int i_layer, int i_phase, double *cfd_displ, double *cfd_displ_diff);
int rcfd_create_shifts_from_displacement_fields(void *handle);
int rcfd_interpolate_shifts(void *handle);
int rcfd_create_shifts_from_interfaces(void *handle, double *interface_face_centers, int interface_face_count);
int rcfd_set_vof(void * handle, int i_layer, double** cell_vof);
int rcfd_write_shifts(void *handle);
int rcfd_write_recurrence(void *handle);
int rcfd_set_fields(void *handle, double *norm, double **vof, double **local_diffusion);
int rcfd_write_fields(void *handle);
int rcfd_write_topo(void *handle);

// getter methods
// from Solver_Dict_type
double  rcfd_get_global_timestep(void *handle);
double  rcfd_get_vof_cutoff_limit(void *handle);
int     rcfd_get_frame_count    (void *handle);
int     rcfd_get_phase_count    (void *handle);
// for preparation mode
int     rcfd_is_vof_on(void *handle);
int     rcfd_is_shifts_from_displacement_fields_on(void *handle);
int     rcfd_is_moving_geometry_on(void *handle);
int     rcfd_is_local_diffusion_on(void *handle);
int     rcfd_get_local_diffusion_format(void *handle);
int     rcfd_get_monitoring_interval(void *handle);

// from Cell_Dict_type
int     rcfd_get_cell_count     (void *handle, int i_layer);
int     rcfd_get_int_cell_count (void *handle, int i_layer);
int     rcfd_get_cell_user_count(void *handle, int i_layer);
int     rcfd_get_cell_rec_user_count(void *handle, int i_layer);

// from Face_Dict_type
int     rcfd_get_face_count     (void *handle, int i_layer);
int     rcfd_get_int_face_count (void *handle, int i_layer);

// from Phase_Dict_type
int     rcfd_get_phase_data_count   (void *handle, int i_phase);
double  rcfd_get_phase_density      (void *handle, int i_phase);
double  rcfd_get_phase_heat_capacity(void *handle, int i_phase);
double  rcfd_get_phase_reference_temperature(void *handle, int i_phase);
int     rcfd_get_phase_user_count   (void *handle, int i_phase);

// from Solver_type
int     rcfd_get_current_state  (void *handle);
int     rcfd_get_next_state     (void *handle);
int     rcfd_get_current_layer  (void *handle);
int     rcfd_get_current_frame  (void *handle);
int     rcfd_get_run_counter    (void *handle);
double  rcfd_get_time           (void *handle);
double  rcfd_get_timestep       (void *handle, int i_layer);
void    rcfd_map_layer          (void *handle, int from, int to);

// access methods

// direct access to Cell_type arrays
int*     rcfd_get_cell_host         (void *handle, int i_layer); // [i_cell]
double** rcfd_get_cell_center       (void *handle, int i_layer); // [i_cell][i_dim]
double*  rcfd_get_cell_volume       (void *handle, int i_layer); // [i_cell]
double** rcfd_get_cell_data         (void *handle, int i_layer, int i_phase); // [i_cell][i_data]
double** rcfd_get_cell_vof          (void *handle, int i_layer, int i_frame); // [i_cell][i_phase]
double** rcfd_get_cell_user         (void *handle, int i_layer);              // [i_cell][i_user]
double** rcfd_get_cell_rec_user     (void *handle, int i_layer, int i_frame); // [i_cell][i_rec_user]
double   rcfd_get_probe_value       (void *handle, int i_probe);
double   rcfd_cell2point_distance   (void *handle, int i_layer, int cell_id, double *point_x);
// for preparation mode
double** rcfd_get_cell_cfd_displ_diff   (void *handle, int i_layer, int i_phase); // [i_cell][i_dim]
double* rcfd_get_cell_local_diffusion   (void *handle, int i_layer, int i_phase); // [i_cell]
double* rcfd_get_cell_crossing_time (void *handle, int i_layer, int i_phase); // [i_cell]
double* rcfd_get_cell_ave_velocity  (void *handle, int i_layer, int i_phase); // [i_cell]

// direct access to Face_type arrays
int*     rcfd_get_face_c0   (void *handle, int i_layer); // [i_face]
int*     rcfd_get_face_c1   (void *handle, int i_layer); // [i_face]
double** rcfd_get_face_area (void *handle, int i_layer); // [i_face][i_dim]

// direct access to Node_type arrays
double** rcfd_get_node      (void *handle, int i_layer); // [i_node][i_dim]

// direct access to Phase_Dict_type arrays
double* rcfd_get_phase_user (void *handle, int i_phase);

// direct access to Rec_type arrays
int  rcfd_get_rec_jumped_at_last_frame  (void *handle);
int  rcfd_get_rec_sequence_length       (void *handle);

// indirect access to Balance_type members
void rcfd_set_balance_mass_source_one (void *handle, int i_phase, int i_data, double ms);
void rcfd_set_balance_mass_source_all (void *handle, int i_phase, double ms);
void rcfd_set_balance_mass_source_each(void *handle, int i_phase, double *ms);

void rcfd_modify_balance_mass_source_one (void *handle, int i_phase, int i_data, double dms);
void rcfd_modify_balance_mass_source_all (void *handle, int i_phase, double dms);
void rcfd_modify_balance_mass_source_each(void *handle, int i_phase, double *dms);

void rcfd_set_balance_mass_integral(void *handle, int i_phase, int i_data, double mi);
void rcfd_set_balance_mass_integral_target(void *handle, int i_phase, int i_data, double mit);
void rcfd_set_balance_mass_integral_global(void *handle, int i_phase, int i_data, double mig);
void rcfd_set_balance_mass_integral_target_global(void *handle, int i_phase, int i_data, double mitg);

double rcfd_get_balance_mass_integral(void *handle, int i_phase, int i_data);
double rcfd_get_balance_mass_integral_target(void *handle, int i_phase, int i_data);
double rcfd_get_balance_mass_integral_global(void *handle, int i_phase, int i_data);
double rcfd_get_balance_mass_integral_target_global(void *handle, int i_phase, int i_data);

// direct access to Norm_Dict_type/Norm_type members
// for preparation mode
int rcfd_get_norm_coarsening(void *handle);
int rcfd_get_norm_format(void *handle);
int rcfd_get_norm_count(void *handle);
double *rcfd_get_norm(void *handle);

#if defined(RCFD_LIB_TEST_MODULES)
int rcfd_test_module(void *handle, const char *module);
#endif

#ifdef __cplusplus
}
#endif

#endif // RCFD_LIBRARY
