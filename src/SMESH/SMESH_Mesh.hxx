//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : SMESH_Mesh.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_MESH_HXX_
#define _SMESH_MESH_HXX_

#include "SMESHDS_Document.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_Command.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_subMesh.hxx"
#include "Utils_SALOME_Exception.hxx"

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <vector>
#include <list>
#include <map>

class SMESH_Gen;

class SMESH_Mesh
{
  public:
	SMESH_Mesh();
	SMESH_Mesh(int localId, int studyId, SMESH_Gen * gen,
		SMESHDS_Document * myDocument);

	virtual ~ SMESH_Mesh();

	void ShapeToMesh(const TopoDS_Shape & aShape) throw(SALOME_Exception);

	bool AddHypothesis(const TopoDS_Shape & aSubShape, int anHypId)
		throw(SALOME_Exception);

	bool RemoveHypothesis(const TopoDS_Shape & aSubShape, int anHypId)
		throw(SALOME_Exception);

	const list <const SMESHDS_Hypothesis * >&
		GetHypothesisList(const TopoDS_Shape & aSubShape)
		throw(SALOME_Exception);

	const list<SMESHDS_Command*> & GetLog() throw(SALOME_Exception);

//   const SMESHDS_ListOfAsciiString& GetLog()
//     throw (SALOME_Exception);

	void ClearLog() throw(SALOME_Exception);

	int GetId();

	SMESHDS_Mesh * GetMeshDS();

	SMESH_Gen *GetGen();

	SMESH_subMesh *GetSubMesh(const TopoDS_Shape & aSubShape)
		throw(SALOME_Exception);

	SMESH_subMesh *GetSubMeshContaining(const TopoDS_Shape & aSubShape)
		throw(SALOME_Exception);

	const list < SMESH_subMesh * >&
		GetSubMeshUsingHypothesis(SMESHDS_Hypothesis * anHyp)
		throw(SALOME_Exception);

	void Export(const char *fileName, const char *fileType)
		throw(SALOME_Exception);

	int NbNodes() throw(SALOME_Exception);

	int NbEdges() throw(SALOME_Exception);

	int NbFaces() throw(SALOME_Exception);

	int NbTriangles() throw(SALOME_Exception);

	int NbQuadrangles() throw(SALOME_Exception);

	int NbVolumes() throw(SALOME_Exception);

	int NbTetras() throw(SALOME_Exception);

	int NbHexas() throw(SALOME_Exception);

	int NbSubMesh() throw(SALOME_Exception);

  private:

	int _id;					// id given by creator (unique within the creator instance)
	int _studyId;
	int _idDoc;					// id given by SMESHDS_Document
	bool _isShapeToMesh;		// set to true when a shape is given (only once)
	list<const SMESHDS_Hypothesis *> _subShapeHypothesisList;
	list <SMESH_subMesh *> _subMeshesUsingHypothesisList;
	SMESHDS_Document * _myDocument;
	SMESHDS_Mesh * _myMeshDS;
	TopTools_IndexedMapOfShape _subShapes;
	map <int, SMESH_subMesh *>_mapSubMesh;
	SMESH_Gen *_gen;
};

#endif
