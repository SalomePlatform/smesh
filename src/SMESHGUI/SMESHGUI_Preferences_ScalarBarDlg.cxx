using namespace std;
//  File      : SMESHGUI_Preferences_ScalarBarDlg.cxx
//  Created   : Tue Jun 11 17:23:32 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#include "SMESHGUI_Preferences_ScalarBarDlg.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a SMESHGUI_Preferences_ScalarBarDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SMESHGUI_Preferences_ScalarBarDlg::SMESHGUI_Preferences_ScalarBarDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    if ( !name )
	setName( "SMESHGUI_Preferences_ScalarBarDlg" );
    setCaption( tr( "SMESH_PREFERENCES_SCALARBAR"  ) );
    setSizeGripEnabled( TRUE );

    grid = new QGridLayout( this ); 
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    /******************************************************************************/
    Properties = new QGroupBox( this, "Properties" );
    Properties->setTitle( tr( "SMESH_PROPERTIES"  ) );
    Properties->setColumnLayout(0, Qt::Vertical );
    Properties->layout()->setSpacing( 0 );
    Properties->layout()->setMargin( 0 );
    grid_4 = new QGridLayout( Properties->layout() );
    grid_4->setAlignment( Qt::AlignTop );
    grid_4->setSpacing( 6 );
    grid_4->setMargin( 11 );

    /* Font */
    grid_5 = new QGridLayout; 
    grid_5->setSpacing( 6 );
    grid_5->setMargin( 0 );
    TextLabel2 = new QLabel( Properties, "TextLabel2" );
    TextLabel2->setText( tr( "SMESH_FONT"  ) );
    grid_5->addWidget( TextLabel2, 0, 0 );
    ComboBox1 = new QComboBox( FALSE, Properties, "ComboBox1" );
    ComboBox1->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    ComboBox1->insertItem( tr( "SMESH_FONT_ARIAL" ) );
    ComboBox1->insertItem( tr( "SMESH_FONT_COURIER" ) );
    ComboBox1->insertItem( tr( "SMESH_FONT_TIMES" ) );
    grid_5->addWidget( ComboBox1, 0, 1 );
    grid_4->addLayout( grid_5, 0, 0 );
    
    /* Font attributes */
    grid_6 = new QGridLayout; 
    grid_6->setSpacing( 6 );
    grid_6->setMargin( 0 );
    Bold = new QCheckBox( Properties, "Bold" );
    Bold->setText( tr( "SMESH_FONT_BOLD"  ) );
    grid_6->addWidget( Bold, 0, 0 );
    Italic = new QCheckBox( Properties, "Italic" );
    Italic->setText( tr( "SMESH_FONT_ITALIC"  ) );
    grid_6->addWidget( Italic, 0, 1 );
    Shadow = new QCheckBox( Properties, "Shadow" );
    Shadow->setText( tr( "SMESH_FONT_SHADOW"  ) );
    grid_6->addWidget( Shadow, 0, 2 );
    grid_4->addLayout( grid_6, 1, 0 );

    grid_7 = new QGridLayout; 
    grid_7->setSpacing( 6 );
    grid_7->setMargin( 0 );
    NumberColors = new QLabel( Properties, "NumberColors" );
    NumberColors->setText( tr( "SMESH_NUMBEROFCOLORS"  ) );
    grid_7->addWidget( NumberColors, 0, 0 );
    SpinBoxColors = new QSpinBox( Properties, "SpinBoxColors" );
    SpinBoxColors->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    SpinBoxColors->setMinValue( 1 );
    grid_7->addWidget( SpinBoxColors, 0, 1 );
    NumberLabels = new QLabel( Properties, "NumberLabels" );
    NumberLabels->setText( tr( "SMESH_NUMBEROFLABELS"  ) );
    grid_7->addWidget( NumberLabels, 1, 0 );
    SpinBoxLabels = new QSpinBox( Properties, "SpinBoxLabels" );
    SpinBoxLabels->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    SpinBoxLabels->setMinValue( 1 );
    grid_7->addWidget( SpinBoxLabels, 1, 1 );
    grid_4->addLayout( grid_7, 2, 0 );

    grid->addWidget( Properties, 0, 0 );

    /******************************************************************************/
    ButtonGroup_Orientation = new QButtonGroup( this, "ButtonGroup_Orientation" );
    ButtonGroup_Orientation->setTitle( tr( "SMESH_ORIENTATION"  ) );
    ButtonGroup_Orientation->setColumnLayout(0, Qt::Vertical );
    ButtonGroup_Orientation->layout()->setSpacing( 0 );
    ButtonGroup_Orientation->layout()->setMargin( 0 );
    grid_2 = new QGridLayout( ButtonGroup_Orientation->layout() );
    grid_2->setAlignment( Qt::AlignTop );
    grid_2->setSpacing( 6 );
    grid_2->setMargin( 11 );
    RadioVert = new QRadioButton( ButtonGroup_Orientation, "RadioVert" );
    RadioVert->setText( tr( "SMESH_VERTICAL"  ) );
    RadioHoriz = new QRadioButton( ButtonGroup_Orientation, "RadioHoriz" );
    RadioHoriz->setText( tr( "SMESH_HORIZONTAL"  ) );
    grid_2->addWidget( RadioVert, 0, 0 );
    grid_2->addWidget( RadioHoriz, 0, 1 );

    grid->addWidget( ButtonGroup_Orientation, 1, 0 );

    /******************************************************************************/
    GroupBox5 = new QGroupBox( this, "GroupBox5" );
    GroupBox5->setTitle( tr( "SMESH_DIMENSIONS"  ) );
    GroupBox5->setColumnLayout(0, Qt::Vertical );
    GroupBox5->layout()->setSpacing( 0 );
    GroupBox5->layout()->setMargin( 0 );
    grid_11 = new QGridLayout( GroupBox5->layout() );
    grid_11->setAlignment( Qt::AlignTop );
    grid_11->setSpacing( 6 );
    grid_11->setMargin( 11 );

    LineEditWidth = new QLineEdit( GroupBox5, "LineEditWidth" );
    grid_11->addWidget( LineEditWidth, 0, 0 );
    Width = new QLabel( GroupBox5, "Width" );
    Width->setText( tr( "SMESH_WIDTH"  ) );
    grid_11->addWidget( Width, 0, 1 );
    LineEditHeight = new QLineEdit( GroupBox5, "LineEditHeight" );
    grid_11->addWidget( LineEditHeight, 1, 0 );
    Height = new QLabel( GroupBox5, "Height" );
    Height->setText( tr( "SMESH_HEIGHT"  ) );
    grid_11->addWidget( Height, 1, 1 );

    grid->addWidget( GroupBox5, 2, 0 );

    /***************************************************************/
    QGroupBox* GroupButtons = new QGroupBox( this, "GroupButtons" );
    GroupButtons->setGeometry( QRect( 10, 10, 281, 48 ) ); 
    GroupButtons->setTitle( tr( ""  ) );
    GroupButtons->setColumnLayout(0, Qt::Vertical );
    GroupButtons->layout()->setSpacing( 0 );
    GroupButtons->layout()->setMargin( 0 );
    grid_15 = new QGridLayout( GroupButtons->layout() );
    grid_15->setAlignment( Qt::AlignTop );
    grid_15->setSpacing( 6 );
    grid_15->setMargin( 11 );
    buttonOk = new QPushButton( GroupButtons, "buttonOk" );
    buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    grid_15->addWidget( buttonOk, 0, 0 );
    grid_15->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 1 );
    buttonCancel = new QPushButton( GroupButtons, "buttonCancel" );
    buttonCancel->setText( tr( "SMESH_BUT_CANCEL"  ) );
    buttonCancel->setAutoDefault( TRUE );
    grid_15->addWidget( buttonCancel, 0, 2 );

    grid->addWidget( GroupButtons, 3, 0 );

    // signals and slots connections
    connect( buttonOk,     SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SMESHGUI_Preferences_ScalarBarDlg::~SMESHGUI_Preferences_ScalarBarDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

