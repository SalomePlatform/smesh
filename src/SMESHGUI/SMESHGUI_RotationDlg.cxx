// SMESH SMESHGUI : GUI for SMESH component
//
// Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
//
// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License. 
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
// File   : SMESHGUI_RotationDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
//

// SMESH includes
#include "SMESHGUI_RotationDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes
#include <SALOMEDSClient_SObject.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

enum { MOVE_ELEMS_BUTTON = 0, COPY_ELEMS_BUTTON, MAKE_MESH_BUTTON }; //!< action type

#define SPACING 8
#define MARGIN  11

//=================================================================================
// class    : SMESHGUI_RotationDlg()
// purpose  :
//=================================================================================
SMESHGUI_RotationDlg::SMESHGUI_RotationDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_MESH_ROTATION")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_ROTATION_TITLE"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_RotationDlgLayout = new QVBoxLayout(this);
  SMESHGUI_RotationDlgLayout->setSpacing(SPACING);
  SMESHGUI_RotationDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(tr("SMESH_ROTATION"), this);
  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* GroupConstructorsLayout = new QHBoxLayout(GroupConstructors);
  GroupConstructorsLayout->setSpacing(SPACING);
  GroupConstructorsLayout->setMargin(MARGIN);

  RadioButton1= new QRadioButton(GroupConstructors);
  RadioButton1->setIcon(image0);

  GroupConstructorsLayout->addWidget(RadioButton1);
  ButtonGroup->addButton(RadioButton1, 0);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);
  SelectElementsButton  = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image1);
  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for axis defining
  GroupAxis = new QGroupBox(tr("SMESH_AXIS"), GroupArguments);
  QGridLayout* GroupAxisLayout = new QGridLayout(GroupAxis);
  GroupAxisLayout->setSpacing(SPACING);
  GroupAxisLayout->setMargin(MARGIN);

  TextLabelPoint = new QLabel(tr("SMESH_POINT"), GroupAxis);
  SelectPointButton  = new QPushButton(GroupAxis);
  SelectPointButton->setIcon(image1);

  TextLabelX = new QLabel(tr("SMESH_X"), GroupAxis);
  SpinBox_X = new SMESHGUI_SpinBox(GroupAxis);
  TextLabelY = new QLabel(tr("SMESH_Y"), GroupAxis);
  SpinBox_Y = new SMESHGUI_SpinBox(GroupAxis);
  TextLabelZ = new QLabel(tr("SMESH_Z"), GroupAxis);
  SpinBox_Z = new SMESHGUI_SpinBox(GroupAxis);

  TextLabelVector = new QLabel(tr("SMESH_VECTOR"), GroupAxis);
  SelectVectorButton = new QPushButton(GroupAxis);
  SelectVectorButton->setIcon(image1);

  TextLabelDX = new QLabel(tr("SMESH_DX"), GroupAxis);
  SpinBox_DX = new SMESHGUI_SpinBox(GroupAxis);
  TextLabelDY = new QLabel(tr("SMESH_DY"), GroupAxis);
  SpinBox_DY = new SMESHGUI_SpinBox(GroupAxis);
  TextLabelDZ = new QLabel(tr("SMESH_DZ"), GroupAxis);
  SpinBox_DZ = new SMESHGUI_SpinBox(GroupAxis);

  GroupAxisLayout->addWidget(TextLabelPoint,     0, 0);
  GroupAxisLayout->addWidget(SelectPointButton,  0, 1);
  GroupAxisLayout->addWidget(TextLabelX,         0, 2);
  GroupAxisLayout->addWidget(SpinBox_X,          0, 3);
  GroupAxisLayout->addWidget(TextLabelY,         0, 4);
  GroupAxisLayout->addWidget(SpinBox_Y,          0, 5);
  GroupAxisLayout->addWidget(TextLabelZ,         0, 6);
  GroupAxisLayout->addWidget(SpinBox_Z,          0, 7);
  GroupAxisLayout->addWidget(TextLabelVector,    1, 0);
  GroupAxisLayout->addWidget(SelectVectorButton, 1, 1);
  GroupAxisLayout->addWidget(TextLabelDX,        1, 2);
  GroupAxisLayout->addWidget(SpinBox_DX,         1, 3);
  GroupAxisLayout->addWidget(TextLabelDY,        1, 4);
  GroupAxisLayout->addWidget(SpinBox_DY,         1, 5);
  GroupAxisLayout->addWidget(TextLabelDZ,        1, 6);
  GroupAxisLayout->addWidget(SpinBox_DZ,         1, 7);

  // Controls for angle defining
  TextLabelAngle = new QLabel(tr("SMESH_ANGLE"), GroupArguments);
  SpinBox_Angle = new SMESHGUI_SpinBox(GroupArguments);

  // action switch
  ActionBox = new QGroupBox(GroupArguments);
  ActionGroup = new QButtonGroup(GroupArguments);
  QVBoxLayout* ActionBoxLayout = new QVBoxLayout(ActionBox);
  ActionBoxLayout->setSpacing(SPACING);
  ActionBoxLayout->setMargin(MARGIN);

  QRadioButton* aMoveElements = new QRadioButton(tr("SMESH_MOVE_ELEMENTS"), ActionBox);
  QRadioButton* aCopyElements = new QRadioButton(tr("SMESH_COPY_ELEMENTS"), ActionBox);
  QRadioButton* aCreateMesh   = new QRadioButton(tr("SMESH_CREATE_MESH"),   ActionBox);

  ActionBoxLayout->addWidget(aMoveElements);
  ActionBoxLayout->addWidget(aCopyElements);
  ActionBoxLayout->addWidget(aCreateMesh);
  ActionGroup->addButton(aMoveElements, MOVE_ELEMS_BUTTON);
  ActionGroup->addButton(aCopyElements, COPY_ELEMS_BUTTON);
  ActionGroup->addButton(aCreateMesh,   MAKE_MESH_BUTTON);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);

  // Name of a mesh to create
  LineEditNewMesh = new QLineEdit(GroupArguments);

  GroupArgumentsLayout->addWidget(TextLabelElements,    0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2, 1, 2);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 4);
  GroupArgumentsLayout->addWidget(GroupAxis,            2, 0, 1, 4);
  GroupArgumentsLayout->addWidget(TextLabelAngle,       3, 0, 1, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Angle,        3, 2);
  GroupArgumentsLayout->addWidget(ActionBox,            4, 0, 3, 3);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      5, 3);
  GroupArgumentsLayout->addWidget(LineEditNewMesh,      6, 3);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_OK"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  SMESHGUI_RotationDlgLayout->addWidget(GroupConstructors);
  SMESHGUI_RotationDlgLayout->addWidget(GroupArguments);
  SMESHGUI_RotationDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox_X->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Y->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Z->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);

  SpinBox_Angle->RangeStepAndValidator(-360.0, +360.0, 5.0, 3);

  myConstructorId = 0;
  RadioButton1->setChecked(true);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Costruction of the logical filter
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QList<SUIT_SelectionFilter*> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "rotation_page.html";

  Init();

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  this, SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectElementsButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectPointButton, SIGNAL (clicked()),    this, SLOT(SetEditCurrentArgument()));
  connect(SelectVectorButton, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));

  connect(SpinBox_DX, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));
  connect(SpinBox_DY, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));
  connect(SpinBox_DZ, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                  SLOT(onSelectMesh(bool)));
  connect(ActionGroup,      SIGNAL(buttonClicked(int)),             SLOT(onActionClicked(int)));

  onActionClicked(MOVE_ELEMS_BUTTON);
}

//=================================================================================
// function : ~SMESHGUI_RotationDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RotationDlg::~SMESHGUI_RotationDlg()
{
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::Init (bool ResetControls)
{
  myBusy = false;

  myEditCurrentArgument = (QWidget*)LineEditElements;
  LineEditElements->setFocus();
  myElementsId = "";
  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();

  if (ResetControls) {
    SpinBox_X->SetValue(0.0);
    SpinBox_Y->SetValue(0.0);
    SpinBox_Z->SetValue(0.0);
    SpinBox_DX->SetValue(0.0);
    SpinBox_DY->SetValue(0.0);
    SpinBox_DZ->SetValue(0.0);

    SpinBox_Angle->SetValue(45);

    ActionGroup->button( MOVE_ELEMS_BUTTON )->setChecked(true);
    CheckBoxMesh->setChecked(false);
//     MakeGroupsCheck->setChecked(false);
//     MakeGroupsCheck->setEnabled(false);
//    onSelectMesh(false);
  }

  onSelectMesh(CheckBoxMesh->isChecked());
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  if (myNbOkElements && IsAxisOk()) {
    QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);

    SMESH::long_array_var anElementsId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    SMESH::AxisStruct anAxis;

    anAxis.x =  SpinBox_X->GetValue();
    anAxis.y =  SpinBox_Y->GetValue();
    anAxis.z =  SpinBox_Z->GetValue();;
    anAxis.vx = SpinBox_DX->GetValue();
    anAxis.vy = SpinBox_DY->GetValue();
    anAxis.vz = SpinBox_DZ->GetValue();

    double anAngle = (SpinBox_Angle->GetValue())*PI/180;
    int actionButton = ActionGroup->checkedId();
    bool makeGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );
    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      QApplication::setOverrideCursor(Qt::WaitCursor);
      switch ( actionButton ) {
      case MOVE_ELEMS_BUTTON:
        aMeshEditor->Rotate(anElementsId, anAxis, anAngle, false);
        break;
      case COPY_ELEMS_BUTTON:
        if ( makeGroups )
          SMESH::ListOfGroups_var groups = 
            aMeshEditor->RotateMakeGroups(anElementsId, anAxis, anAngle);
        else
          aMeshEditor->Rotate(anElementsId, anAxis, anAngle, true);
        break;
      case MAKE_MESH_BUTTON:
        SMESH::SMESH_Mesh_var mesh = 
          aMeshEditor->RotateMakeMesh(anElementsId, anAxis, anAngle, makeGroups,
                                      LineEditNewMesh->text().toLatin1().data());
      }
      QApplication::restoreOverrideCursor();
    } catch (...) {
    }

    SMESH::UpdateView();
    if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() ||
         actionButton == MAKE_MESH_BUTTON )
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
    Init(false);
    SelectionIntoArgument();
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::ClickOnCancel()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
			     tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			     arg(app->resourceMgr()->stringValue("ExternalBrowser",
								 platform)).
			     arg(myHelpFileName));
  }
}

//=======================================================================
// function : onTextChange()
// purpose  :
//=======================================================================
void SMESHGUI_RotationDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if (myBusy) return;
  myBusy = true;

  if (send == LineEditElements)
    myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // hilight entered elements
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    if (send == LineEditElements) {
      Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
      
      TColStd_MapOfInteger newIndices;
      
      QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < aListId.count(); i++) {
	const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt());
	if (e)
	  newIndices.Add(e->GetID());
	myNbOkElements++;
      }

      mySelector->AddOrRemoveIndex( anIO, newIndices, false );
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->highlight( anIO, true, true );
      
      myElementsId = theNewText;
    }
  }

  if (myNbOkElements && IsAxisOk()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_RotationDlg::SelectionIntoArgument()
{
  if (myBusy) return;

  // clear
  myActor = 0;
  QString aString = "";

  myBusy = true;
  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    LineEditElements->setText(aString);
    myNbOkElements = 0;
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);
  }
  myBusy = false;

  if (!GroupButtons->isEnabled()) // inactive
    return;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  myMesh = SMESH::GetMeshByIO(IO);
  if (myMesh->_is_nil())
    return;

  myActor = SMESH::FindActorByObject(myMesh);
  if (!myActor)
    myActor = SMESH::FindActorByEntry(IO->getEntry());
  if (!myActor)
    return;

  int aNbUnits = 0;

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    myElementsId = "";

    // MakeGroups is available if there are groups and "Copy"
    if ( myMesh->NbGroups() == 0 ) {
      MakeGroupsCheck->setChecked(false);
      MakeGroupsCheck->setEnabled(false);
    }
    else if ( ActionGroup->checkedId() != MOVE_ELEMS_BUTTON ) {
      MakeGroupsCheck->setEnabled(true);
    }
    if (CheckBoxMesh->isChecked()) {
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

      if (!SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO)->_is_nil()) { //MESH
        // get IDs from mesh
        SMDS_Mesh* aSMDSMesh = myActor->GetObject()->GetMesh();
        if (!aSMDSMesh)
          return;

        for (int i = aSMDSMesh->MinElementID(); i <= aSMDSMesh->MaxElementID(); i++) {
          const SMDS_MeshElement * e = aSMDSMesh->FindElement(i);
          if (e) {
            myElementsId += QString(" %1").arg(i);
            aNbUnits++;
          }
        }
      } else if (!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil()) { //SUBMESH
        // get submesh
        SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO);

        // get IDs from submesh
        SMESH::long_array_var anElementsIds = new SMESH::long_array;
        anElementsIds = aSubMesh->GetElementsId();
        for (int i = 0; i < anElementsIds->length(); i++) {
          myElementsId += QString(" %1").arg(anElementsIds[i]);
        }
        aNbUnits = anElementsIds->length();
      } else { // GROUP
        // get smesh group
        SMESH::SMESH_GroupBase_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);
        if (aGroup->_is_nil())
          return;

        // get IDs from smesh group
        SMESH::long_array_var anElementsIds = new SMESH::long_array;
        anElementsIds = aGroup->GetListOfID();
        for (int i = 0; i < anElementsIds->length(); i++) {
          myElementsId += QString(" %1").arg(anElementsIds[i]);
        }
        aNbUnits = anElementsIds->length();
      }
    } else {
      aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, IO, aString);
      myElementsId = aString;
    }

    if (aNbUnits < 1)
      return;

    myNbOkElements = true;
  } else {
    aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
    if (aNbUnits != 1)
      return;

    SMDS_Mesh* aMesh =  myActor->GetObject()->GetMesh();
    if (!aMesh)
      return;

    const SMDS_MeshNode * n = aMesh->FindNode(aString.toInt());
    if (!n)
      return;

    double x = n->X();
    double y = n->Y();
    double z = n->Z();

    if (myEditCurrentArgument == (QWidget*)SpinBox_X) {
      SpinBox_X->SetValue(x);
      SpinBox_Y->SetValue(y);
      SpinBox_Z->SetValue(z);
    } else if (myEditCurrentArgument == (QWidget*)SpinBox_DX) {
      SpinBox_DX->SetValue(x - SpinBox_X->GetValue());
      SpinBox_DY->SetValue(y - SpinBox_Y->GetValue());
      SpinBox_DZ->SetValue(z - SpinBox_Z->GetValue());
    }
  }

  myBusy = true;
  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    LineEditElements->setText(aString);
    setNewMeshName();
  }
  myBusy = false;

  // OK
  if (myNbOkElements && IsAxisOk()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  switch (myConstructorId) {
  case 0: /* default constructor */
    {
      SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
      if (send == SelectElementsButton) {
        myEditCurrentArgument = (QWidget*)LineEditElements;
        SMESH::SetPointRepresentation(false);
        if (CheckBoxMesh->isChecked()) {
	  if ( aViewWindow )
	    aViewWindow->SetSelectionMode(ActorSelection);
          mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
        } else {
	  if ( aViewWindow )
	    aViewWindow->SetSelectionMode( CellSelection );
	}
      } else if (send == SelectPointButton) {
        myEditCurrentArgument = (QWidget*)SpinBox_X;
        SMESH::SetPointRepresentation(true);
	if ( aViewWindow )
	  aViewWindow->SetSelectionMode( NodeSelection );
      } else if (send == SelectVectorButton) {
        myEditCurrentArgument = (QWidget*)SpinBox_DX;
        SMESH::SetPointRepresentation(true);
	if ( aViewWindow )
	  aViewWindow->SetSelectionMode( NodeSelection );
      }
      break;
    }
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupArguments->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( CellSelection );
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=================================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=================================================================================
void SMESHGUI_RotationDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=================================================================================
// function : onSelectMesh()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::onSelectMesh (bool toSelectMesh)
{
  if (toSelectMesh)
    TextLabelElements->setText(tr("SMESH_NAME"));
  else
    TextLabelElements->setText(tr("SMESH_ID_ELEMENTS"));

  if (myEditCurrentArgument != LineEditElements) {
    LineEditElements->clear();
    return;
  }

  mySelectionMgr->clearFilters();
  SMESH::SetPointRepresentation(false);

  if (toSelectMesh) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    LineEditElements->setReadOnly(true);
    LineEditElements->setValidator(0);
  } else {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( CellSelection );
    LineEditElements->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(LineEditElements->text());
  }

  SelectionIntoArgument();
}

//=================================================================================
// function : IsAxisOk()
// purpose  :
//=================================================================================
bool SMESHGUI_RotationDlg::IsAxisOk()
{
  return (SpinBox_DX->GetValue() != 0 ||
	  SpinBox_DY->GetValue() != 0 ||
	  SpinBox_DZ->GetValue() != 0);
}

//=================================================================================
// function : onVectorChanged()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::onVectorChanged()
{
  if (IsAxisOk()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  } else {
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);
  }
}


//=======================================================================
//function : onActionClicked
//purpose  : slot called when an action type changed
//=======================================================================

void SMESHGUI_RotationDlg::onActionClicked(int button)
{
  switch ( button ) {
  case MOVE_ELEMS_BUTTON:
    MakeGroupsCheck->setEnabled(false);
    LineEditNewMesh->setEnabled(false);
    break;
  case COPY_ELEMS_BUTTON:
    LineEditNewMesh->setEnabled(false);
    MakeGroupsCheck->setText( tr("SMESH_MAKE_GROUPS"));
    if ( myMesh->_is_nil() || myMesh->NbGroups() > 0)
      MakeGroupsCheck->setEnabled(true);
    else
      MakeGroupsCheck->setEnabled(false);
    break;
  case MAKE_MESH_BUTTON:
    LineEditNewMesh->setEnabled(true);
    MakeGroupsCheck->setText( tr("SMESH_COPY_GROUPS"));
    if ( myMesh->_is_nil() || myMesh->NbGroups() > 0)
      MakeGroupsCheck->setEnabled(true);
    else
      MakeGroupsCheck->setEnabled(false);
    break;
  }
  setNewMeshName();
}

//=======================================================================
//function : setNewMeshName
//purpose  : update contents of LineEditNewMesh
//=======================================================================

void SMESHGUI_RotationDlg::setNewMeshName()
{
  LineEditNewMesh->setText("");
  if ( LineEditNewMesh->isEnabled() && !myMesh->_is_nil() ) {
    QString name;
    if ( CheckBoxMesh->isChecked() ) {
      name = LineEditElements->text();
    }
    else {
      _PTR(SObject) meshSO = SMESH::FindSObject( myMesh );
      name = meshSO->GetName().c_str();
    }
    if ( !name.isEmpty() )
      LineEditNewMesh->setText( SMESH::UniqueMeshName( name, "rotated"));
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RotationDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
