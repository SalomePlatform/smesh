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
//  File   : SMESHGUI_SpinBox.h
//  Author : Lucien PIGNOLONI
//  Module : SMESH
//  $Header$

#ifndef  SMESHSPINBOX_H
#define  SMESHSPINBOX_H

#include "QAD_SpinBoxDbl.h" 

//=================================================================================
// class    : SMESHGUI_SpinBox
// purpose  : Derivated from QAD_SpinBoxDbl class 
//=================================================================================
class SMESHGUI_SpinBox : public QAD_SpinBoxDbl
{
  Q_OBJECT

public :

  SMESHGUI_SpinBox( QWidget* parent, const char* name = 0 ) ;
  ~SMESHGUI_SpinBox() ;
  
  void       RangeStepAndValidator( double         min = -1000000.0, 
				    double         max = +1000000.0, 
				    double         step = 100.0, 
				    unsigned short decimals = 3 ) ;
  void       SetValue( double v ) ;
  double     GetValue() ;
  QString    GetString() ;
  QLineEdit* editor() { return QAD_SpinBoxDbl::editor(); } 

public slots:
  void       SetStep( double newStep );

};
#endif //  SMESHSPINBOX_H
