// Copyright (C) 2007-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_RemoveNodeReconnectionDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef __SMESHGUI_RemoveNodeReconnection_HXX__
#define __SMESHGUI_RemoveNodeReconnection_HXX__

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class SMESHGUI_SpinBox;
class SMESHGUI_MeshEditPreview;
class SMESHGUI_RemoveNodeReconnectionDlg;

/*!
 * \brief Remove a node with elements re-connection
 */
class SMESHGUI_EXPORT SMESHGUI_RemoveNodeReconnectionOp: public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_RemoveNodeReconnectionOp();
  virtual ~SMESHGUI_RemoveNodeReconnectionOp();

  virtual LightApp_Dialog*       dlg() const;  

protected:

  virtual void                   startOperation();
  virtual void                   stopOperation();

  virtual void                   activateSelection();

  bool                           isValid( QString& );

protected slots:
  virtual bool                   onApply();

private slots:
  void                           onSelectionDone();
  void                           redisplayPreview();
  void                           onTextChange( const QString& );
  void                           onOpenView();
  void                           onCloseView();

private:

  SMESHGUI_RemoveNodeReconnectionDlg*  myDlg;

  SUIT_SelectionFilter*         myFilter;
  SMESHGUI*                     mySMESHGUI;
  SMESHGUI_MeshEditPreview*     mySimulation;
  SMESH_Actor*                  myMeshActor;
  bool                          myNoPreview;
};

/*!
 * \brief Dialog to make a mesh pass through a point
 */

class SMESHGUI_EXPORT SMESHGUI_RemoveNodeReconnectionDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_RemoveNodeReconnectionDlg();

private:
  QWidget*                      createMainFrame( QWidget* );

  QWidget*                      myMainFrame;
  QGroupBox*                    myNodeToMoveGrp;
  QLineEdit*                    myId;
  QCheckBox*                    myPreviewChkBox;

  QString                       myHelpFileName;

  friend class SMESHGUI_RemoveNodeReconnectionOp;

};

#endif // SMESHGUI_RemoveNodeReconnectionDLG_H
