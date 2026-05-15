;; (C) 2026
;; Stefan Pirker
;; Particulate Flow Modelling
;; Johannes Kepler University, Linz, Austria
;; www.particulate-flow.at

(define ANSYS_Fluent_dir        "./ansys_fluent")
(define ANSYS_Fluent_file       "Stirrer_SinglePhase")  ; w/o _START
(define rCFD_src_dir            "../../src")
(define rCFD_third_party_dir    "../../third-party")
(define rCFD_user_src_dir       "./user_src")
(define number_of_monitoring_episodes     203)
(define simulation_timestep_width         0.01)
(define monitoring_timesteps_per_episode  5)
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

    (ti-menu-load-string "/define/mesh-interfaces/delete-all yes")

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_init_all::libudf_rcfd_prep\"")

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_write_Topo::libudf_rcfd_prep\"")

    (ti-menu-load-string "/define/mesh-interfaces/create my_interface no 19 20 () no")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p4)      ;; settings database creation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/user-defined-scalars
        3 no no
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"")

    (ti-menu-load-string "/define/materials/change-create water water
        no no no no no
        yes 0.5
        yes defined-per-uds
        0 constant 1e-10
        1 constant 1e-10
        2 constant 1e-10
        -1
        no")

    (ti-menu-load-string "/solve/set/discretization-scheme/
        uds-0 1
        uds-1 1
        uds-2 1")

    (ti-menu-load-string "/define/boundary-conditions/set/wall
        1 6 7 11 12 13 14 15 16 17 18 ()
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
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p4a)      ;; settings database creation (diffusion)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/user-defined-scalars
        9 no no
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"")

    (ti-menu-load-string "/define/materials/change-create water water
        no no no no no
        yes 0.5
        yes defined-per-uds
        0 constant 1e-10
        1 constant 1e-10
        2 constant 1e-10
        3 expression \"TurbulenceViscosity\"
        4 expression \"TurbulenceViscosity\"
        5 expression \"TurbulenceViscosity\"
        6 expression \"TurbulenceViscosity\"
        7 expression \"TurbulenceViscosity\"
        8 expression \"TurbulenceViscosity\"
        -1
        no")

    (ti-menu-load-string "/solve/set/discretization-scheme/
        uds-0 1
        uds-1 1
        uds-2 1
        uds-3 1
        uds-4 1
        uds-5 1
        uds-6 1
        uds-7 1
        uds-8 1")

    (ti-menu-load-string "/define/boundary-conditions/set/wall
        1 6 7 11 12 13 14 15 16 17 18 ()
        uds-bc
            no yes
            no yes
            no yes
            no yes
            no yes
            no yes
            no yes
            no yes
            no yes
        uds
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"
            yes yes \"udf\" \"rCFD_wall_bc::libudf_rcfd_prep\"

        q"
    )

    (ti-menu-load-string (format #f "/file/write-case-data ./~a_START_rCFD_PREP.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_p5)       ;; run CFD for monitoring
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/function-hooks/execute-at-end
        \"rCFD_execute_at_end::libudf_rcfd_prep\"
        \"\""
    )

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
    (ti-menu-load-string "!rm  ./fluent*")
    (ti-menu-load-string "!rm  ./log")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_prep)     ;; run all for preparation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (rCFD_p1)
    (rCFD_p2)
    (rCFD_p3)
    ;(rCFD_p4)
    (rCFD_p4a) ; monitor local diffusivity
    (rCFD_p5)
    (rCFD_p6)
    (rCFD_p7)
    (rCFD_p8)
    (rCFD_p9)
)
