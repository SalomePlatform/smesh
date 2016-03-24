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
//  File   : SMESHGUI_PropertiesDlg.h
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#ifndef SMESHGUI_PROPERTIESDLG_H
#define SMESHGUI_PROPERTIESDLG_H

#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_Dialog.h"

#include <VTKViewer_MarkerDef.h>
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Mesh)

class QCheckBox;
class QFrame;
class QGroupBox;
class QtxColorButton;
class QtxBiColorTool;
class QtxIntSpinBox;
class QtxDoubleSpinBox;
class VTKViewer_MarkerWidget;

class SMESHGUI_EXPORT SMESHGUI_PropertiesDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT
    
public:
  SMESHGUI_PropertiesDlg( const VTK::MarkerMap&, QWidget* parent );
  ~SMESHGUI_PropertiesDlg();

  void              setNodeColor( const QColor& );
  QColor            nodeColor() const;
  void              setNodeMarker( VTK::MarkerType, VTK::MarkerScale );
  void              setNodeCustomMarker( int );
  VTK::MarkerType   nodeMarkerType() const;
  VTK::MarkerScale  nodeMarkerScale() const;
  int               nodeMarkerId() const;

  void              setEdgeColor( const QColor& );
  QColor            edgeColor() const;
  void              setEdgeWidth( int );
  int               edgeWidth() const;

  void              setFaceColor( const QColor&, int );
  QColor            faceColor() const;
  int               faceColorDelta() const;

  void              setVolumeColor( const QColor&, int );
  QColor            volumeColor() const;
  int               volumeColorDelta() const;

  void              setOutlineColor( const QColor& );
  QColor            outlineColor() const;
  void              setOutlineWidth( int );
  int               outlineWidth() const;

  void              setElem0dColor( const QColor& );
  QColor            elem0dColor() const;
  void              setElem0dSize( int );
  int               elem0dSize() const;

  void              setBallColor( const QColor& );
  QColor            ballColor() const;
  // void           setBallSize( int );
  // int            ballSize() const;
  void              setBallScale( double );
  double            ballScale() const;

  void              setOrientationColor( const QColor& );
  QColor            orientationColor() const;
  void              setOrientationSize( int );
  int               orientationSize() const;
  void              setOrientation3d( bool );
  bool              orientation3d() const;

  void              setShrinkCoef( int );
  int               shrinkCoef() const;

  VTK::MarkerMap    customMarkers() const;

  void              showControls( int, bool );

private slots:
  void              onHelp();

private:
  // group boxes
  QGroupBox*              myNodeGrp;
  QGroupBox*              myEdgeGrp;
  QGroupBox*              myFaceGrp;
  QGroupBox*              myVolumeGrp;
  QGroupBox*              myOutlineGrp;
  QGroupBox*              myElem0dGrp;
  QGroupBox*              myBallGrp;
  QGroupBox*              myOrientationGrp;
  QFrame*                 myExtraGrp;
  // widgets
  // - nodes
  QtxColorButton*         myNodeColor;
  VTKViewer_MarkerWidget* myNodeMarker;
  // - edges
  QtxColorButton*         myEdgeColor;
  QtxIntSpinBox*          myEdgeWidth;
  // - faces
  QtxBiColorTool*         myFaceColor; 
  // - volumes
  QtxBiColorTool*         myVolumeColor; 
  // - outlines
  QtxColorButton*         myOutlineColor;
  QtxIntSpinBox*          myOutlineWidth;
  // - 0d elements
  QtxColorButton*         myElem0dColor;
  QtxIntSpinBox*          myElem0dSize;
  // - balls
  QtxColorButton*         myBallColor;
  // QtxIntSpinBox*       myBallSize;
  QtxDoubleSpinBox*       myBallScale;
  // - orientation vectors
  QtxColorButton*         myOrientationColor;
  QtxIntSpinBox*          myOrientationSize;
  QCheckBox*              myOrientation3d;
  // - shrink coefficient
  QtxIntSpinBox*          myShrinkSize;
};

#endif // SMESHGUI_PROPERTIESDLG_H
