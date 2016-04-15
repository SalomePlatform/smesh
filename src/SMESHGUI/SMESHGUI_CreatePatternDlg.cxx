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

// File   : SMESHGUI_CreatePatternDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_CreatePatternDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_PatternWidget.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_PatternUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include <SMESH_NumberFilter.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_FileDlg.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Tools.h>

#include <SALOME_ListIO.hxx>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QApplication>
#include <QKeyEvent>
#include <QFile>
#include <QDir>

#define SPACING 6
#define MARGIN  11

/*!
 *  Class       : SMESHGUI_CreatePatternDlg
 *  Description : Dialog to specify filters for VTK viewer
 */

//=======================================================================
// function : SMESHGUI_CreatePatternDlg()
// purpose  : Constructor
//=======================================================================
SMESHGUI_CreatePatternDlg::SMESHGUI_CreatePatternDlg( SMESHGUI*   theModule,
                                                      const int   theType )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  setModal( false );
  setWindowTitle( tr( "CAPTION" ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( this );
  aDlgLay->setMargin( MARGIN );
  aDlgLay->setSpacing( SPACING );

  QWidget* aMainFrame = createMainFrame( this );
  QWidget* aBtnFrame  = createButtonFrame( this );

  aDlgLay->addWidget( aMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( aMainFrame, 1 );

  myHelpFileName = "pattern_mapping_page.html";

  Init( theType );
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_CreatePatternDlg::createMainFrame( QWidget* theParent )
{
  QPixmap iconSlct    ( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", tr( "ICON_SELECT" ) ) );
  QPixmap icon2d      ( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", tr( "ICON_PATTERN_2d" ) ) );
  QPixmap icon3d      ( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", tr( "ICON_PATTERN_3d" ) ) );
  QPixmap iconSample2d( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", tr( "ICON_PATTERN_SAMPLE_2D" ) ) );

  QWidget* aMainGrp = new QWidget( theParent );
  QVBoxLayout* aMainGrpLayout = new QVBoxLayout( aMainGrp );
  aMainGrpLayout->setMargin( 0 );
  aMainGrpLayout->setSpacing( SPACING );

  // Pattern type group

  QGroupBox* aTypeGrp = new QGroupBox( tr( "PATTERN_TYPE" ), aMainGrp );
  QHBoxLayout* aTypeGrpLayout = new QHBoxLayout( aTypeGrp );
  aTypeGrpLayout->setMargin( MARGIN );
  aTypeGrpLayout->setSpacing( SPACING );

  mySwitch2d = new QRadioButton( aTypeGrp );
  mySwitch3d = new QRadioButton( aTypeGrp );
  mySwitch2d->setIcon( icon2d );
  mySwitch3d->setIcon( icon3d );

  myTypeGrp = new QButtonGroup( aMainGrp );
  myTypeGrp->addButton( mySwitch2d, Type_2d );
  myTypeGrp->addButton( mySwitch3d, Type_3d );

  // ... layout widgets

  aTypeGrpLayout->addWidget( mySwitch2d );
  aTypeGrpLayout->addWidget( mySwitch3d );

  // Mesh and pattern name group

  QGroupBox* aPatternGrp = new QGroupBox( tr( "PATTERN" ), aMainGrp );
  QGridLayout* aPatternGrpLayout = new QGridLayout( aPatternGrp );
  aPatternGrpLayout->setMargin( MARGIN );
  aPatternGrpLayout->setSpacing( SPACING );

  QLabel* aMeshLab = new QLabel( tr( "MESH_OR_SUBMESH" ), aPatternGrp );

  QPushButton* aSelBtn = new QPushButton( aPatternGrp );
  aSelBtn->setIcon( iconSlct );
  myMeshEdit = new QLineEdit( aPatternGrp );
  myMeshEdit->setReadOnly( true );

  QLabel* aNameLab = new QLabel( tr( "PATTERN_NAME" ), aPatternGrp );
  myName = new QLineEdit( aPatternGrp );

  // Picture 2d

  myPicture2d = new SMESHGUI_PatternWidget( aPatternGrp ),
  myPicture2d->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

  // Project check box

  myProjectChk = new QCheckBox( tr( "PROJECT" ), aPatternGrp );

  // ... layout widgets

  aPatternGrpLayout->addWidget( aMeshLab,     0, 0 );
  aPatternGrpLayout->addWidget( aSelBtn,      0, 1 );
  aPatternGrpLayout->addWidget( myMeshEdit,   0, 2 );
  aPatternGrpLayout->addWidget( aNameLab,     1, 0 );
  aPatternGrpLayout->addWidget( myName,       1, 2 );
  aPatternGrpLayout->addWidget( myPicture2d,  2, 0, 1, 3 );
  aPatternGrpLayout->addWidget( myProjectChk, 3, 0, 1, 3 );

  // main layout

  aMainGrpLayout->addWidget( aTypeGrp );
  aMainGrpLayout->addWidget( aPatternGrp );

  // Connect signals and slots

  connect( myTypeGrp,    SIGNAL( buttonClicked( int )  ),
           this,         SLOT( onTypeChanged( int ) ) );
  connect( myProjectChk, SIGNAL( toggled( bool ) ),
           this,         SLOT( onProject( bool ) ) );
  connect( aSelBtn,      SIGNAL( clicked() ),
           this,         SLOT( onSelBtnClicked() ) );

  return aMainGrp;
}

//=======================================================================
// function : createButtonFrame()
// purpose  : Create frame containing buttons
//=======================================================================
QWidget* SMESHGUI_CreatePatternDlg::createButtonFrame( QWidget* theParent )
{
  QFrame* aFrame = new QFrame( theParent );
  aFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

  QPushButton* myOkBtn    = new QPushButton( tr( "SMESH_BUT_OK"    ), aFrame );
  QPushButton* mySaveBtn  = new QPushButton( tr( "SAVE"            ), aFrame );
  QPushButton* myCloseBtn = new QPushButton( tr( "SMESH_BUT_CANCEL"), aFrame );
  QPushButton* myHelpBtn  = new QPushButton( tr( "SMESH_BUT_HELP"),   aFrame );

  QHBoxLayout* aLay = new QHBoxLayout( aFrame );
  aLay->setMargin( MARGIN );
  aLay->setSpacing( SPACING );

  aLay->addWidget( myOkBtn );
  aLay->addSpacing( 10 );
  aLay->addWidget( mySaveBtn );
  aLay->addSpacing( 10 );
  aLay->addStretch();
  aLay->addWidget( myCloseBtn );
  aLay->addWidget( myHelpBtn );

  connect( myOkBtn,    SIGNAL( clicked() ), this, SLOT( onOk() ) );
  connect( myCloseBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( mySaveBtn,  SIGNAL( clicked() ), this, SLOT( onSave() ) );
  connect( myHelpBtn,  SIGNAL( clicked() ), this, SLOT( onHelp() ) );

  return aFrame;
}

//=======================================================================
// function : ~SMESHGUI_CreatePatternDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_CreatePatternDlg::~SMESHGUI_CreatePatternDlg()
{
}

//=======================================================================
// function : onProject()
// purpose  : SLOT. Called when state of "Project nodes on ther face"
//            checkbox is changed
//=======================================================================
void SMESHGUI_CreatePatternDlg::onProject( bool )
{
  loadFromObject( false );
  displayPreview();
}

//=======================================================================
// function : Init()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::Init( const int theType )
{
  myIsLoaded     = false;
  myType         = -1;
  mySubMesh      = SMESH::SMESH_subMesh::_nil();
  myMesh         = SMESH::SMESH_Mesh::_nil();
  myGeomObj      = GEOM::GEOM_Object::_nil();
  myPattern      = SMESH::SMESH_Pattern::_nil();

  erasePreview();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // selection and SMESHGUI
  connect( mySelectionMgr, SIGNAL( currentSelectionChanged() ),
           this,           SLOT( onSelectionDone() ) );
  connect( mySMESHGUI,     SIGNAL( SignalDeactivateActiveDialog() ),
           this,           SLOT( onDeactivate() ) );
  connect( mySMESHGUI,     SIGNAL( SignalCloseAllDialogs() ),
           this,           SLOT( reject() ) );

  mySwitch2d->setEnabled( theType == Type_2d );
  mySwitch3d->setEnabled( theType == Type_3d );

  myTypeGrp->button( theType )->setChecked( true );
  onTypeChanged( theType );

  myName->setText( getDefaultName() );
  myMeshEdit->setText( "" );

  QApplication::instance()->processEvents();
  updateGeometry();
  resize(100,100);

  activateSelection();
  onSelectionDone();

  //show();
}

//=======================================================================
// function : SetMesh()
// purpose  : Set mesh to dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::SetMesh( SMESH::SMESH_Mesh_ptr thePtr )
{
  myMesh = SMESH::SMESH_Mesh::_duplicate( thePtr );
  mySubMesh = SMESH::SMESH_subMesh::_nil();

  bool isValidMesh = false;
  if ( !myMesh->_is_nil() ) {
    _PTR(SObject) aSobj = SMESH::FindSObject( myMesh.in() );
    //Handle(SALOME_InteractiveObject) anIObj =
    //  new SALOME_InteractiveObject(aSobj->GetID().c_str(), "SMESH");
    SUIT_DataOwnerPtr anIObj( new LightApp_DataOwner( aSobj->GetID().c_str() ) );

    isValidMesh = mySelectionMgr->isOk( anIObj );
  }

  if ( isValidMesh ) {
    _PTR(SObject) aSO = SMESH::FindSObject( myMesh.in() );
    myMeshEdit->setText( aSO->GetName().c_str() );
    myGeomObj = SMESH::GetGeom( aSO );
  } 
  else {
    myMeshEdit->setText( "" );
    myGeomObj = GEOM::GEOM_Object::_nil();
  }

  if ( myType == Type_2d ) {
    loadFromObject( false );
    displayPreview();
  }
}

//=======================================================================
// function : isValid()
// purpose  : Verify validity of entry data
//=======================================================================
bool SMESHGUI_CreatePatternDlg::isValid()
{
  if ( myGeomObj->_is_nil() ) {
    SUIT_MessageBox::information( this,
                                  tr( "SMESH_INSUFFICIENT_DATA" ),
                                  tr( "SMESHGUI_INVALID_PARAMETERS" ) );
    return false;
  }
  return true;
}

//=======================================================================
// function : getDefaultName()
// purpose  : Get default pattern name
//=======================================================================
QString SMESHGUI_CreatePatternDlg::getDefaultName() const
{
  return myType == Type_2d ? tr( "DEFAULT_2D" ) : tr( "DEFAULT_3D" );
}

//=======================================================================
// function : onSave()
// purpose  : SLOT called when "Save" button pressed. Build pattern and
//           save it to disk
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSave()
{
  try {
    if ( !isValid() )
      return;

    if ( !myIsLoaded )
      loadFromObject( true );

    // Load pattern from object
    if ( !myIsLoaded )
      return;

    ///////////////////////////////////////////////////////
    SUIT_FileDlg* aDlg = new SUIT_FileDlg( this, false );
    aDlg->setWindowTitle( tr( "SAVE_PATTERN" ) );
    aDlg->setFileMode( QFileDialog::AnyFile );
    aDlg->setNameFilter( tr( "PATTERN_FILT" ) );
    if ( myName->text() != "" )
      aDlg->selectFile( myName->text() );

    if ( aDlg->exec() != Accepted )
      return;

    QString fName = aDlg->selectedFile();
    if ( fName.isEmpty() )
      return;

    if ( QFileInfo( fName ).suffix().isEmpty() )
      fName = autoExtension( fName );

    fName = QDir::toNativeSeparators( fName );

    QString aData( myPattern->GetString() );
    long aLen = aData.length();

    QFile aFile( fName );
    aFile.open( QIODevice::WriteOnly );
    long aWritten = aFile.write( aData.toLatin1(), aLen );
    aFile.close();

    if ( aWritten != aLen ) {
      SUIT_MessageBox::information( this,
                                    tr( "SMESH_ERROR" ),
                                    tr( "ERROR_OF_SAVING" ) );
    } 
    else {
      //SUIT_Application::getDesktop()->setSelectionModes(ActorSelection);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ) )
        aViewWindow->SetSelectionMode( ActorSelection );
      disconnect( mySelectionMgr, 0, this, 0 );
      disconnect( mySMESHGUI, 0, this, 0 );
      mySMESHGUI->ResetState();
      accept();
      emit NewPattern();
    }
  } 
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  } 
  catch (...) {
  }
}

//=======================================================================
// function : GetPatternName()
// purpose  : Get name of pattern
//=======================================================================
QString SMESHGUI_CreatePatternDlg::GetPatternName() const
{
  return myName->text();
}

//=======================================================================
// function : GetPattern()
// purpose  : Get result pattern
//=======================================================================
SMESH::SMESH_Pattern_ptr SMESHGUI_CreatePatternDlg::GetPattern()
{
  return myPattern.in();
}

//=======================================================================
// function : onOk()
// purpose  : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_CreatePatternDlg::onOk()
{
  try {
    if ( !isValid() )
      return;

    if ( !myIsLoaded )
      loadFromObject( true );

    // Load pattern from object
    if ( !myIsLoaded ) {
      return;
    }
    else {
      //SUIT_Application::getDesktop()->setSelectionModes(ActorSelection);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ) )
        aViewWindow->SetSelectionMode( ActorSelection );
      disconnect( mySelectionMgr, 0, this, 0 );
      disconnect( mySMESHGUI, 0, this, 0 );
      mySMESHGUI->ResetState();
      accept();
      emit NewPattern();
    }
  } 
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  } 
  catch (...) {
  }
}

//=======================================================================
// function : reject()
// purpose  : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::reject()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ) )
    aViewWindow->SetSelectionMode( ActorSelection );
  disconnect( mySelectionMgr, 0, this, 0 );
  disconnect( mySMESHGUI, 0, this, 0 );
  mySMESHGUI->ResetState();
  QDialog::reject();
  emit Close();
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePatternDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)( SUIT_Session::session()->activeApplication() );
  if ( app ) 
    app->onHelpContextModule( mySMESHGUI ? app->moduleName( mySMESHGUI->moduleName() ) : QString( "" ), myHelpFileName );
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning( this, 
                              tr( "WRN_WARNING" ),
                              tr( "EXTERNAL_BROWSER_CANNOT_SHOW_PAGE" ).
                              arg( app->resourceMgr()->stringValue( "ExternalBrowser", 
                                                                    platform ) ).
                              arg( myHelpFileName ) );
  }
}

//=======================================================================
// function : loadFromObject()
// purpose  : Load pattern from geom object corresponding to the mesh/submesh
//=======================================================================
bool SMESHGUI_CreatePatternDlg::loadFromObject( const bool theMess )
{
  try {
    myIsLoaded = false;

    if ( myPattern->_is_nil() )
      myPattern = SMESH::GetPattern();

    if ( (myMesh->_is_nil() && mySubMesh->_is_nil()) || myGeomObj->_is_nil() )
      return false;

    SMESH::SMESH_Mesh_ptr aMesh = mySubMesh->_is_nil() ? myMesh.in() : mySubMesh->GetFather();

    myIsLoaded = myType == Type_2d
      ? myPattern->LoadFromFace( aMesh, myGeomObj, myProjectChk->isChecked() )
      : myPattern->LoadFrom3DBlock( aMesh, myGeomObj );

    if ( !myIsLoaded && theMess ) {
      QString aMess;
      SMESH::SMESH_Pattern::ErrorCode aCode = myPattern->GetErrorCode();

      if      ( aCode == SMESH::SMESH_Pattern::ERR_LOAD_EMPTY_SUBMESH )   aMess = tr( "ERR_LOAD_EMPTY_SUBMESH" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADF_NARROW_FACE )    aMess = tr( "ERR_LOADF_NARROW_FACE" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADF_CLOSED_FACE )    aMess = tr( "ERR_LOADF_CLOSED_FACE" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADF_CANT_PROJECT )   aMess = tr( "ERR_LOADF_CANT_PROJECT" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADV_BAD_SHAPE )      aMess = tr( "ERR_LOADV_BAD_SHAPE" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADV_COMPUTE_PARAMS ) aMess = tr( "ERR_LOADV_COMPUTE_PARAMS" );
      else                                                                aMess = tr( "ERROR_OF_CREATION" );

      SUIT_MessageBox::information( this, tr( "SMESH_ERROR" ), aMess );
    }
  } 
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }

  return myIsLoaded;
}

//=======================================================================
// function : onSelectionDone()
// purpose  : SLOT called when selection changed
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSelectionDone()
{
  try {
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects( aList );
    if ( aList.Extent() != 1 )
      return;

    // Get mesh or sub-mesh from selection
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>( anIO );
    SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>( anIO );
    if ( aMesh->_is_nil() && aSubMesh->_is_nil() )
      return;

    // Get geom object corresponding to the mesh
    _PTR(SObject) aSO;
    if ( !aMesh->_is_nil() )
      aSO = SMESH::FindSObject( aMesh.in() );
    else
      aSO = SMESH::FindSObject( aSubMesh.in() );

    GEOM::GEOM_Object_var aGeomObj = SMESH::GetGeom( aSO );
    if ( aGeomObj->_is_nil() )
      return;

    myGeomObj = aGeomObj;

    // init class fields
    if ( !aMesh->_is_nil() ) {
      myMesh = aMesh;
      mySubMesh = SMESH::SMESH_subMesh::_nil();
    } 
    else {
      mySubMesh = aSubMesh;
      myMesh = SMESH::SMESH_Mesh::_nil();
    }

    QString aName;
    SMESH::GetNameOfSelectedIObjects( mySelectionMgr, aName );
    myMeshEdit->setText( aName );

    if ( myType == Type_2d ) {
      loadFromObject( true );
      displayPreview();
    }
  } 
  catch (...) {
    myMesh = SMESH::SMESH_Mesh::_nil();
    mySubMesh = SMESH::SMESH_subMesh::_nil();
    myGeomObj = GEOM::GEOM_Object::_nil();
    erasePreview();
  }
}

//=======================================================================
// function : onDeactivate()
// purpose  : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_CreatePatternDlg::onDeactivate()
{
  disconnect( mySelectionMgr, 0, this, 0 );
  setEnabled( false );
}

//=======================================================================
// function : enterEvent()
// purpose  : Event filter
//=======================================================================
void SMESHGUI_CreatePatternDlg::enterEvent( QEvent* )
{
  // there is a stange problem that enterEvent() comes after onSave()
  if ( isVisible () ) {
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled( true );
    activateSelection();
    connect( mySelectionMgr, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );
  }
}

//=======================================================================
// function : onSelBtnClicked()
// purpose  : SLOT. Called when -> button clicked.
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSelBtnClicked()
{
  onSelectionDone();
}

//================================================================
// function : autoExtension()
// purpose  : Append extension to the file name
//================================================================
QString SMESHGUI_CreatePatternDlg::autoExtension( const QString& theFileName ) const
{
  QString anExt = theFileName.section('.', -1);
  return anExt != "smp" && anExt != "SMP" ? theFileName + ".smp" : theFileName;
}

//=======================================================================
// function : displayPreview()
// purpose  : Display preview
//=======================================================================
void SMESHGUI_CreatePatternDlg::displayPreview()
{
  // Redisplay preview in dialog
  try {
    if ( !myIsLoaded ) {
      erasePreview();
    }
    else {
      SMESH::point_array_var pnts = myPattern->GetPoints();
      SMESH::long_array_var keyPoints = myPattern->GetKeyPoints();
      SMESH::array_of_long_array_var elemPoints = myPattern->GetElementPoints( false );

      if ( pnts->length()       == 0 ||
           keyPoints->length()  == 0 ||
           elemPoints->length() == 0 ) {
        myIsLoaded = false;
        erasePreview();
        return;
      }

      PointVector aPoints( pnts->length() );
      QVector<int> aKeyPoints( keyPoints->length() );
      ConnectivityVector anElemPoints( elemPoints->length() );

      for ( int i = 0, n = pnts->length(); i < n; i++ )
        aPoints[ i ] = pnts[ i ];

      for ( int i2 = 0, n2 = keyPoints->length(); i2 < n2; i2++ )
        aKeyPoints[ i2 ] = keyPoints[ i2 ];

      for (int i3 = 0, n3 = elemPoints->length(); i3 < n3; i3++) {
        QVector<int> aVec (elemPoints[ i3 ].length());
        for (int i4 = 0, n4 = elemPoints[ i3 ].length(); i4 < n4; i4++)
          aVec[ i4 ] = elemPoints[ i3 ][ i4 ];

        anElemPoints[ i3 ] = aVec;
      }

      myPicture2d->SetPoints( aPoints, aKeyPoints, anElemPoints );
    }

    return;

  } 
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch (...) {
  }
  erasePreview();
}

//=======================================================================
// function : erasePreview()
// purpose  : Erase preview
//=======================================================================
void SMESHGUI_CreatePatternDlg::erasePreview()
{
  // Erase preview in 2D viewer
  myPicture2d->SetPoints( PointVector(), QVector<int>(), ConnectivityVector() );
}

//=======================================================================
// function : activateSelection()
// purpose  : Activate selection in accordance with current pattern type
//=======================================================================
void SMESHGUI_CreatePatternDlg::activateSelection()
{
  mySelectionMgr->clearFilters();
  //SUIT_Application::getDesktop()->setSelectionModes(ActorSelection);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ) )
    aViewWindow->SetSelectionMode( ActorSelection );

  if ( myType == Type_2d ) {
    mySelectionMgr->installFilter( new SMESH_NumberFilter( "SMESH",
                                                           TopAbs_SHAPE,
                                                           -1,
                                                           TopAbs_FACE ) );
  } 
  else {
    TColStd_MapOfInteger aTypes;
    aTypes.Add( TopAbs_SHELL );
    aTypes.Add( TopAbs_SOLID );
    mySelectionMgr->installFilter( new SMESH_NumberFilter( "SMESH",
                                                           TopAbs_FACE,
                                                           6,
                                                           aTypes,
                                                           GEOM::GEOM_Object::_nil(),
                                                           true ) );
  }
}

//=======================================================================
// function : onTypeChanged()
// purpose  : SLOT. Called when pattern type changed.
//            Change dialog's look and feel
//=======================================================================
void SMESHGUI_CreatePatternDlg::onTypeChanged( int theType )
{
  if ( myType == theType )
    return;

  myType = theType;

  myPicture2d->setVisible( theType == Type_2d );
  myProjectChk->setVisible( theType == Type_2d );
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePatternDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ){
    e->accept();
    onHelp();
  }
}
