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

// File   : SMESHGUI_MakeNodeAtPointDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MAKENODEATPOINTDLG_H
#define SMESHGUI_MAKENODEATPOINTDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

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
class SMESHGUI_MakeNodeAtPointDlg;

/*!
 * \brief Operation to make a mesh pass through a point
 */
class SMESHGUI_EXPORT SMESHGUI_MakeNodeAtPointOp: public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_MakeNodeAtPointOp();
  virtual ~SMESHGUI_MakeNodeAtPointOp();

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
  void                           onUpdateDestination();
  void                           onDestCoordChanged();
  void                           onOpenView();
  void                           onCloseView();

private:
  int                           GetConstructorId();

  SMESHGUI_MakeNodeAtPointDlg*  myDlg;

  SUIT_SelectionFilter*         myFilter;
  int                           myMeshOldDisplayMode;
  SMESHGUI*                     mySMESHGUI;
  SMESHGUI_MeshEditPreview*     mySimulation;
  SMESH_Actor*                  myMeshActor;
  bool                          myNoPreview;
  bool                          myUpdateDestination;
  bool                          myDestCoordChanged;
};

/*!
 * \brief Dialog to make a mesh pass through a point
 */

class SMESHGUI_EXPORT SMESHGUI_MakeNodeAtPointDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_MakeNodeAtPointDlg();

private:
  QWidget*                      createMainFrame( QWidget* );

  QWidget*                      myMainFrame;

  QButtonGroup*                 myButtonGroup;
  QRadioButton*                 myRButNodeToMove;
  QRadioButton*                 myRButMoveWithoutNode;
  QPushButton*                  myDestBtn;
  QPushButton*                  myUpdateBtn;
  QGroupBox*                    myDestinationGrp;
  QGroupBox*                    myNodeToMoveGrp;
  QPushButton*                  myIdBtn;
  QLineEdit*                    myId;
  SMESHGUI_SpinBox*             myCurrentX;
  SMESHGUI_SpinBox*             myCurrentY;
  SMESHGUI_SpinBox*             myCurrentZ;
  SMESHGUI_SpinBox*             myDestinationX;
  SMESHGUI_SpinBox*             myDestinationY;
  SMESHGUI_SpinBox*             myDestinationZ;
  QLabel*                       myDestDXLabel;
  QLabel*                       myDestDYLabel;
  QLabel*                       myDestDZLabel;
  SMESHGUI_SpinBox*             myDestDX;
  SMESHGUI_SpinBox*             myDestDY;
  SMESHGUI_SpinBox*             myDestDZ;
  QCheckBox*                    myPreviewChkBox;

  QString                       myHelpFileName;

  friend class SMESHGUI_MakeNodeAtPointOp;

private slots:
  void                          ButtonToggled( bool );
  void                          ConstructorsClicked( int );
};

#endif // SMESHGUI_MAKENODEATPOINTDLG_H
