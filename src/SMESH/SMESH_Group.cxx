//  SMESH SMESH : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2004  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESH_Group.cxx
//  Author : Michael Sazonov (OCC)
//  Module : SMESH
//  $Header$

#include <SMESH_Group.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESHDS_Group.hxx>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Group::SMESH_Group (const SMESH_Mesh*         theMesh,
                          const SMDSAbs_ElementType theType,
                          const char*               theName)
     : myName(theName)
{
  myGroupDS = new SMESHDS_Group (const_cast<SMESH_Mesh*>(theMesh)->GetMeshDS(), theType);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Group::~SMESH_Group ()
{
  delete myGroupDS;
}
