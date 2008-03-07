//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_TransparencyDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_CLIPPING_H
#define DIALOGBOX_CLIPPING_H

#include "SMESH_SMESHGUI.hxx"

#include <SMESHGUI_SpinBox.h>

#include <vtkSmartPointer.h>
#include <vtkPlane.h>

#include <vector>

// QT Includes
#include <qdialog.h>
#include <qtable.h>

class QLabel;
class QPushButton;
class QTable;
class QCheckBox;
class QComboBox;

class LightApp_SelectionMgr;
class SVTK_Selector;

class SMESHGUI;
class SMESH_Actor;

class OrientedPlane;


namespace SMESH {

  typedef vtkSmartPointer<OrientedPlane> TVTKPlane;
  typedef std::vector<TVTKPlane> TPlanes;
  enum Orientation {XY, YZ, ZX};

};


//=================================================================================
// class    : SMESHGUI_ClippingDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_ClippingDlg : public QDialog
{
    Q_OBJECT

public:
    SMESHGUI_ClippingDlg (SMESHGUI* theModule,
			  const char* name = 0,
			  bool modal = false,
			  WFlags fl = 0);

    float  getDistance()  { return  (float)SpinBoxDistance->GetValue(); }
    void   setDistance (const float theDistance) { SpinBoxDistance->SetValue(theDistance); }
    double getRotation1() { return SpinBoxRot1->GetValue(); }
    double getRotation2() { return SpinBoxRot2->GetValue(); }
    void   setRotation (const double theRot1, const double theRot2);
    void   Sinchronize();

    void   keyPressEvent(QKeyEvent*);

    ~SMESHGUI_ClippingDlg();

private:

    LightApp_SelectionMgr*  mySelectionMgr;
    SVTK_Selector*          mySelector;
    SMESHGUI*               mySMESHGUI;
    SMESH_Actor*            myActor;
    SMESH::TPlanes          myPlanes;

    QComboBox*        ComboBoxPlanes;
    QPushButton*      buttonNew;
    QPushButton*      buttonDelete;
    QLabel*           TextLabelOrientation;
    QComboBox*        ComboBoxOrientation;
    QLabel*           TextLabelDistance;
    SMESHGUI_SpinBox* SpinBoxDistance;
    QLabel*           TextLabelRot1;
    SMESHGUI_SpinBox* SpinBoxRot1;
    QLabel*           TextLabelRot2;
    SMESHGUI_SpinBox* SpinBoxRot2;
    QCheckBox*        PreviewCheckBox;
    QCheckBox*        AutoApplyCheckBox;
    QPushButton*      buttonOk;
    QPushButton*      buttonCancel;
    QPushButton*      buttonApply;
    QPushButton*      buttonHelp;

    bool myIsSelectPlane;
    QString myHelpFileName;

public slots:

    void onSelectPlane (int theIndex);
    void ClickOnNew();
    void ClickOnDelete();
    void onSelectOrientation (int theItem);
    void SetCurrentPlaneParam();
    void onSelectionChanged();
    void OnPreviewToggle (bool theIsToggled);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void ClickOnHelp();
};

#endif // DIALOGBOX_TRANSPARENCYDLG_H
