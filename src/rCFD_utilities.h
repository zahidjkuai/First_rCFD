/******************************************************************************
 * @internal
 * @file        rCFD_utilities.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_UTILITIES_H
#define RCFD_UTILITIES_H

/**
 * @brief checks if point is inside bounding box (BB)
 * @param x_point   pointer to be checked
 * @param x_min     min point of BB
 * @param x_max     max point of BB
 * @return 1 if point is inside BB or 0 if not
 */
int is_point_inside_box(double *x_point, double *x_min, double *x_max)
{
#if RP_NODE

    if((x_point[0] > x_min[0]) && (x_point[0] < x_max[0]) &&
       (x_point[1] > x_min[1]) && (x_point[1] < x_max[1]) &&
       (x_point[2] > x_min[2]) && (x_point[2] < x_max[2])){

           return 1;
    }
#endif

    return 0;
}

/**
 * @brief calculate squared distance between point and cell center
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_cell    cell index
 * @param x         point coordinates
 * @return distance or -1 if no RP_NODE
 */
double cell2point_distance_sq(rCFD_type *rcfd, int i_layer, int i_cell, const double *x)
{
#if RP_NODE
    const double *cell_center = _C(i_layer).x[i_cell];

    return (cell_center[0] - x[0]) * (cell_center[0] - x[0]) +
           (cell_center[1] - x[1]) * (cell_center[1] - x[1]) +
           (cell_center[2] - x[2]) * (cell_center[2] - x[2]);
#else
    return -1.0;
#endif
}

/**
 * @brief Find the closest (internal) cell a given point
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param i_layer   specifies the cell laye
 * @param x         point coordinates
 * @param dist_sq   receives the squared distance between \p x and the cell center
 * @return local index of the cell closest to \p x
 */
int find_closest_cell_to_point(rCFD_type *rcfd, int i_layer, const double *x, double *dist_sq)
{
#if RP_NODE
    int     i_cell, closest_cell = -1;

    double  min_cell_distance_sq = 1e200;
    double  cell_distance_sq = 0.0;

    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

        cell_distance_sq = cell2point_distance_sq(rcfd, i_layer, i_cell, x);

        if(cell_distance_sq < min_cell_distance_sq){

            min_cell_distance_sq = cell_distance_sq;
            closest_cell = i_cell;
        }
    }

    if(dist_sq) *dist_sq = min_cell_distance_sq;

    return closest_cell;
#else
    return -1;
#endif
}

/**
 * @brief calculate distance between point and cell center
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param i_cell    cell index
 * @param x         point coordinates
 * @return distance or -1 if no RP_NODE
 */
double cell2point_distance(rCFD_type *rcfd, int i_layer, int i_cell, const double *x)
{
#if RP_NODE
    return sqrt(cell2point_distance_sq(rcfd, i_layer, i_cell, x));
#else
    return -1.0;
#endif
}

/**
 * @brief calculate distance between cell centers
 *
 * @param rcfd      pointer to previously created rCFD instance
 * @param i_layer   specifies the cell layer
 * @param c0        specifies the first cell index
 * @param c1        specifies the second cell index
 * @return distance between cell centers
 */
double cell2cell_distance(rCFD_type *rcfd, int i_layer, int c0, int c1)
{
#if RP_NODE
    int     i_dim;

    double  cell_distance = 0.0;

    for(i_dim = 0; i_dim < 3; i_dim++){

        cell_distance +=
            (_C(i_layer).x[c0][i_dim] - _C(i_layer).x[c1][i_dim])
          * (_C(i_layer).x[c0][i_dim] - _C(i_layer).x[c1][i_dim]);
    }

    cell_distance = sqrt(cell_distance);

    return cell_distance;
#else
    return -1.0;
#endif
}

/**
 * @brief calculate scalar projection of vector a on vector b (ab/|b|)
 *
 * @param a_vec first vector
 * @param b_vec second vector
 * @return scalar projection of vector a on vector b or 0 if invalid args
 */
double scalar_projection(const double *a_vec, const double *b_vec)
{
    if((!a_vec) || (!b_vec)) return 0.0;

    int     i_dim;

    double  ab, b;

    ab = 0.0; b = 0.0;

    for(i_dim = 0; i_dim < 3; i_dim++){

        ab += a_vec[i_dim] * b_vec[i_dim];

        b += b_vec[i_dim] * b_vec[i_dim];
    }

    // return the length of b_vec projected onto a_vec
    if(b > 0.0){

        b = sqrt(b);
        return ab / b;
    }

    return 0.0;
}

/**
 * @brief calculates vector magnitude (|a|)
 *
 * @param a_vec vector
 * @return vector magnitude or 0 if invalid args
 */
double vector_magnitude(const double *a_vec)
{
    if(!a_vec) return 0.0;

    int i_dim;

    double magnitude = 0.0;

    for(i_dim = 0; i_dim < 3; i_dim++){

        magnitude += a_vec[i_dim] * a_vec[i_dim];
    }

    return sqrt(magnitude);
}

/**
 * @brief calculate the cross product of two vectors (a x b)
 *
 * @param a_vec 1st vector
 * @param b_vec 2nd vector
 * @param axb resulting cross product
 */
void vector_cross_product(const double *a_vec, const double *b_vec, double *axb)
{
    if(!a_vec || !b_vec || !axb)
        return;

    axb[0] =  a_vec[1] * b_vec[2] - a_vec[2] * b_vec[1];
    axb[1] = -a_vec[0] * b_vec[2] + a_vec[2] * b_vec[0];
    axb[2] =  a_vec[0] * b_vec[1] - a_vec[1] * b_vec[0];

    return;
}

/**
 * @brief calculate inverse of matrix M
 *
 * @param M     pointer to matrix
 * @param M_inv pointer to inverse matrix
 * @return determinant of matrix M or 0 if invalid args
 */
double inverse_3x3(double **M, double **M_inv)
{
    if(!M || !M_inv)
        return 0;

    const double c00 = M[1][1] * M[2][2] - M[1][2] * M[2][1];
    const double c10 = M[1][2] * M[2][0] - M[1][0] * M[2][2];
    const double c20 = M[1][0] * M[2][1] - M[1][1] * M[2][0];
    const double det = M[0][0] * c00 + M[0][1] * c10 + M[0][2] * c20;

    if(det != 0.0){

        const double det_inv = 1.0 / det;

        M_inv[0][0] = c00 * det_inv;
        M_inv[0][1] = (M[0][2] * M[2][1] - M[0][1] * M[2][2]) * det_inv;
        M_inv[0][2] = (M[0][1] * M[1][2] - M[0][2] * M[1][1]) * det_inv;
        M_inv[1][0] = c10 * det_inv;
        M_inv[1][1] = (M[0][0] * M[2][2] - M[0][2] * M[2][0]) * det_inv;
        M_inv[1][2] = (M[0][2] * M[1][0] - M[0][0] * M[1][2]) * det_inv;
        M_inv[2][0] = c20 * det_inv;
        M_inv[2][1] = (M[0][1] * M[2][0] - M[0][0] * M[2][1]) * det_inv;
        M_inv[2][2] = (M[0][0] * M[1][1] - M[0][1] * M[1][0]) * det_inv;
    }

    return det;
}


/**
 * @brief Swaps the values of two int pointers
 *
 * @param xp first pointer
 * @param yp second pointer
 */
void swap_i(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

/**
 * @brief Performs selection sort in decending order but will keep the original
 *        arr unsorted and just return sorted ids.
 *
 * @param arr       double array to be sorted
 * @param n         length of arr
 * @param ascending if 0: orders in descending order - else: orders in ascending order
 * @return int array containing sorted ids, must be freed by caller
 */
int* selectionArgsort(const double *arr, int n, short ascending)
{
    int i, j, max_idx;
    int *ids = (int*)malloc(n * sizeof(int));

    // initialise the ids array
    // (at first arr[i] == arr[ids[i]])
    for(i = 0; i < n; i++){

        ids[i] = i;
    }

    // One by one move boundary of unsorted subarray
    for(i = 0; i < n - 1; i++){

        // Find the minimum element in unsorted array
        max_idx = i;

        for(j = i + 1; j < n; j++){

            if(( ascending && arr[ids[j]] < arr[ids[max_idx]]) ||
               (!ascending && arr[ids[j]] > arr[ids[max_idx]])){

                max_idx = j;
            }
        }

        // Swap the found minimum element with the first element
        swap_i(&ids[max_idx], &ids[i]);
    }

    return ids;
}

int array_contains_value(const int *arr, int len, int value)
{
    int i;
    for(i = 0; i < len; i++){
        if(arr[i] == value)
            return 1;
    }
    return 0;
}
#endif
