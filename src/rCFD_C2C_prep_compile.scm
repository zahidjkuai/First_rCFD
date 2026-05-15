;; this sceme file is loaded in the tutorials and will both compile and recompile the rCFD code for prep

(if fluent_has_compiled
    (define tui_compilation_argument  "y")
    (define tui_compilation_argument  "")
)
(if fluent_has_compiled_headers
    (define tui_compilation_argument_headers  "y")
    (define tui_compilation_argument_headers  "")
)

(ti-menu-load-string (format  #f "/define/user-defined/compiled-functions compile
        libudf_rcfd_prep
        yes ~a
        rCFD_library.c
        rCFD_C2C_prep.c
        tomlc17.c
        \"\" ~a
        rCFD.h
        rCFD_balance.h
        rCFD_defaults.h
        rCFD_dict.h
        rCFD_dict_read_toml.h
        rCFD_dict_write_toml.h
        rCFD_dump.h
        rCFD_fluent.h
        rCFD_free.h
        rCFD_globals.h
        rCFD_init.h
        rCFD_layer.h
        rCFD_library.h
        rCFD_macros.h
        rCFD_memory.h
        rCFD_parallel.h
        rCFD_post.h
        rCFD_prepare.h
        rCFD_report.h
        rCFD_run.h
        rCFD_shift.h
        rCFD_swap.h
        rCFD_topo.h
        rCFD_types.h
        rCFD_user.h
        rCFD_user_defaults.h
        rCFD_utilities.h
        tomlc17.h
        \"\" " tui_compilation_argument tui_compilation_argument_headers) )

(set! fluent_has_compiled #t)
(set! fluent_has_compiled_headers #t)

