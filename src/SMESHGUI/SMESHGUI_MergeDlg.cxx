// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"

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
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

// VTK includes
#include <vtkUnstructuredGrid.h>
#include <vtkRenderer.h>
#include <vtkActor2D.h>
#include <vtkPoints.h>
#include <vtkDataSetMapper.h>
#include <vtkMaskPoints.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkLabeledDataMapper.h>
#include <vtkTextProperty.h>
#include <vtkIntArray.h>
#include <vtkProperty2D.h>
#include <vtkPointData.h>
#include <vtkConfigure.h>
#if !defined(VTK_XVERSION)
#define VTK_XVERSION (VTK_MAJOR_VERSION<<16)+(VTK_MINOR_VERSION<<8)+(VTK_BUILD_VERSION)
#endif

// Qt includes
#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QButtonGroup>

#define SPACING 6
#define MARGIN  11

namespace SMESH
{
  class TIdPreview
  { // to display in the viewer IDs of the selected elements
    SVTK_ViewWindow* myViewWindow;

    vtkUnstructuredGrid* myIdGrid;
    SALOME_Actor* myIdActor;

    vtkUnstructuredGrid* myPointsNumDataSet;
    vtkMaskPoints* myPtsMaskPoints;
    vtkSelectVisiblePoints* myPtsSelectVisiblePoints;
    vtkLabeledDataMapper* myPtsLabeledDataMapper;
    vtkTextProperty* aPtsTextProp;
    bool myIsPointsLabeled;
    vtkActor2D* myPointLabels;

    std::vector<int> myIDs;

  public:
    TIdPreview(SVTK_ViewWindow* theViewWindow):
      myViewWindow(theViewWindow)
    {
      myIdGrid = vtkUnstructuredGrid::New();

      // Create and display actor
      vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
      aMapper->SetInput( myIdGrid );

      myIdActor = SALOME_Actor::New();
      myIdActor->SetInfinitive(true);
      myIdActor->VisibilityOff();
      myIdActor->PickableOff();

      myIdActor->SetMapper( aMapper );
      aMapper->Delete();

      myViewWindow->AddActor(myIdActor);

      //Definition of points numbering pipeline
      myPointsNumDataSet = vtkUnstructuredGrid::New();

      myPtsMaskPoints = vtkMaskPoints::New();
      myPtsMaskPoints->SetInput(myPointsNumDataSet);
      myPtsMaskPoints->SetOnRatio(1);

      myPtsSelectVisiblePoints = vtkSelectVisiblePoints::New();
      myPtsSelectVisiblePoints->SetInput(myPtsMaskPoints->GetOutput());
      myPtsSelectVisiblePoints->SelectInvisibleOff();
      myPtsSelectVisiblePoints->SetTolerance(0.1);
    
      myPtsLabeledDataMapper = vtkLabeledDataMapper::New();
      myPtsLabeledDataMapper->SetInput(myPtsSelectVisiblePoints->GetOutput());
#if (VTK_XVERSION < 0x050200)
      myPtsLabeledDataMapper->SetLabelFormat("%g");
#endif
      myPtsLabeledDataMapper->SetLabelModeToLabelScalars();
    
      vtkTextProperty* aPtsTextProp = vtkTextProperty::New();
      aPtsTextProp->SetFontFamilyToTimes();
      static int aPointsFontSize = 12;
      aPtsTextProp->SetFontSize(aPointsFontSize);
      aPtsTextProp->SetBold(1);
      aPtsTextProp->SetItalic(0);
      aPtsTextProp->SetShadow(0);
      myPtsLabeledDataMapper->SetLabelTextProperty(aPtsTextProp);
      aPtsTextProp->Delete();
  
      myIsPointsLabeled = false;

      myPointLabels = vtkActor2D::New();
      myPointLabels->SetMapper(myPtsLabeledDataMapper);
      myPointLabels->GetProperty()->SetColor(1,1,1);
      myPointLabels->SetVisibility(myIsPointsLabeled);

      AddToRender(myViewWindow->getRenderer());
    }

    void SetPointsData ( SMDS_Mesh* theMesh, 
                         TColStd_MapOfInteger & theNodesIdMap )
    {
      vtkPoints* aPoints = vtkPoints::New();
      aPoints->SetNumberOfPoints(theNodesIdMap.Extent());
      myIDs.clear();
      
      TColStd_MapIteratorOfMapOfInteger idIter( theNodesIdMap );
      for( int i = 0; idIter.More(); idIter.Next(), i++ ) {
        const SMDS_MeshNode* aNode = theMesh->FindNode(idIter.Key());
        aPoints->SetPoint( i, aNode->X(), aNode->Y(), aNode->Z() );
        myIDs.push_back(idIter.Key());
      }

      myIdGrid->SetPoints(aPoints);

      aPoints->Delete();

      myIdActor->GetMapper()->Update();
    }

    void SetElemsData( TColStd_MapOfInteger & theElemsIdMap, 
                       std::list<gp_XYZ> & aGrCentersXYZ )
    {
      vtkPoints* aPoints = vtkPoints::New();
      aPoints->SetNumberOfPoints(theElemsIdMap.Extent());
      myIDs.clear();
      
      TColStd_MapIteratorOfMapOfInteger idIter( theElemsIdMap );
      for( ; idIter.More(); idIter.Next() ) {
        myIDs.push_back(idIter.Key());
      }

      gp_XYZ aXYZ;
      std::list<gp_XYZ>::iterator coordIt = aGrCentersXYZ.begin();
      for( int i = 0; coordIt != aGrCentersXYZ.end(); coordIt++, i++ ) {
        aXYZ = *coordIt;
        aPoints->SetPoint( i, aXYZ.X(), aXYZ.Y(), aXYZ.Z() );
      }
      myIdGrid->SetPoints(aPoints);
      aPoints->Delete();
      
      myIdActor->GetMapper()->Update();
    }

    void AddToRender(vtkRenderer* theRenderer)
    {
      myIdActor->AddToRender(theRenderer);

      myPtsSelectVisiblePoints->SetRenderer(theRenderer);
      theRenderer->AddActor2D(myPointLabels);
    }

    void RemoveFromRender(vtkRenderer* theRenderer)
    {
      myIdActor->RemoveFromRender(theRenderer);

      myPtsSelectVisiblePoints->SetRenderer(theRenderer);
      theRenderer->RemoveActor(myPointLabels);
    }

    void SetPointsLabeled( bool theIsPointsLabeled, bool theIsActorVisible = true )
    {
      myIsPointsLabeled = theIsPointsLabeled && myIdGrid->GetNumberOfPoints();
      
      if ( myIsPointsLabeled ) {
        myPointsNumDataSet->ShallowCopy(myIdGrid);
        vtkDataSet *aDataSet = myPointsNumDataSet;
        int aNbElem = myIDs.size();
        vtkIntArray *anArray = vtkIntArray::New();
        anArray->SetNumberOfValues( aNbElem );
        for ( int i = 0; i < aNbElem; i++ )
          anArray->SetValue( i, myIDs[i] );
        aDataSet->GetPointData()->SetScalars( anArray );
        anArray->Delete();
        myPtsMaskPoints->SetInput( aDataSet );
        myPointLabels->SetVisibility( theIsActorVisible );
      }
      else {
        myPointLabels->SetVisibility( false );
      }
    }
    
    ~TIdPreview()
    {
      RemoveFromRender(myViewWindow->getRenderer());

      myIdGrid->Delete();

      myViewWindow->RemoveActor(myIdActor);
      myIdActor->Delete();

      //Deleting of points numbering pipeline
      //---------------------------------------
      myPointsNumDataSet->Delete();
      
      //myPtsLabeledDataMapper->RemoveAllInputs();        //vtk 5.0 porting
      myPtsLabeledDataMapper->Delete();

      //myPtsSelectVisiblePoints->UnRegisterAllOutputs(); //vtk 5.0 porting
      myPtsSelectVisiblePoints->Delete();

      //myPtsMaskPoints->UnRegisterAllOutputs();          //vtk 5.0 porting
      myPtsMaskPoints->Delete();

      myPointLabels->Delete();

//       myTimeStamp->Delete();
    }
  };
}

static const char * IconFirst[] = {
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
  setWindowTitle(myAction == 1 ? tr("SMESH_MERGE_ELEMENTS") : tr("SMESH_MERGE_NODES"));

  myIdPreview = new SMESH::TIdPreview(SMESH::GetViewWindow( mySMESHGUI ));

  SUIT_ResourceMgr* aResMgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap IconMergeNodes (aResMgr->loadPixmap("SMESH", tr("ICON_SMESH_MERGE_NODES")));
  QPixmap IconMergeElems (aResMgr->loadPixmap("SMESH", tr("ICON_DLG_MERGE_ELEMENTS")));
  QPixmap IconSelect     (aResMgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap IconAdd        (aResMgr->loadPixmap("SMESH", tr("ICON_APPEND")));
  QPixmap IconRemove     (aResMgr->loadPixmap("SMESH", tr("ICON_REMOVE")));

  setSizeGripEnabled(true);

  QVBoxLayout* DlgLayout = new QVBoxLayout(this);
  DlgLayout->setSpacing(SPACING);
  DlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(myAction == 1 ? 
                                    tr("SMESH_MERGE_ELEMENTS") : 
                                    tr("SMESH_MERGE_NODES"), 
                                    this);

  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* GroupConstructorsLayout = new QHBoxLayout(GroupConstructors);
  GroupConstructorsLayout->setSpacing(SPACING);
  GroupConstructorsLayout->setMargin(MARGIN);

  RadioButton = new QRadioButton(GroupConstructors);
  RadioButton->setIcon(myAction == 1 ? IconMergeElems : IconMergeNodes);
  RadioButton->setChecked(true);
  GroupConstructorsLayout->addWidget(RadioButton);
  ButtonGroup->addButton(RadioButton, 0);

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
  // Controls for switch dialog behaviour

  TypeBox = new QGroupBox( tr( "SMESH_MODE" ), this );
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

  myTypeId = 0;

  /***************************************************************/
  // Controls for coincident elements detecting
  GroupCoincident = new QGroupBox(myAction == 1 ? 
                                  tr("COINCIDENT_ELEMENTS") : 
                                  tr("COINCIDENT_NODES"), 
                                  this);

  QVBoxLayout* aCoincidentLayout = new QVBoxLayout(GroupCoincident);
  aCoincidentLayout->setSpacing(SPACING);
  aCoincidentLayout->setMargin(MARGIN);

  if (myAction == 0) { // case merge nodes
    QWidget* foo = new QWidget(GroupCoincident);
    TextLabelTolerance = new QLabel(tr("SMESH_TOLERANCE"), foo);
    SpinBoxTolerance = new SMESHGUI_SpinBox(foo);
    SpinBoxTolerance->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    GroupExclude = new QGroupBox(tr("EXCLUDE_GROUPS"), foo);
    GroupExclude->setCheckable( true );
    GroupExclude->setChecked( false );
    ListExclude = new QListWidget( GroupExclude );
    QVBoxLayout* GroupExcludeLayout = new QVBoxLayout(GroupExclude);
    GroupExcludeLayout->setSpacing(SPACING);
    GroupExcludeLayout->setMargin(MARGIN);
    GroupExcludeLayout->addWidget(ListExclude);

    QGridLayout* fooLayout = new QGridLayout( foo );
    fooLayout->setSpacing(SPACING);
    fooLayout->setMargin(0);
    fooLayout->addWidget(TextLabelTolerance, 0, 0 );
    fooLayout->addWidget(SpinBoxTolerance,   0, 1 );
    fooLayout->addWidget(GroupExclude,       1, 0, 1, 2 );
    aCoincidentLayout->addWidget(foo);
  }
  else {
    TextLabelTolerance = 0;
    SpinBoxTolerance = 0;
    GroupExclude = 0;
    ListExclude = 0;
  }

  GroupCoincidentWidget = new QWidget(GroupCoincident);
  QGridLayout* GroupCoincidentLayout = new QGridLayout(GroupCoincidentWidget);
  GroupCoincidentLayout->setSpacing(SPACING);
  GroupCoincidentLayout->setMargin(0);

  ListCoincident = new QListWidget(GroupCoincidentWidget);
  ListCoincident->setSelectionMode(QListWidget::ExtendedSelection);

  DetectButton      = new QPushButton(tr("DETECT"),           GroupCoincidentWidget);
  AddGroupButton    = new QPushButton(tr("SMESH_BUT_ADD"),    GroupCoincidentWidget);
  RemoveGroupButton = new QPushButton(tr("SMESH_BUT_REMOVE"), GroupCoincidentWidget);

  SelectAllCB = new QCheckBox(tr("SELECT_ALL"), GroupCoincidentWidget);

  GroupCoincidentLayout->addWidget(ListCoincident,    0,   0, 4, 2);
  GroupCoincidentLayout->addWidget(DetectButton,      0,   2);
  GroupCoincidentLayout->addWidget(AddGroupButton,    2, 2);
  GroupCoincidentLayout->addWidget(RemoveGroupButton, 3, 2);
  GroupCoincidentLayout->addWidget(SelectAllCB,       4, 0, 1, 3);
  GroupCoincidentLayout->setRowMinimumHeight(1, 10);
  GroupCoincidentLayout->setRowStretch(1, 5);

  aCoincidentLayout->addWidget(GroupCoincidentWidget);

  /***************************************************************/
  // Controls for editing the selected group
  GroupEdit = new QGroupBox(tr("EDIT_SELECTED_GROUP"), this);
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
  SetFirstButton->setIcon(QPixmap(IconFirst));

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
  DlgLayout->addWidget(GroupConstructors);
  DlgLayout->addWidget(GroupMesh);
  DlgLayout->addWidget(TypeBox);
  DlgLayout->addWidget(GroupCoincident);
  DlgLayout->addWidget(GroupEdit);
  DlgLayout->addWidget(GroupButtons);

  GroupCoincidentWidget->setVisible( myAction != 0 );
  GroupCoincident->setVisible( myAction == 0 );
  //if GroupExclude->setVisible( myAction == 0 );
  GroupEdit->hide();

  this->resize(10,10);

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
  if (myAction == 0) {
    SpinBoxTolerance->RangeStepAndValidator(0.0, COORD_MAX, 0.00001, "len_tol_precision");
    SpinBoxTolerance->SetValue(1e-05);
  }

  RadioButton->setChecked(true);

  GroupType->button(0)->setChecked(true);

  myEditCurrentArgument = (QWidget*)LineEditMesh; 

  myActor = 0;
  mySubMeshOrGroup = SMESH::SMESH_subMesh::_nil();

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myIsBusy = false;
  
  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectMeshButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(DetectButton, SIGNAL (clicked()), this, SLOT(onDetect()));
  connect(ListCoincident, SIGNAL (itemSelectionChanged()), this, SLOT(onSelectGroup()));
  connect(AddGroupButton, SIGNAL (clicked()), this, SLOT(onAddGroup()));
  connect(RemoveGroupButton, SIGNAL (clicked()), this, SLOT(onRemoveGroup()));
  connect(SelectAllCB, SIGNAL(toggled(bool)), this, SLOT(onSelectAll(bool)));
  connect(ListEdit, SIGNAL (itemSelectionChanged()), this, SLOT(onSelectElementFromGroup()));
  connect(AddElemButton, SIGNAL (clicked()), this, SLOT(onAddElement()));
  connect(RemoveElemButton, SIGNAL (clicked()), this, SLOT(onRemoveElement()));
  connect(SetFirstButton, SIGNAL( clicked() ), this, SLOT( onSetFirst() ) );
  connect(GroupType, SIGNAL(buttonClicked(int)), this, SLOT(onTypeChanged(int)));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));

  // Init Mesh field from selection
  SelectionIntoArgument();

  // Update Buttons
  updateControls();
  
  if (myAction == 0)
    myHelpFileName = "merging_nodes_page.html";
  else
    myHelpFileName = "merging_elements_page.html";
}

//=================================================================================
// function : FindGravityCenter()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::FindGravityCenter(TColStd_MapOfInteger & theElemsIdMap, 
                                          std::list< gp_XYZ > & theGrCentersXYZ)
{
  if (!myActor)
    return;

  SMDS_Mesh* aMesh = 0;
  aMesh = myActor->GetObject()->GetMesh();
  if (!aMesh)
    return;

  int nbNodes;

  TColStd_MapIteratorOfMapOfInteger idIter( theElemsIdMap );
  for( ; idIter.More(); idIter.Next() ) {
    const SMDS_MeshElement* anElem = aMesh->FindElement(idIter.Key());
    if ( !anElem )
      continue;

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
    if (myTypeId == 0)
      onDetect();

    SUIT_OverrideCursor aWaitCursor;
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

    SMESH::long_array_var anIds = new SMESH::long_array;
    SMESH::array_of_long_array_var aGroupsOfElements = new SMESH::array_of_long_array;

    if ( ListCoincident->count() == 0) {
      if (myAction == 0)
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

    if( myAction == 0 )
      aMeshEditor->MergeNodes (aGroupsOfElements.inout());
    else
      aMeshEditor->MergeElements (aGroupsOfElements.inout());

    if ( myTypeId == 0 ) {
      if (myAction ==0)
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INFORMATION"),
                                     tr("SMESH_MERGED_NODES").arg(QString::number(ListCoincident->count()).toLatin1().data()));
      else
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INFORMATION"),
                                     tr("SMESH_MERGED_ELEMENTS").arg(QString::number(ListCoincident->count()).toLatin1().data()));
    }
      

  } catch(...) {
  }
  
  ListCoincident->clear();
  
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
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::ClickOnCancel()
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

  reject();
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
  bool enable = !(myMesh->_is_nil()) && (ListCoincident->count() || (myTypeId == 0));
  buttonOk->setEnabled(enable);
  buttonApply->setEnabled(enable);
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
    else src = SMESH::SMESH_IDSource::_duplicate( mySubMeshOrGroup );

    switch (myAction) {
    case 0 :
      for ( int i = 0; GroupExclude->isChecked() && i < ListExclude->count(); i++ ) {
        if ( ListExclude->item( i )->checkState() == Qt::Checked ) {
          aExcludeGroups->length( aExcludeGroups->length()+1 );
          aExcludeGroups[ aExcludeGroups->length()-1 ] = SMESH::SMESH_IDSource::_duplicate( myGroups[i] );
        }
      }
      aMeshEditor->FindCoincidentNodesOnPartBut(src.in(),
                                                SpinBoxTolerance->GetValue(), 
                                                aGroupsArray.out(),
                                                aExcludeGroups.in());
      break;
    case 1 :
      aMeshEditor->FindEqualElements(src.in(), aGroupsArray.out());
      break;
    }
    
    for (int i = 0; i < aGroupsArray->length(); i++) {
      SMESH::long_array& aGroup = aGroupsArray[i];

      QStringList anIDs;
      for (int j = 0; j < aGroup.length(); j++)
        anIDs.append(QString::number(aGroup[j]));

      ListCoincident->addItem(anIDs.join(" "));
    }
   } catch(...) {
  }

  ListCoincident->selectAll();
  updateControls();
}

//=================================================================================
// function : onSelectGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::onSelectGroup()
{
  if (myIsBusy || !myActor)
    return;
  myEditCurrentArgument = (QWidget*)ListCoincident;

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
  
  if (myAction == 0) {
    myIdPreview->SetPointsData(myActor->GetObject()->GetMesh(), anIndices);
    myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
  }
  else {
    std::list< gp_XYZ > aGrCentersXYZ;
    FindGravityCenter(anIndices, aGrCentersXYZ);
    myIdPreview->SetElemsData( anIndices, aGrCentersXYZ);
    myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
  }

  updateControls();
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

  if (myAction == 0) {
    myIdPreview->SetPointsData(myActor->GetObject()->GetMesh(), anIndices);
    myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
  }
  else {
    std::list< gp_XYZ > aGrCentersXYZ;
    FindGravityCenter(anIndices, aGrCentersXYZ);
    myIdPreview->SetElemsData(anIndices, aGrCentersXYZ);
    myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
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
  updateControls();

  myIsBusy = false;
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

  if (send == SelectMeshButton) {
    myEditCurrentArgument = (QWidget*)LineEditMesh;
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    if (myTypeId == 1)
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_MergeDlg::SelectionIntoArgument()
{
  if (myEditCurrentArgument == (QWidget*)LineEditMesh) {
    QString aString = "";
    LineEditMesh->setText(aString);
    
    ListCoincident->clear();
    ListEdit->clear();
    myActor = 0;
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
    
    if (myMesh->_is_nil())
      return;

    LineEditMesh->setText(aString);
    
    myActor = SMESH::FindActorByEntry(IO->getEntry());
    if (!myActor)
      myActor = SMESH::FindActorByObject(myMesh);
    
    if ( myActor && myTypeId ==1 ) {
      mySubMeshOrGroup = SMESH::SMESH_IDSource::_nil();
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
      
      if ((!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil() || //SUBMESH OR GROUP
           !SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO)->_is_nil()) &&
          !SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO)->_is_nil())
        mySubMeshOrGroup = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      
      if (myAction == 0) {
        SMESH::SetPointRepresentation(true);
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(NodeSelection);
      }
      else
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(CellSelection);
    }

    // process groups
    if ( myAction == 0 && !myMesh->_is_nil() && myEntry != aCurrentEntry ) {
      myGroups.clear();
      ListExclude->clear();
      SMESH::ListOfGroups_var aListOfGroups = myMesh->GetGroups();
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
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    TypeBox->setEnabled(false);
    GroupMesh->setEnabled(false);
    GroupCoincident->setEnabled(false);
    GroupEdit->setEnabled(false);
    GroupButtons->setEnabled(false);
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
  GroupConstructors->setEnabled(true);
  TypeBox->setEnabled(true);
  GroupMesh->setEnabled(true);
  GroupCoincident->setEnabled(true);
  GroupEdit->setEnabled(true);
  GroupButtons->setEnabled(true);

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::enterEvent(QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MergeDlg::closeEvent(QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_MergeDlg::hideEvent (QHideEvent *)
{
  if (!isMinimized())
    ClickOnCancel();
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
  case 0: // automatic
    myIdPreview->SetPointsLabeled(false);
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->clearFilters();
    if (myAction == 0)
      GroupCoincidentWidget->hide();
    else
      GroupCoincident->hide();
    GroupEdit->hide();
    break;

  case 1: // manual
    SMESH::UpdateView();

    // Costruction of the logical filter
    SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
    SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);
    
    QList<SUIT_SelectionFilter*> aListOfFilters;
    if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
    if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);
    
    myMeshOrSubMeshOrGroupFilter =
      new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);

    if (myAction == 0) {
      GroupCoincidentWidget->show();
      SMESH::SetPointRepresentation(true);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(NodeSelection);
    }
    else {
      GroupCoincident->show();
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(CellSelection);
    }
    GroupEdit->show();
    break;
  }
  updateControls();

  qApp->processEvents();
  updateGeometry();
  resize(10,10);

  SelectionIntoArgument();
}
