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
// File   : SMESHGUI_FaceGroupsSeparatedByEdges.cxx

#include "SMESHGUI_FaceGroupsSeparatedByEdgesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_Utils.h"

// SALOME GUI includes
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_Actor.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SalomeApp_Tools.h>

#include <SALOMEDS_SObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

// Qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>

#include <Standard_ErrorHandler.hxx>

#define SPACING 6
#define MARGIN  11

/*!
  \class SMESHGUI_FaceGroupsSeparatedByEdgesDlg
  \brief Dialog to create face groups divided by sharp edges
*/

SMESHGUI_FaceGroupsSeparatedByEdgesDlg::
SMESHGUI_FaceGroupsSeparatedByEdgesDlg( SMESHGUI* theModule ) :
  SMESHGUI_PreviewDlg( theModule ),
  mySelectionMgr( SMESH::GetSelectionMgr( theModule ))
{
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("CAPTION"));
  setSizeGripEnabled(true);

  QVBoxLayout* dlgLayout = new QVBoxLayout(this);
  dlgLayout->setSpacing(SPACING);
  dlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupArgs = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgsLayout = new QGridLayout(GroupArgs);
  GroupArgsLayout->setSpacing(SPACING);
  GroupArgsLayout->setMargin(MARGIN);

  // mesh
  QLabel* meshNameLabel = new QLabel(tr("SMESH_MESH"), GroupArgs);
  myMeshName = new QLineEdit( GroupArgs );
  myMeshName->setReadOnly( true );

  // angle
  QLabel* angleLabel = new QLabel( tr("SHARP_ANGLE"), GroupArgs );
  myAngle = new SMESHGUI_SpinBox( GroupArgs );
  myAngle->RangeStepAndValidator( 0, 180.0, 10.0, "angle_precision");

  // check-boxes
  myCreateEdgesCheck = new QCheckBox( tr("CREATE_EDGES" ), GroupArgs);
  myUseExistingCheck = new QCheckBox( tr("USE_EXISTING_EDGES"), GroupArgs);
  myPreviewCheckBox  = new QCheckBox(tr("PREVIEW"), GroupArgs);

  // set previous values
  if ( SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr() )
  {
    myAngle->setValue( resMgr->doubleValue( "SMESH", "prev_sharp_angle", 30. ));
    myCreateEdgesCheck->setChecked( resMgr->booleanValue( "SMESH", "prev_create_edges", false ));
    myUseExistingCheck->setChecked( resMgr->booleanValue( "SMESH", "prev_use_existing_edges", false ));
  }

  // layout
  GroupArgsLayout->addWidget( meshNameLabel,      0, 0);
  GroupArgsLayout->addWidget( myMeshName,         0, 1);
  GroupArgsLayout->addWidget( angleLabel,         1, 0);
  GroupArgsLayout->addWidget( myAngle,            1, 1);
  GroupArgsLayout->addWidget( myCreateEdgesCheck, 2, 0);
  GroupArgsLayout->addWidget( myUseExistingCheck, 3, 0);
  GroupArgsLayout->addWidget( myPreviewCheckBox,  4, 0);

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
  dlgLayout->addWidget(GroupArgs);
  dlgLayout->addWidget(GroupButtons);

  mySMESHGUI->SetActiveDialogBox(this);

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),      this, SLOT(reject()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()),            this, SLOT(onCloseView()));

  connect( myAngle, SIGNAL(valueChanged(double)),     this, SLOT( onArgChange() ));
  connect( myCreateEdgesCheck, SIGNAL(toggled(bool)), this, SLOT( onArgChange() ));
  connect( myUseExistingCheck, SIGNAL(toggled(bool)), this, SLOT( onArgChange() ));

  connectPreviewControl();
  mySimulation->GetActor()->GetProperty()->SetLineWidth( 5 );

  SelectionIntoArgument();
}

SMESHGUI_FaceGroupsSeparatedByEdgesDlg::~SMESHGUI_FaceGroupsSeparatedByEdgesDlg()
{
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================

bool SMESHGUI_FaceGroupsSeparatedByEdgesDlg::ClickOnApply()
{
  if ( mySMESHGUI->isStudyLocked() )
    return false;

  SUIT_OverrideCursor aWaitCursor;

  QStringList aParameters, anEntryList;
  aParameters << myAngle->text();

  try
  {
    SMESH::ListOfGroups_var groups =
      myMesh->FaceGroupsSeparatedByEdges( myAngle->value(),
                                          myCreateEdgesCheck->isChecked(),
                                          myUseExistingCheck->isChecked());

    for ( CORBA::ULong i = 0; i < groups->length(); ++i )
      if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( groups[i].in() ))
        anEntryList.append( aSObject->GetID().c_str() );

    SUIT_MessageBox::information(this, tr("SMESH_INFORMATION"),
                                 tr("NB_GROUPS_CREATED").arg( groups->length() ));
  }
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch (...) {
  }

  mySMESHGUI->updateObjBrowser(true); // new groups may appear
  if( LightApp_Application* anApp =
      dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
  {
    anApp->browseObjects( anEntryList, isApplyAndClose() );
  }

  SMESHGUI::Modified();

  return true;
}
//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================

void SMESHGUI_FaceGroupsSeparatedByEdgesDlg::ClickOnOk()
{
  setIsApplyAndClose( true );
  if( ClickOnApply() )
    reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_FaceGroupsSeparatedByEdgesDlg::reject()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  QDialog::reject();
}
//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================

void SMESHGUI_FaceGroupsSeparatedByEdgesDlg::ClickOnHelp()
{
  static QString myHelpFileName( "face_groups_by_sharp_edges.html" );

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
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================

void SMESHGUI_FaceGroupsSeparatedByEdgesDlg::SelectionIntoArgument()
{
  if (!GroupButtons->isEnabled()) // inactive
    return;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  for ( SALOME_ListIteratorOfListIO it( aList ); it.More(); it.Next() )
  {
    Handle(SALOME_InteractiveObject) IO = it.Value();
    CORBA::Object_var anObj = SMESH::IObjectToObject( IO );
    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( anObj );
    if ( aMesh->_is_nil() || aMesh->NbFaces() == 0 )
      continue;

    if ( !aMesh->_is_equivalent( myMesh ))
      onDisplaySimulation( false );

    myMesh = aMesh;
    myMeshName->setText( IO->getName() );
  }

  bool ok = !myMesh->_is_nil();

  buttonOk->setEnabled( ok );
  buttonApply->setEnabled( ok );

  onDisplaySimulation( ok );
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================

void SMESHGUI_FaceGroupsSeparatedByEdgesDlg::DeactivateActiveDialog()
{
  if (GroupArgs->isEnabled())
  {
    GroupArgs->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================

void SMESHGUI_FaceGroupsSeparatedByEdgesDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupArgs->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  SelectionIntoArgument();
}

//=================================================================================
// function : onDisplaySimulation
// purpose  : Show/Hide preview
//=================================================================================

void SMESHGUI_FaceGroupsSeparatedByEdgesDlg::onDisplaySimulation( bool toDisplayPreview )
{
  SUIT_OverrideCursor aWaitCursor;

  // save current values
  if ( SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr() )
  {
    resMgr->setValue( "SMESH", "prev_sharp_angle",        myAngle->value() );
    resMgr->setValue( "SMESH", "prev_create_edges",       myCreateEdgesCheck->isChecked() );
    resMgr->setValue( "SMESH", "prev_use_existing_edges", myUseExistingCheck->isChecked() );
  }

  if ( myPreviewCheckBox->isChecked() && toDisplayPreview && !myMesh->_is_nil() )
  {
    try
    {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditPreviewer();
      SMESH::ListOfEdges_var edges = aMeshEditor->FindSharpEdges( myAngle->value(),
                                                                  myUseExistingCheck->isChecked());
      SMESH::MeshPreviewStruct_var previewData = aMeshEditor->GetPreviewData();

      mySimulation->SetData( previewData.in() );
      showPreview();

    } catch (...) {
      hidePreview();
    }
  }
  else
  {
    hidePreview();
  }
}
