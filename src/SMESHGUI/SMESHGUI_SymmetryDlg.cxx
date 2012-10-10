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
//  File   : SMESHGUI_SymmetryDlg.cxx
//  Author : Michael ZORIN, Open CASCADE S.A.S.
//  SMESH includes

#include "SMESHGUI_SymmetryDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_MeshEditPreview.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

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

#define SPACING 6
#define MARGIN  11

//To disable automatic genericobj management, the following line should be commented.
//Otherwise, it should be uncommented. Refer to KERNEL_SRC/src/SALOMEDSImpl/SALOMEDSImpl_AttributeIOR.cxx
#define WITHGENERICOBJ

//=================================================================================
// class    : SMESHGUI_SymmetryDlg()
// purpose  :
//=================================================================================

SMESHGUI_SymmetryDlg::SMESHGUI_SymmetryDlg( SMESHGUI* theModule )
  : SMESHGUI_PreviewDlg( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myFilterDlg(0),
    mySelectedObject(SMESH::SMESH_IDSource::_nil())
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_SYMMETRY_POINT")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_SYMMETRY_AXIS")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_SYMMETRY_PLANE")));
  QPixmap image3 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_SYMMETRY"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_SymmetryDlgLayout = new QVBoxLayout(this);
  SMESHGUI_SymmetryDlgLayout->setSpacing(SPACING);
  SMESHGUI_SymmetryDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_SYMMETRY"), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING);
  ConstructorsBoxLayout->setMargin(MARGIN);

  RadioButton1 = new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image0);
  RadioButton2 = new QRadioButton(ConstructorsBox);
  RadioButton2->setIcon(image1);
  RadioButton3 = new QRadioButton(ConstructorsBox);
  RadioButton3->setIcon(image2);

  ConstructorsBoxLayout->addWidget(RadioButton1);
  ConstructorsBoxLayout->addWidget(RadioButton2);
  ConstructorsBoxLayout->addWidget(RadioButton3);
  GroupConstructors->addButton(RadioButton1, 0);
  GroupConstructors->addButton(RadioButton2, 1);
  GroupConstructors->addButton(RadioButton3, 2);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);
  SelectElementsButton  = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image3);
  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);
  LineEditElements->setMaxLength(-1);
  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myFilterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for mirror selection
  GroupMirror = new QGroupBox(GroupArguments);
  QGridLayout* GroupMirrorLayout = new QGridLayout(GroupMirror);
  GroupMirrorLayout->setSpacing(SPACING);
  GroupMirrorLayout->setMargin(MARGIN);

  TextLabelPoint = new QLabel(tr("SMESH_POINT"), GroupMirror);
  SelectPointButton  = new QPushButton(GroupMirror);
  SelectPointButton->setIcon(image3);

  TextLabelX = new QLabel(tr("SMESH_X"), GroupMirror);
  SpinBox_X = new SMESHGUI_SpinBox(GroupMirror);
  TextLabelY = new QLabel(tr("SMESH_Y"), GroupMirror);
  SpinBox_Y = new SMESHGUI_SpinBox(GroupMirror);
  TextLabelZ = new QLabel(tr("SMESH_Z"), GroupMirror);
  SpinBox_Z = new SMESHGUI_SpinBox(GroupMirror);

  TextLabelVector = new QLabel(GroupMirror);
  SelectVectorButton = new QPushButton(GroupMirror);
  SelectVectorButton->setIcon(image3);

  TextLabelDX = new QLabel(tr("SMESH_DX"), GroupMirror);
  SpinBox_DX = new SMESHGUI_SpinBox(GroupMirror);
  TextLabelDY = new QLabel(tr("SMESH_DY"), GroupMirror);
  SpinBox_DY = new SMESHGUI_SpinBox(GroupMirror);
  TextLabelDZ = new QLabel(tr("SMESH_DZ"), GroupMirror);
  SpinBox_DZ = new SMESHGUI_SpinBox(GroupMirror);

  GroupMirrorLayout->addWidget(TextLabelPoint,     0, 0);
  GroupMirrorLayout->addWidget(SelectPointButton,  0, 1);
  GroupMirrorLayout->addWidget(TextLabelX,         0, 2);
  GroupMirrorLayout->addWidget(SpinBox_X,          0, 3);
  GroupMirrorLayout->addWidget(TextLabelY,         0, 4);
  GroupMirrorLayout->addWidget(SpinBox_Y,          0, 5);
  GroupMirrorLayout->addWidget(TextLabelZ,         0, 6);
  GroupMirrorLayout->addWidget(SpinBox_Z,          0, 7);
  GroupMirrorLayout->addWidget(TextLabelVector,    1, 0);
  GroupMirrorLayout->addWidget(SelectVectorButton, 1, 1);
  GroupMirrorLayout->addWidget(TextLabelDX,        1, 2);
  GroupMirrorLayout->addWidget(SpinBox_DX,         1, 3);
  GroupMirrorLayout->addWidget(TextLabelDY,        1, 4);
  GroupMirrorLayout->addWidget(SpinBox_DY,         1, 5);
  GroupMirrorLayout->addWidget(TextLabelDZ,        1, 6);
  GroupMirrorLayout->addWidget(SpinBox_DZ,         1, 7);

  // switch of action type
  ActionBox = new QGroupBox(GroupArguments);
  ActionGroup = new QButtonGroup(GroupArguments);
  QVBoxLayout* ActionBoxLayout = new QVBoxLayout(ActionBox);
  ActionBoxLayout->addSpacing(SPACING);
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
  MakeGroupsCheck->setChecked(false);

  // Name of a mesh to create
  LineEditNewMesh = new QLineEdit(GroupArguments);


  //Preview check box
  myPreviewCheckBox = new QCheckBox(tr("PREVIEW"), GroupArguments);

  // layout
  GroupArgumentsLayout->addWidget(TextLabelElements,    0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2, 1, 1);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 3);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 4);
  GroupArgumentsLayout->addWidget(GroupMirror,          2, 0, 1, 4);
  GroupArgumentsLayout->addWidget(ActionBox,            3, 0, 3, 3);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      4, 3);
  GroupArgumentsLayout->addWidget(LineEditNewMesh,      5, 3);
  GroupArgumentsLayout->addWidget(myPreviewCheckBox,    6, 0);

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
  SMESHGUI_SymmetryDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_SymmetryDlgLayout->addWidget(GroupArguments);
  SMESHGUI_SymmetryDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox_X->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Y->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_Z->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_DX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_DY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_DZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  RadioButton1->setChecked(true);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Costruction of the logical filter
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (SMESH::MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (SMESH::GROUP);

  QList<SUIT_SelectionFilter*> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "symmetry_page.html";

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
  connect(ActionGroup,      SIGNAL(buttonClicked(int)),            SLOT(onActionClicked(int)));

  connect(SpinBox_X,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Y,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_Z,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_DX,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_DY,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox_DZ,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));

  //To Connect preview check box
  connectPreviewControl();

  ConstructorsClicked(0);
  SelectionIntoArgument();
  onActionClicked(MOVE_ELEMS_BUTTON);
}

//=================================================================================
// function : ~SMESHGUI_SymmetryDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_SymmetryDlg::~SMESHGUI_SymmetryDlg()
{
  if ( myFilterDlg != 0 ) {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg;
  }
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::Init (bool ResetControls)
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
    SpinBox_X->SetValue(0.0);
    SpinBox_Y->SetValue(0.0);
    SpinBox_Z->SetValue(0.0);
    SpinBox_DX->SetValue(0.0);
    SpinBox_DY->SetValue(0.0);
    SpinBox_DZ->SetValue(0.0);

    ActionGroup->button( MOVE_ELEMS_BUTTON )->setChecked(true);
    CheckBoxMesh->setChecked(false);
    myPreviewCheckBox->setChecked(false);
    onDisplaySimulation(false);

//     MakeGroupsCheck->setChecked(false);
//     MakeGroupsCheck->setEnabled(false);
    onSelectMesh(false);
  }
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_SymmetryDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  if (constructorId != 0 && !TextLabelVector->isVisible()) {
    TextLabelVector->show();
    SelectVectorButton->show();
    TextLabelDX->show();
    SpinBox_DX->show();
    TextLabelDY->show();
    SpinBox_DY->show();
    TextLabelDZ->show();
    SpinBox_DZ->show();
  }

  switch (constructorId) {
  case 0:
    {
      GroupMirror->setTitle(tr("SMESH_POINT"));

      TextLabelVector->hide();
      SelectVectorButton->hide();
      TextLabelDX->hide();
      SpinBox_DX->hide();
      TextLabelDY->hide();
      SpinBox_DY->hide();
      TextLabelDZ->hide();
      SpinBox_DZ->hide();
      break;
    }
  case 1:
    {
      GroupMirror->setTitle(tr("SMESH_AXIS"));
      TextLabelVector->setText(tr("SMESH_VECTOR"));
      break;
    }
  case 2:
    {
      GroupMirror->setTitle(tr("SMESH_PLANE"));
      TextLabelVector->setText(tr("SMESH_NORMAL"));
      break;
    }
  }

  if (myEditCurrentArgument != (QWidget*)LineEditElements) {
    SMESH::SetPointRepresentation(false);
    if (!CheckBoxMesh->isChecked())
      {
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(CellSelection);
      }
  }

  myEditCurrentArgument = (QWidget*)LineEditElements;
  LineEditElements->setFocus();

  if (CheckBoxMesh->isChecked())
    onSelectMesh(true);

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));

  onDisplaySimulation(true);

  QApplication::instance()->processEvents();
  updateGeometry();
  resize(100,100);
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_SymmetryDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if( !isValid() )
    return false;

  if (myNbOkElements && IsMirrorOk()) {
    QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);

    SMESH::long_array_var anElementsId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    SMESH::AxisStruct aMirror;
    SMESH::SMESH_MeshEditor::MirrorType aMirrorType;
    getMirror(aMirror,aMirrorType);

    QStringList aParameters;
    aParameters << SpinBox_X->text();
    aParameters << SpinBox_Y->text();
    aParameters << SpinBox_Z->text();
    aParameters << ( GetConstructorId() == 0 ? QString::number(0) : SpinBox_DX->text() );
    aParameters << ( GetConstructorId() == 0 ? QString::number(0) : SpinBox_DY->text() );
    aParameters << ( GetConstructorId() == 0 ? QString::number(0) : SpinBox_DZ->text() );

    int actionButton = ActionGroup->checkedId();
    bool makeGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );
    QStringList anEntryList;
    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

      myMesh->SetParameters(aParameters.join(":").toLatin1().constData());

      switch ( actionButton ) {
      case MOVE_ELEMS_BUTTON: {
        if(CheckBoxMesh->isChecked())
          aMeshEditor->MirrorObject(mySelectedObject, aMirror, aMirrorType, false );
        else
          aMeshEditor->Mirror(anElementsId, aMirror, aMirrorType, false );

        break;
      }
      case COPY_ELEMS_BUTTON: {
        SMESH::ListOfGroups_var groups;
        if ( makeGroups ) {
          if(CheckBoxMesh->isChecked())
            groups = aMeshEditor->MirrorObjectMakeGroups(mySelectedObject, aMirror, aMirrorType);
          else
            groups = aMeshEditor->MirrorMakeGroups(anElementsId, aMirror, aMirrorType);
        }
        else {
          if(CheckBoxMesh->isChecked())
            aMeshEditor->MirrorObject(mySelectedObject, aMirror, aMirrorType, true);
          else
            aMeshEditor->Mirror(anElementsId, aMirror, aMirrorType, true);
        }
        break;
        }
      case MAKE_MESH_BUTTON: {
        SMESH::SMESH_Mesh_var mesh;
        if (CheckBoxMesh->isChecked())
          mesh = aMeshEditor->MirrorObjectMakeMesh(mySelectedObject, aMirror, aMirrorType, makeGroups,
                                                   LineEditNewMesh->text().toLatin1().data());
        else
          mesh = aMeshEditor->MirrorMakeMesh(anElementsId, aMirror, aMirrorType, makeGroups,
                                             LineEditNewMesh->text().toLatin1().data());
          if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( mesh ) )
            anEntryList.append( aSObject->GetID().c_str() );
#ifdef WITHGENERICOBJ
          // obj has been published in study. Its refcount has been incremented.
          // It is safe to decrement its refcount
          // so that it will be destroyed when the entry in study will be removed
          mesh->UnRegister();
#endif
        }
        break;
      }
    } catch (...) {
    }

    SMESH::UpdateView();
    if ( ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() ) ||
         actionButton == MAKE_MESH_BUTTON ) {
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
      if( LightApp_Application* anApp =
          dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
        anApp->browseObjects( anEntryList, isApplyAndClose() );
    }
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
void SMESHGUI_SymmetryDlg::ClickOnOk()
{
  setIsApplyAndClose( true );
  if( ClickOnApply() )
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::ClickOnCancel()
{
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
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::ClickOnHelp()
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
void SMESHGUI_SymmetryDlg::onTextChange (const QString& theNewText)
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

    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);

    if (send == LineEditElements) {
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

  if (myNbOkElements &&  IsMirrorOk()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_SymmetryDlg::SelectionIntoArgument()
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
  if(myMesh->_is_nil())
    return;

  myActor = SMESH::FindActorByObject(myMesh);
  if (!myActor)
    myActor = SMESH::FindActorByEntry(IO->getEntry());
  if (!myActor && !CheckBoxMesh->isChecked())
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

      if (!SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO)->_is_nil()) { //MESH
        mySelectedObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      }
      else
        return;
      // get IDs from mesh
      /*
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
        /*
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
      */
    } else {
      aNbUnits = SMESH::GetNameOfSelectedElements( mySelector, IO, aString);
      myElementsId = aString;
      if (aNbUnits < 1)
        return;
    }

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
    LineEditElements->repaint();
    LineEditElements->setEnabled(false); // to update lineedit IPAL 19809
    LineEditElements->setEnabled(true);
    setNewMeshName();
  }
  myBusy = false;

  // OK
  if (myNbOkElements && IsMirrorOk()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
  onDisplaySimulation(true);
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::SetEditCurrentArgument()
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
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(CellSelection);
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
void SMESHGUI_SymmetryDlg::DeactivateActiveDialog()
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
void SMESHGUI_SymmetryDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  ConstructorsBox->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(CellSelection);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::enterEvent (QEvent*)
{
  if (!ConstructorsBox->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=======================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_SymmetryDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================
void SMESHGUI_SymmetryDlg::onSelectMesh (bool toSelectMesh)
{
  if (toSelectMesh)
    TextLabelElements->setText(tr("SMESH_NAME"));
  else
    TextLabelElements->setText(tr("SMESH_ID_ELEMENTS"));
  myFilterBtn->setEnabled(!toSelectMesh);

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
      aViewWindow->SetSelectionMode(CellSelection);
    LineEditElements->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(LineEditElements->text());
    hidePreview();
  }

  SelectionIntoArgument();
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_SymmetryDlg::GetConstructorId()
{
  return GroupConstructors->checkedId();
}

//=================================================================================
// function : IsMirrorOk()
// purpose  :
//=================================================================================
bool SMESHGUI_SymmetryDlg::IsMirrorOk()
{
  bool isOk = true;

  if (GetConstructorId() != 0)
    isOk = (SpinBox_DX->GetValue() != 0 ||
            SpinBox_DY->GetValue() != 0 ||
            SpinBox_DZ->GetValue() != 0);

  return isOk;
}

//=================================================================================
// function : onVectorChanged()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::onVectorChanged()
{
  if (IsMirrorOk()) {
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

void SMESHGUI_SymmetryDlg::onActionClicked(int button)
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
  toDisplaySimulation();
}

//=======================================================================
//function : setNewMeshName
//purpose  : update contents of LineEditNewMesh
//=======================================================================

void SMESHGUI_SymmetryDlg::setNewMeshName()
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
      LineEditNewMesh->setText( SMESH::UniqueMeshName( name, "mirrored"));
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SymmetryDlg::keyPressEvent( QKeyEvent* e )
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
void SMESHGUI_SymmetryDlg::setFilters()
{
  if(myMesh->_is_nil()) {
    SUIT_MessageBox::critical(this,
                              tr("SMESH_ERROR"),
                              tr("NO_MESH_SELECTED"));
   return;
  }
  if ( !myFilterDlg )
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, SMESH::ALL );

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( LineEditElements );

  myFilterDlg->show();
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_SymmetryDlg::isValid()
{
  bool ok = true;
  QString msg;

  ok = SpinBox_X->isValid( msg, true ) && ok;
  ok = SpinBox_Y->isValid( msg, true ) && ok;
  ok = SpinBox_Z->isValid( msg, true ) && ok;
  if (GetConstructorId() != 0) {
    ok = SpinBox_DX->isValid( msg, true ) && ok;
    ok = SpinBox_DY->isValid( msg, true ) && ok;
    ok = SpinBox_DZ->isValid( msg, true ) && ok;
  }

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
void SMESHGUI_SymmetryDlg::onDisplaySimulation( bool toDisplayPreview ) {
  if (myPreviewCheckBox->isChecked() && toDisplayPreview) {
    if ( myNbOkElements && isValid() && IsMirrorOk() ) {
      QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);      
      SMESH::long_array_var anElementsId = new SMESH::long_array;

      anElementsId->length(aListElementsId.count());
      for (int i = 0; i < aListElementsId.count(); i++)
        anElementsId[i] = aListElementsId[i].toInt();

      SMESH::AxisStruct aMirror;
      SMESH::SMESH_MeshEditor::MirrorType aMirrorType;
      
      getMirror(aMirror,aMirrorType);

      try {
        bool copy = ( ActionGroup->checkedId() == COPY_ELEMS_BUTTON ||
                      ActionGroup->checkedId() == MAKE_MESH_BUTTON );
        SUIT_OverrideCursor aWaitCursor;
        SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditPreviewer();
        if(CheckBoxMesh->isChecked())
          aMeshEditor->MirrorObject(mySelectedObject, aMirror, aMirrorType, copy );
        else
          aMeshEditor->Mirror(anElementsId, aMirror, aMirrorType, copy );
        
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
// function : getMirror
// purpose  : return mirror parameters
//=================================================================================
void SMESHGUI_SymmetryDlg::getMirror(SMESH::AxisStruct& theMirror, SMESH::SMESH_MeshEditor::MirrorType& theMirrorType) {
  theMirror.x =  SpinBox_X->GetValue();
  theMirror.y =  SpinBox_Y->GetValue();
  theMirror.z =  SpinBox_Z->GetValue();
  if (GetConstructorId() == 0) {
    theMirror.vx = theMirror.vy = theMirror.vz = 0;
  } else {
    theMirror.vx = SpinBox_DX->GetValue();
    theMirror.vy = SpinBox_DY->GetValue();
    theMirror.vz = SpinBox_DZ->GetValue();
  }
  if (GetConstructorId() == 0)
    theMirrorType = SMESH::SMESH_MeshEditor::POINT;
  if (GetConstructorId() == 1)
    theMirrorType = SMESH::SMESH_MeshEditor::AXIS;
  if (GetConstructorId() == 2)
    theMirrorType = SMESH::SMESH_MeshEditor::PLANE;
}
