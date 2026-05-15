/******************************************************************************
 * @internal
 * @file        rCFD_swap.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_SWAP
#define RCFD_SWAP

#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_topo.h"
#include "rCFD_parallel.h"

    void swap_init(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE
        int i_layer = rcfd->Solver->current_layer;

        int cell_count = _CELL_DICT(i_layer).number_of_cells;
        int data_count = rcfd->Phase_Dict[i_phase].number_of_data;

        memset(_C(i_layer).weight_after_swap, 0, cell_count *              sizeof(double));
        memset(_C(i_layer).data_swap[0],      0, cell_count * data_count * sizeof(double));

        parallel_map_phase_data(rcfd, i_phase);
#endif
    }

    void swap_data(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

#define LOCAL_DEBUG_FLAG    0

#if 1   // local vars
    int     i_layer, i_data, i_while, i_diff, i_frame, i_frame_c0, i_frame_c1, i_face, i_cell;
    int     number_of_diff_loops, c, c0, c1, swap_exchange_active;
    int     *active_swap_face = NULL;
    double  swap_per_loop, data_exchange, average_local_diff, local_cell_distance, local_face_area, max_local_swap_exchange;
    double  vol_c0, vol_c1, vol_flip, max_vol_flip;
    double  *local_swap_exchange = NULL;
#endif

    i_layer = rcfd->Solver->current_layer;
    i_frame = rcfd->Solver->current_frame;

    i_frame_c0 = i_frame;
    i_frame_c1 = i_frame;

    swap_init(rcfd, i_phase);

    if(rcfd->Solver_Dict->local_diff_on){

        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            switch (rcfd->Data_Dict[i_phase][i_data].type){

                case RCFD_DATA_CONCENTRATION:
                case RCFD_DATA_TEMPERATURE:
                {
                    // init swap_exchange_active, calc swap exchange per face[i_face], set active_swap_face[i_face]
                    {
                        // while loop
                        swap_exchange_active = 1;

                        i_while = 0;

                        // swap_exchange_per_face, active_swap_face
                        local_swap_exchange = (double*)malloc(_FACE_DICT(i_layer).number_of_faces * sizeof(double));

                        active_swap_face = (int*)malloc(_FACE_DICT(i_layer).number_of_faces * sizeof(int));

                        max_local_swap_exchange = 0.0;

                        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                            c0 = _F(i_layer).c0[i_face];
                            c1 = _F(i_layer).c1[i_face];

                            average_local_diff = (_C(i_layer).local_diff[i_frame_c0][i_phase][c0] + _C(i_layer).local_diff[i_frame_c1][i_phase][c1]) / 2.0;

                            local_cell_distance = cell2cell_distance(rcfd, i_layer, c0, c1);

                            local_face_area = topo_face_area(rcfd, i_layer, i_face);

                            local_swap_exchange[i_face] =     average_local_diff
                                                            * local_face_area
                                                            / local_cell_distance
                                                            * rcfd->Solver_Dict->global_time_step;   // (m3)

                            if(local_swap_exchange[i_face] > max_local_swap_exchange){

                                max_local_swap_exchange = local_swap_exchange[i_face];
                            }

                            if(fabs(local_swap_exchange[i_face]) > 0.0){

                                active_swap_face[i_face] = 1;
                            }
                            else{

                                active_swap_face[i_face] = 0;
                            }
                        }

                        max_local_swap_exchange = PRF_GRSUM1(max_local_swap_exchange);

                        if(LOCAL_DEBUG_FLAG){

                            report_debug0(rcfd, RCFD_REPORT_CATEGORY_RUN_SWAP,
                                          "swap_data: max_local_swap_exchange = %e\n",
                                          max_local_swap_exchange);
                        }
                    }

                    while((swap_exchange_active) && (i_while < rcfd->Solver_Dict->face_swap_max_loops)){

                        // init loop vars, _C(i_layer).data_swap
                        {
                            i_while++;

                            swap_exchange_active = 0;

                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                                _C(i_layer).data_swap[i_cell][i_data] = 0.0;
                            }
                        }

                        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                            if(active_swap_face[i_face]){

                                // get neighboring cells, vof's, max vol_flip
                                {
                                    c0 = _F(i_layer).c0[i_face];
                                    c1 = _F(i_layer).c1[i_face];

                                    vol_c0 = _C(i_layer).volume[c0] * _C(i_layer).vof[i_frame_c0][c0][i_phase];
                                    vol_c1 = _C(i_layer).volume[c1] * _C(i_layer).vof[i_frame_c1][c1][i_phase];

                                    if(vol_c0 < vol_c1){

                                        max_vol_flip = vol_c0 * rcfd->Solver_Dict->face_swap_max_per_loop;
                                    }
                                    else{

                                        max_vol_flip = vol_c1 * rcfd->Solver_Dict->face_swap_max_per_loop;
                                    }
                                }

                                // calc. vol_flip
                                {
                                    if(local_swap_exchange[i_face] <= max_vol_flip){

                                        vol_flip = local_swap_exchange[i_face];
                                        local_swap_exchange[i_face] = 0.0;
                                        active_swap_face[i_face] = 0;
                                    }
                                    else{

                                        vol_flip = max_vol_flip;
                                        local_swap_exchange[i_face] -= max_vol_flip;
                                    }
                                }

                                // calc. data_exchange
                                data_exchange = (_C(i_layer).data[i_phase][c1][i_data] - _C(i_layer).data[i_phase][c0][i_data]) / 2.;

                                // update _C(i_layer).data_swap
                                {
                                    _C(i_layer).data_swap[c1][i_data] -= data_exchange * vol_flip;
                                    _C(i_layer).data_swap[c0][i_data] += data_exchange * vol_flip;
                                }

                                // update active_swap_face, swap_exchange_active
                                {
                                    if(local_swap_exchange[i_face] > 0.0){

                                        swap_exchange_active++;
                                    }
                                    else{

                                        active_swap_face[i_face] = 0;
                                    }
                                }
                            }   // if active face
                        }

                        // update _C(i_layer).data
                        {
                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                            if((_C(i_layer).volume[i_cell] * _C(i_layer).vof[i_frame][i_cell][i_phase]) > 0.0){

                                    _C(i_layer).data[i_phase][i_cell][i_data] += _C(i_layer).data_swap[i_cell][i_data] / (_C(i_layer).volume[i_cell] * _C(i_layer).vof[i_frame][i_cell][i_phase]);
                                }
                            }
                        }

                        // map host data to corona cells
                        if(rcfd->Solver_Dict->parallel_diffusion_on && i_layer == 0){

                            parallel_communicate_data(rcfd, i_layer, i_phase, i_data, RCFD_PARALLEL_MAP_HOST_FORMAT);
                        }

                        swap_exchange_active = PRF_GISUM1(swap_exchange_active);

                        // local debug
                        if(LOCAL_DEBUG_FLAG){

                            report_debug0(rcfd, RCFD_REPORT_CATEGORY_RUN_SWAP,
                                          "data_swap: i_while = %d, swap_exchange_active = %d",
                                          i_while, swap_exchange_active);
                        }
                    }   // while loop

                    // free local vars
                    {
                        free( local_swap_exchange);
                        free( active_swap_face);
                    }

                    break;
                }
            }
        }
    }
    else{   // global diff

        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            switch (rcfd->Data_Dict[i_phase][i_data].type){

                case RCFD_DATA_CONCENTRATION:
                case RCFD_DATA_TEMPERATURE:
                {

                    // define global number_of_diff_loops, swap_per_loop
                    {
                        if(fabs(rcfd->Data_Dict[i_phase][i_data].face_swap_diffusion) > rcfd->Solver_Dict->face_swap_max_per_loop){

                            number_of_diff_loops = (int)(fabs(rcfd->Data_Dict[i_phase][i_data].face_swap_diffusion) / rcfd->Solver_Dict->face_swap_max_per_loop);  // ( > 1)
                            swap_per_loop = rcfd->Data_Dict[i_phase][i_data].face_swap_diffusion / (double)number_of_diff_loops;
                        }
                        else{

                            number_of_diff_loops = 1;
                            swap_per_loop = rcfd->Data_Dict[i_phase][i_data].face_swap_diffusion;
                        }
                    }

                    // perform face swaps until global diffusive fluxes are achieved
                    for(i_diff = 0; i_diff < number_of_diff_loops; i_diff++){

                        /* init. data_swap */
                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                            _C(i_layer).data_swap[i_cell][i_data] = 0.0;
                        }

                        /* define swap masses */
                        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                            c0 = _F(i_layer).c0[i_face];
                            c1 = _F(i_layer).c1[i_face];

                            if(_C(i_layer).data[i_phase][c0][i_data] > _C(i_layer).data[i_phase][c1][i_data]){

                                c = c0; c0 = c1; c1 = c;
                            }

                            if(_C(i_layer).data[i_phase][c1][i_data] > _C(i_layer).data[i_phase][c0][i_data]){

                                // flip volume
                                {
                                    if((_C(i_layer).volume[c0] * _C(i_layer).vof[i_frame_c0][c0][i_phase]) < (_C(i_layer).volume[c1] * _C(i_layer).vof[i_frame_c1][c1][i_phase])){

                                        vol_flip = _C(i_layer).volume[c0] * _C(i_layer).vof[i_frame_c0][c0][i_phase];
                                    }
                                    else{

                                        vol_flip = _C(i_layer).volume[c1] * _C(i_layer).vof[i_frame_c1][c1][i_phase];
                                    }
                                }

                                // data_exchange
                                {
                                    data_exchange = (_C(i_layer).data[i_phase][c1][i_data] - _C(i_layer).data[i_phase][c0][i_data]) / 2.;

                                    if(swap_per_loop < 0.0){    // neg. diff.

                                        if((1.0 - _C(i_layer).data[i_phase][c1][i_data]) < _C(i_layer).data[i_phase][c0][i_data]){

                                            data_exchange = 1.0 - _C(i_layer).data[i_phase][c1][i_data];
                                        }
                                        else{

                                            data_exchange = _C(i_layer).data[i_phase][c0][i_data];
                                        }
                                    }
                                }

                                // _C(i_layer).data_swap
                                {
                                    _C(i_layer).data_swap[c1][i_data] -= vol_flip * data_exchange * swap_per_loop;
                                    _C(i_layer).data_swap[c0][i_data] += vol_flip * data_exchange * swap_per_loop;
                                }
                            }
                        }

                        /* update data by data_swap */
                        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

                            if((_C(i_layer).volume[i_cell] * _C(i_layer).vof[i_frame][i_cell][i_phase]) > 0.0){

                                _C(i_layer).data[i_phase][i_cell][i_data] += _C(i_layer).data_swap[i_cell][i_data] / (_C(i_layer).volume[i_cell] * _C(i_layer).vof[i_frame][i_cell][i_phase]);
                            }
                        }

                        /* parallel diffusion: map host data to other nodes' corona cells */
                        if(rcfd->Solver_Dict->parallel_diffusion_on && i_layer == 0){

                            parallel_communicate_data(rcfd, i_layer, i_phase, i_data, RCFD_PARALLEL_MAP_HOST_FORMAT);
                        }
                    }

                    break;
                }
            }
        }
    }

#undef  LOCAL_DEBUG_FLAG

#endif
    }

    void swap_drift_data(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

    /* Existing Limitations (11/21)
        L1: drifting pattern might depend on face order
        L2: drifting of tmperature_data not yet implemented
        L3: drifting only implemented for i_layer == 0
        L4: local drift only implemented for concentration_data
    */

#define LOCAL_DEBUG_FLAG    0

#if 1   // local vars
        int     i_layer, i_data, i_face, i_drift, i_cell, i_dim, i_frame;
        int     c0, c1, c, i_frame_c0, i_frame_c1, current_data_index;
        double  d32_fraction, d32_bulk, d32_local_nom, d32_local_denom, d32_local, local_conc;
        double  local_mass_c0, local_mass_c1, local_mass, drift_volume, hindering_factor;
        double  current_conc, sum_of_other_concs, conc_error;
        double  local_drift_exchange, available_c1_mass;
#endif

        i_layer = rcfd->Solver->current_layer;
        i_frame = rcfd->Solver->current_frame;

        i_frame_c0 = i_frame;
        i_frame_c1 = i_frame;

        swap_init(rcfd, i_phase);

        // set local_drift_adaption per face
        {
            if(rcfd->Phase_Dict[i_phase].local_drift_on){

                d32_bulk = rcfd->Phase_Dict[i_phase].d32_diam;

                for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                    c0 = _F(i_layer).c0[i_face];
                    c1 = _F(i_layer).c1[i_face];

                    d32_local_nom = 0.0;
                    d32_local_denom = 0.0;

                    // local d23
                    for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                        if(rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_CONCENTRATION){

                            local_conc = (_C(i_layer).data[i_phase][c0][i_data] + _C(i_layer).data[i_phase][c1][i_data]) / 2.0;

                            d32_local_nom   += local_conc * rcfd->Data_Dict[i_phase][i_data].d3_diam;
                            d32_local_denom += local_conc * rcfd->Data_Dict[i_phase][i_data].d2_diam;
                        }
                    }

                    if(d32_local_denom > 0.0){

                        d32_local = d32_local_nom / d32_local_denom;
                    }
                    else{

                        d32_local = rcfd->Data_Dict[i_phase][i_data].d1_diam; // no drift
                    }

                    for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                        d32_fraction = rcfd->Data_Dict[i_phase][i_data].d1_diam;

                        if(d32_fraction != d32_bulk){

                            _F(i_layer).local_drift_adaption[i_face][i_data] = (d32_fraction - d32_local) / (d32_fraction - d32_bulk);
                        }
                        else{

                            _F(i_layer).local_drift_adaption[i_face][i_data] = 1.0;
                        }
                    }
                }
            }
        }

        // for each data calculate drift exchange
        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            if(rcfd->Data_Dict[i_phase][i_data].drifting_data){

                for(i_drift = 0; i_drift < rcfd->Solver_Dict->number_of_drift_loops; i_drift++){

                    /* init _C(i_layer).drift_exchange */
                    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                        _C(i_layer).drift_exchange[i_cell] = 0.0;
                    }

                    switch (rcfd->Data_Dict[i_phase][i_data].type){

                        case RCFD_DATA_GENERIC:
                        {
                            /* set _C(i_layer).drift_exchange */
                            for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                                c0 = _F(i_layer).c0[i_face];
                                c1 = _F(i_layer).c1[i_face];

                                local_mass_c0 = _C(i_layer).volume[c0] * _C(i_layer).vof[i_frame_c0][c0][i_phase] * rcfd->Phase_Dict[i_phase].density;
                                local_mass_c1 = _C(i_layer).volume[c1] * _C(i_layer).vof[i_frame_c1][c1][i_phase] * rcfd->Phase_Dict[i_phase].density;

                                /* don't drift across interfaces */
                                if(local_mass_c0 * local_mass_c1 > 0.0){

                                    drift_volume = 0.0;

                                    for(i_dim = 0; i_dim < 3; i_dim++){

                                        drift_volume +=   rcfd->Data_Dict[i_phase][i_data].drift_velocity[i_dim]
                                                        * _F(i_layer).area[i_face][i_dim]
                                                        * rcfd->Solver->timestep_width[i_layer]
                                                        / (double)rcfd->Solver_Dict->number_of_drift_loops;
                                    }

                                    /* flip cells, such that flux is from c0 to c1 */
                                    if(drift_volume < 0.0){

                                        c = c0; c0 = c1; c1 = c;

                                        i_frame = i_frame_c0; i_frame_c0 = i_frame_c1; i_frame_c1 = i_frame;

                                        local_mass = local_mass_c0; local_mass_c0 = local_mass_c1; local_mass_c1 = local_mass;

                                        drift_volume *= -1.0;
                                    }

                                    if(drift_volume > _C(i_layer).volume[c0]){

                                        drift_volume = _C(i_layer).volume[c0];
                                    }

                                    local_drift_exchange = 0.0;

                                    if(_C(i_layer).volume[c0] > 0.0){

                                        local_drift_exchange = _C(i_layer).data[i_phase][c0][i_data] * drift_volume /_C(i_layer).volume[c0];
                                    }

                                    _C(i_layer).drift_exchange[c0] -= local_drift_exchange;
                                    _C(i_layer).drift_exchange[c1] += local_drift_exchange;
                                }
                            }

                            /* parallel exchange of local_drift_exchange */
                            {
                                parallel_communicate(rcfd, _C(i_layer).drift_exchange, i_layer, RCFD_PARALLEL_SUM_CELLS_FORMAT);
                            }

                            /* adjust data by _C(i_layer).drift_exchange */
                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                                _C(i_layer).data[i_phase][i_cell][i_data] += _C(i_layer).drift_exchange[i_cell];
                            }

                            break;
                        }
                        case RCFD_DATA_CONCENTRATION:
                        {
                            /* set _C(i_layer).drift_exchange */
                            for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                                c0 = _F(i_layer).c0[i_face];
                                c1 = _F(i_layer).c1[i_face];

                                local_mass_c0 = _C(i_layer).volume[c0] * _C(i_layer).vof[i_frame_c0][c0][i_phase] * rcfd->Phase_Dict[i_phase].density;
                                local_mass_c1 = _C(i_layer).volume[c1] * _C(i_layer).vof[i_frame_c1][c1][i_phase] * rcfd->Phase_Dict[i_phase].density;

                                /* don't drift across interfaces */
                                if(local_mass_c0 * local_mass_c1 > 0.0){

                                    drift_volume = 0.0;

                                    for(i_dim = 0; i_dim < 3; i_dim++){

                                        /* drift_volume is defined per face, w/o information of vof's of c0/c1 */
                                        drift_volume +=   _F(i_layer).local_drift_adaption[i_face][i_data]
                                                        * rcfd->Data_Dict[i_phase][i_data].drift_velocity[i_dim]
                                                        * _F(i_layer).area[i_face][i_dim]
                                                        * rcfd->Solver->timestep_width[i_layer]
                                                        / (double)rcfd->Solver_Dict->number_of_drift_loops;
                                    }

                                    /* flip cells, such that flux is from c0 to c1 */
                                    if(drift_volume < 0.0){

                                        c = c0; c0 = c1; c1 = c;

                                        i_frame = i_frame_c0; i_frame_c0 = i_frame_c1; i_frame_c1 = i_frame;

                                        local_mass = local_mass_c0; local_mass_c0 = local_mass_c1; local_mass_c1 = local_mass;

                                        drift_volume *= -1.0;
                                    }

                                    /* in granular flow, drifting ability depend on solid volume fraction */
                                    if(rcfd->Phase_Dict[i_phase].hindered_drift_on){

                                        hindering_factor = 1.0 - _C(i_layer).vof[i_frame_c0][c0][i_phase] / rcfd->Phase_Dict[i_phase].vof_max;

                                        if(hindering_factor < 0.0){

                                            hindering_factor = 0.0;
                                        }

                                        drift_volume *= hindering_factor;
                                    }

                                    if(drift_volume > _C(i_layer).volume[c0]){

                                        drift_volume = _C(i_layer).volume[c0];
                                    }

                                    local_drift_exchange =    _C(i_layer).data[i_phase][c0][i_data]
                                                            * drift_volume
                                                            * _C(i_layer).vof[i_frame_c0][c0][i_phase]
                                                            * rcfd->Phase_Dict[i_phase].density;    /* [kg] */

                                    available_c1_mass   =     (1.0 - _C(i_layer).data[i_phase][c1][i_data])
                                                            * _C(i_layer).volume[c1]
                                                            * _C(i_layer).vof[i_frame_c1][c1][i_phase]
                                                            * rcfd->Phase_Dict[i_phase].density;

                                    if(local_drift_exchange > available_c1_mass){

                                        local_drift_exchange = available_c1_mass;
                                    }

                                    _C(i_layer).drift_exchange[c0] -= local_drift_exchange;
                                    _C(i_layer).drift_exchange[c1] += local_drift_exchange;
                                }
                            }

                            /* parallel exchange of _C(i_layer).drift_exchange */
                            if(rcfd->Solver_Dict->parallel_diffusion_on){

                                parallel_communicate(rcfd, _C(i_layer).drift_exchange, i_layer, RCFD_PARALLEL_SUM_CELLS_FORMAT);
                            }

                            /* adapt _C(i_layer).data by _C(i_layer).drift_exchange */
                            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                                local_mass =      _C(i_layer).volume[i_cell]
                                                * _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                * rcfd->Phase_Dict[i_phase].density;

                                if(_C(i_layer).vof[i_frame][i_cell][i_phase] > rcfd->Solver_Dict->vof_cutoff_limit){

                                    _C(i_layer).data[i_phase][i_cell][i_data] = ((_C(i_layer).data[i_phase][i_cell][i_data] * local_mass) + _C(i_layer).drift_exchange[i_cell]) / local_mass;

                                    // adjust other conc's such that they still sum up to one
                                    if(rcfd->Phase_Dict[i_phase].drift_control_conc_sum_on)
                                    {
                                        current_data_index = i_data;

                                        current_conc = _C(i_layer).data[i_phase][i_cell][i_data];

                                        sum_of_other_concs = 0.0;

                                        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                                            if(i_data != current_data_index){

                                                sum_of_other_concs += _C(i_layer).data[i_phase][i_cell][i_data];
                                            }
                                        }

                                        if(sum_of_other_concs > 1.0e-6){

                                            conc_error = 1.0 - (sum_of_other_concs + current_conc);

                                            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                                                if(i_data != current_data_index){

                                                    _C(i_layer).data[i_phase][i_cell][i_data] += _C(i_layer).data[i_phase][i_cell][i_data] / sum_of_other_concs * conc_error;

                                                    if(_C(i_layer).data[i_phase][i_cell][i_data] < 0.0) _C(i_layer).data[i_phase][i_cell][i_data] = 0.0;
                                                    if(_C(i_layer).data[i_phase][i_cell][i_data] > 1.0) _C(i_layer).data[i_phase][i_cell][i_data] = 1.0;
                                                }
                                            }
                                        }

                                        i_data = current_data_index;
                                    }
                                }
                            }
                            break;
                        }
                        case RCFD_DATA_TEMPERATURE:
                        {
                            /* to do: replace local_drift_exchange by drift_exchange */
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }
#undef LOCAL_DEBUG_FLAG

#endif
    }

    void swap_binarize_data(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

    /* Existing limitations
        L1: only single phase
    */

#define LOCAL_DEBUG_FLAG    0

#if 1   // local vars
        int     i_layer, i_data, i_cell, i_face;
        int     c0, c1, c;
        double  vol_flip;
#endif

        i_layer = rcfd->Solver->current_layer;

        swap_init(rcfd, i_phase);

        /* data as outer loop, because it is unlikely that many data get binarized */
        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            if(rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_BINARY){

                /* Init data_swap */
                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    _C(i_layer).data_swap[i_cell][i_data] = 0.0;
                }

                /* Artificial Diffusion (mass neutral) */
                for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

                    c0 = _F(i_layer).c0[i_face];
                    c1 = _F(i_layer).c1[i_face];

                    if(_C(i_layer).data[i_phase][c0][i_data] > _C(i_layer).data[i_phase][c1][i_data]){

                        c = c0; c0 = c1; c1 = c;
                    }

                    if(_C(i_layer).data[i_phase][c1][i_data] > _C(i_layer).data[i_phase][c0][i_data]){

                        if(_C(i_layer).volume[c0] < _C(i_layer).volume[c1]){

                            vol_flip = _C(i_layer).volume[c0] * rcfd->Data_Dict[i_phase][i_data].binarization_art_diff;
                        }
                        else{

                            vol_flip = _C(i_layer).volume[c1] * rcfd->Data_Dict[i_phase][i_data].binarization_art_diff;
                        }

                        _C(i_layer).data_swap[c1][i_data] -=   vol_flip
                                                                      * (  _C(i_layer).data[i_phase][c1][i_data]
                                                                         - _C(i_layer).data[i_phase][c0][i_data])
                                                                      * 0.5;

                        _C(i_layer).data_swap[c0][i_data] +=   vol_flip
                                                                      * (  _C(i_layer).data[i_phase][c1][i_data]
                                                                         - _C(i_layer).data[i_phase][c0][i_data])
                                                                      * 0.5;
                    }
                }

                /* Binarization (mass acting) */
                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    if(_C(i_layer).data[i_phase][i_cell][i_data] > 0.5){

                        _C(i_layer).data[i_phase][i_cell][i_data] = 1.0;
                    }
                    else{

                        _C(i_layer).data[i_phase][i_cell][i_data] = 0.0;
                    }
                }
            }
        }

#undef LOCAL_DEBUG_FLAG

#endif
    }

#endif
