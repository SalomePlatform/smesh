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
#include "SALOMEGUI.h"
#include "SALOME_InteractiveObject.hxx"

class QAD_Desktop;
class QAD_Study;

class QDialog;


//=================================================================================
// class    : SMESHGUI
// purpose  :
//=================================================================================
class SMESHGUI : public SALOMEGUI
{
  Q_OBJECT;

private :
  QAD_Desktop* myDesktop;
  QAD_Study* myActiveStudy;
  QDialog* myActiveDialogBox;

  int myState;
  bool myAutomaticUpdate;

public :
  SMESHGUI( const QString& name = "", QObject* parent = 0 );
  static SMESHGUI*    GetSMESHGUI() ;
  ~SMESHGUI();

  QAD_Desktop*        GetDesktop() ;
  QAD_Study*          GetActiveStudy() ;
  bool                ActiveStudyLocked();

  QDialog*            GetActiveDialogBox() ;               
  void                SetActiveDialogBox(QDialog* aDlg) ;  

  void                ResetState() ;                       
  void                SetState(int aState) ;
  bool                DefineDlgPosition(QWidget* aDlg, int& x, int& y) ;

  virtual bool OnGUIEvent        (int theCommandID, QAD_Desktop* parent);
  virtual bool OnMousePress      (QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame);
  virtual bool OnMouseMove       (QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame);
  virtual bool OnKeyPress        (QKeyEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame);
  virtual bool ActiveStudyChanged( QAD_Desktop* parent );
  virtual bool SetSettings       ( QAD_Desktop* parent );
  virtual void DefinePopup       ( QString & theContext, QString & theParent, QString & theObject );
  virtual bool CustomPopup       ( QAD_Desktop* parent, QPopupMenu* popup, const QString & theContext,
                                   const QString & theParent, const QString & theObject );
  virtual void BuildPresentation ( const Handle(SALOME_InteractiveObject)& theIO,
                                   QAD_ViewFrame* = 0 );
  virtual void SupportedViewType (int* buffer, int bufferSize);
  virtual void Deactivate        ();

  /* Non modal dialog boxes magement */
  void EmitSignalDeactivateDialog() ;
  void EmitSignalStudyFrameChanged() ;
  void EmitSignalCloseAllDialogs() ;

signals:
  void SignalDeactivateActiveDialog() ;
  void SignalStudyFrameChanged() ;
  void SignalCloseAllDialogs() ;

};


#endif
