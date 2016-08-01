// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_ComputeDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_COMPUTEDLG_H
#define SMESHGUI_COMPUTEDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_Operation.h"

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>

// Qt includes
#include <QMap>
#include <QList>
#include <QPointer>
#include <QGroupBox>
#include <QThread>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QFrame;
class QPushButton;
class QTableWidget;
class QLabel;
class QtxComboBox;
class QProgressBar;
class SMESHGUI_ComputeDlg;
class SMESHGUI_MeshInfosBox;
class SMESHGUI_PrecomputeDlg;
class SMESHGUI_MeshEditPreview;

namespace SMESH
{
  class TShapeDisplayer;
}

/*!
 * \brief Base operation to compute a mesh and show computation errors
 */
class SMESHGUI_EXPORT SMESHGUI_BaseComputeOp: public SMESHGUI_Operation
{
  Q_OBJECT

public:
  SMESHGUI_BaseComputeOp();
  virtual ~SMESHGUI_BaseComputeOp();

  SMESH::SMESH_Mesh_ptr          getMesh();

protected:
  virtual void                   startOperation();
  virtual void                   stopOperation();

  SMESHGUI_ComputeDlg*           computeDlg() const;
  void                           computeMesh();
  void                           showComputeResult( const bool,
                                                    const bool,
                                                    SMESH::compute_error_array_var&,
                                                    const bool,
                                                    const QString& );
  SMESHGUI_ComputeDlg*           evaluateDlg() const;
  void                           evaluateMesh();
  void                           showEvaluateResult(const SMESH::long_array& theRes,
                                                    const bool,
                                                    const bool,
                                                    SMESH::compute_error_array_var&,
                                                    const bool,
                                                    const QString&);

  virtual bool                   isValid( SUIT_Operation* theOp ) const;
    
protected slots:
  virtual bool                   onApply();
  void                           onPreviewShape();
  void                           onPublishShape();
  void                           onShowBadMesh();
  void                           onGroupOfBadMesh();
  void                           currentCellChanged();

private:
  QTableWidget*                  table();

private:
  QPointer<SMESHGUI_ComputeDlg>  myCompDlg;

protected:
  SMESH::SMESH_Mesh_var            myMesh;
  GEOM::GEOM_Object_var            myMainShape;
  GEOM::GEOM_Object_var            myCurShape;
  SMESH::TShapeDisplayer*          myTShapeDisplayer;
  SMESHGUI_MeshEditPreview*        myBadMeshDisplayer;
  Handle(SALOME_InteractiveObject) myIObject;
};

/*!
 * \brief Operation to compute a mesh and show computation errors
 */
class SMESHGUI_EXPORT SMESHGUI_ComputeOp: public SMESHGUI_BaseComputeOp
{
  Q_OBJECT

public:
  SMESHGUI_ComputeOp();
  virtual ~SMESHGUI_ComputeOp();

  virtual LightApp_Dialog*       dlg() const;

protected:
  virtual void                   startOperation();

protected slots:
};

class SMESHGUI_MeshOrderMgr;

/*!
 * \brief Operation to preview and compute a mesh and show computation errors
 */
class SMESHGUI_EXPORT SMESHGUI_PrecomputeOp: public SMESHGUI_BaseComputeOp
{
  Q_OBJECT

public:
  SMESHGUI_PrecomputeOp();
  virtual ~SMESHGUI_PrecomputeOp();

  virtual LightApp_Dialog*       dlg() const;

  /**
   * \brief returns map of assigned algorithms modes
   */
  static void                    getAssignedAlgos(_PTR(SObject) theMesh,
                                                  QMap<int,int>& theModeMap);

protected:
  virtual void                   startOperation();
  virtual void                   stopOperation();
  virtual void                   resumeOperation();

  virtual void                   initDialog();

protected slots:
  virtual void                   onCancel();

private slots:
  void                           onPreview();
  void                           onCompute();

private:
  //! private fields
  QMap< int, int >               myMapShapeId;
  QPointer<LightApp_Dialog>      myActiveDlg;
  QPointer<SMESHGUI_PrecomputeDlg> myDlg;
  SMESHGUI_MeshEditPreview*      myPreviewDisplayer;
  //! fields for mesh order
  typedef QList<int>             TListOfInt;
  typedef QList<TListOfInt>      TListOfListOfInt;
  TListOfListOfInt               myPrevOrder;
  SMESHGUI_MeshOrderMgr*         myOrderMgr;
};

/*!
 * \brief Operation to evaluate a mesh and show result
 */
class SMESHGUI_EXPORT SMESHGUI_EvaluateOp: public SMESHGUI_BaseComputeOp
{
  Q_OBJECT

public:
  SMESHGUI_EvaluateOp();
  virtual ~SMESHGUI_EvaluateOp();

  virtual LightApp_Dialog*       dlg() const;

protected:
  virtual void                   startOperation();

protected slots:
};

/*!
 * \brief Dialog to compute a mesh and show computation errors
 */

class SMESHGUI_EXPORT SMESHGUI_ComputeDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_ComputeDlg( QWidget*, bool );
  virtual ~SMESHGUI_ComputeDlg();

protected:
  QFrame*                      createMainFrame( QWidget*, bool );

  QLabel*                      myMeshName;
  QGroupBox*                   myMemoryLackGroup;
  QGroupBox*                   myCompErrorGroup;
  QGroupBox*                   myHypErrorGroup;
  QLabel*                      myHypErrorLabel;
  QLabel*                      myWarningLabel;
  QTableWidget*                myTable;
  QPushButton*                 myShowBtn;
  QPushButton*                 myPublishBtn;
  QPushButton*                 myBadMeshBtn;
  QPushButton*                 myBadMeshToGroupBtn;

  SMESHGUI_MeshInfosBox*       myBriefInfo;
  SMESHGUI_MeshInfosBox*       myFullInfo;

  friend class SMESHGUI_BaseComputeOp;
  friend class SMESHGUI_PrecomputeOp;
};

class SMESHGUI_MeshOrderBox;

/*!
 * \brief Dialog to preview and compute a mesh and show computation errors
 */

class SMESHGUI_EXPORT SMESHGUI_PrecomputeDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_PrecomputeDlg( QWidget* );
  virtual ~SMESHGUI_PrecomputeDlg();
  
  void                         setPreviewModes( const QList<int>& );
  int                          getPreviewMode() const;
  
  SMESHGUI_MeshOrderBox*       getMeshOrderBox() const;

signals:
  void                         preview();

private:
  SMESHGUI_MeshOrderBox*       myOrderBox;
  QPushButton*                 myPreviewBtn;
  QtxComboBox*                 myPreviewMode;
};

/*!
 * \brief Thread to launch computation
 */

class SMESHGUI_EXPORT SMESHGUI_ComputeDlg_QThread : public QThread
{
  Q_OBJECT
    
public:
  SMESHGUI_ComputeDlg_QThread(SMESH::SMESH_Gen_var  gen,
                              SMESH::SMESH_Mesh_var mesh,
                              GEOM::GEOM_Object_var mainShape);
  bool                   result();
  void                   cancel();
  SMESH::SMESH_Mesh_var& getMesh() { return myMesh; }

protected:
  void run();
  
private:
  SMESH::SMESH_Gen_var  myGen;
  SMESH::SMESH_Mesh_var myMesh;
  GEOM::GEOM_Object_var myMainShape;
  bool                  myResult;
};

/*!
 * \brief Dialog to display Cancel button
 */

class SMESHGUI_EXPORT SMESHGUI_ComputeDlg_QThreadQDialog : public QDialog
{
  Q_OBJECT
    
public:
  SMESHGUI_ComputeDlg_QThreadQDialog(QWidget *             parent,
                                     SMESH::SMESH_Gen_var  gen,
                                     SMESH::SMESH_Mesh_var mesh,
                                     GEOM::GEOM_Object_var mainShape);
  bool result();
  
protected:
  void timerEvent(QTimerEvent *timer);
  void closeEvent(QCloseEvent *event);
  
private slots:
  void onCancel();
  
private:
  SMESHGUI_ComputeDlg_QThread qthread;
  QPushButton *               cancelButton;
  QLabel *                    nbNodesLabel;
  QLabel *                    nbElemsLabel;
  QLabel *                    freeRAMLabel;
  QProgressBar*               progressBar;
};

#endif // SMESHGUI_COMPUTEDLG_H
