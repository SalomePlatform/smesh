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
//  File   : SMESHGUI_AddMeshElementDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_AddMeshElementDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESH_ActorUtils.h"
 
#include "VTKViewer_ViewFrame.h"
#include "SMDS_Mesh.hxx"
#include "SMESH_Actor.h"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"

#include "utilities.h"

#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>

// QT Includes
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include <list>
#include <TColStd_IndexedMapOfInteger.hxx>

using namespace std;

namespace SMESH{

  class TElementSimulation{
    QAD_Study* myStudy;
    QAD_StudyFrame* myStudyFrame;
    VTKViewer_ViewFrame* myViewFrame;

    SALOME_Actor *myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkUnstructuredGrid* myGrid;

  public:

    TElementSimulation(QAD_Study* theStudy):
      myStudy(theStudy),
      myStudyFrame(theStudy->getActiveStudyFrame()),
      myViewFrame(GetVtkViewFrame(theStudy->getActiveStudyFrame()))
    {
      myGrid = vtkUnstructuredGrid::New();
  
      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInput( myGrid );

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->PickableOff();
      myPreviewActor->VisibilityOff();
      myPreviewActor->SetMapper( myMapper );

      vtkProperty* aProp = vtkProperty::New();
      float anRGB[3];
      anRGB[0] = GetFloat("SMESH:SettingsFillColorRed", 0)/255.;
      anRGB[1] = GetFloat("SMESH:SettingsFillColorGreen", 170)/255.;
      anRGB[2] = GetFloat("SMESH:SettingsFillColorBlue", 255)/255.;
      aProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
      myPreviewActor->SetProperty( aProp );
      aProp->Delete();

      vtkProperty* aBackProp = vtkProperty::New();
      anRGB[0] = GetFloat("SMESH:SettingsBackFaceColorRed", 0)/255.;
      anRGB[1] = GetFloat("SMESH:SettingsBackFaceColorGreen", 0)/255.;
      anRGB[2] = GetFloat("SMESH:SettingsBackFaceColorBlue", 255)/255.;
      aBackProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
      myPreviewActor->SetBackfaceProperty( aBackProp );
      aBackProp->Delete();

      myViewFrame->AddActor( myPreviewActor );

    }


    typedef list<vtkIdType> TVTKIds;
    void SetPosition(SMESH_Actor* theActor, 
		     vtkIdType theType, 
		     const TVTKIds& theIds)
    {
      vtkUnstructuredGrid *aGrid = theActor->GetUnstructuredGrid();
      myGrid->SetPoints(aGrid->GetPoints());

      myGrid->Reset();
      vtkIdList *anIds = vtkIdList::New();
      TVTKIds::const_iterator anIt = theIds.begin();
      for (int i = 0; anIt != theIds.end(); anIt++,i++)
	anIds->InsertId(i, *anIt);
      myGrid->InsertNextCell(theType,anIds);
      anIds->Delete();

      myGrid->Modified();

      SetVisibility(true);
    }


    void SetVisibility(bool theVisibility){
      myPreviewActor->SetVisibility(theVisibility);
      RepaintCurrentView();
    }


    ~TElementSimulation(){
      if(FindVtkViewFrame(myStudy,myStudyFrame)){
	myViewFrame->RemoveActor(myPreviewActor);
      }
      myPreviewActor->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myGrid->Delete();
    }

  };

}

//=================================================================================
// class    : SMESHGUI_AddMeshElementDlg()
// purpose  : 
//=================================================================================
SMESHGUI_AddMeshElementDlg::SMESHGUI_AddMeshElementDlg( QWidget* parent, const char* name,
                                                       SALOME_Selection* Sel, 
                                                       SMDSAbs_ElementType ElementType, int nbNodes,
                                                       bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
	       Qt::WDestructiveClose)
{
  mySimulation = new SMESH::TElementSimulation(SMESH::GetActiveStudy());

  // verify nb nodes and type
  myNbNodes = nbNodes;
  myElementType = ElementType;
  switch ( ElementType ) {
  case SMDSAbs_Face:
    if ( myNbNodes != 3 && myNbNodes != 4 )
      myNbNodes = 3;
    break;
  case SMDSAbs_Volume:
    if ( myNbNodes != 4 && myNbNodes != 8 ) //(nbNodes < 4 || nbNodes > 8 || nbNodes == 7)
      myNbNodes = 4;
    break;
  default:
    myElementType = SMDSAbs_Edge;
    myNbNodes = 2;
  }

  QString elemName;
  switch ( myNbNodes ) {
  case 2: elemName = "EDGE"; break;
  case 3: elemName = "TRIANGLE"; break;
  case 4: elemName =
    myElementType == SMDSAbs_Face ? elemName = "QUADRANGLE" : elemName = "TETRAS"; break;
//   case 5:
//   case 6:
  default: // 8
    elemName = "HEXAS";
  }
  QString iconName      = tr( QString("ICON_DLG_%1").arg(elemName) );
  QString buttonGrTitle = tr( QString("SMESH_%1").arg(elemName) );
  QString caption       = tr( QString("SMESH_ADD_%1_TITLE").arg(elemName) );
  QString grBoxTitle    = tr( QString("SMESH_ADD_%1").arg(elemName) );
    
  QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", iconName ));
  QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr("ICON_SELECT")));

  if ( !name )
    setName( "SMESHGUI_AddMeshElementDlg" );
  resize( 303, 185 ); 
  setCaption( caption );

  setSizeGripEnabled( TRUE );
  SMESHGUI_AddMeshElementDlgLayout = new QGridLayout( this ); 
  SMESHGUI_AddMeshElementDlgLayout->setSpacing( 6 );
  SMESHGUI_AddMeshElementDlgLayout->setMargin( 11 );

  /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  GroupConstructors->setTitle( buttonGrTitle );
  
  GroupConstructors->setExclusive( TRUE );
  GroupConstructors->setColumnLayout(0, Qt::Vertical );
  GroupConstructors->layout()->setSpacing( 0 );
  GroupConstructors->layout()->setMargin( 0 );
  GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
  GroupConstructorsLayout->setAlignment( Qt::AlignTop );
  GroupConstructorsLayout->setSpacing( 6 );
  GroupConstructorsLayout->setMargin( 11 );
  Constructor1 = new QRadioButton( GroupConstructors, "Constructor1" );
  Constructor1->setText( tr( ""  ) );
  Constructor1->setPixmap( image0 );
  Constructor1->setChecked( TRUE );
  Constructor1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth() ) );
  Constructor1->setMinimumSize( QSize( 50, 0 ) );
  GroupConstructorsLayout->addWidget( Constructor1, 0, 0 );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupConstructorsLayout->addItem( spacer, 0, 1 );
  SMESHGUI_AddMeshElementDlgLayout->addWidget( GroupConstructors, 0, 0 );
    
  /***************************************************************/
  GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setGeometry( QRect( 10, 10, 281, 48 ) ); 
  GroupButtons->setTitle( tr( ""  ) );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  buttonCancel = new QPushButton( GroupButtons, "buttonCancel" );
  buttonCancel->setText( tr( "SMESH_BUT_CLOSE"  ) );
  buttonCancel->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
  buttonApply = new QPushButton( GroupButtons, "buttonApply" );
  buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
  buttonApply->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupButtonsLayout->addItem( spacer_9, 0, 2 );
  buttonOk = new QPushButton( GroupButtons, "buttonOk" );
  buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
  SMESHGUI_AddMeshElementDlgLayout->addWidget( GroupButtons, 2, 0 );

  /***************************************************************/
  GroupC1 = new QGroupBox( this, "GroupC1" );
  GroupC1->setTitle( grBoxTitle );

  GroupC1->setMinimumSize( QSize( 0, 0 ) );
  GroupC1->setFrameShape( QGroupBox::Box );
  GroupC1->setFrameShadow( QGroupBox::Sunken );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  GroupC1Layout = new QGridLayout( GroupC1->layout() );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );
  TextLabelC1A1 = new QLabel( GroupC1, "TextLabelC1A1" );
  TextLabelC1A1->setText( tr( "SMESH_ID_NODES"  ) );
  TextLabelC1A1->setMinimumSize( QSize( 50, 0 ) );
  TextLabelC1A1->setFrameShape( QLabel::NoFrame );
  TextLabelC1A1->setFrameShadow( QLabel::Plain );
  GroupC1Layout->addWidget( TextLabelC1A1, 0, 0 );
  SelectButtonC1A1 = new QPushButton( GroupC1, "SelectButtonC1A1" );
  SelectButtonC1A1->setText( tr( ""  ) );
  SelectButtonC1A1->setPixmap( image1 );
  SelectButtonC1A1->setToggleButton( FALSE );
  GroupC1Layout->addWidget( SelectButtonC1A1, 0, 1 );
  LineEditC1A1 = new QLineEdit( GroupC1, "LineEditC1A1" );
//  LineEditC1A1->setReadOnly( TRUE );
  LineEditC1A1->setValidator( new SMESHGUI_IdValidator( this, "validator", myNbNodes ));
  GroupC1Layout->addWidget( LineEditC1A1, 0, 2 );

  if ( myElementType == SMDSAbs_Face ) {
    Reverse = new QCheckBox( GroupC1, "Reverse" );
    Reverse->setText( tr( "SMESH_REVERSE"  ) );
    GroupC1Layout->addWidget( Reverse, 1, 0 );
  }
  else
    Reverse = 0;

  SMESHGUI_AddMeshElementDlgLayout->addWidget( GroupC1, 1, 0 );

  Init(Sel) ; /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_AddMeshElementDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_AddMeshElementDlg::~SMESHGUI_AddMeshElementDlg()
{
    // no need to delete child widgets, Qt does it all for us
  delete mySimulation;
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::Init( SALOME_Selection* Sel )
{

  GroupC1->show();
  Constructor1->setChecked( TRUE );
  myEditCurrentArgument = LineEditC1A1 ;	
  mySelection = Sel;  
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  myOkNodes = false ;
  myActor = 0;

  /* signals and slots connections */
  connect(buttonOk, SIGNAL( clicked() ),     SLOT( ClickOnOk() ) );
  connect(buttonCancel, SIGNAL( clicked() ), SLOT( ClickOnCancel() ) ) ;
  connect(buttonApply, SIGNAL( clicked() ),  SLOT(ClickOnApply() ) );

  connect(SelectButtonC1A1, SIGNAL( clicked() ), SLOT( SetEditCurrentArgument() ) ) ;
  connect(LineEditC1A1, SIGNAL( textChanged(const QString&) ), SLOT(onTextChange(const QString&)));
  connect(mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), SLOT( DeactivateActiveDialog()));
  connect(mySelection, SIGNAL( currentSelectionChanged() ), SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study frame change */
  connect( mySMESHGUI, SIGNAL ( SignalStudyFrameChanged() ), SLOT( ClickOnCancel() ) ) ;

  if ( Reverse )
    connect( Reverse, SIGNAL(stateChanged(int)), SLOT(CheckBox(int)) );

  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */

  // set selection mode
  SMESH::SetPointRepresentation(true);
  QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );

  SelectionIntoArgument();

  myBusy = false;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ClickOnApply()
{
  if ( myOkNodes && !mySMESHGUI->ActiveStudyLocked() ) {

    myBusy = true;
    SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
    anArrayOfIdeces->length( myNbNodes );
    bool reverse = ( Reverse && Reverse->isChecked() );
    QStringList aListId = QStringList::split( " ", myEditCurrentArgument->text(), false);
    for ( int i = 0; i < aListId.count(); i++ )
      if ( reverse )
        anArrayOfIdeces[i] = aListId[ myNbNodes - i - 1 ].toInt();
      else
        anArrayOfIdeces[i] = aListId[ i ].toInt();

    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
    switch ( myElementType ) {
    case SMDSAbs_Edge:
      aMeshEditor->AddEdge(anArrayOfIdeces.inout()); break;
    case SMDSAbs_Face:
      aMeshEditor->AddFace(anArrayOfIdeces.inout()); break;
    case SMDSAbs_Volume:
      aMeshEditor->AddVolume(anArrayOfIdeces.inout()); break;
    default:;
    }
    
    mySelection->ClearIObjects();
    mySelection->AddIObject( myActor->getIO(), false );

    SMESH::UpdateView();
    mySimulation->SetVisibility(false);

    buttonOk->setEnabled( false );
    buttonApply->setEnabled( false );

    myEditCurrentArgument->setText("");

    myBusy = false;
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;
  return ;
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ClickOnCancel()
{
  mySelection->ClearIObjects();
  mySimulation->SetVisibility(false);
  SMESH::SetPointRepresentation(false);
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
  return ;
}

//=======================================================================
//function : onTextChange
//purpose  : 
//=======================================================================

void SMESHGUI_AddMeshElementDlg::onTextChange(const QString& theNewText)
{
  if ( myBusy ) return;
  myBusy = true;

  myOkNodes = false;

  buttonOk->setEnabled( false );
  buttonApply->setEnabled( false );

  mySimulation->SetVisibility(false);

  // hilight entered nodes
  SMDS_Mesh* aMesh = 0;
  if ( myActor )
    aMesh = myActor->GetObject()->GetMesh();
  if ( aMesh ) {

    mySelection->ClearIObjects();
    mySelection->AddIObject( myActor->getIO() );

    QStringList aListId = QStringList::split( " ", theNewText, false);
    bool allOk = true;
    for ( int i = 0; i < aListId.count(); i++ ) {
      const SMDS_MeshNode * n = aMesh->FindNode( aListId[ i ].toInt() );
      if ( n ) {
        if ( mySelection->IsIndexSelected( myActor->getIO(), n->GetID() ))
          allOk = false;
        else
          mySelection->AddOrRemoveIndex (myActor->getIO(), n->GetID(), true);
      }
      else
        allOk = false;
    }
    myOkNodes = (allOk && myNbNodes == aListId.count() );

    if ( myOkNodes ) {
      buttonOk->setEnabled( true );
      buttonApply->setEnabled( true );
      displaySimulation();
    }
  }

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_AddMeshElementDlg::SelectionIntoArgument()
{
  if ( myBusy ) return;

  // clear

  myOkNodes = false;
  myActor = 0;

  myBusy = true;
  myEditCurrentArgument->setText( "" );
  myBusy = false;

  if ( !GroupButtons->isEnabled() ) // inactive
    return;

  buttonOk->setEnabled( false );
  buttonApply->setEnabled( false );

  mySimulation->SetVisibility(false);
//  SMESH::SetPointRepresentation(true);

  // get selected mesh

  int nbSel = mySelection->IObjectCount();
  if(nbSel != 1)
    return;

  myMesh = SMESH::GetMeshByIO( mySelection->firstIObject() );
  if ( myMesh->_is_nil() )
    return;

  myActor = SMESH::FindActorByEntry( mySelection->firstIObject()->getEntry() );
  if ( !myActor )
    return;

  // get selected nodes

  QString aString = "";
  int nbNodes = SMESH::GetNameOfSelectedNodes(mySelection, aString) ;
  myBusy = true;
  myEditCurrentArgument->setText( aString );
  myBusy = false;
  if ( myNbNodes != nbNodes )
    return;

  // OK

  myOkNodes = true;

  buttonOk->setEnabled( true );
  buttonApply->setEnabled( true );

  displaySimulation();
}

//=======================================================================
//function : displaySimulation
//purpose  : 
//=======================================================================

void SMESHGUI_AddMeshElementDlg::displaySimulation()
{
  if ( myOkNodes && GroupButtons->isEnabled() )
  {
    SMESH::TElementSimulation::TVTKIds anIds;
    QStringList aListId = QStringList::split( " ", myEditCurrentArgument->text(), false);
    for ( int i = 0; i < aListId.count(); i++ )
      anIds.push_back( myActor->GetObject()->GetNodeVTKId( aListId[ i ].toInt() ));

    if ( Reverse && Reverse->isChecked() )
      anIds.reverse();

    vtkIdType aType = 0;
    switch ( myNbNodes ) {
    case 2: aType = VTK_LINE; break;
    case 3: aType = VTK_TRIANGLE; break;
    case 4: aType = myElementType == SMDSAbs_Face ? VTK_QUAD : VTK_TETRA; break;
    case 8: aType = VTK_HEXAHEDRON; break;
    default: return;
    }
      
    mySimulation->SetPosition(myActor,aType,anIds);
  }
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  if(send == SelectButtonC1A1) {
    LineEditC1A1->setFocus() ;
    myEditCurrentArgument = LineEditC1A1;
  }
  SelectionIntoArgument() ;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {  
    GroupConstructors->setEnabled(false) ;
    GroupC1->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
    mySimulation->SetVisibility(false);
    mySMESHGUI->ResetState() ;    
    mySMESHGUI->SetActiveDialogBox(0) ;
  }
  return ;
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   

  GroupConstructors->setEnabled(true) ;
  GroupC1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;

  SMESH::SetPointRepresentation(true);
  QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );

  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::enterEvent(QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
  return ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
  return ;
}
//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_AddMeshElementDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    ClickOnCancel() ;
}

//=======================================================================
//function : CheckBox
//purpose  : 
//=======================================================================

void SMESHGUI_AddMeshElementDlg::CheckBox( int state )
{
  if ( !myOkNodes )
    return;
  
  if ( state >= 0 ) {
    mySimulation->SetVisibility(false);
    displaySimulation();
  }
}
