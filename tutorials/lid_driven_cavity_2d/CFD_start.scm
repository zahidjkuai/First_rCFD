;; (C) 2026
;; Stefan Pirker
;; Modified by: Md Zahid Hasan
;; Particulate Flow Modelling
;; Johannes Kepler University, Linz, Austria
;; www.particulate-flow.at

(define ANSYS_Fluent_dir    ".\\ansys_fluent")
(define ANSYS_Fluent_file   "LidDrivenCavity_2D")   ; w/o _INIT
(define CFD_user_src_dir    ".\\user_src")
(define number_of_CFD_episodes      600)    ; sums up to 1min start-up time
(define simulation_timestep_width   0.01)
(define timesteps_per_CFD_episode   10)
(define CFD_post_dir "CFD_start")
(define img-1 "cfd_start_")
(define i 0)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_1)     ;; load all into main folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format  #f "!copy ~a\\~a_INIT.cas.h5 ." ANSYS_Fluent_dir ANSYS_Fluent_file))

    (ti-menu-load-string (format  #f "!copy ~a\\CFD_user.c ." CFD_user_src_dir))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_2)     ;; compile everything into libudf_cfd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!if exist libudf_cfd rmdir /s /q libudf_cfd")

    (ti-menu-load-string "/define/user-defined/compiled-functions compile
                        \"libudf_cfd\" yes
                        \"CFD_user.c\"
                        \"\"
                        \"\"")

    (ti-menu-load-string "!del CFD_user.c")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_3)     ;; load start file, allocate UDMI's and load libudf_cfd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/read-case .\\~a_INIT.cas.h5 OK" ANSYS_Fluent_file))

    (ti-menu-load-string "/define/user-defined/compiled-functions load \"libudf_cfd\"")

    ;(ti-menu-load-string "/solve/initialize/set-defaults k 0.0005" )
    (ti-menu-load-string "/solve/initialize/initialize-flow" )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_5)     ;; run simulation w/o postprocessing
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "/define/user-defined/function-hooks/execute-at-end
                        \"CFD_write_energy::libudf_cfd\"
                         \"\"")

    ;(ti-menu-load-string (format #f "/solve/set/time-step ~d" 0.001))
    ;(ti-menu-load-string (format #f "/solve/dual-time-iterate ~d 30" 10))

    (do  ((i  0 (+ i  1)))
        ((= i  number_of_CFD_episodes))

        (ti-menu-load-string (format #f "/solve/set/time-step ~d" simulation_timestep_width))
        (ti-menu-load-string (format #f "/solve/dual-time-iterate ~d 30" timesteps_per_CFD_episode))
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_6)     ;; write file
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string (format #f "/file/write-case-data .\\~a_START.cas.h5 OK" ANSYS_Fluent_file))
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_7)     ;; clean-up folder
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (ti-menu-load-string "!mkdir post")

    (ti-menu-load-string (format #f "!mkdir post\\~a" CFD_post_dir))

    (ti-menu-load-string (format #f "!move *.out post\\~a" CFD_post_dir))
    (ti-menu-load-string (format #f "!move *.trn post\\~a" CFD_post_dir))
    (ti-menu-load-string (format #f "!move *.jpg post\\~a" CFD_post_dir))

    (ti-menu-load-string "!del tmp*")
    (ti-menu-load-string (format #f "!move *.h5 ~a" ANSYS_Fluent_dir))
    (ti-menu-load-string "!del log")
    (ti-menu-load-string "!del *.flprj")
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (CFD_start)       ;; set up and run cfd simulation w/o post-processing
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (CFD_1)
    (CFD_2)
    (CFD_3)
    ;(CFD_4)
    (CFD_5)
    (CFD_6)
    (CFD_7)
)