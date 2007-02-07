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
//  File   : SMESHGUI_TransparencyDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_TransparencyDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESH_Actor.h"

#include "SUIT_Desktop.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "SALOME_ListIO.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOME_InteractiveObject.hxx"

#include "SalomeApp_Study.h"
#include "LightApp_Application.h"
#include "LightApp_SelectionMgr.h"

#include "SVTK_ViewWindow.h"

// QT Includes
#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qgroupbox.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_TransparencyDlg()
// purpose  :
//
//=================================================================================
SMESHGUI_TransparencyDlg::SMESHGUI_TransparencyDlg( SMESHGUI* theModule,
						    const char* name,
						    bool modal,
						    WFlags fl)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
                WStyle_Title | WStyle_SysMenu | WDestructiveClose ),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
     myViewWindow( SMESH::GetViewWindow( theModule ) )
{
  if (!name)
    setName("SMESHGUI_TransparencyDlg");
  setCaption(tr("SMESH_TRANSPARENCY_TITLE" ));
  setSizeGripEnabled(TRUE);
  QGridLayout* SMESHGUI_TransparencyDlgLayout = new QGridLayout(this);
  SMESHGUI_TransparencyDlgLayout->setSpacing(6);
  SMESHGUI_TransparencyDlgLayout->setMargin(11);

  /*************************************************************************/
  QGroupBox* GroupC1 = new QGroupBox(this, "GroupC1");
  GroupC1->setColumnLayout(0, Qt::Vertical);
  GroupC1->layout()->setSpacing(0);
  GroupC1->layout()->setMargin(0);
  QGridLayout* GroupC1Layout = new QGridLayout(GroupC1->layout());
  GroupC1Layout->setAlignment(Qt::AlignTop);
  GroupC1Layout->setSpacing(6);
  GroupC1Layout->setMargin(11);

  TextLabelTransparent = new QLabel(GroupC1, "TextLabelTransparent");
  TextLabelTransparent->setText(tr("SMESH_TRANSPARENCY_TRANSPARENT" ));
  TextLabelTransparent->setAlignment(AlignLeft);
  GroupC1Layout->addWidget(TextLabelTransparent, 0, 0);

  ValueLab = new QLabel(GroupC1, "ValueLab");
  ValueLab->setAlignment(AlignCenter);
  ValueLab->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  QFont fnt = ValueLab->font(); fnt.setBold(true); ValueLab->setFont(fnt);
  GroupC1Layout->addWidget(ValueLab, 0, 1);

  TextLabelOpaque = new QLabel(GroupC1, "TextLabelOpaque");
  TextLabelOpaque->setText(tr("SMESH_TRANSPARENCY_OPAQUE" ));
  TextLabelOpaque->setAlignment(AlignRight);
  GroupC1Layout->addWidget(TextLabelOpaque, 0, 2);

  Slider1 = new QSlider(0, 10, 1, 5, Horizontal, GroupC1, "Slider1");
  Slider1->setFocusPolicy(QWidget::NoFocus);
  Slider1->setMinimumSize(300, 0);
  Slider1->setTickmarks(QSlider::Above);
  Slider1->setTickInterval(10);
  Slider1->setTracking(true);
  Slider1->setMinValue(0);
  Slider1->setMaxValue(100);
  Slider1->setLineStep(1);
  Slider1->setPageStep(10);
  GroupC1Layout->addMultiCellWidget(Slider1, 1, 1, 0, 2);

  /*************************************************************************/
  QGroupBox* GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  QGridLayout* GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);

  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_CLOSE"));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  buttonHelp = new QPushButton(GroupButtons, "buttonHelp");
  buttonHelp->setText(tr("SMESH_BUT_HELP"));
  buttonHelp->setAutoDefault(TRUE);

  //GroupButtonsLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  GroupButtonsLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1);
  GroupButtonsLayout->addWidget(buttonHelp, 0, 2);  
  //GroupButtonsLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);

  SMESHGUI_TransparencyDlgLayout->addWidget(GroupC1,      0, 0);
  SMESHGUI_TransparencyDlgLayout->addWidget(GroupButtons, 1, 0);

  // Initial state
  this->onSelectionChanged();

  // signals and slots connections : after ValueHasChanged()
  connect(buttonOk, SIGNAL(clicked()),         this, SLOT(ClickOnOk()));
  connect(buttonHelp, SIGNAL(clicked()),       this, SLOT(ClickOnHelp()));
  connect(Slider1,  SIGNAL(valueChanged(int)), this, SLOT(SetTransparency()));
  connect(Slider1,  SIGNAL(sliderMoved(int)),  this, SLOT(ValueHasChanged()));
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnOk()));
  connect(mySelectionMgr,  SIGNAL(currentSelectionChanged()), this, SLOT(onSelectionChanged()));

  myHelpFileName = "transparency.htm";

  this->show();
}

//=================================================================================
// function : ~SMESHGUI_TransparencyDlg()
// purpose  :
//=================================================================================
SMESHGUI_TransparencyDlg::~SMESHGUI_TransparencyDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=======================================================================
// function : ClickOnOk()
// purpose  :
//=======================================================================
void SMESHGUI_TransparencyDlg::ClickOnOk()
{
  close();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_TransparencyDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
		QString platform;
#ifdef WIN32
		platform = "winapplication";
#else
		platform = "application";
#endif
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=================================================================================
// function : SetTransparency()
// purpose  : Called when value of slider change
//          : or the first time as initilisation
//=================================================================================
void SMESHGUI_TransparencyDlg::SetTransparency()
{
  if( myViewWindow ) {
    SUIT_OverrideCursor wc;
    float opacity = this->Slider1->value() / 100.;

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList);

    SALOME_ListIteratorOfListIO It (aList);
    for (;It.More(); It.Next()) {
      Handle(SALOME_InteractiveObject) IOS = It.Value();
      SMESH_Actor* anActor = SMESH::FindActorByEntry(IOS->getEntry());
      if (anActor)
	anActor->SetOpacity(opacity);
    }
    myViewWindow->Repaint();
  }
  ValueHasChanged();
}

//=================================================================================
// function : ValueHasChanged()
// purpose  : Called when user moves a slider
//=================================================================================
void SMESHGUI_TransparencyDlg::ValueHasChanged()
{
  ValueLab->setText(QString::number(this->Slider1->value()) + "%");
}

//=================================================================================
// function : onSelectionChanged()
// purpose  : Called when selection is changed
//=================================================================================
void SMESHGUI_TransparencyDlg::onSelectionChanged()
{
  if( myViewWindow ) {
    int opacity = 100;

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList);

    if (aList.Extent() == 1) {
      Handle(SALOME_InteractiveObject) FirstIOS = aList.First();
      if (!FirstIOS.IsNull()) {
	SMESH_Actor* anActor = SMESH::FindActorByEntry(FirstIOS->getEntry());
	if (anActor)
	  opacity = int(anActor->GetOpacity() * 100. + 0.5);
      }
    } else if (aList.Extent() > 1) {
      SALOME_ListIteratorOfListIO It (aList);
      int setOp = -1;
      for (; It.More(); It.Next()) {
	Handle(SALOME_InteractiveObject) IO = It.Value();
	if (!IO.IsNull()) {
	  SMESH_Actor* anActor = SMESH::FindActorByEntry(IO->getEntry());
	  if (anActor) {
	    int op = int(anActor->GetOpacity() * 100. + 0.5);
	    if (setOp < 0)
	      setOp = op;
	    else if (setOp != op) {
	      setOp = 100;
	      break;
	    }
	  }
	}
      }
      if (setOp >= 0)
	opacity = setOp;
    } else {
    }
    Slider1->setValue(opacity);
  }
  ValueHasChanged();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_TransparencyDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Key_F1 )
    {
      e->accept();
      ClickOnHelp();
    }
}
