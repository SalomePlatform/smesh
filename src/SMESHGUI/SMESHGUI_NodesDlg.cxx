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
// File   : SMESHGUI_NodesDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_NodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_ObjectDef.h>

#include <SMDS_Mesh.hxx>
#include <SMDS_MeshNode.hxx>

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SalomeApp_Application.h>

#include <SVTK_ViewWindow.h>
#include <VTKViewer_Algorithm.h>
#include <VTKViewer_CellLocationsArray.h>

// SALOME KERNEL includes
#include <SALOMEDS_Study.hxx>
#include <SALOMEDS_SObject.hxx>

#include <utilities.h>

// VTK includes
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkPoints.h>

// Qt includes
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QButtonGroup>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

namespace SMESH
{
  long AddNode( SMESH::SMESH_Mesh_ptr theMesh, float x, float y, float z, const QStringList& theParameters )
  {
    long aNodeId = -1;
    SUIT_OverrideCursor wc;
    try {
      _PTR(SObject) aSobj = SMESH::FindSObject( theMesh );
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      theMesh->SetParameters( theParameters.join(":").toLatin1().constData() );
      aNodeId = aMeshEditor->AddNode( x, y, z );
      _PTR(Study) aStudy = GetActiveStudyDocument();
      CORBA::Long anId = aStudy->StudyId();
      if (TVisualObjPtr aVisualObj = SMESH::GetVisualObj( anId, aSobj->GetID().c_str() ) ) {
        aVisualObj->Update( true );
      }
    } 
    catch ( SALOME::SALOME_Exception& exc ) {
      INFOS( "Follow exception was cought:\n\t" << exc.details.text );
    }
    catch ( const std::exception& exc ) {
      INFOS( "Follow exception was cought:\n\t" << exc.what() );
    } 
    catch ( ... ) {
      INFOS( "Unknown exception was cought !!!" );
    }
    return aNodeId;
  }

  class TNodeSimulation 
  {
    SVTK_ViewWindow*  myViewWindow;

    SALOME_Actor*     myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkPoints*        myPoints;

  public:
    TNodeSimulation( SVTK_ViewWindow* theViewWindow ):
      myViewWindow( theViewWindow )
    {
      vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();

      // Create points
      myPoints = vtkPoints::New();
      myPoints->SetNumberOfPoints( 1 );
      myPoints->SetPoint( 0, 0.0, 0.0, 0.0 );

      // Create cells
      vtkIdList *anIdList = vtkIdList::New();
      anIdList->SetNumberOfIds( 1 );

      vtkCellArray *aCells = vtkCellArray::New();
      aCells->Allocate( 2, 0 );

      vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
      aCellTypesArray->SetNumberOfComponents( 1 );
      aCellTypesArray->Allocate( 1 );

      anIdList->SetId( 0, 0 );
      aCells->InsertNextCell( anIdList );
      aCellTypesArray->InsertNextValue( VTK_VERTEX );

      VTKViewer_CellLocationsArray* aCellLocationsArray = VTKViewer_CellLocationsArray::New();
      aCellLocationsArray->SetNumberOfComponents( 1 );
      aCellLocationsArray->SetNumberOfTuples( 1 );

      aCells->InitTraversal();
      vtkIdType npts = 0;
      aCellLocationsArray->SetValue( 0, aCells->GetTraversalLocation( npts ) );

      aGrid->SetCells( aCellTypesArray, aCellLocationsArray, aCells );

      aGrid->SetPoints( myPoints );
      aGrid->SetCells( aCellTypesArray, aCellLocationsArray, aCells );
      aCellLocationsArray->Delete();
      aCellTypesArray->Delete();
      aCells->Delete();
      anIdList->Delete();

      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInputData( aGrid );
      aGrid->Delete();

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->SetInfinitive( true );
      myPreviewActor->VisibilityOff();
      myPreviewActor->PickableOff();
      myPreviewActor->SetMapper( myMapper );

      vtkProperty* aProp = vtkProperty::New();
      aProp->SetRepresentationToPoints();

      double anRGB[3];
      GetColor( "SMESH", "node_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 255, 0 ) );
      aProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );

      double aPointSize = GetFloat( "SMESH:node_size", 3 );
      aProp->SetPointSize( aPointSize );

      myPreviewActor->SetProperty( aProp );
      aProp->Delete();

      myViewWindow->AddActor( myPreviewActor );
    }

    void SetPosition( float x, float y, float z )
    {
      myPoints->SetPoint( 0, x, y, z );
      myPoints->Modified();
      SetVisibility( true );
    }

    void SetVisibility( bool theVisibility )
    {
      myPreviewActor->SetVisibility( theVisibility );
      RepaintCurrentView();
    }

    ~TNodeSimulation()
    {
      myViewWindow->RemoveActor( myPreviewActor );
      myPreviewActor->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myPoints->Delete();
    }
  };
}

//=================================================================================
// class    : SMESHGUI_NodesDlg()
// purpose  :
//=================================================================================
SMESHGUI_NodesDlg::SMESHGUI_NodesDlg( SMESHGUI* theModule ): 
  QDialog( SMESH::GetDesktop( theModule ) ),
  mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
  mySelector( SMESH::GetViewWindow( theModule )->GetSelector() ),
  mySMESHGUI( theModule )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr("MESH_NODE_TITLE") );
  setSizeGripEnabled( true );
  
  mySimulation = new SMESH::TNodeSimulation( SMESH::GetViewWindow( mySMESHGUI ) );
  
  QPixmap image0( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", 
                                                                   tr( "ICON_DLG_NODE" ) ) );
  
  QVBoxLayout* SMESHGUI_NodesDlgLayout = new QVBoxLayout( this );
  SMESHGUI_NodesDlgLayout->setSpacing( SPACING );
  SMESHGUI_NodesDlgLayout->setMargin( MARGIN );

  /***************************************************************/
  GroupConstructors = new QGroupBox( tr( "MESH_NODE" ), this );
  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* GroupConstructorsLayout = new QHBoxLayout( GroupConstructors );
  GroupConstructorsLayout->setSpacing( SPACING );
  GroupConstructorsLayout->setMargin( MARGIN );

  Constructor1 = new QRadioButton( GroupConstructors );
  Constructor1->setIcon( image0 );
  Constructor1->setChecked( true );
  
  GroupConstructorsLayout->addWidget( Constructor1 );
  ButtonGroup->addButton( Constructor1, 0 );

  /***************************************************************/
  GroupCoordinates = new QGroupBox( tr( "SMESH_COORDINATES" ), this );
  QHBoxLayout* GroupCoordinatesLayout = new QHBoxLayout(GroupCoordinates);
  GroupCoordinatesLayout->setSpacing(SPACING);
  GroupCoordinatesLayout->setMargin(MARGIN);

  TextLabel_X = new QLabel( tr( "SMESH_X" ), GroupCoordinates );
  SpinBox_X = new SMESHGUI_SpinBox( GroupCoordinates );

  TextLabel_Y = new QLabel( tr( "SMESH_Y" ), GroupCoordinates );
  SpinBox_Y = new SMESHGUI_SpinBox( GroupCoordinates );

  TextLabel_Z = new QLabel( tr( "SMESH_Z" ), GroupCoordinates );
  SpinBox_Z = new SMESHGUI_SpinBox( GroupCoordinates );

  GroupCoordinatesLayout->addWidget( TextLabel_X );
  GroupCoordinatesLayout->addWidget( SpinBox_X ); 
  GroupCoordinatesLayout->addWidget( TextLabel_Y);
  GroupCoordinatesLayout->addWidget( SpinBox_Y );
  GroupCoordinatesLayout->addWidget( TextLabel_Z );
  GroupCoordinatesLayout->addWidget( SpinBox_Z );
  GroupCoordinatesLayout->setStretch(1, 1);
  GroupCoordinatesLayout->setStretch(3, 1);
  GroupCoordinatesLayout->setStretch(5, 1);

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
  buttonHelp = new QPushButton( tr( "SMESH_BUT_HELP" ), GroupButtons );
  buttonHelp->setAutoDefault( true );

  GroupButtonsLayout->addWidget( buttonOk );
  GroupButtonsLayout->addSpacing( 10 );
  GroupButtonsLayout->addWidget( buttonApply );
  GroupButtonsLayout->addSpacing( 10 );
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget( buttonCancel );
  GroupButtonsLayout->addWidget( buttonHelp );

  /***************************************************************/
  SMESHGUI_NodesDlgLayout->addWidget( GroupConstructors );
  SMESHGUI_NodesDlgLayout->addWidget( GroupCoordinates );
  SMESHGUI_NodesDlgLayout->addWidget( GroupGroups );
  SMESHGUI_NodesDlgLayout->addWidget( GroupButtons );

  myHelpFileName = "adding_nodes_and_elements_page.html#adding_nodes_anchor";

  /* Initialisation and display */
  Init();
}

//=======================================================================
// function : ~SMESHGUI_NodesDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_NodesDlg::~SMESHGUI_NodesDlg()
{
  delete mySimulation;
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::Init()
{
  /* Get setting of step value from file configuration */
  double step = 25.0;

  /* min, max, step and decimals for spin boxes */
  SpinBox_X->RangeStepAndValidator( COORD_MIN, COORD_MAX, step, "length_precision" );
  SpinBox_Y->RangeStepAndValidator( COORD_MIN, COORD_MAX, step, "length_precision" );
  SpinBox_Z->RangeStepAndValidator( COORD_MIN, COORD_MAX, step, "length_precision" );
  SpinBox_X->SetValue( 0.0 );
  SpinBox_Y->SetValue( 0.0 );
  SpinBox_Z->SetValue( 0.0 );

  /* reset "Add to group" control */
  GroupGroups->setChecked( false );

  mySMESHGUI->SetActiveDialogBox( this );

  /* signals and slots connections */
  connect( buttonOk,     SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( buttonApply,  SIGNAL( clicked() ), this, SLOT( ClickOnApply() ) );
  connect( buttonHelp,   SIGNAL( clicked() ), this, SLOT( ClickOnHelp() ) );

  connect( SpinBox_X, SIGNAL( valueChanged( double ) ), SLOT( ValueChangedInSpinBox( double ) ) );
  connect( SpinBox_Y, SIGNAL( valueChanged( double ) ), SLOT( ValueChangedInSpinBox( double ) ) );
  connect( SpinBox_Z, SIGNAL( valueChanged( double ) ), SLOT( ValueChangedInSpinBox( double ) ) );

  connect( mySelectionMgr, SIGNAL( currentSelectionChanged() ),      SLOT( SelectionIntoArgument() ) );
  connect( mySMESHGUI,     SIGNAL( SignalDeactivateActiveDialog() ), SLOT( DeactivateActiveDialog() ) );
  /* to close dialog if study frame change */
  connect( mySMESHGUI,     SIGNAL( SignalStudyFrameChanged() ),      SLOT( reject() ) );
  connect( mySMESHGUI,     SIGNAL( SignalCloseAllDialogs() ),        SLOT( reject() ) );
  connect( mySMESHGUI,     SIGNAL( SignalActivatedViewManager() ),   SLOT( onOpenView() ) );
  connect( mySMESHGUI,     SIGNAL( SignalCloseView() ),              SLOT( onCloseView() ) );
  // set selection mode
  SMESH::SetPointRepresentation( true );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ) )
    aViewWindow->SetSelectionMode( NodeSelection );

  SelectionIntoArgument();
}

//=================================================================================
// function : ValueChangedInSpinBox()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ValueChangedInSpinBox( double newValue )
{
  if ( !myMesh->_is_nil() ) {
    double vx = SpinBox_X->GetValue();
    double vy = SpinBox_Y->GetValue();
    double vz = SpinBox_Z->GetValue();
    mySimulation->SetPosition( vx, vy, vz );
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ClickOnOk()
{
  if ( ClickOnApply() )
    reject();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_NodesDlg::ClickOnApply()
{
  if ( mySMESHGUI->isActiveStudyLocked() )
    return false;

  if ( myMesh->_is_nil() ) {
    SUIT_MessageBox::warning( this, tr( "SMESH_WRN_WARNING" ),
                              tr( "MESH_IS_NOT_SELECTED" ) );
    return false;
  }

  if( !isValid() )
    return false;

  /* Recup args and call method */
  double x = SpinBox_X->GetValue();
  double y = SpinBox_Y->GetValue();
  double z = SpinBox_Z->GetValue();

  QStringList aParameters;
  aParameters << SpinBox_X->text();
  aParameters << SpinBox_Y->text();
  aParameters << SpinBox_Z->text();

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
        if ( res == 1 ) return false;
      }
      aGroup = myGroups[idx-1];
    }
  }
      
  mySimulation->SetVisibility( false );

  long aNodeId = SMESH::AddNode( myMesh, x, y, z, aParameters );

  SMESH::SetPointRepresentation( true );

  if ( aNodeId != -1 && addToGroup && !aGroupName.isEmpty() ) {
    SMESH::SMESH_Group_var aGroupUsed;
    if ( aGroup->_is_nil() ){
      // create new group 
      aGroupUsed = SMESH::AddGroup( myMesh, SMESH::NODE, aGroupName );
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
      anIdList[0] = aNodeId;
      aGroupUsed->Add( anIdList.inout() );
    }
  }

  // select myMesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );
  if ( aList.Extent() != 1 ) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView() ) {
      VTK::ActorCollectionCopy aCopy(aViewWindow->getRenderer()->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
      aCollection->InitTraversal();
      while ( vtkActor *anAct = aCollection->GetNextActor() ) {
        if ( SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>( anAct ) ) {
          if ( anActor->hasIO() ) {
            if ( SMESH_MeshObj *aMeshObj = dynamic_cast<SMESH_MeshObj*>( anActor->GetObject().get() ) ) {
              if ( myMesh->_is_equivalent( aMeshObj->GetMeshServer() ) ) {
                aList.Clear();
                aList.Append( anActor->getIO() );
                mySelectionMgr->setSelectedObjects( aList, false );
                break;
              }
            }
          }
        }
      }
    }
  }

  SMESHGUI::Modified();
  SMESH::UpdateView();
  mySimulation->SetVisibility(false);

  return true;
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::reject()
{
  disconnect( mySelectionMgr, 0, this, 0 );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ) )
    aViewWindow->SetSelectionMode( ActorSelection );
  mySimulation->SetVisibility( false );
  SMESH::SetPointRepresentation( false );
  mySMESHGUI->ResetState();

  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::onOpenView()
{
  if ( mySelector && mySimulation ) {
    mySimulation->SetVisibility(false);
    SMESH::SetPointRepresentation(false);
  }
  else {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    mySelector = aViewWindow->GetSelector();
    mySimulation = new SMESH::TNodeSimulation(aViewWindow);
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::onCloseView()
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
void SMESHGUI_NodesDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)( SUIT_Session::session()->activeApplication() );
  if ( app ) 
    app->onHelpContextModule( mySMESHGUI ? app->moduleName( mySMESHGUI->moduleName() ) : 
                              QString( "" ), myHelpFileName );
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning( this, tr("WRN_WARNING"),
                              tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                              arg( app->resourceMgr()->stringValue( "ExternalBrowser", 
                                                                    platform ) ).
                              arg( myHelpFileName ) );
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_NodesDlg::SelectionIntoArgument()
{
  if ( !GroupConstructors->isEnabled() )
    return;

  mySimulation->SetVisibility( false );
  SMESH::SetPointRepresentation( true );

  QString aCurrentEntry = myEntry;

  const SALOME_ListIO& aList = mySelector->StoredIObjects();
  if ( aList.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    if ( anIO->hasEntry() ) {
      myEntry = anIO->getEntry();
      myMesh = SMESH::GetMeshByIO( anIO );
      if ( myMesh->_is_nil() ) return;
      QString aText;
      if ( SMESH::GetNameOfSelectedNodes( mySelector, anIO, aText ) == 1 ) {
        if ( SMESH_Actor* anActor = SMESH::FindActorByObject( myMesh.in() ) ) {
          if ( SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh() ) {
            if ( const SMDS_MeshNode* aNode = aMesh->FindNode( aText.toInt() ) ) {
              SpinBox_X->SetValue( aNode->X() );
              SpinBox_Y->SetValue( aNode->Y() );
              SpinBox_Z->SetValue( aNode->Z() );
            }
          }
        }
      }
      mySimulation->SetPosition( SpinBox_X->GetValue(),
                                 SpinBox_Y->GetValue(),
                                 SpinBox_Z->GetValue() );
    }
  }

  // process groups
  if ( !myMesh->_is_nil() && myEntry != aCurrentEntry ) {
    myGroups.clear();
    ComboBox_GroupName->clear();
    ComboBox_GroupName->addItem( QString() );
    SMESH::ListOfGroups aListOfGroups = *myMesh->GetGroups();
    for( int i = 0, n = aListOfGroups.length(); i < n; i++ ) {
      SMESH::SMESH_GroupBase_var aGroup = aListOfGroups[i];
      if ( !aGroup->_is_nil() && aGroup->GetType() == SMESH::NODE ) {
        QString aGroupName( aGroup->GetName() );
        if ( !aGroupName.isEmpty() ) {
          myGroups.append(SMESH::SMESH_GroupBase::_duplicate(aGroup));
          ComboBox_GroupName->addItem( aGroupName );
        }
      }
    }
  }
}

//=================================================================================
// function : enterEvent()
// purpose  : to reactivate this dialog box when mouse enter onto the window
//=================================================================================
void SMESHGUI_NodesDlg::enterEvent( QEvent* )
{
  if ( !GroupConstructors->isEnabled() ) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector && !mySimulation) {
      mySelector = aViewWindow->GetSelector();
      mySimulation = new SMESH::TNodeSimulation(aViewWindow);
    }
    ActivateThisDialog();
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : public slot to deactivate if active
//=================================================================================
void SMESHGUI_NodesDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled( false );
    GroupCoordinates->setEnabled( false );
    GroupButtons->setEnabled( false );
    mySimulation->SetVisibility( false );
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox( 0 );
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ActivateThisDialog()
{
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled( true );
  GroupCoordinates->setEnabled( true );
  GroupButtons->setEnabled( true );

  SMESH::SetPointRepresentation( true );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ) )
    aViewWindow->SetSelectionMode( NodeSelection );
  SelectionIntoArgument();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::keyPressEvent( QKeyEvent* e )
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
bool SMESHGUI_NodesDlg::isValid()
{
  QString msg;
  bool ok = true;
  ok = SpinBox_X->isValid( msg, true ) && ok;
  ok = SpinBox_Y->isValid( msg, true ) && ok;
  ok = SpinBox_Z->isValid( msg, true ) && ok;

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }

  if( GroupGroups->isChecked() && ComboBox_GroupName->currentText().isEmpty() ) {
    SUIT_MessageBox::warning( this, tr( "SMESH_WRN_WARNING" ), tr( "GROUP_NAME_IS_EMPTY" ) );
    return false;
  }
  return true;
}
