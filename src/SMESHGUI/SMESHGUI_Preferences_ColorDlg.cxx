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
//  File   : SMESHGUI_Preferences_ColorDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_Preferences_ColorDlg.h"
#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"

#include "utilities.h"

// QT Includes
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcolordialog.h>
#include <qspinbox.h>
#include <qcolor.h>

#include "SUIT_Desktop.h"

using namespace std;

//=================================================================================
// function : SMESHGUI_Preferences_ColorDlg()
// purpose  : Constructs a SMESHGUI_Preferences_ColorDlg which is a child
//            of 'parent', with the name 'name' and widget flags set to 'f'
//            The dialog will by default be modeless, unless you
//            set'modal' to TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_Preferences_ColorDlg::SMESHGUI_Preferences_ColorDlg( SMESHGUI* theModule, const char* name)
     : QDialog( SMESH::GetDesktop( theModule ), name, true, WStyle_Customize |
                WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
    mySMESHGUI( theModule )
{
    if (!name)
	setName("SMESHGUI_Preferences_ColorDlg");
    setCaption(tr("Preferences - Set Color"));
    setSizeGripEnabled(TRUE);
    QGridLayout* SMESHGUI_Preferences_ColorDlgLayout = new QGridLayout(this);
    SMESHGUI_Preferences_ColorDlgLayout->setSpacing(6);
    SMESHGUI_Preferences_ColorDlgLayout->setMargin(11);

    ButtonGroup1 = new QButtonGroup(tr("Elements"), this, "ButtonGroup1");
    ButtonGroup1->setColumnLayout(0, Qt::Vertical);
    ButtonGroup1->layout()->setSpacing(0);
    ButtonGroup1->layout()->setMargin(0);
    QGridLayout* ButtonGroup1Layout = new QGridLayout(ButtonGroup1->layout());
    ButtonGroup1Layout->setAlignment(Qt::AlignTop);
    ButtonGroup1Layout->setSpacing(6);
    ButtonGroup1Layout->setMargin(11);

    TextLabel_Fill = new QLabel(tr("Fill"), ButtonGroup1, "TextLabel_2D_Fill");
    ButtonGroup1Layout->addWidget(TextLabel_Fill, 0, 0);

    btnFillColor = new QPushButton(ButtonGroup1, "btnFillColor");
    btnFillColor->setFixedSize(QSize(25, 25));
    ButtonGroup1Layout->addWidget(btnFillColor, 0, 1);

    TextLabel_Outine = new QLabel(tr("Outline"), ButtonGroup1, "TextLabel_2D_Outine");
    ButtonGroup1Layout->addWidget(TextLabel_Outine, 0, 2);

    btnOutlineColor = new QPushButton(ButtonGroup1, "btnOutlineColor");
    btnOutlineColor->setFixedSize(QSize(25, 25));
    ButtonGroup1Layout->addWidget(btnOutlineColor, 0, 3);

    TextLabel_Width = new QLabel(tr("Width"), ButtonGroup1, "TextLabel_2D_Width");
    ButtonGroup1Layout->addWidget(TextLabel_Width, 0, 4);

    SpinBox_Width = new QSpinBox(0, 5, 1, ButtonGroup1, "SpinBox_Width");
    SpinBox_Width->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    SpinBox_Width->setWrapping(FALSE);
    SpinBox_Width->setButtonSymbols(QSpinBox::PlusMinus);
    ButtonGroup1Layout->addWidget(SpinBox_Width, 0, 5);

    TextLabel_BackFace = new QLabel(tr("Back Face"), ButtonGroup1, "TextLabel_BackFace");
    ButtonGroup1Layout->addWidget(TextLabel_BackFace, 1, 0);

    btnBackFaceColor = new QPushButton(ButtonGroup1, "btnBackFaceColor");
    btnBackFaceColor->setFixedSize(QSize(25, 25));
    ButtonGroup1Layout->addWidget(btnBackFaceColor, 1, 1);

    TextLabel_ShrinkCoeff = new QLabel(tr("Shrink coef."), ButtonGroup1, "TextLabel_ShrinkCoeff");
    ButtonGroup1Layout->addWidget(TextLabel_ShrinkCoeff, 2, 0);

    SpinBox_Shrink = new QSpinBox(20, 100, 1, ButtonGroup1, "SpinBox_Shrink");
    SpinBox_Shrink->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    SpinBox_Shrink->setButtonSymbols(QSpinBox::PlusMinus);
    ButtonGroup1Layout->addMultiCellWidget(SpinBox_Shrink, 2, 2, 1, 5);

    SMESHGUI_Preferences_ColorDlgLayout->addWidget(ButtonGroup1, 0, 0);

    ButtonGroup2 = new QButtonGroup(tr("Nodes"), this, "ButtonGroup2");
    ButtonGroup2->setColumnLayout(0, Qt::Vertical);
    ButtonGroup2->layout()->setSpacing(0);
    ButtonGroup2->layout()->setMargin(0);
    QGridLayout* ButtonGroup2Layout = new QGridLayout(ButtonGroup2->layout());
    ButtonGroup2Layout->setAlignment(Qt::AlignTop);
    ButtonGroup2Layout->setSpacing(6);
    ButtonGroup2Layout->setMargin(11);

    TextLabel_Nodes_Color = new QLabel(tr("Color"), ButtonGroup2, "TextLabel_Nodes_Color");
    ButtonGroup2Layout->addWidget(TextLabel_Nodes_Color, 0, 0);

    btnNodeColor = new QPushButton(ButtonGroup2, "btnNodeColor");
    btnNodeColor->setFixedSize(QSize(25, 25));
    ButtonGroup2Layout->addWidget(btnNodeColor, 0, 1);

    TextLabel_Nodes_Size = new QLabel(tr("Size"), ButtonGroup2, "TextLabel_Nodes_Size");
    ButtonGroup2Layout->addWidget(TextLabel_Nodes_Size, 0, 2);

    SpinBox_Nodes_Size = new QSpinBox(0, 5, 1, ButtonGroup2, "SpinBox_Nodes_Size");
    SpinBox_Nodes_Size->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    SpinBox_Nodes_Size->setWrapping(FALSE);
    SpinBox_Nodes_Size->setButtonSymbols(QSpinBox::PlusMinus);
    ButtonGroup2Layout->addWidget(SpinBox_Nodes_Size, 0, 3);

    SMESHGUI_Preferences_ColorDlgLayout->addWidget(ButtonGroup2, 1, 0);

    GroupButtons = new QButtonGroup(this, "GroupButtons");
    GroupButtons->setColumnLayout(0, Qt::Vertical);
    GroupButtons->layout()->setSpacing(0);
    GroupButtons->layout()->setMargin(0);
    QGridLayout* GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
    GroupButtonsLayout->setAlignment(Qt::AlignTop);
    GroupButtonsLayout->setSpacing(6);
    GroupButtonsLayout->setMargin(11);

    buttonOk = new QPushButton(tr("&OK"), GroupButtons, "buttonOk");
    buttonOk->setAutoDefault(TRUE);
    buttonOk->setDefault(TRUE);
    GroupButtonsLayout->addWidget(buttonOk, 0, 0);

    GroupButtonsLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1);

    buttonCancel = new QPushButton(tr("&Cancel"), GroupButtons, "buttonCancel");
    buttonCancel->setAutoDefault(TRUE);
    GroupButtonsLayout->addWidget(buttonCancel, 0, 2);

    SMESHGUI_Preferences_ColorDlgLayout->addWidget(GroupButtons, 2, 0);

    Init();
}

//=================================================================================
// function : ~SMESHGUI_Preferences_ColorDlg()
// purpose  : Destructor
//=================================================================================
SMESHGUI_Preferences_ColorDlg::~SMESHGUI_Preferences_ColorDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));

  connect(btnFillColor, SIGNAL(clicked()), this, SLOT(SelectFillColor()));
  connect(btnOutlineColor, SIGNAL(clicked()), this, SLOT(SelectOutlineColor()));
  connect(btnNodeColor, SIGNAL(clicked()), this, SLOT(SelectNodeColor()));
  connect(btnBackFaceColor, SIGNAL(clicked()), this, SLOT(SelectBackFaceColor()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
}

//=================================================================================
// function : SelectFillColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SelectFillColor()
{
  QColor color = btnFillColor->palette().active().button();
  color = QColorDialog::getColor(color);
  if (color.isValid()) {
    QPalette pal = btnFillColor->palette();
    pal.setColor(QColorGroup::Button, color);
    btnFillColor->setPalette(pal);
  }
}

//=================================================================================
// function : SelectBackFaceColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SelectBackFaceColor()
{
  QColor color = btnBackFaceColor->palette().active().button();
  color = QColorDialog::getColor(color);
  if (color.isValid()) {
    QPalette pal = btnBackFaceColor->palette();
    pal.setColor(QColorGroup::Button, color);
    btnBackFaceColor->setPalette(pal);
  }
}

//=================================================================================
// function : SelectOutlineColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SelectOutlineColor()
{
  QColor color = btnOutlineColor->palette().active().button();
  color = QColorDialog::getColor(color);
  if (color.isValid()) {
    QPalette pal = btnOutlineColor->palette();
    pal.setColor(QColorGroup::Button, color);
    btnOutlineColor->setPalette(pal);
  }
}

//=================================================================================
// function : SelectNodeColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SelectNodeColor()
{
  QColor color = btnNodeColor->palette().active().button();
  color = QColorDialog::getColor(color);
  if (color.isValid()) {
    QPalette pal = btnNodeColor->palette();
    pal.setColor(QColorGroup::Button, color);
    btnNodeColor->setPalette(pal);
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ClickOnOk()
{
  mySMESHGUI->ResetState();
  accept();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ClickOnCancel()
{
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::DeactivateActiveDialog()
{
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::closeEvent (QCloseEvent*)
{
  this->ClickOnCancel(); /* same than click on cancel button */
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
}

//=================================================================================
// function : SetColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetColor (int type, QColor color)
{
  switch (type) {
  case 1 : // fill
    {
      QPalette pal = btnFillColor->palette();
      pal.setColor(QColorGroup::Button, color);
      btnFillColor->setPalette(pal);
      break;
    }
  case 2 : // outline
    {
      QPalette pal = btnOutlineColor->palette();
      pal.setColor(QColorGroup::Button, color);
      btnOutlineColor->setPalette(pal);
      break;
    }
  case 3 :  // node
    {
      QPalette pal = btnNodeColor->palette();
      pal.setColor(QColorGroup::Button, color);
      btnNodeColor->setPalette(pal);
      break;
    }
  case 4 : // back face
    {
      QPalette pal = btnBackFaceColor->palette();
      pal.setColor(QColorGroup::Button, color);
      btnBackFaceColor->setPalette(pal);
      break;
    }
  }
}

//=================================================================================
// function : GetColor()
// purpose  :
//=================================================================================
QColor SMESHGUI_Preferences_ColorDlg::GetColor (int type)
{
  QColor color;
  switch (type) {
  case 1 : color = btnFillColor->palette().active().button();     break; // fill
  case 2 : color = btnOutlineColor->palette().active().button();  break; // outline
  case 3 : color = btnNodeColor->palette().active().button();     break; // node
  case 4 : color = btnBackFaceColor->palette().active().button(); break; // back face
  }
  return color;
}

//=================================================================================
// function : SetIntValue()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetIntValue (int type, int value)
{
  switch (type) {
  case 1 : SpinBox_Width->setValue(value);      break; // width
  case 2 : SpinBox_Nodes_Size->setValue(value); break; // nodes size = value; break;
  case 3 : SpinBox_Shrink->setValue(value);     break; // shrink coeff
  }
}

//=================================================================================
// function : GetIntValue()
// purpose  :
//=================================================================================
int SMESHGUI_Preferences_ColorDlg::GetIntValue (int type)
{
  int res = 0;
  switch (type) {
  case 1 : res = SpinBox_Width->value();      break; // width
  case 2 : res = SpinBox_Nodes_Size->value(); break; // nodes size
  case 3 : res = SpinBox_Shrink->value();     break; // shrink coeff
  }
  return res;
}
