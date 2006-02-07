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
//  File   : SMESHGUI_ShapeByMeshDlg.h
//  Author : Edward AGAPOV
//  Module : SMESH


#ifndef SMESHGUI_ShapeByMeshDlg_H
#define SMESHGUI_ShapeByMeshDlg_H

#include <qdialog.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QCloseEvent;
class QFrame;
class QLineEdit;
class QPushButton;
class LightApp_SelectionMgr;
class SVTK_ViewWindow;
class QButtonGroup;
class SMESHGUI;

/*!
 * \brief Dialog to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */

class SMESHGUI_ShapeByMeshDlg : public QDialog
{
  Q_OBJECT

public:
                           SMESHGUI_ShapeByMeshDlg( SMESHGUI*   theModule,
                                                    const char* theName = 0);
  virtual                  ~SMESHGUI_ShapeByMeshDlg();

  void                     Init();
  void                     SetMesh (SMESH::SMESH_Mesh_ptr);
  SMESH::SMESH_Mesh_ptr    GetMesh () { return myMesh; }
  GEOM::GEOM_Object_ptr    GetShape();

signals:

  void                     PublishShape();
  void                     Close();

private:

  void                     closeEvent (QCloseEvent* e);
  void                     enterEvent (QEvent*);

private slots:

  void                     onOk();
  void                     onClose();

  void                     onDeactivate();

  void                     onSelectionDone();
  void                     onTypeChanged (int);
  void                     onElemIdChanged (const QString&);

private:

  QFrame*                  createButtonFrame (QWidget*);
  QFrame*                  createMainFrame   (QWidget*);
  //void                     displayPreview();
  //void                     erasePreview();
  void                     activateSelection();
  void                     setElementID(const QString&);

private:

  QButtonGroup*            myElemTypeGroup;
  QLineEdit*               myElementId;
  QLineEdit*               myGeomName;

  QPushButton*             myOkBtn;
  QPushButton*             myCloseBtn;

  SMESHGUI*                mySMESHGUI;
  LightApp_SelectionMgr*   mySelectionMgr;
  SVTK_ViewWindow*         myViewWindow;

  SMESH::SMESH_Mesh_var    myMesh;
  GEOM::GEOM_Object_var    myGeomObj;

  bool                     myIsManualIdEnter;
  bool                     myHasSolids;
};

#endif
