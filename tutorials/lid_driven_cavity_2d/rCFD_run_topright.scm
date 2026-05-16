;; (C) 2026
;; Case for Testing and Prediction with automatic plotting and animation
;; Md Zahid Hasan
;; Particulate Flow Modelling
;; Johannes Kepler University, Linz, Austria
;; www.particulate-flow.at
;;
;; rCFD_run_topright.scm
;; Circular scalar injection at top-right corner
;; CX=0.025, CY=0.025, CR=0.005, CZ=0.005
;; Results saved to post\rCFD_topright

(define ANSYS_Fluent_dir        ".\\\\ansys_fluent")
(define ANSYS_Fluent_file       "LidDrivenCavity_2D")
(define rCFD_src_dir            "..\\\\..\\\\src")
(define rCFD_third_party_dir    "..\\\\..\\\\third-party")
(define rCFD_user_src_dir       ".\\\\user_src")
(define rCFD_post_dir           ".\\\\post\\\\rCFD_topright")
(define python_exe              "C:\\\\Users\\\\zahid\\\\rcfd_env\\\\Scripts\\\\python.exe")
(define number_of_rCFD_episodes 200)
(define i 0)
(define img-1 "rCFD_mix_topright_")
(define post_on 1)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_run)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    (rCFD_r1)
    (rCFD_r2)
    (rCFD_r3)
    (rCFD_r4)
    (if (= post_on 1) (rCFD_r5_post) (rCFD_r5))
    (rCFD_r6)
    (rCFD_r7)
    (rCFD_r8)   ;; moves .jpg .out .trn to post folder FIRST
    (rCFD_r9)   ;; then runs python FROM INSIDE post folder
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r1)       ;; load all into main folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!del *.h5")

    (ti-menu-load-string (format #f "!copy ~a\\\\~a_START.cas.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))
    (ti-menu-load-string (format #f "!copy ~a\\\\~a_START.dat.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))

    (ti-menu-load-string (format #f "!copy ~a\\\\*.c ." rCFD_src_dir))
    (ti-menu-load-string (format #f "!copy ~a\\\\*.h ." rCFD_src_dir))

    (ti-menu-load-string (format #f "!copy ~a\\\\tomlc17\\\\*.c ." rCFD_third_party_dir))
    (ti-menu-load-string (format #f "!copy ~a\\\\tomlc17\\\\*.h ." rCFD_third_party_dir))

    (ti-menu-load-string (format #f "!copy ~a\\\\rCFD_user_run_topright.h .\\\\rCFD_user_run.h" rCFD_user_src_dir))
    (ti-menu-load-string "!move rCFD_user_run.h rCFD_user.h")
)

(if (not (symbol-bound? 'fluent_has_compiled (the-environment)))
    (begin
        (define fluent_has_compiled #f)
        (define fluent_has_compiled_headers #f)
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r2)       ;; compile into libudf_rcfd_run
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!if exist libudf_rcfd_run rmdir /s /q libudf_rcfd_run")

    (load (format #f "~a\\\\rCFD_C2C_run_compile.scm" rCFD_src_dir))

    (ti-menu-load-string "!del *.c")
    (ti-menu-load-string "!del *.h")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r3)       ;; load case, allocate UDMs, load UDFs, init rCFD
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/read-case .\\\\~a_START.cas.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_rcfd_run\"")
    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_cfd\"")

    (ti-menu-load-string (format #f "/file/read-data .\\\\~a_START.dat.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/user-defined-memory 10 q")
    (ti-menu-load-string "/define/user-defined/user-defined-node-memory 2 q")

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_init_all::libudf_rcfd_run\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r4)       ;; read recurrence database
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_read_DB::libudf_rcfd_run\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r5)       ;; run simulation (no post)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (do ((i 0 (+ i 1)))
        ((= i number_of_rCFD_episodes))
        (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_run::libudf_rcfd_run\"")
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r5_post)  ;; run simulation with JPEG post per episode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/display/open-window 1")
    (ti-menu-load-string "/display/set-window 1")

    (ti-menu-load-string "/display/set/picture/driver jpeg")
    (ti-menu-load-string "/display/set/picture/color-mode color")
    (ti-menu-load-string "/display/set/picture/invert-background? no")
    (ti-menu-load-string "/display/set/picture/use-window-resolution? no")
    (ti-menu-load-string "/display/set/picture/x-resolution 1200")
    (ti-menu-load-string "/display/set/picture/y-resolution 640")

    (ti-menu-load-string "/display/objects/create
        contour
        cont-mix
        field
            udm-0
        color-map
            visible no
            log-scale? no
            q
        boundary-values? no
        node-values? no
        range-option
            auto-range-off
            minimum  0.0
            maximum  1.0
            clip-to-range? no
            q
        surfaces-list
            4 ()
        q "
    )

    (ti-menu-load-string "/display/objects/create
        contour
        cont-mix2
        field
            udm-1
        color-map
            visible no
            log-scale? no
            q
        boundary-values? no
        node-values? no
        range-option
            auto-range-off
            minimum  0.0
            maximum  1.0
            clip-to-range? no
            q
        surfaces-list
            4 ()
        q "
    )

    (do ((i 0 (+ i 1)))
        ((= i number_of_rCFD_episodes))

        (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_run::libudf_rcfd_run\"")

        (ti-menu-load-string "/display/set/overlays no")

        (ti-menu-load-string "/display/objects/display cont-mix")
        (ti-menu-load-string "/display/update-scene/select-geometry yes cont-mix-4 no no")
        (ti-menu-load-string "/display/update-scene/display yes no yes yes no no no no no 0 0 0 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 0.0 0.0 0.0 yes 0 0 0 0 0 1 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes cont-mix-4")

        (ti-menu-load-string "/display/set/overlays yes")

        (ti-menu-load-string "/display/objects/display cont-mix2")
        (ti-menu-load-string "/display/update-scene/select-geometry yes cont-mix2-4 no no")
        (ti-menu-load-string "/display/update-scene/display yes no yes yes no no no no no 0 0 0 0")
        (ti-menu-load-string "/display/update-scene/transform no yes 0.12 0.0 0.0 yes 0 0 0 0 0 1 no")
        (ti-menu-load-string "/display/update-scene/select-geometry no yes cont-mix2-4")

        (ti-menu-load-string "/display/views/restore-view front2")

        (ti-menu-load-string (format #f "display/hardcopy ~a~04d.jpg" img-1 i))
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r6)       ;; write final case+data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/write-case-data .\\\\~a_END_rCFD_RUN.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r7)       ;; free rCFD memory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/execute-on-demand \"rCFD_free_all::libudf_rcfd_run\"")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r8)       ;; move all results to post folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!mkdir post")
    (ti-menu-load-string (format #f "!mkdir ~a" rCFD_post_dir))

    ;; move simulation outputs to post folder
    (ti-menu-load-string (format #f "!move *.jpg ~a" rCFD_post_dir))
    (ti-menu-load-string (format #f "!move *.trn ~a" rCFD_post_dir))
    (ti-menu-load-string (format #f "!move *.out ~a" rCFD_post_dir))

    (ti-menu-load-string "!move *.h5 .\\\\ansys_fluent\\\\")

    ;; clean source files
    (ti-menu-load-string "!del *.c")
    (ti-menu-load-string "!del *.h")
    (ti-menu-load-string "!del *.h5 2>nul")
    (ti-menu-load-string "!del fluent* 2>nul")
    (ti-menu-load-string "!del log 2>nul")
    ;; tiff/tif skipped -- Fluent holds lock during session
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (rCFD_r9)       ;; auto post: plots + MP4  (runs AFTER rCFD_r8)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;; copy python scripts + batch file into post folder
    (ti-menu-load-string (format #f "!copy ~a\\\\plot_CoG.py       ~a\\\\" rCFD_user_src_dir rCFD_post_dir))
    (ti-menu-load-string (format #f "!copy ~a\\\\plot_mix_ind.py   ~a\\\\" rCFD_user_src_dir rCFD_post_dir))
    (ti-menu-load-string (format #f "!copy ~a\\\\make_animation.py ~a\\\\" rCFD_user_src_dir rCFD_post_dir))
    (ti-menu-load-string (format #f "!copy ~a\\\\run_post.bat      ~a\\\\" rCFD_user_src_dir rCFD_post_dir))

    ;; run bat -- only pass img prefix as arg
    ;; bat uses %~dp0 (its own folder = post folder) as working dir
    (ti-menu-load-string (format #f
        "!cmd /c start /wait \"rCFD post\" ~a\\\\run_post.bat ~a"
        rCFD_post_dir
        img-1))
)