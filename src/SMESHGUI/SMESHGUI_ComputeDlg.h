//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
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
#include <QPointer>
#include <QGroupBox>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QFrame;
class QPushButton;
class QTableWidget;
class QLabel;
class QtxComboBox;
class SMESHGUI_ComputeDlg;
class SMESHGUI_MeshInfosBox;
class SMESHGUI_PrecomputeDlg;
class SMESHGUI_MeshEditPreview;

class SMESH::compute_error_array;

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
    
protected slots:
  virtual bool                   onApply();
  void                           onPreviewShape();
  void                           onPublishShape();
  void                           onShowBadMesh();
  void                           currentCellChanged();

private:
  QTableWidget*                  table();

private:
  QPointer<SMESHGUI_ComputeDlg>  myCompDlg;

protected:
  SMESH::SMESH_Mesh_var            myMesh;
  GEOM::GEOM_Object_var            myMainShape;
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
  QMap< int, int >               myMapShapeId;
  QPointer<LightApp_Dialog>      myActiveDlg;
  QPointer<SMESHGUI_PrecomputeDlg> myDlg;
  SMESHGUI_MeshEditPreview*      myPreviewDisplayer;
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
  QTableWidget*                myTable;
  QPushButton*                 myShowBtn;
  QPushButton*                 myPublishBtn;
  QPushButton*                 myBadMeshBtn;

  SMESHGUI_MeshInfosBox*       myBriefInfo;
  SMESHGUI_MeshInfosBox*       myFullInfo;

  friend class SMESHGUI_BaseComputeOp;
  friend class SMESHGUI_PrecomputeOp;
};

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

signals:
  void                         preview();

private:
  QPushButton*                 myPreviewBtn;
  QtxComboBox*                 myPreviewMode;
};

#endif // SMESHGUI_COMPUTEDLG_H
