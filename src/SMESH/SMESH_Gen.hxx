//=============================================================================
// File      : SMESH_Gen.hxx
// Created   : jeu mai 16 22:53:13 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_GEN_HXX_
#define _SMESH_GEN_HXX_

#include "Utils_SALOME_Exception.hxx"

#include "SMESH_HypothesisFactory.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Algo.hxx"
#include "SMESH_1D_Algo.hxx"
#include "SMESH_2D_Algo.hxx"
#include "SMESH_3D_Algo.hxx"
#include "SMESH_Mesh.hxx"

#include "SMESHDS_Document.hxx"

#include <TopoDS_Shape.hxx>

#include <map>

typedef struct studyContextStruct
{
  map<int, SMESH_Hypothesis*> mapHypothesis;
  map<int, SMESH_Mesh*> mapMesh;
  Handle (SMESHDS_Document) myDocument;
} StudyContextStruct ;

class SMESH_Gen
{
public:
  SMESH_Gen();
  ~SMESH_Gen();

  SMESH_Hypothesis* CreateHypothesis(const char* anHyp, int studyId)
    throw (SALOME_Exception);
  SMESH_Mesh* Init(int studyId, const TopoDS_Shape& aShape)
    throw (SALOME_Exception);
  bool Compute(::SMESH_Mesh& aMesh, const TopoDS_Shape& aShape)
    throw (SALOME_Exception);
  StudyContextStruct* GetStudyContext(int studyId);

  static int GetShapeDim(const TopoDS_Shape& aShape);
  SMESH_Algo* GetAlgo(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape);

  // inherited methods from SALOMEDS::Driver

  void Save(int studyId, const char *aUrlOfFile);
  void Load(int studyId, const char *aUrlOfFile); 
  void Close(int studyId); 
  const char* ComponentDataType();

  const char* IORToLocalPersistentID(const char* IORString, bool& IsAFile);
  const char* LocalPersistentIDToIOR(const char* aLocalPersistentID);

  SMESH_HypothesisFactory _hypothesisFactory;

  map<int, SMESH_Algo*> _mapAlgo;
  map<int, SMESH_1D_Algo*> _map1D_Algo;
  map<int, SMESH_2D_Algo*> _map2D_Algo;
  map<int, SMESH_3D_Algo*> _map3D_Algo;

private:
  int _localId; // unique Id of created objects, within SMESH_Gen entity
  map<int, StudyContextStruct*> _mapStudyContext;
  map<int, SMESH_Hypothesis*> _mapHypothesis;
};

#endif
