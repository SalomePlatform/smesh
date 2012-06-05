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
//  File   : SMESHGUI_TranslationDlg.cxx
//  Author : Michael ZORIN, Open CASCADE S.A.S.
//  SMESH includes

#include "SMESHGUI_TranslationDlg.h"

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
#include <QSpinBox>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

enum { MOVE_ELEMS_BUTTON = 0, COPY_ELEMS_BUTTON, MAKE_MESH_BUTTON }; //!< action type

/*!
  \class BusyLocker
  \brief Simple 'busy state' flag locker.
  \internal
*/

class BusyLocker
{
public:
  //! Constructor. Sets passed boolean flag to \c true.
  BusyLocker( bool& busy ) : myBusy( busy ) { myBusy = true; }
  //! Destructor. Clear external boolean flag passed as parameter to the constructor to \c false.
  ~BusyLocker() { myBusy = false; }
private:
  bool& myBusy; //! External 'busy state' boolean flag
};

#define SPACING 6
#define MARGIN  11

//To disable automatic genericobj management, the following line should be commented.
//Otherwise, it should be uncommented. Refer to KERNEL_SRC/src/SALOMEDSImpl/SALOMEDSImpl_AttributeIOR.cxx
#define WITHGENERICOBJ

//=================================================================================
// class    : SMESHGUI_TranslationDlg()
// purpose  :
//=================================================================================
SMESHGUI_TranslationDlg::SMESHGUI_TranslationDlg( SMESHGUI* theModule ) : 
  SMESHGUI_PreviewDlg( theModule ),
  mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
  myFilterDlg(0),
  mySelectedObject(SMESH::SMESH_IDSource::_nil())
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_TRANSLATION_POINTS")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_TRANSLATION_VECTOR")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_TRANSLATION"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_TranslationDlgLayout = new QVBoxLayout(this);
  SMESHGUI_TranslationDlgLayout->setSpacing(SPACING);
  SMESHGUI_TranslationDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_TRANSLATION"), this);
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
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
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

  // Controls for vector and points selection
  TextLabel1 = new QLabel(GroupArguments);
  SelectButton1 = new QPushButton(GroupArguments);
  SelectButton1->setIcon(image2);

  TextLabel1_1 = new QLabel(GroupArguments);
  SpinBox1_1 = new SMESHGUI_SpinBox(GroupArguments);
  TextLabel1_2 = new QLabel(GroupArguments);
  SpinBox1_2 = new SMESHGUI_SpinBox(GroupArguments);
  TextLabel1_3 = new QLabel(GroupArguments);
  SpinBox1_3 = new SMESHGUI_SpinBox(GroupArguments);

  TextLabel2 = new QLabel(tr("SMESH_POINT_2"), GroupArguments);
  SelectButton2  = new QPushButton(GroupArguments);
  SelectButton2->setIcon(image2);

  TextLabel2_1 = new QLabel(tr("SMESH_X"), GroupArguments);
  SpinBox2_1 = new SMESHGUI_SpinBox(GroupArguments);
  TextLabel2_2 = new QLabel(tr("SMESH_Y"), GroupArguments);
  SpinBox2_2 = new SMESHGUI_SpinBox(GroupArguments);
  TextLabel2_3 = new QLabel(tr("SMESH_Z"), GroupArguments);
  SpinBox2_3 = new SMESHGUI_SpinBox(GroupArguments);

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
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2, 1, 5);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 7);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 8);
  GroupArgumentsLayout->addWidget(TextLabel1,           2, 0);
  GroupArgumentsLayout->addWidget(SelectButton1,        2, 1);
  GroupArgumentsLayout->addWidget(TextLabel1_1,         2, 2);
  GroupArgumentsLayout->addWidget(SpinBox1_1,           2, 3);
  GroupArgumentsLayout->addWidget(TextLabel1_2,         2, 4);
  GroupArgumentsLayout->addWidget(SpinBox1_2,           2, 5);
  GroupArgumentsLayout->addWidget(TextLabel1_3,         2, 6);
  GroupArgumentsLayout->addWidget(SpinBox1_3,           2, 7);
  GroupArgumentsLayout->addWidget(TextLabel2,           3, 0);
  GroupArgumentsLayout->addWidget(SelectButton2,        3, 1);
  GroupArgumentsLayout->addWidget(TextLabel2_1,         3, 2);
  GroupArgumentsLayout->addWidget(SpinBox2_1,           3, 3);
  GroupArgumentsLayout->addWidget(TextLabel2_2,         3, 4);
  GroupArgumentsLayout->addWidget(SpinBox2_2,           3, 5);
  GroupArgumentsLayout->addWidget(TextLabel2_3,         3, 6);
  GroupArgumentsLayout->addWidget(SpinBox2_3,           3, 7);
  GroupArgumentsLayout->addWidget(ActionBox,            4, 0, 3, 4);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      5, 5, 1, 4);
  GroupArgumentsLayout->addWidget(LineEditNewMesh,      6, 5, 1, 4);
  GroupArgumentsLayout->addWidget(myPreviewCheckBox,    7, 0);

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
  SMESHGUI_TranslationDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_TranslationDlgLayout->addWidget(GroupArguments);
  SMESHGUI_TranslationDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox1_1->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox1_2->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox1_3->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox2_1->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox2_2->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox2_3->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

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
    new SMESH_LogicalFilter(aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "translation_page.html";

  Init();

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(GroupConstructors, SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectElementsButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton1,        SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton2,        SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                  SLOT(onSelectMesh(bool)));
  connect(ActionGroup,      SIGNAL(buttonClicked(int)),             SLOT(onActionClicked(int)));

  connect(SpinBox1_1,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox1_2,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox1_3,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));

  connect(SpinBox2_1,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox2_2,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox2_3,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(SpinBox2_3,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));

  
  //To Connect preview check box
  connectPreviewControl();

  ConstructorsClicked(0);
  SelectionIntoArgument();
  onActionClicked(MOVE_ELEMS_BUTTON);
}

//=================================================================================
// function : ~SMESHGUI_TranslationDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_TranslationDlg::~SMESHGUI_TranslationDlg()
{
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

    ActionGroup->button( MOVE_ELEMS_BUTTON )->setChecked(true);
    CheckBoxMesh->setChecked(false);
//     MakeGroupsCheck->setChecked(false);
//     MakeGroupsCheck->setEnabled(false);
    myPreviewCheckBox->setChecked(false);
    onDisplaySimulation(false);
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

  onDisplaySimulation(true);

  QApplication::instance()->processEvents();
  updateGeometry();
  resize(100,100);
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_TranslationDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if( !isValid() )
    return false;

  if (myNbOkElements) {
    QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);

    SMESH::long_array_var anElementsId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    SMESH::DirStruct aVector;
    QStringList aParameters;
    if (GetConstructorId() == 0) {
      aVector.PS.x = SpinBox2_1->GetValue() - SpinBox1_1->GetValue();
      aVector.PS.y = SpinBox2_2->GetValue() - SpinBox1_2->GetValue();
      aVector.PS.z = SpinBox2_3->GetValue() - SpinBox1_3->GetValue();
      // not supported so far
      // aParameters << QString("%1 - %2").arg( SpinBox2_1->text() ).arg( SpinBox1_1->text() );
      // aParameters << QString("%1 - %2").arg( SpinBox2_2->text() ).arg( SpinBox1_2->text() );
      // aParameters << QString("%1 - %2").arg( SpinBox2_3->text() ).arg( SpinBox1_3->text() );
    } else if (GetConstructorId() == 1) {
      aVector.PS.x = SpinBox1_1->GetValue();
      aVector.PS.y = SpinBox1_2->GetValue();
      aVector.PS.z = SpinBox1_3->GetValue();
      aParameters << SpinBox1_1->text();
      aParameters << SpinBox1_2->text();
      aParameters << SpinBox1_3->text();
    }

    int actionButton = ActionGroup->checkedId();
    bool makeGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );
    QStringList anEntryList;
    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

      myMesh->SetParameters(aParameters.join(":").toLatin1().constData());

      switch ( actionButton ) {
      case MOVE_ELEMS_BUTTON:
        if(CheckBoxMesh->isChecked())
          aMeshEditor->TranslateObject(mySelectedObject, aVector, false);
        else
          aMeshEditor->Translate(anElementsId, aVector, false);
        break;
      case COPY_ELEMS_BUTTON:
        if ( makeGroups ) {
          SMESH::ListOfGroups_var groups;
          if(CheckBoxMesh->isChecked())
            groups = aMeshEditor->TranslateObjectMakeGroups(mySelectedObject,aVector);
          else
            groups = aMeshEditor->TranslateMakeGroups(anElementsId, aVector);
        }
        else {
          if(CheckBoxMesh->isChecked())
            aMeshEditor->TranslateObject(mySelectedObject, aVector, true);
          else
            aMeshEditor->Translate(anElementsId, aVector, true);
        }
        break;
      case MAKE_MESH_BUTTON: {
        SMESH::SMESH_Mesh_var mesh;
        if (CheckBoxMesh->isChecked())
          mesh = aMeshEditor->TranslateObjectMakeMesh(mySelectedObject, aVector, makeGroups,
                                                      LineEditNewMesh->text().toLatin1().data());
        else
          mesh = aMeshEditor->TranslateMakeMesh(anElementsId, aVector, makeGroups,
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
void SMESHGUI_TranslationDlg::ClickOnOk()
{
  setIsApplyAndClose( true );
  if( ClickOnApply() )
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
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
  }
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::ClickOnHelp()
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
void SMESHGUI_TranslationDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if (myBusy) return;
  BusyLocker lock( myBusy );

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
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_TranslationDlg::SelectionIntoArgument()
{
  if (myBusy) return;
  BusyLocker lock( myBusy );
  // clear
  myActor = 0;
  QString aString = "";

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    LineEditElements->setText(aString);
    myNbOkElements = 0;
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);
  }

  if (!GroupButtons->isEnabled()) // inactive
    return;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(IO);
  if (aMesh->_is_nil())
    return;

  SMESH_Actor* anActor = SMESH::FindActorByObject(aMesh);
  if (!anActor)
    anActor = SMESH::FindActorByEntry(IO->getEntry());

  if (!anActor && !CheckBoxMesh->isChecked())
      return;

  int aNbUnits = 0;

  if (myEditCurrentArgument == (QWidget*)LineEditElements)
  {
    myMesh = aMesh;
    myActor = anActor;

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
      SMESH::GetNameOfSelectedIObjects( mySelectionMgr, aString );

      if (!SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO)->_is_nil()) { //MESH, SUBMESH, OR GROUP
        mySelectedObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      }
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
    aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
    if (aNbUnits != 1)
      return;

    SMDS_Mesh* aMesh =  anActor->GetObject()->GetMesh();
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
      SpinBox1_3->SetValue(z);
    } else if (myEditCurrentArgument == (QWidget*)SpinBox2_1) {
      SpinBox2_1->SetValue(x);
      SpinBox2_2->SetValue(y);
      SpinBox2_3->SetValue(z);
    }
  }

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    LineEditElements->setText(aString);
    LineEditElements->repaint();
    LineEditElements->setEnabled(false); // to fully update lineedit IPAL 19809
    LineEditElements->setEnabled(true);
    setNewMeshName();
  }

  // OK
  if (myNbOkElements) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
  onDisplaySimulation(true);
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
void SMESHGUI_TranslationDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  ConstructorsBox->setEnabled(true);
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
  if (!ConstructorsBox->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
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
  myFilterBtn->setEnabled(!toSelectMesh);

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
    LineEditElements->setValidator(0);
  } else {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( CellSelection );
    LineEditElements->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(LineEditElements->text());
    hidePreview();
  }

  SelectionIntoArgument();
}

//=======================================================================
//function : onActionClicked
//purpose  : slot called when an action type changed
//=======================================================================

void SMESHGUI_TranslationDlg::onActionClicked(int button)
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

void SMESHGUI_TranslationDlg::setNewMeshName()
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
      LineEditNewMesh->setText( SMESH::UniqueMeshName( name, "translated"));
  }
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_TranslationDlg::GetConstructorId()
{
  return GroupConstructors->checkedId();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_TranslationDlg::keyPressEvent( QKeyEvent* e )
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
void SMESHGUI_TranslationDlg::setFilters()
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
bool SMESHGUI_TranslationDlg::isValid()
{
  bool ok = true;
  QString msg;

  ok = SpinBox1_1->isValid( msg, true ) && ok;
  ok = SpinBox1_2->isValid( msg, true ) && ok;
  ok = SpinBox1_3->isValid( msg, true ) && ok;
  if (GetConstructorId() == 0) {
    ok = SpinBox2_1->isValid( msg, true ) && ok;
    ok = SpinBox2_2->isValid( msg, true ) && ok;
    ok = SpinBox2_3->isValid( msg, true ) && ok;
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
void SMESHGUI_TranslationDlg::onDisplaySimulation( bool toDisplayPreview ) {
  if (myPreviewCheckBox->isChecked() && toDisplayPreview) {
    
    if (isValid() && myNbOkElements) {
      QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);
      
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

      try {
        bool copy = ( ActionGroup->checkedId() == COPY_ELEMS_BUTTON ||
                      ActionGroup->checkedId() == MAKE_MESH_BUTTON );
        SUIT_OverrideCursor aWaitCursor;
        SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditPreviewer();
        if(CheckBoxMesh->isChecked())
          aMeshEditor->TranslateObject(mySelectedObject, aVector, copy);
        else
          aMeshEditor->Translate(anElementsId, aVector, copy);
        
        SMESH::MeshPreviewStruct_var aMeshPreviewStruct = aMeshEditor->GetPreviewData();
        mySimulation->SetData(aMeshPreviewStruct._retn());      
      } catch (...) {
        
      }
    }
    else {
      hidePreview();
    }
  } else {
    hidePreview();
  }
}
