using namespace std;
//=============================================================================
// File      : SMESH_topo.cxx
// Created   : lun mai 13 15:17:25 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_topo.hxx"
#include "utilities.h"

SMESH_topo::SMESH_topo()
{
  MESSAGE("SMESH_topo()");
}

SMESH_topo::~SMESH_topo()
{
  MESSAGE("~SMESH_topo()");
}

//=============================================================================
/*!
 *  Static method, gives a unique local id for a given CORBA reference of
 *  a shape, within the SALOME session (multi studies). 
 *  Internal geom id or stringified CORBA reference could be used here.
 *  GEOM Id is more efficient (shorter string).
 */
//=============================================================================

string SMESH_topo::GetShapeLocalId(GEOM::GEOM_Shape_ptr aShape)
{
  string str = aShape->ShapeId(); //geomId
  MESSAGE(str);
  return str;
}

