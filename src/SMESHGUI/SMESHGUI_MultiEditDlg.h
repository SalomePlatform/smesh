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
//  File   : SMESHGUI_MultiEditDlg.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_MultiEditDlg_H
#define SMESHGUI_MultiEditDlg_H

#include <qdialog.h>
#include "SMESH_TypeFilter.hxx"
#include <TColStd_MapOfInteger.hxx>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QCloseEvent;
class SMESHGUI_FilterDlg;
class QCheckBox;
class QGroupBox;
class QListBox;
class QFrame;
class QLineEdit;
class SMESHGUI_SpinBox;
class QPushButton;
class SALOME_Selection;
class SMESH_Actor;
class SALOME_Actor;

/*
  Class       : SMESHGUI_MultiEditDlg
  Description : Base class for dialogs of diagonal inversion and 
                union of two neighboring triangles
*/

class SMESHGUI_MultiEditDlg : public QDialog
{ 
  Q_OBJECT

public:
                            SMESHGUI_MultiEditDlg( QWidget*,
                                                   SALOME_Selection*,
                                                   const int,
                                                   const char* = 0 );
  virtual                   ~SMESHGUI_MultiEditDlg();

  void                      Init( SALOME_Selection* ) ;

signals:
  void                      ListContensChanged();

protected slots:

  void                      onOk();
  virtual bool              onApply();
  virtual void              onClose();

  void                      onDeactivate();
  void                      onSelectionDone();
  
  void                      onFilterBtn();
  void                      onAddBtn();
  void                      onRemoveBtn();
  void                      onSortListBtn();
  void                      onListSelectionChanged();
  void                      onSubmeshChk();
  void                      onGroupChk();
  virtual void              onToAllChk();
  void                      onFilterAccepted();
  
protected:

  void                      closeEvent( QCloseEvent* e ) ;
  void                      enterEvent ( QEvent * ) ;            
  void                      hideEvent ( QHideEvent * );                        /* ESC key */
  QFrame*                   createButtonFrame( QWidget* );
  QFrame*                   createMainFrame  ( QWidget* );
  bool                      isValid( const bool ) const;
  SMESH::long_array_var     getIds();
  void                      updateButtons();
  void                      setSelectionMode();
  virtual bool              isIdValid( const int theID ) const;
  virtual bool              process( SMESH::SMESH_MeshEditor_ptr, const SMESH::long_array& ) = 0;
  
protected:

  QPushButton*              myOkBtn;
  QPushButton*              myApplyBtn;
  QPushButton*              myCloseBtn;
  SALOME_Selection*         mySelection;
  SMESH::SMESH_Mesh_var     myMesh;
  SMESH_Actor*              myActor;
  
  QGroupBox*                mySelGrp;
  
  QListBox*                 myListBox;
  QPushButton*              myFilterBtn;
  QPushButton*              myAddBtn;
  QPushButton*              myRemoveBtn;
  QPushButton*              mySortBtn;
  
  QCheckBox*                myToAllChk;
  
  QCheckBox*                mySubmeshChk;
  QPushButton*              mySubmeshBtn;
  QLineEdit*                mySubmesh;
  
  QCheckBox*                myGroupChk;
  QPushButton*              myGroupBtn;
  QLineEdit*                myGroup;
  
  SMESHGUI_FilterDlg*       myFilterDlg;
  TColStd_MapOfInteger      myIds;
  int                       myFilterType;
  Handle(SMESH_TypeFilter)  mySubmeshFilter;
  Handle(SMESH_TypeFilter)  myGroupFilter;
  bool                      myBusy;
};

/*
  Class       : SMESHGUI_ChangeOrientationDlg
  Description : Modification of orientation of faces
*/
class  SMESHGUI_ChangeOrientationDlg : public SMESHGUI_MultiEditDlg
{ 
  Q_OBJECT

public:
                            SMESHGUI_ChangeOrientationDlg( QWidget*,
                                                           SALOME_Selection*,
                                                           const char* = 0 );
  virtual                   ~SMESHGUI_ChangeOrientationDlg();

protected:

  virtual bool              process( SMESH::SMESH_MeshEditor_ptr, const SMESH::long_array& );
};

/*
  Class       : SMESHGUI_UnionOfTrianglesDlg
  Description : Construction of quadrangles by automatic association of triangles
*/
class  SMESHGUI_UnionOfTrianglesDlg : public SMESHGUI_MultiEditDlg
{
  Q_OBJECT

public:
                            SMESHGUI_UnionOfTrianglesDlg( QWidget*,
                                                          SALOME_Selection*,
                                                          const char* = 0 );
  virtual                   ~SMESHGUI_UnionOfTrianglesDlg();

protected:

  virtual bool              process( SMESH::SMESH_MeshEditor_ptr, const SMESH::long_array& );
};

/*
  Class       : SMESHGUI_CuttingOfQuadsDlg
  Description : Construction of quadrangles by automatic association of triangles
*/
class  SMESHGUI_CuttingOfQuadsDlg : public SMESHGUI_MultiEditDlg
{
  Q_OBJECT

public:
                            SMESHGUI_CuttingOfQuadsDlg( QWidget*,
                                                        SALOME_Selection*,
                                                        const char* = 0 );
  virtual                   ~SMESHGUI_CuttingOfQuadsDlg();

protected:
  virtual bool              process( SMESH::SMESH_MeshEditor_ptr, const SMESH::long_array& );
  
protected slots:
  virtual void              onClose();
  void                      onPreviewChk();

private:
  void                      displayPreview();
  void                      erasePreview();
  
private:
  QCheckBox*                myUseDiagChk;
  SALOME_Actor*             myPreviewActor;
  QCheckBox*                myPreviewChk;
};

#endif
