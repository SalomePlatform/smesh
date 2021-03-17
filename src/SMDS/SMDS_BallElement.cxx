// Copyright (C) 2010-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMDS : implementation of Salome mesh data structure
//  Module     : SMESH
//  File       : SMDS_BallElement.cxx
//  Author     : Edward AGAPOV (eap)

#include "SMDS_BallElement.hxx"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"

void SMDS_BallElement::init(const SMDS_MeshNode * node, double diameter )
{
  vtkIdType nodeVtkID = node->GetVtkID();
  vtkIdType vtkID = getGrid()->InsertNextLinkedCell( toVtkType( SMDSEntity_Ball ), 1, &nodeVtkID );
  setVtkID( vtkID );
  getGrid()->SetBallDiameter( GetVtkID(), diameter );
}

double SMDS_BallElement::GetDiameter() const
{
  return getGrid()->GetBallDiameter( GetVtkID() );
}

void SMDS_BallElement::SetDiameter(double diameter)
{
  getGrid()->SetBallDiameter( GetVtkID(), diameter );
  GetMesh()->setMyModified();
}
