// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshersGUI_QuadrangleParamWdg.cxx
//  Author : Open CASCADE S.A.S. (jfa)
//  SMESH includes

#include "StdMeshersGUI_QuadrangleParamWdg.h"

#include "SMESHGUI.h"

#include "SUIT_ResourceMgr.h"

// Qt includes
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QGridLayout>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_BasicHypothesis)

#define SPACING 6
#define MARGIN 0

//================================================================================
// function : Constructor
// purpose  :
//================================================================================
StdMeshersGUI_QuadrangleParamWdg::StdMeshersGUI_QuadrangleParamWdg (QWidget * parent)
  : QWidget(parent),
    myType(0)
{
  myType = new QButtonGroup (this);

  QGridLayout* typeLay = new QGridLayout( this );

  typeLay->setMargin(MARGIN);
  typeLay->setSpacing(SPACING);

  QString aTypeKey ("SMESH_QUAD_TYPE_%1");
  QString aPictKey ("ICON_StdMeshers_Quadrangle_Params_%1");

  int itype = 0;
  for (; itype < int(StdMeshers::QUAD_NB_TYPES); itype++) {
    QRadioButton* rbi = new QRadioButton (tr(aTypeKey.arg(itype).toLatin1()), this);
    QPixmap pmi (SMESHGUI::resourceMgr()->loadPixmap("SMESH", tr(aPictKey.arg(itype).toLatin1())));
    QLabel* pli = new QLabel (this);
    pli->setPixmap(pmi);
    typeLay->addWidget(rbi, itype, 0, 1, 1);
    typeLay->addWidget(pli, itype, 1, 1, 1);
    myType->addButton(rbi, itype);
  }
  myType->button(0)->setChecked(true);

  setLayout(typeLay);
  setMinimumWidth(300);
}

//================================================================================
// function : Destructor
// purpose  :
//================================================================================
StdMeshersGUI_QuadrangleParamWdg::~StdMeshersGUI_QuadrangleParamWdg()
{
}

//=================================================================================
// function : SetType
// purpose  :
//=================================================================================
void StdMeshersGUI_QuadrangleParamWdg::SetType (int theType)
{
  myType->button(theType)->setChecked(true);
}

//=================================================================================
// function : GetType
// purpose  :
//=================================================================================
int StdMeshersGUI_QuadrangleParamWdg::GetType()
{
  return myType->checkedId();
}
