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
//  File   : SMESHGUI_RotationDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header:

#include "SMESHGUI_RotationDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"
#include "SMDS_Mesh.hxx"

#include "SUIT_Desktop.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "LightApp_Application.h"

#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SVTK_Selection.h"
#include "SALOME_ListIO.hxx"
#include "SALOMEDSClient_SObject.hxx"

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
#include <qlayout.h>
#include <qpixmap.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

using namespace std;

enum { MOVE_ELEMS_BUTTON = 0, COPY_ELEMS_BUTTON, MAKE_MESH_BUTTON }; //!< action type

//=================================================================================
// class    : SMESHGUI_RotationDlg()
// purpose  :
//=================================================================================
SMESHGUI_RotationDlg::SMESHGUI_RotationDlg( SMESHGUI* theModule, const char* name,
                                            bool modal, WFlags fl)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
               WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_MESH_ROTATION")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  if (!name)
    setName("SMESHGUI_RotationDlg");
  resize(303, 185);
  setCaption(tr("SMESH_ROTATION_TITLE"));
  setSizeGripEnabled(TRUE);
  SMESHGUI_RotationDlgLayout = new QGridLayout(this);
  SMESHGUI_RotationDlgLayout->setSpacing(6);
  SMESHGUI_RotationDlgLayout->setMargin(11);

  /***************************************************************/
  GroupConstructors = new QButtonGroup(this, "GroupConstructors");
  GroupConstructors->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5,
                                               (QSizePolicy::SizeType)0, 0, 0,
                                               GroupConstructors->sizePolicy().hasHeightForWidth()));
  GroupConstructors->setTitle(tr("SMESH_ROTATION"));
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  RadioButton1= new QRadioButton(GroupConstructors, "RadioButton1");
  RadioButton1->setText(tr("" ));
  RadioButton1->setPixmap(image0);
  RadioButton1->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1,
                                          (QSizePolicy::SizeType)0,
                                          RadioButton1->sizePolicy().hasHeightForWidth()));
  RadioButton1->setMinimumSize(QSize(50, 0));
  GroupConstructorsLayout->addWidget(RadioButton1, 0, 0);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupConstructorsLayout->addItem(spacer, 0, 1);
  SMESHGUI_RotationDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7,
                                          (QSizePolicy::SizeType)0, 0, 0,
                                          GroupButtons->sizePolicy().hasHeightForWidth()));
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr("" ));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  buttonHelp = new QPushButton(GroupButtons, "buttonHelp");
  buttonHelp->setText(tr("SMESH_BUT_HELP" ));
  buttonHelp->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonHelp, 0, 4);
  buttonCancel = new QPushButton(GroupButtons, "buttonCancel");
  buttonCancel->setText(tr("SMESH_BUT_CLOSE"));
  buttonCancel->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonCancel, 0, 3);
  buttonApply = new QPushButton(GroupButtons, "buttonApply");
  buttonApply->setText(tr("SMESH_BUT_APPLY"));
  buttonApply->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonApply, 0, 1);
  QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 2);
  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK"));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  SMESHGUI_RotationDlgLayout->addWidget(GroupButtons, 2, 0);

  /***************************************************************/
  GroupArguments = new QGroupBox(this, "GroupArguments");
  GroupArguments->setTitle(tr("SMESH_ARGUMENTS"));
  GroupArguments->setColumnLayout(0, Qt::Vertical);
  GroupArguments->layout()->setSpacing(0);
  GroupArguments->layout()->setMargin(0);
  GroupArgumentsLayout = new QGridLayout(GroupArguments->layout());
  GroupArgumentsLayout->setAlignment(Qt::AlignTop);
  GroupArgumentsLayout->setSpacing(6);
  GroupArgumentsLayout->setMargin(11);

  // Controls for elements selection
  TextLabelElements  = new QLabel(GroupArguments, "TextLabelElements");
  TextLabelElements->setText(tr("SMESH_ID_ELEMENTS" ));
  //TextLabelElements->setFixedWidth(74);
  GroupArgumentsLayout->addWidget(TextLabelElements, 0, 0);

  SelectElementsButton  = new QPushButton(GroupArguments, "SelectElementsButton");
  SelectElementsButton->setText(tr("" ));
  SelectElementsButton->setPixmap(image1);
  SelectElementsButton->setToggleButton(FALSE);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);

  LineEditElements = new QLineEdit(GroupArguments, "LineEditElements");
  LineEditElements->setValidator(new SMESHGUI_IdValidator(this, "validator"));
  GroupArgumentsLayout->addMultiCellWidget(LineEditElements, 0, 0, 2, 3);

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(GroupArguments, "CheckBoxMesh");
  CheckBoxMesh->setText(tr("SMESH_SELECT_WHOLE_MESH" ));
  GroupArgumentsLayout->addMultiCellWidget(CheckBoxMesh, 1, 1, 0, 3);

  // Controls for axis defining
  GroupAxis = new QGroupBox(GroupArguments, "GroupAxis");
  GroupAxis->setTitle(tr("SMESH_AXIS"));
  GroupAxis->setColumnLayout(0, Qt::Vertical);
  GroupAxis->layout()->setSpacing(0);
  GroupAxis->layout()->setMargin(0);
  QGridLayout* GroupAxisLayout = new QGridLayout(GroupAxis->layout());
  GroupAxisLayout->setAlignment(Qt::AlignTop);
  GroupAxisLayout->setSpacing(6);
  GroupAxisLayout->setMargin(11);

  TextLabelPoint = new QLabel(GroupAxis, "TextLabelPoint");
  TextLabelPoint->setText(tr("SMESH_POINT"));
  GroupAxisLayout->addWidget(TextLabelPoint, 0, 0);

  SelectPointButton  = new QPushButton(GroupAxis, "SelectPointButton");
  SelectPointButton->setPixmap(image1);
  GroupAxisLayout->addWidget(SelectPointButton, 0, 1);

  TextLabelX = new QLabel(GroupAxis, "TextLabelX");
  TextLabelX->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  TextLabelX->setText(tr("SMESH_X"));
  GroupAxisLayout->addWidget(TextLabelX, 0, 2);

  SpinBox_X = new SMESHGUI_SpinBox(GroupAxis, "SpinBox_X");
  GroupAxisLayout->addWidget(SpinBox_X, 0, 3);

  TextLabelY = new QLabel(GroupAxis, "TextLabelY");
  TextLabelY->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  TextLabelY->setText(tr("SMESH_Y"));
  GroupAxisLayout->addWidget(TextLabelY, 0, 4);

  SpinBox_Y = new SMESHGUI_SpinBox(GroupAxis, "SpinBox_Y");
  GroupAxisLayout->addWidget(SpinBox_Y, 0, 5);

  TextLabelZ = new QLabel(GroupAxis, "TextLabelZ");
  TextLabelZ->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  TextLabelZ->setText(tr("SMESH_Z"));
  GroupAxisLayout->addWidget(TextLabelZ, 0, 6);

  SpinBox_Z = new SMESHGUI_SpinBox(GroupAxis, "SpinBox_Z");
  GroupAxisLayout->addWidget(SpinBox_Z, 0, 7);

  TextLabelVector = new QLabel(GroupAxis, "TextLabelVector");
  TextLabelVector->setText(tr("SMESH_VECTOR"));
  GroupAxisLayout->addWidget(TextLabelVector, 1, 0);

  SelectVectorButton = new QPushButton(GroupAxis, "SelectVectorButton");
  SelectVectorButton->setPixmap(image1);
  GroupAxisLayout->addWidget(SelectVectorButton, 1, 1);

  TextLabelDX = new QLabel(GroupAxis, "TextLabelDX");
  TextLabelDX->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  TextLabelDX->setText(tr("SMESH_DX"));
  GroupAxisLayout->addWidget(TextLabelDX, 1, 2);

  SpinBox_DX = new SMESHGUI_SpinBox(GroupAxis, "SpinBox_DX");
  GroupAxisLayout->addWidget(SpinBox_DX, 1, 3);

  TextLabelDY = new QLabel(GroupAxis, "TextLabelDY");
  TextLabelDY->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  TextLabelDY->setText(tr("SMESH_DY"));
  GroupAxisLayout->addWidget(TextLabelDY, 1, 4);

  SpinBox_DY = new SMESHGUI_SpinBox(GroupAxis, "SpinBox_DY");
  GroupAxisLayout->addWidget(SpinBox_DY, 1, 5);

  TextLabelDZ = new QLabel(GroupAxis, "TextLabelDZ");
  TextLabelDZ->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  TextLabelDZ->setText(tr("SMESH_DZ"));
  GroupAxisLayout->addWidget(TextLabelDZ, 1, 6);

  SpinBox_DZ = new SMESHGUI_SpinBox(GroupAxis, "SpinBox_DZ");
  GroupAxisLayout->addWidget(SpinBox_DZ, 1, 7);

  GroupArgumentsLayout->addMultiCellWidget(GroupAxis, 2, 2, 0, 3);

  // Controls for angle defining
  TextLabelAngle = new QLabel(GroupArguments, "TextLabelAngle");
  TextLabelAngle->setText(tr("SMESH_ANGLE"));
  GroupArgumentsLayout->addMultiCellWidget(TextLabelAngle, 3, 3, 0, 2);

  SpinBox_Angle = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Angle");
  GroupArgumentsLayout->addWidget(SpinBox_Angle, 3, 2);

  // action switch
  ActionGroup = new QButtonGroup(1, Qt::Horizontal, GroupArguments, "ActionGroup");
  ActionGroup->setExclusive(true);
  ActionGroup->insert(new QRadioButton(tr("SMESH_MOVE_ELEMENTS"),ActionGroup), MOVE_ELEMS_BUTTON);
  ActionGroup->insert(new QRadioButton(tr("SMESH_COPY_ELEMENTS"),ActionGroup), COPY_ELEMS_BUTTON);
  ActionGroup->insert(new QRadioButton(tr("SMESH_CREATE_MESH"  ),ActionGroup), MAKE_MESH_BUTTON);
  GroupArgumentsLayout->addMultiCellWidget(ActionGroup, 4, 6, 0, 2);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck, 5, 3);

  // Name of a mesh to create
  LineEditNewMesh = new QLineEdit(GroupArguments, "LineEditNewMesh");
  GroupArgumentsLayout->addWidget(LineEditNewMesh, 6, 3);

  SMESHGUI_RotationDlgLayout->addWidget(GroupArguments, 1, 0);

  /* Initialisations */
  SpinBox_X->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Y->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_Z->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  SpinBox_DZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);

  SpinBox_Angle->RangeStepAndValidator(-360.0, +360.0, 5.0, 3);

  GroupArguments->show();
  myConstructorId = 0;
  RadioButton1->setChecked(TRUE);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Costruction of the logical filter
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QPtrList<SUIT_SelectionFilter> aListOfFilters;
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
  connect(GroupConstructors, SIGNAL(clicked(int)), SLOT(ConstructorsClicked(int)));

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
  connect(ActionGroup,      SIGNAL(clicked(int)),                   SLOT(onActionClicked(int)));

  this->show(); /* displays Dialog */

  ConstructorsClicked(0);
  //SelectionIntoArgument();
  onActionClicked(MOVE_ELEMS_BUTTON);
  resize(0,0); // ??
}

//=================================================================================
// function : ~SMESHGUI_RotationDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RotationDlg::~SMESHGUI_RotationDlg()
{
  // no need to delete child widgets, Qt does it all for us
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

    ((QRadioButton*) ActionGroup->find( MOVE_ELEMS_BUTTON ))->setChecked(TRUE);
    CheckBoxMesh->setChecked(false);
//     MakeGroupsCheck->setChecked(false);
//     MakeGroupsCheck->setEnabled(false);
//    onSelectMesh(false);
  }

  onSelectMesh(CheckBoxMesh->isChecked());
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RotationDlg::ConstructorsClicked (int constructorId)
{
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
    QStringList aListElementsId = QStringList::split(" ", myElementsId, false);

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
    int actionButton = ActionGroup->id( ActionGroup->selected() );
    bool makeGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );
    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      QApplication::setOverrideCursor(Qt::waitCursor);
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
                                      LineEditNewMesh->text().latin1());
      }
      QApplication::restoreOverrideCursor();
    } catch (...) {
    }

    SMESH::UpdateView();
    if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() ||
         actionButton == MAKE_MESH_BUTTON )
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
    Init(false);
    //ConstructorsClicked(GetConstructorId());
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
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
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
      
      QStringList aListId = QStringList::split(" ", theNewText, false);
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
    else if ( ActionGroup->id( ActionGroup->selected() ) != MOVE_ELEMS_BUTTON ) {
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
  this->ClickOnCancel();
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
  } else {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( CellSelection );
    LineEditElements->setReadOnly(false);
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
      name = meshSO->GetName();
    }
    if ( !name.isEmpty() )
      LineEditNewMesh->setText( SMESH::UniqueMeshName( name.latin1(), "rotated"));
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

  if ( e->key() == Key_F1 )
    {
      e->accept();
      ClickOnHelp();
    }
}
