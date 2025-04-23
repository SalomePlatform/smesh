// Copyright (C) 2016-2025  CEA, EDF
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
//  File   : HexahedronCanonicalShapesTest.cxx
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
  double axisDirs[9] = {1.,0.,0.,0.,1.,0.,0.,0.,1.};
  std::vector<std::string> grdSpace = { std::to_string(gridSpacing) };
  std::vector<double> intPnts;

  std::unique_ptr<CartesianHypo> aHypo ( new CartesianHypo() );
  aHypo->SetGridSpacing(grdSpace, intPnts, 0 ); // Spacing in dir 0
  aHypo->SetGridSpacing(grdSpace, intPnts, 1 ); // Spacing in dir 1
  aHypo->SetGridSpacing(grdSpace, intPnts, 2 ); // Spacing in dir 2
  aHypo->SetSizeThreshold(theSizeThreshold);    // set threshold
  aHypo->SetAxisDirs(axisDirs);

  grid.GridInitAndInterserctWithShape(theShape, edge2faceIDsMap, aHypo.get(), theNumOfThreads, false);
}

/*!
  * \brief Test runner
  */
bool testShape (const TopoDS_Shape theShape,
                const bool toAddEdges,
                const bool toCreateFaces,
                const double theGridSpacing,
                const double theSizeThreshold,
                const int theNbCreatedExpected)
{
  std::unique_ptr<SMESH_Mesh> aMesh( new SMESH_Mesh_Test() );
  aMesh->ShapeToMesh( theShape );
  SMESH_MesherHelper helper( *aMesh );

  Grid grid;
  grid._helper = &helper;
  grid._toAddEdges = toAddEdges;
  grid._toCreateFaces = toCreateFaces;
  grid._toConsiderInternalFaces = false;
  grid._toUseThresholdForInternalFaces = false;
  grid._toUseQuanta = false;
  grid._sizeThreshold = theSizeThreshold;

  TEdge2faceIDsMap edge2faceIDsMap;
  GridInitAndIntersectWithShape( grid, theGridSpacing, theSizeThreshold,
                                 theShape, edge2faceIDsMap, 1 );

  SMESH_subMesh * aSubMesh = aMesh->GetSubMesh(theShape);
  aSubMesh->DependsOn(); // init sub-meshes

  Hexahedron hex( &grid );
  int nbAdded = hex.MakeElements( helper, edge2faceIDsMap, 1 );
  if (nbAdded != theNbCreatedExpected) {
    std::stringstream buffer;
    buffer << "Number of computed elements does not match: obtained " << nbAdded << " != expected " << theNbCreatedExpected;
    //CPPUNIT_ASSERT_MESSAGE(buffer.str().c_str(), nbAdded == theNbCreatedExpected );
    //MESSAGE(buffer.str().c_str());
    //CppUnitTestFramework::Logger::WriteMessage(buffer.str().c_str());
    std::cerr << buffer.str() << std::endl;
    return false;
  }

  return true;
}

/*!
  * \brief Test some primitive shapes
  */
bool testPrimitives()
{
  bool isOK = true;

  // Test fitting of a box
  BRepPrimAPI_MakeBox aMakeBox (10, 20, 30);
  aMakeBox.Build();
  CPPUNIT_ASSERT_MESSAGE( "Could not create the box!", aMakeBox.IsDone() );
  TopoDS_Shape aShape = aMakeBox.Shape();

  // Test exact fitting of a box
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/4, /*theNbCreatedExpected*/6))
    isOK = false;
  if (!testShape (aShape, /*toAddEdges*/true, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/4, /*theNbCreatedExpected*/6))
    isOK = false;

  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/true,
                  /*gridSpacing*/10, /*theSizeThreshold*/4, /*theNbCreatedExpected*/6))
    isOK = false;

  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/5, /*theSizeThreshold*/4, /*theNbCreatedExpected*/48))
    isOK = false;

  // Test not exact fitting of a box
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/7, /*theSizeThreshold*/4, /*theNbCreatedExpected*/12))
    isOK = false;

  // Test fitting of a cylinder
  gp_Ax2 anAxes (gp::Origin(), gp::DZ());
  BRepPrimAPI_MakeCylinder aMakeCyl (anAxes, 20., 30.);
  aMakeCyl.Build();
  CPPUNIT_ASSERT_MESSAGE( "Could not create the cylinder!", aMakeCyl.IsDone() );
  aShape = aMakeCyl.Shape();

  // test for different threshold values
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/4, /*theNbCreatedExpected*/48))
    isOK = false;
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/2, /*theNbCreatedExpected*/36))
    isOK = false;

  // Test fitting of a sphere
  BRepPrimAPI_MakeSphere aMakeSph (anAxes, 30.);
  aMakeSph.Build();
  CPPUNIT_ASSERT_MESSAGE( "Could not create the sphere!", aMakeSph.IsDone() );
  aShape = aMakeSph.Shape();

  // test for different threshold values
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/4, /*theNbCreatedExpected*/136))
    isOK = false;
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/2, /*theNbCreatedExpected*/88))
    isOK = false;

  // Test fitting of a cone
  BRepPrimAPI_MakeCone aMakeCon (anAxes, 30., 0., 40.);
  aMakeCon.Build();
  CPPUNIT_ASSERT_MESSAGE( "Could not create the cone!", aMakeCon.IsDone() );
  aShape = aMakeCon.Shape();

  // test for different threshold values
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/100, /*theNbCreatedExpected*/72))
    isOK = false;
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/4, /*theNbCreatedExpected*/40))
    isOK = false;
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/1.5, /*theNbCreatedExpected*/32))
    isOK = false;

  // truncated cone
  aMakeCon = BRepPrimAPI_MakeCone(anAxes, 30., 15., 20.);
  aMakeCon.Build();
  CPPUNIT_ASSERT_MESSAGE( "Could not create the cone!", aMakeCon.IsDone() );
  aShape = aMakeCon.Shape();

  // test for different threshold values
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/100, /*theNbCreatedExpected*/56))
    isOK = false;
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/4, /*theNbCreatedExpected*/36))
    isOK = false;
  if (!testShape (aShape, /*toAddEdges*/false, /*toCreateFaces*/false,
                  /*gridSpacing*/10, /*theSizeThreshold*/1.5, /*theNbCreatedExpected*/28))
    isOK = false;

  return isOK;
}

// Entry point for test
int main()
{
  bool isOK = testPrimitives();
  return isOK ? 0 : 1;
}
