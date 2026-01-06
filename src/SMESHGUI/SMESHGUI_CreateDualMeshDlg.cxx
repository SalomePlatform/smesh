// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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
// File   : SMESHGUI_CreateDualMeshDlg.cxx
// Author : Yoann AUDOUIN (EDF)
// SMESH includes
//
#include "SMESHGUI_CreateDualMeshDlg.h"

#include "SMESHGUI_CreateDualMeshOp.h"

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
#include <QLineEdit>

#define SPACING 6
#define MARGIN  11

SMESHGUI_CreateDualMeshDlg::SMESHGUI_CreateDualMeshDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  setWindowTitle( tr( "CAPTION" ) );

  // Create top controls

  // mesh
  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "MESH" ), mainFrame(), 0 );

  myMeshNameLabel = new QLabel(QString(tr("DUAL_MESH_NAME")), mainFrame());
  myMeshName = new QLineEdit(mainFrame());

  myProjShape = new QCheckBox(QString(tr("PROJ_SHAPE")), mainFrame());
  myProjShape->toggle();

  myWarning = new QLabel(QString("<b>%1</b>").arg(tr("NON_TETRA_MESH_WARNING")), mainFrame());

  // Fill layout
  QGridLayout* aLay = new QGridLayout( mainFrame() );
  aLay->setMargin( 5 );
  aLay->setSpacing( 5 );

  aLay->addWidget( objectWg( 0,  Label ),   0, 0 );
  aLay->addWidget( objectWg( 0,  Btn ),     0, 1 );
  aLay->addWidget( objectWg( 0,  Control ), 0, 2 );
  aLay->addWidget( myWarning,               3, 0, 1, 3 );
  aLay->addWidget( myMeshNameLabel,         1, 0 );
  aLay->addWidget( myMeshName,              1, 2 );
  aLay->addWidget( myProjShape,              2, 0 );

}

SMESHGUI_CreateDualMeshDlg::~SMESHGUI_CreateDualMeshDlg()
{
}

void SMESHGUI_CreateDualMeshDlg::ShowWarning(bool toShow)
{
  if ( toShow )
    myWarning->show();
  else
    myWarning->hide();
}

bool SMESHGUI_CreateDualMeshDlg::isWarningShown()
{
  return myWarning->isVisible();
}