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

// File   : SMESHGUI_MultiEditDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MULTIEDITDLG_H
#define SMESHGUI_MULTIEDITDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

// Qt includes 
#include <QDialog>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

class SMESHGUI;
class SMESHGUI_FilterDlg;
class SMESHGUI_SpinBox;

class SMESH_Actor;

class LightApp_SelectionMgr;
class SALOME_Actor;
class SVTK_Selector;

class QListWidget;
class QComboBox;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QButtonGroup;

/*!
 * Class       : SMESHGUI_MultiEditDlg
 * Description : Base class for dialogs of diagonal inversion and
 *               union of two neighboring triangles
 */

class SMESHGUI_EXPORT SMESHGUI_MultiEditDlg : public SMESHGUI_PreviewDlg
{
  Q_OBJECT

public:
  SMESHGUI_MultiEditDlg( SMESHGUI*  theModule,
                         const int  theMode,
                         const bool the3d2d = false,
                         bool       theDoInit = true );
  virtual ~SMESHGUI_MultiEditDlg();

  void                      Init();

  bool                      eventFilter( QObject*, QEvent* );

signals:
  void                      ListContensChanged();

protected slots:
  void                      onOk();
  virtual bool              onApply();
  virtual void              reject();
  void                      onHelp();

  void                      onDeactivate();
  virtual void              onSelectionDone();

  void                      onFilterBtn();
  void                      onAddBtn();
  void                      onRemoveBtn();
  void                      onSortListBtn();
  void                      onListSelectionChanged();
  void                      onSubmeshChk();
  void                      onGroupChk();
  virtual void              onToAllChk();
  void                      onFilterAccepted();
  virtual void              on3d2dChanged(int);
  void                      onOpenView();
  void                      onCloseView();

  SMESH::NumericalFunctor_ptr getNumericalFunctor();

protected:
  void                      enterEvent( QEvent * );
  void                      keyPressEvent( QKeyEvent* );
  QWidget*                  createButtonFrame( QWidget* );
  QWidget*                  createMainFrame( QWidget*, const bool );
  virtual bool              isValid( const bool );
  SMESH::long_array_var     getIds(SMESH::SMESH_IDSource_var& obj);
  void                      updateButtons();
  virtual void              setSelectionMode();
  virtual bool              isIdValid( const int ) const;
  virtual bool              process( SMESH::SMESH_MeshEditor_ptr, 
                                     const SMESH::long_array& ,
                                     SMESH::SMESH_IDSource_ptr obj) = 0;
  virtual int               nbElemsInMesh() = 0;
  int                       entityType();

protected:
  QPushButton*              myOkBtn;
  QPushButton*              myApplyBtn;
  QPushButton*              myCloseBtn;
  QPushButton*              myHelpBtn;
  SMESH_Actor*              myActor;
  SMESH::SMESH_Mesh_var     myMesh;

  LightApp_SelectionMgr*    mySelectionMgr;
  SVTK_Selector*            mySelector;
  SMESHGUI*                 mySMESHGUI;

  QGroupBox*                mySelGrp;
  QGroupBox*                myCriterionGrp;

  QWidget*                  myChoiceWidget;
  QButtonGroup*             myGroupChoice;
  QComboBox*                myComboBoxFunctor;

  QListWidget*              myListBox;
  QPushButton*              myFilterBtn;
  QPushButton*              myAddBtn;
  QPushButton*              myRemoveBtn;
  QPushButton*              mySortBtn;

  QCheckBox*                myToAllChk;
  QButtonGroup*             myEntityTypeGrp;

  QCheckBox*                mySubmeshChk;
  QPushButton*              mySubmeshBtn;
  QLineEdit*                mySubmesh;

  QCheckBox*                myGroupChk;
  QPushButton*              myGroupBtn;
  QLineEdit*                myGroup;

  SMESHGUI_FilterDlg*       myFilterDlg;
  TColStd_MapOfInteger      myIds;
  int                       myFilterType;
  bool                      myBusy;
  int                       myEntityType;

  QString                   myHelpFileName;
};

/*!
 * Class       : SMESHGUI_ChangeOrientationDlg
 * Description : Modification of orientation of faces
 */
class  SMESHGUI_ChangeOrientationDlg : public SMESHGUI_MultiEditDlg
{
  Q_OBJECT

public:
  SMESHGUI_ChangeOrientationDlg( SMESHGUI* );
  virtual ~SMESHGUI_ChangeOrientationDlg();

protected:
  virtual bool process( SMESH::SMESH_MeshEditor_ptr,
                        const SMESH::long_array& ,
                        SMESH::SMESH_IDSource_ptr obj);
  virtual int  nbElemsInMesh();
};

/*!
 * Class       : SMESHGUI_UnionOfTrianglesDlg
 * Description : Construction of quadrangles by automatic association of triangles
 */
class  SMESHGUI_UnionOfTrianglesDlg : public SMESHGUI_MultiEditDlg
{
  Q_OBJECT

public:
  SMESHGUI_UnionOfTrianglesDlg( SMESHGUI* );
  virtual ~SMESHGUI_UnionOfTrianglesDlg();

protected:
  virtual bool      isValid( const bool );
  virtual bool      process( SMESH::SMESH_MeshEditor_ptr,
                             const SMESH::long_array&,
                             SMESH::SMESH_IDSource_ptr obj );
  virtual int       nbElemsInMesh();

protected slots:
   virtual void     onDisplaySimulation( bool );

private:
  SMESHGUI_SpinBox* myMaxAngleSpin;
};

/*
  Class       : SMESHGUI_CuttingOfQuadsDlg
  Description : Construction of quadrangles by automatic association of triangles
*/
class  SMESHGUI_CuttingOfQuadsDlg : public SMESHGUI_MultiEditDlg
{
  Q_OBJECT

public:
  SMESHGUI_CuttingOfQuadsDlg( SMESHGUI* );
  virtual ~SMESHGUI_CuttingOfQuadsDlg();

protected:
  virtual bool  process( SMESH::SMESH_MeshEditor_ptr,
                         const SMESH::long_array& ,
                         SMESH::SMESH_IDSource_ptr obj);
  virtual int   nbElemsInMesh();

protected slots:
  virtual void  reject();
  void          onCriterionRB();
  void          onPreviewChk();

private:
  void          displayPreview();
  void          erasePreview();

private:
  SALOME_Actor* myPreviewActor;
  QCheckBox*    myPreviewChk;
};

/*!
 * Class       : SMESHGUI_SplitVolumesDlg
 * Description : Split all volumes into tetrahedrons
 */
class  SMESHGUI_SplitVolumesDlg : public SMESHGUI_MultiEditDlg
{
  Q_OBJECT

public:
  SMESHGUI_SplitVolumesDlg( SMESHGUI* );
  virtual ~SMESHGUI_SplitVolumesDlg();

protected slots:

  virtual void on3d2dChanged(int);
  virtual void onSelectionDone();

  void         onFacetSelection(bool);
  void         onSetDir();
  void         updateNormalPreview(const QString& s="");

protected:

  virtual bool process( SMESH::SMESH_MeshEditor_ptr,
                        const SMESH::long_array&,
                        SMESH::SMESH_IDSource_ptr obj );
  virtual int  nbElemsInMesh();

  virtual void setSelectionMode();
  void         showFacetByElement( int id );
  bool         isIntoPrisms();

  QGroupBox*        myFacetSelGrp;
  SMESHGUI_SpinBox* myPointSpin[3];
  SMESHGUI_SpinBox* myDirSpin  [3];
  QPushButton*      myFacetSelBtn;
  QPushButton*      myAxisBtn[3];
  QCheckBox*        myAllDomainsChk;

  double            myCellSize;
};

#endif // SMESHGUI_MULTIEDITDLG_H
