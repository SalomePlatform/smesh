//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_topo.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
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

