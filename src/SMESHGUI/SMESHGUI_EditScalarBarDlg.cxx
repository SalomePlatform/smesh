using namespace std;
//  File      : SMESHGUI_EditScalarBarDlg.cxx
//  Created   : Wed Jun 12 12:01:26 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#include "SMESHGUI_EditScalarBarDlg.h"
#include "SMESHGUI.h"

// QT Includes
#include <qframe.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a SMESHGUI_EditScalarBarDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SMESHGUI_EditScalarBarDlg::SMESHGUI_EditScalarBarDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    if ( !name )
	setName( "SMESHGUI_EditScalarBarDlg" );
    resize( 124, 122 ); 
    setCaption( tr( "SMESH_SCALARBAR"  ) );
    setSizeGripEnabled( TRUE );
    grid = new QGridLayout( this ); 
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    grid_2 = new QGridLayout; 
    grid_2->setSpacing( 6 );
    grid_2->setMargin( 0 );

    PushButtonUpdateView = new QPushButton( this, "PushButtonUpdateView" );
    PushButtonUpdateView->setText( tr( "SMESH_UPDATEVIEW"  ) );

    grid_2->addWidget( PushButtonUpdateView, 1, 0 );

    grid_3 = new QGridLayout; 
    grid_3->setSpacing( 6 );
    grid_3->setMargin( 0 );

    grid_4 = new QGridLayout; 
    grid_4->setSpacing( 6 );
    grid_4->setMargin( 0 );

    grid_5 = new QGridLayout; 
    grid_5->setSpacing( 6 );
    grid_5->setMargin( 0 );

    LineEditMax = new QLineEdit( this, "LineEditMax" );

    grid_5->addWidget( LineEditMax, 0, 0 );

    LineEditMin = new QLineEdit( this, "LineEditMin" );

    grid_5->addWidget( LineEditMin, 1, 0 );

    grid_4->addLayout( grid_5, 0, 1 );

    grid_6 = new QGridLayout; 
    grid_6->setSpacing( 6 );
    grid_6->setMargin( 0 );

    TextLabelMax = new QLabel( this, "TextLabelMax" );
    TextLabelMax->setText( tr( "SMESH_MAX"  ) );

    grid_6->addWidget( TextLabelMax, 0, 0 );

    TextLabelMin = new QLabel( this, "TextLabelMin" );
    TextLabelMin->setText( tr( "SMESH_MIN"  ) );

    grid_6->addWidget( TextLabelMin, 1, 0 );

    grid_4->addLayout( grid_6, 0, 0 );

    grid_3->addLayout( grid_4, 0, 0 );

    Line1 = new QFrame( this, "Line1" );
    Line1->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    grid_3->addWidget( Line1, 1, 0 );

    grid_2->addLayout( grid_3, 0, 0 );

    grid->addLayout( grid_2, 0, 0 );

    mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
    if ( mySMESHGUI && mySMESHGUI->GetScalarBar() && mySMESHGUI->GetScalarBar()->GetLookupTable() ) {
      float *range = mySMESHGUI->GetScalarBar()->GetLookupTable()->GetRange();
      LineEditMin->setText( QString("%1").arg(range[0]) );
      LineEditMax->setText( QString("%1").arg(range[1]) );
    }
    
    // signals and slots connections
    connect( PushButtonUpdateView, SIGNAL( clicked() ), this, SLOT( updateView() ) );
    /* to close dialog if study change */
    connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( reject() ) ) ;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SMESHGUI_EditScalarBarDlg::~SMESHGUI_EditScalarBarDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void SMESHGUI_EditScalarBarDlg::updateView()
{
  float MinRange = LineEditMin->text().toFloat();
  float MaxRange = LineEditMax->text().toFloat(); 
  mySMESHGUI->UpdateScalarBar(MinRange,MaxRange);
}
