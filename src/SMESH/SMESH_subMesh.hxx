//=============================================================================
// File      : SMESH_subMesh.hxx
// Created   : jeu mai 30 13:28:36 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_SUBMESH_HXX_
#define _SMESH_SUBMESH_HXX_

#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "Utils_SALOME_Exception.hxx"
#include <TopoDS_Shape.hxx>
#include <TColStd_IndexedMapOfTransient.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#include <set>
#include <list>
#include <map>

class SMESH_Mesh;
class SMESH_Hypothesis;

class SMESH_subMesh
{
public:
  SMESH_subMesh(int Id,
		SMESH_Mesh* father,
		const Handle(SMESHDS_Mesh)& meshDS,
		const TopoDS_Shape & aSubShape);
  virtual ~SMESH_subMesh();

  int GetId();

//   bool Contains(const TopoDS_Shape & aSubShape)
//     throw (SALOME_Exception);

  const Handle(SMESHDS_SubMesh)& GetSubMeshDS()
    throw (SALOME_Exception);

  SMESH_subMesh* GetFirstToCompute()
    throw (SALOME_Exception);

  const map<int, SMESH_subMesh*>& DependsOn();
  const map<int, SMESH_subMesh*>& Dependants();

  const TopoDS_Shape& GetSubShape();

  bool _vertexSet;   // only for vertex subMesh, set to false for dim > 0

  enum compute_state { NOT_READY, READY_TO_COMPUTE,
		       COMPUTE_OK, FAILED_TO_COMPUTE };
  enum algo_state { NO_ALGO, MISSING_HYP, HYP_OK };
  enum algo_event {ADD_HYP, ADD_ALGO,
		   REMOVE_HYP, REMOVE_ALGO,
                   ADD_FATHER_HYP, ADD_FATHER_ALGO,
		   REMOVE_FATHER_HYP, REMOVE_FATHER_ALGO};
  enum compute_event {MODIF_HYP, MODIF_ALGO_STATE, COMPUTE,
		      CLEAN, CLEANDEP, SUBMESH_COMPUTED};

  bool AlgoStateEngine(int event, SMESH_Hypothesis* anHyp)
    throw (SALOME_Exception);

  void SubMeshesAlgoStateEngine(int event, SMESH_Hypothesis* anHyp)
    throw (SALOME_Exception);

  void DumpAlgoState(bool isMain);

  bool ComputeStateEngine(int event)
    throw (SALOME_Exception);

  int GetComputeState() {return _computeState;};

protected:
  void InsertDependence(const TopoDS_Shape aSubShape);
//   void FinalizeDependence(list<TopoDS_Shape>& shapeList);

  bool SubMeshesComputed()
    throw (SALOME_Exception);

  bool SubMeshesReady();

  void RemoveSubMeshElementsAndNodes();
  void UpdateDependantsState();
  void CleanDependants();
  void ExtractDependants(const TopTools_IndexedDataMapOfShapeListOfShape& M,
			 const TopAbs_ShapeEnum etype);
  void SetAlgoState(int state);

  TopoDS_Shape _subShape;
  Handle (SMESHDS_Mesh) _meshDS;
  Handle (SMESHDS_SubMesh) _subMeshDS;
  int _Id;
  SMESH_Mesh* _father;
  map<int, SMESH_subMesh*> _mapDepend;
  map<int, SMESH_subMesh*> _mapDependants;
  bool _dependenceAnalysed;
  bool _dependantsFound;

  int _algoState;
  int _oldAlgoState;
  int _computeState;

};

#endif
