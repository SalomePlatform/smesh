//=============================================================================
// File      : SMESH_Mesh.hxx
// Created   : sam mai 18 08:07:35 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_MESH_HXX_
#define _SMESH_MESH_HXX_

#include "SMESHDS_Document.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESHDS_ListOfCommand.hxx"
//#include "SMESHDS_ListOfAsciiString.hxx"
//#include "SMESHDS_ListIteratorOfListOfAsciiString.hxx"


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
  SMESH_Mesh(int localId,
	     int studyId,
	     SMESH_Gen* gen,
	     const Handle(SMESHDS_Document)& myDocument);

  virtual ~SMESH_Mesh();

  void ShapeToMesh(const TopoDS_Shape& aShape)
    throw (SALOME_Exception);

  bool AddHypothesis(const TopoDS_Shape& aSubShape,
		     int anHypId)
    throw (SALOME_Exception);

  bool RemoveHypothesis(const TopoDS_Shape& aSubShape,
			int anHypId)
    throw (SALOME_Exception);

  const list<SMESHDS_Hypothesis*>&
  GetHypothesisList(const TopoDS_Shape& aSubShape)
    throw (SALOME_Exception);

  const SMESHDS_ListOfCommand& GetLog()
    throw (SALOME_Exception);

//   const SMESHDS_ListOfAsciiString& GetLog()
//     throw (SALOME_Exception);

  void ClearLog()
    throw (SALOME_Exception);

  int GetId();
 
  const Handle(SMESHDS_Mesh)& GetMeshDS();

  SMESH_Gen* GetGen();

  SMESH_subMesh* GetSubMesh(const TopoDS_Shape & aSubShape)
    throw (SALOME_Exception);

  SMESH_subMesh* GetSubMeshContaining(const TopoDS_Shape & aSubShape)
    throw (SALOME_Exception);

  const list <SMESH_subMesh*>&
  GetSubMeshUsingHypothesis(SMESHDS_Hypothesis* anHyp)
    throw (SALOME_Exception);

  void ExportDAT( const char* file )
    throw (SALOME_Exception);
  void ExportMED( const char* file )
    throw (SALOME_Exception);
  void ExportUNV( const char* file )
    throw (SALOME_Exception);
    
  int NbNodes()
    throw (SALOME_Exception);
  
  int NbEdges()
    throw (SALOME_Exception);
  
  int NbFaces()
    throw (SALOME_Exception);

  int NbTriangles()
    throw (SALOME_Exception);

  int NbQuadrangles()
    throw (SALOME_Exception);
  
  int NbVolumes()
    throw (SALOME_Exception);

  int NbTetras()
    throw (SALOME_Exception);

  int NbHexas()
    throw (SALOME_Exception);
  
  int NbSubMesh()
    throw (SALOME_Exception);
  

private:

  int _id;          // id given by creator (unique within the creator instance)
  int _studyId;
  int _idDoc;       // id given by SMESHDS_Document
  bool _isShapeToMesh; // set to true when a shape is given (only once)
  list<SMESHDS_Hypothesis*> _subShapeHypothesisList;
  list<SMESH_subMesh*> _subMeshesUsingHypothesisList;
  Handle (SMESHDS_Document) _myDocument;
  Handle (SMESHDS_Mesh) _myMeshDS;
  TopTools_IndexedMapOfShape _subShapes;
  map<int, SMESH_subMesh*> _mapSubMesh;
  SMESH_Gen* _gen;
};

#endif
