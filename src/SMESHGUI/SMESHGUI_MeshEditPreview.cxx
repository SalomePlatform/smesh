// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MeshEditPreview.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MeshEditPreview.h"

#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"

// SALOME GUI includes
#include <SVTK_Renderer.h>
#include <SVTK_ViewWindow.h>
#include <VTKViewer_CellLocationsArray.h>

// VTK includes
#include <vtkCellArray.h>
#include <vtkCoordinate.h>
#include <vtkDataSetMapper.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextMapper.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

// Qt includes
#include <QColor>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#include <gp_Ax3.hxx>

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
  aMapper->SetInputData( myGrid );

  myPreviewActor = SALOME_Actor::New();
  myPreviewActor->SetInfinitive(true);
  myPreviewActor->VisibilityOn();
  myPreviewActor->PickableOff();

  double aFactor,aUnits;
  myPreviewActor->SetResolveCoincidentTopology(true);
  myPreviewActor->GetPolygonOffsetParameters(aFactor,aUnits);
  myPreviewActor->SetPolygonOffsetParameters(aFactor,0.2*aUnits);

  double anRGB[3];
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

  for ( size_t iA = 0; iA < myLabelActors.size(); ++iA )
    if ( myLabelActors[iA] )
    {
      myPreviewActor->GetRenderer()->RemoveActor( myLabelActors[iA] );
      myLabelActors[iA]->Delete();
    }

  myViewWindow->RemoveActor(myPreviewActor);
  myPreviewActor->Delete();
}

//================================================================================
/*!
 * \brief Returns vtk cell type
 */
//================================================================================

vtkIdType getCellType( const SMDSAbs_ElementType theType,
                       const bool                thePoly,
                       const int                 theNbNodes )
{
  switch( theType ) 
  {
  case SMDSAbs_Ball:              return VTK_VERTEX;
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
    else if ( theNbNodes == 7 )   return VTK_BIQUADRATIC_TRIANGLE;
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

  for ( size_t i = 0; i < aNodesXYZ.length(); i++ ) {
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

  for ( size_t i = 0; i < anElemTypes.length(); i++ ) {
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
 * \brief Set shape of an arrow of a unit length and nb of arrows
 */
//================================================================================

void SMESHGUI_MeshEditPreview::SetArrowShapeAndNb( int         nbArrows,
                                                   double      headLength,
                                                   double      headRadius,
                                                   double      start,
                                                   const char* labels)
{
  const int theNbPoints = 10; // in one arrow
  myUnitArrowPnts.reserve( theNbPoints );
  myUnitArrowPnts.clear();

  // unit arrow || OZ

  for ( int i = 0; i < theNbPoints - 2; ++i )
  {
    double angle = i * 2 * M_PI / ( theNbPoints - 2 );
    myUnitArrowPnts.push_back( gp_Pnt( headRadius * Cos( angle ),
                                       headRadius * Sin( angle ),
                                       1. - headLength ));
  }
  myUnitArrowPnts.push_back( gp_Pnt( 0, 0, start ));
  myUnitArrowPnts.push_back( gp_Pnt( 0, 0, 1 ));


  // nodes of all arrows

  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints( theNbPoints * nbArrows );
  for ( int iP = 0, iA = 0; iA < nbArrows; ++iA )
    for ( int i = 0; i < theNbPoints; ++i, ++iP )
      aPoints->SetPoint( iP,
                         myUnitArrowPnts[i].X(),
                         myUnitArrowPnts[i].Y(),
                         myUnitArrowPnts[i].Z()  );
  myGrid->SetPoints(aPoints);
  aPoints->Delete();

  // connectivity of all arrows

  const int theNbCells = ( theNbPoints - 1 ); // in one arrow
  myGrid->Allocate( theNbCells  * nbArrows );
  for ( int nP = 0, iA = 0; iA < nbArrows; ++iA, nP += theNbPoints )
  {
    vtkIdType conn[3] = { theNbPoints - 1 + nP, // arrow end
                          theNbPoints - 3 + nP, // point on a circle
                          nP };                 // point on a circle
    for ( int i = 0; i < theNbCells-1; ++i )
    {
      myGrid->InsertNextCell( VTK_TRIANGLE, 3, conn );
      conn[1] = conn[2];
      conn[2] = conn[2] + 1;
    }
    conn[1] = theNbPoints - 2 + nP;
    myGrid->InsertNextCell( VTK_LINE, 2, conn );
  }

  myLabelActors.resize( nbArrows, ( vtkTextActor*) NULL );
  char label[] = "X";
  if ( labels )
    for ( int iA = 0; iA < nbArrows; ++iA )
    {
      label[0] = labels[iA];
      vtkTextMapper* text = vtkTextMapper::New();
      text->SetInput( label );
      vtkCoordinate* coord = vtkCoordinate::New();

      myLabelActors[iA] = vtkTextActor::New();
      //myLabelActors[iA]->SetMapper( text );
      myLabelActors[iA]->SetInput( label );
      myLabelActors[iA]->SetTextScaleModeToNone();
      myLabelActors[iA]->PickableOff();
      myLabelActors[iA]->GetPositionCoordinate()->SetReferenceCoordinate( coord );

      text->Delete();
      coord->Delete();

      myPreviewActor->GetRenderer()->AddActor(myLabelActors[iA]);
    }
}

//================================================================================
/*!
 * \brief Set data to show moved/rotated/scaled arrows
 *  \param [in] axes - location and direction of the arrows
 *  \param [in] length - length of arrows
 */
//================================================================================

void SMESHGUI_MeshEditPreview::SetArrows( const gp_Ax1* axes,
                                          double        length )
{
  vtkPoints* aPoints = myGrid->GetPoints();

  for ( int iP = 0, iA = 0; iA < (int) myLabelActors.size(); ++iA )
  {
    gp_Trsf trsf;
    trsf.SetTransformation( gp_Ax3( axes[iA].Location(), axes[iA].Direction() ), gp::XOY() );

    for ( size_t i = 0; i < myUnitArrowPnts.size(); ++i, ++iP )
    {
      gp_Pnt p = myUnitArrowPnts[i].Scaled( gp::Origin(), length );
      p.Transform( trsf );
      aPoints->SetPoint( iP, p.X(), p.Y(), p.Z() );
    }
    if ( myLabelActors[iA] )
      if ( vtkCoordinate* aCoord =
           myLabelActors[iA]->GetPositionCoordinate()->GetReferenceCoordinate() )
      {
        double p[3];
        aPoints->GetPoint( iP-1, p );
        aCoord->SetValue( p );
      }
  }

  myGrid->Modified();
}

//================================================================================
/*!
 * \brief Set visibility
 */
//================================================================================

void SMESHGUI_MeshEditPreview::SetVisibility (bool theVisibility)
{
  myPreviewActor->SetVisibility(theVisibility);
  for ( size_t iA = 0; iA < myLabelActors.size(); ++iA )
    if ( myLabelActors[iA] )
      myLabelActors[iA]->SetVisibility(theVisibility);
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

//================================================================================
/*!
 * \brief Returns the priewed vtkUnstructuredGrid
 */
//================================================================================

vtkUnstructuredGrid* SMESHGUI_MeshEditPreview::GetGrid() const
{
  return myGrid;
}

//================================================================================
/*!
 * \brief Returns myViewWindow
 */
//================================================================================

SVTK_ViewWindow* SMESHGUI_MeshEditPreview::GetViewWindow() const
{
  return myViewWindow;
}
