/******************************************************************************
 * @internal
 * @file        rCFD_run.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_RUN
#define RCFD_RUN

#include "rCFD_layer.h"
#include "rCFD_post.h"
#include "rCFD_balance.h"

void run_update_state(rCFD_type* rcfd)
{
#if RP_NODE
    rcfd->Solver->current_state = 0;

    rcfd->Solver->upcoming_state = 0;
#endif // RP_NODE
}

void run_update_layer(rCFD_type* rcfd)
{
#if RP_NODE
#if MULTI_LAYER_IMPLEMENTATION_COMPLETE
    int prev_layer = rcfd->Solver->current_layer;

    rcfd->Solver->current_layer = rcfd->rCFD_UDF->_user_set_layer(rcfd, rcfd->Solver->current_layer);

    if(rcfd->Solver->current_layer != prev_layer){

        layer_map_from_to(rcfd, prev_layer, rcfd->Solver->current_layer);
    }
#endif // MULTI_LAYER_IMPLEMENTATION_COMPLETE
#endif // RP_NODE
}

void run_update_frame(rCFD_type* rcfd)
{
#if RP_NODE
#if 1   // local vars
    int     i_state, i_state2, i_layer, i_rec;
    int     i_rec_max;

    double  rand_real;
#endif

    i_state     = rcfd->Solver->current_state;
    i_state2    = rcfd->Solver->upcoming_state;
    i_layer     = rcfd->Solver->current_layer;

    rcfd->Rec->jumped_at_last_frame = 0;

    rcfd->Solver->prev_frame = rcfd->Solver->current_frame;

    if(rcfd->Solver->current_frame == (rcfd->Solver_Dict->number_of_frames - 1)){

        rcfd->Rec->jumped_at_last_frame = 1;
    }

    if(rcfd->Solver_Dict->recurrence_process_on){

        if(rcfd->Solver->global_run_counter == 0){

            if(myid == 0){

                unsigned int seed;

                if(rcfd->Rec_Dict->seed < 0){

                    seed = (unsigned int)time(0);
                }
                else{

                    seed = (unsigned int)rcfd->Rec_Dict->seed;
                }

                srand(seed);

                report_info0(rcfd, RCFD_REPORT_CATEGORY_RUN_UPDATE,
                             "run_update_frame: initialized random generator with seed %u", seed);
            }

            return;
        }

        i_rec_max = 1;

        for(i_rec = 0; i_rec < i_layer; i_rec++){

            i_rec_max *= 2;
        }

        for(i_rec = 0; i_rec < i_rec_max; i_rec++){

            if(rcfd->Rec->frame_in_sequence < rcfd->Rec->sequence_length){

                rcfd->Rec->frame_in_sequence++;

                // set next frame (within sequence)
                if(rcfd->Solver->current_frame < (rcfd->Solver_Dict->number_of_frames - 1)){

                    rcfd->Solver->current_frame++;
                }
                else{

                    rcfd->Solver->current_frame = rcfd->Rec->jumps[i_state][i_state2][(rcfd->Solver_Dict->number_of_frames-1)];
                }
            }
            else{ /* new rCFD_seq */

                rcfd->Rec->frame_in_sequence = 0;

                if(myid == 0){

                    rand_real = (double)rand()/(double)RAND_MAX;

                    rcfd->Rec->sequence_length = rcfd->Rec_Dict->min_seq_length +

                        (int)(rand_real*(double)(rcfd->Rec_Dict->max_seq_length - rcfd->Rec_Dict->min_seq_length));
                }

                PRF_BCAST_INT(&rcfd->Rec->sequence_length, 1, 0);

                /* set next frame (new sequence) */
                rcfd->Solver->current_frame = rcfd->Rec->jumps[i_state][i_state2][rcfd->Solver->current_frame];
            }
        }
    }
    else{

        rcfd->Solver->current_frame = 0;
    }

#if VOF_STITCHING_IMPLEMENTATION_COMPLETE
    /* eventually, redistribute data on stitched vof fields */
    if(rcfd->Rec_Dict->adapt_vof_stitching_on){

        layer_map_data_to_new_vof(rcfd);
    }
#endif // VOF_STITCHING_IMPLEMENTATION_COMPLETE
#endif // RP_NODE
}

void run_update_balance(rCFD_type* rcfd)
{
#if RP_NODE
    int     i_layer, i_phase;

    i_layer = rcfd->Solver->current_layer;

    for(i_phase = 0; i_phase < rcfd->Solver_Dict->number_of_phases; i_phase++){

        balance_init_source(rcfd, i_layer, i_phase);
    }
#endif
}

#endif
