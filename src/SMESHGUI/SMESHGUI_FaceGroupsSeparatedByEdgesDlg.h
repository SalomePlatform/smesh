// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef SMESHGUI_FaceGroupsSeparatedByEdges_H
#define SMESHGUI_FaceGroupsSeparatedByEdges_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class LightApp_SelectionMgr;
class QCheckBox;
class QLineEdit;
class QWidget;
class QPushButton;
class SMESHGUI_SpinBox;

/*!
 * \brief Dialog to create face groups divided by sharp edges
 */

class SMESHGUI_EXPORT SMESHGUI_FaceGroupsSeparatedByEdgesDlg :  public SMESHGUI_PreviewDlg
{
  Q_OBJECT

 public:

  SMESHGUI_FaceGroupsSeparatedByEdgesDlg( SMESHGUI* theModule );
  virtual ~SMESHGUI_FaceGroupsSeparatedByEdgesDlg();

 private slots:
  void                   ClickOnOk();
  bool                   ClickOnApply();
  void                   ClickOnHelp();
  void                   SelectionIntoArgument();
  void                   DeactivateActiveDialog();
  void                   ActivateThisDialog();
  void                   reject();
  void                   onArgChange() { onDisplaySimulation( true ); }
  virtual void           onDisplaySimulation( bool = true );

 private:

  LightApp_SelectionMgr* mySelectionMgr;
  QWidget*               GroupArgs;
  QWidget*               GroupButtons;

  QLineEdit*             myMeshName;
  SMESHGUI_SpinBox*      myAngle;
  QCheckBox*             myCreateEdgesCheck;
  QCheckBox*             myUseExistingCheck;

  QPushButton*           buttonOk;
  QPushButton*           buttonCancel;
  QPushButton*           buttonApply;
  QPushButton*           buttonHelp;

  SMESH::SMESH_Mesh_var  myMesh;
};

#endif // SMESHGUI_FaceGroupsSeparatedByEdges_H
