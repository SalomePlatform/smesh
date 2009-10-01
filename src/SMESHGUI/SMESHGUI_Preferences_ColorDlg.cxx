//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_Preferences_ColorDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Preferences_ColorDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <QtxColorButton.h>
#include <QtxIntSpinBox.h>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpinBox>

#define SPACING 6
#define MARGIN  11

//=================================================================================
// function : SMESHGUI_Preferences_ColorDlg()
// purpose  : Constructs a SMESHGUI_Preferences_ColorDlg which is a child
//            of 'parent', with the name 'name' and widget flags set to 'f'
//            The dialog will by default be modeless, unless you
//            set'modal' to true to construct a modal dialog.
//=================================================================================
SMESHGUI_Preferences_ColorDlg::SMESHGUI_Preferences_ColorDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule )
{
  setModal( true );
  setWindowTitle( tr( "Preferences - Set Color" ) );
  setSizeGripEnabled( true );

  // -------------------------------
  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( SPACING );
  topLayout->setMargin( MARGIN );

  // -------------------------------
  QGroupBox* ButtonGroup1 = new QGroupBox( tr( "Elements" ), this );
  QGridLayout* ButtonGroup1Layout = new QGridLayout( ButtonGroup1 );
  ButtonGroup1Layout->setSpacing( SPACING );
  ButtonGroup1Layout->setMargin( MARGIN );

  QLabel* TextLabel_Fill = new QLabel( tr( "Fill" ), ButtonGroup1 );
  btnFillColor = new QtxColorButton( ButtonGroup1 );

  QLabel* TextLabel_BackFace = new QLabel( tr( "Back Face" ), ButtonGroup1 );
  btnBackFaceColor = new QtxColorButton( ButtonGroup1 );

  QLabel* TextLabel_Outine = new QLabel( tr( "Outline" ), ButtonGroup1 );
  btnOutlineColor = new QtxColorButton( ButtonGroup1 );

  QLabel* TextLabel_Width = new QLabel( tr( "Width" ), ButtonGroup1 );
  SpinBox_Width = new QSpinBox( ButtonGroup1 );
  SpinBox_Width->setRange( 0, 5 );
  SpinBox_Width->setSingleStep( 1 );
  SpinBox_Width->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  SpinBox_Width->setButtonSymbols( QSpinBox::PlusMinus );

  QLabel* TextLabel_ShrinkCoeff = new QLabel( tr( "Shrink coef." ), ButtonGroup1 );
  SpinBox_Shrink = new QtxIntSpinBox( ButtonGroup1 );
  SpinBox_Shrink->setRange( 20, 100 );
  SpinBox_Shrink->setSingleStep( 1 );
  SpinBox_Shrink->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  SpinBox_Shrink->setButtonSymbols( QSpinBox::PlusMinus );

  ButtonGroup1Layout->addWidget( TextLabel_Fill,        0, 0 );
  ButtonGroup1Layout->addWidget( btnFillColor,          0, 1 );
  ButtonGroup1Layout->addWidget( TextLabel_BackFace,    0, 2 );
  ButtonGroup1Layout->addWidget( btnBackFaceColor,      0, 3 );
  ButtonGroup1Layout->addWidget( TextLabel_Outine,      1, 0 );
  ButtonGroup1Layout->addWidget( btnOutlineColor,       1, 1 );
  ButtonGroup1Layout->addWidget( TextLabel_Width,       1, 2 );
  ButtonGroup1Layout->addWidget( SpinBox_Width,         1, 3 );
  ButtonGroup1Layout->addWidget( TextLabel_ShrinkCoeff, 2, 0 );
  ButtonGroup1Layout->addWidget( SpinBox_Shrink,        2, 1, 1, 3 );

  // -------------------------------
  QGroupBox* ButtonGroup2 = new QGroupBox( tr( "Nodes" ), this );
  QHBoxLayout* ButtonGroup2Layout = new QHBoxLayout( ButtonGroup2 );
  ButtonGroup2Layout->setSpacing( SPACING );
  ButtonGroup2Layout->setMargin( MARGIN );

  QLabel* TextLabel_Nodes_Color = new QLabel( tr( "Color" ), ButtonGroup2 );
  btnNodeColor = new QtxColorButton( ButtonGroup2 );

  QLabel* TextLabel_Nodes_Size = new QLabel( tr( "Size" ), ButtonGroup2 );
  SpinBox_Nodes_Size = new QSpinBox( ButtonGroup2 );
  SpinBox_Nodes_Size->setRange( 0, 5 );
  SpinBox_Nodes_Size->setSingleStep( 1 );
  SpinBox_Nodes_Size->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  SpinBox_Nodes_Size->setButtonSymbols( QSpinBox::PlusMinus );

  ButtonGroup2Layout->addWidget( TextLabel_Nodes_Color );
  ButtonGroup2Layout->addWidget( btnNodeColor );
  ButtonGroup2Layout->addWidget( TextLabel_Nodes_Size );
  ButtonGroup2Layout->addWidget( SpinBox_Nodes_Size );

  // -------------------------------
  QGroupBox* GroupButtons = new QGroupBox( this );
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout( GroupButtons );
  GroupButtonsLayout->setSpacing( SPACING );
  GroupButtonsLayout->setMargin( MARGIN );

  QPushButton* buttonOk = new QPushButton( tr( "&OK" ), GroupButtons );
  buttonOk->setAutoDefault( true );
  buttonOk->setDefault( true );

  QPushButton* buttonCancel = new QPushButton( tr( "&Cancel" ), GroupButtons );
  buttonCancel->setAutoDefault( true );

  GroupButtonsLayout->addWidget( buttonOk );
  GroupButtonsLayout->addSpacing( 10 );
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget( buttonCancel );

  // -------------------------------
  topLayout->addWidget( ButtonGroup1 );
  topLayout->addWidget( ButtonGroup2 );
  topLayout->addWidget( GroupButtons );

  // -------------------------------
  mySMESHGUI->SetActiveDialogBox( this );

  /* signals and slots connections */
  connect( buttonOk,     SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) );

  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ),
           this,       SLOT( DeactivateActiveDialog() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ),
           this,       SLOT( ClickOnCancel() ) );
}

//=================================================================================
// function : ~SMESHGUI_Preferences_ColorDlg()
// purpose  : Destructor
//=================================================================================
SMESHGUI_Preferences_ColorDlg::~SMESHGUI_Preferences_ColorDlg()
{
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ClickOnOk()
{
  mySMESHGUI->ResetState();
  accept();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ClickOnCancel()
{
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::DeactivateActiveDialog()
{
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::closeEvent( QCloseEvent* )
{
  ClickOnCancel(); /* same than click on cancel button */
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
}

//=================================================================================
// function : SetColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetColor( int type, const QColor& color )
{
  switch ( type ) {
  case 1 : btnFillColor->setColor( color );     break; // fill
  case 2 : btnOutlineColor->setColor( color );  break; // outline
  case 3 : btnNodeColor->setColor( color );     break; // node
  case 4 : btnBackFaceColor->setColor( color ); break; // back face
  default: break;
  }
}

//=================================================================================
// function : GetColor()
// purpose  :
//=================================================================================
QColor SMESHGUI_Preferences_ColorDlg::GetColor( int type )
{
  QColor color;
  switch ( type ) {
  case 1 : color = btnFillColor->color();     break; // fill
  case 2 : color = btnOutlineColor->color();  break; // outline
  case 3 : color = btnNodeColor->color();     break; // node
  case 4 : color = btnBackFaceColor->color(); break; // back face
  default: break;
  }
  return color;
}

//=================================================================================
// function : SetIntValue()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetIntValue( int type, int value )
{
  switch ( type ) {
  case 1 : SpinBox_Width->setValue( value );      break; // width
  case 2 : SpinBox_Nodes_Size->setValue( value ); break; // nodes size = value; break;
  case 3 : SpinBox_Shrink->setValue( value );     break; // shrink coeff
  default: break;
  }
}

//=================================================================================
// function : GetIntValue()
// purpose  :
//=================================================================================
int SMESHGUI_Preferences_ColorDlg::GetIntValue( int type )
{
  int res = 0;
  switch ( type ) {
  case 1 : res = SpinBox_Width->value();      break; // width
  case 2 : res = SpinBox_Nodes_Size->value(); break; // nodes size
  case 3 : res = SpinBox_Shrink->value();     break; // shrink coeff
  default: break;
  }
  return res;
}
