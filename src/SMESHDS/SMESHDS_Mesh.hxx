//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESHDS_Mesh.hxx
//  Module : SMESH

#ifndef _SMESHDS_Mesh_HeaderFile
#define _SMESHDS_Mesh_HeaderFile

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMESHDS_Hypothesis.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESHDS_Script.hxx"

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
//Not portable see http://gcc.gnu.org/onlinedocs/libstdc++/faq/index.html#5_4 to know more.
#include <ext/hash_map>
using namespace __gnu_cxx;

using namespace std;

class SMESHDS_Mesh:public SMDS_Mesh
{

  public:

	SMESHDS_Mesh(int MeshID);
	void ShapeToMesh(const TopoDS_Shape & S);
	bool AddHypothesis(const TopoDS_Shape & SS, const SMESHDS_Hypothesis * H);
	bool RemoveHypothesis(const TopoDS_Shape & S, const SMESHDS_Hypothesis * H);
	SMDS_MeshNode * AddNode(double x, double y, double z);
	virtual void RemoveNode(int ID);
	void MoveNode(int ID, double x, double y, double z);
	SMDS_MeshEdge* AddEdge(int idnode1, int idnode2);
	SMDS_MeshFace* AddFace(int idnode1, int idnode2, int idnode3);
	SMDS_MeshFace* AddFace(int idnode1, int idnode2, int idnode3, int idnode4);
	SMDS_MeshVolume* AddVolume(int idnode1, int idnode2, int idnode3, int idnode4);
	SMDS_MeshVolume* AddVolume(int idnode1, int idnode2, int idnode3, int idnode4,
		int idnode5);
	SMDS_MeshVolume* AddVolume(int idnode1, int idnode2, int idnode3, int idnode4,
		int idnode5, int idnode6);
	SMDS_MeshVolume* AddVolume(int idnode1, int idnode2, int idnode3, int idnode4,
		int idnode5, int idnode6, int idnode7, int idnode8);
	virtual void RemoveElement(int IDelem);
	void SetNodeInVolume(SMDS_MeshNode * aNode, const TopoDS_Shell & S);
	void SetNodeOnFace(SMDS_MeshNode * aNode, const TopoDS_Face & S);
	void SetNodeOnEdge(SMDS_MeshNode * aNode, const TopoDS_Edge & S);
	void SetNodeOnVertex(SMDS_MeshNode * aNode, const TopoDS_Vertex & S);
	void UnSetNodeOnShape(const SMDS_MeshNode * aNode);
	void SetMeshElementOnShape(const SMDS_MeshElement * anElt,
		const TopoDS_Shape & S);
	void UnSetMeshElementOnShape(const SMDS_MeshElement * anElt,
		const TopoDS_Shape & S);
	TopoDS_Shape ShapeToMesh() const;
	bool HasMeshElements(const TopoDS_Shape & S);
	SMESHDS_SubMesh * MeshElements(const TopoDS_Shape & S);
	bool HasHypothesis(const TopoDS_Shape & S);
	const list<const SMESHDS_Hypothesis*>& GetHypothesis(const TopoDS_Shape & S) const;
	SMESHDS_Script * GetScript();
	void ClearScript();
	int ShapeToIndex(const TopoDS_Shape & aShape);
	TopoDS_Shape IndexToShape(int ShapeIndex);
	void NewSubMesh(int Index);
	void SetNodeInVolume(const SMDS_MeshNode * aNode, int Index);
	void SetNodeOnFace(SMDS_MeshNode * aNode, int Index);
	void SetNodeOnEdge(SMDS_MeshNode * aNode, int Index);
	void SetNodeOnVertex(SMDS_MeshNode * aNode, int Index);
	void SetMeshElementOnShape(const SMDS_MeshElement * anElt, int Index);
	 ~SMESHDS_Mesh();

  private:
	struct HashTopoDS_Shape
    {
		size_t operator()(const TopoDS_Shape& S) const {return S.HashCode(2147483647);}
    };
	typedef hash_map<TopoDS_Shape, list<const SMESHDS_Hypothesis*>,HashTopoDS_Shape > ShapeToHypothesis;
	int myMeshID;
	TopoDS_Shape myShape;
  	TopTools_IndexedMapOfShape myIndexToShape;
	map<int,SMESHDS_SubMesh*> myShapeIndexToSubMesh;
	ShapeToHypothesis myShapeToHypothesis;
	SMESHDS_Script * myScript;
};
#endif
