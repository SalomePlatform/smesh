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
//  File   : SMESHGUI_CreatePatternDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_CreatePatternDlg.h"

#include "QAD_Desktop.h"
#include "QAD_FileDlg.h"

#include "SMESHGUI_PatternWidget.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI.h"
#include "SALOME_Selection.h"
#include "SALOME_ListIteratorOfListOfFilter.hxx"
#include "SALOMEGUI_QtCatchCorbaException.hxx"
#include "SMESH_NumberFilter.hxx"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_PatternUtils.h"

#include <TColStd_MapOfInteger.hxx>

#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qmessagebox.h>
#include <qapplication.h>

#define SPACING 5
#define MARGIN  10

/*
  Class       : SMESHGUI_CreatePatternDlg
  Description : Dialog to specify filters for VTK viewer
*/

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::SMESHGUI_CreatePatternDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_CreatePatternDlg::SMESHGUI_CreatePatternDlg( QWidget*          theParent,
                                                      SALOME_Selection* theSelection,
                                                      const int         theType,
                                                      const char*       theName )
: QDialog( theParent, theName, false,
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  setCaption( tr( "CAPTION" ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( this, MARGIN, SPACING );

  QFrame* aMainFrame = createMainFrame  ( this );
  QFrame* aBtnFrame  = createButtonFrame( this );

  aDlgLay->addWidget( aMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( aMainFrame, 1 );

  Init( theSelection, theType );
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_CreatePatternDlg::createMainFrame( QWidget* theParent )
{
  QPixmap iconSlct( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_SELECT" ) ) );
  QPixmap icon2d  ( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_PATTERN_2d" ) ) );
  QPixmap icon3d  ( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_PATTERN_3d" ) ) );

  QPixmap iconSample2d( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", tr( "ICON_PATTERN_SAMPLE_2D" ) ) );

  QGroupBox* aMainGrp = new QGroupBox( 1, Qt::Horizontal, theParent );
  aMainGrp->setFrameStyle( QFrame::NoFrame );
  aMainGrp->setInsideMargin( 0 );

  // Pattern type group

  myTypeGrp = new QButtonGroup( 1, Qt::Vertical, tr( "PATTERN_TYPE" ), aMainGrp );
  mySwitch2d = new QRadioButton( myTypeGrp );
  mySwitch3d = new QRadioButton( myTypeGrp );
  mySwitch2d->setPixmap( icon2d );
  mySwitch3d->setPixmap( icon3d );
  myTypeGrp->insert( mySwitch2d, Type_2d );
  myTypeGrp->insert( mySwitch3d, Type_3d );

  QGroupBox* aPatternGrp = new QGroupBox( 1, Qt::Horizontal, tr( "PATTERN" ), aMainGrp );

  // Mesh and pattern name group
  
  QGroupBox* aMeshGrp = new QGroupBox( 1, Qt::Vertical, aPatternGrp );
  aMeshGrp->setFrameStyle( QFrame::NoFrame );
  aMeshGrp->setInsideMargin( 0 );
  
  new QLabel( tr( "MESH_OR_SUBMESH" ), aMeshGrp );

  QPushButton* aSelBtn = new QPushButton( aMeshGrp );
  aSelBtn->setPixmap( iconSlct );
  myMeshEdit = new QLineEdit( aMeshGrp );
  myMeshEdit->setReadOnly( true );

  QGroupBox* aPattGrp = new QGroupBox( 1, Qt::Vertical, aPatternGrp );
  aPattGrp->setFrameStyle( QFrame::NoFrame );
  aPattGrp->setInsideMargin( 0 );
  
  new QLabel( tr( "PATTERN_NAME" ), aPattGrp );
  myName = new QLineEdit( aPattGrp );

  // Picture 2d
  
  myPicture2d = new SMESHGUI_PatternWidget( aPatternGrp ),
  myPicture2d->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

  // Project check box
  
  myProjectChk = new QCheckBox( tr( "PROJECT" ), aPatternGrp );
  
  // Connect signals and slots

  connect( myTypeGrp,    SIGNAL( clicked( int )  ), SLOT( onTypeChanged( int ) ) );
  connect( myProjectChk, SIGNAL( toggled( bool ) ), SLOT( onProject( bool )    ) );
  connect( aSelBtn,      SIGNAL( clicked()       ), SLOT( onSelBtnClicked() ) );

  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_CreatePatternDlg::createButtonFrame( QWidget* theParent )
{
  QFrame* aFrame = new QFrame( theParent );
  aFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

  myOkBtn    = new QPushButton( tr( "SMESH_BUT_OK"     ), aFrame );
  mySaveBtn  = new QPushButton( tr( "SAVE"             ), aFrame );
  myCloseBtn = new QPushButton( tr( "SMESH_BUT_CANCEL" ), aFrame );

  QSpacerItem* aSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );

  QHBoxLayout* aLay = new QHBoxLayout( aFrame, MARGIN, SPACING );

  aLay->addWidget( myOkBtn );
  aLay->addWidget( mySaveBtn );
  aLay->addItem( aSpacer);
  aLay->addWidget( myCloseBtn );

  connect( myOkBtn,    SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myCloseBtn, SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( mySaveBtn, SIGNAL( clicked() ), SLOT( onSave() ) );

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::~SMESHGUI_CreatePatternDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_CreatePatternDlg::~SMESHGUI_CreatePatternDlg()
{
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onProject
// Purpose : SLOT. Called when state of "Project nodes on ther face"
//           checkbox is changed
//=======================================================================
void SMESHGUI_CreatePatternDlg::onProject( bool )
{
  loadFromObject( false );
  displayPreview();
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::Init( SALOME_Selection* theSelection, const int theType )
{
  myIsLoaded  = false;
  myType      = -1;
  myNbPoints  = -1;
  mySelection = theSelection;
  mySubMesh   = SMESH::SMESH_subMesh::_nil();
  myMesh      = SMESH::SMESH_Mesh::_nil();
  myGeomObj   = GEOM::GEOM_Object::_nil();
  myPattern   = SMESH::SMESH_Pattern::_nil();
  
  erasePreview();

  SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI();
  aSMESHGUI->SetActiveDialogBox( ( QDialog* )this );

  // selection and SMESHGUI
  connect( mySelection, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );
  connect( aSMESHGUI, SIGNAL( SignalDeactivateActiveDialog() ), SLOT( onDeactivate() ) );
  connect( aSMESHGUI, SIGNAL( SignalCloseAllDialogs() ), SLOT( onClose() ) );

  mySwitch2d->setEnabled( theType == Type_2d );
  mySwitch3d->setEnabled( theType == Type_3d );

  if ( theType == Type_2d )
    myProjectChk->show();
  else
    myProjectChk->hide();
  
  myTypeGrp->setButton( theType );
  onTypeChanged( theType );
  
  myName->setText( getDefaultName() );
  myMeshEdit->setText( "" );

  setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ) );
  qApp->processEvents();
  updateGeometry();
  myPicture2d->updateGeometry();
  adjustSize();
  resize( minimumSize() );

  activateSelection();
  onSelectionDone();

  int x, y ;
  aSMESHGUI->DefineDlgPosition( this, x, y );
  this->move( x, y );
  this->show();
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::SetMesh
// Purpose : Set mesh to dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::SetMesh( SMESH::SMESH_Mesh_ptr thePtr )
{
  myMesh = SMESH::SMESH_Mesh::_duplicate( thePtr );
  mySubMesh = SMESH::SMESH_subMesh::_nil();

  bool isValidMesh = false;
  if ( !myMesh->_is_nil() )
  {
    SALOMEDS::SObject_var aSobj = SMESH::FindSObject( thePtr );
    CORBA::String_var anEntry = aSobj->GetID();
    Handle(SALOME_InteractiveObject) anIObj =
      new SALOME_InteractiveObject( anEntry.in(), "SMESH" );

    const SALOME_ListOfFilter& aList = mySelection->StoredFilters();
    SALOME_ListIteratorOfListOfFilter anIter( aList );
    for ( ; anIter.More(); anIter.Next() )
      if ( !anIter.Value()->IsOk( anIObj ) )
        break;

    if ( !anIter.More() )
      isValidMesh = true;
  }


  if ( isValidMesh )
  {
    SALOMEDS::SObject_var aSO = SMESH::FindSObject( myMesh.in() );
    myMeshEdit->setText( aSO->GetName() );
    myGeomObj = getGeom( aSO );
  }
  else
  {
    myMeshEdit->setText( "" );
    myGeomObj = GEOM::GEOM_Object::_nil();
  }

  if ( myType == Type_2d )
  {
    loadFromObject( false );
    displayPreview();
  }
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::isValid
// Purpose : Verify validity of entry data
//=======================================================================
bool SMESHGUI_CreatePatternDlg::isValid()
{
  if ( myGeomObj->_is_nil() )
  {
    QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
      tr( "SMESH_INSUFFICIENT_DATA" ), tr( "SMESHGUI_INVALID_PARAMETERS" ), QMessageBox::Ok );
    return false;
  }
  else
    return true;
}



//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::getDefaultName
// Purpose : Get default pattern name
//=======================================================================
QString SMESHGUI_CreatePatternDlg::getDefaultName() const
{
  return myType == Type_2d ? tr( "DEFAULT_2D" ) : tr( "DEFAULT_3D" );
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onSave
// Purpose : SLOT called when "Save" button pressed. Build pattern and

//           save it to disk
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSave()
{
  try
  {
    if ( !isValid() )
      return;

    if ( !myIsLoaded )
      loadFromObject( true );

    // Load pattern from object
    if ( !myIsLoaded )
      return;

    ///////////////////////////////////////////////////////
    QAD_FileDlg* aDlg = new QAD_FileDlg( this, false );
    aDlg->setCaption( tr( "SAVE_PATTERN" ) );
    aDlg->setMode( QFileDialogP::AnyFile );
    aDlg->setFilters( tr( "PATTERN_FILT" ) );
    if ( myName->text() != "" )
      aDlg->setSelection( myName->text() );

    if ( aDlg->exec() != Accepted )
      return;

    QString fName = aDlg->selectedFile();
    if ( fName.isEmpty() )
      return;

    if ( QFileInfo( fName ).extension().isEmpty() )
      fName = autoExtension( fName );

    fName = QDir::convertSeparators( fName );

    QString aData( myPattern->GetString() );
    long aLen = aData.length();

    QFile aFile( fName );
    aFile.open( IO_WriteOnly );
    long aWritten = aFile.writeBlock( aData, aLen );
    aFile.close();

    if ( aWritten != aLen )
    {
      QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
        tr( "SMESH_ERROR" ), tr( "ERROR_OF_SAVING" ), QMessageBox::Ok );
    }
    else
    {
      QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
      disconnect( mySelection, 0, this, 0 );
      disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
      SMESHGUI::GetSMESHGUI()->ResetState();
      accept();
      emit NewPattern();
    }
  }
  catch( const SALOME::SALOME_Exception& S_ex )
  {
    QtCatchCorbaException( S_ex );
  }
  catch( ... )
  {
  }
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::GetPatternName
// Purpose : Get name of pattern
//=======================================================================
QString SMESHGUI_CreatePatternDlg::GetPatternName() const
{
  return myName->text();
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::GetPattern
// Purpose : Get result pattern
//=======================================================================
SMESH::SMESH_Pattern_ptr SMESHGUI_CreatePatternDlg::GetPattern()
{
  return myPattern.in();
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_CreatePatternDlg::onOk()
{
  try
  {
    if ( !isValid() )
      return;

    if ( !myIsLoaded )
      loadFromObject( true );

    // Load pattern from object
    if ( !myIsLoaded )
      return;
    else
    {
      QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
      disconnect( mySelection, 0, this, 0 );
      disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
      SMESHGUI::GetSMESHGUI()->ResetState();
      accept();
      emit NewPattern();
    }
  }
  catch( const SALOME::SALOME_Exception& S_ex )
  {
    QtCatchCorbaException( S_ex );
  }
  catch( ... )
  {
  }
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::onClose()
{
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
  SMESHGUI::GetSMESHGUI()->ResetState();
  reject();
  emit Close();
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::loadFromObject
// Purpose : Load pattern from geom object corresponding to the mesh/submesh
//=======================================================================
bool SMESHGUI_CreatePatternDlg::loadFromObject( const bool theMess )
{
  try
  {
    myIsLoaded = false;
    
    if ( myPattern->_is_nil() )
      myPattern = SMESH::GetPattern();

    if ( myMesh->_is_nil() && mySubMesh->_is_nil() || myGeomObj->_is_nil() )
      return false;

    SMESH::SMESH_Mesh_ptr aMesh = mySubMesh->_is_nil() ? myMesh.in() : mySubMesh->GetFather();

    myIsLoaded = myType == Type_2d
      ? myPattern->LoadFromFace( aMesh, myGeomObj, myProjectChk->isChecked() )
      : myPattern->LoadFrom3DBlock( aMesh, myGeomObj );

    if ( !myIsLoaded && theMess )
    {
      QString aMess;
      SMESH::SMESH_Pattern::ErrorCode aCode = myPattern->GetErrorCode();
      
      if      ( aCode == SMESH::SMESH_Pattern::ERR_LOAD_EMPTY_SUBMESH   ) aMess = tr( "ERR_LOAD_EMPTY_SUBMESH" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADF_NARROW_FACE    ) aMess = tr( "ERR_LOADF_NARROW_FACE" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADF_CLOSED_FACE    ) aMess = tr( "ERR_LOADF_CLOSED_FACE" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADV_BAD_SHAPE      ) aMess = tr( "ERR_LOADV_BAD_SHAPE" );
      else if ( aCode == SMESH::SMESH_Pattern::ERR_LOADV_COMPUTE_PARAMS ) aMess = tr( "ERR_LOADV_COMPUTE_PARAMS" );
      else                                                                aMess = tr( "ERROR_OF_CREATION" );

      QMessageBox::information(
        SMESHGUI::GetSMESHGUI()->GetDesktop(), tr( "SMESH_ERROR" ), aMess, QMessageBox::Ok );
    }

  }
  catch( const SALOME::SALOME_Exception& S_ex )
  {
    QtCatchCorbaException( S_ex );
  }
    
  return myIsLoaded;
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::getGeom
// Purpose : Retrieve geom object from SObject
//=======================================================================
GEOM::GEOM_Object_ptr SMESHGUI_CreatePatternDlg::getGeom( SALOMEDS::SObject_ptr theSO )
{
  if ( theSO->_is_nil() )
    return GEOM::GEOM_Object::_nil();

  SALOMEDS::Study_var aStudy =
    SMESHGUI::GetSMESHGUI()->GetActiveStudy()->getStudyDocument();

  SALOMEDS::ChildIterator_var anIter = aStudy->NewChildIterator( theSO );
  for( ; anIter->More(); anIter->Next() )
  {
    SALOMEDS::SObject_var aSO = anIter->Value();
    SALOMEDS::SObject_var aRefSO;

    GEOM::GEOM_Object_var aMeshShape = GEOM::GEOM_Object::_narrow(
      aSO->ReferencedObject( aRefSO )? aRefSO->GetObject() : aSO->GetObject() );

    if ( !aMeshShape->_is_nil() )
      return aMeshShape._retn();
  }
  return GEOM::GEOM_Object::_nil();
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSelectionDone()
{
  try
  {
    if ( mySelection->IObjectCount() != 1 )
      return;

    // Get mesh or sub-mesh from selection
    Handle(SALOME_InteractiveObject) anIO = mySelection->firstIObject();
    SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>( anIO );
    SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>( anIO );
    if ( aMesh->_is_nil() && aSubMesh->_is_nil() )
      return;

    // Get geom object corresponding to the mesh
    SALOMEDS::SObject_var aSO;
    if ( !aMesh->_is_nil() )
      aSO = SMESH::FindSObject( aMesh.in() );
    else
      aSO = SMESH::FindSObject( aSubMesh.in() );


    GEOM::GEOM_Object_var aGeomObj = getGeom( aSO );
    if ( aGeomObj->_is_nil() )
      return;

    myGeomObj = aGeomObj;

    // init class fields
    if ( !aMesh->_is_nil() )
    {
      myMesh = aMesh;
      mySubMesh = SMESH::SMESH_subMesh::_nil();
    }
    else
    {
      mySubMesh = aSubMesh;
      myMesh = SMESH::SMESH_Mesh::_nil();
    }

    QString aName;
    SMESH::GetNameOfSelectedIObjects( mySelection, aName );
    myMeshEdit->setText( aName );

    if ( myType == Type_2d )
    {
      loadFromObject( true );
      displayPreview();
    }
  }
  catch( ... )
  {
    myMesh = SMESH::SMESH_Mesh::_nil();
    mySubMesh = SMESH::SMESH_subMesh::_nil();
    myGeomObj = GEOM::GEOM_Object::_nil();
    erasePreview();
  }
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_CreatePatternDlg::onDeactivate()
{
  disconnect( mySelection, 0, this, 0 );
  setEnabled( false );
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_CreatePatternDlg::enterEvent( QEvent* )
{
  SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
  setEnabled( true );
  activateSelection();
  connect( mySelection, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );
}


//=================================================================================
// function : closeEvent()
// purpose  : Close dialog box
//=================================================================================
void SMESHGUI_CreatePatternDlg::closeEvent( QCloseEvent* e )
{
  onClose() ;
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onSelBtnClicked
// Purpose : SLOT. Called when -> button clicked.
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSelBtnClicked()
{
  onSelectionDone();
}

//================================================================
// Function : SMESHGUI_CreatePatternDlg::autoExtension
// Purpose  : Append extension to the file name
//================================================================
QString SMESHGUI_CreatePatternDlg::autoExtension( const QString& theFileName ) const
{
  QString anExt = theFileName.section( '.', -1 );
  return anExt != "smp" && anExt != "SMP" ? theFileName + ".smp" : theFileName;
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::displayPreview
// Purpose : Display preview
//=======================================================================
void SMESHGUI_CreatePatternDlg::displayPreview()
{

  // Redisplay preview in dialog

  try
  {
    if ( !myIsLoaded )
      erasePreview();
    else
    {
      SMESH::point_array_var pnts = myPattern->GetPoints();
      SMESH::long_array_var keyPoints = myPattern->GetKeyPoints();
      SMESH::array_of_long_array_var elemPoints = myPattern->GetElementPoints();

      if ( pnts->length()       == 0 ||
           keyPoints->length()  == 0 ||
           elemPoints->length() == 0 )
      {
        myIsLoaded = false;
        erasePreview();
        return;
      }

      PointVector aPoints( pnts->length() );
      QValueVector<int> aKeyPoints( keyPoints->length() );
      ConnectivityVector anElemPoints( elemPoints->length() );

      for ( int i = 0, n = pnts->length(); i < n; i++ )
        aPoints[ i ] = pnts[ i ];

      for ( int i2 = 0, n2 = keyPoints->length(); i2 < n2; i2++ )
        aKeyPoints[ i2 ] = keyPoints[ i2 ];

      for ( int i3 = 0, n3 = elemPoints->length(); i3 < n3; i3++ )
      {
        QValueVector<int> aVec( elemPoints[ i3 ].length() );
        for ( int i4 = 0, n4 = elemPoints[ i3 ].length(); i4 < n4; i4++ )
          aVec[ i4 ] = elemPoints[ i3 ][ i4 ];

        anElemPoints[ i3 ] = aVec;
      }

      myPicture2d->SetPoints( aPoints, aKeyPoints, anElemPoints );
    }

    return;
  }
  catch( const SALOME::SALOME_Exception& S_ex )
  {
    QtCatchCorbaException( S_ex );
  }
  catch( ... )
  {
  }
  erasePreview();
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::erasePreview
// Purpose : Erase preview
//=======================================================================
void SMESHGUI_CreatePatternDlg::erasePreview()

{
  // Erase preview in 2D viewer
  myPicture2d->SetPoints( PointVector(), QValueVector<int>(), ConnectivityVector() );
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::activateSelection
// Purpose : Activate selection in accordance with current pattern type
//=======================================================================
void SMESHGUI_CreatePatternDlg::activateSelection()
{
  mySelection->ClearFilters();
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  
  if ( myType == Type_2d )
  {
    mySelection->AddFilter(
      new SMESH_NumberFilter( "SMESH", TopAbs_SHAPE, -1, TopAbs_FACE ) );
  }
  else
  {
    TColStd_MapOfInteger aTypes;
    aTypes.Add( TopAbs_SHELL );
    aTypes.Add( TopAbs_SOLID );
    mySelection->AddFilter( new SMESH_NumberFilter(
      "SMESH", TopAbs_FACE, 6, aTypes, GEOM::GEOM_Object::_nil(), true ) );
  }
}

//=======================================================================
// name    : SMESHGUI_CreatePatternDlg::onTypeChanged
// Purpose : SLOT. Called when pattern type changed.
//           Change dialog's look and feel
//=======================================================================
void SMESHGUI_CreatePatternDlg::onTypeChanged( int theType )
{
  if ( myType == theType )
    return;

  myType = theType;

  if ( theType == Type_2d )
    myPicture2d->show();
  else
    myPicture2d->hide();
}





















