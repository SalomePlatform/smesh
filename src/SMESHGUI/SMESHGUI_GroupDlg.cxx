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
//  SMESH SMESHGUI : GUI for SMESH component
//  File   : SMESHGUI_GroupDlg.cxx
//  Author : Natalia KOPNOVA, Open CASCADE S.A.S.
//  SMESH includes

#include "SMESHGUI_GroupDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_ShapeByMeshDlg.h"

#include <SMESH_TypeFilter.hxx>
#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>

// SALOME GEOM includes
#include <GEOMBase.h>
#include <GEOM_SelectionFilter.h>

// SALOME GUI includes
#include <QtxColorButton.h>

#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <SalomeApp_Tools.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_Study.h>
#include <LightApp_SelectionMgr.h>

#include <SALOME_ListIO.hxx>
#include <SALOME_ListIteratorOfListIO.hxx>

#include <SVTK_ViewWindow.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>

// VTK Includes
#include <vtkRenderer.h>
#include <vtkActorCollection.h>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QKeyEvent>
#include <QMenu>

// STL includes
#include <vector>
#include <algorithm>
#include <set>

#define SPACING 6
#define MARGIN  11

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
SMESHGUI_GroupDlg::SMESHGUI_GroupDlg( SMESHGUI* theModule,
				      SMESH::SMESH_Mesh_ptr theMesh )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    mySelector( SMESH::GetViewWindow( theModule )->GetSelector() ),
    myIsBusy( false ),
    myNameChanged( false ),
    myActor( 0 )
{
  initDialog( true );
  if ( !theMesh->_is_nil() )
    init( theMesh );
  else
  {
    mySelectSubMesh->setEnabled( false );
    mySelectGroup->setEnabled( false );
    myGeomGroupBtn->setEnabled( false );
    myGeomGroupLine->setEnabled( false );
  }
}

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
SMESHGUI_GroupDlg::SMESHGUI_GroupDlg( SMESHGUI* theModule,
				      SMESH::SMESH_GroupBase_ptr theGroup,
                                      const bool theIsConvert )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    mySelector( SMESH::GetViewWindow( theModule )->GetSelector() ),
    myIsBusy( false ),
    myNameChanged( false )
{
  initDialog( false );
  if ( !theGroup->_is_nil() )
    init( theGroup, theIsConvert );
  else
  {
    mySelectSubMesh->setEnabled( false );
    mySelectGroup->setEnabled( false );

    myCurrentLineEdit = myMeshGroupLine;
    setSelectionMode( 5 );
  }
}

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::initDialog( bool create)
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );

  myFilterDlg = 0;
  myCreate = create;
  myCurrentLineEdit = 0;

  myShapeByMeshOp = 0;
  myGeomPopup = 0;
  myGeomObjects = new GEOM::ListOfGO();
  myGeomObjects->length( 0 );

  QPixmap image0( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", tr( "ICON_SELECT" ) ) );

  setWindowTitle( create ? tr( "SMESH_CREATE_GROUP_TITLE" ) : tr( "SMESH_EDIT_GROUP_TITLE" ) );
  myHelpFileName = create ? "creating_groups_page.html" : "editing_groups_page.html";

  setSizeGripEnabled( true);

  QGridLayout* aMainLayout = new QGridLayout( this );
  aMainLayout->setMargin( MARGIN );
  aMainLayout->setSpacing( SPACING );

  /***************************************************************/
  QLabel* meshGroupLab = new QLabel( create ? tr( "SMESH_MESH" ) : tr( "SMESH_GROUP" ), this );
  myMeshGroupBtn = new QPushButton( this );
  myMeshGroupBtn->setIcon( image0 );
  myMeshGroupLine = new QLineEdit( this );
  myMeshGroupLine->setReadOnly( true );

  /***************************************************************/
  QGroupBox* aTypeBox = new QGroupBox( tr( "SMESH_ELEMENTS_TYPE" ), this );
  myTypeGroup = new QButtonGroup( this );
  QHBoxLayout* aTypeBoxLayout = new QHBoxLayout( aTypeBox );
  aTypeBoxLayout->setMargin( MARGIN );
  aTypeBoxLayout->setSpacing( SPACING );

  QStringList types;
  types.append( tr( "MESH_NODE" ) );
  types.append( tr( "SMESH_EDGE" ) );
  types.append( tr( "SMESH_FACE" ) );
  types.append( tr( "SMESH_VOLUME" ) );
  QRadioButton* rb;
  for ( int i = 0; i < types.count(); i++ )
  {
    rb = new QRadioButton( types[i], aTypeBox );
    myTypeGroup->addButton( rb, i );
    aTypeBoxLayout->addWidget( rb );
  }
  aTypeBox->setEnabled( create );
  myTypeId = -1;

  /***************************************************************/
  QLabel* aName = new QLabel( tr( "SMESH_NAME" ), this );
  aName->setMinimumWidth( 50 );
  myName = new QLineEdit( this );

  /***************************************************************/
  QGroupBox* aGrpTypeBox = new QGroupBox( tr( "SMESH_GROUP_TYPE" ), this );
  myGrpTypeGroup = new QButtonGroup( this );
  QHBoxLayout* aGrpTypeBoxLayout = new QHBoxLayout( aGrpTypeBox );
  aGrpTypeBoxLayout->setMargin( MARGIN );
  aGrpTypeBoxLayout->setSpacing( SPACING );

  QRadioButton* rb1 = new QRadioButton( tr( "SMESH_GROUP_STANDALONE" ), aGrpTypeBox );
  QRadioButton* rb2 = new QRadioButton( tr( "SMESH_GROUP_GEOMETRY" ),   aGrpTypeBox );
  myGrpTypeGroup->addButton( rb1, 0 );
  myGrpTypeGroup->addButton( rb2, 1 );
  aGrpTypeBoxLayout->addWidget( rb1 );
  aGrpTypeBoxLayout->addWidget( rb2 );
  aGrpTypeBox->setEnabled( create );
  myGrpTypeId = -1;

  /***************************************************************/
  myWGStack = new QStackedWidget( this );
  QWidget* wg1 = new QWidget( myWGStack );
  QWidget* wg2 = new QWidget( myWGStack );

  /***************************************************************/
  QGroupBox* aContentBox = new QGroupBox( tr( "SMESH_CONTENT" ), wg1 );
  QGridLayout* aContentBoxLayout = new QGridLayout( aContentBox );
  aContentBoxLayout->setMargin( MARGIN );
  aContentBoxLayout->setSpacing( SPACING );

  QLabel* aLabel = new QLabel( tr( "SMESH_ID_ELEMENTS" ), aContentBox );
  myElements = new QListWidget( aContentBox );
  myElements->setSelectionMode( QListWidget::ExtendedSelection );

  myFilter = new QPushButton( tr( "SMESH_BUT_FILTER" ), aContentBox );
  QPushButton* aAddBtn = new QPushButton( tr( "SMESH_BUT_ADD" ), aContentBox );
  QPushButton* aRemoveBtn = new QPushButton( tr( "SMESH_BUT_REMOVE" ), aContentBox );
  QPushButton* aSortBtn = new QPushButton( tr( "SMESH_BUT_SORT" ), aContentBox );

  aContentBoxLayout->addWidget( aLabel,     0, 0 );
  aContentBoxLayout->addWidget( myElements, 1, 0, 6, 1 );
  aContentBoxLayout->addWidget( myFilter,   1, 1 );
  aContentBoxLayout->addWidget( aAddBtn,    3, 1 );
  aContentBoxLayout->addWidget( aRemoveBtn, 4, 1 );
  aContentBoxLayout->addWidget( aSortBtn,   6, 1 );

  aContentBoxLayout->setColumnStretch( 0, 1 );
  aContentBoxLayout->setRowStretch( 2, 1 );
  aContentBoxLayout->setRowStretch( 5, 1 );

  /***************************************************************/
  QGroupBox* aSelectBox = new QGroupBox( tr( "SMESH_SELECT_FROM" ), wg1 );
  QGridLayout* aSelectBoxLayout = new QGridLayout( aSelectBox );
  aSelectBoxLayout->setMargin( MARGIN );
  aSelectBoxLayout->setSpacing( SPACING );

  mySelectSubMesh = new QCheckBox( tr( "SMESH_SUBMESH" ), aSelectBox );
  mySubMeshBtn = new QPushButton( aSelectBox );
  mySubMeshBtn->setIcon( image0 );
  mySubMeshLine = new QLineEdit( aSelectBox );
  mySubMeshLine->setReadOnly( true );
  onSelectSubMesh( false );

  mySelectGroup = new QCheckBox( tr( "SMESH_GROUP" ), aSelectBox );
  myGroupBtn = new QPushButton( aSelectBox );
  myGroupBtn->setIcon( image0 );
  myGroupLine = new QLineEdit( aSelectBox );
  myGroupLine->setReadOnly( true );
  onSelectGroup( false );

  aSelectBoxLayout->addWidget( mySelectSubMesh, 0, 0 );
  aSelectBoxLayout->addWidget( mySubMeshBtn,    0, 1 );
  aSelectBoxLayout->addWidget( mySubMeshLine,   0, 2 );
  aSelectBoxLayout->addWidget( mySelectGroup,   1, 0 );
  aSelectBoxLayout->addWidget( myGroupBtn,      1, 1 );
  aSelectBoxLayout->addWidget( myGroupLine,     1, 2 );

  /***************************************************************/
  QVBoxLayout* wg1Layout = new QVBoxLayout( wg1 );
  wg1Layout->setMargin( 0 );
  wg1Layout->setSpacing( SPACING );
  wg1Layout->addWidget( aContentBox );
  wg1Layout->addWidget( aSelectBox );
  wg1Layout->setStretchFactor( aContentBox, 10 );

  /***************************************************************/
  QLabel* geomObject = new QLabel( tr( "SMESH_OBJECT_GEOM" ), wg2 );
  myGeomGroupBtn = new QToolButton( wg2 );
  myGeomGroupBtn->setIcon( image0 );
  myGeomGroupBtn->setCheckable( true );
  myGeomGroupLine = new QLineEdit( wg2 );
  myGeomGroupLine->setReadOnly( true ); //VSR ???
  onSelectGeomGroup( false );

  myGeomGroupBtn->setEnabled( create );
  myGeomGroupLine->setEnabled( create );

  /***************************************************************/
  QGridLayout* wg2Layout = new QGridLayout( wg2 );
  wg2Layout->setMargin( 0 );
  wg1Layout->setSpacing( SPACING );
  wg2Layout->addWidget( geomObject,     0, 0 );
  wg2Layout->addWidget( myGeomGroupBtn, 0, 1 );
  wg2Layout->addWidget( myGeomGroupLine,0, 2 );
  wg2Layout->setRowStretch( 1, 5 );

  /***************************************************************/
  myWGStack->insertWidget( 0, wg1 );
  myWGStack->insertWidget( 1, wg2 );

  /***************************************************************/
  QGroupBox* aColorBox = new QGroupBox(tr( "SMESH_SET_COLOR" ), this);
  QHBoxLayout* aColorBoxLayout = new QHBoxLayout(aColorBox);
  aColorBoxLayout->setMargin(MARGIN);
  aColorBoxLayout->setSpacing(SPACING);

  QLabel* aColorLab = new QLabel(tr( "SMESH_CHECK_COLOR" ), aColorBox );
  myColorBtn = new QtxColorButton(aColorBox);
  myColorBtn->setSizePolicy( QSizePolicy::MinimumExpanding, 
			     myColorBtn->sizePolicy().verticalPolicy() );

  aColorBoxLayout->addWidget(aColorLab);
  aColorBoxLayout->addWidget(myColorBtn);

  /***************************************************************/

  QFrame* aButtons = new QFrame(this);
  aButtons->setFrameStyle( QFrame::Box | QFrame::Sunken );
  QHBoxLayout* aBtnLayout = new QHBoxLayout(aButtons);
  aBtnLayout->setMargin(MARGIN);
  aBtnLayout->setSpacing(SPACING);

  myOKBtn = new QPushButton(tr( "SMESH_BUT_APPLY_AND_CLOSE" ), aButtons);
  myOKBtn->setAutoDefault(true);
  myOKBtn->setDefault(true);
  myApplyBtn = new QPushButton(tr( "SMESH_BUT_APPLY" ), aButtons);
  myApplyBtn->setAutoDefault(true);
  myCloseBtn = new QPushButton(tr( "SMESH_BUT_CLOSE" ), aButtons);
  myCloseBtn->setAutoDefault(true);
  myHelpBtn = new QPushButton(tr( "SMESH_BUT_HELP" ), aButtons);
  myHelpBtn->setAutoDefault(true);

  aBtnLayout->addWidget(myOKBtn);
  aBtnLayout->addSpacing(10);
  aBtnLayout->addWidget(myApplyBtn);
  aBtnLayout->addSpacing(10);
  aBtnLayout->addStretch();
  aBtnLayout->addWidget(myCloseBtn);
  aBtnLayout->addWidget(myHelpBtn);

  /***************************************************************/
  aMainLayout->addWidget(meshGroupLab,    0, 0);
  aMainLayout->addWidget(myMeshGroupBtn,  0, 1);
  aMainLayout->addWidget(myMeshGroupLine, 0, 2);
  aMainLayout->addWidget(aTypeBox,        1, 0, 1, 3);
  aMainLayout->addWidget(aName,           2, 0);
  aMainLayout->addWidget(myName,          2, 2);
  aMainLayout->addWidget(aGrpTypeBox,     3, 0, 1, 3);
  aMainLayout->addWidget(myWGStack,       4, 0, 1, 3);
  aMainLayout->addWidget(aColorBox,       5, 0, 1, 3);
  aMainLayout->addWidget(aButtons,        6, 0, 1, 3);

  /* signals and slots connections */
  connect(myMeshGroupBtn, SIGNAL(clicked()),          this, SLOT(setCurrentSelection()));
  connect(myGrpTypeGroup, SIGNAL(buttonClicked(int)), this, SLOT(onGrpTypeChanged(int)));
  connect(myTypeGroup,    SIGNAL(buttonClicked(int)), this, SLOT(onTypeChanged(int)));

  connect(myName,     SIGNAL(textChanged(const QString&)), this, SLOT(onNameChanged(const QString&)));
  connect(myElements, SIGNAL(itemSelectionChanged()),      this, SLOT(onListSelectionChanged()));

  connect(myFilter,   SIGNAL(clicked()), this, SLOT(setFilters()));
  connect(aAddBtn,    SIGNAL(clicked()), this, SLOT(onAdd()));
  connect(aRemoveBtn, SIGNAL(clicked()), this, SLOT(onRemove()));
  connect(aSortBtn,   SIGNAL(clicked()), this, SLOT(onSort()));

  connect(mySelectSubMesh, SIGNAL(toggled(bool)), this, SLOT(onSelectSubMesh(bool)));
  connect(mySelectGroup,   SIGNAL(toggled(bool)), this, SLOT(onSelectGroup(bool)));
  connect(mySubMeshBtn,    SIGNAL(clicked()), this, SLOT(setCurrentSelection()));
  connect(myGroupBtn,      SIGNAL(clicked()), this, SLOT(setCurrentSelection()));
  connect(myGeomGroupBtn,  SIGNAL(toggled(bool)), this, SLOT(onGeomSelectionButton(bool)));

  connect(myColorBtn, SIGNAL(changed( QColor )), this, SLOT(onColorChanged( QColor )));

  connect(myOKBtn,    SIGNAL(clicked()), this, SLOT(onOK()));
  connect(myApplyBtn, SIGNAL(clicked()), this, SLOT(onApply()));
  connect(myCloseBtn, SIGNAL(clicked()), this, SLOT(onClose()));
  connect(myHelpBtn,  SIGNAL(clicked()), this, SLOT(onHelp()));

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
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),        this, SLOT(onClose()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),  this, SLOT(onObjectSelectionChanged()));

  rb1->setChecked(true); // VSR !!!
  onGrpTypeChanged(0); // VSR!!!

  if (myMesh->_is_nil() )
    myTypeGroup->button(0)->setChecked(true);

  updateButtons();
  //myName->setText(GetDefaultName(tr( "SMESH_GROUP" )));
}

//=================================================================================
// function : ~SMESHGUI_GroupDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_GroupDlg::~SMESHGUI_GroupDlg()
{
  // no need to delete child widgets, Qt does it all for us
  if ( myFilterDlg != 0 ) {
    myFilterDlg->setParent( 0 );
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
  _PTR(SComponent) aMeshCompo (aStudy->FindComponent( "SMESH" ));
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
    isUnique = (aSet.count(aName.toLatin1().data()) == 0);
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
  restoreShowEntityMode();
  myMesh = SMESH::SMESH_Mesh::_duplicate(theMesh);
  setShowEntityMode();
  myGroup = SMESH::SMESH_Group::_nil();
  myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil();

  // NPAL19389: create a group with a selection in another group
  // set actor of myMesh, if it is visible, else try
  // any visible actor of group or submesh of myMesh
  SetAppropriateActor();

  setDefaultGroupColor();

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );
  if( !aList.IsEmpty() )
  {
    QString aName = aList.First()->getName();
    myMeshGroupLine->setText(aName);
    myMeshGroupLine->home( false );
  }

  myCurrentLineEdit = 0;

  myTypeGroup->button(0)->setChecked(true);
  onTypeChanged(0);
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::init (SMESH::SMESH_GroupBase_ptr theGroup,
                              const bool theIsConvert)
{
  restoreShowEntityMode();
  myMesh = theGroup->GetMesh();
  setShowEntityMode();

  myNameChanged = true;
  myName->blockSignals(true);
  myName->setText(theGroup->GetName());
  myName->blockSignals(false);
  myName->home(false);

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
  myTypeGroup->button(aType)->setChecked(true);

  myGroup = SMESH::SMESH_Group::_narrow( theGroup );
  myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow( theGroup );

  if (myGroup->_is_nil() && myGroupOnGeom->_is_nil())
    return;

  // NPAL19389: create a group with a selection in another group
  // set actor of myMesh, if it is visible, else set
  // actor of theGroup, if it is visible, else try
  // any visible actor of group or submesh of myMesh
  // commented, because an attempt to set selection on not displayed cells leads to error
  //SetAppropriateActor();
  myActor = SMESH::FindActorByObject(myMesh);
  if ( !myActor )
    myActor = SMESH::FindActorByObject(theGroup);
  SMESH::SetPickable(myActor);

  int grpType = (!myGroup->_is_nil() ? 0 : (theIsConvert ? 0 : 1));
  myGrpTypeGroup->button(grpType)->setChecked(true);
  onGrpTypeChanged(grpType);

  if ( grpType == 0 ) {
    myCurrentLineEdit = 0;
    myElements->clear();
    setSelectionMode(aType);
    myTypeId = aType;

    setShowEntityMode(); // depends on myTypeId

    myIdList.clear();
    if (!theGroup->IsEmpty()) {
      SMESH::long_array_var anElements = theGroup->GetListOfID();
      int k = anElements->length();
      for (int i = 0; i < k; i++) {
        myIdList.append(anElements[i]);
        myElements->addItem(QString::number(anElements[i]));
      }
      myElements->selectAll();
    }
  }
  else
  {
    QString aShapeName( "" );
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    GEOM::GEOM_Object_var aGroupShape = myGroupOnGeom->GetShape();
    if (!aGroupShape->_is_nil())
    {
      _PTR(SObject) aGroupShapeSO = aStudy->FindObjectID(aGroupShape->GetStudyEntry());
      aShapeName = aGroupShapeSO->GetName().c_str();
    }
    myGeomGroupLine->setText( aShapeName );
    myNameChanged = true;
    myName->blockSignals(true);
    myName->setText( "Group On " + aShapeName);
    myName->blockSignals(false);
  }
  updateButtons();
}

//=================================================================================
// function : updateButtons()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::updateButtons()
{
  bool enable = !myName->text().trimmed().isEmpty();

  if (myGrpTypeId == 0) {
    enable = enable && myElements->count() > 0;
    enable = enable && (!myGroup->_is_nil() || !myMesh->_is_nil());
  }
  else if (myGrpTypeId == 1) {
    if (CORBA::is_nil(myGroupOnGeom)) { // creation mode
      enable = enable && myGeomObjects->length() > 0 && !myMesh->_is_nil();
    }
  }

  myOKBtn->setEnabled(enable);
  myApplyBtn->setEnabled(enable);
}

//=================================================================================
// function : onNameChanged()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onNameChanged (const QString& text)
{
  myOldName = myName->text();
  updateButtons();
  myNameChanged = !myName->text().trimmed().isEmpty();
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
    myTypeId = id;
    setShowEntityMode();
  }
}

//=================================================================================
// function : onGrpTypeChanged()
// purpose  : Group type radio button management
//=================================================================================
void SMESHGUI_GroupDlg::onGrpTypeChanged (int id)
{
  if (myGrpTypeId != id) {
    myWGStack->setCurrentIndex( id );
    myName->blockSignals(true);
    myName->setText(myOldName);
    myName->blockSignals(false);
    onSelectGeomGroup(id == 1);
  }
  myGrpTypeId = id;
}

//=================================================================================
// function : onColorChanged()
// purpose  : Color button management
//=================================================================================
void SMESHGUI_GroupDlg::onColorChanged(QColor theColor)
{
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

  if (myName->text().trimmed().isEmpty())
    return false;

  if (myGrpTypeId == 0) { // on mesh elements
    if (!myElements->count())
      return false;

    mySelectionMgr->clearSelected();

    if (myGroup->_is_nil()) { // creation or conversion
      // check if group on geometry is not null
      if (!CORBA::is_nil(myGroupOnGeom)) {
        if (myMesh->_is_nil())
          return false;
        myGroup = myMesh->ConvertToStandalone( myGroupOnGeom );
	// nullify pointer, because object become dead
        myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil();
      }
    }

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
      for (i = 0; i < k; i++) {
	anIdList[i] = myElements->item(i)->text().toInt();
      }

      myGroup = SMESH::AddGroup(myMesh, aType, myName->text());
      myGroup->Add(anIdList.inout());

      SALOMEDS::Color aColor = getGroupColor();
      myGroup->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroup);

      //SMESH::setFileName ( aMeshGroupSO, QString::number(myColorSpinBox->value()) );
      SMESH::setFileType ( aMeshGroupSO, "COULEURGROUP" );

      /* init for next operation */
      myName->setText( "" );
      myElements->clear();
      myGroup = SMESH::SMESH_Group::_nil();

    } else { // edition
      myGroup->SetName(myName->text().toLatin1().data());

      SALOMEDS::Color aColor = getGroupColor();
      myGroup->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroup);
      if(SMESH_Actor *anActor = SMESH::FindActorByEntry(aMeshGroupSO->GetID().c_str())) {
	switch ( myTypeId ) {
	case 0: anActor->SetNodeColor( aColor.R, aColor.G, aColor.B ); break;
	case 1: anActor->SetEdgeColor( aColor.R, aColor.G, aColor.B ); break;
	case 2:
	case 3: anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B ); break;
	}
      }

      QList<int> aAddList;

      int i, total = myElements->count();
      for (i = 0; i < total; i++) {
	int anId = myElements->item(i)->text().toInt();
	int idx = myIdList.indexOf(anId);
	if ( idx == -1 )
	  aAddList.append(anId);
	else
	  myIdList.removeAt(idx);
      }
      if (!aAddList.empty()) {
	SMESH::long_array_var anIdList = new SMESH::long_array;
	int added = aAddList.count();
	anIdList->length(added);
	for (i = 0; i < added; i++)
	  anIdList[i] = aAddList[i];
	myGroup->Add(anIdList.inout());
      }
      if (!myIdList.empty()) {
	SMESH::long_array_var anIdList = new SMESH::long_array;
	int removed = myIdList.count();
	anIdList->length(removed);
	for (i = 0; i < removed; i++)
	  anIdList[i] = myIdList[i];
	myGroup->Remove(anIdList.inout());
      }
      /* init for next operation */
      myIdList.clear();
      for (i = 0; i < total; i++) {
	myIdList.append(myElements->item(i)->text().toInt());
      }
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
	myGroupOnGeom = myMesh->CreateGroupFromGEOM(aType,
						    myName->text().toLatin1().data(),
						    myGeomObjects[0]);
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
          QString aNewGeomGroupName ( "Auto_group_for_" );
          aNewGeomGroupName += myName->text();
          SALOMEDS::SObject_var aNewGroupSO =
            geomGen->AddInStudy(aSMESHGen->GetCurrentStudy(), aGroupVar,
				aNewGeomGroupName.toLatin1().data(), aMeshShape);
        }

        myGroupOnGeom = myMesh->CreateGroupFromGEOM(aType,
						    myName->text().toLatin1().data(),
						    aGroupVar);
      }

      SALOMEDS::Color aColor = getGroupColor();
      myGroupOnGeom->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroupOnGeom);

      //SMESH::setFileName ( aMeshGroupSO, QString::number(myColorSpinBox->value()) );
      SMESH::setFileType ( aMeshGroupSO,"COULEURGROUP" );

      /* init for next operation */
      myName->setText( "" );
      myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil();
    }
    else { // edition
      myGroupOnGeom->SetName(myName->text().toLatin1().data());

      SALOMEDS::Color aColor = getGroupColor();
      myGroupOnGeom->SetColor(aColor);

      _PTR(SObject) aMeshGroupSO = SMESH::FindSObject(myGroupOnGeom);
      if(SMESH_Actor *anActor = SMESH::FindActorByEntry(aMeshGroupSO->GetID().c_str())) {
	switch ( myTypeId ) {
	case 0: anActor->SetNodeColor( aColor.R, aColor.G, aColor.B ); break;
	case 1: anActor->SetEdgeColor( aColor.R, aColor.G, aColor.B ); break;
	case 2:
	case 3: anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B ); break;
	}
      }
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
  //  MESSAGE( "SMESHGUI_GroupDlg::onListSelectionChanged(); myActor = " << myActor);
  if( myIsBusy || !myActor) return;
    myIsBusy = true;

  if (myCurrentLineEdit == 0) {
    mySelectionMgr->clearSelected();
    TColStd_MapOfInteger aIndexes;
    QList<QListWidgetItem*> selItems = myElements->selectedItems();
    QListWidgetItem* anItem;
    foreach(anItem, selItems) aIndexes.Add(anItem->text().toInt());
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
  if (myCurrentLineEdit == myGeomGroupLine && !myGeomGroupBtn->isChecked()) return;

  myIsBusy = true;

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );

  int aNbSel = aList.Extent();
  myElements->clearSelection();

  if (myCurrentLineEdit)
  {
    myCurrentLineEdit->setText( "" );
    QString aString = "";

    if (myCurrentLineEdit == myMeshGroupLine)
    {
      mySelectSubMesh->setEnabled(false);
      mySelectGroup->setEnabled(false);
      myGroupLine->setText( "" );
      mySubMeshLine->setText( "" );

      myGeomGroupBtn->setEnabled(false);
      myGeomGroupLine->setEnabled(false);
      myGeomGroupLine->setText( "" );
      myGeomObjects = new GEOM::ListOfGO();
      myGeomObjects->length(0);

      if (myGeomGroupBtn->isChecked())
	myGeomGroupBtn->setChecked(false);
      if (!myCreate)
        myName->setText( "" );

      myElements->clear();

      if (aNbSel != 1 ) {
        myGroup = SMESH::SMESH_Group::_nil();
        myGroupOnGeom = SMESH::SMESH_GroupOnGeom::_nil();
        restoreShowEntityMode();
        myMesh = SMESH::SMESH_Mesh::_nil();
        updateGeomPopup();
        updateButtons();
        myIsBusy = false;
        return;
      }
      Handle(SALOME_InteractiveObject) IO = aList.First();

      if (myCreate) {
	restoreShowEntityMode();
	myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
        setShowEntityMode();
        updateGeomPopup();
        if (myMesh->_is_nil())
	{
          updateButtons();
	  myIsBusy = false;
	  return;
	}
        myGroup = SMESH::SMESH_Group::_nil();

        // NPAL19389: create a group with a selection in another group
        // set actor of myMesh, if it is visible, else try
        // any visible actor of group or submesh of myMesh
        SetAppropriateActor();

        aString = aList.First()->getName();
        myMeshGroupLine->setText(aString);
        myMeshGroupLine->home( false );

        mySelectSubMesh->setEnabled(true);
        mySelectGroup->setEnabled(true);
        myGeomGroupBtn->setEnabled(true);
        myGeomGroupLine->setEnabled(true);
        updateButtons();
      }
      else {
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

    }
    else if (myCurrentLineEdit == myGeomGroupLine)
    {
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
      for (; anIt.More(); anIt.Next())
      {
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

    if (aNbSel >= 1) {
      if (aNbSel > 1) {
        if (myCurrentLineEdit == mySubMeshLine)
          aString = tr( "SMESH_SUBMESH_SELECTED" ).arg(aNbSel);
        else if (myCurrentLineEdit == myGroupLine)
          aString = tr( "SMESH_GROUP_SELECTED" ).arg(aNbSel);
        else if (myCurrentLineEdit == myGeomGroupLine)
          aString = tr( "%1 Objects" ).arg(aNbSel);
      }
      else {
        aString = aList.First()->getName();
      }
    }

    myCurrentLineEdit->setText(aString);
    myCurrentLineEdit->home(false);
    // 07.06.2008 skl for IPAL19574:
    // change name of group only if it is empty
    if( myName->text().trimmed().isEmpty() || !myNameChanged ) {
      myOldName = myName->text();
      myName->blockSignals(true);
      myName->setText(aString);
      myName->blockSignals(false);
    }

    updateButtons();
  }
  else // !myCurrentLineEdit: local selection of nodes or elements
  {
    if (aNbSel == 1 && myActor && myActor->hasIO())
    {
#ifdef ENABLE_SWITCH_ACTOR_DURING_ELEMENTS_SELECTION
      // NPAL19389: create a group with a selection in another group
      // Switch myActor to the newly selected one, if the last
      // is visible and belongs to group or submesh of myMesh
      Handle(SALOME_InteractiveObject) curIO = myActor->getIO();
      Handle(SALOME_InteractiveObject) selIO = aList.First();
      if (curIO->hasEntry() && selIO->hasEntry()) {
        const char* selEntry = selIO->getEntry();
        if (strcmp(curIO->getEntry(), selEntry) != 0) {
          // different objects: selected and myActor
          SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView();
          if (aViewWindow && aViewWindow->isVisible(selIO)) {
            // newly selected actor is visible

            // mesh entry
            _PTR(SObject) aSObject = SMESH::FindSObject(myMesh);
            if (aSObject) {
              CORBA::String_var meshEntry = aSObject->GetID().c_str();
              int len = strlen(meshEntry);

              if (strncmp(selEntry, meshEntry, len) == 0) {
                // selected object is myMesh or a part of it
                SMESH_Actor* anActor = SMESH::FindActorByEntry(selEntry);
                if (anActor) {
                  myActor = anActor;
                  SMESH::SetPickable(myActor);
                }
              }
            }
          }
        }
      }
      // NPAL19389 END
#endif // ENABLE_SWITCH_ACTOR_DURING_ELEMENTS_SELECTION

      QString aListStr = "";
      int aNbItems = 0;
      if (myTypeId == 0) {
	aNbItems = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aListStr);
      } else {
	aNbItems = SMESH::GetNameOfSelectedElements(mySelector, myActor->getIO(), aListStr);
      }
      if (aNbItems > 0) {
	QListWidgetItem* anItem;
	QList<QListWidgetItem*> listItemsToSel;
	QStringList anElements = aListStr.split( " ", QString::SkipEmptyParts);
	for (QStringList::iterator it = anElements.begin(); it != anElements.end(); ++it) {
	  QList<QListWidgetItem*> found = myElements->findItems(*it, Qt::MatchExactly);
	  foreach(anItem, found)
	    if (!anItem->isSelected())
	      listItemsToSel.push_back(anItem);
	}
	bool blocked = myElements->signalsBlocked();
	myElements->blockSignals(true);
	foreach(anItem, listItemsToSel) anItem->setSelected(true);
	myElements->blockSignals(blocked);
	onListSelectionChanged();
	listItemsToSel.clear();
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

  // somehow, if we display the mesh, while selecting from another actor,
  // the mesh becomes pickable, and there is no way to select any element
  if (myActor)
    SMESH::SetPickable(myActor);

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
    mySubMeshLine->setText( "" );
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
    myGroupLine->setText( "" );
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
    myGeomGroupBtn->setChecked(false);
    myGeomObjects->length(0);
    myGeomGroupLine->setText( "" );
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
  if(myMesh->_is_nil()) {
    SUIT_MessageBox::critical(this,
			      tr("SMESH_ERROR"),
			      tr("NO_MESH_SELECTED"));
   return;
  }

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
  myFilterDlg->SetSourceWg( myElements, false );

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

  QListWidgetItem* anItem = 0;
  QList<QListWidgetItem*> listItemsToSel;

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
      QStringList anElements = aListStr.split( " ", QString::SkipEmptyParts);
      for (QStringList::iterator it = anElements.begin(); it != anElements.end(); ++it) {
	QList<QListWidgetItem*> found = myElements->findItems(*it, Qt::MatchExactly);
	if (found.count() == 0) {
	  anItem = new QListWidgetItem(*it);
	  myElements->addItem(anItem);
	  if (!anItem->isSelected())
	    listItemsToSel.push_back(anItem);
	}
	else {
	  foreach(anItem, found)
	    if (!anItem->isSelected())
	      listItemsToSel.push_back(anItem);
	}
      }
      bool blocked = myElements->signalsBlocked();
      myElements->blockSignals(true);
      foreach(anItem, listItemsToSel) anItem->setSelected(true);
      myElements->blockSignals(blocked);
      onListSelectionChanged();
      listItemsToSel.clear();
    }
  } else if (myCurrentLineEdit == mySubMeshLine) {
    //SALOME_ListIteratorOfListIO anIt (mySelectionMgr->StoredIObjects());

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects( aList );

    SALOME_ListIteratorOfListIO anIt (aList);
    for ( ; anIt.More(); anIt.Next()) {
      SMESH::SMESH_subMesh_var aSubMesh =
        SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIt.Value());
      if (!aSubMesh->_is_nil()) {
	// check if mesh is the same
	if (aSubMesh->GetFather()->GetId() == myMesh->GetId()) {
          try {
            SMESH::long_array_var anElements = aSubMesh->GetElementsByType(aType);
            int k = anElements->length();
            for (int i = 0; i < k; i++) {
              QString aText = QString::number(anElements[i]);
              QList<QListWidgetItem*> found = myElements->findItems(aText, Qt::MatchExactly);
              if (found.count() == 0) {
                anItem = new QListWidgetItem(aText);
                myElements->addItem(anItem);
		if (!anItem->isSelected())
		  listItemsToSel.push_back(anItem);
              }
	      else {
		foreach(anItem, found)
		  if (!anItem->isSelected())
		    listItemsToSel.push_back(anItem);
	      }
            }
	    bool blocked = myElements->signalsBlocked();
	    myElements->blockSignals(true);
	    foreach(anItem, listItemsToSel) anItem->setSelected(true);
	    myElements->blockSignals(blocked);
	    onListSelectionChanged();
	    listItemsToSel.clear();
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
    for ( ; anIt.More(); anIt.Next()) {
      SMESH::SMESH_GroupBase_var aGroup =
        SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIt.Value());
      if (!aGroup->_is_nil()) {
	// check if mesh is the same
	if (aGroup->GetType() == aType && aGroup->GetMesh()->GetId() == myMesh->GetId()) {
	  SMESH::long_array_var anElements = aGroup->GetListOfID();
	  int k = anElements->length();
	  for (int i = 0; i < k; i++) {
	    QString aText = QString::number(anElements[i]);
	    QList<QListWidgetItem*> found = myElements->findItems(aText, Qt::MatchExactly);
	    if (found.count() == 0) {
	      anItem = new QListWidgetItem(aText);
	      myElements->addItem(anItem);
	      if (!anItem->isSelected())
		listItemsToSel.push_back(anItem);
	    }
	    else {
	      foreach(anItem, found)
		if (!anItem->isSelected())
		  listItemsToSel.push_back(anItem);
	    }
	  }
	  bool blocked = myElements->signalsBlocked();
	  myElements->blockSignals(true);
	  foreach(anItem, listItemsToSel) anItem->setSelected(true);
	  myElements->blockSignals(blocked);
	  onListSelectionChanged();
	  listItemsToSel.clear();
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
      for (int i = 0; i < k; i++) {
	QString aText = QString::number(anElements[i]);
	QList<QListWidgetItem*> found = myElements->findItems(aText, Qt::MatchExactly);
	if (found.count() == 0) {
	  anItem = new QListWidgetItem(aText);
	  myElements->addItem(anItem);
	  if (!anItem->isSelected())
	    listItemsToSel.push_back(anItem);
	}
	else {
	  foreach(anItem, found)
	    if (!anItem->isSelected())
	      listItemsToSel.push_back(anItem);
	}
      }
      bool blocked = myElements->signalsBlocked();
      myElements->blockSignals(true);
      foreach(anItem, listItemsToSel) anItem->setSelected(true);
      myElements->blockSignals(blocked);
      onListSelectionChanged();
      listItemsToSel.clear();
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
    QList<QListWidgetItem*> selItems = myElements->selectedItems();
    QListWidgetItem* item;
    foreach(item, selItems) delete item;
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
      for ( ; anIt.More(); anIt.Next()) {
	SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIt.Value());
	if (!aSubMesh->_is_nil()) {
	  // check if mesh is the same
	  if (aSubMesh->GetFather()->GetId() == myMesh->GetId()) {
	    if (aType == SMESH::NODE) {
	      try {
		SMESH::long_array_var anElements = aSubMesh->GetNodesId();
		int k = anElements->length();
		for (int i = 0; i < k; i++) {
		  QList<QListWidgetItem*> found = 
		    myElements->findItems(QString::number(anElements[i]), Qt::MatchExactly);
		  QListWidgetItem* anItem;
		  foreach(anItem, found) delete anItem;
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
		for (int i = 0; i < k; i++) {
		  QList<QListWidgetItem*> found = 
		    myElements->findItems(QString::number(anElements[i]), Qt::MatchExactly);
		  QListWidgetItem* anItem;
		  foreach(anItem, found) delete anItem;
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
      for ( ; anIt.More(); anIt.Next()) {
	SMESH::SMESH_Group_var aGroup = SMESH::IObjectToInterface<SMESH::SMESH_Group>(anIt.Value());
	if (aRes && !aGroup->_is_nil()) {
	  // check if mesh is the same
	  if (aGroup->GetType() == aType && aGroup->GetMesh()->GetId() == myMesh->GetId()) {
	    SMESH::long_array_var anElements = aGroup->GetListOfID();
	    int k = anElements->length();
	    for (int i = 0; i < k; i++) {
	      QList<QListWidgetItem*> found = 
		myElements->findItems(QString::number(anElements[i]), Qt::MatchExactly);
	      QListWidgetItem* anItem;
	      foreach(anItem, found) delete anItem;
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
    QList<int> aSelected;
    std::vector<int> anArray(k);
    //    QMemArray<int> anArray(k);
    // fill the array
    for (i = 0; i < k; i++) {
      int id = myElements->item(i)->text().toInt();
      anArray[i] = id;
      if (myElements->item(i)->isSelected())
	aSelected.append(id);
    }
    // sort & update list
    std::sort(anArray.begin(), anArray.end());
    //    anArray.sort();
    myElements->clear();
    QListWidgetItem* anItem;
    QList<QListWidgetItem*> listItemsToSel;
    for (i = 0; i < k; i++) {
      anItem = new QListWidgetItem(QString::number(anArray[i]));
      myElements->addItem(anItem);
      if (aSelected.contains(anArray[i]))
	listItemsToSel.push_back(anItem);
    }
    bool blocked = myElements->signalsBlocked();
    myElements->blockSignals(true);
    foreach(anItem, listItemsToSel) anItem->setSelected(true);
    myElements->blockSignals(blocked);
    listItemsToSel.clear();
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
    restoreShowEntityMode();
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
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString( "" ), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr( "WRN_WARNING" ),
			     tr( "EXTERNAL_BROWSER_CANNOT_SHOW_PAGE" ).
			     arg(app->resourceMgr()->stringValue( "ExternalBrowser",
								 platform)).
			     arg(myHelpFileName));
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

  if ( e->key() == Qt::Key_F1 )
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
        myGeomPopup = new QMenu(this);
        myActions[myGeomPopup->addAction( tr( "DIRECT_GEOM_SELECTION" ) )] = DIRECT_GEOM_INDEX;
        myActions[myGeomPopup->addAction( tr( "GEOM_BY_MESH_ELEM_SELECTION" ) )] = GEOM_BY_MESH_INDEX;
        connect( myGeomPopup, SIGNAL( triggered( QAction* ) ), SLOT( onGeomPopup( QAction* ) ) );
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
      QAction* a = myGeomPopup->exec( QCursor::pos() );
      if (!a || myActions[a] == DIRECT_GEOM_INDEX)
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
void SMESHGUI_GroupDlg::onGeomPopup( QAction* a )
{
  int index = myActions[a];
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
      if ( _PTR(SObject) aGeomSO = aStudy->FindObjectID( ID.toLatin1().data() )) {
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
    myColorBtn->setColor( theColor );
}

//=================================================================================
// function : getGroupQColor()
// purpose  :
//=================================================================================
QColor SMESHGUI_GroupDlg::getGroupQColor() const
{
  return myColorBtn->color();
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

    QList<SALOMEDS::Color> aReservedColors;
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

//=================================================================================
// function : SetAppropriateActor()
// purpose  : Find more appropriate of visible actors, set it to myActor, allow picking
//            NPAL19389: create a group with a selection in another group.
//            if mesh actor is not visible - find any first visible group or submesh
//=================================================================================
bool SMESHGUI_GroupDlg::SetAppropriateActor()
{
  bool isActor = false;

  if (myMesh->_is_nil()) return false;

  SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView();

  // try mesh actor
  myActor = SMESH::FindActorByObject(myMesh);
  if (myActor && myActor->hasIO())
  {
    isActor = true;
    if (aViewWindow && !aViewWindow->isVisible(myActor->getIO()))
        isActor = false;
  }

  // try current group actor
  if (!isActor) {
    if (!myGroup->_is_nil()) {
      myActor = SMESH::FindActorByObject(myGroup);
      if (myActor && myActor->hasIO())
      {
        isActor = true;
        if (aViewWindow && !aViewWindow->isVisible(myActor->getIO()))
            isActor = false;
      }
    }
  }

  // try current group on geometry actor
  if (!isActor) {
    if (!myGroupOnGeom->_is_nil()) {
      myActor = SMESH::FindActorByObject(myGroupOnGeom);
      if (myActor && myActor->hasIO())
      {
        isActor = true;
        if (aViewWindow && !aViewWindow->isVisible(myActor->getIO()))
          isActor = false;
      }
    }
  }

  // try any visible actor of group or submesh of current mesh
  if (!isActor && aViewWindow) {
    // mesh entry
    _PTR(SObject) aSObject = SMESH::FindSObject(myMesh);
    if (aSObject) {
      CORBA::String_var meshEntry = aSObject->GetID().c_str();
      int len = strlen(meshEntry);

      // iterate on all actors in current view window, search for
      // any visible actor, that belongs to group or submesh of current mesh
      vtkActorCollection *aCollection = aViewWindow->getRenderer()->GetActors();
      int nbItems = aCollection->GetNumberOfItems();
      for (int i=0; i<nbItems && !isActor; i++)
      {
        SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(aCollection->GetItemAsObject(i));
        if (anActor && anActor->hasIO()) {
          Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
          if (aViewWindow->isVisible(anIO)) {
            if (anIO->hasEntry() && strncmp(anIO->getEntry(), meshEntry, len) == 0) {
              myActor = anActor;
              isActor = true;
            }
          }
        }
      }
    }
  }

  if (isActor)
    SMESH::SetPickable(myActor);

  return isActor;
}

//=======================================================================
//function : setShowEntityMode
//purpose  : make shown only entity corresponding to my type
//=======================================================================
void SMESHGUI_GroupDlg::setShowEntityMode()
{
  if ( !myMesh->_is_nil() ) {
    if ( SMESH_Actor* actor = SMESH::FindActorByObject(myMesh) ) {
      if (!myStoredShownEntity)
        myStoredShownEntity = actor->GetEntityMode();
      switch ( myTypeId ) {
      case 0: restoreShowEntityMode(); break;
      case 1: actor->SetEntityMode( SMESH_Actor::eEdges ); break;
      case 2: actor->SetEntityMode( SMESH_Actor::eFaces ); break;
      case 3: actor->SetEntityMode( SMESH_Actor::eVolumes ); break;
      }
    }
  }
}

//=======================================================================
//function : restoreShowEntityMode
//purpose  : restore ShowEntity mode of myActor
//=======================================================================
void SMESHGUI_GroupDlg::restoreShowEntityMode()
{
  if ( myStoredShownEntity && !myMesh->_is_nil() ) {
    if ( SMESH_Actor* actor = SMESH::FindActorByObject(myMesh) ) {
      actor->SetEntityMode(myStoredShownEntity);
    }
  }
  myStoredShownEntity = 0;
}
