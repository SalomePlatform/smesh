//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File   : SMESHGUI_Make2DFrom3D.cxx
// Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#include "SMESHGUI_Make2DFrom3DOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"

// SALOME GUI includes
#include <LightApp_UpdateFlags.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_Study.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

// Qt includes
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QGridLayout>

#define SPACING 6
#define MARGIN  11

/*!
  \class SMESHGUI_Make2DFrom3DDlg
  \brief Copy Mesh dialog box
*/

SMESHGUI_Make2DFrom3DDlg::SMESHGUI_Make2DFrom3DDlg( QWidget* parent )
 : SMESHGUI_Dialog( parent, false, true, OK | Apply | Close | Help )
{
  // title
  setWindowTitle( tr("CAPTION") );

  // mesh
  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "MESH" ), mainFrame(), Mesh );

  // mode
  QGroupBox* aModeGrp = new QGroupBox( tr( "MODE" ), mainFrame() );
  QHBoxLayout* aModeGrpLayout = new QHBoxLayout( aModeGrp );
  aModeGrpLayout->setMargin( MARGIN );
  aModeGrpLayout->setSpacing( SPACING );
  my2dFrom3dRB = new QRadioButton( tr( "2D_FROM_3D" ), aModeGrp );
  my1dFrom2dRB = new QRadioButton( tr( "1D_FROM_2D" ), aModeGrp );
  my1dFrom3dRB = new QRadioButton( tr( "1D_FROM_3D" ), aModeGrp );
  aModeGrpLayout->addWidget( my2dFrom3dRB );
  aModeGrpLayout->addWidget( my1dFrom2dRB );
  aModeGrpLayout->addWidget( my1dFrom3dRB );

  // target
  QGroupBox* aTargetGrp = new QGroupBox( tr( "TARGET" ), mainFrame() );
  QGridLayout* aTargetGrpLayout = new QGridLayout( aTargetGrp );
  aTargetGrpLayout->setMargin( MARGIN );
  aTargetGrpLayout->setSpacing( SPACING );
  myThisMeshRB   = new QRadioButton( tr( "THIS_MESH" ), aTargetGrp );
  myNewMeshRB    = new QRadioButton( tr( "NEW_MESH" ),  aTargetGrp );
  myMeshName     = new QLineEdit( aTargetGrp );
  myCopyCheck    = new QCheckBox( tr( "COPY_SRC" ),     aTargetGrp );
  myMissingCheck = new QCheckBox( tr( "MISSING_ONLY" ), aTargetGrp );
  aTargetGrpLayout->addWidget( myThisMeshRB,    0, 0 );
  aTargetGrpLayout->addWidget( myNewMeshRB,     1, 0 );
  aTargetGrpLayout->addWidget( myMeshName,     1, 1 );
  aTargetGrpLayout->addWidget( myCopyCheck,    2, 0 );
  aTargetGrpLayout->addWidget( myMissingCheck, 2, 1 );
  myGroupCheck = new QCheckBox( tr( "CREATE_GROUP" ), mainFrame() );
  myGroupName  = new QLineEdit( mainFrame() );

  // layout
  QGridLayout* aDlgLay = new QGridLayout( mainFrame() );
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );
  aDlgLay->addWidget( objectWg( Mesh,  Label ),   0, 0 );
  aDlgLay->addWidget( objectWg( Mesh,  Btn ),     0, 1 );
  aDlgLay->addWidget( objectWg( Mesh,  Control ), 0, 2 );
  aDlgLay->addWidget( aModeGrp,     1, 0, 1, 3 );
  aDlgLay->addWidget( aTargetGrp,   2, 0, 1, 3 );
  aDlgLay->addWidget( myGroupCheck, 3, 0 );
  aDlgLay->addWidget( myGroupName,  3, 1, 1, 2 );

  // connect signals  
  connect( myThisMeshRB, SIGNAL( clicked() ), this, SLOT( onTargetChanged() ) );
  connect( myNewMeshRB,  SIGNAL( clicked() ), this, SLOT( onTargetChanged() ) );
  connect( myGroupCheck, SIGNAL( clicked() ), this, SLOT( onGroupChecked() ) );

  // init dlg
  my2dFrom3dRB->setChecked( true );
  myThisMeshRB->setChecked( true );
  myMissingCheck->setChecked( true );
  onTargetChanged();
  onGroupChecked();
}

SMESHGUI_Make2DFrom3DDlg::~SMESHGUI_Make2DFrom3DDlg()
{
}

SMESH::Bnd_Dimension SMESHGUI_Make2DFrom3DDlg::mode() const
{
  if ( my2dFrom3dRB->isChecked() )
    return SMESH::BND_2DFROM3D;
  else if ( my1dFrom2dRB->isChecked() )
    return SMESH::BND_1DFROM2D;
  else
    return SMESH::BND_1DFROM3D;
}

bool SMESHGUI_Make2DFrom3DDlg::needNewMesh() const
{
  return myNewMeshRB->isChecked();
}

QString SMESHGUI_Make2DFrom3DDlg::getNewMeshName() const
{
  return myMeshName->text().trimmed();
}

void SMESHGUI_Make2DFrom3DDlg::setNewMeshName( const QString& name )
{
  myMeshName->setText( name );
}

bool SMESHGUI_Make2DFrom3DDlg::needGroup() const
{
  return myGroupCheck->isChecked();
}

QString SMESHGUI_Make2DFrom3DDlg::getGroupName() const
{
  return myGroupName->text().trimmed();
}

void SMESHGUI_Make2DFrom3DDlg::setGroupName( const QString& name )
{
  myGroupName->setText( name );
}

bool SMESHGUI_Make2DFrom3DDlg::copySource() const
{
  return myCopyCheck->isChecked();
}

bool SMESHGUI_Make2DFrom3DDlg::copyMissingOnly() const
{
  return myMissingCheck->isChecked();
}

void SMESHGUI_Make2DFrom3DDlg::onTargetChanged()
{
  myMeshName->setEnabled( myNewMeshRB->isChecked() );
  myCopyCheck->setEnabled( myNewMeshRB->isChecked() );
  myMissingCheck->setEnabled( myNewMeshRB->isChecked() );
}

void SMESHGUI_Make2DFrom3DDlg::onGroupChecked()
{
  myGroupName->setEnabled( myGroupCheck->isChecked() );
}

/*!
  \class SMESHGUI_Make2DFrom3DOp
  \brief Copy Mesh operation class
*/

SMESHGUI_Make2DFrom3DOp::SMESHGUI_Make2DFrom3DOp()
 : SMESHGUI_SelectionOp()
{
}

SMESHGUI_Make2DFrom3DOp::~SMESHGUI_Make2DFrom3DOp()
{
  if ( myDlg )
    delete myDlg;
}

LightApp_Dialog* SMESHGUI_Make2DFrom3DOp::dlg() const
{
  return myDlg;
}

void SMESHGUI_Make2DFrom3DOp::startOperation()
{
  if( !myDlg )
    myDlg = new SMESHGUI_Make2DFrom3DDlg( desktop() );

  mySrc = SMESH::SMESH_IDSource::_nil();
  
  myHelpFileName = "make_2dmesh_from_3d_page.html";

  SMESHGUI_SelectionOp::startOperation();

  myDlg->activateObject( SMESHGUI_Make2DFrom3DDlg::Mesh );
  myDlg->setNewMeshName( SMESH::UniqueName( "Mesh_1" ) );
  myDlg->setGroupName( SMESH::UniqueName( "Group" ) );
  myDlg->show();

  selectionDone();
}

void SMESHGUI_Make2DFrom3DOp::selectionDone()
{
  if ( !dlg() ) return;

  if ( dlg()->isVisible() ) {
    try {
      QStringList names, ids;
      LightApp_Dialog::TypesList types;
      selected( names, types, ids );
      if ( names.count() == 1 )
        myDlg->selectObject( names, types, ids );
      else
        myDlg->clearSelection();
    }
    catch ( const SALOME::SALOME_Exception& S_ex ) {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
    }
    catch ( ... ) {
    }
  }
}

SUIT_SelectionFilter* SMESHGUI_Make2DFrom3DOp::createFilter( const int theId ) const
{
  SUIT_SelectionFilter* f = 0;
  if ( theId == SMESHGUI_Make2DFrom3DDlg::Mesh ) {
    QList<SUIT_SelectionFilter*> filters;
    filters.append( new SMESH_TypeFilter( MESHorSUBMESH ) );
    filters.append( new SMESH_TypeFilter( GROUP ) );
    f = new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );
  }
  return f;
}

bool SMESHGUI_Make2DFrom3DOp::isValid( QString& msg ) const
{
  if ( !dlg() ) return false;

  // check if any source data is selected
  QString entry = myDlg->selectedObject( SMESHGUI_Make2DFrom3DDlg::Mesh );
  SMESH::SMESH_IDSource_var obj;
  _PTR(SObject) sobj = SMESHGUI::activeStudy()->studyDS()->FindObjectID( entry.toLatin1().constData() );
  if ( sobj )
    obj = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );  

  if ( obj->_is_nil() ) {
    msg = tr( "SMESH_ERR_NO_INPUT_MESH" );
    return false;
  }

  // check if source contains elements of required type
  SMESH::Bnd_Dimension mode = myDlg->mode();
  SMESH::array_of_ElementType_var types = obj->GetTypes();
  
  bool has3d = false;
  bool has2d = false;
  for ( int i = 0; i < types->length(); i++ ) {
    if      ( types[i] == SMESH::VOLUME ) has3d = true;
    else if ( types[i] == SMESH::FACE )   has2d = true;
  }

  if ( ( mode == SMESH::BND_2DFROM3D || mode == SMESH::BND_1DFROM3D ) && !has3d ) {
    msg = tr( "SMESH_ERR_NO_3D_ELEMENTS" );
    return false;
  }
  else if ( mode == SMESH::BND_1DFROM2D && !has2d ) {
    msg = tr( "SMESH_ERR_NO_2D_ELEMENTS" );
    return false;
  }

  // check if new mesh name is specified
  if ( myDlg->needNewMesh() && myDlg->getNewMeshName().isEmpty() ) {
    msg = tr( "SMESH_ERR_MESH_NAME_NOT_SPECIFIED" );
    return false;
  }

  // check if group name is specified
  if ( myDlg->needGroup() && myDlg->getGroupName().isEmpty() ) {
    msg = tr( "SMESH_ERR_GRP_NAME_NOT_SPECIFIED" );
    return false;
  }
    
  return true;
}

bool SMESHGUI_Make2DFrom3DOp::compute2DMesh()
{
  SUIT_OverrideCursor wc;

  bool ok = false;
  try {
    QString entry = myDlg->selectedObject( SMESHGUI_Make2DFrom3DDlg::Mesh );
    _PTR(SObject) sobj = SMESHGUI::activeStudy()->studyDS()->FindObjectID( entry.toLatin1().constData() );
    SMESH::SMESH_IDSource_var obj = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );  
    
    SMESH::Bnd_Dimension mode = myDlg->mode();
    QString meshName  = myDlg->needNewMesh() ? myDlg->getNewMeshName() : QString();
    QString groupName = myDlg->needGroup()   ? myDlg->getGroupName()   : QString();
    bool copySrc = myDlg->copySource();
    bool copyAll = !myDlg->copyMissingOnly();

    SMESH::SMESH_Mesh_var srcMesh = SMESH::SMESH_Mesh::_narrow( obj );
    if ( CORBA::is_nil( srcMesh ) ) {
      SMESH::SMESH_subMesh_var subMesh = SMESH::SMESH_subMesh::_narrow( obj );
      if ( !CORBA::is_nil( subMesh ) )
        srcMesh = subMesh->GetFather();
    }
    if ( CORBA::is_nil( srcMesh ) ) {
      SMESH::SMESH_GroupBase_var grp = SMESH::SMESH_GroupBase::_narrow( obj );
      if ( !CORBA::is_nil( grp ) )
        srcMesh = grp->GetMesh();
    }

    if ( !CORBA::is_nil( srcMesh ) ) {
      SMESH::SMESH_MeshEditor_var aMeshEditor = srcMesh->GetMeshEditor();
      SMESH::SMESH_Group_var newGrp;
      SMESH::SMESH_Mesh_var mesh = aMeshEditor->MakeBoundaryMesh( obj.in(), 
                                                                  mode,
                                                                  groupName.toLatin1().constData(),
                                                                  meshName.toLatin1().constData(),
                                                                  copySrc,
                                                                  copyAll,
                                                                  newGrp.out() );
      if ( !mesh->_is_nil() ) {
#ifdef WITHGENERICOBJ
        mesh->Destroy();
#endif
      }
      if ( !newGrp->_is_nil() ) {
#ifdef WITHGENERICOBJ
        newGrp->Destroy();
#endif
      }
      ok = true;
    }
  }
  catch ( ... ) {
  }
  return ok;
}

bool SMESHGUI_Make2DFrom3DOp::onApply()
{
  if ( isStudyLocked() )
    return false;

  QString msg;
  if ( !isValid( msg ) ) {
    dlg()->show();
    if ( msg != "" )
      SUIT_MessageBox::warning( myDlg, tr( "SMESH_ERROR" ), msg );
    return false;
  }

  bool res = false;
  try {
    res = compute2DMesh();
  }
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch ( ... ) {
  }

  if ( res ) {
    SMESHGUI::Modified();
    update( UF_ObjBrowser | UF_Model );
    myDlg->setNewMeshName( SMESH::UniqueName( "Mesh_1" ) );
    myDlg->setGroupName( SMESH::UniqueName( "Group" ) );
  }
  else {
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_ERROR" ), tr( "SMESH_OPERATION_FAILED" ) );
  }

  return res;
}
