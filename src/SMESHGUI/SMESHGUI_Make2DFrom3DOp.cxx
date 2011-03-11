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
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SVTK_ViewModel.h>
#include <SalomeApp_Study.h>
#include <SalomeApp_Tools.h>

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
#include <QToolButton>

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

  // Groups of mesh faces
  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "Groups" ), mainFrame(), Groups );
  setNameIndication( Groups, ListOfNames );
  objectWg( Groups, Btn )->hide();

  // target
  QGroupBox* aTargetGrp = new QGroupBox( tr( "TARGET" ), mainFrame() );
  QGridLayout* aTargetGrpLayout = new QGridLayout( aTargetGrp );
  aTargetGrpLayout->setMargin( MARGIN );
  aTargetGrpLayout->setSpacing( SPACING );
  myThisMeshRB   = new QRadioButton( tr( "THIS_MESH" ), aTargetGrp );
  myNewMeshRB    = new QRadioButton( tr( "NEW_MESH" ),  aTargetGrp );
  myMeshName     = new QLineEdit( aTargetGrp );
  myCopyCheck    = new QCheckBox( tr( "COPY_SRC" ),     aTargetGrp );
  aTargetGrpLayout->addWidget( myThisMeshRB,    0, 0 );
  aTargetGrpLayout->addWidget( myNewMeshRB,     1, 0 );
  aTargetGrpLayout->addWidget( myMeshName,     1, 1 );
  aTargetGrpLayout->addWidget( myCopyCheck,    2, 0 );
  myGroupCheck = new QCheckBox( tr( "CREATE_GROUP" ), mainFrame() );
  myGroupName  = new QLineEdit( mainFrame() );

  // layout
  QGridLayout* aDlgLay = new QGridLayout( mainFrame() );
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );
  aDlgLay->addWidget( aModeGrp,     0, 0, 1, 3 );
  aDlgLay->addWidget( objectWg( Groups,  Label ),   1, 0 );
  aDlgLay->addWidget( objectWg( Groups,  Control ), 1, 1 );
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

void SMESHGUI_Make2DFrom3DDlg::onTargetChanged()
{
  myMeshName->setEnabled( myNewMeshRB->isChecked() );
  myCopyCheck->setEnabled( myNewMeshRB->isChecked() );
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

  myHelpFileName = "make_2dmesh_from_3d_page.html";

  SMESHGUI_SelectionOp::startOperation();

  myDlg->setNewMeshName( SMESH::UniqueName( "Mesh_1" ) );
  myDlg->setGroupName( SMESH::UniqueName( "Group" ) );
  myDlg->show();

  connect( myDlg->my2dFrom3dRB, SIGNAL( toggled(bool) ), this, SLOT( onModeChanged() ) );
  connect( myDlg->my1dFrom2dRB, SIGNAL( toggled(bool) ), this, SLOT( onModeChanged() ) );
  connect( myDlg->my1dFrom3dRB, SIGNAL( toggled(bool) ), this, SLOT( onModeChanged() ) );

  onModeChanged();
}

void SMESHGUI_Make2DFrom3DOp::onModeChanged()
{
  QRadioButton* b = dynamic_cast<QRadioButton*>( sender());
  if ( b && !b->isChecked() )
    return;

  // enable "2D groups" field
  bool enableGroups = ( myDlg->mode() == SMESH::BND_1DFROM3D );
  myDlg->setObjectEnabled( SMESHGUI_Make2DFrom3DDlg::Groups, enableGroups );
  ((QToolButton*) myDlg->objectWg( SMESHGUI_Make2DFrom3DDlg::Groups,
                                   SMESHGUI_Make2DFrom3DDlg::Btn ))->setChecked( enableGroups );
  
  // install filter
  int id =  enableGroups ? SMESHGUI_Make2DFrom3DDlg::Groups : SMESHGUI_Make2DFrom3DDlg::Mesh;
  onDeactivateObject( id );
  onActivateObject( id );
}

void SMESHGUI_Make2DFrom3DOp::selectionDone()
{
  mySrcMesh = SMESH::SMESH_Mesh::_nil();
  myDlg->clearSelection( SMESHGUI_Make2DFrom3DDlg::Groups );

  if ( !dlg() ) return;

  
  if ( dlg()->isVisible() ) {
    try {
      QStringList names, ids;
      LightApp_Dialog::TypesList types;
      selected( names, types, ids );
      myDlg->selectObject( names, types, ids );

      SALOME_ListIO sel; selectionMgr()->selectedObjects( sel, SVTK_Viewer::Type() );
      if ( !sel.IsEmpty() )
      {
        SMESH::SMESH_IDSource_var IS = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(sel.First());
        if(!CORBA::is_nil(IS))
          mySrcMesh = IS->GetMesh();
      }
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
  MeshObjectType type = ( theId == SMESHGUI_Make2DFrom3DDlg::Groups ? GROUP_FACE : MESH );
  SUIT_SelectionFilter* f = new SMESH_TypeFilter( type );
  return f;
}

bool SMESHGUI_Make2DFrom3DOp::isValid( QString& msg ) const
{
  if ( !dlg() ) return false;

  // check if a mesh is selected
  if ( mySrcMesh->_is_nil() )
  {
    msg = tr( "SMESH_ERR_NO_INPUT_MESH" );
    return false;
  }
  // check if groups are selected
  SMESH::Bnd_Dimension mode = myDlg->mode();
  if ( mode == SMESH::BND_1DFROM3D )
  {
    SMESH::SMESH_GroupBase_var grp;
    QStringList entries;
    dlg()->selectedObject( SMESHGUI_Make2DFrom3DDlg::Groups, entries );
    if ( !entries.isEmpty() )
    {
      _PTR(SObject) sobj = SMESHGUI::activeStudy()->studyDS()->FindObjectID( entries[0].toLatin1().constData() );
      if ( sobj )
        grp = SMESH::SObjectToInterface<SMESH::SMESH_GroupBase>( sobj );
    }
    if ( grp->_is_nil() ) {
      msg = tr( "SMESH_ERR_NO_INPUT_GROUP" );
      return false;
    }
  }
  else
  {
    // check if mesh contains elements of required type
    SMESH::Bnd_Dimension mode = myDlg->mode();

    if ( mode == SMESH::BND_2DFROM3D && mySrcMesh->NbVolumes() == 0 ) {
      msg = tr( "SMESH_ERR_NO_3D_ELEMENTS" );
      return false;
    }
    else if ( mode == SMESH::BND_1DFROM2D && mySrcMesh->NbFaces() == 0  ) {
      msg = tr( "SMESH_ERR_NO_2D_ELEMENTS" );
      return false;
    }
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
    QStringList entries;
    dlg()->selectedObject( SMESHGUI_Make2DFrom3DDlg::Groups, entries );
    SMESH::ListOfIDSources_var groups = new SMESH::ListOfIDSources;
    groups->length( entries.count() );
    for ( int i = 0; i < entries.count(); ++i )
    {
      _PTR(SObject) sobj = SMESHGUI::activeStudy()->studyDS()->FindObjectID( entries[i].toLatin1().constData() );
      SMESH::SMESH_IDSource_var grp = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );  
      groups[i] = grp;
    }
    SMESH::Bnd_Dimension mode = myDlg->mode();
    QString meshName  = myDlg->needNewMesh() ? myDlg->getNewMeshName() : QString();
    QString groupName = myDlg->needGroup()   ? myDlg->getGroupName()   : QString();
    bool copyAll = myDlg->copySource();

    if ( !CORBA::is_nil( mySrcMesh ) ) {
      SMESH::SMESH_MeshEditor_var aMeshEditor = mySrcMesh->GetMeshEditor();
      SMESH::SMESH_Group_var newGrp;
      SMESH::SMESH_Mesh_var newMesh;
      CORBA::Long nbAdded = aMeshEditor->MakeBoundaryElements( mode,
                                                               groupName.toLatin1().constData(),
                                                               meshName.toLatin1().constData(),
                                                               copyAll,
                                                               groups,
                                                               newMesh.out(),
                                                               newGrp.out() );
      SUIT_MessageBox::information( myDlg,
                                    tr("SMESH_INFORMATION"),
                                    tr("NB_ADDED").arg( nbAdded ));
      if ( !newMesh->_is_nil() ) {
#ifdef WITHGENERICOBJ
        newMesh->UnRegister();
#endif
      }
      if ( !newGrp->_is_nil() ) {
#ifdef WITHGENERICOBJ
        newGrp->UnRegister();
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
