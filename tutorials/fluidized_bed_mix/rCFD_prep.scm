;; (C) 2026
;; Stefan Pirker
;; Particulate Flow Modelling
;; Johannes Kepler University, Linz, Austria
;; www.particulate-flow.at

(define ANSYS_Fluent_dir        "./ansys_fluent")
(define ANSYS_Fluent_file       "FB_Mix")   ; w/o _START
(define rCFD_src_dir            "../../src")
(define rCFD_third_party_dir    "../../third-party")
(define rCFD_user_src_dir       "./user_src")
(define number_of_monitoring_episodes     502)     ; sums up to 5 s
(define simulation_timestep_width         0.001)
(define monitoring_timesteps_per_episode  10)
(define i 0)
(define post_on 0)
(define img-1 "CFD_")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p1)       ;; load all into main folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format  #f "!cp ~a/~a_START.cas.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))
    (ti-menu-load-string (format  #f "!cp ~a/~a_START.dat.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))

    (ti-menu-load-string (format  #f "!cp ~a/*.c ." rCFD_src_dir))
    (ti-menu-load-string (format  #f "!cp ~a/*.h ." rCFD_src_dir))

    (ti-menu-load-string (format  #f "!cp ~a/tomlc17/*.c ." rCFD_third_party_dir))
    (ti-menu-load-string (format  #f "!cp ~a/tomlc17/*.h ." rCFD_third_party_dir))

    (ti-menu-load-string (format  #f "!cp ~a/*.h ." rCFD_user_src_dir))
    (ti-menu-load-string (format  #f "!cp ~a/*.c ." rCFD_user_src_dir))
)

;; fluent_has_compiled lets the compile-scheme know if any udf has already been compiled
;; to enable recompilation
(if (not (symbol-bound? 'fluent_has_compiled (the-environment)))
    (begin
        (define fluent_has_compiled #f)
        (define fluent_has_compiled_headers #f)
    )
)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p2)       ;; compile everything into libudf_rcfd_prep
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!rm -r libudf_rcfd_prep")

    (ti-menu-load-string "!mv  ./rCFD_user_prep.h ./rCFD_user.h")

    ;; load compile scheme
    (load (format  #f "~a/rCFD_C2C_prep_compile.scm" rCFD_src_dir) )

    (ti-menu-load-string "!rm  ./*.c")
    (ti-menu-load-string "!rm  ./*.h")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p3)       ;; load start file, allocate UDMI's, load libudf_rcfd_prep and init rCFD vars
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/read-case ./~a_START.cas.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/user-defined-memory 10 q" )

    (ti-menu-load-string "/define/user-defined/user-defined-node-memory 2 q" )

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_cfd\"")

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_rcfd_prep\"")

    (ti-menu-load-string (format #f "/file/read-data ./~a_START.dat.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_init_all::libudf_rcfd_prep\"")

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_write_Topo::libudf_rcfd_prep\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p4)      ;; settings database creation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/user-defined-scalars
        6 no no
        air     yes \"mass flow rate\" \"default\"
        air     yes \"mass flow rate\" \"default\"
        air     yes \"mass flow rate\" \"default\"
        solids  yes \"mass flow rate\" \"default\"
        solids  yes \"mass flow rate\" \"default\"
        solids  yes \"mass flow rate\" \"default\"")

    (ti-menu-load-string "/define/materials/change-create air air
        no no no no no no
        yes defined-per-uds
        0 constant 1e-10
        1 constant 1e-10
        2 constant 1e-10
        -1
        no")

    (ti-menu-load-string "/define/materials/change-create solids solids
        no no no no no no
        yes defined-per-uds
        3 constant 1e-10
        4 constant 1e-10
        5 constant 1e-10
        -1
        no")

    (ti-menu-load-string "/solve/set/discretization-scheme/
        uds-0 0
        uds-1 0
        uds-2 0
        uds-3 0
        uds-4 0
        uds-5 0")

    (ti-menu-load-string "/define/boundary-conditions/set/wall
        wall-9 wall ()
        solids
        uds
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
        uds-bc
            no yes
            no yes
            no yes
        q"
    )

    ;; gas inlet is considered as wall bc for solids propergator
    (ti-menu-load-string "/define/boundary-conditions/set/velocity-inlet
        v_in_prim v_in_sec 7 ()
        solids
        uds
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
        uds-bc
            no yes
            no yes
            no yes
        q"
    )

    (ti-menu-load-string "/define/boundary-conditions/set/wall
        wall-9 wall ()
        air
        uds
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
        uds-bc
            no yes
            no yes
            no yes
        q"
    )

    (ti-menu-load-string "/define/boundary-conditions/set/velocity-inlet
        v_in_prim v_in_sec ()
        air
        uds
            yes yes \"udf\" \"rCFD_inlet_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_inlet_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_inlet_bc::libudf_rcfd_prep\"
        uds-bc
            no yes
            no yes
            no yes
        q"
    )

    ;; execute at end fcts
    (ti-menu-load-string "/define/user-defined/function-hooks/execute-at-end
        \"rCFD_execute_at_end::libudf_rcfd_prep\"
        \"\""
    )

    (ti-menu-load-string (format #f "/file/write-case-data ./~a_START_rCFD_PREP.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p5)       ;; run CFD for monitoring
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (do  ((i  0 (+ i  1)))
        ((= i  number_of_monitoring_episodes))

        (ti-menu-load-string (format #f "/solve/set/time-step ~d" simulation_timestep_width))
        (ti-menu-load-string (format #f "/solve/dual-time-iterate ~d 30" monitoring_timesteps_per_episode))
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p6)       ;; write file
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/write-case-data ./~a_END_rCFD_PREP.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p7)       ;; analyze and write recurrence path,
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_write_Rec::libudf_rcfd_prep\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p8)      ;; free memory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_free_all::libudf_rcfd_prep\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p9)     ;; clean-up folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!rm  ./tmp*")
    (ti-menu-load-string "!mv  ./*.h5 ./ansys_fluent/")

    (ti-menu-load-string "!mv  ./*.trn ./data/")

    (ti-menu-load-string "!rm  ./*.c")
    (ti-menu-load-string "!rm  ./*.h")
    (ti-menu-load-string "!rm  ./*.out")
    (ti-menu-load-string "!rm  ./fluent*")
    (ti-menu-load-string "!rm  ./log")
    (ti-menu-load-string "!rm  ./*.flprj")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_prep)     ;; run all for preparation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (rCFD_p1)
    (rCFD_p2)
    (rCFD_p3)
    (rCFD_p4)
    (rCFD_p5)
    (rCFD_p6)
    (rCFD_p7)
    (rCFD_p8)
    (rCFD_p9)
)
