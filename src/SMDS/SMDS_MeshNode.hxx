// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMDS_MeshNode.hxx
//  Module : SMESH
//
#ifndef _SMDS_MeshNode_HeaderFile
#define _SMDS_MeshNode_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_Position.hxx"

class SMDS_EXPORT SMDS_MeshNode: public SMDS_MeshElement
{
 public:

  void setXYZ(double x, double y, double z);
  double X() const; // ! NOT thread safe methods !
  double Y() const;
  double Z() const;
  void   GetXYZ(double xyz[3]) const; // thread safe getting coords

  SMDS_ElemIteratorPtr    GetInverseElementIterator(SMDSAbs_ElementType type=SMDSAbs_All) const;
  int                     NbInverseElements(SMDSAbs_ElementType type=SMDSAbs_All) const;

  SMDS_PositionPtr GetPosition() const; // WARNING result is std::unique_ptr !
  void SetPosition(const SMDS_PositionPtr& aPos, int shapeID = 0 );

  virtual SMDSAbs_ElementType  GetType() const       { return SMDSAbs_Node; }
  virtual VTKCellType          GetVtkType() const    { return VTK_VERTEX; }
  virtual SMDSAbs_EntityType   GetEntityType() const { return SMDSEntity_Node;}
  virtual SMDSAbs_GeometryType GetGeomType() const   { return SMDSGeom_NONE; }
  virtual int                  NbNodes() const       { return 1; }
  virtual int                  NbEdges() const       { return 0; }
  virtual int                  NbFaces() const       { return 0; }

  virtual SMDS_ElemIteratorPtr nodesIterator() const;
  virtual SMDS_NodeIteratorPtr nodeIterator() const;
  virtual const SMDS_MeshNode* GetNode(const int ind) const;

  virtual bool IsPoly() const { return false; }
  virtual bool IsQuadratic() const { return false; }
  virtual bool IsMediumNode(const SMDS_MeshNode* node) const  { return false; }
  virtual int  NbCornerNodes() const { return 1; }

  void Print(std::ostream & OS) const;

 private:

  void init(double x=0, double y=0, double z=0);

  double* getCoord() const;
  void AddInverseElement   (const SMDS_MeshElement * elem);
  void RemoveInverseElement(const SMDS_MeshElement * elem);
  void ClearInverseElements();

  friend class SMDS_Mesh;

};

#endif
