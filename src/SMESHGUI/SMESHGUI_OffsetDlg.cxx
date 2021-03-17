// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
//  File   : SMESHGUI_OffsetDlg.cxx

//  SMESH includes

#include "SMESHGUI_OffsetDlg.h"

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
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Tools.h>

// SALOME KERNEL includes
#include <SALOMEDSClient.hxx>
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

//=================================================================================
// class    : SMESHGUI_OffsetDlg()
// purpose  :
//=================================================================================
SMESHGUI_OffsetDlg::SMESHGUI_OffsetDlg( SMESHGUI* theModule ) :
  SMESHGUI_MultiPreviewDlg( theModule ),
  mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
  myFilterDlg(0)
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_MESH_OFFSET")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_OFFSET_TITLE"));
  setSizeGripEnabled(true);

  QVBoxLayout* dlgLayout = new QVBoxLayout(this);
  dlgLayout->setSpacing(SPACING);
  dlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_OFFSET"), this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING);
  ConstructorsBoxLayout->setMargin(MARGIN);

  QRadioButton* RadioButton1= new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image0);

  ConstructorsBoxLayout->addWidget(RadioButton1);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);
  // SelectElementsButton = new QPushButton(GroupArguments);
  // SelectElementsButton->setIcon(image1);
  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);
  LineEditElements->setMaxLength(-1);
  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myFilterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // offset
  QLabel* TextLabel1 = new QLabel(tr("OFFSET_VALUE"), GroupArguments);
  SpinBox = new SMESHGUI_SpinBox(GroupArguments);


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
  //GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2, 1, 5);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 7);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 8);
  GroupArgumentsLayout->addWidget(TextLabel1,           2, 0);
  GroupArgumentsLayout->addWidget(SpinBox,              2, 3);
  GroupArgumentsLayout->addWidget(ActionBox,            3, 0, 3, 4);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      4, 5, 1, 4);
  GroupArgumentsLayout->addWidget(LineEditNewMesh,      5, 5, 1, 4);
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
  dlgLayout->addWidget(ConstructorsBox);
  dlgLayout->addWidget(GroupArguments);
  dlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox->RangeStepAndValidator(COORD_MIN, COORD_MAX, 1.0, "length_precision");

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
    new SMESH_LogicalFilter(aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "Offset_page.html";

  Init();

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  //connect(SelectElementsButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),      this, SLOT(reject()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()),            this, SLOT(onCloseView()));

  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                  SLOT(onSelectMesh(bool)));
  connect(ActionGroup,      SIGNAL(buttonClicked(int)),             SLOT(onActionClicked(int)));

  connect(SpinBox,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));

  //To Connect preview check box
  connectPreviewControl();

  SelectionIntoArgument();
  onActionClicked(MOVE_ELEMS_BUTTON);
}

//=================================================================================
// function : ~SMESHGUI_OffsetDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_OffsetDlg::~SMESHGUI_OffsetDlg()
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
void SMESHGUI_OffsetDlg::Init (bool ResetControls)
{
  myBusy = false;
  myObjects.clear();
  myObjectsNames.clear();
  myMeshes.clear();

  LineEditElements->clear();
  myElementsId = "";
  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  myActor = 0;

  if (ResetControls)
  {
    SpinBox->SetValue(1.0);
    myPreviewCheckBox->setChecked(false);
    onDisplaySimulation(false);

    ActionGroup->button( MOVE_ELEMS_BUTTON )->setChecked(true);
    CheckBoxMesh->setChecked(true);
    onSelectMesh(true);
  }
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_OffsetDlg::ClickOnApply()
{
  if (mySMESHGUI->isStudyLocked())
    return false;

  if( !isValid() )
    return false;

  SUIT_OverrideCursor aWaitCursor;

  if (myNbOkElements)
  {
    QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);

    SMESH::smIdType_array_var anElementsId = new SMESH::smIdType_array;
    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    double offsetValue = SpinBox->value();

    QStringList aParameters;
    aParameters << SpinBox->text();

    int  actionButton = ActionGroup->checkedId();
    bool copyElements = ( actionButton == COPY_ELEMS_BUTTON );
    bool   makeGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );
    SMESH::ListOfGroups_var groups;
    SMESH::SMESH_Mesh_var mesh;
    QStringList anEntryList;
    try
    {
      switch ( actionButton ) {

      case MOVE_ELEMS_BUTTON:
        makeGroups = true;
        if ( CheckBoxMesh->isChecked() )
          for ( int i = 0; i < myObjects.count(); i++ ) {
            SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[i]->GetMeshEditor();
            myMeshes[i]->SetParameters( aParameters.join( ":" ).toLatin1().constData() );
            mesh = aMeshEditor->Offset( myObjects[i], offsetValue, makeGroups,
                                        copyElements, "", groups.out() );
          }
        else {
          SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[0]->GetMeshEditor();
          SMESH::IDSource_wrap src = aMeshEditor->MakeIDSource(anElementsId, SMESH::FACE);
          myMeshes[0]->SetParameters( aParameters.join( ":" ).toLatin1().constData() );
          mesh = aMeshEditor->Offset( src, offsetValue, makeGroups,
                                      copyElements, "", groups.out() );
        }
        break;

      case COPY_ELEMS_BUTTON:
        if ( CheckBoxMesh->isChecked() )
          for ( int i = 0; i < myObjects.count(); i++ ) {
            SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[i]->GetMeshEditor();
            myMeshes[i]->SetParameters(aParameters.join( ":" ).toLatin1().constData());
            mesh = aMeshEditor->Offset( myObjects[i], offsetValue, makeGroups,
                                        copyElements, "", groups.out() );
          }
        else {
          SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[0]->GetMeshEditor();
          SMESH::IDSource_wrap src = aMeshEditor->MakeIDSource(anElementsId, SMESH::FACE );
          myMeshes[0]->SetParameters(aParameters.join( ":" ).toLatin1().constData());
          mesh = aMeshEditor->Offset( src, offsetValue, makeGroups,
                                      copyElements, "", groups.out() );
        }
        break;

      case MAKE_MESH_BUTTON: {
        SMESH::SMESH_Mesh_var mesh;
        if ( CheckBoxMesh->isChecked() ) {
          for ( int i = 0; i < myObjects.count(); i++ ) {
            QString aName =
              SMESH::UniqueMeshName( LineEditNewMesh->text().replace( "*", myObjectsNames[i] ));
            SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[i]->GetMeshEditor();
            myMeshes[i]->SetParameters(aParameters.join( ":" ).toLatin1().constData());
            mesh = aMeshEditor->Offset( myObjects[i], offsetValue, makeGroups, copyElements,
                                        aName.toLatin1().data(), groups.out() );
            if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( mesh ))
              anEntryList.append( aSObject->GetID().c_str() );
          }
        }
        else {
          SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[0]->GetMeshEditor();
          myMeshes[0]->SetParameters(aParameters.join( ":" ).toLatin1().constData());
          SMESH::IDSource_wrap src = aMeshEditor->MakeIDSource(anElementsId, SMESH::FACE );
          mesh = aMeshEditor->Offset( src, offsetValue, makeGroups, copyElements,
                                      LineEditNewMesh->text().toLatin1().data(), groups.out() );
          if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( mesh ) )
            anEntryList.append( aSObject->GetID().c_str() );
        }
        break;
      }
      }
    }
    catch ( const SALOME::SALOME_Exception& S_ex ) {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
    }
    catch (...) {
    }

    for ( int i = 0; i < myObjects.count(); i++ ) {
      SMESH_Actor* actor = SMESH::FindActorByObject( myObjects[i] );
      if ( actor ) SMESH::Update( actor->getIO(), true );
    }

    if ( makeGroups || actionButton == MAKE_MESH_BUTTON )
    {
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
      if( LightApp_Application* anApp =
          dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
        anApp->browseObjects( anEntryList, isApplyAndClose() );
    }
    if ( !isApplyAndClose() )
    {
      Init(false);
      SelectionIntoArgument();
    }
    SMESHGUI::Modified();
  }

  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_OffsetDlg::ClickOnOk()
{
  setIsApplyAndClose( true );
  if( ClickOnApply() )
    reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_OffsetDlg::reject()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
  }
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_OffsetDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_OffsetDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_OffsetDlg::ClickOnHelp()
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

void SMESHGUI_OffsetDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) return;
  BusyLocker lock( myBusy );

  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // highlight entered elements
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    Handle(SALOME_InteractiveObject) anIO = myActor->getIO();

    SVTK_TVtkIDsMap newIndices;

    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
    for (int i = 0; i < aListId.count(); i++)
    {
      if ( const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt()))
        newIndices.Add( e->GetID() );
      myNbOkElements++;
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
void SMESHGUI_OffsetDlg::SelectionIntoArgument()
{
  if (myBusy) return;
  if (myFilterDlg && myFilterDlg->isVisible()) return; // filter dlg active

  BusyLocker lock( myBusy );

  // clear
  myActor = 0;
  QString aString = "";
  onDisplaySimulation(false);

  LineEditElements->setText(aString);
  myNbOkElements = 0;
  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  if (!GroupButtons->isEnabled()) // inactive
    return;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  int nbSel = aList.Extent();
  if (nbSel < 1)
    return;

  myElementsId = "";
  myObjects.clear();
  myObjectsNames.clear();
  myMeshes.clear();

  for ( SALOME_ListIteratorOfListIO it( aList ); it.More(); it.Next() )
  {
    Handle(SALOME_InteractiveObject) IO = it.Value();
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( IO );
    if ( aMesh->_is_nil() || aMesh->NbFaces() == 0 )
      return;

    myActor = SMESH::FindActorByObject( aMesh );
    if ( !myActor )
      myActor = SMESH::FindActorByEntry( IO->getEntry() );

    SMESH::SMESH_IDSource_var idSrc = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
    if ( _PTR(SObject) obj = SMESH::FindSObject( idSrc ))
    {
      std::string name = obj->GetName();
      if ( !name.empty() )
      {
        myObjects << idSrc;
        myObjectsNames << name.c_str();
        myMeshes << aMesh;
      }
    }
  }

  // MakeGroups is available if there are groups and "Copy"
  int aNbGroups = 0;
  for ( int i = 0; i < myMeshes.count(); i++ )
    aNbGroups += myMeshes[i]->NbGroups();

  if ( aNbGroups == 0 ) {
    MakeGroupsCheck->setChecked(false);
    MakeGroupsCheck->setEnabled(false);
  }
  else if ( ActionGroup->checkedId() != MOVE_ELEMS_BUTTON ) {
    MakeGroupsCheck->setEnabled(true);
  }

  if (CheckBoxMesh->isChecked()) {
    if (myMeshes.isEmpty())
      return;
    SMESH::GetNameOfSelectedIObjects( mySelectionMgr, aString );
  }
  else {
    int aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, aList.First(), aString);
    myElementsId = aString;
    if (aNbUnits < 1)
      return;
  }

  myNbOkElements = true;
  
  LineEditElements->setText(aString);
  LineEditElements->repaint();
  LineEditElements->setEnabled(false); // to fully update lineedit IPAL 19809
  LineEditElements->setEnabled(true);
  setNewMeshName();

  buttonOk->setEnabled(true);
  buttonApply->setEnabled(true);

  onDisplaySimulation(true);
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================

void SMESHGUI_OffsetDlg::DeactivateActiveDialog()
{
  if (ConstructorsBox->isEnabled())
  {
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

void SMESHGUI_OffsetDlg::ActivateThisDialog()
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

void SMESHGUI_OffsetDlg::enterEvent (QEvent*)
{
  if (!ConstructorsBox->isEnabled()) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    ActivateThisDialog();
  }
}

//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================

void SMESHGUI_OffsetDlg::onSelectMesh (bool toSelectMesh)
{
  if (toSelectMesh)
    TextLabelElements->setText(tr("SMESH_NAME"));
  else
    TextLabelElements->setText(tr("SMESH_ID_ELEMENTS"));
  myFilterBtn->setEnabled(!toSelectMesh);

  mySelectionMgr->clearFilters();
  SMESH::SetPointRepresentation(false);

  if (toSelectMesh)
  {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( ActorSelection );
    mySelectionMgr->installFilter( myMeshOrSubMeshOrGroupFilter );
    LineEditElements->setReadOnly( true );
    LineEditElements->setValidator( 0 );
  }
  else
  {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( FaceSelection );
    LineEditElements->setReadOnly( false );
    LineEditElements->setValidator( myIdValidator );
    onTextChange(LineEditElements->text());
    hidePreview();
  }

  SelectionIntoArgument();
}

//=======================================================================
//function : onActionClicked
//purpose  : slot called when an action type changed
//=======================================================================

void SMESHGUI_OffsetDlg::onActionClicked(int button)
{
  int aNbGroups = 0;
  for ( int i = 0; i < myMeshes.count(); i++ )
    aNbGroups += myMeshes[i]->NbGroups();

  switch ( button ) {
  case MOVE_ELEMS_BUTTON:
    MakeGroupsCheck->setEnabled(false);
    LineEditNewMesh->setEnabled(false);
    break;
  case COPY_ELEMS_BUTTON:
    LineEditNewMesh->setEnabled(false);
    MakeGroupsCheck->setText( tr("SMESH_MAKE_GROUPS"));
    MakeGroupsCheck->setEnabled( myMeshes.isEmpty() || aNbGroups > 0 );
    break;
  case MAKE_MESH_BUTTON:
    LineEditNewMesh->setEnabled(true);
    MakeGroupsCheck->setText( tr("SMESH_COPY_GROUPS"));
    MakeGroupsCheck->setEnabled( myMeshes.isEmpty() || aNbGroups > 0 );
    break;
  }
  setNewMeshName();
  //toDisplaySimulation();
}

//=======================================================================
//function : setNewMeshName
//purpose  : update contents of LineEditNewMesh
//=======================================================================

void SMESHGUI_OffsetDlg::setNewMeshName()
{
  LineEditNewMesh->setText("");
  if ( LineEditNewMesh->isEnabled() && !myMeshes.isEmpty() ) {
    QString name;
    if ( CheckBoxMesh->isChecked() ) {
      name = myObjects.count() > 1 ? "*" : LineEditElements->text();
    }
    else {
      _PTR(SObject) meshSO = SMESH::FindSObject( myMeshes[0] );
      name = meshSO->GetName().c_str();
    }
    if ( !name.isEmpty() )
      LineEditNewMesh->setText( SMESH::UniqueMeshName( name, "Offset"));
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================

void SMESHGUI_OffsetDlg::keyPressEvent( QKeyEvent* e )
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

void SMESHGUI_OffsetDlg::setFilters()
{
  if ( myMeshes.isEmpty() ) {
    SUIT_MessageBox::critical(this, tr("SMESH_ERROR"), tr("NO_MESH_SELECTED"));
    return;
  }
  if ( !myFilterDlg ) {
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, SMESH::ALL );
    connect(myFilterDlg, SIGNAL(Accepted()), SLOT(onFilterAccepted()));
  }

  myFilterDlg->Init( SMESH::FACE );
  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMeshes[0] );
  myFilterDlg->SetSourceWg( LineEditElements );

  myFilterDlg->show();
}

//=======================================================================
// name    : onFilterAccepted()
// Purpose : SLOT. Called when Filter dlg closed with OK button.
//           Activate [Apply] if no Actor is available
//=======================================================================

void SMESHGUI_OffsetDlg::onFilterAccepted()
{
  if ( myMeshes.length() > 0 && !buttonOk->isEnabled() )
  {
    myElementsId = LineEditElements->text();
    QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);
    myNbOkElements = aListElementsId.count();
    buttonOk->setEnabled   ( myNbOkElements );
    buttonApply->setEnabled( myNbOkElements );
  }
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================

bool SMESHGUI_OffsetDlg::isValid()
{
  return true;
}

//=================================================================================
// function : onDisplaySimulation
// purpose  : Show/Hide preview
//=================================================================================
void SMESHGUI_OffsetDlg::onDisplaySimulation( bool toDisplayPreview )
{
  SUIT_OverrideCursor aWaitCursor;

  if (myPreviewCheckBox->isChecked() && toDisplayPreview)
  {
    if ( myNbOkElements && isValid() )
    {
      double offsetValue = SpinBox->value();

      SMESH::ListOfGroups_var groups;
      SMESH::SMESH_Mesh_var mesh;

      try
      {
        QList<SMESH::MeshPreviewStruct_var> aMeshPreviewStruct;

        if ( CheckBoxMesh->isChecked())
          for ( int i = 0; i < myObjects.count(); i++ )
          {
            SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[i]->GetMeshEditPreviewer();
            mesh = aMeshEditor->Offset( myObjects[i], offsetValue, false, false, "", groups.out() );
            aMeshPreviewStruct << aMeshEditor->GetPreviewData();
          }
        else
        {
          SMESH::smIdType_array_var anElementsId = new SMESH::smIdType_array;
          QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);
          anElementsId->length(aListElementsId.count());
          for (int i = 0; i < aListElementsId.count(); i++)
            anElementsId[i] = aListElementsId[i].toInt();

          SMESH::SMESH_MeshEditor_var aMeshEditor = myMeshes[0]->GetMeshEditPreviewer();
          SMESH::IDSource_wrap src = aMeshEditor->MakeIDSource(anElementsId, SMESH::FACE);
          mesh = aMeshEditor->Offset( src, offsetValue, false, false, "", groups.out() );
          aMeshPreviewStruct << aMeshEditor->GetPreviewData();
        }
        setSimulationPreview(aMeshPreviewStruct);

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
