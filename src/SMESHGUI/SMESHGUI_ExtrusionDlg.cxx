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
// File   : SMESHGUI_ExtrusionDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
//

// SMESH includes
#include "SMESHGUI_ExtrusionDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>

#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Session.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
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
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_EDGE")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_TRIANGLE")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

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

  RadioButton1= new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image0);
  RadioButton2= new QRadioButton(ConstructorsBox);
  RadioButton2->setIcon(image1);

  ConstructorsBoxLayout->addWidget(RadioButton1);
  ConstructorsBoxLayout->addWidget(RadioButton2);

  GroupConstructors->addButton(RadioButton1, 0);
  GroupConstructors->addButton(RadioButton2, 1);

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
  GroupArguments = new QGroupBox(tr("EXTRUSION_1D"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image2);

  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(new SMESHGUI_IdValidator(this));

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for vector selection
  TextLabelVector = new QLabel(tr("SMESH_VECTOR"), GroupArguments);

  TextLabelDx = new QLabel(tr("SMESH_DX"), GroupArguments);
  SpinBox_Dx = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelDy = new QLabel(tr("SMESH_DY"), GroupArguments);
  SpinBox_Dy = new SMESHGUI_SpinBox(GroupArguments);

  TextLabelDz = new QLabel(tr("SMESH_DZ"), GroupArguments);
  SpinBox_Dz = new SMESHGUI_SpinBox(GroupArguments);

  // Controls for nb. steps defining
  TextLabelNbSteps = new QLabel(tr("SMESH_NUMBEROFSTEPS"), GroupArguments);
  SpinBox_NbSteps = new QSpinBox(GroupArguments);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);

  GroupArgumentsLayout->addWidget(TextLabelElements,    0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2, 1, 6);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         0, 0, 1, 8);
  GroupArgumentsLayout->addWidget(TextLabelVector,      2, 0);
  GroupArgumentsLayout->addWidget(TextLabelDx,          2, 2);
  GroupArgumentsLayout->addWidget(SpinBox_Dx,           2, 3);
  GroupArgumentsLayout->addWidget(TextLabelDy,          2, 4);
  GroupArgumentsLayout->addWidget(SpinBox_Dy,           2, 5);
  GroupArgumentsLayout->addWidget(TextLabelDz,          2, 6);
  GroupArgumentsLayout->addWidget(SpinBox_Dz,           2, 7);
  GroupArgumentsLayout->addWidget(TextLabelNbSteps,     3, 0);
  GroupArgumentsLayout->addWidget(SpinBox_NbSteps,      3, 2, 1, 6);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      4, 0, 1, 8);

  /***************************************************************/
  SMESHGUI_ExtrusionDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupArguments);
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox_Dx->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Dy->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Dz->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);

  SpinBox_NbSteps->setRange(1, 999999);

  RadioButton1->setChecked(true);
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

  // to update state of the Ok & Apply buttons
  connect(SpinBox_Dx, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dy, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dz, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));

  connect(GroupConstructors,    SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));
  connect(SelectElementsButton, SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI,           SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr,       SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,           SIGNAL(SignalCloseAllDialogs()),   this, SLOT(ClickOnCancel()));
  connect(LineEditElements,     SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,         SIGNAL(toggled(bool)),               SLOT(onSelectMesh(bool)));

  /***************************************************************/
  
  ConstructorsClicked(0);
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionDlg::~SMESHGUI_ExtrusionDlg()
{
}

//=================================================================================
// function : Init()
// purpose  : initialization
//=================================================================================
void SMESHGUI_ExtrusionDlg::Init (bool ResetControls)
{
  myBusy = false;

  LineEditElements->clear();
  myNbOkElements = 0;

  myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();

  if (ResetControls) {
    SpinBox_NbSteps->setValue(1);
    SpinBox_Dx->SetValue(0);
    SpinBox_Dy->SetValue(0);
    SpinBox_Dz->SetValue(0);

    CheckBoxMesh->setChecked(false);
    onSelectMesh(false);
  }

  CheckIsEnable();
}

//=================================================================================
// function : CheckIsEnable()
// purpose  : Check whether the Ok and Apply buttons should be enabled or not
//=================================================================================
void SMESHGUI_ExtrusionDlg::CheckIsEnable()
{
  double aX = SpinBox_Dx->GetValue();
  double aY = SpinBox_Dy->GetValue();
  double aZ = SpinBox_Dz->GetValue();
  double aModule = sqrt(aX*aX + aY*aY + aZ*aZ);
  
  bool anIsEnable = myNbOkElements > 0 && aModule > 1.0E-38;

  buttonOk->setEnabled(anIsEnable);
  buttonApply->setEnabled(anIsEnable);
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_ExtrusionDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  switch (constructorId) {
  case 0:
    {
      GroupArguments->setTitle(tr("EXTRUSION_1D"));
      if (!CheckBoxMesh->isChecked())
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(EdgeSelection);
	}
      break;
    }
  case 1:
    {
      GroupArguments->setTitle(tr("EXTRUSION_2D"));
      if (!CheckBoxMesh->isChecked())
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(FaceSelection);
	}
      break;
    }
  }

  myEditCurrentArgument = LineEditElements;
  LineEditElements->setFocus();

  if (CheckBoxMesh->isChecked())
    onSelectMesh(true);

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
}

//=================================================================================
// function : ClickOnApply()
// purpose  : Called when user presses <Apply> button
//=================================================================================
bool SMESHGUI_ExtrusionDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (myNbOkElements) {

    SMESH::DirStruct aVector;
    aVector.PS.x = SpinBox_Dx->GetValue();
    aVector.PS.y = SpinBox_Dy->GetValue();
    aVector.PS.z = SpinBox_Dz->GetValue();

    long aNbSteps = (long)SpinBox_NbSteps->value();

    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      QApplication::setOverrideCursor(Qt::WaitCursor);

      if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() )
        SMESH::ListOfGroups_var groups = 
          aMeshEditor->ExtrusionSweepMakeGroups(myElementsId.inout(), aVector, aNbSteps);
      else
        aMeshEditor->ExtrusionSweep(myElementsId.inout(), aVector, aNbSteps);

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
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  SMESH::SetPickable(); // ???
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
      SMESH::ElementType SMESHType = GetConstructorId() ? SMESH::FACE : SMESH::EDGE;
      SMDSAbs_ElementType SMDSType = GetConstructorId() ? SMDSAbs_Face: SMDSAbs_Edge;

      myElementsId = new SMESH::long_array;
      myElementsId->length( aListId.count() );
      TColStd_MapOfInteger newIndices;
      for (int i = 0; i < aListId.count(); i++) {
        int id = aListId[ i ].toInt();
        bool validId = false;
        if ( id > 0 ) {
          if ( aMesh ) {
            const SMDS_MeshElement * e = aMesh->FindElement( id );
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
  myActor = 0;
  myIO.Nullify();
  QString aString = "";

  // set busy flag
  myBusy = true;

  myEditCurrentArgument->setText(aString);
  myNbOkElements = 0;
  myBusy = false;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList, SVTK_Viewer::Type());
  int nbSel = SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  myMesh = SMESH::GetMeshByIO(IO);
  if (myMesh->_is_nil())
    return;
  myIO = IO;
  myActor = SMESH::FindActorByObject(myMesh);

  if (myEditCurrentArgument == LineEditElements) {
    int aNbElements = 0;

    // MakeGroups is available if there are groups
    if ( myMesh->NbGroups() == 0 ) {
      MakeGroupsCheck->setChecked(false);
      MakeGroupsCheck->setEnabled(false);
    } else {
      MakeGroupsCheck->setEnabled(true);
    }

    if (CheckBoxMesh->isChecked()) {
      SMESH::ElementType neededType = GetConstructorId() ? SMESH::FACE : SMESH::EDGE;

      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

      SMESH::SMESH_Mesh_var mesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);

      if (!mesh->_is_nil()) { //MESH
        // get elements from mesh
          myElementsId = mesh->GetElementsByType(neededType);
          aNbElements = myElementsId->length();
      } else {
        SMESH::SMESH_subMesh_var aSubMesh =
          SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO);
        
        if (!aSubMesh->_is_nil()) { //SUBMESH
          // get IDs from submesh
          myElementsId = aSubMesh->GetElementsByType(neededType);
          aNbElements = myElementsId->length();
        } else {
          SMESH::SMESH_GroupBase_var aGroup = 
            SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);

          if (!aGroup->_is_nil() && aGroup->GetType() == neededType) { // GROUP
            // get IDs from smesh group
            myElementsId = aGroup->GetListOfID();
            aNbElements = myElementsId->length();
          }
        }
      }
    } else {
      // get indices of selcted elements
      TColStd_IndexedMapOfInteger aMapIndex;
      mySelector->GetIndex(IO,aMapIndex);
      aNbElements = aMapIndex.Extent();

      myElementsId = new SMESH::long_array;
      myElementsId->length( aNbElements );
      aString = "";
      for ( int i = 0; i < aNbElements; ++i )
        aString += QString(" %1").arg( myElementsId[ i ] = aMapIndex( i+1 ) );
    }

    if (aNbElements < 1)
      return;

    myNbOkElements = true;
  }

  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myBusy = false;

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
    myEditCurrentArgument = LineEditElements;
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
// function : onSelectMesh()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::onSelectMesh (bool toSelectMesh)
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
