;; (C) 2026
;; Stefan Pirker
;; Particulate Flow Modelling
;; Johannes Kepler University, Linz, Austria
;; www.particulate-flow.at

(define ANSYS_Fluent_dir    "./ansys_fluent")
(define ANSYS_Fluent_file   "FB_Mix")       ; w/o _START
(define CFD_user_src_dir    "./user_src")
(define number_of_CFD_episodes      500)    ; sums up to 5s
(define simulation_timestep_width   0.001)
(define timesteps_per_CFD_episode   10)
(define post_on 1)
(define CFD_post_dir    "CFD_reference")
(define img-1 "CFD_Mix_")
(define i 0)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_1)     ;; load all into main folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format  #f "!cp ~a/~a_START.cas.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))
    (ti-menu-load-string (format  #f "!cp ~a/~a_START.dat.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))

    (ti-menu-load-string (format  #f "!cp ~a/CFD_user.c ." CFD_user_src_dir))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_2)     ;; compile everything into libudf_cfd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!rm -r libudf_cfd")

    (ti-menu-load-string "/define/user-defined/compiled-functions compile
                        \"libudf_cfd\" yes
                        \"CFD_user.c\"
                        \"\"
                        \"\"")

    (ti-menu-load-string "!rm CFD_user.c")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_3)     ;; load start file, allocate UDMI's and load libudf_cfd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/read-case ./~a_START.cas.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_cfd\"")

    (ti-menu-load-string (format #f "/file/read-data ./~a_START.dat.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_4)     ;; specific setting (additional eqns, define regions and patch spec fields)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;; set mix and sec_gas UDSI
    (ti-menu-load-string "/define/user-defined/user-defined-scalars
        2 no no
        solids yes \"mass flow rate\" \"default\"
        air    yes \"mass flow rate\" \"default\"")

    (ti-menu-load-string "/define/materials/change-create solids solids
        no no no no no no
        yes defined-per-uds
        0 constant 1e-5
        -1
        no")

    (ti-menu-load-string "/define/materials/change-create air air
        no no no no no no
        yes defined-per-uds
        1 constant 1e-5
        -1
        no")

    (ti-menu-load-string "/solve/set/discretization-scheme/
        uds-0 0
        uds-1 0")

    ;; init mix and sec_gas UDSI
    (ti-menu-load-string "/solve/patch solids 3 10 () () uds-0 0.0")
    (ti-menu-load-string "/solve/patch air    3 10 () () uds-1 0.0")

    (ti-menu-load-string "/mesh/adapt/cell-registers
        add \"region_init_solids_mix\"
        type hexahedron
        inside? yes
        min-point -1.0  0.0 -1.0
        max-point  1.0  1.0  1.0
        q")

    (ti-menu-load-string "/solve/patch solids () \"region_init_solids_mix\" () uds-0 1.0")

    (ti-menu-load-string "/mesh/adapt/cell-registers
        add \"region_init_sec_air\"
        type hexahedron
        inside? yes
        min-point -1.0  -1.0   -1.0
        max-point  1.0  -0.075  1.0
        q")

    (ti-menu-load-string "/solve/patch air () \"region_init_sec_air\" () uds-1 1.0")

    ;; set execute at end fcts
    (ti-menu-load-string "/define/user-defined/function-hooks/execute-at-end
                        \"CFD_write_mix_ind::libudf_cfd\"
                        \"CFD_write_mass::libudf_cfd\"
                        \"CFD_set_solid_mix_UDMI::libudf_cfd\"
                        \"\"")

    (ti-menu-load-string (format #f "/file/write-case-data ./~a_START_CFD_RUN.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_5)     ;; run simulation w/o post-proc
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (do  ((i  0 (+ i  1)))
        ((= i  number_of_CFD_episodes))

        (ti-menu-load-string (format #f "/solve/set/time-step ~d" simulation_timestep_width))
        (ti-menu-load-string (format #f "/solve/dual-time-iterate ~d 30" timesteps_per_CFD_episode))

    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_5_post)     ;; run simulation with post-proc
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;; graphics setting
    (ti-menu-load-string "/display/open-window 1")
    (ti-menu-load-string "/display/set-window 1")

    (ti-menu-load-string "/display/set/picture/driver jpeg")
    (ti-menu-load-string "/display/set/picture/color-mode color")
    (ti-menu-load-string "/display/set/picture/invert-background? no")
    (ti-menu-load-string "/display/set/picture/invert-background? no")
    (ti-menu-load-string "/display/set/picture/use-window-resolution? no")
    (ti-menu-load-string "/display/set/picture/x-resolution 1200")
    (ti-menu-load-string "/display/set/picture/y-resolution 640")

    ;; generate lhs contour
    (ti-menu-load-string "/display/objects/create
        contour
        cont-left
        field air
            uds-1
        color-map
            visible no
            log-scale? no
            q
        boundary-values? no
        node-values? yes
        range-option
            auto-range-off
            minimum  0.0
            maximum  1.0
            clip-to-range? no
            q
        surfaces-list
            5 ()
        q "
    )

    ;; generate mid contour
    (ti-menu-load-string "/display/objects/create
        contour
        cont-mid
        field solids
            vof
        color-map
            visible no
            log-scale? no
            q
        boundary-values? no
        node-values? yes
        range-option
            auto-range-off
            minimum 0.0
            maximum 0.5
            clip-to-range? no
            q
        surfaces-list
            5 ()
        q "
    )

    ;; generate rhs contour
    (ti-menu-load-string "/display/objects/create
        contour
        cont-right
        field mixture
            udm-0
        color-map
            visible no
            log-scale? no
            q
        boundary-values? no
        node-values? yes
        range-option
            auto-range-off
            minimum 0.0
            maximum 0.5
            clip-to-range? no
            q
        surfaces-list
            5 ()
        q "
    )

    (do  ((i  0 (+ i  1)))
        ((= i  number_of_CFD_episodes))

        (ti-menu-load-string (format #f "/solve/set/time-step ~d" simulation_timestep_width))
        (ti-menu-load-string (format #f "/solve/dual-time-iterate ~d 30" timesteps_per_CFD_episode))

        (ti-menu-load-string "/display/set/overlays no")

        (ti-menu-load-string "/display/objects/display cont-left")
        (ti-menu-load-string "/display/update-scene/select-geometry yes cont-left-5 no no")
        (ti-menu-load-string "/display/update-scene/display yes no yes yes no no no no no 0 0 0 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 0.0 0.0 0.0 yes 0 0 0 0 0 1 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes cont-left-5")

        (ti-menu-load-string "/display/set/overlays yes")

        (ti-menu-load-string "/display/objects/display cont-mid")
        (ti-menu-load-string "/display/update-scene/select-geometry yes cont-mid-5 no no")
        (ti-menu-load-string "/display/update-scene/display yes no yes yes no no no no no 0 0 0 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 0.0 0.2 0.0 yes 0 0 0 0 0 1 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes cont-mid-5")

        (ti-menu-load-string "/display/objects/display cont-right")
        (ti-menu-load-string "/display/update-scene/select-geometry yes cont-right-5 no no")
        (ti-menu-load-string "/display/update-scene/display yes no yes yes no no no no no 0 0 0 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 0.0 0.4 0.0 yes 0 0 0 0 0 1 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes cont-right-5")

        (ti-menu-load-string "/display/views/restore-view view_x=0")

        (ti-menu-load-string (format #f "display/hardcopy ~a~04d.jpg" img-1 i))
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_6)     ;; write file
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/write-case-data ./~a_END_CFD_RUN.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_7)     ;; clean-up folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!mkdir ./post")

    (ti-menu-load-string (format #f "!mkdir ./post/~a" CFD_post_dir))

    (ti-menu-load-string (format #f "!mv ./*.out ./post/~a" CFD_post_dir))
    (ti-menu-load-string (format #f "!mv ./*.trn ./post/~a" CFD_post_dir))
    (ti-menu-load-string (format #f "!mv ./*.jpg ./post/~a" CFD_post_dir))

    (ti-menu-load-string "!rm tmp*")
    (ti-menu-load-string (format #f "!mv ./*.h5 ./~a" ANSYS_Fluent_dir))

    (ti-menu-load-string "!rm log")
    (ti-menu-load-string "!rm *.tif")
    (ti-menu-load-string "!rm *.flprj")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_run)       ;; set up and run cfd simulation w/o post-processing
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (CFD_1)
    (CFD_2)
    (CFD_3)
    (CFD_4)
    (if (= post_on 1) (CFD_5_post) (CFD_5))
    (CFD_6)
    (CFD_7)
)
