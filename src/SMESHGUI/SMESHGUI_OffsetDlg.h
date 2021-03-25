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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_OffsetDlg.h
//
#ifndef SMESHGUI_OffsetDLG_H
#define SMESHGUI_OffsetDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class SMESHGUI;
class SMESHGUI_IdValidator;
class SMESHGUI_SpinBox;
class SMESHGUI_FilterDlg;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESH_LogicalFilter;

//=================================================================================
// class    : SMESHGUI_OffsetDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_OffsetDlg : public SMESHGUI_MultiPreviewDlg
{
  Q_OBJECT

 public:
  SMESHGUI_OffsetDlg( SMESHGUI* );
  ~SMESHGUI_OffsetDlg();

 private:
  void                   Init( bool = true );
  void                   enterEvent( QEvent* );           /* mouse enter the QWidget */
  void                   keyPressEvent( QKeyEvent* );
  int                    GetConstructorId();
  void                   setNewMeshName();

  bool                   isValid();
  void                   getOffset( SMESH::PointStruct& thePoint,
                                    SMESH::double_array_var& theOffsetFact);

  SMESHGUI_IdValidator*  myIdValidator;
  LightApp_SelectionMgr* mySelectionMgr;          /* User shape selection */
  QString                myElementsId;
  int                    myNbOkElements;          /* to check when elements are defined */

  SVTK_Selector*         mySelector;

  QWidget*               myEditCurrentArgument;

  bool                   myBusy;
  SMESH_Actor*           myActor;
  SMESH_LogicalFilter*   myMeshOrSubMeshOrGroupFilter;

  QList<SMESH::SMESH_IDSource_var> myObjects;
  QList<QString>                   myObjectsNames;
  QList<SMESH::SMESH_Mesh_var>     myMeshes;


  QGroupBox*             ConstructorsBox;
  QGroupBox*             GroupButtons;
  QPushButton*           buttonOk;
  QPushButton*           buttonCancel;
  QPushButton*           buttonApply;
  QPushButton*           buttonHelp;
  QGroupBox*             GroupArguments;
  QLabel*                TextLabelElements;
  //QPushButton*           SelectElementsButton;
  QLineEdit*             LineEditElements;
  QCheckBox*             CheckBoxMesh;
  SMESHGUI_SpinBox*      SpinBox;
  QGroupBox*             ActionBox;
  QButtonGroup*          ActionGroup;
  QCheckBox*             MakeGroupsCheck;
  QLineEdit*             LineEditNewMesh;

  QString                myHelpFileName;

  QPushButton*           myFilterBtn;
  SMESHGUI_FilterDlg*    myFilterDlg;


protected slots:
  virtual void           onDisplaySimulation( bool );
  virtual void           reject();
  void                   onFilterAccepted();
   
private slots:
  void                   ClickOnOk();
  bool                   ClickOnApply();
  void                   ClickOnHelp();
  void                   SelectionIntoArgument();
  void                   DeactivateActiveDialog();
  void                   ActivateThisDialog();
  void                   onTextChange( const QString& );
  void                   onSelectMesh( bool );
  void                   onActionClicked( int );
  void                   onOpenView();
  void                   onCloseView();
  void                   setFilters();
};

#endif // SMESHGUI_OffsetDLG_H
