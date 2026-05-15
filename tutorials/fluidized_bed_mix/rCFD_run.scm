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
(define rCFD_post_dir           "./post/rCFD")
(define number_of_rCFD_episodes 500)
(define i 0)
(define img-1 "rCFD_mix_")
(define post_on 1)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r1)       ;; load all into main folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!rm ./*.h5")

    (ti-menu-load-string (format  #f "!cp ~a/~a_START.cas.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))
    (ti-menu-load-string (format  #f "!cp ~a/~a_START.dat.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))

    (ti-menu-load-string (format  #f "!cp ~a/*.c ." rCFD_src_dir))
    (ti-menu-load-string (format  #f "!cp ~a/*.h ." rCFD_src_dir))

    (ti-menu-load-string (format  #f "!cp ~a/tomlc17/*.c ." rCFD_third_party_dir))
    (ti-menu-load-string (format  #f "!cp ~a/tomlc17/*.h ." rCFD_third_party_dir))

    (ti-menu-load-string (format  #f "!cp ~a/*.h ." rCFD_user_src_dir))

    (ti-menu-load-string "!mv rCFD_user_run.h rCFD_user.h")
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
(define (rCFD_r2)       ;; compile everything into libudf_rcfd_run
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!rm -r libudf_rcfd_run")

    ;; load compile scheme
    (load (format  #f "~a/rCFD_C2C_run_compile.scm" rCFD_src_dir) )

    (ti-menu-load-string "!rm  ./*.c")
    (ti-menu-load-string "!rm  ./*.h")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r3)       ;; load start file, allocate UDMI's, load libudf_rcfd_run and init rCFD vars
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/read-case ./~a_START.cas.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_rcfd_run\"")

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_cfd\"")

    (ti-menu-load-string (format #f "/file/read-data ./~a_START.dat.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/user-defined-memory 10 q" )
    (ti-menu-load-string "/define/user-defined/user-defined-node-memory 2 q" )

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_init_all::libudf_rcfd_run\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r4)     ;; read database
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_read_DB::libudf_rcfd_run\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r5)     ;; run simulation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (do  ((i  0 (+ i  1)))
        ((= i  number_of_rCFD_episodes))

        (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_run::libudf_rcfd_run\"")
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r5_post)     ;; run simulation
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
        field mixture
            udm-1
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
        field mixture
            udm-2
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
        ((= i  number_of_rCFD_episodes))

        (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_run::libudf_rcfd_run\"")

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r6)       ;; write file
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/write-case-data ./~a_END_rCFD_RUN.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r7)       ;; run.6: free memory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_free_all::libudf_rcfd_run\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r8)       ;; run.7: clean-up folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!mkdir ./post")

    (ti-menu-load-string (format #f "!mkdir ~a" rCFD_post_dir))

    (ti-menu-load-string (format #f "!mv ./*.jpg ~a" rCFD_post_dir))
    (ti-menu-load-string (format #f "!mv ./*.trn ~a" rCFD_post_dir))
    (ti-menu-load-string (format #f "!mv ./*.out ~a" rCFD_post_dir))

    (ti-menu-load-string "!mv  ./*.h5 ./ansys_fluent/")

    (ti-menu-load-string "!rm  ./*.c")
    (ti-menu-load-string "!rm  ./*.h")
    (ti-menu-load-string "!rm  ./*.h5")
    (ti-menu-load-string "!rm  ./*.tiff")
    (ti-menu-load-string "!rm  ./*.tif")
    (ti-menu-load-string "!rm  ./fluent*")
    (ti-menu-load-string "!rm  ./log")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_run)          ;; run.1-8
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (rCFD_r1)
    (rCFD_r2)
    (rCFD_r3)
    (rCFD_r4)
    (if (= post_on 1) (rCFD_r5_post) (rCFD_r5))
    (rCFD_r6)
    (rCFD_r7)
    (rCFD_r8)
)
