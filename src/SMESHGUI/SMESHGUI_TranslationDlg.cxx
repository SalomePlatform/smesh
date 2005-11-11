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
//  File   : SMESHGUI_TranslationDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header:

#include "SMESHGUI_TranslationDlg.h"

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
#include "SUIT_ResourceMgr.h"

#include "SVTK_ViewModel.h"
#include "SVTK_Selection.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
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
#include <qlayout.h>
#include <qspinbox.h>
#include <qpixmap.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

//=================================================================================
// class    : SMESHGUI_TranslationDlg()
// purpose  :
//=================================================================================
SMESHGUI_TranslationDlg::SMESHGUI_TranslationDlg( SMESHGUI* theModule, const char* name,
                                                  bool modal, WFlags fl)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
                WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_TRANSLATION_POINTS")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_TRANSLATION_VECTOR")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  if (!name)
    setName("SMESHGUI_TranslationDlg");
  resize(303, 185);
  setCaption(tr("SMESH_TRANSLATION"));
  setSizeGripEnabled(TRUE);
  SMESHGUI_TranslationDlgLayout = new QGridLayout(this);
  SMESHGUI_TranslationDlgLayout->setSpacing(6);
  SMESHGUI_TranslationDlgLayout->setMargin(11);

  /***************************************************************/
  GroupConstructors = new QButtonGroup(this, "GroupConstructors");
  GroupConstructors->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, GroupConstructors->sizePolicy().hasHeightForWidth()));
  GroupConstructors->setTitle(tr("SMESH_TRANSLATION" ));
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
  GroupConstructorsLayout->addWidget(RadioButton1, 0, 0);
  RadioButton2= new QRadioButton(GroupConstructors, "RadioButton2");
  RadioButton2->setText(tr("" ));
  RadioButton2->setPixmap(image1);
  GroupConstructorsLayout->addWidget(RadioButton2, 0, 2 );
  SMESHGUI_TranslationDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, GroupButtons->sizePolicy().hasHeightForWidth()));
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr("" ));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
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
  SMESHGUI_TranslationDlgLayout->addWidget(GroupButtons, 2, 0);

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

  // Controls for vector and points selection
  TextLabel1 = new QLabel(GroupArguments, "TextLabel1");
  GroupArgumentsLayout->addWidget(TextLabel1, 2, 0);

  SelectButton1  = new QPushButton(GroupArguments, "SelectButton1");
  SelectButton1->setText(tr("" ));
  SelectButton1->setPixmap(image2);
  SelectButton1->setToggleButton(FALSE);
  GroupArgumentsLayout->addWidget(SelectButton1, 2, 1);

  TextLabel1_1 = new QLabel(GroupArguments, "TextLabel1_1");
  GroupArgumentsLayout->addWidget(TextLabel1_1, 2, 2);

  SpinBox1_1 = new SMESHGUI_SpinBox(GroupArguments, "SpinBox1_1");
  GroupArgumentsLayout->addWidget(SpinBox1_1, 2, 3);

  TextLabel1_2 = new QLabel(GroupArguments, "TextLabel1_2");
  GroupArgumentsLayout->addWidget(TextLabel1_2, 2, 4);

  SpinBox1_2 = new SMESHGUI_SpinBox(GroupArguments, "SpinBox1_2");
  GroupArgumentsLayout->addWidget(SpinBox1_2, 2, 5);

  TextLabel1_3 = new QLabel(GroupArguments, "TextLabel1_3");
  GroupArgumentsLayout->addWidget(TextLabel1_3, 2, 6);

  SpinBox1_3 = new SMESHGUI_SpinBox(GroupArguments, "SpinBox1_3");
  GroupArgumentsLayout->addWidget(SpinBox1_3, 2, 7);

  TextLabel2 = new QLabel(GroupArguments, "TextLabel2");
  TextLabel2->setText(tr("SMESH_POINT_2" ));
  GroupArgumentsLayout->addWidget(TextLabel2, 3, 0);

  SelectButton2  = new QPushButton(GroupArguments, "SelectButton2");
  SelectButton2->setText(tr("" ));
  SelectButton2->setPixmap(image2);
  SelectButton2->setToggleButton(FALSE);
  GroupArgumentsLayout->addWidget(SelectButton2, 3, 1);

  TextLabel2_1 = new QLabel(GroupArguments, "TextLabel2_1");
  TextLabel2_1->setText(tr("SMESH_X" ));
  GroupArgumentsLayout->addWidget(TextLabel2_1, 3, 2);

  SpinBox2_1 = new SMESHGUI_SpinBox(GroupArguments, "SpinBox2_1");
  GroupArgumentsLayout->addWidget(SpinBox2_1, 3, 3);

  TextLabel2_2 = new QLabel(GroupArguments, "TextLabel2_2");
  TextLabel2_2->setText(tr("SMESH_Y" ));
  GroupArgumentsLayout->addWidget(TextLabel2_2, 3, 4);

  SpinBox2_2 = new SMESHGUI_SpinBox(GroupArguments, "SpinBox2_2");
  GroupArgumentsLayout->addWidget(SpinBox2_2, 3, 5);

  TextLabel2_3 = new QLabel(GroupArguments, "TextLabel2_3");
  TextLabel2_3->setText(tr("SMESH_Z"));
  GroupArgumentsLayout->addWidget(TextLabel2_3, 3, 6);

  SpinBox2_3 = new SMESHGUI_SpinBox(GroupArguments, "SpinBox2_3");
  GroupArgumentsLayout->addWidget(SpinBox2_3, 3, 7);

  // Controls for "Create a copy" option
  CheckBoxCopy = new QCheckBox(GroupArguments, "CheckBoxCopy");
  CheckBoxCopy->setText(tr("SMESH_CREATE_COPY"));
  GroupArgumentsLayout->addMultiCellWidget(CheckBoxCopy, 4, 4, 0, 2);


  SMESHGUI_TranslationDlgLayout->addWidget(GroupArguments, 1, 0);

  /* Initialisations */
  SpinBox1_1->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  SpinBox1_2->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  SpinBox1_3->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  SpinBox2_1->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  SpinBox2_2->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  SpinBox2_3->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);

  GroupArguments->show();
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
    new SMESH_LogicalFilter(aListOfFilters, SMESH_LogicalFilter::LO_OR);

  Init();

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  this, SLOT(ClickOnApply()));
  connect(GroupConstructors, SIGNAL(clicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectElementsButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton1,        SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton2,        SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                  SLOT(onSelectMesh(bool)));

  /* Move widget on the botton right corner of main widget */
  int x, y;
  mySMESHGUI->DefineDlgPosition(this, x, y);
  this->move(x, y);
  this->show(); /* displays Dialog */

  ConstructorsClicked(0);
  SelectionIntoArgument();
  resize(0,0); // ??
}

//=================================================================================
// function : ~SMESHGUI_TranslationDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_TranslationDlg::~SMESHGUI_TranslationDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::Init (bool ResetControls)
{
  myBusy = false;

  myEditCurrentArgument = 0;
  LineEditElements->clear();
  myElementsId = "";
  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();

  if (ResetControls) {
    SpinBox1_1->SetValue(0.0);
    SpinBox1_2->SetValue(0.0);
    SpinBox1_3->SetValue(0.0);
    SpinBox2_1->SetValue(0.0);
    SpinBox2_2->SetValue(0.0);
    SpinBox2_3->SetValue(0.0);

    CheckBoxCopy->setChecked(false);
    CheckBoxMesh->setChecked(false);
    onSelectMesh(false);
  }
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_TranslationDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  switch (constructorId) {
  case 0:
    {
      TextLabel1->setText(tr("SMESH_POINT_1"));
      TextLabel1_1->setText(tr("SMESH_X"));
      TextLabel1_2->setText(tr("SMESH_Y"));
      TextLabel1_3->setText(tr("SMESH_Z"));

      SelectButton1->show();
      TextLabel2->show();
      SelectButton2->show();
      TextLabel2_1->show();
      SpinBox2_1->show();
      TextLabel2_2->show();
      SpinBox2_2->show();
      TextLabel2_3->show();
      SpinBox2_3->show();
      break;
    }
  case 1:
    {
      TextLabel1->setText(tr("SMESH_VECTOR"));
      TextLabel1_1->setText(tr("SMESH_DX"));
      TextLabel1_2->setText(tr("SMESH_DY"));
      TextLabel1_3->setText(tr("SMESH_DZ"));

      SelectButton1->hide();
      TextLabel2->hide();
      SelectButton2->hide();
      TextLabel2_1->hide();
      SpinBox2_1->hide();
      TextLabel2_2->hide();
      SpinBox2_2->hide();
      TextLabel2_3->hide();
      SpinBox2_3->hide();
      break;
    }
  }

  if (myEditCurrentArgument != (QWidget*)LineEditElements) {
    SMESH::SetPointRepresentation(false);
    if (!CheckBoxMesh->isChecked())
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode( CellSelection );
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
void SMESHGUI_TranslationDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  if (myNbOkElements) {
    QStringList aListElementsId = QStringList::split(" ", myElementsId, false);

    SMESH::long_array_var anElementsId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    SMESH::DirStruct aVector;
    if (GetConstructorId() == 0) {
      aVector.PS.x = SpinBox2_1->GetValue() - SpinBox1_1->GetValue();
      aVector.PS.y = SpinBox2_2->GetValue() - SpinBox1_2->GetValue();
      aVector.PS.z = SpinBox2_3->GetValue() - SpinBox1_3->GetValue();
    } else if (GetConstructorId() == 1) {
      aVector.PS.x = SpinBox1_1->GetValue();
      aVector.PS.y = SpinBox1_2->GetValue();
      aVector.PS.z = SpinBox1_3->GetValue();
    }

    bool toCreateCopy = CheckBoxCopy->isChecked();

    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      QApplication::setOverrideCursor(Qt::waitCursor);
      aMeshEditor->Translate(anElementsId.inout(), aVector, toCreateCopy);
      QApplication::restoreOverrideCursor();
    } catch (...) {
    }

    SMESH::UpdateView();
    Init(false);
    ConstructorsClicked(GetConstructorId());
    SelectionIntoArgument();
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::ClickOnCancel()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  mySMESHGUI->ResetState();
  reject();
}

//=======================================================================
// function : onTextChange()
// purpose  :
//=======================================================================
void SMESHGUI_TranslationDlg::onTextChange (const QString& theNewText)
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
    Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
    
    TColStd_MapOfInteger newIndices;

    QStringList aListId = QStringList::split(" ", theNewText, false);

    if (send == LineEditElements) {
      for (int i = 0; i < aListId.count(); i++) {
	const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt());
	if (e)
	  newIndices.Add(e->GetID());
	myNbOkElements++;
      }
    }

    mySelector->AddOrRemoveIndex( anIO, newIndices, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( anIO, true, true );
    
    myElementsId = theNewText;
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
void SMESHGUI_TranslationDlg::SelectionIntoArgument()
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

    if (CheckBoxMesh->isChecked()) {
      SMESH::GetNameOfSelectedIObjects( mySelectionMgr, aString );

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
    aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aString);
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

    if (myEditCurrentArgument == (QWidget*)SpinBox1_1) {
      SpinBox1_1->SetValue(x);
      SpinBox1_2->SetValue(y);
      SpinBox2_3->SetValue(z);
    } else if (myEditCurrentArgument == (QWidget*)SpinBox2_1) {
      SpinBox2_1->SetValue(x);
      SpinBox2_2->SetValue(y);
      SpinBox2_3->SetValue(z);
    }
  }

  myBusy = true;
  if (myEditCurrentArgument == (QWidget*)LineEditElements)
    LineEditElements->setText(aString);
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
void SMESHGUI_TranslationDlg::SetEditCurrentArgument()
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
	aViewWindow->SetSelectionMode( ActorSelection );
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    } else {

      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode( CellSelection );
    }
  } else if (send == SelectButton1) {
    myEditCurrentArgument = (QWidget*)SpinBox1_1;
    SMESH::SetPointRepresentation(true);

    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( NodeSelection );
  } else if (send == SelectButton2) {
    myEditCurrentArgument = (QWidget*)SpinBox2_1;
    SMESH::SetPointRepresentation(true);

    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( NodeSelection );
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::DeactivateActiveDialog()
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
void SMESHGUI_TranslationDlg::ActivateThisDialog()
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
void SMESHGUI_TranslationDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  this->ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_TranslationDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================
void SMESHGUI_TranslationDlg::onSelectMesh (bool toSelectMesh)
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
      aViewWindow->SetSelectionMode( ActorSelection );
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
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_TranslationDlg::GetConstructorId()
{
  if (GroupConstructors != NULL && GroupConstructors->selected() != NULL)
    return GroupConstructors->id(GroupConstructors->selected());
  return -1;
}
