// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : StdMeshers_Cartesian_VL.hxx
// Created   : Tue May 24 12:32:01 2022
// Author    : Edward AGAPOV (eap)

#ifndef __StdMeshers_Cartesian_VL_HXX__
#define __StdMeshers_Cartesian_VL_HXX__

#include <BOPAlgo_Builder.hxx>
#include <BRepOffset_MakeOffset.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>

#include <set>
#include <map>
#include <vector>

class TopoDS_Face;
class StdMeshers_ViscousLayers;
class SMESH_Mesh;

namespace StdMeshers_Cartesian_VL
{
  class ViscousBuilder
  {
  public:

    ViscousBuilder( const StdMeshers_ViscousLayers* hypViscousLayers,
                    const SMESH_Mesh &              theMesh,
                    const TopoDS_Shape &            theShape);
    ~ViscousBuilder();

    TopoDS_Shape MakeOffsetShape(const TopoDS_Shape & theShape,
                                 SMESH_Mesh &         theMesh,
                                 std::string &        theError );                              

    SMESH_Mesh*  MakeOffsetMesh();

    bool         MakeViscousLayers( SMESH_Mesh &         offsetMesh,
                                    SMESH_Mesh &         theMesh,
                                    const TopoDS_Shape & theShape );                 
    
  private:
    
    TopoDS_Shape MakeOffsetSolid(const TopoDS_Shape & theShape,
                                 SMESH_Mesh &         theMesh,
                                 std::string &        theError );   

    bool         CheckGeometryMaps( SMESH_Mesh &         offsetMesh,
                                    const TopoDS_Shape & theShape );             

    void getOffsetSubShape( const TopoDS_Shape& S, std::vector<TopoDS_Shape>& listOfShapes );

    const StdMeshers_ViscousLayers* _hyp;
    BRepOffset_MakeOffset           _makeOffset;
    std::vector<BRepOffset_MakeOffset*> _makeOffsetCollection;  // collection to  
    BRepOffsetAPI_MakeOffset        _makeFaceOffset;            // to define shrink of planar faces. The face is shrink in all 
    BOPAlgo_Builder                 _solidCompound;             // to glue solids with common faces after shrinking then with BRepOffset_MakeOffset
    SMESH_Mesh*                     _offsetMesh;
    TopoDS_Shape                    _offsetShape;
    std::set< int >                 _shapesWVL;                 // shapes with viscous layers
    std::map< int, std::vector< int > > _edge2facesWOVL;        // EDGE 2 FACEs w/o VL
  };
}

#endif
