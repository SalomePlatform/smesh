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
// File   : SMESHGUI_RevolutionDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes

#include "SMESHGUI_RevolutionDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_FilterDlg.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>
#include <SUIT_OverrideCursor.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_IntSpinBox.h>

#include <SVTK_ViewWindow.h>
#include <SVTK_Selector.h>
#include <SALOME_ListIO.hxx>
#include <VTKViewer_CellLocationsArray.h>

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
#include <QMenu>

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
  : SMESHGUI_PreviewDlg( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myVectorDefinition(NONE_SELECT),
    myFilterDlg( 0 ),
    mySelectedObject(SMESH::SMESH_IDSource::_nil()),
    myActor(0)
{
  mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));

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

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image2);

  LineEditElements  = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);
  LineEditElements->setMaxLength(-1);
  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myFilterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

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

  // Controls for angle defining & number of steps defining
  GroupAngleBox = new QGroupBox(tr("SMESH_ANGLE"), GroupArguments);
  GroupAngle = new QButtonGroup(GroupAngleBox);
  QGridLayout* GroupAngleLayout = new QGridLayout(GroupAngleBox);
  GroupAngleLayout->setSpacing(6);
  GroupAngleLayout->setMargin(11);

  RadioButton3 = new QRadioButton(tr("ANGLE_BY_STEP"), GroupAngleBox);
  RadioButton4 = new QRadioButton(tr("TOTAL_ANGLE"),   GroupAngleBox);
  GroupAngle->addButton(RadioButton3, 0);
  GroupAngle->addButton(RadioButton4, 1);

  TextLabelAngle = new QLabel(tr("SMESH_ANGLE"), GroupAngleBox);
  SpinBox_Angle = new SMESHGUI_SpinBox(GroupAngleBox);

  TextLabelNbSteps = new QLabel(tr("SMESH_NUMBEROFSTEPS"), GroupAngleBox);
  SpinBox_NbSteps = new SalomeApp_IntSpinBox(GroupAngleBox);

  GroupAngleLayout->addWidget(RadioButton3,     0, 0);
  GroupAngleLayout->addWidget(RadioButton4,     0, 1);
  GroupAngleLayout->addWidget(TextLabelAngle,   1, 0);
  GroupAngleLayout->addWidget(SpinBox_Angle,    1, 1);
  GroupAngleLayout->addWidget(TextLabelNbSteps, 2, 0);
  GroupAngleLayout->addWidget(SpinBox_NbSteps,  2, 1);

  // Controls for tolerance defining
  TextLabelTolerance = new QLabel(tr("SMESH_TOLERANCE"), GroupArguments);
  SpinBox_Tolerance = new SMESHGUI_SpinBox(GroupArguments);

  // Control for mesh preview
  myPreviewCheckBox = new QCheckBox(tr("PREVIEW"), GroupArguments);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  MakeGroupsCheck->setChecked(true);

  GroupArgumentsLayout->addWidget(TextLabelElements,    0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 3);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 4);
  GroupArgumentsLayout->addWidget(GroupAxis,            2, 0, 1, 4);
  GroupArgumentsLayout->addWidget(GroupAngleBox,        3, 0, 1, 4);
  GroupArgumentsLayout->addWidget(TextLabelTolerance,   4, 0, 1, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Tolerance,    4, 2, 1, 2);
  GroupArgumentsLayout->addWidget(myPreviewCheckBox,    5, 0, 1, 4);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      6, 0, 1, 4);

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
  SMESHGUI_RevolutionDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_RevolutionDlgLayout->addWidget(GroupArguments);
  SMESHGUI_RevolutionDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox_X->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Y->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Z->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_DX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_DY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_DZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  RadioButton3->setChecked(true);

  SpinBox_Angle->RangeStepAndValidator(COORD_MIN, COORD_MAX, 5.0, "angle_precision");

  SpinBox_NbSteps->setRange(1, 999999);

  SpinBox_Tolerance->RangeStepAndValidator(0.0, COORD_MAX, 0.00001, "len_tol_precision");

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

  /*Create menu to vector selection*/
  SelectVectorMenu = new QMenu(this);
  myMenuActions[SelectVectorMenu->addAction( tr( "MEN_POINT_SELECT" ) )] = POINT_SELECT;
  myMenuActions[SelectVectorMenu->addAction( tr( "MEN_FACE_SELECT" ) )] = FACE_SELECT;
  connect( SelectVectorMenu, SIGNAL( triggered( QAction* ) ), SLOT( onSelectVectorMenu( QAction* ) ) );

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(GroupConstructors, SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectElementsButton, SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectPointButton,    SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectVectorButton,   SIGNAL(clicked()), this, SLOT(onSelectVectorButton()));

  connect(SpinBox_X,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Y,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Z,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));

  connect(SpinBox_DX, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));
  connect(SpinBox_DY, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));
  connect(SpinBox_DZ, SIGNAL(valueChanged(double)), this, SLOT(onVectorChanged()));

  connect(mySMESHGUI,     SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),      this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL(SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),   SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                 SLOT(onSelectMesh(bool)));

  connect(GroupAngle,        SIGNAL(buttonClicked(int)),   this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Angle,     SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_NbSteps,   SIGNAL(valueChanged(int)),    this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Tolerance, SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));

  //To Connect preview check box
  connectPreviewControl();

  connect(SpinBox_Angle, SIGNAL(textChanged(const QString&)), this, SLOT(onAngleTextChange(const QString&)));

  ConstructorsClicked(0);
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_RevolutionDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RevolutionDlg::~SMESHGUI_RevolutionDlg()
{
  delete mySimulation;
  if ( myFilterDlg ) {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg;
    myFilterDlg = 0;
  }
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
  myIDs.clear();

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
    myPreviewCheckBox->setChecked(false);
    onDisplaySimulation(false);
  }
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RevolutionDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  /*  SALOME_ListIO io;
  mySelectionMgr->selectedObjects( io );
  SALOME_ListIO aList;
  mySelectionMgr->setSelectedObjects( aList );*/

  buttonApply->setEnabled(false);
  buttonOk->setEnabled(false);
  mySimulation->SetVisibility(false);

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
      LineEditElements->clear();
      myIDs.clear();
      myNbOkElements = 0;
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(aSelMode);
    }

  myEditCurrentArgument = (QWidget*)LineEditElements;
  LineEditElements->setFocus();

  if (CheckBoxMesh->isChecked())
    onSelectMesh(true);

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  //  mySelectionMgr->setSelectedObjects( io );
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_RevolutionDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (!isValid())
    return false;

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

    double anAngle = (SpinBox_Angle->GetValue())*M_PI/180.;
    long aNbSteps = (long)SpinBox_NbSteps->value();
    double aTolerance = SpinBox_Tolerance->GetValue();

    if ( GroupAngle->checkedId() == 1 )
      anAngle = anAngle/aNbSteps;

    QStringList aParameters;
    aParameters << SpinBox_X->text();
    aParameters << SpinBox_Y->text();
    aParameters << SpinBox_Z->text();
    aParameters << SpinBox_DX->text();
    aParameters << SpinBox_DY->text();
    aParameters << SpinBox_DZ->text();
    aParameters << SpinBox_Angle->text();
    aParameters << SpinBox_NbSteps->text();
    aParameters << SpinBox_Tolerance->text();

    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      
      myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

      if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() ) {
        if( CheckBoxMesh->isChecked() ) {
          if( GetConstructorId() == 0 )
            SMESH::ListOfGroups_var groups = 
              aMeshEditor->RotationSweepObject1DMakeGroups(mySelectedObject, anAxis,
                                                           anAngle, aNbSteps, aTolerance);
          else
            SMESH::ListOfGroups_var groups = 
              aMeshEditor->RotationSweepObject2DMakeGroups(mySelectedObject, anAxis,
                                                           anAngle, aNbSteps, aTolerance);
        }
        else
          SMESH::ListOfGroups_var groups = 
            aMeshEditor->RotationSweepMakeGroups(anElementsId.inout(), anAxis,
                                                 anAngle, aNbSteps, aTolerance);
      }
      else {
        if( CheckBoxMesh->isChecked() ) {
          if( GetConstructorId() == 0 )
            aMeshEditor->RotationSweepObject1D(mySelectedObject, anAxis, anAngle, aNbSteps, aTolerance);
          else
            aMeshEditor->RotationSweepObject2D(mySelectedObject, anAxis, anAngle, aNbSteps, aTolerance);
        }
        else
          aMeshEditor->RotationSweep(anElementsId.inout(), anAxis, anAngle, aNbSteps, aTolerance);
      }

    } catch (...) {
    }

    SMESH::UpdateView();
    SMESH::Update(myIO, SMESH::eDisplay);
    if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() )
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
    Init(false);
    ConstructorsClicked(GetConstructorId());
    mySelectedObject = SMESH::SMESH_IDSource::_nil();
    SelectionIntoArgument();

    SMESHGUI::Modified();
  }

  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnOk()
{
  if( ClickOnApply() )
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnCancel()
{
  reject();
}

void SMESHGUI_RevolutionDlg::reject()
{
  close();
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
// function : onAngleTextChange()
// purpose  :
//=======================================================================
void SMESHGUI_RevolutionDlg::onAngleTextChange (const QString& theNewText)
{
  bool isNumber;
  SpinBox_Angle->text().toDouble( &isNumber );
  if( !isNumber )
    RadioButton3->setChecked( true );
  RadioButton4->setEnabled( isNumber );
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
  onDisplaySimulation(true);

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
  SMESH::SMESH_Mesh_var aMeshVar = SMESH::GetMeshByIO(IO);
  if (aMeshVar->_is_nil())
    return;

  SMESH_Actor* anActor = SMESH::FindActorByObject(aMeshVar);
  if (!anActor)
    anActor = SMESH::FindActorByEntry(IO->getEntry());
  if (!anActor && !CheckBoxMesh->isChecked())
    return;

  int aNbUnits = 0;

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    myElementsId = "";
    myMesh = aMeshVar;
    myActor = anActor;
    myIO = IO;

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
      aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, IO, aString);
      myElementsId = aString;
      if (aNbUnits < 1)
        return;
    }
    myNbOkElements = true;
  } else {

    SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
    if (!aMesh)
      return;

    bool isNodeSelected = (myEditCurrentArgument == (QWidget*)SpinBox_X ||
                           (myEditCurrentArgument == (QWidget*)SpinBox_DX && 
                            myVectorDefinition==POINT_SELECT));

    bool isFaceSelected = (myEditCurrentArgument == (QWidget*)SpinBox_DX && 
                           myVectorDefinition==FACE_SELECT);

    if(isNodeSelected) {
      aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
    }
    else if(isFaceSelected){
      aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, IO, aString);
    }
    
    if (aNbUnits != 1)
      return;

    if(isNodeSelected) {
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
    else if(isFaceSelected){
      const SMDS_MeshFace* face = dynamic_cast<const SMDS_MeshFace*>(aMesh->FindElement(aString.toInt()));
      if (!face)
        return;
      
      gp_XYZ aNormale = SMESH::getNormale(face);
      SpinBox_DX->SetValue(aNormale.X());
      SpinBox_DY->SetValue(aNormale.Y());
      SpinBox_DZ->SetValue(aNormale.Z());
      
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

  onDisplaySimulation(true);
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
    mySimulation->SetVisibility(false);
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
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
  }
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySMESHGUI->ResetState();
}

//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================
void SMESHGUI_RevolutionDlg::onSelectMesh (bool toSelectMesh)
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
    mySimulation->SetVisibility(false);
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

    LineEditElements->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(LineEditElements->text());
    mySimulation->SetVisibility(false);
  }

  SelectionIntoArgument();

  if (!toSelectMesh)
    LineEditElements->setText( myIDs );
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
  onDisplaySimulation(true);
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

//=================================================================================
// function : onDisplaySimulation()
// purpose  : Show/Hide preview
//=================================================================================
void SMESHGUI_RevolutionDlg::onDisplaySimulation(bool toDisplayPreview)
{
  if (myPreviewCheckBox->isChecked() && toDisplayPreview)
  {
    //display preview
    if (myNbOkElements && IsAxisOk())
    {
      QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);
      
      SMESH::long_array_var anElementsId = new SMESH::long_array;
      
      anElementsId->length(aListElementsId.count());
      for (int i = 0; i < aListElementsId.count(); i++)
        anElementsId[i] = aListElementsId[i].toInt();
      
      SMESH::AxisStruct anAxis;
      
      anAxis.x =  SpinBox_X->GetValue();
      anAxis.y =  SpinBox_Y->GetValue();
      anAxis.z =  SpinBox_Z->GetValue();
      anAxis.vx = SpinBox_DX->GetValue();
      anAxis.vy = SpinBox_DY->GetValue();
      anAxis.vz = SpinBox_DZ->GetValue();

      double anAngle = (SpinBox_Angle->GetValue())*M_PI/180.;
      long aNbSteps = (long)SpinBox_NbSteps->value();
      double aTolerance = SpinBox_Tolerance->GetValue();
      
      if (GroupAngle->checkedId() == 1)
        anAngle = anAngle/aNbSteps;
      
      try {
        SUIT_OverrideCursor aWaitCursor;
        SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditPreviewer();
        if( CheckBoxMesh->isChecked() ) {
          if( GetConstructorId() == 0 )
            aMeshEditor->RotationSweepObject1D(mySelectedObject, anAxis,
                                               anAngle, aNbSteps, aTolerance);
          else
            aMeshEditor->RotationSweepObject2D(mySelectedObject, anAxis,
                                               anAngle, aNbSteps, aTolerance);
        }
        else
          aMeshEditor->RotationSweep(anElementsId.inout(), 
                                     anAxis, 
                                     anAngle, 
                                     aNbSteps, 
                                     aTolerance);
        SMESH::MeshPreviewStruct_var aMeshPreviewStruct = aMeshEditor->GetPreviewData();
        mySimulation->SetData(aMeshPreviewStruct._retn());
      } catch (...) {}
    }
    else
    {
      mySimulation->SetVisibility(false);
    }
  }
  else
  {
    //erase preview
    mySimulation->SetVisibility(false);
  }
}

//=================================================================================
// function : onSelectVectorButton()
// purpose  : [slot]
//=================================================================================
void SMESHGUI_RevolutionDlg::onSelectVectorButton(){
  if(SelectVectorMenu) {
    SelectVectorMenu->exec( QCursor::pos() );
  }
}

//=================================================================================
// function : onSelectVectorMenu()
// purpose  : [slot]
//=================================================================================
void SMESHGUI_RevolutionDlg::onSelectVectorMenu( QAction* action){
  if(!action)
    return;

  disconnect(mySelectionMgr, 0, this, 0);

  switch(myMenuActions[action]) {
  case POINT_SELECT: 
    SMESH::SetPointRepresentation(true);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);
    break;
    
  case FACE_SELECT: 
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(FaceSelection);
    break;
  }

  myVectorDefinition = myMenuActions[action];
  myEditCurrentArgument = (QWidget*)SpinBox_DX;
  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : setFilters()
// purpose  : SLOT. Called when "Filter" button pressed.
//=================================================================================
void SMESHGUI_RevolutionDlg::setFilters()
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
    types.append( SMESH::EDGE );
    types.append( SMESH::FACE );
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, types );
  }
  myFilterDlg->Init( GetConstructorId() ? SMESH::FACE : SMESH::EDGE );

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( LineEditElements );

  myFilterDlg->show();
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_RevolutionDlg::isValid()
{
  QString msg;
  bool ok = true;
  ok = SpinBox_X->isValid( msg, true ) && ok;
  ok = SpinBox_Y->isValid( msg, true ) && ok;
  ok = SpinBox_Z->isValid( msg, true ) && ok;
  ok = SpinBox_DX->isValid( msg, true ) && ok;
  ok = SpinBox_DY->isValid( msg, true ) && ok;
  ok = SpinBox_DZ->isValid( msg, true ) && ok;
  ok = SpinBox_Angle->isValid( msg, true ) && ok;
  ok = SpinBox_NbSteps->isValid( msg, true ) && ok;
  ok = SpinBox_Tolerance->isValid( msg, true ) && ok;

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }
  return true;
}
