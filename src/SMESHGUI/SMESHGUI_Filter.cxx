//  SMESHGUI_Filter : Filters for VTK viewer
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
//  File   : SMESHGUI_Filter.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_Filter.h"

#include <vtkCell.h>

#include <gp_Vec.hxx>
#include <Precision.hxx>
#include "SMESH_Actor.h"
#include "SMESHGUI.h"

IMPLEMENT_STANDARD_HANDLE(SMESHGUI_Filter, VTKViewer_Filter)
IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_Filter, VTKViewer_Filter)

/*
  Class       : SMESHGUI_Filter
  Description : Selection filter for VTK viewer. This class aggregate object
                of SMESH_Filter class and uses it for verification of criterion
*/

//=======================================================================
// name    : SMESHGUI_Filter::SMESHGUI_Filter
// Purpose : Constructor
//=======================================================================
SMESHGUI_Filter::SMESHGUI_Filter()
{
  myActor = 0;
}

SMESHGUI_Filter::~SMESHGUI_Filter()
{
}

//=======================================================================
// name    : SMESHGUI_Filter::IsValid
// Purpose : Verify whether entry id satisfies to criterion of the filter
//=======================================================================
bool SMESHGUI_Filter::IsValid( const int theCellId ) const
{
  if ( myActor == 0 || myPred->_is_nil() )
    return false;

  SMESH_Actor* anActor = ( SMESH_Actor* )myActor;

  int aMeshId = anActor->GetElemObjId( theCellId );

  return myPred->IsSatisfy( aMeshId );
}

//=======================================================================
// name    : SMESHGUI_Filter::SetPredicate
// Purpose : Set new pridicate to the filter
//=======================================================================
void SMESHGUI_Filter::SetPredicate( SMESH::Predicate_ptr thePred )
{
  myPred = thePred;
}

//=======================================================================
// name    : SMESHGUI_Filter::GetActor
// Purpose : Get actor of the filter
//=======================================================================
SALOME_Actor* SMESHGUI_Filter::GetActor() const
{
  return myActor;
}

//=======================================================================
// name    : SMESHGUI_Filter::SetActor
// Purpose : Set new actor
//=======================================================================
void SMESHGUI_Filter::SetActor( SALOME_Actor* theActor )
{
  if ( myActor == theActor )
    return;
  VTKViewer_Filter::SetActor( theActor );

  if ( myActor != 0 && !myPred->_is_nil() )
  {
    Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
    if ( !anIO.IsNull() )
    {
      Standard_Boolean aRes = false;
      SMESH::SMESH_Mesh_ptr aMeshPtr = SMESHGUI::GetSMESHGUI()->ConvertIOinMesh( anIO, aRes );
      if ( aRes )
        myPred->SetMesh( aMeshPtr );
    }
  }
}
