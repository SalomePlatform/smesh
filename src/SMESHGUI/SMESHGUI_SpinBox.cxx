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
//  File   : SMESHGUI_SpinBox.cxx
//  Author : Lucien PIGNOLONI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI.h"

#include <qvalidator.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_SpinBox()
// purpose  : constructor of specific widget accepting floats in double precision.
//=================================================================================
SMESHGUI_SpinBox::SMESHGUI_SpinBox( QWidget* parent, const char* name )
: QAD_SpinBoxDbl( parent, name)
{
}


//=================================================================================
// function : SetStep()  [SLOT]
// purpose  :
//=================================================================================
void SMESHGUI_SpinBox::SetStep( double newStep )
{
  setLineStep( newStep );
}

//=================================================================================
// function : ~SMESHGUI_SpinBox()
// purpose  : destructor
//=================================================================================
SMESHGUI_SpinBox::~SMESHGUI_SpinBox()
{
}

//=================================================================================
// function : SetValue()
// purpose  :
//=================================================================================
void SMESHGUI_SpinBox::SetValue( double v )
{
  setValue( v );
}

//=================================================================================
// function : GetValue()
// purpose  : returns a double
//=================================================================================
double SMESHGUI_SpinBox::GetValue( )
{
  return value();
}

//=================================================================================
// function : GetString()
// purpose  : returns a QString
//=================================================================================
QString SMESHGUI_SpinBox::GetString( )
{
  return cleanText();
}

//=================================================================================
// function : RangeStepAndValidator()
// purpose  :
//=================================================================================
void SMESHGUI_SpinBox::RangeStepAndValidator( double min, double max, double step, unsigned short decimals )
{
  setRange( min, max );
  setLineStep( step );
  ( ( QDoubleValidator* )validator() )->setRange( min, max, decimals ) ;
}
