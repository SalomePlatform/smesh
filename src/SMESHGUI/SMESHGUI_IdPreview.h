// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef SMESHGUI_IdPreview_H
#define SMESHGUI_IdPreview_H

#include "SMESH_SMESHGUI.hxx"

#include <list>
#include <vector>

#include <TColStd_MapOfInteger.hxx>
#include <SVTK_Hash.h>

#include <gp_XYZ.hxx>

class SALOME_Actor;
class SMDS_Mesh;
class SVTK_ViewWindow;
class vtkActor2D;
class vtkLabeledDataMapper;
class vtkMaskPoints;
class vtkRenderer;
class vtkSelectVisiblePoints;
class vtkTextProperty;
class vtkUnstructuredGrid;

/*!
 * \brief To display in the viewer IDs of selected elements or nodes
 */
class SMESHGUI_IdPreview
{
public:
  SMESHGUI_IdPreview(SVTK_ViewWindow* theViewWindow);
  ~SMESHGUI_IdPreview();

  void SetPointsData( SMDS_Mesh* theMesh, const SVTK_TVtkIDsMap & theNodesIdMap );
  void SetElemsData ( const std::vector<int> &  theElemsIdMap,
                      const std::list<gp_XYZ> & theGrCentersXYZ );
  template< class INT_ITER, class XYZ_ITER >
  void SetElemsData ( INT_ITER theElemsBegin, INT_ITER theElemsEnd,
                      XYZ_ITER theGrCentersBegin, XYZ_ITER theGrCentersEnd )
  {
    std::vector<int> elemsIds( theElemsBegin, theElemsEnd );
    std::list<gp_XYZ> gcXYZ( theGrCentersBegin, theGrCentersEnd );
    SetElemsData( elemsIds, gcXYZ );
  }
  void SetPointsLabeled( bool theIsPointsLabeled, bool theIsActorVisible = true );

  void AddToRender     ( vtkRenderer* theRenderer );
  void RemoveFromRender( vtkRenderer* theRenderer );

protected:

  SVTK_ViewWindow*        myViewWindow;

  vtkUnstructuredGrid*    myIdGrid;
  SALOME_Actor*           myIdActor;

  vtkUnstructuredGrid*    myPointsNumDataSet;
  vtkMaskPoints*          myPtsMaskPoints;
  vtkSelectVisiblePoints* myPtsSelectVisiblePoints;
  vtkLabeledDataMapper*   myPtsLabeledDataMapper;
  bool                    myIsPointsLabeled;
  vtkActor2D*             myPointLabels;

  std::vector<int>        myIDs;
};

#endif
