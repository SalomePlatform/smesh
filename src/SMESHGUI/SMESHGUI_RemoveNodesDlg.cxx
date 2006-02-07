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
//  File   : SMESHGUI_RemoveNodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RemoveNodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"

#include "SUIT_ResourceMgr.h"
#include "SUIT_Desktop.h"

#include "SVTK_Selector.h"
#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

// QT Includes
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_RemoveNodesDlg()
// purpose  :
//=================================================================================
SMESHGUI_RemoveNodesDlg
::SMESHGUI_RemoveNodesDlg(SMESHGUI* theModule, 
			  const char* name,
			  bool modal, 
			  WFlags fl)
  : QDialog(SMESH::GetDesktop(theModule), 
	    name, 
	    modal, 
	    WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
    mySelector(SMESH::GetViewWindow(theModule)->GetSelector()),
    mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
    mySMESHGUI(theModule),
    myBusy(false)
{
    QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_REM_NODE")));
    QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

    if (!name)
	setName("SMESHGUI_RemoveNodesDlg");
    resize(303, 185);
    setCaption(tr("SMESH_REMOVE_NODES_TITLE" ));
    setSizeGripEnabled(TRUE);
    SMESHGUI_RemoveNodesDlgLayout = new QGridLayout(this);
    SMESHGUI_RemoveNodesDlgLayout->setSpacing(6);
    SMESHGUI_RemoveNodesDlgLayout->setMargin(11);

    /***************************************************************/
    GroupConstructors = new QButtonGroup(this, "GroupConstructors");
    GroupConstructors->setTitle(tr("SMESH_NODES" ));
    GroupConstructors->setExclusive(TRUE);
    GroupConstructors->setColumnLayout(0, Qt::Vertical);
    GroupConstructors->layout()->setSpacing(0);
    GroupConstructors->layout()->setMargin(0);
    GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
    GroupConstructorsLayout->setAlignment(Qt::AlignTop);
    GroupConstructorsLayout->setSpacing(6);
    GroupConstructorsLayout->setMargin(11);
    Constructor1 = new QRadioButton(GroupConstructors, "Constructor1");
    Constructor1->setText(tr("" ));
    Constructor1->setPixmap(image0);
    Constructor1->setChecked(TRUE);
    Constructor1->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth()));
    Constructor1->setMinimumSize(QSize(50, 0));
    GroupConstructorsLayout->addWidget(Constructor1, 0, 0);
    QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    GroupConstructorsLayout->addItem(spacer, 0, 1);
    SMESHGUI_RemoveNodesDlgLayout->addWidget(GroupConstructors, 0, 0);

    /***************************************************************/
    GroupButtons = new QGroupBox(this, "GroupButtons");
    GroupButtons->setGeometry(QRect(10, 10, 281, 48));
    GroupButtons->setTitle(tr("" ));
    GroupButtons->setColumnLayout(0, Qt::Vertical);
    GroupButtons->layout()->setSpacing(0);
    GroupButtons->layout()->setMargin(0);
    GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
    GroupButtonsLayout->setAlignment(Qt::AlignTop);
    GroupButtonsLayout->setSpacing(6);
    GroupButtonsLayout->setMargin(11);
    buttonCancel = new QPushButton(GroupButtons, "buttonCancel");
    buttonCancel->setText(tr("SMESH_BUT_CLOSE" ));
    buttonCancel->setAutoDefault(TRUE);
    GroupButtonsLayout->addWidget(buttonCancel, 0, 3);
    buttonApply = new QPushButton(GroupButtons, "buttonApply");
    buttonApply->setText(tr("SMESH_BUT_APPLY" ));
    buttonApply->setAutoDefault(TRUE);
    GroupButtonsLayout->addWidget(buttonApply, 0, 1);
    QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    GroupButtonsLayout->addItem(spacer_9, 0, 2);
    buttonOk = new QPushButton(GroupButtons, "buttonOk");
    buttonOk->setText(tr("SMESH_BUT_OK" ));
    buttonOk->setAutoDefault(TRUE);
    buttonOk->setDefault(TRUE);
    GroupButtonsLayout->addWidget(buttonOk, 0, 0);
    SMESHGUI_RemoveNodesDlgLayout->addWidget(GroupButtons, 2, 0);

    /***************************************************************/
    GroupC1 = new QGroupBox(this, "GroupC1");
    GroupC1->setTitle(tr("SMESH_REMOVE" ));
    GroupC1->setMinimumSize(QSize(0, 0));
    GroupC1->setFrameShape(QGroupBox::Box);
    GroupC1->setFrameShadow(QGroupBox::Sunken);
    GroupC1->setColumnLayout(0, Qt::Vertical);
    GroupC1->layout()->setSpacing(0);
    GroupC1->layout()->setMargin(0);
    GroupC1Layout = new QGridLayout(GroupC1->layout());
    GroupC1Layout->setAlignment(Qt::AlignTop);
    GroupC1Layout->setSpacing(6);
    GroupC1Layout->setMargin(11);
    TextLabelC1A1 = new QLabel(GroupC1, "TextLabelC1A1");
    TextLabelC1A1->setText(tr("SMESH_ID_NODES" ));
    TextLabelC1A1->setMinimumSize(QSize(50, 0));
    TextLabelC1A1->setFrameShape(QLabel::NoFrame);
    TextLabelC1A1->setFrameShadow(QLabel::Plain);
    GroupC1Layout->addWidget(TextLabelC1A1, 0, 0);
    SelectButtonC1A1 = new QPushButton(GroupC1, "SelectButtonC1A1");
    SelectButtonC1A1->setText(tr("" ));
    SelectButtonC1A1->setPixmap(image1);
    SelectButtonC1A1->setToggleButton(FALSE);
    GroupC1Layout->addWidget(SelectButtonC1A1, 0, 1);
    LineEditC1A1 = new QLineEdit(GroupC1, "LineEditC1A1");
    LineEditC1A1->setValidator(new SMESHGUI_IdValidator(this, "validator"));
    GroupC1Layout->addWidget(LineEditC1A1, 0, 2);
    SMESHGUI_RemoveNodesDlgLayout->addWidget(GroupC1, 1, 0);

    Init(); /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_RemoveNodesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RemoveNodesDlg::~SMESHGUI_RemoveNodesDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::Init()
{
  GroupC1->show();
  myConstructorId = 0;
  Constructor1->setChecked(TRUE);
  myEditCurrentArgument = LineEditC1A1;

  myNbOkNodes = 0;
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myActor = 0;
  myBusy = false;

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(GroupConstructors, SIGNAL(clicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectButtonC1A1, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(myEditCurrentArgument, SIGNAL(textChanged(const QString&)),
	  SLOT(onTextChange(const QString&)));
  
  /* Move widget on the botton right corner of main widget */
  int x, y;
  mySMESHGUI->DefineDlgPosition(this, x, y);
  this->move(x, y);
  this->show(); /* displays Dialog */

  SMESH::SetPointRepresentation(true);
  
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  SelectionIntoArgument();
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ConstructorsClicked (int)
{
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  if (myNbOkNodes) {
    QStringList aListId = QStringList::split(" ", myEditCurrentArgument->text(), false);
    SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
    anArrayOfIdeces->length(aListId.count());
    for (int i = 0; i < aListId.count(); i++)
      anArrayOfIdeces[i] = aListId[ i ].toInt();

    bool aResult = false;
    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      aResult = aMeshEditor->RemoveNodes(anArrayOfIdeces.inout());
    } catch(...) {
    }

    if (aResult) {
      myEditCurrentArgument->clear();
      mySelector->ClearIndex();
      SMESH::UpdateView();
    }

    SMESH::SetPointRepresentation(true);
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ClickOnCancel()
{
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
}

//=======================================================================
//function : onTextChange
//purpose  :
//=======================================================================
void SMESHGUI_RemoveNodesDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) return;
  myBusy = true;

  myNbOkNodes = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // hilight entered nodes
  if(myActor){
    if(SMDS_Mesh* aMesh = myActor->GetObject()->GetMesh()){
      Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
           
      TColStd_MapOfInteger newIndices;
      
      QStringList aListId = QStringList::split(" ", theNewText, false);
      for (int i = 0; i < aListId.count(); i++) {
	if (const SMDS_MeshNode *aNode = aMesh->FindNode(aListId[i].toInt())) {
	  newIndices.Add(aNode->GetID());
	  myNbOkNodes++;
	}
      }

      mySelector->AddOrRemoveIndex(anIO,newIndices,false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->highlight(anIO,true,true);
    }
  }

  if (myNbOkNodes) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_RemoveNodesDlg::SelectionIntoArgument()
{
  if (myBusy) return;

  // clear

  myNbOkNodes = false;
  myActor = 0;

  myBusy = true;
  myEditCurrentArgument->setText("");
  myBusy = false;

  if (!GroupButtons->isEnabled()) // inactive
    return;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) anIO = aList.First();
  myMesh = SMESH::GetMeshByIO(anIO);
  if (myMesh->_is_nil())
    return;

  myActor = SMESH::FindActorByEntry(anIO->getEntry());
  if (!myActor)
    return;

  // get selected nodes

  QString aString = "";
  int nbNodes = SMESH::GetNameOfSelectedNodes(mySelector,anIO,aString);
  if(nbNodes < 1)
    return;
  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myBusy = false;

  // OK

  myNbOkNodes = true;

  buttonOk->setEnabled(true);
  buttonApply->setEnabled(true);
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  switch (myConstructorId)
    {
    case 0: /* default constructor */
      {
	if(send == SelectButtonC1A1) {
	  LineEditC1A1->setFocus();
	  myEditCurrentArgument = LineEditC1A1;
	}
	SelectionIntoArgument();
	break;
      }
    }
  return;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupC1->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState(); // ??
    mySMESHGUI->SetActiveDialogBox(0); // ??
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();

  GroupConstructors->setEnabled(true);
  GroupC1->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this); // ??

  SMESH::SetPointRepresentation(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  SelectionIntoArgument(); // ??
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  this->ClickOnCancel();
  return;
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_RemoveNodesDlg::hideEvent (QHideEvent * e)
{
  if (!isMinimized())
    ClickOnCancel();
}
