/******************************************************************************
 * @internal
 * @file        rCFD_post.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_POST
#define RCFD_POST

#include <time.h>
#include "rCFD_types.h"
#include "rCFD_macros.h"


    void post_init_all(rCFD_type* rcfd)
    {
#if RP_NODE

        report_stop_timer(rcfd, RCFD_TIMER_INIT);

        report_info0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                     "post_init_all in %.3e %s",
                     report_get_timer_total(rcfd, RCFD_TIMER_INIT),
                     report_get_time_unit_name(rcfd));
#endif
    }

    void post_read(rCFD_type* rcfd)
    {
#if RP_NODE

        report_stop_timer(rcfd, RCFD_TIMER_READ_DB);

#endif
    }

    void post_read_shift(rCFD_type* rcfd)
    {
#if RP_NODE

        report_stop_timer(rcfd, RCFD_TIMER_READ_SHIFT);

        if(rcfd->Solver_Dict->profiling_on){

            report_info0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                          "post_read_shift: read %d C2Cs from %s in %.3e %s",
                          rcfd->Solver->total_shift_read_count,
                          rcfd->File_Dict->C2C_filename,
                          report_get_timer_total(rcfd, RCFD_TIMER_READ_SHIFT),
                          report_get_time_unit_name(rcfd));
        }
        else{

            report_info0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                          "post_read_shift: read %d C2Cs from %s",
                          rcfd->Solver->total_shift_read_count,
                          rcfd->File_Dict->C2C_filename);
        }

#endif
    }

    void post_run(rCFD_type* rcfd)
    {
#if RP_NODE

        report_stop_timer(rcfd, RCFD_TIMER_RUN);

        if(rcfd->Solver_Dict->profiling_on){

            double global_time_all = PRF_GRHIGH1(report_get_timer_total(rcfd, RCFD_TIMER_GLOBAL));

            report_info0(rcfd, RCFD_REPORT_CATEGORY_RUN,
                          "post_run: global run counter %d (of %d), global time %e and global compute time %.3e %s",
                          rcfd->Solver->global_run_counter,
                          rcfd->Solver_Dict->number_of_runs,
                          rcfd->Solver_Dict->global_time_step * rcfd->Solver->global_run_counter,
                          global_time_all,
                          report_get_time_unit_name(rcfd));
        }
        else{

            report_info0(rcfd, RCFD_REPORT_CATEGORY_RUN,
                          "post_run: global run counter %d (of %d), global time %.3e",
                          rcfd->Solver->global_run_counter,
                          rcfd->Solver_Dict->number_of_runs,
                          rcfd->Solver_Dict->global_time_step * rcfd->Solver->global_run_counter);
        }

#endif
    }

    void post_run_update_frame(rCFD_type* rcfd)
    {
#if RP_NODE
        FILE    *f_out = NULL;

        report_stop_timer(rcfd, RCFD_TIMER_RUN_UPDATE);

        /* write file */
        if((rcfd->Rec_Dict->monitor_rec_frames_on) && (myid == 0)){

            // open file
            {
                if(rcfd->Solver->rec_frames_monitor_file_opened == 0){

                    f_out = fopen(rcfd->File_Dict->Rec_Frames_filename, "w");

                    rcfd->Solver->rec_frames_monitor_file_opened = 1;
                }
                else{

                    f_out = fopen(rcfd->File_Dict->Rec_Frames_filename, "a");
                }
            }

            if(f_out){

                fprintf(f_out, "%d ", rcfd->Solver->current_frame);

                fprintf(f_out, "\n");

                fclose(f_out);
            }
        }

        report_info0(rcfd, RCFD_REPORT_CATEGORY_RUN_UPDATE,
                     "post_run_update_frame: global_run_counter %d, i_frame %d, i_state %d",
                     rcfd->Solver->global_run_counter, rcfd->Solver->current_frame, rcfd->Solver->current_state);

#endif
    }

    void post_run_shift(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

        report_stop_timer(rcfd, RCFD_TIMER_RUN_SHIFT);

        report_info0(rcfd, RCFD_REPORT_CATEGORY_RUN_SHIFT,
                     "post_run_shift: global_run_counter %d, i_phase %d",
                     rcfd->Solver->global_run_counter, i_phase);
#endif
    }

    void post_run_swap(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

        report_stop_timer(rcfd, RCFD_TIMER_RUN_SWAP);

        report_info0(rcfd, RCFD_REPORT_CATEGORY_RUN_SWAP,
                     "post_run_swap: global_run_counter %d, i_phase %d",
                     rcfd->Solver->global_run_counter, i_phase);
#endif
    }

    void post_run_balance(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

        report_stop_timer(rcfd, RCFD_TIMER_RUN_BALANCE);

#if 1   // local vars

        int     i_data;
        FILE    *f_balance = NULL;

#endif

        if(myid == 0){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                if( (rcfd->Balance_Dict[i_phase][i_data].write_balance_to_file) &&
                    ((rcfd->Solver->global_run_counter % rcfd->Balance_Dict[i_phase][i_data].write_balance_to_file_interval) == 0)){

                    // open file
                    {
                        if(rcfd->Solver->balance_file_opened == 0){

                            f_balance = fopen(rcfd->File_Dict->Balance_filename, "w");

                            rcfd->Solver->balance_file_opened = 1;
                        }
                        else{

                            f_balance = fopen(rcfd->File_Dict->Balance_filename, "a");
                        }
                    }

                    // write balance and close file
                    if(f_balance){

                        fprintf(f_balance, "%d, %d, %d, %e, %e\n",
                                rcfd->Solver->global_run_counter,
                                i_phase,
                                i_data,
                                rcfd->Balance[i_phase][i_data].mass_integral_global,
                                rcfd->Balance[i_phase][i_data].mass_integral_target_global);

                        fclose(f_balance);
                    }
                }
            }
        }

        report_info0(rcfd, RCFD_REPORT_CATEGORY_RUN_BALANCE,
                     "post_run_balance: global_run_counter %d, i_phase %d",
                     rcfd->Solver->global_run_counter, i_phase);
#endif
    }

    void post_free(rCFD_type* rcfd, int mode)
    {
#if RP_NODE
        report_stop_timer(rcfd, RCFD_TIMER_GLOBAL);

        report_summary(rcfd);

        if(myid == 0){

            time_t  current_time = time(NULL);
            char    *c_time_string = ctime(&current_time);

            report_print0(rcfd, "\n...rCFD_%s finished at %s",
                          (mode == RCFD_MODE_RUN) ? "run" : "prep",
                          c_time_string);
        }
#endif
    }

#endif
