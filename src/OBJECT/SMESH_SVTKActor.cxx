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

//  SMESH OBJECT : interactive object for SMESH visualization
//  File   : SMESH_SVTKActor.cxx
//  Author : Roman NIKOLAEV
//  Module : SMESH
//

#include "SMESH_SVTKActor.h"

#include <SVTK_Utils.h>
#include <SALOME_Actor.h>


#include <SVTK_DeviceActor.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCell.h>
#include <vtkDataSetMapper.h>

vtkStandardNewMacro(SMESH_SVTKActor);

/*!
  Constructor
*/
SMESH_SVTKActor::SMESH_SVTKActor():
  my0DGrid(vtkUnstructuredGrid::New()),
  myBallGrid(vtkUnstructuredGrid::New())
{
  my0DActor = SVTK_DeviceActor::New();
  myBallActor = SVTK_DeviceActor::New();

  myBallActor->SetResolveCoincidentTopology(false);
  myBallActor->SetCoincident3DAllowed(true);
  myBallActor->PickableOff();
  my0DActor->SetResolveCoincidentTopology(false);
  my0DActor->SetCoincident3DAllowed(true);
  my0DActor->PickableOff();

  my0DGrid->Allocate();
  myBallGrid->Allocate();
}

/*!
  Constructor
*/
SMESH_SVTKActor::~SMESH_SVTKActor() {
  my0DActor->Delete();
  myBallActor->Delete();
  my0DGrid->Delete();
  myBallGrid->Delete();
}

/*!
  Publishes the actor in all its internal devices
*/
void SMESH_SVTKActor::AddToRender(vtkRenderer* theRenderer) {
  Superclass::AddToRender(theRenderer);
  float a0D = my0DActor->GetProperty()->GetPointSize();
  float aBall = myBallActor->GetProperty()->GetPointSize();
  my0DActor->GetProperty()->DeepCopy(GetProperty());
  myBallActor->GetProperty()->DeepCopy(GetProperty());
  my0DActor->GetProperty()->SetPointSize(a0D);
  myBallActor->GetProperty()->SetPointSize(aBall);
  theRenderer->AddActor(my0DActor);
  theRenderer->AddActor(myBallActor);
  
}

/*!
  Removes the actor from all its internal devices
*/
void 
SMESH_SVTKActor
::RemoveFromRender(vtkRenderer* theRenderer)
{
  Superclass::RemoveFromRender(theRenderer);
  theRenderer->RemoveActor( myBallActor );
  theRenderer->RemoveActor( my0DActor );
}

void
SMESH_SVTKActor
::MapCells(SALOME_Actor* theMapActor,
           const TColStd_IndexedMapOfInteger& theMapIndex)
{
  myUnstructuredGrid->Initialize();
  myUnstructuredGrid->Allocate();

  my0DGrid->Initialize();
  my0DGrid->Allocate();

  myBallGrid->Initialize();
  myBallGrid->Allocate();

  vtkDataSet *aSourceDataSet = theMapActor->GetInput();
  SVTK::CopyPoints( GetSource(), aSourceDataSet );
  SVTK::CopyPoints( myBallGrid, aSourceDataSet );
  SVTK::CopyPoints( my0DGrid,    aSourceDataSet );

  int aNbOfParts = theMapIndex.Extent();
  for(int ind = 1; ind <= aNbOfParts; ind++){
    int aPartId = theMapIndex( ind );
    if(vtkCell* aCell = theMapActor->GetElemCell(aPartId))
    {
#if VTK_XVERSION > 50700
      if (aCell->GetCellType() != VTK_POLYHEDRON)
#endif
      {
        if(aCell->GetCellType() == VTK_VERTEX ) {
          my0DGrid->InsertNextCell(aCell->GetCellType(),aCell->GetPointIds());
        } else if(aCell->GetCellType() == VTK_POLY_VERTEX ) {
          myBallGrid->InsertNextCell(aCell->GetCellType(),aCell->GetPointIds());
        } else {
          myUnstructuredGrid->InsertNextCell(aCell->GetCellType(),aCell->GetPointIds());
        }
      }
#if VTK_XVERSION > 50700
      else
      {
        vtkPolyhedron *polyhedron = dynamic_cast<vtkPolyhedron*>(aCell);
        if (!polyhedron)
          throw SALOME_Exception(LOCALIZED ("not a polyhedron"));
        vtkIdType *pts = polyhedron->GetFaces();
        myUnstructuredGrid->InsertNextCell(aCell->GetCellType(),pts[0], pts+1);
      }
#endif
    }
  }

  UnShrink();
  if(theMapActor->IsShrunk()){
    SetShrinkFactor(theMapActor->GetShrinkFactor());
    SetShrink();
  }

  myMapIndex = theMapIndex;
}

void
SMESH_SVTKActor
::Initialize()
{
  Superclass::Initialize();
  my0DActor->SetInput(my0DGrid);
  myBallActor->SetInput(myBallGrid);
}


void SMESH_SVTKActor::SetVisibility( int theVisibility ) {
  Superclass::SetVisibility( theVisibility );  
  my0DActor->SetVisibility( theVisibility );
  myBallActor->SetVisibility( theVisibility );
}


void SMESH_SVTKActor::Set0DSize(float theSize) {
  my0DActor->GetProperty()->SetPointSize(theSize);  
}

void SMESH_SVTKActor::SetBallSize(float theSize) {
  myBallActor->GetProperty()->SetPointSize(theSize);
}

