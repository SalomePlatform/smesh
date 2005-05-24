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
//  File   : SMESHGUI_MoveNodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_MoveNodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"

#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"

#include "SALOME_Selection.h"
#include "VTKViewer_ViewFrame.h"
#include "QAD_Desktop.h"
#include "QAD_RightFrame.h"
#include "QAD_MessageBox.h"

#include "utilities.h"

#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#define MARGIN  10
#define SPACING 5


//=================================================================================
// class    : SMESHGUI_MoveNodesDlg()
// purpose  : 
//=================================================================================
SMESHGUI_MoveNodesDlg::SMESHGUI_MoveNodesDlg( QWidget*          theParent,
                                              SALOME_Selection* theSelection,
                                              const char*       theName )

: QDialog( theParent, theName, false,
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  myPreviewActor = 0;
  myBusy = false;
  mySelection = 0;
  
  setCaption( tr( "CAPTION" ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( this, MARGIN, SPACING );

  QFrame* aMainFrame = createMainFrame  ( this );
  QFrame* aBtnFrame  = createButtonFrame( this );

  aDlgLay->addWidget( aMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( aMainFrame, 1 );

  Init( theSelection ) ;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_MoveNodesDlg::createButtonFrame( QWidget* theParent )
{
  QFrame* aFrame = new QFrame( theParent );
  aFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

  myOkBtn     = new QPushButton( tr( "SMESH_BUT_OK"    ), aFrame );
  myApplyBtn  = new QPushButton( tr( "SMESH_BUT_APPLY" ), aFrame );
  myCloseBtn  = new QPushButton( tr( "SMESH_BUT_CLOSE" ), aFrame );

  QSpacerItem* aSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );

  QHBoxLayout* aLay = new QHBoxLayout( aFrame, MARGIN, SPACING );

  aLay->addWidget( myOkBtn );
  aLay->addWidget( myApplyBtn );
  aLay->addItem( aSpacer);
  aLay->addWidget( myCloseBtn );

  connect( myOkBtn,    SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myCloseBtn, SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( myApplyBtn, SIGNAL( clicked() ), SLOT( onApply() ) );

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_MoveNodesDlg::createMainFrame( QWidget* theParent )
{
  QFrame* aFrame = new QFrame( theParent );

  QPixmap iconMoveNode(
    QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr( "ICON_DLG_MOVE_NODE" ) ) );
  QPixmap iconSelect(
    QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr( "ICON_SELECT" ) ) );

  QButtonGroup* aPixGrp = new QButtonGroup( 1, Qt::Vertical, tr( "MESH_NODE" ), aFrame );
  aPixGrp->setExclusive( TRUE );
  QRadioButton* aRBut = new QRadioButton( aPixGrp );
  aRBut->setPixmap( iconMoveNode );
  aRBut->setChecked( TRUE );

  QGroupBox* anIdGrp = new QGroupBox( 1, Qt::Vertical, tr( "SMESH_MOVE" ), aFrame );
  new QLabel( tr( "NODE_ID" ), anIdGrp );
  ( new QPushButton( anIdGrp ) )->setPixmap( iconSelect );
  myId = new QLineEdit( anIdGrp );
  myId->setValidator( new SMESHGUI_IdValidator( this, "validator", 1 ));

  QGroupBox* aCoordGrp = new QGroupBox( 1, Qt::Vertical, tr( "SMESH_COORDINATES" ), aFrame );
  new QLabel( tr( "SMESH_X" ), aCoordGrp );
  myX = new SMESHGUI_SpinBox( aCoordGrp );
  new QLabel( tr( "SMESH_Y" ), aCoordGrp );
  myY = new SMESHGUI_SpinBox( aCoordGrp );
  new QLabel( tr( "SMESH_Z" ), aCoordGrp );
  myZ = new SMESHGUI_SpinBox( aCoordGrp );

  myX->RangeStepAndValidator( COORD_MIN, COORD_MAX, 25.0, 3 );
  myY->RangeStepAndValidator( COORD_MIN, COORD_MAX, 25.0, 3 );
  myZ->RangeStepAndValidator( COORD_MIN, COORD_MAX, 25.0, 3 );  

  QVBoxLayout* aLay = new QVBoxLayout( aFrame );
  aLay->addWidget( aPixGrp );
  aLay->addWidget( anIdGrp );
  aLay->addWidget( aCoordGrp );

  // connect signale and slots
  connect( myX, SIGNAL ( valueChanged( double) ), this, SLOT( redisplayPreview() ) );
  connect( myY, SIGNAL ( valueChanged( double) ), this, SLOT( redisplayPreview() ) );
  connect( myZ, SIGNAL ( valueChanged( double) ), this, SLOT( redisplayPreview() ) );
  connect( myId, SIGNAL( textChanged(const QString&) ), SLOT( onTextChange(const QString&) ));

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::~SMESHGUI_MoveNodesDlg
// Purpose :
//=======================================================================
SMESHGUI_MoveNodesDlg::~SMESHGUI_MoveNodesDlg()
{
  erasePreview();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::Init
// Purpose : Init dialog fields
//=======================================================================
void SMESHGUI_MoveNodesDlg::Init( SALOME_Selection* theSelection )
{
  myPreviewActor = 0;
  myMeshActor = 0;
  myBusy = false;
  mySelection = theSelection;

  SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI();
  aSMESHGUI->SetActiveDialogBox( ( QDialog* )this ) ;

  // selection and SMESHGUI
  connect( mySelection, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );
  connect( aSMESHGUI, SIGNAL( SignalDeactivateActiveDialog() ), SLOT( onDeactivate() ) );
  connect( aSMESHGUI, SIGNAL( SignalCloseAllDialogs() ), SLOT( onClose() ) );

  reset();
  setEnabled( true );

  int x, y ;
  aSMESHGUI->DefineDlgPosition( this, x, y );
  this->move( x, y );
  this->show();

  // set selection mode
  SMESH::SetPointRepresentation(true);
  QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );
  onSelectionDone();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::isValid
// Purpose : Verify validity of entry information
//=======================================================================
bool SMESHGUI_MoveNodesDlg::isValid( const bool theMess ) const
{
  if ( myId->text().isEmpty() )
  {
    if ( theMess )
      QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
        tr( "SMESH_WARNING" ), tr( "NODE_ID_IS_NOT_DEFINED" ), QMessageBox::Ok );
    return false;
  }
  return true;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::reset
// Purpose : Reset the dialog state
//=======================================================================
void SMESHGUI_MoveNodesDlg::reset()
{
  myId->clear();
  myX->SetValue( 0 );
  myY->SetValue( 0 );
  myZ->SetValue( 0 );
  redisplayPreview();
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::nApply
// Purpose : SLOT called when "Apply" button pressed.
//=======================================================================
bool SMESHGUI_MoveNodesDlg::onApply()
{
  if (SMESHGUI::GetSMESHGUI()->ActiveStudyLocked())
    return false;

  if ( !isValid( true ) )
    return false;

  SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( myMeshActor->getIO() );
  if(aMesh->_is_nil() )
  {
    QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
    tr( "SMESH_ERROR" ), tr( "SMESHG_NO_MESH" ), QMessageBox::Ok );
    return false;
  }

  SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
  if ( aMeshEditor->_is_nil() )
    return false;

  int anId = myId->text().toInt();
  bool aResult = false;
  try
  {
    aResult = aMeshEditor->MoveNode( anId, myX->GetValue(), myY->GetValue(), myZ->GetValue() );
  }
  catch( ... )
  {
  }

  if ( aResult )
  {
    Handle(SALOME_InteractiveObject) anIO = myMeshActor->getIO();
    mySelection->ClearIObjects();
    SMESH::UpdateView();
    mySelection->AddIObject( anIO, false );
    reset();
  }

  return aResult;
}


//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_MoveNodesDlg::onOk()
{
  if ( onApply() )
    onClose();
}


//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_MoveNodesDlg::onClose()
{
  mySelection->ClearIObjects();
  SMESH::SetPointRepresentation(false);
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
  SMESHGUI::GetSMESHGUI()->ResetState();
  reject();
}



//=======================================================================
//function : onTextChange
//purpose  : 
//=======================================================================

void SMESHGUI_MoveNodesDlg::onTextChange(const QString& theNewText)
{
  if ( myBusy ) return;

  myOkBtn->setEnabled( false );
  myApplyBtn->setEnabled( false );
  erasePreview();

  // select entered node
  SMDS_Mesh* aMesh = 0;
  if ( myMeshActor )
    aMesh = myMeshActor->GetObject()->GetMesh();
  if ( aMesh ) {

    myBusy = true;
    mySelection->ClearIObjects();
    mySelection->AddIObject( myMeshActor->getIO() );
    myBusy = false;

    const SMDS_MeshElement * e = aMesh->FindElement( theNewText.toInt() );
    if ( e )
      mySelection->AddOrRemoveIndex (myMeshActor->getIO(), e->GetID(), true);
  }
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_MoveNodesDlg::onSelectionDone()
{
  if ( myBusy ) return;
  myMeshActor = 0;

  if ( mySelection->IObjectCount() == 1 ) {
    myMeshActor = SMESH::FindActorByEntry(mySelection->firstIObject()->getEntry());
    if ( myMeshActor )
    {
      QString aText;
      if ( SMESH::GetNameOfSelectedNodes( mySelection, aText ) == 1 ) {
        if(SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh()) {
          if(const SMDS_MeshNode* aNode = aMesh->FindNode(aText.toInt())) {
            myBusy = true;
            myId->setText( aText );
            myX->SetValue( aNode->X() );
            myY->SetValue( aNode->Y() );
            myZ->SetValue( aNode->Z() );
            myBusy = false;
            erasePreview(); // avoid overlapping of a selection and a preview
            updateButtons();
            return;
          }
        }
      }
    }
  }

  reset();
}


//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_MoveNodesDlg::onDeactivate()
{
  setEnabled( false );
  erasePreview();
}


//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_MoveNodesDlg::enterEvent( QEvent* )
{
  if ( !isEnabled() )
  {
    SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();

    // set selection mode
    SMESH::SetPointRepresentation(true);
    QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );

    redisplayPreview();
  
    setEnabled( true );
  }
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::closeEvent( QCloseEvent* e )
{
  onClose() ;                  
  SMESH::GetCurrentVtkView()->Repaint();
}

//=======================================================================
//function : hideEvent
//purpose  : may be caused by ESC key
//=======================================================================

void SMESHGUI_MoveNodesDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    onClose();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::updateButtons
// Purpose : Update buttons state
//=======================================================================
void SMESHGUI_MoveNodesDlg::updateButtons()
{
  bool isEnabled = isValid( false );
  myOkBtn->setEnabled( isEnabled );
  myApplyBtn->setEnabled( isEnabled );
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::erasePreview
// Purpose : Erase preview
//=======================================================================
void  SMESHGUI_MoveNodesDlg::erasePreview()
{
  if ( myPreviewActor == 0 )
    return;

  if ( VTKViewer_ViewFrame* vf = SMESH::GetCurrentVtkView() )
    vf->RemoveActor(myPreviewActor);
  myPreviewActor->Delete();
  myPreviewActor = 0;
  SMESH::GetCurrentVtkView()->Repaint();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::redisplayPreview
// Purpose : Redisplay preview
//=======================================================================
void SMESHGUI_MoveNodesDlg::redisplayPreview()
{
  if ( myBusy )
    return;
  
  if ( myPreviewActor != 0 )
    erasePreview();
  
  if ( !isValid( false ) )
    return;

  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();
  
  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints( 1 );
  aPoints->SetPoint( 0, myX->GetValue(), myY->GetValue(), myZ->GetValue() );

  // Create cells
  
  vtkIdList *anIdList = vtkIdList::New();
  anIdList->SetNumberOfIds( 1 );

  vtkCellArray *aCells = vtkCellArray::New();
  aCells->Allocate( 2, 0 );

  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents( 1 );
  aCellTypesArray->Allocate( 1 );

  anIdList->SetId( 0, 0 );
  aCells->InsertNextCell( anIdList );
  aCellTypesArray->InsertNextValue( VTK_VERTEX );

  vtkIntArray* aCellLocationsArray = vtkIntArray::New();
  aCellLocationsArray->SetNumberOfComponents( 1 );
  aCellLocationsArray->SetNumberOfTuples( 1 );

  aCells->InitTraversal();
  vtkIdType npts;
  aCellLocationsArray->SetValue( 0, aCells->GetTraversalLocation( npts ) );

  aGrid->SetCells( aCellTypesArray, aCellLocationsArray, aCells );

  aGrid->SetPoints( aPoints );
  aGrid->SetCells( aCellTypesArray, aCellLocationsArray,aCells );

  // Create and display actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInput( aGrid );

  myPreviewActor = SALOME_Actor::New();
  myPreviewActor->PickableOff();
  myPreviewActor->SetMapper( aMapper );

  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor( 250, 0, 250 );
  aProp->SetPointSize( 5 );
  myPreviewActor->SetProperty( aProp );

  SMESH::GetCurrentVtkView()->AddActor( myPreviewActor );
  SMESH::GetCurrentVtkView()->Repaint();

  aProp->Delete();
  aCellLocationsArray->Delete();
  aCellTypesArray->Delete();
  aCells->Delete();
  anIdList->Delete(); 
  aPoints->Delete();
  aGrid->Delete();
}
  
