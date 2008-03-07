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
//  File   : SMESHGUI_FilterLibraryDlg.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_FilterLibraryDlg_H
#define SMESHGUI_FilterLibraryDlg_H

#include "SMESH_SMESHGUI.hxx"

#include <qdialog.h>
#include <SMESHGUI_FilterDlg.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)

class QCloseEvent;
class QListBox;
class QFrame;
class QEvent;
class QLineEdit;
class QPushButton;
class QGroupBox;

/*
  Class       : SMESHGUI_FilterLibraryDlg
  Description : Dialog to specify filters for VTK viewer
*/

class SMESHGUI_EXPORT SMESHGUI_FilterLibraryDlg : public QDialog
{ 
  Q_OBJECT
  
  // Buttons
  enum { BTN_OK, BTN_Cancel, BTN_Apply, BTN_Close, BTN_Help };

  class Dialog;
  
public:
  
  // Mode
  enum { ADD_TO, COPY_FROM, EDIT };
    
public:
  
                              SMESHGUI_FilterLibraryDlg( SMESHGUI*,
							 QWidget*,
                                                         const QValueList<int>& types,
                                                         const int              mode,
                                                         const char*            name = 0 );
                                                
                              SMESHGUI_FilterLibraryDlg( SMESHGUI*,
							 QWidget*,
                                                         const int              type,
                                                         const int              mode,
                                                         const char*            name = 0 );
                                                         
  virtual                     ~SMESHGUI_FilterLibraryDlg();
                            
  void                        Init( const QValueList<int>& types, const int theMode );
  void                        Init( const int type, const int theMode );
  
  const SMESHGUI_FilterTable* GetTable() const;
  void                        SetTable( const SMESHGUI_FilterTable* );

private:

  void                        closeEvent( QCloseEvent* e ) ;
  void                        enterEvent ( QEvent * ) ;            
  void                        keyPressEvent( QKeyEvent* e );
  
private slots:

  void                        onOk();
  bool                        onApply();
  void                        onClose();
  void                        onHelp();

  void                        onDeactivate();
  
  void                        onBrowse();
  void                        onReturnPressed();
  void                        onFilterChanged( const QString& );
  void                        onAddBtnPressed();
  void                        onDeleteBtnPressed();
  void                        onFilterNameChanged( const QString& );
  void                        onEntityTypeChanged( const int );
  void                        onNeedValidation();

private:

  void                        construct( const QValueList<int>& types, const int mode ); 
  QFrame*                     createButtonFrame( QWidget* );
  QFrame*                     createMainFrame  ( QWidget* );
  bool                        isValid( const bool theMess = true ) const;
  bool                        isNameValid( const bool theMess = true ) const;
  SMESH::Filter_ptr           createFilter( const int theType = -1 );
  QStringList                 prepareFilters() const;
  QString                     autoExtension( const QString& ) const;
  bool                        setSelected( const QString& );
  int                         getIndex( const QString& ) const;
  void                        updateControlsVisibility();
  void                        updateMainButtons();
  void                        enableMainButtons();
  void                        processNewLibrary();
  QString                     getFileName() const;
  void                        setFileName( const QString& txt, const bool autoExtension = true );
  QStringList                 filterWildCards( const QString& theFilter ) const;
  QString&                    getDefaultLibraryName() const;
  QString                     getDefaultFilterName() const;
  void                        addFilterToLib( const QString& name );
  void                        updateList();
  bool                        isPermissionValid( const bool theIsExistingOnly );

private:

  QFrame*                     myMainFrame;
  QMap<int, QPushButton*>     myButtons;
  SMESHGUI_FilterTable*       myTable;
  QLineEdit*                  myFileName;
  QPushButton*                myOpenBtn;
  QListBox*                   myListBox;
  QPushButton*                myAddBtn;
  QPushButton*                myDeleteBtn;
  QGroupBox*                  myNameGrp;
  QLineEdit*                  myName;
  SMESHGUI*                   mySMESHGUI;

  QValueList<int>             myTypes;
  int                         myMode;
      
  SMESH::FilterLibrary_var    myLibrary;
  QString                     myCurrFilterName;
  int                         myCurrFilter;

  QString                     myHelpFileName;
};

#endif
