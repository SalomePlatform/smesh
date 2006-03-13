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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
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

  SMESHGUI_ExtrusionDlgLayout->addWidget(GroupArguments, 1, 0);

  /* Initialisations */
  SpinBox_Dx->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  SpinBox_Dy->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  SpinBox_Dz->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);

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

  Init();

  /***************************************************************/
  // signals and slots connections
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  this, SLOT(ClickOnApply()));
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
  myElementsId = "";
  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

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
    QStringList aListElementsId = QStringList::split(" ", myElementsId, false);

    SMESH::long_array_var anElementsId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    SMESH::DirStruct aVector;
    aVector.PS.x = SpinBox_Dx->GetValue();
    aVector.PS.y = SpinBox_Dy->GetValue();
    aVector.PS.z = SpinBox_Dz->GetValue();

    long aNbSteps = (long)SpinBox_NbSteps->value();

    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      QApplication::setOverrideCursor(Qt::waitCursor);
      aMeshEditor->ExtrusionSweep(anElementsId.inout(), aVector, aNbSteps);
      QApplication::restoreOverrideCursor();
    } catch (...) {
    }

    SMESH::UpdateView();
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

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // hilight entered elements/nodes
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    QStringList aListId = QStringList::split(" ", theNewText, false);

    if (send == LineEditElements) {
      const Handle(SALOME_InteractiveObject)& anIO = myActor->getIO();
      TColStd_MapOfInteger newIndices;
      for (int i = 0; i < aListId.count(); i++) {
	const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt());
	if (e)
	  newIndices.Add(e->GetID());
	myNbOkElements++;
      }
      mySelector->AddOrRemoveIndex(anIO, newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->highlight( anIO, true, true );
      myElementsId = theNewText;
    }
  }

  if (myNbOkElements) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }

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
  QString aString = "";

  // set busy flag
  myBusy = true;

  myEditCurrentArgument->setText(aString);
  myNbOkElements = 0;
  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);
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

  myActor = SMESH::FindActorByObject(myMesh);
  if (!myActor)
    return;

  if (myEditCurrentArgument == LineEditElements) {
    int aNbElements = 0;
    myElementsId = "";

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
              aNbElements++;
            }
          }
        } else if (aConstructorId == 1) {
          SMDS_FaceIteratorPtr anIter = aSMDSMesh->facesIterator();
          while (anIter->more()) {
            const SMDS_MeshFace * face = anIter->next();
            if (face) {
              myElementsId += QString(" %1").arg(face->GetID());
              aNbElements++;
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
        aNbElements = anElementsIds->length();
      } else { // GROUP
        // get smesh group
        SMESH::SMESH_GroupBase_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);
        if (aGroup->_is_nil())
          return;

        if ((aConstructorId == 0 && aGroup->GetType() != SMESH::EDGE) ||
            (aConstructorId == 1 && aGroup->GetType()!= SMESH::FACE))
          return;

        // get IDs from smesh group
        SMESH::long_array_var anElementsIds = new SMESH::long_array;
        anElementsIds = aGroup->GetListOfID();
        for (int i = 0; i < anElementsIds->length(); i++)
          myElementsId += QString(" %1").arg(anElementsIds[i]);
        aNbElements = anElementsIds->length();
      }
    } else {
      aNbElements = SMESH::GetNameOfSelectedElements(mySelector, myActor->getIO(), aString);
      myElementsId = aString;
    }

    if (aNbElements < 1)
      return;

    myNbOkElements = true;
  }

  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myBusy = false;

  // OK
  if (myNbOkElements) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
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
