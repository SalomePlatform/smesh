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

// File   : SMESHGUI_ShapeByMeshDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SHAPEBYMESHDLG_H
#define SMESHGUI_SHAPEBYMESHDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QFrame;
class QLineEdit;
class QButtonGroup;
class QGroupBox;

/*!
 * \brief Dialog to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */

class SMESHGUI_EXPORT SMESHGUI_ShapeByMeshDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_ShapeByMeshDlg(bool isMultipleAllowed);
  virtual ~SMESHGUI_ShapeByMeshDlg();

private:
  QFrame*                  createMainFrame( QWidget* );

  QGroupBox*               myElemTypeBox;
  QButtonGroup*            myElemTypeGroup;
  QLineEdit*               myElementId;
  QLineEdit*               myGeomName;

  bool                     myIsMultipleAllowed;
  void                     setMultipleAllowed( bool );

  friend class SMESHGUI_ShapeByMeshOp;
};

/*!
 * \brief Operation to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */
class SMESHGUI_ShapeByMeshOp: public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_ShapeByMeshOp( bool = false );
  virtual ~SMESHGUI_ShapeByMeshOp();

  virtual LightApp_Dialog* dlg() const;  

  void                     Init();
  void                     SetMesh( SMESH::SMESH_Mesh_ptr );
  SMESH::SMESH_Mesh_ptr    GetMesh();
  GEOM::GEOM_Object_ptr    GetShape();

protected:
  virtual void             commitOperation();
  virtual void             startOperation();

  void                     activateSelection();
  void                     setElementID( const QString& );

protected slots:
  virtual bool             onApply();

private slots:
  void                     onSelectionDone();
  void                     onTypeChanged( int );
  void                     onElemIdChanged( const QString& );

private:
  SMESHGUI_ShapeByMeshDlg* myDlg;
  SMESH::SMESH_Mesh_var    myMesh;
  GEOM::GEOM_Object_var    myGeomObj;

  bool                     myIsManualIdEnter;
  bool                     myHasSolids;
  bool                     myIsMultipleAllowed;
};

#endif
