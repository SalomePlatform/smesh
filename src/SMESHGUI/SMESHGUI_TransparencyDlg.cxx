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
//  File   : SMESHGUI_TransparencyDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_TransparencyDlg.h"
#include "SMESHGUI.h"

// QT Includes
#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qgroupbox.h>

#include "VTKViewer_ViewFrame.h"
#include "VTKViewer_RenderWindowInteractor.h"
#include "QAD_RightFrame.h"
#include "QAD_WaitCursor.h"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SMESH_Actor.h"
#include "SALOME_Selection.h"
#include "SALOME_InteractiveObject.hxx"

static SMESH_Actor* FindActorByEntry(const char* theEntry)
{
  QAD_Study* aStudy = SMESHGUI::GetSMESHGUI()->GetActiveStudy();
  QAD_StudyFrame *aStudyFrame = aStudy->getActiveStudyFrame();
  VTKViewer_ViewFrame* aViewFrame = dynamic_cast<VTKViewer_ViewFrame*>( aStudyFrame->getRightFrame()->getViewFrame() );

  if(aViewFrame){
    vtkRenderer *aRenderer = aViewFrame->getRenderer();
    vtkActorCollection *aCollection = aRenderer->GetActors();
    aCollection->InitTraversal();
    while(vtkActor *anAct = aCollection->GetNextActor()){
      if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	if(anActor->hasIO()){
	  Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
	  if(anIO->hasEntry() && strcmp(anIO->getEntry(),theEntry) == 0){
	    return anActor;
	  }
	}
      }
    }
  }
  return NULL;
}

//=================================================================================
// class    : SMESHGUI_TransparencyDlg()
// purpose  : 
//
//=================================================================================
SMESHGUI_TransparencyDlg::SMESHGUI_TransparencyDlg( QWidget* parent,
						    const char* name,
						    bool modal,
						    WFlags fl )
  : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose )
{
  if ( !name )
    setName( "SMESHGUI_TransparencyDlg" );
  setCaption( tr( "SMESH_TRANSPARENCY_TITLE"  ) );
  setSizeGripEnabled( TRUE );
  QGridLayout* SMESHGUI_TransparencyDlgLayout = new QGridLayout( this ); 
  SMESHGUI_TransparencyDlgLayout->setSpacing( 6 );
  SMESHGUI_TransparencyDlgLayout->setMargin( 11 );

  /*************************************************************************/
  QGroupBox* GroupC1 = new QGroupBox( this, "GroupC1" );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );
  
  TextLabelTransparent = new QLabel( GroupC1, "TextLabelTransparent" );
  TextLabelTransparent->setText( tr( "SMESH_TRANSPARENCY_TRANSPARENT"  ) );
  TextLabelTransparent->setAlignment( AlignLeft );
  GroupC1Layout->addWidget( TextLabelTransparent, 0, 0 );
  
  ValueLab = new QLabel( GroupC1, "ValueLab" );
  ValueLab->setAlignment( AlignCenter );
  ValueLab->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  QFont fnt = ValueLab->font(); fnt.setBold( true ); ValueLab->setFont( fnt );
  GroupC1Layout->addWidget( ValueLab, 0, 1 );

  TextLabelOpaque = new QLabel( GroupC1, "TextLabelOpaque" );
  TextLabelOpaque->setText( tr( "SMESH_TRANSPARENCY_OPAQUE"  ) );
  TextLabelOpaque->setAlignment( AlignRight );
  GroupC1Layout->addWidget( TextLabelOpaque, 0, 2 );
  
  Slider1 = new QSlider( 0, 10, 1, 5, Horizontal, GroupC1, "Slider1" );
  Slider1->setFocusPolicy( QWidget::NoFocus );
  Slider1->setMinimumSize( 300, 0 );
  Slider1->setTickmarks( QSlider::Above );
  Slider1->setTickInterval( 10 );
  Slider1->setTracking( true );
  Slider1->setMinValue( 0 ) ;
  Slider1->setMaxValue( 100 );
  Slider1->setLineStep( 1 );
  Slider1->setPageStep( 10 );
  GroupC1Layout->addMultiCellWidget( Slider1, 1, 1, 0, 2 );

  /*************************************************************************/
  QGroupBox* GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  QGridLayout* GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );

  buttonOk = new QPushButton( GroupButtons, "buttonOk" );
  buttonOk->setText( tr( "SMESH_BUT_CLOSE" ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0 );
  GroupButtonsLayout->addWidget( buttonOk, 0, 1 );
  GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2 );

  SMESHGUI_TransparencyDlgLayout->addWidget( GroupC1,      0, 0 );
  SMESHGUI_TransparencyDlgLayout->addWidget( GroupButtons, 1, 0 );
  
  mySelection = SALOME_Selection::Selection( SMESHGUI::GetSMESHGUI()->GetActiveStudy()->getSelection());

  // Initial state
  this->onSelectionChanged() ;
  
  // signals and slots connections : after ValueHasChanged()
  connect( buttonOk, SIGNAL( clicked() ),         this, SLOT( ClickOnOk() ) );
  connect( Slider1,  SIGNAL( valueChanged(int) ), this, SLOT( SetTransparency() ) );
  connect( Slider1,  SIGNAL( sliderMoved(int) ),  this, SLOT( ValueHasChanged() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnOk() ) ) ;
  connect( mySelection,  SIGNAL( currentSelectionChanged() ), this, SLOT( onSelectionChanged() ) );
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  SMESHGUI::GetSMESHGUI()->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show();
}


//=================================================================================
// function : ~SMESHGUI_TransparencyDlg()
// purpose  :
//=================================================================================
SMESHGUI_TransparencyDlg::~SMESHGUI_TransparencyDlg()
{
  // no need to delete child widgets, Qt does it all for us
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
// function : SetTransparency()
// purpose  : Called when value of slider change
//          : or the first time as initilisation
//=================================================================================
void SMESHGUI_TransparencyDlg::SetTransparency()
{
  if ( SMESHGUI::GetSMESHGUI()->GetActiveStudy()->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {
    QAD_WaitCursor wc;
    float opacity = this->Slider1->value() / 100. ;
    SALOME_ListIteratorOfListIO It( mySelection->StoredIObjects() );
    for( ;It.More(); It.Next() ) {
      Handle(SALOME_InteractiveObject) IOS = It.Value();
      SMESH_Actor* anActor = FindActorByEntry(IOS->getEntry());
      if ( anActor )
	anActor->SetOpacity( opacity );
    }
    SMESHGUI::GetSMESHGUI()->GetActiveStudy()->getActiveStudyFrame()->getRightFrame()->getViewFrame()->Repaint();
  }
  ValueHasChanged();
}

//=================================================================================
// function : ValueHasChanged()
// purpose  : Called when user moves a slider
//=================================================================================
void SMESHGUI_TransparencyDlg::ValueHasChanged()
{
  ValueLab->setText( QString::number( this->Slider1->value() ) + "%" );
}

//=================================================================================
// function : onSelectionChanged()
// purpose  : Called when selection is changed
//=================================================================================
void SMESHGUI_TransparencyDlg::onSelectionChanged()
{
  if ( SMESHGUI::GetSMESHGUI()->GetActiveStudy()->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {
    int opacity = 100;
    if ( mySelection->IObjectCount() == 1 ) {
      Handle(SALOME_InteractiveObject) FirstIOS = mySelection->firstIObject();
      if( !FirstIOS.IsNull() ) {
	SMESH_Actor* anActor = FindActorByEntry( FirstIOS->getEntry() );
	if ( anActor )
	  opacity = int( anActor->GetOpacity() * 100. + 0.5 );
      }
    }
    else if ( mySelection->IObjectCount() > 1 ) {
      SALOME_ListIteratorOfListIO It( mySelection->StoredIObjects() );
      int setOp = -1;
      for ( ; It.More(); It.Next() ) {
	Handle(SALOME_InteractiveObject) IO = It.Value();
	if( !IO.IsNull() ) {
	  SMESH_Actor* anActor = FindActorByEntry( IO->getEntry() );
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
    Slider1->setValue( opacity ) ;
  }
  ValueHasChanged();
}
