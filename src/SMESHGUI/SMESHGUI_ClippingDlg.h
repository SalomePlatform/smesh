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
// File   : SMESHGUI_ClippingDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_CLIPPINGDLG_H
#define SMESHGUI_CLIPPINGDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME includes
#include <SVTK_ViewWindow.h>

// Qt includes
#include <QDialog>

// VTK includes
#include <vtkSmartPointer.h>

// STL includes
#include <vector>

class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;
class LightApp_SelectionMgr;
class SVTK_Selector;
class SMESHGUI;
class SMESH_Actor;
class OrientedPlane;
class SMESHGUI_SpinBox;

namespace SMESH
{
  typedef vtkSmartPointer<OrientedPlane> TVTKPlane;
  typedef std::vector<TVTKPlane> TPlanes;
  enum Orientation { XY, YZ, ZX };
};


//=================================================================================
// class    : SMESHGUI_ClippingDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_ClippingDlg : public QDialog
{
  Q_OBJECT

public:
  SMESHGUI_ClippingDlg( SMESHGUI* );
  ~SMESHGUI_ClippingDlg();
  
  double                  getDistance() const;
  void                    setDistance( const double );
  double                  getRotation1() const;
  double                  getRotation2() const;
  void                    setRotation( const double, const double );
  void                    Sinchronize();

  // used in SMESHGUI::restoreVisualParameters() to avoid
  // declaration of OrientedPlane outside of SMESHGUI_ClippingDlg.cxx
  static void             AddPlane (SMESH_Actor*         theActor,
                                    SVTK_ViewWindow*     theViewWindow,
                                    SMESH::Orientation   theOrientation,
                                    double               theDistance,
                                    vtkFloatingPointType theAngle[2]);

  static void             GetPlaneParam (SMESH_Actor*          theActor,
                                         int                   thePlaneIndex,
                                         SMESH::Orientation&   theOrientation,
                                         double&               theDistance,
                                         vtkFloatingPointType* theAngle);

protected:  
  void                    keyPressEvent( QKeyEvent* );

private:
  LightApp_SelectionMgr*  mySelectionMgr;
  SVTK_Selector*          mySelector;
  SMESHGUI*               mySMESHGUI;
  SMESH_Actor*            myActor;
  SMESH::TPlanes          myPlanes;
  
  QComboBox*              ComboBoxPlanes;
  QPushButton*            buttonNew;
  QPushButton*            buttonDelete;
  QLabel*                 TextLabelOrientation;
  QComboBox*              ComboBoxOrientation;
  QLabel*                 TextLabelDistance;
  SMESHGUI_SpinBox*       SpinBoxDistance;
  QLabel*                 TextLabelRot1;
  SMESHGUI_SpinBox*       SpinBoxRot1;
  QLabel*                 TextLabelRot2;
  SMESHGUI_SpinBox*       SpinBoxRot2;
  QCheckBox*              PreviewCheckBox;
  QCheckBox*              AutoApplyCheckBox;
  QPushButton*            buttonOk;
  QPushButton*            buttonCancel;
  QPushButton*            buttonApply;
  QPushButton*            buttonHelp;
  
  bool                    myIsSelectPlane;
  QString                 myHelpFileName;

public slots:
  void                    onSelectPlane( int );
  void                    ClickOnNew();
  void                    ClickOnDelete();
  void                    onSelectOrientation( int );
  void                    SetCurrentPlaneParam();
  void                    onSelectionChanged();
  void                    OnPreviewToggle( bool );
  void                    ClickOnOk();
  void                    ClickOnCancel();
  void                    ClickOnApply();
  void                    ClickOnHelp();
};

#endif // SMESHGUI_CLIPPINGDLG_H
