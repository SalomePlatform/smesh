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
//  Module : SMESH
//  $Header$

#ifndef _SMESHDS_GroupBase_HeaderFile
#define _SMESHDS_GroupBase_HeaderFile

#include <string>
#include "SMDSAbs_ElementType.hxx"
#include "SMDS_MeshElement.hxx"
  
class SMESHDS_Mesh;

class SMESHDS_GroupBase
{
 public:

  SMESHDS_GroupBase (const int                 theID,
                     const SMESHDS_Mesh*       theMesh,
                     const SMDSAbs_ElementType theType);

  int GetID() const { return myID; }

  const SMESHDS_Mesh* GetMesh() const { return myMesh; }

  virtual void SetType(SMDSAbs_ElementType theType);

  SMDSAbs_ElementType GetType() const { return myType; }

  void SetStoreName (const char* theName) { myStoreName = theName; }

  const char* GetStoreName () const { return myStoreName.c_str(); }

  virtual int Extent();

  virtual bool IsEmpty();

  virtual bool Contains (const int theID);

  virtual SMDS_ElemIteratorPtr GetElements() = 0;

  int GetID (const int theIndex);
  // use it for iterations 1..Extent()

  virtual ~SMESHDS_GroupBase() {}

  void SetColorGroup (int theColorGroup)
  { myColorGroup = theColorGroup;}
  
  int GetColorGroup() const
  { return myColorGroup;}
  
 protected:
  const SMDS_MeshElement* findInMesh (const int theID) const;
  void resetIterator();

 private:
  SMESHDS_GroupBase (const SMESHDS_GroupBase& theOther);
  // prohibited copy constructor
  SMESHDS_GroupBase& operator = (const SMESHDS_GroupBase& theOther);
  // prohibited assign operator

  int                  myID;
  const SMESHDS_Mesh*  myMesh;
  SMDSAbs_ElementType  myType;
  std::string          myStoreName;
  // for GetID()
  int                  myCurIndex;
  int                  myCurID;
  SMDS_ElemIteratorPtr myIterator;
  int                  myColorGroup;
};

#endif
