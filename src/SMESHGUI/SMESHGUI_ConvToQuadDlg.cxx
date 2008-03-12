// SMESH SMESHGUI : GUI for SMESH component
//
// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
//
// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License. 
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
// File   : SMESHGUI_ConvToQuadDlg.cxx
// Author : Open CASCADE S.A.S.
//

// SMESH includes
#include "SMESHGUI_ConvToQuadDlg.h"

// Qt includes
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>

#define SPACING 6
#define MARGIN  11

SMESHGUI_ConvToQuadDlg::SMESHGUI_ConvToQuadDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  setWindowTitle( tr( "CAPTION" ) );

  // Create top controls  

  // mesh
  createObject( tr( "MESH" ), mainFrame(), 0 );

  //Create check box
  myMedNdsOnGeom = new QCheckBox( tr( "MEDIUMNDS" ), mainFrame() );

  //Create RadioButtons
  myBGBox = new QGroupBox( mainFrame() );
  myBG = new QButtonGroup( mainFrame() );
  QHBoxLayout* aBGLayout = new QHBoxLayout( myBGBox );
  aBGLayout->setMargin(MARGIN);
  aBGLayout->setSpacing(SPACING);
  
  myRB1 = new QRadioButton( tr( "RADIOBTN_1" ), myBGBox );
  myRB2 = new QRadioButton( tr( "RADIOBTN_2" ), myBGBox );

  aBGLayout->addWidget(myRB1);
  aBGLayout->addWidget(myRB2);
  myBG->addButton(myRB1, 0);
  myBG->addButton(myRB2, 1);
  myRB1->setChecked( true );

  // Fill layout
  QGridLayout* aLay = new QGridLayout( mainFrame() );
  aLay->setMargin( 0 );
  aLay->setSpacing( SPACING );

  aLay->addWidget( objectWg( 0,  Label ),   0, 0 );
  aLay->addWidget( objectWg( 0,  Btn ),     0, 1 );
  aLay->addWidget( objectWg( 0,  Control ), 0, 2 );
  aLay->addWidget( myMedNdsOnGeom,          1, 0, 1, 3 );
  aLay->addWidget( myBGBox,                 2, 0, 1, 3 );
  
  connect(myBG, SIGNAL( buttonClicked( int ) ), this, SIGNAL( onClicked( int ) ) );
}

SMESHGUI_ConvToQuadDlg::~SMESHGUI_ConvToQuadDlg()
{
}

bool SMESHGUI_ConvToQuadDlg::IsMediumNdsOnGeom() const
{
  return !myMedNdsOnGeom->isChecked();
}

void SMESHGUI_ConvToQuadDlg::SetMediumNdsOnGeom(const bool theCheck)
{
  myMedNdsOnGeom->setChecked(theCheck);
}

bool SMESHGUI_ConvToQuadDlg::IsEnabledCheck() const
{
  return myMedNdsOnGeom->isEnabled();
}

void SMESHGUI_ConvToQuadDlg::SetEnabledCheck( const bool theCheck )
{
  myMedNdsOnGeom->setEnabled( theCheck );
}

int SMESHGUI_ConvToQuadDlg::CurrentRB( )
{
  return myBG->checkedId();
}

void SMESHGUI_ConvToQuadDlg::SetEnabledControls( const bool theCheck )
{
  myBGBox->setEnabled( theCheck );
  myMedNdsOnGeom->setEnabled( theCheck );
  setButtonEnabled( theCheck, QtxDialog::OK | QtxDialog::Apply );
}

void SMESHGUI_ConvToQuadDlg::SetEnabledRB( const int idx, const bool theCheck ) 
{
  if(idx)
  {
    myRB2->setEnabled( theCheck );
    myRB1->setEnabled( !theCheck );
    myRB1->setChecked( true );
  }
  else
  {
    myRB1->setEnabled( theCheck );
    myRB2->setEnabled( !theCheck );
    myRB2->setChecked( true );
  }
  emit onClicked( myBG->checkedId() );
}


