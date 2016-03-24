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
// File   : SMESHGUI_FilterLibraryDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_FILTERLIBRARYDLG_H
#define SMESHGUI_FILTERLIBRARYDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>
#include <QList>
#include <QMap>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QPushButton;
class SMESHGUI;
class SMESHGUI_FilterTable;

/*
  Class       : SMESHGUI_FilterLibraryDlg
  Description : Dialog to specify filters for VTK viewer
*/

class SMESHGUI_EXPORT SMESHGUI_FilterLibraryDlg : public QDialog
{ 
  Q_OBJECT
  
  // Buttons
  enum { BTN_OK, BTN_Apply, BTN_Close, BTN_Help };

  class Dialog;
  
public:
  
  // Mode
  enum { ADD_TO, COPY_FROM, EDIT };
    
public:
  SMESHGUI_FilterLibraryDlg( SMESHGUI*, QWidget*, const QList<int>&, const int );
  SMESHGUI_FilterLibraryDlg( SMESHGUI*, QWidget*, const int, const int );
  virtual ~SMESHGUI_FilterLibraryDlg();
                            
  void                        Init( const QList<int>&, const int );
  void                        Init( const int, const int );
  
  const SMESHGUI_FilterTable* GetTable() const;
  void                        SetTable( const SMESHGUI_FilterTable* );

private:
  void                        enterEvent( QEvent* );            
  void                        keyPressEvent( QKeyEvent* );
  
protected slots:
  virtual void                reject();

private slots:
  void                        onOk();
  bool                        onApply();
  void                        onHelp();

  void                        onDeactivate();
  
  void                        onBrowse();
  void                        onReturnPressed();
  void                        onFilterChanged();
  void                        onAddBtnPressed();
  void                        onDeleteBtnPressed();
  void                        onFilterNameChanged( const QString& );
  void                        onEntityTypeChanged( const int );
  void                        onNeedValidation();
  void                        onSelectionDone();

private:
  void                        construct( const QList<int>&, const int ); 
  QWidget*                    createButtonFrame( QWidget* );
  QWidget*                    createMainFrame( QWidget* );
  bool                        isValid( const bool = true ) const;
  bool                        isNameValid( const bool = true ) const;
  SMESH::Filter_ptr           createFilter( const int = -1 );
  QStringList                 prepareFilters() const;
  QString                     autoExtension( const QString& ) const;
  bool                        setSelected( const QString& );
  int                         getIndex( const QString& ) const;
  void                        updateControlsVisibility();
  void                        updateMainButtons();
  void                        enableMainButtons();
  void                        processNewLibrary();
  QString                     getFileName() const;
  void                        setFileName( const QString&, const bool = true );
  QStringList                 filterWildCards( const QString& ) const;
  QString&                    getDefaultLibraryName() const;
  QString                     getDefaultFilterName() const;
  void                        addFilterToLib( const QString& );
  void                        updateList();
  bool                        isPermissionValid( const bool );

private:

  QWidget*                    myMainFrame;
  QMap<int, QPushButton*>     myButtons;
  SMESHGUI_FilterTable*       myTable;
  QLineEdit*                  myFileName;
  QPushButton*                myOpenBtn;
  QListWidget*                myListBox;
  QPushButton*                myAddBtn;
  QPushButton*                myDeleteBtn;
  QWidget*                    myNameGrp;
  QLineEdit*                  myName;
  SMESHGUI*                   mySMESHGUI;

  QList<int>                  myTypes;
  int                         myMode;
      
  SMESH::FilterLibrary_var    myLibrary;
  QString                     myCurrFilterName;
  int                         myCurrFilter;

  QString                     myHelpFileName;
};

#endif // SMESHGUI_FILTERLIBRARYDLG_H
