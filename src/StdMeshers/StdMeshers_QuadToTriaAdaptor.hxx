//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESH : implementaion of SMESH idl descriptions
//  File   : StdMeshers_QuadToTriaAdaptor.hxx
//  Module : SMESH
//
#ifndef _SMESH_QuadToTriaAdaptor_HXX_
#define _SMESH_QuadToTriaAdaptor_HXX_

#include "SMESH_StdMeshers.hxx"
#include "SMDS_FaceOfNodes.hxx"

class SMESH_Mesh;
class SMDS_MeshElement;
class SMDS_MeshNode;
class Handle(TColgp_HArray1OfPnt);
class Handle(TColgp_HArray1OfVec);
class TopoDS_Shape;
class gp_Pnt;
class gp_Vec;


#include <map>
#include <list>
#include <vector>

class STDMESHERS_EXPORT StdMeshers_QuadToTriaAdaptor
{
public:

  ~StdMeshers_QuadToTriaAdaptor();

  bool Compute(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape);

  bool Compute(SMESH_Mesh& aMesh);

  const std::list<const SMDS_FaceOfNodes*>* GetTriangles(const SMDS_MeshElement* aFace);

protected:

  //bool CheckDegenerate(const SMDS_MeshElement* aFace);

  int Preparation(const SMDS_MeshElement* face,
                  Handle(TColgp_HArray1OfPnt)& PN,
                  Handle(TColgp_HArray1OfVec)& VN,
                  std::vector<const SMDS_MeshNode*>& FNodes,
                  gp_Pnt& PC, gp_Vec& VNorm,
                  const SMDS_MeshElement** volumes=0);

  bool CheckIntersection(const gp_Pnt& P, const gp_Pnt& PC,
                         gp_Pnt& Pint, SMESH_Mesh& aMesh,
                         const TopoDS_Shape& aShape,
                         const TopoDS_Shape& NotCheckedFace);

  bool Compute2ndPart(SMESH_Mesh& aMesh);

  typedef std::list<const SMDS_FaceOfNodes* >                        TTriaList;
  typedef std::multimap<const SMDS_MeshElement*, TTriaList >         TQuad2Trias;
  typedef std::map<const SMDS_MeshElement*, TTriaList *, TIDCompare> TPyram2Trias;

  TQuad2Trias  myResMap;
  TPyram2Trias myPyram2Trias;

  std::list< const SMDS_MeshNode* > myDegNodes;

};

#endif
