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
//  File   : SMESHGUI_aParameter.cxx
//  Module : SMESH
//  $Header$

#include "SMESHGUI_aParameter.h"
  
#include <qspinbox.h>
#include <qvalidator.h>
#include <qtextedit.h>

#include "QAD_SpinBoxDbl.h"

SMESHGUI_aParameter::~SMESHGUI_aParameter() {}

//=================================================================================
// class    : SMESHGUI_intParameter
// purpose  :
//=================================================================================
SMESHGUI_intParameter::SMESHGUI_intParameter(const int      theInitValue,
                                             const QString& theLabel,
                                             const int      theBottom,   
                                             const int      theTop)
     :SMESHGUI_aParameter(theLabel),
       _top(theTop), _bottom(theBottom), _initValue( theInitValue )
{
}
SMESHGUI_aParameter::Type SMESHGUI_intParameter::GetType() const
{
  return SMESHGUI_aParameter::INT;
}
bool SMESHGUI_intParameter::GetNewInt( int & theValue ) const
{
  theValue = _newValue;
  return _newValue != _initValue;
}
bool SMESHGUI_intParameter::GetNewDouble( double & Value ) const
{
  return false;
}
bool SMESHGUI_intParameter::GetNewText( QString & Value ) const
{
  return false;
}
void SMESHGUI_intParameter::InitializeWidget( QWidget* theQWidget) const
{
  QSpinBox * aSpin = dynamic_cast< QSpinBox *>( theQWidget );
  if ( aSpin ) {
    aSpin->setMinValue( _bottom );
    aSpin->setMaxValue( _top );
    aSpin->setValue( _initValue );
  }
}
void SMESHGUI_intParameter::TakeValue( QWidget* theQWidget)
{
  QSpinBox * aSpin = dynamic_cast< QSpinBox *>( theQWidget );
  if ( aSpin )
    _newValue = aSpin->value();
}
//=================================================================================
// class    : SMESHGUI_doubleParameter
// purpose  :
//=================================================================================
SMESHGUI_doubleParameter::SMESHGUI_doubleParameter(const double   theInitValue,
                                                   const QString& theLabel,
                                                   const double   theBottom,   
                                                   const double   theTop,
                                                   const double   theStep,
                                                   const int      theDecimals)
     :SMESHGUI_aParameter( theLabel ),
       _top( theTop ), _bottom( theBottom ), _step( theStep ),
       _initValue( theInitValue ), _decimals( theDecimals )
{
}
SMESHGUI_aParameter::Type SMESHGUI_doubleParameter::GetType() const
{
  return SMESHGUI_aParameter::DOUBLE;
}
bool SMESHGUI_doubleParameter::GetNewInt( int & theValue ) const
{
  return false;
}
bool SMESHGUI_doubleParameter::GetNewDouble( double & Value ) const
{
  Value = _newValue;
  return _newValue != _initValue;
}
bool SMESHGUI_doubleParameter::GetNewText( QString & Value ) const
{
  return false;
}
void SMESHGUI_doubleParameter::InitializeWidget( QWidget* theQWidget) const
{
  QAD_SpinBoxDbl * aSpin = dynamic_cast< QAD_SpinBoxDbl *>( theQWidget );
  if ( aSpin ) {
    aSpin->setRange( _bottom, _top );
    ((QDoubleValidator*)(aSpin->validator()))->setRange( _bottom, _top, _decimals );
    aSpin->setValue( _initValue );
    aSpin->setLineStep( _step );
  }
}
void SMESHGUI_doubleParameter::TakeValue( QWidget* theQWidget)
{
  QAD_SpinBoxDbl * aSpin = dynamic_cast< QAD_SpinBoxDbl *>( theQWidget );
  if ( aSpin )
    _newValue = aSpin->value();
}

//=================================================================================
// class    : SMESHGUI_strParameter
// purpose  :
//=================================================================================
SMESHGUI_strParameter::SMESHGUI_strParameter(const QString& theInitValue,
                                             const QString& theLabel)
     :SMESHGUI_aParameter(theLabel),
      _initValue( theInitValue )
{
}
SMESHGUI_aParameter::Type SMESHGUI_strParameter::GetType() const
{
  return SMESHGUI_aParameter::TEXT;
}
bool SMESHGUI_strParameter::GetNewInt( int & theValue ) const
{
  return false;
}
bool SMESHGUI_strParameter::GetNewDouble( double & Value ) const
{
  return false;
}
bool SMESHGUI_strParameter::GetNewText( QString & theValue ) const
{
  theValue = _newValue;
  return _newValue != _initValue;
}
void SMESHGUI_strParameter::InitializeWidget( QWidget* theQWidget) const
{
  QTextEdit * anEdit = dynamic_cast< QTextEdit *>( theQWidget );
  if ( anEdit ) {
    anEdit->setText( _initValue );
  }
}
void SMESHGUI_strParameter::TakeValue( QWidget* theQWidget)
{
  QTextEdit * anEdit = dynamic_cast< QTextEdit *>( theQWidget );
  if ( anEdit )
    _newValue = anEdit->text();
}
