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
//  File   : SMESHGUI_GroupDlg.cxx
//  Author : Natalia KOPNOVA
//  Module : SMESH
//  $Header$

#include "SMESHGUI_GroupDlg.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_ShapeByMeshDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include "SMESH_TypeFilter.hxx"
#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"

#include "GEOMBase.h"
#include "GEOM_SelectionFilter.h"

#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "SalomeApp_Tools.h"
#include "SalomeApp_Application.h"
#include "SalomeApp_Study.h"
#include "LightApp_Application.h"
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
#include <qcursor.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmemarray.h>
#include <qwidgetstack.h>
#include <qcolordialog.h>

#include <QtxIntSpinBox.h>

// STL includes
#include <vector>
#include <algorithm>
#include <set>

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
}

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
SMESHGUI_GroupDlg::SMESHGUI_GroupDlg( SMESHGUI* theModule, const char* name,
				      SMESH::SMESH_GroupBase_ptr theGroup, bool modal, WFlags fl)
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

  myShapeByMeshOp = 0;
  myGeomPopup = 0;
  myGeomObjects = new GEOM::ListOfGO();
  myGeomObjects->length(0);

  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  if (create) {
    setCaption(tr("SMESH_CREATE_GROUP_TITLE"));
    myHelpFileName = "creating_groups_page.html";
  }
  else {
    setCaption(tr("SMESH_EDIT_GROUP_TITLE"));
    myHelpFileName = "editing_groups_page.html";
  }

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
  myGeomGroupBtn = new QToolButton(wg2, "geometry group button");
  myGeomGroupBtn->setIconSet( QIconSet(image0) );
  myGeomGroupBtn->setToggleButton(true);
  myGeomGroupLine = new QLineEdit(wg2, "geometry group line");
  myGeomGroupLine->setReadOnly(true); //VSR ???
  onSelectGeomGroup(false);
  
  if (!create)
    {
      myGeomGroupBtn->setEnabled(false);
      myGeomGroupLine->setEnabled(false);
    }
   
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
  QGroupBox* aColorBox = new QGroupBox(2, Qt::Horizontal, this, "color box");
  aColorBox->setTitle(tr("SMESH_SET_COLOR"));

  new QLabel( tr("SMESH_CHECK_COLOR"), aColorBox, "color label" );
  myColorBtn = new QPushButton(aColorBox, "color button");
  myColorBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

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
  QPushButton* aHelpBtn = new QPushButton(aButtons, "help");
  aHelpBtn->setText(tr("SMESH_BUT_HELP"));
  aHelpBtn->setAutoDefault(true);

  aBtnLayout->addWidget(aOKBtn);
  aBtnLayout->addWidget(aApplyBtn);
  aBtnLayout->addStretch();
  aBtnLayout->addWidget(aCloseBtn);
  aBtnLayout->addWidget(aHelpBtn);

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
  aMainLayout->addMultiCellWidget(aColorBox,   6, 6, 0, 2);
  aMainLayout->addMultiCellWidget(aButtons,       7, 7, 0, 2);

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
  connect(myGeomGroupBtn, SIGNAL(toggled(bool)), this, SLOT(onGeomSelectionButton(bool)));
  connect(myColorBtn, SIGNAL(clicked()), this, SLOT(onSelectColor()));
  
  connect(aOKBtn, SIGNAL(clicked()), this, SLOT(onOK()));
  connect(aApplyBtn, SIGNAL(clicked()), this, SLOT(onApply()));
  connect(aCloseBtn, SIGNAL(clicked()), this, SLOT(onClose()));
  connect(aHelpBtn, SIGNAL(clicked()), this, SLOT(onHelp()));

  /* Init selection */
  mySMESHGUI->SetActiveDialogBox(this);
  mySMESHGUI->SetState(800);

  mySelectionMode = -1;
  myMeshFilter = new SMESH_TypeFilter(MESH);
  mySubMeshFilter = new SMESH_TypeFilter(SUBMESH);
  myGroupFilter = new SMESH_TypeFilter(GROUP);
  SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>( mySMESHGUI->application()->activeStudy() );
  myGeomFilter = new GEOM_SelectionFilter( aStudy, true );
  
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), this, SLOT(onClose()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(onObjectSelectionChanged()));

  myGrpTypeGroup->setButton(myGrpTypeGroup->id(rb1)); // VSR !!!
  onGrpTypeChanged(myGrpTypeGroup->id(rb1)); // VSR!!!

  if (myMesh->_is_nil() )
    myTypeGroup->setButton(0);

  updateButtons();
  //myName->setText(GetDefaultName(tr("SMESH_GROUP")));
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
// function : GetDefaultName()
// purpose  : Get the Group Name if Create new Group
//=================================================================================
QString SMESHGUI_GroupDlg::GetDefaultName(const QString& theOperation)
{
    QString aName = "";

    // collect all object names of SMESH component
    SalomeApp_Study* appStudy =
      dynamic_cast<SalomeApp_Study*>( SUIT_Session::session()->activeApplication()->activeStudy() );
    if ( !appStudy ) return aName;
    _PTR(Study) aStudy = appStudy->studyDS();

    std::set<std::string> aSet;
    _PTR(SComponent) aMeshCompo (aStudy->FindComponent("SMESH"));
    if (aMeshCompo) {
      _PTR(ChildIterator) it (aStudy->NewChildIterator(aMeshCompo));
      _PTR(SObject) obj;
      for (it->InitEx(true); it->More(); it->Next()) {
	obj = it->Value();
	aSet.insert(obj->GetName());
      }
    }

    // build a unique name
    int aNumber = 0;
    bool isUnique = false;
    while (!isUnique) {
      aName = theOperation + "_" + QString::number(++aNumber);
      isUnique = (aSet.count(aName.latin1()) == 0);
    }

    return aName;
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
  myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil();

  myActor = SMESH::FindActorByObject(myMesh);
  SMESH::SetPickable(myActor);

  setDefaultGroupColor();

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
void SMESHGUI_GroupDlg::init (SMESH::SMESH_GroupBase_ptr theGroup)
{
  myMesh = theGroup->GetMesh();
  
  myName->setText(theGroup->GetName());
  myName->home(false);
  myOldName = myName->text();

  SALOMEDS::Color aColor = theGroup->GetColor();
  setGroupColor( aColor );
  
  myMeshGroupLine->setText(theGroup->GetName());

  int aType = 0;
  switch(theGroup->GetType()) {
  case SMESH::NODE: aType= 0; break;
  case SMESH::EDGE: aType = 1; break;
  case SMESH::FACE: aType = 2; break;
  case SMESH::VOLUME: aType = 3; break;
  }
  myTypeGroup->setButton(aType);
  
  myGroup = SMESH::SMESH_Group::_narrow( theGroup );

  if ( !myGroup->_is_nil() )
    {
      myActor = SMESH::FindActorByObject(myMesh);
      if ( !myActor )
	myActor = SMESH::FindActorByObject(myGroup);
      SMESH::SetPickable(myActor);

      myGrpTypeGroup->setButton(0);
      onGrpTypeChanged(0);
      
      myCurrentLineEdit = 0;
      myElements->clear();
      setSelectionMode(aType);
      myTypeId = aType;
      
      myIdList.clear();
      if (!myGroup->IsEmpty()) {
	SMESH::long_array_var anElements = myGroup->GetListOfID();
	int k = anElements->length();
	for (int i = 0; i < k; i++) {
	  myIdList.append(anElements[i]);
	  myElements->insertItem(QString::number(anElements[i]));
	}
	myElements->selectAll(true);
      }
    }
  else
    {
      myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow( theGroup );
      
      if ( !myGroupOnGeom->_is_nil() )
	{
	  myActor = SMESH::FindActorByObject(myMesh);
	  if ( !myActor )
	    myActor = SMESH::FindActorByObject(myGroup);
	  SMESH::SetPickable(myActor);

	  myGrpTypeGroup->setButton(1);
	  onGrpTypeChanged(1);
	  
	  QString aShapeName("");
	  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
	  GEOM::GEOM_Object_var aGroupShape = myGroupOnGeom->GetShape();
	  if (!aGroupShape->_is_nil())
	    {
	      _PTR(SObject) aGroupShapeSO = aStudy->FindObjectID(aGroupShape->GetStudyEntry());
	      aShapeName = aGroupShapeSO->GetName().c_str();
	    }
	  myGeomGroupLine->setText( aShapeName );
	  myName->setText("Group On " + aShapeName);
	}
    }
}

//=================================================================================
// function : updateButtons()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::updateButtons()
{
  bool enable = !myName->text().stripWhiteSpace().isEmpty();

  if (myGrpTypeId == 0) {
    enable = enable && myElements->count() > 0;
    enable = enable && (!myGroup->_is_nil() || !myMesh->_is_nil());
  }
  else if (myGrpTypeId == 1) {
    if (CORBA::is_nil(myGroupOnGeom)) { // creation mode
      enable = enable && myGeomObjects->length() > 0 && !myMesh->_is_nil();
    }
  }

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
    if (id == 0)
      myName->setText(myOldName);
    onSelectGeomGroup(id == 1);
  }
  myGrpTypeId = id;
}

//=================================================================================
// function : onSelectColor()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onSelectColor()
{
  QColor color = getGroupQColor();
  color = QColorDialog::getColor( color );
  setGroupQColor( color );

  updateButtons();
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
    if (myActor)
      myActor->SetPointRepresentation(false);
    else
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
      else if (theMode == 7)
	mySelectionMgr->installFilter(myGeomFilter);
      
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

  if (myName->text().stripWhiteSpace().isEmpty())
    return false;

  if (myGrpTypeId == 0) { // on mesh elements
    if (!myElements->count())
      return false;

    mySelectionMgr->clearSelected();

    if (myGroup->_is_nil()) { // creation
      if (myMesh->_is_nil())
        return false;

      SMESH::ElementType aType = SMESH::ALL;
      switch (myTypeId) {
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

      SALOMEDS::Color aColor = getGroupColor();
      myGroup->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroup);

      //SMESH::setFileName ( aMeshGroupSO, QString::number(myColorSpinBox->value()) );
      SMESH::setFileType ( aMeshGroupSO, "COULEURGROUP" );

      /* init for next operation */
      myName->setText("");
      myElements->clear();
      myGroup = SMESH::SMESH_Group::_nil();

    } else { // edition
      myGroup->SetName(myName->text());
        
      SALOMEDS::Color aColor = getGroupColor();
      myGroup->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroup);
      if(SMESH_Actor *anActor = SMESH::FindActorByEntry(aMeshGroupSO->GetID().c_str()))
	anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B );

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
  }
  else if (myGrpTypeId == 1) { // on geom object
    if (CORBA::is_nil(myGroupOnGeom)) { // creation
      if (myMesh->_is_nil() || !myGeomObjects->length())
        return false;

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

      if (myGeomObjects->length() == 1) {
	myGroupOnGeom = myMesh->CreateGroupFromGEOM(aType, myName->text(),myGeomObjects[0]);
      }
      else {
        SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
        if ( aSMESHGen->_is_nil() )
          return false;

        // create a geometry group
        GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
        _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

        if (geomGen->_is_nil() || !aStudy)
          return false;

        GEOM::GEOM_IGroupOperations_var op =
          geomGen->GetIGroupOperations(aStudy->StudyId());
        if (op->_is_nil())
          return false;

        // check and add all selected GEOM objects: they must be
        // a sub-shapes of the main GEOM and must be of one type
        TopAbs_ShapeEnum aGroupType = TopAbs_SHAPE;
        for ( int i =0; i < myGeomObjects->length(); i++) {
          TopAbs_ShapeEnum aSubShapeType = (TopAbs_ShapeEnum)myGeomObjects[i]->GetShapeType();
          if (i == 0)
            aGroupType = aSubShapeType;
          else if (aSubShapeType != aGroupType) {
            aGroupType = TopAbs_SHAPE;
            break;
          }
        }

        GEOM::GEOM_Object_var aMeshShape = myMesh->GetShapeToMesh();
        GEOM::GEOM_Object_var aGroupVar = op->CreateGroup(aMeshShape, aGroupType);
        op->UnionList(aGroupVar, myGeomObjects);

        if (op->IsDone()) {
          // publish the GEOM group in study
          QString aNewGeomGroupName ("Auto_group_for_");
          aNewGeomGroupName += myName->text();
          SALOMEDS::SObject_var aNewGroupSO =
            geomGen->AddInStudy(aSMESHGen->GetCurrentStudy(), aGroupVar, aNewGeomGroupName, aMeshShape);
        }

        myGroupOnGeom = myMesh->CreateGroupFromGEOM(aType, myName->text(), aGroupVar);
      }

      SALOMEDS::Color aColor = getGroupColor();
      myGroupOnGeom->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroupOnGeom);

      //SMESH::setFileName ( aMeshGroupSO, QString::number(myColorSpinBox->value()) );
      SMESH::setFileType ( aMeshGroupSO,"COULEURGROUP" );

      /* init for next operation */
      myName->setText("");
      myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil();
    }
    else { // edition
      myGroupOnGeom->SetName(myName->text());

      SALOMEDS::Color aColor = getGroupColor();
      myGroupOnGeom->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroupOnGeom);
      if(SMESH_Actor *anActor = SMESH::FindActorByEntry(aMeshGroupSO->GetID().c_str()))
	anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B );
    }

    mySMESHGUI->updateObjBrowser(true);
    mySelectionMgr->clearSelected();
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
  if (myCurrentLineEdit == myGeomGroupLine && !myGeomGroupBtn->isOn()) return;

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
      myGeomObjects = new GEOM::ListOfGO();
      myGeomObjects->length(0);

      if (myGeomGroupBtn->isOn())
	myGeomGroupBtn->setOn(false);
      if (!myCreate)
        myName->setText("");

      myElements->clear();

      if (aNbSel != 1 ) {
        myGroup = SMESH::SMESH_Group::_nil();
	myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil(); 
        myMesh = SMESH::SMESH_Mesh::_nil();
	updateGeomPopup();
        updateButtons();
        myIsBusy = false;
        return;
      }
      Handle(SALOME_InteractiveObject) IO = aList.First();

      if (myCreate) {
        myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
	updateGeomPopup();
        if (myMesh->_is_nil())
	{
          updateButtons();
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
        SMESH::SMESH_GroupBase_var aGroup = SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);
        if (aGroup->_is_nil())
	{
	  myIsBusy = false;
          return;
	}
        myIsBusy = false;
        myCurrentLineEdit = 0;

	myGroup = SMESH::SMESH_Group::_nil();
	myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil();
	
        init(aGroup);
        myIsBusy = true;
        mySelectSubMesh->setEnabled(true);
        mySelectGroup->setEnabled(true);
      }
      myCurrentLineEdit = 0;
      myIsBusy = false;
      if (!myCreate)
        return;

      if (myGrpTypeId == 0)
	{
	  if (myTypeId == -1)
	    onTypeChanged(0);
	  else
	    {
	      myElements->clear();
	      setSelectionMode(myTypeId);
	    }
	}

      myIsBusy = false;
      return;

    } else if (myCurrentLineEdit == myGeomGroupLine) {

      myGeomObjects = new GEOM::ListOfGO();

      // The mesh SObject
      _PTR(SObject) aMeshSO = SMESH::FindSObject(myMesh);

      if (aNbSel == 0 || !aMeshSO)
	{
	  myGeomObjects->length(0);
          updateButtons();
	  myIsBusy = false;
	  return;
	}

      myGeomObjects->length(aNbSel);

      GEOM::GEOM_Object_var aGeomGroup;
      Standard_Boolean testResult;
      int i = 0;
      
      SALOME_ListIteratorOfListIO anIt (aList);
      for (; anIt.More(); anIt.Next()) {
	
	testResult = Standard_False;
	aGeomGroup = GEOMBase::ConvertIOinGEOMObject(anIt.Value(), testResult);

	// Check if the object is a geometry group
	if (!testResult || CORBA::is_nil(aGeomGroup))
	  continue;
	

	// Check if group constructed on the same shape as a mesh or on its child
	_PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
	GEOM::GEOM_IGroupOperations_var anOp =
	  SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());

	// The main shape of the group
	GEOM::GEOM_Object_var aGroupMainShape;
	if (aGeomGroup->GetType() == 37)
	  aGroupMainShape = anOp->GetMainShape(aGeomGroup);
	else
	  aGroupMainShape = GEOM::GEOM_Object::_duplicate(aGeomGroup);
	_PTR(SObject) aGroupMainShapeSO =
	  //aStudy->FindObjectIOR(aStudy->ConvertObjectToIOR(aGroupMainShape));
	  aStudy->FindObjectID(aGroupMainShape->GetStudyEntry());
	
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
	if (isRefOrSubShape)
	  myGeomObjects[i++] = aGeomGroup;
      }
      
      myGeomObjects->length(i);
      if ( i == 0 )
	{
	  myIsBusy = false;
	  return;
	}
      
      aNbSel = i;
    }

    if(aNbSel >= 1) {
      if(aNbSel > 1) {
	if(myCurrentLineEdit == mySubMeshLine)
	  aString = tr("SMESH_SUBMESH_SELECTED").arg(aNbSel);
	else if(myCurrentLineEdit == myGroupLine)
	  aString = tr("SMESH_GROUP_SELECTED").arg(aNbSel);
	else if(myCurrentLineEdit == myGeomGroupLine)
	  aString = tr("%1 Objects").arg(aNbSel);
      } else {
	aString = aList.First()->getName();
      }
    }

    myCurrentLineEdit->setText(aString);
    myOldName = myName->text();
    myName->setText(aString);
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
    else if(!myGroupOnGeom->_is_nil())
      myActor = SMESH::FindActorByObject(myGroupOnGeom);
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
    updateGeomPopup();
    setSelectionMode(8);
  }
  else {
    myGeomGroupBtn->setOn(false);
    myGeomObjects->length(0);
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

  } else if (myCurrentLineEdit == myGeomGroupLine && myGeomObjects->length() == 1) {
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    GEOM::GEOM_IGroupOperations_var aGroupOp =
      SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());

    SMESH::ElementType aGroupType = SMESH::ALL;
    switch(aGroupOp->GetType(myGeomObjects[0])) {
    case 7: aGroupType = SMESH::NODE; break;
    case 6: aGroupType = SMESH::EDGE; break;
    case 4: aGroupType = SMESH::FACE; break;
    case 2: aGroupType = SMESH::VOLUME; break;
    default: myIsBusy = false; return;
    }

    if (aGroupType == aType) {
      _PTR(SObject) aGroupSO =
        //aStudy->FindObjectIOR(aStudy->ConvertObjectToIOR(myGeomGroup));
        aStudy->FindObjectID(myGeomObjects[0]->GetStudyEntry());
      // Construct filter
      SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
      SMESH::Filter_var aFilter = aFilterMgr->CreateFilter();
      SMESH::BelongToGeom_var aBelongToGeom = aFilterMgr->CreateBelongToGeom();;
      aBelongToGeom->SetGeom(myGeomObjects[0]);
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
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onHelp()
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
  if (!isMinimized() && !myIsBusy)
    onClose();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Key_F1 )
    {
      e->accept();
      onHelp();
    }
}

//================================================================================
/*!
 * \brief Enable showing of the popup when Geometry selection btn is clicked
  * \param enable - true to enable
 */
//================================================================================

enum { DIRECT_GEOM_INDEX = 0, GEOM_BY_MESH_INDEX };

void SMESHGUI_GroupDlg::updateGeomPopup()
{
  bool enable = false;

  if ( !myMesh->_is_nil() )
    enable = myMesh->NbEdges() > 0;

  if ( myGeomGroupBtn )
  {
    disconnect( myGeomGroupBtn, SIGNAL( toggled(bool) ), this, SLOT( onGeomSelectionButton(bool) ));
    if ( enable ) {
      if ( !myGeomPopup ) {
        myGeomPopup = new QPopupMenu();
        myGeomPopup->insertItem( tr("DIRECT_GEOM_SELECTION"), DIRECT_GEOM_INDEX );
        myGeomPopup->insertItem( tr("GEOM_BY_MESH_ELEM_SELECTION"), GEOM_BY_MESH_INDEX );
        connect( myGeomPopup, SIGNAL( activated( int ) ), SLOT( onGeomPopup( int ) ) );
      }
      connect( myGeomGroupBtn, SIGNAL( toggled(bool) ), this, SLOT( onGeomSelectionButton(bool) ));
    }
  }
}


//=================================================================================
// function : onGeomSelectionButton()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onGeomSelectionButton(bool isBtnOn)
{
  if ( myGeomPopup && isBtnOn )
    {
      myCurrentLineEdit = myGeomGroupLine;
      int id = myGeomPopup->exec( QCursor::pos() );
      if (id == DIRECT_GEOM_INDEX || id == -1)
	setSelectionMode(7);
    }
  else if (!isBtnOn)
    {
      myCurrentLineEdit = 0;
      setSelectionMode(8);
    }
}

//=================================================================================
// function : onGeomPopup()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onGeomPopup( int index )
{
  if ( index == GEOM_BY_MESH_INDEX )
    {
      mySelectionMode = -1;
      if ( !myShapeByMeshOp ) {
	myShapeByMeshOp = new SMESHGUI_ShapeByMeshOp(true);
	connect(myShapeByMeshOp, SIGNAL(committed(SUIT_Operation*)),
		SLOT(onPublishShapeByMeshDlg(SUIT_Operation*)));
	connect(myShapeByMeshOp, SIGNAL(aborted(SUIT_Operation*)),
		SLOT(onCloseShapeByMeshDlg(SUIT_Operation*)));
      }
      // set mesh object to SMESHGUI_ShapeByMeshOp and start it
      if ( !myMesh->_is_nil() ) {
	myIsBusy = true;
        hide(); // stop processing selection
	myIsBusy = false;
        myShapeByMeshOp->setModule( mySMESHGUI );
        myShapeByMeshOp->setStudy( 0 ); // it's really necessary
        myShapeByMeshOp->SetMesh( myMesh );
        myShapeByMeshOp->start();
      }
    }
}

//================================================================================
/*!
 * \brief SLOT. Is called when Ok is pressed in SMESHGUI_ShapeByMeshDlg
 */
//================================================================================

void SMESHGUI_GroupDlg::onPublishShapeByMeshDlg(SUIT_Operation* op)
{
  if ( myShapeByMeshOp == op ) {
    mySMESHGUI->getApp()->updateObjectBrowser();
    show();
    // Select a found geometry object
    GEOM::GEOM_Object_var aGeomVar = myShapeByMeshOp->GetShape();
    if ( !aGeomVar->_is_nil() )
    {
      QString ID = aGeomVar->GetStudyEntry();
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
      if ( _PTR(SObject) aGeomSO = aStudy->FindObjectID( ID.latin1() )) {
	SALOME_ListIO anIOList;
	Handle(SALOME_InteractiveObject) anIO = new SALOME_InteractiveObject
	  ( aGeomSO->GetID().c_str(), "SMESH", aGeomSO->GetName().c_str() );
	anIOList.Append( anIO );
      	mySelectionMgr->setSelectedObjects( anIOList, false );
	onObjectSelectionChanged();
      }
    }
  }
}

//================================================================================
/*!
 * \brief SLOT. Is called when Close is pressed in SMESHGUI_ShapeByMeshDlg
 */
//================================================================================

void SMESHGUI_GroupDlg::onCloseShapeByMeshDlg(SUIT_Operation* op)
{
  if ( myShapeByMeshOp == op )
    {
      show();
      setSelectionMode(7);
    }
}

//=================================================================================
// function : setGroupColor()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::setGroupColor( const SALOMEDS::Color& theColor )
{
  QColor aQColor( (int)( theColor.R * 255.0 ),
		  (int)( theColor.G * 255.0 ),
		  (int)( theColor.B * 255.0 ) );
  setGroupQColor( aQColor );
}

//=================================================================================
// function : getGroupColor()
// purpose  :
//=================================================================================
SALOMEDS::Color SMESHGUI_GroupDlg::getGroupColor() const
{
  QColor aQColor = getGroupQColor();

  SALOMEDS::Color aColor;
  aColor.R = (float)aQColor.red() / 255.0;
  aColor.G = (float)aQColor.green() / 255.0;
  aColor.B = (float)aQColor.blue() / 255.0;

  return aColor;
}

//=================================================================================
// function : setGroupQColor()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::setGroupQColor( const QColor& theColor )
{
  if( theColor.isValid() )
  {
    QPalette pal = myColorBtn->palette();
    pal.setColor(QColorGroup::Button, theColor);
    myColorBtn->setPalette(pal);
  }
}

//=================================================================================
// function : getGroupQColor()
// purpose  :
//=================================================================================
QColor SMESHGUI_GroupDlg::getGroupQColor() const
{
  QColor aColor = myColorBtn->palette().active().button();
  return aColor;
}

//=================================================================================
// function : setDefaultGroupColor()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::setDefaultGroupColor()
{
  if( myMesh->_is_nil() )
    return;

  bool isAutoColor = myMesh->GetAutoColor();

  QColor aQColor;
  if( !isAutoColor )
  {
    int r = 0, g = 0, b = 0;
    SMESH::GetColor( "SMESH", "fill_color", r, g, b, QColor( 0, 170, 255 ) );
    aQColor.setRgb( r, g, b );
  }
  else
  {
    SMESH::ListOfGroups aListOfGroups = *myMesh->GetGroups();

    QValueList<SALOMEDS::Color> aReservedColors;
    for( int i = 0, n = aListOfGroups.length(); i < n; i++ )
    {
      SMESH::SMESH_GroupBase_var aGroupObject = aListOfGroups[i];
      SALOMEDS::Color aReservedColor = aGroupObject->GetColor();
      aReservedColors.append( aReservedColor );
    }

    SALOMEDS::Color aColor = SMESHGUI::getUniqueColor( aReservedColors );
    aQColor.setRgb( (int)( aColor.R * 255.0 ),
		    (int)( aColor.G * 255.0 ),
		    (int)( aColor.B * 255.0 ) );

  }

  setGroupQColor( aQColor );
}
