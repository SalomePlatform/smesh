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
//  File   : SMESHGUI_GroupDlg.cxx
//  Author : Natalia KOPNOVA
//  Module : SMESH
//  $Header$

#include "SMESHGUI_GroupDlg.h"
#include "SMESHGUI_FilterDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include "SMESH_TypeFilter.hxx"
#include "SMESH_Actor.h"
#include "GEOMBase.h"

#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"

#include "SalomeApp_Tools.h"
#include "SALOMEDSClient_Study.hxx"
#include "SALOME_ListIO.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"

#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"

#include "utilities.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>

// QT Includes
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmemarray.h>
#include <qwidgetstack.h>

// STL includes
#include <vector>
#include <algorithm>

using namespace std;

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
SMESHGUI_GroupDlg::SMESHGUI_GroupDlg( SMESHGUI* theModule, const char* name,
				      SMESH::SMESH_Mesh_ptr theMesh, bool modal, WFlags fl)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
                WStyle_Title | WStyle_SysMenu | WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
     mySelector(SMESH::GetViewWindow( theModule )->GetSelector()),
     myIsBusy( false ),
     myActor( 0 )
{
  if (!name) setName("SMESHGUI_GroupDlg");
  initDialog(true);
  if (!theMesh->_is_nil())
    init(theMesh);
  else {
    mySelectSubMesh->setEnabled(false);
    mySelectGroup->setEnabled(false);
    myGeomGroupBtn->setEnabled(false);
    myGeomGroupLine->setEnabled(false);
  }


  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition(this, x, y);
  this->move(x, y);
}

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
SMESHGUI_GroupDlg::SMESHGUI_GroupDlg( SMESHGUI* theModule, const char* name,
				      SMESH::SMESH_Group_ptr theGroup, bool modal, WFlags fl)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
                WStyle_Title | WStyle_SysMenu | WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
     mySelector(SMESH::GetViewWindow( theModule )->GetSelector()),
     myIsBusy( false )
{
  if (!name) setName("SMESHGUI_GroupDlg");

  initDialog(false);
  if (!theGroup->_is_nil())
    init(theGroup);
  else {
    mySelectSubMesh->setEnabled(false);
    mySelectGroup->setEnabled(false);

    myCurrentLineEdit = myMeshGroupLine;
    setSelectionMode(5);
  }
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition(this, x, y);
  this->move(x, y);
}

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::initDialog(bool create)
{
  myFilterDlg = 0;
  myCreate = create;
  myCurrentLineEdit = 0;

  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  if (create)
    setCaption(tr("SMESH_CREATE_GROUP_TITLE"));
  else
    setCaption(tr("SMESH_EDIT_GROUP_TITLE"));

  setSizeGripEnabled(TRUE);

  QGridLayout* aMainLayout = new QGridLayout(this, 7, 3, 11, 6);

  /***************************************************************/
  QLabel* meshGroupLab = new QLabel(this, "mesh/group label");
  if (create)
    meshGroupLab->setText(tr("SMESH_MESH"));
  else
    meshGroupLab->setText(tr("SMESH_GROUP"));
  myMeshGroupBtn = new QPushButton(this, "mesh/group button");
  myMeshGroupBtn->setPixmap(image0);
  myMeshGroupLine = new QLineEdit(this, "mesh/group line");
  myMeshGroupLine->setReadOnly(true);

  /***************************************************************/
  myTypeGroup = new QButtonGroup(1, Qt::Vertical, this, "Group types");
  myTypeGroup->setTitle(tr("SMESH_ELEMENTS_TYPE"));
  myTypeGroup->setExclusive(true);

  QStringList types;
  types.append(tr("MESH_NODE"));
  types.append(tr("SMESH_EDGE"));
  types.append(tr("SMESH_FACE"));
  types.append(tr("SMESH_VOLUME"));
  QRadioButton* rb;
  for (int i = 0; i < types.count(); i++) {
    rb = new QRadioButton(types[i], myTypeGroup);
  }
  myTypeGroup->setEnabled(create);
  myTypeId = -1;

  /***************************************************************/
  QLabel* aName = new QLabel(this, "name label");
  aName->setText(tr("SMESH_NAME"));
  aName->setMinimumSize(50,0);
  myName = new QLineEdit(this, "name");

  /***************************************************************/
  myGrpTypeGroup = new QButtonGroup(1, Qt::Vertical, this, "Type of group");
  myGrpTypeGroup->setTitle(tr("SMESH_GROUP_TYPE"));
  myGrpTypeGroup->setExclusive(true);
  QRadioButton* rb1 = new QRadioButton( tr("SMESH_GROUP_STANDALONE"), myGrpTypeGroup);
  QRadioButton* rb2 = new QRadioButton( tr("SMESH_GROUP_GEOMETRY"),   myGrpTypeGroup);
  myGrpTypeGroup->setEnabled(create);
  myGrpTypeId = -1;

  /***************************************************************/
  myWGStack = new QWidgetStack( this, "widget stack");
  QWidget* wg1 = new QFrame( myWGStack, "first widget" );
  QWidget* wg2 = new QFrame( myWGStack, "second widget" );

  /***************************************************************/
  QGroupBox* aContentBox = new QGroupBox(1, Qt::Horizontal, wg1, "content box");
  aContentBox->setTitle(tr("SMESH_CONTENT"));
  QFrame* aContent = new QFrame(aContentBox, "content");
  QGridLayout* aLayout = new QGridLayout(aContent, 7, 4);
  aLayout->setSpacing(6);
  aLayout->setAutoAdd(false);

  QLabel* aLabel = new QLabel(aContent, "elements label");
  aLabel->setText(tr("SMESH_ID_ELEMENTS"));
  myElements = new QListBox(aContent, "elements list");
  myElements->setSelectionMode(QListBox::Extended);

  myFilter = new QPushButton(aContent, "filter");
  myFilter->setText(tr("SMESH_BUT_FILTER"));
  QPushButton* aAddBtn = new QPushButton(aContent, "add");
  aAddBtn->setText(tr("SMESH_BUT_ADD"));
  QPushButton* aRemoveBtn = new QPushButton(aContent, "remove");
  aRemoveBtn->setText(tr("SMESH_BUT_REMOVE"));
  QPushButton* aSortBtn = new QPushButton(aContent, "sort");
  aSortBtn->setText(tr("SMESH_BUT_SORT"));

  aLayout->addWidget(aLabel, 0, 0);
  aLayout->addMultiCellWidget(myElements, 1, 6, 0, 0);
  aLayout->addWidget(myFilter, 1, 2);
  aLayout->addWidget(aAddBtn, 3, 2);
  aLayout->addWidget(aRemoveBtn, 4, 2);
  aLayout->addWidget(aSortBtn, 6, 2);

  aLayout->setColStretch(0, 1);
  aLayout->addColSpacing(1, 20);
  aLayout->addColSpacing(3, 20);
  aLayout->setRowStretch(2, 1);
  aLayout->setRowStretch(5, 1);

  /***************************************************************/
  QGroupBox* aSelectBox = new QGroupBox(3, Qt::Horizontal, wg1, "select box");
  aSelectBox->setTitle(tr("SMESH_SELECT_FROM"));

  mySelectSubMesh = new QCheckBox(aSelectBox, "submesh checkbox");
  mySelectSubMesh->setText(tr("SMESH_SUBMESH"));
  mySelectSubMesh->setMinimumSize(50, 0);
  mySubMeshBtn = new QPushButton(aSelectBox, "submesh button");
  mySubMeshBtn->setText("");
  mySubMeshBtn->setPixmap(image0);
  mySubMeshLine = new QLineEdit(aSelectBox, "submesh line");
  mySubMeshLine->setReadOnly(true);
  onSelectSubMesh(false);

  mySelectGroup = new QCheckBox(aSelectBox, "group checkbox");
  mySelectGroup->setText(tr("SMESH_GROUP"));
  mySelectGroup->setMinimumSize(50, 0);
  myGroupBtn = new QPushButton(aSelectBox, "group button");
  myGroupBtn->setText("");
  myGroupBtn->setPixmap(image0);
  myGroupLine = new QLineEdit(aSelectBox, "group line");
  myGroupLine->setReadOnly(true);
  onSelectGroup(false);

  /***************************************************************/
  QGridLayout* wg1Layout = new QGridLayout( wg1, 3, 1, 0, 6 );
  wg1Layout->addWidget(aContentBox, 0, 0);
  wg1Layout->addWidget(aSelectBox, 1, 0);
  wg1Layout->setRowStretch(2, 5);

  /***************************************************************/
  QLabel* geomObject = new QLabel(wg2, "geometry object label");
  geomObject->setText(tr("SMESH_OBJECT_GEOM"));
  myGeomGroupBtn = new QPushButton(wg2, "geometry group button");
  myGeomGroupBtn->setText("");
  myGeomGroupBtn->setPixmap(image0);
  myGeomGroupLine = new QLineEdit(wg2, "geometry group line");
  myGeomGroupLine->setReadOnly(true); //VSR ???
  onSelectGeomGroup(false);

  /***************************************************************/
  QGridLayout* wg2Layout = new QGridLayout( wg2, 2, 3, 0, 6 );
  wg2Layout->addWidget(geomObject,     0, 0);
  wg2Layout->addWidget(myGeomGroupBtn, 0, 1);
  wg2Layout->addWidget(myGeomGroupLine,0, 2);
  wg2Layout->setRowStretch(1, 5);

  /***************************************************************/
  QVBoxLayout* dumb = new QVBoxLayout(myWGStack);
  dumb->addWidget(wg1);
  dumb->addWidget(wg2);
  myWGStack->addWidget( wg1, myGrpTypeGroup->id(rb1) );
  myWGStack->addWidget( wg2, myGrpTypeGroup->id(rb2) );

  /***************************************************************/
  QFrame* aButtons = new QFrame(this, "button box");
  aButtons->setFrameStyle(QFrame::Box | QFrame::Sunken);
  QHBoxLayout* aBtnLayout = new QHBoxLayout(aButtons, 11, 6);
  aBtnLayout->setAutoAdd(false);

  QPushButton* aOKBtn = new QPushButton(aButtons, "ok");
  aOKBtn->setText(tr("SMESH_BUT_OK"));
  aOKBtn->setAutoDefault(true);
  aOKBtn->setDefault(true);
  QPushButton* aApplyBtn = new QPushButton(aButtons, "apply");
  aApplyBtn->setText(tr("SMESH_BUT_APPLY"));
  aApplyBtn->setAutoDefault(true);
  QPushButton* aCloseBtn = new QPushButton(aButtons, "close");
  aCloseBtn->setText(tr("SMESH_BUT_CLOSE"));
  aCloseBtn->setAutoDefault(true);

  aBtnLayout->addWidget(aOKBtn);
  aBtnLayout->addWidget(aApplyBtn);
  aBtnLayout->addStretch();
  aBtnLayout->addWidget(aCloseBtn);

  /***************************************************************/
  aMainLayout->addWidget(meshGroupLab,    0, 0);
  aMainLayout->addWidget(myMeshGroupBtn,  0, 1);
  aMainLayout->addWidget(myMeshGroupLine, 0, 2);
  aMainLayout->addMultiCellWidget(myTypeGroup,    1, 1, 0, 2);
  aMainLayout->addWidget(aName,      2, 0);
  aMainLayout->addWidget(myName,     2, 2);
  aMainLayout->addMultiCellWidget(myGrpTypeGroup, 3, 3, 0, 2);
  aMainLayout->addMultiCellWidget(myWGStack,      4, 4, 0, 2);
  aMainLayout->setRowStretch( 5, 5 );
  aMainLayout->addMultiCellWidget(aButtons,       6, 6, 0, 2);

  /* signals and slots connections */
  connect(myMeshGroupBtn, SIGNAL(clicked()), this, SLOT(setCurrentSelection()));

  connect(myGrpTypeGroup, SIGNAL(clicked(int)), this, SLOT(onGrpTypeChanged(int)));

  connect(myTypeGroup, SIGNAL(clicked(int)), this, SLOT(onTypeChanged(int)));

  connect(myName, SIGNAL(textChanged(const QString&)), this, SLOT(onNameChanged(const QString&)));
  connect(myElements, SIGNAL(selectionChanged()), this, SLOT(onListSelectionChanged()));

  connect(myFilter, SIGNAL(clicked()), this, SLOT(setFilters()));
  connect(aAddBtn, SIGNAL(clicked()), this, SLOT(onAdd()));
  connect(aRemoveBtn, SIGNAL(clicked()), this, SLOT(onRemove()));
  connect(aSortBtn, SIGNAL(clicked()), this, SLOT(onSort()));

  connect(mySelectSubMesh, SIGNAL(toggled(bool)), this, SLOT(onSelectSubMesh(bool)));
  connect(mySelectGroup, SIGNAL(toggled(bool)), this, SLOT(onSelectGroup(bool)));
  connect(mySubMeshBtn, SIGNAL(clicked()), this, SLOT(setCurrentSelection()));
  connect(myGroupBtn, SIGNAL(clicked()), this, SLOT(setCurrentSelection()));
  connect(myGeomGroupBtn, SIGNAL(clicked()), this, SLOT(setCurrentSelection()));

  connect(aOKBtn, SIGNAL(clicked()), this, SLOT(onOK()));
  connect(aApplyBtn, SIGNAL(clicked()), this, SLOT(onApply()));
  connect(aCloseBtn, SIGNAL(clicked()), this, SLOT(onClose()));

  /* Init selection */
  mySMESHGUI->SetActiveDialogBox(this);
  mySMESHGUI->SetState(800);

  mySelectionMode = -1;
  myMeshFilter = new SMESH_TypeFilter(MESH);
  mySubMeshFilter = new SMESH_TypeFilter(SUBMESH);
  myGroupFilter = new SMESH_TypeFilter(GROUP);

  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), this, SLOT(onClose()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(onObjectSelectionChanged()));

  myGrpTypeGroup->setButton(myGrpTypeGroup->id(rb1)); // VSR !!!
  onGrpTypeChanged(myGrpTypeGroup->id(rb1)); // VSR!!!

  if (myMesh->_is_nil() )
    myTypeGroup->setButton(0);

  updateButtons();
}

//=================================================================================
// function : ~SMESHGUI_GroupDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_GroupDlg::~SMESHGUI_GroupDlg()
{
  // no need to delete child widgets, Qt does it all for us
  if ( myFilterDlg != 0 )
  {
    myFilterDlg->reparent( 0, QPoint() );
    delete myFilterDlg;
  }
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::init (SMESH::SMESH_Mesh_ptr theMesh)
{
  mySelectionMgr->installFilter(myMeshFilter);

  /* init data from current selection */
  myMesh = SMESH::SMESH_Mesh::_duplicate(theMesh);
  myGroup = SMESH::SMESH_Group::_nil();

  myActor = SMESH::FindActorByObject(myMesh);
  SMESH::SetPickable(myActor);

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );
  if( !aList.IsEmpty() )
  {
    QString aName = aList.First()->getName();
    myMeshGroupLine->setText(aName) ;
    myMeshGroupLine->home( false );
  }

  myCurrentLineEdit = 0;

  myTypeGroup->setButton(0);
  onTypeChanged(0);
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::init (SMESH::SMESH_Group_ptr theGroup)
{
  myMesh = theGroup->GetMesh();
  myGroup = SMESH::SMESH_Group::_duplicate(theGroup);

  myActor = SMESH::FindActorByObject(myMesh);
  if ( !myActor )
    myActor = SMESH::FindActorByObject(myGroup);
  SMESH::SetPickable(myActor);

  int aType = 0;
  switch(theGroup->GetType()) {
  case SMESH::NODE: aType= 0; break;
  case SMESH::EDGE: aType = 1; break;
  case SMESH::FACE: aType = 2; break;
  case SMESH::VOLUME: aType = 3; break;
  }

  myName->setText(myGroup->GetName());
  myName->home(false);
  myMeshGroupLine->setText(myGroup->GetName());

  myCurrentLineEdit = 0;
  myTypeGroup->setButton(aType);
  myElements->clear();
  setSelectionMode(aType);
  myTypeId = aType;

  myIdList.clear();
  if (!theGroup->IsEmpty()) {
    SMESH::long_array_var anElements = myGroup->GetListOfID();
    int k = anElements->length();
    for (int i = 0; i < k; i++) {
      myIdList.append(anElements[i]);
      myElements->insertItem(QString::number(anElements[i]));
    }
    myElements->selectAll(true);
  }
}

//=================================================================================
// function : updateButtons()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::updateButtons()
{
  bool enable;

  if (myGrpTypeId == 0)
    enable = !myName->text().stripWhiteSpace().isEmpty() && myElements->count() > 0;
  else if (myGrpTypeId == 1)
    enable = !myName->text().stripWhiteSpace().isEmpty() && !CORBA::is_nil( myGeomGroup );
  QPushButton* aBtn;
  aBtn = (QPushButton*) child("ok", "QPushButton");
  if (aBtn) aBtn->setEnabled(enable);
  aBtn = (QPushButton*) child("apply", "QPushButton");
  if (aBtn) aBtn->setEnabled(enable);
}

//=================================================================================
// function : onNameChanged()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onNameChanged (const QString& text)
{
  updateButtons();
}

//=================================================================================
// function : onTypeChanged()
// purpose  : Group elements type radio button management
//=================================================================================
void SMESHGUI_GroupDlg::onTypeChanged (int id)
{
  if (myTypeId != id) {
    myElements->clear();
    if (myCurrentLineEdit == 0)
      setSelectionMode(id);
  }
  myTypeId = id;
}

//=================================================================================
// function : onGrpTypeChanged()
// purpose  : Group type radio button management
//=================================================================================
void SMESHGUI_GroupDlg::onGrpTypeChanged (int id)
{
  if (myGrpTypeId != id) {
    myWGStack->raiseWidget( id );
    onSelectGeomGroup(id == 1);
  }
  myGrpTypeId = id;
}

//=================================================================================
// function : setSelectionMode()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_GroupDlg::setSelectionMode (int theMode)
{
  // PAL7314
  if (myMesh->_is_nil())
    return;

  if (mySelectionMode != theMode) {
    // [PAL10408] mySelectionMgr->clearSelected();
    mySelectionMgr->clearFilters();
    SMESH::SetPointRepresentation(false);
    if (theMode < 4) {
      switch (theMode) {
      case 0:
        if (myActor)
          myActor->SetPointRepresentation(true);
        else
          SMESH::SetPointRepresentation(true);
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(NodeSelection);
	break;
      case 1:
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(EdgeSelection);
	break;
      case 2:
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(FaceSelection);
	break;
      default:
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(VolumeSelection);
      }
    } else {
      if (theMode == 4)
	mySelectionMgr->installFilter(mySubMeshFilter);
      else if (theMode == 5)
	mySelectionMgr->installFilter(myGroupFilter);
      else if (theMode == 6)
	mySelectionMgr->installFilter(myMeshFilter);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(ActorSelection);
    }
    mySelectionMode = theMode;
  }
}

//=================================================================================
// function : onApply()
// purpose  :
//=================================================================================
bool SMESHGUI_GroupDlg::onApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (myGrpTypeId == 0 &&
      !myName->text().stripWhiteSpace().isEmpty() &&
      myElements->count() > 0) {
    mySelectionMgr->clearSelected();
    if (myGroup->_is_nil()) {
      SMESH::ElementType aType = SMESH::ALL;
      switch(myTypeId) {
      case 0: aType = SMESH::NODE; break;
      case 1: aType = SMESH::EDGE; break;
      case 2: aType = SMESH::FACE; break;
      case 3: aType = SMESH::VOLUME; break;
      }
      SMESH::long_array_var anIdList = new SMESH::long_array;
      int i, k = myElements->count();
      anIdList->length(k);
      QListBoxItem* anItem;
      for (i = 0, anItem = myElements->firstItem(); anItem != 0; i++, anItem = anItem->next()) {
	anIdList[i] = anItem->text().toInt();
      }

      myGroup = SMESH::AddGroup(myMesh, aType, myName->text());
      myGroup->Add(anIdList.inout());

      /* init for next operation */
      myName->setText("");
      myElements->clear();
      myGroup = SMESH::SMESH_Group::_nil();

    } else {
      myGroup->SetName(myName->text());

      QValueList<int> aAddList;
      QValueList<int>::iterator anIt;
      QListBoxItem* anItem;

      for (anItem = myElements->firstItem(); anItem != 0; anItem = anItem->next()) {
	int anId = anItem->text().toInt();
	if ((anIt = myIdList.find(anId)) == myIdList.end())
	  aAddList.append(anId);
	else
	  myIdList.remove(anIt);
      }
      if (!aAddList.empty()) {
	SMESH::long_array_var anIdList = new SMESH::long_array;
	anIdList->length(aAddList.count());
        int i;
	for (i = 0, anIt = aAddList.begin(); anIt != aAddList.end(); anIt++, i++)
	  anIdList[i] = *anIt;
	myGroup->Add(anIdList.inout());
      }
      if (!myIdList.empty()) {
	SMESH::long_array_var anIdList = new SMESH::long_array;
	anIdList->length(myIdList.count());
        int i;
	for (i = 0, anIt = myIdList.begin(); anIt != myIdList.end(); anIt++, i++)
	  anIdList[i] = *anIt;
	myGroup->Remove(anIdList.inout());
      }
      /* init for next operation */
      myIdList.clear();
      for (anItem = myElements->firstItem(); anItem != 0; anItem = anItem->next())
	myIdList.append(anItem->text().toInt());
    }

    mySMESHGUI->updateObjBrowser(true);
    SMESH::UpdateView(); // asv: fix of BUG PAL5515
    mySelectionMgr->clearSelected();
    return true;
  } else if (myGrpTypeId == 1 &&
             !myName->text().stripWhiteSpace().isEmpty() &&
             !CORBA::is_nil(myGeomGroup))
  {
    SMESH::ElementType aType = SMESH::ALL;
    switch (myTypeId) {
    case 0: aType = SMESH::NODE; break;
    case 1: aType = SMESH::EDGE; break;
    case 2: aType = SMESH::FACE; break;
    case 3: aType = SMESH::VOLUME; break;
    }

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    GEOM::GEOM_IGroupOperations_var aGroupOp =
      SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());

    SMESH::SMESH_GroupOnGeom_var aGroupOnGeom =
      myMesh->CreateGroupFromGEOM(aType, myName->text(),myGeomGroup);

    mySMESHGUI->updateObjBrowser(true);
    mySelectionMgr->clearSelected();
    /* init for next operation */
    myName->setText("");
    return true;
  }

  return false;
}

//=================================================================================
// function : onOK()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onOK()
{
  if ( onApply() )
    onClose();
}

//=================================================================================
// function : onListSelectionChanged()
// purpose  : Called when selection in element list is changed
//=================================================================================
void SMESHGUI_GroupDlg::onListSelectionChanged()
{
  //  MESSAGE("SMESHGUI_GroupDlg::onListSelectionChanged(); myActor = " << myActor);
  if( myIsBusy || !myActor) return;
    myIsBusy = true;

  if (myCurrentLineEdit == 0) {
    mySelectionMgr->clearSelected();
    TColStd_MapOfInteger aIndexes;
    QListBoxItem* anItem;
    for (anItem = myElements->firstItem(); anItem != 0; anItem = anItem->next()) {
      if (anItem->isSelected()) {
	int anId = anItem->text().toInt();
	aIndexes.Add(anId);
      }
    }
    mySelector->AddOrRemoveIndex(myActor->getIO(), aIndexes, false);
    SALOME_ListIO aList;
    aList.Append(myActor->getIO());
    mySelectionMgr->setSelectedObjects(aList,false);
  }
  myIsBusy = false;
}

//=================================================================================
// function : onObjectSelectionChanged()
// purpose  : Called when selection in 3D view or ObjectBrowser is changed
//=================================================================================
void SMESHGUI_GroupDlg::onObjectSelectionChanged()
{
  if ( myIsBusy || !isEnabled()) return;
    myIsBusy = true;

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );
  
  int aNbSel = aList.Extent();
  myElements->clearSelection();

  if (myCurrentLineEdit) {
    myCurrentLineEdit->setText("");
    QString aString = "";

    if (myCurrentLineEdit == myMeshGroupLine) {
      mySelectSubMesh->setEnabled(false);
      mySelectGroup->setEnabled(false);
      myGroupLine->setText("");
      mySubMeshLine->setText("");

      myGeomGroupBtn->setEnabled(false);
      myGeomGroupLine->setEnabled(false);
      myGeomGroupLine->setText("");
      if (!myCreate)
        myName->setText("");

      myElements->clear();

      if (aNbSel != 1 ) {
        myGroup = SMESH::SMESH_Group::_nil();
        myMesh = SMESH::SMESH_Mesh::_nil();
        myIsBusy = false;
        return;
      }
      Handle(SALOME_InteractiveObject) IO = aList.First();

      if (myCreate) {
        myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
        if (myMesh->_is_nil())
	{
	  myIsBusy = false;
          return;
	}
        myGroup = SMESH::SMESH_Group::_nil();

        myActor = SMESH::FindActorByObject(myMesh);
        SMESH::SetPickable(myActor);

        aString = aList.First()->getName();
        myMeshGroupLine->setText(aString) ;
        myMeshGroupLine->home( false );

        mySelectSubMesh->setEnabled(true);
        mySelectGroup->setEnabled(true);
        myGeomGroupBtn->setEnabled(true);
        myGeomGroupLine->setEnabled(true);
        updateButtons();
      } else {
        SMESH::SMESH_Group_var aGroup = SMESH::IObjectToInterface<SMESH::SMESH_Group>(IO);
        if (aGroup->_is_nil())
	{
	  myIsBusy = false;
          return;
	}
        myIsBusy = false;
        myCurrentLineEdit = 0;
        init(aGroup);
        myIsBusy = true;
        mySelectSubMesh->setEnabled(true);
        mySelectGroup->setEnabled(true);
        myGeomGroupBtn->setEnabled(true);
        myGeomGroupLine->setEnabled(true);
      }
      myCurrentLineEdit = 0;
      myIsBusy = false;
      if (!myCreate)
        return;

      if (myTypeId == -1)
        onTypeChanged(0);
      else {
        myElements->clear();
        setSelectionMode(myTypeId);
      }

      myIsBusy = false;
      return;

    } else if (myCurrentLineEdit == myGeomGroupLine) {
      if (aNbSel != 1) {
        myGeomGroup = GEOM::GEOM_Object::_nil();
        myIsBusy = false;
        return;
      }

      Standard_Boolean testResult = Standard_False;
      myGeomGroup = GEOMBase::ConvertIOinGEOMObject(aList.First(), testResult);

      // Check if the object is a geometry group
      if (!testResult || CORBA::is_nil(myGeomGroup)) {
        myGeomGroup = GEOM::GEOM_Object::_nil();
        myIsBusy = false;
        return;
      }
      // Check if group constructed on the same shape as a mesh or on its child
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
      GEOM::GEOM_IGroupOperations_var anOp =
        SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());

      // The main shape of the group
      GEOM::GEOM_Object_var aGroupMainShape;
      if (myGeomGroup->GetType() == 37)
        aGroupMainShape = anOp->GetMainShape(myGeomGroup);
      else
        aGroupMainShape = GEOM::GEOM_Object::_duplicate(myGeomGroup);
      _PTR(SObject) aGroupMainShapeSO =
        //aStudy->FindObjectIOR(aStudy->ConvertObjectToIOR(aGroupMainShape));
        aStudy->FindObjectID(aGroupMainShape->GetStudyEntry());

      // The mesh SObject
      _PTR(SObject) aMeshSO = SMESH::FindSObject(myMesh);
      if (!aMeshSO) {
        myGeomGroup = GEOM::GEOM_Object::_nil();
        myIsBusy = false;
        return;
      }
      _PTR(SObject) anObj, aRef;
      bool isRefOrSubShape = false;
      if (aMeshSO->FindSubObject(1, anObj) &&  anObj->ReferencedObject(aRef)) {
        //if (strcmp(aRef->GetID(), aGroupMainShapeSO->GetID()) == 0) {
        if (aRef->GetID() == aGroupMainShapeSO->GetID()) {
          isRefOrSubShape = true;
        } else {
          _PTR(SObject) aFather = aGroupMainShapeSO->GetFather();
          _PTR(SComponent) aComponent = aGroupMainShapeSO->GetFatherComponent();
	  //while (!isRefOrSubShape && strcmp(aFather->GetID(), aComponent->GetID()) != 0) {
	  while (!isRefOrSubShape && aFather->GetID() != aComponent->GetID()) {
            //if (strcmp(aRef->GetID(), aFather->GetID()) == 0)
            if (aRef->GetID() == aFather->GetID())
              isRefOrSubShape = true;
            else
              aFather = aFather->GetFather();
          }
	}
      }
      if (!isRefOrSubShape) {
        myGeomGroup = GEOM::GEOM_Object::_nil();
        myIsBusy = false;
        return;
      }
    }

    if(aNbSel >= 1) {
      if(aNbSel > 1) {
	if(myCurrentLineEdit == mySubMeshLine)
	  aString = tr("SMESH_SUBMESH_SELECTED").arg(aNbSel);
	else if(myCurrentLineEdit == myGroupLine || myCurrentLineEdit == myGeomGroupLine)
	  aString = tr("SMESH_GROUP_SELECTED").arg(aNbSel);
      } else {
	aString = aList.First()->getName();
      }
    }

    myCurrentLineEdit->setText(aString);
    myCurrentLineEdit->home(false);

    updateButtons();

  } else {
    if (aNbSel == 1 && myActor ) {
      QString aListStr = "";
      int aNbItems = 0;
      if (myTypeId == 0) {
	aNbItems = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aListStr);
      } else {
	aNbItems = SMESH::GetNameOfSelectedElements(mySelector, myActor->getIO(), aListStr);
      }
      if (aNbItems > 0) {
	QStringList anElements = QStringList::split(" ", aListStr);
	QListBoxItem* anItem = 0;
	for (QStringList::iterator it = anElements.begin(); it != anElements.end(); ++it) {
	  anItem = myElements->findItem(*it, Qt::ExactMatch);
	  if (anItem) myElements->setSelected(anItem, true);
	}
      }
    }
  }

  if (!myActor) {
    if (!myGroup->_is_nil())
      myActor = SMESH::FindActorByObject(myGroup);
    else
      myActor = SMESH::FindActorByObject(myMesh);
  }

  myIsBusy = false;
}

//=================================================================================
// function : onSelectSubMesh()
// purpose  : Called when selection in 3D view or ObjectBrowser is changed
//=================================================================================
void SMESHGUI_GroupDlg::onSelectSubMesh(bool on)
{
  if (on) {
    if (mySelectGroup->isChecked()) {
      mySelectGroup->setChecked(false);
    }
    //VSR: else if (mySelectGeomGroup->isChecked()) {
    //VSR:   mySelectGeomGroup->setChecked(false);
    //VSR: }
    myCurrentLineEdit = mySubMeshLine;
    setSelectionMode(4);
  }
  else {
    mySubMeshLine->setText("");
    myCurrentLineEdit = 0;
    if (myTypeId != -1)
      setSelectionMode(myTypeId);
  }
  mySubMeshBtn->setEnabled(on);
  mySubMeshLine->setEnabled(on);
}


//=================================================================================
// function : (onSelectGroup)
// purpose  : Called when selection in 3D view or ObjectBrowser is changed
//=================================================================================
void SMESHGUI_GroupDlg::onSelectGroup(bool on)
{
  if (on) {
    if (mySelectSubMesh->isChecked()) {
      mySelectSubMesh->setChecked(false);
    }
    myCurrentLineEdit = myGroupLine;
    setSelectionMode(5);
  }
  else {
    myGroupLine->setText("");
    myCurrentLineEdit = 0;
    if (myTypeId != -1)
      setSelectionMode(myTypeId);
  }
  myGroupBtn->setEnabled(on);
  myGroupLine->setEnabled(on);
}


//=================================================================================
// function : (onSelectGeomGroup)
// purpose  : Called when selection in 3D view or ObjectBrowser is changed
//=================================================================================
void SMESHGUI_GroupDlg::onSelectGeomGroup(bool on)
{
  if (on) {
    if (mySelectSubMesh->isChecked()) {
      mySelectSubMesh->setChecked(false);
    }
    else if (mySelectGroup->isChecked()) {
      mySelectGroup->setChecked(false);
    }
    myCurrentLineEdit = myGeomGroupLine;
    setSelectionMode(7);
  }
  else {
    myGeomGroupLine->setText("");
    myCurrentLineEdit = 0;
    if (myTypeId != -1)
      setSelectionMode(myTypeId);
  }
}


//=================================================================================
// function : setCurrentSelection()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::setCurrentSelection()
{
  QPushButton* send = (QPushButton*)sender();
  myCurrentLineEdit = 0;
  if (send == myMeshGroupBtn) {
    myCurrentLineEdit = myMeshGroupLine;
    if (myCreate)
      setSelectionMode(6);
    else
      setSelectionMode(5);
    onObjectSelectionChanged();
  }
  else if (send == mySubMeshBtn) {
    myCurrentLineEdit = mySubMeshLine;
    onObjectSelectionChanged();
  }
  else if (send == myGroupBtn) {
    myCurrentLineEdit = myGroupLine;
    onObjectSelectionChanged();
  }
  else if (send == myGeomGroupBtn) {
    myCurrentLineEdit = myGeomGroupLine;
    setSelectionMode(7);
    onObjectSelectionChanged();
  }
}


//=================================================================================
// function : setFilters()
// purpose  : SLOT. Called when "Filter" button pressed.
//=================================================================================
void SMESHGUI_GroupDlg::setFilters()
{
  SMESH::ElementType aType = SMESH::ALL;
  switch ( myTypeId )
  {
    case 0 : aType = SMESH::NODE; break;
    case 1 : aType = SMESH::EDGE; break;
    case 2 : aType = SMESH::FACE; break;
    case 3 : aType = SMESH::VOLUME; break;
    default: return;
  }

  if ( myFilterDlg == 0 )
  {
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, aType );
    connect( myFilterDlg, SIGNAL( Accepted() ), SLOT( onFilterAccepted() ) );
  }
  else
    myFilterDlg->Init( aType );

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( myElements );

  myFilterDlg->show();
}

//=================================================================================
// function : onFilterAccepted()
// purpose  : SLOT. Called when Filter dlg closed with OK button.
//            Uncheck "Select submesh" and "Select group" checkboxes
//=================================================================================
void SMESHGUI_GroupDlg::onFilterAccepted()
{
  if ( mySelectSubMesh->isChecked() || mySelectGroup->isChecked() )
  {
    mySelectionMode = myTypeId;
    mySelectSubMesh->setChecked( false );
    mySelectGroup->setChecked( false );
  }
}

//=================================================================================
// function : onAdd()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onAdd()
{
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );

  int aNbSel = aList.Extent();

  if (aNbSel == 0 || !myActor || myMesh->_is_nil()) return;

  myIsBusy = true;

  SMESH::ElementType aType = SMESH::ALL;
  switch(myTypeId) {
  case 0: 
    aType = SMESH::NODE; 
    mySelector->SetSelectionMode(NodeSelection);
    break;
  case 1: 
    aType = SMESH::EDGE; 
    mySelector->SetSelectionMode(EdgeSelection);
    break;
  case 2: 
    aType = SMESH::FACE; 
    mySelector->SetSelectionMode(FaceSelection);
    break;
  case 3: 
    aType = SMESH::VOLUME; 
    mySelector->SetSelectionMode(VolumeSelection);
    break;
  default:
    mySelector->SetSelectionMode(ActorSelection);
  }


  if (myCurrentLineEdit == 0) {
    //if (aNbSel != 1) { myIsBusy = false; return; }
    QString aListStr = "";
    int aNbItems = 0;
    if (myTypeId == 0) {
      aNbItems = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aListStr);
    }
    else {
      aNbItems = SMESH::GetNameOfSelectedElements(mySelector, myActor->getIO(), aListStr);
    }
    if (aNbItems > 0) {
      QStringList anElements = QStringList::split(" ", aListStr);
      QListBoxItem* anItem = 0;
      for (QStringList::iterator it = anElements.begin(); it != anElements.end(); ++it) {
	anItem = myElements->findItem(*it, Qt::ExactMatch);
	if (!anItem) {
	  anItem = new QListBoxText(*it);
	  myElements->insertItem(anItem);
	}
	myElements->setSelected(anItem, true);
      }
    }
  } else if (myCurrentLineEdit == mySubMeshLine) {
    //SALOME_ListIteratorOfListIO anIt (mySelectionMgr->StoredIObjects());
    
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects( aList );

    SALOME_ListIteratorOfListIO anIt (aList);
    for (; anIt.More(); anIt.Next()) {
      SMESH::SMESH_subMesh_var aSubMesh =
        SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIt.Value());
      if (!aSubMesh->_is_nil()) {
	// check if mesh is the same
	if (aSubMesh->GetFather()->GetId() == myMesh->GetId()) {
          try {
            SMESH::long_array_var anElements = aSubMesh->GetElementsByType(aType);
            int k = anElements->length();
            QListBoxItem* anItem = 0;
            for (int i = 0; i < k; i++) {
              QString aText = QString::number(anElements[i]);
              anItem = myElements->findItem(aText, Qt::ExactMatch);
              if (!anItem) {
                anItem = new QListBoxText(aText);
                myElements->insertItem(anItem);
              }
              myElements->setSelected(anItem, true);
            }
          }
          catch (const SALOME::SALOME_Exception& ex) {
            SalomeApp_Tools::QtCatchCorbaException(ex);
          }
        }
      }
    }
    mySelectSubMesh->setChecked(false);
    myIsBusy = false;
    onListSelectionChanged();

  } else if (myCurrentLineEdit == myGroupLine) {
    //SALOME_ListIteratorOfListIO anIt (mySelectionMgr->StoredIObjects());
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects( aList );
    
    SALOME_ListIteratorOfListIO anIt (aList);
    for (; anIt.More(); anIt.Next()) {
      SMESH::SMESH_Group_var aGroup =
        SMESH::IObjectToInterface<SMESH::SMESH_Group>(anIt.Value());
      if (!aGroup->_is_nil()) {
	// check if mesh is the same
	if (aGroup->GetType() == aType && aGroup->GetMesh()->GetId() == myMesh->GetId()) {
	  SMESH::long_array_var anElements = aGroup->GetListOfID();
	  int k = anElements->length();
	  QListBoxItem* anItem = 0;
	  for (int i = 0; i < k; i++) {
	    QString aText = QString::number(anElements[i]);
	    anItem = myElements->findItem(aText, Qt::ExactMatch);
	    if (!anItem) {
	      anItem = new QListBoxText(aText);
	      myElements->insertItem(anItem);
	    }
	    myElements->setSelected(anItem, true);
	  }
	}
      }
    }
    mySelectGroup->setChecked(false);
    myIsBusy = false;
    onListSelectionChanged();

  } else if (myCurrentLineEdit == myGeomGroupLine && !CORBA::is_nil(myGeomGroup)) {
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    GEOM::GEOM_IGroupOperations_var aGroupOp =
      SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());

    SMESH::ElementType aGroupType = SMESH::ALL;
    switch(aGroupOp->GetType(myGeomGroup)) {
    case 7: aGroupType = SMESH::NODE; break;
    case 6: aGroupType = SMESH::EDGE; break;
    case 4: aGroupType = SMESH::FACE; break;
    case 2: aGroupType = SMESH::VOLUME; break;
    default: myIsBusy = false; return;
    }

    if (aGroupType == aType) {
      _PTR(SObject) aGroupSO =
        //aStudy->FindObjectIOR(aStudy->ConvertObjectToIOR(myGeomGroup));
        aStudy->FindObjectID(myGeomGroup->GetStudyEntry());
      // Construct filter
      SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
      SMESH::Filter_var aFilter = aFilterMgr->CreateFilter();
      SMESH::BelongToGeom_var aBelongToGeom = aFilterMgr->CreateBelongToGeom();;
      aBelongToGeom->SetGeom(myGeomGroup);
      aBelongToGeom->SetShapeName(aGroupSO->GetName().c_str());
      aBelongToGeom->SetElementType(aType);
      aFilter->SetPredicate(aBelongToGeom);

      SMESH::long_array_var anElements = aFilter->GetElementsId(myMesh);

      int k = anElements->length();
      QListBoxItem* anItem = 0;
      for (int i = 0; i < k; i++) {
	QString aText = QString::number(anElements[i]);
	anItem = myElements->findItem(aText, Qt::ExactMatch);
	if (!anItem) {
	  anItem = new QListBoxText(aText);
	  myElements->insertItem(anItem);
	}
	myElements->setSelected(anItem, true);
      }
    }

    //VSR: mySelectGeomGroup->setChecked(false);
    myIsBusy = false;
    onListSelectionChanged();
  }
  myIsBusy = false;
  //  mySelectionMgr->clearSelected();
  updateButtons();
}

//=================================================================================
// function : onRemove()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onRemove()
{
  myIsBusy = true;
  if (myCurrentLineEdit == 0) {
    for (int i = myElements->count(); i > 0; i--) {
      if (myElements->isSelected(i-1)) {
	myElements->removeItem(i-1);
      }
    }
  } else {
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects( aList );

    int aNbSel = aList.Extent();

    if (aNbSel == 0) { myIsBusy = false; return; }

    SMESH::ElementType aType = SMESH::ALL;
    switch(myTypeId) {
    case 0: aType = SMESH::NODE; break;
    case 1: aType = SMESH::EDGE; break;
    case 2: aType = SMESH::FACE; break;
    case 3: aType = SMESH::VOLUME; break;
    }

    if (myCurrentLineEdit == mySubMeshLine) {
      //SALOME_ListIteratorOfListIO anIt (mySelectionMgr->StoredIObjects());
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects( aList );

      SALOME_ListIteratorOfListIO anIt (aList);
      for (; anIt.More(); anIt.Next()) {
	SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIt.Value());
	if (!aSubMesh->_is_nil()) {
	  // check if mesh is the same
	  if (aSubMesh->GetFather()->GetId() == myMesh->GetId()) {
	    if (aType == SMESH::NODE) {
	      try {
		SMESH::long_array_var anElements = aSubMesh->GetNodesId();
		int k = anElements->length();
		QListBoxItem* anItem = 0;
		for (int i = 0; i < k; i++) {
		  anItem = myElements->findItem(QString::number(anElements[i]), Qt::ExactMatch);
		  if (anItem) delete anItem;
		}
	      }
	      catch (const SALOME::SALOME_Exception& ex) {
		SalomeApp_Tools::QtCatchCorbaException(ex);
	      }
	    }
	    else {
	      try {
		SMESH::long_array_var anElements = aSubMesh->GetElementsId();
		int k = anElements->length();
		QListBoxItem* anItem = 0;
		for (int i = 0; i < k; i++) {
		  anItem = myElements->findItem(QString::number(anElements[i]), Qt::ExactMatch);
		  if (anItem) delete anItem;
		}
	      }
	      catch (const SALOME::SALOME_Exception& ex) {
		SalomeApp_Tools::QtCatchCorbaException(ex);
	      }
	    }
	  }
	}
      }
    }
    else if (myCurrentLineEdit == myGroupLine) {
      Standard_Boolean aRes;
      //SALOME_ListIteratorOfListIO anIt (mySelectionMgr->StoredIObjects());
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects( aList );

      SALOME_ListIteratorOfListIO anIt (aList);
      for (; anIt.More(); anIt.Next()) {
	SMESH::SMESH_Group_var aGroup = SMESH::IObjectToInterface<SMESH::SMESH_Group>(anIt.Value());
	if (aRes && !aGroup->_is_nil()) {
	  // check if mesh is the same
	  if (aGroup->GetType() == aType && aGroup->GetMesh()->GetId() == myMesh->GetId()) {
	    SMESH::long_array_var anElements = aGroup->GetListOfID();
	    int k = anElements->length();
	    QListBoxItem* anItem = 0;
	    for (int i = 0; i < k; i++) {
	      anItem = myElements->findItem(QString::number(anElements[i]), Qt::ExactMatch);
	      if (anItem) delete anItem;
	    }
	  }
	}
      }
    }
  }
  myIsBusy = false;
  updateButtons();
}

//=================================================================================
// function : onSort()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onSort()
{
  // PAL5412: sorts items in ascending by "string" value
  // myElements->sort(true);
  // myElements->update();
  int i, k = myElements->count();
  if (k > 0) {
    myIsBusy = true;
    QStringList aSelected;
    std::vector<int> anArray(k);
    //    QMemArray<int> anArray(k);
    QListBoxItem* anItem;
    // fill the array
    for (anItem = myElements->firstItem(), i = 0; anItem != 0; anItem = anItem->next(), i++) {
      anArray[i] = anItem->text().toInt();
      if (anItem->isSelected())
	aSelected.append(anItem->text());
    }
    // sort & update list
    std::sort(anArray.begin(), anArray.end());
    //    anArray.sort();
    myElements->clear();
    for (i = 0; i < k; i++) {
      myElements->insertItem(QString::number(anArray[i]));
    }
    for (QStringList::iterator it = aSelected.begin(); it != aSelected.end(); ++it) {
      anItem = myElements->findItem(*it, Qt::ExactMatch);
      if (anItem) myElements->setSelected(anItem, true);
    }
    myIsBusy = false;
  }
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::closeEvent (QCloseEvent*)
{
  onClose();
}

//=================================================================================
// function : SMESHGUI_GroupDlg::onClose
// purpose  : SLOT called when "Close" button pressed. Close dialog
//=================================================================================
void SMESHGUI_GroupDlg::onClose()
{
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
    SMESH::SetPickable();
  }

  mySelectionMgr->clearSelected();
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySelectionMgr->clearFilters();
  mySMESHGUI->ResetState();

  reject();
}

//=================================================================================
// function : SMESHGUI_GroupDlg::onDeactivate
// purpose  : SLOT called when dialog must be deativated
//=================================================================================
void SMESHGUI_GroupDlg::onDeactivate()
{
  mySMESHGUI->ResetState();
  setEnabled(false);
}

//=================================================================================
// function : SMESHGUI_GroupDlg::enterEvent
// purpose  : Event filter
//=================================================================================
void SMESHGUI_GroupDlg::enterEvent (QEvent*)
{
  if (!isEnabled()) {
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
    mySelectionMode = -1;
    setSelectionMode(myTypeId);
    //mySMESHGUI->SetActiveDialogBox((QDialog*)this);
    mySMESHGUI->SetActiveDialogBox(this);
    mySMESHGUI->SetState(800);
  }
}

//=================================================================================
// function : hideEvent
// purpose  : caused by ESC key
//=================================================================================
void SMESHGUI_GroupDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    onClose();
}
