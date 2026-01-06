// Copyright (C) 2016-2026  CEA, EDF
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
//            The main difference between this unit test and integration tests is the fine grained control we have over the class methods and the ability to diagnose/solve bugs before the code goes into production environment.
//            This test class can be used as reference for the development of future tests in other stdMesh algorithms

#include "StdMeshers_Cartesian_3D_Hexahedron.hxx"
#include "StdMeshers_CartesianParameters3D.hxx"

// CPP TEST
#include <cppunit/TestAssert.h>

// OCC
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>

#include <iostream>
#include <memory>
#include <numeric>

using namespace StdMeshers::Cartesian3D;

// Helper functions!
// Build Grid
//      Require building mesh
//      Require building shape.

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

/*!
  * \brief Initialize the grid and intesersectors of grid with the geometry
  */
void GridInitAndIntersectWithShape (Grid& grid,
                                    double gridSpacing,
                                    double theSizeThreshold,
                                    const TopoDS_Shape theShape,
                                    TEdge2faceIDsMap& edge2faceIDsMap,
                                    const int theNumOfThreads)
{
  // Canonical axes(i,j,k)
  double axisDirs[9] = {1.,0.,0., 0.,1.,0., 0.,0.,1.};
  std::vector<std::string> grdSpace = { std::to_string(gridSpacing) };
  std::vector<double> intPnts;

  std::unique_ptr<CartesianHypo> aHypo ( new CartesianHypo() );
  aHypo->SetAxisDirs(axisDirs);
  aHypo->SetGridSpacing(grdSpace, intPnts, 0 ); // Spacing in dir 0
  aHypo->SetGridSpacing(grdSpace, intPnts, 1 ); // Spacing in dir 1
  aHypo->SetGridSpacing(grdSpace, intPnts, 2 ); // Spacing in dir 2
  aHypo->SetSizeThreshold(theSizeThreshold);    // set threshold

  grid.GridInitAndInterserctWithShape(theShape, edge2faceIDsMap, aHypo.get(), theNumOfThreads, false);
}

/*!
  * \brief Reproduce conditions of TBPERF_GRIDS_PERF_SMESH_M1 test to detect and solve segfault in unit test.
  */
bool testNRTM1()
{
  TopoDS_Shape aShape;
  loadBrepShape( "data/HexahedronTest/NRTM1.brep", aShape );
  CPPUNIT_ASSERT_MESSAGE( "Could not load the brep shape!", !aShape.IsNull() );

  const auto numOfCores = std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency();
  std::vector<int> numberOfThreads(numOfCores);
  std::iota (std::begin(numberOfThreads), std::end(numberOfThreads), 1);

  for (auto nThreads : numberOfThreads )
  {
    for (size_t i = 0; i < 10 /*trials*/; i++)
    {
      std::unique_ptr<SMESH_Mesh> aMesh( new SMESH_Mesh_Test() );
      aMesh->ShapeToMesh( aShape );
      SMESH_MesherHelper helper( *aMesh );

      Grid grid;
      grid._helper = &helper;
      grid._toAddEdges = false;
      grid._toCreateFaces = false;
      grid._toConsiderInternalFaces = false;
      grid._toUseThresholdForInternalFaces = false;
      grid._toUseQuanta = false;
      grid._sizeThreshold = 4.0;

      TEdge2faceIDsMap edge2faceIDsMap;
      GridInitAndIntersectWithShape( grid, 1.0, 4.0, aShape, edge2faceIDsMap, nThreads );

      SMESH_subMesh * aSubMesh = aMesh->GetSubMesh(aShape);
      aSubMesh->DependsOn(); // init sub-meshes

      Hexahedron hex( &grid );
      int nbAdded = hex.MakeElements( helper, edge2faceIDsMap, nThreads );
      CPPUNIT_ASSERT_MESSAGE( "Number of computed elements does not match", nbAdded == 1024 );
    }
  }
  return true;
}

/*!
  * \brief Reproduce conditions of TBPERF_GRIDS_PERF_SMESH_J4 test to detect and solve segfault in unit test.
  */
bool testNRTJ4()
{
  TopoDS_Shape aShape;
  loadBrepShape( "data/HexahedronTest/NRTMJ4.brep", aShape );
  CPPUNIT_ASSERT_MESSAGE( "Could not load the brep shape!", !aShape.IsNull() );

  const auto numOfCores = std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency()/2;
  std::vector<int> numberOfThreads(numOfCores);
  std::iota (std::begin(numberOfThreads), std::end(numberOfThreads), 1);

  // Test with face creation
  for (auto nThreads : numberOfThreads )
  {
    for (size_t i = 0; i < 10 /*trials*/; i++)
    {
      std::unique_ptr<SMESH_Mesh> aMesh( new SMESH_Mesh_Test() );
      aMesh->ShapeToMesh( aShape );
      SMESH_MesherHelper helper( *aMesh );

      Grid grid;
      grid._helper = &helper;
      grid._toAddEdges = false;
      grid._toConsiderInternalFaces = false;
      grid._toUseThresholdForInternalFaces = false;
      grid._toUseQuanta = false;
      double testThreshold = 1.000001;
      grid._toCreateFaces = true;
      grid._sizeThreshold = testThreshold;

      TEdge2faceIDsMap edge2faceIDsMap;
      GridInitAndIntersectWithShape( grid, 2.0, testThreshold,
                                     aShape, edge2faceIDsMap, nThreads );

      SMESH_subMesh * aSubMesh = aMesh->GetSubMesh(aShape);
      aSubMesh->DependsOn(); // init sub-meshes

      Hexahedron hex( &grid );
      int nbAdded = hex.MakeElements( helper, edge2faceIDsMap, nThreads );
      CPPUNIT_ASSERT_MESSAGE( "Number of computed elements does not match", nbAdded == 35150 );
    }
  }
  return true;
}

// Entry point for test
int main()
{
  bool isOK = testNRTM1();
  if (!testNRTJ4())
    isOK = false;

  return isOK ? 0 : 1;
}
