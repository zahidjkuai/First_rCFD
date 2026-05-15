/******************************************************************************
 * @internal
 * @file        rCFD_types.h
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_TYPES
#define RCFD_TYPES

#include "rCFD.h"
#include <time.h>

/**
 * To modify [Solver_Dict](@ref Solver_Dict_type) options register the user-
 * defined function `"user_set_solver_dict"` (cf. [Registering User-Defined
 * Functions](userdoc.html#register_user_defined_functions)) via the
 * `rcfd_register_user_function()` function.
 * Within that user-defined function call one of the `rcfd_set_dict_property_*`
 * functions, i.e.:
 * - `rcfd_set_dict_property_i()`
 * - `rcfd_set_dict_property_r()`
 * - `rcfd_set_dict_property_s()`
 *
 * to modify the desired entry.
 * Alternatively, Solver_Dict options may be specified in the TOML configuration
 * file using a table with the header `[solver]`.
 * Possible options are:
 */
typedef struct Solver_Dict_struct
{
    /* reporting options */

    short   report_priority;        /**< [report priority](@ref rcfd_report_priority)
                                         of log output; only messages with equal
                                         or higher priority get logged */
    short   profiling_on;           /**< switch on/off timing: `0` (off), `1` (on) */

    short   multifile_output_on;    /**< flag indicating on/off status of multifile output */

    short   write_config_to_file_on;    /**< write all configuration settings to
                                             config_out.toml after initialization */

    /* main characteristics */

    short   mode;

    short   number_of_frames;   /**< number of database entries; must be > 0 */
    short   number_of_states;   /**< number of flow states; must be > 0 */
    short   number_of_phases;   /**< number of phases; must be > 0 */
    short   number_of_layers;   /**< number of grid resolution layers; must be > 0 */
    short   number_of_runs;     /**< number of time steps to execute in one call to `rcfd_run()` */

    /* sub-model flags */

    short   recurrence_process_on;  /**< switch on/off frame sequences:
                                         `0` (off, stay at frame 0), `1` (on) */
    short   data_convection_on;     /**< switch on/off cell shift operation:
                                         `0` (off), `1` (on) */
    short   face_swap_diffusion_on; /**< switch on/off face swap operation:
                                         `0` (off), `1` (on) */
    short   local_diff_on;          /**< switch between global/local diffusion:
                                         `0` (global), `1` (local) */
    short   parallel_diffusion_on;  /**< switch on/off diffusion across process
                                         boundaries: `0` (off), `1` (on) */
    short   data_binarization_on;   /**< switch on/off binarization of binary
                                         data after diffusion operation:
                                         `0` (off), `1` (on) */
    short   data_drifting_on;       /**< switch on/off drifting of generic and
                                         concentration data after diffusion
                                         operation: `0` (off), `1` (on) */
    short   balance_correction_on;  /**< currently unused (always on) */
    short   interpolation_on;       /**< switch on/off interpolation of
                                         cell-to-cell shifts (only used in
                                         preparation mode): `0` (off), `1` (on) */
    short   shifts_from_displacement_fields_on; /**< switch on/off cell-to-cell
                                                     shift generation via displacement
                                                     fields (instead of tracers;
                                                     only used in preparation mode):
                                                     `0` (off), `1` (on) */
    short   moving_geometry_on;         /**< switch on/off moving geometries:
                                             `0` (off), `1` (on) */
    short   grid_interface_shifts_on;   /**< switch on/off cell-to-cell shift generation
                                             at grid interfaces: `0` (off), `1` (on) */

    /* sub-model params */

    short   max_number_of_cells_per_time_step;  /**< only used in (ANSYS Fluent)
                                                     preparation mode */

    int     time_steps_per_monitoring_interval; /**< number of CFD time steps
                                                     giving one rCFD time step;
                                                     only used in preparation mode */
    double  start_time_for_monitoring;          /**< only used in (ANSYS Fluent)
                                                     preparation mode */

    short   single_phase_vof;       /**< switch to indicate single-phase-VOF
                                         (tow-fluid model) simulation:
                                         `0` (off), `1` (on) */
    short   single_phase_vof_id;    /**< ID of VOF phase in a single-phase-VOF
                                         simulation; only used in (ANSYS Fluent)
                                         preparation mode */
    double  vof_cutoff_limit;       /**< volume fractions below the cutoff limit are ignored */

    /* cell layers */

    int     min_number_of_cells_per_layer;  /**< minimum number of cells in coarser cell layers */
    short   max_number_of_faces_per_cell;   /**< maximum number of cell faces in coarser cell layers */
    short   max_number_of_children;         /**< maximum number of child cells in coarser cell layers */
    short   number_redist_loops_per_layer;  /**< maximum number of iterations for optimizing inter-layer cell mapping */

    double  global_time_step;       /**< rCFD time step size in seconds */

    short   max_fill_loops;         /**< maximum number of loops to fill holes
                                         in imperfect cell-to-cell shift patterns
                                         via diffusion */

    double  face_swap_max_per_loop; /**< maximum fraction of data to exchange
                                         during face swap operation */
    short   face_swap_max_loops;    /**< maximum number of loops for face swap operations */
    short   local_diff_format;      /**< [local diffusion format](@ref rcfd_local_diffusion_format):
                                         use either the viscosity of the carrier fluid or the
                                         diffusion of the transported scalar */
    double  grid_interface_shift_weight; /**< weight associated to shifts at grid interfaces: range ]0..1] */

    short   number_of_drift_loops;  /**< number of loops for applying data drifting */

    short   balance_correction_update;  /**< every how many time steps balance corrections are applied */
    short   control_conc_sum_on;    /**< adjust concentration data such that their sum is one:
                                         `0` (off), `1` (on) */

    short   max_phase_data_count;  // set by init_phase_dict
} Solver_Dict_type;

/**
 * To modify [File_Dict](@ref File_Dict_type) options register the user-defined
 * function`"user_set_file_dict"` (cf. [Registering User-Defined Functions]
 * (userdoc.html#register_user_defined_functions)) via the
 * `rcfd_register_user_function()` function and use the
 * `rcfd_set_dict_property_s()` function to modify the desired entry.
 * Alternatively, File_Dict options may be specified in the TOML configuration
 * file using a table with the header `[files]`.
 * Possible options are:
 */
typedef struct File_Dict_struct
{
    /* directory names */
    char *working_dir;          /**< path to rCFD working directory */
    char *data_dir;             /**< data directory inside `working_dir` */
    char *tmp_sub_dir;          /**< folder for temporary files inside `data_dir` directory */
    char *vof_sub_dir;          /**< folder for vof files inside `data_dir` directory */
    char *topo_sub_dir;         /**< folder for topology files inside `data_dir` directory */
    char *local_diff_sub_dir;   /**< folder for local diffusion files inside `data_dir` directory */
    char *c2c_sub_dir;          /**< folder for cell-to-cell shift database files inside `data_dir` directory */
    char *rec_sub_dir;          /**< folder for recurrence property files inside `data_dir` directory */
    char *user_sub_dir;         /**< folder for user-defined recurrence data inside `data_dir` directory */
    char *post_dir;             /**< post folder inside `working_dir` */

    /* filenames */
    char *metadata_filename;                /**< file containing description of database */
    char *tracer_start_position_filename;   /**< file containing start positions of tracers; only used in (ANSYS Fluent) preparation mode */
    char *Prep_Transscript_filename;        /**< report filename; only used in preparation mode  */
    char *Run_Transscript_filename;         /**< report filename; only used in run mode */
    char *C2C_filename;         /**< base name; state, phase and partition IDs are appended  */
    char *Norm_filename;        /**< base name; state and partition IDs are appended; only used in preparation mode */
    char *vof_filename;         /**< base name; state, phase and partition IDs are appended */
    char *local_diff_filename;  /**< base name; state, phase and partition IDs are appended */
    char *rec_user_filename;    /**< base name; state and partition IDs are appended */
    char *Jump_filename;        /**< base name; states and island IDs are appended */
    char *Matrix_filename;      /**< base name; states and island IDs are appended; only used in preparation mode */
    char *Balance_filename;     /**< only used in run mode if `write_balance_to_file` is set in `Balance_Dict` */
    char *Rec_Frames_filename;  /**< only used in run mode if `monitor_rec_frames_on` is set in `Rec_Dict` */
    char *Topo_filename;        /**< base name; partition IDs are appended */
    char *Dump_filename;        /**< base name; partition IDs (in multifile mode) and timestep are appended */
    char *Probe_filename;       /**< only used if any probes are defined  */

} File_Dict_type;

/**
 * Each phase in an rCFD simulation can be characterized by the options defined
 * in the [Phase_Dict](@ref Phase_Dict_type).
 * To modify Phase_Dict options register the user-defined function
 * `"user_set_phase_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function.
 * Within that user-defined function call one of the `rcfd_set_dict_property_*`
 * functions, i.e.:
 * - `rcfd_set_dict_property_i()`
 * - `rcfd_set_dict_property_r()`
 *
 * to modify the desired entry.
 * Note that each phase has its own properties and thus the `i_phase`
 * parameter must be specified properly.
 * Alternatively, Phase_Dict options may be specified in the TOML configuration
 * file using an array of tables with the header `[[phases]]`.
 * Possible options are:
 */
typedef struct Phase_Dict_struct
{
    int     number_of_data;         /**< the number of data transported in this phase;
                                         each data is describe by a corresponding
                                         [Data_Dict](#data_dict) */

    int     number_of_concentration_data;

    double  time_step;              /**< phase-specific time step; only used in
                                         (ANSYS Fluent) preparation mode */

    double  density;                /**< phase density */

    double  heat_capacity;          /**< heat capacity of phase */

    double  reference_temperature;  /**< zero offset of temperature data */

    double  vof_max;                /**< maximum volume fraction of phase;
                                         e.g. used to calculate hindering factor */

    short   hindered_drift_on;      /**< switch on/off drift hindrance
                                         (based on volume fraction):
                                         `0` (off), `1` (on) */

    short   local_drift_on;         /**< switch on/off drift adaption based on
                                         local concentration (concentration data only):
                                         `0` (off), `1` (on) */

    short   drift_control_conc_sum_on;  /**< switch on/off conservation of concentration
                                             upon drift (concentration data only):
                                             `0` (off), `1` (on) */

    double  d32_diam;               /**< Sauter mean diameter of bulk used in local drift adaption */

    int     number_of_user_vars;    /**< number of user-defined variables related to a specific phase */

    double  *user;

} Phase_Dict_type;

/**
 * Cell-to-cell shift interpolation formats
 */
typedef enum {
    RCFD_INTERP_NEAREST_ALIGNED_NEIGHBOR_CELLS,
    RCFD_INTERP_MAX
} rcfd_interpolation_format;

/**
 * Tracers can be used as an alternative to displacements fields in the (ANSYS
 * Fluent) preparation mode to obtain shift patterns from the fluid flow.
 * Interpolation may be used to enhance cell-to-cell shift patterns in
 * preparation mode.
 * The [Prep_Dict](@ref Prep_Dict_type) allows to adjust certain aspects of
 * the tracking procedure and the creation of additional cell-to-cell shifts.
 * To modify these options register the user-defined function
 * `"user_set_calibration_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function.
 * Within that user-defined function call one of the `rcfd_set_dict_property_*`
 * functions, i.e.:
 * - `rcfd_set_dict_property_i()`
 * - `rcfd_set_dict_property_r()`
 * - `rcfd_set_dict_property_s()`
 *
 * to modify the desired entry.
 * Alternatively, Prep_Dict options may be specified in the TOML configuration
 * file using a table with the header `[prep]`.
 * Possible options are:
 */
typedef struct Prep_Dict_struct
{
    // start tracers
    int     tracer_format;              /**< [tracer guiding format](@ref rcfd_tracer_format)
                                             for tracer-based cell-to-cell shift generation;
                                             determines how tracers are advanced in time */
    int     tracers_per_cell_count;     /**< number of initial tracer positions per cell */
    int     tracer_coarse_graining;     /**< consider every n-th tracer only */

    short   tracer_reverse_direction_on;/**< switch on/off reversing of tracer direction:
                                             `0` off (forward), `1` on (backward) */
    short   tracer_splitting_on;        /**< switch on/off splitting of tracers in slow
                                             transport cells: `0` off, `1` on */

    short   tracer_region_of_interest_exists;   /**< switch on/off region of interest:
                                                     `0` off, `1` on */
    double  tracer_ROI_x_min;      /**< minimum x-coordinate of region of interest */
    double  tracer_ROI_x_max;      /**< maximum x-coordinate of region of interest */
    double  tracer_ROI_y_min;      /**< minimum y-coordinate of region of interest */
    double  tracer_ROI_y_max;      /**< maximum y-coordinate of region of interest */
    double  tracer_ROI_z_min;      /**< minimum z-coordinate of region of interest */
    double  tracer_ROI_z_max;      /**< maximum z-coordinate of region of interest */

    short   *tracer_random_walk_on; /**< switch on/off (phase-specific!) random
                                         component of tracer propagation:
                                         `0` (off), `1` (on) */ /* [i_phase] */
    double  *tracer_random_walk_velocity_ratio; /**< fraction of tracer velocity to be used
                                                     for random walk velocity (phase-specific!) */

    // end tracers

    // start interpolation

    short   interpolation_format; /**< `0` (interpolate nearest aligned neighbor cells) */

    short   interpolation_max_number_of_aligned_neigh_cells;    /**< maximum number of neighbor cells to consider for interpolation */

    short   interpolation_pruning_on;               /**< switch on/off pruning of negligible cell-to-cell shifts:
                                                         `0` off, `1` on. If turned on all shifts with a weight
                                                        below a certain pruning weight will be deleted. */
    short   interpolation_min_shifts_after_pruning; /**< minimal number of cell-to-cell shifts
                                                         to each destination cell after pruning. */
    double  interpolation_pruning_weight;           /**< maximal weight of pruned shifts (weights of
                                                         shifts to a specific cell sum up to `1.0`) */

    short   interpolation_multiply_shift_weights_with_vof_on;   /**< switch on/off whether to adapt shift weights
                                                                     with the vof of the originating cell:
                                                                     `0` off, `1` on */
    // end interpolation

} Prep_Dict_type;

/**
 * The recurrence norm determining the similarity of two frames is calculated in
 * the preparation mode.
 * Certain aspects of this calculation can be defined in the [Norm_Dict]
 * (@ref Norm_Dict_type).
 * To modify Norm_Dict options register the user-defined function
 * `"user_set_norm_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function and use the `rcfd_set_dict_property_i()` function to modify the
 * desired entry.
 * Alternatively, Norm_Dict options may be specified in the TOML configuration
 * file using a table with the header `[norm]`.
 * Possible options are:
 */
typedef struct Norm_Dict_struct
{
    short   format;                     /**< [recurrence norm format](@ref rcfd_norm_format) */

    int     coarse_graining;            /**< consider every n-th cell only for the
                                             calculation of the recurrence norm */

    short   larger_than_mean_norm_only; /**< for the similarity of two frames consider
                                             all cell values (`0`) or only cell values,
                                             the difference of which between two frames
                                             is larger than the mean cell value (`1`) */

} Norm_Dict_type;

/**
 * The way rCFD eventually navigates through the recurrence matrix is largely
 * determined by the jump vector files.
 * What these files contain and how individual frames are finally connected to
 * form a time series is defined by the [Rec_Dict](@ref Rec_Dict_type) settings.
 * To modify Rec_Dict options register the user-defined function
 * `"user_set_rec_dict"`(cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function.
 * Within that user-defined function call one of the `rcfd_set_dict_property_*`
 * functions, i.e.:
 * - `rcfd_set_dict_property_i()`
 * - `rcfd_set_dict_property_s()`
 *
 * to modify the desired entry.
 * Alternatively, Rec_Dict options may be specified in the TOML configuration
 * file using a table with the header `[rec]`.
 * Possible options are:
 */
typedef struct Rec_Dict_struct
{
    short   format;                 /**< [recurrence path format](@ref rcfd_rec_format) determines
                                         how frame sequences are selected from the database */

    short   monitor_rec_frames_on;  /**< switch on/off logging the sequence of frames: `0` off, `1` on */

    short   adapt_vof_stitching_on; /**< on/off switch; on if multiple phases or single_phase_vof is set in Phase_Dict */

    short   min_seq_length;         /**< minimum length of frame sequence */
    short   max_seq_length;         /**< maximum length of frame sequence */

    short   off_diagonal_band_width;/**< band width in case of off diagonal band format */

    short   number_of_adapt_vof_loops;  /**< if `adapt_vof_stitching_on` is on, defines the maximum
                                             number of iterations when adapting data to a new vof field */

    int     seed;                   /**< value to initialize pseudo-random number generator (must be >= 0 or -1 if current
                                         time should be used for initialization) */

} Rec_Dict_type;

/**
 * The [Data_Dict](@ref Data_Dict_type) can be used to define the details of a
 * single datum in a specific phase.
 * To modify Data_Dict options register the user-defined function
 * `"user_set_data_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function.
 * Within that user-defined function call one of the `rcfd_set_dict_property_*`
 * functions, i.e.:
 * - `rcfd_set_dict_property_i()`
 * - `rcfd_set_dict_property_r()`
 * - `rcfd_set_dict_property_r_3d()`
 * - `rcfd_set_dict_property_s()`
 *
 * to modify the desired entry.
 * Note that data is phase-specific and thus the `i_phase` parameter must be
 * specified properly.
 * Furthermore, since a phase can host multiple data the `i_data` index must be
 * given.
 * Finally, global balancing for `generic`, `concentration` and `temperature`
 * data is done by summation over `cell phase volume`, `cell mass` and
 * `cell heat content`.
 * Alternatively, Data_Dict options may be specified in the TOML configuration
 * file using an array of tables with the header `[[data]]`, where each table
 * in the array refers to a phase.
 * All data items of a phase are specified via an array of sub-tables with the
 * header `[[data.items]]`.
 * Possible options are:
 */
typedef struct Data_Dict_struct
{
    short   type;                   /**< [type of transported data](@ref rcfd_data_type) */

    double  face_swap_diffusion;    /**< amount of data exchange between cells during a
                                         face swap operation (only for global diffusion,
                                         ignored if local diffusion is on) */

    double  binarization_art_diff;  /**< amount of artificial diffusion for binary data transport  */

    short   drifting_data;          /**< switch on/off applying drift velocity: `0` off, `1` on */

    double* drift_velocity;         /**< additional velocity applied to transport of data */

    double  d1_diam;                /**< diameter used for calculating local Sauter mean diameter */
    double  d2_diam;
    double  d3_diam;

    int     number_of_user_vars;    /**< number of user-defined variables related to a single datum of a specific phase */
    double* user;

} Data_Dict_type;


/**
 * To modify [Balance_Dict](@ref Balance_Dict_type) options register the
 * user-defined function `"user_set_balance_dict"` (cf.
 * [Registering User-Defined Functions](#register_user_defined_functions)) via
 * the `rcfd_register_user_function()` function.
 * Within that user-defined function call one of the `rcfd_set_dict_property_*`
 * functions, i.e.:
 * - `rcfd_set_dict_property_i()`
 * - `rcfd_set_dict_property_r()`
 * - `rcfd_set_dict_property_s()`
 *
 * to modify the desired entry.
 * Balance options are phase- and data-specific.
 * Alternatively, Balance_Dict options may be specified in the TOML configuration
 * file using an array of tables with the header `[[balances]]`, where each table
 * in the array refers to a phase.
 * All options for data items of a phase are specified via an array of sub-tables
 * with the header `[[balances.items]]`.
 * Possible options are:
 */
typedef struct Balance_Dict_struct
{
    short       type;                   /**< [balancing type](@ref rcfd_balance_mode) */
    short       max_correction_loops;   /**< maximum number of iterations for balance correction */
    double      accuracy_level;         /**< accepted deviation from target value */
    short       write_balance_to_file;  /**< switch on/off writing balance file: `0` off, `1` on */
    int         write_balance_to_file_interval; /**< every how many time steps to write to balance file */

} Balance_Dict_type;

/**
 * To modify [Cell_Dict](@ref Cell_Dict_type) options register the user-defined
 * function `"user_set_cell_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function and use the `rcfd_set_dict_property_i()` function to modify the
 * desired entry.
 * Alternatively, Cell_Dict options may be specified in the TOML configuration
 * file using a table with the header `[cells]`.
 * Note that the number of int/ext cells must be given as an array containing a
 * value for each compute node.
 * Possible options are:
 */
typedef struct Cell_Dict_struct
{
    int     number_of_cells;        /**< sum of internal and external cells */
    int     number_of_int_cells;    /**< number of cells hosted on partition */
    int     number_of_ext_cells;    /**< number of boundary cells hosted by neighboring partitions */
    int     number_of_user_vars;    /**< number of user-defined cell-variables */
    int     number_of_rec_user_vars;/**< number of recurrence-specific user-defined cell-variables */
    double  next_neigh_cell_skin_factor; /**< multiplication factor for neighbor cell cutoff distance */

} Cell_Dict_type;

/**
 * To modify [Face_Dict](@ref Face_Dict_type) options register the user-defined
 * function `"user_set_face_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function and use the `rcfd_set_dict_property_i()` function to modify the
 * desired entry.
 * Alternatively, Face_Dict options may be specified in the TOML configuration
 * file using a table with the header `[faces]`.
 * Note that the number of int/ext faces must be given as an array containing a
 * value for each compute node.
 * Possible options are:
 */
typedef struct Face_Dict_struct
{
    int number_of_int_faces;    /**< number of faces between internal cells */
    int number_of_ext_faces;    /**< number of faces between internal and external cells */
    int number_of_faces;        /**< sum of internal and external faces */

} Face_Dict_type;

/**
 * To modify [Node_Dict](@ref Node_Dict_type) options register the user-defined
 * function `"user_set_node_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function and use the `rcfd_set_dict_property_i()` function to modify the
 * desired entry.
 * Alternatively, Node_Dict options may be specified in the TOML configuration
 * file using a table with the header `[nodes]`.
 * Note that the number of nodes must be given as an array containing a value
 * for each compute node.
 * Possible options are:
 */
typedef struct Node_Dict_struct
{
    int number_of_nodes;                /**< number of unique cell nodes */
    int number_of_int_node_references;  /**< number of nodes referenced by internal cells
                                             (includes multiple counting) */
    int number_of_ext_node_references;  /**< number of nodes referenced by external cells
                                             (includes multiple counting) */

} Node_Dict_type;

typedef struct Topo_Dict_struct
{
    Cell_Dict_type  *Cell_Dict;

    Face_Dict_type  *Face_Dict;

    Node_Dict_type  *Node_Dict;

} Topo_Dict_type;

/**
 * Probe types
 */
typedef enum {
    RCFD_PROBE_CELL = 0,
    RCFD_PROBE_VOF,
    RCFD_PROBE_USER,
    RCFD_PROBE_REC_USER,
    RCFD_PROBE_MAX
} rcfd_probe_type;

/**
 * To modify [Dump_Dict](@ref Dump_Dict_type) options register the user-defined
 * function `"user_set_dump_dict"` (cf. [Registering User-Defined Functions]
 * (#register_user_defined_functions)) via the `rcfd_register_user_function()`
 * function.
 * Within that user-defined function call one of the `rcfd_set_dict_property_*`
 * functions, i.e.:
 * - `rcfd_set_dict_property_i()`
 * - `rcfd_set_dict_property_r_3d()`
 *
 * to modify the desired entry.
 * Alternatively, Dump_Dict options may be specified in the TOML configuration
 * file using a table with the header `[dump]`.
 * Phase-specific settings are to be given via an array of sub-tables with the
 * header `[[dump.phases]]`.
 * Data-specific settings in a phase are to be given as an array (e.g.
 * `dump_cell_data`).
 * Phase-independent probes (user or rec_user as defined in Cell_Dict) can be
 * defined via an array of sub-tables with the header `[[dump.probes]]`.
 * Phase-specifc probes (cell data as defined in Data_Dict or vof) can be
 * defined via an array of sub-tables with the header `[[dump.phases.probes]]`
 * in the corresponding phase table.
 * Data-specific probes (user, rec_user or cell data) require a data index to be
 * defined.
 * Possible options are:
 */
typedef struct Dump_Dict_struct
{
    int     dump_interval;          /**< write interval for dump files (`<1` means off) */
    int     overwrite_interval;     /**< overwrite interval for dump files (`<1` means off) */
    int     dump_file_type;         /**< flag indicating [dump file type](@ref rcfd_file_type) */
    short   multifile_output_on;    /**< switch on/off per-partition dump files: `0` (off), `1` (on) */
    short   dump_vof;               /**< switch on/off writing of vof information (for all phases): `0` (off), `1` (on) */

    short   **dump_cell_data;       /**< switch on/off writing of each cell data of each phase: `0` (off), `1` (on) */ // [i_phase][i_data]
    short   *dump_user_data;        /**< switch on/off writing of each user cell data: `0` (off), `1` (on) */ // [i_user]
    short   *dump_rec_user_data;    /**< switch on/off writing of each recurrence user cell data: `0` (off), `1` (on) */ // [i_rec_user]

    int     probe_interval;         /**< write interval for probe file (`<1` means off) */
    int     probe_count;            /**< global number of data probes */

    int     *probe_layer;           /**< array of probed cell layers */ // [i_probe]
    int     *probe_cell;            /**< array of probed cells */ // [i_probe]
    int     *probe_type;            /**< array of probe types (one of PROBE_TYPE) */ // [i_probe]
    int     *probe_phase;           /**< array of phase indices */ // [i_probe]
    int     *probe_data;            /**< array of data indices */ // [i_probe]
    double  *probe_value;           /**< value at probe coordinates */ // [i_probe]
    double  **probe_x;              /**< probe coordinates */ // [i_probe][i_dim]

} Dump_Dict_type;


typedef struct Solver_struct
{
    int         current_state, upcoming_state;
    int         current_layer;
    int         current_frame;
    int         prev_frame;

    // start only in RCFD_MODE_PREPARATION
    int         analyse_CFD_counter;
    int         shifts_ready2write;
    int         tracer_monitoring_started;
    int         displ_reset_counter;
    // ... end only in RCFD_MODE_PREPARATION

    int         c2c_shift_format;
    int         c2c_file_type;
    int         c2c_shift_type_count;
    int         total_shift_read_count;

    int         global_run_counter;
    double      global_time;
    double      *timestep_width;

    short       balance_file_opened;
    short       rec_frames_monitor_file_opened;

} Solver_type;

typedef struct Cell_struct
{
    double      **x;
    double      *volume;
    double      *grid_spacing;

    double      x_min_node[3];
    double      x_max_node[3];
    double      x_min_global[3];
    double      x_max_global[3];

    int         *number_of_neigh_cells;
    int         *number_of_next_neigh_cells;
    int         **neigh_cell;       /* neighbor cells */
    int         **next_neigh_cell;  /* neighbor and next neighbor cells */

    double      **average_velocity; /* [i_phase][i_cell] */
    double      **crossing_time;

    short       *hit_by_other_cell;

    double      *weight_after_shift;
    double      *weight_after_swap;

    double      ***vof;             /* [i_frame][i_cell][i_phase] */

    double      **vof_changed;      /* [i_phase][i_cell] */

    double      ***data;            /* [i_phase][i_cell][i_data] */
    double      **data_shift;       // [i_cell][i_data]
    double      **data_swap;
    short       *data_shift_updated;// [i_cell]

    double      ***local_diff;      /* [i_frame][i_phase][i_cell] */

    double      *drift_exchange;    /* [i_cell] */

    double      ***cfd_displ;       /**< displacement vector field */ /* [i_phase][i_cell][i_dim] */
    double      ***cfd_displ_diff;  /* [i_phase][i_cell][i_dim] ; i_dim == {x, y, z, xx, yy, zz} */

    double      *tmp;               /* [i_cell] */

    double      **user;             /* [i_cell][i_user] */
    double      ***rec_user;        /* [i_frame][i_cell][i_rec_user] */

    short       *marked;            /* [i_cell] */
    int         *parent_cell;
    short       *number_of_children;
    int         **child_index;

    int         *type;              /**< VTK cell type */
    int         *number_of_nodes;   /**< number of cell nodes */
    int         **nodes;            /**< indices into cell node array [i_cell][i_node] */
    // polyhedra
    int         number_of_polyhedra;        /**< number of all polyhedral cells */
    int         *number_of_poly_faces;      /**< number of cell faces for polyhedral cells [i_polyhedron] */
    int         **number_of_poly_face_nodes;/**< number of face nodes for polyhedral cells [i_polyhedron][i_face] */
    int         number_of_poly_nodes;       /**< total number of all poly nodes (length of poly_nodes array) */
    int         *poly_nodes;                /**< indices into face node array, consecutive for all polyhedral cells [i_node] */

} Cell_type;

typedef struct Face_struct
{
    int     *c0, *c1;

    double  **area;

    double  **local_drift_adaption;     // used by Phase_Dict.local_drift_on

} Face_type;

typedef struct Node_struct
{
    double      **x;

} Node_type;

typedef struct Topo_struct
{
    Cell_type   *Cell;  // [i_layer]

    Face_type   *Face;

    Node_type   *Node;

} Topo_type;

/**
 * Cell-to-cell shift format names
 * The actual enum values are composed of the sizes of the format components in bytes,
 * i.e. each hexadecimal digit represents the type size of a format component.
 * The components are in the order c0,n0,c1,n1,w0 with the first component (c0)
 * corresponding to the least significant (rightmost) bits.
 */
typedef enum {
    RCFD_SHIFT_C0_N0_C1_N1_W0       = 0x00084444,
    RCFD_SHIFT_C0_N0_C1_W0          = 0x00080444,
    RCFD_SHIFT_C0_N0_C1_W0CHAR      = 0x00010444,
    RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR  = 0x00010414
} rcfd_c2c_shift_format;

/**
 * Cell-to-cell shift type names
 */
typedef enum {
    RCFD_SHIFT_TYPE_PREPARATION,
    RCFD_SHIFT_TYPE_ANY,
    RCFD_SHIFT_TYPE_INTERIOR,
    RCFD_SHIFT_TYPE_INCOMING,
    RCFD_SHIFT_TYPE_OUTGOING    = RCFD_SHIFT_TYPE_INCOMING,
    RCFD_SHIFT_TYPE_INTER_NODE  = RCFD_SHIFT_TYPE_INCOMING,
    RCFD_SHIFT_TYPE_MAX
} rcfd_c2c_shift_type;

typedef struct C2C_Shift_struct
{
    int           shifts_count;                 // max number of shifts as allocated
    int           valid_shifts_count;           // < shift_count

    int           *c0, *node0, *c1, *node1;     // [i_shift]

    unsigned char *node0char, *w0char;

    double        *w0;

    double        *dist_sq;                     // only RCFD_MODE_PREPARATION

    double        **c0_coords, **displ;         // only RCFD_MODE_PREPARATION

} C2C_Shift_type;

typedef struct C2C_struct
{
    short waterproof_shifts;
    short invalid_shifts_exist;
    short shift_weights_add_to_one;
    short c1_is_interior_cell;

    C2C_Shift_type  shifts;         // interior/intra-node shifts
    C2C_Shift_type  shifts_in;      // incoming shifts
    C2C_Shift_type  shifts_out;     // outgoing shifts

    int *shifts_to_node_count;      // [i_node],      only used by Node-0
    int *out2in;                    // [i_shift_out], only used by Node-0

} C2C_type;

typedef struct Balance_struct
{
    double      mass_integral, mass_integral_target;

    double      mass_integral_global, mass_integral_target_global;

    double      mass_source, mass_source_global;

    double      mass_error, mass_error_global;

} Balance_type;

typedef struct Norm_struct
{
    int     number_of_norms;

    int     frame_counter; // used for all fields!

    double  *norm;

} Norm_type;

typedef struct Rec_struct
{
    int     frame_in_sequence;
    int     sequence_length;
    short   jumped_at_last_frame;

    int     ***jumps;                  /* [state, state2, frame] */

} Rec_type;

typedef enum{ // parallel_communication_formats

    RCFD_PARALLEL_MAP_HOST_FORMAT,
    RCFD_PARALLEL_SUM_CELLS_FORMAT,
    RCFD_PARALLEL_FORMAT_MAX

} rcfd_parallel_format_names;

typedef struct MPI_cells_struct
{
    int     total_number_of_ext_cells;          // node-0

    int     number_of_host_cells;

    int     *number_of_ext_cells_per_node;

    int     *number_of_host_cells_per_node;

    int     *host_of_cell;                      /* all nodes: list of host of all cells */

    int     *host_of_ext_cells;                 /* node-0: total list of all ext. cells */

    int     *hosting_cell_index;                /* all nodes: list of cells, which host ext. cells of other nodes */

    int     *host2ext_index;

    double  *data;                              // all nodes: cell_data of corona cells

} MPI_cells_type;

typedef struct MPI_topo_struct
{
    MPI_cells_type  *MPI_Cell;

} MPI_topo_type;

typedef struct Parallel_struct{

    int     rank;
    int     size;

    MPI_Comm world;

    int     finalize;

    struct  MPI_topo_struct *MPI_Topo;

    double  *transfer_data_in, *transfer_data_out;

} Parallel_type;

typedef struct rCFD_UDF_struct
{
    void (*_user_set_solver_dict)   (void *handle);
    void (*_user_set_file_dict)     (void *handle);
    void (*_user_set_phase_dict)    (void *handle);
    void (*_user_set_prep_dict)     (void *handle);
    void (*_user_set_norm_dict)     (void *handle);
    void (*_user_set_rec_dict)      (void *handle);
    void (*_user_set_data_dict)     (void *handle);
    void (*_user_set_balance_dict)  (void *handle);
    void (*_user_set_dump_dict)     (void *handle);
    void (*_user_set_topo_dict)     (void *handle);
    void (*_user_set_cell_dict)     (void *handle, const short i_layer);
    void (*_user_set_face_dict)     (void *handle, const short i_layer);
    void (*_user_set_node_dict)     (void *handle, const short i_layer);

    void (*_user_pre_proc)          (void *handle); // preparation only

    void (*_user_init_cells)        (void *handle, const short i_layer);
    void (*_user_init_faces)        (void *handle, const short i_layer);
    void (*_user_init_cell_hosts)   (void *handle, const short i_layer);
    void (*_user_init_nodes)        (void *handle, const short i_layer);
    void (*_user_init_data)         (void *handle, const short i_layer); // run only

    short (*_user_set_layer)        (void *handle, const short current_layer); // run only

    void (*_user_access_data_before_phase_loop) (void *handle); // run only
    void (*_user_access_data_before_shift)      (void *handle, const short i_phase); // run only
    void (*_user_access_data_after_swap)        (void *handle, const short i_phase); // run only

    void (*_user_post)(void *handle); // run only

    void (*_user_set_norm)      (void *handle); // preparation only
    void (*_user_set_rec_vars)  (void *handle, const short i_layer); // preparation only

} rCFD_UDF_type;


typedef clock_t ticks_t;

typedef struct Timer_struct
{
    ticks_t m_value;
    ticks_t m_total_value;
    short is_running;
} Timer_type;

/**
 * Time units usable in reporting module
 */
typedef enum {
    RCFD_TIMER_NANOSEC = 0,
    RCFD_TIMER_MICROSEC,
    RCFD_TIMER_MILLISEC,
    RCFD_TIMER_SEC
} rcfd_timer_time_unit;

/**
 * Code units timeable in reporting module
 */
typedef enum {
    RCFD_TIMER_GLOBAL = 0,
    RCFD_TIMER_INIT,
    RCFD_TIMER_READ_DB,
    RCFD_TIMER_READ_SHIFT,
    RCFD_TIMER_RUN,
    RCFD_TIMER_RUN_UPDATE,
    RCFD_TIMER_RUN_SHIFT,
    RCFD_TIMER_RUN_SWAP,
    RCFD_TIMER_RUN_BALANCE,
    RCFD_TIMER_USER,
    RCFD_TIMER_MAX
} rcfd_timer_code_unit;

/**
 * Code section categories for reporting module
 */
typedef enum {
    RCFD_REPORT_CATEGORY_NOTSET = 0,
    RCFD_REPORT_CATEGORY_INITIALIZE,
    RCFD_REPORT_CATEGORY_CREATE_SHIFT,
    RCFD_REPORT_CATEGORY_WRITE_SHIFT,
    RCFD_REPORT_CATEGORY_WRITE_FIELDS,
    RCFD_REPORT_CATEGORY_WRITE_RECURRENCE,
    RCFD_REPORT_CATEGORY_WRITE_TOPOLOGY,
    RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
    RCFD_REPORT_CATEGORY_READ_DB,
    RCFD_REPORT_CATEGORY_RUN_UPDATE,
    RCFD_REPORT_CATEGORY_RUN_SHIFT,
    RCFD_REPORT_CATEGORY_RUN_SWAP,
    RCFD_REPORT_CATEGORY_RUN_BALANCE,
    RCFD_REPORT_CATEGORY_RUN,
    RCFD_REPORT_CATEGORY_POSTPROCESS,
    RCFD_REPORT_CATEGORY_USER,
    RCFD_REPORT_CATEGORY_UNKNOWN,
    RCFD_REPORT_CATEGORY_MAX
} rcfd_report_category;


typedef struct Report_struct
{
    /* public members / determined by dict entries */
    short       console_output_on;      /**< flag indicating on/off status of console output */
    short       file_output_on;         /**< flag indicating on/off status of file output */
    short       timing_on;              /**< flag indicating on/off status of timing */
    short       fflush_on;              /**< flag indicating on/off status of stream flush */
    short       priority;               /**< report priority determining visibility of messages in output stream */
    short       time_unit;              /**< unit of measured time, one of @ref rcfd_timer_time_unit */
    /* private members / internal usage only */
    short       file_is_open;           /**< flag indicating if per-process output file is open */
    short       file0_is_open;          /**< flag indicating if node0 output file is open */
    int         **number_of_reports;    /**< statistics of the number of messages and their priority for each report category */
    double      ticks_2_time_unit;      /**< conversion factor from clock ticks to time unit */
    FILE        *report_file;           /**< pointer to per-process output FILE object */
#if defined(RCFD_LIB_MPI)
    MPI_File    report_file0;           /**< shared MPI_File output object */
#else
    FILE        *report_file0;          /**< pointer to node0 output FILE object */
#endif
    Timer_type  *timers[RCFD_TIMER_MAX];/**< array of timers for all timeable code units */
} Report_type;


typedef struct rCFD_struct
{
    int     dict_write_permission_mask; /**< mask for setting write permission of individual dictionaries */
    int     initialized;                /**< flag indicating if all components are initialized */
    void    *config;

    Report_type         *Report;

    File_Dict_type      *File_Dict;

    Solver_Dict_type    *Solver_Dict;

    Solver_type         *Solver;

    Rec_Dict_type       *Rec_Dict;

    Rec_type            *Rec;

    Topo_Dict_type      *Topo_Dict;

    rCFD_UDF_type       *rCFD_UDF;

    Parallel_type       *Parallel;

#if RP_NODE
    Phase_Dict_type     *Phase_Dict;

    Norm_Dict_type      *Norm_Dict;

    Data_Dict_type      **Data_Dict;

    Balance_Dict_type   **Balance_Dict;

    Dump_Dict_type      *Dump_Dict;

    Prep_Dict_type      *Prep_Dict;

    C2C_Shift_type      *C2C_Prep;          // only RCFD_MODE_PREPARATION, [i_phase]

    C2C_Shift_type      *C2C_Interface;     // only RCFD_MODE_PREPARATION

    C2C_type            ****C2C;            // only RCFD_MODE_RUN, [i_state, i_phase, i_frame, i_layer]

    Topo_type           *Topo;

    Norm_type           *Norms;

    Balance_type        **Balance;
#endif
} rCFD_type;

#endif
