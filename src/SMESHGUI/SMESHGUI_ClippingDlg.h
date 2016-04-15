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

class QGroupBox;
class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;
class QListWidget;
class QListWidgetItem;
class QStackedLayout;
class QSlider;
class QMenu;
class SALOME_Actor;
class SMESHGUI;
class SMESH_Actor;
class QtxDoubleSpinBox;
class QtxDoubleSpinSlider;
class QtxIntSpinSlider;
class vtkActor;
class vtkDataSetMapper;
class vtkPlaneSource;
class vtkCallbackCommand;
class vtkObject;
class vtkImplicitPlaneWidget;

namespace SMESH
{
  enum Mode { Absolute, Relative };
  enum Orientation { XY, YZ, ZX };

  class OrientedPlane: public vtkPlane
  {
    vtkDataSetMapper* myMapper;

    public:
      static OrientedPlane *New();
      static OrientedPlane *New(SVTK_ViewWindow* theViewWindow);
      vtkTypeMacro (OrientedPlane, vtkPlane);

      QPointer<SVTK_ViewWindow> myViewWindow;
      SMESH::Orientation myRelativeOrientation;
      float myDistance;
      double myAngle[2];
      double X, Y, Z, Dx, Dy, Dz;
      int myAbsoluteOrientation;
      bool IsInvert;
      bool IsOpenGLClipping;
      Mode PlaneMode;

      vtkPlaneSource* myPlaneSource;
      SALOME_Actor *myActor;

      void SetOrientation (SMESH::Orientation theOrientation) { myRelativeOrientation = theOrientation; }
      SMESH::Orientation GetOrientation() { return myRelativeOrientation; }

      void SetDistance (float theDistance) { myDistance = theDistance; }
      float GetDistance() { return myDistance; }

      void ShallowCopy (OrientedPlane* theOrientedPlane);
      OrientedPlane* InvertPlane ();

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

  static bool AddPlane ( SMESH::TActorList          theActorList,
                         SMESH::OrientedPlane*      thePlane );
protected:
  void        keyPressEvent( QKeyEvent* );
  static void ProcessEvents( vtkObject* theObject,
                             unsigned long theEvent,
                             void* theClientData,
                             void* theCallData);
private:
  double  getDistance() const;
  void    setDistance( const double );
  double  getRotation1() const;
  double  getRotation2() const;
  void    setRotation( const double, const double );

  void    setOrigin(double theVal[3]);
  void    setDirection(double theVal[3]);

  void    initializePlaneData();
  void    initParam();
  void    synchronize();
  void    updateActorList();
  void    updateActorItem( QListWidgetItem* theItem,
                           bool theUpdateSelectAll,
                           bool theUpdateClippingPlaneMap );
  SMESH::TActorList getCurrentActors();

  void    dumpPlaneData() const;
  void    absolutePlaneToRelative ( double theOrigin[3], double theDir[3] );
  void    setBoundsForPreviewWidget();
  vtkImplicitPlaneWidget* createPreviewWidget();


private:
  SMESHGUI*               mySMESHGUI;
  SVTK_ViewWindow*        myViewWindow;
  SMESH::TPlaneDataVector myPlanes;

  vtkCallbackCommand*     myCallback;
  vtkImplicitPlaneWidget* myPreviewWidget;
  double                  myBounds[6];
  
  QComboBox*              ComboBoxPlanes;
  QCheckBox*              isOpenGLClipping;
  QPushButton*            buttonNew;
  QMenu*                  MenuMode;
  QPushButton*            buttonDelete;

  QListWidget*            ActorList;
  QCheckBox*              SelectAllCheckBox;

  QStackedLayout*         ModeStackedLayout;

  QGroupBox*              GroupAbsolutePoint;
  QLabel*                 TextLabelX;
  QLabel*                 TextLabelY;
  QLabel*                 TextLabelZ;
  QtxDoubleSpinBox*       SpinBox_X;
  QtxDoubleSpinBox*       SpinBox_Y;
  QtxDoubleSpinBox*       SpinBox_Z;
  QPushButton*            resetButton;

  QGroupBox*              GroupAbsoluteDirection;
  QLabel*                 TextLabelDx;
  QLabel*                 TextLabelDy;
  QLabel*                 TextLabelDz;
  QtxDoubleSpinBox*       SpinBox_Dx;
  QtxDoubleSpinBox*       SpinBox_Dy;
  QtxDoubleSpinBox*       SpinBox_Dz;
  QPushButton*            invertButton;
  QComboBox*              CBAbsoluteOrientation;

  QGroupBox*              GroupRelative;
  QLabel*                 TextLabelOrientation;
  QLabel*                 TextLabelDistance;
  QLabel*                 TextLabelRotation1;
  QLabel*                 TextLabelRotation2;
  QtxDoubleSpinSlider*    SpinSliderDistance;
  QtxIntSpinSlider*       SpinSliderRotation1;
  QtxIntSpinSlider*       SpinSliderRotation2;
  QComboBox*              CBRelativeOrientation;

  QCheckBox*              PreviewCheckBox;
  QCheckBox*              AutoApplyCheckBox;

  QPushButton*            buttonOk;
  QPushButton*            buttonCancel;
  QPushButton*            buttonApply;
  QPushButton*            buttonHelp;
  
  bool                    myIsSelectPlane;
  bool                    myIsPreviewMoved;
  QString                 myHelpFileName;

  SMESH::Mode             CurrentMode;

protected slots:
  virtual void reject();

public slots:
  void onModeAbsolute();
  void onModeRelative();
  void ClickOnNew();
  void ClickOnDelete();
  void onSelectPlane( int );
  void onSelectAll( int );
  void onActorItemChanged( QListWidgetItem* );
  void SetCurrentPlaneParam();

  void onIsOpenGLClipping(bool);
  void OnPreviewToggle( bool );
  void onAutoApply(bool);
  void ClickOnOk();
  void ClickOnApply();
  void ClickOnHelp();

  void onSelectAbsoluteOrientation( int );
  void onSelectRelativeOrientation( int );
  void onReset();
  void onInvert();
};

#endif // SMESHGUI_CLIPPINGDLG_H
