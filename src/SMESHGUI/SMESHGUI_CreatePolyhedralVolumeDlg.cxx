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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_CreatePolyhedralVolumeDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#include "SMESHGUI_CreatePolyhedralVolumeDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESH_ActorUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMDS_Mesh.hxx"

#include "SUIT_ResourceMgr.h"

#include "SalomeApp_Application.h"
#include "SalomeApp_Study.h"
#include "SUIT_Desktop.h"
#include "SUIT_MessageBox.h"
#include "LightApp_SelectionMgr.h"
#include "utilities.h"

#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"

#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h> 
#include <qpixmap.h>
#include <qlistbox.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

namespace SMESH{

class TPolySimulation{
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
      myMapper->SetInput( myGrid );

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->PickableOff();
      myPreviewActor->VisibilityOff();
      myPreviewActor->SetMapper( myMapper );
      myPreviewActor->SetRepresentation( 3 );

      float anRGB[3];
      vtkProperty* aProp = vtkProperty::New();
      GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
      aProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
      myPreviewActor->SetProperty( aProp );
      aProp->Delete();

      vtkProperty* aBackProp = vtkProperty::New();
      GetColor( "SMESH", "backface_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 0, 255 ) );
      aBackProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
      myPreviewActor->SetBackfaceProperty( aBackProp );
      aBackProp->Delete();

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

      if (theReset) ResetGrid(theReset);
      
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
SMESHGUI_CreatePolyhedralVolumeDlg::SMESHGUI_CreatePolyhedralVolumeDlg( SMESHGUI* theModule, const char* name,
					                                bool modal, WFlags fl )
  : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH",tr("ICON_SELECT")));

  if ( !name )
    setName( "SMESHGUI_CreatePolyhedralVolumeDlg" );
  resize( 300, 185 ); 
  setCaption( tr( "SMESH_CREATE_POLYHEDRAL_VOLUME_TITLE" ) );
  setSizeGripEnabled( TRUE );
  SMESHGUI_CreatePolyhedralVolumeDlgLayout = new QGridLayout( this ); 
  SMESHGUI_CreatePolyhedralVolumeDlgLayout->setSpacing( 6 );
  SMESHGUI_CreatePolyhedralVolumeDlgLayout->setMargin( 11 );

  /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  GroupConstructors->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, GroupConstructors->sizePolicy().hasHeightForWidth() ) );
  GroupConstructors->setTitle( tr( "SMESH_ELEMENTS_TYPE" ) );
  GroupConstructors->setExclusive( TRUE );
  GroupConstructors->setColumnLayout(0, Qt::Vertical );
  GroupConstructors->layout()->setSpacing( 0 );
  GroupConstructors->layout()->setMargin( 0 );
  GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
  GroupConstructorsLayout->setAlignment( Qt::AlignTop );
  GroupConstructorsLayout->setSpacing( 6 );
  GroupConstructorsLayout->setMargin( 11 );
  RadioButton1= new QRadioButton( GroupConstructors, "RadioButton1" );
  RadioButton1->setText( tr( "MESH_NODE"  ) );
  GroupConstructorsLayout->addWidget( RadioButton1, 0, 0 );
  RadioButton2= new QRadioButton( GroupConstructors, "RadioButton2" );
  RadioButton2->setText( tr( "SMESH_FACE"  ) );
  GroupConstructorsLayout->addWidget( RadioButton2, 0, 2  );
  SMESHGUI_CreatePolyhedralVolumeDlgLayout->addWidget( GroupConstructors, 0, 0 );
  
  /***************************************************************/
  GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, GroupButtons->sizePolicy().hasHeightForWidth() ) );
  GroupButtons->setGeometry( QRect( 10, 10, 281, 48 ) ); 
  GroupButtons->setTitle( tr( ""  ) );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  buttonCancel = new QPushButton( GroupButtons, "cancel" );
  buttonCancel->setText( tr( "SMESH_BUT_CLOSE"  ) );
  buttonCancel->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
  buttonApply = new QPushButton( GroupButtons, "apply" );
  buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
  buttonApply->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupButtonsLayout->addItem( spacer_9, 0, 2 );
  buttonOk = new QPushButton( GroupButtons, "ok" );
  buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
  SMESHGUI_CreatePolyhedralVolumeDlgLayout->addWidget( GroupButtons, 2, 0 );

  /***************************************************************/
  GroupContent = new QGroupBox( this, "GroupContent" );
  GroupContent->setTitle( tr( "SMESH_CONTENT" ) );
  GroupContent->setColumnLayout(0, Qt::Vertical );
  GroupContent->layout()->setSpacing( 0 );
  GroupContent->layout()->setMargin( 0 );
  GroupContentLayout = new QGridLayout( GroupContent->layout() );
  GroupContentLayout->setAlignment( Qt::AlignTop );
  GroupContentLayout->setSpacing( 6 );
  GroupContentLayout->setMargin( 11 );

  QFrame* anIds = new QFrame(GroupContent, "anIds");
  QGridLayout* aLayout = new QGridLayout(anIds, 1, 3);
  aLayout->setSpacing(6);
  aLayout->setAutoAdd(false);
  
  TextLabelIds = new QLabel( anIds, "TextLabelIds" );
  TextLabelIds->setText( tr( "SMESH_ID_NODES" ) );
  TextLabelIds->setFixedWidth(74);
  aLayout->addWidget( TextLabelIds, 0, 0 );

  SelectElementsButton  = new QPushButton( anIds, "SelectElementsButton" );
  SelectElementsButton->setText( tr( "" ) );
  SelectElementsButton->setPixmap( image0 );
  SelectElementsButton->setToggleButton( FALSE );
  aLayout->addWidget( SelectElementsButton, 0, 1 );

  LineEditElements  = new QLineEdit( anIds, "LineEditElements" );
  LineEditElements->setValidator( new SMESHGUI_IdValidator( this, "validator" ));
  aLayout->addWidget( LineEditElements, 0, 2 );
  
  GroupContentLayout->addMultiCellWidget(anIds, 0, 0, 0, 1);

  myFacesByNodesLabel = new QLabel(GroupContent, "faces by nodes label");
  myFacesByNodesLabel->setText(tr("FACES_BY_NODES"));
  GroupContentLayout->addWidget( myFacesByNodesLabel, 1, 0 );

  myFacesByNodes = new QListBox(GroupContent, "faces by nodes list");
  myFacesByNodes->setSelectionMode(QListBox::Extended);
  myFacesByNodes->setMinimumHeight(150);
  GroupContentLayout->addMultiCellWidget( myFacesByNodes, 2, 4, 0, 0 );

  AddButton = new QPushButton(GroupContent, "add");
  AddButton->setText(tr("SMESH_BUT_ADD"));
  AddButton->setMaximumWidth(85);
  GroupContentLayout->addWidget( AddButton, 2, 1 );

  RemoveButton = new QPushButton(GroupContent, "remove");
  RemoveButton->setText(tr("SMESH_BUT_REMOVE"));
  RemoveButton->setMaximumWidth(85);
  GroupContentLayout->addWidget( RemoveButton, 3, 1 );

  Preview = new QCheckBox( GroupContent, "Preview" );
  Preview->setText( tr( "SMESH_POLYEDRE_PREVIEW"  ) );
  GroupContentLayout->addWidget( Preview , 5, 0 );

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();
  
  SMESHGUI_CreatePolyhedralVolumeDlgLayout->addWidget( GroupContent, 1, 0 );
  
  GroupContent->show();
  RadioButton1->setChecked( TRUE );
 
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;
  
  Init();
}

//=================================================================================
// function : ~SMESHGUI_CreatePolyhedralVolumeDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_CreatePolyhedralVolumeDlg::~SMESHGUI_CreatePolyhedralVolumeDlg()
{
  // no need to delete child widgets, Qt does it all for us
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
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  myNbOkElements = 0;
  myActor = 0;

  mySimulation = new SMESH::TPolySimulation( dynamic_cast<SalomeApp_Application*>( mySMESHGUI->application() ) );

  /* signals and slots connections */
  connect(buttonOk, SIGNAL( clicked() ),     SLOT( ClickOnOk() ) );
  connect(buttonCancel, SIGNAL( clicked() ), SLOT( ClickOnCancel() ) ) ;
  connect(buttonApply, SIGNAL( clicked() ),  SLOT(ClickOnApply() ) );

  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );
  connect(SelectElementsButton, SIGNAL( clicked() ), SLOT( SetEditCurrentArgument() ) ) ;
  connect(LineEditElements, SIGNAL( textChanged(const QString&) ), SLOT(onTextChange(const QString&)));

  connect(myFacesByNodes, SIGNAL(selectionChanged()), this, SLOT(onListSelectionChanged()));
  connect(AddButton, SIGNAL(clicked()), this, SLOT(onAdd()));
  connect(RemoveButton, SIGNAL(clicked()), this, SLOT(onRemove()));
  
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelectionMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  connect( Preview, SIGNAL(toggled(bool)), this, SLOT(ClickOnPreview(bool)));
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */

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
  if ( myNbOkElements>0 && !mySMESHGUI->isActiveStudyLocked())
    {
      busy = true;
      if (GetConstructorId() == 0)
	{
	  SMESH::long_array_var anIdsOfNodes = new SMESH::long_array;
	  SMESH::long_array_var aQuantities  = new SMESH::long_array;

	  aQuantities->length( myFacesByNodes->count() );

	  TColStd_ListOfInteger aNodesIds;

	  QListBoxItem* anItem;
	  int aNbQuantities = 0;
	  for (anItem = myFacesByNodes->firstItem(); anItem != 0; anItem = anItem->next()) {
	    QStringList anIds = QStringList::split(" ", anItem->text());
	    int aNbNodesInFace = 0;
	    for (QStringList::iterator it = anIds.begin(); it != anIds.end(); ++it, ++aNbNodesInFace)
	      aNodesIds.Append( (*it).toInt() );

	    aQuantities[aNbQuantities++] = aNbNodesInFace;
	  }

	  anIdsOfNodes->length(aNodesIds.Extent());

	  int aNbIdsOfNodes = 0;
	  TColStd_ListIteratorOfListOfInteger It;
	  It.Initialize(aNodesIds);
	  for(;It.More();It.Next())
	    anIdsOfNodes[aNbIdsOfNodes++] = It.Value();
	    
	  try{
	    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
	    QApplication::setOverrideCursor(Qt::waitCursor);
	    aMeshEditor->AddPolyhedralVolume(anIdsOfNodes, aQuantities);
	    QApplication::restoreOverrideCursor();
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
	  
	  QStringList aListId = QStringList::split( " ", myEditCurrentArgument->text() );
	  anIdsOfFaces->length(aListId.count());
	  for ( int i = 0; i < aListId.count(); i++ )
	    anIdsOfFaces[i] = aListId[i].toInt();
	  
	  try{
	    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
	    QApplication::setOverrideCursor(Qt::waitCursor);
	    aMeshEditor->AddPolyhedralVolumeByFaces(anIdsOfFaces);
	    QApplication::restoreOverrideCursor();
	  }catch(SALOME::SALOME_Exception& exc){
	    INFOS("Follow exception was cought:\n\t"<<exc.details.text);
	  }catch(std::exception& exc){
	    INFOS("Follow exception was cought:\n\t"<<exc.what());
	  }catch(...){
	    INFOS("Unknown exception was cought !!!");
	  }
	}
      
      //SALOME_ListIO aList;
      //mySelectionMgr->setSelectedObjects( aList );
      SMESH::UpdateView();
      if( myActor ){
	unsigned int anEntityMode = myActor->GetEntityMode();
	myActor->SetEntityMode(SMESH_Actor::eVolumes | anEntityMode);
      }
      ConstructorsClicked( GetConstructorId() );
      busy = false;
    }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ClickOnOk()
{
  ClickOnApply() ;
  ClickOnCancel() ;
}

	
//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ClickOnCancel()
{
  mySelectionMgr->clearFilters();
  //SALOME_ListIO aList;
  //mySelectionMgr->setSelectedObjects( aList );
  SMESH::SetPointRepresentation(false);
  mySimulation->SetVisibility(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  disconnect( mySelectionMgr, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
}

//=======================================================================
//function : onTextChange
//purpose  : 
//=======================================================================

void SMESHGUI_CreatePolyhedralVolumeDlg::onTextChange(const QString& theNewText)
{
  if ( busy ) return;
  busy = true;

  mySimulation->SetVisibility(false);

  SMDS_Mesh* aMesh = 0;
  if ( myActor )
    aMesh = myActor->GetObject()->GetMesh();

  if (GetConstructorId() == 0)
  {
    if ( aMesh ) {
      TColStd_MapOfInteger newIndices;
      
      QStringList aListId = QStringList::split( " ", theNewText, false);
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
      
      aListId = QStringList::split( " ", theNewText, false);

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
  
  // get selected mesh
  
  SALOME_ListIO selected;
  mySelectionMgr->selectedObjects( selected );
  int nbSel = selected.Extent();
  if(nbSel != 1){
    return;
  }
  
  myMesh = SMESH::GetMeshByIO( selected.First() );
  if ( myMesh->_is_nil() )
    return;
  
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

//=======================================================================
//function : displaySimulation
//purpose  : 
//=======================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::displaySimulation()
{
  if ( (myNbOkElements || AddButton->isEnabled()) && GroupButtons->isEnabled() && myActor)
    {
      SMESH::TPolySimulation::TVTKIds aVTKIds;
      vtkIdType aType = VTK_CONVEX_POINT_SET ;
      if (GetConstructorId() == 0){
	if (!AddButton->isEnabled()){
	  QListBoxItem* anItem;
	  mySimulation->ResetGrid(true);
	  for (anItem = myFacesByNodes->firstItem(); anItem != 0; anItem = anItem->next()) {
	    QStringList anIds = QStringList::split(" ", anItem->text());
	    SMESH::TPolySimulation::TVTKIds aVTKIds_faces;
	    for (QStringList::iterator it = anIds.begin(); it != anIds.end(); ++it){
	      vtkIdType aId = myActor->GetObject()->GetNodeVTKId( (*it).toInt() ) ;
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
	  QStringList anEditIds = QStringList::split( " ", myEditCurrentArgument->text(), false);
	  for ( int i = 0; i < anEditIds.count(); i++ )
	    aVTKIds.push_back( myActor->GetObject()->GetNodeVTKId( anEditIds[ i ].toInt() ));
	  aType = VTK_POLYGON;
	  mySimulation->SetPosition(myActor, aType, aVTKIds);
	}
      }else if(GetConstructorId() == 1){
	SMDS_Mesh* aMesh = 0;
	if ( myActor ){
	  aMesh = myActor->GetObject()->GetMesh();
	}
	if ( aMesh ) {
	  QStringList aListId = QStringList::split( " ", myEditCurrentArgument->text(), false);
	  for ( int i = 0; i < aListId.count(); i++ )
	    {
	      const SMDS_MeshElement * anElem = aMesh->FindElement( aListId[ i ].toInt() );
	      if ( !anElem )
		return;
	      
	      SMDS_ElemIteratorPtr anIter = anElem->nodesIterator();
	      SMESH::TPolySimulation::TVTKIds aVTKIds_faces;
	      while( anIter->more() )
		if ( const SMDS_MeshNode* aNode = (SMDS_MeshNode*)anIter->next() ){
		  vtkIdType aId = myActor->GetObject()->GetNodeVTKId( aNode->GetID() ) ;
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
    LineEditElements->setFocus() ;
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
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled(false) ;
    GroupContent->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
    mySimulation->SetVisibility(false);
    mySMESHGUI->ResetState() ;    
    mySMESHGUI->SetActiveDialogBox(0) ;
  }
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupContent->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( FaceSelection );
  SelectionIntoArgument();
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::enterEvent(QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePolyhedralVolumeDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
}


//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_CreatePolyhedralVolumeDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    ClickOnCancel();
}


//=================================================================================
// function : GetConstructorId()
// purpose  : 
//=================================================================================
int SMESHGUI_CreatePolyhedralVolumeDlg::GetConstructorId()
{ 
  if ( GroupConstructors != NULL && GroupConstructors->selected() != NULL )
    return GroupConstructors->id( GroupConstructors->selected() );
  return -1;
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

  busy = true;

  if ( !(myEditCurrentArgument->text().isEmpty()) )
    {
      myFacesByNodes->insertItem(myEditCurrentArgument->text());
      //myFacesByNodes->setSelected(myFacesByNodes->count() - 1, true);
      myNbOkElements = 1;
      myEditCurrentArgument->clear();
      AddButton->setEnabled(false);
      buttonOk->setEnabled( true );
      if(myFacesByNodes->count()>1) buttonApply->setEnabled( true );
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
  for (int i = myFacesByNodes->count(); i > 0; i--) {
    if (myFacesByNodes->isSelected(i-1)) {
      myFacesByNodes->removeItem(i-1);
      myNbOkElements = 1;
    }
  }
  if (myFacesByNodes->count() < 1){
    RemoveButton->setEnabled(false);
    buttonOk->setEnabled( false );
    buttonApply->setEnabled( false );
  } 
  else if (myFacesByNodes->count() == 1){
    buttonOk->setEnabled( false );
    buttonApply->setEnabled( false );
  }
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
  bool isSelected=false;
  SALOME_ListIO aList;
  mySelectionMgr->setSelectedObjects( aList );
  TColStd_MapOfInteger aIndexes;
  QListBoxItem* anItem;
  for (anItem = myFacesByNodes->firstItem(); anItem != 0; anItem = anItem->next()) {
    if (anItem->isSelected()) {
      isSelected = true;
      QStringList anIds = QStringList::split(" ", anItem->text());
      for (QStringList::iterator it = anIds.begin(); it != anIds.end(); ++it)
	aIndexes.Add((*it).toInt());
    }
  }
  if(isSelected) RemoveButton->setEnabled(true);
  else RemoveButton->setEnabled(false);
  mySelector->AddOrRemoveIndex(myActor->getIO(), aIndexes, true );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->highlight( myActor->getIO(), true, true );
  mySelectionMgr->clearFilters(); 
  aList.Append( myActor->getIO() );
  mySelectionMgr->setSelectedObjects( aList );
  
  busy = false;
}
