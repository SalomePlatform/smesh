//  File      : SMESHGUI_Swig.i
//  Created   : Mon Jun 17 13:40:36 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$


%{
#include "SMESHGUI_Swig.hxx"
%}

%include "typemaps.i"

class SMESH_Swig
{
 public:
  SMESH_Swig();
  ~SMESH_Swig();

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

  void SetName(const char* Entry, const char* Name);
};
