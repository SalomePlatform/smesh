// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_AddNodeOnFaceDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_AddNodeOnFaceDLG_H
#define SMESHGUI_AddNodeOnFaceDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

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
class SMESHGUI_AddNodeOnFaceDlg;
class vtkCellPicker;

/*!
 * \brief Operation to split a face into triangles by creating a new node
 *        on the face and connecting it to the face nodes
 */
class SMESHGUI_EXPORT SMESHGUI_AddNodeOnFaceOp: public SMESHGUI_InteractiveOp
{
  Q_OBJECT

public:
  SMESHGUI_AddNodeOnFaceOp();
  virtual ~SMESHGUI_AddNodeOnFaceOp();

  virtual LightApp_Dialog*      dlg() const;  

protected:

  virtual void                  startOperation() override;
  virtual void                  stopOperation() override;

  virtual void                  activateSelection() override;

  bool                          isValid( QString& );

  virtual void                   processStyleEvents(unsigned long event,
                                                    void* calldata)  override;

  virtual void                   processInteractorEvents(unsigned long event,
                                                         void* calldata) override;


protected slots:
  virtual bool                  onApply();

private slots:
  void                          onSelectionDone();
  void                          redisplayPreview();
  void                          onSelTypeChange();
  void                          onTextChange( const QString& );
//  void                          onDestCoordChanged();
  void                          onOpenView();
  void                          onCloseView();
  void                          pointLocationChanged(bool);
  void                          onDestCoordChanged();

private:
  SMESHGUI_AddNodeOnFaceDlg*    myDlg;

  SUIT_SelectionFilter*         myFilter;
  SMESHGUI*                     mySMESHGUI;
  SMESHGUI_MeshEditPreview*     mySimulation;
  SMESH_Actor*                  myMeshActor;
  bool                          myNoPreview;
  bool                          myUpdateDestination;
  bool                          myDestCoordChanged;
  vtkCellPicker*                myFacePicker;      
};

/*!
 * \brief Dialog to split a face into triangles by creating a new node
 *        on the face and connecting it to the face nodes
 */

class SMESHGUI_EXPORT SMESHGUI_AddNodeOnFaceDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_AddNodeOnFaceDlg();

private:
  QWidget*                      createMainFrame( QWidget* );

  QWidget*                      myMainFrame;

  QPushButton*                  myDestBtn;
  QPushButton*                  myIdBtn;
  QLineEdit*                    myId;
  SMESHGUI_SpinBox*             myDestinationX;
  SMESHGUI_SpinBox*             myDestinationY;
  SMESHGUI_SpinBox*             myDestinationZ;
  QCheckBox*                    myPointOnFace;
  QCheckBox*                    myPreviewChkBox;

  QString                       myHelpFileName;

  friend class SMESHGUI_AddNodeOnFaceOp;

signals:
  void                          selTypeChanged();

 private slots:
  void                          ButtonToggled( bool );
};

#endif // SMESHGUI_AddNodeOnFaceDLG_H
