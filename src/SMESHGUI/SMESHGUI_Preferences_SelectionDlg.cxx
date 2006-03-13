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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_Preferences_SelectionDlg.cxx
//  Author : Natalia KOPNOVA
//  Module : SMESH
//  $Header$

#include "SMESHGUI_Preferences_SelectionDlg.h"
#include "SMESHGUI.h"

#include "SMESHGUI_Utils.h"

#include "SUIT_Desktop.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qpalette.h>
#include <qcolordialog.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_LineEdit
// purpose  : 
//=================================================================================
SMESHGUI_LineEdit::SMESHGUI_LineEdit(QWidget* parent, const char *name)
  : QLineEdit(parent, name)
{
}

SMESHGUI_LineEdit::SMESHGUI_LineEdit(const QString& text, QWidget* parent, const char *name)
  : QLineEdit(text, parent, name)
{
}

void SMESHGUI_LineEdit::focusOutEvent(QFocusEvent* e)
{
  const QValidator* aVal = validator();
  QString aText = text();
  int aCurPos = cursorPosition();
  if (aVal && aVal->validate(aText, aCurPos) != QValidator::Acceptable) {
    QString aValid = aText;
    aVal->fixup(aValid);
    if (aText != aValid) {
      setText(aValid);
      update();
      return;
    }
  }
  QLineEdit::focusOutEvent(e);
}


//=================================================================================
// class    : SMESHGUI_DoubleValidator
// purpose  : 
//=================================================================================
SMESHGUI_DoubleValidator::SMESHGUI_DoubleValidator(QObject * parent, const char *name)
  : QDoubleValidator(parent, name)
{
}

SMESHGUI_DoubleValidator::SMESHGUI_DoubleValidator(double bottom, double top, int decimals,
						 QObject * parent, const char *name)
  : QDoubleValidator(bottom, top, decimals, parent, name)
{
}

void SMESHGUI_DoubleValidator::fixup(QString& theText) const
{
  bool ok;
  double aValue = theText.toDouble(&ok);
  if (ok) {
    if (aValue < bottom())
      theText = QString::number(bottom(), 'g', decimals());
    if (aValue > top())
      theText = QString::number(top(), 'g', decimals());
  }
}


//=================================================================================
// class    : SMESHGUI_Preferences_SelectionDlg()
// purpose  : 
//=================================================================================
SMESHGUI_Preferences_SelectionDlg::SMESHGUI_Preferences_SelectionDlg( SMESHGUI* theModule, const char* name )
  : QDialog( SMESH::GetDesktop( theModule ), name, true, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
    mySMESHGUI( theModule )
{
  if ( !name ) setName( "SMESHGUI_Preferences_SelectionDlg" );
  setCaption( tr( "SMESH_PREF_SELECTION"  ) );

  QVBoxLayout* aMainLayout = new QVBoxLayout(this, 11, 6);
  QLabel* aLabel;
  
  /***************************************************************/
  QGroupBox* aSelectBox = new QGroupBox(4, Qt::Horizontal, this, "selection");
  aSelectBox->setTitle(tr("SMESH_SELECTION"));

  aLabel = new QLabel(aSelectBox, "selection color label");
  aLabel->setText(tr("SMESH_OUTLINE_COLOR"));
  myColor[2] = new QPushButton(aSelectBox, "outline color");
  myColor[2]->setFixedSize(QSize(25, 25));

  aSelectBox->addSpace(0);
  aSelectBox->addSpace(0);

  aLabel = new QLabel(aSelectBox, "selection color label");
  aLabel->setText(tr("SMESH_ELEMENTS_COLOR"));
  myColor[1] = new QPushButton(aSelectBox, "elements color");
  myColor[1]->setFixedSize(QSize(25, 25));

  aLabel = new QLabel(aSelectBox, "selection width label");
  aLabel->setText(tr("SMESH_WIDTH"));
  myWidth[1] = new QSpinBox(0, 5, 1, aSelectBox, "selection width");
  myWidth[1]->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  myWidth[1]->setButtonSymbols(QSpinBox::PlusMinus);
  myWidth[1]->setMinimumWidth(50);
  
  /***************************************************************/
  QGroupBox* aPreSelectBox = new QGroupBox(1, Qt::Vertical, this, "preselection");
  aPreSelectBox->setTitle(tr("SMESH_PRESELECTION"));

  aLabel = new QLabel(aPreSelectBox, "preselection color label");
  aLabel->setText(tr("SMESH_HILIGHT_COLOR"));
  myColor[0] = new QPushButton(aPreSelectBox, "preselection color");
  myColor[0]->setFixedSize(QSize(25, 25));

  aLabel = new QLabel(aPreSelectBox, "preselection width label");
  aLabel->setText(tr("SMESH_WIDTH"));
  myWidth[0] = new QSpinBox(0, 5, 1, aPreSelectBox, "preselection width");
  myWidth[0]->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  myWidth[0]->setButtonSymbols(QSpinBox::PlusMinus);
  myWidth[0]->setMinimumWidth(50);
  
  /***************************************************************/
  QGroupBox* aPrecisionBox = new QGroupBox(1, Qt::Vertical, this, "preselection");
  aPrecisionBox->setTitle(tr("SMESH_PRECISION"));
  QDoubleValidator* aValidator = new SMESHGUI_DoubleValidator(aPrecisionBox);
  aValidator->setBottom(0.001);
  aValidator->setDecimals(6);

  aLabel = new QLabel(aPrecisionBox, "node tol label");
  aLabel->setText(tr("SMESH_NODES"));
  myPrecision[0] = new SMESHGUI_LineEdit(aPrecisionBox, "node precision");
  myPrecision[0]->setValidator(aValidator);

  aLabel = new QLabel(aPrecisionBox, "item tol label");
  aLabel->setText(tr("SMESH_ELEMENTS"));
  myPrecision[1] = new SMESHGUI_LineEdit(aPrecisionBox, "item precision");
  myPrecision[1]->setValidator(aValidator);

  /***************************************************************/
  QFrame* aButtons = new QFrame(this, "button box");
  aButtons->setFrameStyle(QFrame::Box | QFrame::Sunken);
  QHBoxLayout* aBtnLayout = new QHBoxLayout(aButtons, 11, 6);
  aBtnLayout->setAutoAdd(false);

  QPushButton* aOKBtn = new QPushButton(aButtons, "ok");
  aOKBtn->setText(tr("SMESH_BUT_OK"));
  aOKBtn->setAutoDefault(true);
  aOKBtn->setDefault(true);
  QPushButton* aCloseBtn = new QPushButton(aButtons, "close");
  aCloseBtn->setText(tr("SMESH_BUT_CLOSE"));
  aCloseBtn->setAutoDefault(true);

  aBtnLayout->addWidget(aOKBtn);
  aBtnLayout->addStretch();
  aBtnLayout->addWidget(aCloseBtn);

  /***************************************************************/
  aMainLayout->addWidget(aSelectBox);
  aMainLayout->addWidget(aPreSelectBox);
  aMainLayout->addWidget(aPrecisionBox);
  aMainLayout->addWidget(aButtons);

  for (int i = 0; i < 3; i++)
    connect(myColor[i], SIGNAL(clicked()), this, SLOT(onSelectColor()));

  connect(aOKBtn, SIGNAL(clicked()), this, SLOT(accept()));
  connect(aCloseBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

//=================================================================================
// function : ~SMESHGUI_Preferences_SelectionDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_Preferences_SelectionDlg::~SMESHGUI_Preferences_SelectionDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_SelectionDlg::closeEvent( QCloseEvent* e )
{
  reject();
}

//=================================================================================
// function : onSelectColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_SelectionDlg::onSelectColor()
{
  QPushButton* aSender = (QPushButton*)sender();
  QColor aColor = aSender->palette().active().button();
  aColor = QColorDialog::getColor(aColor, this);
  if (aColor.isValid()) {
    QPalette aPal = aSender->palette();
    aPal.setColor(QColorGroup::Button, aColor);
    aSender->setPalette(aPal);
  }
}

//=================================================================================
// function : SetColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_SelectionDlg::SetColor(int type, QColor color)
{
  if (type > 0 && type <= 3) {
    QPalette aPal = myColor[type-1]->palette();
    aPal.setColor(QColorGroup::Button, color);
    myColor[type-1]->setPalette(aPal);
 }
}

//=================================================================================
// function : GetColor()
// purpose  :
//=================================================================================
QColor SMESHGUI_Preferences_SelectionDlg::GetColor(int type)
{
  QColor aColor;
  if (type > 0 && type <= 3)
    aColor = myColor[type-1]->palette().active().button();
  return aColor;
}

//=================================================================================
// function : SetWidth()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_SelectionDlg::SetWidth(int type, int value)
{
  if (type > 0 && type <= 2)
    myWidth[type-1]->setValue(value);
}

//=================================================================================
// function : GetWidth()
// purpose  :
//=================================================================================
int SMESHGUI_Preferences_SelectionDlg::GetWidth(int type)
{
  if (type > 0 && type <= 2)
    return myWidth[type-1]->value();
  return 0;
}

//=================================================================================
// function : SetPrecision()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_SelectionDlg::SetPrecision(int type, double value)
{
  if (type > 0 && type <= 2)
    myPrecision[type-1]->setText(QString::number(value));
}

//=================================================================================
// function : GetPrecision()
// purpose  :
//=================================================================================
double SMESHGUI_Preferences_SelectionDlg::GetPrecision(int type)
{
  if (type > 0 && type <= 2)
    return myPrecision[type-1]->text().toDouble();
  return 0;
}
