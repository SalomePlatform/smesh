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
// File   : SMESHGUI_SpinBox.cxx
// Author : Lucien PIGNOLONI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SpinBox.h"

// Qt includes
#include <QDoubleValidator>
#include <QLineEdit>

//=================================================================================
// class    : SMESHGUI_SpinBox()
// purpose  : constructor of specific widget accepting floats in double precision.
//=================================================================================
SMESHGUI_SpinBox::SMESHGUI_SpinBox( QWidget* parent )
  : SalomeApp_DoubleSpinBox( parent )
{
}

//=================================================================================
// function : ~SMESHGUI_SpinBox()
// purpose  : destructor
//=================================================================================
SMESHGUI_SpinBox::~SMESHGUI_SpinBox()
{
}

//=================================================================================
// function : SetStep()  [SLOT]
// purpose  :
//=================================================================================
void SMESHGUI_SpinBox::SetStep( double newStep )
{
  setSingleStep( newStep );
}

//=================================================================================
// function : SetValue()
// purpose  :
//=================================================================================
void SMESHGUI_SpinBox::SetValue( double v )
{
  setValue(v);
  editor()->setCursorPosition( 0 );
}

//=================================================================================
// function : GetValue()
// purpose  : returns a double
//=================================================================================
double SMESHGUI_SpinBox::GetValue() const
{
  return value();
}

//=================================================================================
// function : GetString()
// purpose  : returns a QString
//=================================================================================
QString SMESHGUI_SpinBox::GetString() const
{
  return cleanText();
}

//=================================================================================
// function : editor()
// purpose  : returns editor
//=================================================================================
QLineEdit* SMESHGUI_SpinBox::editor() const
{
  return SalomeApp_DoubleSpinBox::lineEdit();
} 

//=================================================================================
// function : validator()
// purpose  : returns validator
//=================================================================================
QDoubleValidator* SMESHGUI_SpinBox::validator() const
{
  return (QDoubleValidator*)editor()->validator();
}

//=================================================================================
// function : RangeStepAndValidator()
// purpose  :
//=================================================================================
void SMESHGUI_SpinBox::RangeStepAndValidator( double min,
					      double max,
					      double step,
					      unsigned short precision )
{
  setPrecision(precision*(-1)); // PAL8769. Minus is for using 'g' double->string conversion specifier,
  //                               see QtxDoubleSpinBox::mapValueToText( double v )
  setDecimals(32);
  setRange(min, max);
  setSingleStep( step );
  setDefaultValue( min );
}
