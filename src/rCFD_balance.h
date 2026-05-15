/******************************************************************************
 * @internal
 * @file        rCFD_balance.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_BALANCE
#define RCFD_BALANCE

#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_topo.h"

    void balance_update_mass(rCFD_type* rcfd, const int i_layer, const int i_phase)
    {
#if RP_NODE

#if 1   // local vars
        int     i_data, i_cell, i_frame;
#endif

        // init. Balance.mass_integral
        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            rcfd->Balance[i_phase][i_data].mass_integral = 0.0;
        }

        i_frame = rcfd->Solver->current_frame;

        // accumulate Balance.mass_integral
        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                switch (rcfd->Data_Dict[i_phase][i_data].type){

                    case RCFD_DATA_TEMPERATURE:
                    {
                        rcfd->Balance[i_phase][i_data].mass_integral +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                                        * _C(i_layer).volume[i_cell]
                                                                        * _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                                        * rcfd->Phase_Dict[i_phase].density
                                                                        * rcfd->Phase_Dict[i_phase].heat_capacity;
                        break;
                    }
                    case RCFD_DATA_CONCENTRATION:
                    {
                        rcfd->Balance[i_phase][i_data].mass_integral +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                                        * _C(i_layer).volume[i_cell]
                                                                        * _C(i_layer).vof[i_frame][i_cell][i_phase]
                                                                        * rcfd->Phase_Dict[i_phase].density;
                        break;
                    }
                    case RCFD_DATA_GENERIC:
                    {
                        rcfd->Balance[i_phase][i_data].mass_integral += _C(i_layer).data[i_phase][i_cell][i_data]
                                                                        * _C(i_layer).volume[i_cell]
                                                                        * _C(i_layer).vof[i_frame][i_cell][i_phase];
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        // calc. Balance.mass_integral_global
        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            rcfd->Balance[i_phase][i_data].mass_integral_global = PRF_GRSUM1(rcfd->Balance[i_phase][i_data].mass_integral);
        }

#endif
    }

    void balance_init_source(rCFD_type* rcfd, const int i_layer, const int i_phase)
    {
#if RP_NODE

#if 1   // local vars
        int     i_data;
#endif

        // reset Balance.mass_source
        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            rcfd->Balance[i_phase][i_data].mass_source = 0.0;
            rcfd->Balance[i_phase][i_data].mass_source_global = 0.0;
        }

#endif
    }

    void balance_update_source(rCFD_type* rcfd, const int i_layer, const int i_phase)
    {
#if RP_NODE

#if 1   // local vars
        int     i_data;
#endif

        // add sources to Balance.mass_integral_target
        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            rcfd->Balance[i_phase][i_data].mass_integral_target += rcfd->Balance[i_phase][i_data].mass_source;
            rcfd->Balance[i_phase][i_data].mass_source_global = PRF_GRSUM1(rcfd->Balance[i_phase][i_data].mass_source);
            rcfd->Balance[i_phase][i_data].mass_integral_target_global += rcfd->Balance[i_phase][i_data].mass_source_global;
        }
#endif
    }

    void balance_update_error(rCFD_type* rcfd, const int i_layer, const int i_phase)
    {
#if RP_NODE

#if 1   // local vars
        int     i_data;
#endif

        // calc. Balance.mass_error
        for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

            rcfd->Balance[i_phase][i_data].mass_error =   rcfd->Balance[i_phase][i_data].mass_integral_target
                                                        - rcfd->Balance[i_phase][i_data].mass_integral;
            rcfd->Balance[i_phase][i_data].mass_error_global = PRF_GRSUM1(rcfd->Balance[i_phase][i_data].mass_error);
        }
#endif
    }

    void balance_update(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE
        int i_layer = rcfd->Solver->current_layer;

        balance_update_mass(rcfd, i_layer, i_phase);
        balance_update_source(rcfd, i_layer, i_phase);
        balance_update_error(rcfd, i_layer, i_phase);
#endif
    }

    /**
     * @brief Initialize the data_swap field of a specific phase and data with the given value
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_layer specifies the cell layer
     * @param i_data  specifies the data
     * @param value   the initialization value
     */
    void init_data_swap(rCFD_type* rcfd, int i_layer, int i_data, double value)
    {
#if RP_NODE
        int i_cell;

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

            _C(i_layer).data_swap[i_cell][i_data] = value;
        }
#endif
    }

    /**
     * @brief Calculate the exchange between cells for a single face
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_layer specifies the cell layer
     * @param i_phase specifies the phase
     * @param i_data  specifies the data
     * @param c0      1st face neighboring cell
     * @param c1      2nd face neighboring cell
     * @return the amount of face exchange or zero if called by ANSYS Fluent HOST
     */
    double get_face_exchange(rCFD_type* rcfd, int i_layer, int i_phase, int i_data, int c0, int c1)
    {
        double face_exchange = 0.0;

#if RP_NODE

        int     i_frame_c0 = rcfd->Solver->current_frame;
        int     i_frame_c1 = rcfd->Solver->current_frame;

        double  vol_flip = 0.0;

        if(   (_C(i_layer).volume[c0] * _C(i_layer).vof[i_frame_c0][c0][i_phase])
            < (_C(i_layer).volume[c1] * _C(i_layer).vof[i_frame_c1][c1][i_phase])){

            vol_flip = _C(i_layer).volume[c0] * _C(i_layer).vof[i_frame_c0][c0][i_phase];
        }
        else{

            vol_flip = _C(i_layer).volume[c1] * _C(i_layer).vof[i_frame_c1][c1][i_phase];
        }

        face_exchange = vol_flip * rcfd->Solver_Dict->face_swap_max_per_loop;

        if((rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_CONCENTRATION) ||
           (rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_TEMPERATURE)){

            face_exchange *= rcfd->Phase_Dict[i_phase].density;
        }
        if(rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_TEMPERATURE){

            face_exchange *= rcfd->Phase_Dict[i_phase].heat_capacity;
        }

#endif
        return face_exchange;
    }

    /**
     * @brief Calculate the global maximal available amount of face exchange
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_layer specifies the cell layer
     * @param i_phase specifies the phase
     * @param i_data  specifies the data
     * @return the global amount of face exchange or zero if called by ANSYS Fluent HOST
     */
    double get_available_face_exchange(rCFD_type* rcfd, int i_layer, int i_phase, int i_data)
    {
        double available_exchange = 0.0;

#if RP_NODE

        int i_face;
        int c0, c1, c;
        double face_exchange;

        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_int_faces; i_face++){

            c0 = _F(i_layer).c0[i_face];
            c1 = _F(i_layer).c1[i_face];

            if(_C(i_layer).data[i_phase][c0][i_data] > _C(i_layer).data[i_phase][c1][i_data]){

                c = c0; c0 = c1; c1 = c;
            }

            if(_C(i_layer).data[i_phase][c1][i_data] > _C(i_layer).data[i_phase][c0][i_data]){

                face_exchange = get_face_exchange(rcfd, i_layer, i_phase, i_data, c0, c1);

                available_exchange += 0.5
                                    * (_C(i_layer).data[i_phase][c1][i_data] - _C(i_layer).data[i_phase][c0][i_data])
                                    * face_exchange;
            }
        }

        if(rcfd->Balance_Dict[i_phase][i_data].type == RCFD_BALANCE_GLOBAL){

            available_exchange = PRF_GRSUM1(available_exchange);
        }

#endif

        return available_exchange;
    }

    /**
     * @brief Calculate the ratio between needed and available amount of face exchange
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_phase specifies the phase
     * @param i_data  specifies the data
     * @param available_exchange the globally available face exchange
     * @return the ratio between needed and available amount of face exchange or zero if called by ANSYS Fluent HOST
     */
    double get_face_exchange_ratio(rCFD_type* rcfd, int i_phase, int i_data, double available_exchange)
    {
        double face_exchange_ratio = 0.0;

#if RP_NODE

        if(rcfd->Balance_Dict[i_phase][i_data].type == RCFD_BALANCE_GLOBAL){

            if(available_exchange > fabs(rcfd->Balance[i_phase][i_data].mass_error_global)){

                if(available_exchange > 0.0){

                    face_exchange_ratio = fabs(rcfd->Balance[i_phase][i_data].mass_error_global)
                                          / available_exchange;
                }
            }
            else{

                face_exchange_ratio = 1.0;
            }
        }

#endif

        return face_exchange_ratio;
    }

    /**
     * @brief Set the data_swap field of a specific phase and data based on the given face exchange ratio
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_layer specifies the cell layer
     * @param i_phase specifies the phase
     * @param i_data  specifies the data
     * @param face_exchange_ratio ratio between globally needed and available face exchange
     */
    void set_data_swap(rCFD_type* rcfd, int i_layer, int i_phase, int i_data, double face_exchange_ratio)
    {
#if RP_NODE

        int i_face;
        int c0, c1, c;
        double face_exchange;

        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_int_faces; i_face++){

            c0 = _F(i_layer).c0[i_face];
            c1 = _F(i_layer).c1[i_face];

            if(_C(i_layer).data[i_phase][c0][i_data] > _C(i_layer).data[i_phase][c1][i_data]){

                c = c0, c1 = c0, c0 = c;
            }

            if(_C(i_layer).data[i_phase][c0][i_data] < _C(i_layer).data[i_phase][c1][i_data]){

                face_exchange = get_face_exchange(rcfd, i_layer, i_phase, i_data, c0, c1);

                if(rcfd->Balance[i_phase][i_data].mass_error_global > 0.0){

                    _C(i_layer).data_swap[c0][i_data] += 0.5
                                                                * (_C(i_layer).data[i_phase][c1][i_data] - _C(i_layer).data[i_phase][c0][i_data])
                                                                * face_exchange_ratio
                                                                * face_exchange;
                }

                if(rcfd->Balance[i_phase][i_data].mass_error_global < 0.0){

                    _C(i_layer).data_swap[c1][i_data] -= 0.5
                                                                * (_C(i_layer).data[i_phase][c1][i_data] - _C(i_layer).data[i_phase][c0][i_data])
                                                                * face_exchange_ratio
                                                                * face_exchange;
                }
            }
        }
#endif
    }

    /**
     * @brief updates data by data_swap and adjusts balance (helper fct for balance_correct)
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_layer specifies the cell layer
     * @param i_phase specifies the phase
     * @param i_data  specifies the data
     */
    void update_data_and_balance_by_swap(rCFD_type* rcfd, int i_layer, int i_phase, int i_data)
    {
#if RP_NODE

        int i_cell;
        int i_frame = rcfd->Solver->current_frame;

        rcfd->Balance[i_phase][i_data].mass_integral = 0.0;

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

            double cell_phase_mass = _C(i_layer).volume[i_cell]
                                   * _C(i_layer).vof[i_frame][i_cell][i_phase];

            if((rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_CONCENTRATION) ||
               (rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_TEMPERATURE)){

                cell_phase_mass *=  rcfd->Phase_Dict[i_phase].density;
            }

            if(rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_TEMPERATURE){

                cell_phase_mass *=  rcfd->Phase_Dict[i_phase].heat_capacity;
            }

            if(cell_phase_mass > 0.0){

                _C(i_layer).data[i_phase][i_cell][i_data] = (   _C(i_layer).data[i_phase][i_cell][i_data]
                                                              * cell_phase_mass
                                                              + _C(i_layer).data_swap[i_cell][i_data]
                                                            )
                                                            / cell_phase_mass;
            }

            rcfd->Balance[i_phase][i_data].mass_integral +=   _C(i_layer).data[i_phase][i_cell][i_data]
                                                            * cell_phase_mass;
        }

        rcfd->Balance[i_phase][i_data].mass_integral_global = PRF_GRSUM1(rcfd->Balance[i_phase][i_data].mass_integral);
        rcfd->Balance[i_phase][i_data].mass_error_global =    rcfd->Balance[i_phase][i_data].mass_integral_target_global
                                                            - rcfd->Balance[i_phase][i_data].mass_integral_global;
#endif
    }

    /**
     * @brief checks if balance is allready ok (helper fct for balance_correct)
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_phase specifies the phase
     * @param i_data  specifies the data
     * @return 1 if balance error is within accuracy or 0 elsewise
     */
    int balance_error_within_accuracy_limit(rCFD_type* rcfd, int i_phase, int i_data)
    {
#if RP_NODE
        if(  fabs(rcfd->Balance[i_phase][i_data].mass_error_global)
           < fabs(  rcfd->Balance_Dict[i_phase][i_data].accuracy_level
                  * rcfd->Balance[i_phase][i_data].mass_integral_target_global)){

            return 1;
        }
#endif
        return 0;
    }

    /**
     * @brief Correct all data fields of the specified phase by global balance
     *
     * @param rcfd    pointer to previously created rCFD instance
     * @param i_phase specifies the phase
     */
    void balance_correct(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

#if 1   // local vars
        int     i_layer, i_data, i_while;
        int     balance_error_exists;
        double  available_face_exchange, face_exchange_ratio;
#endif

        i_layer = rcfd->Solver->current_layer;

        if((rcfd->Solver->global_run_counter % rcfd->Solver_Dict->balance_correction_update) == 0){

            for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                i_while = 0;
                balance_error_exists = 1;

                while(   (balance_error_exists)
                      && (i_while < rcfd->Balance_Dict[i_phase][i_data].max_correction_loops)){

                    init_data_swap(rcfd, i_layer, i_data, 0.0);

                    available_face_exchange = get_available_face_exchange(rcfd, i_layer, i_phase, i_data);

                    face_exchange_ratio = get_face_exchange_ratio(rcfd, i_phase, i_data, available_face_exchange);

                    set_data_swap(rcfd, i_layer, i_phase, i_data, face_exchange_ratio);

                    update_data_and_balance_by_swap(rcfd, i_layer, i_phase, i_data);

                    if(balance_error_within_accuracy_limit(rcfd, i_phase, i_data)){

                        balance_error_exists = 0;
                    }
                    else{

                        i_while++;
                    }
                }

                if(balance_error_exists){

                    report_warning0(rcfd, RCFD_REPORT_CATEGORY_RUN_BALANCE,
                                    "Balance for i_phase %d i_data %d beyond accuracy level %f",
                                    i_phase, i_data, rcfd->Balance_Dict[i_phase][i_data].accuracy_level);
                }
            }
        }
#endif
    }

    void balance_control_conc_sum(rCFD_type* rcfd, const int i_phase)
    {
#if RP_NODE

#if 1   // local vars
        int     i_layer, i_data, i_cell;
        double  sum_of_conc;
#endif

        i_layer = rcfd->Solver->current_layer;

        if(rcfd->Solver_Dict->control_conc_sum_on){

            if(rcfd->Phase_Dict[i_phase].number_of_concentration_data > 1){

                for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                    sum_of_conc = 0.0;

                    // crop conc values to [0..1], calc sum_of_conc
                    for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                        if(rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_CONCENTRATION){

                            if(_C(i_layer).data[i_phase][i_cell][i_data] < 0.0){

                                _C(i_layer).data[i_phase][i_cell][i_data] = 0.0;
                            }
                            else if(_C(i_layer).data[i_phase][i_cell][i_data] > 1.0){

                                _C(i_layer).data[i_phase][i_cell][i_data] = 1.0;
                            }

                            sum_of_conc += _C(i_layer).data[i_phase][i_cell][i_data];
                        }
                    }

                    // adapt conc values
                    for(i_data = 0; i_data < rcfd->Phase_Dict[i_phase].number_of_data; i_data++){

                        if(rcfd->Data_Dict[i_phase][i_data].type == RCFD_DATA_CONCENTRATION){

                            if(sum_of_conc > 0.0){

                                _C(i_layer).data[i_phase][i_cell][i_data] /= sum_of_conc;
                            }
                        }
                    }
                }
            }
        }
#endif
    }

#endif
