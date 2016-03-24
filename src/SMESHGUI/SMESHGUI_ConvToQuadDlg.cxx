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
// File   : SMESHGUI_ConvToQuadDlg.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ConvToQuadDlg.h"

#include "SMESHGUI_ConvToQuadOp.h"

// Qt includes
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

#define SPACING 6
#define MARGIN  11

SMESHGUI_ConvToQuadDlg::SMESHGUI_ConvToQuadDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  setWindowTitle( tr( "CAPTION" ) );

  // Create top controls  

  // mesh
  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "MESH" ), mainFrame(), 0 );

  //Create check box
  myMedNdsOnGeom = new QCheckBox( tr( "MEDIUMNDS" ), mainFrame() );

  //Create RadioButtons
  myBGBox = new QGroupBox( mainFrame() );
  myBG = new QButtonGroup( mainFrame() );
  QVBoxLayout* aBGLayout = new QVBoxLayout( myBGBox );
  aBGLayout->setMargin(MARGIN);
  aBGLayout->setSpacing(SPACING);
  
  myRB2Lin = new QRadioButton( tr( "RADIOBTN_1" ), myBGBox );
  myRB2Quad = new QRadioButton( tr( "RADIOBTN_2" ), myBGBox );
  myRB2BiQua = new QRadioButton( tr( "RADIOBTN_3" ), myBGBox );

  aBGLayout->addWidget(myRB2Lin);
  aBGLayout->addWidget(myRB2Quad);
  aBGLayout->addWidget(myRB2BiQua);
  myBG->addButton(myRB2Lin, 0);
  myBG->addButton(myRB2Quad, 1);
  myBG->addButton(myRB2BiQua, 2);
  myRB2Lin->setChecked( true );

  myWarning = new QLabel(QString("<b>%1</b>").arg(tr("NON_CONFORM_WARNING")), mainFrame());

  // Fill layout
  QGridLayout* aLay = new QGridLayout( mainFrame() );
  aLay->setMargin( 5 );
  aLay->setSpacing( 5 );

  aLay->addWidget( objectWg( 0,  Label ),   0, 0 );
  aLay->addWidget( objectWg( 0,  Btn ),     0, 1 );
  aLay->addWidget( objectWg( 0,  Control ), 0, 2 );
  aLay->addWidget( myMedNdsOnGeom,          1, 0, 1, 3 );
  aLay->addWidget( myBGBox,                 2, 0, 1, 3 );
  aLay->addWidget( myWarning,               3, 0, 1, 3 );
  
  connect(myBG, SIGNAL( buttonClicked( int ) ), this, SIGNAL( onClicked( int ) ) );
}

SMESHGUI_ConvToQuadDlg::~SMESHGUI_ConvToQuadDlg()
{
}

bool SMESHGUI_ConvToQuadDlg::IsBiQuadratic() const
{
  return myRB2BiQua->isChecked();
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

void SMESHGUI_ConvToQuadDlg::ShowWarning(bool toShow)
{
  if ( toShow )
    myWarning->show();
  else
    myWarning->hide();
}

bool SMESHGUI_ConvToQuadDlg::isWarningShown()
{
  return myWarning->isVisible();
}

void SMESHGUI_ConvToQuadDlg::SetEnabledControls( const bool theCheck )
{
  //myBGBox->setEnabled( theCheck );
  myRB2Lin->setEnabled( theCheck );
  myRB2Quad->setEnabled( theCheck );
  myRB2BiQua->setEnabled( theCheck );
  myMedNdsOnGeom->setEnabled( theCheck );
  //setButtonEnabled( theCheck, QtxDialog::OK | QtxDialog::Apply );
}

void SMESHGUI_ConvToQuadDlg::SetEnabledRB( const int idx, const bool theCheck ) 
{
  myRB2Lin  ->setEnabled( idx & SMESHGUI_ConvToQuadOp::Linear );
  myRB2Quad ->setEnabled( idx & SMESHGUI_ConvToQuadOp::Quadratic );
  myRB2BiQua->setEnabled( idx & SMESHGUI_ConvToQuadOp::BiQuadratic );

  if ( idx & SMESHGUI_ConvToQuadOp::Linear )
  {
    myRB2Lin->setChecked( true );
    myRB2Quad->setChecked( false );
  }
  else
  {
    myRB2Lin->setChecked( false );
    myRB2Quad->setChecked( true );
  }
  myRB2BiQua->setChecked( false );

  myMedNdsOnGeom->setEnabled( theCheck );

  emit onClicked( myBG->checkedId() );
}
