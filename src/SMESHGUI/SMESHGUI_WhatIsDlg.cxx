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
//  File   : SMESHGUI_WhatIsDlg.cxx
//  Author : Vladimir TURIN
//  Module : SMESH
//  $Header: 

#include "SMESHGUI_WhatIsDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_VolumeTool.hxx"

#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "LightApp_Application.h"

#include "SVTK_ViewModel.h"
#include "SVTK_Selection.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <gp_XYZ.hxx>

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qpixmap.h>
#include <qtextbrowser.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

using namespace std;

//=================================================================================
// class    : SMESHGUI_WhatIsDlg()
// purpose  :
//=================================================================================
SMESHGUI_WhatIsDlg::SMESHGUI_WhatIsDlg( SMESHGUI* theModule, const char* name,
					bool modal, WFlags fl)
  : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
	     WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  if (!name)
    setName("SMESHGUI_WhatIsDlg");
  resize(300, 500);
  setCaption(tr("SMESH_WHAT_IS_TITLE"));
  setSizeGripEnabled(TRUE);
  SMESHGUI_WhatIsDlgLayout = new QGridLayout(this);
  SMESHGUI_WhatIsDlgLayout->setSpacing(6);
  SMESHGUI_WhatIsDlgLayout->setMargin(11);
  
  /***************************************************************/
  GroupMesh = new QButtonGroup(this, "GroupSelections");
  GroupMesh->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, GroupMesh->sizePolicy().hasHeightForWidth()));
  GroupMesh->setTitle(tr(""));
  GroupMesh->setColumnLayout(0, Qt::Vertical);
  GroupMesh->layout()->setSpacing(0);
  GroupMesh->layout()->setMargin(0);
  GroupMeshLayout = new QGridLayout(GroupMesh->layout());
  GroupMeshLayout->setAlignment(Qt::AlignTop);
  GroupMeshLayout->setSpacing(6);
  GroupMeshLayout->setMargin(11);
  MeshLabel = new QLabel(GroupMesh, "MeshLabel");
  MeshLabel->setText(tr("SMESH_NAME"));
  GroupMeshLayout->addWidget(MeshLabel, 0, 0);
  MeshName = new QLabel(GroupMesh, "MeshName");
  MeshName->setText(tr(""));
  GroupMeshLayout->addWidget(MeshName, 0, 1);
  SMESHGUI_WhatIsDlgLayout->addWidget(GroupMesh, 0, 0);

  /***************************************************************/
  GroupSelections = new QButtonGroup(this, "GroupSelections");
  GroupSelections->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, GroupSelections->sizePolicy().hasHeightForWidth()));
  GroupSelections->setTitle(tr("ENTITY_TYPE" ));
  GroupSelections->setExclusive(TRUE);
  GroupSelections->setColumnLayout(0, Qt::Vertical);
  GroupSelections->layout()->setSpacing(0);
  GroupSelections->layout()->setMargin(0);
  GroupSelectionsLayout = new QGridLayout(GroupSelections->layout());
  GroupSelectionsLayout->setAlignment(Qt::AlignTop);
  GroupSelectionsLayout->setSpacing(6);
  GroupSelectionsLayout->setMargin(11);
  RadioButtonNodes = new QRadioButton(GroupSelections, "RadioButtonNodes");
  RadioButtonNodes->setText(tr("SMESH_NODES"));
  GroupSelectionsLayout->addWidget(RadioButtonNodes, 0, 0);
  RadioButtonElements = new QRadioButton(GroupSelections, "RadioButtonElements");
  RadioButtonElements->setText(tr("SMESH_ELEMENTS"));
  GroupSelectionsLayout->addWidget(RadioButtonElements, 0, 1 );
  SMESHGUI_WhatIsDlgLayout->addWidget(GroupSelections, 1, 0);

  /***************************************************************/
  GroupArguments = new QGroupBox(this, "GroupArguments");
  GroupArguments->setTitle(tr("SMESH_INFORMATION"));
  GroupArguments->setColumnLayout(0, Qt::Vertical);
  GroupArguments->layout()->setSpacing(0);
  GroupArguments->layout()->setMargin(0);
  GroupArgumentsLayout = new QGridLayout(GroupArguments->layout());
  GroupArgumentsLayout->setAlignment(Qt::AlignTop);
  GroupArgumentsLayout->setSpacing(6);
  GroupArgumentsLayout->setMargin(11);

  // Controls for elements selection
  TextLabelElements  = new QLabel(GroupArguments, "TextLabelElements");
  TextLabelElements->setText(tr("SMESH_ID_ELEMENTS" ));
  TextLabelElements->setFixedWidth(74);
  GroupArgumentsLayout->addWidget(TextLabelElements, 0, 0);

  LineEditElements  = new QLineEdit(GroupArguments, "LineEditElements");
  LineEditElements->setValidator(new SMESHGUI_IdValidator(this, "validator"));
  GroupArgumentsLayout->addMultiCellWidget(LineEditElements, 0, 0, 2, 7);

  // information text browser
  Info = new QTextBrowser(GroupArguments, "Info");
  Info->setHScrollBarMode(QScrollView::AlwaysOff);
  Info->setVScrollBarMode(QScrollView::AlwaysOff);
  GroupArgumentsLayout->addMultiCellWidget(Info, 1, 1, 0, 7);

  SMESHGUI_WhatIsDlgLayout->addWidget(GroupArguments, 2, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, GroupButtons->sizePolicy().hasHeightForWidth()));
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
  GroupButtonsLayout->addWidget(buttonHelp, 0, 3);
  QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 1);
  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK" ));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  SMESHGUI_WhatIsDlgLayout->addWidget(GroupButtons, 3, 0);

  GroupArguments->show();
  RadioButtonNodes->setChecked(TRUE);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Costruction of the logical filter
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QPtrList<SUIT_SelectionFilter> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter(aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "/files/viewing_mesh_info.htm#element_infos";

  Init();

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonHelp, SIGNAL(clicked()),   this, SLOT(ClickOnHelp()));
  connect(GroupSelections, SIGNAL(clicked(int)), SLOT(SelectionsClicked(int)));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));
  this->show(); /* displays Dialog */

  SelectionsClicked(0);
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_WhatIsDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_WhatIsDlg::~SMESHGUI_WhatIsDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::Init (bool ResetControls)
{
  myBusy = false;

  LineEditElements->clear();

  myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();

  if (ResetControls) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( CellSelection );
    onTextChange(LineEditElements->text());
    
    SelectionIntoArgument();
  }
}

//=================================================================================
// function : SelectionsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_WhatIsDlg::SelectionsClicked (int selectionId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  mySelectionMgr->clearFilters();

  switch (selectionId) {
  case 0:
    {
      SMESH::SetPointRepresentation(true);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode( NodeSelection );
      break;
    }    
  case 1:
    {
      SMESH::SetPointRepresentation(false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode( CellSelection );
      break;
    }
  }

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::ClickOnOk()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  SMESH::UpdateView();
  Init(false);
  SelectionIntoArgument();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::ClickOnCancel()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", "application")).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=======================================================================
// function : onTextChange()
// purpose  :
//=======================================================================
void SMESHGUI_WhatIsDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) return;
  myBusy = true;

  // hilight entered elements
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
    
    TColStd_MapOfInteger newIndices;

    QStringList aListId = QStringList::split(" ", theNewText, false);

    for (int i = 0; i < aListId.count(); i++) {
      const SMDS_MeshElement * e = RadioButtonNodes->isChecked()?
	aMesh->FindNode(aListId[ i ].toInt()):
	aMesh->FindElement(aListId[ i ].toInt());
      if (e)
	newIndices.Add(e->GetID());
    }

    mySelector->AddOrRemoveIndex( anIO, newIndices, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( anIO, true, true );
  }

  SelectionIntoArgument();

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_WhatIsDlg::SelectionIntoArgument()
{
  int curBusy = myBusy;

  // clear
  myActor = 0;
  QString aString = "";

  myBusy = true;
  if(!curBusy)
    LineEditElements->setText(aString);
  MeshName->setText(aString);
  GroupMesh->setTitle(tr(""));
  Info->clear();
  myBusy = curBusy;

  if (!GroupButtons->isEnabled()) // inactive
    return;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  int nbSel = aList.Extent();

  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  myMesh = SMESH::GetMeshByIO(IO);
  if (myMesh->_is_nil())
    return;

  myActor = SMESH::FindActorByObject(myMesh);
  if (!myActor)
    myActor = SMESH::FindActorByEntry(IO->getEntry());
  if (!myActor)
    return;

  QString aName;
  SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aName);
  MeshName->setText(aName);
  if(!SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO)->_is_nil()) {
    GroupMesh->setTitle(tr("SMESH_MESH"));
  } else if(!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil()) {
    GroupMesh->setTitle(tr("SMESH_SUBMESH"));
  } else if(!SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO)->_is_nil()) {
    GroupMesh->setTitle(tr("SMESH_GROUP"));
  }

  int aNbUnits = 0;
  
  aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, IO, aString);
  
  if (aNbUnits < 1)
    return;

  const SMDS_MeshElement * e = RadioButtonNodes->isChecked()?
    myActor->GetObject()->GetMesh()->FindNode(aString.toInt()):
    myActor->GetObject()->GetMesh()->FindElement(aString.toInt());
  if (e) {
    QString anInfo;
    anInfo=tr("ENTITY_TYPE") + ": ";
    if(e->GetType() == SMDSAbs_Node) {
      anInfo+=tr("MESH_NODE")+"\n";
      const SMDS_MeshNode *en = (SMDS_MeshNode*) e;
    } else if(e->GetType() == SMDSAbs_Edge) {
      anInfo+=tr("SMESH_EDGE")+"\n";
      anInfo+=tr("SMESH_MESHINFO_TYPE")+": ";
      const SMDS_MeshEdge *ee = (SMDS_MeshEdge*) e;
      anInfo+=(ee->IsQuadratic()?tr("SMESH_MESHINFO_ORDER2"):tr("SMESH_MESHINFO_ORDER1"))+"\n";
    } else if(e->GetType() == SMDSAbs_Face) {
      const SMDS_MeshFace *ef = (SMDS_MeshFace*) e;
      anInfo+=tr("SMESH_FACE")+"\n";
      anInfo+=tr("SMESH_MESHINFO_TYPE")+": ";
      if(!ef->IsPoly())
	anInfo+=(ef->IsQuadratic()?tr("SMESH_MESHINFO_ORDER2"):tr("SMESH_MESHINFO_ORDER1"))+" ";
      switch(ef->NbNodes()) {
      case 3:
      case 6:
	{
	  anInfo+=tr("SMESH_TRIANGLE");
	  break;
	}
      case 4:
      case 8:
	{
	  anInfo+=tr("SMESH_QUADRANGLE");
	  break;
	}
      default:
	break;
      }
      anInfo+="\n";
    } else if(e->GetType() == SMDSAbs_Volume) {
      anInfo+=tr("SMESH_VOLUME")+"\n";
      anInfo+=tr("SMESH_MESHINFO_TYPE")+": ";
      const SMDS_MeshVolume *ev = (SMDS_MeshVolume*) e;
      SMDS_VolumeTool vt(ev);
      if(vt.GetVolumeType() != SMDS_VolumeTool::POLYHEDA)
	anInfo+=(ev->IsQuadratic()?tr("SMESH_MESHINFO_ORDER2"):tr("SMESH_MESHINFO_ORDER1"))+" ";
      switch(vt.GetVolumeType()) {
      case SMDS_VolumeTool::TETRA:
      case SMDS_VolumeTool::QUAD_TETRA:
	{
	  anInfo+=tr("SMESH_TETRAS");
	  break;
	}
      case SMDS_VolumeTool::PYRAM:
      case SMDS_VolumeTool::QUAD_PYRAM:
	{
	  anInfo+=tr("SMESH_PYRAMID");
	  break;
	}
      case SMDS_VolumeTool::PENTA:
      case SMDS_VolumeTool::QUAD_PENTA:
	{
	  anInfo+=tr("SMESH_PRISM");
	  break;
	}
      case SMDS_VolumeTool::HEXA:
      case SMDS_VolumeTool::QUAD_HEXA:
	{
	  anInfo+=tr("SMESH_HEXAS");
	  break;
	}
      case SMDS_VolumeTool::POLYHEDA:
	{
	  anInfo+=tr("SMESH_POLYEDRON");
	  break;
	}
      default:
	break;
      }
      anInfo+="\n";
    }
    if(e->GetType() != SMDSAbs_Node)
      anInfo+=tr("GRAVITY_CENTER") + ":\n";
    gp_XYZ anXYZ(0.,0.,0.);
    SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
    int nbNodes = 0;
    for(; nodeIt->more(); nbNodes++) {
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
      anXYZ.Add( gp_XYZ( node->X(), node->Y(), node->Z() ) );
    }
    anXYZ.Divide(e->NbNodes()) ;
    anInfo+=QString("X=%1\nY=%2\nZ=%3\n").arg(anXYZ.X()).arg(anXYZ.Y()).arg(anXYZ.Z());
    Info->setText(anInfo);
  }

  if(!curBusy) {
    myBusy = true;
    LineEditElements->setText(aString);
    myBusy = false;
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::DeactivateActiveDialog()
{
  if (GroupArguments->isEnabled()) {
    GroupSelections->setEnabled(false);
    GroupMesh->setEnabled(false);
    GroupArguments->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);
  GroupSelections->setEnabled(true);
  GroupMesh->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    SelectionsClicked(RadioButtonNodes->isChecked()?0:1);

  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::enterEvent (QEvent*)
{
  if (!GroupArguments->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  this->ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_WhatIsDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_WhatIsDlg::keyPressEvent( QKeyEvent* e )
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
