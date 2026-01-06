// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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

// File   : SMESHGUI_AddNodeOnSegmentDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_AddNodeOnSegmentDLG_H
#define SMESHGUI_AddNodeOnSegmentDLG_H

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_InteractiveOp.h"

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class SMESHGUI_SpinBox;
class SMESHGUI_MeshEditPreview;
class SMESHGUI_AddNodeOnSegmentDlg;

/*!
 * \brief Operation to make a mesh pass through a point
 */
class SMESHGUI_EXPORT SMESHGUI_AddNodeOnSegmentOp: public SMESHGUI_InteractiveOp
{
  Q_OBJECT

public:
  SMESHGUI_AddNodeOnSegmentOp();
  virtual ~SMESHGUI_AddNodeOnSegmentOp();

  virtual LightApp_Dialog*       dlg() const;  

protected:

  virtual void                   startOperation() override;
  virtual void                   stopOperation() override;

  virtual void                   activateSelection();

  bool                           isValid( QString&, SMESH::smIdType& n1, SMESH::smIdType& n2 );

  virtual void                   processStyleEvents(unsigned long event,
    void* calldata)  override;

  virtual void                   processInteractorEvents(unsigned long event,
    void* calldata) override;


protected slots:
  virtual bool                   onApply() override;


private slots:
  void                           onSelectionDone();
  void                           redisplayPreview();
  void                           onTextChange( const QString& );
  void                           onSelTypeChange();
  void                           onOpenView();
  void                           onCloseView();

private:

  SMESHGUI_AddNodeOnSegmentDlg* myDlg;

  SMESHGUI*                     mySMESHGUI;
  SMESHGUI_MeshEditPreview*     mySimulation;
  SMESH_Actor*                  myMeshActor;
  bool                          myNoPreview;
};

/*!
 * \brief Dialog to make a mesh pass through a point
 */

class SMESHGUI_EXPORT SMESHGUI_AddNodeOnSegmentDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_AddNodeOnSegmentDlg();

signals:

  void                          selTypeChanged();

private:
  QWidget*                      createMainFrame( QWidget* );

  QPushButton*                  mySegmentBtn;
  QLineEdit*                    mySegment;
  QPushButton*                  myPositionBtn;
  SMESHGUI_SpinBox*             myPositionSpin;
  QCheckBox*                    myPreviewChkBox;

  QString                       myHelpFileName;

  friend class SMESHGUI_AddNodeOnSegmentOp;

private slots:
  void                          ButtonToggled( bool );
};

#endif // SMESHGUI_AddNodeOnSegmentDLG_H
