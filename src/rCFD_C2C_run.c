/******************************************************************************
 * @internal
 * @file        rCFD_C2C_run.c
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#include "udf.h"
#include "rCFD_library.h"
#include "rCFD_fluent.h"
#include "rCFD_user.h"


/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_init_all)
/*************************************************************************************/
{
    fluent_init_topo();

    rcfd_open_no_mpi(0, NULL, &myrcfd);

    rcfd_register_user_function(myrcfd, "user_set_node_dict",   (void(*)(void))fluent_set_node_dict);
    rcfd_register_user_function(myrcfd, "user_init_cells",      (void(*)(void))fluent_init_cells);
    rcfd_register_user_function(myrcfd, "user_init_faces",      (void(*)(void))fluent_init_faces);
    rcfd_register_user_function(myrcfd, "user_init_cell_hosts", (void(*)(void))fluent_init_cell_hosts);
    rcfd_register_user_function(myrcfd, "user_init_nodes",      (void(*)(void))fluent_init_nodes);

    user_register_functions(myrcfd); // defined in rCFD_user.h

    rcfd_init(myrcfd, RCFD_MODE_RUN);
}

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_read_DB)
/*************************************************************************************/
{
    rcfd_read_db(myrcfd);
}

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_run)
/*************************************************************************************/
{
    rcfd_run(myrcfd);
}

/*************************************************************************************/
DEFINE_ON_DEMAND(rCFD_free_all)
/*************************************************************************************/
{
    rcfd_close(myrcfd);

    fluent_free_topo();
}
/*****************************************************************************/
/* Execute-on-demand: inject UDM-0 = 1 inside circle (extruded in z)        */
/* Circle centre (0.035, 0.035) m, radius 0.005 m, full 5 mm depth          */
/*****************************************************************************/
DEFINE_ON_DEMAND(inject_circle_udm)
{
    Domain *d = Get_Domain(1);
    Thread *t;
    cell_t c;
    real x[ND_ND];
    real dist;

    Message("\n>>> Injecting UDM-0: cylinder at (0.035,0.035) m, r=0.005 m <<<\n");

    thread_loop_c(t, d)
    {
        begin_c_loop(c, t)
        {
            C_CENTROID(x, c, t);
            dist = sqrt((x[0]-0.035)*(x[0]-0.035) + (x[1]-0.035)*(x[1]-0.035));
            if (dist <= 0.005)
                C_UDMI(c, t, 0) = 1.0;
            else
                C_UDMI(c, t, 0) = 0.0;
        }
        end_c_loop(c, t)
    }
}