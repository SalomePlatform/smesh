//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  SMESH SMESHGUI : GUI for SMESH component
//  File   : SMESHGUI_MeshEditPreview.cxx
//  Module : SMESH
//  $Header:
//
#ifndef SMESHGUI_MESHEDITPREVIEW_H
#define SMESHGUI_MESHEDITPREVIEW_H

class SVTK_ViewWindow;
class vtkUnstructuredGrid;
class SALOME_Actor;
namespace SMESH {
  class MeshPreviewStruct;
}

/*!
 * \brief Displayer of the mesh edition preview
 */
class SMESHGUI_MeshEditPreview {
  SVTK_ViewWindow* myViewWindow;

  vtkUnstructuredGrid* myGrid;
  SALOME_Actor* myPreviewActor;

public:

  SMESHGUI_MeshEditPreview(SVTK_ViewWindow* theViewWindow);
  ~SMESHGUI_MeshEditPreview();

  void SetData (const SMESH::MeshPreviewStruct* theMeshPreviewStruct);

  void SetVisibility (bool theVisibility);

  void SetColor(double R, double G, double B);

  SALOME_Actor* GetActor() { return myPreviewActor; }

};

#endif
