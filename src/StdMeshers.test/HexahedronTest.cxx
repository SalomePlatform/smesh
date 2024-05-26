// Copyright (C) 2016-2024  CEA, EDF
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : HexahedronTest.cxx
//  Module : SMESH
//  Purpose: Implement unit tests for StdMeshers_Cartesian_3D_Hexahedron class to reproduce bugs that manifest in integration tests.
//            The main difference between this unit test and integration tests is the fine grained control we have over the class methods and the hability to diagnose/solve bugs before the code goes into production enviroment. 
//            This test class can be used as reference for the development of future tests in other stdMesh algorithms

#include "StdMeshers_Cartesian_3D_Hexahedron.hxx"
#include "StdMeshers_CartesianParameters3D.hxx"

// CPP TEST
#include <cppunit/TestAssert.h>

// OCC 
#include<BRep_Builder.hxx>
#include<BRepTools.hxx>

#include<iostream>
#include <memory>

// Helper functions!
// Build Grid
//      Require building mesh
//      Require building shape. For test load shapes from memory in .brep files seems the simplest
//      

/*!
  * \brief Mock mesh
  */
struct SMESH_Mesh_Test: public SMESH_Mesh
{
  SMESH_Mesh_Test() {
    _isShapeToMesh = (_id = 0);
    _meshDS  = new SMESHDS_Mesh( _id, true );
  }
};

/*!
  * \brief Mock Hypothesis
  */
struct CartesianHypo: public StdMeshers_CartesianParameters3D
{
  CartesianHypo() : StdMeshers_CartesianParameters3D(0/*zero hypoId*/, nullptr/*NULL generator*/) 
  {
  }
}; 

/*!
  * \brief Shape loader
  */
void loadBrepShape( std::string shapeName, TopoDS_Shape & shape )
{
  BRep_Builder b;
  BRepTools::Read(shape, shapeName.c_str(), b);  
}

// Initialize the grid and intesersectors of grid with the geometry
void GridInitAndInterserctWithShape( Grid& grid, double gridSpacing, TopoDS_Shape& theShape, 
                                      std::map< TGeomID, vector< TGeomID > >& edge2faceIDsMap, const int numOfThreads )
{
  std::vector< TopoDS_Shape > faceVec;
  TopTools_MapOfShape faceMap;
  TopExp_Explorer fExp;
  for ( fExp.Init( theShape, TopAbs_FACE ); fExp.More(); fExp.Next() )
  {
    bool isNewFace = faceMap.Add( fExp.Current() );
    if ( !grid._toConsiderInternalFaces )
      if ( !isNewFace || fExp.Current().Orientation() == TopAbs_INTERNAL )
        // remove an internal face
        faceMap.Remove( fExp.Current() );
  }
  faceVec.reserve( faceMap.Extent() );
  faceVec.assign( faceMap.cbegin(), faceMap.cend() );
  
  vector<FaceGridIntersector> facesItersectors( faceVec.size() );

  Bnd_Box shapeBox;
  for ( size_t i = 0; i < faceVec.size(); ++i )
  {
    facesItersectors[i]._face   = TopoDS::Face( faceVec[i] );
    facesItersectors[i]._faceID = grid.ShapeID( faceVec[i] );
    facesItersectors[i]._grid   = &grid;
    shapeBox.Add( facesItersectors[i].GetFaceBndBox() );
  }
  // Canonical axes(i,j,k)
  double axisDirs[9] = {1.,0.,0.,0.,1.,0.,0.,0.,1.};

  Tools::GetExactBndBox( faceVec, axisDirs, shapeBox );
  vector<double> xCoords, yCoords, zCoords;  
  std::unique_ptr<CartesianHypo> myHypo( new CartesianHypo() );  
  std::vector<std::string> grdSpace = { std::to_string(gridSpacing) };
  std::vector<double> intPnts;
  myHypo->SetGridSpacing(grdSpace, intPnts, 0 ); // Spacing in dir 0
  myHypo->SetGridSpacing(grdSpace, intPnts, 1 ); // Spacing in dir 1
  myHypo->SetGridSpacing(grdSpace, intPnts, 2 ); // Spacing in dir 2
  myHypo->SetSizeThreshold(4.0);                 // set threshold
  myHypo->GetCoordinates(xCoords, yCoords, zCoords, shapeBox);
  grid.SetCoordinates( xCoords, yCoords, zCoords, axisDirs, shapeBox );

  for ( size_t i = 0; i < facesItersectors.size(); ++i )
    facesItersectors[i].Intersect();
  
  for ( size_t i = 0; i < facesItersectors.size(); ++i )
    facesItersectors[i].StoreIntersections();

  grid.ComputeNodes( *grid._helper );
  grid.GetEdgesToImplement( edge2faceIDsMap, theShape, faceVec );
}

// ADD test for parallel intersection of grid with solid

// Reproduce conditions of TBPERF_GRIDS_PERF_SMESH_M1 test to detect and solve segfault in unit test.
bool testNRTM1()
{
  for (auto numOfThreads : {1, 2, 12, 16} )
  {
    for (size_t i = 0; i < 10; i++)
    {
      TopoDS_Shape myShape;
      loadBrepShape( "data/HexahedronTest/NRTM1.brep", myShape );
      CPPUNIT_ASSERT_MESSAGE( "Could not load the brep shape!", !myShape.IsNull() );      
      std::unique_ptr<SMESH_Mesh> myMesh( new SMESH_Mesh_Test() );
      myMesh->ShapeToMesh( myShape );
      SMESH_MesherHelper helper( *myMesh );
      Grid grid;
      grid._helper = &helper;
      grid._toAddEdges = false; grid._toCreateFaces = false; grid._toConsiderInternalFaces = false; grid._toUseThresholdForInternalFaces = false; grid._toUseQuanta = false;
      grid._sizeThreshold = 4.0;
      grid.InitGeometry( myShape );
      
      std::map< TGeomID, vector< TGeomID > > edge2faceIDsMap;
      GridInitAndInterserctWithShape( grid, 1.0, myShape, edge2faceIDsMap, numOfThreads );
      Hexahedron hex( &grid );
      int nbAdded = hex.MakeElements( helper, edge2faceIDsMap, numOfThreads );
      CPPUNIT_ASSERT_MESSAGE( "Number of computed elements does not match", nbAdded == 1024 );
    }
  }  
  return true;
}

// Entry point for test
int main()
{
  auto t0 = testNRTM1();
  return 0;
}