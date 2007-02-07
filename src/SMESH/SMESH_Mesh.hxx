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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESH_Mesh.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_MESH_HXX_
#define _SMESH_MESH_HXX_

#include "SMESH_SMESH.hxx"

#include "SMESH_Hypothesis.hxx"
//#include "SMESH_subMesh.hxx"

#include "SMESHDS_Document.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_Command.hxx"
#include "SMDSAbs_ElementType.hxx"

//#include "NMTTools_IndexedDataMapOfShapeIndexedMapOfShape.hxx"
#include "SMESH_IndexedDataMapOfShapeIndexedMapOfShape.hxx"

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
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#include <vector>
#include <list>
#include <map>

#include <string>
#include <iostream>

class SMESH_Gen;
class SMESH_Group;
class TopTools_ListOfShape;
class SMESH_subMesh;
class SMESH_HypoFilter;

//typedef NMTTools_IndexedDataMapOfShapeIndexedMapOfShape IndexedMapOfChain;
typedef SMESH_IndexedDataMapOfShapeIndexedMapOfShape IndexedMapOfChain;

class SMESH_EXPORT SMESH_Mesh
{
  SMESH_Mesh();
  SMESH_Mesh(const SMESH_Mesh&);
public:
  SMESH_Mesh(int theLocalId, 
	     int theStudyId, 
	     SMESH_Gen* theGen,
	     bool theIsEmbeddedMode,
	     SMESHDS_Document* theDocument);
  
  virtual ~SMESH_Mesh();
  
  void ShapeToMesh(const TopoDS_Shape & aShape);
  
  int UNVToMesh(const char* theFileName);
  /*!
   * consult DriverMED_R_SMESHDS_Mesh::ReadStatus for returned value
   */
  int MEDToMesh(const char* theFileName, const char* theMeshName);
  
  int STLToMesh(const char* theFileName);

  SMESH_Hypothesis::Hypothesis_Status
  AddHypothesis(const TopoDS_Shape & aSubShape, int anHypId)
    throw(SALOME_Exception);
  
  SMESH_Hypothesis::Hypothesis_Status
  RemoveHypothesis(const TopoDS_Shape & aSubShape, int anHypId)
    throw(SALOME_Exception);
  
  const list <const SMESHDS_Hypothesis * >&
  GetHypothesisList(const TopoDS_Shape & aSubShape) const
    throw(SALOME_Exception);

  const SMESH_Hypothesis * GetHypothesis(const TopoDS_Shape &    aSubShape,
                                         const SMESH_HypoFilter& aFilter,
                                         const bool              andAncestors) const;
  
  int GetHypotheses(const TopoDS_Shape &                aSubShape,
                    const SMESH_HypoFilter&             aFilter,
                    list <const SMESHDS_Hypothesis * >& aHypList,
                    const bool                          andAncestors) const;

  const list<SMESHDS_Command*> & GetLog() throw(SALOME_Exception);
  
  void ClearLog() throw(SALOME_Exception);
  
  int GetId();
  
  SMESHDS_Mesh * GetMeshDS();
  
  SMESH_Gen *GetGen();
  
  SMESH_subMesh *GetSubMesh(const TopoDS_Shape & aSubShape)
    throw(SALOME_Exception);
  
  SMESH_subMesh *GetSubMeshContaining(const TopoDS_Shape & aSubShape)
    throw(SALOME_Exception);
  
  SMESH_subMesh *GetSubMeshContaining(const int aShapeID)
    throw(SALOME_Exception);
  
  void NotifySubMeshesHypothesisModification(const SMESH_Hypothesis* theChangedHyp);
  // Say all submeshes that theChangedHyp has been modified

  const list < SMESH_subMesh * >&
  GetSubMeshUsingHypothesis(SMESHDS_Hypothesis * anHyp)
    throw(SALOME_Exception);
  
  bool IsUsedHypothesis(SMESHDS_Hypothesis *  anHyp,
			const SMESH_subMesh * aSubMesh);
  // Return True if anHyp is used to mesh aSubShape
  
  bool IsNotConformAllowed() const;
  // check if a hypothesis alowing notconform mesh is present
  
  bool IsMainShape(const TopoDS_Shape& theShape) const;

  const TopTools_ListOfShape& GetAncestors(const TopoDS_Shape& theSubShape) const;
  // return list of ancestors of theSubShape in the order
  // that lower dimention shapes come first.

  /*! Check group names for duplications.
   *  Consider maximum group name length stored in MED file.
   */
  bool HasDuplicatedGroupNamesMED();

  void ExportMED(const char *file, 
		 const char* theMeshName = NULL, 
		 bool theAutoGroups = true, 
		 int theVersion = 0) 
    throw(SALOME_Exception);

  void ExportDAT(const char *file) throw(SALOME_Exception);
  void ExportUNV(const char *file) throw(SALOME_Exception);
  void ExportSTL(const char *file, const bool isascii) throw(SALOME_Exception);
  
  int NbNodes() throw(SALOME_Exception);
  
  /*!
   * ElementOrder points out entities of what order are requested
   */
  enum ElementOrder {
    ORDER_ANY,          /*! entities of any order */
    ORDER_LINEAR,       /*! entities of 1st order */
    ORDER_QUADRATIC     /*! entities of 2nd order */
  };

  int NbEdges(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);
  
  int NbFaces(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);
  
  int NbTriangles(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);
  
  int NbQuadrangles(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);

  int NbPolygons() throw(SALOME_Exception);
  
  int NbVolumes(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);
  
  int NbTetras(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);
  
  int NbHexas(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);
  
  int NbPyramids(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);

  int NbPrisms(ElementOrder order = ORDER_ANY) throw(SALOME_Exception);
  
  int NbPolyhedrons() throw(SALOME_Exception);
  
  int NbSubMesh() throw(SALOME_Exception);
  
  int NbGroup() const { return _mapGroup.size(); }
  
  SMESH_Group* AddGroup (const SMDSAbs_ElementType theType,
			 const char*               theName,
			 int&                      theId,
                         const TopoDS_Shape&       theShape=TopoDS_Shape());
  
  SMESH_Group* GetGroup (const int theGroupID);
  
  list<int> GetGroupIds();
  
  void RemoveGroup (const int theGroupID);

  // Propagation hypothesis management

  const SMESH_Hypothesis* IsLocal1DHypothesis (const TopoDS_Shape& theEdge);
  // Returns a local 1D hypothesis used for theEdge.

  bool IsPropagationHypothesis (const TopoDS_Shape& theEdge);
  // Returns true, if a local Propagation hypothesis is set directly on <theEdge>

  bool IsPropagatedHypothesis (const TopoDS_Shape& theEdge,
                               TopoDS_Shape&       theMainEdge);
  // Returns true, if a local 1D hypothesis is
  // propagated on <theEdge> from some other edge.
  // Returns through <theMainEdge> the edge, from
  // which the 1D hypothesis is propagated on <theEdge>

  bool IsReversedInChain (const TopoDS_Shape& theEdge,
                          const TopoDS_Shape& theMainEdge);
  // Returns true if theEdge should be reversed to be
  // co-directed with theMainEdge

  bool RebuildPropagationChains();
  bool RemovePropagationChain (const TopoDS_Shape& theMainEdge);
  bool BuildPropagationChain (const TopoDS_Shape& theMainEdge);
  
  SMDSAbs_ElementType GetElementType( const int id, const bool iselem );

  //
  
  ostream& Dump(ostream & save);
  
private:
  // Propagation hypothesis management
  void CleanMeshOnPropagationChain(const TopoDS_Shape& theMainEdge);
  //
  
private:
  int                        _id;           // id given by creator (unique within the creator instance)
  int                        _studyId;
  int                        _idDoc;        // id given by SMESHDS_Document
  int                        _groupId;      // id generator for group objects
  bool                       _isShapeToMesh;// set to true when a shape is given (only once)
  list <SMESH_subMesh *>     _subMeshesUsingHypothesisList;
  SMESHDS_Document *         _myDocument;
  SMESHDS_Mesh *             _myMeshDS;
  map <int, SMESH_subMesh *> _mapSubMesh;
  map <int, SMESH_Group *>   _mapGroup;
  SMESH_Gen *                _gen;
  
  TopTools_IndexedDataMapOfShapeListOfShape _mapAncestors;

  IndexedMapOfChain _mapPropagationChains; // Propagation hypothesis management
};

#endif
