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
// File   : SMESHGUI_AddMeshElementDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//  SMESH includes
//
#include "SMESHGUI_AddMeshElementDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_FaceOrientationFilter.h>
#include <SMDS_Mesh.hxx>

// SALOME GUI inclues
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ViewManager.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SalomeApp_Application.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

// IDL incldues
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// VTK includes
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

// Qt includes
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVariant>
#include <QCheckBox>
#include <QKeyEvent>
#include <QButtonGroup>

#define SPACING 6
#define MARGIN  11

namespace SMESH
{
  class TElementSimulation
  {
    SalomeApp_Application* myApplication;
    SUIT_ViewWindow* myViewWindow;
    SVTK_ViewWindow* myVTKViewWindow;

    SALOME_Actor* myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkUnstructuredGrid* myGrid;

    SALOME_Actor* myFaceOrientation;
    vtkPolyDataMapper* myFaceOrientationDataMapper;
    SMESH_FaceOrientationFilter* myFaceOrientationFilter;

  public:
    TElementSimulation (SalomeApp_Application* theApplication)
    {
      myApplication = theApplication;
      SUIT_ViewManager* mgr = theApplication->activeViewManager();
      if (!mgr) return;
      myViewWindow = mgr->getActiveView();
      myVTKViewWindow = GetVtkViewWindow(myViewWindow);

      myGrid = vtkUnstructuredGrid::New();

      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInput(myGrid);

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->PickableOff();
      myPreviewActor->VisibilityOff();
      myPreviewActor->SetMapper(myMapper);

      vtkFloatingPointType anRGB[3];
      vtkProperty* aProp = vtkProperty::New();
      GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
      aProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );
      myPreviewActor->SetProperty( aProp );
      aProp->Delete();

      vtkProperty* aBackProp = vtkProperty::New();
      GetColor( "SMESH", "backface_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 0, 255 ) );
      aBackProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );
      myPreviewActor->SetBackfaceProperty( aBackProp );
      aBackProp->Delete();

      myVTKViewWindow->AddActor(myPreviewActor);

      // Orientation of faces
      myFaceOrientationFilter = SMESH_FaceOrientationFilter::New();
      myFaceOrientationFilter->SetInput(myGrid);

      myFaceOrientationDataMapper = vtkPolyDataMapper::New();
      myFaceOrientationDataMapper->SetInput(myFaceOrientationFilter->GetOutput());

      myFaceOrientation = SALOME_Actor::New();
      myFaceOrientation->PickableOff();
      myFaceOrientation->VisibilityOff();
      myFaceOrientation->SetMapper(myFaceOrientationDataMapper);

      vtkProperty* anOrientationProp = vtkProperty::New();
      GetColor( "SMESH", "orientation_color", anRGB[0], anRGB[1], anRGB[2], QColor( 255, 255, 255 ) );
      anOrientationProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );
      myFaceOrientation->SetProperty( anOrientationProp );
      anOrientationProp->Delete();

      myVTKViewWindow->AddActor(myFaceOrientation);
    }

    typedef std::vector<vtkIdType> TVTKIds;
    void SetPosition (SMESH_Actor* theActor,
                      vtkIdType    theType,
                      TVTKIds&     theIds)
    {
      vtkUnstructuredGrid *aGrid = theActor->GetUnstructuredGrid();
      myGrid->SetPoints(aGrid->GetPoints());
      myGrid->Reset();

      const std::vector<int>& interlace = SMDS_MeshCell::toVtkOrder( VTKCellType( theType ));
      SMDS_MeshCell::applyInterlace( interlace, theIds );

      vtkIdList *anIds = vtkIdList::New();
      for (int i = 0, iEnd = theIds.size(); i < iEnd; i++)
        anIds->InsertId(i,theIds[i]);

      myGrid->InsertNextCell(theType,anIds);
      anIds->Delete();

      myGrid->Modified();

      SetVisibility(true, theActor->GetFacesOriented());
    }


    void SetVisibility (bool theVisibility, bool theShowOrientation = false)
    {
      myPreviewActor->SetVisibility(theVisibility);
      myFaceOrientation->SetVisibility(theShowOrientation);
      RepaintCurrentView();
    }


    ~TElementSimulation()
    {
      if (FindVtkViewWindow(myApplication->activeViewManager(), myViewWindow)) {
        myVTKViewWindow->RemoveActor(myPreviewActor);
        myVTKViewWindow->RemoveActor(myFaceOrientation);
      }
      myPreviewActor->Delete();
      myFaceOrientation->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myFaceOrientationFilter->Delete();

      myFaceOrientationDataMapper->RemoveAllInputs();
      myFaceOrientationDataMapper->Delete();

      myGrid->Delete();
    }
  };
}

//=================================================================================
// function : SMESHGUI_AddMeshElementDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_AddMeshElementDlg::SMESHGUI_AddMeshElementDlg( SMESHGUI*          theModule,
                                                        SMDSAbs_EntityType ElementType)
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myBusy ( false )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );

  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>
    (SUIT_Session::session()->activeApplication());
  myIsPoly = false;
  mySimulation = new SMESH::TElementSimulation (anApp);
  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();
  myGeomType = ElementType;
  myElementType = SMDSAbs_Volume;

  // verify nb nodes and type
  QString elemName;
  switch ( myGeomType ) {
  case SMDSEntity_0D:
    myNbNodes = 1;
    myElementType = SMDSAbs_0DElement;
    elemName = "ELEM0D";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_0delems_anchor";
    break;
  case SMDSEntity_Ball:
    myNbNodes = 1;
    myElementType = SMDSAbs_Ball;
    elemName = "BALL";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_ball_anchor";
    break;
  case SMDSEntity_Edge:
    myNbNodes = 2;
    myElementType = SMDSAbs_Edge;
    elemName = "EDGE";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_edges_anchor";
    break;
  case SMDSEntity_Triangle:
    myNbNodes = 3;
    elemName = "TRIANGLE";
    myElementType = SMDSAbs_Face;
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_triangles_anchor";
    break;
  case SMDSEntity_Quadrangle:
    myNbNodes = 4;
    myElementType = SMDSAbs_Face;
    elemName = "QUADRANGLE";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_quadrangles_anchor";
    break;
  case SMDSEntity_Polygon:
    myNbNodes = 0;
    myElementType = SMDSAbs_Face;
    elemName = "POLYGON";
    myIsPoly = true;
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_polygons_anchor";
    break;
  case SMDSEntity_Tetra:
    myNbNodes = 4;
    elemName = "TETRAS";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_tetrahedrons_anchor";
    break;
  case SMDSEntity_Pyramid:
    myNbNodes = 5;
    elemName = "PYRAMID";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_pyramids_anchor";
    break;
  case SMDSEntity_Hexa:
    myNbNodes = 8;
    elemName = "HEXAS";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_hexahedrons_anchor";
    break;
  case SMDSEntity_Penta:
    myNbNodes = 6;
    elemName = "PENTA";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_pentahedrons_anchor";
    break;
  case SMDSEntity_Hexagonal_Prism:
    myNbNodes = 12;
    elemName = "OCTA";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_octahedrons_anchor";
    break;
  default:
    myNbNodes = 2;
    elemName = "EDGE";
    myHelpFileName = "adding_nodes_and_elements_page.html#adding_edges_anchor";
  }

  QString iconName      = tr(QString("ICON_DLG_%1").arg(elemName).toLatin1().data());
  QString buttonGrTitle = tr(QString("SMESH_%1").arg(elemName).toLatin1().data());
  QString caption       = tr(QString("SMESH_ADD_%1_TITLE").arg(elemName).toLatin1().data());
  QString grBoxTitle    = tr(QString("SMESH_ADD_%1").arg(elemName).toLatin1().data());

  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", iconName));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setWindowTitle(caption);
  setSizeGripEnabled(true);

  QVBoxLayout* aTopLayout = new QVBoxLayout(this);
  aTopLayout->setSpacing(SPACING);
  aTopLayout->setMargin(MARGIN);

  /* Constructor *************************************************/
  GroupConstructors = new QGroupBox(buttonGrTitle, this);
  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* GroupConstructorsLayout = new QHBoxLayout(GroupConstructors);
  GroupConstructorsLayout->setSpacing(SPACING);
  GroupConstructorsLayout->setMargin(MARGIN);

  Constructor1 = new QRadioButton(GroupConstructors);
  Constructor1->setIcon(image0);
  Constructor1->setChecked(true);

  GroupConstructorsLayout->addWidget(Constructor1);
  ButtonGroup->addButton( Constructor1, 0 );

  /* Nodes & Reverse *********************************************/
  GroupC1 = new QGroupBox(grBoxTitle, this);
  QGridLayout* GroupC1Layout = new QGridLayout(GroupC1);
  GroupC1Layout->setSpacing(SPACING);
  GroupC1Layout->setMargin(MARGIN);

  TextLabelC1A1 = new QLabel(tr("SMESH_ID_NODES"), GroupC1);
  SelectButtonC1A1 = new QPushButton(GroupC1);
  SelectButtonC1A1->setIcon(image1);
  LineEditC1A1 = new QLineEdit(GroupC1);
  LineEditC1A1->setValidator(new SMESHGUI_IdValidator(this, myIsPoly ? 1000 : myNbNodes));

  Reverse = (myElementType == SMDSAbs_Face || myElementType == SMDSAbs_Volume ) ? new QCheckBox(tr("SMESH_REVERSE"), GroupC1) : 0;

  DiameterSpinBox = ( myGeomType == SMDSEntity_Ball ) ? new SMESHGUI_SpinBox(GroupC1) : 0;
  QLabel* diameterLabel = DiameterSpinBox ? new QLabel( tr("BALL_DIAMETER"),GroupC1) : 0;

  GroupC1Layout->addWidget(TextLabelC1A1,    0, 0);
  GroupC1Layout->addWidget(SelectButtonC1A1, 0, 1);
  GroupC1Layout->addWidget(LineEditC1A1,     0, 2);
  if ( Reverse ) {
    GroupC1Layout->addWidget(Reverse, 1, 0, 1, 3);
  }
  if ( DiameterSpinBox ) {
    GroupC1Layout->addWidget(diameterLabel,   1, 0);
    GroupC1Layout->addWidget(DiameterSpinBox, 1, 1, 1, 2);

    DiameterSpinBox->RangeStepAndValidator( 1e-7, 1e+9, 0.1 );
    DiameterSpinBox->SetValue( 1. );
  }
  /* Add to group ************************************************/
  GroupGroups = new QGroupBox( tr( "SMESH_ADD_TO_GROUP" ), this );
  GroupGroups->setCheckable( true );
  QHBoxLayout* GroupGroupsLayout = new QHBoxLayout(GroupGroups);
  GroupGroupsLayout->setSpacing(SPACING);
  GroupGroupsLayout->setMargin(MARGIN);

  TextLabel_GroupName = new QLabel( tr( "SMESH_GROUP" ), GroupGroups );
  ComboBox_GroupName = new QComboBox( GroupGroups );
  ComboBox_GroupName->setEditable( true );
  ComboBox_GroupName->setInsertPolicy( QComboBox::NoInsert );

  GroupGroupsLayout->addWidget( TextLabel_GroupName );
  GroupGroupsLayout->addWidget( ComboBox_GroupName, 1 );

  /* Apply etc ***************************************************/
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
  aTopLayout->addWidget(GroupConstructors);
  aTopLayout->addWidget(GroupC1);
  aTopLayout->addWidget(GroupGroups);
  aTopLayout->addWidget(GroupButtons);

  Init(); /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_AddMeshElementDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_AddMeshElementDlg::~SMESHGUI_AddMeshElementDlg()
{
  delete mySimulation;
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::Init()
{
  GroupC1->show();
  Constructor1->setChecked(true);
  myEditCurrentArgument = LineEditC1A1;
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  /* reset "Add to group" control */
  GroupGroups->setChecked( false );
  //GroupGroups->setVisible( myElementType != SMDSAbs_0DElement );

  myNbOkNodes = 0;
  myActor = 0;

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()),   SLOT(ClickOnHelp()));

  connect(SelectButtonC1A1, SIGNAL(clicked()), SLOT(SetEditCurrentArgument()));
  connect(LineEditC1A1, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(SelectionIntoArgument()));
  /* to close dialog if study frame change */
  connect(mySMESHGUI, SIGNAL (SignalStudyFrameChanged()), SLOT(ClickOnCancel()));
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), SLOT(ClickOnCancel()));    

  if (Reverse)
    connect(Reverse, SIGNAL(stateChanged(int)), SLOT(CheckBox(int)));

  // set selection mode
  SMESH::SetPointRepresentation(true);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( NodeSelection );

  myBusy = false;

  SelectionIntoArgument();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ClickOnApply()
{
  if( !isValid() )
    return;

  if (myNbOkNodes && !mySMESHGUI->isActiveStudyLocked()) {
    myBusy = true;
    SMESH::long_array_var anArrayOfIndices = new SMESH::long_array;
    anArrayOfIndices->length(myNbNodes);
    QStringList aListId = myEditCurrentArgument->text().split(" ", QString::SkipEmptyParts);
    const std::vector<int>& revIndex = SMDS_MeshCell::reverseSmdsOrder( myGeomType );
    if ( Reverse && Reverse->isChecked() && !revIndex.empty() )
      for (int i = 0; i < aListId.count(); i++)
        anArrayOfIndices[i] = aListId[ revIndex[i] ].toInt();
    else if ( Reverse && Reverse->isChecked() && revIndex.empty() ) // polygon
      for (int i = 0; i < aListId.count(); i++)
        anArrayOfIndices[i] = aListId[ aListId.count()-1 - i ].toInt();
    else
      for (int i = 0; i < aListId.count(); i++)
        anArrayOfIndices[i] = aListId[ i ].toInt();

    bool addToGroup = GroupGroups->isChecked();
    QString aGroupName;

    SMESH::SMESH_GroupBase_var aGroup;
    int idx = 0;
    if( addToGroup ) {
      aGroupName = ComboBox_GroupName->currentText();
      for ( int i = 1; i < ComboBox_GroupName->count(); i++ ) {
        QString aName = ComboBox_GroupName->itemText( i );
        if ( aGroupName == aName && ( i == ComboBox_GroupName->currentIndex() || idx == 0 ) )
          idx = i;
      }
      if ( idx > 0 && idx < myGroups.count() ) {
        SMESH::SMESH_GroupOnGeom_var aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( myGroups[idx-1] );
        if ( !aGeomGroup->_is_nil() ) {
          int res = SUIT_MessageBox::question( this, tr( "SMESH_WRN_WARNING" ),
                                               tr( "MESH_STANDALONE_GRP_CHOSEN" ).arg( aGroupName ),
                                               tr( "SMESH_BUT_YES" ), tr( "SMESH_BUT_NO" ), 0, 1 );
          if ( res == 1 ) return;
        }
        aGroup = myGroups[idx-1];
      }
    }

    long anElemId = -1;
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
    switch (myElementType) {
    case SMDSAbs_0DElement:
      anElemId = aMeshEditor->Add0DElement(anArrayOfIndices[0]); break;
    case SMDSAbs_Ball:
      if ( myGeomType == SMDSEntity_Ball )
        anElemId = aMeshEditor->AddBall(anArrayOfIndices[0],
                                        DiameterSpinBox->GetValue()); break;
    case SMDSAbs_Edge:
      anElemId = aMeshEditor->AddEdge(anArrayOfIndices.inout()); break;
    case SMDSAbs_Face:
      if ( myIsPoly )
        anElemId = aMeshEditor->AddPolygonalFace(anArrayOfIndices.inout());
      else
        anElemId = aMeshEditor->AddFace(anArrayOfIndices.inout());
      break;
    default:
      anElemId = aMeshEditor->AddVolume(anArrayOfIndices.inout()); break;
    }

    if ( anElemId != -1 && addToGroup && !aGroupName.isEmpty() ) {
      SMESH::SMESH_Group_var aGroupUsed;
      if ( aGroup->_is_nil() ) {
        // create new group 
        aGroupUsed = SMESH::AddGroup( myMesh, (SMESH::ElementType)myElementType, aGroupName );
        if ( !aGroupUsed->_is_nil() ) {
          myGroups.append(SMESH::SMESH_GroupBase::_duplicate(aGroupUsed));
          ComboBox_GroupName->addItem( aGroupName );
        }
      }
      else {
        SMESH::SMESH_GroupOnGeom_var aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGroup );
        if ( !aGeomGroup->_is_nil() ) {
          aGroupUsed = myMesh->ConvertToStandalone( aGeomGroup );
          if ( !aGroupUsed->_is_nil() && idx > 0 ) {
            myGroups[idx-1] = SMESH::SMESH_GroupBase::_duplicate(aGroupUsed);
            SMESHGUI::GetSMESHGUI()->getApp()->updateObjectBrowser();
          }
        }
        else
          aGroupUsed = SMESH::SMESH_Group::_narrow( aGroup );
      }

      if ( !aGroupUsed->_is_nil() ) {
        SMESH::long_array_var anIdList = new SMESH::long_array;
        anIdList->length( 1 );
        anIdList[0] = anElemId;
        aGroupUsed->Add( anIdList.inout() );
      }
    }

    SALOME_ListIO aList; aList.Append( myActor->getIO() );
    mySelector->ClearIndex();
    mySelectionMgr->setSelectedObjects( aList, false );

    SMESH::UpdateView();
    mySimulation->SetVisibility(false);

    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);

    myEditCurrentArgument->setText("");

    myBusy = false;

    SMESHGUI::Modified();
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ClickOnCancel()
{
  //mySelectionMgr->clearSelected();
  mySimulation->SetVisibility(false);
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app)
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""),
                             myHelpFileName);
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
// function : onTextChange()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) return;
  myBusy = true;

  myNbOkNodes = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  mySimulation->SetVisibility(false);

  // hilight entered nodes
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    TColStd_MapOfInteger newIndices;

    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
    bool allOk = true;
    for (int i = 0; i < aListId.count(); i++) {
      if( const SMDS_MeshNode * n = aMesh->FindNode( aListId[ i ].toInt() ) )
        {
          newIndices.Add( n->GetID() );
          myNbOkNodes++;
        }
      else
        allOk = false;  
    }

    mySelector->AddOrRemoveIndex( myActor->getIO(), newIndices, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( myActor->getIO(), true, true );

    myNbOkNodes = ( allOk && myNbNodes == aListId.count() );

    if (myIsPoly)
      {
        if ( !allOk || myElementType != SMDSAbs_Face || aListId.count() < 3 )
          myNbOkNodes = 0;
        else
          myNbOkNodes = aListId.count();
      }
  }

  if(myNbOkNodes) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
    displaySimulation();
  }

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_AddMeshElementDlg::SelectionIntoArgument()
{
  if (myBusy) return;

  // clear
  myNbOkNodes = 0;
  myActor = 0;

  myBusy = true;
  myEditCurrentArgument->setText("");
  myBusy = false;

  if (!GroupButtons->isEnabled()) // inactive
    return;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  mySimulation->SetVisibility(false);
  //  SMESH::SetPointRepresentation(true);

  QString aCurrentEntry = myEntry;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  if (aList.Extent() != 1)
    return;

  Handle(SALOME_InteractiveObject) anIO = aList.First();
  myEntry = anIO->getEntry();
  myMesh = SMESH::GetMeshByIO(anIO);
  if (myMesh->_is_nil())
    return;

  // process groups
  if ( !myMesh->_is_nil() && myEntry != aCurrentEntry ) {
    myGroups.clear();
    ComboBox_GroupName->clear();
    ComboBox_GroupName->addItem( QString() );
    SMESH::ListOfGroups aListOfGroups = *myMesh->GetGroups();
    for ( int i = 0, n = aListOfGroups.length(); i < n; i++ ) {
      SMESH::SMESH_GroupBase_var aGroup = aListOfGroups[i];
      if ( !aGroup->_is_nil() && aGroup->GetType() == (SMESH::ElementType)myElementType ) {
        QString aGroupName( aGroup->GetName() );
        if ( !aGroupName.isEmpty() ) {
          myGroups.append(SMESH::SMESH_GroupBase::_duplicate(aGroup));
          ComboBox_GroupName->addItem( aGroupName );
        }
      }
    }
  }

  myActor = SMESH::FindActorByEntry(anIO->getEntry());
  if (!myActor)
    return;

  // get selected nodes
  QString aString = "";
  int nbNodes = SMESH::GetNameOfSelectedNodes(mySelector,myActor->getIO(),aString);
  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myBusy = false;
  if (myIsPoly && myElementType == SMDSAbs_Face && nbNodes >= 3 ) {
    myNbNodes = nbNodes;
  } else if (myNbNodes != nbNodes) {
    return;
  }

  // OK
  myNbOkNodes = nbNodes;

  buttonOk->setEnabled(true);
  buttonApply->setEnabled(true);

  displaySimulation();
}

//=================================================================================
// function : displaySimulation()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::displaySimulation()
{
  if (myNbOkNodes && GroupButtons->isEnabled()) {
    SMESH::TElementSimulation::TVTKIds anIds;
    QStringList aListId = myEditCurrentArgument->text().split(" ", QString::SkipEmptyParts);
    for (int i = 0; i < aListId.count(); i++)
      anIds.push_back(myActor->GetObject()->GetNodeVTKId(aListId[ i ].toInt()));

    if (Reverse && Reverse->isChecked())
    {
      const std::vector<int>& i = SMDS_MeshCell::reverseSmdsOrder( myGeomType );
      if ( i.empty() ) // polygon
        std::reverse( anIds.begin(), anIds.end() );
      else
        SMDS_MeshCell::applyInterlace( i, anIds );
    }

    vtkIdType aType = SMDS_MeshCell::toVtkType( myGeomType );
    mySimulation->SetPosition(myActor,aType,anIds);
    SMESH::UpdateView();
  }
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  if (send == SelectButtonC1A1) {
    LineEditC1A1->setFocus();
    myEditCurrentArgument = LineEditC1A1;
  }
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupC1->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySimulation->SetVisibility(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();

  GroupConstructors->setEnabled(true);
  GroupC1->setEnabled(true);
  GroupButtons->setEnabled(true);

  SMESH::SetPointRepresentation(true);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( NodeSelection );
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::enterEvent (QEvent*)
{
  if (GroupConstructors->isEnabled())
    return;
  ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=================================================================================
// function : hideEvent()
// purpose  : caused by ESC key
//=================================================================================
void SMESHGUI_AddMeshElementDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=================================================================================
// function : CheckBox()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::CheckBox (int state)
{
  if (!myNbOkNodes)
    return;

  if (state >= 0) {
    mySimulation->SetVisibility(false);
    displaySimulation();
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::keyPressEvent( QKeyEvent* e )
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
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_AddMeshElementDlg::isValid()
{
  if( GroupGroups->isChecked() && ComboBox_GroupName->currentText().isEmpty() ) {
    SUIT_MessageBox::warning( this, tr( "SMESH_WRN_WARNING" ), tr( "GROUP_NAME_IS_EMPTY" ) );
    return false;
  }
  return true;
}
