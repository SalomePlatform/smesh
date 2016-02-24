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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MergeDlg.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MergeDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_IdPreview.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// OCCT includes
#include <TColStd_MapIteratorOfMapOfInteger.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#define SPACING 6
#define MARGIN  11

namespace
{
  enum ActionType { MERGE_NODES, MERGE_ELEMENTS, TYPE_AUTO=0, TYPE_MANUAL };
}


QPixmap SMESHGUI_MergeDlg::IconFirst()
{
  static const char * iconFirst[] = {
    "18 10 2 1",
    "       g None",
    ".      g #000000",
    "         .     .  ",
    "  ..    ..    ..  ",
    "  ..   ...   ...  ",
    "  ..  ....  ....  ",
    "  .. ..... .....  ",
    "  .. ..... .....  ",
    "  ..  ....  ....  ",
    "  ..   ...   ...  ",
    "  ..    ..    ..  ",
    "         .     .  "};
  return QPixmap( iconFirst );
}

//=================================================================================
// class    : SMESHGUI_MergeDlg()
// purpose  :
//=================================================================================
SMESHGUI_MergeDlg::SMESHGUI_MergeDlg (SMESHGUI* theModule, int theAction)
  : QDialog(SMESH::GetDesktop(theModule)),
    mySMESHGUI(theModule),
    mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
    myAction(theAction)
{
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(myAction == MERGE_ELEMENTS ? tr("SMESH_MERGE_ELEMENTS") : tr("SMESH_MERGE_NODES"));

  myIdPreview = new SMESHGUI_IdPreview(SMESH::GetViewWindow( mySMESHGUI ));

  SUIT_ResourceMgr* aResMgr = SMESH::GetResourceMgr( mySMESHGUI );
  // QPixmap IconMergeNodes (aResMgr->loadPixmap("SMESH", tr("ICON_SMESH_MERGE_NODES")));
  // QPixmap IconMergeElems (aResMgr->loadPixmap("SMESH", tr("ICON_DLG_MERGE_ELEMENTS")));
  QPixmap IconSelect     (aResMgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap IconAdd        (aResMgr->loadPixmap("SMESH", tr("ICON_APPEND")));
  QPixmap IconRemove     (aResMgr->loadPixmap("SMESH", tr("ICON_REMOVE")));

  setSizeGripEnabled(true);

  /***************************************************************/
  // Controls to switch dialog behaviour (myTypeId)

  TypeBox   = new QGroupBox( tr("SMESH_MODE"), this );
  GroupType = new QButtonGroup( this );
  QHBoxLayout* aTypeBoxLayout = new QHBoxLayout( TypeBox );
  aTypeBoxLayout->setMargin( MARGIN );
  aTypeBoxLayout->setSpacing( SPACING );

  QRadioButton* rb1 = new QRadioButton( tr( "SMESH_AUTOMATIC" ), TypeBox );
  QRadioButton* rb2 = new QRadioButton( tr( "SMESH_MANUAL" ),   TypeBox );
  GroupType->addButton( rb1, 0 );
  GroupType->addButton( rb2, 1 );
  aTypeBoxLayout->addWidget( rb1 );
  aTypeBoxLayout->addWidget( rb2 );

  myTypeId = TYPE_AUTO;

  /***************************************************************/
  // Controls for mesh defining

  GroupMesh = new QGroupBox(tr("SMESH_SELECT_WHOLE_MESH"), this);
  QHBoxLayout* GroupMeshLayout = new QHBoxLayout(GroupMesh);
  GroupMeshLayout->setSpacing(SPACING);
  GroupMeshLayout->setMargin(MARGIN);

  TextLabelName = new QLabel(tr("SMESH_NAME"), GroupMesh);
  SelectMeshButton = new QPushButton(GroupMesh);
  SelectMeshButton->setIcon(IconSelect);
  LineEditMesh = new QLineEdit(GroupMesh);
  LineEditMesh->setReadOnly(true);

  GroupMeshLayout->addWidget(TextLabelName);
  GroupMeshLayout->addWidget(SelectMeshButton);
  GroupMeshLayout->addWidget(LineEditMesh);

  /***************************************************************/
  // Controls for coincident elements detecting

  GroupCoincident = new QGroupBox(myAction == MERGE_ELEMENTS ?
                                  tr("COINCIDENT_ELEMENTS") :
                                  tr("COINCIDENT_NODES"),
                                  this);

  QGridLayout* aCoincidentLayout = new QGridLayout(GroupCoincident);
  aCoincidentLayout->setSpacing(SPACING);
  aCoincidentLayout->setMargin(MARGIN);

  if (myAction == MERGE_NODES) // case merge nodes
  {
    /***************************************************************/
    // Node specific Controls: tolerance, ...

    NodeSpecWidget = new QWidget( this );

    QLabel* TextLabelTolerance = new QLabel(tr("SMESH_TOLERANCE"), NodeSpecWidget);
    SpinBoxTolerance = new SMESHGUI_SpinBox( NodeSpecWidget );
    SpinBoxTolerance->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    SeparateCornersAndMedium = new QCheckBox(tr("SEPARATE_CORNERS_AND_MEDIUM"), NodeSpecWidget );
    SeparateCornersAndMedium->setEnabled( false );

    QGridLayout* NodeSpecLayout = new QGridLayout(NodeSpecWidget);
    NodeSpecLayout->setSpacing(SPACING);
    NodeSpecLayout->setMargin(0);

    NodeSpecLayout->addWidget(TextLabelTolerance,       0, 0 );
    NodeSpecLayout->addWidget(SpinBoxTolerance,         0, 1 );
    NodeSpecLayout->addWidget(SeparateCornersAndMedium, 1, 0, 1, 2 );

    /***************************************************************/
    // Exclude groups

    GroupExclude = new QGroupBox(tr("EXCLUDE_GROUPS"), this );
    GroupExclude->setCheckable( true );
    GroupExclude->setChecked( false );
    ListExclude = new QListWidget( GroupExclude );
    QVBoxLayout* GroupExcludeLayout = new QVBoxLayout(GroupExclude);
    GroupExcludeLayout->setSpacing(SPACING);
    GroupExcludeLayout->setMargin(MARGIN);
    GroupExcludeLayout->addWidget(ListExclude);

    /***************************************************************/
    // Nodes to keep

    GroupKeep = new QGroupBox(tr("KEEP_NODES"), this);
    SelectKeepNodesButton = new QPushButton( GroupKeep );
    SelectKeepNodesButton->setIcon( IconSelect );
    QLabel*       selectLabel = new QLabel(tr("SELECT"));
    QRadioButton*   idsButton = new QRadioButton(tr("SMESH_NODES"), GroupKeep);
    QRadioButton* groupButton = new QRadioButton(tr("GROUP_SUBMESH"), GroupKeep);
    KeepFromButGroup = new QButtonGroup( this );
    KeepFromButGroup->addButton( idsButton,   0 );
    KeepFromButGroup->addButton( groupButton, 1 );
    groupButton->setChecked( true );
    KeepList = new QListWidget( GroupKeep );
    KeepList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    KeepList->setFlow(QListView::TopToBottom);
    AddKeepNodesButton    = new QPushButton(tr("SMESH_BUT_ADD"), GroupKeep );
    RemoveKeepNodesButton = new QPushButton(tr("SMESH_BUT_REMOVE"), GroupKeep );
    QGridLayout* GroupKeepLayout = new QGridLayout(GroupKeep);
    GroupKeepLayout->setSpacing( SPACING );
    GroupKeepLayout->setMargin ( MARGIN );
    GroupKeepLayout->addWidget( SelectKeepNodesButton, 0, 0 );
    GroupKeepLayout->addWidget( selectLabel,           0, 1 );
    GroupKeepLayout->addWidget( idsButton,             0, 2 );
    GroupKeepLayout->addWidget( groupButton,           0, 3, 1, 2 );
    GroupKeepLayout->addWidget( KeepList,              1, 0, 3, 4 );
    GroupKeepLayout->addWidget( AddKeepNodesButton,    1, 4, 1, 1 );
    GroupKeepLayout->addWidget( RemoveKeepNodesButton, 2, 4, 1, 1 );
    GroupKeepLayout->setRowStretch(3, 5);

    // Costruction of the logical filter
    QList<SUIT_SelectionFilter*> aListOfFilters;
    aListOfFilters << new SMESH_TypeFilter (SMESH::SUBMESH)
                   << new SMESH_TypeFilter (SMESH::GROUP);
    mySubMeshOrGroupFilter =
      new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR, /*takeOwnership=*/true);
  }
  else {
    NodeSpecWidget         = 0;
    SpinBoxTolerance       = 0;
    GroupExclude           = 0;
    ListExclude            = 0;
    KeepFromButGroup       = 0;
    SelectKeepNodesButton  = 0;
    AddKeepNodesButton     = 0;
    RemoveKeepNodesButton  = 0;
    KeepList               = 0;
    mySubMeshOrGroupFilter = 0;
  }

  ListCoincident = new QListWidget(GroupCoincident);
  ListCoincident->setSelectionMode(QListWidget::ExtendedSelection);

  DetectButton      = new QPushButton(tr("DETECT"),           GroupCoincident);
  AddGroupButton    = new QPushButton(tr("SMESH_BUT_ADD"),    GroupCoincident);
  RemoveGroupButton = new QPushButton(tr("SMESH_BUT_REMOVE"), GroupCoincident);

  SelectAllCB = new QCheckBox(tr("SELECT_ALL"), GroupCoincident);
  ShowIDs = new QCheckBox(myAction == MERGE_ELEMENTS ? tr("SHOW_ELEMS_IDS") : tr("SHOW_NODES_IDS"), GroupCoincident);

  aCoincidentLayout->addWidget(ListCoincident,    0, 0, 4, 2);
  aCoincidentLayout->addWidget(DetectButton,      0, 2);
  aCoincidentLayout->addWidget(AddGroupButton,    2, 2);
  aCoincidentLayout->addWidget(RemoveGroupButton, 3, 2);
  aCoincidentLayout->addWidget(SelectAllCB,       4, 0);
  aCoincidentLayout->addWidget(ShowIDs,           4, 1);
  aCoincidentLayout->setRowMinimumHeight(1, 10);
  aCoincidentLayout->setRowStretch(1, 5);

  /***************************************************************/
  // Controls for editing the selected group

  GroupEdit = new QGroupBox(myAction == MERGE_NODES ?
                            tr("EDIT_SELECTED_NODE_GROUP") :
                            tr("EDIT_SELECTED_ELEM_GROUP"), this);
  QGridLayout* GroupEditLayout = new QGridLayout(GroupEdit);
  GroupEditLayout->setSpacing(SPACING);
  GroupEditLayout->setMargin(MARGIN);

  ListEdit = new QListWidget(GroupEdit);
  //ListEdit->setRowMode(QListBox::FixedNumber);
  //ListEdit->setHScrollBarMode(QScrollView::AlwaysOn);
  //ListEdit->setVScrollBarMode(QScrollView::AlwaysOff);
  ListEdit->setFlow( QListView::LeftToRight );
  ListEdit->setSelectionMode(QListWidget::ExtendedSelection);

  AddElemButton = new QPushButton(GroupEdit);
  AddElemButton->setIcon(IconAdd);
  RemoveElemButton = new QPushButton(GroupEdit);
  RemoveElemButton->setIcon(IconRemove);
  SetFirstButton = new QPushButton(GroupEdit);
  SetFirstButton->setIcon(IconFirst());

  GroupEditLayout->addWidget(ListEdit,         0, 0, 2, 1);
  GroupEditLayout->addWidget(AddElemButton,    0, 1);
  GroupEditLayout->addWidget(RemoveElemButton, 0, 2);
  GroupEditLayout->addWidget(SetFirstButton,   1, 1, 1, 2);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  if (myAction == MERGE_NODES)
  {
    QWidget* LeftWdg = new QWidget( this );
    QVBoxLayout* LeftLayout = new QVBoxLayout(LeftWdg);
    LeftLayout->setSpacing(SPACING);
    LeftLayout->setMargin(0);
    LeftLayout->addWidget(TypeBox);
    LeftLayout->addWidget(GroupMesh);
    LeftLayout->addWidget(NodeSpecWidget);
    LeftLayout->addWidget(GroupCoincident);
    LeftLayout->addStretch();
    LeftLayout->addWidget(GroupButtons);
    LeftLayout->setStretch( 3, 10 );

    QWidget* RightWdg = new QWidget( this );
    QVBoxLayout* RightLayout = new QVBoxLayout(RightWdg);
    RightLayout->setSpacing(SPACING);
    RightLayout->setMargin(0);
    RightLayout->addWidget(GroupExclude);
    RightLayout->addWidget(GroupKeep);
    RightLayout->addWidget(GroupEdit);
    RightLayout->setStretch( 0, 4 );
    RightLayout->setStretch( 1, 5 );

    QHBoxLayout* DlgLayout = new QHBoxLayout(this);
    DlgLayout->setSpacing(SPACING*2);
    DlgLayout->setMargin(MARGIN);
    DlgLayout->addWidget( LeftWdg );
    DlgLayout->addWidget( RightWdg );
  }
  else
  {
    QVBoxLayout* DlgLayout = new QVBoxLayout(this);
    DlgLayout->setSpacing(SPACING);
    DlgLayout->setMargin(MARGIN);
    DlgLayout->addWidget(TypeBox);
    DlgLayout->addWidget(GroupMesh);
    DlgLayout->addWidget(GroupCoincident);
    DlgLayout->addWidget(GroupEdit);
    DlgLayout->addWidget(GroupButtons);
  }

  GroupCoincident->hide();
  GroupEdit->hide();

  this->resize(10,10);

  ShowIDs->setChecked( true );

  Init(); // Initialisations
}

//=================================================================================
// function : ~SMESHGUI_MergeDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_MergeDlg::~SMESHGUI_MergeDlg()
{
  delete myIdPreview;
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::Init()
{
  if ( myAction == MERGE_NODES ) {
    SpinBoxTolerance->RangeStepAndValidator(0.0, COORD_MAX, 0.00001, "len_tol_precision");
    SpinBoxTolerance->SetValue(1e-05);
  }

  GroupType->button(0)->setChecked(true);

  myEditCurrentArgument = (QWidget*)LineEditMesh;

  myActor = 0;
  mySubMeshOrGroup = SMESH::SMESH_subMesh::_nil();

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myIsBusy = false;

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  if ( KeepList )
  {
    connect(SelectKeepNodesButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
    connect(KeepFromButGroup, SIGNAL (buttonClicked(int)), SLOT(onKeepNodeSourceChanged(int)));
    connect(AddKeepNodesButton, SIGNAL (clicked()), this, SLOT(onAddKeepNode()));
    connect(RemoveKeepNodesButton, SIGNAL (clicked()), this, SLOT(onRemoveKeepNode()));
    connect(KeepList, SIGNAL (itemSelectionChanged()), this, SLOT(onSelectKeepNode()));
  }
  connect(SelectMeshButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(DetectButton, SIGNAL (clicked()), this, SLOT(onDetect()));
  connect(ListCoincident, SIGNAL (itemSelectionChanged()), this, SLOT(onSelectGroup()));
  connect(AddGroupButton, SIGNAL (clicked()), this, SLOT(onAddGroup()));
  connect(RemoveGroupButton, SIGNAL (clicked()), this, SLOT(onRemoveGroup()));
  connect(SelectAllCB, SIGNAL(toggled(bool)), this, SLOT(onSelectAll(bool)));
  connect(ShowIDs, SIGNAL(toggled(bool)), this, SLOT(onSelectGroup()));
  connect(ListEdit, SIGNAL (itemSelectionChanged()), this, SLOT(onSelectElementFromGroup()));
  connect(AddElemButton, SIGNAL (clicked()), this, SLOT(onAddElement()));
  connect(RemoveElemButton, SIGNAL (clicked()), this, SLOT(onRemoveElement()));
  connect(SetFirstButton, SIGNAL( clicked() ), this, SLOT( onSetFirst()));
  connect(GroupType, SIGNAL(buttonClicked(int)), this, SLOT(onTypeChanged(int)));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(reject()));
  connect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this,  SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL (SignalCloseView()), this, SLOT(onCloseView()));
  // Init Mesh field from selection
  SelectionIntoArgument();

  // Update Buttons
  updateControls();
  
  if ( myAction == MERGE_NODES )
    myHelpFileName = "merging_nodes_page.html";
  else
    myHelpFileName = "merging_elements_page.html";
}

//=================================================================================
// function : FindGravityCenter()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::FindGravityCenter(TColStd_MapOfInteger & theElemsIdMap,
                                          std::vector<int>&      theIDs,
                                          std::list< gp_XYZ > &  theGrCentersXYZ)
{
  if (!myActor)
    return;

  SMDS_Mesh* aMesh = 0;
  aMesh = myActor->GetObject()->GetMesh();
  if (!aMesh)
    return;

  int nbNodes;

  theIDs.reserve( theElemsIdMap.Extent() );
  TColStd_MapIteratorOfMapOfInteger idIter( theElemsIdMap );
  for( ; idIter.More(); idIter.Next() ) {
    const SMDS_MeshElement* anElem = aMesh->FindElement(idIter.Key());
    if ( !anElem )
      continue;
    theIDs.push_back( idIter.Key() );

    gp_XYZ anXYZ(0., 0., 0.);
    SMDS_ElemIteratorPtr nodeIt = anElem->nodesIterator();
    for ( nbNodes = 0; nodeIt->more(); nbNodes++ ) {
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
      anXYZ.Add( gp_XYZ( node->X(), node->Y(), node->Z() ) );
    }
    anXYZ.Divide( nbNodes );
    
    theGrCentersXYZ.push_back( anXYZ );
  }
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_MergeDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked() || myMesh->_is_nil())
    return false;

  try {
    if (myTypeId == TYPE_AUTO)
      onDetect();

    SUIT_OverrideCursor aWaitCursor;
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

    SMESH::long_array_var anIds = new SMESH::long_array;
    SMESH::array_of_long_array_var aGroupsOfElements = new SMESH::array_of_long_array;

    if ( ListCoincident->count() == 0) {
      if ( myAction == MERGE_NODES )
        SUIT_MessageBox::warning(this,
                                 tr("SMESH_WARNING"),
                                 tr("SMESH_NO_NODES_DETECTED"));
      else
        SUIT_MessageBox::warning(this,
                                 tr("SMESH_WARNING"),
                                 tr("SMESH_NO_ELEMENTS_DETECTED"));
      return false;
    }

    aGroupsOfElements->length(ListCoincident->count());

    int anArrayNum = 0;
    for (int i = 0; i < ListCoincident->count(); i++) {
      QStringList aListIds = ListCoincident->item(i)->text().split(" ", QString::SkipEmptyParts);

      anIds->length(aListIds.count());
      for (int i = 0; i < aListIds.count(); i++)
        anIds[i] = aListIds[i].toInt();

      aGroupsOfElements[anArrayNum++] = anIds.inout();
    }

    SMESH::ListOfIDSources_var nodesToKeep;
    SMESH::IDSource_wrap tmpIdSource;
    if ( myAction == MERGE_NODES )
    {
      nodesToKeep = new SMESH::ListOfIDSources();
      int i, nb = KeepList->count();
      if ( isKeepNodesIDsSelection() )
      {
        SMESH::long_array_var anIdList = new SMESH::long_array();
        anIdList->length(nb);
        for (i = 0; i < nb; i++)
          anIdList[i] = KeepList->item(i)->text().toInt();

        if ( nb > 0 )
        {
          tmpIdSource = aMeshEditor->MakeIDSource( anIdList, SMESH::NODE );
          nodesToKeep->length( 1 );
          nodesToKeep[0] = SMESH::SMESH_IDSource::_duplicate( tmpIdSource.in() );
        }
      }
      else
      {
        nodesToKeep->length( nb );
        int nbObj = 0;
        for (i = 0; i < nb; i++)
        {
          QString entry = KeepList->item( i )->data( Qt::UserRole ).toString();
          Handle(SALOME_InteractiveObject) anIO =
            new SALOME_InteractiveObject( entry.toStdString().c_str(), "SMESH" );
          SMESH::SMESH_IDSource_var idSrc =
            SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( anIO );
          if ( !idSrc->_is_nil() )
            nodesToKeep[ nbObj++ ] = SMESH::SMESH_IDSource::_duplicate( idSrc );
        }
        nodesToKeep->length( nbObj );
      }
      KeepList->clear();
    }

    if( myAction == MERGE_NODES )
      aMeshEditor->MergeNodes (aGroupsOfElements.inout(), nodesToKeep);
    else
      aMeshEditor->MergeElements (aGroupsOfElements.inout());

    if ( myTypeId == TYPE_AUTO ) {
      if (myAction == MERGE_NODES )
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INFORMATION"),
                                     tr("SMESH_MERGED_NODES").arg(QString::number(ListCoincident->count()).toLatin1().data()));
      else
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INFORMATION"),
                                     tr("SMESH_MERGED_ELEMENTS").arg(QString::number(ListCoincident->count()).toLatin1().data()));
    }
    if ( & nodesToKeep.in() )
      nodesToKeep->length(0); // release before tmpIdSource calls UnRegister()

  }
  catch(...) {
  }

  ListCoincident->clear();

  myEditCurrentArgument = (QWidget*)LineEditMesh;

  SMESH::UpdateView();
  SMESHGUI::Modified();

  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::ClickOnOk()
{
  if (ClickOnApply())
    reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::reject()
{
  myIdPreview->SetPointsLabeled(false);
  SMESH::SetPointRepresentation(false);
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();

  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);

  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::ClickOnHelp()
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
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser", 
                                                                 platform)).
                             arg(myHelpFileName));
  }
}

//=================================================================================
// function : onEditGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onEditGroup()
{
  QList<QListWidgetItem*> selItems = ListCoincident->selectedItems();
  if ( selItems.count() != 1 ) {
    ListEdit->clear();
    return;
  }

  QStringList aNewIds;

  for (int i = 0; i < ListEdit->count(); i++ )
    aNewIds.append(ListEdit->item(i)->text());

  ListCoincident->clearSelection();
  selItems.first()->setText(aNewIds.join(" "));
  selItems.first()->setSelected(true);
}

//=================================================================================
// function : updateControls()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::updateControls()
{
  if (ListEdit->count() == 0)
    SetFirstButton->setEnabled(false);

  bool groupsEmpty = ( myTypeId != TYPE_AUTO );
  for (int i = 0; i < ListCoincident->count() && groupsEmpty; i++) {
    QStringList aListIds = ListCoincident->item(i)->text().split(" ", QString::SkipEmptyParts);
    groupsEmpty = ( aListIds.count() < 2 );
  }
  bool enable = ( !myMesh->_is_nil() && !groupsEmpty );
  buttonOk->setEnabled(enable);
  buttonApply->setEnabled(enable);
  DetectButton->setEnabled( !myMesh->_is_nil() );

  if ( myAction == MERGE_NODES )
  {
    bool has2ndOrder = (( !myMesh->_is_nil() ) &&
                        ( myMesh->NbEdgesOfOrder( SMESH::ORDER_QUADRATIC ) > 0 ||
                          myMesh->NbFacesOfOrder( SMESH::ORDER_QUADRATIC ) > 0 ||
                          myMesh->NbVolumesOfOrder( SMESH::ORDER_QUADRATIC ) > 0 ));

    SeparateCornersAndMedium->setEnabled( has2ndOrder );

    if ( myEditCurrentArgument != KeepList )
    {
      AddKeepNodesButton->setEnabled( false );
      RemoveKeepNodesButton->setEnabled( false );
      KeepList->clearSelection();
    }
  }
}

//=================================================================================
// function : onDetect()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onDetect()
{
  if ( myMesh->_is_nil() || LineEditMesh->text().isEmpty() )
    return;

  try {
    SUIT_OverrideCursor aWaitCursor;
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

    ListCoincident->clear();
    ListEdit->clear();

    SMESH::array_of_long_array_var aGroupsArray;
    SMESH::ListOfIDSources_var aExcludeGroups = new SMESH::ListOfIDSources;

    SMESH::SMESH_IDSource_var src;
    if ( mySubMeshOrGroup->_is_nil() ) src = SMESH::SMESH_IDSource::_duplicate( myMesh );
    else                               src = SMESH::SMESH_IDSource::_duplicate( mySubMeshOrGroup );

    switch (myAction) {
    case MERGE_NODES :
      for ( int i = 0; GroupExclude->isChecked() && i < ListExclude->count(); i++ ) {
        if ( ListExclude->item( i )->checkState() == Qt::Checked ) {
          aExcludeGroups->length( aExcludeGroups->length()+1 );
          aExcludeGroups[ aExcludeGroups->length()-1 ] = SMESH::SMESH_IDSource::_duplicate( myGroups[i] );
        }
      }
      aMeshEditor->FindCoincidentNodesOnPartBut(src.in(),
                                                SpinBoxTolerance->GetValue(), 
                                                aGroupsArray.out(),
                                                aExcludeGroups.in(),
                                                SeparateCornersAndMedium->isEnabled() &&
                                                SeparateCornersAndMedium->isChecked());
      break;
    case MERGE_ELEMENTS :
      aMeshEditor->FindEqualElements(src.in(), aGroupsArray.out());
      break;
    }
    
    for (int i = 0; i < (int)aGroupsArray->length(); i++) {
      SMESH::long_array& aGroup = aGroupsArray[i];

      QStringList anIDs;
      for (int j = 0; j < (int)aGroup.length(); j++)
        anIDs.append(QString::number(aGroup[j]));

      ListCoincident->addItem(anIDs.join(" "));
    }
  } catch(...) {
  }

  ListCoincident->selectAll();
  updateControls();
  SMESH::UpdateView();
}

//=================================================================================
// function : onSelectGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onSelectGroup()
{
  if (myIsBusy || !myActor)
    return;

  if( ListCoincident->count() != ListCoincident->selectedItems().count() )
    SelectAllCB->setChecked( false );

  if ( myEditCurrentArgument == (QWidget*)KeepList && KeepList &&
       !isKeepNodesIDsSelection() )
  {
    // restore selection of nodes after selection of sub-meshes
    mySelectionMgr->clearFilters();
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( NodeSelection );
    SMESH::SetPointRepresentation( true );
    myEditCurrentArgument = ListCoincident;
  }

  myEditCurrentArgument = (QWidget*)ListCoincident;

  myIsBusy = true;
  ListEdit->clear();

  TColStd_MapOfInteger anIndices;
  QList<QListWidgetItem*> selItems = ListCoincident->selectedItems();
  QListWidgetItem* anItem;
  QStringList aListIds;

  ListEdit->clear();

  foreach(anItem, selItems) {
    aListIds = anItem->text().split(" ", QString::SkipEmptyParts);
    for (int i = 0; i < aListIds.count(); i++)
      anIndices.Add(aListIds[i].toInt());
  }

  if (selItems.count() == 1) {
    ListEdit->addItems(aListIds);
    ListEdit->selectAll();
  }

  mySelector->AddOrRemoveIndex(myActor->getIO(), anIndices, false);
  SALOME_ListIO aList;
  aList.Append(myActor->getIO());
  mySelectionMgr->setSelectedObjects(aList,false);
  
  if (ShowIDs->isChecked()) 
    if ( myAction == MERGE_NODES ) {
      myIdPreview->SetPointsData(myActor->GetObject()->GetMesh(), anIndices);
      myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
    }
    else {
      std::list< gp_XYZ > aGrCentersXYZ;
      std::vector<int>    anIDs;
      FindGravityCenter(anIndices, anIDs, aGrCentersXYZ);
      myIdPreview->SetElemsData( anIDs, aGrCentersXYZ );
      myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
    }
  else
    myIdPreview->SetPointsLabeled(false);

  updateControls();
  myIsBusy = false;
}

//=================================================================================
// function : onSelectAll()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onSelectAll (bool isToggled)
{
  if ( isToggled )
    ListCoincident->selectAll();
  else
    ListCoincident->clearSelection();
}

//=================================================================================
// function : onSelectElementFromGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onSelectElementFromGroup()
{
  if (myIsBusy || !myActor)
    return;

  TColStd_MapOfInteger anIndices;
  QList<QListWidgetItem*> selItems = ListEdit->selectedItems();
  QListWidgetItem* anItem;

  foreach(anItem, selItems)
    anIndices.Add(anItem->text().toInt());

  SetFirstButton->setEnabled(selItems.count() == 1);

  mySelector->AddOrRemoveIndex(myActor->getIO(), anIndices, false);
  SALOME_ListIO aList;
  aList.Append(myActor->getIO());
  mySelectionMgr->setSelectedObjects(aList);
  
  if (ShowIDs->isChecked())
    if (myAction == MERGE_NODES) {
      myIdPreview->SetPointsData(myActor->GetObject()->GetMesh(), anIndices);
      myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
    }
    else {
      std::list< gp_XYZ > aGrCentersXYZ;
      std::vector<int>    anIDs;
      FindGravityCenter(anIndices, anIDs, aGrCentersXYZ);
      myIdPreview->SetElemsData(anIDs, aGrCentersXYZ);
      myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
    }
  else 
    myIdPreview->SetPointsLabeled(false);

  if ( myEditCurrentArgument == (QWidget*)KeepList && KeepList &&
       !isKeepNodesIDsSelection() )
  {
    // restore selection of nodes after selection of sub-meshes
    mySelectionMgr->clearFilters();
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( NodeSelection );
    SMESH::SetPointRepresentation( true );
    myEditCurrentArgument = ListCoincident;
  }
}

//=================================================================================
// function : onAddGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onAddGroup()
{
  if ( myMesh->_is_nil() || LineEditMesh->text().isEmpty() )
    return;

  QString anIDs = "";
  int aNbElements = 0;
  aNbElements = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), anIDs);

  if (aNbElements < 1)
    return;
  
  ListCoincident->clearSelection();
  ListCoincident->addItem(anIDs);
  int nbGroups = ListCoincident->count();
  if (nbGroups) {
    ListCoincident->setCurrentRow(nbGroups-1);
    ListCoincident->item(nbGroups-1)->setSelected(true);
  }
  else {
    // VSR ? this code seems to be never executed!!!
    ListCoincident->setCurrentRow(0);
    //ListCoincident->setSelected(0, true); // VSR: no items - no selection
  }

  updateControls();
}

//=================================================================================
// function : onRemoveGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onRemoveGroup()
{
  if (myEditCurrentArgument != (QWidget*)ListCoincident)
    return;
  myIsBusy = true;

  QList<QListWidgetItem*> selItems = ListCoincident->selectedItems();
  QListWidgetItem* anItem;

  foreach(anItem, selItems)
    delete anItem;

  ListEdit->clear();
  myIdPreview->SetPointsLabeled(false);
  updateControls();
  SMESH::UpdateView();
  myIsBusy = false;

  if( ListCoincident->count() == 0 ) {
    myEditCurrentArgument = (QWidget*)LineEditMesh;
    SelectAllCB->setChecked( false );
  }
}

//=================================================================================
// function : onAddElement()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onAddElement()
{
  if (!myActor)
    return;
  myIsBusy = true;

  QString aListStr = "";
  int aNbNnodes = 0;

  aNbNnodes = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aListStr);
  if (aNbNnodes < 1)
    return;

  QStringList aNodes = aListStr.split(" ", QString::SkipEmptyParts);

  for (QStringList::iterator it = aNodes.begin(); it != aNodes.end(); ++it) {
    QList<QListWidgetItem*> found = ListEdit->findItems(*it, Qt::MatchExactly);
    if ( found.count() == 0 ) {
      QListWidgetItem* anItem = new QListWidgetItem(*it);
      ListEdit->addItem(anItem);
      anItem->setSelected(true);
    }
    else {
      QListWidgetItem* anItem;
      foreach(anItem, found) anItem->setSelected(true);
    }
  }

  myIsBusy = false;
  onEditGroup();
}

//=================================================================================
// function : onRemoveElement()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onRemoveElement()
{
  if (myEditCurrentArgument != (QWidget*)ListCoincident)
    return;
  myIsBusy = true;

  QList<QListWidgetItem*> selItems = ListEdit->selectedItems();
  QListWidgetItem* anItem;

  foreach(anItem, selItems)
    delete anItem;
  
  myIsBusy = false;
  onEditGroup();

  if( ListCoincident->count() == 0 ) {
    myEditCurrentArgument = (QWidget*)LineEditMesh;
    SelectAllCB->setChecked( false );
  }
}

//=================================================================================
// function : onSetFirst()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onSetFirst()
{
  if (myEditCurrentArgument != (QWidget*)ListCoincident)
    return;
  myIsBusy = true;
  
  QList<QListWidgetItem*> selItems = ListEdit->selectedItems();
  QListWidgetItem* anItem;
  
  foreach(anItem, selItems) {
    ListEdit->takeItem(ListEdit->row(anItem));
    ListEdit->insertItem(0, anItem);
  }

  myIsBusy = false;
  onEditGroup();
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  if (send == SelectMeshButton)
  {
    myEditCurrentArgument = (QWidget*)LineEditMesh;
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    if (myTypeId == TYPE_MANUAL)
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
  }
  else if ( send == SelectKeepNodesButton && send )
  {
    myEditCurrentArgument = (QWidget*)KeepList;
    KeepList->setWrapping( isKeepNodesIDsSelection() );
    if ( isKeepNodesIDsSelection() )
    {
      SMESH::SetPointRepresentation( true );
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode( NodeSelection );
    }
    else
    {
      SMESH::SetPointRepresentation( false );
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode( ActorSelection );
      mySelectionMgr->installFilter( mySubMeshOrGroupFilter );
    }
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed or other case
//=================================================================================
void SMESHGUI_MergeDlg::SelectionIntoArgument()
{
  if (myEditCurrentArgument == (QWidget*)LineEditMesh)
  {
    QString aString = "";
    LineEditMesh->setText(aString);

    ListCoincident->clear();
    ListEdit->clear();
    myActor = 0;
    myMesh = SMESH::SMESH_Mesh::_nil();
    QString aCurrentEntry = myEntry;

    int nbSel = SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
    if (nbSel != 1) {
      myIdPreview->SetPointsLabeled(false);
      SMESH::SetPointRepresentation(false);
      mySelectionMgr->clearFilters();
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(ActorSelection);
      return;
    }

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList);

    Handle(SALOME_InteractiveObject) IO = aList.First();
    myEntry = IO->getEntry();
    myMesh = SMESH::GetMeshByIO(IO);

    if ( myEntry != aCurrentEntry && KeepList )
      KeepList->clear();

    if (myMesh->_is_nil())
      return;

    LineEditMesh->setText(aString);

    myActor = SMESH::FindActorByEntry(IO->getEntry());
    if (!myActor)
      myActor = SMESH::FindActorByObject(myMesh);

    if ( myActor && myTypeId == TYPE_MANUAL && mySelector->IsSelectionEnabled() ) {
      mySubMeshOrGroup = SMESH::SMESH_IDSource::_nil();
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);

      if ((!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil() || //SUBMESH OR GROUP
           !SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO)->_is_nil()) &&
          !SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO)->_is_nil())
        mySubMeshOrGroup = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);

      if (myAction == MERGE_NODES) {
        SMESH::SetPointRepresentation(true);
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(NodeSelection);
      }
      else
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(CellSelection);
    }

    // process groups
    if ( myAction == MERGE_NODES && !myMesh->_is_nil() && myEntry != aCurrentEntry ) {
      myGroups.clear();
      ListExclude->clear();
      SMESH::ListOfGroups_var aListOfGroups = myMesh->GetGroups();
      GroupExclude->setEnabled( aListOfGroups->length() > 0 );
      for( int i = 0, n = aListOfGroups->length(); i < n; i++ ) {
        SMESH::SMESH_GroupBase_var aGroup = aListOfGroups[i];
        if ( !aGroup->_is_nil() ) { // && aGroup->GetType() == SMESH::NODE
          QString aGroupName( aGroup->GetName() );
          if ( !aGroupName.isEmpty() ) {
            myGroups.append(SMESH::SMESH_GroupBase::_duplicate(aGroup));
            QListWidgetItem* item = new QListWidgetItem( aGroupName );
            item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
            item->setCheckState( Qt::Unchecked );
            ListExclude->addItem( item );
          }
        }
      }
    }

    updateControls();
  }

  else if (myEditCurrentArgument == (QWidget*)KeepList && KeepList)
  {
    AddKeepNodesButton->setEnabled( false );
    RemoveKeepNodesButton->setEnabled( false );
    if ( isKeepNodesIDsSelection() )
    {
      if (!myMesh->_is_nil() && !myActor)
        myActor = SMESH::FindActorByObject(myMesh);

      if ( mySelector && myActor )
      {
        KeepList->clearSelection();
        QString anIDs = "";
        int aNbNodes = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), anIDs);
        if (aNbNodes > 0)
        {
          QStringList anNodes = anIDs.split( " ", QString::SkipEmptyParts);
          QList<QListWidgetItem*> listItemsToSel;
          QListWidgetItem* anItem;
          int nbFound = 0;
          for (QStringList::iterator it = anNodes.begin(); it != anNodes.end(); ++it)
          {
            QList<QListWidgetItem*> found = KeepList->findItems(*it, Qt::MatchExactly);
            foreach(anItem, found)
              if (!anItem->isSelected())
                listItemsToSel.push_back(anItem);
            nbFound += found.count();
          }
          bool blocked = KeepList->signalsBlocked();
          KeepList->blockSignals(true);
          foreach(anItem, listItemsToSel) anItem->setSelected(true);
          KeepList->blockSignals(blocked);
          //onSelectKeepNode();
          AddKeepNodesButton->setEnabled( nbFound < aNbNodes );
          RemoveKeepNodesButton->setEnabled( nbFound > 0 );
        }
      }
    }
    else if ( !myMesh->_is_nil() )
    {
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects(aList);
      bool hasNewSelected = false;
      SALOME_ListIteratorOfListIO anIt (aList);
      for ( ; anIt.More() && !hasNewSelected; anIt.Next())
        if ( anIt.Value()->hasEntry() )
          hasNewSelected = isNewKeepNodesGroup( anIt.Value()->getEntry() );

      AddKeepNodesButton->setEnabled( hasNewSelected );
      //RemoveKeepNodesButton->setEnabled( KeepList->selectedItems().count() );
    }
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::DeactivateActiveDialog()
{
  if (TypeBox->isEnabled()) {
    TypeBox->setEnabled(false);
    GroupMesh->setEnabled(false);
    GroupCoincident->setEnabled(false);
    GroupEdit->setEnabled(false);
    GroupButtons->setEnabled(false);
    if (myAction == MERGE_NODES)
    {
      GroupExclude->setEnabled(false);
      GroupKeep->setEnabled(false);
    }
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }

  mySelectionMgr->clearSelected();
  disconnect(mySelectionMgr, 0, this, 0);
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  TypeBox->setEnabled(true);
  GroupMesh->setEnabled(true);
  GroupCoincident->setEnabled(true);
  GroupEdit->setEnabled(true);
  GroupButtons->setEnabled(true);
  if (myAction == MERGE_NODES)
  {
    GroupExclude->setEnabled(false);
    GroupKeep->setEnabled(false);
  }

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::enterEvent (QEvent*)
{
  if ( !TypeBox->isEnabled() ) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    ActivateThisDialog();
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::keyPressEvent( QKeyEvent* e)
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}

//=================================================================================
// function : onTypeChanged()
// purpose  : the type radio button management
//=================================================================================
void SMESHGUI_MergeDlg::onTypeChanged (int id)
{
  if (myTypeId == id)
    return;

  myTypeId = id;
  switch (id)
  {
  case TYPE_AUTO: // automatic

    myIdPreview->SetPointsLabeled(false);
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->clearFilters();
    GroupCoincident->hide();
    GroupEdit->hide();

    GroupMesh->hide(); // <--- a trick to make the dialog take a minimal size
    GroupMesh->show();
    break;

  case TYPE_MANUAL: // manual

    SMESH::UpdateView();

    // Costruction of the logical filter
    SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (SMESH::MESHorSUBMESH);
    SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (SMESH::GROUP);
    
    QList<SUIT_SelectionFilter*> aListOfFilters;
    if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
    if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);
    
    myMeshOrSubMeshOrGroupFilter =
      new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR, true);

    if (myAction == MERGE_NODES) {
      SMESH::SetPointRepresentation(true);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        if( mySelector->IsSelectionEnabled() )
          aViewWindow->SetSelectionMode(NodeSelection);
    }
    else {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        if( mySelector->IsSelectionEnabled() )
          aViewWindow->SetSelectionMode(CellSelection);
    }
    GroupCoincident->show();
    GroupEdit->show();
    break;
  }
  SelectionIntoArgument();

  updateControls();

  qApp->processEvents();
  updateGeometry();
  resize(10,10);
}

//=======================================================================
//function : isKeepNodesIDsSelection
//purpose  : Return true of Nodes to keep are selected by IDs
//=======================================================================

bool SMESHGUI_MergeDlg::isKeepNodesIDsSelection()
{
  return KeepFromButGroup && KeepFromButGroup->checkedId() == 0;
}

//=======================================================================
//function : isNewKeepNodesGroup
//purpose  : Return true if an object with given entry is NOT present in KeepList
//=======================================================================

bool SMESHGUI_MergeDlg::isNewKeepNodesGroup( const char* entry )
{
  if ( !entry || isKeepNodesIDsSelection() )
    return false;

  for ( int i = 0; i < KeepList->count(); i++ )
    if ( KeepList->item( i )->data( Qt::UserRole ).toString() == entry )
      return false;

  return true;
}

//=======================================================================
//function : onAddKeepNode
//purpose  : SLOT called when [Add] of Nodes To Keep group is pressed
//=======================================================================

void SMESHGUI_MergeDlg::onAddKeepNode()
{
  if ( myIsBusy )
    return;
  myIsBusy = true;

  if ( isKeepNodesIDsSelection() )
  {
    //KeepList->clearSelection();
    QString anIDs = "";
    int aNbNodes = 0;
    if ( myActor )
      aNbNodes = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), anIDs);
    if (aNbNodes > 0)
    {
      QStringList anNodes = anIDs.split( " ", QString::SkipEmptyParts);
      QList<QListWidgetItem*> listItemsToSel;
      QListWidgetItem* anItem;
      for (QStringList::iterator it = anNodes.begin(); it != anNodes.end(); ++it)
      {
        QList<QListWidgetItem*> found = KeepList->findItems(*it, Qt::MatchExactly);
        if (found.count() == 0) {
          anItem = new QListWidgetItem(*it);
          KeepList->addItem(anItem);
          if (!anItem->isSelected())
            listItemsToSel.push_back(anItem);
        }
        else {
          foreach(anItem, found)
            if (!anItem->isSelected())
              listItemsToSel.push_back(anItem);
        }
      }
      bool blocked = KeepList->signalsBlocked();
      KeepList->blockSignals(true);
      foreach(anItem, listItemsToSel) anItem->setSelected(true);
      KeepList->blockSignals(blocked);
      //onSelectKeepNode();
    }
    RemoveKeepNodesButton->setEnabled( aNbNodes > 0 );
  }
  else
  {
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList);
    SALOME_ListIteratorOfListIO anIt (aList);
    for ( ; anIt.More(); anIt.Next()) {
      Handle(SALOME_InteractiveObject) anIO = anIt.Value();
      if ( isNewKeepNodesGroup( anIO->getEntry() ))
      {
        QListWidgetItem* anItem = new QListWidgetItem( anIO->getName() );
        anItem->setData( Qt::UserRole, QString( anIO->getEntry() ));
        KeepList->addItem(anItem);
      }
    }
    //RemoveKeepNodesButton->setEnabled( KeepList->selectedItems().count() );
  }

  AddKeepNodesButton->setEnabled( false );

  myIsBusy = false;
}

//=======================================================================
//function : onRemoveKeepNode
//purpose  : SLOT called when [Remove] of Nodes To Keep group is pressed
//=======================================================================

void SMESHGUI_MergeDlg::onRemoveKeepNode()
{
  // if ( isKeepNodesIDsSelection() )
  // {
  // }
  // else
  {
    QList<QListWidgetItem*> selItems = KeepList->selectedItems();
    QListWidgetItem* item;
    foreach(item, selItems) delete item;
  }
  if ( isKeepNodesIDsSelection() )
  {
    AddKeepNodesButton->setEnabled( false );
  }
  RemoveKeepNodesButton->setEnabled( false );
}

//=======================================================================
//function : onSelectKeepNode
//purpose  : SLOT called when selection in KeepList changes
//=======================================================================

void SMESHGUI_MergeDlg::onSelectKeepNode()
{
  if ( myIsBusy || !isEnabled() ) return;
  myIsBusy = true;

  if ( isKeepNodesIDsSelection() )
  {
    if ( myActor )
    {
      mySelectionMgr->clearSelected();
      TColStd_MapOfInteger aIndexes;
      QList<QListWidgetItem*> selItems = KeepList->selectedItems();
      QListWidgetItem* anItem;
      foreach(anItem, selItems) aIndexes.Add(anItem->text().toInt());
      mySelector->AddOrRemoveIndex(myActor->getIO(), aIndexes, false);
      SALOME_ListIO aList;
      aList.Append(myActor->getIO());
      mySelectionMgr->setSelectedObjects(aList,false);

      AddKeepNodesButton->setEnabled( false );
      RemoveKeepNodesButton->setEnabled( aIndexes.Extent() > 0 );
    }
  }
  else
  {
    RemoveKeepNodesButton->setEnabled( KeepList->selectedItems().count() );
  }
  myIsBusy = false;
}

//=======================================================================
//function : onKeepNodeSourceChanged
//purpose  : SLOT called when type of source of Nodes To Keep change from
//           IDs to groups or vice versa
//=======================================================================

void SMESHGUI_MergeDlg::onKeepNodeSourceChanged(int isGroup)
{
  KeepList->clear();
  SelectKeepNodesButton->click();
}
