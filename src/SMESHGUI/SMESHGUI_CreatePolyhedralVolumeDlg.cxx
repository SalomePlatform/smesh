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
// File   : SMESHGUI_CreatePolyhedralVolumeDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_CreatePolyhedralVolumeDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_IdValidator.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ViewManager.h>
#include <SUIT_OverrideCursor.h>

#include <SalomeApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include "utilities.h"

#include <SVTK_ViewWindow.h>

// OCCT includes
#include <TColStd_ListOfInteger.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

// VTK includes
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

namespace SMESH
{
  class TPolySimulation
  {
    SVTK_ViewWindow* myViewWindow;

    SALOME_Actor *myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkUnstructuredGrid* myGrid;

  public:

    TPolySimulation(SalomeApp_Application* app)
    {
      SUIT_ViewManager* mgr = app->activeViewManager();
      myViewWindow = mgr ? dynamic_cast<SVTK_ViewWindow*>( mgr->getActiveView() ) : NULL;

      myGrid = vtkUnstructuredGrid::New();
  
      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInputData( myGrid );

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->PickableOff();
      myPreviewActor->VisibilityOff();
      myPreviewActor->SetMapper( myMapper );
      myPreviewActor->SetRepresentation( 3 );

      double anRGB[3];
      vtkProperty* aProp = vtkProperty::New();
      GetColor( "SMESH", "selection_element_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
      aProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
      myPreviewActor->SetProperty( aProp );
      double aFactor,aUnits;
      myPreviewActor->SetResolveCoincidentTopology(true);
      myPreviewActor->GetPolygonOffsetParameters(aFactor,aUnits);
      myPreviewActor->SetPolygonOffsetParameters(aFactor,0.2*aUnits);
      aProp->Delete();

      myViewWindow->AddActor( myPreviewActor );
      
    }


    typedef std::vector<vtkIdType> TVTKIds;
    void SetPosition(SMESH_Actor* theActor, 
                     vtkIdType theType, 
                     const TVTKIds& theIds,
                     bool theReset=true)
    {
      vtkUnstructuredGrid *aGrid = theActor->GetUnstructuredGrid();
      myGrid->SetPoints(aGrid->GetPoints());

      ResetGrid(theReset);
      
      vtkIdList *anIds = vtkIdList::New();

      for (int i = 0, iEnd = theIds.size(); i < iEnd; i++)
        anIds->InsertId(i,theIds[i]);

      myGrid->InsertNextCell(theType,anIds);
      if(theIds.size()!=0){
        myGrid->InsertNextCell(theType,anIds);
        myGrid->Modified();
      }
        
      anIds->Delete();

      SetVisibility(true);

    }
  
    void ResetGrid(bool theReset=true){
      if (theReset) myGrid->Reset();
    }

    void SetVisibility(bool theVisibility){
      myPreviewActor->SetVisibility(theVisibility);
      RepaintCurrentView();
    }


    ~TPolySimulation(){
      if( myViewWindow )
        myViewWindow->RemoveActor(myPreviewActor);

      myPreviewActor->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myGrid->Delete();
    }

  };
}

//=================================================================================
// class    : SMESHGUI_CreatePolyhedralVolumeDlgDlg()
// purpose  : 
//=================================================================================
SMESHGUI_CreatePolyhedralVolumeDlg::SMESHGUI_CreatePolyhedralVolumeDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH",tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle( tr( "SMESH_CREATE_POLYHEDRAL_VOLUME_TITLE" ) );
  setSizeGripEnabled( true );

  QVBoxLayout* topLayout = new QVBoxLayout( this ); 
  topLayout->setSpacing( SPACING );
  topLayout->setMargin( MARGIN );

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr( "SMESH_ELEMENTS_TYPE" ), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout( ConstructorsBox );
  ConstructorsBoxLayout->setSpacing( SPACING );
  ConstructorsBoxLayout->setMargin( MARGIN );

  RadioButton1 = new QRadioButton( tr( "MESH_NODE" ),  ConstructorsBox );
  RadioButton2 = new QRadioButton( tr( "SMESH_FACE" ), ConstructorsBox );

  ConstructorsBoxLayout->addWidget( RadioButton1 );
  ConstructorsBoxLayout->addWidget( RadioButton2 );
  GroupConstructors->addButton(RadioButton1, 0);
  GroupConstructors->addButton(RadioButton2, 1);
  
  /***************************************************************/
  GroupContent = new QGroupBox( tr( "SMESH_CONTENT" ), this );
  QGridLayout* GroupContentLayout = new QGridLayout( GroupContent );
  GroupContentLayout->setSpacing( SPACING );
  GroupContentLayout->setMargin( MARGIN );
  
  TextLabelIds = new QLabel( tr( "SMESH_ID_NODES" ), GroupContent );
  SelectElementsButton  = new QPushButton( GroupContent );
  SelectElementsButton->setIcon( image0 );
  LineEditElements  = new QLineEdit( GroupContent );
  LineEditElements->setValidator( new SMESHGUI_IdValidator( this ) );

  myFacesByNodesLabel = new QLabel( tr( "FACES_BY_NODES" ), GroupContent );
  myFacesByNodes = new QListWidget( GroupContent);
  myFacesByNodes->setSelectionMode( QListWidget::ExtendedSelection );
  myFacesByNodes->setMinimumHeight( 150);

  AddButton = new QPushButton( tr( "SMESH_BUT_ADD" ), GroupContent );
  RemoveButton = new QPushButton( tr( "SMESH_BUT_REMOVE" ), GroupContent );

  Preview = new QCheckBox( tr( "SMESH_POLYEDRE_PREVIEW" ), GroupContent );

  GroupContentLayout->addWidget( TextLabelIds,         0, 0 );
  GroupContentLayout->addWidget( SelectElementsButton, 0, 1 );
  GroupContentLayout->addWidget( LineEditElements,     0, 2, 1, 2 );
  GroupContentLayout->addWidget( myFacesByNodesLabel,  1, 0 );
  GroupContentLayout->addWidget( myFacesByNodes,       2, 0, 3, 3 );
  GroupContentLayout->addWidget( AddButton,            2, 3 );
  GroupContentLayout->addWidget( RemoveButton,         3, 3 );
  GroupContentLayout->addWidget( Preview,              5, 0, 1, 4 );

  /***************************************************************/
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

  /***************************************************************/
  GroupButtons = new QGroupBox( this );
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout( GroupButtons );
  GroupButtonsLayout->setSpacing( SPACING );
  GroupButtonsLayout->setMargin( MARGIN );

  buttonOk = new QPushButton( tr( "SMESH_BUT_APPLY_AND_CLOSE" ), GroupButtons );
  buttonOk->setAutoDefault( true );
  buttonOk->setDefault( true );
  buttonApply = new QPushButton( tr( "SMESH_BUT_APPLY" ), GroupButtons );
  buttonApply->setAutoDefault( true );
  buttonCancel = new QPushButton( tr( "SMESH_BUT_CLOSE" ), GroupButtons );
  buttonCancel->setAutoDefault( true );
  buttonHelp = new QPushButton( tr("SMESH_BUT_HELP" ), GroupButtons );
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget( buttonOk );
  GroupButtonsLayout->addSpacing( 10 );
  GroupButtonsLayout->addWidget( buttonApply );
  GroupButtonsLayout->addSpacing( 10 );
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget( buttonCancel );
  GroupButtonsLayout->addWidget( buttonHelp);

  /***************************************************************/
  topLayout->addWidget( ConstructorsBox );
  topLayout->addWidget( GroupContent );
  topLayout->addWidget( GroupGroups );
  topLayout->addWidget( GroupButtons );
  
  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();
  
  RadioButton1->setChecked( true );
 
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this );

  myHelpFileName = "adding_nodes_and_elements_page.html#adding_polyhedrons_anchor";
  
  Init();
}

//=================================================================================
// function : ~SMESHGUI_CreatePolyhedralVolumeDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_CreatePolyhedralVolumeDlg::~SMESHGUI_CreatePolyhedralVolumeDlg()
{
  delete mySimulation;
}

static bool busy = false;

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::Init()
{
  myEditCurrentArgument = LineEditElements;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this );

  /* reset "Add to group" control */
  GroupGroups->setChecked( false );

  myNbOkElements = 0;
  myActor = 0;

  mySimulation = new SMESH::TPolySimulation( dynamic_cast<SalomeApp_Application*>( mySMESHGUI->application() ) );

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL( clicked() ), SLOT( ClickOnOk() ) );
  connect(buttonCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  connect(buttonApply,  SIGNAL( clicked() ), SLOT( ClickOnApply() ) );
  connect(buttonHelp,   SIGNAL( clicked() ), SLOT( ClickOnHelp() ) );

  connect(GroupConstructors, SIGNAL(buttonClicked(int) ), SLOT( ConstructorsClicked(int) ) );
  connect(SelectElementsButton, SIGNAL( clicked() ), SLOT( SetEditCurrentArgument() ) );
  connect(LineEditElements, SIGNAL( textChanged(const QString&) ), SLOT(onTextChange(const QString&)));

  connect(myFacesByNodes, SIGNAL(itemSelectionChanged()), this, SLOT(onListSelectionChanged()));
  connect(AddButton, SIGNAL(clicked()), this, SLOT(onAdd()));
  connect(RemoveButton, SIGNAL(clicked()), this, SLOT(onRemove()));
  
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) );
  connect( mySelectionMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  connect( Preview, SIGNAL(toggled(bool)), this, SLOT(ClickOnPreview(bool)));
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ),      this, SLOT( reject() ) );
  connect( mySMESHGUI, SIGNAL ( SignalActivatedViewManager() ), this, SLOT( onOpenView() ) );
  connect( mySMESHGUI, SIGNAL ( SignalCloseView() ),            this, SLOT( onCloseView() ) );
  ConstructorsClicked(0);
  SelectionIntoArgument();
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ConstructorsClicked(int constructorId)
{
  //disconnect(mySelectionMgr, 0, this, 0);

  SALOME_ListIO io;
  mySelectionMgr->selectedObjects( io );
  SALOME_ListIO aList;
  mySelectionMgr->setSelectedObjects( aList );
  myEditCurrentArgument->clear();
  myNbOkElements = 0;
  buttonApply->setEnabled(false);
  buttonOk->setEnabled(false);
  mySimulation->SetVisibility(false);

  switch(constructorId)
    {
    case 0 :
      { 
        if ( myActor ){
          myActor->SetPointRepresentation(true);
        }
        else
          SMESH::SetPointRepresentation(true);
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(NodeSelection);
        
        AddButton->setEnabled(false);
        RemoveButton->setEnabled(false);
        TextLabelIds->setText( tr( "SMESH_ID_NODES" ) );
        myFacesByNodesLabel->show();
        myFacesByNodes->clear();
        myFacesByNodes->show();
        AddButton->show();
        RemoveButton->show();
        Preview->show();
        break;
      }
    case 1 :
      {
        if( myActor ){
          myActor->SetPointRepresentation(false);
        } else {
          SMESH::SetPointRepresentation(false);
        }
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->SetSelectionMode(FaceSelection);
        
        TextLabelIds->setText( tr( "SMESH_ID_FACES" ) );
        myFacesByNodesLabel->hide();
        myFacesByNodes->hide();
        AddButton->hide();
        RemoveButton->hide();
        Preview->show();
        break;
      }
    }
  
  //connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  mySelectionMgr->setSelectedObjects( io );

  QApplication::instance()->processEvents();
  updateGeometry();
  resize(100,100);
}

//=================================================================================
// function : ClickOnPreview()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ClickOnPreview(bool theToggled){
  Preview->setChecked(theToggled);
  displaySimulation();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ClickOnApply()
{
  if( !isValid() )
    return;

  if ( myNbOkElements>0 && !mySMESHGUI->isActiveStudyLocked())
    {
      if(checkEditLine(false) == -1) {return;}
      busy = true;
      long anElemId = -1;

      bool addToGroup = GroupGroups->isChecked();
      QString aGroupName;
      
      SMESH::SMESH_GroupBase_var aGroup;
      int idx = 0;
      if( addToGroup ) {
        aGroupName = ComboBox_GroupName->currentText();
        for ( int i = 1; i <= ComboBox_GroupName->count(); i++ ) {
          QString aName = ComboBox_GroupName->itemText( i );
          if ( aGroupName == aName && ( i == ComboBox_GroupName->currentIndex() || idx == 0 ) )
            idx = i;
        }
        if ( idx > 0 && idx <= myGroups.count() ) {
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

      if (GetConstructorId() == 0)
        {
          SMESH::long_array_var anIdsOfNodes = new SMESH::long_array;
          SMESH::long_array_var aQuantities  = new SMESH::long_array;

          aQuantities->length( myFacesByNodes->count() );

          TColStd_ListOfInteger aNodesIds;

          int aNbQuantities = 0;
          for (int i = 0; i < myFacesByNodes->count(); i++ ) {
            QStringList anIds = myFacesByNodes->item(i)->text().split( " ", QString::SkipEmptyParts );
            for (QStringList::iterator it = anIds.begin(); it != anIds.end(); ++it)
              aNodesIds.Append( (*it).toInt() );

            aQuantities[aNbQuantities++] = anIds.count();
          }

          anIdsOfNodes->length(aNodesIds.Extent());

          int aNbIdsOfNodes = 0;
          TColStd_ListIteratorOfListOfInteger It;
          It.Initialize(aNodesIds);
          for( ;It.More();It.Next())
            anIdsOfNodes[aNbIdsOfNodes++] = It.Value();
            
          try{
            SUIT_OverrideCursor aWaitCursor;
            SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
            anElemId = aMeshEditor->AddPolyhedralVolume(anIdsOfNodes, aQuantities);
          }catch(SALOME::SALOME_Exception& exc){
            INFOS("Follow exception was cought:\n\t"<<exc.details.text);
          }catch(std::exception& exc){
            INFOS("Follow exception was cought:\n\t"<<exc.what());
          }catch(...){
            INFOS("Unknown exception was cought !!!");
          }
        }
      else if (GetConstructorId() == 1)
        {
          SMESH::long_array_var anIdsOfFaces = new SMESH::long_array;
          
          QStringList aListId = myEditCurrentArgument->text().split( " ", QString::SkipEmptyParts );
          anIdsOfFaces->length(aListId.count());
          for ( int i = 0; i < aListId.count(); i++ )
            anIdsOfFaces[i] = aListId[i].toInt();
          
          try{
            SUIT_OverrideCursor aWaitCursor;
            SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
            anElemId = aMeshEditor->AddPolyhedralVolumeByFaces(anIdsOfFaces);
          }catch(SALOME::SALOME_Exception& exc){
            INFOS("Follow exception was cought:\n\t"<<exc.details.text);
          }catch(std::exception& exc){
            INFOS("Follow exception was cought:\n\t"<<exc.what());
          }catch(...){
            INFOS("Unknown exception was cought !!!");
          }
        }

      if ( anElemId != -1 && addToGroup && !aGroupName.isEmpty() ) {
        SMESH::SMESH_Group_var aGroupUsed;
        if ( aGroup->_is_nil() ) {
          // create new group 
          aGroupUsed = SMESH::AddGroup( myMesh, SMESH::VOLUME, aGroupName );
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

      //SALOME_ListIO aList;
      //mySelectionMgr->setSelectedObjects( aList );
      SMESH::UpdateView();
      if( myActor ){
        unsigned int anEntityMode = myActor->GetEntityMode();
        myActor->SetEntityMode(SMESH_Actor::eVolumes | anEntityMode);
      }
      //ConstructorsClicked( GetConstructorId() );
      busy = false;

      SMESHGUI::Modified();
    }
    myFacesByNodes->clear();
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ClickOnOk()
{
  if(checkEditLine(false) == -1) {return;}
  ClickOnApply();
  reject();
}

        
//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::reject()
{
  mySelectionMgr->clearFilters();
  //SALOME_ListIO aList;
  //mySelectionMgr->setSelectedObjects( aList );
  SMESH::SetPointRepresentation(false);
  mySimulation->SetVisibility(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  disconnect( mySelectionMgr, 0, this, 0 );
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::onOpenView()
{
  if ( mySelector && mySimulation ) {
    mySimulation->SetVisibility(false);
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    mySimulation = new SMESH::TPolySimulation(
      dynamic_cast<SalomeApp_Application*>( mySMESHGUI->application() ) );
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
  delete mySimulation;
  mySimulation = 0;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ClickOnHelp()
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

//=======================================================================
//function : onTextChange
//purpose  : 
//=======================================================================

void SMESHGUI_CreatePolyhedralVolumeDlg::onTextChange(const QString& theNewText)
{
  if ( busy ) return;
  if (checkEditLine() == -1) return;
  busy = true;

  mySimulation->SetVisibility(false);

  SMDS_Mesh* aMesh = 0;
  if ( myActor )
    aMesh = myActor->GetObject()->GetMesh();

  if (GetConstructorId() == 0)
    {
      if ( aMesh ) {
        TColStd_MapOfInteger newIndices;
      
        QStringList aListId = theNewText.split( " ", QString::SkipEmptyParts );
        for ( int i = 0; i < aListId.count(); i++ ) {
          const SMDS_MeshNode * n = aMesh->FindNode( aListId[ i ].toInt() );
          if ( n ) {
            newIndices.Add(n->GetID());
            myNbOkElements++;
          }
        }
      
        mySelector->AddOrRemoveIndex( myActor->getIO(), newIndices, false );
      
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->highlight( myActor->getIO(), true, true );
      
        if ( myNbOkElements>0 && aListId.count()>=3)
          AddButton->setEnabled(true);
        else
          AddButton->setEnabled(false);
      
        displaySimulation();
      }
    } else if (GetConstructorId() == 1)
      {
        myNbOkElements = 0;
        buttonOk->setEnabled( false );
        buttonApply->setEnabled( false );
      
        // check entered ids of faces and hilight them
        QStringList aListId;
        if ( aMesh ) {
          TColStd_MapOfInteger newIndices;
      
          aListId = theNewText.split( " ", QString::SkipEmptyParts );

          for ( int i = 0; i < aListId.count(); i++ ) {
            const SMDS_MeshElement * e = aMesh->FindElement( aListId[ i ].toInt() );
            if ( e ) {
              newIndices.Add(e->GetID());
              myNbOkElements++;  
            }
          }

          mySelector->AddOrRemoveIndex( myActor->getIO(), newIndices, false );
          if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
            aViewWindow->highlight( myActor->getIO(), true, true );
      
          if ( myNbOkElements ) {
            if (aListId.count()>1){ 
              buttonOk->setEnabled( true );
              buttonApply->setEnabled( true );
            }
            else{
              buttonOk->setEnabled( false );
              buttonApply->setEnabled( false );
            }
            if(aListId.count()>1)
              displaySimulation();
          }
        }
      }
  busy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::SelectionIntoArgument()
{
  if ( busy ) return;
  
  // clear
  
  if (GetConstructorId() == 1 || myFacesByNodes->count() <= 1)
    {
      myNbOkElements = 0;
      AddButton->setEnabled(false);
      buttonOk->setEnabled( false );
      buttonApply->setEnabled( false );
    }

  myActor = 0;

  busy = true;
  myEditCurrentArgument->setText( "" );
  busy = false;
  if ( !GroupButtons->isEnabled() ) // inactive
    return;
  
  mySimulation->SetVisibility(false);
  
  QString aCurrentEntry = myEntry;

  // get selected mesh
  
  SALOME_ListIO selected;
  mySelectionMgr->selectedObjects( selected );
  int nbSel = selected.Extent();
  if(nbSel != 1){
    return;
  }
  
  myEntry = selected.First()->getEntry();
  myMesh = SMESH::GetMeshByIO( selected.First() );
  if ( myMesh->_is_nil() )
    return;
  
  // process groups
  if ( !myMesh->_is_nil() && myEntry != aCurrentEntry ) {
    myGroups.clear();
    ComboBox_GroupName->clear();
    ComboBox_GroupName->addItem( QString() );
    SMESH::ListOfGroups aListOfGroups = *myMesh->GetGroups();
    for ( int i = 0, n = aListOfGroups.length(); i < n; i++ ) {
      SMESH::SMESH_GroupBase_var aGroup = aListOfGroups[i];
      if ( !aGroup->_is_nil() && aGroup->GetType() == SMESH::VOLUME ) {
        QString aGroupName( aGroup->GetName() );
        if ( !aGroupName.isEmpty() ) {
          myGroups.append(SMESH::SMESH_GroupBase::_duplicate(aGroup));
          ComboBox_GroupName->addItem( aGroupName );
        }
      }
    }
  }

  myActor = SMESH::FindActorByObject(myMesh);
  if ( !myActor )
    return;
  
  // get selected nodes/faces
  QString aString = "";
  int anbNodes=0,aNbFaces=0;
  switch(GetConstructorId()){
  case 0:{
    anbNodes = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aString);
    if (anbNodes >= 3)
      AddButton->setEnabled(true);
    else if (anbNodes < 3){
      AddButton->setEnabled(false);
    }
    busy = true;
    myEditCurrentArgument->setText( aString );
    if (checkEditLine() == -1) {busy = false;return;}
    busy = false;
    break;
  }
  case 1:{
    // get selected faces
    aNbFaces = SMESH::GetNameOfSelectedElements(mySelector, myActor->getIO(), aString);
    if (aNbFaces<=1){
      buttonOk->setEnabled( false );
      buttonApply->setEnabled( false );
    } else {
      buttonOk->setEnabled( true );
      buttonApply->setEnabled( true );
    }
    busy = true;
    myEditCurrentArgument->setText( aString );
    if (checkEditLine() == -1) {busy = false;return;}
    busy = false;
    
    // OK
    myNbOkElements = 1;
    break;
  }
  default: return;
  }
  if(anbNodes>2 || aNbFaces>1)
    displaySimulation();
}

/*\brief int SMESHGUI_CreatePolyhedralVolumeDlg::checkEditLine()
 * Checking of indices in edit line.
 * If incorecct indices in edit line warning message appear and myEditCurrentArgument remove last index.
 * \retval 1 - if all ok(or no indices in edit line), -1 - if there are incorrect indices.
 */
int SMESHGUI_CreatePolyhedralVolumeDlg::checkEditLine(bool checkLast)
{
  QString aString = "";
  SMDS_Mesh* aMesh = 0;
  
  if(myMesh->_is_nil()) return 1;
  if(!myActor){
    myActor = SMESH::FindActorByObject(myMesh);
    if(!myActor)
      return 1;
  }
    
  aMesh = myActor->GetObject()->GetMesh();

  // checking for nodes
  if (checkLast && myEditCurrentArgument->text().right(1) != QString(" ") ) return 1;
  QStringList aListId = myEditCurrentArgument->text().split( " ", QString::SkipEmptyParts );
  for ( int i = 0; i < aListId.count(); i++ ){
    switch (GetConstructorId()){
    case 0:{ // nodes
      const SMDS_MeshNode    * aNode = aMesh->FindNode( aListId[ i ].toInt() );
      if( !aNode ){
        SUIT_MessageBox::warning(this,
                                 tr("SMESH_POLYEDRE_CREATE_ERROR"),
                                 tr("The incorrect indices of nodes!"));
        
        myEditCurrentArgument->clear();
        myEditCurrentArgument->setText( aString );
        return -1;
      }

      break;
    }
    case 1:{ // faces
      bool aElemIsOK = true;
      const SMDS_MeshElement * aElem = aMesh->FindElement( aListId[ i ].toInt() );
      if (!aElem)
        {
          aElemIsOK = false;
        }
      else
        {
          SMDSAbs_ElementType aType = aMesh->GetElementType( aElem->GetID(),true );
          if (aType != SMDSAbs_Face){
            aElemIsOK = false;
          }
        }
      if (!aElemIsOK){
        SUIT_MessageBox::warning(this,
                                 tr("SMESH_POLYEDRE_CREATE_ERROR"),
                                 tr("The incorrect indices of faces!"));
        
        myEditCurrentArgument->clear();
        myEditCurrentArgument->setText( aString );
        return -1;
      }
      break;
    }
    }
    aString += aListId[ i ] + " "; 
  }

  return 1;
}

//=======================================================================
//function : displaySimulation
//purpose  : 
//=======================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::displaySimulation()
{
  if ( (myNbOkElements || AddButton->isEnabled()) && GroupButtons->isEnabled() && myActor)
    {
      SMESH::TPolySimulation::TVTKIds aVTKIds;
      vtkIdType aType = VTK_CONVEX_POINT_SET;
      SMDS_Mesh* aMesh = 0;
      if ( myActor ){
        aMesh = myActor->GetObject()->GetMesh();
      }
      if (GetConstructorId() == 0 && aMesh){
        if (!AddButton->isEnabled()){
          mySimulation->ResetGrid(true);
          for (int i = 0; i < myFacesByNodes->count(); i++) {
            QStringList anIds = myFacesByNodes->item(i)->text().split( " ", QString::SkipEmptyParts );
            SMESH::TPolySimulation::TVTKIds aVTKIds_faces;
            for (QStringList::iterator it = anIds.begin(); it != anIds.end(); ++it){
              const SMDS_MeshNode* aNode = aMesh->FindNode( (*it).toInt() );
              if (!aNode) continue;
              vtkIdType aId = myActor->GetObject()->GetNodeVTKId( (*it).toInt() );
              aVTKIds.push_back(aId);
              aVTKIds_faces.push_back(aId);
            }
            if(!Preview->isChecked()){
              aType = VTK_POLYGON;
              mySimulation->SetPosition(myActor, aType, aVTKIds_faces,false);
            }
          }
          if(myFacesByNodes->count() == 0){
            mySimulation->SetVisibility(false);
          } else {
            mySimulation->SetVisibility(true);
          }
          if(Preview->isChecked()){
            mySimulation->SetPosition(myActor, aType, aVTKIds);
          }
        } else {
          // add ids from edit line
          QStringList anEditIds = myEditCurrentArgument->text().split( " ", QString::SkipEmptyParts );
          for ( int i = 0; i < anEditIds.count(); i++ )
            aVTKIds.push_back( myActor->GetObject()->GetNodeVTKId( anEditIds[ i ].toInt() ));
          aType = VTK_POLYGON;
          mySimulation->SetPosition(myActor, aType, aVTKIds);
        }
      }else if(GetConstructorId() == 1 && aMesh){
        QStringList aListId = myEditCurrentArgument->text().split( " ", QString::SkipEmptyParts );
        for ( int i = 0; i < aListId.count(); i++ )
          {
            const SMDS_MeshElement * anElem = aMesh->FindElement( aListId[ i ].toInt() );
            if ( !anElem ) continue;
            SMDSAbs_ElementType aFaceType = aMesh->GetElementType( anElem->GetID(),true );
            if (aFaceType != SMDSAbs_Face) continue;
              
            SMDS_ElemIteratorPtr anIter = anElem->nodesIterator();
            SMESH::TPolySimulation::TVTKIds aVTKIds_faces;
            while( anIter->more() )
              if ( const SMDS_MeshNode* aNode = (SMDS_MeshNode*)anIter->next() ){
                vtkIdType aId = myActor->GetObject()->GetNodeVTKId( aNode->GetID() );
                aVTKIds.push_back(aId);
                aVTKIds_faces.push_back(aId);
              }
            if(!Preview->isChecked()){
              aType = VTK_POLYGON;
              mySimulation->SetPosition(myActor, aType, aVTKIds_faces);
            }
          }
        if(Preview->isChecked())
          mySimulation->SetPosition(myActor, aType, aVTKIds);
      }
      SMESH::UpdateView();
    }
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  if(send == SelectElementsButton) {
    LineEditElements->setFocus();
    myEditCurrentArgument = LineEditElements;
  }
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::DeactivateActiveDialog()
{
  if ( ConstructorsBox->isEnabled() ) {
    ConstructorsBox->setEnabled(false);
    GroupContent->setEnabled(false);
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
void SMESHGUI_CreatePolyhedralVolumeDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();   
  ConstructorsBox->setEnabled(true);
  GroupContent->setEnabled(true);
  GroupButtons->setEnabled(true);
  
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this );

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( FaceSelection );
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::enterEvent (QEvent*)
{
  if ( !ConstructorsBox->isEnabled() ) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector && !mySimulation) {
      mySelector = aViewWindow->GetSelector();
      mySimulation = new SMESH::TPolySimulation(
        dynamic_cast<SalomeApp_Application*>( mySMESHGUI->application() ) );
    }
    ActivateThisDialog();
  }
}

//=================================================================================
// function : GetConstructorId()
// purpose  : 
//=================================================================================
int SMESHGUI_CreatePolyhedralVolumeDlg::GetConstructorId()
{ 
  return GroupConstructors->checkedId();
}

//=================================================================================
// function : onAdd()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::onAdd()
{
  SALOME_ListIO selected;
  mySelectionMgr->selectedObjects( selected );
  int aNbSel = selected.Extent();
  if (aNbSel == 0 || !myActor || myMesh->_is_nil()) return;
  
  if (checkEditLine(false) == -1) return;

  busy = true;
  if ( !(myEditCurrentArgument->text().isEmpty()) )
    {
      myFacesByNodes->addItem(myEditCurrentArgument->text());
      //myFacesByNodes->setSelected(myFacesByNodes->count() - 1, true);
      myNbOkElements = 1;
      myEditCurrentArgument->clear();
      AddButton->setEnabled(false);
      buttonApply->setEnabled( myFacesByNodes->count() > 1 );
      buttonOk->setEnabled( myFacesByNodes->count() > 1 );
    }
  busy = false;
  onListSelectionChanged();
  displaySimulation();
}

//=================================================================================
// function : onRemove()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::onRemove()
{
  busy = true;
  QList<QListWidgetItem*> selItems = myFacesByNodes->selectedItems();
  QListWidgetItem* anItem;

  if ( selItems.count() > 0 ) myNbOkElements = 1;

  foreach( anItem, selItems )
    delete anItem;

  RemoveButton->setEnabled( myFacesByNodes->count() > 0 );
  buttonOk->setEnabled( myFacesByNodes->count() > 1 );
  buttonApply->setEnabled( myFacesByNodes->count() > 1 );

  busy = false;
  displaySimulation();
}

//=================================================================================
// function : onListSelectionChanged()
// purpose  : Called when selection in element list is changed
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::onListSelectionChanged()
{
  if (busy || !myActor) return;
  busy = true;

  SALOME_ListIO aList;
  mySelectionMgr->setSelectedObjects( aList );
  TColStd_MapOfInteger aIndexes;

  QList<QListWidgetItem*> selItems = myFacesByNodes->selectedItems();
  QListWidgetItem* anItem;
  foreach( anItem, selItems ) {
    QStringList anIds = anItem->text().split( " ", QString::SkipEmptyParts );
    for (QStringList::iterator it = anIds.begin(); it != anIds.end(); ++it)
      aIndexes.Add((*it).toInt());
  }
  RemoveButton->setEnabled(selItems.count() > 0);
  mySelector->AddOrRemoveIndex(myActor->getIO(), aIndexes, true );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->highlight( myActor->getIO(), true, true );
  mySelectionMgr->clearFilters(); 
  aList.Append( myActor->getIO() );
  mySelectionMgr->setSelectedObjects( aList );
  
  busy = false;
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::keyPressEvent( QKeyEvent* e )
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
bool SMESHGUI_CreatePolyhedralVolumeDlg::isValid()
{
  if( GroupGroups->isChecked() && ComboBox_GroupName->currentText().isEmpty() ) {
    SUIT_MessageBox::warning( this, tr( "SMESH_WRN_WARNING" ), tr( "GROUP_NAME_IS_EMPTY" ) );
    return false;
  }
  return true;
}
