// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File   : SMESHGUI_Make2DFrom3D.cxx
// Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#include "SMESHGUI_Make2DFrom3DOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"

// SALOME GUI includes
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Session.h>
#include <SVTK_ViewModel.h>
//#include <SVTK_ViewWindow.h>
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

#include <Standard_ErrorHandler.hxx>

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
  //my1dFrom3dRB = new QRadioButton( tr( "1D_FROM_3D" ), aModeGrp );
  aModeGrpLayout->addWidget( my2dFrom3dRB );
  aModeGrpLayout->addWidget( my1dFrom2dRB );
  //aModeGrpLayout->addWidget( my1dFrom3dRB );

//   // Groups of mesh faces
//   setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
//   createObject( tr( "Groups" ), mainFrame(), Groups );
//   setNameIndication( Groups, ListOfNames );
//   objectWg( Groups, Btn )->hide();

  // Mesh or Groups
  //setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "Groups" ), mainFrame(), MeshOrGroups );
  setNameIndication( MeshOrGroups, ListOfNames );
  objectWg( MeshOrGroups, Btn )->hide();

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
  aDlgLay->addWidget( objectWg( MeshOrGroups,  Label ),   1, 0 );
  aDlgLay->addWidget( objectWg( MeshOrGroups,  Control ), 1, 1 );
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

void SMESHGUI_Make2DFrom3DDlg::setNewMeshEnabled( bool enable )
{
  if ( !enable )
    myThisMeshRB->setChecked( true );

  myNewMeshRB->setEnabled( enable );

  onTargetChanged();
}

bool SMESHGUI_Make2DFrom3DDlg::getNewMeshEnabled() const
{
  return myNewMeshRB->isEnabled();
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
  : SMESHGUI_SelectionOp(),
    myMeshFilter(SMESH::MESH),
    myGroupFilter(SMESH::GROUP)
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
  //connect( myDlg->my1dFrom3dRB, SIGNAL( toggled(bool) ), this, SLOT( onModeChanged() ) );

  //onModeChanged();

  myDlg->activateObject( SMESHGUI_Make2DFrom3DDlg::MeshOrGroups );
  selectionDone();
}

//================================================================================
/*!
 * \brief Set filter corresponding to dimension
 */
//================================================================================

void SMESHGUI_Make2DFrom3DOp::onModeChanged()
{
//   QRadioButton* b = dynamic_cast<QRadioButton*>( sender());
//   if ( b && !b->isChecked() )
//     return;

//   // enable "2D groups" field
//   bool enableGroups = ( myDlg->mode() == SMESH::BND_1DFROM3D );
//   myDlg->setObjectEnabled( SMESHGUI_Make2DFrom3DDlg::Groups, enableGroups );
//   ((QToolButton*) myDlg->objectWg( SMESHGUI_Make2DFrom3DDlg::Groups,
//                                    SMESHGUI_Make2DFrom3DDlg::Btn ))->setChecked( enableGroups );
  
//   // install filter
//   int id =  enableGroups ? SMESHGUI_Make2DFrom3DDlg::Groups : SMESHGUI_Make2DFrom3DDlg::Mesh;
//   onDeactivateObject( SMESHGUI_Make2DFrom3DDlg::MeshOrGroups );
//   onActivateObject  ( SMESHGUI_Make2DFrom3DDlg::MeshOrGroups );
//   selectionDone();
}

void SMESHGUI_Make2DFrom3DOp::selectionDone()
{
  myDlg->clearSelection( SMESHGUI_Make2DFrom3DDlg::MeshOrGroups );
  mySrcMesh = SMESH::SMESH_Mesh::_nil();

  if ( !dlg() ) return;

  
  if ( dlg()->isVisible() ) {
    try {
      QStringList names, ids;
      LightApp_Dialog::TypesList types;
      selected( names, types, ids );
      for ( int i = 0; i < names.count(); ++i )
        names[i] = names[i].trimmed();
      myDlg->selectObject( names, types, ids );

      // enable/desable "new mesh" button
      bool isMesh = true;
      for ( int i = 0; i < ids.count() && isMesh; ++i )
      {
        _PTR(SObject) sobj =
          SMESHGUI::activeStudy()->studyDS()->FindObjectID( ids[i].toLatin1().constData() );
        mySrcMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( sobj );  
        //isMesh = !mySrcMesh->_is_nil(); // EAP - it's sometimes necessary to copy to a new mesh
      }
      myDlg->setNewMeshEnabled( isMesh );
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
  SMESHGUI_Make2DFrom3DOp* me = (SMESHGUI_Make2DFrom3DOp*) this;

  QList<SUIT_SelectionFilter*> subFilters;
  subFilters.append( & me->myMeshFilter );
  subFilters.append( & me->myGroupFilter );

  SUIT_SelectionFilter* f = new SMESH_LogicalFilter( subFilters, SMESH_LogicalFilter::LO_OR );
  return f;
}

bool SMESHGUI_Make2DFrom3DOp::isValid( QString& msg ) const
{
  if ( !dlg() ) return false;

  // check if a mesh is selected
  if ( !myDlg->hasSelection( SMESHGUI_Make2DFrom3DDlg::MeshOrGroups ))
  {
    msg = tr( "SMESH_ERR_NO_INPUT_MESH" );
    return false;
  }
  QStringList entries;
  dlg()->selectedObject( SMESHGUI_Make2DFrom3DDlg::MeshOrGroups, entries );
  const bool isMeshSelected = ( !mySrcMesh->_is_nil() );
  if ( isMeshSelected )
  {
    // only one mesh is allowed
    if ( entries.size() > 1 ) {
      msg = tr( "SMESH_TOO_MANY_MESHES" );
      return false;
    }
  }
  else
  {
    // check if only groups are selected
    for ( int i = 0; i < entries.count(); ++i )
    {
      SMESH::SMESH_GroupBase_var grp;
      if ( _PTR(SObject) sobj = SMESHGUI::activeStudy()->studyDS()->FindObjectID( entries[i].toLatin1().constData() ))
        grp = SMESH::SObjectToInterface<SMESH::SMESH_GroupBase>( sobj );
      if ( grp->_is_nil() ) {
        msg = tr( "SMESH_NOT_ONLY_GROUPS" );
        return false;
      }
    }
  }
  // check if the selected objects contains elements of required type
  bool hasFaces = false, hasVolumes = false;
  SMESH::Bnd_Dimension mode = myDlg->mode();
  for ( int i = 0; i < entries.count(); ++i )
  {
    SMESH::SMESH_IDSource_var idSource;
    if ( _PTR(SObject) sobj = SMESHGUI::activeStudy()->studyDS()->FindObjectID( entries[i].toLatin1().constData() ))
      idSource = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );
    if ( !idSource->_is_nil() ) {
      SMESH::array_of_ElementType_var types = idSource->GetTypes();
      for ( int j = 0; j < (int) types->length(); ++j )
        if ( types[j] == SMESH::VOLUME )
          hasVolumes = true;
        else if ( types[j] == SMESH::FACE )
          hasFaces = true;
    }
  }
  if ( mode == SMESH::BND_2DFROM3D && !hasVolumes ) {
    msg = tr( "SMESH_ERR_NO_3D_ELEMENTS" );
    return false;
  }
  else if ( mode == SMESH::BND_1DFROM2D && !hasFaces  ) {
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

bool SMESHGUI_Make2DFrom3DOp::compute2DMesh( QStringList& theEntryList )
{
  SUIT_OverrideCursor wc;

  bool ok = false;
  try {
    SMESH::Bnd_Dimension mode = myDlg->mode();
    QString meshName          = myDlg->needNewMesh() ? myDlg->getNewMeshName() : QString();
    QString groupName         = myDlg->needGroup()   ? myDlg->getGroupName()   : QString();
    bool copyAll              = myDlg->copySource();

    QStringList entries;
    dlg()->selectedObject( SMESHGUI_Make2DFrom3DDlg::MeshOrGroups, entries );
    SMESH::ListOfIDSources_var groups = new SMESH::ListOfIDSources;
    QString wrongGroups = "";

    if ( mySrcMesh->_is_nil() ) // get selected groups, find groups of wrong type
    {
      int nbGroups = 0;
      int goodType = ( mode == SMESH::BND_2DFROM3D ? SMESH::VOLUME : SMESH::FACE );
      groups->length( entries.count() );
      for ( int i = 0; i < entries.count(); ++i )
      {
        _PTR(SObject) sobj =
          SMESHGUI::activeStudy()->studyDS()->FindObjectID( entries[i].toLatin1().constData() );
        SMESH::SMESH_IDSource_var grp = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );  
        SMESH::array_of_ElementType_var types = grp->GetTypes();
        if ( types->length() < 1 || types[0] != goodType )
        {
          if ( !wrongGroups.isEmpty() )
            wrongGroups += ", ";
          wrongGroups += sobj->GetName().c_str();
        }
        else
        {
          groups[ nbGroups++ ] = grp;
        }
      }
      groups->length( nbGroups );
      mySrcMesh = groups[0]->GetMesh();
    }

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
      QString msg = tr("NB_ADDED").arg( nbAdded );
      if ( !wrongGroups.isEmpty() )
        msg += ".\n" + tr("WRONG_GROUPS").arg( wrongGroups );
      SUIT_MessageBox::information( myDlg, tr("SMESH_INFORMATION"), msg);

      if ( !newMesh->_is_nil() ) {
        if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( newMesh ) )
          theEntryList.append( aSObject->GetID().c_str() );
      }
      ok = true;

      for ( int i = 0; i < entries.count(); ++i )
        if ( SMESH_Actor* actor = SMESH::FindActorByEntry( entries[i].toLatin1().constData() ))
          SMESH::Update(actor->getIO(),actor->GetVisibility());
      SMESH::RepaintCurrentView();
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

  QStringList anEntryList;
  bool res = false;
  try {
    res = compute2DMesh( anEntryList );
  }
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch ( ... ) {
  }

  if ( res ) {
    SMESHGUI::Modified();
    update( UF_ObjBrowser | UF_Model );
    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      anApp->browseObjects( anEntryList, isApplyAndClose() );
    myDlg->setNewMeshName( SMESH::UniqueName( "Mesh_1" ) );
    myDlg->setGroupName( SMESH::UniqueName( "Group" ) );
  }
  else {
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_ERROR" ), tr( "SMESH_OPERATION_FAILED" ) );
  }

  return res;
}
