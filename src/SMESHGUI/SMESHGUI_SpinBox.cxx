using namespace std;
//  File      : SMESHGUI_SpinBox.cxx
//  Created   : 14 august 2002
//  Author    : Lucien PIGNOLONI
//  Project   : SALOME
//  Module    : SMESHGUI
//  Copyright : OPEN CASCADE
//  $Header$

#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI.h"
#include <qvalidator.h>

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
