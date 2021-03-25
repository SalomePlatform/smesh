// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
    PyThreadState* _save;
  };

  PyAllowThreadsGuard guard;

  $action
}

%include "typemaps.i"
%include "std_vector.i"
%include "std_pair.i"

namespace std
{
  %template(VectorInt) vector<int>;
  %template() std::pair<int,int>;
  %template(PairVector) std::vector<std::pair<int,int> >;
};

// See SMESH_Actor.h
typedef enum
{
  EntityNone    = 0x00,
  Entity0d      = 0x01,   // SMESH_Actor::e0DElements
  EntityEdges   = 0x02,   // SMESH_Actor::eEdges
  EntityFaces   = 0x04,   // SMESH_Actor::eFaces
  EntityVolumes = 0x08,   // SMESH_Actor::eVolumes
  EntityBalls   = 0x10,   // SMESH_Actor::eBallElem
  EntityAll     = 0xff    // SMESH_Actor::eAllEntity
} EntityMode;

// See SVTK_Selection.h
enum SelectionMode
{
  Undefined = -1,
  Node = 0,       // NodeSelection
  Cell,           // CellSelection
  EdgeOfCell,     // EdgeOfCellSelection
  Edge,           // EdgeSelection
  Face,           // FaceSelection
  Volume,         // VolumeSelection
  Actor,          // ActorSelection
  Elem0D,         // Elem0DSelection
  Ball            // BallSelection
};

// See SMESH_Actor.h
enum DisplayMode
{
  UndefinedMode = -1,
  PointMode = 0,      // SMESH_Actor::ePoint
  EdgeMode,           // SMESH_Actor::eEdge
  SurfaceMode         // SMESH_Actor::eSurface
};

struct ColorData
{
  double r, g, b;
};

struct BicolorData
{
  double r, g, b;
  int delta;
};

struct Properties
{
  ColorData nodeColor;
  int markerType;
  int markerScale;

  ColorData edgeColor;
  int edgeWidth;

  BicolorData surfaceColor;
  BicolorData volumeColor;

  ColorData elem0dColor;
  int elem0dSize;

  ColorData ballColor;
  double ballScale;

  ColorData outlineColor;
  int outlineWidth;

  ColorData orientationColor;
  double orientationScale;
  bool orientation3d;

  double shrinkFactor;
  double opacity;
};

typedef ColorData nodeColorStruct; // deprecated
typedef ColorData edgeColorStruct; // deprecated
typedef BicolorData surfaceColorStruct; // deprecated
typedef BicolorData volumeColorStruct; // deprecated
typedef Properties actorAspect; // deprecated

class SMESH_Swig
{
public:
  SMESH_Swig();
  ~SMESH_Swig();


  // Initialization =============================================

  void Init(); // deprecated

  // Publishing =================================================
  
  const char* publish(const char*, const char* = 0);
  void rename(const char*, const char*);

  const char* AddNewMesh(const char*, const char* = 0); // deprecated
  const char* AddNewHypothesis(const char*, const char* = 0); // deprecated
  const char* AddNewAlgorithms(const char*, const char* = 0); // deprecated
  const char* AddNewAlgorithm(const char*, const char* = 0); // deprecated

  void SetShape(const char*, const char*); // deprecated

  void SetHypothesis(const char*, const char*); // deprecated
  void SetAlgorithms(const char*, const char*); // deprecated

  void UnSetHypothesis(const char*); // deprecated

  const char* AddSubMesh(const char*, const char*, int, const char* = 0); // deprecated
  const char* AddSubMeshOnShape (const char*, const char*, const char*, int, const char* = 0); // deprecated

  void SetName(const char*, const char*); // deprecated

  void SetMeshIcon(const char*, const bool, const bool); // deprecated

  // Visualization  =============================================

  void display(const char*, int = 0, bool = true);
  void erase(const char*, int = 0, bool = true);
  void update(const char*);

  Properties properties(const char*, int = 0);
  void setProperties(const char*, const Properties&, int = 0);

  bool nodesNumbering(const char*, int = 0);
  void setNodesNumbering(const char*, bool, int = 0);
  bool elementsNumbering(const char*, int = 0);
  void setElementsNumbering(const char*, bool, int = 0);

  DisplayMode displayMode(const char*, int = 0);
  void setDisplayMode(const char*, DisplayMode, int = 0);

  bool shrinkMode(const char*, int = 0);
  void setShrinkMode(const char*, bool, int = 0);

  double opacity(const char*, int = 0);
  void setOpacity(const char*, double, int = 0);

  bool isOrientationShown(const char*, int = 0);
  void setOrientationShown(const char*, bool, int = 0);

  int entitiesShown(const char*, int = 0);
  void setEntitiesShown(const char*, int, int = 0);
  bool isEntityShown(const char*, EntityMode, int = 0);
  void setEntityShown(const char*, EntityMode, bool, int = 0);

  void CreateAndDisplayActor(const char*); // deprecated
  void EraseActor(const char*, const bool = false); // deprecated
  void UpdateActor(const char*); // deprecated

  actorAspect GetActorAspect(const char*, int = 0); // deprecated
  void SetActorAspect(const actorAspect&, const char*, int = 0); // deprecated

  // Selection  =================================================

  SelectionMode getSelectionMode(int = 0);
  void setSelectionMode(SelectionMode, int = 0);

  std::vector<int> getSelected(const char*);
  std::vector<std::pair<int,int> > getSelectedEdgeOfCell(const char*);

  void select(const char*, std::vector<int>, bool = false);
  void select(const char*, int, bool = false);
  void select(const char*, std::vector<std::pair<int,int> >, bool = false);
};
