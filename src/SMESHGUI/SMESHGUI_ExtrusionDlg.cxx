// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_ExtrusionDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ExtrusionDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_MeshEditPreview.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>

#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Session.h>
#include <SUIT_OverrideCursor.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

#include <SalomeApp_IntSpinBox.h>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <gp_XYZ.hxx>

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

#define SPACING 6
#define MARGIN  11

//=================================================================================
// function : SMESHGUI_ExtrusionDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_ExtrusionDlg::SMESHGUI_ExtrusionDlg (SMESHGUI* theModule)
  : SMESHGUI_PreviewDlg( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myFilterDlg( 0 ),
    mySelectedObject(SMESH::SMESH_IDSource::_nil())
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_EDGE")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_TRIANGLE")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap image3 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_NODE")));

  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("EXTRUSION_ALONG_LINE"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_ExtrusionDlgLayout = new QVBoxLayout(this);
  SMESHGUI_ExtrusionDlgLayout->setSpacing(SPACING);
  SMESHGUI_ExtrusionDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_EXTRUSION"), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING);
  ConstructorsBoxLayout->setMargin(MARGIN);

  RadioButton0= new QRadioButton(ConstructorsBox);
  RadioButton0->setIcon(image3);
  RadioButton1= new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image0);
  RadioButton2= new QRadioButton(ConstructorsBox);
  RadioButton2->setIcon(image1);

  ConstructorsBoxLayout->addWidget(RadioButton0);
  ConstructorsBoxLayout->addWidget(RadioButton1);
  ConstructorsBoxLayout->addWidget(RadioButton2);

  GroupConstructors->addButton(RadioButton0, 0);
  GroupConstructors->addButton(RadioButton1, 1);
  GroupConstructors->addButton(RadioButton2, 2);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
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
  GroupArguments = new QGroupBox(tr("EXTRUSION_0D"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image2);

  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);
  LineEditElements->setMaxLength(-1);
  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myFilterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  RadioButton3 = new QRadioButton(GroupArguments);
  RadioButton3->setText( tr("SMESH_EXTRUSION_TO_DISTANCE") );
  RadioButton4 = new QRadioButton(GroupArguments);
  RadioButton4->setText( tr("SMESH_EXTRUSION_ALONG_VECTOR") );

  //Control for the Distance selection
  TextLabelDistance = new QLabel(tr("SMESH_DISTANCE"), GroupArguments);
  
  TextLabelDx = new QLabel(tr("SMESH_X"), GroupArguments);
  SpinBox_Dx = new SMESHGUI_SpinBox(GroupArguments);
  
  TextLabelDy = new QLabel(tr("SMESH_Y"), GroupArguments);
  SpinBox_Dy = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelDz = new QLabel(tr("SMESH_Z"), GroupArguments);
  SpinBox_Dz = new SMESHGUI_SpinBox(GroupArguments);

  // Controls for vector selection

  TextLabelVector = new QLabel(tr("SMESH_VECTOR"), GroupArguments);

  SelectVectorButton = new QPushButton(GroupArguments);
  SelectVectorButton->setIcon(image2);

  TextLabelVx = new QLabel(tr("SMESH_DX"), GroupArguments);
  SpinBox_Vx = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelVy = new QLabel(tr("SMESH_DY"), GroupArguments);
  SpinBox_Vy = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelVz = new QLabel(tr("SMESH_DZ"), GroupArguments);
  SpinBox_Vz = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelDist = new QLabel(tr("SMESH_DISTANCE"), GroupArguments);
  SpinBox_VDist = new SMESHGUI_SpinBox(GroupArguments);

  // Controls for nb. steps defining
  TextLabelNbSteps = new QLabel(tr("SMESH_NUMBEROFSTEPS"), GroupArguments);
  SpinBox_NbSteps = new SalomeApp_IntSpinBox(GroupArguments);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);

  //Preview check box
  myPreviewCheckBox = new QCheckBox(tr("PREVIEW"), GroupArguments);

  GroupArgumentsLayout->addWidget(TextLabelElements,    0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2, 1, 5);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 7);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 8);
  GroupArgumentsLayout->addWidget(RadioButton3,         2, 1, 1, 3);
  GroupArgumentsLayout->addWidget(RadioButton4,         2, 5, 1, 3);
  GroupArgumentsLayout->addWidget(TextLabelDistance,    3, 0);
  GroupArgumentsLayout->addWidget(TextLabelDx,          3, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Dx,           3, 3);
  GroupArgumentsLayout->addWidget(TextLabelDy,          3, 4);
  GroupArgumentsLayout->addWidget(SpinBox_Dy,           3, 5);
  GroupArgumentsLayout->addWidget(TextLabelDz,          3, 6);
  GroupArgumentsLayout->addWidget(SpinBox_Dz,           3, 7);
  GroupArgumentsLayout->addWidget(TextLabelVector,      4, 0);
  GroupArgumentsLayout->addWidget(SelectVectorButton,   4, 1);
  GroupArgumentsLayout->addWidget(TextLabelVx,          4, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Vx,           4, 3);
  GroupArgumentsLayout->addWidget(TextLabelVy,          4, 4);
  GroupArgumentsLayout->addWidget(SpinBox_Vy,           4, 5);
  GroupArgumentsLayout->addWidget(TextLabelVz,          4, 6);
  GroupArgumentsLayout->addWidget(SpinBox_Vz,           4, 7);
  GroupArgumentsLayout->addWidget(TextLabelDist,        5, 0);
  GroupArgumentsLayout->addWidget(SpinBox_VDist,         5, 3);
  GroupArgumentsLayout->addWidget(TextLabelNbSteps,     6, 0, 1, 3);
  GroupArgumentsLayout->addWidget(SpinBox_NbSteps,      6, 3);
  GroupArgumentsLayout->addWidget(myPreviewCheckBox,    7, 0, 1, 8);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      8, 0, 1, 8);
  GroupArgumentsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 8, 0);


  /***************************************************************/
  SMESHGUI_ExtrusionDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupArguments);
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox_Vx->RangeStepAndValidator(COORD_MIN, COORD_MAX, 0.01, "length_precision");
  SpinBox_Vy->RangeStepAndValidator(COORD_MIN, COORD_MAX, 0.01, "length_precision");
  SpinBox_Vz->RangeStepAndValidator(COORD_MIN, COORD_MAX, 0.01, "length_precision");

  SpinBox_Dx->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Dy->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Dz->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  
  SpinBox_NbSteps->setRange(1, 999999);
  SpinBox_VDist->RangeStepAndValidator(0, COORD_MAX, 10.0, "length_precision");

  RadioButton0->setChecked(true);
  RadioButton3->setChecked(true);
  MakeGroupsCheck->setChecked(true);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox(this);

  // Costruction of the logical filter for the elements: mesh/sub-mesh/group
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QList<SUIT_SelectionFilter*> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "extrusion_page.html";

  Init();

  /***************************************************************/
  // signals and slots connections
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(RadioButton3, SIGNAL(clicked()), this, SLOT(ClickOnRadio()));
  connect(RadioButton4, SIGNAL(clicked()), this, SLOT(ClickOnRadio()));

  // to update state of the Ok & Apply buttons
  connect(SpinBox_Vx, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Vy, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Vz, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dx, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dy, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dz, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));

  connect(GroupConstructors,    SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));
  connect(SelectElementsButton, SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectVectorButton,   SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI,           SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr,       SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,           SIGNAL(SignalCloseAllDialogs()),   this, SLOT(ClickOnCancel()));
  connect(LineEditElements,     SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,         SIGNAL(toggled(bool)),               SLOT(onSelectMesh(bool)));

  connect(SpinBox_Dx,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Dy,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Dz,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Vx,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Vy,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Vz,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_VDist,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_NbSteps,  SIGNAL(valueChanged(int)), this, SLOT(toDisplaySimulation()));

  //To Connect preview check box
  connectPreviewControl();

  /***************************************************************/
  
  ConstructorsClicked(0);
  ClickOnRadio();
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionDlg::~SMESHGUI_ExtrusionDlg()
{
  if ( myFilterDlg != 0 ) {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg;
  }
}

//=================================================================================
// function : Init()
// purpose  : initialization
//=================================================================================
void SMESHGUI_ExtrusionDlg::Init (bool ResetControls)
{
  myBusy = false;
  myIDs.clear();

  LineEditElements->clear();
  myNbOkElements = 0;

  myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();

  if (ResetControls) {
    SpinBox_NbSteps->setValue(1);
    SpinBox_VDist->setValue(10);
    SpinBox_Dx->SetValue(0);
    SpinBox_Dy->SetValue(0);
    SpinBox_Dz->SetValue(0);
    SpinBox_Vx->SetValue(0);
    SpinBox_Vy->SetValue(0);
    SpinBox_Vz->SetValue(0);

    CheckBoxMesh->setChecked(false);
    onSelectMesh(false);
    myPreviewCheckBox->setChecked(false);
    onDisplaySimulation(false);
  }

  CheckIsEnable();
}

//=================================================================================
// function : CheckIsEnable()
// purpose  : Check whether the Ok and Apply buttons should be enabled or not
//=================================================================================
void SMESHGUI_ExtrusionDlg::CheckIsEnable()
{  
  bool anIsEnable = myNbOkElements > 0 && isValuesValid();

  buttonOk->setEnabled(anIsEnable);
  buttonApply->setEnabled(anIsEnable);
}

//=================================================================================
// function : isValuesValid()
// purpose  : Return true in case if values entered into dialog are valid
//=================================================================================
bool SMESHGUI_ExtrusionDlg::isValuesValid() {
  double aX, aY, aZ, aModule = 0;
  if ( RadioButton3->isChecked() ) {
    aX = SpinBox_Dx->GetValue();
    aY = SpinBox_Dy->GetValue();
    aZ = SpinBox_Dz->GetValue();
    aModule = sqrt(aX*aX + aY*aY + aZ*aZ);
  } else   if ( RadioButton4->isChecked() ) {
    aX = SpinBox_Vx->GetValue();
    aY = SpinBox_Vy->GetValue();
    aZ = SpinBox_Vz->GetValue();
    aModule = sqrt(aX*aX + aY*aY + aZ*aZ);
  }
  return aModule > 1.0E-38;
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_ExtrusionDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  hidePreview();

  TextLabelElements->setText(tr( constructorId ? "SMESH_ID_ELEMENTS" : "SMESH_ID_NODES"));

  switch (constructorId) {
  case 0:
    {
      GroupArguments->setTitle(tr("EXTRUSION_0D"));
      if (!CheckBoxMesh->isChecked())
      {
        LineEditElements->clear();
        myIDs.clear();
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(NodeSelection);
      }
      break;
    }
  case 1:
    {
      GroupArguments->setTitle(tr("EXTRUSION_1D"));
      if (!CheckBoxMesh->isChecked())
      {
        LineEditElements->clear();
        myIDs.clear();
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(EdgeSelection);
      }
      break;
    }
  case 2:
    {
      GroupArguments->setTitle(tr("EXTRUSION_2D"));
      if (!CheckBoxMesh->isChecked())
      {
        LineEditElements->clear();
        myIDs.clear();
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(FaceSelection);
      }
      break;
    }
  }

  myEditCurrentArgument = (QWidget*)LineEditElements;
  LineEditElements->setFocus();

  if (CheckBoxMesh->isChecked())
    onSelectMesh(true);

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnRadio()
{
  if ( RadioButton3->isChecked() ) {
    TextLabelDistance->show();
    TextLabelDx->show();
    SpinBox_Dx->show();
    TextLabelDy->show();
    SpinBox_Dy->show();
    TextLabelDz->show();
    SpinBox_Dz->show();

    TextLabelVector->hide();
    TextLabelVx->hide();
    SpinBox_Vx->hide();
    TextLabelVy->hide();
    SpinBox_Vy->hide();
    TextLabelVz->hide();
    SpinBox_Vz->hide();
    TextLabelDist->hide();
    SpinBox_VDist->hide();
    SelectVectorButton->hide();
  } else if ( RadioButton4->isChecked() ) {
    TextLabelDistance->hide();
    TextLabelDx->hide();
    SpinBox_Dx->hide();
    TextLabelDy->hide();
    SpinBox_Dy->hide();
    TextLabelDz->hide();
    SpinBox_Dz->hide();

    TextLabelVector->show();
    TextLabelVx->show();
    SpinBox_Vx->show();
    TextLabelVy->show();
    SpinBox_Vy->show();
    TextLabelVz->show();
    SpinBox_Vz->show();
    TextLabelDist->show();
    SpinBox_VDist->show();
    SelectVectorButton->show();
  }
  onDisplaySimulation(true);
  // AdjustSize
  qApp->processEvents();
  updateGeometry();
  resize( minimumSizeHint() );
}

//=================================================================================
// function : ClickOnApply()
// purpose  : Called when user presses <Apply> button
//=================================================================================
bool SMESHGUI_ExtrusionDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (!isValid())
    return false;

  if (myNbOkElements) {

    SMESH::DirStruct aVector;
    getExtrusionVector(aVector);
    
    QStringList aParameters;
    if ( RadioButton3->isChecked() ) {
      aParameters << SpinBox_Dx->text();
      aParameters << SpinBox_Dy->text();
      aParameters << SpinBox_Dz->text();
    } else if ( RadioButton4->isChecked() ) {
      // only 3 coords in a python dump command :(
      // aParameters << SpinBox_Vx->text();
      // aParameters << SpinBox_Vy->text();
      // aParameters << SpinBox_Vz->text();
      // aParameters << SpinBox_VDist->text();
    }

    long aNbSteps = (long)SpinBox_NbSteps->value();

    aParameters << SpinBox_NbSteps->text();

    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

      myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

      if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() ) {
        if( CheckBoxMesh->isChecked() ) 
          switch (GetConstructorId() ) {
            case 0:
              {
                SMESH::ListOfGroups_var groups = 
                  aMeshEditor->ExtrusionSweepObject0DMakeGroups(mySelectedObject, aVector, aNbSteps);
                break;
              }
            case 1:
              {
                SMESH::ListOfGroups_var groups = 
                  aMeshEditor->ExtrusionSweepObject1DMakeGroups(mySelectedObject, aVector, aNbSteps);
                break;
              }
            case 2:
              {
                SMESH::ListOfGroups_var groups = 
                  aMeshEditor->ExtrusionSweepObject2DMakeGroups(mySelectedObject, aVector, aNbSteps);
                break;
              }
          }
        else
        {
          SMESH::ListOfGroups_var groups;
          if (GetConstructorId() == 0)
            groups = aMeshEditor->ExtrusionSweepMakeGroups0D(myElementsId.inout(), aVector, aNbSteps);
          else
            groups = aMeshEditor->ExtrusionSweepMakeGroups(myElementsId.inout(), aVector, aNbSteps);
        }

      }
      else {
        if( CheckBoxMesh->isChecked() ) 
          switch( GetConstructorId() ) {
            case 0:
              {
              aMeshEditor->ExtrusionSweepObject0D(mySelectedObject, aVector, aNbSteps);
                break;
              }
            case 1:
              {
              aMeshEditor->ExtrusionSweepObject1D(mySelectedObject, aVector, aNbSteps);
                break;
              }
            case 2:
              {
              aMeshEditor->ExtrusionSweepObject2D(mySelectedObject, aVector, aNbSteps);
                break;
          }
        }
        else
          if (GetConstructorId() == 0)
            aMeshEditor->ExtrusionSweep0D(myElementsId.inout(), aVector, aNbSteps);
          else
            aMeshEditor->ExtrusionSweep(myElementsId.inout(), aVector, aNbSteps);
      }

    } catch (...) {
    }

    SMESH::Update(myIO, SMESH::eDisplay);
    if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() )
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
    Init(false);
    ConstructorsClicked(GetConstructorId());
    mySelectionMgr->clearSelected();
    mySelectedObject = SMESH::SMESH_IDSource::_nil();
    SelectionIntoArgument();

    SMESHGUI::Modified();
  }
  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  : Called when user presses <OK> button
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnOk()
{
  if (ClickOnApply())
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  : Called when dialog box is closed
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnCancel()
{
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnHelp()
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

//=================================================================================
// function : onTextChange()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  // return if busy
  if (myBusy) return;

  // set busy flag
  myBusy = true;

  if (send == LineEditElements)
    myNbOkElements = 0;

  // hilight entered elements/nodes

  if (!myIO.IsNull()) {
    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);

    if (send == LineEditElements)
    {
      SMDS_Mesh* aMesh = myActor ? myActor->GetObject()->GetMesh() : 0;
      SMESH::ElementType SMESHType;
      SMDSAbs_ElementType SMDSType;
      switch (GetConstructorId()) {
      case 0:
        {
          SMESHType = SMESH::NODE;
          SMDSType = SMDSAbs_Node;
          break;
        }
      case 1:
        {
          SMESHType = SMESH::EDGE;
          SMDSType = SMDSAbs_Edge;
          break;                  
        }
      case 2:
        {
          SMESHType = SMESH::FACE;
          SMDSType = SMDSAbs_Face;
          break;
        }
      }
      myElementsId = new SMESH::long_array;
      myElementsId->length( aListId.count() );
      TColStd_MapOfInteger newIndices;
      for (int i = 0; i < aListId.count(); i++) {
        int id = aListId[ i ].toInt();
        bool validId = false;
        if ( id > 0 ) {
          if ( aMesh ) {
            const SMDS_MeshElement * e;
            if (SMDSType == SMDSAbs_Node)
              e = aMesh->FindNode( id ); 
            else
              e = aMesh->FindElement( id );
            validId = ( e && e->GetType() == SMDSType );
          } else {
            validId = ( myMesh->GetElementType( id, true ) == SMESHType );
          }
        }
        if ( validId && newIndices.Add( id ))
          myElementsId[ newIndices.Extent()-1 ] = id;
      }
      myElementsId->length( myNbOkElements = newIndices.Extent() );
      mySelector->AddOrRemoveIndex(myIO, newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->highlight( myIO, true, true );
    }
  }

  CheckIsEnable();

  onDisplaySimulation(true);

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_ExtrusionDlg::SelectionIntoArgument()
{
  if (myBusy) return;

  // return if dialog box is inactive
  if (!GroupButtons->isEnabled())
    return;

  // clear
  if(myEditCurrentArgument != (QWidget*)SpinBox_Vx) {
    myActor = 0;
    Handle(SALOME_InteractiveObject) resIO = myIO;
    myIO.Nullify();
  }

  QString aString = "";
  // set busy flag
  if(myEditCurrentArgument == (QWidget*)LineEditElements) {
    myBusy = true;
    LineEditElements->setText(aString);
    myNbOkElements = 0;
    myBusy = false;
  }
  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList, SVTK_Viewer::Type());
  int nbSel = SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();

  if(myEditCurrentArgument != (QWidget*)SpinBox_Vx) {
    myMesh = SMESH::GetMeshByIO(IO);
    if (myMesh->_is_nil())
      return;
    myIO = IO;
    myActor = SMESH::FindActorByObject(myMesh);
  }

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {    
    int aNbElements = 0;

    // MakeGroups is available if there are groups
    if ( myMesh->NbGroups() == 0 ) {
      MakeGroupsCheck->setChecked(false);
      MakeGroupsCheck->setEnabled(false);
    } else {
      MakeGroupsCheck->setEnabled(true);
    }

    if (CheckBoxMesh->isChecked()) {
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

      if (!SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO)->_is_nil())
        mySelectedObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      else
        return;
    } else {
      // get indices of selcted elements
      TColStd_IndexedMapOfInteger aMapIndex;
      mySelector->GetIndex(IO,aMapIndex);
      aNbElements = aMapIndex.Extent();

      if (aNbElements < 1)
        return;

      myElementsId = new SMESH::long_array;
      myElementsId->length( aNbElements );
      aString = "";
      for ( int i = 0; i < aNbElements; ++i )
        aString += QString(" %1").arg( myElementsId[ i ] = aMapIndex( i+1 ) );
    }

    myNbOkElements = true;

    myBusy = true;
    ((QLineEdit*)myEditCurrentArgument)->setText(aString);
    myBusy = false;
  }
  else if(myEditCurrentArgument == (QWidget*)SpinBox_Vx){
    TColStd_IndexedMapOfInteger aMapIndex;
    mySelector->GetIndex(IO,aMapIndex);
    int aNbElements = aMapIndex.Extent();
    SMESH::SMESH_Mesh_var aMesh_var = SMESH::GetMeshByIO(IO);
    SMESH_Actor* anActor = SMESH::FindActorByObject(aMesh_var);
    SMDS_Mesh* aMesh =  anActor ? anActor->GetObject()->GetMesh() : 0;

    if(aNbElements != 1 || !aMesh)
      return;
    
    const SMDS_MeshFace* face = dynamic_cast<const SMDS_MeshFace*>(aMesh->FindElement(aMapIndex(aNbElements)));

    if (!face)
      return;

    gp_XYZ aNormale = SMESH::getNormale(face);
    SpinBox_Vx->SetValue(aNormale.X());
    SpinBox_Vy->SetValue(aNormale.Y());
    SpinBox_Vz->SetValue(aNormale.Z());
    
  }
  
  onDisplaySimulation(true);
  
  // OK
  CheckIsEnable();
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  if (send == SelectElementsButton) {
    myEditCurrentArgument = (QWidget*)LineEditElements;
    if (CheckBoxMesh->isChecked()) {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(ActorSelection);
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    } else {
      int aConstructorId = GetConstructorId();
      switch(aConstructorId) {
          case 0:
          {   
            if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
              aViewWindow->SetSelectionMode(NodeSelection);
            break;
          }
          case 1:
          {
            if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
              aViewWindow->SetSelectionMode(EdgeSelection);
            break;
          }
          case 2:
          {
            if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
            aViewWindow->SetSelectionMode(FaceSelection);
            break;
          }
      }
    }
  }
  else if (send == SelectVectorButton){
    myEditCurrentArgument = (QWidget*)SpinBox_Vx;
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(FaceSelection);
  }
  
  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : Deactivates this dialog
//=================================================================================
void SMESHGUI_ExtrusionDlg::DeactivateActiveDialog()
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
// purpose  : Activates this dialog
//=================================================================================
void SMESHGUI_ExtrusionDlg::ActivateThisDialog()
{
  // Emit a signal to deactivate the active dialog
  mySMESHGUI->EmitSignalDeactivateDialog();
  ConstructorsBox->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox(this);

  ConstructorsClicked(GetConstructorId());
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  : Mouse enter event
//=================================================================================
void SMESHGUI_ExtrusionDlg::enterEvent (QEvent*)
{
  if (!ConstructorsBox->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::closeEvent( QCloseEvent* )
{
  /* same than click on cancel button */
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
    SMESH::SetPickable();
  }
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySMESHGUI->ResetState();
}

void SMESHGUI_ExtrusionDlg::reject()
{
  QDialog::reject();
  close();
}

//=================================================================================
// function : onSelectMesh()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::onSelectMesh (bool toSelectMesh)
{
  if (toSelectMesh) {
    myIDs = LineEditElements->text();
    TextLabelElements->setText(tr("SMESH_NAME"));
  }
  else
    TextLabelElements->setText(tr("SMESH_ID_ELEMENTS"));

  myFilterBtn->setEnabled(!toSelectMesh);

  if (myEditCurrentArgument != LineEditElements) {
    LineEditElements->clear();
    return;
  }

  mySelectionMgr->clearFilters();

  if (toSelectMesh) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    LineEditElements->setReadOnly(true);
    LineEditElements->setValidator(0);
  } else {
    int aConstructorId = GetConstructorId();
    switch(aConstructorId) {
      case 0:
        {
          if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
            aViewWindow->SetSelectionMode(NodeSelection);
          break;
        }
      case 1:
        {
          if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
            aViewWindow->SetSelectionMode(EdgeSelection);
          break;
        }
      case 2:
        {
          if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
            aViewWindow->SetSelectionMode(FaceSelection);
          break;
        }
    }
    LineEditElements->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(LineEditElements->text());
  }

  SelectionIntoArgument();

  if (!toSelectMesh)
    LineEditElements->setText( myIDs );
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_ExtrusionDlg::GetConstructorId()
{
  return GroupConstructors->checkedId();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}

//=================================================================================
// function : setFilters()
// purpose  : SLOT. Called when "Filter" button pressed.
//=================================================================================
void SMESHGUI_ExtrusionDlg::setFilters()
{
  if(myMesh->_is_nil()) {
    SUIT_MessageBox::critical(this,
                              tr("SMESH_ERROR"),
                              tr("NO_MESH_SELECTED"));
   return;
  }
  if ( !myFilterDlg )
  {
    QList<int> types;  
    types.append( SMESH::NODE );
    types.append( SMESH::EDGE );
    types.append( SMESH::FACE );
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, types );
  }
  switch( GetConstructorId() ){
    case 0: 
      {
      myFilterDlg->Init( SMESH::NODE );
        break;
      }
    case 1:
      {
      myFilterDlg->Init( SMESH::EDGE );
        break;
      }
    case 2:
      {
      myFilterDlg->Init( SMESH::FACE );
        break;
      }
  }
  

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( LineEditElements );

  myFilterDlg->show();
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_ExtrusionDlg::isValid()
{
  QString msg;
  bool ok = true;
  if ( RadioButton3->isChecked() ) {
    ok = SpinBox_Dx->isValid( msg, true ) && ok;
    ok = SpinBox_Dy->isValid( msg, true ) && ok;
    ok = SpinBox_Dz->isValid( msg, true ) && ok;
  } else if ( RadioButton4->isChecked() ) {
    ok = SpinBox_Vx->isValid( msg, true ) && ok;
    ok = SpinBox_Vy->isValid( msg, true ) && ok;
    ok = SpinBox_Vz->isValid( msg, true ) && ok;
    ok = SpinBox_VDist->isValid( msg, true ) && ok;
  }
  ok = SpinBox_NbSteps->isValid( msg, true ) && ok;

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }
  return true;
}

//=================================================================================
// function : onDisplaySimulation
// purpose  : Show/Hide preview
//=================================================================================
void SMESHGUI_ExtrusionDlg::onDisplaySimulation( bool toDisplayPreview ) {
  if (myPreviewCheckBox->isChecked() && toDisplayPreview) {
    if (myNbOkElements && isValid() && isValuesValid()) {
      //Get input vector
      SMESH::DirStruct aVector;
      getExtrusionVector(aVector);

      //Get Number of the steps 
      long aNbSteps = (long)SpinBox_NbSteps->value();
      
      try {
        SUIT_OverrideCursor aWaitCursor;
        SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditPreviewer();
        if( CheckBoxMesh->isChecked() ) {
          switch (GetConstructorId()) {
            case 0:
              {
                aMeshEditor->ExtrusionSweepObject0D(mySelectedObject, aVector, aNbSteps);
                                        break;
              }
            case 1:
              {
                aMeshEditor->ExtrusionSweepObject1D(mySelectedObject, aVector, aNbSteps);
                                        break;
              }
            case 2:
              {
                aMeshEditor->ExtrusionSweepObject2D(mySelectedObject, aVector, aNbSteps);
                                        break;
              }
          }
        }
        else
          if(GetConstructorId() == 0)
            aMeshEditor->ExtrusionSweep0D(myElementsId.inout(), aVector, aNbSteps);
          else
            aMeshEditor->ExtrusionSweep(myElementsId.inout(), aVector, aNbSteps);
        
        SMESH::MeshPreviewStruct_var aMeshPreviewStruct = aMeshEditor->GetPreviewData();
        mySimulation->SetData(aMeshPreviewStruct._retn());
      } catch (...) {
        hidePreview();
      }
    } else {
      hidePreview();
    }
  } else {
    hidePreview();
  }
}

//=================================================================================
// function : getExtrusionVector()
// purpose  : get direction of the extrusion
//=================================================================================
void SMESHGUI_ExtrusionDlg::getExtrusionVector(SMESH::DirStruct& aVector) {
  if ( RadioButton3->isChecked() ) {
    aVector.PS.x = SpinBox_Dx->GetValue();
    aVector.PS.y = SpinBox_Dy->GetValue();
    aVector.PS.z = SpinBox_Dz->GetValue();      
  } else if ( RadioButton4->isChecked() ) {
    gp_XYZ aNormale(SpinBox_Vx->GetValue(),
                    SpinBox_Vy->GetValue(),
                    SpinBox_Vz->GetValue());
    
    
    aNormale /= aNormale.Modulus();
    double aVDist = (double)SpinBox_VDist->value();
    
    aVector.PS.x = aNormale.X()*aVDist;
    aVector.PS.y = aNormale.Y()*aVDist;
    aVector.PS.z = aNormale.Z()*aVDist;
  }
}
