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
//  File   : SMESHGUI_DeleteGroupDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_DeleteGroupDlg.h"

#include "SMESHGUI.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include "QAD_Desktop.h"
#include "SALOME_Selection.h"
#include "SALOME_ListIteratorOfListIO.hxx"

#include <qframe.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlist.h>
#include <qmessagebox.h>

#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#define SPACING 5
#define MARGIN  10

/*
  Class       : SMESHGUI_DeleteGroupDlg
  Description : Delete groups and their contents
*/

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::SMESHGUI_DeleteGroupDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_DeleteGroupDlg::SMESHGUI_DeleteGroupDlg( QWidget*          theParent, 
                                                  SALOME_Selection* theSelection )
: QDialog( theParent, "SMESHGUI_DeleteGroupDlg", false, 
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
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
// name    : SMESHGUI_DeleteGroupDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_DeleteGroupDlg::createMainFrame( QWidget* theParent )
{
  QGroupBox* aMainGrp = new QGroupBox( 1, Qt::Horizontal, tr( "SELECTED_GROUPS" ), theParent );
  
  myListBox = new QListBox( aMainGrp );
  myListBox->setMinimumHeight( 100 );
  myListBox->setSelectionMode( QListBox::NoSelection );
  myListBox->setRowMode( QListBox::FitToWidth );
  
  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_DeleteGroupDlg::createButtonFrame( QWidget* theParent )
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
  
  // connect signals and slots
  connect( myOkBtn,    SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myCloseBtn, SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( myApplyBtn, SIGNAL( clicked() ), SLOT( onApply() ) );
  
  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::~SMESHGUI_DeleteGroupDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_DeleteGroupDlg::~SMESHGUI_DeleteGroupDlg()
{
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_DeleteGroupDlg::Init( SALOME_Selection* theSelection )
{
  myBlockSelection = false;
  mySelection = theSelection;  
  SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI();
  aSMESHGUI->SetActiveDialogBox( ( QDialog* )this ) ;
  
  // selection and SMESHGUI
  connect( mySelection, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );
  connect( aSMESHGUI, SIGNAL( SignalDeactivateActiveDialog() ), SLOT( onDeactivate() ) );
  connect( aSMESHGUI, SIGNAL( SignalCloseAllDialogs() ), SLOT( ClickOnClose() ) );
  
  int x, y ;
  aSMESHGUI->DefineDlgPosition( this, x, y );
  this->move( x, y );
  this->show(); 

  // set selection mode
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection, true ); 
  mySelection->AddFilter( new SMESH_TypeFilter( GROUP ) );
  onSelectionDone();

  return;
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_DeleteGroupDlg::isValid()
{
  if ( myListBox->count() == 0 )
  {
    QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
      tr( "SMESH_INSUFFICIENT_DATA" ), tr( "NO_SELECTED_GROUPS" ), QMessageBox::Ok ); 
    return false;
  }
  
  return !SMESHGUI::GetSMESHGUI()->ActiveStudyLocked();
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::onApply
// Purpose : SLOT called when "Apply" button pressed. 
//=======================================================================
bool SMESHGUI_DeleteGroupDlg::onApply()
{
  if ( !isValid() )
    return false;

  myBlockSelection = true;
  
  QValueList<SMESH::SMESH_GroupBase_var>::iterator anIter;
  for ( anIter = myListGrp.begin(); anIter != myListGrp.end(); ++anIter )
  {
    SMESH::SMESH_Mesh_ptr aMesh = (*anIter)->GetMesh();
    if ( !aMesh->_is_nil() )
      aMesh->RemoveGroupWithContents( *anIter );
  }

  myListBox->clear();
  myListGrp.clear();
  mySelection->ClearIObjects();
  SMESH::UpdateView();
  SMESHGUI::GetSMESHGUI()->GetActiveStudy()->updateObjBrowser( true );
   
  myBlockSelection = false;
  return false;
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::onOk
// Purpose : SLOT called when "Ok" button pressed. 
//=======================================================================
void SMESHGUI_DeleteGroupDlg::onOk()
{
  if ( onApply() )
    onClose();
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_DeleteGroupDlg::onClose()
{
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
  SMESHGUI::GetSMESHGUI()->ResetState() ;
  mySelection->ClearFilters();
  reject();
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_DeleteGroupDlg::onSelectionDone()
{
  if ( myBlockSelection )
    return;
  
  myListGrp.clear();
  QStringList aNames;
  
  const SALOME_ListIO& aListIO = mySelection->StoredIObjects();
  SALOME_ListIteratorOfListIO anIter( aListIO );
  for( ; anIter.More(); anIter.Next() )
  {
    SMESH::SMESH_GroupBase_var aGroup = 
      SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>( anIter.Value() );
    if ( !aGroup->_is_nil() )
    {
      aNames.append( aGroup->GetName() );
      myListGrp.append( aGroup );
    }
  }
    
  myListBox->clear();
  myListBox->insertStringList( aNames );
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_DeleteGroupDlg::onDeactivate()
{
  mySelection->ClearFilters();
  setEnabled( false );
}

//=======================================================================
// name    : SMESHGUI_DeleteGroupDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_DeleteGroupDlg::enterEvent( QEvent* )
{
  SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog() ;   
  setEnabled( true );
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection, true ); 
  mySelection->AddFilter( new SMESH_TypeFilter( GROUP ) );
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_DeleteGroupDlg::closeEvent( QCloseEvent* )
{
  onClose() ;
}

