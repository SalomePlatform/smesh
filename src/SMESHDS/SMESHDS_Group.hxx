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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHDS_Group.hxx
//  Author : Michael Sazonov (OCC)
//  Module : SMESH
//  $Header$

#ifndef _SMESHDS_Group_HeaderFile
#define _SMESHDS_Group_HeaderFile

#include <SMDS_MeshGroup.hxx>
#include <string>

class SMESHDS_Mesh;

class SMESHDS_Group : public SMDS_MeshGroup
{
 public:

  SMESHDS_Group (const SMESHDS_Mesh*       theMesh,
                 const SMDSAbs_ElementType theType);

  void SetStoreName (const char* theName)
  { myStoreName = theName; }

  const char* GetStoreName () const
  { return myStoreName.c_str(); }

  bool Contains (const int theID) const;

  bool Add (const int theID);
  bool Remove (const int theID);

  int GetID (const int theIndex);
  // use it for iterations 1..Extent() as alternative to parent's
  // InitIterator(), More(), Next()

 private:
  SMESHDS_Group (const SMESHDS_Group& theOther);
  // prohibited copy constructor
  SMESHDS_Group& operator = (const SMESHDS_Group& theOther);
  // prohibited assign operator

  const SMDS_MeshElement* findInMesh (const int theID) const;

  string myStoreName;
  int    myCurIndex;
  int    myCurID;

};

#endif
