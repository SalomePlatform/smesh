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
#include <qmap.h>
#include <qvaluelist.h>
#include <SALOME_Selection.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QCloseEvent;
class QWidgetStack;
class QStringList;
class QTableItem;
class QFrame;
class QEvent;
class QPushButton;
class QTable;
class SALOME_Selection;
class SMESHGUI;
class QComboBox;
class QButtonGroup;
class QCheckBox;
class QGroupBox;


/*
  Class       : SMESHGUI_FilterDlg
  Description : Dialog to specify filters for VTK viewer
*/

class SMESHGUI_FilterDlg : public QDialog
{ 
  Q_OBJECT

  class Table;

  // Edge criteria
  enum { FreeBorders = 0, MultiBorders, Length }; 

  // Face criteria
  enum { AspectRatio = 0, Warping, MinimumAngle, Taper, Skew, Area }; 

  // Comparision
  enum { LessThan = 0, MoreThan, EqualTo };

  // Logical operations
  enum { LO_And = 0, LO_Or, LO_Undefined };

  // Source elements to be selected
  enum { Mesh, Selection, Dialog, None };

  typedef QMap<int, Table*> TableMap;

public:
                            SMESHGUI_FilterDlg( QWidget*    parent, 
                                                const int   type,
                                                const bool  theModal = true,
                                                const char* name = 0 );
  virtual                   ~SMESHGUI_FilterDlg();

  void                      Init( const int type );

  void                      SetSelection( SALOME_Selection* );
  void                      SetMesh( SMESH::SMESH_Mesh_ptr );
  void                      SetSourceWg( QWidget* );

private slots:

  void                      onAddBtn();
  void                      onRemoveBtn();
  void                      onClearBtn();
  void                      onCriterionChanged( int, int );

  void                      onOk();
  bool                      onApply();
  void                      onClose();
  void                      onDeactivate();

private:

  void                      closeEvent( QCloseEvent* e );
  void                      enterEvent ( QEvent * );

  void                      updateBtnState();

  // dialog creation
  QFrame*                   createButtonFrame( QWidget*, const bool );
  QFrame*                   createMainFrame( QWidget* );
  Table*                    createTable( QWidget*, const int );
  QButtonGroup*             createSourceGroup( QWidget* );
  void                      addRow( Table*, const int );
  QTableItem*               getCriterionItem( QTable*, const int );
  QTableItem*               getCompareItem( QTable* );
  QTableItem*               getNotItem( QTable* );
  QTableItem*               getLogOpItem( QTable* );
  const QStringList&        getCriteria( const int theType ) const;
  const QStringList&        getCompare () const;
  
  // query
  int                       getCurrType() const;
  int                       getCriterion( const int theType, const int theRow ) const;

  // execution
  bool                      isValid() const;
  SMESH::Predicate_ptr      createPredicate( const int theType );
  void                      insertFilterInViewer( SMESH::Predicate_ptr thePred );
  void                      selectInViewer( const int theType,
                                            const QValueList<int>& theIds );
  void                      filterSource( const int            theType,
                                          SMESH::Predicate_ptr thePred,
                                          QValueList<int>&     theResIds );
  void                      filterSelectionSource( const int            theType,
                                                   SMESH::Predicate_ptr thePred,
                                                   QValueList<int>&     theResIds );
  void                      getIdsFromWg( const QWidget*, QValueList<int>& ) const;
  void                      setIdsToWg( QWidget*, const QValueList<int>& );
  Selection_Mode            getSelMode( const int ) const;

private:

  // widgets
  QFrame*                   myMainFrame;
  QGroupBox*                myTableGrp;

  TableMap                  myTables;
  QPushButton*              myAddBtn;
  QPushButton*              myRemoveBtn;
  QPushButton*              myClearBtn;
  QButtonGroup*             mySourceGrp;
  QCheckBox*                mySetInViewer;

  QPushButton*              myOkBtn;
  QPushButton*              myApplyBtn;
  QPushButton*              myCloseBtn;

  // initial fields
  int                       myType;
  SMESHGUI*                 mySMESHGUI;
  SALOME_Selection*         mySelection;
  SMESH::SMESH_Mesh_ptr     myMesh;
  QWidget*                  mySourceWg;
};

#endif




















