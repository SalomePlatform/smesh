//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  CEA
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
// File      : SMESHGUI_GroupOnShapeDlg.cxx
// Created   : Wed Sep 17 18:36:51 2008
// Author    : Edward AGAPOV (eap)
// Module    : SMESH


// IDL Headers
#include "SMESHGUI_GroupOnShapeDlg.h"

#include "SMESH_TypeFilter.hxx"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include <GeometryGUI.h>
#include <GEOM_SelectionFilter.h>

#include <SUIT_Session.h>
#include <SUIT_OverrideCursor.h>
#include <LightApp_UpdateFlags.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

#include <qgroupbox.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>

enum { _MESH, _ELEM_GEOM, _NODE_GEOM };

SMESHGUI_GroupOnShapeDlg::SMESHGUI_GroupOnShapeDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  QPixmap image (resourceMgr()->loadPixmap("SMESH", tr("ICON_SELECT")));

  QGroupBox* aGrp = new QGroupBox( 3, Qt::Horizontal, mainFrame() );
  aGrp->setFrameStyle( QFrame::NoFrame );
  aGrp->setInsideMargin( 0 );

  // Name
  new QLabel( tr( "SMESH_NAME" ), aGrp ); new QLabel( "", aGrp );
  myGrpNameLine = new QLineEdit( aGrp, "myGrpNameLine" );

  // Mesh
  new QLabel( tr( "SMESH_OBJECT_MESH" ), aGrp );
  myMeshBtn = new QPushButton( aGrp, "myMeshBtn");
  myMeshBtn->setToggleButton(true);
  myMeshBtn->setPixmap(image);
  myMeshLine = new QLineEdit( aGrp, "myMeshLine" );
  myMeshLine->setReadOnly(true);

  // Elem geom

  QGroupBox* elemsGrp = new QGroupBox( 1, Qt::Horizontal, tr( "SMESH_ELEMENTS" ), mainFrame() );
  QFrame* elemFr = new QFrame(elemsGrp, "elemFr");

  QLabel* label = new QLabel( tr( "SMESH_OBJECT_GEOM" ), elemFr );
  myElemGeomBtn = new QPushButton( elemFr, "myElemGeomBtn");
  myElemGeomBtn->setToggleButton(true);
  myElemGeomBtn->setPixmap(image);
  myElemGeomList = new QListBox( elemFr, "myElemGeomList" );
  myElemGeomList->setSelectionMode(QListBox::NoSelection);

  QGridLayout* elemLay = new QGridLayout(elemFr, 2, 3);
  elemLay->setSpacing(6);
  elemLay->setAutoAdd(false);
  elemLay->setRowStretch( 1, 1 );
  elemLay->addWidget( label, 0, 0 );
  elemLay->addWidget( myElemGeomBtn, 0, 1 );
  elemLay->addMultiCellWidget(myElemGeomList, 0, 1, 2, 2);

  // Node geom

  QGroupBox* nodesGrp = new QGroupBox( 1, Qt::Horizontal, tr( "SMESH_NODES" ), mainFrame() );
  QFrame* nodeFr = new QFrame(nodesGrp, "nodeFr");

  label = new QLabel( tr( "SMESH_OBJECT_GEOM" ), nodeFr );
  myNodeGeomBtn = new QPushButton( nodeFr, "myNodeGeomBtn");
  myNodeGeomBtn->setToggleButton(true);
  myNodeGeomBtn->setPixmap(image);
  myNodeGeomList = new QListBox( nodeFr, "myNodeGeomList" );
  myNodeGeomList->setSelectionMode(QListBox::NoSelection);

  QGridLayout* nodeLay = new QGridLayout(nodeFr, 2, 3);
  nodeLay->setSpacing(6);
  nodeLay->setAutoAdd(false);
  nodeLay->setRowStretch( 1, 1 );
  nodeLay->addWidget( label, 0, 0 );
  nodeLay->addWidget( myNodeGeomBtn, 0, 1 );
  nodeLay->addMultiCellWidget(myNodeGeomList, 0, 1, 2, 2);

  // Fill layout
  QVBoxLayout* aLay = new QVBoxLayout( mainFrame(), 0, 5 );
  aLay->addWidget( aGrp );
  aLay->addWidget( elemsGrp );
  aLay->addWidget( nodesGrp );

  setCaption( tr( "SMESH_CREATE_GROUP_FROM_GEOM" ) );
}

//================================================================================
/*!
 * \brief slot to enable/diable [Apply]
 */
//================================================================================

void SMESHGUI_GroupOnShapeDlg::updateButtons()
{
  bool enable =
    !myGrpNameLine->text().isEmpty() && myElemGeomList->count() + myNodeGeomList->count();

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
  myGrpNameLine->setText("");

  myMeshBtn->setOn( true );
  myMeshLine->setText("");

  myElemGeomBtn->setOn(false);
  myElemGeomBtn->setEnabled(false);
  myElemGeomList->clear();
  myNodeGeomBtn->setOn(false);
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
  myHelpFileName = "creating_groups_page.html";
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

static SMESH::ElementType elementType(GEOM::GEOM_Object_var& geom)
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
    GEOM::GEOM_IShapesOperations_var aShapeOp =
      SMESH::GetGEOMGen()->GetIShapesOperations(aStudy->StudyId());

    if ( geom->GetType() == 37 ) { // geom group
      GEOM::GEOM_IGroupOperations_var  aGroupOp =
        SMESH::GetGEOMGen()->GetIGroupOperations(aStudy->StudyId());
      if ( !aGroupOp->_is_nil() ) {
        GEOM::GEOM_Object_var mainShape = aGroupOp->GetMainShape( geom );
        GEOM::ListOfLong_var        ids = aGroupOp->GetObjects( geom );
        if ( ids->length() && !mainShape->_is_nil() && !aShapeOp->_is_nil() ) {
          GEOM::GEOM_Object_var member = aShapeOp->GetSubShape( mainShape, ids[0] );
          return elementType( member );
        }
      }
    }
    else if ( !aShapeOp->_is_nil() ) { // just a compoud shape
      GEOM::ListOfLong_var ids = aShapeOp->SubShapeAllIDs( geom, GEOM::SHAPE, false );
      if ( ids->length() ) {
        GEOM::GEOM_Object_var member = aShapeOp->GetSubShape( geom, ids[0] );
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
    connect(myDlg->myGrpNameLine, SIGNAL(textChanged(const QString&)),myDlg,SLOT(updateButtons()));
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
  _PTR(SObject)       meshSO = aStudy->FindObjectID( myMeshID );
  SMESH::SMESH_Mesh_var mesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( meshSO );
  if ( mesh->_is_nil() ) return false;

  // names of all existing groups
  SMESH::ListOfGroups_var groups = mesh->GetGroups();
  QStringList groupNames;
  for ( int i = 0; i < groups->length(); ++i ) {
    CORBA::String_var name = groups[i]->GetName();
    groupNames.append( name.in() );
  }

  // create groups
  SMESH::SMESH_GroupOnGeom_var group;
  for ( int isNode = 0; isNode < 2; ++isNode ) // elems and then nodes
  {
    QStringList::iterator geomID = isNode ? myNodeGeoIDs.begin() : myElemGeoIDs.begin();
    QStringList::iterator geomEnd = isNode ? myNodeGeoIDs.end() : myElemGeoIDs.end();

    for ( ; geomID != geomEnd; ++geomID )
    {
      // selected geom
      _PTR(SObject) geomSO = aStudy->FindObjectID( *geomID );
      GEOM::GEOM_Object_var geom = SMESH::SObjectToInterface<GEOM::GEOM_Object>(geomSO);
      if ( geom->_is_nil() ) continue;

      // group type
      SMESH::ElementType elemType = isNode ? SMESH::NODE : elementType( geom );
      if ( elemType == SMESH::ALL )
        continue;

      // make a unique name
      int nb = 1;
      QString name = myDlg->myGrpNameLine->text() + "_" + QString::number(nb);
      while ( groupNames.contains( name ))
        name = myDlg->myGrpNameLine->text() + "_" + QString::number(++nb);
      groupNames.append( name );

      //printf( "apply() %s %s\n", (*geomID).latin1(), name.latin1() );
      group = mesh->CreateGroupFromGEOM( elemType, name, geom );
    }
  }
  update( UF_ObjBrowser | UF_Model );

  init();

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

  if ( sender() == myDlg->myMeshBtn && myDlg->myMeshBtn->isOn() )
  {
    myDlg->myElemGeomBtn->setOn(false);
    myDlg->myNodeGeomBtn->setOn(false);
    onActivateObject( _MESH ); // install filter
  }
  else if ( sender() == myDlg->myElemGeomBtn && myDlg->myElemGeomBtn->isOn() )
  {
    myDlg->myMeshBtn->setOn(false);
    myDlg->myNodeGeomBtn->setOn(false);
    onActivateObject( _ELEM_GEOM ); // install filter
  }
  else if ( sender() == myDlg->myNodeGeomBtn && myDlg->myNodeGeomBtn->isOn() )
  {
    myDlg->myMeshBtn->setOn(false);
    myDlg->myElemGeomBtn->setOn(false);
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
    return new SMESH_TypeFilter( MESH );
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

  if ( myDlg->myMeshBtn->isOn() ) // mesh selected
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
    return;
  }

  // Filter off inappropriate shapes

  QStringList goodNames, goodIds;
  if (nbSelected > 0) {
    // study
    if (_PTR(Study) aStudy = SMESH::GetActiveStudyDocument()) {
      // mesh
      if (_PTR(SObject)  meshSO = aStudy->FindObjectID( myMeshID )) {
        // shape to mesh
        _PTR(SObject) anObj, shapeToMesh;
        if (meshSO->FindSubObject(1, anObj) && anObj->ReferencedObject(shapeToMesh)) {
          // loop on selected
          QStringList::iterator name = names.begin(), id = ids.begin(), idEnd = ids.end();
          for (; id != idEnd; ++id, ++name ) {
            // shape SO
            if (_PTR(SObject) shapeSO = aStudy->FindObjectID( *id )) {
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

  if ( myDlg->myElemGeomBtn->isOn() ) // elem geomerty selection
  {
    myDlg->myElemGeomList->clear();
    myDlg->myElemGeomList->insertStringList( goodNames );
    myElemGeoIDs = goodIds;
  }
  else if ( myDlg->myNodeGeomBtn->isOn() ) // Node geomerty selection
  {
    myDlg->myNodeGeomList->clear();
    myDlg->myNodeGeomList->insertStringList( goodNames );
    myNodeGeoIDs = goodIds;
  }

  // enable/diable Apply, which can change at selection
  myDlg->updateButtons();
}
