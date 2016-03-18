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
// File   : SMESHGUI.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_H
#define SMESHGUI_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME GUI includes
#include <SalomeApp_Module.h>
#include <VTKViewer_MarkerDef.h>
#include <SALOME_InteractiveObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkType.h>

class vtkActor;
class vtkCallbackCommand;
class vtkObject;

class QDialog;

class SUIT_Desktop;
class SUIT_Study;
class SUIT_ViewWindow;
class SUIT_ResourceMgr;
class SUIT_ViewManager;

class LightApp_Operation;
class SalomeApp_Study;
class LightApp_Selection;
class LightApp_SelectionMgr;

class SMESH_Actor;
class SMESHGUI_FilterLibraryDlg;

typedef std::map<int, VTK::MarkerMap> SMESHGUI_StudyId2MarkerMap;

namespace SMESH
{
  class OrientedPlane;
  struct ClippingPlaneInfo
  {
    OrientedPlane*       Plane;
    std::list<vtkActor*> ActorList;
  };
}

typedef std::list<SMESH::ClippingPlaneInfo>                         SMESHGUI_ClippingPlaneInfoList;
typedef std::map<SUIT_ViewManager*, SMESHGUI_ClippingPlaneInfoList> SMESHGUI_ClippingPlaneInfoMap;

//=================================================================================
// class    : SMESHGUI
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI : public SalomeApp_Module
{
  Q_OBJECT

public :
  SMESHGUI();
  ~SMESHGUI();

  static SMESH::SMESH_Gen_var     GetSMESHGen();
  static SMESHGUI*                GetSMESHGUI();
  static LightApp_SelectionMgr*   selectionMgr();
  static SUIT_ResourceMgr*        resourceMgr();
  static SUIT_Desktop*            desktop();
  static SalomeApp_Study*         activeStudy();
  
  bool                            isActiveStudyLocked();

  static bool                     automaticUpdate(unsigned int requestedSize = 0, bool* limitExceeded = 0);
  static bool                     automaticUpdate( SMESH::SMESH_IDSource_ptr, int*, bool*, int*, long* );

  static void                     Modified( bool = true );

  virtual LightApp_Displayer*     displayer();
  virtual QString                 engineIOR() const;
  virtual void                    initialize( CAM_Application* );
  virtual void                    windows( QMap<int, int>& ) const;
  virtual void                    viewManagers( QStringList& ) const;

  QDialog*                        GetActiveDialogBox();
  void                            SetActiveDialogBox( QDialog* );

  void                            ResetState();
  void                            SetState( int );
  bool                            DefineDlgPosition( QWidget*, int&, int& );
  void                            switchToOperation( int );

  virtual bool                    OnGUIEvent( int );
  virtual bool                    OnMousePress( QMouseEvent*, SUIT_ViewWindow* );
  virtual bool                    OnMouseMove( QMouseEvent*, SUIT_ViewWindow* );
  virtual bool                    OnKeyPress( QKeyEvent*, SUIT_ViewWindow* );

  virtual LightApp_Selection*     createSelection() const;

  virtual void                    BuildPresentation ( const Handle(SALOME_InteractiveObject)&,
                                                      SUIT_ViewWindow* = 0 );

  /* Non modal dialog boxes management */
  void                            EmitSignalDeactivateDialog();
  void                            EmitSignalStudyFrameChanged();
  void                            EmitSignalCloseAllDialogs();
  void                            EmitSignalVisibilityChanged();
  void                            EmitSignalCloseView();
  void                            EmitSignalActivatedViewManager();

  virtual void                    contextMenuPopup( const QString&, QMenu*, QString& );
  virtual void                    createPreferences();
  virtual void                    preferencesChanged( const QString&, const QString& );

  virtual void                    message( const QString& );

  virtual void                    update( const int );

  static SALOMEDS::Color          getUniqueColor( const QList<SALOMEDS::Color>& );
  static SALOMEDS::Color          getPredefinedUniqueColor();

  virtual void                    storeVisualParameters  (int savePoint);
  virtual void                    restoreVisualParameters(int savePoint);

  virtual void                    addActorAsObserver( SMESH_Actor* theActor );
  
  virtual bool                    renameAllowed( const QString& ) const;
  virtual bool                    renameObject( const QString&, const QString& );


  SMESHGUI_ClippingPlaneInfoMap&  getClippingPlaneInfoMap() { return myClippingPlaneInfoMap; }

public slots:
  virtual bool                    deactivateModule( SUIT_Study* );
  virtual bool                    activateModule( SUIT_Study* );
  virtual void                    studyClosed( SUIT_Study* );
  void                            onViewClosed( SUIT_ViewWindow* );

private slots:
  void                            OnGUIEvent();
  void                            onViewManagerActivated( SUIT_ViewManager* );
  void                            onViewManagerRemoved( SUIT_ViewManager* );
  void                            onOperationCommited( SUIT_Operation* );
  void                            onOperationAborted( SUIT_Operation* );
  void                            onHypothesisEdit( int result );
  void                            onUpdateControlActions();

signals:
  void                            SignalDeactivateActiveDialog();
  void                            SignalStudyFrameChanged();
  void                            SignalCloseAllDialogs();
  void                            SignalVisibilityChanged();
  void                            SignalCloseView();
  void                            SignalActivatedViewManager();

protected:
  void                            createSMESHAction( const int,
                                                     const QString&,
                                                     const QString& = QString(),
                                                     const int = 0,
                                                     const bool = false,
                                                     const QString& = QString() );
  void                            createPopupItem( const int,
                                                   const QString&,
                                                   const QString&,
                                                   const QString& = QString(),
                                                   const int = -1 );

  virtual LightApp_Operation*     createOperation( const int ) const;

  virtual bool                    isSelectionCompatible();

  virtual bool                    reusableOperation( const int id ); 

  static void                     ProcessEvents( vtkObject* theObject, 
                                                 unsigned long theEvent,
                                                 void* theClientData, 
                                                 void* theCallData );

private:
  void                            OnEditDelete();
  int                             addVtkFontPref( const QString&, 
                                                  const int, 
                                                  const QString&,
                                                  const bool = false);

  void                            connectView( const SUIT_ViewWindow* );
 

private :
  static SMESH::SMESH_Gen_var     myComponentSMESH;
  QDialog*                        myActiveDialogBox;
  int                             myState;
  QMap<int, QString>              myRules;
  LightApp_Displayer*             myDisplayer;

  SMESHGUI_FilterLibraryDlg*      myFilterLibraryDlg;

  SMESHGUI_StudyId2MarkerMap      myMarkerMap;
  SMESHGUI_ClippingPlaneInfoMap   myClippingPlaneInfoMap;

  vtkSmartPointer<vtkCallbackCommand> myEventCallbackCommand;
  double            myPriority;
};

#endif // SMESHGUI_H
