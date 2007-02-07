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
//  File   : SMESHGUI_RemoveElementsDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RemoveElementsDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"

#include "SUIT_ResourceMgr.h"
#include "SUIT_Desktop.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "SVTK_Selector.h"
#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SALOME_ListIO.hxx"

#include "SalomeApp_Tools.h"
#include "LightApp_Application.h"
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
// class    : SMESHGUI_RemoveElementsDlg()
// purpose  :
//=================================================================================
SMESHGUI_RemoveElementsDlg
::SMESHGUI_RemoveElementsDlg (SMESHGUI* theModule, 
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
    QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_REM_ELEMENT")));
    QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

    if (!name)
      setName("SMESHGUI_RemoveElementsDlg");
    resize(303, 185);
    setCaption(tr("SMESH_REMOVE_ELEMENTS_TITLE"));
    setSizeGripEnabled(TRUE);
    SMESHGUI_RemoveElementsDlgLayout = new QGridLayout(this);
    SMESHGUI_RemoveElementsDlgLayout->setSpacing(6);
    SMESHGUI_RemoveElementsDlgLayout->setMargin(11);

    /***************************************************************/
    GroupConstructors = new QButtonGroup(this, "GroupConstructors");
    GroupConstructors->setTitle(tr("SMESH_ELEMENTS" ));
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
    SMESHGUI_RemoveElementsDlgLayout->addWidget(GroupConstructors, 0, 0);

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
    buttonHelp = new QPushButton(GroupButtons, "buttonHelp");
    buttonHelp->setText(tr("SMESH_BUT_HELP" ));
    buttonHelp->setAutoDefault(TRUE);
    GroupButtonsLayout->addWidget(buttonHelp, 0, 4);
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
    SMESHGUI_RemoveElementsDlgLayout->addWidget(GroupButtons, 2, 0);

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
    TextLabelC1A1->setText(tr("SMESH_ID_ELEMENTS" ));
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
    SMESHGUI_RemoveElementsDlgLayout->addWidget(GroupC1, 1, 0);

    myHelpFileName = "/files/removing_nodes_and_elements.htm#remove_an_element";

    Init(); /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_RemoveElementsDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RemoveElementsDlg::~SMESHGUI_RemoveElementsDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::Init()
{
  GroupC1->show();
  myConstructorId = 0;
  Constructor1->setChecked(TRUE);
  myEditCurrentArgument = LineEditC1A1;

  myNbOkElements = false;
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myActor = 0;
  myBusy = false;

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(GroupConstructors, SIGNAL(clicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectButtonC1A1, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(myEditCurrentArgument, SIGNAL(textChanged(const QString&)),
           SLOT(onTextChange(const QString&)));

  this->show(); /* displays Dialog */

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(CellSelection);

  SelectionIntoArgument();
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RemoveElementsDlg::ConstructorsClicked (int)
{
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;
  if (myNbOkElements) {
    QStringList aListId = QStringList::split(" ", myEditCurrentArgument->text(), false);
    SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
    anArrayOfIdeces->length(aListId.count());
    for (int i = 0; i < aListId.count(); i++)
      anArrayOfIdeces[i] = aListId[ i ].toInt();

    bool aResult = false;
    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      aResult = aMeshEditor->RemoveElements(anArrayOfIdeces.inout());
    } catch (const SALOME::SALOME_Exception& S_ex) {
      SalomeApp_Tools::QtCatchCorbaException(S_ex);
      myEditCurrentArgument->clear();
    } catch (...){
      myEditCurrentArgument->clear();
    }

    if (aResult) {
      myEditCurrentArgument->clear();
      mySelector->ClearIndex();
      SMESH::UpdateView();
    }
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::ClickOnOk()
{
  this->ClickOnApply();
  this->ClickOnCancel();

  return;
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::ClickOnCancel()
{
  //mySelectionMgr->clearSelected();
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
  return;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::ClickOnHelp()
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

//=======================================================================
//function : onTextChange
//purpose  :
//=======================================================================
void SMESHGUI_RemoveElementsDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) 
    return;
  myBusy = true;

  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // hilight entered elements
  if(myActor){
    if(SMDS_Mesh* aMesh = myActor->GetObject()->GetMesh()){
      Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
      
      TColStd_MapOfInteger newIndices;
      
      QStringList aListId = QStringList::split(" ", theNewText, false);
      for (int i = 0; i < aListId.count(); i++) {
	if(const SMDS_MeshElement *anElem = aMesh->FindElement(aListId[i].toInt())) {
	  newIndices.Add(anElem->GetID());
	  myNbOkElements++;
	}
      }
      
      mySelector->AddOrRemoveIndex(anIO,newIndices,false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->highlight(anIO,true,true);
    }
  }
  
  if (myNbOkElements) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
  
  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_RemoveElementsDlg::SelectionIntoArgument()
{
  if (myBusy) return;

  // clear

  myNbOkElements = false;
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
  int nbElems = SMESH::GetNameOfSelectedElements(mySelector,anIO,aString);
  if(nbElems < 1)
    return;
  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myBusy = false;

  // OK

  myNbOkElements = nbElems;

  buttonOk->setEnabled(true);
  buttonApply->setEnabled(true);
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::SetEditCurrentArgument()
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
void SMESHGUI_RemoveElementsDlg::DeactivateActiveDialog()
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
void SMESHGUI_RemoveElementsDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();

  GroupConstructors->setEnabled(true);
  GroupC1->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this); // ??

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  SelectionIntoArgument(); // ??
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  this->ClickOnCancel();
  return;
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_RemoveElementsDlg::hideEvent (QHideEvent * e)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RemoveElementsDlg::keyPressEvent( QKeyEvent* e )
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
