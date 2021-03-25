// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMDS : implementation of Salome mesh data structure
//  File   : SMDS_FacePosition.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH
//
#include "SMDS_FacePosition.hxx"
#include "SMDS_EdgePosition.hxx"

//=======================================================================
//function : SMDS_FacePosition
//purpose  :
//=======================================================================

SMDS_FacePosition::SMDS_FacePosition(const double aUParam,
                                     const double aVParam)
{
  SetParameters( aUParam,aVParam );
}

//=======================================================================
//function : GetTypeOfPosition
//purpose  :
//=======================================================================

SMDS_TypeOfPosition SMDS_FacePosition::GetTypeOfPosition() const
{
  return SMDS_TOP_FACE;
}

void SMDS_FacePosition::SetUParameter(double aUparam)
{
  myParameter[0] = aUparam;
}

//=======================================================================
//function : SetVParameter
//purpose  :
//=======================================================================

void SMDS_FacePosition::SetVParameter(double aVparam)
{
  myParameter[1] = aVparam;
}

//=======================================================================
//function : GetUParameter
//purpose  :
//=======================================================================

double SMDS_FacePosition::GetUParameter() const
{
  return myParameter[0];
}

//=======================================================================
//function : GetVParameter
//purpose  :
//=======================================================================

double SMDS_FacePosition::GetVParameter() const
{
  return myParameter[1];
}

//=======================================================================
//function : SetParameters
//purpose  : 
//=======================================================================

void SMDS_FacePosition::SetParameters(double aUparam, double aVparam)
{
  myParameter[0] = aUparam;
  myParameter[1] = aVparam;
}
