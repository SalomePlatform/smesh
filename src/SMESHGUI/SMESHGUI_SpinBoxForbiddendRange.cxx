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
// File   : SMESHGUI_SpinBoxForbiddendRange.cxx
// Author : Konstantin Leontev, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SpinBoxForbiddendRange.h"

#include <QToolTip>

#include "utilities.h"


//=================================================================================
// class    : SMESHGUI_SpinBoxForbiddendRange()
// purpose  : constructor
//=================================================================================
SMESHGUI_SpinBoxForbiddendRange::SMESHGUI_SpinBoxForbiddendRange(QWidget* parent)
  : SMESHGUI_SpinBox(parent), forbiddenMin(0.0), forbiddenMax(1.0)
{
  // This property allows to return QValidator::Invalid from QtxDoubleSpinBox::validate()
  // for any values outside of the min/max range.
  // Otherwise a user could type any intermediate value.
  setProperty("strict_validity_check", true);
}

//=================================================================================
// function : ~SMESHGUI_SpinBoxForbiddendRange()
// purpose  : destructor
//=================================================================================
SMESHGUI_SpinBoxForbiddendRange::~SMESHGUI_SpinBoxForbiddendRange()
{
}

//=================================================================================
// function : stepBy()
// purpose  : Validates value in ranges on each step
//=================================================================================
void SMESHGUI_SpinBoxForbiddendRange::stepBy(const int steps)
{
  double const newValue = value() + (steps * singleStep());
  double const validatedValue = GetValueInRange(newValue);

  setValue(validatedValue);
}

//=================================================================================
// function : valueFromText()
// purpose  : Validates value entered by user 
//=================================================================================
double SMESHGUI_SpinBoxForbiddendRange::valueFromText(QString const& text) const
{
  double const newValue = SMESHGUI_SpinBox::valueFromText(text);
  return GetValueInRange(newValue);
}

QValidator::State SMESHGUI_SpinBoxForbiddendRange::validate(QString& text, int& pos) const
{
  // Do parent's min/max and all other validation first
  const QValidator::State res = SMESHGUI_SpinBox::validate(text, pos);
  if (res != QValidator::Acceptable)
    return res;

  // Now we know that we have a valid numeric value in the text
  // that we can check against forbidden range.
  bool ok = false;
  const double inputValue = text.toDouble(&ok);

  // Return intermediate to allow a user input any value.
  // If press ok button, it will be checked again with checkRange() call.
  const QValidator::State state = ok && IsValueInRange(inputValue) ?
    QValidator::Acceptable : QValidator::Intermediate;

  showValidationToolTip(state);

  return state;
}

//=================================================================================
// function : SetForbiddenRange()
// purpose  : Sets min and max values for forbidden range
//=================================================================================
void SMESHGUI_SpinBoxForbiddendRange::SetForbiddenRange(const double min, const double max)
{
  forbiddenMin = min;
  forbiddenMax = max;
}

//=================================================================================
// function : checkRange()
// purpose  : adds checking against forbidden range to the parent's implementation
//=================================================================================
bool SMESHGUI_SpinBoxForbiddendRange::checkRange(const double value) const
{
  return SalomeApp_DoubleSpinBox::checkRange(value) && IsValueInRange(value);
}

//=================================================================================
// function : IsValueInRange()
// purpose  : Validates a given value against forbidden range
//=================================================================================
bool SMESHGUI_SpinBoxForbiddendRange::IsValueInRange(const double inputValue) const
{
  // A given value must be outside of [forbiddenMin, forbiddenMax] scope
  return inputValue < forbiddenMin || inputValue > forbiddenMax;
}

//=================================================================================
// function : GetValueInRange()
// purpose  : Validates value considering forbidden range
//=================================================================================
double SMESHGUI_SpinBoxForbiddendRange::GetValueInRange(const double inputValue) const
{
  // Check if a given value is outside of [forbiddenMin, forbiddenMax] scope
  if (IsValueInRange(inputValue))
    return inputValue;

  const double curValue = value();
  const double step = SMESHGUI_SpinBox::singleStep();

  // Find the nearest value outside of forbidden range
  if (inputValue > curValue)
    return forbiddenMax + step;
  else
    return forbiddenMin - step;
}

//=================================================================================
// function : showValidationToolTip()
// purpose  : Shows tooltip with a valid range
//=================================================================================
void SMESHGUI_SpinBoxForbiddendRange::showValidationToolTip(const QValidator::State state) const
{
  if (state == QValidator::Acceptable)
  {
    QToolTip::hideText();
    return;
  }

  // All the rest is an edited copy from SalomeApp_DoubleSpinBox::validate()

  // Get position and min/max values
  const QPoint pos(size().width(), 0.);
  const QPoint globalPos = mapToGlobal(pos);
  const QString minVal = textFromValue(minimum());
  const QString maxVal = textFromValue(maximum());

  // Same stuff as in QtxDoubleSpinBox::textFromValue()
  int digits = getPrecision();

  // For 'g' format, max. number of digits after the decimal point is getPrecision() - 1
  // See also QtxDoubleSpinBox::validate()
  if (digits < 0)
    digits = qAbs(digits) - 1;

  QString msg = QString(tr("VALID_RANGE_NOVAR_MSG")).
    arg(minVal).
    arg(forbiddenMin).
    arg(forbiddenMax).
    arg(maxVal).
    arg(digits);

  // It is not possible to hide and shortly afterwards reshow a tooltip
  // with the same text because of some interference between the internal hideTimer of the tooltip and
  // the QToolTip::showText(). As a result, the tooltip does not persist - it disappears immediately.
  // If we are here, then QToolTip::hideText() was just called from the parent.
  // So, below is a workaround that changes the text and lets the tooltip stay in place.
  toolTipFlag = !toolTipFlag;
  if (toolTipFlag)
  {
    msg.append(" ");
  }

  // Show tooltip
  QToolTip::showText(globalPos, msg, const_cast<SMESHGUI_SpinBoxForbiddendRange*>(this));
}
