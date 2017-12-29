// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : libSMESH_Swig.i
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
%module libSMESH_Swig

%{
#include "libSMESH_Swig.h"
%}

/* Exception handler for all functions */
%exception 
{
  class PyAllowThreadsGuard
  {
  public:
    // Py_BEGIN_ALLOW_THREADS
    PyAllowThreadsGuard() { _save = PyEval_SaveThread(); }
    // Py_END_ALLOW_THREADS
    ~PyAllowThreadsGuard() { PyEval_RestoreThread(_save); }
  private:
    PyThreadState *_save;
  };

  PyAllowThreadsGuard guard;

  $action
}

%include "typemaps.i"
%include "std_vector.i"
%include "std_pair.i"

namespace std {
  
    %template(VectorInt) vector<int>;
    %template() std::pair<int,int>;
    %template(PairVector) std::vector<std::pair<int,int> >;
};


/* Selection mode enumeration (corresponds to constants from the SALOME_Selection.h) */
enum SelectionMode
  {
    Undefined = -1,
    Node = 0,
    Cell,
    EdgeOfCell,
    Edge,
    Face,
    Volume,
    Actor,
    Elem0D,
    Ball
  };

typedef struct
{
  double r, g, b;
  int delta;
} surfaceColorStruct;

typedef struct
{
  double r, g, b;
  int delta;
} volumeColorStruct;

typedef struct
{
  double r, g, b;
} edgeColorStruct;

typedef struct
{
  double r, g, b;
} nodeColorStruct;

struct actorAspect
{
  surfaceColorStruct surfaceColor;
  volumeColorStruct volumeColor;
  edgeColorStruct edgeColor;
  nodeColorStruct nodeColor;
  double opacity;
};

class SMESH_Swig
{
 public:
  SMESH_Swig();
  ~SMESH_Swig();

  void Init();

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

  void SetMeshIcon(const char* Mesh_Entry, const bool isComputed, const bool isEmpty);

  void CreateAndDisplayActor( const char* Mesh_Entry );
  void EraseActor( const char* Mesh_Entry, const bool allViewers = false );
  void UpdateActor( const char* Mesh_Entry );

  void setSelectionMode( SelectionMode selectionMode);
  std::vector<int> getSelected( const char* Mesh_Entry );
  std::vector<std::pair<int,int> > getSelectedEdgeOfCell( const char* Mesh_Entry );

  actorAspect GetActorAspect(const char* Mesh_Entry, int viewId = 0 );
  void SetActorAspect( const actorAspect& actorPres, const char* Mesh_Entry, int viewId = 0 );

  void setSelectionMode( SelectionMode selectionMode);
  std::vector<int> getSelected( const char* Mesh_Entry );

  // --------------------- for the test purposes -----------------------
  SelectionMode  getSelectionMode();
  void select( const char *id, std::vector<int> ids, bool append = false );
  void select( const char *id, int id1, bool append = false );
  void select( const char *id, std::vector<std::pair<int,int> >, bool apend = false );

};
