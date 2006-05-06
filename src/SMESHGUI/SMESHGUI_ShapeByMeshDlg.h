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

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

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

class SMESHGUI_ShapeByMeshDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
                           SMESHGUI_ShapeByMeshDlg();
  virtual                  ~SMESHGUI_ShapeByMeshDlg();

private:

//   void                     closeEvent (QCloseEvent* e);
//   void                     enterEvent (QEvent*);

private:

  //QFrame*                  createButtonFrame (QWidget*);
  QFrame*                  createMainFrame   (QWidget*);
  //void                     displayPreview();
  //void                     erasePreview();
private:

  QButtonGroup*            myElemTypeGroup;
  QLineEdit*               myElementId;
  QLineEdit*               myGeomName;

//   QPushButton*             myOkBtn;
//   QPushButton*             myCloseBtn;

//   SMESHGUI*                mySMESHGUI;
//   LightApp_SelectionMgr*   mySelectionMgr;
//   SVTK_ViewWindow*         myViewWindow;

  friend class SMESHGUI_ShapeByMeshOp;
};

class SMESHGUI_ShapeByMeshOp: public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_ShapeByMeshOp();
  virtual ~SMESHGUI_ShapeByMeshOp();

  virtual LightApp_Dialog*       dlg() const;  

  void                     Init();
  void                     SetMesh (SMESH::SMESH_Mesh_ptr);
  SMESH::SMESH_Mesh_ptr    GetMesh () { return myMesh; }
  GEOM::GEOM_Object_ptr    GetShape();

protected:

  virtual void                   commitOperation();
  virtual void                   startOperation();
  //virtual void                   selectionDone();
  //virtual SUIT_SelectionFilter*  createFilter( const int ) const;
  //virtual bool                   isValid( SUIT_Operation* ) const;

  void                     activateSelection();
  void                     setElementID(const QString&);

/* signals: */

/*   void                     PublishShape(); */
/*   void                     Close(); */

protected slots:

  virtual bool                   onApply() { return true; }
/*   void                           onCreateHyp( const int theHypType, const int theIndex ); */
/*   void                           onEditHyp( const int theHypType, const int theIndex ); */
/*   void                           onHypoSet( const QString& theSetName ); */
/*   void                           onGeomSelectionByMesh( bool ); */
/*   void                           onPublishShapeByMeshDlg(); */
/*   void                           onCloseShapeByMeshDlg(); */

private slots:

//   void                     onOk();
//   void                     onClose();

//   void                     onDeactivate();

  void                     onSelectionDone();
  void                     onTypeChanged (int);
  void                     onElemIdChanged (const QString&);

private:

  SMESHGUI_ShapeByMeshDlg* myDlg;
  SMESH::SMESH_Mesh_var    myMesh;
  GEOM::GEOM_Object_var    myGeomObj;

  bool                     myIsManualIdEnter;
  bool                     myHasSolids;
};

#endif
