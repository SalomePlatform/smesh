//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_SmoothingDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SmoothingDlg.h"

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
#include <SUIT_OverrideCursor.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_Selector.h>
#include <SVTK_ViewWindow.h>
#include <SVTK_Selection.h>
#include <SALOME_ListIO.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QButtonGroup>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

//=================================================================================
// function : SMESHGUI_SmoothingDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_SmoothingDlg::SMESHGUI_SmoothingDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_SMOOTHING")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_SMOOTHING"));
  setSizeGripEnabled(true);

  /***************************************************************/
  QVBoxLayout* SMESHGUI_SmoothingDlgLayout = new QVBoxLayout(this);
  SMESHGUI_SmoothingDlgLayout->setSpacing(SPACING);
  SMESHGUI_SmoothingDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(tr("SMESH_SMOOTHING"), this);
  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* GroupConstructorsLayout = new QHBoxLayout(GroupConstructors);
  GroupConstructorsLayout->setSpacing(SPACING);
  GroupConstructorsLayout->setMargin(MARGIN);

  Constructor1 = new QRadioButton(GroupConstructors);
  Constructor1->setIcon(image0);
  Constructor1->setChecked(true);
  GroupConstructorsLayout->addWidget(Constructor1);
  ButtonGroup->addButton(Constructor1, 0);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image1);

  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for nodes selection
  TextLabelNodes = new QLabel(tr("FIXED_NODES_IDS"), GroupArguments);

  SelectNodesButton  = new QPushButton(GroupArguments);
  SelectNodesButton->setIcon(image1);

  LineEditNodes  = new QLineEdit(GroupArguments);
  LineEditNodes->setValidator(myIdValidator);

  // Controls for method selection
  TextLabelMethod = new QLabel(tr("METHOD"), GroupArguments);

  ComboBoxMethod = new QComboBox(GroupArguments);

  // Controls for iteration limit defining
  TextLabelLimit = new QLabel(tr("ITERATION_LIMIT"), GroupArguments);

  SpinBox_IterationLimit = new QSpinBox(GroupArguments);

  // Controls for max. aspect ratio defining
  TextLabelAspectRatio = new QLabel(tr("MAX_ASPECT_RATIO"), GroupArguments);

  SpinBox_AspectRatio = new SMESHGUI_SpinBox(GroupArguments);

  // Check box "Is Parametric"
  CheckBoxParametric = new QCheckBox( tr("IS_PARAMETRIC"), GroupArguments );

  GroupArgumentsLayout->addWidget(TextLabelElements,      0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton,   0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,       0, 2);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,           1, 0, 1, 3);
  GroupArgumentsLayout->addWidget(TextLabelNodes,         2, 0);
  GroupArgumentsLayout->addWidget(SelectNodesButton,      2, 1);
  GroupArgumentsLayout->addWidget(LineEditNodes,          2, 2);
  GroupArgumentsLayout->addWidget(TextLabelMethod,        3, 0);
  GroupArgumentsLayout->addWidget(ComboBoxMethod,         3, 2);
  GroupArgumentsLayout->addWidget(TextLabelLimit,         4, 0);
  GroupArgumentsLayout->addWidget(SpinBox_IterationLimit, 4, 2);
  GroupArgumentsLayout->addWidget(TextLabelAspectRatio,   5, 0);
  GroupArgumentsLayout->addWidget(SpinBox_AspectRatio,    5, 2);
  GroupArgumentsLayout->addWidget(CheckBoxParametric,     6, 0, 1, 3);

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
  SMESHGUI_SmoothingDlgLayout->addWidget(GroupConstructors);
  SMESHGUI_SmoothingDlgLayout->addWidget(GroupArguments);
  SMESHGUI_SmoothingDlgLayout->addWidget(GroupButtons);

  /***************************************************************/
  /* Initialisations */
  ComboBoxMethod->addItem(tr("LAPLACIAN"));
  ComboBoxMethod->addItem(tr("CENTROIDAL"));

  ComboBoxMethod->setCurrentIndex(0);

  CheckBoxParametric->setChecked( true );
  
  SpinBox_IterationLimit->setRange(1, 999999);
  SpinBox_IterationLimit->setValue(20);
  SpinBox_AspectRatio->RangeStepAndValidator(0.0, +999999.999, 0.1, 3);
  SpinBox_AspectRatio->SetValue(1.1);

  GroupArguments->show();
  myConstructorId = 0;
  Constructor1->setChecked(true);
  
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

  myHelpFileName = "smoothing_page.html";

  Init();

  /***************************************************************/
  // signals and slots connections
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  this, SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectElementsButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectNodesButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),
           SLOT(onTextChange(const QString&)));
  connect(LineEditNodes, SIGNAL(textChanged(const QString&)),
           SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh, SIGNAL(toggled(bool)),
           SLOT(onSelectMesh(bool)));
}

//=================================================================================
// function : ~SMESHGUI_SmoothingDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_SmoothingDlg::~SMESHGUI_SmoothingDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  : initialization
//=================================================================================
void SMESHGUI_SmoothingDlg::Init()
{
  myBusy = false;

//   ComboBoxMethod->setCurrentItem(0);

//   SpinBox_IterationLimit->setValue(20);
//   SpinBox_AspectRatio->SetValue(1.1);

  myEditCurrentArgument = LineEditElements;
  LineEditElements->setFocus();
  LineEditElements->clear();
  LineEditNodes->clear();
  myElementsId = "";
  myNbOkElements = 0;
  myNbOkNodes = 0;
  myActor     = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();

  CheckBoxMesh->setChecked(false);
  onSelectMesh(false);
}

//=================================================================================
// function : ClickOnApply()
// purpose  : Called when user presses <Apply> button
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  if (myNbOkElements && (myNbOkNodes || LineEditNodes->text().trimmed().isEmpty())) {
    QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);
    QStringList aListNodesId    = LineEditNodes->text().split(" ", QString::SkipEmptyParts);

    SMESH::long_array_var anElementsId = new SMESH::long_array;
    SMESH::long_array_var aNodesId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    if ( myNbOkNodes && aListNodesId.count() > 0 ) {
      aNodesId->length(aListNodesId.count());
      for (int i = 0; i < aListNodesId.count(); i++)
        aNodesId[i] = aListNodesId[i].toInt();
    } else {
      aNodesId->length(0);
    }

    long anIterationLimit = (long)SpinBox_IterationLimit->value();
    double aMaxAspectRatio = SpinBox_AspectRatio->GetValue();

    SMESH::SMESH_MeshEditor::Smooth_Method aMethod = SMESH::SMESH_MeshEditor::LAPLACIAN_SMOOTH;
    if (ComboBoxMethod->currentIndex() > 0)
      aMethod =  SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH;

    bool aResult = false;
    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

      if ( CheckBoxParametric->isChecked() )
	aResult = aMeshEditor->SmoothParametric(anElementsId.inout(), aNodesId.inout(),
						anIterationLimit, aMaxAspectRatio, aMethod);
      else
	aResult = aMeshEditor->Smooth(anElementsId.inout(), aNodesId.inout(),
				      anIterationLimit, aMaxAspectRatio, aMethod);
    } catch (...) {
    }

    if (aResult) {
      Handle(SALOME_InteractiveObject) anIO = myActor->getIO();

      SALOME_ListIO aList;
      aList.Append(anIO);
      mySelectionMgr->setSelectedObjects(aList, false);
      SMESH::UpdateView();
      Init();
    }
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  : Called when user presses <OK> button
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  : Called when dialog box is closed
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnCancel()
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
void SMESHGUI_SmoothingDlg::ClickOnHelp()
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
void SMESHGUI_SmoothingDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  // return if busy
  if (myBusy) return;

  // set busy flag
  myBusy = true;

  if (send == LineEditElements)
    myNbOkElements = 0;
  else if (send == LineEditNodes)
    myNbOkNodes = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // hilight entered elements/nodes
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    
    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);

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

    } else if (send == LineEditNodes) {
      TColStd_MapOfInteger newIndices;
      
      for (int i = 0; i < aListId.count(); i++) {
	const SMDS_MeshNode * n = aMesh->FindNode(aListId[ i ].toInt());
	if (n)
	  newIndices.Add(n->GetID());
	myNbOkNodes++;
      }
      mySelector->AddOrRemoveIndex(myActor->getIO(), newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->highlight( myActor->getIO(), true, true );
    }
  }

  if (myNbOkElements && (myNbOkNodes || LineEditNodes->text().trimmed().isEmpty())) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_SmoothingDlg::SelectionIntoArgument()
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
    myActor = 0;
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
    return;

  int aNbUnits = 0;

  if (myEditCurrentArgument == LineEditElements) {
    myElementsId = "";

    if (CheckBoxMesh->isChecked()) {
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

      if (!SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO)->_is_nil()) { //MESH
        // get IDs from mesh
        SMDS_Mesh* aSMDSMesh = myActor->GetObject()->GetMesh();
        if (!aSMDSMesh)
          return;

        for (int i = aSMDSMesh->MinElementID(); i <= aSMDSMesh->MaxElementID(); i++ ) {
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
  } else if (myEditCurrentArgument == LineEditNodes && !myMesh->_is_nil() && myActor) {
    myNbOkNodes = 0;
    aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
  } else {
  }

  if (aNbUnits < 1)
    return;

  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myEditCurrentArgument->repaint();
  myEditCurrentArgument->setEnabled(false); // to update lineedit IPAL 19809
  myEditCurrentArgument->setEnabled(true); 
  myBusy = false;

  // OK
  if (myEditCurrentArgument == LineEditElements)
    myNbOkElements = true;
  else if (myEditCurrentArgument == LineEditNodes)
    myNbOkNodes = true;

  if (myNbOkElements && (myNbOkNodes || LineEditNodes->text().trimmed().isEmpty())) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  switch (myConstructorId) {
  case 0: /* default constructor */
    {
      disconnect(mySelectionMgr, 0, this, 0);
      mySelectionMgr->clearSelected();
      mySelectionMgr->clearFilters();

      if (send == SelectElementsButton) {
        myEditCurrentArgument = LineEditElements;
        SMESH::SetPointRepresentation(false);
        if (CheckBoxMesh->isChecked()) {
	  //          mySelectionMgr->setSelectionModes(ActorSelection);
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(ActorSelection);
	  mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
        } else {
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(FaceSelection);
	}
      } else if (send == SelectNodesButton) {
	LineEditNodes->clear();
        myEditCurrentArgument = LineEditNodes;
        SMESH::SetPointRepresentation(true);
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI )) {
	  aViewWindow->SetSelectionMode(NodeSelection);
	}
      }

      myEditCurrentArgument->setFocus();
      connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
      SelectionIntoArgument();
      break;
    }
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : Deactivates this dialog
//=================================================================================
void SMESHGUI_SmoothingDlg::DeactivateActiveDialog()
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
void SMESHGUI_SmoothingDlg::ActivateThisDialog()
{
  // Emit a signal to deactivate the active dialog
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox(this);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(FaceSelection);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  : Mouse enter event
//=================================================================================
void SMESHGUI_SmoothingDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=======================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_SmoothingDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=======================================================================
// function : onSelectMesh()
// purpose  :
//=======================================================================
void SMESHGUI_SmoothingDlg::onSelectMesh (bool toSelectMesh)
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
    //    mySelectionMgr->setSelectionModes(ActorSelection);
    mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    LineEditElements->setReadOnly(true);
    LineEditElements->setValidator(0);
  } else {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(FaceSelection);
    LineEditElements->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(LineEditElements->text());
  }

  SelectionIntoArgument();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
