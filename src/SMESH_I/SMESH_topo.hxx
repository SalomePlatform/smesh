//=============================================================================
// File      : SMESH_topo.hxx
// Created   : lun mai 13 15:17:20 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

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
