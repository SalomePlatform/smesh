//  SMESH SMESHDS : management of mesh data and SMESH document
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHDS_Group.hxx
//  Module : SMESH
//  $Header$

#ifndef _SMESHDS_Group_HeaderFile
#define _SMESHDS_Group_HeaderFile

#include "SMESH_SMESHDS.hxx"

#include <string>
#include "SMESHDS_GroupBase.hxx"
#include "SMDS_MeshGroup.hxx"

class SMESHDS_Mesh;

class SMESHDS_EXPORT SMESHDS_Group : public SMESHDS_GroupBase
{
 public:

  SMESHDS_Group (const int                 theID,
                 const SMESHDS_Mesh*       theMesh,
                 const SMDSAbs_ElementType theType);

  virtual void SetType(SMDSAbs_ElementType theType);

  virtual int Extent();

  virtual bool IsEmpty();

  virtual bool Contains (const int theID);

  virtual SMDS_ElemIteratorPtr GetElements();

  bool Add (const int theID);

  bool Remove (const int theID);

  void Clear();

  SMDS_MeshGroup& SMDSGroup() { return myGroup; }

 private:

  SMDS_MeshGroup myGroup;

};

#endif
