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

// File   : SMESHGUI_MeshOrderOp.cxx
// Author : Pavel TELKOV, Open CASCADE S.A.S.
//
#include "SMESHGUI_MeshOrderOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDSClient_SObject.hxx>

// STL includes
#include <set>

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_MeshOrderOp::SMESHGUI_MeshOrderOp()
  : SMESHGUI_Operation(), myDlg(0), myMgr(0)
{
  myDlg = new SMESHGUI_MeshOrderDlg( desktop() );
  
  myHelpFileName = "constructing_meshes_page.html#mesh_order_anchor";
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_MeshOrderOp::~SMESHGUI_MeshOrderOp()
{
}

//================================================================================
/*!
 * \brief Return operation dialog
 */
//================================================================================

LightApp_Dialog* SMESHGUI_MeshOrderOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief perform it's intention action: compute 2D mesh on 3D
 */
//================================================================================

void SMESHGUI_MeshOrderOp::startOperation()
{
  SMESHGUI_Operation::startOperation();
  if (myMgr)
    myDlg->show();
}

//================================================================================
/*!
 * \brief Init dialog and mesh order box
 */
//================================================================================

void SMESHGUI_MeshOrderOp::initDialog()
{
  if (!myDlg )
    return;
  
  SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_nil();
  // check selection
  LightApp_SelectionMgr *Sel = selectionMgr();
  SALOME_ListIO selected; Sel->selectedObjects( selected );

  if (selected.Extent() == 1)
    aMesh = SMESH::GetMeshByIO(selected.First());
  if (aMesh->_is_nil()) {
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_NO_AVAILABLE_DATA"));
    onCancel();
    return;
  }

  myMgr = new SMESHGUI_MeshOrderMgr( myDlg->GetMeshOrderBox() );
  myMgr->SetMesh( aMesh );
  if ( !myMgr->GetMeshOrder() ) {
    SUIT_MessageBox::information(desktop(),
                             tr("SMESH_INFORMATION"),
                             tr("SMESH_NO_CONCURENT_MESH"));
    
    onCancel();
    return;
  }
}

//================================================================================
/*!
 * \brief Apply changes
 */
//================================================================================

bool SMESHGUI_MeshOrderOp::onApply()
{
  SUIT_OverrideCursor aWaitCursor;
  bool res = myMgr ? myMgr->SetMeshOrder() : false;

  if( res )
    SMESHGUI::Modified();

  delete myMgr;
  myMgr = 0;

  return res;
}

//================================================================================
/*!
 * \brief Apply changes
 */
//================================================================================

void SMESHGUI_MeshOrderOp::onCancel()
{
  delete myMgr;
  myMgr = 0;

  abort();
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_MeshOrderMgr::SMESHGUI_MeshOrderMgr( SMESHGUI_MeshOrderBox* theBox )
: myBox( theBox )
{
  myMesh = SMESH::SMESH_Mesh::_nil();
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_MeshOrderMgr::~SMESHGUI_MeshOrderMgr()
{
}

//================================================================================
/*!
 * \brief Set root mesh object
 */
//================================================================================

void SMESHGUI_MeshOrderMgr::SetMesh(SMESH::SMESH_Mesh_var& theMesh)
{
  myMesh = SMESH::SMESH_Mesh::_duplicate(theMesh);
  _PTR(SObject) aMeshSObj = SMESH::FindSObject(theMesh);
  if ( myBox && aMeshSObj )
    myBox->setTitle( aMeshSObj->GetName().c_str() );
}  

//================================================================================
/*!
 * \brief Check for concurents between submesh objects
 */
//================================================================================

bool SMESHGUI_MeshOrderMgr::GetMeshOrder()
{
  ListListId   idListList;
  return GetMeshOrder(idListList);
}

//================================================================================
/*!
 * \brief Check for concurents between submesh objects
 */
//================================================================================

bool SMESHGUI_MeshOrderMgr::GetMeshOrder(ListListId& theIdListList)
{
  if (!myBox || myMesh->_is_nil())
    return false;
  myBox->Clear();
  SMESH::submesh_array_array_var meshOrder = myMesh->GetMeshOrder();
  if ( !meshOrder.operator->() || !meshOrder->length() )
    return false;
  ListListName nameListList;
  for ( int i = 0, n = meshOrder->length(); i < n; i++ )
  {
    QList<int> idList;
    QStringList nameList;
    const SMESH::submesh_array& aSMArray = meshOrder[i];
    for ( int j = 0, jn = aSMArray.length(); j < jn; j++ )
    {
      const SMESH::SMESH_subMesh_var subMesh = aSMArray[j];
      
      _PTR(SObject) aSubMeshSObj = SMESH::FindSObject(subMesh);
      if ( !aSubMeshSObj )
        continue;

      idList.append(subMesh->GetId() );
      nameList.append( QString(aSubMeshSObj->GetName().c_str()) );
    }
    theIdListList.append(idList);
    nameListList.append(nameList);
  }
  myBox->SetMeshes(nameListList, theIdListList);
  return !theIdListList.isEmpty();
}

//================================================================================
/*!
 * \brief Returns status is order changed by user
 */
//================================================================================

bool SMESHGUI_MeshOrderMgr::IsOrderChanged() const
{
  return myBox && myBox->IsOrderChanged();
}

//================================================================================
/*!
 * \brief Store submesh priority order
 */
//================================================================================

bool SMESHGUI_MeshOrderMgr::SetMeshOrder()
{
  return myBox ? SetMeshOrder(myBox->GetMeshIds()) : false;
}

//================================================================================
/*!
 * \brief Store submesh priority order
 */
//================================================================================

bool SMESHGUI_MeshOrderMgr::SetMeshOrder( const  ListListId& theListListIds )
{
  if (theListListIds.isEmpty() || myMesh->_is_nil())
    return false;

  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
  _PTR(SObject) aMeshSObj = SMESH::FindSObject(myMesh);
  if ( !aStudy || !aMeshSObj )
    return false;

  std::map<int, SMESH::SMESH_subMesh_var> mapOfSubMesh;
  for (int i = SMESH::Tag_FirstSubMesh; i <= SMESH::Tag_LastSubMesh; i++) {
    _PTR(SObject) aSubmeshRoot;
    if ( !aMeshSObj->FindSubObject( i, aSubmeshRoot ) )
      continue;
    _PTR(ChildIterator) smIter = aStudy->NewChildIterator( aSubmeshRoot );
    for ( ; smIter->More(); smIter->Next() ) {
      _PTR(SObject) aSmObj = smIter->Value();
      SMESH::SMESH_subMesh_var sm =
        SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( aSmObj );
      if ( !sm->_is_nil() )
        mapOfSubMesh[ sm->GetId() ] = SMESH::SMESH_subMesh::_duplicate(sm);
    }
  }

  // is it enought to set modifid attribute on root mesh objects only?
  //  it is seems that modifaction flag will be set on child submeshes 
  //  automatically  (see SMESH::ModifiedMesh for details)
  SMESH::ModifiedMesh( aMeshSObj, false, false );

  SMESH::submesh_array_array_var meshOrder = new SMESH::submesh_array_array();
  meshOrder->length(theListListIds.count() );
  ListListId::const_iterator it = theListListIds.constBegin();
  for ( int i = 0; it != theListListIds.constEnd(); ++it ) {
    const QList<int>& ids = *it;
    SMESH::submesh_array_var subMeshList = new SMESH::submesh_array();
    subMeshList->length( ids.count() );
    QList<int>::const_iterator subIt = ids.constBegin();
    for( int j = 0; subIt != ids.constEnd(); ++subIt )
      if ( mapOfSubMesh.find( *subIt ) != mapOfSubMesh.end() )
        subMeshList[ j++ ] = mapOfSubMesh[ *subIt ];

    meshOrder[ i++ ] = subMeshList;
  }
  // update object browser
  SMESHGUI::GetSMESHGUI()->updateObjBrowser( true, 0 );

  return myMesh->SetMeshOrder(meshOrder);
}
