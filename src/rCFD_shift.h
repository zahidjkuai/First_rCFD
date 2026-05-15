/******************************************************************************
 * @internal
 * @file        rCFD_shift.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_SHIFT
#define RCFD_SHIFT

#include <stdio.h>
#include <stdlib.h> // strtol()
#include <ctype.h> // isspace()
#include "rCFD_post.h"
#include "rCFD_utilities.h"
#include "tomlc17.h"

int shift_initialize_lists(rCFD_type* rcfd, C2C_Shift_type *shifts);
int shift_allocate_lists  (rCFD_type* rcfd, C2C_Shift_type *shifts, int shifts_count, int shifts_type);
int shift_allocate_run    (rCFD_type* rcfd);

int shift_copy_one_in_list     (rCFD_type* rcfd,
                                C2C_Shift_type *dest,   int dest_index,
                                C2C_Shift_type *source, int source_index,
                                int shift_type);
int shift_copy_sequence_in_list(rCFD_type* rcfd,
                                C2C_Shift_type *dest,   int dest_index,
                                C2C_Shift_type *source, int source_index, int sequence_len,
                                int shift_type);

int shift_is_index_valid (rCFD_type* rcfd, C2C_Shift_type *shift, int i_shift, int shift_type);
int shift_is_valid       (rCFD_type* rcfd, C2C_Shift_type *shift, int shift_type);
int shifts_get_node0     (rCFD_type* rcfd, C2C_Shift_type *shifts, int i_shift);

void shift_read      (rCFD_type* rcfd);
void shift_local_data(rCFD_type* rcfd, int i_state, int i_phase, int i_frame, int i_layer);
void shift_data      (rCFD_type* rcfd, const int i_phase);
void shift_fill      (rCFD_type* rcfd, const int i_phase);
int  shift_write     (rCFD_type* rcfd, int i_state, int i_frame, int i_phase, int i_layer);

void parallel_shift_data          (rCFD_type* rcfd, int i_state, int i_phase, int i_frame, int i_layer);
int  parallel_map_phase_data_shift(rCFD_type* rcfd, int i_phase);
int  parallel_map_phase_data      (rCFD_type* rcfd, int i_phase);

/**
 * @brief Initializes all members of shift structure to NULL or 0
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param shifts    pointer to shift structure
 * @return 0 if success or -1 if error
 */
int shift_initialize_lists(rCFD_type* rcfd, C2C_Shift_type* shifts)
{
#if RP_NODE
    if(shifts == NULL)
        return -1;

    shifts->c0 =        NULL;
    shifts->node0 =     NULL;
    shifts->node0char = NULL;
    shifts->c1 =        NULL;
    shifts->node1 =     NULL;
    shifts->w0 =        NULL;
    shifts->w0char =    NULL;

    shifts->dist_sq =   NULL;
    shifts->c0_coords = NULL;
    shifts->displ =     NULL;

    shifts->shifts_count = 0;
    shifts->valid_shifts_count = 0;

    return 0;
#endif

    return -1;
}

/**
 * @brief Allocates memory for member lists of a specific cell-to-cell shift struct
 *
 * @param rcfd           pointer to previously created rCFD instance
 * @param shifts         pointer to shifts to be initialized
 * @param shifts_count   length of list of shifts
 * @return 0 if success or -1 if error
 */
int shift_allocate_lists(rCFD_type* rcfd, C2C_Shift_type* shifts, int shifts_count, int shifts_type)
{
#if RP_NODE

    shift_initialize_lists(rcfd, shifts);

    if(shifts_count < 0){

        return -1;
    }

    if(shifts_count == 0){

        return 0;
    }

    shifts->shifts_count = shifts_count;

    if((shifts_type == RCFD_SHIFT_TYPE_PREPARATION) ||
       (shifts_type == RCFD_SHIFT_TYPE_ANY)){

        shifts->c0      = malloc(shifts_count * sizeof(int));
        shifts->node0   = malloc(shifts_count * sizeof(int));
        shifts->c1      = malloc(shifts_count * sizeof(int));
        shifts->node1   = malloc(shifts_count * sizeof(int));
        shifts->w0      = malloc(shifts_count * sizeof(double));

        if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

            shifts->dist_sq     = malloc(shifts_count * sizeof(double));
            shifts->c0_coords   = malloc_r_2d(shifts_count, 3);
            shifts->displ       = malloc_r_2d(shifts_count, 3);
        }

        return 0;
    }

    int shift_format = rcfd->Solver->c2c_shift_format;

    if(shifts_type == RCFD_SHIFT_TYPE_INTERIOR){

        shifts->c0 = malloc(shifts_count * sizeof(int));
        shifts->c1 = malloc(shifts_count * sizeof(int));

        switch(shift_format){

            case RCFD_SHIFT_C0_N0_C1_N1_W0:
            case RCFD_SHIFT_C0_N0_C1_W0:
            {
                shifts->w0 = malloc(shifts_count * sizeof(double));
                return 0;
            }
            case RCFD_SHIFT_C0_N0_C1_W0CHAR:
            case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
            {
                shifts->w0char = malloc(shifts_count * sizeof(unsigned char));
                return 0;
            }
            default:
                return -1;
        }
    }

    if(shifts_type == RCFD_SHIFT_TYPE_INTER_NODE){

        shifts->c0 = malloc(shifts_count * sizeof(int));
        shifts->c1 = malloc(shifts_count * sizeof(int));

        switch(shift_format){

            case RCFD_SHIFT_C0_N0_C1_N1_W0:
            {
                shifts->node0 = malloc(shifts_count * sizeof(int));
                shifts->node1 = malloc(shifts_count * sizeof(int));
                shifts->w0 =    malloc(shifts_count * sizeof(double));

                return 0;
            }
            case RCFD_SHIFT_C0_N0_C1_W0:
            {
                shifts->node0 = malloc(shifts_count * sizeof(int));
                shifts->w0 =    malloc(shifts_count * sizeof(double));

                return 0;
            }
            case RCFD_SHIFT_C0_N0_C1_W0CHAR:
            {
                shifts->node0 =  malloc(shifts_count * sizeof(int));
                shifts->w0char = malloc(shifts_count * sizeof(unsigned char));

                return 0;
            }
            case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
            {
                shifts->node0char = malloc(shifts_count * sizeof(unsigned char));
                shifts->w0char =    malloc(shifts_count * sizeof(unsigned char));

                return 0;
            }
            default:
                return -1;
        }
    }

#endif
    return -1;
}

/**
 * @brief Allocates memory for cell-to-cell shifts for RCFD_MODE_RUN
 *
 * @param rcfd   pointer to previously created rCFD instance
 * @return 0 if success or -1 if error
 */
int shift_allocate_run(rCFD_type *rcfd)
{
#if RP_NODE
    rcfd->C2C = NULL;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_RUN){

        int state_count =  rcfd->Solver_Dict->number_of_states;
        int phase_count =  rcfd->Solver_Dict->number_of_phases;
        int frame_count =  rcfd->Solver_Dict->number_of_frames;
        int layer_count =  rcfd->Solver_Dict->number_of_layers;

        if((state_count  <= 0) ||
           (phase_count  <= 0) ||
           (frame_count  <= 0) ||
           (layer_count  <= 0)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                "[%d] shift_allocate_run: invalid counters detected",
                myid);

            return -1;
        }

        rcfd->C2C = malloc(state_count * sizeof(C2C_type***));

        int i_state, i_phase, i_frame, i_layer;

        for(i_state = 0; i_state < state_count; i_state++){

            rcfd->C2C[i_state] = malloc(phase_count * sizeof(C2C_type**));

            for(i_phase = 0; i_phase < phase_count; i_phase++){

                rcfd->C2C[i_state][i_phase] = malloc(frame_count * sizeof(C2C_type*));

                for(i_frame = 0; i_frame < frame_count; i_frame++){

                    rcfd->C2C[i_state][i_phase][i_frame] = malloc(layer_count * sizeof(C2C_type));

                    for(i_layer = 0; i_layer < layer_count; i_layer++){

                        rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts_to_node_count = malloc(compute_node_count * sizeof(int));
                    }
                }
            }
        }
    }

    return 0;
#endif
    return -1;
}

/**
 * @brief Copies the values of one cell-to-cell shift entry from the \p source structure to the \p dest structure
 *
 * @param rcfd          pointer to previously created rCFD instance
 * @param dest          pointer to destination shift structure
 * @param dest_index    index in \p dest shift array members (existing entries will be overwritten)
 * @param source        pointer to source shift structure
 * @param source_index  index in \p source shift array members
 */
int shift_copy_one_in_list(rCFD_type* rcfd, C2C_Shift_type *dest, int dest_index, C2C_Shift_type *source, int source_index, int shift_type)
{
#if RP_NODE

    if(!dest) return -1;
    if(!source) return -1;

    if( (dest_index   >= dest->   shifts_count) ||
        (source_index >= source-> shifts_count) ||
        (dest_index   <  0) ||
        (source_index <  0)){

        return -1;
    }

    if(shift_type == RCFD_SHIFT_TYPE_PREPARATION){

        dest->c0     [dest_index] = source->c0     [source_index];
        dest->node0  [dest_index] = source->node0  [source_index];
        dest->c1     [dest_index] = source->c1     [source_index];
        dest->node1  [dest_index] = source->node1  [source_index];
        dest->w0     [dest_index] = source->w0     [source_index];
        dest->dist_sq[dest_index] = source->dist_sq[source_index];

        memcpy(&dest->c0_coords[dest_index][0], &source->c0_coords[source_index][0], 3 * sizeof(double));
        memcpy(&dest->displ    [dest_index][0], &source->displ    [source_index][0], 3 * sizeof(double));

        return 0;
    }

    if(shift_type == RCFD_SHIFT_TYPE_INTER_NODE){

        int shift_format = rcfd->Solver->c2c_shift_format;

        dest->c0[dest_index] = source->c0[source_index];
        dest->c1[dest_index] = source->c1[source_index];

        if( (shift_format == RCFD_SHIFT_C0_N0_C1_N1_W0)  ||
            (shift_format == RCFD_SHIFT_C0_N0_C1_W0)     ||
            (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR)){

            dest->node0[dest_index] = source->node0[source_index];
        }
        else{

            dest->node0char[dest_index] = source->node0char[source_index];
        }

        if( (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR)     ||
            (shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            dest->w0char[dest_index] = source->w0char[source_index];
        }
        else{

            dest->w0[dest_index] = source->w0[source_index];
        }

        return 0;
    }

#endif
    return -1;
}

/**
 * @brief Copies sequence cell-to-cell shifts from source list to destination list
 *
 * @param rcfd          pointer to previously created rCFD instance
 * @param dest          destination shift list
 * @param dest_index    start index in destination shift list
 * @param source        source shift list
 * @param source_index  start index in source shift list
 * @param sequence_len  length of sequence
 * @retval -1 Error
 * @retval 0 OK
 */
int shift_copy_sequence_in_list(rCFD_type* rcfd,
                                C2C_Shift_type *dest, int dest_index,
                                C2C_Shift_type *source, int source_index,
                                int sequence_len,
                                int shift_type)
{
#if RP_NODE

    if(!dest) return -1;
    if(!source) return -1;

    if( ((dest_index   + sequence_len) > dest->   shifts_count) ||
        ((source_index + sequence_len) > source-> shifts_count) ||
        (dest_index   <  0) ||
        (source_index <  0) ||
        (sequence_len <  0)){

        return -1;
    }

    if(sequence_len == 0)   return 0;

    if(shift_type == RCFD_SHIFT_TYPE_PREPARATION){

        memcpy(&dest->c0     [dest_index], &source->c0     [source_index], sequence_len * sizeof(int));
        memcpy(&dest->node0  [dest_index], &source->node0  [source_index], sequence_len * sizeof(int));
        memcpy(&dest->c1     [dest_index], &source->c1     [source_index], sequence_len * sizeof(int));
        memcpy(&dest->node1  [dest_index], &source->node1  [source_index], sequence_len * sizeof(int));
        memcpy(&dest->w0     [dest_index], &source->w0     [source_index], sequence_len * sizeof(double));
        memcpy(&dest->dist_sq[dest_index], &source->dist_sq[source_index], sequence_len * sizeof(double));

        memcpy(&dest->c0_coords[dest_index][0], &source->c0_coords[source_index][0], 3 * sequence_len * sizeof(double));
        memcpy(&dest->displ    [dest_index][0], &source->displ    [source_index][0], 3 * sequence_len * sizeof(double));

        return 0;
    }

#endif
    return -1;
}

/**
 * @brief Checks if one shift entry is valid
 *
 * @param rcfd          pointer to previously created rCFD instance
 * @param shift         shift structure to be checked
 * @param i_shift       index of shift entry to be checked
 * @param shift_type    shift type
 * @return 1 if valid shift or 0 if invalid or called by host
 */
int shift_is_index_valid(rCFD_type* rcfd, C2C_Shift_type *shift, int i_shift, int shift_type)
{
#if RP_NODE

    if(!shift) return 0;

    if( (shift->shifts_count <= 0) ||
        (shift->valid_shifts_count < 0) ||
        (shift->shifts_count < shift->valid_shifts_count))  return 0;

    if( (i_shift < 0) ||
        (i_shift >= shift->valid_shifts_count)) return 0;

    if(shift_type == RCFD_SHIFT_TYPE_PREPARATION){

        int i_layer = rcfd->Solver->current_layer;
        int cell_count = _CELL_DICT(i_layer).number_of_cells;

        if((shift->c0     [i_shift] < 0) || (shift->c0   [i_shift] >= cell_count        )) return 0;
        if((shift->node0  [i_shift] < 0) || (shift->node0[i_shift] >= compute_node_count)) return 0;
        if((shift->c1     [i_shift] < 0) || (shift->c1   [i_shift] >= cell_count        )) return 0;
        if((shift->node1  [i_shift] < 0) || (shift->node1[i_shift] >= compute_node_count)) return 0;
        if (shift->w0     [i_shift] < 0.0) return 0;
        if (shift->dist_sq[i_shift] < 0.0) return 0;

        // no checks for co_coords, displ

        return 1;
    }
#endif
    return 0;
}

/**
 * @brief Checks if shift structure is non-zero and has valid entries
 *
 * @param rcfd          pointer to previously created rCFD instance
 * @param shift         shift structure to be checked
 * @param shift_type    shift type
 * @return 1 if valid shift or 0 if invalid or called by host
 */
int shift_is_valid(rCFD_type* rcfd, C2C_Shift_type *shift, int shift_type)
{
#if RP_NODE

    if(!shift) return 0;

    if( (shift->shifts_count <= 0) ||
        (shift->valid_shifts_count < 0) ||
        (shift->shifts_count < shift->valid_shifts_count))  return 0;

    if(shift_type == RCFD_SHIFT_TYPE_PREPARATION){

        if(!shift->c0     ) return 0;
        if(!shift->node0  ) return 0;
        if(!shift->c1     ) return 0;
        if(!shift->node1  ) return 0;
        if(!shift->w0     ) return 0;
        if(!shift->dist_sq) return 0;

        if(!shift->c0_coords   ) return 0;
        if(!shift->displ       ) return 0;
        if(!shift->c0_coords[0]) return 0;
        if(!shift->displ[0]    ) return 0;

        int i_shift;

        for(i_shift = 0; i_shift < shift->valid_shifts_count; i_shift++){

            if(!shift_is_index_valid(rcfd, shift, i_shift, shift_type)) return 0;
        }

        return 1;
    }
#endif
    return 0;
}

int shifts_get_node0(rCFD_type* rcfd, C2C_Shift_type *shifts, int i_shift)
{
#if RP_NODE

    if(shifts == NULL) return -1;
    if((i_shift < 0) || (i_shift >= shifts->shifts_count)) return -1;

    int shift_format = rcfd->Solver->c2c_shift_format;

    if( (shift_format == RCFD_SHIFT_C0_N0_C1_N1_W0)  ||
        (shift_format == RCFD_SHIFT_C0_N0_C1_W0)     ||
        (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR)){

        return shifts->node0[i_shift];
    }
    else{

        return (int)shifts->node0char[i_shift];
    }
#endif
    return -1;
}

/**
 * @brief Reads cell-to-cell shifts of one frame from the given file and stores
 *      them in \p frame_shifts and \p frame_shifts_in
 *
 * @param rcfd            pointer to previously created rCFD instance
 * @param fi              pointer to open c2c file
 * @param frame_shifts    pointer to intra-node shifts
 * @param frame_shifts_in pointer to inter-node shifts
 * @return number of read shifts or -1 if error or called by host
 */
int read_shifts_of_frame(rCFD_type* rcfd, FILE *fi, C2C_Shift_type* frame_shifts, C2C_Shift_type* frame_shifts_in)
{
#if RP_NODE

#if 1 // local vars (with initializations)
    int     i_shift;

    int     i_read, i_read_inside, i_read_to;

    int     c0, node0, c1, node1, w0int;

    int     file_type = rcfd->Solver->c2c_file_type;
    int     shift_format = rcfd->Solver->c2c_shift_format;
    int     shift_types = rcfd->Solver->c2c_shift_type_count;

    int     shifts_count, shifts_inside_node_count, shifts_to_node_count;

    double  w0;

    C2C_Shift_type  tmp_frame_shifts;
#endif

    switch(file_type){

        case RCFD_FILE_ASCII:
        {
            // read per-frame header line
            switch(shift_types){

                case 1:
                {
                    fscanf(fi, "%d \n", &shifts_count);
                    break;
                }
                case 2:
                {
                    fscanf(fi, "%d %d\n", &shifts_inside_node_count, &shifts_to_node_count);
                    shifts_count = shifts_inside_node_count + shifts_to_node_count;
                    break;
                }
                default:
                    return -1;
            }

            // allocate frame_shifts and init their shifts_count
            switch(shift_format){

                case RCFD_SHIFT_C0_N0_C1_N1_W0:
                {
                    shift_allocate_lists(rcfd, &tmp_frame_shifts, shifts_count, RCFD_SHIFT_TYPE_ANY);
                    break;
                }
                case RCFD_SHIFT_C0_N0_C1_W0:
                case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                {
                    shift_allocate_lists(rcfd, frame_shifts,    shifts_inside_node_count, RCFD_SHIFT_TYPE_INTERIOR);
                    shift_allocate_lists(rcfd, frame_shifts_in, shifts_to_node_count    , RCFD_SHIFT_TYPE_INCOMING);
                    break;
                }
                default:
                    return -1;
            }

            // fill frame_shifts(_in)
            switch (shift_format){

                case RCFD_SHIFT_C0_N0_C1_N1_W0:
                {
                    // read unsorted shifts into tmp_frame_shifts
                    {
                        i_read_inside = 0;
                        i_read_to =     0;
                        i_read =        0;

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            fscanf(fi, "%d %d %d %d %le\n", &c0, &node0, &c1, &node1, &w0);

                            tmp_frame_shifts.c0   [i_read] = c0;
                            tmp_frame_shifts.node0[i_read] = node0;
                            tmp_frame_shifts.c1   [i_read] = c1;
                            tmp_frame_shifts.node1[i_read] = node1;
                            tmp_frame_shifts.w0   [i_read] = w0;

                            if(node0 == node1){

                                i_read_inside++;
                            }
                            else{

                                i_read_to++;
                            }

                            i_read++;
                        }

                        shifts_inside_node_count = i_read_inside;
                        shifts_to_node_count =     i_read_to;

                    }

                    // write sorted shifts into frame_shifts and fram_shifts_in
                    {
                        shift_allocate_lists(rcfd, frame_shifts,    shifts_inside_node_count, RCFD_SHIFT_TYPE_INTERIOR);
                        shift_allocate_lists(rcfd, frame_shifts_in, shifts_to_node_count    , RCFD_SHIFT_TYPE_INCOMING);

                        i_read_inside = 0;
                        i_read_to =     0;

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            node0 = tmp_frame_shifts.node0[i_shift];
                            node1 = tmp_frame_shifts.node1[i_shift];

                            if(node0 == node1){

                                frame_shifts->c0   [i_read_inside] = tmp_frame_shifts.c0   [i_shift];
                                frame_shifts->c1   [i_read_inside] = tmp_frame_shifts.c1   [i_shift];
                                frame_shifts->w0   [i_read_inside] = tmp_frame_shifts.w0   [i_shift];

                                i_read_inside++;
                            }
                            else{

                                frame_shifts_in->c0   [i_read_to] = tmp_frame_shifts.c0   [i_shift];
                                frame_shifts_in->node0[i_read_to] = tmp_frame_shifts.node0[i_shift];
                                frame_shifts_in->c1   [i_read_to] = tmp_frame_shifts.c1   [i_shift];
                                frame_shifts_in->node1[i_read_to] = tmp_frame_shifts.node1[i_shift];
                                frame_shifts_in->w0   [i_read_to] = tmp_frame_shifts.w0   [i_shift];

                                i_read_to++;
                            }
                        }
                    }

                    break;
                }
                case RCFD_SHIFT_C0_N0_C1_W0:
                {
                    for(i_shift = 0; i_shift < shifts_inside_node_count; i_shift++){

                        fscanf(fi,"%d %d %le\n", &c0, &c1, &w0);

                        frame_shifts->c0 [i_shift] = c0;
                        frame_shifts->c1 [i_shift] = c1;
                        frame_shifts->w0 [i_shift] = w0;
                    }

                    for(i_shift = 0; i_shift < shifts_to_node_count; i_shift++){

                        fscanf(fi,"%d %d %d %le\n", &c0, &node0, &c1, &w0);

                        frame_shifts_in->c0    [i_shift] = c0;
                        frame_shifts_in->node0 [i_shift] = node0;
                        frame_shifts_in->c1    [i_shift] = c1;
                        frame_shifts_in->w0    [i_shift] = w0;
                    }

                    break;
                }
                case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                {
                    for(i_shift = 0; i_shift < shifts_inside_node_count; i_shift++){

                        fscanf(fi,"%d %d %d\n", &c0, &c1, &w0int);

                        frame_shifts->c0     [i_shift] = c0;
                        frame_shifts->c1     [i_shift] = c1;
                        frame_shifts->w0char [i_shift] = (unsigned char)w0int;
                    }

                    for(i_shift = 0; i_shift < shifts_to_node_count; i_shift++){

                        fscanf(fi,"%d %d %d %d\n", &c0, &node0, &c1, &w0int);

                        frame_shifts_in->c0     [i_shift] = c0;
                        frame_shifts_in->node0  [i_shift] = node0;
                        frame_shifts_in->c1     [i_shift] = c1;
                        frame_shifts_in->w0char [i_shift] = (unsigned char)w0int;
                    }

                    break;
                }
                case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                {
                    for(i_shift = 0; i_shift < shifts_inside_node_count; i_shift++){

                        fscanf(fi,"%d %d %d\n", &c0, &c1, &w0int);

                        frame_shifts->c0     [i_shift] = c0;
                        frame_shifts->c1     [i_shift] = c1;
                        frame_shifts->w0char [i_shift] = (unsigned char)w0int;
                    }

                    for(i_shift = 0; i_shift < shifts_to_node_count; i_shift++){

                        fscanf(fi,"%d %d %d %d\n", &c0, &node0, &c1, &w0int);

                        frame_shifts_in->c0        [i_shift] = c0;
                        frame_shifts_in->node0char [i_shift] = (unsigned char)node0;
                        frame_shifts_in->c1        [i_shift] = c1;
                        frame_shifts_in->w0char    [i_shift] = (unsigned char)w0int;
                    }

                    break;
                }
                default:
                    return -1;
            }

            break;
        }
        case RCFD_FILE_BINARY:
        {
            // read per-frame header line
            switch (shift_format){

                case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                {
                    fread(&shifts_inside_node_count, sizeof(int), 1, fi);
                    fread(&shifts_to_node_count,     sizeof(int), 1, fi);

                    shifts_count = shifts_inside_node_count + shifts_to_node_count;

                    break;
                }
                default:
                    return -1;
            }

            // allocate frame_shifts and init their shifts_count
            switch (shift_format){

                case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                {
                    shift_allocate_lists(rcfd, frame_shifts,    shifts_inside_node_count, RCFD_SHIFT_TYPE_INTERIOR);
                    shift_allocate_lists(rcfd, frame_shifts_in, shifts_to_node_count    , RCFD_SHIFT_TYPE_INCOMING);

                    break;
                }
                default:
                    return -1;
            }

            // fill interior frame_shifts
            switch (shift_format){

                case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                {
                    for(i_shift = 0; i_shift < shifts_inside_node_count; i_shift++){

                        fread(&frame_shifts->c0     [i_shift], sizeof(int),           1, fi);
                        fread(&frame_shifts->c1     [i_shift], sizeof(int),           1, fi);
                        fread(&frame_shifts->w0char [i_shift], sizeof(unsigned char), 1, fi);
                    }

                    break;
                }
                default:
                    return -1;
            }

            // fill inter-node frame_shifts_in
            switch (shift_format){

                case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                {
                    for(i_shift = 0; i_shift < shifts_to_node_count; i_shift++){

                        fread(&frame_shifts_in->c0    [i_shift], sizeof(int),           1, fi);
                        fread(&frame_shifts_in->node0 [i_shift], sizeof(int),           1, fi);
                        fread(&frame_shifts_in->c1    [i_shift], sizeof(int),           1, fi);
                        fread(&frame_shifts_in->w0char[i_shift], sizeof(unsigned char), 1, fi);
                    }

                    break;
                }
                case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                {
                    for(i_shift = 0; i_shift < shifts_to_node_count; i_shift++){

                        fread(&frame_shifts_in->c0        [i_shift], sizeof(int),           1, fi);
                        fread(&frame_shifts_in->node0char [i_shift], sizeof(unsigned char), 1, fi);
                        fread(&frame_shifts_in->c1        [i_shift], sizeof(int),           1, fi);
                        fread(&frame_shifts_in->w0char    [i_shift], sizeof(unsigned char), 1, fi);
                    }

                    break;
                }
                default:
                    return -1;
            }

            break;
        }
        default:
            return -1;
    }

    return shifts_count;
#endif
    return -1;
}

/**
 * @brief get metadata information for c2c shift files
 * @param rcfd              pointer to previously created rCFD instance
 * @param[out] file_type    receives file type (ascii or binary)
 * @param[out] shift_format receives shift format used in file
 * @param[out] shift_types  receives shift type count, i.e. 1 common or 2 separate
 *                          shift formats for interior and inter-node shifts (where
 *                          the interior format is the same as the inter-node format
 *                          except that it omits the n0 component)
 * @return 0 on success, else non-zero error code
 * @todo    replace primitive file parsing in this function by reading metadata
 *          file at startup via toml parser and retrieve values from toml tree
 */
int shift_get_c2c_metadata(rCFD_type *rcfd, int *file_type, int *shift_format, int *shift_types)
{
    int error = 0;
    // defaults if there is no metadata file
    *file_type = RCFD_FILE_ASCII;
    *shift_format = RCFD_SHIFT_C0_N0_C1_N1_W0;
    *shift_types = 1;

    if(myid == 0){

        const char *metadata_filename = rcfd->File_Dict->metadata_filename;

#ifdef TOMLC17_H
        do{
            // if we need metadata from this file not related to c2c shifts
            // read in the file at a more central place and keep it in the
            // rCFD_struct object like the config file data
            FILE *fmeta = fopen(metadata_filename, "r");

            if(!fmeta){

                break; // assuming legacy case
            }

            toml_result_t metadata = toml_parse_file_ex(metadata_filename);

            fclose(fmeta);

            if(!metadata.ok){

                report_error(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                             "[%d] %s", myid, metadata.errmsg);
                toml_free(metadata);
                error =  -1;
                break;
            }

            const char* dict = "database";
            toml_datum_t database = toml_get(metadata.toptab, dict);

            if(database.type == TOML_ARRAY){

                database = database.u.arr.elem[0];
            }

            if(database.type != TOML_TABLE){

                report_debug0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                              "'%s' dict does not exist"
                              " or is not a table in metadata file",
                              dict);
                toml_free(metadata);
                error = -1;
                break;
            }

            toml_datum_t property = toml_get(database, "partitions");

            if(property.type == TOML_INT64){

                if(compute_node_count != property.u.int64){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                  "shift_get_c2c_metadata: database was prepared with different partitions");
                    toml_free(metadata);
                    error = -1;
                    break;
                }
            }
            else{

                report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                              "shift_get_c2c_metadata: missing or invalid partitions entry for database");
                toml_free(metadata);
                error = -1;
                break;
            }

            property = toml_get(database, "frames");

            if(property.type == TOML_INT64){

                if(rcfd->Solver_Dict->number_of_frames > property.u.int64){

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                  "shift_get_c2c_metadata: solver dict request more frames than stored in database");
                    toml_free(metadata);
                    error = -1;
                    break;
                }
            }
            else{

                report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                              "shift_get_c2c_metadata: missing or invalid frames entry for database");
                toml_free(metadata);
                error = -1;
                break;
            }

            toml_datum_t files = toml_get(database, "files");

            if(files.type == TOML_TABLE){

                toml_datum_t c2c = toml_get(files, "c2c");

                if(c2c.type == TOML_TABLE){

                    property = toml_get(c2c, "file_version");

                    if(property.type == TOML_FP64){

                        int version_major = (int)property.u.fp64;

                        // check version and break upon incompatibility
                        if(version_major > RCFD_C2C_FILE_VERSION_MAJOR){

                            report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                          "shift_get_c2c_metadata: c2c file version not supported");
                            toml_free(metadata);
                            error = -8;
                            break;
                        }
                    }
                    else{

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                      "shift_get_c2c_metadata: missing or invalid file version for c2c file");
                        toml_free(metadata);
                        error = -8;
                        break;
                    }

                    property = toml_get(c2c, "file_type");

                    if(property.type == TOML_STRING){

                        if(strncmp(property.u.str.ptr, "ascii", 5) == 0){

                            *file_type = RCFD_FILE_ASCII;
                        }
                        else if(strncmp(property.u.str.ptr, "binary", 6) == 0){

                            *file_type = RCFD_FILE_BINARY;
                        }
                        else{

                            report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                         "shift_get_c2c_metadata: unrecognized file type for c2c file");
                            toml_free(metadata);
                            error = -16;
                            break;
                        }
                    }
                    else{

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                      "shift_get_c2c_metadata: missing or invalid file_type for c2c file");
                        toml_free(metadata);
                        error = -16;
                        break;
                    }

                    property = toml_get(c2c, "shift_format");

                    if(property.type == TOML_INT64){

                        *shift_format = property.u.int64;
                    }
                    else{

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                      "shift_get_c2c_metadata: missing or invalid shift_format for c2c file");
                        toml_free(metadata);
                        error = -32;
                        break;
                    }

                    property = toml_get(c2c, "shift_types");

                    if(property.type == TOML_INT64){

                        *shift_types = property.u.int64;

                        if((*shift_types < 1) || (*shift_types > 2)){

                            report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                          "shift_get_c2c_metadata: invalid shift_types value for c2c file");
                            toml_free(metadata);
                            error = -64;
                            break;
                        }
                    }
                    else{

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                      "shift_get_c2c_metadata: missing or invalid shift_types for c2c file");
                        toml_free(metadata);
                        error = -64;
                        break;
                    }
                }
                else{

                    report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                  "shift_get_c2c_metadata: missing or invalid c2c table for database");
                    toml_free(metadata);
                    error = -4;
                    break;
                }
            }
            else{

                report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                              "shift_get_c2c_metadata: missing or invalid files table for database");
                toml_free(metadata);
                error = -2;
                break;
            }

            toml_free(metadata);

        } while(0);
#else
        // primitive file parsing
        FILE *fmeta = fopen(metadata_filename, "r");

        if(fmeta){

            char buffer[READ_BUFFER_SIZE];

            // search for database.files.c2c table
            while(fgets(buffer, READ_BUFFER_SIZE, fmeta)){

                if(strncmp(buffer, "[database.files.c2c]", 11) == 0)
                    break;
            }

            while(!feof(fmeta) && fgets(buffer, READ_BUFFER_SIZE, fmeta)){

                // skip any leading space
                int line_len = strlen(buffer) - 1;
                int key_start_index = 0;
                while(isspace(buffer[key_start_index])){

                    ++key_start_index;
                }

                // stop if we find next table
                if(buffer[key_start_index] == '['){

                    break;
                }

                int value_start_index = key_start_index;

                // search for '='
                while(value_start_index < line_len &&
                      buffer[value_start_index] != '='){

                    ++value_start_index;
                }

                ++value_start_index;

                // skip spaces between '=' and value
                while(value_start_index < line_len &&
                      isspace(buffer[value_start_index])){

                    ++value_start_index;
                }

                if(value_start_index >= line_len)
                    continue;

                // look for key-value pairs
                if(strncmp(&buffer[key_start_index], "file_version", 12) == 0){

                    double version = strtod(&buffer[value_start_index], NULL);
                    int version_major = (int)version;

                    // check version and break upon incompatibility
                    if(version_major > RCFD_C2C_FILE_VERSION_MAJOR){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                      "shift_get_c2c_metadata: c2c file version not supported");
                        error = -8;
                        break;
                    }
                }
                else if(strncmp(&buffer[key_start_index], "file_type", 9) == 0){

                    if(strncmp(&buffer[value_start_index], "\"ascii\"", 7) == 0){

                        *file_type = RCFD_FILE_ASCII;
                    }
                    else if(strncmp(&buffer[value_start_index], "\"binary\"", 8) == 0){

                        *file_type = RCFD_FILE_BINARY;
                    }
                    else{

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                     "shift_get_c2c_metadata: unrecognized file type for c2c file");
                        error = -16;
                        break;
                    }
                }
                else if(strncmp(&buffer[key_start_index], "shift_format", 12) == 0){

                    *shift_format = strtol(&buffer[value_start_index], NULL, 16);
                }
                else if(strncmp(&buffer[key_start_index], "shift_types", 11) == 0){

                    *shift_types = strtol(&buffer[value_start_index], NULL, 10);

                    if((*shift_types < 1) || (*shift_types > 2)){

                        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                      "shift_get_c2c_metadata: invalid shift_types value for c2c file");
                        error = -64;
                        break;
                    }
                }
            }

            fclose(fmeta);
        }
#endif
    }

    PRF_BCAST_INT(&error,       1, 0);

    if(error)
        return error;

    PRF_BCAST_INT(file_type,    1, 0);
    PRF_BCAST_INT(shift_format, 1, 0);
    PRF_BCAST_INT(shift_types,  1, 0);

    return 0;
}

/**
 * @brief read cell-to-cell shift database from files
 * @param rcfd  pointer to previously created rCFD instance
 */
void shift_read(rCFD_type* rcfd)
{
#if RP_NODE

#if 1   // local vars declare and init
    int     i_state, i_phase, i_frame;

    int     base_layer = 0;

    int     state_count = rcfd->Solver_Dict->number_of_states;
    int     phase_count = rcfd->Solver_Dict->number_of_phases;
    int     frame_count = rcfd->Solver_Dict->number_of_frames;

    int     file_type = rcfd->Solver->c2c_file_type;
    int     shift_format = rcfd->Solver->c2c_shift_format;
    int     shift_types = rcfd->Solver->c2c_shift_type_count;

    int     total_shifts_count =  0;
    int     shifts_read_count =   0;
    int     shifts_pruned_count = 0;

    int     waterproof_shifts        = 0;
    int     invalid_shifts_exist     = 1;
    int     shift_weights_add_to_one = 0;

    FILE    *fi = NULL;
    char    filename[40];
#endif

    int retval = shift_get_c2c_metadata(rcfd, &file_type, &shift_format, &shift_types);

    if(retval != 0){

        report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                      "shift_read: encountered error (%d) while reading c2c metadata",
                      retval);
        return;
    }

    if(file_type == RCFD_FILE_BINARY){

        if((shift_format != RCFD_SHIFT_C0_N0_C1_W0CHAR) &&
           (shift_format != RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            report_error0(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                          "shift_read: binary file type only supports"
                          " shift format RCFD_SHIFT_C0_N0_C1_W0CHAR and"
                          " RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR");
            return;
        }
    }

    rcfd->Solver->c2c_file_type = file_type;
    rcfd->Solver->c2c_shift_format = shift_format;
    rcfd->Solver->c2c_shift_type_count = shift_types;

    for(i_state = 0; i_state < state_count; i_state++){

        for(i_phase = 0; i_phase < phase_count; i_phase++){

            // open file, exit if error
            {
                sprintf(filename,"%s_%d_%d_%d", rcfd->File_Dict->C2C_filename, i_state, i_phase, myid);

                if(file_type == RCFD_FILE_ASCII){

                    fi = fopen(filename, "r");
                }
                else{

                    fi = fopen(filename, "rb");
                }

                if(!fi){

                    report_error(rcfd, RCFD_REPORT_CATEGORY_READ_DB,
                                 "[%d] shift_read: failed to open file %s",
                                 myid, filename);
                    return;
                }
            }

            for(i_frame = 0; i_frame < frame_count; i_frame++){

                shifts_read_count = read_shifts_of_frame(rcfd,
                                                         fi,
                                                         &rcfd->C2C[i_state][i_phase][i_frame][base_layer].shifts,
                                                         &rcfd->C2C[i_state][i_phase][i_frame][base_layer].shifts_in);

                //shifts_pruned_count = prune_shifts_of_frame(rcfd, &frame_shifts, &frame_shifts_in);

                total_shifts_count += PRF_GISUM1(shifts_read_count - shifts_pruned_count);

                /*check_cure_shifts_of_frame(rcfd,
                                           &frame_shifts, &frame_shifts_in,
                                           &waterproof_shifts, &invalid_shifts_exist, &shift_weights_add_to_one);*/

                rcfd->C2C[i_state][i_phase][i_frame][base_layer].waterproof_shifts        = waterproof_shifts;
                rcfd->C2C[i_state][i_phase][i_frame][base_layer].invalid_shifts_exist     = invalid_shifts_exist;
                rcfd->C2C[i_state][i_phase][i_frame][base_layer].shift_weights_add_to_one = shift_weights_add_to_one;
            }

            fclose(fi);
        }
    }

    rcfd->Solver->total_shift_read_count = total_shifts_count;
#endif
}

/**
 * @brief Updates shift_update field by local (per-node) cell-to-cell shifts
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param i_state   index of current state
 * @param i_phase   index of current phase
 * @param i_frame   index of current frame
 * @param i_layer   index of current layer
 */
void shift_local_data(rCFD_type* rcfd, int i_state, int i_phase, int i_frame, int i_layer)
{
#if RP_NODE

#if 1   // local vars
    int     i_shift, i_data;

    int     c0, c1;

    int     cell_count   = _CELL_DICT(i_layer).number_of_cells;
    int     shifts_count = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.shifts_count;
    int     data_count   = rcfd->Phase_Dict[i_phase].number_of_data;
    int     shift_format = rcfd->Solver->c2c_shift_format;

    int     invalid_shifts_exist     = rcfd->C2C[i_state][i_phase][i_frame][i_layer].invalid_shifts_exist;
    int     shift_weights_add_to_one = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shift_weights_add_to_one;

    double  w0, data0, w0_old, data_old;
#endif

    // map parallel data, init data_shift, data_weight_after_shift, data_shift_updated
    {
        parallel_map_phase_data(rcfd, i_phase);

        memset(_C(i_layer).data_shift[0]     , 0, cell_count * data_count * sizeof(double));
        memset(_C(i_layer).weight_after_shift, 0, cell_count * sizeof(double));
        memset(_C(i_layer).data_shift_updated, 0, cell_count * sizeof(short));
    }

    // event. fill w0 by w0char
    {
        if((shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR) ||
           (shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0 = malloc(shifts_count * sizeof(double));

            for(i_shift = 0; i_shift < shifts_count; i_shift++){

                rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0[i_shift] =
                    (double)rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0char[i_shift] / 255.0;
            }
        }
    }

    // shift data depending on shift characteristics
    {
        if(shift_weights_add_to_one && (!invalid_shifts_exist)){  // sum(w0(c1)) = 1 && w0 > 0

            for(i_shift = 0; i_shift < shifts_count; i_shift++){

                c0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.c0[i_shift];
                c1 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.c1[i_shift];
                w0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0[i_shift];

                for(i_data = 0; i_data < data_count; i_data++){

                    data0 = _C(i_layer).data[i_phase][c0][i_data];

                    _C(i_layer).data_shift[c1][i_data] += w0 * data0;
                }

                _C(i_layer).weight_after_shift[c1] = 1.0;
                _C(i_layer).data_shift_updated[c1] = 1;
            }
        }
        else if(!invalid_shifts_exist){ // w0 > 0

            for(i_shift = 0; i_shift < shifts_count; i_shift++){

                c0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.c0[i_shift];
                c1 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.c1[i_shift];
                w0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0[i_shift];

                for(i_data = 0; i_data < data_count; i_data++){

                    data0 = _C(i_layer).data[i_phase][c0][i_data];

                    w0_old =   _C(i_layer).weight_after_shift[c1];
                    data_old = _C(i_layer).data_shift[c1][i_data];

                    _C(i_layer).data_shift[c1][i_data] = (w0 * data0 + w0_old * data_old) / (w0 + w0_old);
                }

                _C(i_layer).weight_after_shift[c1] += w0;
                _C(i_layer).data_shift_updated[c1] = 1;
            }
        }
        else{

            for(i_shift = 0; i_shift < shifts_count; i_shift++){

                w0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0[i_shift];

                if(w0 > 0){

                    c0 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.c0[i_shift];
                    c1 = rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.c1[i_shift];

                    for(i_data = 0; i_data < data_count; i_data++){

                        data0 = _C(i_layer).data[i_phase][c0][i_data];

                        w0_old =   _C(i_layer).weight_after_shift[c1];
                        data_old = _C(i_layer).data_shift[c1][i_data];

                        _C(i_layer).data_shift[c1][i_data] = (w0 * data0 + w0_old * data_old) / (w0 + w0_old);
                    }

                    _C(i_layer).weight_after_shift[c1] += w0;
                    _C(i_layer).data_shift_updated[c1] = 1;
                }
            }
        }
    }

    // event. free shifts.w0
    {
        if( (shift_format == RCFD_SHIFT_C0_N0_C1_W0CHAR) ||
            (shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR)){

            free(rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0);
            rcfd->C2C[i_state][i_phase][i_frame][i_layer].shifts.w0 = NULL;
        }
    }

#endif
}

void shift_data(rCFD_type* rcfd, const int i_phase)
{
#if RP_NODE
    int i_state, i_layer, i_frame;

    i_state = rcfd->Solver->current_state;
    i_layer = rcfd->Solver->current_layer;
    i_frame = rcfd->Solver->current_frame;

    // sanity checks
    {
        if(rcfd->Solver_Dict->mode != RCFD_MODE_RUN){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN_SHIFT,
                         "[%d] shift_data: only available in run mode", myid);
            return;
        }

        if((i_layer < 0) || (i_layer >= rcfd->Solver_Dict->number_of_layers)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN_SHIFT,
                         "[%d] shift_data: invalid i_layer %d", myid, i_layer);
            return;
        }

        if((i_frame < 0) || (i_frame >= rcfd->Solver_Dict->number_of_frames)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN_SHIFT,
                         "[%d] shift_data: invalid i_frame %d", myid, i_frame);
            return;
        }

        if((i_phase < 0) || (i_phase >= rcfd->Solver_Dict->number_of_phases)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN_SHIFT,
                         "[%d] shift_data: invalid i_phase %d", myid, i_phase);
            return;
        }

        if((i_state < 0) || (i_state >= rcfd->Solver_Dict->number_of_states)){

            report_error(rcfd, RCFD_REPORT_CATEGORY_RUN_SHIFT,
                         "[%d] shift_data: invalid i_state %d", myid, i_state);
            return;
        }
    }

    shift_local_data(rcfd, i_state, i_phase, i_frame, i_layer);

    parallel_shift_data(rcfd, i_state, i_phase, i_frame, i_layer);

#endif
}

void shift_fill(rCFD_type* rcfd, const int i_phase)
{
#if RP_NODE

#if 1   // local vars
    int     i_face, i_data, i_cell;

    int     i_state = rcfd->Solver->current_state;
    int     i_layer = rcfd->Solver->current_layer;
    int     i_frame = rcfd->Solver->current_frame;

    int     unhit_cell_count = 1, fill_loop_count, c0, c1, c;

    int     data_count = rcfd->Phase_Dict[i_phase].number_of_data;
    int     int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;
    int     cell_count =     _CELL_DICT(i_layer).number_of_cells;
    int     int_face_count = _FACE_DICT(i_layer).number_of_int_faces;
    int     max_fill_loop_count = rcfd->Solver_Dict->max_fill_loops;

    int     waterproof_shifts = rcfd->C2C[i_state][i_phase][i_frame][i_layer].waterproof_shifts;
#endif

    if(i_layer > 0) return; // TODO: implement for layer>0

    if(!waterproof_shifts){

        parallel_map_phase_data_shift(rcfd, i_phase);

        memset(_C(i_layer).data_swap[0],      0, cell_count * data_count * sizeof(double));
        memset(_C(i_layer).weight_after_swap, 0, cell_count * sizeof(double));

        fill_loop_count =  0;

        while((unhit_cell_count > 0) && (fill_loop_count < max_fill_loop_count)){

            for(i_face = 0; i_face < int_face_count; i_face++){

                c0 = _F(i_layer).c0[i_face];
                c1 = _F(i_layer).c1[i_face];

                if((  _C(i_layer).data_shift_updated[c0]) &&
                   ( !_C(i_layer).data_shift_updated[c1])){

                    c = c0; c0 = c1; c1 = c;
                }

                if(!_C(i_layer).data_shift_updated[c0]){

                    for(i_data = 0; i_data < data_count; i_data++){

                        _C(i_layer).data_swap[c0][i_data] =  (    _C(i_layer).weight_after_shift[c1]
                                                                * _C(i_layer).data_shift[c1][i_data]
                                                                + _C(i_layer).weight_after_swap [c0]
                                                                * _C(i_layer).data_swap [c0][i_data]
                                                              )
                                                              /
                                                              (
                                                                  _C(i_layer).weight_after_shift[c1]
                                                                + _C(i_layer).weight_after_swap [c0]
                                                              );
                    }

                    _C(i_layer).weight_after_swap[c0] += _C(i_layer).weight_after_shift[c1];
                    _C(i_layer).data_shift_updated[c0] = 1;
                }
            }

            unhit_cell_count = 0;

            for(i_cell = 0; i_cell < int_cell_count; i_cell++){

                if(_C(i_layer).weight_after_swap[i_cell] > 0.0){

                    for(i_data = 0; i_data < data_count; i_data++){

                        _C(i_layer).data_shift[i_cell][i_data] = _C(i_layer).data_swap[i_cell][i_data];
                    }

                    _C(i_layer).weight_after_shift[i_cell] = _C(i_layer).weight_after_swap[i_cell];
                    _C(i_layer).weight_after_swap [i_cell] = 0.0;
                }

                if(( !_C(i_layer).data_shift_updated[i_cell]) && (_C(i_layer).vof[i_frame][i_cell][i_phase] > 0.0)){

                    unhit_cell_count++;
                }
            }

            fill_loop_count++;
        }
    }

    // data = data_shift
    for(i_cell = 0; i_cell < int_cell_count; i_cell++){

        if(_C(i_layer).data_shift_updated[i_cell]){

            for(i_data = 0; i_data < data_count; i_data++){

                _C(i_layer).data[i_phase][i_cell][i_data] = _C(i_layer).data_shift[i_cell][i_data];
            }
        }
    }

#endif // RP_NODE
}

/**
 * @brief gets number of internal and external shifts (helper fct to shift_write)
 *
 * @param handle          pointer to previously created rCFD instance
 * @param i_state         current state index
 * @param i_phase         current phase index
 * @param i_frame         current frame index
 * @param int_shift_count number of internal cell-to-cell shifts
 * @param ext_shift_count number of external cell-to-cell shifts
 */
void shift_get_int_ext_count(rCFD_type *rcfd, int i_state, int i_phase, int *int_shifts_count, int *ext_shifts_count)
{
#if RP_NODE
    int i_shift, i_int_shifts = 0, i_ext_shifts = 0;

    int shifts_count, interface_shifts_count, node0, node1;

    if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

        shifts_count = rcfd->C2C_Prep[i_phase].valid_shifts_count;

        for(i_shift = 0; i_shift < shifts_count; i_shift++){

            node0 = rcfd->C2C_Prep[i_phase].node0[i_shift];
            node1 = rcfd->C2C_Prep[i_phase].node1[i_shift];

            if(node0 == node1){

                i_int_shifts++;
            }
            else{

                i_ext_shifts++;
            }
        }

        interface_shifts_count = rcfd->C2C_Interface->valid_shifts_count;

        for(i_shift = 0; i_shift < interface_shifts_count; i_shift++){

            node0 = rcfd->C2C_Interface->node0[i_shift];
            node1 = rcfd->C2C_Interface->node1[i_shift];

            if(node0 == node1){

                i_int_shifts++;
            }
            else{

                i_ext_shifts++;
            }
        }
    }

    *int_shifts_count = i_int_shifts;
    *ext_shifts_count = i_ext_shifts;
#endif
}

/**
 * @brief writes cell-to-cell shifts to file (first intra-node shifts then inter-node shifts)
 *        depending on their shift_format
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param i_state   current state index
 * @param i_phase   current phase index
 * @param i_frame   current frame index
 * @param i_layer   current layer index
 * @return number of written shifts or -1 if error or called by host
 */
int shift_write(rCFD_type* rcfd, int i_state, int i_frame, int i_phase, int i_layer)
{
#if RP_NODE

#if 1   /* local vars */
    int     i_shift;
    int     stored_shifts_count = 0, shifts_count, int_shifts_count = 0, ext_shifts_count = 0;
    int     interface_shifts_count, global_interface_shifts_count;
    int     file_type = rcfd->Solver->c2c_file_type;
    int     shift_format = rcfd->Solver->c2c_shift_format;
    int     *c0 = NULL, *c1 = NULL, *node0 = NULL, *node1 = NULL;
    int     *c0_interface = NULL, *c1_interface = NULL, *node0_interface = NULL, *node1_interface = NULL;
    double  *w0 = NULL, *w0_interface = NULL;
    FILE    *f_out = NULL;
    char    file_name[80];
#endif

    /* adjust w0's such that they sum up to one for each cell */
    {
        int i_cell, i_layer = 0;
        int c1_id;
        int int_cell_count = _CELL_DICT(i_layer).number_of_int_cells;

        double *cell_w0 = malloc(int_cell_count * sizeof(double));

        for(i_cell = 0; i_cell < int_cell_count; i_cell++){

            cell_w0[i_cell] = 0.0;
        }

        shifts_count = rcfd->C2C_Prep[i_phase].valid_shifts_count;

        for(i_shift = 0; i_shift < shifts_count; i_shift++){

            c1_id = rcfd->C2C_Prep[i_phase].c1[i_shift];

            if((c1_id > -1) && (c1_id < int_cell_count)){

                cell_w0[c1_id]+= rcfd->C2C_Prep[i_phase].w0[i_shift];
            }
        }

        interface_shifts_count = rcfd->C2C_Interface->valid_shifts_count;

        for(i_shift = 0; i_shift < interface_shifts_count; i_shift++){

            c1_id = rcfd->C2C_Interface->c1[i_shift];

            if((c1_id > -1) && (c1_id < int_cell_count)){

                cell_w0[c1_id]+= rcfd->C2C_Interface->w0[i_shift];
            }
        }

        for(i_shift = 0; i_shift < shifts_count; i_shift++){

            c1_id = rcfd->C2C_Prep[i_phase].c1[i_shift];

            if((c1_id > -1) && (c1_id < int_cell_count) &&
               (cell_w0[c1_id] > 0.0)){

                rcfd->C2C_Prep[i_phase].w0[i_shift] /= cell_w0[c1_id];
            }
        }

        for(i_shift = 0; i_shift < interface_shifts_count; i_shift++){

            c1_id = rcfd->C2C_Interface->c1[i_shift];

            if((c1_id > -1) && (c1_id < int_cell_count) &&
               (cell_w0[c1_id] > 0.0)){

                rcfd->C2C_Interface->w0[i_shift] /= cell_w0[c1_id];
            }
        }

        free(cell_w0);
    }

    /* open f_out */
    {
        sprintf(file_name, "%s_%d_%d_%d", rcfd->File_Dict->C2C_filename, i_state, i_phase, myid);

        if(file_type == RCFD_FILE_ASCII){

            f_out = fopen(file_name, (i_frame == 0) ? "w" : "a");
        }
        else{

            f_out = fopen(file_name, (i_frame == 0) ? "wb" : "ab");
        }

        if(f_out == NULL){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_SHIFT,
                         "[%d] shift_write: failed to open file %s",
                         myid, file_name);

            return -1;
        }
    }

    /* write C2C shifts, close f_out, reset C2C_Prep[i_phase].valid_shifts_count */
    {
        c0 =    rcfd->C2C_Prep[i_phase].c0;     // c0 points to array of int
        node0 = rcfd->C2C_Prep[i_phase].node0;
        c1 =    rcfd->C2C_Prep[i_phase].c1;
        node1 = rcfd->C2C_Prep[i_phase].node1;
        w0 =    rcfd->C2C_Prep[i_phase].w0;

        shifts_count = rcfd->C2C_Prep[i_phase].valid_shifts_count;

        c0_interface =    rcfd->C2C_Interface->c0;     // c0 points to array of int
        node0_interface = rcfd->C2C_Interface->node0;
        c1_interface =    rcfd->C2C_Interface->c1;
        node1_interface = rcfd->C2C_Interface->node1;
        w0_interface =    rcfd->C2C_Interface->w0;

        interface_shifts_count = rcfd->C2C_Interface->valid_shifts_count;
        global_interface_shifts_count = PRF_GISUM1(interface_shifts_count);

        switch(file_type){

            case RCFD_FILE_ASCII:
            {
                if(global_interface_shifts_count > 0){

                    report_warning0(rcfd, RCFD_REPORT_CATEGORY_WRITE_SHIFT,
                                    "shift_write: interface shifts are only supported for binary files");

                    rcfd->C2C_Interface->valid_shifts_count = 0;
                }

                switch(shift_format){

                    case RCFD_SHIFT_C0_N0_C1_N1_W0:
                    {
                        fprintf(f_out,"%d \n", shifts_count);

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] == node1[i_shift]){

                                fprintf(f_out, "%d %d %d %d %e\n",
                                        c0[i_shift], node0[i_shift], c1[i_shift], node1[i_shift], w0[i_shift]);
                            }
                        }

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] != node1[i_shift]){

                                fprintf(f_out, "%d %d %d %d %e\n",
                                        c0[i_shift], node0[i_shift], c1[i_shift], node1[i_shift], w0[i_shift]);
                            }
                        }

                        stored_shifts_count = shifts_count;

                        break;
                    }

                    case RCFD_SHIFT_C0_N0_C1_W0:
                    {
                        shift_get_int_ext_count(rcfd, i_state, i_phase, &int_shifts_count, &ext_shifts_count);

                        fprintf(f_out,"%d %d\n", int_shifts_count, ext_shifts_count);

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] == node1[i_shift]){

                                fprintf(f_out, "%d %d %d %e\n",
                                        c0[i_shift], node0[i_shift], c1[i_shift], w0[i_shift]);
                            }
                        }

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] != node1[i_shift]){

                                fprintf(f_out, "%d %d %d %e\n",
                                        c0[i_shift], node0[i_shift], c1[i_shift], w0[i_shift]);
                            }
                        }
                        stored_shifts_count = shifts_count;

                        break;
                    }

                    case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                    case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                    {
                        unsigned char w0_char =    0;

                        shift_get_int_ext_count(rcfd, i_state, i_phase, &int_shifts_count, &ext_shifts_count);

                        fprintf(f_out,"%d %d\n", int_shifts_count, ext_shifts_count);

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] == node1[i_shift]){

                                w0_char = (unsigned char)(w0[i_shift] * 255.0);

                                if(w0_char == 0) w0_char = 1;

                                fprintf(f_out,"%d %d %d\n", c0[i_shift], c1[i_shift], w0_char);
                            }
                        }

                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] != node1[i_shift]){

                                w0_char = (unsigned char)(w0[i_shift] * 255.0);

                                if(w0_char == 0) w0_char = 1;

                                fprintf(f_out,"%d %d %d %d\n", c0[i_shift], node0[i_shift], c1[i_shift], w0_char);
                            }
                        }

                        stored_shifts_count = shifts_count;

                        break;
                    }

                    default: break;
                }

                break;
            }

            case RCFD_FILE_BINARY:
            {
                switch (shift_format){

                    case RCFD_SHIFT_C0_N0_C1_W0CHAR:
                    case RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR:
                    {
                        unsigned char w0_char =    0;
                        unsigned char node0_char = 0;

                        shift_get_int_ext_count(rcfd, i_state, i_phase, &int_shifts_count, &ext_shifts_count);

                        fwrite(&int_shifts_count, sizeof(int_shifts_count), 1, f_out);
                        fwrite(&ext_shifts_count, sizeof(ext_shifts_count), 1, f_out);

                        // write interior shifts (omitting node0)
                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] == node1[i_shift]){

                                w0_char = (unsigned char)(w0[i_shift] * 255.0);

                                if(w0_char == 0) w0_char = 1;

                                fwrite(&c0[i_shift], sizeof(c0[i_shift]), 1, f_out);
                                fwrite(&c1[i_shift], sizeof(c1[i_shift]), 1, f_out);
                                fwrite(&w0_char,     sizeof(w0_char),     1, f_out);
                            }
                        }

                        for(i_shift = 0; i_shift < interface_shifts_count; i_shift++){

                            if(node0_interface[i_shift] == node1_interface[i_shift]){

                                w0_char = (unsigned char)(w0_interface[i_shift] * 255.0);

                                if(w0_char == 0) w0_char = 1;

                                fwrite(&c0_interface[i_shift], sizeof(c0_interface[i_shift]), 1, f_out);
                                fwrite(&c1_interface[i_shift], sizeof(c1_interface[i_shift]), 1, f_out);
                                fwrite(&w0_char,               sizeof(w0_char),               1, f_out);
                            }
                        }

                        // write inter-node shifts
                        for(i_shift = 0; i_shift < shifts_count; i_shift++){

                            if(node0[i_shift] != node1[i_shift]){

                                w0_char = (unsigned char)(w0[i_shift] * 255.0);

                                if(w0_char == 0) w0_char = 1;

                                fwrite(&c0[i_shift],    sizeof(c0[i_shift]),    1, f_out);
                                if(shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR){
                                    node0_char = (unsigned char)node0[i_shift];
                                    fwrite(&node0_char,     sizeof(node0_char),     1, f_out);
                                }else{
                                    fwrite(&node0[i_shift], sizeof(node0[i_shift]), 1, f_out);
                                }
                                fwrite(&c1[i_shift],    sizeof(c1[i_shift]),    1, f_out);
                                fwrite(&w0_char,        sizeof(w0_char),        1, f_out);
                            }
                        }

                        for(i_shift = 0; i_shift < interface_shifts_count; i_shift++){

                            if(node0_interface[i_shift] != node1_interface[i_shift]){

                                w0_char = (unsigned char)(w0_interface[i_shift] * 255.0);

                                if(w0_char == 0) w0_char = 1;

                                fwrite(&c0_interface[i_shift], sizeof(c0_interface[i_shift]), 1, f_out);
                                if(shift_format == RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR){
                                    node0_char = (unsigned char)node0_interface[i_shift];
                                    fwrite(&node0_char, sizeof(node0_char), 1, f_out);
                                }else{
                                    fwrite(&node0_interface[i_shift], sizeof(node0_interface[i_shift]), 1, f_out);
                                }
                                fwrite(&c1_interface[i_shift], sizeof(c1_interface[i_shift]), 1, f_out);
                                fwrite(&w0_char,               sizeof(w0_char),               1, f_out);
                            }
                        }

                        stored_shifts_count = shifts_count + interface_shifts_count;

                        break;
                    }

                    default:
                    {
                        report_error0(rcfd, RCFD_REPORT_CATEGORY_WRITE_SHIFT,
                                     "shift_write: binary file type only supports"
                                     " shift format RCFD_SHIFT_C0_N0_C1_W0CHAR and"
                                     " RCFD_SHIFT_C0_N0CHAR_C1_W0CHAR");
                        break;
                    }
                }

                break;
            }

            default: break;
        }

        rcfd->C2C_Prep[i_phase].valid_shifts_count = 0;

        fclose(f_out);
    }

    return stored_shifts_count;

#else
    return -1;
#endif
}

#endif // RCFD_SHIFT
