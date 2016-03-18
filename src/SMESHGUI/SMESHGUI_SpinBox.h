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
// File   : SMESHGUI_SpinBox.h
// Author : Lucien PIGNOLONI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SPINBOX_H
#define SMESHGUI_SPINBOX_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME GUI includes
#include <SalomeApp_DoubleSpinBox.h>

#define COORD_MIN -1e+15
#define COORD_MAX +1e+15
#define DBL_DIGITS_DISPLAY 14

class QLineEdit;
class QDoubleValidator;

//=================================================================================
// class    : SMESHGUI_SpinBox
// purpose  : Derivated from QtxDoubleSpinBox class 
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_SpinBox : public SalomeApp_DoubleSpinBox
{
  Q_OBJECT

public:
  SMESHGUI_SpinBox( QWidget* );
  ~SMESHGUI_SpinBox();

  void       RangeStepAndValidator( double         = -1000000.0,
                                    double         = +1000000.0,
                                    double         = 100.0,
                                    const char*    = "length_precision" );
  void              SetValue( double );
  double            GetValue() const;
  QString           GetString() const;
  QLineEdit*        editor() const;

public slots:
  void              SetStep( double );
};

#endif // SMESHGUI_SPINBOX_H
