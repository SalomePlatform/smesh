// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESHGUI : GUI for SMESH component
//  File   : SMESHGUI_GroupDlg.cxx
//  Author : Natalia KOPNOVA, Open CASCADE S.A.S.
//  SMESH includes
//
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
//#include <SMESH_ActorUtils.h>
#include <SMESH_LogicalFilter.hxx>

// SALOME GEOM includes
#include <GEOMBase.h>
#include <GEOM_SelectionFilter.h>
#include <GEOM_wrap.hxx>

// SALOME GUI includes
#include <QtxColorButton.h>

#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

#include <SalomeApp_Tools.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_Study.h>
#include <LightApp_SelectionMgr.h>

#include <SALOME_ListIO.hxx>

#include <SVTK_ViewWindow.h>

#include <VTKViewer_Algorithm.h>

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

enum grpSelectionMode {
  grpNoSelection       = -1,
  grpNodeSelection     = 0,
  grp0DSelection       = 1,
  grpBallSelection     = 2,
  grpEdgeSelection     = 3,
  grpFaceSelection     = 4,
  grpVolumeSelection   = 5,
  grpSubMeshSelection  = 6,
  grpGroupSelection    = 7,
  grpMeshSelection     = 8,
  grpGeomSelection     = 9,
  grpAllSelection      = 10,
};

//=================================================================================
// function : SMESHGUI_GroupDlg()
// purpose  :
//=================================================================================
SMESHGUI_GroupDlg::SMESHGUI_GroupDlg( SMESHGUI* theModule,
                                      SMESH::SMESH_Mesh_ptr theMesh )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ), myStoredShownEntity(0),
    mySelector( SMESH::GetViewWindow( theModule )->GetSelector() ),
    myIsBusy( false ),
    myNameChanged( false ),
    myNbChangesOfContents(0),
    myIsApplyAndClose( false )
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
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ), myStoredShownEntity(0),
    mySelector( SMESH::GetViewWindow( theModule )->GetSelector() ),
    myIsBusy( false ),
    myNameChanged( false ),
    myNbChangesOfContents(0) // just not to use uninitialized variable
{
  initDialog( false );
  if ( !theGroup->_is_nil() )
    init( theGroup, theIsConvert );
  else
  {
    mySelectSubMesh->setEnabled( false );
    mySelectGroup->setEnabled( false );

    myCurrentLineEdit = myMeshGroupLine;
    setSelectionMode( grpGroupSelection );
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
  types.append( tr( "SMESH_ELEM0D" ) );
  types.append( tr( "SMESH_BALL_ELEM" ) );
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
  QRadioButton* rb3 = new QRadioButton( tr( "SMESH_GROUP_FILTER" ),     aGrpTypeBox );
  myGrpTypeGroup->addButton( rb1, 0 );
  myGrpTypeGroup->addButton( rb2, 1 );
  myGrpTypeGroup->addButton( rb3, 2 );
  aGrpTypeBoxLayout->addWidget( rb1 );
  aGrpTypeBoxLayout->addWidget( rb2 );
  aGrpTypeBoxLayout->addWidget( rb3 );
  aGrpTypeBox->setEnabled( create );
  myGrpTypeId = -1;

  /***************************************************************/
  myWGStack = new QStackedWidget( this );
  QWidget* wg1 = new QWidget( myWGStack );
  QWidget* wg2 = new QWidget( myWGStack );
  QWidget* wg3 = new QWidget( myWGStack );

  /***************************************************************/
  QGroupBox* aContentBox         = new QGroupBox( tr( "SMESH_CONTENT" ), wg1 );
  QGridLayout* aContentBoxLayout = new QGridLayout( aContentBox );
  aContentBoxLayout->setMargin( MARGIN );
  aContentBoxLayout->setSpacing( SPACING );

  mySelectAll       = new QCheckBox( tr( "SELECT_ALL" ), aContentBox );
  myAllowElemsModif = new QCheckBox( tr( "ALLOW_ELEM_LIST_MODIF" ), aContentBox );

  myElementsLab = new QLabel( tr( "SMESH_ID_ELEMENTS" ), aContentBox );
  myElements    = new QListWidget( aContentBox );
  myElements->setSelectionMode( QListWidget::ExtendedSelection );

  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), aContentBox );
  myAddBtn    = new QPushButton( tr( "SMESH_BUT_ADD" ), aContentBox );
  myRemoveBtn = new QPushButton( tr( "SMESH_BUT_REMOVE" ), aContentBox );
  mySortBtn   = new QPushButton( tr( "SMESH_BUT_SORT" ), aContentBox );

  aContentBoxLayout->addWidget( mySelectAll,       0, 0 );
  aContentBoxLayout->addWidget( myAllowElemsModif, 1, 0 );
  aContentBoxLayout->addWidget( myFilterBtn,       1, 1 );
  aContentBoxLayout->addWidget( myElementsLab,     2, 0 );
  aContentBoxLayout->addWidget( myElements,        3, 0, 6, 1 );
  aContentBoxLayout->addWidget( myAddBtn,          3, 1 );
  aContentBoxLayout->addWidget( myRemoveBtn,       4, 1 );
  aContentBoxLayout->addWidget( mySortBtn,         8, 1 );

  aContentBoxLayout->setColumnStretch( 0, 1 );
  aContentBoxLayout->setRowStretch( 3, 1 );
  aContentBoxLayout->setRowStretch( 6, 1 );

  /***************************************************************/
  mySelectBox = new QGroupBox( tr( "SMESH_SELECT_FROM" ), wg1 );
  QGridLayout* mySelectBoxLayout = new QGridLayout( mySelectBox );
  mySelectBoxLayout->setMargin( MARGIN );
  mySelectBoxLayout->setSpacing( SPACING );

  mySelectSubMesh = new QCheckBox( tr( "SMESH_SUBMESH" ), mySelectBox );
  mySubMeshBtn = new QPushButton( mySelectBox );
  mySubMeshBtn->setIcon( image0 );
  mySubMeshLine = new QLineEdit( mySelectBox );
  mySubMeshLine->setReadOnly( true );
  onSelectSubMesh( false );

  mySelectGroup = new QCheckBox( tr( "SMESH_GROUP" ), mySelectBox );
  myGroupBtn = new QPushButton( mySelectBox );
  myGroupBtn->setIcon( image0 );
  myGroupLine = new QLineEdit( mySelectBox );
  myGroupLine->setReadOnly( true );
  onSelectGroup( false );

  mySelectBoxLayout->addWidget( mySelectSubMesh, 0, 0 );
  mySelectBoxLayout->addWidget( mySubMeshBtn,    0, 1 );
  mySelectBoxLayout->addWidget( mySubMeshLine,   0, 2 );
  mySelectBoxLayout->addWidget( mySelectGroup,   1, 0 );
  mySelectBoxLayout->addWidget( myGroupBtn,      1, 1 );
  mySelectBoxLayout->addWidget( myGroupLine,     1, 2 );

  /***************************************************************/
  QVBoxLayout* wg1Layout = new QVBoxLayout( wg1 );
  wg1Layout->setMargin( 0 );
  wg1Layout->setSpacing( SPACING );
  wg1Layout->addWidget( aContentBox );
  wg1Layout->addWidget( mySelectBox );
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
  wg2Layout->setSpacing( SPACING );
  wg2Layout->addWidget( geomObject,     0, 0 );
  wg2Layout->addWidget( myGeomGroupBtn, 0, 1 );
  wg2Layout->addWidget( myGeomGroupLine,0, 2 );
  wg2Layout->setRowStretch( 1, 5 );

  /***************************************************************/
  QPushButton * aFilter2 = new QPushButton( tr( "SMESH_BUT_FILTER" ), wg3 );
  QGridLayout* wg3Layout = new QGridLayout( wg3 );
  wg3Layout->setMargin( 0 );
  wg3Layout->setSpacing( SPACING );
  wg3Layout->addWidget( aFilter2, 0, 0 );
  wg3Layout->setRowStretch( 1, 5 );

  /***************************************************************/
  myWGStack->insertWidget( 0, wg1 );
  myWGStack->insertWidget( 1, wg2 );
  myWGStack->insertWidget( 2, wg3 );

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
  connect(myMeshGroupBtn,  SIGNAL(clicked()),          this, SLOT(setCurrentSelection()));
  connect(myGrpTypeGroup,  SIGNAL(buttonClicked(int)), this, SLOT(onGrpTypeChanged(int)));
  connect(myTypeGroup,     SIGNAL(buttonClicked(int)), this, SLOT(onTypeChanged(int)));

  connect(myName,          SIGNAL(textChanged(const QString&)), this, SLOT(onNameChanged(const QString&)));
  connect(myElements,      SIGNAL(itemSelectionChanged()),      this, SLOT(onListSelectionChanged()));

  connect(myFilterBtn,     SIGNAL(clicked()),     this, SLOT(setFilters()));
  connect(aFilter2,        SIGNAL(clicked()),     this, SLOT(setFilters()));
  connect(mySelectAll,     SIGNAL(toggled(bool)), this, SLOT(onSelectAll()));
  connect(myAllowElemsModif,SIGNAL(toggled(bool)), this, SLOT(onSelectAll()));
  connect(myAddBtn,        SIGNAL(clicked()),     this, SLOT(onAdd()));
  connect(myRemoveBtn,     SIGNAL(clicked()),     this, SLOT(onRemove()));
  connect(mySortBtn,       SIGNAL(clicked()),     this, SLOT(onSort()));

  connect(mySelectSubMesh, SIGNAL(toggled(bool)), this, SLOT(onSelectSubMesh(bool)));
  connect(mySelectGroup,   SIGNAL(toggled(bool)), this, SLOT(onSelectGroup(bool)));
  connect(mySubMeshBtn,    SIGNAL(clicked()),     this, SLOT(setCurrentSelection()));
  connect(myGroupBtn,      SIGNAL(clicked()),     this, SLOT(setCurrentSelection()));
  connect(myGeomGroupBtn,  SIGNAL(toggled(bool)), this, SLOT(onGeomSelectionButton(bool)));

  connect(myColorBtn,      SIGNAL(changed( QColor )),  this, SLOT(onColorChanged( QColor )));

  connect(myOKBtn,         SIGNAL(clicked()), this, SLOT(onOK()));
  connect(myApplyBtn,      SIGNAL(clicked()), this, SLOT(onApply()));
  connect(myCloseBtn,      SIGNAL(clicked()), this, SLOT(reject()));
  connect(myHelpBtn,       SIGNAL(clicked()), this, SLOT(onHelp()));

  /* Init selection */
  mySMESHGUI->SetActiveDialogBox(this);
  mySMESHGUI->SetState(800);

  SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>( mySMESHGUI->application()->activeStudy() );
  mySelectionMode = grpNoSelection;

  myMeshFilter    = new SMESH_TypeFilter(SMESH::MESH);
  mySubMeshFilter = new SMESH_LogicalFilter(QList<SUIT_SelectionFilter*>(),
                                            SMESH_LogicalFilter::LO_OR,
                                            /*takeOwnership=*/true);
  myGroupFilter   = new SMESH_LogicalFilter(QList<SUIT_SelectionFilter*>(),
                                            SMESH_LogicalFilter::LO_OR,
                                            /*takeOwnership=*/true);
  myGeomFilter    = new GEOM_SelectionFilter( aStudy, true );

  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),        this, SLOT(reject()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),  this, SLOT(onObjectSelectionChanged()));
  connect(mySMESHGUI, SIGNAL(SignalVisibilityChanged()),      this, SLOT(onVisibilityChanged()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()),   this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()),              this, SLOT(onCloseView()));
  rb1->setChecked(true); // VSR !!!
  onGrpTypeChanged(0); // VSR!!!

  if (myMesh->_is_nil() )
    myTypeGroup->button(0)->setChecked(true);

  onSelectAll(); //updateButtons();
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
  if ( myMeshFilter )    delete myMeshFilter;
  if ( mySubMeshFilter ) delete mySubMeshFilter;
  if ( myGroupFilter )   delete myGroupFilter;
  if ( myGeomFilter )    delete myGeomFilter;
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
    isUnique = (aSet.count(std::string(SMESH::toUtf8(aName))) == 0);
  }

  return aName;
}

void  SMESHGUI_GroupDlg::setDefaultName() const
{
  QString aResName;
  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
  int i=1;
  QString aPrefix ="Group_";
  _PTR(SObject) anObj;
  do
  {
    aResName = aPrefix + QString::number( i++ );
    anObj = aStudy->FindObject( SMESH::toUtf8(aResName) );
  }
  while ( anObj );
  myName->setText(aResName); 
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
  myGroupOnFilter = SMESH::SMESH_GroupOnFilter::_nil();

  // NPAL19389: create a group with a selection in another group
  // set actor of myMesh, if it is visible, else try
  // any visible actor of group or submesh of myMesh
  SetAppropriateActor();

  setDefaultGroupColor();
  setDefaultName();


  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );
  if( !aList.IsEmpty() )
  {
    QString aName = aList.First()->getName();
    myMeshGroupLine->setText(aName);//??????
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
                              const bool                 theIsConvert)
{
  restoreShowEntityMode();
  myMesh = theGroup->GetMesh();
  setShowEntityMode();

  myNameChanged = true;
  myName->blockSignals(true);
  myName->setText(SMESH::fromUtf8(theGroup->GetName()));
  myName->blockSignals(false);
  myName->home(false);

  SALOMEDS::Color aColor = theGroup->GetColor();
  setGroupColor( aColor );

  myMeshGroupLine->setText(SMESH::fromUtf8(theGroup->GetName()));

  int aType = 0;
  switch(theGroup->GetType()) {
  case SMESH::NODE:   aType = grpNodeSelection;   break;
  case SMESH::ELEM0D: aType = grp0DSelection;     break;
  case SMESH::BALL:   aType = grpBallSelection;   break;
  case SMESH::EDGE:   aType = grpEdgeSelection;   break;
  case SMESH::FACE:   aType = grpFaceSelection;   break;
  case SMESH::VOLUME: aType = grpVolumeSelection; break;
  case SMESH::ALL:
  case SMESH::NB_ELEMENT_TYPES: break;
  }
  myTypeGroup->button(aType)->setChecked(true);

  myGroup         = SMESH::SMESH_Group::_narrow( theGroup );
  myGroupOnGeom   = SMESH::SMESH_GroupOnGeom::_narrow( theGroup );
  myGroupOnFilter = SMESH::SMESH_GroupOnFilter::_narrow( theGroup );
  myFilter        = SMESH::Filter::_nil();

  if (myGroup->_is_nil() && myGroupOnGeom->_is_nil() && myGroupOnFilter->_is_nil() )
    return;

  // NPAL19389: create a group with a selection in another group
  // set actor of myMesh, if it is visible, else set
  // actor of theGroup, if it is visible, else try
  // any visible actor of group or submesh of myMesh
  // commented, because an attempt to set selection on not displayed cells leads to error
  SetAppropriateActor();

  /*  SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh);
  if ( !anActor )
    anActor = SMESH::FindActorByObject(theGroup);
  SMESH::SetPickable(anActor);*/

  int grpType = (!myGroup->_is_nil() ? 0 : (theIsConvert ? 0 : myGroupOnGeom->_is_nil() ? 2 : 1));
  myGrpTypeGroup->button(grpType)->setChecked(true);
  onGrpTypeChanged(grpType);

  myTypeId = aType;
  if ( grpType == 0 ) { // standalone group
    myCurrentLineEdit = 0;
    myElements->clear();
    myAllowElemsModif->setChecked( true );

    setSelectionMode(aType);

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
  else if ( grpType == 1 ) // group on geom
  {
    QString aShapeName( "" );
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    GEOM::GEOM_Object_var aGroupShape = myGroupOnGeom->GetShape();
    if (!aGroupShape->_is_nil())
    {
      _PTR(SObject) aGroupShapeSO = aStudy->FindObjectID(aGroupShape->GetStudyEntry());
      if ( aGroupShapeSO )
        aShapeName = aGroupShapeSO->GetName().c_str();
    }
    myGeomGroupLine->setText( aShapeName );
  }
  else // group on filter
  {
    myFilter = myGroupOnFilter->GetFilter();
    if ( !myFilter->_is_nil() ) {
      SMESH::Predicate_var perdicate = myFilter->GetPredicate();
      if ( perdicate->_is_nil() )
        myFilter = SMESH::Filter::_nil();
    }
  }

  if ( grpType != 0 )
  {
    myNameChanged = true;
    myName->blockSignals(true);
    myName->setText(SMESH::fromUtf8(theGroup->GetName()));
    myName->blockSignals(false);
  }

  onSelectAll(); //updateButtons();
}

//=================================================================================
// function : updateButtons()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::updateButtons()
{
  bool enable = !myName->text().trimmed().isEmpty();
  if ( enable )
  {
    if (myGrpTypeId == 0) { // standalone
      if ( !mySelectAll->isChecked() )
      {
        if ( myAllowElemsModif->isChecked() )
        {
          enable = ( myElements->count() > 0 );
        }
        else if ((enable = !myFilter->_is_nil() ))
        {
          SMESH::array_of_ElementType_var types = myFilter->GetTypes();
          enable = types->length();
        }
      }
      enable = enable && (!myGroup->_is_nil() || !myMesh->_is_nil());
    }
    else if (myGrpTypeId == 1) // on geom
    {
      if (CORBA::is_nil(myGroupOnGeom)) // creation mode
        enable = ( myGeomObjects->length() > 0 && !myMesh->_is_nil() );
    }
    else if (myGrpTypeId == 2) // on filter
    {
      if (( enable = !myFilter->_is_nil() ))
        if (CORBA::is_nil(myGroupOnFilter) )  // creation mode
          enable = !myMesh->_is_nil();
    }
  }

  bool meshHasGeom = ( myMesh->_is_nil() || myMesh->HasShapeToMesh() );
  if ( myGrpTypeId != 1 )
  {
    myGrpTypeGroup->button(1)->setEnabled( meshHasGeom );
  }
  else
  {
    myGeomGroupBtn->setEnabled( meshHasGeom );
    myGeomGroupLine->setEnabled( meshHasGeom );
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
    myTypeId = id;
    if ( myGrpTypeId == 0 && myCurrentLineEdit == 0)
      setSelectionMode(id);
    else
      setSelectionMode( mySelectionMode++ ); // update according to mySelectionMode

    onObjectSelectionChanged();
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
    myGrpTypeId = id;
    myWGStack->setCurrentIndex( id );
    myName->blockSignals(true);
    myName->setText(myOldName);
    myName->blockSignals(false);
    onSelectGeomGroup(id != 0);
  }
  updateButtons();
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
  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
  bool isSelectAll = mySelectAll->isChecked() || !myAllowElemsModif->isChecked() || myGrpTypeId != 0;
  if (mySelectionMode != theMode) {
    // [PAL10408] mySelectionMgr->clearSelected();
    mySelectionMgr->clearFilters();
    SMESH::RemoveFilters();

    if (myActorsList.count() > 0)
      for (QListIterator<SMESH_Actor*> it( myActorsList ); it.hasNext(); )
        it.next()->SetPointRepresentation(false);
    else
      SMESH::SetPointRepresentation(false);

    switch (theMode) {
    case grpNodeSelection:
      if ( myGrpTypeId == 0 ) // standalone
      {
        if (myActorsList.count() > 0)
          for (QListIterator<SMESH_Actor*> it( myActorsList ); it.hasNext(); )
            it.next()->SetPointRepresentation(true);
        else
          SMESH::SetPointRepresentation(true);
      }
      if ( aViewWindow ) aViewWindow->SetSelectionMode(isSelectAll ? ActorSelection : NodeSelection);
      break;
    case grpEdgeSelection:
      if ( aViewWindow ) aViewWindow->SetSelectionMode(isSelectAll ? ActorSelection : EdgeSelection);
      break;
    case grpBallSelection:
      if ( aViewWindow ) aViewWindow->SetSelectionMode(isSelectAll ? ActorSelection : BallSelection);
      break;
    case grp0DSelection:
      if ( aViewWindow ) aViewWindow->SetSelectionMode(isSelectAll ? ActorSelection : Elem0DSelection);
      break;
    case grpFaceSelection:
      if ( aViewWindow ) aViewWindow->SetSelectionMode(isSelectAll ? ActorSelection : FaceSelection);
      break;
    case grpVolumeSelection:
      if ( aViewWindow ) aViewWindow->SetSelectionMode(isSelectAll ? ActorSelection : VolumeSelection);
      break;
    case grpSubMeshSelection: {

      SMESH_TypeFilter* f = 0;
      switch (myTypeId) {
      case grpNodeSelection:   f = new SMESH_TypeFilter(SMESH::SUBMESH); break;
      case grpEdgeSelection:   f = new SMESH_TypeFilter(SMESH::SUBMESH_EDGE); break;
      case grpFaceSelection:   f = new SMESH_TypeFilter(SMESH::SUBMESH_FACE); break;
      case grpVolumeSelection: f = new SMESH_TypeFilter(SMESH::SUBMESH_SOLID); break;
      default:                 f = new SMESH_TypeFilter(SMESH::SUBMESH);
      }
      QList<SUIT_SelectionFilter*> filtList;
      filtList.append( f );
      filtList.append( new SMESH_TypeFilter(SMESH::SUBMESH_COMPOUND));
      mySubMeshFilter->setFilters( filtList );

      mySelectionMgr->installFilter( mySubMeshFilter );

      if ( aViewWindow ) aViewWindow->SetSelectionMode(ActorSelection);
      break;
    }
    case grpGroupSelection: {

      SMESH_TypeFilter* f = 0;
      switch (myTypeId) {
      case grpNodeSelection:   f = new SMESH_TypeFilter(SMESH::GROUP_NODE);   break;
      case grp0DSelection:     f = new SMESH_TypeFilter(SMESH::GROUP_0D);     break;
      case grpBallSelection:   f = new SMESH_TypeFilter(SMESH::GROUP_BALL);   break;
      case grpEdgeSelection:   f = new SMESH_TypeFilter(SMESH::GROUP_EDGE);   break;
      case grpFaceSelection:   f = new SMESH_TypeFilter(SMESH::GROUP_FACE);   break;
      case grpVolumeSelection: f = new SMESH_TypeFilter(SMESH::GROUP_VOLUME); break;
      default:                 f = new SMESH_TypeFilter(SMESH::GROUP);
      }
      QList<SUIT_SelectionFilter*> filtList;
      filtList.append( f );
      myGroupFilter->setFilters( filtList );

      mySelectionMgr->installFilter(myGroupFilter);
      if ( aViewWindow ) aViewWindow->SetSelectionMode(ActorSelection);
      break;
    }
    case grpMeshSelection:
      mySelectionMgr->installFilter(myMeshFilter);
      if ( aViewWindow ) aViewWindow->SetSelectionMode(ActorSelection);
      break;
    case grpGeomSelection:
      mySelectionMgr->installFilter(myGeomFilter);
      if ( aViewWindow ) aViewWindow->SetSelectionMode(ActorSelection);
      break;
    default:
      if ( aViewWindow ) aViewWindow->SetSelectionMode(ActorSelection);
      break;
    }
    if ( aViewWindow ) aViewWindow->Repaint();
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

  SMESH::ElementType aType = SMESH::ALL;
  switch (myTypeId) {
  case grpNodeSelection:   aType = SMESH::NODE;   break;
  case grp0DSelection:     aType = SMESH::ELEM0D; break;
  case grpBallSelection:   aType = SMESH::BALL;   break;
  case grpEdgeSelection:   aType = SMESH::EDGE;   break;
  case grpFaceSelection:   aType = SMESH::FACE;   break;
  case grpVolumeSelection: aType = SMESH::VOLUME; break;
  }

  bool anIsOk = false;
  QStringList anEntryList;

  SMESH::SMESH_GroupBase_var resultGroup;
  bool isCreation = false, isConversion = false;
    
  SUIT_OverrideCursor wc;

  if (myGrpTypeId == 0)  // standalone
  {
    if (!mySelectAll->isChecked() && !myElements->count() && myAllowElemsModif->isChecked())
      return false;

    mySelectionMgr->clearSelected();

    if (myGroup->_is_nil()) { // creation or conversion
      // check if group on geometry is not null
      if (!myGroupOnGeom->_is_nil() || !myGroupOnFilter->_is_nil()) {
        if (myMesh->_is_nil())
          return false;
        if ( myGroupOnGeom->_is_nil() )
          myGroup = myMesh->ConvertToStandalone( myGroupOnFilter );
        else
          myGroup = myMesh->ConvertToStandalone( myGroupOnGeom );

        myGroupOnGeom   = SMESH::SMESH_GroupOnGeom::_nil();
        myGroupOnFilter = SMESH::SMESH_GroupOnFilter::_nil();
        isConversion    = true;
      }
    }

    if (myGroup->_is_nil()) { // creation
      if (myMesh->_is_nil())
        return false;

      myGroup = SMESH::AddGroup(myMesh, aType, myName->text());

      resultGroup = SMESH::SMESH_GroupBase::_narrow( myGroup );
      isCreation = true;

      if ( mySelectAll->isChecked() ) {
        // select all
        myGroup->AddFrom(myMesh.in());
      }
      else {
        // select manually

        if ( !myFilter->_is_nil() &&
             ( myNbChangesOfContents == 1 || !myAllowElemsModif->isChecked()))
        {
          myGroup->AddFrom( myFilter );
        }
        else
        {
          SMESH::long_array_var anIdList = new SMESH::long_array;
          int i, k = myElements->count();
          anIdList->length(k);
          for (i = 0; i < k; i++) {
            anIdList[i] = myElements->item(i)->text().toInt();
          }
          myGroup->Add(anIdList.inout());
        }
      }


    } else { // edition

      resultGroup = SMESH::SMESH_GroupBase::_narrow( myGroup );
      isCreation = false;

      if ( mySelectAll->isChecked() ) {
        // select all
        myGroup->Clear();
        myGroup->AddFrom(myMesh.in());
      }
      else {
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
    }

    anIsOk = true;
  }
  else if (myGrpTypeId == 1) // on geom object
  {
    if (CORBA::is_nil(myGroupOnGeom)) { // creation
      if (myMesh->_is_nil() || !myGeomObjects->length())
        return false;

      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

      if (myGeomObjects->length() == 1) {
        myGroupOnGeom = myMesh->CreateGroupFromGEOM(aType,
                                                    SMESH::toUtf8(myName->text()),
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

        GEOM::GEOM_IGroupOperations_wrap op = geomGen->GetIGroupOperations(aStudy->StudyId());
        if (op->_is_nil())
          return false;

        // check and add all selected GEOM objects: they must be
        // a sub-shapes of the main GEOM and must be of one type
        TopAbs_ShapeEnum aGroupType = TopAbs_SHAPE;
        for ( int i =0; i < (int)myGeomObjects->length(); i++) {
          TopAbs_ShapeEnum aSubShapeType = (TopAbs_ShapeEnum)myGeomObjects[i]->GetShapeType();
          if (i == 0)
            aGroupType = aSubShapeType;
          else if (aSubShapeType != aGroupType) {
            aGroupType = TopAbs_SHAPE;
            break;
          }
        }

        GEOM::GEOM_Object_var  aMeshShape = myMesh->GetShapeToMesh();
        GEOM::GEOM_Object_wrap aGroupVar = op->CreateGroup(aMeshShape, aGroupType);
        op->UnionList(aGroupVar, myGeomObjects);

        if (op->IsDone()) {
          // publish the GEOM group in study
          QString aNewGeomGroupName ( "Auto_group_for_" );
          aNewGeomGroupName += myName->text();
          SALOMEDS::SObject_var aNewGroupSO =
            geomGen->AddInStudy(aSMESHGen->GetCurrentStudy(), aGroupVar,
                                SMESH::toUtf8(aNewGeomGroupName), aMeshShape);
        }

        myGroupOnGeom = myMesh->CreateGroupFromGEOM(aType,
                                                    SMESH::toUtf8(myName->text()),
                                                    aGroupVar);
      }
      resultGroup = SMESH::SMESH_GroupBase::_narrow( myGroupOnGeom );
      isCreation = true;

    }
    else { // edition

      resultGroup = SMESH::SMESH_GroupBase::_narrow( myGroupOnGeom );
      isCreation = false;
    }      
    anIsOk = true;
  }
  if (myGrpTypeId == 2) // group on filter
  {
    if ( myFilter->_is_nil() ) return false;

    if (CORBA::is_nil(myGroupOnFilter)) { // creation
      if (myMesh->_is_nil())
        return false;

      myGroupOnFilter = myMesh->CreateGroupFromFilter(aType,
                                                      SMESH::toUtf8(myName->text()),
                                                      myFilter);

      resultGroup = SMESH::SMESH_GroupBase::_narrow( myGroupOnFilter );
      isCreation = true;
    }
    else
    {
      myGroupOnFilter->SetFilter( myFilter );

      resultGroup = SMESH::SMESH_GroupBase::_narrow( myGroupOnFilter );
      isCreation = false;
    }
    anIsOk = true;
  }

  if( anIsOk )
  {
    SALOMEDS::Color aColor = getGroupColor();
    resultGroup->SetColor(aColor);

    _PTR(SObject) aMeshGroupSO = SMESH::FindSObject( resultGroup );
    if( aMeshGroupSO )
      anEntryList.append( aMeshGroupSO->GetID().c_str() );

    resultGroup->SetName(SMESH::toUtf8(myName->text().trimmed()));

    if ( isCreation )
    {
      SMESH::setFileType ( aMeshGroupSO, "COULEURGROUP" );

      /* init for the next operation */
      setDefaultName();
      myElements->clear();
      myGroup         = SMESH::SMESH_Group::_nil();
      myGroupOnGeom   = SMESH::SMESH_GroupOnGeom::_nil();
      myGroupOnFilter = SMESH::SMESH_GroupOnFilter::_nil();
      myFilter        = SMESH::Filter::_nil();

      setDefaultGroupColor(); // reset color for case if 'auto-color' feature is enabled.
    }
    else
    {
      if ( aMeshGroupSO )
      {
        if ( SMESH_Actor *anActor = SMESH::FindActorByEntry(aMeshGroupSO->GetID().c_str()))
        {
          Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
          if ( isConversion ) { // need to reset TVisualObj and actor
            SMESH::RemoveVisualObjectWithActors( anIO->getEntry(), true );
            SMESH::Update( anIO,true);
            myActorsList.clear();
            anActor = SMESH::FindActorByEntry( anIO->getEntry() );
            if ( !anActor ) return false;
            myActorsList.append( anActor );
          }
          anActor->setName(SMESH::toUtf8(myName->text()));
          QColor c;
          int delta;
          switch ( myTypeId ) {
          case grpNodeSelection:   anActor->SetNodeColor( aColor.R, aColor.G, aColor.B ); break;
          case grp0DSelection:     anActor->Set0DColor  ( aColor.R, aColor.G, aColor.B ); break;
          case grpBallSelection:   anActor->SetBallColor( aColor.R, aColor.G, aColor.B ); break;
          case grpEdgeSelection:   anActor->SetEdgeColor( aColor.R, aColor.G, aColor.B ); break;
          case grpVolumeSelection: 
            SMESH::GetColor("SMESH", "volume_color", c , delta, "255,0,170|-100");
            anActor->SetVolumeColor( aColor.R, aColor.G, aColor.B, delta ); break;          
            break;
          case grpFaceSelection:   
          default:
            SMESH::GetColor("SMESH", "fill_color", c , delta, "0,170,255|-100");
            anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B, delta ); break;          
            break;
          }
          // update a visible group accoding to a changed contents
          if ( !isConversion && anActor->GetVisibility() )
          {
            SMESH::Update( anIO, true );
            SMESH::RepaintCurrentView();
          }
        }
      }
    }
    SMESHGUI::Modified();
    mySMESHGUI->updateObjBrowser(true);
    mySelectionMgr->clearSelected();

    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      myObjectToSelect = anApp->browseObjects( anEntryList, isApplyAndClose() );
  }
  return anIsOk;
}

//=================================================================================
// function : onOK()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onOK()
{
  setIsApplyAndClose( true );
  if ( onApply() )
    reject();
  setIsApplyAndClose( false );

  if ( myFilterDlg ) myFilterDlg->UnRegisterFilters();
}

//=================================================================================
// function : onListSelectionChanged()
// purpose  : Called when selection in element list is changed
//=================================================================================
void SMESHGUI_GroupDlg::onListSelectionChanged()
{
  //MESSAGE( "SMESHGUI_GroupDlg::onListSelectionChanged(); myActorsList.count() = " << myActorsList.count());
  if( myIsBusy || myActorsList.count() == 0 ) return;
  myIsBusy = true;

  if (myCurrentLineEdit == 0) {
    mySelectionMgr->clearSelected();
    TColStd_MapOfInteger aIndexes;
    QList<QListWidgetItem*> selItems = myElements->selectedItems();
    QListWidgetItem* anItem;
    foreach(anItem, selItems) aIndexes.Add(anItem->text().toInt());
    mySelector->AddOrRemoveIndex(myActorsList.first()->getIO(), aIndexes, false);
    SALOME_ListIO aList;
    aList.Append(myActorsList.first()->getIO());
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

        if ( myFilterDlg && !myMesh->_is_nil()){
          myFilterDlg->SetMesh( myMesh );
        }
        myGroup = SMESH::SMESH_Group::_nil();

        // NPAL19389: create a group with a selection in another group
        // set actor of myMesh, if it is visible, else try
        // any visible actor of group or submesh of myMesh
        SetAppropriateActor();

        setDefaultGroupColor();
        if (myName->text().isEmpty())
          setDefaultName();

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
      int i = 0;

      SALOME_ListIteratorOfListIO anIt (aList);
      for (; anIt.More(); anIt.Next())
      {
        aGeomGroup = GEOMBase::ConvertIOinGEOMObject(anIt.Value());

        // Check if the object is a geometry group
        if (CORBA::is_nil(aGeomGroup))
          continue;

        // Check if group constructed on the same shape as a mesh or on its child
        _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

        // The main shape of the group
        GEOM::GEOM_Object_var aGroupMainShape;
        if (aGeomGroup->GetType() == 37) {
          GEOM::GEOM_IGroupOperations_wrap anOp =
            SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());
          aGroupMainShape = anOp->GetMainShape(aGeomGroup);
          // aGroupMainShape is an existing servant => GEOM_Object_var not GEOM_Object_wrap
        }
        else {
          aGroupMainShape = aGeomGroup;
          aGroupMainShape->Register();
        }
        _PTR(SObject) aGroupMainShapeSO =
          aStudy->FindObjectID(aGroupMainShape->GetStudyEntry());

        _PTR(SObject) anObj, aRef;
        bool isRefOrSubShape = false;
        if (aMeshSO->FindSubObject(1, anObj) &&  anObj->ReferencedObject(aRef)) {
          if (aRef->GetID() == aGroupMainShapeSO->GetID()) {
            isRefOrSubShape = true;
          } else {
            _PTR(SObject) aFather = aGroupMainShapeSO->GetFather();
            _PTR(SComponent) aComponent = aGroupMainShapeSO->GetFatherComponent();
            while (!isRefOrSubShape && aFather->GetID() != aComponent->GetID()) {
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
    if (aNbSel == 1 && myActorsList.count() > 0 )
    {
      // NPAL19389: create a group with a selection in another group
      // Switch myActor to the newly selected one, if the last
      // is visible and belongs to group or submesh of myMesh
      /*      Handle(SALOME_InteractiveObject) curIO = myActor->getIO();
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
      }*/
      // NPAL19389 END

      QString aListStr = "";
      int aNbItems = 0;
      if (myTypeId == 0) {
        QListIterator<SMESH_Actor*> it( myActorsList );
        while ( it.hasNext() ) {
          QString tmpStr;
          aNbItems += SMESH::GetNameOfSelectedNodes(mySelector, it.next()->getIO(), tmpStr);
          aListStr += tmpStr;
        }
      } else {
        QListIterator<SMESH_Actor*> it( myActorsList );
        while ( it.hasNext() ) {
          QString tmpStr;
          aNbItems += SMESH::GetNameOfSelectedElements(mySelector, it.next()->getIO(), tmpStr);
          aListStr += tmpStr;
        }
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
  
  if (myActorsList.count() == 0) {
    if (!myGroup->_is_nil()) {
      SMESH_Actor* anActor = SMESH::FindActorByObject(myGroup);
      if ( anActor )
        myActorsList.append( anActor  );
    }
    else if(!myGroupOnGeom->_is_nil()) {
      SMESH_Actor* anActor = SMESH::FindActorByObject(myGroupOnGeom);
      if ( anActor )
        myActorsList.append( anActor );
    }
    else {
      SMESH_Actor* anActor = SMESH::FindActorByObject( myMesh );
      if ( anActor )
        myActorsList.append( anActor );
    }
  }

  // somehow, if we display the mesh, while selecting from another actor,
  // the mesh becomes pickable, and there is no way to select any element
  if (myActorsList.count() > 0) {
    QListIterator<SMESH_Actor*> it( myActorsList );
    while ( it.hasNext() ) {
      SMESH_Actor* anActor = it.next();
      if ( IsActorVisible(anActor) )
        anActor->SetPickable(true);
    }
  }

  myIsBusy = false;
}

//=================================================================================
// function : onSelectAll()
// purpose  : Called when "Select all" is checked
//=================================================================================
void SMESHGUI_GroupDlg::onSelectAll()
{
  bool noElemsModif = ( mySelectAll->isChecked() || !myAllowElemsModif->isChecked() );

  myElementsLab->setEnabled( !noElemsModif );
  myElements->setEnabled   ( !noElemsModif );
  myFilterBtn->setEnabled  ( !noElemsModif );
  myAddBtn->setEnabled     ( !noElemsModif );
  myRemoveBtn->setEnabled  ( !noElemsModif );
  mySortBtn->setEnabled    ( !noElemsModif );
  mySelectBox->setEnabled  ( !noElemsModif );
  myAllowElemsModif->setEnabled( !mySelectAll->isChecked() );

  int selMode     = mySelectionMode;
  mySelectionMode = grpNoSelection;
  setSelectionMode( selMode );
  updateButtons();
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
    setSelectionMode(grpSubMeshSelection);
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
    setSelectionMode(grpGroupSelection);
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
// purpose  : Called when group type changed. on == "on geometry" or "on filter"
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
    if ( myGrpTypeId == 1 ) { // on geometry
      myCurrentLineEdit = myGeomGroupLine;
      updateGeomPopup();
    }
    else { // on filter
      myCurrentLineEdit = 0;
    }
    setSelectionMode(grpAllSelection);
  }
  else {
    myGeomGroupBtn->setChecked(false);
    myGeomObjects->length(0);
    myGeomGroupLine->setText( "" );
    myCurrentLineEdit = 0;
    if (myTypeId != -1)
      setSelectionMode( myTypeId );
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
    disconnect(myMeshGroupBtn, SIGNAL(clicked()), this, SLOT(setCurrentSelection()));
    mySelectionMgr->clearSelected();
    if (myCreate)
      setSelectionMode(grpMeshSelection);
    else
      setSelectionMode(grpGroupSelection);
    connect(myMeshGroupBtn, SIGNAL(clicked()), this, SLOT(setCurrentSelection()));
    myCurrentLineEdit = myMeshGroupLine;
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
    case grpNodeSelection:   aType = SMESH::NODE;   break;
    case grp0DSelection:     aType = SMESH::ELEM0D; break;
    case grpBallSelection:   aType = SMESH::BALL;   break;
    case grpEdgeSelection:   aType = SMESH::EDGE;   break;
    case grpFaceSelection:   aType = SMESH::FACE;   break;
    case grpVolumeSelection: aType = SMESH::VOLUME; break;
    default:                 return;
  }

  if ( myFilterDlg == 0 )
  {
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, aType );
    connect( myFilterDlg, SIGNAL( Accepted() ), SLOT( onFilterAccepted() ) );
  }
  else
    myFilterDlg->Init( aType );

  if ( !myGroupOnFilter->_is_nil() )
  {
    myFilterDlg->SetFilter( myFilter, aType );
    myFilterDlg->Init( aType );
  }

  bool isStandalone = ( sender() == myFilterBtn );
  myFilterDlg->SetEnabled( /*setInViewer=*/isStandalone,
                           /*diffSources=*/isStandalone );
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetGroup( myGroupOnFilter );
  myFilterDlg->SetSelection();
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
  // get a filter from myFilterDlg
  myFilter = myFilterDlg->GetFilter();
  if ( !myFilter->_is_nil() ) {
    SMESH::Predicate_var perdicate = myFilter->GetPredicate();
    if ( perdicate->_is_nil() )
      myFilter = SMESH::Filter::_nil();
  }
  // set mesh to myFilter
  if ( !myFilter->_is_nil() ) {
    SMESH::SMESH_Mesh_var mesh = myMesh;
    if ( mesh->_is_nil() ) {
      if ( !myGroup->_is_nil() )
        mesh = myGroup->GetMesh();
      else if ( !myGroupOnGeom->_is_nil() )
        mesh = myGroupOnGeom->GetMesh();
      else if ( !myGroupOnFilter->_is_nil() )
        mesh = myGroupOnFilter->GetMesh();
    }
    myFilter->SetMesh( mesh );

    // highlight ids if selection changed in the Viewer (IPAL52924)
    myCurrentLineEdit = 0;
    onObjectSelectionChanged();
  }

  updateButtons();
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

  if (aNbSel == 0 || myActorsList.count() == 0 || myMesh->_is_nil()) return;

  SUIT_OverrideCursor wc;

  myIsBusy = true;
  int sizeBefore = myElements->count();

  SMESH::ElementType aType = SMESH::ALL;
  switch(myTypeId) {
  case grpNodeSelection:
    aType = SMESH::NODE;
    mySelector->SetSelectionMode(NodeSelection);
    break;
  case grpBallSelection:
    aType = SMESH::BALL;
    mySelector->SetSelectionMode(BallSelection);
    break;
  case grp0DSelection:
    aType = SMESH::ELEM0D;
    mySelector->SetSelectionMode(Elem0DSelection);
    break;
  case grpEdgeSelection:
    aType = SMESH::EDGE;
    mySelector->SetSelectionMode(EdgeSelection);
    break;
  case grpFaceSelection:
    aType = SMESH::FACE;
    mySelector->SetSelectionMode(FaceSelection);
    break;
  case grpVolumeSelection:
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
      QListIterator<SMESH_Actor*> it( myActorsList );
      while ( it.hasNext() ) {
        QString tmpStr;
        aNbItems += SMESH::GetNameOfSelectedNodes(mySelector, it.next()->getIO(), tmpStr);
        aListStr += tmpStr;
      }
    }
    else {
      QListIterator<SMESH_Actor*> it( myActorsList );
      while ( it.hasNext() ) {
        QString tmpStr;
        aNbItems += SMESH::GetNameOfSelectedElements(mySelector, it.next()->getIO(), tmpStr);
        aListStr += tmpStr;
      }
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
    GEOM::GEOM_IGroupOperations_wrap aGroupOp =
      SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());

    SMESH::ElementType aGroupType = SMESH::ALL;
    switch(aGroupOp->GetType(myGeomObjects[0])) {
    case TopAbs_VERTEX: aGroupType = SMESH::NODE; break;
    case TopAbs_EDGE:   aGroupType = SMESH::EDGE; break;
    case TopAbs_FACE:   aGroupType = SMESH::FACE; break;
    case TopAbs_SOLID:  aGroupType = SMESH::VOLUME; break;
    default: myIsBusy = false; return;
    }

    if (aGroupType == aType) {
      _PTR(SObject) aGroupSO =
        //aStudy->FindObjectIOR(aStudy->ConvertObjectToIOR(myGeomGroup));
        aStudy->FindObjectID(myGeomObjects[0]->GetStudyEntry());
      // Construct filter
      SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
      SMESH::Filter_var aFilter = aFilterMgr->CreateFilter();
      SMESH::BelongToGeom_var aBelongToGeom = aFilterMgr->CreateBelongToGeom();
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
  if ( sizeBefore < myElements->count() )
    ++myNbChangesOfContents;
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
  int sizeBefore = myElements->count();

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
    case grpNodeSelection:   aType = SMESH::NODE;   break;
    case grp0DSelection:     aType = SMESH::ELEM0D; break;
    case grpBallSelection:   aType = SMESH::BALL;   break;
    case grpEdgeSelection:   aType = SMESH::EDGE;   break;
    case grpFaceSelection:   aType = SMESH::FACE;   break;
    case grpVolumeSelection: aType = SMESH::VOLUME; break;
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
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects( aList );

      SALOME_ListIteratorOfListIO anIt (aList);
      for ( ; anIt.More(); anIt.Next()) {
        SMESH::SMESH_Group_var aGroup = SMESH::IObjectToInterface<SMESH::SMESH_Group>(anIt.Value());
        if (!aGroup->_is_nil()) {
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
  if ( sizeBefore > myElements->count() )
    myNbChangesOfContents += 2; // it's used to detect that "Add" was only once
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
// function : onVisibilityChanged()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onVisibilityChanged()
{
  SetAppropriateActor();
}

//=================================================================================
// function : SMESHGUI_GroupDlg::reject
// purpose  : SLOT called when "Close" button pressed. Close dialog
//=================================================================================
void SMESHGUI_GroupDlg::reject()
{
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
    SMESH::SetPickable();
    restoreShowEntityMode();
  }

  if( isApplyAndClose() && !myObjectToSelect.isEmpty() ) {
    SUIT_DataOwnerPtrList aList;
    aList.append( new LightApp_DataOwner( myObjectToSelect ) );
    mySelectionMgr->setSelected( aList );
  }
  else
    mySelectionMgr->clearSelected();
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySelectionMgr->clearFilters();
  mySMESHGUI->ResetState();

  QDialog::reject();

  if ( myFilterDlg ) myFilterDlg->UnRegisterFilters();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onCloseView()
{
  onDeactivate();
  mySelector = 0;
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_GroupDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app)
  {
    app->onHelpContextModule
      ( mySMESHGUI ? app->moduleName( mySMESHGUI->moduleName() ) : QString(""), myHelpFileName );
  }
  else
  {
#ifdef WIN32
    QString platform = "winapplication";
#else
    QString platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr( "WRN_WARNING" ),
                             tr( "EXTERNAL_BROWSER_CANNOT_SHOW_PAGE" ).
                             arg(app->resourceMgr()->stringValue( "ExternalBrowser", platform)).
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
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
    mySelectionMode = grpNoSelection;
    setSelectionMode(myTypeId);
    //mySMESHGUI->SetActiveDialogBox((QDialog*)this);
    mySMESHGUI->SetActiveDialogBox(this);
    mySMESHGUI->SetState(800);
  }
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
      setSelectionMode(grpGeomSelection);
  }
  else if (!isBtnOn)
  {
    myCurrentLineEdit = 0;
    setSelectionMode(grpAllSelection);
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
    mySelectionMode = grpNoSelection;
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
    setSelectionMode(grpGeomSelection);
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

  QColor aQColor = myColorBtn->color();
  if( !isAutoColor )
  {
    if ( !aQColor.isValid() ) {
      int r = 0, g = 0, b = 0;
      SMESH::GetColor( "SMESH", "default_grp_color", r, g, b, QColor( 255, 170, 0 ) );
      aQColor.setRgb( r, g, b );
    }
  }
  else
  {
#ifdef SIMPLE_AUTOCOLOR   // simplified algorithm for auto-colors
    SALOMEDS::Color aColor = SMESHGUI::getPredefinedUniqueColor();
#else                     // old algorithm  for auto-colors
    SMESH::ListOfGroups aListOfGroups = *myMesh->GetGroups();

    QList<SALOMEDS::Color> aReservedColors;
    for( int i = 0, n = aListOfGroups.length(); i < n; i++ )
    {
      SMESH::SMESH_GroupBase_var aGroupObject = aListOfGroups[i];
      SALOMEDS::Color aReservedColor = aGroupObject->GetColor();
      aReservedColors.append( aReservedColor );
    }

    SALOMEDS::Color aColor = SMESHGUI::getUniqueColor( aReservedColors );
#endif                    // SIMPLE_AUTOCOLOR

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
//            if mesh actor is not visible - find any first visible group or sub-mesh
//=================================================================================
bool SMESHGUI_GroupDlg::SetAppropriateActor()
{
  bool isActor = false;
  myActorsList.clear();

  if (myMesh->_is_nil()) return false;

  SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView();

  if (myGrpTypeGroup->checkedId() > 0) {   // try current group on geometry actor
    SMESH_Actor* anActor = 0;
    if (!myGroupOnGeom->_is_nil())
      anActor = SMESH::FindActorByObject(myGroupOnGeom);
    if (!myGroupOnFilter->_is_nil())
      anActor = SMESH::FindActorByObject(myGroupOnFilter);
    if (anActor && anActor->hasIO())
    {
      isActor = true;
      if (aViewWindow && !aViewWindow->isVisible(anActor->getIO()))
        isActor = false;
      else
        myActorsList.append(anActor);
    }
    return anActor;
  }
  else {
    // try mesh actor
    SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh);
    if (anActor && anActor->hasIO()) {
      isActor = true;
      if (aViewWindow && !aViewWindow->isVisible(anActor->getIO()))
        isActor = false;
      else
        myActorsList.append(anActor);
    }

    // try group actor
    SMESH_Actor* aGroupActor = 0;
    if (!isActor && !myGroup->_is_nil()) {
      aGroupActor = SMESH::FindActorByObject(myGroup);
      if (aGroupActor && aGroupActor->hasIO())
        myActorsList.append(aGroupActor);
    }

    // try any visible actor of group or sub-mesh of current mesh
    if (aViewWindow) {
      // mesh entry
      _PTR(SObject) aSObject = SMESH::FindSObject(myMesh);
      if (aSObject) {
        CORBA::String_var meshEntry = aSObject->GetID().c_str();
        int len = strlen(meshEntry);

        // iterate on all actors in current view window, search for
        // any visible actor, that belongs to group or submesh of current mesh
        VTK::ActorCollectionCopy aCopy(aViewWindow->getRenderer()->GetActors());
        vtkActorCollection *aCollection = aCopy.GetActors();
        int nbItems = aCollection->GetNumberOfItems();
        for (int i=0; i<nbItems && !isActor; i++)
        {
          SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(aCollection->GetItemAsObject(i));
          if (anActor && anActor->hasIO()) {
            Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
            if (aViewWindow->isVisible(anIO)) {
              if (anIO->hasEntry() && strncmp(anIO->getEntry(), meshEntry, len) == 0 && !myActorsList.contains(anActor) )
                myActorsList.append(anActor);
            }
          }
        }
      }
    }

    // Show a standalone group if nothing else is visible (IPAL52227)
    if ( myActorsList.count() == 1 &&
         myActorsList[0] == aGroupActor &&
         aViewWindow && !aViewWindow->isVisible(aGroupActor->getIO()))
      SMESH::UpdateView( aViewWindow, SMESH::eDisplay, aGroupActor->getIO()->getEntry() );
  }


  if (myActorsList.count() > 0) {
    QListIterator<SMESH_Actor*> it( myActorsList );
    while ( it.hasNext() ) {
      SMESH_Actor* anActor = it.next();
      if ( IsActorVisible(anActor) )
        anActor->SetPickable(true);
    }
  }

  return ( isActor || (myActorsList.count() > 0) );
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
      case grpNodeSelection:   restoreShowEntityMode();                          break;
      case grp0DSelection:     actor->SetEntityMode( SMESH_Actor::e0DElements ); break;
      case grpBallSelection:   actor->SetEntityMode( SMESH_Actor::eBallElem );   break;
      case grpEdgeSelection:   actor->SetEntityMode( SMESH_Actor::eEdges );      break;
      case grpFaceSelection:   actor->SetEntityMode( SMESH_Actor::eFaces );      break;
      case grpVolumeSelection: actor->SetEntityMode( SMESH_Actor::eVolumes );    break;
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

//=======================================================================
//function : IsActorVisible
//purpose  : return visibility of the actor
//=======================================================================
bool SMESHGUI_GroupDlg::IsActorVisible( SMESH_Actor* theActor )
{
  SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView();
  if (theActor && aViewWindow)
    return aViewWindow->isVisible(theActor->getIO());
  return false;
}

//================================================================
//function : setIsApplyAndClose
//purpose  : Set value of the flag indicating that the dialog is
//           accepted by Apply & Close button
//================================================================
void SMESHGUI_GroupDlg::setIsApplyAndClose( const bool theFlag )
{
  myIsApplyAndClose = theFlag;
}

//================================================================
//function : isApplyAndClose
//purpose  : Get value of the flag indicating that the dialog is
//           accepted by Apply & Close button
//================================================================
bool SMESHGUI_GroupDlg::isApplyAndClose() const
{
  return myIsApplyAndClose;
}
