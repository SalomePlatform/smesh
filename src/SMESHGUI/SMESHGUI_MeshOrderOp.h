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

// File   : SMESHGUI_MeshOrderOp.h
// Author : Pavel TELKOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MeshOrderOp_H
#define SMESHGUI_MeshOrderOp_H


// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Operation.h"
#include "SMESHGUI_MeshOrderDlg.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

/*!
 * \brief Operator to check and modify mesh computation submesh priority (order)
 */
class SMESHGUI_EXPORT SMESHGUI_MeshOrderMgr
{
public:
  SMESHGUI_MeshOrderMgr( SMESHGUI_MeshOrderBox* );
  virtual ~SMESHGUI_MeshOrderMgr();
  //! Set root mesh object
  void                   SetMesh( SMESH::SMESH_Mesh_var& theMesh );
  //! Check for concurents between submesh objects
  bool                   GetMeshOrder();
  //! Check for concurents between submesh objects
  bool                   GetMeshOrder( ListListId& theIds );
  //! Store submesh priority order
  bool                   SetMeshOrder();
  //! Store given submesh priority order
  bool                   SetMeshOrder( const  ListListId& theIds );

  //! Returns status is order changed by user
  bool                   IsOrderChanged() const;

private:
  SMESH::SMESH_Mesh_var  myMesh;
  SMESHGUI_MeshOrderBox* myBox;
};

/*!
 * \brief Operator to check and modify mesh computation submesh priority (order)
 */
class SMESHGUI_EXPORT SMESHGUI_MeshOrderOp: public SMESHGUI_Operation
{
  Q_OBJECT

public:
  SMESHGUI_MeshOrderOp();
  virtual ~SMESHGUI_MeshOrderOp();

protected:
  virtual LightApp_Dialog* dlg() const;

  virtual void           startOperation();

  //! sets the dialog widgets to state just after operation start
  virtual void           initDialog();

 protected slots:
  virtual bool           onApply();
  virtual void           onCancel();

 private:
  SMESHGUI_MeshOrderDlg* myDlg;
  SMESHGUI_MeshOrderMgr* myMgr;
};

#endif // SMESHGUI_MeshOrderOp_H
