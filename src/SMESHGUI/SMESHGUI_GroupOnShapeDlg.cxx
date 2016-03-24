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

//  SMESH SMESHGUI : GUI for SMESH component
// File      : SMESHGUI_GroupOnShapeDlg.cxx
// Created   : Wed Sep 17 18:36:51 2008
// Author    : Edward AGAPOV (eap)
// Module    : SMESH
// IDL Headers
//
#include "SMESHGUI_GroupOnShapeDlg.h"

#include "SMESH_TypeFilter.hxx"
#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include <GeometryGUI.h>
#include <GEOM_SelectionFilter.h>
#include <GEOM_wrap.hxx>

#include <SUIT_Session.h>
#include <SUIT_OverrideCursor.h>
#include <LightApp_Application.h>
#include <LightApp_UpdateFlags.h>
#include <SUIT_ResourceMgr.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

#include <QGroupBox>
#include <QLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>

#define SPACING 6
#define MARGIN  11

enum { _MESH, _ELEM_GEOM, _NODE_GEOM };

SMESHGUI_GroupOnShapeDlg::SMESHGUI_GroupOnShapeDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  QPixmap image (resourceMgr()->loadPixmap("SMESH", tr("ICON_SELECT")));

  // Name
  //QLabel* nameLabel = new QLabel( tr( "SMESH_NAME" ), mainFrame() );
  //myGrpNameLine = new QLineEdit( mainFrame() );

  // Mesh
  QLabel* meshLabel = new QLabel( tr( "SMESH_OBJECT_MESH" ), mainFrame() );
  myMeshBtn = new QPushButton( mainFrame() );
  myMeshBtn->setCheckable(true);
  myMeshBtn->setIcon(image);
  myMeshLine = new QLineEdit( mainFrame() );
  myMeshLine->setReadOnly(true);

  // Elem geom

  QGroupBox* elemsGrp = new QGroupBox( tr( "SMESH_ELEMENTS" ), mainFrame() );
  QLabel* label = new QLabel( tr( "SMESH_GEOM" ), elemsGrp );
  myElemGeomBtn = new QPushButton( elemsGrp );
  myElemGeomBtn->setCheckable(true);
  myElemGeomBtn->setIcon(image);
  myElemGeomList = new QListWidget( elemsGrp );
  myElemGeomList->setSelectionMode(QListWidget::NoSelection);

  QGridLayout* elemLay = new QGridLayout(elemsGrp);
  elemLay->setSpacing( SPACING );
  elemLay->setMargin( MARGIN );
  elemLay->setRowStretch( 1, 1 );
  elemLay->addWidget( label, 0, 0 );
  elemLay->addWidget( myElemGeomBtn, 0, 1 );
  elemLay->addWidget( myElemGeomList, 0, 2, 2, 1);

  // Node geom

  QGroupBox* nodesGrp = new QGroupBox( tr( "SMESH_NODES" ), mainFrame() );

  label = new QLabel( tr( "SMESH_GEOM" ), nodesGrp );
  myNodeGeomBtn = new QPushButton( nodesGrp );
  myNodeGeomBtn->setCheckable(true);
  myNodeGeomBtn->setIcon(image);
  myNodeGeomList = new QListWidget( nodesGrp );
  myNodeGeomList->setSelectionMode(QListWidget::NoSelection);

  QGridLayout* nodeLay = new QGridLayout(nodesGrp);
  nodeLay->setSpacing( SPACING );
  nodeLay->setMargin( MARGIN );
  nodeLay->setRowStretch( 1, 1 );
  nodeLay->addWidget( label, 0, 0 );
  nodeLay->addWidget( myNodeGeomBtn, 0, 1 );
  nodeLay->addWidget(myNodeGeomList, 0, 2, 2, 1);

  // Fill layout
  QGridLayout* aLay = new QGridLayout( mainFrame());
  aLay->setSpacing( SPACING );
  aLay->setMargin( MARGIN );
  //aLay->addWidget( nameLabel, 0, 0 );
  //aLay->addWidget( myGrpNameLine, 0, 2 );
  aLay->addWidget( meshLabel, 1, 0 );
  aLay->addWidget( myMeshBtn, 1, 1 );
  aLay->addWidget( myMeshLine,1, 2 );
  aLay->addWidget( elemsGrp, 2, 1, 1, 3 );
  aLay->addWidget( nodesGrp, 3, 1, 1, 3 );

  setWindowTitle( tr( "SMESH_CREATE_GROUP_FROM_GEOM" ) );
}

SMESHGUI_GroupOnShapeDlg::~SMESHGUI_GroupOnShapeDlg()
{
}

//================================================================================
/*!
 * \brief slot to enable/diable [Apply]
 */
//================================================================================

void SMESHGUI_GroupOnShapeDlg::updateButtons()
{
  bool enable =
    /*!myGrpNameLine->text().isEmpty() &&*/ myElemGeomList->count() + myNodeGeomList->count();

  button(Apply)->setEnabled( enable );
  button(OK)->setEnabled( enable );
}

//================================================================================
/*!
 * \brief initialization
 */
//================================================================================

void SMESHGUI_GroupOnShapeDlg::init()
{
  //myGrpNameLine->setText("");

  myMeshBtn->setChecked( true );
  myMeshLine->setText("");

  myElemGeomBtn->setChecked(false);
  myElemGeomBtn->setEnabled(false);
  myElemGeomList->clear();
  myNodeGeomBtn->setChecked(false);
  myNodeGeomBtn->setEnabled(false);
  myNodeGeomList->clear();

  updateButtons();
}

//================================================================================
/*!
 * \brief operation constructor
 */
//================================================================================

SMESHGUI_GroupOnShapeOp::SMESHGUI_GroupOnShapeOp()
  : SMESHGUI_SelectionOp(ActorSelection),
    myDlg( 0 )
{
  myHelpFileName = "create_groups_from_geometry_page.html";
}

SMESHGUI_GroupOnShapeOp::~SMESHGUI_GroupOnShapeOp()
{
  if ( myDlg )
    delete myDlg;
}
//================================================================================
/*!
 * \brief Gets dialog of this operation
  * \retval LightApp_Dialog* - pointer to dialog of this operation
*/
//================================================================================

LightApp_Dialog* SMESHGUI_GroupOnShapeOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief return type of mesh group by geom object
 */
//================================================================================

static SMESH::ElementType elementType(GEOM::GEOM_Object_var geom)
{
  if ( !geom->_is_nil() ) {
    switch ( geom->GetShapeType() ) {
    case GEOM::VERTEX:   return SMESH::NODE;
    case GEOM::EDGE:     return SMESH::EDGE;
    case GEOM::WIRE:     return SMESH::EDGE;
    case GEOM::FACE:     return SMESH::FACE;
    case GEOM::SHELL:    return SMESH::FACE;
    case GEOM::SOLID:    return SMESH::VOLUME;
    case GEOM::COMPSOLID:return SMESH::VOLUME;
    case GEOM::COMPOUND: break;
    default:             return SMESH::ALL;
    }
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    GEOM::GEOM_IShapesOperations_wrap aShapeOp =
      SMESH::GetGEOMGen()->GetIShapesOperations(aStudy->StudyId());

    if ( geom->GetType() == 37 ) { // geom group
      GEOM::GEOM_IGroupOperations_wrap aGroupOp =
        SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());
      if ( !aGroupOp->_is_nil() ) {
        // mainShape is an existing servant => GEOM_Object_var not GEOM_Object_wrap
        GEOM::GEOM_Object_var mainShape = aGroupOp->GetMainShape( geom );
        GEOM::ListOfLong_var        ids = aGroupOp->GetObjects( geom );
        if ( ids->length() && !mainShape->_is_nil() && !aShapeOp->_is_nil() ) {
          GEOM::GEOM_Object_wrap member = aShapeOp->GetSubShape( mainShape, ids[0] );
          return elementType( member );
        }
      }
    }
    else if ( !aShapeOp->_is_nil() ) { // just a compoud shape
      GEOM::ListOfLong_var ids = aShapeOp->SubShapeAllIDs( geom, GEOM::SHAPE, false );
      if ( ids->length() ) {
        GEOM::GEOM_Object_wrap member = aShapeOp->GetSubShape( geom, ids[0] );
        return elementType( member );
      }
    }
  }
  return SMESH::ALL;
}

//================================================================================
/*!
 * \brief initialization
 */
//================================================================================

void SMESHGUI_GroupOnShapeOp::init()
{
  myMeshID="";
  myElemGeoIDs.clear();
  myNodeGeoIDs.clear();

  myDlg->init();
  removeCustomFilters();
  onActivateObject( _MESH ); // install filter
}

//================================================================================
/*!
 * \brief start operation
 */
//================================================================================

void SMESHGUI_GroupOnShapeOp::startOperation()
{
  if (!myDlg)
  {
    myDlg = new SMESHGUI_GroupOnShapeDlg();
    connect(myDlg->myMeshBtn,     SIGNAL(clicked()), this, SLOT(onButtonClick()));
    connect(myDlg->myElemGeomBtn, SIGNAL(clicked()), this, SLOT(onButtonClick()));
    connect(myDlg->myNodeGeomBtn, SIGNAL(clicked()), this, SLOT(onButtonClick()));
    //connect(myDlg->myGrpNameLine, SIGNAL(textChanged(const QString&)),myDlg,SLOT(updateButtons()));
  }
  SMESHGUI_SelectionOp::startOperation();

  init();
  myDlg->show();
}

//================================================================================
/*!
 * \brief create groups
 */
//================================================================================

bool SMESHGUI_GroupOnShapeOp::onApply()
{
  SUIT_OverrideCursor aWaitCursor;

  if (isStudyLocked())
    return false;

  // study
  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
  if ( !aStudy ) return false;

  // mesh
  _PTR(SObject) meshSO = aStudy->FindObjectID( myMeshID.toLatin1().data() );
  SMESH::SMESH_Mesh_var mesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( meshSO );
  if ( mesh->_is_nil() ) return false;

  // names of all existing groups
//   QStringList groupNames;
//   QString givenName = myDlg->myGrpNameLine->text();
//   if ( !givenName.isEmpty() ) {
//     SMESH::ListOfGroups_var groups = mesh->GetGroups();
//     for ( int i = 0; i < groups->length(); ++i ) {
//       CORBA::String_var name = groups[i]->GetName();
//       groupNames.append( name.in() );
//     }
//   }

  // create groups
  SMESH::SMESH_GroupOnGeom_var group;
  QStringList anEntryList;
  for ( int isNode = 0; isNode < 2; ++isNode ) // elems and then nodes
  {
    QStringList::iterator geomID = isNode ? myNodeGeoIDs.begin() : myElemGeoIDs.begin();
    QStringList::iterator geomEnd = isNode ? myNodeGeoIDs.end() : myElemGeoIDs.end();

    for ( int i = 0; geomID != geomEnd; ++geomID, ++i )
    {
      // selected geom
      _PTR(SObject) geomSO = aStudy->FindObjectID( geomID->toLatin1().data() );
      GEOM::GEOM_Object_var geom = SMESH::SObjectToInterface<GEOM::GEOM_Object>(geomSO);
      if ( geom->_is_nil() ) continue;

      // group type
      SMESH::ElementType elemType = isNode ? SMESH::NODE : elementType( geom );
      if ( elemType == SMESH::ALL )
        continue;

      // make a unique name
      QString name =
        isNode ? myDlg->myNodeGeomList->item(i)->text() : myDlg->myElemGeomList->item(i)->text();
//       int nb = 1;
//       QString name = myDlg->myGrpNameLine->text() + "_" + QString::number(nb);
//       while ( groupNames.contains( name ))
//         name = myDlg->myGrpNameLine->text() + "_" + QString::number(++nb);
//       groupNames.append( name );

      //printf( "apply() %s %s\n", (*geomID).latin1(), name.latin1() );
      group = mesh->CreateGroupFromGEOM( elemType, name.toLatin1().data(), geom );
      if( !group->_is_nil() )
        if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( group ) )
          anEntryList.append( aSObject->GetID().c_str() );
    }
  }
  SMESHGUI::Modified();

  update( UF_ObjBrowser | UF_Model );

  // Re-init controls to create the next group
  myElemGeoIDs.clear();
  myNodeGeoIDs.clear();
  removeCustomFilters();
  myDlg->myNodeGeomList->clear();
  myDlg->myElemGeomList->clear();
  myDlg->myElemGeomBtn->setChecked(false); 
  myDlg->myNodeGeomBtn->setChecked(false);
  myDlg->updateButtons();

  if( LightApp_Application* anApp =
      dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
    anApp->browseObjects( anEntryList, isApplyAndClose() );

  return !group->_is_nil();
}

//================================================================================
/*!
 * \brief slot connected to selection buttons
 */
//================================================================================

void SMESHGUI_GroupOnShapeOp::onButtonClick()
{
  removeCustomFilters();

  if ( sender() == myDlg->myMeshBtn && myDlg->myMeshBtn->isChecked() )
  {
    myDlg->myElemGeomBtn->setChecked(false);
    myDlg->myNodeGeomBtn->setChecked(false);
    onActivateObject( _MESH ); // install filter
  }
  else if ( sender() == myDlg->myElemGeomBtn && myDlg->myElemGeomBtn->isChecked() )
  {
    myDlg->myMeshBtn->setChecked(false);
    myDlg->myNodeGeomBtn->setChecked(false);
    onActivateObject( _ELEM_GEOM ); // install filter
  }
  else if ( sender() == myDlg->myNodeGeomBtn && myDlg->myNodeGeomBtn->isChecked() )
  {
    myDlg->myMeshBtn->setChecked(false);
    myDlg->myElemGeomBtn->setChecked(false);
    onActivateObject( _NODE_GEOM ); // install filter
  }
  //selectionDone();
}

//================================================================================
/*!
 * \brief Creates selection filter
  * \param theId - identifier of current selection widget
  * \retval SUIT_SelectionFilter* - pointer to the created filter or null
 *
 * Creates selection filter in accordance with identifier of current selection widget
 */
//================================================================================
SUIT_SelectionFilter* SMESHGUI_GroupOnShapeOp::createFilter( const int theId ) const
{
  if ( theId == _ELEM_GEOM || theId == _NODE_GEOM )
    return new GEOM_SelectionFilter( (SalomeApp_Study*)study(), true );
  else if ( theId == _MESH )
    return new SMESH_TypeFilter( SMESH::MESH );
  else
    return ( SUIT_SelectionFilter*) 0;
}
//================================================================================
/*!
 * \brief Updates dialog's look and feel
 *
 * Virtual method redefined from the base class updates dialog's look and feel
 */
//================================================================================
void SMESHGUI_GroupOnShapeOp::selectionDone()
{
  QStringList names, ids;
  LightApp_Dialog::TypesList types;
  selected( names, types, ids );
  int nbSelected = names.size();

  if ( myDlg->myMeshBtn->isChecked() ) // mesh selected
  {
    myDlg->myMeshLine->setText("");
    myMeshID = "";
    if ( nbSelected == 1 ) {
      myDlg->myMeshLine->setText( names.front() );
      myMeshID = ids.front();
    }
    myDlg->myElemGeomList->clear();
    myDlg->myElemGeomBtn->setEnabled( nbSelected == 1 );
    myDlg->myNodeGeomList->clear();
    myDlg->myNodeGeomBtn->setEnabled( nbSelected == 1 );
    myDlg->myElemGeomBtn->click();
    return;
  }

  // Filter off inappropriate shapes

  QStringList goodNames, goodIds;
  if (nbSelected > 0) {
    // study
    if (_PTR(Study) aStudy = SMESH::GetActiveStudyDocument()) {
      // mesh
      if (_PTR(SObject)  meshSO = aStudy->FindObjectID( myMeshID.toLatin1().data() )) {
        // shape to mesh
        _PTR(SObject) anObj, shapeToMesh;
        if (meshSO->FindSubObject(1, anObj) && anObj->ReferencedObject(shapeToMesh)) {
          // loop on selected
          QStringList::iterator name = names.begin(), id = ids.begin(), idEnd = ids.end();
          for (; id != idEnd; ++id, ++name ) {
            // shape SO
            if (_PTR(SObject) shapeSO = aStudy->FindObjectID( id->toLatin1().data() )) {
            // check if shape SO is a child of shape to mesh 
              while ( shapeSO && shapeSO->GetID() != shapeToMesh->GetID() )
                if  ( shapeSO->Depth() < 2 )
                  shapeSO.reset();
                else
                  shapeSO = shapeSO->GetFather();
              if ( shapeSO ) {
                //printf( "selectionDone() %s %s\n", (*id).latin1(), (*name).latin1() );
                if ( !goodIds.contains( *id )) {
                  goodIds.append( *id );
                  goodNames.append( *name );
                }
              }
            }
          }
        }
      }
    }
  }

  if ( myDlg->myElemGeomBtn->isChecked() ) // elem geometry selection
  {
    myDlg->myElemGeomList->clear();
    myDlg->myElemGeomList->addItems( goodNames );
    myElemGeoIDs = goodIds;
  }
  else if ( myDlg->myNodeGeomBtn->isChecked() ) // Node geometry selection
  {
    myDlg->myNodeGeomList->clear();
    myDlg->myNodeGeomList->addItems( goodNames );
    myNodeGeoIDs = goodIds;
  }

  // enable/diable Apply, which can change at selection
  myDlg->updateButtons();
}
