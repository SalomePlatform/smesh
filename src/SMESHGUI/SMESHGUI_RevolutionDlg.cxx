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
// File   : SMESHGUI_RevolutionDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
//

// SMESH includes
#include "SMESHGUI_RevolutionDlg.h"

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
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewWindow.h>
#include <SVTK_Selector.h>
#include <SALOME_ListIO.hxx>

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
#include <QSpinBox>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

//=================================================================================
// class    : SMESHGUI_RevolutionDlg()
// purpose  :
//=================================================================================
SMESHGUI_RevolutionDlg::SMESHGUI_RevolutionDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap image0 ( mgr->loadPixmap("SMESH", tr("ICON_DLG_EDGE")));
  QPixmap image1 ( mgr->loadPixmap("SMESH", tr("ICON_DLG_TRIANGLE")));
  QPixmap image2 ( mgr->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("REVOLUTION_AROUND_AXIS"));
  setSizeGripEnabled(true);
  
  QVBoxLayout* SMESHGUI_RevolutionDlgLayout = new QVBoxLayout(this);
  SMESHGUI_RevolutionDlgLayout->setSpacing(SPACING);
  SMESHGUI_RevolutionDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_REVOLUTION"), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING);
  ConstructorsBoxLayout->setMargin(MARGIN);

  RadioButton1 = new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image0);
  RadioButton2 = new QRadioButton(ConstructorsBox);
  RadioButton2->setIcon(image1);

  ConstructorsBoxLayout->addWidget(RadioButton1);
  ConstructorsBoxLayout->addWidget(RadioButton2);
  GroupConstructors->addButton(RadioButton1, 0);
  GroupConstructors->addButton(RadioButton2, 1);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("REVOLUTION_1D"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image2);

  LineEditElements  = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(new SMESHGUI_IdValidator(this));

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for axis defining
  GroupAxis = new QGroupBox(tr("SMESH_AXIS"), GroupArguments);
  QGridLayout* GroupAxisLayout = new QGridLayout(GroupAxis);
  GroupAxisLayout->setSpacing(SPACING);
  GroupAxisLayout->setMargin(MARGIN);

  TextLabelPoint = new QLabel(tr("SMESH_POINT"), GroupAxis);
  SelectPointButton  = new QPushButton(GroupAxis);
  SelectPointButton->setIcon(image2);

  TextLabelX = new QLabel(tr("SMESH_X"), GroupAxis);
  SpinBox_X = new SMESHGUI_SpinBox(GroupAxis);

  TextLabelY = new QLabel(tr("SMESH_Y"), GroupAxis);
  SpinBox_Y = new SMESHGUI_SpinBox(GroupAxis);

  TextLabelZ = new QLabel(tr("SMESH_Z"), GroupAxis);
  SpinBox_Z = new SMESHGUI_SpinBox(GroupAxis);

  TextLabelVector = new QLabel(tr("SMESH_VECTOR"), GroupAxis);
  SelectVectorButton = new QPushButton(GroupAxis);
  SelectVectorButton->setIcon(image2);

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

  // Controls for nb. steps defining
  TextLabelNbSteps = new QLabel(tr("SMESH_NUMBEROFSTEPS"), GroupArguments);
  SpinBox_NbSteps = new QSpinBox(GroupArguments);

  // Controls for tolerance defining
  TextLabelTolerance = new QLabel(tr("SMESH_TOLERANCE"), GroupArguments);
  SpinBox_Tolerance = new SMESHGUI_SpinBox(GroupArguments);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  MakeGroupsCheck->setChecked(true);

  GroupArgumentsLayout->addWidget(TextLabelElements,    0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 3);
  GroupArgumentsLayout->addWidget(GroupAxis,            2, 0, 1, 3);
  GroupArgumentsLayout->addWidget(TextLabelAngle,       3, 0, 1, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Angle,        3, 2);
  GroupArgumentsLayout->addWidget(TextLabelNbSteps,     4, 0, 1, 2);
  GroupArgumentsLayout->addWidget(SpinBox_NbSteps,      4, 2);
  GroupArgumentsLayout->addWidget(TextLabelTolerance,   5, 0, 1, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Tolerance,    5, 2);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      6, 0, 1, 3);

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
  SMESHGUI_RevolutionDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_RevolutionDlgLayout->addWidget(GroupArguments);
  SMESHGUI_RevolutionDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox_X->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Y->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Z->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);

  SpinBox_Angle->RangeStepAndValidator(COORD_MIN, COORD_MAX, 5.0, 3);

  SpinBox_NbSteps->setRange(1, 999999);

  SpinBox_Tolerance->RangeStepAndValidator(0.0, COORD_MAX, 0.1, 6);

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

  myHelpFileName = "revolution_page.html";

  Init();

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(GroupConstructors, SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectElementsButton, SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectPointButton,    SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectVectorButton,   SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));

  connect(SpinBox_DX, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));
  connect(SpinBox_DY, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));
  connect(SpinBox_DZ, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));

  connect(mySMESHGUI,     SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),      this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL(SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),   SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                 SLOT(onSelectMesh(bool)));

  ConstructorsClicked(0);
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_RevolutionDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RevolutionDlg::~SMESHGUI_RevolutionDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::Init (bool ResetControls)
{
  myBusy = false;

  myEditCurrentArgument = 0;
  LineEditElements->clear();
  myElementsId = "";
  myNbOkElements = 0;

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
    SpinBox_NbSteps->setValue(1);
    SpinBox_Tolerance->SetValue(1e-05);

    CheckBoxMesh->setChecked(false);
    onSelectMesh(false);
  }
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RevolutionDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  Selection_Mode aSelMode = ActorSelection;

  switch (constructorId) {
  case 0:
    {
      GroupArguments->setTitle(tr("REVOLUTION_1D"));
      aSelMode = EdgeSelection;
      break;
    }
  case 1:
    {
      GroupArguments->setTitle(tr("REVOLUTION_2D"));
      aSelMode = FaceSelection;
      break;
    }
  }

  if (myEditCurrentArgument != (QWidget*)LineEditElements) {
    SMESH::SetPointRepresentation(false);
  }

  if (!CheckBoxMesh->isChecked())
    {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(aSelMode);
    }

  myEditCurrentArgument = (QWidget*)LineEditElements;
  LineEditElements->setFocus();

  if (CheckBoxMesh->isChecked())
    onSelectMesh(true);

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnApply()
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
    long aNbSteps = (long)SpinBox_NbSteps->value();
    double aTolerance = SpinBox_Tolerance->GetValue();

    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      QApplication::setOverrideCursor(Qt::WaitCursor);

      if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() )
        SMESH::ListOfGroups_var groups = 
          aMeshEditor->RotationSweepMakeGroups(anElementsId.inout(), anAxis,
                                               anAngle, aNbSteps, aTolerance);
      else
        aMeshEditor->RotationSweep(anElementsId.inout(), anAxis, anAngle, aNbSteps, aTolerance);

      QApplication::restoreOverrideCursor();
    } catch (...) {
    }

    SMESH::UpdateView();
    if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() )
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
    Init(false);
    ConstructorsClicked(GetConstructorId());
    SelectionIntoArgument();
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnCancel()
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
void SMESHGUI_RevolutionDlg::ClickOnHelp()
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
void SMESHGUI_RevolutionDlg::onTextChange (const QString& theNewText)
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

      mySelector->AddOrRemoveIndex(myActor->getIO(), newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->highlight( myActor->getIO(), true, true );
      
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
void SMESHGUI_RevolutionDlg::SelectionIntoArgument()
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
  const SALOME_ListIO& aList = mySelector->StoredIObjects();

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

    // MakeGroups is available if there are groups
    if ( myMesh->NbGroups() == 0 ) {
      MakeGroupsCheck->setChecked(false);
      MakeGroupsCheck->setEnabled(false);
    } else {
      MakeGroupsCheck->setEnabled(true);
    }

    if (CheckBoxMesh->isChecked()) {
      int aConstructorId = GetConstructorId();

      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

      if (!SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO)->_is_nil()) { //MESH
        // get IDs from mesh
        SMDS_Mesh* aSMDSMesh = myActor->GetObject()->GetMesh();
        if (!aSMDSMesh)
          return;

        if (aConstructorId == 0) {
          SMDS_EdgeIteratorPtr anIter = aSMDSMesh->edgesIterator();

          while (anIter->more()) {
            const SMDS_MeshEdge * edge = anIter->next();
            if (edge) {
              myElementsId += QString(" %1").arg(edge->GetID());
              aNbUnits++;
            }
          }
        } else if (aConstructorId == 1) {
          SMDS_FaceIteratorPtr anIter = aSMDSMesh->facesIterator();
          while (anIter->more()) {
            const SMDS_MeshFace * face = anIter->next();
            if (face) {
              myElementsId += QString(" %1").arg(face->GetID());
              aNbUnits++;
            }
          }
        }
      } else if (!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil()) { //SUBMESH
        // get submesh
        SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO);

        // get IDs from submesh
        SMESH::long_array_var anElementsIds = new SMESH::long_array;
        if (aConstructorId == 0)
          anElementsIds = aSubMesh->GetElementsByType(SMESH::EDGE);
        else if (aConstructorId == 1)
          anElementsIds = aSubMesh->GetElementsByType(SMESH::FACE);

        for (int i = 0; i < anElementsIds->length(); i++)
          myElementsId += QString(" %1").arg(anElementsIds[i]);
        aNbUnits = anElementsIds->length();
      } else { // GROUP
        // get smesh group
        SMESH::SMESH_GroupBase_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);
        if (aGroup->_is_nil())
          return;

        if ((aConstructorId == 0 && aGroup->GetType()!= SMESH::EDGE) ||
            (aConstructorId == 1 && aGroup->GetType()!= SMESH::FACE))
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
  if (myEditCurrentArgument == (QWidget*)LineEditElements)
    LineEditElements->setText(aString);
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
void SMESHGUI_RevolutionDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  if (send == SelectElementsButton) {
    myEditCurrentArgument = (QWidget*)LineEditElements;
    SMESH::SetPointRepresentation(false);
    if (CheckBoxMesh->isChecked()) {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(ActorSelection);
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    } else {
      int aConstructorId = GetConstructorId();
      if (aConstructorId == 0)
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(EdgeSelection);
	}
      else if (aConstructorId == 1)
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(FaceSelection);
	}
    }
  } else if (send == SelectPointButton) {
    myEditCurrentArgument = (QWidget*)SpinBox_X;
    SMESH::SetPointRepresentation(true);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);
  } else if (send == SelectVectorButton) {
    myEditCurrentArgument = (QWidget*)SpinBox_DX;
    SMESH::SetPointRepresentation(true);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);
  } else {
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::DeactivateActiveDialog()
{
  if (ConstructorsBox->isEnabled()) {
    ConstructorsBox->setEnabled(false);
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
void SMESHGUI_RevolutionDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  ConstructorsBox->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  ConstructorsClicked(GetConstructorId());
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::enterEvent (QEvent*)
{
  if (!ConstructorsBox->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=======================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_RevolutionDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================
void SMESHGUI_RevolutionDlg::onSelectMesh (bool toSelectMesh)
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
  } else {
    int aConstructorId = GetConstructorId();
    if (aConstructorId == 0)
      {
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(EdgeSelection);
      }
    else if (aConstructorId == 0)
      {
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(FaceSelection);
      }

    LineEditElements->setReadOnly(false);
    onTextChange(LineEditElements->text());
  }

  SelectionIntoArgument();
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_RevolutionDlg::GetConstructorId()
{
  return GroupConstructors->checkedId();
}

//=================================================================================
// function : IsAxisOk()
// purpose  :
//=================================================================================
bool SMESHGUI_RevolutionDlg::IsAxisOk()
{
  return (SpinBox_DX->GetValue() != 0 ||
	  SpinBox_DY->GetValue() != 0 ||
	  SpinBox_DZ->GetValue() != 0);
}

//=================================================================================
// function : onVectorChanged()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::onVectorChanged()
{
  if (IsAxisOk()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  } else {
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
