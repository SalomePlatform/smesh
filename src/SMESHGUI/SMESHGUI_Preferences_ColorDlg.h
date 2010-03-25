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
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_Preferences_ColorDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_PREFERENCES_COLORDLG_H
#define SMESHGUI_PREFERENCES_COLORDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME GUI includes
#include <VTKViewer_MarkerDef.h>

// Qt includes
#include <QDialog>

class QCheckBox;
class QSpinBox;
class SMESHGUI;
class QtxDoubleSpinBox;
class QtxIntSpinBox;
class QtxColorButton;
class VTKViewer_MarkerWidget;

class SMESHGUI_EXPORT SMESHGUI_Preferences_ColorDlg : public QDialog
{ 
  Q_OBJECT
    
public:
  SMESHGUI_Preferences_ColorDlg( SMESHGUI* );
  ~SMESHGUI_Preferences_ColorDlg();

  void                  SetColor( int, const QColor& );
  QColor                GetColor( int );
  void                  SetIntValue( int, int );
  int                   GetIntValue( int );
  void                  SetDoubleValue( int, double );
  double                GetDoubleValue( int );
  void                  SetBooleanValue( int, bool );
  bool                  GetBooleanValue( int );

  void                  setCustomMarkerMap( VTK::MarkerMap );
  VTK::MarkerMap        getCustomMarkerMap();

  void                  setStandardMarker( VTK::MarkerType, VTK::MarkerScale );
  void                  setCustomMarker( int );
  VTK::MarkerType       getMarkerType() const;
  VTK::MarkerScale      getStandardMarkerScale() const;
  int                   getCustomMarkerID() const;

protected:
  void                  closeEvent( QCloseEvent* );

private slots:
  void                  ClickOnOk();
  void                  ClickOnCancel();
  void                  DeactivateActiveDialog();
  void                  ActivateThisDialog();
  
private:
  SMESHGUI*             mySMESHGUI;            
  
  QtxColorButton*       btnFillColor;
  QtxColorButton*       btnBackFaceColor;
  QtxColorButton*       btnOutlineColor;
  QtxColorButton*       btn0DElementsColor;
  QSpinBox*             SpinBox_0DElements_Size;
  QSpinBox*             SpinBox_Width;
  QtxIntSpinBox*        SpinBox_Shrink;
  QtxColorButton*       btnNodeColor;
  VTKViewer_MarkerWidget* MarkerWidget;
  QtxColorButton*       btnOrientationColor;
  QtxDoubleSpinBox*     SpinBox_Orientation_Scale;
  QCheckBox*            CheckBox_Orientation_3DVectors;
};

#endif // SMESHGUI_PREFERENCES_COLORDLG_H
