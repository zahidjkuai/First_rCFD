/******************************************************************************
 * @internal
 * @file        rCFD_topo.h
 * @copyright   (C) 2023
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_TOPO
#define RCFD_TOPO

#include "rCFD_types.h"
#include "rCFD_macros.h"
#include "rCFD_utilities.h"

/**
 * @brief Read the cell count from the (process-specific) topology file
 * @param rcfd  pointer to rCFD instance
 * @return the number of cells stored in the file
 */
int topo_read_cell_count(rCFD_type* rcfd)
{
    int     number_of_cells = 0;

#if RP_NODE
    FILE    *f_topo = NULL;
    char    filename[80];
    int     number_of_faces = 0;
    int     number_of_nodes = 0;

    snprintf(filename, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);

    f_topo = fopen(filename, "r");

    if(f_topo == NULL){

        report_info(rcfd, RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
                    "[%d] topo_read_cell_count: failed to open: '%s'. "
                    "Expecting topology to be set via user functions.",
                    myid, filename);
        return 0;
    }

    fscanf(f_topo, "%d %d %d \n", &number_of_cells, &number_of_faces, &number_of_nodes);

    fclose(f_topo);

#endif

    return number_of_cells;
}

/**
 * @brief Read the face count from the (process-specific) topology file
 * @param rcfd  pointer to rCFD instance
 * @return the number of faces stored in the file
 */
int topo_read_face_count(rCFD_type* rcfd)
{
    int     number_of_faces = 0;

#if RP_NODE
    FILE    *f_topo = NULL;
    char    filename[80];
    int     number_of_cells = 0;
    int     number_of_nodes = 0;

    snprintf(filename, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);

    f_topo = fopen(filename, "r");

    if(f_topo == NULL){

        report_info(rcfd, RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
                    "[%d] topo_read_face_count: failed to open: '%s'. "
                    "Expecting topology to be set via user functions.",
                    myid, filename);
        return 0;
    }

    fscanf(f_topo, "%d %d %d \n", &number_of_cells, &number_of_faces, &number_of_nodes);

    fclose(f_topo);

#endif

    return number_of_faces;
}

/**
 * @brief Read the node count from the (process-specific) topology file
 * @param rcfd  pointer to rCFD instance
 * @return the number of nodes stored in the file
 */
int topo_read_node_count(rCFD_type* rcfd)
{
    int     number_of_nodes = 0;

#if RP_NODE
    FILE    *f_topo = NULL;
    char    filename[80];
    int     number_of_cells = 0;
    int     number_of_faces = 0;

    snprintf(filename, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);

    f_topo = fopen(filename, "r");

    if(f_topo == NULL){

        report_info(rcfd, RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
                    "[%d] topo_read_node_count: failed to open: '%s'. "
                    "No node information will be available in data dump.",
                    myid, filename);
        return 0;
    }

    fscanf(f_topo, "%d %d %d \n", &number_of_cells, &number_of_faces, &number_of_nodes);

    fclose(f_topo);

#endif

    return number_of_nodes;
}

/**
 * @brief Read cell data from the given (process-specific) topology file
 * @param rcfd              pointer to rCFD instance
 * @param i_layer           specifies the layer of the topology data
 * @param f_topo            pointer to open topology file with position indicator assumed to be at the start of the cell section
 * @param number_of_cells   the number of cells to read
 * @retval -1 Error
 * @retval 0 OK
 */
int topo_read_cells(rCFD_type* rcfd, int i_layer, FILE *f_topo, int number_of_cells)
{
#if RP_NODE
    int i_cell;
    int number_of_int_cells = 0;
    int host;

    if(_CELL_DICT(i_layer).number_of_cells > 0 && _CELL_DICT(i_layer).number_of_cells != number_of_cells){

        report_info(rcfd, RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
                    "[%d] topo_read_cells: mismatch of cell count in Cell_Dict (%d) and Topo file (%d)."
                    "Expecting topology to be set via user functions",
                    myid, _CELL_DICT(i_layer).number_of_cells, number_of_cells);

        fclose(f_topo);
        return -1;
    }

    _CELL_DICT(i_layer).number_of_cells = number_of_cells;

    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

        fscanf(f_topo, "%le %le %le %le %d \n", &_C(i_layer).x[i_cell][0], &_C(i_layer).x[i_cell][1], &_C(i_layer).x[i_cell][2], &_C(i_layer).volume[i_cell], &host);

        if(host == myid){

            ++number_of_int_cells;
        }
    }

    _CELL_DICT(i_layer).number_of_int_cells = number_of_int_cells;

    _CELL_DICT(i_layer).number_of_ext_cells = number_of_cells - number_of_int_cells;
#endif
    return 0;
}

/**
 * @brief Read face data from the given (process-specific) topology file
 * @param rcfd              pointer to rCFD instance
 * @param i_layer           specifies the layer of the topology data
 * @param f_topo            pointer to open topology file with position indicator assumed to be at the start of the face section
 * @param number_of_faces   the number of faces to read
 * @retval -1 Error
 * @retval 0 OK
 */
int topo_read_faces(rCFD_type* rcfd, int i_layer, FILE *f_topo, int number_of_faces)
{
#if RP_NODE
    int i_face;
    int number_of_int_faces = 0;

    _FACE_DICT(i_layer).number_of_faces = number_of_faces;

    for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

        fscanf(f_topo, "%d %d %le %le %le \n", &_F(i_layer).c0[i_face], &_F(i_layer).c1[i_face], &_F(i_layer).area[i_face][0], &_F(i_layer).area[i_face][1], &_F(i_layer).area[i_face][2]);

        if(_F(i_layer).c1[i_face] < _CELL_DICT(i_layer).number_of_int_cells){ // c0 is always internal

            ++number_of_int_faces;
        }
    }

    _FACE_DICT(i_layer).number_of_int_faces = number_of_int_faces;

    _FACE_DICT(i_layer).number_of_ext_faces = number_of_faces - number_of_int_faces;
#endif
    return 0;
}

/**
 * @brief set cell shape type and nodes of the given cell; cell shapes must be set in consecutive ascending order of cell indices;
 *          cell shapes must not be reset;
 * @param rcfd              pointer to rCFD instance
 * @param i_layer           specifies the layer of the topology data
 * @param i_cell            specifies the cell to define
 * @param cell_type         specifies cell shape type (one of CELL_TYPE as in VTK)
 * @param cell_node_ids     array containing the number of cell nodes followed by the node indices
 * @param poly_face_nodes   in case of polyhedral cells, array containing the number of polyfaces followed by the the node count of each face,
 *                          otherwise might be NULL
 * @retval -1 Error
 * @retval 0 OK
 */
int topo_set_cell_nodes(rCFD_type *rcfd, int i_layer, int i_cell, int cell_type, const int *cell_node_ids, const int *poly_face_nodes)
{
#if RP_NODE
    int i_face, i_node;
    int i_polyhedron = _C(i_layer).number_of_polyhedra;
    int number_of_cell_nodes;

    // check for valid input
    if(!cell_node_ids)
        return -1;

    // check if cell shape memory is allocated (i.e. if dictionary was set up correctly)
    if(!_C(i_layer).type || !_C(i_layer).nodes)
        return -1;

    // check if this cell was already set
    if(_C(i_layer).number_of_nodes[i_cell] != 0)
        return -1;

    number_of_cell_nodes = cell_node_ids[0];

    switch(cell_type){

    case RCFD_CELL_VERTEX:
        if(number_of_cell_nodes != 1) return -1;
        break;
    case RCFD_CELL_TRIANGLE:
        if(number_of_cell_nodes != 3) return -1;
        break;
    case RCFD_CELL_QUADRILATERAL:
    case RCFD_CELL_TETRAHEDRON:
        if(number_of_cell_nodes != 4) return -1;
        break;
    case RCFD_CELL_PYRAMID:
        if(number_of_cell_nodes != 5) return -1;
        break;
    case RCFD_CELL_WEDGE:
        if(number_of_cell_nodes != 6) return -1;
        break;
    case RCFD_CELL_HEXAHEDRON:
        if(number_of_cell_nodes != 8) return -1;
        break;
    case RCFD_CELL_POLYHEDRON:
        if(!poly_face_nodes) return -1;

        _C(i_layer).number_of_poly_faces = realloc(_C(i_layer).number_of_poly_faces, sizeof(int) * (i_polyhedron+1));
        _C(i_layer).number_of_poly_face_nodes = realloc_i_2d(_C(i_layer).number_of_poly_face_nodes, i_polyhedron+1, RCFD_MAX_FACES_PER_POLY_CELL);

        _C(i_layer).number_of_poly_faces[i_polyhedron] = poly_face_nodes[0];

        for(i_face = 0; i_face < _C(i_layer).number_of_poly_faces[i_polyhedron]; ++i_face){

            _C(i_layer).number_of_poly_face_nodes[i_polyhedron][i_face] = poly_face_nodes[1 + i_face];
        }

        _C(i_layer).poly_nodes = realloc(_C(i_layer).poly_nodes, sizeof(int) * (_C(i_layer).number_of_poly_nodes+number_of_cell_nodes));
        break;
    default:
        report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                     "[%d] topo_set_cell_nodes: unknown cell type '%d' for cell %d. "
                     "Cell node information will not be available for dump.",
                     myid, _C(i_layer).type[i_cell], i_cell);

        _NODE_DICT(i_layer).number_of_nodes = -1;
        return -1;
    }

    _C(i_layer).type[i_cell] = cell_type;

    if(i_cell < _CELL_DICT(i_layer).number_of_int_cells){

        _NODE_DICT(i_layer).number_of_int_node_references += number_of_cell_nodes;

        /* in case of polyhedra add number of faces and number of nodes per face to dump right amount of data in VTK */
        if(_C(i_layer).type[i_cell] == RCFD_CELL_POLYHEDRON){

            _NODE_DICT(i_layer).number_of_int_node_references += 1 + _C(i_layer).number_of_poly_faces[i_polyhedron];
        }
    }
    else{

        _NODE_DICT(i_layer).number_of_ext_node_references += number_of_cell_nodes;

        if(_C(i_layer).type[i_cell] == RCFD_CELL_POLYHEDRON){

            _NODE_DICT(i_layer).number_of_ext_node_references += 1 + _C(i_layer).number_of_poly_faces[i_polyhedron];
        }
    }

    if(_C(i_layer).type[i_cell] == RCFD_CELL_POLYHEDRON){

        for(i_node = 0; i_node < number_of_cell_nodes; ++i_node){

            _C(i_layer).poly_nodes[_C(i_layer).number_of_poly_nodes+i_node] = cell_node_ids[1 + i_node];
        }

        _C(i_layer).number_of_poly_nodes += number_of_cell_nodes;
        ++i_polyhedron;
    }
    else{

        for(i_node = 0; i_node < number_of_cell_nodes; ++i_node){

            _C(i_layer).nodes[i_cell][i_node] = cell_node_ids[1 + i_node];
        }
    }

    _C(i_layer).number_of_nodes[i_cell] = number_of_cell_nodes;
    _C(i_layer).number_of_polyhedra = i_polyhedron;

#endif // RP_NODE
    return 0;
}

/**
 * @brief Read cell node data from the given (process-specific) topology file
 * @param rcfd              pointer to rCFD instance
 * @param i_layer           specifies the layer of the topology data
 * @param f_topo            pointer to open topology file with position indicator assumed to be at the start of the node section
 * @param number_of_nodes   the number of nodes to read
 * @retval -1 Error
 * @retval 0 OK
 */
int topo_read_nodes(rCFD_type* rcfd, int i_layer, FILE *f_topo, int number_of_nodes)
{
#if RP_NODE
    if(_NODE_DICT(i_layer).number_of_nodes != number_of_nodes){
        report_error(rcfd, RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
                     "[%d] topo_read_nodes: inconsistent cell node count. "
                     "Cell node information will not be available for dump.",
                     myid);

        _NODE_DICT(i_layer).number_of_nodes = -1;
        fclose(f_topo);
        return -1;
    }

    if(number_of_nodes > 0){

        int i_cell;
        int i_face;
        int i_node;
        int i_polyhedron = 0;
        int number_of_cell_nodes;

        // read node positions
        for(i_node = 0; i_node < _NODE_DICT(i_layer).number_of_nodes; ++i_node){

            fscanf(f_topo, "%le %le %le \n", &_N(i_layer).x[i_node][0], &_N(i_layer).x[i_node][1], &_N(i_layer).x[i_node][2]);
        }

        // read cell type and node IDs
        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            fscanf(f_topo, "%d ", &_C(i_layer).type[i_cell]);

            number_of_cell_nodes = 0;

            switch(_C(i_layer).type[i_cell]){

            case RCFD_CELL_VERTEX:
                number_of_cell_nodes = 1;
                break;
            case RCFD_CELL_TRIANGLE:
                number_of_cell_nodes = 3;
                break;
            case RCFD_CELL_QUADRILATERAL:
            case RCFD_CELL_TETRAHEDRON:
                number_of_cell_nodes = 4;
                break;
            case RCFD_CELL_PYRAMID:
                number_of_cell_nodes = 5;
                break;
            case RCFD_CELL_WEDGE:
                number_of_cell_nodes = 6;
                break;
            case RCFD_CELL_HEXAHEDRON:
                number_of_cell_nodes = 8;
                break;
            case RCFD_CELL_POLYHEDRON:
                number_of_cell_nodes = 0;
                _C(i_layer).number_of_poly_faces = realloc(_C(i_layer).number_of_poly_faces, sizeof(int) * (i_polyhedron+1));
                _C(i_layer).number_of_poly_face_nodes = realloc_i_2d(_C(i_layer).number_of_poly_face_nodes, i_polyhedron+1, RCFD_MAX_FACES_PER_POLY_CELL);

                fscanf(f_topo, "%d ", &_C(i_layer).number_of_poly_faces[i_polyhedron]);

                for(i_face = 0; i_face < _C(i_layer).number_of_poly_faces[i_polyhedron]; ++i_face){

                    fscanf(f_topo, "%d ", &_C(i_layer).number_of_poly_face_nodes[i_polyhedron][i_face]);
                    number_of_cell_nodes += _C(i_layer).number_of_poly_face_nodes[i_polyhedron][i_face];
                }

                _C(i_layer).poly_nodes = realloc(_C(i_layer).poly_nodes, sizeof(int) * (_C(i_layer).number_of_poly_nodes+number_of_cell_nodes));
                break;
            default:
                report_error(rcfd, RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
                             "[%d] topo_read_nodes: unknown cell type '%d' for cell %d. "
                             "Cell node information will not be available for dump.",
                             myid, _C(i_layer).type[i_cell], i_cell);

                _NODE_DICT(i_layer).number_of_nodes = -1;
                fclose(f_topo);
                return -1;
            }


            if(i_cell < _CELL_DICT(i_layer).number_of_int_cells){

                _NODE_DICT(i_layer).number_of_int_node_references += number_of_cell_nodes;

                /* in case of polyhedra add number of faces and number of nodes per face to dump right amount of data in VTK */
                if(_C(i_layer).type[i_cell] == RCFD_CELL_POLYHEDRON){

                    _NODE_DICT(i_layer).number_of_int_node_references += 1 + _C(i_layer).number_of_poly_faces[i_polyhedron];
                }
            }
            else{

                _NODE_DICT(i_layer).number_of_ext_node_references += number_of_cell_nodes;

                if(_C(i_layer).type[i_cell] == RCFD_CELL_POLYHEDRON){

                    _NODE_DICT(i_layer).number_of_ext_node_references += 1 + _C(i_layer).number_of_poly_faces[i_polyhedron];
                }
            }

            if(_C(i_layer).type[i_cell] == RCFD_CELL_POLYHEDRON){

                for(i_node = 0; i_node < number_of_cell_nodes; ++i_node){

                    fscanf(f_topo, "%d ", &_C(i_layer).poly_nodes[_C(i_layer).number_of_poly_nodes+i_node]);
                }

                _C(i_layer).number_of_poly_nodes += number_of_cell_nodes;
                ++i_polyhedron;
            }
            else{

                for(i_node = 0; i_node < number_of_cell_nodes; ++i_node){

                    fscanf(f_topo, "%d ", &_C(i_layer).nodes[i_cell][i_node]);
                }
            }

            fscanf(f_topo, "\n");

            _C(i_layer).number_of_nodes[i_cell] = number_of_cell_nodes;
            _C(i_layer).number_of_polyhedra = i_polyhedron;
        }
    }
#endif
    return 0;
}

/**
 * @brief Read cell, face and node data from (process-specific) topology files
 * @param rcfd  pointer to rCFD instance
 */
void topo_read(rCFD_type* rcfd)
{
#if RP_NODE
    FILE    *f_topo = NULL;
    int     i_layer = 0;
    int     number_of_cells = 0;
    int     number_of_faces = 0;
    int     number_of_nodes = 0;

    // open topo file
    {
        char    filename[80];

        snprintf(filename, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);

        f_topo = fopen(filename, "r");

        if(f_topo == NULL){

            report_info(rcfd, RCFD_REPORT_CATEGORY_READ_TOPOLOGY,
                        "[%d] topo_read: failed to open '%s'. "
                        "Expecting topology to be set via user functions",
                        myid, filename);
            return;
        }
    }

    fscanf(f_topo, "%d %d %d \n", &number_of_cells, &number_of_faces, &number_of_nodes);

    // read cell data
    if(topo_read_cells(rcfd, i_layer, f_topo, number_of_cells) < 0)
        return;

    // read face data
    if(topo_read_faces(rcfd, i_layer, f_topo, number_of_faces) < 0)
        return;

    // read cell node data
    if(topo_read_nodes(rcfd, i_layer, f_topo, number_of_nodes) < 0)
        return;

    // close topo file
    fclose(f_topo);
#endif
}

/**
 * @brief Write cell data to the given (process-specific) topology file
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the layer of the topology data
 * @param f_topo    pointer to open topology file with position indicator assumed to be at the start of the cell section
 * @retval -1 Error
 * @retval 0 OK
 */
int topo_write_cells(rCFD_type* rcfd, int i_layer, FILE *f_topo)
{
#if RP_NODE
    int i_cell;

    if(_CELL_DICT(i_layer).number_of_cells <= 0){

        report_info(rcfd, RCFD_REPORT_CATEGORY_WRITE_TOPOLOGY, "[%d] topo_write_cells: no cells to write", myid);

        fclose(f_topo);
        return -1;
    }

    // write internal cells
    for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_int_cells; i_cell++){

        fprintf(f_topo, "%e %e %e %e %d \n",
                _C(i_layer).x[i_cell][0], _C(i_layer).x[i_cell][1], _C(i_layer).x[i_cell][2],
                _C(i_layer).volume[i_cell], myid);
    }
    // write external cells
    for(; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

        fprintf(f_topo, "%e %e %e %e %d \n",
                _C(i_layer).x[i_cell][0], _C(i_layer).x[i_cell][1], _C(i_layer).x[i_cell][2],
                _C(i_layer).volume[i_cell], _MPI_Cell.host_of_cell[i_cell]);
    }
#endif
    return 0;
}

/**
 * @brief Write face data to the given (process-specific) topology file
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the layer of the topology data
 * @param f_topo    pointer to open topology file with position indicator assumed to be at the start of the face section
 * @retval -1 Error
 * @retval 0 OK
 */
int topo_write_faces(rCFD_type* rcfd, int i_layer, FILE *f_topo)
{
#if RP_NODE
    int i_face;

    if(_FACE_DICT(i_layer).number_of_faces <= 0){

        report_info(rcfd, RCFD_REPORT_CATEGORY_WRITE_TOPOLOGY, "[%d] topo_write_faces: no faces to write", myid);

        fclose(f_topo);
        return -1;
    }

    for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

        fprintf(f_topo, "%d %d %e %e %e \n",
               _F(i_layer).c0[i_face], _F(i_layer).c1[i_face],
               _F(i_layer).area[i_face][0], _F(i_layer).area[i_face][1], _F(i_layer).area[i_face][2]);
    }
#endif
    return 0;
}

/**
 * @brief Write cell node data to the given (process-specific) topology file
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the layer of the topology data
 * @param f_topo    pointer to open topology file with position indicator assumed to be at the start of the node section
 * @retval -1 Error
 * @retval 0 OK
 */
int topo_write_nodes(rCFD_type* rcfd, int i_layer, FILE *f_topo)
{
#if RP_NODE
    if(_NODE_DICT(i_layer).number_of_nodes <= 0){

        report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_TOPOLOGY, "[%d] topo_write_nodes: no cell nodes to write. ", myid);

        fclose(f_topo);
        return -1;
    }

    {
        int i_cell, i_face, i_node;
        int i_polyhedron = 0, i_poly_node = 0;
        int cell_type;
        int number_of_cell_nodes;

        // write node positions
        for(i_node = 0; i_node < _NODE_DICT(i_layer).number_of_nodes; ++i_node){

            fprintf(f_topo, "%e %e %e \n",
                    _N(i_layer).x[i_node][0], _N(i_layer).x[i_node][1], _N(i_layer).x[i_node][2]);
        }

        // write cell type and node IDs
        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            cell_type = _C(i_layer).type[i_cell];
            number_of_cell_nodes = _C(i_layer).number_of_nodes[i_cell];

            // write the cell type
            fprintf(f_topo, "%d ", cell_type);

            if(cell_type == RCFD_CELL_POLYHEDRON){

                int n_faces = _C(i_layer).number_of_poly_faces[i_polyhedron];

                // write face count
                fprintf(f_topo, "%d ", n_faces);

                // write the node count for each face
                for(i_face = 0; i_face < n_faces; ++i_face){

                    fprintf(f_topo, "%d ", _C(i_layer).number_of_poly_face_nodes[i_polyhedron][i_face]);
                }

                // write the nodes of all cell faces
                for(i_node = 0; i_node < number_of_cell_nodes; ++i_node, ++i_poly_node){

                    fprintf(f_topo, "%d ", _C(i_layer).poly_nodes[i_poly_node]);
                }

                ++i_polyhedron;
            }
            else{

                for(i_node = 0; i_node < number_of_cell_nodes; ++i_node){

                    fprintf(f_topo, "%d ", _C(i_layer).nodes[i_cell][i_node]);
                }
            }

            fprintf(f_topo, "\n");
        }
    }
#endif
    return 0;
}

/**
 * @brief Write cell, face and node data to (process-specific) topology files
 * @param rcfd  pointer to rCFD instance
 */
int topo_write(rCFD_type* rcfd)
{
#if RP_NODE
    FILE    *f_topo = NULL;
    int     i_layer = 0;
    int     write_cell_nodes = 1;
    int     number_of_cells, number_of_faces, number_of_nodes;
    char    file_name[80];

    // topo path
    {
        snprintf(file_name, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);

        f_topo = fopen(file_name, "w");

        if(f_topo == NULL){

            report_error(rcfd, RCFD_REPORT_CATEGORY_WRITE_TOPOLOGY,
                         "[%d] topo_write: failed to open file %s",
                         myid, file_name);
            return -1;
        }
    }

    number_of_cells = _CELL_DICT(i_layer).number_of_cells;
    number_of_faces = _FACE_DICT(i_layer).number_of_faces;
    number_of_nodes = _NODE_DICT(i_layer).number_of_nodes;

    fprintf(f_topo, "%d %d %d \n", number_of_cells, number_of_faces, write_cell_nodes ? number_of_nodes : 0);

    if(topo_write_cells(rcfd, i_layer, f_topo) != 0)
        return -1;

    if(topo_write_faces(rcfd, i_layer, f_topo) != 0)
        return -1;

    // append node and cell type (for dump during run) of all cells
    if(write_cell_nodes){

        if(topo_write_nodes(rcfd, i_layer, f_topo) != 0)
            return -1;
    }

    fclose(f_topo);

#endif // RP_NODE

    return 0;
}

/**
 * @brief Get the area of the given face.
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the layer of the topology data
 * @param i_face    specifies the face to query
 * @return the face area
 */
double topo_face_area(rCFD_type* rcfd, const int i_layer, const int i_face)
{
#if RP_NODE
    int     i_dim;

    double  face_area;

    face_area = 0.0;

    for(i_dim = 0; i_dim < 3; i_dim++){

        face_area += _F(i_layer).area[i_face][i_dim] * _F(i_layer).area[i_face][i_dim];
    }

    face_area = sqrt(face_area);

    return face_area;
#endif
    return 0;
}

/**
 * @brief Get the neighbor cutoff distance for the given cell.
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the layer of the topology data
 * @param i_cell    specifies the cell to query
 * @return the cutoff distance
 */
double topo_get_neigh_cutoff_dist(rCFD_type *rcfd, const short i_layer, const int i_cell)
{
#if RP_NODE
    double max_dist = 0.0;
    double dist;
    int i_nb;

    for(i_nb = 0; i_nb < _C(i_layer).number_of_neigh_cells[i_cell]; i_nb++){

        dist = cell2cell_distance(rcfd, i_layer, i_cell, _C(i_layer).neigh_cell[i_cell][i_nb]);

        if(dist > max_dist) max_dist = dist;
    }

    return max_dist * _CELL_DICT(i_layer).next_neigh_cell_skin_factor;
#endif
    return 0;
}

/**
 * @brief Initialize the neighbor cell lists.
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the layer of the topology data
 */
void topo_init_neigh_cells(rCFD_type *rcfd, const short i_layer)
{
#if RP_NODE
#if 1   // local vars
    int i_face, i_cell, i_nb;
    int c0, c1;
#endif

    // loop faces and accumulate _C(i_layer).number_of_neigh_cells, allocate per-cell nb lists
    {
        _C(i_layer).number_of_neigh_cells = (int*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int));
        _C(i_layer).neigh_cell = (int**)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int*));

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            _C(i_layer).number_of_neigh_cells[i_cell] = 0;
            _C(i_layer).neigh_cell[i_cell] = NULL;
        }

        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

            c0 = _F(i_layer).c0[i_face];
            c1 = _F(i_layer).c1[i_face];

            _C(i_layer).number_of_neigh_cells[c0]++;
            _C(i_layer).number_of_neigh_cells[c1]++;
        }

        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            _C(i_layer).neigh_cell[i_cell] = (int*)malloc(_C(i_layer).number_of_neigh_cells[i_cell] * sizeof(int));

            for(i_nb = 0; i_nb < _C(i_layer).number_of_neigh_cells[i_cell]; i_nb++){

                _C(i_layer).neigh_cell[i_cell][i_nb] = -1;
            }
        }
    }

    // loop faces and fill per-cell neighbor lists
    {
        for(i_face = 0; i_face < _FACE_DICT(i_layer).number_of_faces; i_face++){

            c0 = _F(i_layer).c0[i_face];
            c1 = _F(i_layer).c1[i_face];

            // c1 is neighbor of c0
            {
                i_nb = 0;

                while((i_nb < _C(i_layer).number_of_neigh_cells[c0]) && (_C(i_layer).neigh_cell[c0][i_nb] > -1)) i_nb++;

                if(i_nb == _C(i_layer).number_of_neigh_cells[c0]){

                    report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] topo_init_neigh_cells: i_nb exceeds number_of_neigh_cells", myid);
                    return;
                }

                _C(i_layer).neigh_cell[c0][i_nb] = c1;
            }

            // c0 is neighbor of c1
            {
                i_nb = 0;

                while((i_nb < _C(i_layer).number_of_neigh_cells[c1]) && (_C(i_layer).neigh_cell[c1][i_nb] > -1)) i_nb++;

                if(i_nb == _C(i_layer).number_of_neigh_cells[c1]){

                    report_error(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                 "[%d] topo_init_neigh_cells: i_nb exceeds number_of_neigh_cells", myid);
                    return;
                }

                _C(i_layer).neigh_cell[c1][i_nb] = c0;
            }
        }
    }

    // loop neighbor cells and get next neighbor cells
    if(rcfd->Solver_Dict->mode == RCFD_MODE_PREPARATION){

        // allocate temporary neighbor lists (no need to init them)
        int     i_nnb, neigh_cell, i_warning = 0;
        int     max_neighbors   = rcfd->Prep_Dict->interpolation_max_number_of_aligned_neigh_cells * 2;
        int     *neighbor_ids   = (int*)malloc(max_neighbors * sizeof(int));
        double  *neighbor_dists = (double*)malloc(max_neighbors * sizeof(double));

        // allocate and init field vars
        _C(i_layer).number_of_next_neigh_cells = (int*)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int));
        _C(i_layer).next_neigh_cell = (int**)malloc(_CELL_DICT(i_layer).number_of_cells * sizeof(int*));

        // iterate all cells and store references to their direct and indirect neighbors ordered by distance
        for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

            int found_neighbors_count = _C(i_layer).number_of_neigh_cells[i_cell];

            // calc. cutoff for next neighbor cells of i_cell based on the max. distance to its neighbor cells
            double neigh_cutoff_dist = topo_get_neigh_cutoff_dist(rcfd, i_layer, i_cell);

            // fill the neighbor_ids with direct neighbors
            for(i_nb = 0; i_nb < _C(i_layer).number_of_neigh_cells[i_cell]; i_nb++){

                neighbor_ids[i_nb] = _C(i_layer).neigh_cell[i_cell][i_nb];
            }

            // use the list of neighbor_ids to further fill in indirect neighbors
            i_nb = 0;

            // stop when no new neighbors can be found or when the list is full
            while(i_nb < found_neighbors_count && found_neighbors_count < max_neighbors){

                // look at neighbors of neighbor_ids[i_nb]
                for(i_nnb = 0; i_nnb < _C(i_layer).number_of_neigh_cells[neighbor_ids[i_nb]]; i_nnb++){

                    neigh_cell = _C(i_layer).neigh_cell[neighbor_ids[i_nb]][i_nnb];

                    // if neighbor is valid and new, add it to the list of neighbor_ids,
                    // if neighbor_ids is full - report error and return
                    if( (neigh_cell != i_cell)
                        && !array_contains_value(neighbor_ids, found_neighbors_count, neigh_cell)           // new next neighbor cell
                        && (cell2cell_distance(rcfd, i_layer, i_cell, neigh_cell) < neigh_cutoff_dist)){    // within cutoff distance

                        if(found_neighbors_count >= max_neighbors){

                            i_warning++;

                            i_nb = found_neighbors_count;
                            break;
                        }
                        neighbor_ids[found_neighbors_count] = neigh_cell;
                        found_neighbors_count++;
                    }
                }
                i_nb++;
            }

            // alloc memory for the exact amount of found (in)direct neighbors and write the sorted neighbor_ids into the list
            {
                int *sorted_ids;

                _C(i_layer).number_of_next_neigh_cells[i_cell] = found_neighbors_count;

                _C(i_layer).next_neigh_cell[i_cell] = (int*)malloc(found_neighbors_count * sizeof(int));

                // sort next neighbor list by distance

                // store distances in a list and get their sorted IDs

                for(i_nnb = 0; i_nnb < found_neighbors_count; i_nnb++){

                    neighbor_dists[i_nnb] = cell2cell_distance(rcfd, i_layer, i_cell, neighbor_ids[i_nnb]);
                }
                sorted_ids = selectionArgsort(neighbor_dists, found_neighbors_count, 1);

                // use the sorted IDs to rearrange the list of neighbors
                for(i_nnb = 0; i_nnb < found_neighbors_count; i_nnb++){

                    _C(i_layer).next_neigh_cell[i_cell][i_nnb] = neighbor_ids[sorted_ids[i_nnb]];
                }

                free(sorted_ids);
            }
        }

        // report on incomplete neighbor lists
        {
            i_warning = PRF_GISUM1(i_warning);

            if(i_warning > 0){

                report_warning0(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                                "topo_init_neigh_cells: in %d cells number_of_neigh_cells "
                                "exceeds double of max_number_of_aligned_neigh_cells (%d)",
                                i_warning, max_neighbors);
            }
        }

        free(neighbor_dists);
        free(neighbor_ids);
    }
    else{

        _C(i_layer).number_of_next_neigh_cells = NULL;
        _C(i_layer).next_neigh_cell = NULL;
    }
#endif
}

/**
 * @brief Initialize the global and process-local bounding boxes
 * @param rcfd      pointer to rCFD instance
 * @param i_layer   specifies the layer of the topology data
 */
void topo_init_bounding_box(rCFD_type *rcfd, const short i_layer)
{
#if RP_NODE
#if 1   // local vars

    int     i_cell, i_node, i_dim;

    double  x_min_node[3], x_max_node[3], x_min_global[3], x_max_global[3];
#endif

    // loop cells and get min/max coord'sb_type
    {
        for(i_dim = 0; i_dim < 3; i_dim++){

            x_min_node[i_dim] =   1.0e10;
            x_max_node[i_dim] = - 1.0e10;
        }

        if(_NODE_DICT(i_layer).number_of_nodes > 0){

            for(i_node = 0; i_node < _NODE_DICT(i_layer).number_of_nodes; i_node++){

                for(i_dim = 0; i_dim < 3; i_dim++){

                    if(x_min_node[i_dim] > _N(i_layer).x[i_node][i_dim]) x_min_node[i_dim] = _N(i_layer).x[i_node][i_dim];
                    if(x_max_node[i_dim] < _N(i_layer).x[i_node][i_dim]) x_max_node[i_dim] = _N(i_layer).x[i_node][i_dim];
                }
            }
        }
        else{

            double min_volume = 1e10;

            for(i_cell = 0; i_cell < _CELL_DICT(i_layer).number_of_cells; i_cell++){

                for(i_dim = 0; i_dim < 3; i_dim++){

                    if(x_min_node[i_dim] > _C(i_layer).x[i_cell][i_dim]) x_min_node[i_dim] = _C(i_layer).x[i_cell][i_dim];
                    if(x_max_node[i_dim] < _C(i_layer).x[i_cell][i_dim]) x_max_node[i_dim] = _C(i_layer).x[i_cell][i_dim];
                }

                if(min_volume > _C(i_layer).volume[i_cell]){

                    min_volume = _C(i_layer).volume[i_cell];
                }
            }

            // make sure BB has some volume (e.g. in 2D cases)
            double min_length = pow((3.0/(4.0*M_PI)*min_volume), (1.0/3.0));

            for(i_dim = 0; i_dim < 3; i_dim++){

                if(x_max_node[i_dim] - x_min_node[i_dim] < min_length){

                    x_min_node[i_dim] -= min_length;
                    x_max_node[i_dim] += min_length;
                }
            }
        }

        for(i_dim = 0; i_dim < 3; i_dim++){

            x_min_global[i_dim] = PRF_GRLOW1(x_min_node[i_dim]);
            x_max_global[i_dim] = PRF_GRHIGH1(x_max_node[i_dim]);
        }
    }

    // set _C bounding box vars
    {
        for(i_dim = 0; i_dim < 3; i_dim++){

            _C(i_layer).x_min_node[i_dim] = x_min_node[i_dim];
            _C(i_layer).x_max_node[i_dim] = x_max_node[i_dim];
            _C(i_layer).x_min_global[i_dim] = x_min_global[i_dim];
            _C(i_layer).x_max_global[i_dim] = x_max_global[i_dim];
        }
    }

    report_trace(rcfd, RCFD_REPORT_CATEGORY_INITIALIZE,
                 "[%d] local bounding box min/max (%e %e %e)/(%e %e %e)",
                 myid,
                 x_min_node[0], x_min_node[1], x_min_node[2],
                 x_max_node[0], x_max_node[1], x_max_node[2]);
#endif
}

/*****************************************************************************/

#if defined(RCFD_LIB_TEST_MODULES)

// forward declarations of required functions
void init_topo_dict(rCFD_type *rcfd);
void init_topo(rCFD_type *rcfd);
void free_topo_dict(rCFD_type *rcfd);
void free_topo(rCFD_type *rcfd);
char* create_sub_dir_name(const char *working_dir, const char *base_dir, const char *sub_dir);
void recreate_filename(char *dir_name, char **filename);

#define MY_GRID_SPACING         0.01
#define MY_CELL_HEIGHT          0.05
#define MY_CELL_AREA            MY_GRID_SPACING*MY_CELL_HEIGHT
#define MY_CELL_VOLUME          MY_CELL_AREA*MY_GRID_SPACING
#define MY_INT_CELL_COUNT       16
#define MY_INT_FACE_COUNT       24
#define MY_INT_FACE_COUNT_HALF  12
#define MY_NODE_COUNT           50
#define MY_NODE_COUNT_HALF      25
#define MY_NODE_COUNT_HALF_SQRT 5

int rcfd_get_rank(void *handle){
#if RP_NODE
    if(handle){
        rCFD_type *rcfd = (rCFD_type*)handle;
        return myid;
    }
#endif
    return 0;
}

void test_topo_set_cell_dict(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){
        rcfd_set_dict_property_i(handle, "Cell_Dict", i_layer, -1, -1, "number_of_int_cells", MY_INT_CELL_COUNT);
        rcfd_set_dict_property_i(handle, "Cell_Dict", i_layer, -1, -1, "number_of_ext_cells", 0);
    }
#endif
}
void test_topo_set_face_dict(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){
        rcfd_set_dict_property_i(handle, "Face_Dict", i_layer, -1, -1, "number_of_int_faces", MY_INT_FACE_COUNT);
        rcfd_set_dict_property_i(handle, "Face_Dict", i_layer, -1, -1, "number_of_ext_faces", 0);
    }
#endif
}
void test_topo_set_node_dict(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){
        rcfd_set_dict_property_i(handle, "Node_Dict", i_layer, -1, -1, "number_of_nodes", MY_NODE_COUNT);
    }
#endif
}
void test_topo_init_cells(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){
        int     myrank = rcfd_get_rank(handle);
        int     i_cell;
        double  **cell_x    = rcfd_get_cell_center(handle, i_layer);
        double  *cell_volume= rcfd_get_cell_volume(handle, i_layer);

        for(i_cell = 0; i_cell < MY_INT_CELL_COUNT; ++i_cell){
            cell_x[i_cell][0]   = (i_cell%4)*MY_GRID_SPACING+4*MY_GRID_SPACING*myrank;
            cell_x[i_cell][1]   = (i_cell/4)*MY_GRID_SPACING;
            cell_x[i_cell][2]   = 0.0;
            cell_volume[i_cell] = MY_CELL_VOLUME;
        }
    }
#endif
}
void test_topo_init_faces(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){
        int i_face;
        int     *face_c0    = rcfd_get_face_c0  (handle, i_layer); // [i_face]
        int     *face_c1    = rcfd_get_face_c1  (handle, i_layer); // [i_face]
        double  **face_area = rcfd_get_face_area(handle, i_layer); // [i_face][i_dim]

        // internal faces with normal in x-direction
        for(i_face = 0; i_face < MY_INT_FACE_COUNT_HALF; ++i_face){
            face_c0[i_face] = i_face+i_face/3;
            face_c1[i_face] = i_face+i_face/3+1;
            face_area[i_face][0] = MY_CELL_AREA;
            face_area[i_face][1] = 0.0;
            face_area[i_face][2] = 0.0;
        }
        // internal faces with normal in y-direction
        for(; i_face < MY_INT_FACE_COUNT; ++i_face){
            face_c0[i_face] = i_face-MY_INT_FACE_COUNT_HALF;
            face_c1[i_face] = i_face-MY_INT_FACE_COUNT_HALF + 4;
            face_area[i_face][0] = 0.0;
            face_area[i_face][1] = MY_CELL_AREA;
            face_area[i_face][2] = 0.0;
        }
    }
#endif
}
void test_topo_init_cell_hosts(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){
        int     myrank = rcfd_get_rank(handle);
        int     i_cell;
        int     *cell_host = rcfd_get_cell_host(handle, i_layer);

        for(i_cell = 0; i_cell < MY_INT_CELL_COUNT; ++i_cell){
            cell_host[i_cell] = myrank;
        }
    }
#endif
}
void test_topo_init_nodes(void *handle, const short i_layer)
{
#if RP_NODE
    if(i_layer == 0){
        int     myrank = rcfd_get_rank(handle);
        int     i_cell, i_node, i_node_depth;
        int cell_node_ids[9] = {0};
        double  node_z = 0.0;
        double  **node_x  = rcfd_get_node(handle, i_layer);

        for(i_node_depth = 0; i_node_depth < 2; ++i_node_depth) {
            for(i_node = 0; i_node < MY_NODE_COUNT_HALF; ++i_node){
                node_x[i_node + i_node_depth*MY_NODE_COUNT_HALF][0] = -0.5*MY_GRID_SPACING + (i_node%MY_NODE_COUNT_HALF_SQRT)*MY_GRID_SPACING + 4*MY_GRID_SPACING*myrank;
                node_x[i_node + i_node_depth*MY_NODE_COUNT_HALF][1] = -0.5*MY_GRID_SPACING + (i_node/MY_NODE_COUNT_HALF_SQRT)*MY_GRID_SPACING;
                node_x[i_node + i_node_depth*MY_NODE_COUNT_HALF][2] = node_z;
            }
            node_z += MY_CELL_HEIGHT;
        }

        cell_node_ids[0] = 8;

        for(i_cell = 0; i_cell < MY_INT_CELL_COUNT; ++i_cell){
            cell_node_ids[1] = i_cell + (i_cell/4);
            cell_node_ids[2] = i_cell + (i_cell/4) + 1;
            cell_node_ids[3] = i_cell + (i_cell/4) + MY_NODE_COUNT_HALF_SQRT + 1;
            cell_node_ids[4] = i_cell + (i_cell/4) + MY_NODE_COUNT_HALF_SQRT;
            cell_node_ids[5] = i_cell + (i_cell/4) + MY_NODE_COUNT_HALF;
            cell_node_ids[6] = i_cell + (i_cell/4) + MY_NODE_COUNT_HALF + 1;
            cell_node_ids[7] = i_cell + (i_cell/4) + MY_NODE_COUNT_HALF + MY_NODE_COUNT_HALF_SQRT + 1;
            cell_node_ids[8] = i_cell + (i_cell/4) + MY_NODE_COUNT_HALF + MY_NODE_COUNT_HALF_SQRT;
            rcfd_set_cell_shape(handle, i_layer, i_cell, RCFD_CELL_HEXAHEDRON, cell_node_ids, NULL);
        }
    }
#endif
}

/**
 * @brief free current topology (if any), write a topo files consisting of a
 *           4x4 cells grid on each process and re-initalize the topology with
 *           this created file; free previous topology, initialize it via user
 *           functions and write to an other topology file; at the end restore
 *           original topology
 * @param rcfd  pointer to rCFD instance
 * @return 0 on success, else non-zero error code
 */
int test_topo(rCFD_type *rcfd)
{
    int success = 0;

#if RP_NODE
    char    filename[80];
    char    *orig_topo_filename = rcfd->File_Dict->Topo_filename;
    FILE    *f_topo = NULL;

    PRF_GSYNC();

    Message0("\nStarting rCFD test 'topo'\n");

    Message0("Test loading topology from file\n");

    rcfd->File_Dict->Topo_filename = (char*)malloc(64*sizeof(char));
    strcpy(rcfd->File_Dict->Topo_filename, rcfd->File_Dict->working_dir);
    strcat(rcfd->File_Dict->Topo_filename, "/");
    strcat(rcfd->File_Dict->Topo_filename, rcfd->File_Dict->data_dir);
    strcat(rcfd->File_Dict->Topo_filename, "/");
    strcat(rcfd->File_Dict->Topo_filename, rcfd->File_Dict->topo_sub_dir);
    strcat(rcfd->File_Dict->Topo_filename, "/topo_test_file");
    snprintf(filename, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);
    f_topo = fopen(filename, "wt");


    if(!f_topo){

        free(rcfd->File_Dict->Topo_filename);
        rcfd->File_Dict->Topo_filename = orig_topo_filename;
        success |= 1;
    }
    else{ // write topo files (only internal cells and internal faces)

        int i_cell, i_face, i_node, i_node_depth;
        double node_z = 0.0;

        fprintf(f_topo, "%d %d %d \n", MY_INT_CELL_COUNT, MY_INT_FACE_COUNT, MY_NODE_COUNT);

        // internal cells
        for(i_cell = 0; i_cell < MY_INT_CELL_COUNT; ++i_cell){

            fprintf(f_topo, "%e %e %e %e %d \n",
                            (i_cell%4)*MY_GRID_SPACING+4*MY_GRID_SPACING*myid,
                            (i_cell/4)*MY_GRID_SPACING,
                            0.0, MY_CELL_VOLUME, myid);
        }

        // internal faces with normal in x-direction
        for(i_face = 0; i_face < MY_INT_FACE_COUNT_HALF; ++i_face){

            fprintf(f_topo, "%d %d %e %e %e \n",
                    i_face+i_face/3,   // 0 1 2  4 5 6  8  9 10  12 13 14
                    i_face+i_face/3+1, // 1 2 3  5 6 7  9 10 11  13 14 15
                     MY_CELL_AREA, 0.0, 0.0);
        }
        // internal faces with normal in y-direction
        for(; i_face < MY_INT_FACE_COUNT; ++i_face){

            fprintf(f_topo, "%d %d %e %e %e \n",
                    i_face-MY_INT_FACE_COUNT_HALF,
                    i_face-MY_INT_FACE_COUNT_HALF + 4,
                    0.0, MY_CELL_AREA, 0.0);
        }

        // cell node positions
        for(i_node_depth = 0; i_node_depth < 2; ++i_node_depth) {

            for(i_node = 0; i_node < MY_NODE_COUNT_HALF; ++i_node){

                fprintf(f_topo, "%e %e %e \n",
                        -0.5*MY_GRID_SPACING + (i_node%MY_NODE_COUNT_HALF_SQRT)*MY_GRID_SPACING + 4*MY_GRID_SPACING*myid,
                        -0.5*MY_GRID_SPACING + (i_node/MY_NODE_COUNT_HALF_SQRT)*MY_GRID_SPACING,
                        node_z);
            }

            node_z += MY_CELL_HEIGHT;
        }

        // cell node indices
        for(i_cell = 0; i_cell < MY_INT_CELL_COUNT; ++i_cell){

            fprintf(f_topo, "%d %d %d %d %d %d %d %d %d \n",
                    RCFD_CELL_HEXAHEDRON,
                    i_cell + (i_cell/4),
                    i_cell + (i_cell/4) + 1,
                    i_cell + (i_cell/4) + MY_NODE_COUNT_HALF_SQRT + 1,
                    i_cell + (i_cell/4) + MY_NODE_COUNT_HALF_SQRT,
                    i_cell + (i_cell/4) + MY_NODE_COUNT_HALF,
                    i_cell + (i_cell/4) + MY_NODE_COUNT_HALF + 1,
                    i_cell + (i_cell/4) + MY_NODE_COUNT_HALF + MY_NODE_COUNT_HALF_SQRT + 1,
                    i_cell + (i_cell/4) + MY_NODE_COUNT_HALF + MY_NODE_COUNT_HALF_SQRT);
        }

        fclose(f_topo);
    }

    // free any existing topo
    free_topo(rcfd);
    free_topo_dict(rcfd);

    // initialize test topo
    init_topo_dict(rcfd);
    init_topo(rcfd);

    // free test topo
    free_topo(rcfd);
    free_topo_dict(rcfd);

    Message0("Test creating and writing topology via user functions\n");

    // test setting topo via functions
    rcfd->File_Dict->Topo_filename = (char*)malloc(64*sizeof(char));
    strcpy(rcfd->File_Dict->Topo_filename, rcfd->File_Dict->working_dir);
    strcat(rcfd->File_Dict->Topo_filename, "/");
    strcat(rcfd->File_Dict->Topo_filename, rcfd->File_Dict->data_dir);
    strcat(rcfd->File_Dict->Topo_filename, "/");
    strcat(rcfd->File_Dict->Topo_filename, rcfd->File_Dict->topo_sub_dir);
    strcat(rcfd->File_Dict->Topo_filename, "/topo_test_func");
    snprintf(filename, 80, "%s_%d", rcfd->File_Dict->Topo_filename, myid);

    // remove any old test file such that we do not read from it upon init
    remove(filename);
    PRF_GSYNC();

    // store old function pointers
    void (*orig_user_set_cell_dict)(void *, const short i_layer)    = rcfd->rCFD_UDF->_user_set_cell_dict;
    void (*orig_user_set_face_dict)(void *, const short i_layer)    = rcfd->rCFD_UDF->_user_set_face_dict;
    void (*orig_user_set_node_dict)(void *, const short i_layer)    = rcfd->rCFD_UDF->_user_set_node_dict;
    void (*orig_user_init_cells)(void *, const short i_layer)       = rcfd->rCFD_UDF->_user_init_cells;
    void (*orig_user_init_faces)(void *, const short i_layer)       = rcfd->rCFD_UDF->_user_init_faces;
    void (*orig_user_init_cell_hosts)(void *, const short i_layer)  = rcfd->rCFD_UDF->_user_init_cell_hosts;
    void (*orig_user_init_nodes)(void *, const short i_layer)       = rcfd->rCFD_UDF->_user_init_nodes;

    // register test functions
    int retval = 0;
    retval |= rcfd_register_user_function(rcfd, "user_set_cell_dict",   (void(*)(void))test_topo_set_cell_dict);
    retval |= rcfd_register_user_function(rcfd, "user_set_face_dict",   (void(*)(void))test_topo_set_face_dict);
    retval |= rcfd_register_user_function(rcfd, "user_set_node_dict",   (void(*)(void))test_topo_set_node_dict);
    retval |= rcfd_register_user_function(rcfd, "user_init_cells",      (void(*)(void))test_topo_init_cells);
    retval |= rcfd_register_user_function(rcfd, "user_init_faces",      (void(*)(void))test_topo_init_faces);
    retval |= rcfd_register_user_function(rcfd, "user_init_cell_hosts", (void(*)(void))test_topo_init_cell_hosts);
    retval |= rcfd_register_user_function(rcfd, "user_init_nodes",      (void(*)(void))test_topo_init_nodes);

    if(retval != 0)
        success |= 2;

    // initialize test topo
    init_topo_dict(rcfd);
    init_topo(rcfd);

    // write test topo to file
     retval = topo_write(rcfd);

     if(retval != 0)
         success |= 4;

    // free test topo
    free_topo(rcfd);
    free_topo_dict(rcfd);

    retval = 0;
    // reset udfs
    retval |= rcfd_register_user_function(rcfd, "user_set_cell_dict",   (void(*)(void))orig_user_set_cell_dict);
    retval |= rcfd_register_user_function(rcfd, "user_set_face_dict",   (void(*)(void))orig_user_set_face_dict);
    retval |= rcfd_register_user_function(rcfd, "user_set_node_dict",   (void(*)(void))orig_user_set_node_dict);
    retval |= rcfd_register_user_function(rcfd, "user_init_cells",      (void(*)(void))orig_user_init_cells);
    retval |= rcfd_register_user_function(rcfd, "user_init_faces",      (void(*)(void))orig_user_init_faces);
    retval |= rcfd_register_user_function(rcfd, "user_init_cell_hosts", (void(*)(void))orig_user_init_cell_hosts);
    retval |= rcfd_register_user_function(rcfd, "user_init_nodes",      (void(*)(void))orig_user_init_nodes);

    if(retval != 0)
        success |= 8;

    // reset to original topo
    free(rcfd->File_Dict->Topo_filename);
    rcfd->File_Dict->Topo_filename = orig_topo_filename;

    Message0("Reload original topology\n");

    init_topo_dict(rcfd);
    init_topo(rcfd);

#endif // RP_NODE

    return success;
}

#undef MY_GRID_SPACING
#undef MY_CELL_HEIGHT
#undef MY_CELL_AREA
#undef MY_CELL_VOLUME
#undef MY_INT_CELL_COUNT
#undef MY_INT_FACE_COUNT
#undef MY_INT_FACE_COUNT_HALF
#undef MY_NODE_COUNT
#undef MY_NODE_COUNT_HALF
#undef MY_NODE_COUNT_HALF_SQRT

#endif // RCFD_LIB_TEST_MODULES

#endif
