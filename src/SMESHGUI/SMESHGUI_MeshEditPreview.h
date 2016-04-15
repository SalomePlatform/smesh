// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
// File   : SMESHGUI_MeshEditPreview.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MESHEDITPREVIEW_H
#define SMESHGUI_MESHEDITPREVIEW_H

#include "SMESH_SMESHGUI.hxx"
#include <vector>
#include <gp_Ax1.hxx>
#include <gp_Pnt.hxx>

class SALOME_Actor;
class SVTK_ViewWindow;
class vtkTextActor;
class vtkUnstructuredGrid;

namespace SMESH
{
  class MeshPreviewStruct;
}

/*!
 * \brief Displayer of the mesh edition preview
 */
class SMESHGUI_EXPORT SMESHGUI_MeshEditPreview
{
  SVTK_ViewWindow*     myViewWindow;

  vtkUnstructuredGrid* myGrid;
  SALOME_Actor*        myPreviewActor;

  std::vector<gp_Pnt>        myUnitArrowPnts;
  std::vector<vtkTextActor*> myLabelActors;

public:
  SMESHGUI_MeshEditPreview( SVTK_ViewWindow* );
  ~SMESHGUI_MeshEditPreview();

  void                 SetData( const SMESH::MeshPreviewStruct* );

  void                 SetVisibility( bool );
  void                 SetColor( double, double, double );

  void                 SetArrowShapeAndNb( int         nbArrows,
                                           double      headLength,
                                           double      headRadius,
                                           double      start=0.,
                                           const char* labels=0);
  void                 SetArrows( const gp_Ax1* axes,
                                  double        length);

  SALOME_Actor*        GetActor() const;
  vtkUnstructuredGrid* GetGrid() const;
  SVTK_ViewWindow*     GetViewWindow() const;
};

#endif // SMESHGUI_MESHEDITPREVIEW_H
