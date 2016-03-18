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
// File   : SMESHGUI_FilterDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_FILTERDLG_H
#define SMESHGUI_FILTERDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QWidget>
#include <QDialog>
#include <QMap>
#include <QList>

// SALOME GUI includes
#include <SALOME_DataMapOfIOMapOfInteger.hxx>
#include <SVTK_Selection.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class LightApp_SelectionMgr;
class QButtonGroup;
class QCheckBox;
class QFrame;
class QGroupBox;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTableWidgetItem;
class SMESHGUI;
class SMESHGUI_FilterLibraryDlg;
class SMESH_Actor;
class SVTK_Selector;

/*!
 *  Class       : SMESHGUI_FilterTable
 *  Description : Frame containig 
 *                  - Button group for switching entity type
 *                  - Table for displaying filter criterions
 *                  - Buttons for editing table
 */

class SMESHGUI_EXPORT SMESHGUI_FilterTable : public QWidget
{
  Q_OBJECT

  class Table;
  class ComboItem;
  class IntSpinItem;
  class DoubleSpinItem;
  class CheckItem;
  class AdditionalWidget;
  class ComboDelegate;

  typedef QMap<int, Table*> TableMap;

public:  
  SMESHGUI_FilterTable( SMESHGUI*, QWidget*, const int );
  SMESHGUI_FilterTable( SMESHGUI*, QWidget*, const QList<int>& );
  virtual ~SMESHGUI_FilterTable();

  void                      Init( const QList<int>& );

  QGroupBox*                GetTableGrp();

  bool                      IsValid( const bool = true, const int = -1 ) const;
  int                       GetType() const;
  void                      SetType(const int);
  void                      RestorePreviousEntityType();
  int                       NumRows( const int = -1 ) const;
  void                      Clear( const int = -1 );
  void                      SetEditable( const bool );
  void                      SetEnabled( const bool );
  void                      SetLibsEnabled( const bool );
  bool                      IsEditable() const;

  int                       GetCriterionType( const int, const int = -1 ) const;

  void                      GetCriterion( const int,
                                          SMESH::Filter::Criterion&,
                                          const int = -1 ) const;

  void                      SetCriterion( const int,
                                          const SMESH::Filter::Criterion&,
                                          const int = -1 );

  void                      AddCriterion( const SMESH::Filter::Criterion&,
                                          const int = -1 );

  void                      Copy( const SMESHGUI_FilterTable* );
  void                      SetValidity( const bool );

  bool                      CurrentCell( int&, int& ) const;
  void                      SetThreshold( const int,
                                          const QString&,
                                          const int = -1 );

  bool                      GetThreshold( const int,
                                          QString&,
                                          const int = -1 );

  void                      SetID( const int,
                                   const QString&,
                                   const int = -1 ); 
  
  bool                      GetID( const int,
                                   QString&,
                                   const int = -1 );

  void                      Update();

signals:
  void                      CopyFromClicked();
  void                      AddToClicked();
  void                      EntityTypeChanged( const int );
  void                      NeedValidation();
  void                      CriterionChanged( const int, const int );
  void                      ThresholdChanged( const int, const int );
  void                      CurrentChanged( int, int );

private slots:
  void                      onAddBtn();
  void                      onInsertBtn();
  void                      onRemoveBtn();
  void                      onClearBtn();
  void                      onCopyFromBtn();
  void                      onAddToBtn();
  void                      onCriterionChanged( int, int );
  void                      onEntityType( int );
  void                      onCurrentChanged( int, int );

private:
  void                      addRow( Table*, const int, const bool = true );
  QTableWidgetItem*         getCriterionItem( const int ) const;
  QTableWidgetItem*         getCompareItem() const;
  QTableWidgetItem*         getUnaryItem() const;
  QTableWidgetItem*         getBinaryItem() const;
  const QMap<int, QString>& getCriteria( const int ) const;
  const QMap<int, QString>& getCompare() const;
  Table*                    createTable( QWidget*, const int );
  QWidget*                  createAdditionalFrame( QWidget* );
  int                       getFirstSelectedRow() const;
  void                      onCriterionChanged( const int, const int, const int );

  void                      updateBtnState();
  void                      removeAdditionalWidget( QTableWidget*, const int );
  void                      updateAdditionalWidget();
  const char*               getPrecision( const int );

  const QMap<int, QString>& getSupportedTypes() const;

private:
  SMESHGUI*                 mySMESHGUI;

  QGroupBox*                myTableGrp;
  QWidget*                  mySwitchTableGrp;

  TableMap                  myTables;
  QPushButton*              myAddBtn;
  QPushButton*              myInsertBtn;
  QPushButton*              myRemoveBtn;
  QPushButton*              myClearBtn;  
  QPushButton*              myCopyFromBtn;  
  QPushButton*              myAddToBtn;

  QGroupBox*                myEntityTypeBox;
  QButtonGroup*             myEntityTypeGrp;
  int                       myEntityType;
  int                       myIsValid;
  bool                      myIsLocked;

  SMESHGUI_FilterLibraryDlg* myLibDlg;

  QStackedWidget*           myWgStack;

  QMap<QTableWidgetItem*, AdditionalWidget*> myAddWidgets;
};


/*!
 *  Class       : SMESHGUI_FilterDlg
 *  Description : Dialog to specify filters for VTK viewer
 */

class SMESHGUI_FilterDlg : public QDialog
{
  Q_OBJECT

  // Source elements to be selected
  enum { Mesh, Selection, Dialog, None };

  // Buttons
  enum { BTN_OK, BTN_Apply, BTN_Close, BTN_Help };

public:
  SMESHGUI_FilterDlg( SMESHGUI*, const QList<int>& );
  SMESHGUI_FilterDlg( SMESHGUI*, const int );
  virtual ~SMESHGUI_FilterDlg();

  void                      Init( const QList<int>&, const bool setInViewer=true );
  void                      Init( const int, const bool setInViewer=true );

  void                      SetSelection();
  void                      SetMesh (SMESH::SMESH_Mesh_var);
  void                      SetGroup (SMESH::SMESH_GroupOnFilter_var);
  void                      SetSourceWg( QWidget*, const bool initOnApply = true );
  void                      SetEnabled( bool setInViewer, bool diffSources );

  static SMESH::Filter::Criterion createCriterion();

  SMESH::Filter_var         GetFilter() const;
  void                      SetFilter(SMESH::Filter_var filter, int type);
  void                      UnRegisterFilters();

signals:

  void                      Accepted();

protected slots:
  virtual void              reject();

private slots:

  void                      onOk();
  bool                      onApply();
  void                      onHelp();
  void                      onDeactivate();
  void                      onSelectionDone();
  void                      onCriterionChanged( const int, const int );
  void                      onThresholdChanged( const int, const int );
  void                      onCurrentChanged( int, int );
  void                      onOpenView();
  void                      onCloseView();

private:

  void                      construct( const QList<int>& ); 

  void                      enterEvent( QEvent* );
  void                      keyPressEvent( QKeyEvent* );

  // dialog creation
  QWidget*                  createButtonFrame( QWidget* );
  QWidget*                  createMainFrame( QWidget* );
  QWidget*                  createSourceGroup( QWidget* );
  void                      updateMainButtons();

  // execution
  bool                      isValid() const;
  bool                      createFilter( const int );
  void                      insertFilterInViewer();
  void                      selectInViewer( const int, const QList<int>& );
  void                      filterSource( const int, QList<int>& );
  void                      filterSelectionSource( const int, QList<int>& );
  void                      getIdsFromWg( const QWidget*, QList<int>& ) const;
  void                      setIdsToWg( QWidget*, const QList<int>& );
  Selection_Mode            getSelMode( const int ) const;
  void                      updateSelection();
  SMESH_Actor*              getActor();

private:
  // widgets
  QWidget*                  myMainFrame;
  QButtonGroup*             mySourceGrp;

  QCheckBox*                mySetInViewer;

  QMap<int, QPushButton*>   myButtons;
  
  SMESHGUI_FilterTable*     myTable;

  // initial fields
  QList<int>                myTypes;
  SMESHGUI*                 mySMESHGUI;
  LightApp_SelectionMgr*    mySelectionMgr;
  SVTK_Selector*            mySelector;
  SMESH::SMESH_Mesh_var     myMesh;
  SMESH::SMESH_GroupOnFilter_var myGroup;
  bool                      myInitSourceWgOnApply;
  bool                      myInsertEnabled;
  bool                      myDiffSourcesEnabled;
  QWidget*                  mySourceWg;

  SALOME_DataMapOfIOMapOfInteger myIObjects;
  bool                           myIsSelectionChanged;
  QMap< int, SMESH::Filter_var > myFilter;
  QMap< int, bool >              myInsertState;
  QMap< int, int  >              myApplyToState;

  QString                   myHelpFileName;

  bool                      myToRestoreSelMode;
  int                       mySelModeToRestore;
  void                      restoreSelMode();
};

#endif // SMESHGUI_FILTERDLG_H
