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
//  File   : SMESH_topo.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_TOPO_HXX_
#define _SMESH_TOPO_HXX_

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(GEOM_Shape)

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <string>
#include <map>

static const char* const SMESH_shapeTypeNames[9] = { "0=TopAbs_COMPOUND",
						     "1=TopAbs_COMPSOLID",
						     "2=TopAbs_SOLID",
						     "3=TopAbs_SHELL",
						     "4=TopAbs_FACE",
						     "5=TopAbs_WIRE",
						     "6=TopAbs_EDGE",
						     "7=TopAbs_VERTEX",
						     "8=TopAbs_SHAPE" };

class SMESH_topo
{
public:

  static string GetShapeLocalId(GEOM::GEOM_Shape_ptr aShape);

  SMESH_topo();
  virtual ~SMESH_topo();

  map <string, int> _mapIndShapes[9];
  TopTools_IndexedMapOfShape _myShapes[9];

protected:


};

#endif
