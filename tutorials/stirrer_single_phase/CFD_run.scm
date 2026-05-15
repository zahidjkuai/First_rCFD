;; (C) 2026
;; Stefan Pirker
;; Particulate Flow Modelling
;; Johannes Kepler University, Linz, Austria
;; www.particulate-flow.at

(define ANSYS_Fluent_dir    "./ansys_fluent")
(define ANSYS_Fluent_file   "Stirrer_SinglePhase")  ; w/o _START
(define CFD_user_src_dir    "./user_src")
(define number_of_CFD_episodes      200)    ; sums up to 10s
(define simulation_timestep_width   0.01)
(define timesteps_per_CFD_episode   5)
(define post_on 1)
(define CFD_post_dir "CFD_reference")
(define img-1 "CFD_mix_")
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

    (ti-menu-load-string "/define/user-defined/user-defined-memory 5 q" )

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_cfd\"")

    (ti-menu-load-string (format #f "/file/read-data ./~a_START.dat.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_4)     ;; specific setting (additional eqns, define regions and patch spec fields)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   (ti-menu-load-string "/define/user-defined/user-defined-scalars
        2 no no
        yes \"mass flow rate\" \"default\"
        yes \"mass flow rate\" \"default\"")

    (ti-menu-load-string "/define/materials/change-create water water
        no no no no no
        yes 0.5
        yes defined-per-uds
        0 constant 1
        1 constant 1
        -1
        no")

    (ti-menu-load-string "/solve/set/discretization-scheme/
        uds-0 1
        uds-1 1")

    (ti-menu-load-string "/mesh/adapt/cell-registers
        add \"region_mix_z\"
        type cylinder
        inside? yes
        axis-begin 0 0 -99
        axis-end   0 0 0.3
        radius 99
        q")

    (ti-menu-load-string "/mesh/adapt/cell-registers
        add \"region_mix_r\"
        type cylinder
        inside? yes
        axis-begin 0 0 -99
        axis-end   0 0 99
        radius 0.25
        q")

    (ti-menu-load-string "/solve/patch () \"region_mix_z\" () uds-0 1")
    (ti-menu-load-string "/solve/patch () \"region_mix_r\" () uds-1 1")

    (ti-menu-load-string "/define/user-defined/function-hooks/execute-at-end
                        \"CFD_write_mixing_indices::libudf_cfd\"
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

    ;; generate lhs mesh
    (ti-menu-load-string "/display/objects/create
        mesh
        mesh-z
        options faces? yes q
        surfaces-list 6 7 ()
        coloring uniform face \"dark gray\" q")

    (ti-menu-load-string "/display/objects/create
        mesh
        mesh-z-edge
        options faces? no edges? yes q
        edge-type outline
        surfaces-list 3 4 9 ()
        coloring uniform edges \"dark gray\" q")

    ;; generate rhs mesh
    (ti-menu-load-string "/display/objects/create
        mesh
        mesh-r
        options faces? yes q
        surfaces-list 6 7 ()
        coloring uniform face \"dark gray\" q")

    (ti-menu-load-string "/display/objects/create
        mesh
        mesh-r-edge
        options faces? no edges? yes q
        edge-type outline
        surfaces-list 3 4 9 ()
        coloring uniform edges \"dark gray\" q")

    ;; generate lhs contour
    (ti-menu-load-string "/display/objects/create
        contour
        cont-mix-z
        field
            uds-0
        color-map
            visible no
            log-scale? no
            q
        boundary-values? no
        node-values? no
        range-option
            auto-range-off
            minimum 0
            maximum 1
            clip-to-range? no
            q
        surfaces-list
            17 ()
        q "
    )

    ;; generate rhs contour
    (ti-menu-load-string "/display/objects/create
        contour
        cont-mix-r
        field
            uds-1
        color-map
            visible no
            log-scale? no
            q
        boundary-values? no
        node-values? no
        range-option
            auto-range-off
            minimum 0
            maximum 1
            clip-to-range? no
            q
        surfaces-list
            17 ()
        q "
    )

    (do  ((i  0 (+ i  1)))
        ((= i  number_of_CFD_episodes))

        (ti-menu-load-string (format #f "/solve/set/time-step ~d" simulation_timestep_width))
        (ti-menu-load-string (format #f "/solve/dual-time-iterate ~d 30" timesteps_per_CFD_episode))

        (ti-menu-load-string "/display/set/overlays no")

        (ti-menu-load-string "/display/objects/display mesh-z")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-z-6 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes yes yes no no no no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-z-6")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-z-7 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes yes yes no no no no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-z-7")

        (ti-menu-load-string "/display/set/overlays yes")

        (ti-menu-load-string "/display/objects/display mesh-z-edge")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-z-edge-3 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes no no no yes yes no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-z-edge-3")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-z-edge-4 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes no no no yes yes no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-z-edge-4")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-z-edge-9 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes no no no yes yes no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-z-edge-9")

        (ti-menu-load-string "/display/objects/display mesh-r")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-r-6 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes yes yes no no no no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 1.0 1.0 0.0 yes 0 0 0 0 0 0 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-r-6")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-r-7 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes yes yes no no no no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 1.0 1.0 0.0 yes 0 0 0 0 0 0 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-r-7")

        (ti-menu-load-string "/display/objects/display mesh-r-edge")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-r-edge-3 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes no no no yes yes no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 1.0 1.0 0.0 yes 0 0 0 0 0 0 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-r-edge-3")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-r-edge-4 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes no no no yes yes no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 1.0 1.0 0.0 yes 0 0 0 0 0 0 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-r-edge-4")
        (ti-menu-load-string "/display/update-scene/select-geometry yes mesh-r-edge-9 no no")
        (ti-menu-load-string "/display/update-scene/display yes yes no no no yes yes no no 60 60 60 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 1.0 1.0 0.0 yes 0 0 0 0 0 0 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes mesh-r-edge-9")

        (ti-menu-load-string "/display/objects/display cont-mix-z")
        (ti-menu-load-string "/display/update-scene/select-geometry yes cont-mix-z-17 no no")
        (ti-menu-load-string "/display/update-scene/display yes no yes yes no no no no no 0 0 0 30")
        (ti-menu-load-string "/display/update-scene/transform no yes 0.0 0.0 0.0 yes 0 0 90 0 0 1 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes cont-mix-z-17")

        (ti-menu-load-string "/display/objects/display cont-mix-r")
        (ti-menu-load-string "/display/update-scene/select-geometry yes cont-mix-r-17 no no")
        (ti-menu-load-string "/display/update-scene/display yes no yes yes no no no no no 0 0 0 30")
        (ti-menu-load-string "/display/update-scene/transform no yes 1.0 1.0 0.0 yes 0 0 90 0 0 1 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes cont-mix-r-17")

        (ti-menu-load-string "/display/views/restore-view view-2x")

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
