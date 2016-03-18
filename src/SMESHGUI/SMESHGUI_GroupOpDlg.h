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
// File   : SMESHGUI_GroupOpDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_GROUPOPDLG_H
#define SMESHGUI_GROUPOPDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

class LightApp_SelectionMgr;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QListWidget;
class QPushButton;
class QtxColorButton;
class SMESHGUI;
class SVTK_Selector;

/*
  Class       : SMESHGUI_GroupOpDlg
  Description : Perform boolean operations on groups
*/

class SMESHGUI_EXPORT SMESHGUI_GroupOpDlg : public QDialog
{ 
  Q_OBJECT
    
public:
  //enum { UNION, INTERSECT, CUT };
    
public:
  SMESHGUI_GroupOpDlg( SMESHGUI* );
  virtual ~SMESHGUI_GroupOpDlg();

  void                      Init();

protected slots:

  virtual bool              onApply();
  virtual void              onSelectionDone();
  virtual void              setVisible ( bool visible );

protected:

  virtual void              reset();

  QString                   getName() const;
  void                      setName( const QString& theName );

  QGroupBox*                getArgGrp() const;
  void                      setHelpFileName( const QString& theFName );
  SMESHGUI*                 getSMESHGUI() const;
  bool                      isValid( const QList<SMESH::SMESH_GroupBase_var>& theListGrp );
  bool                      getSelectedGroups( QList<SMESH::SMESH_GroupBase_var>& theOutList,
                                               QStringList& theOutNames );
  SMESH::ListOfGroups*      convert( const QList<SMESH::SMESH_GroupBase_var>& );

  SALOMEDS::Color           getColor() const;
  void                      setDefaultGroupColor();

  void                      setIsApplyAndClose( const bool theFlag );
  bool                      isApplyAndClose() const;

private:
  void                      enterEvent( QEvent* );            
  void                      keyPressEvent( QKeyEvent* );
  
protected slots:
  virtual void              reject();

private slots:
  void                      onOk();
  void                      onHelp();

  void                      onDeactivate();

  void                      onOpenView();
  void                      onCloseView();

private:
  QWidget*                  createButtonFrame( QWidget* );
  QWidget*                  createMainFrame  ( QWidget* );
    
private:
  QPushButton*              myOkBtn;
  QPushButton*              myApplyBtn;
  QPushButton*              myCloseBtn;
  QPushButton*              myHelpBtn;
  
  QLineEdit*                myNameEdit;
  QGroupBox*                myArgGrp;
  QtxColorButton*           myColorBtn;
  
  SMESHGUI*                 mySMESHGUI;
  LightApp_SelectionMgr*    mySelectionMgr;
  SVTK_Selector*            mySelector;
  
  QString                   myHelpFileName;

  bool                      myIsApplyAndClose;
};

/*
  Class       : SMESHGUI_UnionGroupsDlg
  Description : Perform union of several groups
*/

class SMESHGUI_EXPORT SMESHGUI_UnionGroupsDlg : public SMESHGUI_GroupOpDlg
{ 
  Q_OBJECT
    
public:

  SMESHGUI_UnionGroupsDlg( SMESHGUI* );
  virtual ~SMESHGUI_UnionGroupsDlg();

protected slots:
  virtual bool                      onApply();
  virtual void                      onSelectionDone();

protected:
  virtual void                      reset();

private:
  QListWidget*                      myListWg;
  QList<SMESH::SMESH_GroupBase_var> myGroups;
};

/*
  Class       : SMESHGUI_IntersectGroupsDlg
  Description : Perform intersection of several groups
*/

class SMESHGUI_EXPORT SMESHGUI_IntersectGroupsDlg : public SMESHGUI_GroupOpDlg
{ 
  Q_OBJECT
    
public:

  SMESHGUI_IntersectGroupsDlg( SMESHGUI* );
  virtual ~SMESHGUI_IntersectGroupsDlg();

protected slots:
  virtual bool                      onApply();
  virtual void                      onSelectionDone();

protected:
  virtual void                      reset();
    
private:
  QListWidget*                      myListWg;
  QList<SMESH::SMESH_GroupBase_var> myGroups;
};

/*
  Class       : SMESHGUI_CutGroupsDlg
  Description : Perform cut of several groups
*/

class SMESHGUI_EXPORT SMESHGUI_CutGroupsDlg : public SMESHGUI_GroupOpDlg
{ 
  Q_OBJECT
    
public:

  SMESHGUI_CutGroupsDlg( SMESHGUI* );
  virtual ~SMESHGUI_CutGroupsDlg();

protected slots:
  virtual bool                      onApply();
  virtual void                      onSelectionDone();

protected:
  virtual void                      reset();

private:
  QPushButton*                      myBtn1;
  QPushButton*                      myBtn2;
  QListWidget*                      myListWg1;
  QListWidget*                      myListWg2;
  QList<SMESH::SMESH_GroupBase_var> myGroups1;
  QList<SMESH::SMESH_GroupBase_var> myGroups2;
};

/*
  Class       : SMESHGUI_DimGroupDlg
  Description : Dialog for creating groups of entities from existing 
                groups of superior dimensions
*/

class SMESHGUI_EXPORT SMESHGUI_DimGroupDlg : public SMESHGUI_GroupOpDlg
{ 
  Q_OBJECT
    
public:

  SMESHGUI_DimGroupDlg( SMESHGUI* );
  virtual ~SMESHGUI_DimGroupDlg();

  SMESH::ElementType                getElementType() const;
  void                              setElementType( const SMESH::ElementType& theElemType );

protected:
  virtual void                      reset();

protected slots:
  virtual bool                      onApply();
  virtual void                      onSelectionDone();

private:
  QComboBox*                        myTypeCombo;
  QComboBox*                        myNbNoCombo;
  QListWidget*                      myListWg;
  QCheckBox*                        myUnderlOnlyChk;
  
  QList<SMESH::SMESH_GroupBase_var> myGroups;
};

#endif // SMESHGUI_GROUPOPDLG_H


