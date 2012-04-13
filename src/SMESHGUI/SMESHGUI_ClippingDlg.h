// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
#include <QPointer>

// VTK includes
#include <vtkPlane.h>
#include <vtkSmartPointer.h>

// STL includes
#include <list>
#include <map>
#include <vector>

class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;
class QListWidget;
class QListWidgetItem;
class SALOME_Actor;
class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_SpinBox;
class vtkActor;
class vtkDataSetMapper;
class vtkPlaneSource;

namespace SMESH
{
  enum Orientation { XY, YZ, ZX };

  class OrientedPlane: public vtkPlane
  {
    QPointer<SVTK_ViewWindow> myViewWindow;
    vtkDataSetMapper* myMapper;

  public:
    static OrientedPlane *New();
    static OrientedPlane *New(SVTK_ViewWindow* theViewWindow);
    vtkTypeMacro (OrientedPlane, vtkPlane);

    SMESH::Orientation myOrientation;
    float myDistance;
    double myAngle[2];

    vtkPlaneSource* myPlaneSource;
    SALOME_Actor *myActor;

    void SetOrientation (SMESH::Orientation theOrientation) { myOrientation = theOrientation; }
    SMESH::Orientation GetOrientation() { return myOrientation; }

    void SetDistance (float theDistance) { myDistance = theDistance; }
    float GetDistance() { return myDistance; }

    void ShallowCopy (OrientedPlane* theOrientedPlane);

  protected:
    OrientedPlane(SVTK_ViewWindow* theViewWindow);
    OrientedPlane();

    void Init();

    ~OrientedPlane();
  private:
    // Not implemented.
    OrientedPlane (const OrientedPlane&);
    void operator= (const OrientedPlane&);
  };

  typedef vtkSmartPointer<OrientedPlane>    TPlane;
  typedef std::list<vtkActor*>              TActorList;

  struct TPlaneData
  {
    TPlaneData( TPlane thePlane,
                TActorList theActorList )
    {
      Plane = thePlane;
      ActorList = theActorList;
    }
    TPlane     Plane;
    TActorList ActorList;
  };

  typedef std::vector<TPlane>               TPlaneVector;
  typedef std::vector<TPlaneData>           TPlaneDataVector;
};

//=================================================================================
// class    : SMESHGUI_ClippingDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_ClippingDlg : public QDialog
{
  Q_OBJECT

public:
  SMESHGUI_ClippingDlg( SMESHGUI*, SVTK_ViewWindow* );
  ~SMESHGUI_ClippingDlg();
  
  double                  getDistance() const;
  void                    setDistance( const double );
  double                  getRotation1() const;
  double                  getRotation2() const;
  void                    setRotation( const double, const double );

  // used in SMESHGUI::restoreVisualParameters() to avoid
  // declaration of OrientedPlane outside of SMESHGUI_ClippingDlg.cxx
  static SMESH::OrientedPlane* AddPlane (SMESH::TActorList          theActorList,
                                         SVTK_ViewWindow*           theViewWindow,
                                         SMESH::Orientation         theOrientation,
                                         double                     theDistance,
                                         const vtkFloatingPointType theAngle[2]);

protected:  
  void                    keyPressEvent( QKeyEvent* );

private:
  void                    initializePlaneData();

  void                    synchronize();

  void                    updateActorList();
  SMESH::TActorList       getCurrentActors();

  void                    updateActorItem( QListWidgetItem* theItem,
                                           bool theUpdateSelectAll,
                                           bool theUpdateClippingPlaneMap );

  void                    dumpPlaneData() const;

private:
  SMESHGUI*               mySMESHGUI;
  SVTK_ViewWindow*        myViewWindow;
  SMESH::TPlaneDataVector myPlanes;
  
  QComboBox*              ComboBoxPlanes;
  QPushButton*            buttonNew;
  QPushButton*            buttonDelete;
  QListWidget*            ActorList;
  QCheckBox*              SelectAllCheckBox;
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
  void                    onActorItemChanged( QListWidgetItem* );
  void                    onSelectAll( int );
  void                    onSelectOrientation( int );
  void                    SetCurrentPlaneParam();
  void                    OnPreviewToggle( bool );
  void                    ClickOnOk();
  void                    ClickOnCancel();
  void                    ClickOnApply();
  void                    ClickOnHelp();
};

#endif // SMESHGUI_CLIPPINGDLG_H
