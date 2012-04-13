// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MeshEditPreview.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MeshEditPreview.h"

#include "SMESHGUI_VTKUtils.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>

// SALOME GUI includes
#include <VTKViewer_CellLocationsArray.h>
#include <SVTK_ViewWindow.h>

// VTK includes
#include <vtkPoints.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

// Qt includes
#include <QColor>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

SMESHGUI_MeshEditPreview::SMESHGUI_MeshEditPreview(SVTK_ViewWindow* theViewWindow):
  myViewWindow(theViewWindow)
{
  myGrid = vtkUnstructuredGrid::New();

  // Create and display actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInput( myGrid );

  myPreviewActor = SALOME_Actor::New();
  myPreviewActor->SetInfinitive(true);
  myPreviewActor->VisibilityOn();
  myPreviewActor->PickableOff();

  vtkFloatingPointType aFactor,aUnits;
  myPreviewActor->SetResolveCoincidentTopology(true);
  myPreviewActor->GetPolygonOffsetParameters(aFactor,aUnits);
  myPreviewActor->SetPolygonOffsetParameters(aFactor,0.2*aUnits);

  vtkFloatingPointType anRGB[3];
  SMESH::GetColor( "SMESH", "selection_element_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
  SetColor( anRGB[0], anRGB[1], anRGB[2] );

  myPreviewActor->SetMapper( aMapper );
  aMapper->Delete();

  myViewWindow->AddActor(myPreviewActor);

}

//================================================================================
/*!
 * \brief Destroy
 */
//================================================================================

SMESHGUI_MeshEditPreview::~SMESHGUI_MeshEditPreview()
{
  myGrid->Delete();

  myViewWindow->RemoveActor(myPreviewActor);
  myPreviewActor->Delete();

}

//================================================================================
/*!
 * \brief Returns vtk cell type
 */
//================================================================================

vtkIdType getCellType( const SMDSAbs_ElementType theType,
                       const bool thePoly,
                       const int theNbNodes )
{
  switch( theType ) 
  {
  case SMDSAbs_Node:              return VTK_VERTEX;
  case SMDSAbs_Edge: 
    if( theNbNodes == 2 )         return VTK_LINE;
    else if ( theNbNodes == 3 )   return VTK_QUADRATIC_EDGE;
    else return VTK_EMPTY_CELL;

  case SMDSAbs_Face  :
    if (thePoly && theNbNodes>2 ) return VTK_POLYGON;
    else if ( theNbNodes == 3 )   return VTK_TRIANGLE;
    else if ( theNbNodes == 4 )   return VTK_QUAD;
    else if ( theNbNodes == 6 )   return VTK_QUADRATIC_TRIANGLE;
    else if ( theNbNodes == 8 )   return VTK_QUADRATIC_QUAD;
    else if ( theNbNodes == 9 )   return VTK_BIQUADRATIC_QUAD;
    else return VTK_EMPTY_CELL;

  case SMDSAbs_Volume:
    if (thePoly && theNbNodes>3 ) return VTK_CONVEX_POINT_SET;
    else if ( theNbNodes == 4 )   return VTK_TETRA;
    else if ( theNbNodes == 5 )   return VTK_PYRAMID;
    else if ( theNbNodes == 6 )   return VTK_WEDGE;
    else if ( theNbNodes == 8 )   return VTK_HEXAHEDRON;
    else if ( theNbNodes == 10 )  return VTK_QUADRATIC_TETRA;
    else if ( theNbNodes == 20 )  return VTK_QUADRATIC_HEXAHEDRON;
    else if ( theNbNodes == 27 )  return VTK_TRIQUADRATIC_HEXAHEDRON;
    else if ( theNbNodes == 15  ) return VTK_QUADRATIC_WEDGE;
    else if ( theNbNodes == 13  ) return VTK_QUADRATIC_PYRAMID;//VTK_CONVEX_POINT_SET;
    else return VTK_EMPTY_CELL;

  default: return VTK_EMPTY_CELL;
  }
}

//================================================================================
/*!
 * \brief Set preview data
 */
//================================================================================

void SMESHGUI_MeshEditPreview::SetData (const SMESH::MeshPreviewStruct* previewData)
{
  // Create points
  const SMESH::nodes_array& aNodesXYZ = previewData->nodesXYZ;
  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints(aNodesXYZ.length());

  for ( int i = 0; i < aNodesXYZ.length(); i++ ) {
    aPoints->SetPoint( i, aNodesXYZ[i].x, aNodesXYZ[i].y, aNodesXYZ[i].z );
  }
  myGrid->SetPoints(aPoints);

  aPoints->Delete();

  // Create cells
  const SMESH::long_array&  anElemConnectivity = previewData->elementConnectivities;
  const SMESH::types_array& anElemTypes = previewData->elementTypes;

  vtkIdType aCellsSize = anElemConnectivity.length() + anElemTypes.length();
  vtkIdType aNbCells = anElemTypes.length();

  vtkCellArray* aConnectivity = vtkCellArray::New();
  aConnectivity->Allocate( aCellsSize, 0 );

  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents( 1 );
  aCellTypesArray->Allocate( aNbCells * aCellTypesArray->GetNumberOfComponents() );

  vtkIdList *anIdList = vtkIdList::New();
  int aNodePos = 0;

  for ( int i = 0; i < anElemTypes.length(); i++ ) {
    const SMESH::ElementSubType& anElementSubType = anElemTypes[i];
    SMDSAbs_ElementType aType = SMDSAbs_ElementType(anElementSubType.SMDS_ElementType);
    vtkIdType aNbNodes = anElementSubType.nbNodesInElement;
    anIdList->SetNumberOfIds( aNbNodes );

    for ( vtkIdType aNodeId = 0; aNodeId < aNbNodes; aNodeId++ ){
      anIdList->SetId( aNodeId, anElemConnectivity[aNodePos] );
      aNodePos++;
    }

    aConnectivity->InsertNextCell( anIdList );
    aCellTypesArray->InsertNextValue( getCellType( aType,
                                                   anElemTypes[i].isPoly, 
                                                   aNbNodes ) );
  }
  anIdList->Delete();

  // Insert cells in grid
  VTKViewer_CellLocationsArray* aCellLocationsArray = VTKViewer_CellLocationsArray::New();
  aCellLocationsArray->SetNumberOfComponents( 1 );
  aCellLocationsArray->SetNumberOfTuples( aNbCells );

  aConnectivity->InitTraversal();
  for( vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell( npts, pts ); idType++ )
    aCellLocationsArray->SetValue( idType, aConnectivity->GetTraversalLocation( npts ) );

  myGrid->SetCells( aCellTypesArray, aCellLocationsArray, aConnectivity );

  myPreviewActor->GetMapper()->Update();

  aCellTypesArray->Delete();
  aCellLocationsArray->Delete();
  aConnectivity->Delete();

  SetVisibility(true);
}

//================================================================================
/*!
 * \brief Set visibility
 */
//================================================================================

void SMESHGUI_MeshEditPreview::SetVisibility (bool theVisibility)
{
  myPreviewActor->SetVisibility(theVisibility);
  SMESH::RepaintCurrentView();
}

//================================================================================
/*!
 * \brief Set preview color
 */
//================================================================================

void SMESHGUI_MeshEditPreview::SetColor(double R, double G, double B)
{
  myPreviewActor->SetColor( R, G, B );
}

//================================================================================
/*!
 * \brief Get preview actor
 */
//================================================================================
SALOME_Actor* SMESHGUI_MeshEditPreview::GetActor() const
{ 
  return myPreviewActor;
}
