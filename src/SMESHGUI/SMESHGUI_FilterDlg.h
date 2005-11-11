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
//  File   : SMESHGUI_FilterDlg.h
//  Author : Sergey LITONIN      
//  Module : SMESH


#ifndef SMESHGUI_FilterDlg_H
#define SMESHGUI_FilterDlg_H

#include <qdialog.h>
#include <qframe.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "LightApp_SelectionMgr.h"
#include "SALOME_InteractiveObject.hxx"
#include "SALOME_DataMapOfIOMapOfInteger.hxx"
#include "SVTK_Selection.h"

#include <TColStd_IndexedMapOfInteger.hxx>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QCheckBox;
class QCloseEvent;
class QComboBox;
class QEvent;
class QGroupBox;
class QPushButton;
class QStringList;
class QTable;
class QTableItem;
class QWidgetStack;
class LightApp_SelectionMgr;
class SMESHGUI;
class SMESHGUI_FilterLibraryDlg;
class SVTK_Selector;
class SVTK_ViewWindow;

/*!
 *  Class       : SMESHGUI_FilterTable
 *  Description : Frame containig 
 *                  - Button group for switching entity type
 *                  - Table for displaying filter criterions
 *                  - Buttons for editing table
 */

class SMESHGUI_FilterTable : public QFrame
{
  Q_OBJECT

  class Table;
  class ComboItem;
  class AdditionalWidget;

  typedef QMap<int, Table*> TableMap;

public:  
                            SMESHGUI_FilterTable( SMESHGUI*,
						  QWidget* parent, 
                                                  const int type);
                            SMESHGUI_FilterTable( SMESHGUI*,
						  QWidget* parent, 
                                                  const QValueList<int>& type);
  virtual                   ~SMESHGUI_FilterTable();

  void                      Init (const int type);
  void                      Init (const QValueList<int>& types);

  QGroupBox*                GetTableGrp();

  bool                      IsValid (const bool = true, const int theEntityType = -1) const;
  int                       GetType() const;
  void                      SetType (const int);
  void                      RestorePreviousEntityType();
  int                       NumRows (const int theEntityType = -1) const;
  void                      Clear (const int theEntityType = -1);
  void                      SetEditable (const bool);
  void                      SetEnabled (const bool);
  void                      SetLibsEnabled (const bool);
  bool                      IsEditable() const;

  int                       GetCriterionType (const int theRow, const int theType = -1) const;

  void                      GetCriterion (const int                 theRow,
                                          SMESH::Filter::Criterion& theCriterion,
                                          const int                 theEntityType = -1) const;

  void                      SetCriterion (const int                       theRow,
                                          const SMESH::Filter::Criterion& theCriterion,
                                          const int                       theEntityType = -1);

  void                      AddCriterion (const SMESH::Filter::Criterion& theCriterion,
                                          const int    theEntityType = -1);

  void                      Copy (const SMESHGUI_FilterTable*);
  void                      SetValidity (const bool);

  bool                      CurrentCell (int& theRow, int& theCol) const;
  void                      SetThreshold (const int      theRow,
                                          const QString& theText,
                                          const int      theEntityType = -1);

  bool                      GetThreshold (const int      theRow,
                                          QString&       theText,
                                          const int      theEntityType = -1);

  void                      SetID( const int      theRow,
				   const QString& theText,
				   const int      theEntityType = -1 ); 
  
  bool                      GetID( const int      theRow,
				   QString&       theText,
				   const int      theEntityType = -1 );

  void                      Update();

signals:

  void                      CopyFromClicked();
  void                      AddToClicked();
  void                      EntityTypeChanged (const int);
  void                      NeedValidation();
  void                      CriterionChanged (const int theRow, const int theEntityType);
  void                      CurrentChanged (int, int);

private slots:

  void                      onAddBtn();
  void                      onInsertBtn();
  void                      onRemoveBtn();
  void                      onClearBtn();
  void                      onCopyFromBtn();
  void                      onAddToBtn();
  void                      onCriterionChanged (int, int);
  void                      onEntityType (int);
  void                      onCurrentChanged (int, int);

private:

  void                      addRow (Table*, const int, const bool toTheEnd = true);
  QTableItem*               getCriterionItem (QTable*, const int);
  QTableItem*               getCompareItem (QTable*);
  QTableItem*               getUnaryItem (QTable*);
  QTableItem*               getBinaryItem (QTable*);
  const QMap<int, QString>& getCriteria (const int theType) const;
  const QMap<int, QString>& getCompare() const;
  Table*                    createTable (QWidget*, const int);
  QWidget*                  createAdditionalFrame (QWidget* theParent);
  int                       getFirstSelectedRow() const;
  void                      onCriterionChanged (const int, const int, const int);

  void                      updateBtnState();
  void                      removeAdditionalWidget (QTable* theTable, const int theRow);
  void                      updateAdditionalWidget();

  const QMap<int, QString>& getSupportedTypes() const;

private:
  SMESHGUI*                 mySMESHGUI;

  QGroupBox*                myTableGrp;
  QGroupBox*                mySwitchTableGrp;

  TableMap                  myTables;
  QPushButton*              myAddBtn;
  QPushButton*              myInsertBtn;
  QPushButton*              myRemoveBtn;
  QPushButton*              myClearBtn;  
  QPushButton*              myCopyFromBtn;  
  QPushButton*              myAddToBtn;

  QButtonGroup*             myEntityTypeGrp;
  int                       myEntityType;
  int                       myIsValid;
  bool                      myIsLocked;

  SMESHGUI_FilterLibraryDlg* myLibDlg;

  QWidgetStack*              myWgStack;

  QMap<QTableItem*, AdditionalWidget*> myAddWidgets;
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
  enum { BTN_OK, BTN_Cancel, BTN_Apply, BTN_Close };

public:
                            SMESHGUI_FilterDlg( SMESHGUI*,
                                                const QValueList<int>& types,
                                                const char*            name = 0);

                            SMESHGUI_FilterDlg( SMESHGUI*,
                                                const int              type,
                                                const char*            name = 0);
  virtual                   ~SMESHGUI_FilterDlg();

  void                      Init (const QValueList<int>& types);
  void                      Init (const int type);

  void                      SetSelection();
  void                      SetMesh (SMESH::SMESH_Mesh_ptr);
  void                      SetSourceWg (QWidget*);

  static SMESH::Filter::Criterion createCriterion();

signals:

  void                      Accepted();

private slots:

  void                      onOk();
  bool                      onApply();
  void                      onClose();
  void                      onDeactivate();
  void                      onSelectionDone();
  void                      onCriterionChanged (const int, const int);
  void                      onCurrentChanged (int, int);

private:

  void                      construct (const QValueList<int>& types); 

  void                      closeEvent (QCloseEvent*);
  void                      enterEvent (QEvent*);

  // dialog creation
  QFrame*                   createButtonFrame (QWidget*);
  QFrame*                   createMainFrame (QWidget*);
  QButtonGroup*             createSourceGroup (QWidget*);
  void                      updateMainButtons();

  // execution
  bool                      isValid() const;
  bool                      createFilter (const int theType);
  void                      insertFilterInViewer();
  void                      selectInViewer (const int              theType,
                                            const QValueList<int>& theIds);
  void                      filterSource (const int        theType,
                                          QValueList<int>& theResIds);
  void                      filterSelectionSource (const int        theType,
                                                   QValueList<int>& theResIds);
  void                      getIdsFromWg (const QWidget*, QValueList<int>&) const;
  void                      setIdsToWg (QWidget*, const QValueList<int>&);
  Selection_Mode            getSelMode (const int) const;
  void                      updateSelection();

private:

  // widgets
  QFrame*                   myMainFrame;
  QButtonGroup*             mySourceGrp;

  QCheckBox*                mySetInViewer;

  QMap<int, QPushButton*>   myButtons;
  
  SMESHGUI_FilterTable*     myTable;

  // initial fields
  QValueList<int>           myTypes;
  SMESHGUI*                 mySMESHGUI;
  LightApp_SelectionMgr*    mySelectionMgr;
  SVTK_Selector*            mySelector;
  SMESH::SMESH_Mesh_ptr     myMesh;
  QWidget*                  mySourceWg;

  SALOME_DataMapOfIOMapOfInteger myIObjects;
  bool                           myIsSelectionChanged;
  QMap< int, SMESH::Filter_var > myFilter;
  QMap< int, bool >              myInsertState;
  QMap< int, int  >              myApplyToState;
};

#endif
