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
//  File   : SMESHGUI_ExtrusionAlongPathDlg.cxx
//  Author : Vadim SANDLER
//  Module : SMESH
//  $Header: 

#include "SMESHGUI_ExtrusionAlongPathDlg.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESH_Actor.h"
#include "SMESH_NumberFilter.hxx"
#include "SMDS_Mesh.hxx"
#include "GEOM_ShapeTypeFilter.hxx"
#include "GEOMBase.h"

#include "QAD_Application.h"
#include "QAD_WaitCursor.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"
#include "utilities.h"

#include <BRep_Tool.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qvalidator.h>
#include <qptrlist.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::SMESHGUI_ExtrusionAlongPathDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_ExtrusionAlongPathDlg::SMESHGUI_ExtrusionAlongPathDlg( QWidget* parent, SALOME_Selection* Sel, bool modal )
     : QDialog( parent, "SMESHGUI_ExtrusionAlongPathDlg", modal, 
	        WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose )
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::SMESHGUI_ExtrusionAlongPathDlg");
  myType = -1;
  QPixmap edgeImage  ( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_DLG_EDGE" ) ) );
  QPixmap faceImage  ( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_DLG_TRIANGLE" ) ) );
  QPixmap selectImage( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_SELECT" ) ) );
  QPixmap addImage   ( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_APPEND" ) ) );
  QPixmap removeImage( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_REMOVE" ) ) );

  setCaption( tr( "EXTRUSION_ALONG_PATH" ) );
  setSizeGripEnabled( TRUE );

  QGridLayout* topLayout = new QGridLayout( this ); 
  topLayout->setSpacing( 6 ); topLayout->setMargin( 11 );

  /***************************************************************/
  // Elements type group box (1d / 2d elements)
  ElementsTypeGrp = new QButtonGroup( tr( "SMESH_EXTRUSION" ), this );
  ElementsTypeGrp->setColumnLayout( 0, Qt::Vertical );
  ElementsTypeGrp->layout()->setSpacing( 0 );  ElementsTypeGrp->layout()->setMargin( 0 );
  QGridLayout* ElementsTypeGrpLayout = new QGridLayout( ElementsTypeGrp->layout() );
  ElementsTypeGrpLayout->setAlignment( Qt::AlignTop );
  ElementsTypeGrpLayout->setSpacing( 6 ); ElementsTypeGrpLayout->setMargin( 11 );

  Elements1dRB = new QRadioButton( ElementsTypeGrp );
  Elements1dRB->setPixmap( edgeImage );
  Elements2dRB = new QRadioButton( ElementsTypeGrp );
  Elements2dRB->setPixmap( faceImage );
  Elements1dRB->setChecked( true );

  // layouting
  ElementsTypeGrpLayout->addWidget( Elements1dRB, 0, 0 );
  ElementsTypeGrpLayout->addWidget( Elements2dRB, 0, 2 );
  
  /***************************************************************/
  // Arguments group box
  ArgumentsGrp = new QGroupBox( tr( "EXTRUSION_1D" ), this );
  ArgumentsGrp->setColumnLayout( 0, Qt::Vertical );
  ArgumentsGrp->layout()->setSpacing( 0 ); ArgumentsGrp->layout()->setMargin( 0 );
  QGridLayout* ArgumentsGrpLayout = new QGridLayout( ArgumentsGrp->layout() );
  ArgumentsGrpLayout->setAlignment( Qt::AlignTop );
  ArgumentsGrpLayout->setSpacing( 6 ); ArgumentsGrpLayout->setMargin( 11 );

  // Controls for elements selection
  ElementsLab = new QLabel( tr( "SMESH_ID_ELEMENTS" ), ArgumentsGrp );

  SelectElementsButton = new QToolButton( ArgumentsGrp );
  SelectElementsButton->setPixmap( selectImage );

  ElementsLineEdit = new QLineEdit( ArgumentsGrp );
  ElementsLineEdit->setValidator( new SMESHGUI_IdValidator( this ) );

  // Controls for the whole mesh selection
  MeshCheck = new QCheckBox( tr( "SMESH_SELECT_WHOLE_MESH" ), ArgumentsGrp );

  // Controls for path selection
  PathGrp = new QGroupBox( tr( "SMESH_PATH" ), ArgumentsGrp );
  PathGrp->setColumnLayout( 0, Qt::Vertical );
  PathGrp->layout()->setSpacing( 0 ); PathGrp->layout()->setMargin( 0 );
  QGridLayout* PathGrpLayout = new QGridLayout( PathGrp->layout() );
  PathGrpLayout->setAlignment( Qt::AlignTop );
  PathGrpLayout->setSpacing( 6 ); PathGrpLayout->setMargin( 11 );
  
  // Controls for path mesh selection
  PathMeshLab = new QLabel( tr( "SMESH_PATH_MESH" ), PathGrp );

  SelectPathMeshButton = new QToolButton( PathGrp );
  SelectPathMeshButton->setPixmap( selectImage );

  PathMeshLineEdit = new QLineEdit( PathGrp );
  PathMeshLineEdit->setReadOnly( true );

  // Controls for path shape selection
  PathShapeLab = new QLabel( tr( "SMESH_PATH_SHAPE" ), PathGrp );

  SelectPathShapeButton = new QToolButton( PathGrp );
  SelectPathShapeButton->setPixmap( selectImage );

  PathShapeLineEdit = new QLineEdit( PathGrp );
  PathShapeLineEdit->setReadOnly( true );

  // Controls for path starting point selection
  StartPointLab = new QLabel( tr( "SMESH_PATH_START" ), PathGrp );

  SelectStartPointButton = new QToolButton( PathGrp );
  SelectStartPointButton->setPixmap( selectImage );

  StartPointLineEdit = new QLineEdit( PathGrp );
  StartPointLineEdit->setValidator( new QIntValidator( this ) );

  // layouting
  PathGrpLayout->addWidget( PathMeshLab,            0, 0 );
  PathGrpLayout->addWidget( SelectPathMeshButton,   0, 1 );
  PathGrpLayout->addWidget( PathMeshLineEdit,       0, 2 );
  PathGrpLayout->addWidget( PathShapeLab,           1, 0 );
  PathGrpLayout->addWidget( SelectPathShapeButton,  1, 1 );
  PathGrpLayout->addWidget( PathShapeLineEdit,      1, 2 );
  PathGrpLayout->addWidget( StartPointLab,          2, 0 );
  PathGrpLayout->addWidget( SelectStartPointButton, 2, 1 );
  PathGrpLayout->addWidget( StartPointLineEdit,     2, 2 );

  // Controls for angles defining
  AnglesCheck = new QCheckBox( tr( "SMESH_USE_ANGLES" ), ArgumentsGrp );
  
  AnglesGrp = new QGroupBox( tr( "SMESH_ANGLES" ), ArgumentsGrp );
  AnglesGrp->setColumnLayout( 0, Qt::Vertical );
  AnglesGrp->layout()->setSpacing( 0 ); AnglesGrp->layout()->setMargin( 0 );
  QGridLayout* AnglesGrpLayout = new QGridLayout( AnglesGrp->layout() );
  AnglesGrpLayout->setAlignment( Qt::AlignTop );
  AnglesGrpLayout->setSpacing( 6 ); AnglesGrpLayout->setMargin( 11 );
  
  AnglesList = new QListBox( AnglesGrp );
  AnglesList->setSelectionMode( QListBox::Extended );

  AddAngleButton = new QToolButton( AnglesGrp ); 
  AddAngleButton->setPixmap( addImage );

  RemoveAngleButton = new QToolButton( AnglesGrp ); 
  RemoveAngleButton->setPixmap( removeImage );

  AngleSpin = new SMESHGUI_SpinBox( AnglesGrp );

  // layouting
  QVBoxLayout* bLayout = new QVBoxLayout(); 
  bLayout->addWidget( AddAngleButton );
  bLayout->addSpacing( 6 );
  bLayout->addWidget( RemoveAngleButton );
  bLayout->addStretch();
  AnglesGrpLayout->addMultiCellWidget( AnglesList, 0, 1, 0, 0 );
  AnglesGrpLayout->addMultiCellLayout( bLayout,    0, 1, 1, 1 );
  AnglesGrpLayout->addWidget(          AngleSpin,  0,    2    );
  AnglesGrpLayout->setRowStretch( 1, 10 );
  
  // Controls for base point defining
  BasePointCheck = new QCheckBox( tr( "SMESH_USE_BASE_POINT" ), ArgumentsGrp );

  BasePointGrp = new QGroupBox( tr( "SMESH_BASE_POINT" ), ArgumentsGrp );
  BasePointGrp->setColumnLayout( 0, Qt::Vertical );
  BasePointGrp->layout()->setSpacing( 0 ); BasePointGrp->layout()->setMargin( 0 );
  QGridLayout* BasePointGrpLayout = new QGridLayout( BasePointGrp->layout() );
  BasePointGrpLayout->setAlignment( Qt::AlignTop );
  BasePointGrpLayout->setSpacing( 6 ); BasePointGrpLayout->setMargin( 11 );
  
  SelectBasePointButton = new QToolButton( BasePointGrp );
  SelectBasePointButton->setPixmap( selectImage );

  XLab  = new QLabel( tr( "SMESH_X" ), BasePointGrp );
  XSpin = new SMESHGUI_SpinBox( BasePointGrp );
  YLab  = new QLabel( tr( "SMESH_Y" ), BasePointGrp );
  YSpin = new SMESHGUI_SpinBox( BasePointGrp );
  ZLab  = new QLabel( tr( "SMESH_Z" ), BasePointGrp );
  ZSpin = new SMESHGUI_SpinBox( BasePointGrp );

  // layouting
  BasePointGrpLayout->addWidget( SelectBasePointButton, 0, 0 );
  BasePointGrpLayout->addWidget( XLab,                  0, 1 );
  BasePointGrpLayout->addWidget( XSpin,                 0, 2 );
  BasePointGrpLayout->addWidget( YLab,                  0, 3 );
  BasePointGrpLayout->addWidget( YSpin,                 0, 4 );
  BasePointGrpLayout->addWidget( ZLab,                  0, 5 );
  BasePointGrpLayout->addWidget( ZSpin,                 0, 6 );

  // layouting
  ArgumentsGrpLayout->addWidget(          ElementsLab,            0,    0    );
  ArgumentsGrpLayout->addWidget(          SelectElementsButton,   0,    1    );
  ArgumentsGrpLayout->addWidget(          ElementsLineEdit,       0,    2    );
  ArgumentsGrpLayout->addMultiCellWidget( MeshCheck,              1, 1, 0, 2 );
  ArgumentsGrpLayout->addMultiCellWidget( PathGrp,                2, 2, 0, 2 );
  ArgumentsGrpLayout->addWidget(          AnglesCheck,            3,    0    );
  ArgumentsGrpLayout->addMultiCellWidget( AnglesGrp,              3, 4, 1, 2 );
  ArgumentsGrpLayout->addWidget(          BasePointCheck,         5,    0    );
  ArgumentsGrpLayout->addMultiCellWidget( BasePointGrp,           5, 6, 1, 2 );
  ArgumentsGrpLayout->setRowStretch( 4, 10 );

  /***************************************************************/
  // common buttons group box
  ButtonsGrp = new QGroupBox( this );
  ButtonsGrp->setColumnLayout( 0, Qt::Vertical );
  ButtonsGrp->layout()->setSpacing( 0 ); ButtonsGrp->layout()->setMargin( 0 );
  QGridLayout* ButtonsGrpLayout = new QGridLayout( ButtonsGrp->layout() );
  ButtonsGrpLayout->setAlignment( Qt::AlignTop );
  ButtonsGrpLayout->setSpacing( 6 ); ButtonsGrpLayout->setMargin( 11 );

  OkButton = new QPushButton( tr( "SMESH_BUT_OK" ), ButtonsGrp );
  OkButton->setAutoDefault( true );
  OkButton->setDefault( true );

  ApplyButton = new QPushButton( tr( "SMESH_BUT_APPLY" ), ButtonsGrp );
  ApplyButton->setAutoDefault( true );

  CloseButton = new QPushButton( tr( "SMESH_BUT_CLOSE" ), ButtonsGrp );
  CloseButton->setAutoDefault( true );

  // layouting
  ButtonsGrpLayout->addWidget( OkButton,    0, 0 );
  ButtonsGrpLayout->addWidget( ApplyButton, 0, 1 );
  ButtonsGrpLayout->addWidget( CloseButton, 0, 3 );
  ButtonsGrpLayout->addColSpacing( 2, 10 );
  ButtonsGrpLayout->setColStretch( 2, 10 );

  /***************************************************************/
  // layouting
  topLayout->addWidget( ElementsTypeGrp, 0, 0 );
  topLayout->addWidget( ArgumentsGrp,    1, 0 );
  topLayout->addWidget( ButtonsGrp,      2, 0 );

  /***************************************************************/
  // Initialisations
  XSpin->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  YSpin->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  ZSpin->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  AngleSpin->RangeStepAndValidator( -999999.999, +999999.999, 5.0, 3 );
 
  mySelection = Sel;  
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( this ) ;
  
  // Costruction of the logical filter for the elements: mesh/sub-mesh/group
  SMESH_ListOfFilter aListOfFilters;
  Handle(SMESH_TypeFilter) aMeshOrSubMeshFilter = new SMESH_TypeFilter( MESHorSUBMESH );
  if ( !aMeshOrSubMeshFilter.IsNull() )
    aListOfFilters.Append( aMeshOrSubMeshFilter );
  Handle(SMESH_TypeFilter) aSmeshGroupFilter = new SMESH_TypeFilter( GROUP );
  if ( !aSmeshGroupFilter.IsNull() )
    aListOfFilters.Append( aSmeshGroupFilter );
  
  myElementsFilter = new SMESH_LogicalFilter( aListOfFilters, SMESH_LogicalFilter::LO_OR );
  
  myPathMeshFilter = new SMESH_TypeFilter( MESH );
  
  Init();

  /***************************************************************/
  // signals-slots connections
  connect( OkButton,     SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( CloseButton,  SIGNAL( clicked() ), this, SLOT( reject() ) ) ;
  connect( ApplyButton,  SIGNAL( clicked() ), this, SLOT( ClickOnApply() ) );

  connect( AddAngleButton,    SIGNAL( clicked() ), this, SLOT( OnAngleAdded() ) );
  connect( RemoveAngleButton, SIGNAL( clicked() ), this, SLOT( OnAngleRemoved() ) );

  connect( ElementsTypeGrp, SIGNAL( clicked( int ) ), SLOT( TypeChanged( int ) ) );
  
  connect( SelectElementsButton,   SIGNAL( clicked() ), this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectPathMeshButton,   SIGNAL( clicked() ), this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectPathShapeButton,  SIGNAL( clicked() ), this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectStartPointButton, SIGNAL( clicked() ), this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectBasePointButton,  SIGNAL( clicked() ), this, SLOT( SetEditCurrentArgument() ) ) ;

  connect( mySMESHGUI,  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ),      this, SLOT( SelectionIntoArgument() ) );
  connect( mySMESHGUI,  SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( reject() ) ) ;

  connect( ElementsLineEdit, SIGNAL( textChanged( const QString& ) ),
	   SLOT( onTextChange( const QString& ) ) );
  connect( StartPointLineEdit, SIGNAL( textChanged( const QString& ) ),
	   SLOT( onTextChange( const QString& ) ) );

  connect( MeshCheck,      SIGNAL( toggled( bool ) ), SLOT( onSelectMesh() ) );
  connect( AnglesCheck,    SIGNAL( toggled( bool ) ), SLOT( onAnglesCheck() ) );
  connect( BasePointCheck, SIGNAL( toggled( bool ) ), SLOT( onBasePointCheck() ) );

  AnglesList->installEventFilter( this );
  ElementsLineEdit->installEventFilter( this );
  StartPointLineEdit->installEventFilter( this );
  XSpin->editor()->installEventFilter( this );
  YSpin->editor()->installEventFilter( this );
  ZSpin->editor()->installEventFilter( this );

  /***************************************************************/
  // set position and show dialog box
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; // displays Dialog
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::~SMESHGUI_ExtrusionAlongPathDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionAlongPathDlg::~SMESHGUI_ExtrusionAlongPathDlg()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::~SMESHGUI_ExtrusionAlongPathDlg");
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::eventFilter
// purpose  : event filter
//=================================================================================
bool SMESHGUI_ExtrusionAlongPathDlg::eventFilter( QObject* object, QEvent* event )
{
  if ( event->type() == QEvent::KeyPress ) {
    QKeyEvent* ke = ( QKeyEvent* )event;
    if ( object == AnglesList ) {
      if ( ke->key() == Key_Delete )
	OnAngleRemoved();
    }
  }
  else if ( event->type() == QEvent::FocusIn ) {
    if ( object == ElementsLineEdit ) {
      if ( myEditCurrentArgument != ElementsLineEdit )
	SetEditCurrentArgument( SelectElementsButton );
    }
    else if ( object == StartPointLineEdit ) {
      if ( myEditCurrentArgument != StartPointLineEdit )
	SetEditCurrentArgument( SelectStartPointButton );
    }
    else if ( object == XSpin->editor() || object == YSpin->editor() || object == ZSpin->editor() ) {
      if ( myEditCurrentArgument != XSpin )
	SetEditCurrentArgument( SelectBasePointButton );
    }
  }
  return QDialog::eventFilter( object, event );
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::Init()
// purpose  : initialization
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::Init( bool ResetControls )
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::Init");
  myBusy = false;
  myEditCurrentArgument = 0;
 
  myMesh      = SMESH::SMESH_Mesh::_nil();
  myIDSource  = SMESH::SMESH_IDSource::_nil();
  myMeshActor = 0;
  myPathMesh  = SMESH::SMESH_Mesh::_nil();
  myPathShape = GEOM::GEOM_Object::_nil();
  
  ElementsLineEdit->clear();
  PathMeshLineEdit->clear();
  PathShapeLineEdit->clear();
  StartPointLineEdit->clear();

  if( ResetControls ) {
    XSpin->SetValue( 0.0 );
    YSpin->SetValue( 0.0 );
    ZSpin->SetValue( 0.0 );
    
    AngleSpin->SetValue( 45 );
    MeshCheck->setChecked( false );
    TypeChanged( 0 );
    onSelectMesh();
    onAnglesCheck();
    onBasePointCheck();
  }
  SetEditCurrentArgument( 0 );
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::TypeChanged()
// purpose  : Called when user changes type of elements ( 1d / 2d )
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::TypeChanged( int type )
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::TypeChanged");
  if ( myType != type ) {
    disconnect( mySelection, 0, this, 0 );
    
    if ( type == 0 )
      ArgumentsGrp->setTitle( tr( "EXTRUSION_1D" ) );
    else if ( type == 1 )
      ArgumentsGrp->setTitle( tr( "EXTRUSION_2D" ) );
    
    // clear elements ID list
    if ( !MeshCheck->isChecked() ) {
      ElementsLineEdit->clear();
    }
    // set selection mode if necessary
    if( myEditCurrentArgument == ElementsLineEdit ) {
      mySelection->ClearIObjects();
      mySelection->ClearFilters();
      SMESH::SetPickable();
  
      SMESH::SetPointRepresentation( false );
      if ( MeshCheck->isChecked() ) {
	QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
	mySelection->AddFilter( myElementsFilter );
      }
      else  {
	if ( type == 0 )
	  QAD_Application::getDesktop()->SetSelectionMode( EdgeSelection, true );
	if ( type == 1 )
	  QAD_Application::getDesktop()->SetSelectionMode( FaceSelection, true );
      }
    }
    connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  }
  myType = type;
} 


//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::ClickOnApply()
// purpose  : Called when user presses <Apply> button
//=================================================================================
bool SMESHGUI_ExtrusionAlongPathDlg::ClickOnApply()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::ClickOnApply");
  if ( mySMESHGUI->ActiveStudyLocked() ) {
    return false;
  }
  
  if( myMesh->_is_nil() || MeshCheck->isChecked() && myIDSource->_is_nil() || !myMeshActor || myPathMesh->_is_nil() || myPathShape->_is_nil() ) {
    return false;
  }

  SMESH::long_array_var anElementsId = new SMESH::long_array;

  if ( MeshCheck->isChecked() ) {
    // If "Select whole mesh, submesh or group" check box is on ->
    // get all elements of the required type from the object selected

    // if MESH object is selected
    if ( !CORBA::is_nil( SMESH::SMESH_Mesh::_narrow( myIDSource ) ) ) {
      // get mesh
      SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( myIDSource );
      // get IDs from mesh...
      if ( Elements1dRB->isChecked() )
	// 1d elements
	anElementsId = aMesh->GetElementsByType( SMESH::EDGE );
      else if ( Elements2dRB->isChecked() ) {
	anElementsId = aMesh->GetElementsByType( SMESH::FACE );
      }
    }
    // SUBMESH is selected
    if ( !CORBA::is_nil( SMESH::SMESH_subMesh::_narrow( myIDSource ) ) ) {
      // get submesh
      SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( myIDSource );
      // get IDs from submesh
      if ( Elements1dRB->isChecked() )
	// 1d elements
	anElementsId = aSubMesh->GetElementsByType( SMESH::EDGE );
      else if ( Elements2dRB->isChecked() )
	// 2d elements
	anElementsId = aSubMesh->GetElementsByType( SMESH::FACE );
    }  
    // GROUP is selected
    if ( !CORBA::is_nil( SMESH::SMESH_GroupBase::_narrow( myIDSource ) ) ) {
      // get smesh group
      SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( myIDSource );
      // get IDs from group
      // 1d elements or 2d elements
      if ( Elements1dRB->isChecked() && aGroup->GetType() == SMESH::EDGE ||
	   Elements2dRB->isChecked() && aGroup->GetType() == SMESH::FACE )
	anElementsId = aGroup->GetListOfID();
    }
  }
  else {
    // If "Select whole mesh, submesh or group" check box is off ->
    // use only elements of given type selected by user

    SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh();
    if ( aMesh ) {
      QStringList aListElementsId = QStringList::split( " ", ElementsLineEdit->text(), false );
      anElementsId = new SMESH::long_array;
      anElementsId->length( aListElementsId.count() );
      bool bOk;
      int j = 0;
      for ( int i = 0; i < aListElementsId.count(); i++ ) {
	long ind = aListElementsId[ i ].toLong( &bOk );
	if  ( bOk ) {
	  const SMDS_MeshElement* e = aMesh->FindElement( ind );
	  if ( e ) {
	    bool typeMatch = Elements1dRB->isChecked() && e->GetType() == SMDSAbs_Edge || 
	                     Elements2dRB->isChecked() && e->GetType() == SMDSAbs_Face;
	    if ( typeMatch )
	      anElementsId[ j++ ] = ind;
	  }
	}
      }
      anElementsId->length( j );
    }
  }
      
  if ( anElementsId->length() <= 0 ) {
    return false;
  }

  if ( StartPointLineEdit->text().stripWhiteSpace().isEmpty() ) {
    return false;
  }
  
  bool bOk;
  long aNodeStart = StartPointLineEdit->text().toLong( &bOk );
  if ( !bOk ) {
    return false;
  }
  
  // get angles
  SMESH::double_array_var anAngles = new SMESH::double_array;
  if ( AnglesCheck->isChecked() ) {
    anAngles->length( AnglesList->count() );
    int j = 0;
    bool bOk;
    for ( int i = 0; i < AnglesList->count(); i++ ) {
      long angle = AnglesList->text( i ).toLong( &bOk );
      if  ( bOk )
	anAngles[ j++ ] = angle;
    }
    anAngles->length( j );
  }

  // get base point
  SMESH::PointStruct aBasePoint;
  if ( BasePointCheck->isChecked() ) {
    aBasePoint.x = XSpin->GetValue();
    aBasePoint.y = YSpin->GetValue();
    aBasePoint.z = ZSpin->GetValue();
  }
  
  try {
    QAD_WaitCursor wc;
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
    aMeshEditor->ExtrusionAlongPath( anElementsId.inout(), myPathMesh, myPathShape, aNodeStart, 
				     AnglesCheck->isChecked(), anAngles.inout(), 
				     BasePointCheck->isChecked(), aBasePoint );
  }
  catch( ... ) {
    return false;
  }
    
  mySelection->ClearIObjects();
  SMESH::UpdateView();
  Init( false );
  TypeChanged( GetConstructorId() );
  return true;
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::ClickOnOk()
// purpose  : Called when user presses <OK> button
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::ClickOnOk()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::ClickOnOk");
  if ( !ClickOnApply() )
    return;
  reject();
}

//=======================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::onTextChange
// purpose  : 
//=======================================================================

void SMESHGUI_ExtrusionAlongPathDlg::onTextChange(const QString& theNewText)
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::onTextChange");
  QLineEdit* send = (QLineEdit*)sender();
  if ( send != StartPointLineEdit && send != ElementsLineEdit )
    send = ElementsLineEdit;

  // return if busy
  if ( myBusy ) 
    return;

  // set busy flag
  SetBusy sb( this );
  
  if ( send == ElementsLineEdit && myEditCurrentArgument == ElementsLineEdit ) {
    // hilight entered elements
    SMDS_Mesh* aMesh = 0;
    if ( myMeshActor )
      aMesh = myMeshActor->GetObject()->GetMesh();
    if ( aMesh ) {
      mySelection->ClearIObjects();
      mySelection->AddIObject( myMeshActor->getIO() );
      
      QStringList aListId = QStringList::split( " ", theNewText, false );
      bool bOk;
      for ( int i = 0; i < aListId.count(); i++ ) {
	long ind = aListId[ i ].toLong( &bOk );
	if ( bOk ) {
	  const SMDS_MeshElement* e = aMesh->FindElement( ind );
	  if ( e ) {
	    // check also type of element
	    bool typeMatch = Elements1dRB->isChecked() && e->GetType() == SMDSAbs_Edge || 
	                     Elements2dRB->isChecked() && e->GetType() == SMDSAbs_Face;
	    if ( typeMatch ) {
	      if ( !mySelection->IsIndexSelected( myMeshActor->getIO(), e->GetID() ) )
		mySelection->AddOrRemoveIndex( myMeshActor->getIO(), e->GetID(), true );
	    }
	  }
	}
      }
    }
  }
  else if ( send == StartPointLineEdit && myEditCurrentArgument == StartPointLineEdit ) {
    if ( !myPathMesh->_is_nil() ) {
      SMESH_Actor* aPathActor = SMESH::FindActorByObject( myPathMesh );
      SMDS_Mesh* aMesh = 0;
      if ( aPathActor )
	aMesh = aPathActor->GetObject()->GetMesh();
      if ( aMesh ) {
	mySelection->ClearIObjects();
	mySelection->AddIObject( aPathActor->getIO() );
      
	bool bOk;
	long ind = theNewText.toLong( &bOk );
	if ( bOk ) {
	  const SMDS_MeshNode* n = aMesh->FindNode( ind );
	  if ( n ) {
	    if ( !mySelection->IsIndexSelected( aPathActor->getIO(), n->GetID() ) ) {
	      mySelection->AddOrRemoveIndex( aPathActor->getIO(), n->GetID(), true );
	    }
	  }
	}
      }
    }
  }
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::SelectionIntoArgument()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::SelectionIntoArgument");
  // return if busy
  if ( myBusy ) 
    return;
  
  // return if dialog box is inactive
  if ( !ButtonsGrp->isEnabled() )
    return;

  // selected objects count
  int nbSel = mySelection->IObjectCount();

  // set busy flag
  SetBusy sb( this );

  if ( myEditCurrentArgument == ElementsLineEdit ) {
    // we are now selecting mesh elements (or whole mesh/submesh/group)
    // reset
    ElementsLineEdit->clear();
    myMesh      = SMESH::SMESH_Mesh::_nil();
    myIDSource  = SMESH::SMESH_IDSource::_nil();
    myMeshActor = 0;
    
    // only one object is acceptable
    if( nbSel != 1 )
      return;

    // try to get mesh from selection
    Handle( SALOME_InteractiveObject ) IO = mySelection->firstIObject();
    myMesh = SMESH::GetMeshByIO( IO );
    if( myMesh->_is_nil() )
      return;
    // find actor
    myMeshActor = SMESH::FindActorByObject( myMesh );
    if ( !myMeshActor )
      return;

    if ( MeshCheck->isChecked() ) {
      // If "Select whole mesh, submesh or group" check box is on ->
      // get ID source and put it's name to the edit box
      QString aString;
      SMESH::GetNameOfSelectedIObjects( mySelection, aString );

      myIDSource = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
      ElementsLineEdit->setText( aString );
    }
    else {
      // If "Select whole mesh, submesh or group" check box is off ->
      // try to get selected elements IDs
      QString aString;
      int aNbUnits = SMESH::GetNameOfSelectedElements( mySelection, aString );
      ElementsLineEdit->setText( aString );
    }
  }
  else if ( myEditCurrentArgument == PathMeshLineEdit ) {
    // we are now selecting path mesh
    // reset
    PathMeshLineEdit->clear();
    myPathMesh = SMESH::SMESH_Mesh::_nil();
    PathShapeLineEdit->clear();
    myPathShape = GEOM::GEOM_Object::_nil();
    StartPointLineEdit->clear();

    // only one object is acceptable
    if( nbSel != 1 )
      return;

    // try to get mesh from selection
    Handle( SALOME_InteractiveObject ) IO = mySelection->firstIObject();
    myPathMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>( IO );
    if( myPathMesh->_is_nil() )
      return;

    QString aString;
    SMESH::GetNameOfSelectedIObjects( mySelection, aString );
    PathMeshLineEdit->setText( aString );
  }
  else if ( myEditCurrentArgument == PathShapeLineEdit ) {
    // we are now selecting path mesh
    // reset
    PathShapeLineEdit->clear();
    myPathShape = GEOM::GEOM_Object::_nil();
    StartPointLineEdit->clear();

    // return if path mesh is not yet selected
    if ( myPathMesh->_is_nil() )
      return;

    // only one object is acceptable
    if( nbSel != 1 )
      return;

    // try to get shape from selection
    Handle( SALOME_InteractiveObject ) IO = mySelection->firstIObject();
    myPathShape = SMESH::IObjectToInterface<GEOM::GEOM_Object>( IO );
    if ( myPathShape->_is_nil() )
      return;

    QString aString;
    SMESH::GetNameOfSelectedIObjects( mySelection, aString );
    PathShapeLineEdit->setText( aString );
  }
  else if ( myEditCurrentArgument == StartPointLineEdit ) {
    // we are now selecting start point of path
    // reset
    StartPointLineEdit->clear();

    // return if path mesh or path shape is not yet selected
    if ( myPathMesh->_is_nil() || myPathShape->_is_nil() )
      return;

    // only one object is acceptable
    if( nbSel != 1 )
      return;

    // try to get shape from selection
    Handle( SALOME_InteractiveObject ) IO = mySelection->firstIObject();
    
    QString aString;
    int aNbUnits = SMESH::GetNameOfSelectedElements( mySelection, aString );
    if ( aNbUnits == 1 )
      StartPointLineEdit->setText( aString.stripWhiteSpace() );
  }
  else if ( myEditCurrentArgument == XSpin ) {
    // we are now selecting base point
    // reset is not performed here!

    // return if is not enabled
    if ( !BasePointGrp->isEnabled() ) 
      return;

    // only one object is acceptable
    if( nbSel != 1 )
      return;
    
    // try to get shape from selection
    Handle( SALOME_InteractiveObject ) IO = mySelection->firstIObject();
    
    // check if geom vertex is selected
    GEOM::GEOM_Object_var aGeomObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>( IO );
    TopoDS_Vertex aVertex;
    if( !aGeomObj->_is_nil() ) {
      if(  aGeomObj->IsShape() && GEOMBase::GetShape( aGeomObj, aVertex ) && !aVertex.IsNull() ) {
	gp_Pnt aPnt = BRep_Tool::Pnt( aVertex );
	XSpin->SetValue( aPnt.X() );
	YSpin->SetValue( aPnt.Y() );
	ZSpin->SetValue( aPnt.Z() );
      }
      return;
    }

    // check if smesh node is selected
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( IO );
    if( aMesh->_is_nil() )
      return;

    QString aString;
    int aNbUnits = SMESH::GetNameOfSelectedNodes( mySelection, aString );
    // return if more than one node is selected
    if( aNbUnits != 1 )
      return;
      
    SMESH_Actor* aMeshActor = SMESH::FindActorByObject( aMesh );
    if ( !aMeshActor )
      return;

    SMDS_Mesh* mesh = aMeshActor->GetObject()->GetMesh();
    if (!mesh)
      return;

    const SMDS_MeshNode* n = mesh->FindNode( aString.toLong() );
    if ( !n )
      return;

    XSpin->SetValue( n->X() );
    YSpin->SetValue( n->Y() );
    ZSpin->SetValue( n->Z() );
  }
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument");
  QToolButton* send = (QToolButton*)sender();
  if ( send != SelectElementsButton   && 
       send != SelectPathMeshButton   && 
       send != SelectPathShapeButton  && 
       send != SelectStartPointButton && 
       send != SelectBasePointButton )
    return;
  SetEditCurrentArgument( send );
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument( QToolButton* button )
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument(button)");
  disconnect( mySelection, 0, this, 0 );
  mySelection->ClearIObjects();
  mySelection->ClearFilters();
  SMESH::SetPickable();
  
  if( button == SelectElementsButton ) {
    myEditCurrentArgument = ElementsLineEdit;
    SMESH::SetPointRepresentation( false );
    if ( MeshCheck->isChecked() ) {
      QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
      mySelection->AddFilter( myElementsFilter );
    }
    else  {
      if ( Elements1dRB->isChecked() )
	QAD_Application::getDesktop()->SetSelectionMode( EdgeSelection, true );
      else if ( Elements2dRB->isChecked() )
	QAD_Application::getDesktop()->SetSelectionMode( FaceSelection, true );
    }
  }
  else if ( button == SelectPathMeshButton ) {
    myEditCurrentArgument = PathMeshLineEdit;
    SMESH::SetPointRepresentation( false );
    QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
    mySelection->AddFilter( myPathMeshFilter );
  }
  else if ( button == SelectPathShapeButton ) {
    myEditCurrentArgument = PathShapeLineEdit;
    SMESH::SetPointRepresentation( false );
    QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );

    if ( !myPathMesh->_is_nil() ) {
      TColStd_MapOfInteger aTypes;
      aTypes.Add( TopAbs_EDGE );
      aTypes.Add( TopAbs_WIRE );
      GEOM::GEOM_Object_var aMainShape = myPathMesh->GetShapeToMesh();
      SMESH_Actor* aPathActor = SMESH::FindActorByObject( myPathMesh );

      if ( !aMainShape->_is_nil() && aPathActor )
	mySelection->AddFilter( new SMESH_NumberFilter( "GEOM", TopAbs_SHAPE, -1, aTypes, aMainShape ) );
	//SMESH::SetPickable( aPathActor );
    }
  }
  else if ( button == SelectStartPointButton ) {
    myEditCurrentArgument = StartPointLineEdit;
    if ( !myPathMesh->_is_nil() ) {
      SMESH_Actor* aPathActor = SMESH::FindActorByObject( myPathMesh );
      if ( aPathActor ) {
	SMESH::SetPointRepresentation( true );
	QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );
	SMESH::SetPickable( aPathActor );
      }
    }
  }
  else if ( button == SelectBasePointButton ) {
    myEditCurrentArgument = XSpin;
    SMESH::SetPointRepresentation( true );
    QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );

    SMESH_ListOfFilter aListOfFilters;
    Handle(SMESH_TypeFilter) aMeshOrSubMeshFilter = new SMESH_TypeFilter( MESHorSUBMESH );
    if ( !aMeshOrSubMeshFilter.IsNull() )
      aListOfFilters.Append( aMeshOrSubMeshFilter );
    Handle(SMESH_TypeFilter) aSmeshGroupFilter = new SMESH_TypeFilter( GROUP );
    if ( !aSmeshGroupFilter.IsNull() )
      aListOfFilters.Append( aSmeshGroupFilter );
    //Handle(GEOM_ShapeTypeFilter) aVertexFilter = new GEOM_ShapeTypeFilter( TopAbs_VERTEX );
    Handle(SMESH_NumberFilter) aVertexFilter = new SMESH_NumberFilter( "GEOM", TopAbs_SHAPE, -1, TopAbs_VERTEX, GEOM::GEOM_Object::_nil() );
    if ( !aVertexFilter.IsNull() )
      aListOfFilters.Append( aVertexFilter );
  
    mySelection->AddFilter( new SMESH_LogicalFilter( aListOfFilters, SMESH_LogicalFilter::LO_OR ) );
  }

  if ( myEditCurrentArgument && !myEditCurrentArgument->hasFocus() )
    myEditCurrentArgument->setFocus();

  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  SelectionIntoArgument();
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::DeactivateActiveDialog()
// purpose  : Deactivates this dialog
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::DeactivateActiveDialog()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::DeactivateActiveDialog");
  if ( ElementsTypeGrp->isEnabled() ) {
    ElementsTypeGrp->setEnabled( false );
    ArgumentsGrp->setEnabled( false );
    ButtonsGrp->setEnabled( false );
    mySMESHGUI->ResetState();    
    mySMESHGUI->SetActiveDialogBox( 0 );
  }
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::ActivateThisDialog()
// purpose  : Activates this dialog
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::ActivateThisDialog()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::ActivateThisDialog");
  // Emit a signal to deactivate the active dialog
  mySMESHGUI->EmitSignalDeactivateDialog();   
  ElementsTypeGrp->setEnabled( true );
  ArgumentsGrp->setEnabled( true );
  ButtonsGrp->setEnabled( true );
  
  mySMESHGUI->SetActiveDialogBox( this );

  TypeChanged( GetConstructorId() );
  SelectionIntoArgument();
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::enterEvent()
// purpose  : Mouse enter event
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::enterEvent(QEvent* e)
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::enterEvent");
  if ( ElementsTypeGrp->isEnabled() )
    return;  
  ActivateThisDialog();
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::reject()
// purpose  : Called when dialog box is closed
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::reject()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::reject");
  disconnect( mySelection, 0, this, 0 );
  mySelection->ClearFilters();
  mySelection->ClearIObjects();
  SMESH::SetPickable();
  SMESH::SetPointRepresentation( false );
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  mySMESHGUI->ResetState() ;
  QDialog::reject();
}

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::GetConstructorId()
// purpose  : 
//=================================================================================
int SMESHGUI_ExtrusionAlongPathDlg::GetConstructorId()
{ 
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::GetConstructorId");
  if ( ElementsTypeGrp != NULL && ElementsTypeGrp->selected() != NULL )
    return ElementsTypeGrp->id( ElementsTypeGrp->selected() );
  return -1;
}

//=======================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::onSelectMesh
// purpose  :
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onSelectMesh()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::onSelectMesh");
  bool toSelectMesh = MeshCheck->isChecked();
  
  ElementsLineEdit->setReadOnly( toSelectMesh );
  ElementsLab->setText( toSelectMesh ? tr( "SMESH_NAME" ) : tr( "SMESH_ID_ELEMENTS" ) );
  ElementsLineEdit->clear();
  
  SetEditCurrentArgument( SelectElementsButton );
}

//=======================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::onAnglesCheck
// purpose  : called when "Use Angles" check box is switched
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onAnglesCheck()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::onAnglesCheck");
  AnglesGrp->setEnabled( AnglesCheck->isChecked() );
}

//=======================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::onBasePointCheck
// purpose  : called when "Use Base Point" check box is switched
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onBasePointCheck()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::onBasePointCheck");
  BasePointGrp->setEnabled( BasePointCheck->isChecked() );
}

//=======================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::OnAngleAdded
// purpose  : Called when user adds angle to the list
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::OnAngleAdded()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::OnAngleAdded");
  AnglesList->insertItem( QString::number( AngleSpin->GetValue() ) );
}

//=======================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg::OnAngleRemoved
// purpose  : Called when user removes angle(s) from the list
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::OnAngleRemoved()
{
  MESSAGE("SMESHGUI_ExtrusionAlongPathDlg::OnAngleRemoved");
  QList<QListBoxItem> aList;
  aList.setAutoDelete( false );
  for ( int i = 0; i < AnglesList->count(); i++ )
    if ( AnglesList->isSelected( i ) )
      aList.append( AnglesList->item( i ) );

  for ( int i = 0; i < aList.count(); i++ )
    delete aList.at( i );
}

