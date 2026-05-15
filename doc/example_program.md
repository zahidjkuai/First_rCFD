## Example of Program Structure {#example_program}

Assuming rCFD has been compiled as a standalone library, a database has been
created and the topology has been written to the corresponding rCFD files,
the following is a minimum example for a standalone C program acting as driver
for the rCFD simulation:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
// include the main rCFD library header
#include "rCFD_library.h"

void my_set_solver_dict(void *handle)
{
    // set the time step for the rCFD simulation (typically the interval used for monitoring CFD)
    rcfd_set_dict_property_r(handle, "Solver_Dict", -1, -1, -1, "global_time_step", 0.005);
    // specify the number of database entries to use
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_frames", 100);
    // specify the number of rCFD time steps to execute in one call to rcfd_run()
    rcfd_set_dict_property_i(handle, "Solver_Dict", -1, -1, -1, "number_of_runs", 100);
}

int main(int argc, char **argv)
{
    void *myrcfd;

    // create a handle for the main rCFD object
    rcfd_open_no_mpi(0, NULL, &myrcfd);

    // register the user-defined function for setting dictionary options
    rcfd_register_user_function(myrcfd, "user_set_solver_dict", (void(*)(void))my_set_solver_dict);

    // initialize the main rCFD object
    rcfd_init(myrcfd, RCFD_MODE_RUN);

    // load the rCFD database
    rcfd_read_db(myrcfd);

    // execute the specified number of rCFD time steps
    rcfd_run(myrcfd);

    // shut down rCFD and clean up
    rcfd_close(myrcfd);

    return 0;
}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

