//  File      : SMESHGUI_Swig.hxx
//  Created   : Mon Jun 17 13:41:12 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$


#ifndef _SMESHGUI_SWIG_HXX_
#define _SMESHGUI_SWIG_HXX_

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SALOMEDS)
#include CORBA_SERVER_HEADER(SALOMEDS_Attributes)

class SMESH_Swig
{
public:
  SMESH_Swig();
  ~SMESH_Swig();
    
  static void setOrb();

  void Init(int studyID);

  const char* AddNewMesh(const char* IOR);

  const char* AddNewHypothesis(const char* IOR);
  const char* AddNewAlgorithms(const char* IOR);

  void SetShape(const char* ShapeEntry, const char* MeshEntry);

  void SetHypothesis(const char* Mesh_Or_SubMesh_Entry, const char* Hypothesis_Entry);
  void SetAlgorithms(const char* Mesh_Or_SubMesh_Entry, const char* Algorithms_Entry);

  void UnSetHypothesis(const char* Applied_Hypothesis_Entry );

  const char* AddSubMesh (const char* Mesh_Entry, const char* SM_IOR, int ST);
  const char* AddSubMeshOnShape (const char* Mesh_Entry, const char* GeomShape_Entry, const char* SM_IOR, int ST);

  void CreateAndDisplayActor( const char* Mesh_Entry );

  void SetName(const char* Entry, const char* Name);

private:
  SALOMEDS::Study_var        myStudy;
  SALOMEDS::StudyBuilder_var myStudyBuilder;
  SALOMEDS::SComponent_var   mySComponentMesh;

  // Tags definition 
  long Tag_HypothesisRoot;
  long Tag_AlgorithmsRoot;
  
  long Tag_RefOnShape;
  long Tag_RefOnAppliedHypothesis;
  long Tag_RefOnAppliedAlgorithms;
  
  long Tag_SubMeshOnVertex;
  long Tag_SubMeshOnEdge;
  long Tag_SubMeshOnFace;
  long Tag_SubMeshOnSolid;
  long Tag_SubMeshOnCompound;
};


#endif
