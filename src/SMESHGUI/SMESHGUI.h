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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_HeaderFile
#define SMESHGUI_HeaderFile

// SALOME Includes
#include <SalomeApp_Module.h>
#include <SALOME_InteractiveObject.hxx>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)

class QDialog;

class SUIT_Desktop;
class SUIT_Study;
class SUIT_ViewWindow;
class SUIT_ResourceMgr;
class SUIT_ViewManager;

class LightApp_Operation;
class SalomeApp_Study;
class LightApp_SelectionMgr;


//=================================================================================
// class    : SMESHGUI
// purpose  :
//=================================================================================
class SMESHGUI : public SalomeApp_Module
{
  Q_OBJECT;

public :
  SMESHGUI();
  ~SMESHGUI();

  static SMESH::SMESH_Gen_var     GetSMESHGen();
  static SMESHGUI*                GetSMESHGUI();
  static LightApp_SelectionMgr*   selectionMgr();
  static SUIT_ResourceMgr*        resourceMgr();
  static SUIT_Desktop*            desktop() ;
  static SalomeApp_Study*         activeStudy();
  bool                            isActiveStudyLocked();

  static bool                     automaticUpdate();

  virtual LightApp_Displayer*     displayer();
  virtual QString     engineIOR() const;
  virtual void        initialize( CAM_Application* );
  virtual void        windows( QMap<int, int>& ) const;
  virtual void        viewManagers( QStringList& ) const;

  QDialog*            GetActiveDialogBox() ;
  void                SetActiveDialogBox(QDialog* aDlg) ;

  void                ResetState() ;
  void                SetState(int aState) ;
  bool                DefineDlgPosition(QWidget* aDlg, int& x, int& y) ;

  virtual bool OnGUIEvent        ( int id );
  virtual bool OnMousePress      ( QMouseEvent*, SUIT_ViewWindow* );
  virtual bool OnMouseMove       ( QMouseEvent*, SUIT_ViewWindow* );
  virtual bool OnKeyPress        ( QKeyEvent*, SUIT_ViewWindow* );

  virtual void contextMenuPopup( const QString&, QPopupMenu*, QString& );

  virtual void BuildPresentation ( const Handle(SALOME_InteractiveObject)&,
                                   SUIT_ViewWindow* = 0 );

  /* Non modal dialog boxes management */
  void EmitSignalDeactivateDialog() ;
  void EmitSignalStudyFrameChanged() ;
  void EmitSignalCloseAllDialogs() ;

  virtual void                createPreferences();
  virtual void                preferencesChanged( const QString&, const QString& );
  
  virtual void                update( const int );

public slots:
  virtual bool                deactivateModule( SUIT_Study* );
  virtual bool                activateModule( SUIT_Study* );

private slots:
  void                        OnGUIEvent();
  void                        onViewManagerActivated( SUIT_ViewManager* );
  void                        onOperationCommited( SUIT_Operation* );
  void                        onOperationAborted( SUIT_Operation* );


signals:
  void SignalDeactivateActiveDialog() ;
  void SignalStudyFrameChanged() ;
  void SignalCloseAllDialogs() ;

protected:
  void createSMESHAction( const int, const QString&, const QString& = QString(""),
                          const int = 0, const bool = false );
  void createPopupItem( const int, const QString&, const QString&,
                        const QString& = QString::null, const int = -1 );
  
  virtual LightApp_Operation*      createOperation( const int ) const;

private:
  void OnEditDelete();

private :
  static SMESH::SMESH_Gen_var      myComponentSMESH;
  QDialog*                         myActiveDialogBox;
  int                              myState;
  QMap<int,QString>                myRules;
  LightApp_Displayer*              myDisplayer;
};

#endif
