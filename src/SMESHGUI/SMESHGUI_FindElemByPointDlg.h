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

#ifndef SMESHGUI_FindElemByPointDLG_H
#define SMESHGUI_FindElemByPointDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QLineEdit;
class QPushButton;
class QListWidget;
class QtxComboBox;
class SMESHGUI_SpinBox;
class SMESHGUI_MeshEditPreview;
class SMESHGUI_FindElemByPointDlg;

/*!
 * \brief Operation to find elements by a point coordinates
 */
class SMESHGUI_EXPORT SMESHGUI_FindElemByPointOp: public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_FindElemByPointOp();
  virtual ~SMESHGUI_FindElemByPointOp();

  virtual LightApp_Dialog*       dlg() const;  

protected:

  virtual void                   startOperation();
  virtual void                   stopOperation();

  virtual void                   activateSelection();

protected slots:

  virtual void                   onFind();
  virtual bool                   onApply();

private slots:

  void                           onRejectedDlg();
  void                           onSelectionDone();
  void                           onElemSelected();
  void                           onElemTypeChange(int);
  void                           redisplayPreview();
  void                           onOpenView();
  void                           onCloseView();

private:
  SMESHGUI_FindElemByPointDlg*     myDlg;

  SUIT_SelectionFilter*            myFilter;
  SMESHGUI*                        mySMESHGUI;
  SMESHGUI_MeshEditPreview*        mySimulation; // to show point coordinates
  SMESH::SMESH_IDSource_var        myMeshOrPart;
  SMESH::MeshPreviewStruct_var     myPreview;
  Handle(SALOME_InteractiveObject) myMeshIO;
};

/*!
 * \brief Dialog to find elements by a point coordinates
 */

class SMESHGUI_EXPORT SMESHGUI_FindElemByPointDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_FindElemByPointDlg();

  void setTypes(SMESH::array_of_ElementType_var & types);

private:
  QWidget*                      createMainFrame( QWidget* );

  QLineEdit*                    myMeshName;
  SMESHGUI_SpinBox*             myX;
  SMESHGUI_SpinBox*             myY;
  SMESHGUI_SpinBox*             myZ;
  QtxComboBox*                  myElemTypeCombo;
  QPushButton*                  myFindBtn;
  QListWidget*                  myFoundList;

  QString                       myHelpFileName;

  friend class SMESHGUI_FindElemByPointOp;

signals:
  void                           rejectedDlg();

protected slots:
  virtual void                   reject();

private slots:
//void                          ButtonToggled( bool );
};

#endif // SMESHGUI_FindElemByPointDLG_H
