// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
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

#include "SMESHGUI_EditMeshDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_SpinBox.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"
#include "SMESHGUI_MeshUtils.h"
#include "SMDS_Mesh.hxx"

#include "GEOMBase.h"

#include "SUIT_ResourceMgr.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "LightApp_Application.h"

#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SVTK_Selection.h"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// OCCT Includes
#include <gp_XYZ.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>

//IDL Headers
#include CORBA_SERVER_HEADER(SMESH_Group)

// VTK Includes
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
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkPointData.h>

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qheader.h>

using namespace std;

namespace SMESH {
  class TIdPreview { // to display in the viewer IDs of the selected elements
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

    vector<int> myIDs;

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
      myPtsLabeledDataMapper->SetLabelFormat("%g");
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
		       list<gp_XYZ> & aGrCentersXYZ )
    {
      vtkPoints* aPoints = vtkPoints::New();
      aPoints->SetNumberOfPoints(theElemsIdMap.Extent());
      myIDs.clear();
      
      TColStd_MapIteratorOfMapOfInteger idIter( theElemsIdMap );
      for( ; idIter.More(); idIter.Next() ) {
	myIDs.push_back(idIter.Key());
      }

      gp_XYZ aXYZ;
      list<gp_XYZ>::iterator coordIt = aGrCentersXYZ.begin();
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
      
      myPtsLabeledDataMapper->RemoveAllInputs();
      myPtsLabeledDataMapper->Delete();

      myPtsSelectVisiblePoints->UnRegisterAllOutputs();
      myPtsSelectVisiblePoints->Delete();

      myPtsMaskPoints->UnRegisterAllOutputs();
      myPtsMaskPoints->Delete();

      myPointLabels->Delete();

//       myTimeStamp->Delete();
    }
  };
}

static const char * IconFirst[] = {
"18 10 2 1",
" 	g None",
".	g #000000",
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
// class    : SMESHGUI_EditMeshDlg()
// purpose  :
//=================================================================================
SMESHGUI_EditMeshDlg::SMESHGUI_EditMeshDlg (SMESHGUI* theModule, 
					    int theAction)
  : QDialog(SMESH::GetDesktop(theModule), "SMESHGUI_EditMeshDlg", false, WStyle_Customize |
            WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
    mySMESHGUI(theModule),
    mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
    myAction(theAction)
{
  setCaption(tr("SMESH_MERGE_NODES"));

  myIdPreview = new SMESH::TIdPreview(SMESH::GetViewWindow( mySMESHGUI ));

  SUIT_ResourceMgr* aResMgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap IconMergeNodes (aResMgr->loadPixmap("SMESH", tr("ICON_SMESH_MERGE_NODES")));
  QPixmap IconMergeElems (aResMgr->loadPixmap("SMESH", tr("ICON_DLG_MERGE_ELEMENTS")));
  QPixmap IconSelect     (aResMgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap IconAdd        (aResMgr->loadPixmap("SMESH", tr("ICON_APPEND")));
  QPixmap IconRemove     (aResMgr->loadPixmap("SMESH", tr("ICON_REMOVE")));

  setSizeGripEnabled(TRUE);
  DlgLayout = new QGridLayout (this);
  DlgLayout->setSpacing(6);
  DlgLayout->setMargin(11);

  /***************************************************************/
  GroupConstructors = new QButtonGroup (this, "GroupConstructors");
  GroupConstructors->setTitle(tr("SMESH_MERGE_NODES"));
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  RadioButton = new QRadioButton(GroupConstructors, "RadioButton");
  RadioButton->setPixmap(IconMergeNodes);
  if (myAction == 1) RadioButton->setPixmap(IconMergeElems);
  RadioButton->setChecked(TRUE);
  GroupConstructorsLayout->addWidget(RadioButton, 0, 0);
  DlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox (this, "GroupButtons");
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
  QSpacerItem* spacer3 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer3, 0, 2);
  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK" ));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  DlgLayout->addWidget(GroupButtons, 4, 0);

  /***************************************************************/

  // Controls for mesh defining
  GroupMesh = new QGroupBox(this, "GroupMesh");
  GroupMesh->setTitle(tr("SMESH_SELECT_WHOLE_MESH"));
  GroupMesh->setColumnLayout(0, Qt::Vertical);
  GroupMesh->layout()->setSpacing(0);
  GroupMesh->layout()->setMargin(0);
  GroupMeshLayout = new QGridLayout(GroupMesh->layout());
  GroupMeshLayout->setAlignment(Qt::AlignTop);
  GroupMeshLayout->setSpacing(6);
  GroupMeshLayout->setMargin(11);

  TextLabelName = new QLabel(GroupMesh, "TextLabelName");
  TextLabelName->setText(tr("SMESH_NAME"));
  GroupMeshLayout->addWidget(TextLabelName, 0, 0);

  SelectMeshButton = new QPushButton(GroupMesh, "SelectMeshButton");
  SelectMeshButton->setPixmap(IconSelect);
  GroupMeshLayout->addWidget(SelectMeshButton, 0, 1);

  LineEditMesh = new QLineEdit(GroupMesh, "LineEditMesh");
  LineEditMesh->setReadOnly(true);
  GroupMeshLayout->addWidget(LineEditMesh, 0, 2);

  DlgLayout->addWidget(GroupMesh, 1, 0);

  /***************************************************************/

  // Controls for coincident elements detecting
  GroupCoincident = new QGroupBox(this, "GroupCoincident");
  GroupCoincident->setTitle(tr("COINCIDENT_NODES"));
  GroupCoincident->setColumnLayout(0, Qt::Vertical);
  GroupCoincident->layout()->setSpacing(0);
  GroupCoincident->layout()->setMargin(0);
  GroupCoincidentLayout = new QGridLayout(GroupCoincident->layout());
  GroupCoincidentLayout->setAlignment(Qt::AlignTop);
  GroupCoincidentLayout->setSpacing(6);
  GroupCoincidentLayout->setMargin(11);
  
  if (myAction == 0) { // case merge nodes
    TextLabelTolerance = new QLabel(GroupCoincident, "TextLabelTolerance");
    TextLabelTolerance->setText(tr("SMESH_TOLERANCE"));
    TextLabelTolerance->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    GroupCoincidentLayout->addWidget(TextLabelTolerance, 0, 0);

    SpinBoxTolerance = new SMESHGUI_SpinBox(GroupCoincident, "SpinBoxTolerance");
    GroupCoincidentLayout->addWidget(SpinBoxTolerance, 0, 1);
  }

  DetectButton = new QPushButton(GroupCoincident, "DetectButton");
  DetectButton->setText(tr("DETECT"));
  DetectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum));
  GroupCoincidentLayout->addWidget(DetectButton, 0, 2);

  ListCoincident = new QListBox(GroupCoincident, "ListCoincident");
  ListCoincident->setSelectionMode(QListBox::Extended);
  if (myAction == 0) // case merge nodes
    GroupCoincidentLayout->addMultiCellWidget(ListCoincident, 1, 3, 0, 1);
  else // case merge elements
    GroupCoincidentLayout->addMultiCellWidget(ListCoincident, 0, 3, 0, 1);

  QSpacerItem* spacer1 = new QSpacerItem(20, 21, QSizePolicy::Minimum, QSizePolicy::Expanding);
  GroupCoincidentLayout->addItem(spacer1, 1, 2);

  AddGroupButton = new QPushButton(GroupCoincident, "AddGroupButton");
  AddGroupButton->setText(tr("SMESH_BUT_ADD"));
  GroupCoincidentLayout->addWidget(AddGroupButton, 2, 2);

  RemoveGroupButton = new QPushButton(GroupCoincident, "RemoveGroupButton");
  RemoveGroupButton->setText(tr("SMESH_BUT_REMOVE"));
  GroupCoincidentLayout->addWidget(RemoveGroupButton, 3, 2);

  SelectAllCB = new QCheckBox(GroupCoincident, "SelectAllCB");
  SelectAllCB->setText(tr("SELECT_ALL"));
  GroupCoincidentLayout->addWidget(SelectAllCB, 4, 0);

  DlgLayout->addWidget(GroupCoincident, 2, 0);

  /***************************************************************/

  // Controls for editing the selected group
  GroupEdit = new QGroupBox(this, "GroupEdit");
  GroupEdit->setTitle(tr("EDIT_SELECTED_GROUP"));
  GroupEdit->setColumnLayout(0, Qt::Vertical);
  GroupEdit->layout()->setSpacing(0);
  GroupEdit->layout()->setMargin(0);
  GroupEditLayout = new QGridLayout(GroupEdit->layout());
  GroupEditLayout->setAlignment(Qt::AlignTop);
  GroupEditLayout->setSpacing(6);
  GroupEditLayout->setMargin(11);

  ListEdit = new QListBox(GroupEdit, "ListEdit");
  ListEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
  ListEdit->setRowMode(QListBox::FixedNumber);
  ListEdit->setMinimumHeight(ListEdit->sizeHint().height());
  ListEdit->setHScrollBarMode(QScrollView::AlwaysOn);
  ListEdit->setVScrollBarMode(QScrollView::AlwaysOff);
  ListEdit->setSelectionMode(QListBox::Extended);
  GroupEditLayout->addMultiCellWidget(ListEdit, 0, 1, 0, 0);

  AddElemButton = new QPushButton(GroupEdit, "AddElemButton");
  AddElemButton->setPixmap(IconAdd);
  GroupEditLayout->addWidget(AddElemButton, 0, 1);

  RemoveElemButton = new QPushButton(GroupEdit, "RemoveElemButton");
  RemoveElemButton->setPixmap(IconRemove);
  GroupEditLayout->addWidget(RemoveElemButton, 0, 2);

  SetFirstButton = new QPushButton(GroupEdit, "SetFirstButton");
  SetFirstButton->setIconSet(QPixmap(IconFirst));
  SetFirstButton->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  GroupEditLayout->addMultiCellWidget(SetFirstButton, 1, 1, 1, 2);

  DlgLayout->addWidget(GroupEdit, 3, 0);

  Init(); // Initialisations
}

//=================================================================================
// function : ~SMESHGUI_EditMeshDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_EditMeshDlg::~SMESHGUI_EditMeshDlg()
{
  // no need to delete child widgets, Qt does it all for us
  delete myIdPreview;
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::Init()
{
  if (myAction == 0) {
    SpinBoxTolerance->RangeStepAndValidator(0.0, COORD_MAX, 0.1, 3);
    SpinBoxTolerance->SetValue(1e-05);
  }

  RadioButton->setChecked(TRUE);

  myEditCurrentArgument = (QWidget*)LineEditMesh; 

  myActor = 0;
  mySubMeshOrGroup = SMESH::SMESH_subMesh::_nil();

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myIsBusy = false;
  
  // Costruction of the logical filter
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);
  
  QPtrList<SUIT_SelectionFilter> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);
  
  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectMeshButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(DetectButton, SIGNAL (clicked()), this, SLOT(onDetect()));
  connect(ListCoincident, SIGNAL (selectionChanged()), this, SLOT(onSelectGroup()));
  connect(AddGroupButton, SIGNAL (clicked()), this, SLOT(onAddGroup()));
  connect(RemoveGroupButton, SIGNAL (clicked()), this, SLOT(onRemoveGroup()));
  connect(SelectAllCB, SIGNAL(toggled(bool)), this, SLOT(onSelectAll(bool)));
  connect(ListEdit, SIGNAL (selectionChanged()), this, SLOT(onSelectElementFromGroup()));
  connect(AddElemButton, SIGNAL (clicked()), this, SLOT(onAddElement()));
  connect(RemoveElemButton, SIGNAL (clicked()), this, SLOT(onRemoveElement()));
  connect(SetFirstButton, SIGNAL( clicked() ), this, SLOT( onSetFirst() ) );

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));

  this->show(); /* displays Dialog */
  
  SetFirstButton->setEnabled(false);
  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // Init Mesh field from selection
  SelectionIntoArgument();

  // dialog customization
  if (myAction == 1) {
    setCaption(tr("SMESH_MERGE_ELEMENTS"));
    GroupConstructors->setTitle(tr("SMESH_MERGE_ELEMENTS"));
    GroupCoincident->setTitle(tr("COINCIDENT_ELEMENTS"));
  }
    
  myHelpFileName = "merge_elements.htm";
}

//=================================================================================
// function : FindGravityCenter()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::FindGravityCenter(TColStd_MapOfInteger & theElemsIdMap, 
					     list< gp_XYZ > & theGrCentersXYZ)
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
bool SMESHGUI_EditMeshDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked() || myMesh->_is_nil())
    return false;

  try {
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

    QApplication::setOverrideCursor(Qt::waitCursor);

    SMESH::long_array_var anIds = new SMESH::long_array;
    SMESH::array_of_long_array_var aGroupsOfElements = new SMESH::array_of_long_array;

    aGroupsOfElements->length(ListCoincident->count());
    QListBoxItem* item = ListCoincident->firstItem();

    int anArrayNum = 0;
    while (item) {
      QStringList aListIds = QStringList("");
      aListIds = QStringList::split(" ", item->text(), false);

      anIds->length(aListIds.count());
      for (int i = 0; i < aListIds.count(); i++)
        anIds[i] = aListIds[i].toInt();

      aGroupsOfElements[anArrayNum++] = anIds.inout();
      item = item->next();
    }

    if( myAction == 0 )
      aMeshEditor->MergeNodes (aGroupsOfElements.inout());
    else
      aMeshEditor->MergeElements (aGroupsOfElements.inout());

    QApplication::restoreOverrideCursor();
  } catch(...) {
  }
  
  SMESH::UpdateView();

  onDetect();
  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::ClickOnOk()
{
  if (ClickOnApply())
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::ClickOnCancel()
{
  myIdPreview->SetPointsLabeled(false);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::ClickOnHelp()
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

//=================================================================================
// function : onEditGroup()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onEditGroup()
{
  int nbSel = 0;
  for (int i = 0; i < ListCoincident->count(); i++) {
    if (ListCoincident->isSelected(i))
      nbSel++;
    if (nbSel > 1) {
      ListEdit->clear();
      return;
    }
  }
  if (nbSel == 0) {
    ListEdit->clear();
    return;
  }

  QString aNewIds = "";

  QListBoxItem* anItem;
  for (anItem = ListEdit->firstItem(); anItem != 0; anItem = anItem->next())
    aNewIds+=QString(" %1").arg(anItem->text());

  ListCoincident->changeItem(aNewIds, ListCoincident->currentItem());
  ListCoincident->setSelected(ListCoincident->currentItem(), true);
  
}

//=================================================================================
// function : updateControls()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::updateControls()
{
  if (ListEdit->count() == 0)
    SetFirstButton->setEnabled(false);
  bool enable = !(myMesh->_is_nil()) && ListCoincident->count();
  buttonOk->setEnabled(enable);
  buttonApply->setEnabled(enable);
}

//=================================================================================
// function : onDetect()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onDetect()
{
  if ( myMesh->_is_nil() || LineEditMesh->text().isEmpty() )
    return;

  try {
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

    QApplication::setOverrideCursor(Qt::waitCursor);
    ListCoincident->clear();
    ListEdit->clear();

    SMESH::array_of_long_array_var aGroupsArray;

    switch (myAction) {
    case 0 :
      if(!mySubMeshOrGroup->_is_nil())
	aMeshEditor->FindCoincidentNodesOnPart(mySubMeshOrGroup, SpinBoxTolerance->GetValue(), aGroupsArray);
      else
	aMeshEditor->FindCoincidentNodes(SpinBoxTolerance->GetValue(), aGroupsArray);
      break;
    case 1 :
      if(!mySubMeshOrGroup->_is_nil())
	aMeshEditor->FindEqualElements(mySubMeshOrGroup, aGroupsArray);
      else
	aMeshEditor->FindEqualElements(myMesh, aGroupsArray);
      break;
    }
    
    QListBoxItem* anItem = 0;
    for (int i = 0; i < aGroupsArray->length(); i++) {
      SMESH::long_array& aGroup = aGroupsArray[i];

      QString anIDs;
      for (int j = 0; j < aGroup.length(); j++)
        anIDs+=QString(" %1").arg(aGroup[j]);

      anItem = new QListBoxText(anIDs);
      ListCoincident->insertItem(anItem);
    }
    QApplication::restoreOverrideCursor();
  } catch(...) {
  }

  ListCoincident->selectAll(true);
  updateControls();
}

//=================================================================================
// function : onSelectGroup()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onSelectGroup()
{
  if (myIsBusy || !myActor)
    return;
  myEditCurrentArgument = (QWidget*)ListCoincident;

  ListEdit->clear();
  
  TColStd_MapOfInteger anIndices;
  QListBoxItem* anItem;
  int NbOfSelected = 0;
  for (anItem = ListCoincident->firstItem(); anItem != 0; anItem = anItem->next()) {
    if (anItem->isSelected()) {
      QStringList aListIds = QStringList("");
      aListIds = QStringList::split(" ", anItem->text(), false);
      for (int i = 0; i < aListIds.count(); i++)
	anIndices.Add(aListIds[i].toInt());
      NbOfSelected++;
      ListEdit->clear();
      if (NbOfSelected == 1) {
	ListEdit->insertStringList(aListIds);
	ListEdit->selectAll(true);
      }
    }
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
    list< gp_XYZ > aGrCentersXYZ;
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
void SMESHGUI_EditMeshDlg::onSelectAll (bool isToggled)
{
  ListCoincident->selectAll(isToggled);
}

//=================================================================================
// function : onSelectElementFromGroup()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onSelectElementFromGroup()
{
  if (myIsBusy || !myActor)
    return;

  int nbSel = 0;
  TColStd_MapOfInteger anIndices;
  QListBoxItem* anItem;
  for (anItem = ListEdit->firstItem(); anItem != 0; anItem = anItem->next()) {
    if (anItem->isSelected()) {
      int anId = anItem->text().toInt();
      anIndices.Add(anId);
      nbSel++;
      if (nbSel == 1)
	SetFirstButton->setEnabled(true);
    }
  }
  if (nbSel == 0 || nbSel > 1)
    SetFirstButton->setEnabled(false);

  mySelector->AddOrRemoveIndex(myActor->getIO(), anIndices, false);
  SALOME_ListIO aList;
  aList.Append(myActor->getIO());
  mySelectionMgr->setSelectedObjects(aList);

  if (myAction == 0) {
    myIdPreview->SetPointsData(myActor->GetObject()->GetMesh(), anIndices);
    myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
  }
  else {
    list< gp_XYZ > aGrCentersXYZ;
    FindGravityCenter(anIndices, aGrCentersXYZ);
    myIdPreview->SetElemsData(anIndices, aGrCentersXYZ);
    myIdPreview->SetPointsLabeled(!anIndices.IsEmpty(), myActor->GetVisibility());
  }
}

//=================================================================================
// function : onAddGroup()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onAddGroup()
{
  if ( myMesh->_is_nil() || LineEditMesh->text().isEmpty() )
    return;

  QString anIDs = "";
  SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), anIDs);
  
  ListCoincident->clearSelection();
  QListBoxItem* anItem = new QListBoxText(anIDs);
  ListCoincident->insertItem(anItem);
  int nbGroups = ListCoincident->count();
  if (nbGroups) {
    ListCoincident->setCurrentItem(nbGroups-1);
    ListCoincident->setSelected(nbGroups-1, true);
  }
  else {
    ListCoincident->setCurrentItem(0);
    ListCoincident->setSelected(0, true);
  }

  updateControls();
}

//=================================================================================
// function : onRemoveGroup()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onRemoveGroup()
{
  if (myEditCurrentArgument != (QWidget*)ListCoincident)
    return;
  myIsBusy = true;

  for (int i = ListCoincident->count(); i > 0; i--)
    if (ListCoincident->isSelected(i-1))
      ListCoincident->removeItem(i-1);

  ListEdit->clear();
  updateControls();

  myIsBusy = false;
}

//=================================================================================
// function : onAddElement()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onAddElement()
{
  if (!myActor)
    return;
  myIsBusy = true;

  QString aListStr = "";
  int aNbNnodes = 0;

  aNbNnodes = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aListStr);
  if (aNbNnodes < 1)
    return;

  QStringList aNodes = QStringList::split(" ", aListStr);
  QListBoxItem* anItem = 0;

  for (QStringList::iterator it = aNodes.begin(); it != aNodes.end(); ++it) {
    anItem = ListEdit->findItem(*it, Qt::ExactMatch);
    if (!anItem) {
      anItem = new QListBoxText(*it);
      ListEdit->insertItem(anItem);
    }
    ListEdit->setSelected(anItem, true);
  }

  myIsBusy = false;
  onEditGroup();
}

//=================================================================================
// function : onRemoveElement()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onRemoveElement()
{
  if (myEditCurrentArgument != (QWidget*)ListCoincident)
    return;
  myIsBusy = true;

  for (int i = ListEdit->count(); i > 0; i--)
    if (ListEdit->isSelected(i-1))
      ListEdit->removeItem(i-1);

  myIsBusy = false;
  onEditGroup();
}

//=================================================================================
// function : onSetFirst()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::onSetFirst()
{
  if (myEditCurrentArgument != (QWidget*)ListCoincident)
    return;
  myIsBusy = true;
  
  QListBoxItem* anItem;
  for (anItem = ListEdit->firstItem(); anItem != 0; anItem = anItem->next()) {
    if (anItem->isSelected()) {
      ListEdit->takeItem(anItem);
      ListEdit->insertItem(anItem, 0);
    }
  }

  myIsBusy = false;
  onEditGroup();
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::SetEditCurrentArgument()
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
void SMESHGUI_EditMeshDlg::SelectionIntoArgument()
{
  if (myEditCurrentArgument == (QWidget*)LineEditMesh) {
    QString aString = "";
    LineEditMesh->setText(aString);
    
    ListCoincident->clear();
    ListEdit->clear();
    myActor = 0;
    
    int nbSel = SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
    if (nbSel != 1)
      return;

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList, SVTK_Viewer::Type());
    
    Handle(SALOME_InteractiveObject) IO = aList.First();
    myMesh = SMESH::GetMeshByIO(IO);
    
    if (myMesh->_is_nil())
      return;
    
    myActor = SMESH::FindActorByEntry(IO->getEntry());
    if (!myActor)
      myActor = SMESH::FindActorByObject(myMesh);
    if(!myActor)
      return;
    
    mySubMeshOrGroup = SMESH::SMESH_IDSource::_nil();
    
    if ((!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil() || //SUBMESH OR GROUP
         !SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO)->_is_nil()) &&
        !SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO)->_is_nil())
      mySubMeshOrGroup = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
     
    LineEditMesh->setText(aString);

    if (myAction == 0) {
      SMESH::SetPointRepresentation(true);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(NodeSelection);
    }
    else
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(CellSelection);
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupMesh->setEnabled(false);
    GroupCoincident->setEnabled(false);
    GroupEdit->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupMesh->setEnabled(true);
  GroupCoincident->setEnabled(true);
  GroupEdit->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::enterEvent(QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::closeEvent(QCloseEvent*)
{
  /* same than click on cancel button */
  this->ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_EditMeshDlg::hideEvent (QHideEvent *)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_EditMeshDlg::keyPressEvent( QKeyEvent* e)
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
