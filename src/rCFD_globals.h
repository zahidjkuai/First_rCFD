/******************************************************************************
 * @internal
 * @file        rCFD_globals.h
 * @copyright   (C) 2021
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Stefan Pirker
 *****************************************************************************/

#ifndef RCFD_GLOBALS
#define RCFD_GLOBALS

// forward declaration
typedef struct Fluent_Topo_struct Fluent_Topo_type;

static void *myrcfd = NULL;
#if RP_NODE
static Fluent_Topo_type *Fluent_Topo = NULL;
#endif
#endif
