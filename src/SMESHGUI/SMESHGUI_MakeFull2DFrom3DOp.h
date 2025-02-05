// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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
// File   : SMESHGUI_MakeFull2DFrom3DOp.h
// Author : Cesar Conopoima, Open CASCADE S.A.S. (cesar.conopoima@opencascade.com)

#ifndef SMESHGUI_MakeFull2DFrom3DOp_H
#define SMESHGUI_MakeFull2DFrom3DOp_H

// SMESH includes
#include "SMESHGUI_Make2DFrom3DOp.h"

/*!
 * \brief Dialog for options of generating 2D mesh from 3D.
 */

class SMESHGUI_EXPORT SMESHGUI_MakeFull2DFrom3DDlg :  public SMESHGUI_Make2DFrom3DDlg
{
  Q_OBJECT

public:
  SMESHGUI_MakeFull2DFrom3DDlg( QWidget* );
  virtual ~SMESHGUI_MakeFull2DFrom3DDlg();

  friend class SMESHGUI_MakeFull2DFrom3DOp;
};

/*!
 * \brief Operation to compute 2D mesh from 3D mesh
 */

class SMESHGUI_EXPORT SMESHGUI_MakeFull2DFrom3DOp : public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_MakeFull2DFrom3DOp();
  virtual ~SMESHGUI_MakeFull2DFrom3DOp();

  virtual LightApp_Dialog*           dlg() const;

protected:
  virtual void                       startOperation();
  virtual void                       selectionDone();
  virtual SUIT_SelectionFilter*      createFilter( const int ) const;
  bool                               isValid( QString& ) const;

protected slots:
  virtual bool                       onApply();

private:
  bool                               compute2DMesh( QStringList& );

private:
  SMESH::SMESH_Mesh_var              mySrcMesh;
  QPointer<SMESHGUI_MakeFull2DFrom3DDlg> myDlg;

  SMESH_TypeFilter                   myMeshFilter;
  SMESH_TypeFilter                   myGroupFilter;
};

#endif // SMESHGUI_MakeFull2DFrom3DOp_H
