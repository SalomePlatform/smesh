//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_WhatIsDlg.cxx
// Author : Vladimir TURIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_WhatIsDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_VolumeTool.hxx"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_Selection.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>
#include <SALOME_ListIteratorOfListIO.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <gp_XYZ.hxx>

// Qt includes
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

#define SPACING 6
#define MARGIN  11

//=================================================================================
// class    : SMESHGUI_WhatIsDlg()
// purpose  :
//=================================================================================
SMESHGUI_WhatIsDlg::SMESHGUI_WhatIsDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("SMESH_WHAT_IS_TITLE"));
  setSizeGripEnabled(true);
  QVBoxLayout* SMESHGUI_WhatIsDlgLayout = new QVBoxLayout(this);
  SMESHGUI_WhatIsDlgLayout->setSpacing(SPACING);
  SMESHGUI_WhatIsDlgLayout->setMargin(MARGIN);
  
  /***************************************************************/
  GroupMesh = new QGroupBox(this);
  QHBoxLayout* GroupMeshLayout = new QHBoxLayout(GroupMesh);
  GroupMeshLayout->setSpacing(SPACING);
  GroupMeshLayout->setMargin(MARGIN);

  MeshLabel = new QLabel(tr("SMESH_NAME"), GroupMesh);
  GroupMeshLayout->addWidget(MeshLabel);
  MeshName = new QLineEdit(GroupMesh);
  MeshName->setReadOnly(true);
  GroupMeshLayout->addWidget(MeshName);

  /***************************************************************/
  GroupSelections = new QGroupBox(tr("ENTITY_TYPE"), this);
  QButtonGroup* GroupSel = new QButtonGroup(this);
  QHBoxLayout* GroupSelectionsLayout = new QHBoxLayout(GroupSelections);
  GroupSelectionsLayout->setSpacing(SPACING);
  GroupSelectionsLayout->setMargin(MARGIN);

  RadioButtonNodes = new QRadioButton(tr("SMESH_NODES"), GroupSelections);
  GroupSelectionsLayout->addWidget(RadioButtonNodes);
  GroupSel->addButton(RadioButtonNodes, 0);
  RadioButtonElements = new QRadioButton(tr("SMESH_ELEMENTS"), GroupSelections);
  GroupSelectionsLayout->addWidget(RadioButtonElements);
  GroupSel->addButton(RadioButtonElements, 1);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_INFORMATION"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  // Controls for elements selection
  TextLabelElements  = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);
  GroupArgumentsLayout->addWidget(TextLabelElements, 0, 0);

  LineEditElements  = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(new SMESHGUI_IdValidator(this));
  GroupArgumentsLayout->addWidget(LineEditElements, 0, 1);

  // information text browser
  Info = new QTextBrowser(GroupArguments);
  Info->setMinimumSize(300, 200);
  GroupArgumentsLayout->addWidget(Info, 1, 0, 1, 2);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_OK"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonHelp);

  SMESHGUI_WhatIsDlgLayout->addWidget(GroupMesh);
  SMESHGUI_WhatIsDlgLayout->addWidget(GroupSelections);
  SMESHGUI_WhatIsDlgLayout->addWidget(GroupArguments);
  SMESHGUI_WhatIsDlgLayout->addWidget(GroupButtons);

  RadioButtonNodes->setChecked(true);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Costruction of the logical filter
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QList<SUIT_SelectionFilter*> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter(aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "mesh_infos_page.html#mesh_element_info_anchor";

  Init();

  /* signals and slots connections */
  connect(buttonOk,         SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonHelp,       SIGNAL(clicked()),     this, SLOT(ClickOnHelp()));
  connect(GroupSel,         SIGNAL(buttonClicked(int)), SLOT(SelectionsClicked(int)));

  connect(mySMESHGUI,       SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr,   SIGNAL(currentSelectionChanged()),      this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL(SignalCloseAllDialogs()),  this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));

  SelectionsClicked(0);
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_WhatIsDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_WhatIsDlg::~SMESHGUI_WhatIsDlg()
{
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
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser",
                                                                 "application")).
                             arg(myHelpFileName));
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

    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);

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

  if (nbSel < 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  myMesh = SMESH::GetMeshByIO(IO);
  if (myMesh->_is_nil())
    return;

  if (nbSel != 1) {
    //check if all selected objects belongs to one mesh
    SALOME_ListIteratorOfListIO io( aList );
    for (io.Next(); io.More(); io.Next() ) {
      SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO(io.Value());
      if (!mesh->_is_nil() && !mesh->_is_equivalent( myMesh ))
        return;
    }
    // select IO with any element selected (for case of selection by rectangle)
    IO.Nullify();
    for (io.Initialize(aList); io.More() && IO.IsNull(); io.Next() )
      if ( mySelector->HasIndex( io.Value() ))
        IO = io.Value();
    if ( IO.IsNull() ) return;
    // unhilight others
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI )) {
      for (io.Initialize(aList); io.More(); io.Next() )
        if ( !IO->isSame( io.Value() ))
          aViewWindow->highlight( io.Value(), false, true );
    }
  }

  myActor = SMESH::FindActorByObject(myMesh);
  if (!myActor)
    myActor = SMESH::FindActorByEntry(IO->getEntry());
  if (!myActor)
    return;

  QString aName = IO->getName();
  // cut off wite spaces from tail, else meaningful head is not visible
  int size = aName.length();
  while (size && aName.at(size-1).isSpace() )
    --size;
  if ( size != aName.length() )
    aName.truncate( size );
  MeshName->setText(aName); // can be something like "2 objects"

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
    anInfo="<b>" + tr("ENTITY_TYPE") + ":</b> ";
    if(e->GetType() == SMDSAbs_Node) {
      anInfo+=tr("MESH_NODE")+"<br>";
      //const SMDS_MeshNode *en = (SMDS_MeshNode*) e; // VSR: not used!
    } else if (e->GetType() == SMDSAbs_0DElement) {
      anInfo+=tr("SMESH_ELEM0D")+"<br>";
    } else if(e->GetType() == SMDSAbs_Edge) {
      anInfo+=tr("SMESH_EDGE")+"<br>";
      anInfo+="<b>" + tr("SMESH_MESHINFO_TYPE")+":</b> ";
      const SMDS_MeshEdge *ee = (SMDS_MeshEdge*) e;
      anInfo+=(ee->IsQuadratic()?tr("SMESH_MESHINFO_ORDER2"):tr("SMESH_MESHINFO_ORDER1"))+"<br>";
    } else if(e->GetType() == SMDSAbs_Face) {
      const SMDS_MeshFace *ef = (SMDS_MeshFace*) e;
      anInfo+=tr("SMESH_FACE")+"<br>";
      anInfo+="<b>" + tr("SMESH_MESHINFO_TYPE")+":</b> ";
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
      anInfo+="<br>";
    } else if(e->GetType() == SMDSAbs_Volume) {
      anInfo+=tr("SMESH_VOLUME")+"<br>";
      anInfo+="<b>" + tr("SMESH_MESHINFO_TYPE")+":</b> ";
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
      anInfo+="<br>";
    }
    gp_XYZ anXYZ(0.,0.,0.);
    SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
    int nbNodes = 0;
    QString aNodesInfo="";
    for( ; nodeIt->more(); nbNodes++) {
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
      anXYZ.Add( gp_XYZ( node->X(), node->Y(), node->Z() ) );
      if(e->GetType() != SMDSAbs_Node)
        aNodesInfo+=QString("<b>Node %1:</b><br>Id=%2, X=%3, Y=%4, Z=%5<br>").arg(nbNodes+1).arg(node->GetID()).arg(node->X()).arg(node->Y()).arg(node->Z());
      // Calculate Connectivity
      SMDS_ElemIteratorPtr it = node->GetInverseElementIterator();
      if (it) {
        aNodesInfo+="<b>" + tr("CONNECTED_ELEMENTS") + ":</b>";
        while (it->more()) {
          const SMDS_MeshElement* elem = it->next();
          aNodesInfo+=QString(" %1").arg(elem->GetID());
        }
        if ( (nbNodes+1) != e->NbNodes())
          aNodesInfo+=QString("<br><br>");
      }
    }
    if(e->GetType() != SMDSAbs_Node)
      anInfo+="<b>" + tr("GRAVITY_CENTER") + ":</b><br>";
    anXYZ.Divide(e->NbNodes());
    anInfo+=QString("X=%1, Y=%2, Z=%3").arg(anXYZ.X()).arg(anXYZ.Y()).arg(anXYZ.Z());
    if(e->GetType() != SMDSAbs_Node)
      anInfo+="<br>";
    if (aNodesInfo!="")
      anInfo+= "<br>" + aNodesInfo;
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

  if ( SMESH::GetViewWindow( mySMESHGUI ))
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
  ClickOnCancel();
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

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
