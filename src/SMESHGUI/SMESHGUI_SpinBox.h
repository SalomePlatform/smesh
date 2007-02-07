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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_SpinBox.h
//  Author : Lucien PIGNOLONI
//  Module : SMESH
//  $Header$

#ifndef  SMESH_SPINBOX_H
#define  SMESH_SPINBOX_H

#include "SMESH_SMESHGUI.hxx"

#include <QtxDblSpinBox.h>

// like in GEOM_SRC/src/DlgRef/DlgRef_SpinBox.h
#define COORD_MIN -1e+15
#define COORD_MAX +1e+15

//=================================================================================
// class    : SMESHGUI_SpinBox
// purpose  : Derivated from QtxDblSpinBox class 
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_SpinBox : public QtxDblSpinBox
{
  Q_OBJECT

public:
  SMESHGUI_SpinBox (QWidget* parent, const char* name = 0);
  ~SMESHGUI_SpinBox();

  void       RangeStepAndValidator (double         min = -1000000.0,
				    double         max = +1000000.0,
				    double         step = 100.0,
				    unsigned short decimals = 3);
  void       SetValue (double v);
  double     GetValue();
  QString    GetString();
  QLineEdit* editor() { return QtxDblSpinBox::editor(); } 

public slots:
  void       SetStep (double newStep);
};
#endif //  SMESH_SPINBOX_H
