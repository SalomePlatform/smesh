// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_RemoveNodesDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_RemoveNodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_FilterDlg.h"

#include <SMESH_TypeFilter.hxx>
#include <SMESH_Actor.h>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Tools.h>

#include <SVTK_Selector.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QButtonGroup>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

//=================================================================================
// class    : SMESHGUI_RemoveNodesDlg()
// purpose  :
//=================================================================================
SMESHGUI_RemoveNodesDlg
::SMESHGUI_RemoveNodesDlg(SMESHGUI* theModule)
  : QDialog(SMESH::GetDesktop(theModule)),
    mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
    mySelector(SMESH::GetViewWindow(theModule)->GetSelector()),
    mySMESHGUI(theModule),
    myBusy(false),
    myFilterDlg(0)
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("SMESH_REMOVE_NODES_TITLE"));
  setSizeGripEnabled(true);
  
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_REM_NODE")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  QVBoxLayout* SMESHGUI_RemoveNodesDlgLayout = new QVBoxLayout(this);
  SMESHGUI_RemoveNodesDlgLayout->setSpacing(SPACING);
  SMESHGUI_RemoveNodesDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(tr("SMESH_NODES"), this);
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
  GroupC1 = new QGroupBox(tr("SMESH_REMOVE"), this);
  QHBoxLayout* GroupC1Layout = new QHBoxLayout(GroupC1);
  GroupC1Layout->setSpacing(SPACING);
  GroupC1Layout->setMargin(MARGIN);

  TextLabelC1A1 = new QLabel(tr("SMESH_ID_NODES"), GroupC1);
  SelectButtonC1A1 = new QPushButton(GroupC1);
  SelectButtonC1A1->setIcon(image1);
  LineEditC1A1 = new QLineEdit(GroupC1);
  LineEditC1A1->setValidator(new SMESHGUI_IdValidator(this));
  LineEditC1A1->setMaxLength(-1);
  QPushButton* filterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupC1 );
  connect(filterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

  GroupC1Layout->addWidget(TextLabelC1A1);
  GroupC1Layout->addWidget(SelectButtonC1A1);
  GroupC1Layout->addWidget(LineEditC1A1);
  GroupC1Layout->addWidget(filterBtn );

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
  SMESHGUI_RemoveNodesDlgLayout->addWidget(GroupConstructors);
  SMESHGUI_RemoveNodesDlgLayout->addWidget(GroupC1);
  SMESHGUI_RemoveNodesDlgLayout->addWidget(GroupButtons);

  myHelpFileName = "removing_nodes_and_elements_page.html#removing_nodes_anchor";

  Init(); /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_RemoveNodesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RemoveNodesDlg::~SMESHGUI_RemoveNodesDlg()
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
void SMESHGUI_RemoveNodesDlg::Init()
{
  myConstructorId = 0;
  Constructor1->setChecked(true);
  myEditCurrentArgument = LineEditC1A1;

  myNbOkNodes = 0;
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myActor = 0;
  myBusy = false;

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectButtonC1A1, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()),      this, SLOT(reject()));
  connect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL (SignalCloseView()),            this, SLOT(onCloseView()));
  connect(myEditCurrentArgument, SIGNAL(textChanged(const QString&)),
          this,                  SLOT (onTextChange(const QString&)));

  SMESH::SetPointRepresentation(true);

  mySelectionMgr->clearFilters();
  mySelectionMgr->installFilter( new SMESH_TypeFilter( SMESH::IDSOURCE ));

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  //SelectionIntoArgument();
  mySelectionMgr->setSelectedObjects( SALOME_ListIO() );
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  if (myNbOkNodes) {
    QStringList aListId = myEditCurrentArgument->text().split(" ", QString::SkipEmptyParts);
    SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
    anArrayOfIdeces->length(aListId.count());
    for (int i = 0; i < aListId.count(); i++)
      anArrayOfIdeces[i] = aListId[ i ].toInt();

    bool aResult = false;
    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      aResult = aMeshEditor->RemoveNodes(anArrayOfIdeces.in());
    } catch (const SALOME::SALOME_Exception& S_ex) {
      SalomeApp_Tools::QtCatchCorbaException(S_ex);
      myEditCurrentArgument->clear();
    } catch (...){
      myEditCurrentArgument->clear();
    }

    if (aResult) {
      myEditCurrentArgument->clear();
      mySelector->ClearIndex();
      SMESH::UpdateView();
      SMESHGUI::Modified();
    }

    SMESH::SetPointRepresentation(true);
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ClickOnOk()
{
  ClickOnApply();
  reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::reject()
{
  //mySelectionMgr->clearSelected();
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
  }
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  mySMESHGUI->ResetState();
  QDialog::reject();
}


//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::onOpenView()
{
  if ( mySelector) {
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
void SMESHGUI_RemoveNodesDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ClickOnHelp()
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
//function : onTextChange
//purpose  :
//=======================================================================
void SMESHGUI_RemoveNodesDlg::onTextChange(const QString& theNewText)
{
  if (myBusy) return;
  myBusy = true;

  myNbOkNodes = 0;

  // hilight entered nodes
  if(myActor){
    if(SMDS_Mesh* aMesh = myActor->GetObject()->GetMesh()){
      Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
           
      TColStd_MapOfInteger newIndices;
      
      QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
      for (int i = 0; i < aListId.count(); i++) {
        if (const SMDS_MeshNode *aNode = aMesh->FindNode(aListId[i].toInt())) {
          newIndices.Add(aNode->GetID());
          myNbOkNodes++;
        }
      }

      mySelector->AddOrRemoveIndex(anIO,newIndices,false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->highlight(anIO,true,true);
    }
  }

  myBusy = false;
  updateButtons();
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_RemoveNodesDlg::SelectionIntoArgument()
{
  if (myBusy) return;                                  // busy
  if (myFilterDlg && myFilterDlg->isVisible()) return; // filter dlg active
  if (!GroupButtons->isEnabled()) return;              // inactive
    
  // clear

  myNbOkNodes = 0;
  myActor = 0;

  myBusy = true;
  myEditCurrentArgument->setText("");
  myBusy = false;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  int nbSel = aList.Extent();
  if (nbSel == 1) {

    Handle(SALOME_InteractiveObject) anIO = aList.First();
    myMesh = SMESH::GetMeshByIO(anIO);

    if (!myMesh->_is_nil()) {

      myActor = SMESH::FindActorByEntry(anIO->getEntry());
      if (myActor) {

        // get selected nodes
        QString aString = "";
        int nbNodes = SMESH::GetNameOfSelectedNodes(mySelector,anIO,aString);
        if (nbNodes > 0) {
          myBusy = true;
          myEditCurrentArgument->setText(aString);
          myBusy = false;

          // OK

          myNbOkNodes = nbNodes;
        } // if (nbNodes > 0)
      } // if (myActor)
    } // if (!myMesh->_is_nil())
  } // if (nbSel == 1)

  updateButtons();
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  switch (myConstructorId) {
  case 0: /* default constructor */
    {
      if(send == SelectButtonC1A1) {
        LineEditC1A1->setFocus();
        myEditCurrentArgument = LineEditC1A1;
      }
      SelectionIntoArgument();
      break;
    }
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupC1->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState(); // ??
    mySMESHGUI->SetActiveDialogBox(0); // ??
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();

  GroupConstructors->setEnabled(true);
  GroupC1->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this); // ??

  mySelectionMgr->clearFilters();
  mySelectionMgr->installFilter( new SMESH_TypeFilter( SMESH::IDSOURCE ));

  SMESH::SetPointRepresentation(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  SelectionIntoArgument(); // ??
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::enterEvent(QEvent*)
{
  if (!GroupConstructors->isEnabled()) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    ActivateThisDialog();
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::keyPressEvent( QKeyEvent* e )
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
void SMESHGUI_RemoveNodesDlg::setFilters()
{
  if(myMesh->_is_nil()) {
    SUIT_MessageBox::critical(this,
                              tr("SMESH_ERROR"),
                              tr("NO_MESH_SELECTED"));
   return;
  }
  if ( !myFilterDlg )
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, SMESH::NODE );

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( LineEditC1A1 );

  myFilterDlg->show();
}

//=================================================================================
// function : updateButtons
// purpose  : enable / disable control buttons
//=================================================================================
void SMESHGUI_RemoveNodesDlg::updateButtons()
{
  buttonOk->setEnabled(myNbOkNodes > 0);
  buttonApply->setEnabled(myNbOkNodes > 0);
}
