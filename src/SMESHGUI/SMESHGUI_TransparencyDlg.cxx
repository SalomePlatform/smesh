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
// File   : SMESHGUI_TransparencyDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_TransparencyDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESH_Actor.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>

#include <SALOME_ListIO.hxx>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewWindow.h>

// Qt includes
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>

#define SPACING 6
#define MARGIN  11

//=================================================================================
// class    : SMESHGUI_TransparencyDlg()
// purpose  :
//
//=================================================================================
SMESHGUI_TransparencyDlg::SMESHGUI_TransparencyDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myViewWindow( SMESH::GetViewWindow( theModule ) )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "SMESH_TRANSPARENCY_TITLE" ) );
  setSizeGripEnabled( true );

  QVBoxLayout* SMESHGUI_TransparencyDlgLayout = new QVBoxLayout( this );
  SMESHGUI_TransparencyDlgLayout->setSpacing( SPACING );
  SMESHGUI_TransparencyDlgLayout->setMargin( MARGIN );

  /*************************************************************************/
  QGroupBox* GroupC1 = new QGroupBox( this );
  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1 );
  GroupC1Layout->setSpacing( SPACING );
  GroupC1Layout->setMargin( MARGIN );

  TextLabelTransparent = new QLabel( tr( "SMESH_TRANSPARENCY_TRANSPARENT" ), GroupC1 );
  TextLabelTransparent->setAlignment( Qt::AlignRight );

  ValueLab = new QLabel( GroupC1 );
  ValueLab->setAlignment( Qt::AlignCenter );
  ValueLab->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  QFont fnt = ValueLab->font(); fnt.setBold( true ); ValueLab->setFont( fnt );

  TextLabelOpaque = new QLabel( tr( "SMESH_TRANSPARENCY_OPAQUE" ), GroupC1 );
  TextLabelOpaque->setAlignment( Qt::AlignLeft );

  Slider1 = new QSlider( Qt::Horizontal, GroupC1 );
  Slider1->setRange( 0, 100 );
  Slider1->setSingleStep( 1 );
  Slider1->setPageStep( 10 );
  Slider1->setTickPosition( QSlider::TicksAbove );
  Slider1->setTickInterval( 10 );
  Slider1->setTracking( true );
  Slider1->setFocusPolicy( Qt::NoFocus );
  Slider1->setMinimumWidth( 300 );

  GroupC1Layout->addWidget( TextLabelOpaque, 0, 0 );
  GroupC1Layout->addWidget( ValueLab, 0, 1 );
  GroupC1Layout->addWidget( TextLabelTransparent, 0, 2 );
  GroupC1Layout->addWidget( Slider1, 1, 0, 1, 3 );

  /*************************************************************************/
  QGroupBox* GroupButtons = new QGroupBox( this );
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout( GroupButtons );
  GroupButtonsLayout->setSpacing( SPACING );
  GroupButtonsLayout->setMargin( MARGIN );

  buttonOk = new QPushButton( tr( "SMESH_BUT_CLOSE" ), GroupButtons );
  buttonOk->setAutoDefault( true );
  buttonOk->setDefault( true );
  buttonHelp = new QPushButton( tr( "SMESH_BUT_HELP" ), GroupButtons );
  buttonHelp->setAutoDefault( true );

  GroupButtonsLayout->addWidget( buttonOk );
  GroupButtonsLayout->addSpacing( 10 );
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget( buttonHelp );  

  /*************************************************************************/
  SMESHGUI_TransparencyDlgLayout->addWidget( GroupC1 );
  SMESHGUI_TransparencyDlgLayout->addWidget( GroupButtons );

  // Initial state
  onSelectionChanged();

  // signals and slots connections : after ValueHasChanged()
  connect( buttonOk,       SIGNAL( clicked() ),                 this, SLOT( ClickOnOk() ) );
  connect( buttonHelp,     SIGNAL( clicked() ),                 this, SLOT( ClickOnHelp() ) );
  connect( Slider1,        SIGNAL( valueChanged( int ) ),       this, SLOT( SetTransparency() ) );
  connect( Slider1,        SIGNAL( sliderMoved( int ) ),        this, SLOT( ValueHasChanged() ) );
  connect( mySMESHGUI,     SIGNAL( SignalCloseAllDialogs() ),   this, SLOT( ClickOnOk() ) );
  connect( mySelectionMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( onSelectionChanged() ) );

  myHelpFileName = "transparency_page.html";
}

//=================================================================================
// function : ~SMESHGUI_TransparencyDlg()
// purpose  :
//=================================================================================
SMESHGUI_TransparencyDlg::~SMESHGUI_TransparencyDlg()
{
}

//=======================================================================
// function : ClickOnOk()
// purpose  :
//=======================================================================
void SMESHGUI_TransparencyDlg::ClickOnOk()
{
  close();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_TransparencyDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)( SUIT_Session::session()->activeApplication() );
  if ( app )
    app->onHelpContextModule( mySMESHGUI ? app->moduleName( mySMESHGUI->moduleName() ) : 
                              QString( "" ), myHelpFileName );
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning( this, tr( "WRN_WARNING" ),
                              tr( "EXTERNAL_BROWSER_CANNOT_SHOW_PAGE" ).
                              arg( app->resourceMgr()->stringValue( "ExternalBrowser", 
                                                                    platform ) ).
                              arg( myHelpFileName ) );
  }
}

//=================================================================================
// function : SetTransparency()
// purpose  : Called when value of slider change
//          : or the first time as initilisation
//=================================================================================
void SMESHGUI_TransparencyDlg::SetTransparency()
{
  if ( myViewWindow ) {
    SUIT_OverrideCursor wc;
    float opacity = ( 100 - Slider1->value() ) / 100.;

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects( aList );

    SALOME_ListIteratorOfListIO It( aList );
    for ( ; It.More(); It.Next() ) {
      Handle(SALOME_InteractiveObject) IOS = It.Value();
      SMESH_Actor* anActor = SMESH::FindActorByEntry( IOS->getEntry() );
      if ( anActor )
        anActor->SetOpacity( opacity );
    }
    myViewWindow->Repaint();
  }
  ValueHasChanged();
}

//=================================================================================
// function : ValueHasChanged()
// purpose  : Called when user moves a slider
//=================================================================================
void SMESHGUI_TransparencyDlg::ValueHasChanged()
{
  ValueLab->setText( QString::number( Slider1->value() ) + "%") ;
}

//=================================================================================
// function : onSelectionChanged()
// purpose  : Called when selection is changed
//=================================================================================
void SMESHGUI_TransparencyDlg::onSelectionChanged()
{
  if ( myViewWindow ) {
    int opacity = 100;

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects( aList );

    if ( aList.Extent() == 1 ) {
      Handle(SALOME_InteractiveObject) FirstIOS = aList.First();
      if ( !FirstIOS.IsNull() ) {
        SMESH_Actor* anActor = SMESH::FindActorByEntry( FirstIOS->getEntry() );
        if ( anActor )
          opacity = int( anActor->GetOpacity() * 100. + 0.5 );
      }
    } 
    else if ( aList.Extent() > 1 ) {
      SALOME_ListIteratorOfListIO It( aList );
      int setOp = -1;
      for ( ; It.More(); It.Next() ) {
        Handle(SALOME_InteractiveObject) IO = It.Value();
        if ( !IO.IsNull() ) {
          SMESH_Actor* anActor = SMESH::FindActorByEntry( IO->getEntry() );
          if ( anActor ) {
            int op = int( anActor->GetOpacity() * 100. + 0.5 );
            if ( setOp < 0 )
              setOp = op;
            else if ( setOp != op ) {
              setOp = 100;
              break;
            }
          }
        }
      }
      if ( setOp >= 0 )
        opacity = setOp;
    } 
    else {
    }
    Slider1->setValue( 100 - opacity );
  }
  ValueHasChanged();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_TransparencyDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
