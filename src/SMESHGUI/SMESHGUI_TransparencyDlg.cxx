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
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qapplication.h>
#include <qgroupbox.h>

#include "VTKViewer_ViewFrame.h"
#include "VTKViewer_RenderWindowInteractor.h"
#include "QAD_RightFrame.h"
#include "SALOME_ListIteratorOfListIO.hxx"

//=================================================================================
// class    : SMESHGUI_TransparencyDlg()
// purpose  : 
//
//=================================================================================
SMESHGUI_TransparencyDlg::SMESHGUI_TransparencyDlg( QWidget* parent,
						    const char* name,
						    SALOME_Selection* Sel,
						    bool modal,
						    WFlags fl )
  : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  if ( !name )
    setName( "SMESHGUI_TransparencyDlg" );
  resize( 152, 107 ); 
  setCaption( tr( "SMESH_TRANSPARENCY_TITLE"  ) );
  setSizeGripEnabled( TRUE );
  SMESHGUI_TransparencyDlgLayout = new QGridLayout( this ); 
  SMESHGUI_TransparencyDlgLayout->setSpacing( 6 );
  SMESHGUI_TransparencyDlgLayout->setMargin( 11 );

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
  buttonOk->setText( tr( "GEOM_BUT_OK" ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0 );
  GroupButtonsLayout->addWidget( buttonOk, 0, 1 );
  GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2 );

  /*************************************************************************/
  QGroupBox* GroupC1 = new QGroupBox( this, "GroupC1" );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );
  
  TextLabelOpaque = new QLabel( GroupC1, "TextLabelOpaque" );
  TextLabelOpaque->setText( tr( "SMESH_TRANSPARENCY_OPAQUE"  ) );
  TextLabelOpaque->setAlignment( int( QLabel::AlignLeft ) );
  GroupC1Layout->addWidget( TextLabelOpaque, 0, 0 );
  GroupC1Layout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 1 );
  
  TextLabelTransparent = new QLabel( GroupC1, "TextLabelTransparent" );
  TextLabelTransparent->setText( tr( "SMESH_TRANSPARENCY_TRANSPARENT"  ) );
  TextLabelTransparent->setAlignment( int( QLabel::AlignRight ) );
  GroupC1Layout->addWidget( TextLabelTransparent, 0, 2 );
  
  Slider1 = new QSlider( 0, 10, 1, 5, Horizontal, GroupC1, "Slider1" );
  Slider1->setMinimumSize( 300, 0 );
  Slider1->setTickmarks( QSlider::Left );
  GroupC1Layout->addMultiCellWidget( Slider1, 1, 1, 0, 2 );

  SMESHGUI_TransparencyDlgLayout->addWidget( GroupC1,      0, 0 );
  SMESHGUI_TransparencyDlgLayout->addWidget( GroupButtons, 1, 0 );
  
  /* Initialisations */
  this->mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  this->mySel = Sel ;
  
  /* First call valueChanged() method for initialisation               */
  /* The default value of transparency will change with the selection  */
  this->myFirstInit = true ;
//  Slider1->setMaxValue( 10 );
//  Slider1->setValue( 5 ) ;


  this->ValueHasChanged( Slider1->value() ) ;
  
  // signals and slots connections : after ValueHasChanged()
  connect( buttonOk, SIGNAL( clicked() ),         this, SLOT( ClickOnOk() ) );
  connect( Slider1,  SIGNAL( valueChanged(int) ), this, SLOT( ValueHasChanged(int) ) );
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* Displays this Dialog */
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
  accept() ;
  return ;
}


//=======================================================================
// function : ClickOnClose()
// purpose  :
//=======================================================================
void SMESHGUI_TransparencyDlg::ClickOnClose()
{
  accept() ;
  return ;
}


//=================================================================================
// function : ValueHasChanged()
// purpose  : Called when value of slider change
//          : or the first time as initilisation
//=================================================================================
void SMESHGUI_TransparencyDlg::ValueHasChanged( int newValue )
{

  if ( mySMESHGUI->GetActiveStudy()->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {
    VTKViewer_RenderWindowInteractor* myRenderInter= ((VTKViewer_ViewFrame*)mySMESHGUI->GetActiveStudy()->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRWInteractor();
    SALOME_ListIteratorOfListIO It( this->mySel->StoredIObjects() );
    Handle(SALOME_InteractiveObject) FirstIOS =  mySel->firstIObject();
    if( !FirstIOS.IsNull() ) {
      /* The first time as initialisation */
      if( this->myFirstInit ) {	
	this->myFirstInit = false ;
	float transp = ( myRenderInter->GetTransparency(FirstIOS))*10.0 ;
        this->Slider1->setValue( int(transp) ) ;
	return;
      }
    }
    
    QApplication::setOverrideCursor( Qt::waitCursor );
    for( ;It.More(); It.Next() ) {
      Handle(SALOME_InteractiveObject) IOS = It.Value();
      myRenderInter->SetTransparency( IOS, newValue/10.0 );
    }
    QApplication::restoreOverrideCursor();
  }
  QApplication::restoreOverrideCursor();
  return ;
}
