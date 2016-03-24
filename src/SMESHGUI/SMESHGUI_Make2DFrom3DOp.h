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
// File   : SMESHGUI_Make2DFrom3D.h
// Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#ifndef SMESHGUI_Make2DFrom3DOp_H
#define SMESHGUI_Make2DFrom3DOp_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"
#include "SMESH_TypeFilter.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QCheckBox;
class QLineEdit;
class QRadioButton;
class SMESHGUI_Make2DFrom3DOp;

/*!
 * \brief Dialog to show result mesh statistic
 */

class SMESHGUI_EXPORT SMESHGUI_Make2DFrom3DDlg :  public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  enum { MeshOrGroups };

  SMESHGUI_Make2DFrom3DDlg( QWidget* );
  virtual ~SMESHGUI_Make2DFrom3DDlg();

  SMESH::Bnd_Dimension mode() const;

  bool                 needNewMesh() const;
  QString              getNewMeshName() const;
  void                 setNewMeshName( const QString& );
  void                 setNewMeshEnabled( bool );
  bool                 getNewMeshEnabled() const;

  bool                 needGroup() const;
  QString              getGroupName() const;
  void                 setGroupName( const QString& );

  bool                 copySource() const;

private slots:
  void                 onTargetChanged();
  void                 onGroupChecked();

private:
  QRadioButton* my2dFrom3dRB;
  QRadioButton* my1dFrom2dRB;
  QRadioButton* my1dFrom3dRB;
  QRadioButton* myThisMeshRB;
  QRadioButton* myNewMeshRB;
  QLineEdit*    myMeshName;
  QCheckBox*    myCopyCheck;
  QCheckBox*    myGroupCheck;
  QLineEdit*    myGroupName;

  friend class SMESHGUI_Make2DFrom3DOp;
};

/*!
 * \brief Operation to compute 2D mesh on 3D
 */

class SMESHGUI_EXPORT SMESHGUI_Make2DFrom3DOp : public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_Make2DFrom3DOp();
  virtual ~SMESHGUI_Make2DFrom3DOp();

  virtual LightApp_Dialog*           dlg() const;

protected:
  virtual void                       startOperation();
  virtual void                       selectionDone();
  virtual SUIT_SelectionFilter*      createFilter( const int ) const;
  bool                               isValid( QString& ) const;

protected slots:
  virtual bool                       onApply();
  void                               onModeChanged();

private:
  bool                               compute2DMesh( QStringList& );

private:
  SMESH::SMESH_Mesh_var              mySrcMesh;
  QPointer<SMESHGUI_Make2DFrom3DDlg> myDlg;

  SMESH_TypeFilter                   myMeshFilter;
  SMESH_TypeFilter                   myGroupFilter;
};

#endif // SMESHGUI_Make2DFrom3DOp_H
