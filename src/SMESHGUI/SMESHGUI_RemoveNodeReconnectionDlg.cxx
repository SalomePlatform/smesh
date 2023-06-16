// Copyright (C) 2007-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_MakeNodeAtPointDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_RemoveNodeReconnectionDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshEditPreview.h"

#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_LogicalFilter.hxx>

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_TypeFilter.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>

// Qt includes
#include <QApplication>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>

// VTK includes
#include <vtkProperty.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

//=======================================================================
/*!
 * \brief Dialog to Remove a node with elements re-connection
 */
//=======================================================================

SMESHGUI_RemoveNodeReconnectionDlg::SMESHGUI_RemoveNodeReconnectionDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  setWindowTitle(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin(0);
  aDlgLay->setSpacing(SPACING);
  myMainFrame = createMainFrame(mainFrame());

  aDlgLay->addWidget(myMainFrame);

  aDlgLay->setStretchFactor(myMainFrame, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================

QWidget* SMESHGUI_RemoveNodeReconnectionDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  // Node to remove

  myNodeToMoveGrp = new QGroupBox(tr("NODE_2REMOVE"), aFrame);
  myNodeToMoveGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  QLabel* idLabel = new QLabel(tr("NODE_2REMOVE_ID"), myNodeToMoveGrp);
  myId = new QLineEdit(myNodeToMoveGrp);
  myId->setValidator(new SMESHGUI_IdValidator(this, 1));

  QGridLayout* myNodeToMoveGrpLayout = new QGridLayout(myNodeToMoveGrp);
  myNodeToMoveGrpLayout->setSpacing(SPACING);
  myNodeToMoveGrpLayout->setMargin(MARGIN);

  myNodeToMoveGrpLayout->addWidget( idLabel, 0, 0 );
  myNodeToMoveGrpLayout->addWidget( myId,    0, 2 );

  // Preview

  myPreviewChkBox = new QCheckBox( tr("PREVIEW"), aFrame);

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(myNodeToMoveGrp);
  aLay->addWidget(myPreviewChkBox);

  return aFrame;
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

SMESHGUI_RemoveNodeReconnectionOp::SMESHGUI_RemoveNodeReconnectionOp()
{
  mySimulation = 0;
  mySMESHGUI = 0;
  myDlg = new SMESHGUI_RemoveNodeReconnectionDlg;
  myFilter = 0;
  myHelpFileName = "removing_nodes_and_elements.html#removing-nodes-reconnect-anchor";

  myNoPreview = false;

  // connect signals and slots
  connect(myDlg->myId,            SIGNAL (textChanged(const QString&)),SLOT(redisplayPreview()));
  connect(myDlg->myPreviewChkBox, SIGNAL (toggled(bool)),              SLOT(redisplayPreview()));
  connect(myDlg->myId,            SIGNAL (textChanged(const QString&)),SLOT(onTextChange(const QString&)));
}

//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================

void SMESHGUI_RemoveNodeReconnectionOp::startOperation()
{
  myNoPreview = false;
  myMeshActor = 0;

  // init simulation with a current View
  if ( mySimulation ) delete mySimulation;
  mySMESHGUI = getSMESHGUI();
  mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ) );
  connect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL (SignalCloseView()), this, SLOT(onCloseView()));
  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor(250, 0, 250);
  aProp->SetPointSize(5);
  aProp->SetLineWidth( SMESH::GetFloat("SMESH:element_width",1) + 1);
  mySimulation->GetActor()->SetProperty(aProp);
  aProp->Delete();

  // SalomeApp_TypeFilter depends on a current study
  if ( myFilter ) delete myFilter;
  // QList<SUIT_SelectionFilter*> filters;
  // filters.append( new SalomeApp_TypeFilter((SalomeApp_Study*)study(), "SMESH" ));
  // myFilter = new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );

  // IPAL19360
  SMESHGUI_SelectionOp::startOperation(); // this method should be called only after filter creation
  myDlg->myId->setText("");

  myDlg->show();

  onSelectionDone(); // init myMeshActor
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_RemoveNodeReconnectionOp::stopOperation()
{
  myNoPreview = true;
  if ( mySimulation )
  {
    mySimulation->SetVisibility(false);
    delete mySimulation;
    mySimulation = 0;
  }
  if ( myMeshActor ) {
    myMeshActor = 0;
  }
  SMESH::SetPointRepresentation( false );
  SMESH::RepaintCurrentView();

  disconnect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this, SLOT(onOpenView()));
  disconnect(mySMESHGUI, SIGNAL (SignalCloseView()),            this, SLOT(onCloseView()));
  //selectionMgr()->removeFilter( myFilter );
  SMESHGUI_SelectionOp::stopOperation();
}

//================================================================================
/*!
 * \brief perform it's intention action: move or create a node
 */
//================================================================================

bool SMESHGUI_RemoveNodeReconnectionOp::onApply()
{
  if( SMESHGUI::isStudyLocked() )
    return false;

  if ( !myMeshActor ) {
    SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ), tr("INVALID_MESH") );
    dlg()->show();
    return false;
  }

  QString msg;
  if ( !isValid( msg ) ) { // node id is invalid
    if ( !msg.isEmpty() )
      SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ), tr("INVALID_ID") );
    dlg()->show();
    return false;
  }

  try {
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( myMeshActor->getIO() );
    if ( aMesh->_is_nil() )
    {
      SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"), tr("SMESHG_NO_MESH") );
      return true;
    }
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    if ( aMeshEditor->_is_nil() )
      return true;

    bool ok;
    int anId = myDlg->myId->text().toInt( &ok );
    myDlg->myId->setText( "" );

    aMeshEditor->RemoveNodeWithReconnection( anId );

    SALOME_ListIO aList;
    selectionMgr()->setSelectedObjects(aList,false);
    aList.Append( myMeshActor->getIO() );
    selectionMgr()->setSelectedObjects(aList,false);
    SMESH::UpdateView();
    SMESHGUI::Modified();

  }
  catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...) {
  }

  return true;
}

//================================================================================
/*!
 * \brief Check selected node id validity
 */
//================================================================================

bool SMESHGUI_RemoveNodeReconnectionOp::isValid( QString& msg )
{
  bool ok = true;
  if ( myMeshActor )
  {
    ok = false;
    int id = myDlg->myId->text().toInt();
    if ( id > 0 )
      if ( SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh() )
        ok = aMesh->FindNode( id );
    if ( !ok )
      msg += tr("INVALID_ID") + "\n";
  }

  return ok;
}

//================================================================================
/*!
 * \brief SLOT called when selection changed
 */
//================================================================================

void SMESHGUI_RemoveNodeReconnectionOp::onSelectionDone()
{
  if ( !myDlg->isVisible() || !myDlg->isEnabled() )
    return;

  myDlg->myId->setText("");
  myNoPreview = true;
  try
  {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects( aList, SVTK_Viewer::Type() );
    if ( aList.Extent() != 1)
      return;
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    myMeshActor = SMESH::FindActorByEntry( anIO->getEntry() );

    QString aString;
    int nbElems = SMESH::GetNameOfSelectedElements( selector(), anIO, aString );
    if ( nbElems == 1 )
      myDlg->myId->setText( aString );

  } catch (...) {
  }

  myNoPreview = false;
  redisplayPreview();
}

//================================================================================
/*!
 * \brief update preview
 */
//================================================================================

void SMESHGUI_RemoveNodeReconnectionOp::redisplayPreview()
{
  if ( myNoPreview )
    return;
  myNoPreview = true;

  if ( !myMeshActor )
    onSelectionDone();

  SMESH::MeshPreviewStruct_var aMeshPreviewStruct;

  QString msg;
  if ( myMeshActor && isValid( msg ) && myDlg->myPreviewChkBox->isChecked() )
    try {
      SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( myMeshActor->getIO() );
      if ( !aMesh->_is_nil() )
      {
        SMESH::SMESH_MeshEditor_var aPreviewer = aMesh->GetMeshEditPreviewer();
        if (!aPreviewer->_is_nil())
        {
          int anId = myDlg->myId->text().toInt();
          aPreviewer->RemoveNodeWithReconnection( anId );
          aMeshPreviewStruct = aPreviewer->GetPreviewData();
        }
      }
    }
    catch (...) {
    }

  if (!mySimulation)
    mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));

  // display data
  if ( & aMeshPreviewStruct.in() )
  {
    mySimulation->SetData( aMeshPreviewStruct.in() );
  }
  else
  {
    mySimulation->SetVisibility( false );
  }

  myNoPreview = false;
}

//=================================================================================
/*!
 * \brief SLOT called when the viewer opened
 */
//=================================================================================

void SMESHGUI_RemoveNodeReconnectionOp::onOpenView()
{
  if ( mySimulation )
  {
    mySimulation->SetVisibility( false );
    SMESH::SetPointRepresentation( false );
  }
  else
  {
    mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));
  }
}

//=================================================================================
/*!
 * \brief SLOT called when the viewer closed
 */
//=================================================================================

void SMESHGUI_RemoveNodeReconnectionOp::onCloseView()
{
  delete mySimulation;
  mySimulation = 0;
}

//================================================================================
/*!
 * \brief SLOT called when the node id is manually changed
 */
//================================================================================

void SMESHGUI_RemoveNodeReconnectionOp::onTextChange( const QString& theText )
{
  if( myMeshActor )
  {
    if( SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh() )
    {
      Handle(SALOME_InteractiveObject) anIO = myMeshActor->getIO();
      SALOME_ListIO aList;
      aList.Append( anIO );
      selectionMgr()->setSelectedObjects( aList, false );

      if ( const SMDS_MeshNode* aNode = aMesh->FindNode( theText.toInt() ))
      {
        SVTK_TVtkIDsMap aListInd;
        aListInd.Add( FromSmIdType<int>( aNode->GetID()) );
        selector()->AddOrRemoveIndex( anIO, aListInd, false );
        if( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() ) )
          aViewWindow->highlight( anIO, true, true );
      }
    }
  }
}

//================================================================================
/*!
 * \brief Activate Node selection
 */
//================================================================================

void SMESHGUI_RemoveNodeReconnectionOp::activateSelection()
{
  selectionMgr()->clearFilters();
  SMESH::SetPointRepresentation( true );
  //selectionMgr()->installFilter( myFilter );
  setSelectionMode( NodeSelection );
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESHGUI_RemoveNodeReconnectionOp::~SMESHGUI_RemoveNodeReconnectionOp()
{
  if ( myDlg )        delete myDlg;
  if ( mySimulation ) delete mySimulation;
  if ( myFilter )     delete myFilter;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_RemoveNodeReconnectionOp::dlg() const
{
  return myDlg;
}

