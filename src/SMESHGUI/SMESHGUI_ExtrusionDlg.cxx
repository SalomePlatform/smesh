//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_ExtrusionDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header:

#include "SMESHGUI_ExtrusionDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_NumberFilter.hxx"
#include "SMESH_LogicalFilter.hxx"

#include "SMDS_Mesh.hxx"

#include "SUIT_ResourceMgr.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_Desktop.h"
#include "SUIT_MessageBox.h"
#include "SUIT_Session.h"

#include "LightApp_Application.h"

#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SVTK_Selection.h"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvalidator.h>
#include <qpixmap.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

using namespace std;

//=================================================================================
// function : SMESHGUI_ExtrusionDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_ExtrusionDlg::SMESHGUI_ExtrusionDlg (SMESHGUI* theModule,
					      bool modal)
     : QDialog( SMESH::GetDesktop( theModule ), "SMESHGUI_ExtrusionDlg", modal, WStyle_Customize |
                WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_EDGE")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_TRIANGLE")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  resize(303, 185);
  setCaption(tr("EXTRUSION_ALONG_LINE"));
  setSizeGripEnabled(TRUE);

  QGridLayout* SMESHGUI_ExtrusionDlgLayout = new QGridLayout(this);
  SMESHGUI_ExtrusionDlgLayout->setSpacing(6);
  SMESHGUI_ExtrusionDlgLayout->setMargin(11);

  /***************************************************************/
  GroupConstructors = new QButtonGroup("GroupConstructors", this);
  GroupConstructors->setTitle(tr("SMESH_EXTRUSION"));
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  QGridLayout* GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  RadioButton1= new QRadioButton(GroupConstructors, "RadioButton1");
  RadioButton1->setText(tr(""));
  RadioButton1->setPixmap(image0);
  GroupConstructorsLayout->addWidget(RadioButton1, 0, 0);
  RadioButton2= new QRadioButton(GroupConstructors, "RadioButton2");
  RadioButton2->setText(tr(""));
  RadioButton2->setPixmap(image1);
  GroupConstructorsLayout->addWidget(RadioButton2, 0, 2);
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr(""));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  QGridLayout* GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  buttonHelp = new QPushButton(GroupButtons, "buttonHelp");
  buttonHelp->setText(tr("SMESH_BUT_HELP" ));
  buttonHelp->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonHelp, 0, 4);
  buttonCancel = new QPushButton(GroupButtons, "buttonCancel");
  buttonCancel->setText(tr("SMESH_BUT_CLOSE" ));
  buttonCancel->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonCancel, 0, 3);
  buttonApply = new QPushButton(GroupButtons, "buttonApply");
  buttonApply->setText(tr("SMESH_BUT_APPLY" ));
  buttonApply->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonApply, 0, 1);
  QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 2);
  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK" ));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupButtons, 2, 0);

  /***************************************************************/
  GroupArguments = new QGroupBox(this, "GroupArguments");
  GroupArguments->setTitle(tr("EXTRUSION_1D"));
  GroupArguments->setColumnLayout(0, Qt::Vertical);
  GroupArguments->layout()->setSpacing(0);
  GroupArguments->layout()->setMargin(0);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments->layout());
  GroupArgumentsLayout->setAlignment(Qt::AlignTop);
  GroupArgumentsLayout->setSpacing(6);
  GroupArgumentsLayout->setMargin(11);

  // Controls for elements selection
  TextLabelElements  = new QLabel(GroupArguments, "TextLabelElements");
  TextLabelElements->setText(tr("SMESH_ID_ELEMENTS" ));
  TextLabelElements->setFixedWidth(74);
  GroupArgumentsLayout->addWidget(TextLabelElements, 0, 0);

  SelectElementsButton  = new QPushButton(GroupArguments, "SelectElementsButton");
  SelectElementsButton->setText(tr("" ));
  SelectElementsButton->setPixmap(image2);
  SelectElementsButton->setToggleButton(FALSE);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);

  LineEditElements  = new QLineEdit(GroupArguments, "LineEditElements");
  LineEditElements->setValidator(new SMESHGUI_IdValidator(this, "validator"));
  GroupArgumentsLayout->addMultiCellWidget(LineEditElements, 0, 0, 2, 7);

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(GroupArguments, "CheckBoxMesh");
  CheckBoxMesh->setText(tr("SMESH_SELECT_WHOLE_MESH" ));
  GroupArgumentsLayout->addMultiCellWidget(CheckBoxMesh, 1, 1, 0, 7);

  // Controls for vector selection
  TextLabelVector = new QLabel(GroupArguments, "TextLabelVector");
  TextLabelVector->setText(tr("SMESH_VECTOR"));
  GroupArgumentsLayout->addWidget(TextLabelVector, 2, 0);

  TextLabelDx = new QLabel(GroupArguments, "TextLabelDx");
  TextLabelDx->setText(tr("SMESH_DX"));
  GroupArgumentsLayout->addWidget(TextLabelDx, 2, 2);

  SpinBox_Dx = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Dx");
  GroupArgumentsLayout->addWidget(SpinBox_Dx, 2, 3);

  TextLabelDy = new QLabel(GroupArguments, "TextLabelDy");
  TextLabelDy->setText(tr("SMESH_DY"));
  GroupArgumentsLayout->addWidget(TextLabelDy, 2, 4);

  SpinBox_Dy = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Dy");
  GroupArgumentsLayout->addWidget(SpinBox_Dy, 2, 5);

  TextLabelDz = new QLabel(GroupArguments, "TextLabelDz");
  TextLabelDz->setText(tr("SMESH_DZ"));
  GroupArgumentsLayout->addWidget(TextLabelDz, 2, 6);

  SpinBox_Dz = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Dz");
  GroupArgumentsLayout->addWidget(SpinBox_Dz, 2, 7);

  // Controls for nb. steps defining
  TextLabelNbSteps = new QLabel(GroupArguments, "TextLabelNbSteps");
  TextLabelNbSteps->setText(tr("SMESH_NUMBEROFSTEPS" ));
  GroupArgumentsLayout->addMultiCellWidget(TextLabelNbSteps, 3, 3, 0, 1);

  SpinBox_NbSteps = new QSpinBox(GroupArguments, "SpinBox_NbSteps");
  GroupArgumentsLayout->addMultiCellWidget(SpinBox_NbSteps, 3, 3,  2, 7);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  MakeGroupsCheck->setChecked(true);
  GroupArgumentsLayout->addMultiCellWidget(MakeGroupsCheck, 4, 4,  0, 7);


  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupArguments, 1, 0);

  /* Initialisations */
  SpinBox_Dx->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Dy->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Dz->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);

  QIntValidator* anIntValidator = new QIntValidator(SpinBox_NbSteps);
  SpinBox_NbSteps->setValidator(anIntValidator);
  SpinBox_NbSteps->setRange(1, 999999);

  GroupArguments->show();
  RadioButton1->setChecked(TRUE);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox(this);

  // Costruction of the logical filter for the elements: mesh/sub-mesh/group
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QPtrList<SUIT_SelectionFilter> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "extrusion_page.html";

  Init();

  /***************************************************************/
  // signals and slots connections
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  this, SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()),  this, SLOT(ClickOnHelp()));

  // to update state of the Ok & Apply buttons
  connect(SpinBox_Dx, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dy, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));
  connect(SpinBox_Dz, SIGNAL(valueChanged(double)), SLOT(CheckIsEnable()));

  connect(GroupConstructors, SIGNAL(clicked(int)), SLOT(ConstructorsClicked(int)));
  connect(SelectElementsButton, SIGNAL (clicked()),            this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                  SLOT(onSelectMesh(bool)));

  /***************************************************************/
  
  this->show(); // displays Dialog

  ConstructorsClicked(0);
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionDlg::~SMESHGUI_ExtrusionDlg()
{
  // no need to delete child widgets, Qt does it all for us
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
      QApplication::setOverrideCursor(Qt::waitCursor);

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
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
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
    QStringList aListId = QStringList::split(" ", theNewText, false);

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
// purpose  : Activates this dialog
//=================================================================================
void SMESHGUI_ExtrusionDlg::ActivateThisDialog()
{
  // Emit a signal to deactivate the active dialog
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
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
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
//void SMESHGUI_ExtrusionDlg::closeEvent (QCloseEvent*)
//{
//  /* same than click on cancel button */
//  this->ClickOnCancel();
//}
//
//=================================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=================================================================================
//void SMESHGUI_ExtrusionDlg::hideEvent (QHideEvent*)
//{
//  if (!isMinimized())
//    ClickOnCancel();
//}

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
  if (GroupConstructors != NULL && GroupConstructors->selected() != NULL)
    return GroupConstructors->id(GroupConstructors->selected());
  return -1;
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

  if ( e->key() == Key_F1 )
    {
      e->accept();
      ClickOnHelp();
    }
}
