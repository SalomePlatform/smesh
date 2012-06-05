// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
//  File   : SMESH_MeshEditor_i.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH

#ifdef WNT
#define NOMINMAX
#endif

#include "SMESH_MeshEditor_i.hxx"

#include "SMDS_LinearEdge.hxx"
#include "SMDS_Mesh0DElement.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMDS_PolyhedralVolumeOfNodes.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "SMESH_subMesh_i.hxx"

#include "utilities.h"
#include "Utils_ExceptHandlers.hxx"
#include "Utils_CorbaException.hxx"

#include <BRepAdaptor_Surface.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Vec.hxx>

#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
#define NO_CAS_CATCH
#endif

#include <Standard_Failure.hxx>

#ifdef NO_CAS_CATCH
#include <Standard_ErrorHandler.hxx>
#endif

#include <sstream>
#include <limits>

#define cast2Node(elem) static_cast<const SMDS_MeshNode*>( elem )

using namespace std;
using SMESH::TPythonDump;
using SMESH::TVar;

namespace {

  //=============================================================================
  /*!
   * \brief Mesh to apply modifications for preview purposes
   */
  //=============================================================================

  struct TPreviewMesh: public SMESH_Mesh
  {
    SMDSAbs_ElementType myPreviewType; // type to show
    //!< Constructor
    TPreviewMesh(SMDSAbs_ElementType previewElements = SMDSAbs_All) {
      _isShapeToMesh = (_id =_studyId =_idDoc = 0);
      _myMeshDS  = new SMESHDS_Mesh( _id, true );
      myPreviewType = previewElements;
    }
    //!< Destructor
    virtual ~TPreviewMesh() { delete _myMeshDS; _myMeshDS = 0; }
    //!< Copy a set of elements
    void Copy(const TIDSortedElemSet & theElements,
              TIDSortedElemSet&        theCopyElements,
              SMDSAbs_ElementType      theSelectType = SMDSAbs_All,
              SMDSAbs_ElementType      theAvoidType = SMDSAbs_All)
    {
      // loop on theIDsOfElements
      TIDSortedElemSet::const_iterator eIt = theElements.begin();
      for ( ; eIt != theElements.end(); ++eIt )
      {
        const SMDS_MeshElement* anElem = *eIt;
        if ( !anElem ) continue;
        SMDSAbs_ElementType type = anElem->GetType();
        if ( type == theAvoidType ||
             ( theSelectType != SMDSAbs_All && type != theSelectType ))
          continue;
        const SMDS_MeshElement* anElemCopy;
        if ( type == SMDSAbs_Node)
          anElemCopy = Copy( cast2Node(anElem) );
        else
          anElemCopy = Copy( anElem );
        if ( anElemCopy )
          theCopyElements.insert( theCopyElements.end(), anElemCopy );
      }
    }
    //!< Copy an element
    SMDS_MeshElement* Copy( const SMDS_MeshElement* anElem )
    {
      // copy element nodes
      int anElemNbNodes = anElem->NbNodes();
      vector< int > anElemNodesID( anElemNbNodes ) ;
      SMDS_ElemIteratorPtr itElemNodes = anElem->nodesIterator();
      for ( int i = 0; itElemNodes->more(); i++)
      {
        const SMDS_MeshNode* anElemNode = cast2Node( itElemNodes->next() );
        Copy( anElemNode );
        anElemNodesID[i] = anElemNode->GetID();
      }

      // creates a corresponding element on copied nodes
      SMDS_MeshElement* anElemCopy = 0;
      if ( anElem->IsPoly() && anElem->GetType() == SMDSAbs_Volume )
      {
        const SMDS_VtkVolume* ph =
          dynamic_cast<const SMDS_VtkVolume*> (anElem);
        if ( ph )
          anElemCopy = _myMeshDS->AddPolyhedralVolumeWithID
            (anElemNodesID, ph->GetQuantities(),anElem->GetID());
      }
      else {
        anElemCopy = ::SMESH_MeshEditor(this).AddElement( anElemNodesID,
                                                          anElem->GetType(),
                                                          anElem->IsPoly() );
      }
      return anElemCopy;
    }
    //!< Copy a node
    SMDS_MeshNode* Copy( const SMDS_MeshNode* anElemNode )
    {
      return _myMeshDS->AddNodeWithID(anElemNode->X(), anElemNode->Y(), anElemNode->Z(),
                                      anElemNode->GetID());
    }
  };// struct TPreviewMesh

  static SMESH_NodeSearcher *    theNodeSearcher    = 0;
  static SMESH_ElementSearcher * theElementSearcher = 0;

  //=============================================================================
  /*!
   * \brief Deleter of theNodeSearcher at any compute event occured
   */
  //=============================================================================

  struct TSearchersDeleter : public SMESH_subMeshEventListener
  {
    SMESH_Mesh* myMesh;
    string      myMeshPartIOR;
    //!< Constructor
    TSearchersDeleter(): SMESH_subMeshEventListener( false, // won't be deleted by submesh
                                                     "SMESH_MeshEditor_i::TSearchersDeleter"),
                         myMesh(0) {}
    //!< Delete theNodeSearcher
    static void Delete()
    {
      if ( theNodeSearcher )    delete theNodeSearcher;    theNodeSearcher    = 0;
      if ( theElementSearcher ) delete theElementSearcher; theElementSearcher = 0;
    }
    typedef map < int, SMESH_subMesh * > TDependsOnMap;
    //!< The meshod called by submesh: do my main job
    void ProcessEvent(const int, const int eventType, SMESH_subMesh* sm,
                      SMESH_subMeshEventListenerData*,const SMESH_Hypothesis*)
    {
      if ( eventType == SMESH_subMesh::COMPUTE_EVENT ) {
        Delete();
        Unset( sm->GetFather() );
      }
    }
    //!< set self on all submeshes and delete theNodeSearcher if other mesh is set
    void Set(SMESH_Mesh* mesh, const string& meshPartIOR = string())
    {
      if ( myMesh != mesh || myMeshPartIOR != meshPartIOR)
      {
        if ( myMesh ) {
          Delete();
          Unset( myMesh );
        }
        myMesh = mesh;
        myMeshPartIOR = meshPartIOR;
        if ( SMESH_subMesh* myMainSubMesh = mesh->GetSubMeshContaining(1) ) {
          const TDependsOnMap & subMeshes = myMainSubMesh->DependsOn();
          TDependsOnMap::const_iterator sm;
          for (sm = subMeshes.begin(); sm != subMeshes.end(); sm++)
            sm->second->SetEventListener( this, 0, sm->second );
        }
      }
    }
    //!<  delete self from all submeshes
    void Unset(SMESH_Mesh* mesh)
    {
      if ( SMESH_subMesh* myMainSubMesh = mesh->GetSubMeshContaining(1) ) {
        const TDependsOnMap & subMeshes = myMainSubMesh->DependsOn();
        TDependsOnMap::const_iterator sm;
        for (sm = subMeshes.begin(); sm != subMeshes.end(); sm++)
          sm->second->DeleteEventListener( this );
      }
      myMesh = 0;
    }

  } theSearchersDeleter;

  TCollection_AsciiString mirrorTypeName( SMESH::SMESH_MeshEditor::MirrorType theMirrorType )
  {
    TCollection_AsciiString typeStr;
    switch ( theMirrorType ) {
    case  SMESH::SMESH_MeshEditor::POINT:
      typeStr = "SMESH.SMESH_MeshEditor.POINT";
      break;
    case  SMESH::SMESH_MeshEditor::AXIS:
      typeStr = "SMESH.SMESH_MeshEditor.AXIS";
      break;
    default:
      typeStr = "SMESH.SMESH_MeshEditor.PLANE";
    }
    return typeStr;
  }
  //================================================================================
  /*!
   * \brief function for conversion of long_array to TIDSortedElemSet
   * \param IDs - array of IDs
   * \param aMesh - mesh
   * \param aMap - collection to fill
   * \param aType - element type
   */
  //================================================================================

  void arrayToSet(const SMESH::long_array & IDs,
                  const SMESHDS_Mesh*       aMesh,
                  TIDSortedElemSet&         aMap,
                  const SMDSAbs_ElementType aType = SMDSAbs_All )
  {
    for (int i=0; i<IDs.length(); i++) {
      CORBA::Long ind = IDs[i];
      const SMDS_MeshElement * elem =
        (aType == SMDSAbs_Node ? aMesh->FindNode(ind) : aMesh->FindElement(ind));
      if ( elem && ( aType == SMDSAbs_All || elem->GetType() == aType ))
        aMap.insert( elem );
    }
  }
  //================================================================================
  /*!
   * \brief Retrieve elements of given type from SMESH_IDSource
   */
  //================================================================================

  bool idSourceToSet(SMESH::SMESH_IDSource_ptr  theIDSource,
                     const SMESHDS_Mesh*        theMeshDS,
                     TIDSortedElemSet&          theElemSet,
                     const SMDSAbs_ElementType  theType,
                     const bool                 emptyIfIsMesh=false)

  {
    if ( CORBA::is_nil( theIDSource ) )
      return false;
    if ( emptyIfIsMesh && SMESH::DownCast<SMESH_Mesh_i*>( theIDSource ))
      return true;

    SMESH::long_array_var anIDs = theIDSource->GetIDs();
    if ( anIDs->length() == 0 )
      return false;
    SMESH::array_of_ElementType_var types = theIDSource->GetTypes();
    if ( types->length() == 1 && types[0] == SMESH::NODE ) // group of nodes
    {
      if ( theType == SMDSAbs_All || theType == SMDSAbs_Node )
        arrayToSet( anIDs, theMeshDS, theElemSet, SMDSAbs_Node );
      else
        return false;
    }
    else
    {
      arrayToSet( anIDs, theMeshDS, theElemSet, theType);
    }
    return true;
  }
  //================================================================================
  /*!
   * \brief Retrieve nodes from SMESH_IDSource
   */
  //================================================================================

  void idSourceToNodeSet(SMESH::SMESH_IDSource_ptr  theObject,
                         const SMESHDS_Mesh*        theMeshDS,
                         TIDSortedNodeSet&          theNodeSet)

  {
    if ( CORBA::is_nil( theObject ) )
      return;
    SMESH::array_of_ElementType_var types = theObject->GetTypes();
    SMESH::long_array_var     aElementsId = theObject->GetIDs();
    if ( types->length() == 1 && types[0] == SMESH::NODE)
    {
      for(int i = 0; i < aElementsId->length(); i++)
        if ( const SMDS_MeshNode * n = theMeshDS->FindNode( aElementsId[i] ))
          theNodeSet.insert( theNodeSet.end(), n);
    }
    else if ( SMESH::DownCast<SMESH_Mesh_i*>( theObject ))
    {
      SMDS_NodeIteratorPtr nIt = theMeshDS->nodesIterator();
      while ( nIt->more( ))
        if( const SMDS_MeshElement * elem = nIt->next() )
          theNodeSet.insert( elem->begin_nodes(), elem->end_nodes());
    }
    else
    {
      for(int i = 0; i < aElementsId->length(); i++)
        if( const SMDS_MeshElement * elem = theMeshDS->FindElement( aElementsId[i] ))
          theNodeSet.insert( elem->begin_nodes(), elem->end_nodes());
    }
  }

  //================================================================================
  /*!
   * \brief Returns elements connected to the given elements
   */
  //================================================================================

  void getElementsAround(const TIDSortedElemSet& theElements,
                         const SMESHDS_Mesh*     theMeshDS,
                         TIDSortedElemSet&       theElementsAround)
  {
    if ( theElements.empty() ) return;

    SMDSAbs_ElementType elemType    = (*theElements.begin())->GetType();
    bool sameElemType = ( elemType == (*theElements.rbegin())->GetType() );
    if ( sameElemType &&
         theMeshDS->GetMeshInfo().NbElements( elemType ) == theElements.size() )
      return; // all the elements are in theElements

    if ( !sameElemType )
      elemType = SMDSAbs_All;

    TIDSortedElemSet visitedNodes;
    TIDSortedElemSet::const_iterator elemIt = theElements.begin();
    for ( ; elemIt != theElements.end(); ++elemIt )
    {
      const SMDS_MeshElement* e = *elemIt;
      int i = e->NbCornerNodes();
      while ( --i != -1 )
      {
        const SMDS_MeshNode* n = e->GetNode( i );
        if ( visitedNodes.insert( n ).second )
        {
          SMDS_ElemIteratorPtr invIt = n->GetInverseElementIterator(elemType);
          while ( invIt->more() )
          {
            const SMDS_MeshElement* elemAround = invIt->next();
            if ( !theElements.count( elemAround ))
              theElementsAround.insert( elemAround );
          }
        }
      }
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_MeshEditor_i::SMESH_MeshEditor_i(SMESH_Mesh_i* theMesh, bool isPreview)
{
  myMesh_i = theMesh;
  myMesh = & theMesh->GetImpl();
  myPreviewMode = isPreview;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESH_MeshEditor_i::~SMESH_MeshEditor_i()
{
}

//================================================================================
/*!
 * \brief Clear members
 */
//================================================================================

void SMESH_MeshEditor_i::initData(bool deleteSearchers)
{
  if ( myPreviewMode ) {
    myPreviewData = new SMESH::MeshPreviewStruct();
  }
  else {
    myLastCreatedElems = new SMESH::long_array();
    myLastCreatedNodes = new SMESH::long_array();
    if ( deleteSearchers )
      TSearchersDeleter::Delete();
  }
}

//=======================================================================
//function : MakeIDSource
//purpose  : Wrap a sequence of ids in a SMESH_IDSource
//=======================================================================

struct _IDSource : public POA_SMESH::SMESH_IDSource
{
  SMESH::long_array     _ids;
  SMESH::ElementType    _type;
  SMESH::SMESH_Mesh_ptr _mesh;
  SMESH::long_array* GetIDs()      { return new SMESH::long_array( _ids ); }
  SMESH::long_array* GetMeshInfo() { return 0; }
  SMESH::SMESH_Mesh_ptr GetMesh()  { return SMESH::SMESH_Mesh::_duplicate( _mesh ); }
  bool IsMeshInfoCorrect()         { return true; }
  SMESH::array_of_ElementType* GetTypes()
  {
    SMESH::array_of_ElementType_var types = new SMESH::array_of_ElementType;
    if ( _ids.length() > 0 ) {
      types->length( 1 );
      types[0] = _type;
    }
    return types._retn();
  }
};

SMESH::SMESH_IDSource_ptr SMESH_MeshEditor_i::MakeIDSource(const SMESH::long_array& ids,
                                                           SMESH::ElementType       type)
{
  _IDSource* anIDSource = new _IDSource;
  anIDSource->_ids = ids;
  anIDSource->_type = type;
  anIDSource->_mesh = myMesh_i->_this();
  SMESH::SMESH_IDSource_var anIDSourceVar = anIDSource->_this();

  return anIDSourceVar._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean
SMESH_MeshEditor_i::RemoveElements(const SMESH::long_array & IDsOfElements)
{
  initData();

  ::SMESH_MeshEditor anEditor( myMesh );
  list< int > IdList;

  for (int i = 0; i < IDsOfElements.length(); i++)
    IdList.push_back( IDsOfElements[i] );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".RemoveElements( " << IDsOfElements << " )";

  // Remove Elements
  bool ret = anEditor.Remove( IdList, false );
  myMesh->GetMeshDS()->Modified();
  if ( IDsOfElements.length() )
    myMesh->SetIsModified( true ); // issue 0020693
  return ret;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveNodes(const SMESH::long_array & IDsOfNodes)
{
  initData();

  ::SMESH_MeshEditor anEditor( myMesh );
  list< int > IdList;
  for (int i = 0; i < IDsOfNodes.length(); i++)
    IdList.push_back( IDsOfNodes[i] );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".RemoveNodes( " << IDsOfNodes << " )";

  bool ret = anEditor.Remove( IdList, true );
  myMesh->GetMeshDS()->Modified();
  if ( IDsOfNodes.length() )
    myMesh->SetIsModified( true ); // issue 0020693
  return ret;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::RemoveOrphanNodes()
{
  initData();

  ::SMESH_MeshEditor anEditor( myMesh );

  // Update Python script
  TPythonDump() << "nbRemoved = " << this << ".RemoveOrphanNodes()";

  // Create filter to find all orphan nodes
  SMESH::Controls::Filter::TIdSequence seq;
  SMESH::Controls::PredicatePtr predicate( new SMESH::Controls::FreeNodes() );
  SMESH::Controls::Filter::GetElementsId( GetMeshDS(), predicate, seq );

  // remove orphan nodes (if there are any)
  list< int > IdList;
  for ( int i = 0; i < seq.size(); i++ )
    IdList.push_back( seq[i] );

  int nbNodesBefore = myMesh->NbNodes();
  anEditor.Remove( IdList, true );
  myMesh->GetMeshDS()->Modified();
  if ( IdList.size() )
    myMesh->SetIsModified( true );
  int nbNodesAfter = myMesh->NbNodes();

  return nbNodesBefore - nbNodesAfter;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddNode(CORBA::Double x,
                                        CORBA::Double y, CORBA::Double z)
{
  initData();

  const SMDS_MeshNode* N = GetMeshDS()->AddNode(x, y, z);

  // Update Python script
  TPythonDump() << "nodeID = " << this << ".AddNode( "
                << TVar( x ) << ", " << TVar( y ) << ", " << TVar( z )<< " )";

  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true ); // issue 0020693
  return N->GetID();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::Add0DElement(CORBA::Long IDOfNode)
{
  initData();

  const SMDS_MeshNode* aNode = GetMeshDS()->FindNode(IDOfNode);
  SMDS_MeshElement* elem = GetMeshDS()->Add0DElement(aNode);

  // Update Python script
  TPythonDump() << "elem0d = " << this << ".Add0DElement( " << IDOfNode <<" )";

  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true ); // issue 0020693

  if (elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddEdge(const SMESH::long_array & IDsOfNodes)
{
  initData();

  int NbNodes = IDsOfNodes.length();
  SMDS_MeshElement* elem = 0;
  if (NbNodes == 2)
  {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    elem = GetMeshDS()->AddEdge(GetMeshDS()->FindNode(index1), GetMeshDS()->FindNode(index2));

    // Update Python script
    TPythonDump() << "edge = " << this << ".AddEdge([ "
                  << index1 << ", " << index2 <<" ])";
  }
  if (NbNodes == 3) {
    CORBA::Long n1 = IDsOfNodes[0];
    CORBA::Long n2 = IDsOfNodes[1];
    CORBA::Long n12 = IDsOfNodes[2];
    elem = GetMeshDS()->AddEdge(GetMeshDS()->FindNode(n1),
                                GetMeshDS()->FindNode(n2),
                                GetMeshDS()->FindNode(n12));
    // Update Python script
    TPythonDump() << "edgeID = " << this << ".AddEdge([ "
                  <<n1<<", "<<n2<<", "<<n12<<" ])";
  }

  myMesh->GetMeshDS()->Modified();
  if(elem)
    return myMesh->SetIsModified( true ), elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddFace
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddFace(const SMESH::long_array & IDsOfNodes)
{
  initData();

  int NbNodes = IDsOfNodes.length();
  if (NbNodes < 3)
  {
    return 0;
  }

  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  SMDS_MeshElement* elem = 0;
  if (NbNodes == 3) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2]);
  }
  else if (NbNodes == 4) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3]);
  }
  else if (NbNodes == 6) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                nodes[4], nodes[5]);
  }
  else if (NbNodes == 8) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                nodes[4], nodes[5], nodes[6], nodes[7]);
  }
  else if (NbNodes == 9) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                nodes[4], nodes[5], nodes[6], nodes[7], nodes[8] );
  }
  else if (NbNodes > 2) {
    elem = GetMeshDS()->AddPolygonalFace(nodes);
  }

  // Update Python script
  TPythonDump() << "faceID = " << this << ".AddFace( " << IDsOfNodes << " )";

  myMesh->GetMeshDS()->Modified();
  if(elem)
    return myMesh->SetIsModified( true ), elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolygonalFace
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolygonalFace (const SMESH::long_array & IDsOfNodes)
{
  initData();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolygonalFace(nodes);

  // Update Python script
  TPythonDump() <<"faceID = "<<this<<".AddPolygonalFace( "<<IDsOfNodes<<" )";

  myMesh->GetMeshDS()->Modified();
  return elem ? ( myMesh->SetIsModified( true ), elem->GetID()) : 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddVolume(const SMESH::long_array & IDsOfNodes)
{
  initData();

  int NbNodes = IDsOfNodes.length();
  vector< const SMDS_MeshNode*> n(NbNodes);
  for(int i=0;i<NbNodes;i++)
    n[i]=GetMeshDS()->FindNode(IDsOfNodes[i]);

  SMDS_MeshElement* elem = 0;
  switch(NbNodes)
  {
  case 4 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3]); break;
  case 5 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4]); break;
  case 6 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5]); break;
  case 8 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]); break;
  case 10:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],
                                        n[6],n[7],n[8],n[9]);
    break;
  case 12:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],
                                        n[6],n[7],n[8],n[9],n[10],n[11]);
    break;
  case 13:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],
                                        n[7],n[8],n[9],n[10],n[11],n[12]);
    break;
  case 15:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],
                                        n[9],n[10],n[11],n[12],n[13],n[14]);
    break;
  case 20:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],
                                        n[8],n[9],n[10],n[11],n[12],n[13],n[14],
                                        n[15],n[16],n[17],n[18],n[19]);
    break;
  case 27:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],
                                        n[8],n[9],n[10],n[11],n[12],n[13],n[14],
                                        n[15],n[16],n[17],n[18],n[19],
                                        n[20],n[21],n[22],n[23],n[24],n[25],n[26]);
    break;
  }

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddVolume( " << IDsOfNodes << " )";

  myMesh->GetMeshDS()->Modified();
  if(elem)
    return myMesh->SetIsModified( true ), elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolume
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolume (const SMESH::long_array & IDsOfNodes,
                                                     const SMESH::long_array & Quantities)
{
  initData();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> n (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    {
      const SMDS_MeshNode* aNode = GetMeshDS()->FindNode(IDsOfNodes[i]);
      if (!aNode) return 0;
      n[i] = aNode;
    }

  int NbFaces = Quantities.length();
  std::vector<int> q (NbFaces);
  for (int j = 0; j < NbFaces; j++)
    q[j] = Quantities[j];

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolyhedralVolume(n, q);

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddPolyhedralVolume( "
                << IDsOfNodes << ", " << Quantities << " )";
  myMesh->GetMeshDS()->Modified();

  return elem ? ( myMesh->SetIsModified( true ), elem->GetID()) : 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolumeByFaces
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolumeByFaces (const SMESH::long_array & IdsOfFaces)
{
  initData();

  int NbFaces = IdsOfFaces.length();
  std::vector<const SMDS_MeshNode*> poly_nodes;
  std::vector<int> quantities (NbFaces);

  for (int i = 0; i < NbFaces; i++) {
    const SMDS_MeshElement* aFace = GetMeshDS()->FindElement(IdsOfFaces[i]);
    quantities[i] = aFace->NbNodes();

    SMDS_ElemIteratorPtr It = aFace->nodesIterator();
    while (It->more()) {
      poly_nodes.push_back(static_cast<const SMDS_MeshNode *>(It->next()));
    }
  }

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolyhedralVolume(poly_nodes, quantities);

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddPolyhedralVolumeByFaces( "
                << IdsOfFaces << " )";
  myMesh->GetMeshDS()->Modified();

  return elem ? ( myMesh->SetIsModified( true ), elem->GetID()) : 0;
}

//=============================================================================
/*!
 * \brief Bind a node to a vertex
 * \param NodeID - node ID
 * \param VertexID - vertex ID available through GEOM_Object.GetSubShapeIndices()[0]
 * \retval boolean - false if NodeID or VertexID is invalid
 */
//=============================================================================

void SMESH_MeshEditor_i::SetNodeOnVertex(CORBA::Long NodeID, CORBA::Long VertexID)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  SMESHDS_Mesh * mesh = GetMeshDS();
  SMDS_MeshNode* node = const_cast<SMDS_MeshNode*>( mesh->FindNode(NodeID) );
  if ( !node )
    THROW_SALOME_CORBA_EXCEPTION("Invalid NodeID", SALOME::BAD_PARAM);

  if ( mesh->MaxShapeIndex() < VertexID )
    THROW_SALOME_CORBA_EXCEPTION("Invalid VertexID", SALOME::BAD_PARAM);

  TopoDS_Shape shape = mesh->IndexToShape( VertexID );
  if ( shape.ShapeType() != TopAbs_VERTEX )
    THROW_SALOME_CORBA_EXCEPTION("Invalid VertexID", SALOME::BAD_PARAM);

  mesh->SetNodeOnVertex( node, VertexID );

  myMesh->SetIsModified( true );
}

//=============================================================================
/*!
 * \brief Store node position on an edge
 * \param NodeID - node ID
 * \param EdgeID - edge ID available through GEOM_Object.GetSubShapeIndices()[0]
 * \param paramOnEdge - parameter on edge where the node is located
 * \retval boolean - false if any parameter is invalid
 */
//=============================================================================

void SMESH_MeshEditor_i::SetNodeOnEdge(CORBA::Long NodeID, CORBA::Long EdgeID,
                                       CORBA::Double paramOnEdge)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  SMESHDS_Mesh * mesh = GetMeshDS();
  SMDS_MeshNode* node = const_cast<SMDS_MeshNode*>( mesh->FindNode(NodeID) );
  if ( !node )
    THROW_SALOME_CORBA_EXCEPTION("Invalid NodeID", SALOME::BAD_PARAM);

  if ( mesh->MaxShapeIndex() < EdgeID )
    THROW_SALOME_CORBA_EXCEPTION("Invalid EdgeID", SALOME::BAD_PARAM);

  TopoDS_Shape shape = mesh->IndexToShape( EdgeID );
  if ( shape.ShapeType() != TopAbs_EDGE )
    THROW_SALOME_CORBA_EXCEPTION("Invalid EdgeID", SALOME::BAD_PARAM);

  Standard_Real f,l;
  BRep_Tool::Range( TopoDS::Edge( shape ), f,l);
  if ( paramOnEdge < f || paramOnEdge > l )
    THROW_SALOME_CORBA_EXCEPTION("Invalid paramOnEdge", SALOME::BAD_PARAM);

  mesh->SetNodeOnEdge( node, EdgeID, paramOnEdge );

  myMesh->SetIsModified( true );
}

//=============================================================================
/*!
 * \brief Store node position on a face
 * \param NodeID - node ID
 * \param FaceID - face ID available through GEOM_Object.GetSubShapeIndices()[0]
 * \param u - U parameter on face where the node is located
 * \param v - V parameter on face where the node is located
 * \retval boolean - false if any parameter is invalid
 */
//=============================================================================

void SMESH_MeshEditor_i::SetNodeOnFace(CORBA::Long NodeID, CORBA::Long FaceID,
                                       CORBA::Double u, CORBA::Double v)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  SMESHDS_Mesh * mesh = GetMeshDS();
  SMDS_MeshNode* node = const_cast<SMDS_MeshNode*>( mesh->FindNode(NodeID) );
  if ( !node )
    THROW_SALOME_CORBA_EXCEPTION("Invalid NodeID", SALOME::BAD_PARAM);

  if ( mesh->MaxShapeIndex() < FaceID )
    THROW_SALOME_CORBA_EXCEPTION("Invalid FaceID", SALOME::BAD_PARAM);

  TopoDS_Shape shape = mesh->IndexToShape( FaceID );
  if ( shape.ShapeType() != TopAbs_FACE )
    THROW_SALOME_CORBA_EXCEPTION("Invalid FaceID", SALOME::BAD_PARAM);

  BRepAdaptor_Surface surf( TopoDS::Face( shape ));
  bool isOut = ( u < surf.FirstUParameter() ||
                 u > surf.LastUParameter()  ||
                 v < surf.FirstVParameter() ||
                 v > surf.LastVParameter() );

  if ( isOut ) {
#ifdef _DEBUG_
    MESSAGE ( "FACE " << FaceID << " (" << u << "," << v << ") out of "
              << " u( " <<  surf.FirstUParameter()
              << "," <<  surf.LastUParameter()
              << ") v( " <<  surf.FirstVParameter()
              << "," <<  surf.LastVParameter() << ")" );
#endif
    THROW_SALOME_CORBA_EXCEPTION("Invalid UV", SALOME::BAD_PARAM);
  }

  mesh->SetNodeOnFace( node, FaceID, u, v );
  myMesh->SetIsModified( true );
}

//=============================================================================
/*!
 * \brief Bind a node to a solid
 * \param NodeID - node ID
 * \param SolidID - vertex ID available through GEOM_Object.GetSubShapeIndices()[0]
 * \retval boolean - false if NodeID or SolidID is invalid
 */
//=============================================================================

void SMESH_MeshEditor_i::SetNodeInVolume(CORBA::Long NodeID, CORBA::Long SolidID)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  SMESHDS_Mesh * mesh = GetMeshDS();
  SMDS_MeshNode* node = const_cast<SMDS_MeshNode*>( mesh->FindNode(NodeID) );
  if ( !node )
    THROW_SALOME_CORBA_EXCEPTION("Invalid NodeID", SALOME::BAD_PARAM);

  if ( mesh->MaxShapeIndex() < SolidID )
    THROW_SALOME_CORBA_EXCEPTION("Invalid SolidID", SALOME::BAD_PARAM);

  TopoDS_Shape shape = mesh->IndexToShape( SolidID );
  if ( shape.ShapeType() != TopAbs_SOLID &&
       shape.ShapeType() != TopAbs_SHELL)
    THROW_SALOME_CORBA_EXCEPTION("Invalid SolidID", SALOME::BAD_PARAM);

  mesh->SetNodeInVolume( node, SolidID );

  // myMesh->SetIsModified( true ); - SetNodeInVolume() can't prevent re-compute, I believe
}

//=============================================================================
/*!
 * \brief Bind an element to a shape
 * \param ElementID - element ID
 * \param ShapeID - shape ID available through GEOM_Object.GetSubShapeIndices()[0]
 * \retval boolean - false if ElementID or ShapeID is invalid
 */
//=============================================================================

void SMESH_MeshEditor_i::SetMeshElementOnShape(CORBA::Long ElementID,
                                               CORBA::Long ShapeID)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  SMESHDS_Mesh * mesh = GetMeshDS();
  SMDS_MeshElement* elem = const_cast<SMDS_MeshElement*>(mesh->FindElement(ElementID));
  if ( !elem )
    THROW_SALOME_CORBA_EXCEPTION("Invalid ElementID", SALOME::BAD_PARAM);

  if ( mesh->MaxShapeIndex() < ShapeID )
    THROW_SALOME_CORBA_EXCEPTION("Invalid ShapeID", SALOME::BAD_PARAM);

  TopoDS_Shape shape = mesh->IndexToShape( ShapeID );
  if ( shape.ShapeType() != TopAbs_EDGE &&
       shape.ShapeType() != TopAbs_FACE &&
       shape.ShapeType() != TopAbs_SOLID &&
       shape.ShapeType() != TopAbs_SHELL )
    THROW_SALOME_CORBA_EXCEPTION("Invalid shape type", SALOME::BAD_PARAM);

  mesh->SetMeshElementOnShape( elem, ShapeID );

  myMesh->SetIsModified( true );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::InverseDiag(CORBA::Long NodeID1,
                                               CORBA::Long NodeID2)
{
  initData();

  const SMDS_MeshNode * n1 = GetMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = GetMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TPythonDump() << "isDone = " << this << ".InverseDiag( "
                << NodeID1 << ", " << NodeID2 << " )";


  ::SMESH_MeshEditor aMeshEditor( myMesh );
  int ret =  aMeshEditor.InverseDiag ( n1, n2 );
  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );
  return ret;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::DeleteDiag(CORBA::Long NodeID1,
                                              CORBA::Long NodeID2)
{
  initData();

  const SMDS_MeshNode * n1 = GetMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = GetMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DeleteDiag( "
                << NodeID1 << ", " << NodeID2 <<  " )";

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  bool stat = aMeshEditor.DeleteDiag ( n1, n2 );

  myMesh->GetMeshDS()->Modified();
  if ( stat )
    myMesh->SetIsModified( true ); // issue 0020693

  storeResult(aMeshEditor);

  return stat;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::Reorient(const SMESH::long_array & IDsOfElements)
{
  initData();

  ::SMESH_MeshEditor anEditor( myMesh );
  for (int i = 0; i < IDsOfElements.length(); i++)
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = GetMeshDS()->FindElement(index);
    if ( elem )
      anEditor.Reorient( elem );
  }
  // Update Python script
  TPythonDump() << "isDone = " << this << ".Reorient( " << IDsOfElements << " )";

  myMesh->GetMeshDS()->Modified();
  if ( IDsOfElements.length() )
    myMesh->SetIsModified( true ); // issue 0020693

  return true;
}


//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::ReorientObject(SMESH::SMESH_IDSource_ptr theObject)
{
  initData();

  TPythonDump aTPythonDump; // suppress dump in Reorient()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = Reorient(anElementsId);

  // Update Python script
  aTPythonDump << "isDone = " << this << ".ReorientObject( " << theObject << " )";

  return isDone;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::TriToQuad (const SMESH::long_array &   IDsOfElements,
                                              SMESH::NumericalFunctor_ptr Criterion,
                                              CORBA::Double               MaxAngle)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();
  TIDSortedElemSet faces;
  arrayToSet(IDsOfElements, aMesh, faces, SMDSAbs_Face);

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    dynamic_cast<SMESH::NumericalFunctor_i*>( SMESH_Gen_i::GetServant( Criterion ).in() );
  SMESH::Controls::NumericalFunctorPtr aCrit;
  if ( !aNumericalFunctor )
    aCrit.reset( new SMESH::Controls::AspectRatio() );
  else
    aCrit = aNumericalFunctor->GetNumericalFunctor();

  // Update Python script
  TPythonDump() << "isDone = " << this << ".TriToQuad( "
                << IDsOfElements << ", " << aNumericalFunctor << ", " << TVar( MaxAngle ) << " )";

  ::SMESH_MeshEditor anEditor( myMesh );

  bool stat = anEditor.TriToQuad( faces, aCrit, MaxAngle );
  myMesh->GetMeshDS()->Modified();
  if ( stat )
    myMesh->SetIsModified( true ); // issue 0020693

  storeResult(anEditor);

  return stat;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::TriToQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
                                                    SMESH::NumericalFunctor_ptr Criterion,
                                                    CORBA::Double               MaxAngle)
{
  initData();

  TPythonDump aTPythonDump;  // suppress dump in TriToQuad()
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = TriToQuad(anElementsId, Criterion, MaxAngle);

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    SMESH::DownCast<SMESH::NumericalFunctor_i*>( Criterion );

  // Update Python script
  aTPythonDump << "isDone = " << this << ".TriToQuadObject("
               << theObject << ", " << aNumericalFunctor << ", " << TVar( MaxAngle ) << " )";

  return isDone;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::QuadToTri (const SMESH::long_array &   IDsOfElements,
                                              SMESH::NumericalFunctor_ptr Criterion)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();
  TIDSortedElemSet faces;
  arrayToSet(IDsOfElements, aMesh, faces, SMDSAbs_Face);

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    dynamic_cast<SMESH::NumericalFunctor_i*>( SMESH_Gen_i::GetServant( Criterion ).in() );
  SMESH::Controls::NumericalFunctorPtr aCrit;
  if ( !aNumericalFunctor )
    aCrit.reset( new SMESH::Controls::AspectRatio() );
  else
    aCrit = aNumericalFunctor->GetNumericalFunctor();


  // Update Python script
  TPythonDump() << "isDone = " << this << ".QuadToTri( " << IDsOfElements << ", " << aNumericalFunctor << " )";

  ::SMESH_MeshEditor anEditor( myMesh );
  CORBA::Boolean stat = anEditor.QuadToTri( faces, aCrit );
  myMesh->GetMeshDS()->Modified();
  if ( stat )
    myMesh->SetIsModified( true ); // issue 0020693

  storeResult(anEditor);

  return stat;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::QuadToTriObject (SMESH::SMESH_IDSource_ptr   theObject,
                                                    SMESH::NumericalFunctor_ptr Criterion)
{
  initData();

  TPythonDump aTPythonDump;  // suppress dump in QuadToTri()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = QuadToTri(anElementsId, Criterion);

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    SMESH::DownCast<SMESH::NumericalFunctor_i*>( Criterion );

  // Update Python script
  aTPythonDump << "isDone = " << this << ".QuadToTriObject( " << theObject << ", " << aNumericalFunctor << " )";

  return isDone;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::SplitQuad (const SMESH::long_array & IDsOfElements,
                                              CORBA::Boolean            Diag13)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();
  TIDSortedElemSet faces;
  arrayToSet(IDsOfElements, aMesh, faces, SMDSAbs_Face);

  // Update Python script
  TPythonDump() << "isDone = " << this << ".SplitQuad( "
                << IDsOfElements << ", " << Diag13 << " )";

  ::SMESH_MeshEditor anEditor( myMesh );
  CORBA::Boolean stat = anEditor.QuadToTri( faces, Diag13 );
  myMesh->GetMeshDS()->Modified();
  if ( stat )
    myMesh->SetIsModified( true ); // issue 0020693


  storeResult(anEditor);

  return stat;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::SplitQuadObject (SMESH::SMESH_IDSource_ptr theObject,
                                                    CORBA::Boolean            Diag13)
{
  initData();

  TPythonDump aTPythonDump;  // suppress dump in SplitQuad()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = SplitQuad(anElementsId, Diag13);

  // Update Python script
  aTPythonDump << "isDone = " << this << ".SplitQuadObject( "
               << theObject << ", " << Diag13 << " )";

  return isDone;
}


//=============================================================================
/*!
 *  BestSplit
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::BestSplit (CORBA::Long                 IDOfQuad,
                                           SMESH::NumericalFunctor_ptr Criterion)
{
  initData();

  const SMDS_MeshElement* quad = GetMeshDS()->FindElement(IDOfQuad);
  if (quad && quad->GetType() == SMDSAbs_Face && quad->NbNodes() == 4)
  {
    SMESH::NumericalFunctor_i* aNumericalFunctor =
      dynamic_cast<SMESH::NumericalFunctor_i*>(SMESH_Gen_i::GetServant(Criterion).in());
    SMESH::Controls::NumericalFunctorPtr aCrit;
    if (aNumericalFunctor)
      aCrit = aNumericalFunctor->GetNumericalFunctor();
    else
      aCrit.reset(new SMESH::Controls::AspectRatio());

    ::SMESH_MeshEditor anEditor (myMesh);
    return anEditor.BestSplit(quad, aCrit);
  }
  return -1;
}

//================================================================================
/*!
 * \brief Split volumic elements into tetrahedrons
 */
//================================================================================

void SMESH_MeshEditor_i::SplitVolumesIntoTetra (SMESH::SMESH_IDSource_ptr elems,
                                                CORBA::Short              methodFlags)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  initData();

  SMESH::long_array_var anElementsId = elems->GetIDs();
  TIDSortedElemSet elemSet;
  arrayToSet( anElementsId, GetMeshDS(), elemSet, SMDSAbs_Volume );

  ::SMESH_MeshEditor anEditor (myMesh);
  anEditor.SplitVolumesIntoTetra( elemSet, int( methodFlags ));
  myMesh->GetMeshDS()->Modified();

  storeResult(anEditor);

//   if ( myLastCreatedElems.length() ) - it does not influence Compute()
//     myMesh->SetIsModified( true ); // issue 0020693

  TPythonDump() << this << ".SplitVolumesIntoTetra( "
                << elems << ", " << methodFlags << " )";
}

//=======================================================================
//function : Smooth
//purpose  :
//=======================================================================

CORBA::Boolean
SMESH_MeshEditor_i::Smooth(const SMESH::long_array &              IDsOfElements,
                           const SMESH::long_array &              IDsOfFixedNodes,
                           CORBA::Long                            MaxNbOfIterations,
                           CORBA::Double                          MaxAspectRatio,
                           SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smooth( IDsOfElements, IDsOfFixedNodes, MaxNbOfIterations,
                 MaxAspectRatio, Method, false );
}


//=======================================================================
//function : SmoothParametric
//purpose  :
//=======================================================================

CORBA::Boolean
SMESH_MeshEditor_i::SmoothParametric(const SMESH::long_array &              IDsOfElements,
                                     const SMESH::long_array &              IDsOfFixedNodes,
                                     CORBA::Long                            MaxNbOfIterations,
                                     CORBA::Double                          MaxAspectRatio,
                                     SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smooth( IDsOfElements, IDsOfFixedNodes, MaxNbOfIterations,
                 MaxAspectRatio, Method, true );
}


//=======================================================================
//function : SmoothObject
//purpose  :
//=======================================================================

CORBA::Boolean
SMESH_MeshEditor_i::SmoothObject(SMESH::SMESH_IDSource_ptr              theObject,
                                 const SMESH::long_array &              IDsOfFixedNodes,
                                 CORBA::Long                            MaxNbOfIterations,
                                 CORBA::Double                          MaxAspectRatio,
                                 SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smoothObject (theObject, IDsOfFixedNodes, MaxNbOfIterations,
                       MaxAspectRatio, Method, false);
}


//=======================================================================
//function : SmoothParametricObject
//purpose  :
//=======================================================================

CORBA::Boolean
SMESH_MeshEditor_i::SmoothParametricObject(SMESH::SMESH_IDSource_ptr              theObject,
                                           const SMESH::long_array &              IDsOfFixedNodes,
                                           CORBA::Long                            MaxNbOfIterations,
                                           CORBA::Double                          MaxAspectRatio,
                                           SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smoothObject (theObject, IDsOfFixedNodes, MaxNbOfIterations,
                       MaxAspectRatio, Method, true);
}


//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean
SMESH_MeshEditor_i::smooth(const SMESH::long_array &              IDsOfElements,
                           const SMESH::long_array &              IDsOfFixedNodes,
                           CORBA::Long                            MaxNbOfIterations,
                           CORBA::Double                          MaxAspectRatio,
                           SMESH::SMESH_MeshEditor::Smooth_Method Method,
                           bool                                   IsParametric)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  TIDSortedElemSet elements;
  arrayToSet(IDsOfElements, aMesh, elements, SMDSAbs_Face);

  set<const SMDS_MeshNode*> fixedNodes;
  for (int i = 0; i < IDsOfFixedNodes.length(); i++) {
    CORBA::Long index = IDsOfFixedNodes[i];
    const SMDS_MeshNode * node = aMesh->FindNode(index);
    if ( node )
      fixedNodes.insert( node );
  }
  ::SMESH_MeshEditor::SmoothMethod method = ::SMESH_MeshEditor::LAPLACIAN;
  if ( Method != SMESH::SMESH_MeshEditor::LAPLACIAN_SMOOTH )
    method = ::SMESH_MeshEditor::CENTROIDAL;

  ::SMESH_MeshEditor anEditor( myMesh );
  anEditor.Smooth(elements, fixedNodes, method,
                  MaxNbOfIterations, MaxAspectRatio, IsParametric );

  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true ); // issue 0020693

  storeResult(anEditor);

  // Update Python script
  TPythonDump() << "isDone = " << this << "."
                << (IsParametric ? "SmoothParametric( " : "Smooth( ")
                << IDsOfElements << ", "     << IDsOfFixedNodes << ", "
                << TVar( MaxNbOfIterations ) << ", " << TVar( MaxAspectRatio ) << ", "
                << "SMESH.SMESH_MeshEditor."
                << ( Method == SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH ?
                     "CENTROIDAL_SMOOTH )" : "LAPLACIAN_SMOOTH )");

  return true;
}


//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean
SMESH_MeshEditor_i::smoothObject(SMESH::SMESH_IDSource_ptr              theObject,
                                 const SMESH::long_array &              IDsOfFixedNodes,
                                 CORBA::Long                            MaxNbOfIterations,
                                 CORBA::Double                          MaxAspectRatio,
                                 SMESH::SMESH_MeshEditor::Smooth_Method Method,
                                 bool                                   IsParametric)
{
  initData();

  TPythonDump aTPythonDump;  // suppress dump in smooth()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = smooth (anElementsId, IDsOfFixedNodes, MaxNbOfIterations,
                                  MaxAspectRatio, Method, IsParametric);

  // Update Python script
  aTPythonDump << "isDone = " << this << "."
               << (IsParametric ? "SmoothParametricObject( " : "SmoothObject( ")
               << theObject << ", " << IDsOfFixedNodes << ", "
               << TVar( MaxNbOfIterations ) << ", " << TVar( MaxAspectRatio ) << ", "
               << "SMESH.SMESH_MeshEditor."
               << ( Method == SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH ?
                    "CENTROIDAL_SMOOTH )" : "LAPLACIAN_SMOOTH )");

  return isDone;
}


//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberNodes()
{
  // Update Python script
  TPythonDump() << this << ".RenumberNodes()";

  GetMeshDS()->Renumber( true );
}


//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberElements()
{
  // Update Python script
  TPythonDump() << this << ".RenumberElements()";

  GetMeshDS()->Renumber( false );
}

//=======================================================================
/*!
 * \brief Return groups by their IDs
 */
//=======================================================================

SMESH::ListOfGroups* SMESH_MeshEditor_i::getGroups(const std::list<int>* groupIDs)
{
  if ( !groupIDs )
    return 0;
  myMesh_i->CreateGroupServants();
  return myMesh_i->GetGroups( *groupIDs );
}

//=======================================================================
//function : rotationSweep
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::rotationSweep(const SMESH::long_array & theIDsOfElements,
                                  const SMESH::AxisStruct & theAxis,
                                  CORBA::Double             theAngleInRadians,
                                  CORBA::Long               theNbOfSteps,
                                  CORBA::Double             theTolerance,
                                  const bool                theMakeGroups,
                                  const SMDSAbs_ElementType theElementType)
{
  initData();

  TIDSortedElemSet inElements, copyElements;
  arrayToSet(theIDsOfElements, GetMeshDS(), inElements, theElementType);

  TIDSortedElemSet* workElements = & inElements;
  TPreviewMesh      tmpMesh( SMDSAbs_Face );
  SMESH_Mesh*       mesh = 0;
  bool              makeWalls=true;
  if ( myPreviewMode )
  {
    SMDSAbs_ElementType select = SMDSAbs_All, avoid = SMDSAbs_Volume;
    tmpMesh.Copy( inElements, copyElements, select, avoid );
    mesh = &tmpMesh;
    workElements = & copyElements;
    //makeWalls = false;
  }
  else
  {
    mesh = myMesh;
  }

  gp_Ax1 Ax1 (gp_Pnt( theAxis.x,  theAxis.y,  theAxis.z ),
              gp_Vec( theAxis.vx, theAxis.vy, theAxis.vz ));

  ::SMESH_MeshEditor anEditor( mesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.RotationSweep (*workElements, Ax1, theAngleInRadians,
                              theNbOfSteps, theTolerance, theMakeGroups, makeWalls);
  storeResult(anEditor);
  myMesh->GetMeshDS()->Modified();

  //  myMesh->SetIsModified( true ); -- it does not influence Compute()

  return theMakeGroups ? getGroups(groupIds.get()) : 0;
}

//=======================================================================
//function : RotationSweep
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotationSweep(const SMESH::long_array & theIDsOfElements,
                                       const SMESH::AxisStruct & theAxis,
                                       CORBA::Double             theAngleInRadians,
                                       CORBA::Long               theNbOfSteps,
                                       CORBA::Double             theTolerance)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".RotationSweep( "
                  << theIDsOfElements          << ", "
                  << theAxis                   << ", "
                  << TVar( theAngleInRadians ) << ", "
                  << TVar( theNbOfSteps      ) << ", "
                  << TVar( theTolerance      ) << " )";
  }
  rotationSweep(theIDsOfElements,
                theAxis,
                theAngleInRadians,
                theNbOfSteps,
                theTolerance,
                false);
}

//=======================================================================
//function : RotationSweepMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::RotationSweepMakeGroups(const SMESH::long_array& theIDsOfElements,
                                            const SMESH::AxisStruct& theAxis,
                                            CORBA::Double            theAngleInRadians,
                                            CORBA::Long              theNbOfSteps,
                                            CORBA::Double            theTolerance)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups *aGroups = rotationSweep(theIDsOfElements,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".RotationSweepMakeGroups( "
                << theIDsOfElements        << ", "
                << theAxis                   << ", "
                << TVar( theAngleInRadians ) << ", "
                << TVar( theNbOfSteps      ) << ", "
                << TVar( theTolerance      ) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : RotationSweepObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotationSweepObject(SMESH::SMESH_IDSource_ptr theObject,
                                             const SMESH::AxisStruct & theAxis,
                                             CORBA::Double             theAngleInRadians,
                                             CORBA::Long               theNbOfSteps,
                                             CORBA::Double             theTolerance)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".RotationSweepObject( "
                  << theObject << ", "
                  << theAxis << ", "
                  << theAngleInRadians << ", "
                  << theNbOfSteps << ", "
                  << theTolerance << " )";
  }
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  rotationSweep(anElementsId,
                theAxis,
                theAngleInRadians,
                theNbOfSteps,
                theTolerance,
                false);
}

//=======================================================================
//function : RotationSweepObject1D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotationSweepObject1D(SMESH::SMESH_IDSource_ptr theObject,
                                               const SMESH::AxisStruct & theAxis,
                                               CORBA::Double             theAngleInRadians,
                                               CORBA::Long               theNbOfSteps,
                                               CORBA::Double             theTolerance)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".RotationSweepObject1D( "
                  << theObject                 << ", "
                  << theAxis                   << ", "
                  << TVar( theAngleInRadians ) << ", "
                  << TVar( theNbOfSteps      ) << ", "
                  << TVar( theTolerance      ) << " )";
  }
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  rotationSweep(anElementsId,
                theAxis,
                theAngleInRadians,
                theNbOfSteps,
                theTolerance,
                false,
                SMDSAbs_Edge);
}

//=======================================================================
//function : RotationSweepObject2D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotationSweepObject2D(SMESH::SMESH_IDSource_ptr theObject,
                                               const SMESH::AxisStruct & theAxis,
                                               CORBA::Double             theAngleInRadians,
                                               CORBA::Long               theNbOfSteps,
                                               CORBA::Double             theTolerance)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".RotationSweepObject2D( "
                  << theObject                 << ", "
                  << theAxis                   << ", "
                  << TVar( theAngleInRadians ) << ", "
                  << TVar( theNbOfSteps      ) << ", "
                  << TVar( theTolerance      ) << " )";
  }
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  rotationSweep(anElementsId,
                theAxis,
                theAngleInRadians,
                theNbOfSteps,
                theTolerance,
                false,
                SMDSAbs_Face);
}

//=======================================================================
//function : RotationSweepObjectMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::RotationSweepObjectMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                                  const SMESH::AxisStruct&  theAxis,
                                                  CORBA::Double             theAngleInRadians,
                                                  CORBA::Long               theNbOfSteps,
                                                  CORBA::Double             theTolerance)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups *aGroups = rotationSweep(anElementsId,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".RotationSweepObjectMakeGroups( "
                << theObject << ", "
                << theAxis << ", "
                << theAngleInRadians << ", "
                << theNbOfSteps << ", "
                << theTolerance << " )";
  }
  return aGroups;
}

//=======================================================================
//function : RotationSweepObject1DMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::RotationSweepObject1DMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                                    const SMESH::AxisStruct&  theAxis,
                                                    CORBA::Double             theAngleInRadians,
                                                    CORBA::Long               theNbOfSteps,
                                                    CORBA::Double             theTolerance)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups *aGroups = rotationSweep(anElementsId,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true,
                                               SMDSAbs_Edge);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".RotationSweepObject1DMakeGroups( "
                << theObject                 << ", "
                << theAxis                   << ", "
                << TVar( theAngleInRadians ) << ", "
                << TVar( theNbOfSteps )      << ", "
                << TVar( theTolerance )      << " )";
  }
  return aGroups;
}

//=======================================================================
//function : RotationSweepObject2DMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::RotationSweepObject2DMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                                    const SMESH::AxisStruct&  theAxis,
                                                    CORBA::Double             theAngleInRadians,
                                                    CORBA::Long               theNbOfSteps,
                                                    CORBA::Double             theTolerance)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups *aGroups = rotationSweep(anElementsId,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true,
                                               SMDSAbs_Face);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".RotationSweepObject2DMakeGroups( "
                << theObject                 << ", "
                << theAxis                   << ", "
                << TVar( theAngleInRadians ) << ", "
                << TVar( theNbOfSteps      ) << ", "
                << TVar( theTolerance      ) << " )";
  }
  return aGroups;
}


//=======================================================================
//function : extrusionSweep
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::extrusionSweep(const SMESH::long_array & theIDsOfElements,
                                   const SMESH::DirStruct &  theStepVector,
                                   CORBA::Long               theNbOfSteps,
                                   bool                      theMakeGroups,
                                   const SMDSAbs_ElementType theElementType)
{
  initData();

  try {
#ifdef NO_CAS_CATCH
    OCC_CATCH_SIGNALS;
#endif
    TIDSortedElemSet elements, copyElements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements, theElementType);

    const SMESH::PointStruct * P = &theStepVector.PS;
    gp_Vec stepVec( P->x, P->y, P->z );

    TIDSortedElemSet* workElements = & elements;

    SMDSAbs_ElementType aType = SMDSAbs_Face;
    //::SMESH_MeshEditor::ExtrusionFlags aFlag = ::SMESH_MeshEditor::ExtrusionFlags::EXTRUSION_FLAG_BOUNDARY;
    if (theElementType == SMDSAbs_Node)
    {
      aType = SMDSAbs_Edge;
      //aFlag = ::SMESH_MeshEditor::ExtrusionFlags::EXTRUSION_FLAG_SEW;
    }
    TPreviewMesh      tmpMesh( aType );
    SMESH_Mesh* mesh = myMesh;

    if ( myPreviewMode ) {
      SMDSAbs_ElementType select = SMDSAbs_All, avoid = SMDSAbs_Volume;
      tmpMesh.Copy( elements, copyElements, select, avoid );
      mesh = &tmpMesh;
      workElements = & copyElements;
      theMakeGroups = false;
    }

    TElemOfElemListMap aHystory;
    ::SMESH_MeshEditor anEditor( mesh );
    ::SMESH_MeshEditor::PGroupIDs groupIds = 
        anEditor.ExtrusionSweep (*workElements, stepVec, theNbOfSteps, aHystory, theMakeGroups);

    myMesh->GetMeshDS()->Modified();
    storeResult(anEditor);

    return theMakeGroups ? getGroups(groupIds.get()) : 0;

  } catch(Standard_Failure) {
    Handle(Standard_Failure) aFail = Standard_Failure::Caught();
    INFOS( "SMESH_MeshEditor_i::ExtrusionSweep fails - "<< aFail->GetMessageString() );
  }
  return 0;
}

//=======================================================================
//function : ExtrusionSweep
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweep(const SMESH::long_array & theIDsOfElements,
                                        const SMESH::DirStruct &  theStepVector,
                                        CORBA::Long               theNbOfSteps)
{
  extrusionSweep (theIDsOfElements, theStepVector, theNbOfSteps, false );
  if (!myPreviewMode) {
    TPythonDump() << this << ".ExtrusionSweep( "
                  << theIDsOfElements << ", " << theStepVector <<", " << TVar(theNbOfSteps) << " )";
  }
}

//=======================================================================
//function : ExtrusionSweep0D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweep0D(const SMESH::long_array & theIDsOfElements,
                                          const SMESH::DirStruct &  theStepVector,
                                          CORBA::Long               theNbOfSteps)
{
  extrusionSweep (theIDsOfElements, theStepVector, theNbOfSteps, false, SMDSAbs_Node );
  if (!myPreviewMode) {
    TPythonDump() << this << ".ExtrusionSweep0D( "
                  << theIDsOfElements << ", " << theStepVector <<", " << TVar(theNbOfSteps)<< " )";
  }
}

//=======================================================================
//function : ExtrusionSweepObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject(SMESH::SMESH_IDSource_ptr theObject,
                                              const SMESH::DirStruct &  theStepVector,
                                              CORBA::Long               theNbOfSteps)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  extrusionSweep (anElementsId, theStepVector, theNbOfSteps, false );
  if (!myPreviewMode) {
    TPythonDump() << this << ".ExtrusionSweepObject( "
                  << theObject << ", " << theStepVector << ", " << theNbOfSteps << " )";
  }
}

//=======================================================================
//function : ExtrusionSweepObject0D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject0D(SMESH::SMESH_IDSource_ptr theObject,
                                                const SMESH::DirStruct &  theStepVector,
                                                CORBA::Long               theNbOfSteps)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  extrusionSweep (anElementsId, theStepVector, theNbOfSteps, false, SMDSAbs_Node );
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".ExtrusionSweepObject0D( "
                  << theObject << ", " << theStepVector << ", " << TVar( theNbOfSteps ) << " )";
  }
}

//=======================================================================
//function : ExtrusionSweepObject1D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject1D(SMESH::SMESH_IDSource_ptr theObject,
                                                const SMESH::DirStruct &  theStepVector,
                                                CORBA::Long               theNbOfSteps)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  extrusionSweep (anElementsId, theStepVector, theNbOfSteps, false, SMDSAbs_Edge );
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".ExtrusionSweepObject1D( "
                  << theObject << ", " << theStepVector << ", " << TVar( theNbOfSteps ) << " )";
  }
}

//=======================================================================
//function : ExtrusionSweepObject2D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject2D(SMESH::SMESH_IDSource_ptr theObject,
                                                const SMESH::DirStruct &  theStepVector,
                                                CORBA::Long               theNbOfSteps)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  extrusionSweep (anElementsId, theStepVector, theNbOfSteps, false, SMDSAbs_Face );
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".ExtrusionSweepObject2D( "
                  << theObject << ", " << theStepVector << ", " << TVar( theNbOfSteps ) << " )";
  }
}

//=======================================================================
//function : ExtrusionSweepMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionSweepMakeGroups(const SMESH::long_array& theIDsOfElements,
                                             const SMESH::DirStruct&  theStepVector,
                                             CORBA::Long              theNbOfSteps)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups* aGroups = extrusionSweep(theIDsOfElements, theStepVector, theNbOfSteps, true);

  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".ExtrusionSweepMakeGroups( " << theIDsOfElements
                << ", " << theStepVector <<", " << TVar( theNbOfSteps ) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionSweepMakeGroups0D
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionSweepMakeGroups0D(const SMESH::long_array& theIDsOfElements,
                                               const SMESH::DirStruct&  theStepVector,
                                               CORBA::Long              theNbOfSteps)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups* aGroups = extrusionSweep(theIDsOfElements, theStepVector, theNbOfSteps, true,SMDSAbs_Node);

  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".ExtrusionSweepMakeGroups0D( " << theIDsOfElements
                << ", " << theStepVector <<", " << TVar( theNbOfSteps ) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionSweepObjectMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionSweepObjectMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                                   const SMESH::DirStruct&   theStepVector,
                                                   CORBA::Long               theNbOfSteps)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionSweep(anElementsId, theStepVector, theNbOfSteps, true);

  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".ExtrusionSweepObjectMakeGroups( " << theObject
                << ", " << theStepVector << ", " << theNbOfSteps << " )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionSweepObject0DMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionSweepObject0DMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                                     const SMESH::DirStruct&   theStepVector,
                                                     CORBA::Long               theNbOfSteps)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionSweep(anElementsId, theStepVector,
                                                 theNbOfSteps, true, SMDSAbs_Node);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".ExtrusionSweepObject0DMakeGroups( " << theObject
                << ", " << theStepVector << ", " << TVar( theNbOfSteps ) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionSweepObject1DMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionSweepObject1DMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                                     const SMESH::DirStruct&   theStepVector,
                                                     CORBA::Long               theNbOfSteps)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionSweep(anElementsId, theStepVector,
                                                 theNbOfSteps, true, SMDSAbs_Edge);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".ExtrusionSweepObject1DMakeGroups( " << theObject
                << ", " << theStepVector << ", " << TVar( theNbOfSteps ) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionSweepObject2DMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionSweepObject2DMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                                     const SMESH::DirStruct&   theStepVector,
                                                     CORBA::Long               theNbOfSteps)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionSweep(anElementsId, theStepVector,
                                                 theNbOfSteps, true, SMDSAbs_Face);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".ExtrusionSweepObject2DMakeGroups( " << theObject
                << ", " << theStepVector << ", " << TVar( theNbOfSteps ) << " )";
  }
  return aGroups;
}


//=======================================================================
//function : advancedExtrusion
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::advancedExtrusion(const SMESH::long_array & theIDsOfElements,
                                      const SMESH::DirStruct &  theStepVector,
                                      CORBA::Long               theNbOfSteps,
                                      CORBA::Long               theExtrFlags,
                                      CORBA::Double             theSewTolerance,
                                      const bool                theMakeGroups)
{
  initData();

  TIDSortedElemSet elements;
  arrayToSet(theIDsOfElements, GetMeshDS(), elements);

  const SMESH::PointStruct * P = &theStepVector.PS;
  gp_Vec stepVec( P->x, P->y, P->z );

  ::SMESH_MeshEditor anEditor( myMesh );
  TElemOfElemListMap aHystory;
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps, aHystory,
                               theMakeGroups, theExtrFlags, theSewTolerance);
  storeResult(anEditor);

  return theMakeGroups ? getGroups(groupIds.get()) : 0;
}

//=======================================================================
//function : AdvancedExtrusion
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::AdvancedExtrusion(const SMESH::long_array & theIDsOfElements,
                                           const SMESH::DirStruct &  theStepVector,
                                           CORBA::Long               theNbOfSteps,
                                           CORBA::Long               theExtrFlags,
                                           CORBA::Double             theSewTolerance)
{
  if ( !myPreviewMode ) {
    TPythonDump() << "stepVector = " << theStepVector;
    TPythonDump() << this << ".AdvancedExtrusion("
                  << theIDsOfElements
                  << ", stepVector, "
                  << theNbOfSteps << ","
                  << theExtrFlags << ", "
                  << theSewTolerance <<  " )";
  }
  advancedExtrusion( theIDsOfElements,
                     theStepVector,
                     theNbOfSteps,
                     theExtrFlags,
                     theSewTolerance,
                     false);
}

//=======================================================================
//function : AdvancedExtrusionMakeGroups
//purpose  :
//=======================================================================
SMESH::ListOfGroups*
SMESH_MeshEditor_i::AdvancedExtrusionMakeGroups(const SMESH::long_array& theIDsOfElements,
                                                const SMESH::DirStruct&  theStepVector,
                                                CORBA::Long              theNbOfSteps,
                                                CORBA::Long              theExtrFlags,
                                                CORBA::Double            theSewTolerance)
{
  if (!myPreviewMode) {
    TPythonDump() << "stepVector = " << theStepVector;
  }
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = advancedExtrusion( theIDsOfElements,
                                                     theStepVector,
                                                     theNbOfSteps,
                                                     theExtrFlags,
                                                     theSewTolerance,
                                                     true);

  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".AdvancedExtrusionMakeGroups("
                << theIDsOfElements
                << ", stepVector, "
                << theNbOfSteps << ","
                << theExtrFlags << ", "
                << theSewTolerance <<  " )";
  }
  return aGroups;
}


//================================================================================
/*!
 * \brief Convert extrusion error to IDL enum
 */
//================================================================================

#define RETCASE(enm) case ::SMESH_MeshEditor::enm: return SMESH::SMESH_MeshEditor::enm;

static SMESH::SMESH_MeshEditor::Extrusion_Error convExtrError( const::SMESH_MeshEditor::Extrusion_Error e )
{
  switch ( e ) {
    RETCASE( EXTR_OK );
    RETCASE( EXTR_NO_ELEMENTS );
    RETCASE( EXTR_PATH_NOT_EDGE );
    RETCASE( EXTR_BAD_PATH_SHAPE );
    RETCASE( EXTR_BAD_STARTING_NODE );
    RETCASE( EXTR_BAD_ANGLES_NUMBER );
    RETCASE( EXTR_CANT_GET_TANGENT );
  }
  return SMESH::SMESH_MeshEditor::EXTR_OK;
}


//=======================================================================
//function : extrusionAlongPath
//purpose  :
//=======================================================================
SMESH::ListOfGroups*
SMESH_MeshEditor_i::extrusionAlongPath(const SMESH::long_array &   theIDsOfElements,
                                       SMESH::SMESH_Mesh_ptr       thePathMesh,
                                       GEOM::GEOM_Object_ptr       thePathShape,
                                       CORBA::Long                 theNodeStart,
                                       CORBA::Boolean              theHasAngles,
                                       const SMESH::double_array & theAngles,
                                       CORBA::Boolean              theHasRefPoint,
                                       const SMESH::PointStruct &  theRefPoint,
                                       const bool                  theMakeGroups,
                                       SMESH::SMESH_MeshEditor::Extrusion_Error & theError,
                                       const SMDSAbs_ElementType   theElementType)
{
  MESSAGE("extrusionAlongPath");
  initData();

  if ( thePathMesh->_is_nil() || thePathShape->_is_nil() ) {
    theError = SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;
    return 0;
  }
  SMESH_Mesh_i* aMeshImp = SMESH::DownCast<SMESH_Mesh_i*>( thePathMesh );

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( thePathShape );
  SMESH_subMesh* aSubMesh = aMeshImp->GetImpl().GetSubMesh( aShape );

  if ( !aSubMesh || !aSubMesh->GetSubMeshDS()) {
    theError = SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;
    return 0;
  }

  SMDS_MeshNode* nodeStart = (SMDS_MeshNode*)aMeshImp->GetImpl().GetMeshDS()->FindNode(theNodeStart);
  if ( !nodeStart ) {
    theError = SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE;
    return 0;
  }

  TIDSortedElemSet elements;
  arrayToSet(theIDsOfElements, GetMeshDS(), elements, theElementType);

  list<double> angles;
  for (int i = 0; i < theAngles.length(); i++) {
    angles.push_back( theAngles[i] );
  }

  gp_Pnt refPnt( theRefPoint.x, theRefPoint.y, theRefPoint.z );

  int nbOldGroups = myMesh->NbGroup();

  ::SMESH_MeshEditor anEditor( myMesh );
  ::SMESH_MeshEditor::Extrusion_Error error =
      anEditor.ExtrusionAlongTrack( elements, aSubMesh, nodeStart,
                                    theHasAngles, angles, false,
                                    theHasRefPoint, refPnt, theMakeGroups );
  myMesh->GetMeshDS()->Modified();
  storeResult(anEditor);
  theError = convExtrError( error );

  if ( theMakeGroups ) {
    list<int> groupIDs = myMesh->GetGroupIds();
    list<int>::iterator newBegin = groupIDs.begin();
    std::advance( newBegin, nbOldGroups ); // skip old groups
    groupIDs.erase( groupIDs.begin(), newBegin );
    return getGroups( & groupIDs );
  }
  return 0;
}


//=======================================================================
//function : extrusionAlongPathX
//purpose  :
//=======================================================================
SMESH::ListOfGroups*
SMESH_MeshEditor_i::extrusionAlongPathX(const SMESH::long_array &  IDsOfElements,
                                        SMESH::SMESH_IDSource_ptr  Path,
                                        CORBA::Long                NodeStart,
                                        CORBA::Boolean             HasAngles,
                                        const SMESH::double_array& Angles,
                                        CORBA::Boolean             LinearVariation,
                                        CORBA::Boolean             HasRefPoint,
                                        const SMESH::PointStruct&  RefPoint,
                                        bool                       MakeGroups,
                                        const SMDSAbs_ElementType  ElementType,
                                        SMESH::SMESH_MeshEditor::Extrusion_Error & Error)
{
  SMESH::ListOfGroups* EmptyGr = new SMESH::ListOfGroups;

  initData();

  list<double> angles;
  for (int i = 0; i < Angles.length(); i++) {
    angles.push_back( Angles[i] );
  }
  gp_Pnt refPnt( RefPoint.x, RefPoint.y, RefPoint.z );
  int nbOldGroups = myMesh->NbGroup();

  if ( Path->_is_nil() ) {
    Error = SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;
    return EmptyGr;
  }

  TIDSortedElemSet elements, copyElements;
  arrayToSet(IDsOfElements, GetMeshDS(), elements, ElementType);

  TIDSortedElemSet* workElements = &elements;
  TPreviewMesh      tmpMesh( SMDSAbs_Face );
  SMESH_Mesh*       mesh = myMesh;

  if ( myPreviewMode )
  {
    SMDSAbs_ElementType select = SMDSAbs_All, avoid = SMDSAbs_Volume;
    tmpMesh.Copy( elements, copyElements, select, avoid );
    mesh = &tmpMesh;
    workElements = & copyElements;
    MakeGroups = false;
  }

  ::SMESH_MeshEditor anEditor( mesh );
  ::SMESH_MeshEditor::Extrusion_Error error;

  if ( SMESH_Mesh_i* aMeshImp = SMESH::DownCast<SMESH_Mesh_i*>( Path ))
  {
    // path as mesh
    SMDS_MeshNode* aNodeStart =
      (SMDS_MeshNode*)aMeshImp->GetImpl().GetMeshDS()->FindNode(NodeStart);
    if ( !aNodeStart ) {
      Error = SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE;
      return EmptyGr;
    }
    error = anEditor.ExtrusionAlongTrack( *workElements, &(aMeshImp->GetImpl()), aNodeStart,
                                          HasAngles, angles, LinearVariation,
                                          HasRefPoint, refPnt, MakeGroups );
    myMesh->GetMeshDS()->Modified();
  }
  else if ( SMESH_subMesh_i* aSubMeshImp = SMESH::DownCast<SMESH_subMesh_i*>( Path ))
  {
    // path as submesh
    SMESH::SMESH_Mesh_ptr aPathMesh = aSubMeshImp->GetFather();
    aMeshImp = SMESH::DownCast<SMESH_Mesh_i*>( aPathMesh );
    SMDS_MeshNode* aNodeStart =
      (SMDS_MeshNode*)aMeshImp->GetImpl().GetMeshDS()->FindNode(NodeStart);
    if ( !aNodeStart ) {
      Error = SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE;
      return EmptyGr;
    }
    SMESH_subMesh* aSubMesh =
      aMeshImp->GetImpl().GetSubMeshContaining(aSubMeshImp->GetId());
    error = anEditor.ExtrusionAlongTrack( *workElements, aSubMesh, aNodeStart,
                                          HasAngles, angles, LinearVariation,
                                          HasRefPoint, refPnt, MakeGroups );
    myMesh->GetMeshDS()->Modified();
  }
  else if ( SMESH::DownCast<SMESH_Group_i*>( Path ))
  {
    // path as group of 1D elements
    // ????????
  }
  else
  {
    // invalid path
    Error = SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;
    return EmptyGr;
  }

  storeResult(anEditor);
  Error = convExtrError( error );

  if ( MakeGroups ) {
    list<int> groupIDs = myMesh->GetGroupIds();
    list<int>::iterator newBegin = groupIDs.begin();
    std::advance( newBegin, nbOldGroups ); // skip old groups
    groupIDs.erase( groupIDs.begin(), newBegin );
    return getGroups( & groupIDs );
  }
  return EmptyGr;
}


//=======================================================================
//function : ExtrusionAlongPath
//purpose  :
//=======================================================================
SMESH::SMESH_MeshEditor::Extrusion_Error
SMESH_MeshEditor_i::ExtrusionAlongPath(const SMESH::long_array &   theIDsOfElements,
                                       SMESH::SMESH_Mesh_ptr       thePathMesh,
                                       GEOM::GEOM_Object_ptr       thePathShape,
                                       CORBA::Long                 theNodeStart,
                                       CORBA::Boolean              theHasAngles,
                                       const SMESH::double_array & theAngles,
                                       CORBA::Boolean              theHasRefPoint,
                                       const SMESH::PointStruct &  theRefPoint)
{
  MESSAGE("ExtrusionAlongPath");
  if ( !myPreviewMode ) {
    TPythonDump() << "error = " << this << ".ExtrusionAlongPath( "
                  << theIDsOfElements << ", "
                  << thePathMesh      << ", "
                  << thePathShape     << ", "
                  << theNodeStart     << ", "
                  << theHasAngles     << ", "
                  << theAngles        << ", "
                  << theHasRefPoint   << ", "
                  << "SMESH.PointStruct( "
                  << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  SMESH::SMESH_MeshEditor::Extrusion_Error anError;
  extrusionAlongPath( theIDsOfElements,
                      thePathMesh,
                      thePathShape,
                      theNodeStart,
                      theHasAngles,
                      theAngles,
                      theHasRefPoint,
                      theRefPoint,
                      false,
                      anError);
  return anError;
}

//=======================================================================
//function : ExtrusionAlongPathObject
//purpose  :
//=======================================================================
SMESH::SMESH_MeshEditor::Extrusion_Error
SMESH_MeshEditor_i::ExtrusionAlongPathObject(SMESH::SMESH_IDSource_ptr   theObject,
                                             SMESH::SMESH_Mesh_ptr       thePathMesh,
                                             GEOM::GEOM_Object_ptr       thePathShape,
                                             CORBA::Long                 theNodeStart,
                                             CORBA::Boolean              theHasAngles,
                                             const SMESH::double_array & theAngles,
                                             CORBA::Boolean              theHasRefPoint,
                                             const SMESH::PointStruct &  theRefPoint)
{
  if ( !myPreviewMode ) {
    TPythonDump() << "error = " << this << ".ExtrusionAlongPathObject( "
                  << theObject        << ", "
                  << thePathMesh      << ", "
                  << thePathShape     << ", "
                  << theNodeStart     << ", "
                  << theHasAngles     << ", "
                  << theAngles        << ", "
                  << theHasRefPoint   << ", "
                  << "SMESH.PointStruct( "
                  << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  SMESH::SMESH_MeshEditor::Extrusion_Error anError;
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  extrusionAlongPath( anElementsId,
                      thePathMesh,
                      thePathShape,
                      theNodeStart,
                      theHasAngles,
                      theAngles,
                      theHasRefPoint,
                      theRefPoint,
                      false,
                      anError);
  return anError;
}

//=======================================================================
//function : ExtrusionAlongPathObject1D
//purpose  :
//=======================================================================
SMESH::SMESH_MeshEditor::Extrusion_Error
SMESH_MeshEditor_i::ExtrusionAlongPathObject1D(SMESH::SMESH_IDSource_ptr   theObject,
                                               SMESH::SMESH_Mesh_ptr       thePathMesh,
                                               GEOM::GEOM_Object_ptr       thePathShape,
                                               CORBA::Long                 theNodeStart,
                                               CORBA::Boolean              theHasAngles,
                                               const SMESH::double_array & theAngles,
                                               CORBA::Boolean              theHasRefPoint,
                                               const SMESH::PointStruct &  theRefPoint)
{
  if ( !myPreviewMode ) {
    TPythonDump() << "error = " << this << ".ExtrusionAlongPathObject1D( "
                  << theObject        << ", "
                  << thePathMesh      << ", "
                  << thePathShape     << ", "
                  << theNodeStart     << ", "
                  << theHasAngles     << ", "
                  << theAngles        << ", "
                  << theHasRefPoint   << ", "
                  << "SMESH.PointStruct( "
                  << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  SMESH::SMESH_MeshEditor::Extrusion_Error anError;
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  extrusionAlongPath( anElementsId,
                      thePathMesh,
                      thePathShape,
                      theNodeStart,
                      theHasAngles,
                      theAngles,
                      theHasRefPoint,
                      theRefPoint,
                      false,
                      anError,
                      SMDSAbs_Edge);
  return anError;
}

//=======================================================================
//function : ExtrusionAlongPathObject2D
//purpose  :
//=======================================================================
SMESH::SMESH_MeshEditor::Extrusion_Error
SMESH_MeshEditor_i::ExtrusionAlongPathObject2D(SMESH::SMESH_IDSource_ptr   theObject,
                                               SMESH::SMESH_Mesh_ptr       thePathMesh,
                                               GEOM::GEOM_Object_ptr       thePathShape,
                                               CORBA::Long                 theNodeStart,
                                               CORBA::Boolean              theHasAngles,
                                               const SMESH::double_array & theAngles,
                                               CORBA::Boolean              theHasRefPoint,
                                               const SMESH::PointStruct &  theRefPoint)
{
  if ( !myPreviewMode ) {
    TPythonDump() << "error = " << this << ".ExtrusionAlongPathObject2D( "
                  << theObject        << ", "
                  << thePathMesh      << ", "
                  << thePathShape     << ", "
                  << theNodeStart     << ", "
                  << theHasAngles     << ", "
                  << theAngles        << ", "
                  << theHasRefPoint   << ", "
                  << "SMESH.PointStruct( "
                  << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                  << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  SMESH::SMESH_MeshEditor::Extrusion_Error anError;
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  extrusionAlongPath( anElementsId,
                      thePathMesh,
                      thePathShape,
                      theNodeStart,
                      theHasAngles,
                      theAngles,
                      theHasRefPoint,
                      theRefPoint,
                      false,
                      anError,
                      SMDSAbs_Face);
  return anError;
}


//=======================================================================
//function : ExtrusionAlongPathMakeGroups
//purpose  :
//=======================================================================
SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionAlongPathMakeGroups(const SMESH::long_array&   theIDsOfElements,
                                                 SMESH::SMESH_Mesh_ptr      thePathMesh,
                                                 GEOM::GEOM_Object_ptr      thePathShape,
                                                 CORBA::Long                theNodeStart,
                                                 CORBA::Boolean             theHasAngles,
                                                 const SMESH::double_array& theAngles,
                                                 CORBA::Boolean             theHasRefPoint,
                                                 const SMESH::PointStruct&  theRefPoint,
                                                 SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups =  extrusionAlongPath( theIDsOfElements,
                                                       thePathMesh,
                                                       thePathShape,
                                                       theNodeStart,
                                                       theHasAngles,
                                                       theAngles,
                                                       theHasRefPoint,
                                                       theRefPoint,
                                                       true,
                                                       Error);
  if (!myPreviewMode) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    if (isDumpGroups)
      aPythonDump << "(" << aGroups << ", error)";
    else
      aPythonDump <<"error";

    aPythonDump<<" = "<< this << ".ExtrusionAlongPathMakeGroups( "
               << theIDsOfElements << ", "
               << thePathMesh      << ", "
               << thePathShape     << ", "
               << theNodeStart     << ", "
               << theHasAngles     << ", "
               << theAngles        << ", "
               << theHasRefPoint   << ", "
               << "SMESH.PointStruct( "
               << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
               << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
               << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionAlongPathObjectMakeGroups
//purpose  :
//=======================================================================
SMESH::ListOfGroups* SMESH_MeshEditor_i::
ExtrusionAlongPathObjectMakeGroups(SMESH::SMESH_IDSource_ptr  theObject,
                                   SMESH::SMESH_Mesh_ptr      thePathMesh,
                                   GEOM::GEOM_Object_ptr      thePathShape,
                                   CORBA::Long                theNodeStart,
                                   CORBA::Boolean             theHasAngles,
                                   const SMESH::double_array& theAngles,
                                   CORBA::Boolean             theHasRefPoint,
                                   const SMESH::PointStruct&  theRefPoint,
                                   SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionAlongPath( anElementsId,
                                                      thePathMesh,
                                                      thePathShape,
                                                      theNodeStart,
                                                      theHasAngles,
                                                      theAngles,
                                                      theHasRefPoint,
                                                      theRefPoint,
                                                      true,
                                                      Error);

  if (!myPreviewMode) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    if (isDumpGroups)
      aPythonDump << "(" << aGroups << ", error)";
    else
      aPythonDump <<"error";

    aPythonDump << " = " << this << ".ExtrusionAlongPathObjectMakeGroups( "
                << theObject << ", "
                << thePathMesh      << ", "
                << thePathShape     << ", "
                << theNodeStart     << ", "
                << theHasAngles     << ", "
                << theAngles        << ", "
                << theHasRefPoint   << ", "
                << "SMESH.PointStruct( "
                << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionAlongPathObject1DMakeGroups
//purpose  :
//=======================================================================
SMESH::ListOfGroups* SMESH_MeshEditor_i::
ExtrusionAlongPathObject1DMakeGroups(SMESH::SMESH_IDSource_ptr  theObject,
                                     SMESH::SMESH_Mesh_ptr      thePathMesh,
                                     GEOM::GEOM_Object_ptr      thePathShape,
                                     CORBA::Long                theNodeStart,
                                     CORBA::Boolean             theHasAngles,
                                     const SMESH::double_array& theAngles,
                                     CORBA::Boolean             theHasRefPoint,
                                     const SMESH::PointStruct&  theRefPoint,
                                     SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionAlongPath( anElementsId,
                                                      thePathMesh,
                                                      thePathShape,
                                                      theNodeStart,
                                                      theHasAngles,
                                                      theAngles,
                                                      theHasRefPoint,
                                                      theRefPoint,
                                                      true,
                                                      Error,
                                                      SMDSAbs_Edge);

  if (!myPreviewMode) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    if (isDumpGroups)
      aPythonDump << "(" << aGroups << ", error)";
    else
      aPythonDump << "error";

    aPythonDump << " = " << this << ".ExtrusionAlongPathObject1DMakeGroups( "
                << theObject << ", "
                << thePathMesh      << ", "
                << thePathShape     << ", "
                << theNodeStart     << ", "
                << theHasAngles     << ", "
                << theAngles        << ", "
                << theHasRefPoint   << ", "
                << "SMESH.PointStruct( "
                << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  return aGroups;
}

//=======================================================================
//function : ExtrusionAlongPathObject2DMakeGroups
//purpose  :
//=======================================================================
SMESH::ListOfGroups* SMESH_MeshEditor_i::
ExtrusionAlongPathObject2DMakeGroups(SMESH::SMESH_IDSource_ptr  theObject,
                                     SMESH::SMESH_Mesh_ptr      thePathMesh,
                                     GEOM::GEOM_Object_ptr      thePathShape,
                                     CORBA::Long                theNodeStart,
                                     CORBA::Boolean             theHasAngles,
                                     const SMESH::double_array& theAngles,
                                     CORBA::Boolean             theHasRefPoint,
                                     const SMESH::PointStruct&  theRefPoint,
                                     SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionAlongPath( anElementsId,
                                                      thePathMesh,
                                                      thePathShape,
                                                      theNodeStart,
                                                      theHasAngles,
                                                      theAngles,
                                                      theHasRefPoint,
                                                      theRefPoint,
                                                      true,
                                                      Error,
                                                      SMDSAbs_Face);

  if (!myPreviewMode) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    if (isDumpGroups)
      aPythonDump << "(" << aGroups << ", error)";
    else
      aPythonDump << "error";

    aPythonDump << " = " << this << ".ExtrusionAlongPathObject2DMakeGroups( "
                << theObject << ", "
                << thePathMesh      << ", "
                << thePathShape     << ", "
                << theNodeStart     << ", "
                << theHasAngles     << ", "
                << theAngles        << ", "
                << theHasRefPoint   << ", "
                << "SMESH.PointStruct( "
                << ( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                << ( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                << ( theHasRefPoint ? theRefPoint.z : 0 ) << " ) )";
  }
  return aGroups;
}


//=======================================================================
//function : ExtrusionAlongPathObjX
//purpose  :
//=======================================================================
SMESH::ListOfGroups* SMESH_MeshEditor_i::
ExtrusionAlongPathObjX(SMESH::SMESH_IDSource_ptr  Object,
                       SMESH::SMESH_IDSource_ptr  Path,
                       CORBA::Long                NodeStart,
                       CORBA::Boolean             HasAngles,
                       const SMESH::double_array& Angles,
                       CORBA::Boolean             LinearVariation,
                       CORBA::Boolean             HasRefPoint,
                       const SMESH::PointStruct&  RefPoint,
                       CORBA::Boolean             MakeGroups,
                       SMESH::ElementType         ElemType,
                       SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::long_array_var anElementsId = Object->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionAlongPathX(anElementsId,
                                                      Path,
                                                      NodeStart,
                                                      HasAngles,
                                                      Angles,
                                                      LinearVariation,
                                                      HasRefPoint,
                                                      RefPoint,
                                                      MakeGroups,
                                                      (SMDSAbs_ElementType)ElemType,
                                                      Error);

  if (!myPreviewMode) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    if (isDumpGroups)
      aPythonDump << "(" << *aGroups << ", error)";
    else
      aPythonDump << "error";

    aPythonDump << " = " << this << ".ExtrusionAlongPathObjX( "
                << Object          << ", "
                << Path            << ", "
                << NodeStart       << ", "
                << HasAngles       << ", "
                << TVar( Angles )  << ", "
                << LinearVariation << ", "
                << HasRefPoint     << ", "
                << "SMESH.PointStruct( "
                << TVar( HasRefPoint ? RefPoint.x : 0 ) << ", "
                << TVar( HasRefPoint ? RefPoint.y : 0 ) << ", "
                << TVar( HasRefPoint ? RefPoint.z : 0 ) << " ), "
                << MakeGroups << ", "
                << ElemType << " )";
  }
  return aGroups;
}


//=======================================================================
//function : ExtrusionAlongPathX
//purpose  :
//=======================================================================
SMESH::ListOfGroups* SMESH_MeshEditor_i::
ExtrusionAlongPathX(const SMESH::long_array&   IDsOfElements,
                    SMESH::SMESH_IDSource_ptr  Path,
                    CORBA::Long                NodeStart,
                    CORBA::Boolean             HasAngles,
                    const SMESH::double_array& Angles,
                    CORBA::Boolean             LinearVariation,
                    CORBA::Boolean             HasRefPoint,
                    const SMESH::PointStruct&  RefPoint,
                    CORBA::Boolean             MakeGroups,
                    SMESH::ElementType         ElemType,
                    SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = extrusionAlongPathX(IDsOfElements,
                                                      Path,
                                                      NodeStart,
                                                      HasAngles,
                                                      Angles,
                                                      LinearVariation,
                                                      HasRefPoint,
                                                      RefPoint,
                                                      MakeGroups,
                                                      (SMDSAbs_ElementType)ElemType,
                                                      Error);

  if (!myPreviewMode) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    if (isDumpGroups)
      aPythonDump << "(" << *aGroups << ", error)";
    else
      aPythonDump <<"error";

    aPythonDump << " = " << this << ".ExtrusionAlongPathX( "
                << IDsOfElements   << ", "
                << Path            << ", "
                << NodeStart       << ", "
                << HasAngles       << ", "
                << TVar( Angles )  << ", "
                << LinearVariation << ", "
                << HasRefPoint     << ", "
                << "SMESH.PointStruct( "
                << TVar( HasRefPoint ? RefPoint.x : 0 ) << ", "
                << TVar( HasRefPoint ? RefPoint.y : 0 ) << ", "
                << TVar( HasRefPoint ? RefPoint.z : 0 ) << " ), "
                << MakeGroups << ", "
                << ElemType << " )";
  }
  return aGroups;
}


//================================================================================
/*!
 * \brief Compute rotation angles for ExtrusionAlongPath as linear variation
 * of given angles along path steps
 * \param PathMesh mesh containing a 1D sub-mesh on the edge, along
 *                which proceeds the extrusion
 * \param PathShape is shape(edge); as the mesh can be complex, the edge
 *                 is used to define the sub-mesh for the path
 */
//================================================================================

SMESH::double_array*
SMESH_MeshEditor_i::LinearAnglesVariation(SMESH::SMESH_Mesh_ptr       thePathMesh,
                                          GEOM::GEOM_Object_ptr       thePathShape,
                                          const SMESH::double_array & theAngles)
{
  SMESH::double_array_var aResult = new SMESH::double_array();
  int nbAngles = theAngles.length();
  if ( nbAngles > 0 && !thePathMesh->_is_nil() && !thePathShape->_is_nil() )
  {
    SMESH_Mesh_i* aMeshImp = SMESH::DownCast<SMESH_Mesh_i*>( thePathMesh );
    TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( thePathShape );
    SMESH_subMesh* aSubMesh = aMeshImp->GetImpl().GetSubMesh( aShape );
    if ( !aSubMesh || !aSubMesh->GetSubMeshDS())
      return aResult._retn();
    int nbSteps = aSubMesh->GetSubMeshDS()->NbElements();
    if ( nbSteps == nbAngles )
    {
      aResult.inout() = theAngles;
    }
    else
    {
      aResult->length( nbSteps );
      double rAn2St = double( nbAngles ) / double( nbSteps );
      double angPrev = 0, angle;
      for ( int iSt = 0; iSt < nbSteps; ++iSt )
      {
        double angCur = rAn2St * ( iSt+1 );
        double angCurFloor  = floor( angCur );
        double angPrevFloor = floor( angPrev );
        if ( angPrevFloor == angCurFloor )
          angle = rAn2St * theAngles[ int( angCurFloor ) ];
        else
        {
          int iP = int( angPrevFloor );
          double angPrevCeil = ceil(angPrev);
          angle = ( angPrevCeil - angPrev ) * theAngles[ iP ];

          int iC = int( angCurFloor );
          if ( iC < nbAngles )
            angle += ( angCur - angCurFloor ) * theAngles[ iC ];

          iP = int( angPrevCeil );
          while ( iC-- > iP )
            angle += theAngles[ iC ];
        }
        aResult[ iSt ] = angle;
        angPrev = angCur;
      }
    }
  }
  // Update Python script
  TPythonDump() << "rotAngles = " << theAngles;
  TPythonDump() << "rotAngles = " << this << ".LinearAnglesVariation( "
                << thePathMesh  << ", "
                << thePathShape << ", "
                << "rotAngles )";

  return aResult._retn();
}


//=======================================================================
//function : mirror
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::mirror(TIDSortedElemSet &                  theElements,
                           const SMESH::AxisStruct &           theAxis,
                           SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                           CORBA::Boolean                      theCopy,
                           bool                                theMakeGroups,
                           ::SMESH_Mesh*                       theTargetMesh)
{
  initData();

  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  if ( theTargetMesh )
    theCopy = false;

  gp_Trsf aTrsf;
  switch ( theMirrorType ) {
  case  SMESH::SMESH_MeshEditor::POINT:
    aTrsf.SetMirror( P );
    break;
  case  SMESH::SMESH_MeshEditor::AXIS:
    aTrsf.SetMirror( gp_Ax1( P, V ));
    break;
  default:
    aTrsf.SetMirror( gp_Ax2( P, V ));
  }

  TIDSortedElemSet  copyElements;
  TPreviewMesh      tmpMesh;
  TIDSortedElemSet* workElements = & theElements;
  SMESH_Mesh*       mesh = myMesh;

  if ( myPreviewMode )
  {
    tmpMesh.Copy( theElements, copyElements);
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( theElements, GetMeshDS(), elemsAround );
      tmpMesh.Copy( elemsAround, elemsAroundCopy);
    }
    mesh = &tmpMesh;
    workElements = & copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor anEditor( mesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if(theCopy || myPreviewMode)
    storeResult(anEditor); // store preview data or new elements

  if ( !myPreviewMode )
  {
    if ( theTargetMesh )
    {
      theTargetMesh->GetMeshDS()->Modified();
    }
    else
    {
      myMesh->GetMeshDS()->Modified();
      myMesh->SetIsModified( true );
    }
  }
  return theMakeGroups ? getGroups(groupIds.get()) : 0;
}

//=======================================================================
//function : Mirror
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Mirror(const SMESH::long_array &           theIDsOfElements,
                                const SMESH::AxisStruct &           theAxis,
                                SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                                CORBA::Boolean                      theCopy)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".Mirror( "
                  << theIDsOfElements              << ", "
                  << theAxis                       << ", "
                  << mirrorTypeName(theMirrorType) << ", "
                  << theCopy                       << " )";
  }
  if ( theIDsOfElements.length() > 0 )
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements);
    mirror(elements, theAxis, theMirrorType, theCopy, false);
  }
}


//=======================================================================
//function : MirrorObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MirrorObject(SMESH::SMESH_IDSource_ptr           theObject,
                                      const SMESH::AxisStruct &           theAxis,
                                      SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                                      CORBA::Boolean                      theCopy)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".MirrorObject( "
                  << theObject                     << ", "
                  << theAxis                       << ", "
                  << mirrorTypeName(theMirrorType) << ", "
                  << theCopy                       << " )";
  }
  TIDSortedElemSet elements;

  bool emptyIfIsMesh = myPreviewMode ? false : true;

  if (idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
    mirror(elements, theAxis, theMirrorType, theCopy, false);
}

//=======================================================================
//function : MirrorMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::MirrorMakeGroups(const SMESH::long_array&            theIDsOfElements,
                                     const SMESH::AxisStruct&            theMirror,
                                     SMESH::SMESH_MeshEditor::MirrorType theMirrorType)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  if ( theIDsOfElements.length() > 0 )
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements);
    aGroups = mirror(elements, theMirror, theMirrorType, true, true);
  }
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".MirrorMakeGroups( "
                << theIDsOfElements              << ", "
                << theMirror                     << ", "
                << mirrorTypeName(theMirrorType) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : MirrorObjectMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::MirrorObjectMakeGroups(SMESH::SMESH_IDSource_ptr           theObject,
                                           const SMESH::AxisStruct&            theMirror,
                                           SMESH::SMESH_MeshEditor::MirrorType theMirrorType)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  TIDSortedElemSet elements;
  if ( idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    aGroups = mirror(elements, theMirror, theMirrorType, true, true);

  if (!myPreviewMode)
  {
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".MirrorObjectMakeGroups( "
                << theObject                     << ", "
                << theMirror                     << ", "
                << mirrorTypeName(theMirrorType) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : MirrorMakeMesh
//purpose  :
//=======================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::MirrorMakeMesh(const SMESH::long_array&            theIDsOfElements,
                                   const SMESH::AxisStruct&            theMirror,
                                   SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                                   CORBA::Boolean                      theCopyGroups,
                                   const char*                         theMeshName)
{
  SMESH_Mesh_i* mesh_i;
  SMESH::SMESH_Mesh_var mesh;
  { // open new scope to dump "MakeMesh" command
    // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation

    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );
    if (mesh_i && theIDsOfElements.length() > 0 )
    {
      TIDSortedElemSet elements;
      arrayToSet(theIDsOfElements, GetMeshDS(), elements);
      mirror(elements, theMirror, theMirrorType,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }

    if (!myPreviewMode) {
      pydump << mesh << " = " << this << ".MirrorMakeMesh( "
             << theIDsOfElements              << ", "
             << theMirror                     << ", "
             << mirrorTypeName(theMirrorType) << ", "
             << theCopyGroups                 << ", '"
             << theMeshName                   << "' )";
    }
  }

  //dump "GetGroups"
  if (!myPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();
}

//=======================================================================
//function : MirrorObjectMakeMesh
//purpose  :
//=======================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::MirrorObjectMakeMesh(SMESH::SMESH_IDSource_ptr           theObject,
                                         const SMESH::AxisStruct&            theMirror,
                                         SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                                         CORBA::Boolean                      theCopyGroups,
                                         const char*                         theMeshName)
{
  SMESH_Mesh_i* mesh_i;
  SMESH::SMESH_Mesh_var mesh;
  { // open new scope to dump "MakeMesh" command
    // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation

    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );
    TIDSortedElemSet elements;
    if ( mesh_i &&
         idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    {
      mirror(elements, theMirror, theMirrorType,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if (!myPreviewMode) {
      pydump << mesh << " = " << this << ".MirrorObjectMakeMesh( "
             << theObject                     << ", "
             << theMirror                     << ", "
             << mirrorTypeName(theMirrorType) << ", "
             << theCopyGroups                 << ", '"
             << theMeshName                   << "' )";
    }
  }

  //dump "GetGroups"
  if (!myPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();
}

//=======================================================================
//function : translate
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::translate(TIDSortedElemSet        & theElements,
                              const SMESH::DirStruct &  theVector,
                              CORBA::Boolean            theCopy,
                              bool                      theMakeGroups,
                              ::SMESH_Mesh*             theTargetMesh)
{
  initData();

  if ( theTargetMesh )
    theCopy = false;

  gp_Trsf aTrsf;
  const SMESH::PointStruct * P = &theVector.PS;
  aTrsf.SetTranslation( gp_Vec( P->x, P->y, P->z ));

  TIDSortedElemSet  copyElements;
  TIDSortedElemSet* workElements = &theElements;
  TPreviewMesh      tmpMesh;
  SMESH_Mesh*       mesh = myMesh;

  if ( myPreviewMode )
  {
    tmpMesh.Copy( theElements, copyElements);
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( theElements, GetMeshDS(), elemsAround );
      tmpMesh.Copy( elemsAround, elemsAroundCopy);
    }
    mesh = &tmpMesh;
    workElements = & copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor anEditor( mesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if(theCopy || myPreviewMode)
    storeResult(anEditor);

  if ( !myPreviewMode )
  {
    if ( theTargetMesh )
    {
      theTargetMesh->GetMeshDS()->Modified();
    }
    else
    {
      myMesh->GetMeshDS()->Modified();
      myMesh->SetIsModified( true );
    }
  }

  return theMakeGroups ? getGroups(groupIds.get()) : 0;
}

//=======================================================================
//function : Translate
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Translate(const SMESH::long_array & theIDsOfElements,
                                   const SMESH::DirStruct &  theVector,
                                   CORBA::Boolean            theCopy)
{
  if (!myPreviewMode) {
    TPythonDump() << this << ".Translate( "
                  << theIDsOfElements << ", "
                  << theVector        << ", "
                  << theCopy          << " )";
  }
  if (theIDsOfElements.length()) {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements);
    translate(elements, theVector, theCopy, false);
  }
}

//=======================================================================
//function : TranslateObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::TranslateObject(SMESH::SMESH_IDSource_ptr theObject,
                                         const SMESH::DirStruct &  theVector,
                                         CORBA::Boolean            theCopy)
{
  if (!myPreviewMode) {
    TPythonDump() << this << ".TranslateObject( "
                  << theObject << ", "
                  << theVector << ", "
                  << theCopy   << " )";
  }
  TIDSortedElemSet elements;

  bool emptyIfIsMesh = myPreviewMode ? false : true;
  
  if (idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
    translate(elements, theVector, theCopy, false);
}

//=======================================================================
//function : TranslateMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::TranslateMakeGroups(const SMESH::long_array& theIDsOfElements,
                                        const SMESH::DirStruct&  theVector)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  if (theIDsOfElements.length()) {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements);
    aGroups = translate(elements,theVector,true,true);
  }
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".TranslateMakeGroups( "
                << theIDsOfElements << ", "
                << theVector        << " )";
  }
  return aGroups;
}

//=======================================================================
//function : TranslateObjectMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::TranslateObjectMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                              const SMESH::DirStruct&   theVector)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  TIDSortedElemSet elements;
  if (idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    aGroups = translate(elements, theVector, true, true);

  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".TranslateObjectMakeGroups( "
                << theObject << ", "
                << theVector << " )";
  }
  return aGroups;
}

//=======================================================================
//function : TranslateMakeMesh
//purpose  :
//=======================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::TranslateMakeMesh(const SMESH::long_array& theIDsOfElements,
                                      const SMESH::DirStruct&  theVector,
                                      CORBA::Boolean           theCopyGroups,
                                      const char*              theMeshName)
{
  SMESH_Mesh_i* mesh_i;
  SMESH::SMESH_Mesh_var mesh;

  { // open new scope to dump "MakeMesh" command
    // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation

    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

    if ( mesh_i && theIDsOfElements.length() )
    {
      TIDSortedElemSet elements;
      arrayToSet(theIDsOfElements, GetMeshDS(), elements);
      translate(elements, theVector, false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }

    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".TranslateMakeMesh( "
             << theIDsOfElements << ", "
             << theVector        << ", "
             << theCopyGroups    << ", '"
             << theMeshName      << "' )";
    }
  }

  //dump "GetGroups"
  if (!myPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();
}

//=======================================================================
//function : TranslateObjectMakeMesh
//purpose  :
//=======================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::TranslateObjectMakeMesh(SMESH::SMESH_IDSource_ptr theObject,
                                            const SMESH::DirStruct&   theVector,
                                            CORBA::Boolean            theCopyGroups,
                                            const char*               theMeshName)
{
  SMESH_Mesh_i* mesh_i;
  SMESH::SMESH_Mesh_var mesh;
  { // open new scope to dump "MakeMesh" command
    // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation
    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

    TIDSortedElemSet elements;
    if ( mesh_i &&
      idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    {
      translate(elements, theVector,false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".TranslateObjectMakeMesh( "
             << theObject     << ", "
             << theVector     << ", "
             << theCopyGroups << ", '"
             << theMeshName   << "' )";
    }
  }

  // dump "GetGroups"
  if (!myPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();
}

//=======================================================================
//function : rotate
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::rotate(TIDSortedElemSet &        theElements,
                           const SMESH::AxisStruct & theAxis,
                           CORBA::Double             theAngle,
                           CORBA::Boolean            theCopy,
                           bool                      theMakeGroups,
                           ::SMESH_Mesh*             theTargetMesh)
{
  initData();

  if ( theTargetMesh )
    theCopy = false;

  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  gp_Trsf aTrsf;
  aTrsf.SetRotation( gp_Ax1( P, V ), theAngle);

  TIDSortedElemSet  copyElements;
  TIDSortedElemSet* workElements = &theElements;
  TPreviewMesh      tmpMesh;
  SMESH_Mesh*       mesh = myMesh;

  if ( myPreviewMode ) {
    tmpMesh.Copy( theElements, copyElements );
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( theElements, GetMeshDS(), elemsAround );
      tmpMesh.Copy( elemsAround, elemsAroundCopy);
    }
    mesh = &tmpMesh;
    workElements = &copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor anEditor( mesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if(theCopy || myPreviewMode)
    storeResult(anEditor);

  if ( !myPreviewMode )
  {
    if ( theTargetMesh )
    {
      theTargetMesh->GetMeshDS()->Modified();
    }
    else
    {
      myMesh->GetMeshDS()->Modified();
      myMesh->SetIsModified( true );
    }
  }

  return theMakeGroups ? getGroups(groupIds.get()) : 0;
}

//=======================================================================
//function : Rotate
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Rotate(const SMESH::long_array & theIDsOfElements,
                                const SMESH::AxisStruct & theAxis,
                                CORBA::Double             theAngle,
                                CORBA::Boolean            theCopy)
{
  if (!myPreviewMode) {
    TPythonDump() << this << ".Rotate( "
                  << theIDsOfElements << ", "
                  << theAxis          << ", "
                  << TVar( theAngle ) << ", "
                  << theCopy          << " )";
  }
  if (theIDsOfElements.length() > 0)
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements);
    rotate(elements,theAxis,theAngle,theCopy,false);
  }
}

//=======================================================================
//function : RotateObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotateObject(SMESH::SMESH_IDSource_ptr theObject,
                                      const SMESH::AxisStruct & theAxis,
                                      CORBA::Double             theAngle,
                                      CORBA::Boolean            theCopy)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".RotateObject( "
                  << theObject        << ", "
                  << theAxis          << ", "
                  << TVar( theAngle ) << ", "
                  << theCopy          << " )";
  }
  TIDSortedElemSet elements;
  bool emptyIfIsMesh = myPreviewMode ? false : true;
  if (idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
    rotate(elements,theAxis,theAngle,theCopy,false);
}

//=======================================================================
//function : RotateMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::RotateMakeGroups(const SMESH::long_array& theIDsOfElements,
                                     const SMESH::AxisStruct& theAxis,
                                     CORBA::Double            theAngle)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  if (theIDsOfElements.length() > 0)
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements);
    aGroups = rotate(elements,theAxis,theAngle,true,true);
  }
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".RotateMakeGroups( "
                << theIDsOfElements << ", "
                << theAxis          << ", "
                << TVar( theAngle ) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : RotateObjectMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::RotateObjectMakeGroups(SMESH::SMESH_IDSource_ptr theObject,
                                           const SMESH::AxisStruct&  theAxis,
                                           CORBA::Double             theAngle)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  TIDSortedElemSet elements;
  if (idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    aGroups = rotate(elements, theAxis, theAngle, true, true);

  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".RotateObjectMakeGroups( "
                << theObject        << ", "
                << theAxis          << ", "
                << TVar( theAngle ) << " )";
  }
  return aGroups;
}

//=======================================================================
//function : RotateMakeMesh
//purpose  :
//=======================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::RotateMakeMesh(const SMESH::long_array& theIDsOfElements,
                                   const SMESH::AxisStruct& theAxis,
                                   CORBA::Double            theAngleInRadians,
                                   CORBA::Boolean           theCopyGroups,
                                   const char*              theMeshName)
{
  SMESH::SMESH_Mesh_var mesh;
  SMESH_Mesh_i* mesh_i;

  { // open new scope to dump "MakeMesh" command
    // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation

    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

    if ( mesh_i && theIDsOfElements.length() > 0 )
    {
      TIDSortedElemSet elements;
      arrayToSet(theIDsOfElements, GetMeshDS(), elements);
      rotate(elements, theAxis, theAngleInRadians,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".RotateMakeMesh( "
             << theIDsOfElements          << ", "
             << theAxis                   << ", "
             << TVar( theAngleInRadians ) << ", "
             << theCopyGroups             << ", '"
             << theMeshName               << "' )";
    }
  }

  // dump "GetGroups"
  if (!myPreviewMode && mesh_i && theIDsOfElements.length() > 0 )
    mesh_i->GetGroups();

  return mesh._retn();
}

//=======================================================================
//function : RotateObjectMakeMesh
//purpose  :
//=======================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::RotateObjectMakeMesh(SMESH::SMESH_IDSource_ptr theObject,
                                         const SMESH::AxisStruct&  theAxis,
                                         CORBA::Double             theAngleInRadians,
                                         CORBA::Boolean            theCopyGroups,
                                         const char*               theMeshName)
{
  SMESH::SMESH_Mesh_var mesh;
  SMESH_Mesh_i* mesh_i;

  {// open new scope to dump "MakeMesh" command
   // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation
    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

    TIDSortedElemSet elements;
    if (mesh_i &&
        idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    {
      rotate(elements, theAxis, theAngleInRadians,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".RotateObjectMakeMesh( "
             << theObject                 << ", "
             << theAxis                   << ", "
             << TVar( theAngleInRadians ) << ", "
             << theCopyGroups             << ", '"
             << theMeshName               << "' )";
    }
  }

  // dump "GetGroups"
  if (!myPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();
}

//=======================================================================
//function : scale
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::scale(SMESH::SMESH_IDSource_ptr  theObject,
                          const SMESH::PointStruct&  thePoint,
                          const SMESH::double_array& theScaleFact,
                          CORBA::Boolean             theCopy,
                          bool                       theMakeGroups,
                          ::SMESH_Mesh*              theTargetMesh)
{
  initData();
  if ( theScaleFact.length() < 1 )
    THROW_SALOME_CORBA_EXCEPTION("Scale factor not given", SALOME::BAD_PARAM);
  if ( theScaleFact.length() == 2 )
    THROW_SALOME_CORBA_EXCEPTION("Invalid nb of scale factors : 2", SALOME::BAD_PARAM);

  if ( theTargetMesh )
    theCopy = false;

  TIDSortedElemSet elements;
  bool emptyIfIsMesh = myPreviewMode ? false : true;
  if ( !idSourceToSet(theObject, GetMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
    return 0;

  double S[3] = {
    theScaleFact[0],
    (theScaleFact.length() == 1) ? theScaleFact[0] : theScaleFact[1],
    (theScaleFact.length() == 1) ? theScaleFact[0] : theScaleFact[2],
  };
  double tol = std::numeric_limits<double>::max();
  gp_Trsf aTrsf;
  aTrsf.SetValues( S[0], 0,    0,    thePoint.x * (1-S[0]),
                   0,    S[1], 0,    thePoint.y * (1-S[1]),
                   0,    0,    S[2], thePoint.z * (1-S[2]),   tol, tol);

  TIDSortedElemSet  copyElements;
  TPreviewMesh      tmpMesh;
  TIDSortedElemSet* workElements = &elements;
  SMESH_Mesh*       mesh = myMesh;
  
  if ( myPreviewMode )
  {
    tmpMesh.Copy( elements, copyElements);
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( elements, GetMeshDS(), elemsAround );
      tmpMesh.Copy( elemsAround, elemsAroundCopy);
    }
    mesh = &tmpMesh;
    workElements = & copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor anEditor( mesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if(theCopy || myPreviewMode )
    storeResult(anEditor);

  if ( !myPreviewMode )
  {
    if ( theTargetMesh )
    {
      theTargetMesh->GetMeshDS()->Modified();
    }
    else
    {
      myMesh->GetMeshDS()->Modified();
      myMesh->SetIsModified( true );
    }
  }

  return theMakeGroups ? getGroups(groupIds.get()) : 0;
}

//=======================================================================
//function : Scale
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Scale(SMESH::SMESH_IDSource_ptr  theObject,
                               const SMESH::PointStruct&  thePoint,
                               const SMESH::double_array& theScaleFact,
                               CORBA::Boolean             theCopy)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".Scale( "
                  << theObject            << ", "
                  << thePoint             << ", "
                  << TVar( theScaleFact ) << ", "
                  << theCopy              << " )";
  }
  scale(theObject, thePoint, theScaleFact, theCopy, false);
}


//=======================================================================
//function : ScaleMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ScaleMakeGroups(SMESH::SMESH_IDSource_ptr  theObject,
                                    const SMESH::PointStruct&  thePoint,
                                    const SMESH::double_array& theScaleFact)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = scale(theObject, thePoint, theScaleFact, true, true);
  if (!myPreviewMode) {
    DumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".Scale("
                << theObject            << ","
                << thePoint             << ","
                << TVar( theScaleFact ) << ",True,True)";
  }
  return aGroups;
}


//=======================================================================
//function : ScaleMakeMesh
//purpose  :
//=======================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::ScaleMakeMesh(SMESH::SMESH_IDSource_ptr  theObject,
                                  const SMESH::PointStruct&  thePoint,
                                  const SMESH::double_array& theScaleFact,
                                  CORBA::Boolean             theCopyGroups,
                                  const char*                theMeshName)
{
  SMESH_Mesh_i* mesh_i;
  SMESH::SMESH_Mesh_var mesh;
  { // open new scope to dump "MakeMesh" command
    // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation
    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

    if ( mesh_i )
    {
      scale(theObject, thePoint, theScaleFact,false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myPreviewMode )
      pydump << mesh << " = " << this << ".ScaleMakeMesh( "
             << theObject            << ", "
             << thePoint             << ", "
             << TVar( theScaleFact ) << ", "
             << theCopyGroups        << ", '"
             << theMeshName          << "' )";
  }

  // dump "GetGroups"
  if (!myPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();
}


//=======================================================================
//function : FindCoincidentNodes
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::FindCoincidentNodes (CORBA::Double                  Tolerance,
                                              SMESH::array_of_long_array_out GroupsOfNodes)
{
  initData();

  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  ::SMESH_MeshEditor anEditor( myMesh );
  TIDSortedNodeSet nodes; // no input nodes
  anEditor.FindCoincidentNodes( nodes, Tolerance, aListOfListOfNodes );

  GroupsOfNodes = new SMESH::array_of_long_array;
  GroupsOfNodes->length( aListOfListOfNodes.size() );
  ::SMESH_MeshEditor::TListOfListOfNodes::iterator llIt = aListOfListOfNodes.begin();
  for ( CORBA::Long i = 0; llIt != aListOfListOfNodes.end(); llIt++, i++ ) {
    list< const SMDS_MeshNode* >& aListOfNodes = *llIt;
    list< const SMDS_MeshNode* >::iterator lIt = aListOfNodes.begin();;
    SMESH::long_array& aGroup = (*GroupsOfNodes)[ i ];
    aGroup.length( aListOfNodes.size() );
    for ( int j = 0; lIt != aListOfNodes.end(); lIt++, j++ )
      aGroup[ j ] = (*lIt)->GetID();
  }
  TPythonDump() << "coincident_nodes = " << this << ".FindCoincidentNodes( "
                << Tolerance << " )";
}

//=======================================================================
//function : FindCoincidentNodesOnPart
//purpose  :
//=======================================================================
void SMESH_MeshEditor_i::FindCoincidentNodesOnPart(SMESH::SMESH_IDSource_ptr      theObject,
                                                   CORBA::Double                  Tolerance,
                                                   SMESH::array_of_long_array_out GroupsOfNodes)
{
  initData();

  TIDSortedNodeSet nodes;
  idSourceToNodeSet( theObject, GetMeshDS(), nodes );

  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  ::SMESH_MeshEditor anEditor( myMesh );
  if(!nodes.empty())
    anEditor.FindCoincidentNodes( nodes, Tolerance, aListOfListOfNodes );

  GroupsOfNodes = new SMESH::array_of_long_array;
  GroupsOfNodes->length( aListOfListOfNodes.size() );
  ::SMESH_MeshEditor::TListOfListOfNodes::iterator llIt = aListOfListOfNodes.begin();
  for ( CORBA::Long i = 0; llIt != aListOfListOfNodes.end(); llIt++, i++ )
  {
    list< const SMDS_MeshNode* >& aListOfNodes = *llIt;
    list< const SMDS_MeshNode* >::iterator lIt = aListOfNodes.begin();;
    SMESH::long_array& aGroup = (*GroupsOfNodes)[ i ];
    aGroup.length( aListOfNodes.size() );
    for ( int j = 0; lIt != aListOfNodes.end(); lIt++, j++ )
      aGroup[ j ] = (*lIt)->GetID();
  }
  TPythonDump() << "coincident_nodes_on_part = " << this << ".FindCoincidentNodesOnPart( "
                <<theObject<<", "
                << Tolerance << " )";
}

//================================================================================
/*!
 * \brief Finds nodes coinsident with Tolerance within Object excluding nodes within
 *        ExceptSubMeshOrGroups
 */
//================================================================================

void SMESH_MeshEditor_i::
FindCoincidentNodesOnPartBut(SMESH::SMESH_IDSource_ptr      theObject,
                             CORBA::Double                  theTolerance,
                             SMESH::array_of_long_array_out theGroupsOfNodes,
                             const SMESH::ListOfIDSources&  theExceptSubMeshOrGroups)
{
  initData();

  TIDSortedNodeSet nodes;
  idSourceToNodeSet( theObject, GetMeshDS(), nodes );

  for ( int i = 0; i < theExceptSubMeshOrGroups.length(); ++i )
  {
    TIDSortedNodeSet exceptNodes;
    idSourceToNodeSet( theExceptSubMeshOrGroups[i], GetMeshDS(), exceptNodes );
    TIDSortedNodeSet::iterator avoidNode = exceptNodes.begin();
    for ( ; avoidNode != exceptNodes.end(); ++avoidNode)
      nodes.erase( *avoidNode );
  }
  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  ::SMESH_MeshEditor anEditor( myMesh );
  if(!nodes.empty())
    anEditor.FindCoincidentNodes( nodes, theTolerance, aListOfListOfNodes );

  theGroupsOfNodes = new SMESH::array_of_long_array;
  theGroupsOfNodes->length( aListOfListOfNodes.size() );
  ::SMESH_MeshEditor::TListOfListOfNodes::iterator llIt = aListOfListOfNodes.begin();
  for ( CORBA::Long i = 0; llIt != aListOfListOfNodes.end(); llIt++, i++ )
  {
    list< const SMDS_MeshNode* >& aListOfNodes = *llIt;
    list< const SMDS_MeshNode* >::iterator lIt = aListOfNodes.begin();;
    SMESH::long_array& aGroup = (*theGroupsOfNodes)[ i ];
    aGroup.length( aListOfNodes.size() );
    for ( int j = 0; lIt != aListOfNodes.end(); lIt++, j++ )
      aGroup[ j ] = (*lIt)->GetID();
  }
  TPythonDump() << "coincident_nodes_on_part = " << this << ".FindCoincidentNodesOnPartBut( "
                << theObject<<", "
                << theTolerance << ", "
                << theExceptSubMeshOrGroups << " )";
}

//=======================================================================
//function : MergeNodes
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  TPythonDump aTPythonDump;
  aTPythonDump << this << ".MergeNodes([";
  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  for (int i = 0; i < GroupsOfNodes.length(); i++)
  {
    const SMESH::long_array& aNodeGroup = GroupsOfNodes[ i ];
    aListOfListOfNodes.push_back( list< const SMDS_MeshNode* >() );
    list< const SMDS_MeshNode* >& aListOfNodes = aListOfListOfNodes.back();
    for ( int j = 0; j < aNodeGroup.length(); j++ )
    {
      CORBA::Long index = aNodeGroup[ j ];
      const SMDS_MeshNode * node = aMesh->FindNode(index);
      if ( node )
        aListOfNodes.push_back( node );
    }
    if ( aListOfNodes.size() < 2 )
      aListOfListOfNodes.pop_back();

    if ( i > 0 ) aTPythonDump << ", ";
    aTPythonDump << aNodeGroup;
  }
  ::SMESH_MeshEditor anEditor( myMesh );
  anEditor.MergeNodes( aListOfListOfNodes );

  aTPythonDump <<  "])";
  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );
}

//=======================================================================
//function : FindEqualElements
//purpose  :
//=======================================================================
void SMESH_MeshEditor_i::FindEqualElements(SMESH::SMESH_IDSource_ptr      theObject,
                                           SMESH::array_of_long_array_out GroupsOfElementsID)
{
  initData();

  SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow(theObject);
  if ( !(!group->_is_nil() && group->GetType() == SMESH::NODE) )
  {
    typedef list<int> TListOfIDs;
    set<const SMDS_MeshElement*> elems;
    SMESH::long_array_var aElementsId = theObject->GetIDs();
    SMESHDS_Mesh* aMesh = GetMeshDS();

    for(int i = 0; i < aElementsId->length(); i++) {
      CORBA::Long anID = aElementsId[i];
      const SMDS_MeshElement * elem = aMesh->FindElement(anID);
      if (elem) {
        elems.insert(elem);
      }
    }

    ::SMESH_MeshEditor::TListOfListOfElementsID aListOfListOfElementsID;
    ::SMESH_MeshEditor anEditor( myMesh );
    anEditor.FindEqualElements( elems, aListOfListOfElementsID );

    GroupsOfElementsID = new SMESH::array_of_long_array;
    GroupsOfElementsID->length( aListOfListOfElementsID.size() );

    ::SMESH_MeshEditor::TListOfListOfElementsID::iterator arraysIt = aListOfListOfElementsID.begin();
    for (CORBA::Long j = 0; arraysIt != aListOfListOfElementsID.end(); ++arraysIt, ++j) {
      SMESH::long_array& aGroup = (*GroupsOfElementsID)[ j ];
      TListOfIDs& listOfIDs = *arraysIt;
      aGroup.length( listOfIDs.size() );
      TListOfIDs::iterator idIt = listOfIDs.begin();
      for (int k = 0; idIt != listOfIDs.end(); ++idIt, ++k ) {
        aGroup[ k ] = *idIt;
      }
    }

    TPythonDump() << "equal_elements = " << this << ".FindEqualElements( "
                  <<theObject<<" )";
  }
}

//=======================================================================
//function : MergeElements
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeElements(const SMESH::array_of_long_array& GroupsOfElementsID)
{
  initData();

  TPythonDump aTPythonDump;
  aTPythonDump << this << ".MergeElements( [";

  ::SMESH_MeshEditor::TListOfListOfElementsID aListOfListOfElementsID;

  for (int i = 0; i < GroupsOfElementsID.length(); i++) {
    const SMESH::long_array& anElemsIDGroup = GroupsOfElementsID[ i ];
    aListOfListOfElementsID.push_back( list< int >() );
    list< int >& aListOfElemsID = aListOfListOfElementsID.back();
    for ( int j = 0; j < anElemsIDGroup.length(); j++ ) {
      CORBA::Long id = anElemsIDGroup[ j ];
      aListOfElemsID.push_back( id );
    }
    if ( aListOfElemsID.size() < 2 )
      aListOfListOfElementsID.pop_back();
    if ( i > 0 ) aTPythonDump << ", ";
    aTPythonDump << anElemsIDGroup;
  }

  ::SMESH_MeshEditor anEditor( myMesh );
  anEditor.MergeElements(aListOfListOfElementsID);
  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );

  aTPythonDump << "] )";
}

//=======================================================================
//function : MergeEqualElements
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeEqualElements()
{
  initData();

  ::SMESH_MeshEditor anEditor( myMesh );
  anEditor.MergeEqualElements();

  myMesh->GetMeshDS()->Modified();

  TPythonDump() << this << ".MergeEqualElements()";
}

//=============================================================================
/*!
 * Move the node to a given point
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::MoveNode(CORBA::Long   NodeID,
                                            CORBA::Double x,
                                            CORBA::Double y,
                                            CORBA::Double z)
{
  initData(/*deleteSearchers=*/false);

  const SMDS_MeshNode * node = GetMeshDS()->FindNode( NodeID );
  if ( !node )
    return false;

  if ( theNodeSearcher )
    theSearchersDeleter.Set( myMesh ); // remove theNodeSearcher if mesh is other

  if ( myPreviewMode ) // make preview data
  {
    // in a preview mesh, make edges linked to a node
    TPreviewMesh tmpMesh;
    TIDSortedElemSet linkedNodes;
    ::SMESH_MeshEditor::GetLinkedNodes( node, linkedNodes );
    TIDSortedElemSet::iterator nIt = linkedNodes.begin();
    SMDS_MeshNode *nodeCpy1 = tmpMesh.Copy(node);
    for ( ; nIt != linkedNodes.end(); ++nIt )
    {
      SMDS_MeshNode *nodeCpy2 = tmpMesh.Copy ( cast2Node( *nIt ));
      tmpMesh.GetMeshDS()->AddEdge(nodeCpy1, nodeCpy2);
    }
    // move copied node
    if ( nodeCpy1 )
      tmpMesh.GetMeshDS()->MoveNode(nodeCpy1, x, y, z);
    // fill preview data
    ::SMESH_MeshEditor anEditor( & tmpMesh );
    storeResult( anEditor );
  }
  else if ( theNodeSearcher ) // move node and update theNodeSearcher data accordingly
    theNodeSearcher->MoveNode(node, gp_Pnt( x,y,z ));
  else
    GetMeshDS()->MoveNode(node, x, y, z);

  if ( !myPreviewMode )
  {
    // Update Python script
    TPythonDump() << "isDone = " << this << ".MoveNode( "
                  << NodeID << ", " << TVar(x) << ", " << TVar(y) << ", " << TVar(z) << " )";
    myMesh->GetMeshDS()->Modified();
    myMesh->SetIsModified( true );
  }

  return true;
}

//================================================================================
/*!
 * \brief Return ID of node closest to a given point
 */
//================================================================================

CORBA::Long SMESH_MeshEditor_i::FindNodeClosestTo(CORBA::Double x,
                                                  CORBA::Double y,
                                                  CORBA::Double z)
{
  theSearchersDeleter.Set( myMesh ); // remove theNodeSearcher if mesh is other

  if ( !theNodeSearcher ) {
    ::SMESH_MeshEditor anEditor( myMesh );
    theNodeSearcher = anEditor.GetNodeSearcher();
  }
  gp_Pnt p( x,y,z );
  if ( const SMDS_MeshNode* node = theNodeSearcher->FindClosestTo( p ))
    return node->GetID();

  return 0;
}

//================================================================================
/*!
 * \brief If the given ID is a valid node ID (nodeID > 0), just move this node, else
 * move the node closest to the point to point's location and return ID of the node
 */
//================================================================================

CORBA::Long SMESH_MeshEditor_i::MoveClosestNodeToPoint(CORBA::Double x,
                                                       CORBA::Double y,
                                                       CORBA::Double z,
                                                       CORBA::Long   theNodeID)
{
  // We keep theNodeSearcher until any mesh modification:
  // 1) initData() deletes theNodeSearcher at any edition,
  // 2) TSearchersDeleter - at any mesh compute event and mesh change

  initData(/*deleteSearchers=*/false);

  theSearchersDeleter.Set( myMesh ); // remove theNodeSearcher if mesh is other

  int nodeID = theNodeID;
  const SMDS_MeshNode* node = GetMeshDS()->FindNode( nodeID );
  if ( !node ) // preview moving node
  {
    if ( !theNodeSearcher ) {
      ::SMESH_MeshEditor anEditor( myMesh );
      theNodeSearcher = anEditor.GetNodeSearcher();
    }
    gp_Pnt p( x,y,z );
    node = theNodeSearcher->FindClosestTo( p );
  }
  if ( node ) {
    nodeID = node->GetID();
    if ( myPreviewMode ) // make preview data
    {
      // in a preview mesh, make edges linked to a node
      TPreviewMesh tmpMesh;
      TIDSortedElemSet linkedNodes;
      ::SMESH_MeshEditor::GetLinkedNodes( node, linkedNodes );
      TIDSortedElemSet::iterator nIt = linkedNodes.begin();
      for ( ; nIt != linkedNodes.end(); ++nIt )
      {
        SMDS_LinearEdge edge( node, cast2Node( *nIt ));
        tmpMesh.Copy( &edge );
      }
      // move copied node
      node = tmpMesh.GetMeshDS()->FindNode( nodeID );
      if ( node )
        tmpMesh.GetMeshDS()->MoveNode(node, x, y, z);
      // fill preview data
      ::SMESH_MeshEditor anEditor( & tmpMesh );
      storeResult( anEditor );
    }
    else if ( theNodeSearcher ) // move node and update theNodeSearcher data accordingly
    {
      theNodeSearcher->MoveNode(node, gp_Pnt( x,y,z ));
    }
    else
    {
      GetMeshDS()->MoveNode(node, x, y, z);
    }
  }

  if ( !myPreviewMode )
  {
    TPythonDump() << "nodeID = " << this
                  << ".MoveClosestNodeToPoint( "<< x << ", " << y << ", " << z
                  << ", " << nodeID << " )";

    myMesh->GetMeshDS()->Modified();
    myMesh->SetIsModified( true );
  }

  return nodeID;
}

//=======================================================================
/*!
 * Return elements of given type where the given point is IN or ON.
 *
 * 'ALL' type means elements of any type excluding nodes
 */
//=======================================================================

SMESH::long_array* SMESH_MeshEditor_i::FindElementsByPoint(CORBA::Double      x,
                                                           CORBA::Double      y,
                                                           CORBA::Double      z,
                                                           SMESH::ElementType type)
{
  SMESH::long_array_var res = new SMESH::long_array;
  vector< const SMDS_MeshElement* > foundElems;

  theSearchersDeleter.Set( myMesh );
  if ( !theElementSearcher ) {
    ::SMESH_MeshEditor anEditor( myMesh );
    theElementSearcher = anEditor.GetElementSearcher();
  }
  theElementSearcher->FindElementsByPoint( gp_Pnt( x,y,z ),
                                           SMDSAbs_ElementType( type ),
                                           foundElems);
  res->length( foundElems.size() );
  for ( int i = 0; i < foundElems.size(); ++i )
    res[i] = foundElems[i]->GetID();

  if ( !myPreviewMode ) // call from tui
    TPythonDump() << "res = " << this << ".FindElementsByPoint( "
                  << x << ", "
                  << y << ", "
                  << z << ", "
                  << type << " )";

  return res._retn();
}

//=======================================================================
//function : FindAmongElementsByPoint
//purpose  : Searching among the given elements, return elements of given type 
//           where the given point is IN or ON.
//           'ALL' type means elements of any type excluding nodes
//=======================================================================

SMESH::long_array*
SMESH_MeshEditor_i::FindAmongElementsByPoint(SMESH::SMESH_IDSource_ptr elementIDs,
                                             CORBA::Double             x,
                                             CORBA::Double             y,
                                             CORBA::Double             z,
                                             SMESH::ElementType        type)
{
  SMESH::long_array_var res = new SMESH::long_array;
  
  SMESH::array_of_ElementType_var types = elementIDs->GetTypes();
  if ( types->length() == 1 && // a part contains only nodes or 0D elements
       ( types[0] == SMESH::NODE || types[0] == SMESH::ELEM0D ) &&
       type != types[0] ) // but search of elements of dim > 0
    return res._retn();

  if ( SMESH::DownCast<SMESH_Mesh_i*>( elementIDs )) // elementIDs is the whole mesh 
    return FindElementsByPoint( x,y,z, type );

  string partIOR = SMESH_Gen_i::GetORB()->object_to_string( elementIDs );
  if ( SMESH_Group_i* group_i = SMESH::DownCast<SMESH_Group_i*>( elementIDs ))
    // take into account passible group modification
    partIOR += SMESH_Comment( ((SMESHDS_Group*)group_i->GetGroupDS())->SMDSGroup().Tic() );
  partIOR += SMESH_Comment( type );

  TIDSortedElemSet elements; // elems should live until FindElementsByPoint() finishes

  theSearchersDeleter.Set( myMesh, partIOR );
  if ( !theElementSearcher )
  {
    // create a searcher from elementIDs
    SMESH::SMESH_Mesh_var mesh = elementIDs->GetMesh();
    SMESHDS_Mesh* meshDS = SMESH::DownCast<SMESH_Mesh_i*>( mesh )->GetImpl().GetMeshDS();

    if ( !idSourceToSet( elementIDs, meshDS, elements,
                         SMDSAbs_ElementType(type), /*emptyIfIsMesh=*/true))
      return res._retn();

    typedef SMDS_SetIterator<const SMDS_MeshElement*, TIDSortedElemSet::const_iterator > TIter;
    SMDS_ElemIteratorPtr elemsIt( new TIter( elements.begin(), elements.end() ));

    ::SMESH_MeshEditor anEditor( myMesh );
    theElementSearcher = anEditor.GetElementSearcher(elemsIt);
  }

  vector< const SMDS_MeshElement* > foundElems;

  theElementSearcher->FindElementsByPoint( gp_Pnt( x,y,z ),
                                           SMDSAbs_ElementType( type ),
                                           foundElems);
  res->length( foundElems.size() );
  for ( int i = 0; i < foundElems.size(); ++i )
    res[i] = foundElems[i]->GetID();

  if ( !myPreviewMode ) // call from tui
    TPythonDump() << "res = " << this << ".FindAmongElementsByPoint( "
                  << elementIDs << ", "
                  << x << ", "
                  << y << ", "
                  << z << ", "
                  << type << " )";

  return res._retn();
  
}
//=======================================================================
//function : GetPointState
//purpose  : Return point state in a closed 2D mesh in terms of TopAbs_State enumeration.
//           TopAbs_UNKNOWN state means that either mesh is wrong or the analysis fails.
//=======================================================================

CORBA::Short SMESH_MeshEditor_i::GetPointState(CORBA::Double x,
                                               CORBA::Double y,
                                               CORBA::Double z)
{
  theSearchersDeleter.Set( myMesh );
  if ( !theElementSearcher ) {
    ::SMESH_MeshEditor anEditor( myMesh );
    theElementSearcher = anEditor.GetElementSearcher();
  }
  return CORBA::Short( theElementSearcher->GetPointState( gp_Pnt( x,y,z )));
}

//=======================================================================
//function : convError
//purpose  :
//=======================================================================

#define RETCASE(enm) case ::SMESH_MeshEditor::enm: return SMESH::SMESH_MeshEditor::enm;

static SMESH::SMESH_MeshEditor::Sew_Error convError( const::SMESH_MeshEditor::Sew_Error e )
{
  switch ( e ) {
    RETCASE( SEW_OK );
    RETCASE( SEW_BORDER1_NOT_FOUND );
    RETCASE( SEW_BORDER2_NOT_FOUND );
    RETCASE( SEW_BOTH_BORDERS_NOT_FOUND );
    RETCASE( SEW_BAD_SIDE_NODES );
    RETCASE( SEW_VOLUMES_TO_SPLIT );
    RETCASE( SEW_DIFF_NB_OF_ELEMENTS );
    RETCASE( SEW_TOPO_DIFF_SETS_OF_ELEMENTS );
    RETCASE( SEW_BAD_SIDE1_NODES );
    RETCASE( SEW_BAD_SIDE2_NODES );
  }
  return SMESH::SMESH_MeshEditor::SEW_OK;
}

//=======================================================================
//function : SewFreeBorders
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
SMESH_MeshEditor_i::SewFreeBorders(CORBA::Long FirstNodeID1,
                                   CORBA::Long SecondNodeID1,
                                   CORBA::Long LastNodeID1,
                                   CORBA::Long FirstNodeID2,
                                   CORBA::Long SecondNodeID2,
                                   CORBA::Long LastNodeID2,
                                   CORBA::Boolean CreatePolygons,
                                   CORBA::Boolean CreatePolyedrs)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeID1  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeID1 );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeID1   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeID2  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( SecondNodeID2 );
  const SMDS_MeshNode* aSide2ThirdNode   = aMesh->FindNode( LastNodeID2   );

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode ||
      !aSide2ThirdNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER2_NOT_FOUND;

  TPythonDump() << "error = " << this << ".SewFreeBorders( "
                << FirstNodeID1  << ", "
                << SecondNodeID1 << ", "
                << LastNodeID1   << ", "
                << FirstNodeID2  << ", "
                << SecondNodeID2 << ", "
                << LastNodeID2   << ", "
                << CreatePolygons<< ", "
                << CreatePolyedrs<< " )";

  ::SMESH_MeshEditor anEditor( myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                       aBorderSecondNode,
                                       aBorderLastNode,
                                       aSide2FirstNode,
                                       aSide2SecondNode,
                                       aSide2ThirdNode,
                                       true,
                                       CreatePolygons,
                                       CreatePolyedrs) );

  storeResult(anEditor);

  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );

  return error;
}


//=======================================================================
//function : SewConformFreeBorders
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
SMESH_MeshEditor_i::SewConformFreeBorders(CORBA::Long FirstNodeID1,
                                          CORBA::Long SecondNodeID1,
                                          CORBA::Long LastNodeID1,
                                          CORBA::Long FirstNodeID2,
                                          CORBA::Long SecondNodeID2)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeID1  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeID1 );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeID1   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeID2  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( SecondNodeID2 );
  const SMDS_MeshNode* aSide2ThirdNode   = 0;

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode )
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER2_NOT_FOUND;

  TPythonDump() << "error = " << this << ".SewConformFreeBorders( "
                << FirstNodeID1  << ", "
                << SecondNodeID1 << ", "
                << LastNodeID1   << ", "
                << FirstNodeID2  << ", "
                << SecondNodeID2 << " )";

  ::SMESH_MeshEditor anEditor( myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                       aBorderSecondNode,
                                       aBorderLastNode,
                                       aSide2FirstNode,
                                       aSide2SecondNode,
                                       aSide2ThirdNode,
                                       true,
                                       false, false) );

  storeResult(anEditor);

  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );

  return error;
}


//=======================================================================
//function : SewBorderToSide
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
SMESH_MeshEditor_i::SewBorderToSide(CORBA::Long FirstNodeIDOnFreeBorder,
                                    CORBA::Long SecondNodeIDOnFreeBorder,
                                    CORBA::Long LastNodeIDOnFreeBorder,
                                    CORBA::Long FirstNodeIDOnSide,
                                    CORBA::Long LastNodeIDOnSide,
                                    CORBA::Boolean CreatePolygons,
                                    CORBA::Boolean CreatePolyedrs)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeIDOnFreeBorder  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeIDOnFreeBorder );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeIDOnFreeBorder   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeIDOnSide  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( LastNodeIDOnSide );
  const SMDS_MeshNode* aSide2ThirdNode   = 0;

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode  )
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode)
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE_NODES;

  TPythonDump() << "error = " << this << ".SewBorderToSide( "
                << FirstNodeIDOnFreeBorder  << ", "
                << SecondNodeIDOnFreeBorder << ", "
                << LastNodeIDOnFreeBorder   << ", "
                << FirstNodeIDOnSide        << ", "
                << LastNodeIDOnSide         << ", "
                << CreatePolygons           << ", "
                << CreatePolyedrs           << ") ";

  ::SMESH_MeshEditor anEditor( myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                       aBorderSecondNode,
                                       aBorderLastNode,
                                       aSide2FirstNode,
                                       aSide2SecondNode,
                                       aSide2ThirdNode,
                                       false,
                                       CreatePolygons,
                                       CreatePolyedrs) );

  storeResult(anEditor);

  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );

  return error;
}


//=======================================================================
//function : SewSideElements
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
SMESH_MeshEditor_i::SewSideElements(const SMESH::long_array& IDsOfSide1Elements,
                                    const SMESH::long_array& IDsOfSide2Elements,
                                    CORBA::Long NodeID1OfSide1ToMerge,
                                    CORBA::Long NodeID1OfSide2ToMerge,
                                    CORBA::Long NodeID2OfSide1ToMerge,
                                    CORBA::Long NodeID2OfSide2ToMerge)
{
  initData();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aFirstNode1ToMerge  = aMesh->FindNode( NodeID1OfSide1ToMerge );
  const SMDS_MeshNode* aFirstNode2ToMerge  = aMesh->FindNode( NodeID1OfSide2ToMerge );
  const SMDS_MeshNode* aSecondNode1ToMerge = aMesh->FindNode( NodeID2OfSide1ToMerge );
  const SMDS_MeshNode* aSecondNode2ToMerge = aMesh->FindNode( NodeID2OfSide2ToMerge );

  if (!aFirstNode1ToMerge ||
      !aFirstNode2ToMerge )
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE1_NODES;
  if (!aSecondNode1ToMerge||
      !aSecondNode2ToMerge)
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE2_NODES;

  TIDSortedElemSet aSide1Elems, aSide2Elems;
  arrayToSet(IDsOfSide1Elements, aMesh, aSide1Elems);
  arrayToSet(IDsOfSide2Elements, aMesh, aSide2Elems);

  TPythonDump() << "error = " << this << ".SewSideElements( "
                << IDsOfSide1Elements << ", "
                << IDsOfSide2Elements << ", "
                << NodeID1OfSide1ToMerge << ", "
                << NodeID1OfSide2ToMerge << ", "
                << NodeID2OfSide1ToMerge << ", "
                << NodeID2OfSide2ToMerge << ")";

  ::SMESH_MeshEditor anEditor( myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewSideElements (aSide1Elems, aSide2Elems,
                                         aFirstNode1ToMerge,
                                         aFirstNode2ToMerge,
                                         aSecondNode1ToMerge,
                                         aSecondNode2ToMerge));

  storeResult(anEditor);

  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );

  return error;
}

//================================================================================
/*!
 * \brief Set new nodes for given element
 * \param ide - element id
 * \param newIDs - new node ids
 * \retval CORBA::Boolean - true if result is OK
 */
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::ChangeElemNodes(CORBA::Long ide,
                                                   const SMESH::long_array& newIDs)
{
  initData();

  const SMDS_MeshElement* elem = GetMeshDS()->FindElement(ide);
  if(!elem) return false;

  int nbn = newIDs.length();
  int i=0;
  vector<const SMDS_MeshNode*> aNodes(nbn);
  int nbn1=-1;
  for(; i<nbn; i++) {
    const SMDS_MeshNode* aNode = GetMeshDS()->FindNode(newIDs[i]);
    if(aNode) {
      nbn1++;
      aNodes[nbn1] = aNode;
    }
  }
  TPythonDump() << "isDone = " << this << ".ChangeElemNodes( "
                << ide << ", " << newIDs << " )";

  MESSAGE("ChangeElementNodes");
  bool res = GetMeshDS()->ChangeElementNodes( elem, & aNodes[0], nbn1+1 );

  myMesh->GetMeshDS()->Modified();
  if ( res )
    myMesh->SetIsModified( true );

  return res;
}

//================================================================================
/*!
 * \brief Update myLastCreated* or myPreviewData
 * \param anEditor - it contains last modification results
 */
//================================================================================

void SMESH_MeshEditor_i::storeResult(::SMESH_MeshEditor& anEditor)
{
  if ( myPreviewMode ) { // --- MeshPreviewStruct filling ---

    list<int> aNodesConnectivity;
    typedef map<int, int> TNodesMap;
    TNodesMap nodesMap;

    TPreviewMesh * aPreviewMesh = dynamic_cast< TPreviewMesh* >( anEditor.GetMesh() );
    SMDSAbs_ElementType previewType = aPreviewMesh->myPreviewType;

    SMESHDS_Mesh* aMeshDS = anEditor.GetMeshDS();
    int nbEdges = aMeshDS->NbEdges();
    int nbFaces = aMeshDS->NbFaces();
    int nbVolum = aMeshDS->NbVolumes();
    switch ( previewType ) {
    case SMDSAbs_Edge  : nbFaces = nbVolum = 0; break;
    case SMDSAbs_Face  : nbEdges = nbVolum = 0; break;
    case SMDSAbs_Volume: nbEdges = nbFaces = 0; break;
    default:;
    }
    myPreviewData->nodesXYZ.length(aMeshDS->NbNodes());
    myPreviewData->elementTypes.length(nbEdges + nbFaces + nbVolum);
    int i = 0, j = 0;
    SMDS_ElemIteratorPtr itMeshElems = aMeshDS->elementsIterator();

    while ( itMeshElems->more() ) {
      const SMDS_MeshElement* aMeshElem = itMeshElems->next();
      if ( previewType != SMDSAbs_All && aMeshElem->GetType() != previewType )
        continue;

      SMDS_ElemIteratorPtr itElemNodes = aMeshElem->nodesIterator();
      while ( itElemNodes->more() ) {
        const SMDS_MeshNode* aMeshNode =
          static_cast<const SMDS_MeshNode*>( itElemNodes->next() );
        int aNodeID = aMeshNode->GetID();
        TNodesMap::iterator anIter = nodesMap.find(aNodeID);
        if ( anIter == nodesMap.end() ) {
          // filling the nodes coordinates
          myPreviewData->nodesXYZ[j].x = aMeshNode->X();
          myPreviewData->nodesXYZ[j].y = aMeshNode->Y();
          myPreviewData->nodesXYZ[j].z = aMeshNode->Z();
          anIter = nodesMap.insert( make_pair(aNodeID, j) ).first;
          j++;
        }
        aNodesConnectivity.push_back(anIter->second);
      }

      // filling the elements types
      SMDSAbs_ElementType aType;
      bool isPoly;
      /*if (aMeshElem->GetType() == SMDSAbs_Volume) {
        aType = SMDSAbs_Node;
        isPoly = false;
        }
        else*/ {
        aType = aMeshElem->GetType();
        isPoly = aMeshElem->IsPoly();
      }

      myPreviewData->elementTypes[i].SMDS_ElementType = (SMESH::ElementType) aType;
      myPreviewData->elementTypes[i].isPoly = isPoly;
      myPreviewData->elementTypes[i].nbNodesInElement = aMeshElem->NbNodes();
      i++;

    }
    myPreviewData->nodesXYZ.length( j );

    // filling the elements connectivities
    list<int>::iterator aConnIter = aNodesConnectivity.begin();
    myPreviewData->elementConnectivities.length(aNodesConnectivity.size());
    for( int i = 0; aConnIter != aNodesConnectivity.end(); aConnIter++, i++ )
      myPreviewData->elementConnectivities[i] = *aConnIter;

    return;
  }

  {
    // append new nodes into myLastCreatedNodes
    const SMESH_SequenceOfElemPtr& aSeq = anEditor.GetLastCreatedNodes();
    int j = myLastCreatedNodes->length();
    int newLen = j + aSeq.Length();
    myLastCreatedNodes->length( newLen );
    for(int i=0; j<newLen; i++,j++)
      myLastCreatedNodes[j] = aSeq.Value(i+1)->GetID();
  }
  {
    // append new elements into myLastCreatedElems
    const SMESH_SequenceOfElemPtr& aSeq = anEditor.GetLastCreatedElems();
    int j = myLastCreatedElems->length();
    int newLen = j + aSeq.Length();
    myLastCreatedElems->length( newLen );
    for(int i=0; j<newLen; i++,j++)
      myLastCreatedElems[j] = aSeq.Value(i+1)->GetID();
  }
}

//================================================================================
/*!
 * Return data of mesh edition preview
 */
//================================================================================

SMESH::MeshPreviewStruct* SMESH_MeshEditor_i::GetPreviewData()
{
  return myPreviewData._retn();
}

//================================================================================
/*!
 * \brief Returns list of it's IDs of created nodes
 * \retval SMESH::long_array* - list of node ID
 */
//================================================================================

SMESH::long_array* SMESH_MeshEditor_i::GetLastCreatedNodes()
{
  return myLastCreatedNodes._retn();
}

//================================================================================
/*!
 * \brief Returns list of it's IDs of created elements
 * \retval SMESH::long_array* - list of elements' ID
 */
//================================================================================

SMESH::long_array* SMESH_MeshEditor_i::GetLastCreatedElems()
{
  return myLastCreatedElems._retn();
}

//=======================================================================
//function : ConvertToQuadratic
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ConvertToQuadratic(CORBA::Boolean theForce3d)
{
  ::SMESH_MeshEditor anEditor( myMesh );
  anEditor.ConvertToQuadratic(theForce3d);
  TPythonDump() << this << ".ConvertToQuadratic( " << theForce3d << " )";
  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );
}

//=======================================================================
//function : ConvertFromQuadratic
//purpose  :
//=======================================================================

CORBA::Boolean SMESH_MeshEditor_i::ConvertFromQuadratic()
{
  ::SMESH_MeshEditor anEditor( myMesh );
  CORBA::Boolean isDone = anEditor.ConvertFromQuadratic();
  TPythonDump() << this << ".ConvertFromQuadratic()";
  myMesh->GetMeshDS()->Modified();
  if ( isDone )
    myMesh->SetIsModified( true );
  return isDone;
}
//================================================================================
/*!
 * \brief Makes a part of the mesh quadratic
 */
//================================================================================

void SMESH_MeshEditor_i::ConvertToQuadraticObject(CORBA::Boolean            theForce3d,
                                                  SMESH::SMESH_IDSource_ptr theObject)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  TPythonDump pyDump;
  TIDSortedElemSet elems;
  if ( idSourceToSet( theObject, GetMeshDS(), elems, SMDSAbs_All, /*emptyIfIsMesh=*/true ))
  {
    if ( elems.empty() )
    {
      ConvertToQuadratic( theForce3d );
    }
    else if ( (*elems.begin())->GetType() == SMDSAbs_Node )
    {
      THROW_SALOME_CORBA_EXCEPTION("Group of nodes is not allowed", SALOME::BAD_PARAM);
    }
    else
    {
      ::SMESH_MeshEditor anEditor( myMesh );
      anEditor.ConvertToQuadratic(theForce3d, elems);
    }
  }
  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );

  pyDump << this << ".ConvertToQuadraticObject( "<<theForce3d<<", "<<theObject<<" )";
}

//================================================================================
/*!
 * \brief Makes a part of the mesh linear
 */
//================================================================================

void SMESH_MeshEditor_i::ConvertFromQuadraticObject(SMESH::SMESH_IDSource_ptr theObject)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  TPythonDump pyDump;
  TIDSortedElemSet elems;
  if ( idSourceToSet( theObject, GetMeshDS(), elems, SMDSAbs_All, /*emptyIfIsMesh=*/true ))
  {
    if ( elems.empty() )
    {
      ConvertFromQuadratic();
    }
    else if ( (*elems.begin())->GetType() == SMDSAbs_Node )
    {
      THROW_SALOME_CORBA_EXCEPTION("Group of nodes is not allowed", SALOME::BAD_PARAM);
    }
    else
    {
      ::SMESH_MeshEditor anEditor( myMesh );
      anEditor.ConvertFromQuadratic(elems);
    }
  }
  myMesh->GetMeshDS()->Modified();
  myMesh->SetIsModified( true );

  pyDump << this << ".ConvertFromQuadraticObject( "<<theObject<<" )";
}

//=======================================================================
//function : makeMesh
//purpose  : create a named imported mesh
//=======================================================================

SMESH::SMESH_Mesh_ptr SMESH_MeshEditor_i::makeMesh(const char* theMeshName)
{
  SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen();
  SMESH::SMESH_Mesh_var mesh = gen->CreateEmptyMesh();
  SALOMEDS::Study_var study = gen->GetCurrentStudy();
  SALOMEDS::SObject_var meshSO = gen->ObjectToSObject( study, mesh );
  gen->SetName( meshSO, theMeshName, "Mesh" );
  gen->SetPixMap( meshSO, "ICON_SMESH_TREE_MESH_IMPORTED");

  return mesh._retn();
}

//=======================================================================
//function : DumpGroupsList
//purpose  :
//=======================================================================
void SMESH_MeshEditor_i::DumpGroupsList(TPythonDump &               theDumpPython,
                                        const SMESH::ListOfGroups * theGroupList)
{
  bool isDumpGroupList = theGroupList && theGroupList->length() > 0;
  if(isDumpGroupList) {
    theDumpPython << theGroupList << " = ";
  }
}

//================================================================================
/*!
  \brief Generates the unique group name.
  \param thePrefix name prefix
  \return unique name
*/
//================================================================================
string SMESH_MeshEditor_i::generateGroupName(const string& thePrefix)
{
  SMESH::ListOfGroups_var groups = myMesh_i->GetGroups();
  set<string> groupNames;

  // Get existing group names
  for (int i = 0, nbGroups = groups->length(); i < nbGroups; i++ ) {
    SMESH::SMESH_GroupBase_var aGroup = groups[i];
    if (CORBA::is_nil(aGroup))
      continue;

    groupNames.insert(aGroup->GetName());
  }

  // Find new name
  string name = thePrefix;
  int index = 0;

  while (!groupNames.insert(name).second) {
    if (index == 0) {
      name += "_1";
    }
    else {
      TCollection_AsciiString nbStr(index+1);
      name.resize( name.rfind('_')+1 );
      name += nbStr.ToCString();
    }
    ++index;
  }

  return name;
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  \param theNodes - identifiers of nodes to be doubled
  \param theModifiedElems - identifiers of elements to be updated by the new (doubled)
         nodes. If list of element identifiers is empty then nodes are doubled but
         they not assigned to elements
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNode(), DoubleNodeGroup(), DoubleNodeGroups()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodes( const SMESH::long_array& theNodes,
                                                const SMESH::long_array& theModifiedElems )
{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );
  list< int > aListOfNodes;
  int i, n;
  for ( i = 0, n = theNodes.length(); i < n; i++ )
    aListOfNodes.push_back( theNodes[ i ] );

  list< int > aListOfElems;
  for ( i = 0, n = theModifiedElems.length(); i < n; i++ )
    aListOfElems.push_back( theModifiedElems[ i ] );

  bool aResult = aMeshEditor.DoubleNodes( aListOfNodes, aListOfElems );

  myMesh->GetMeshDS()->Modified();
  storeResult( aMeshEditor) ;
  if ( aResult )
    myMesh->SetIsModified( true );

  // Update Python script
  TPythonDump() << this << ".DoubleNodes( " << theNodes << ", "<< theModifiedElems << " )";

  return aResult;
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  This method provided for convenience works as DoubleNodes() described above.
  \param theNodeId - identifier of node to be doubled.
  \param theModifiedElems - identifiers of elements to be updated.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodes(), DoubleNodeGroup(), DoubleNodeGroups()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNode( CORBA::Long              theNodeId,
                                               const SMESH::long_array& theModifiedElems )
{
  SMESH::long_array_var aNodes = new SMESH::long_array;
  aNodes->length( 1 );
  aNodes[ 0 ] = theNodeId;

  TPythonDump pyDump; // suppress dump by the next line

  CORBA::Boolean done = DoubleNodes( aNodes, theModifiedElems );

  pyDump << this << ".DoubleNode( " << theNodeId << ", " << theModifiedElems << " )";

  return done;
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  This method provided for convenience works as DoubleNodes() described above.
  \param theNodes - group of nodes to be doubled.
  \param theModifiedElems - group of elements to be updated.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNode(), DoubleNodes(), DoubleNodeGroups()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeGroup(SMESH::SMESH_GroupBase_ptr theNodes,
                                                   SMESH::SMESH_GroupBase_ptr theModifiedElems )
{
  if ( CORBA::is_nil( theNodes ) && theNodes->GetType() != SMESH::NODE )
    return false;

  SMESH::long_array_var aNodes = theNodes->GetListOfID();
  SMESH::long_array_var aModifiedElems;
  if ( !CORBA::is_nil( theModifiedElems ) )
    aModifiedElems = theModifiedElems->GetListOfID();
  else
  {
    aModifiedElems = new SMESH::long_array;
    aModifiedElems->length( 0 );
  }

  TPythonDump pyDump; // suppress dump by the next line

  bool done = DoubleNodes( aNodes, aModifiedElems );

  pyDump << this << ".DoubleNodeGroup( " << theNodes << ", " << theModifiedElems << " )";

  return done;
}

/*!
 * \brief Creates a hole in a mesh by doubling the nodes of some particular elements.
 * Works as DoubleNodeGroup(), but returns a new group with newly created nodes.
 * \param theNodes - group of nodes to be doubled.
 * \param theModifiedElems - group of elements to be updated.
 * \return a new group with newly created nodes
 * \sa DoubleNodeGroup()
 */
SMESH::SMESH_Group_ptr
SMESH_MeshEditor_i::DoubleNodeGroupNew( SMESH::SMESH_GroupBase_ptr theNodes,
                                        SMESH::SMESH_GroupBase_ptr theModifiedElems )
{
  SMESH::SMESH_Group_var aNewGroup;

  if ( CORBA::is_nil( theNodes ) && theNodes->GetType() != SMESH::NODE )
    return aNewGroup._retn();

  // Duplicate nodes
  SMESH::long_array_var aNodes = theNodes->GetListOfID();
  SMESH::long_array_var aModifiedElems;
  if ( !CORBA::is_nil( theModifiedElems ) )
    aModifiedElems = theModifiedElems->GetListOfID();
  else {
    aModifiedElems = new SMESH::long_array;
    aModifiedElems->length( 0 );
  }

  TPythonDump pyDump; // suppress dump by the next line

  bool aResult = DoubleNodes( aNodes, aModifiedElems );
  if ( aResult )
  {
    // Create group with newly created nodes
    SMESH::long_array_var anIds = GetLastCreatedNodes();
    if (anIds->length() > 0) {
      string anUnindexedName (theNodes->GetName());
      string aNewName = generateGroupName(anUnindexedName + "_double");
      aNewGroup = myMesh_i->CreateGroup(SMESH::NODE, aNewName.c_str());
      aNewGroup->Add(anIds);
      pyDump << aNewGroup << " = ";
    }
  }

  pyDump << this << ".DoubleNodeGroupNew( " << theNodes << ", "
         << theModifiedElems << " )";

  return aNewGroup._retn();
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  This method provided for convenience works as DoubleNodes() described above.
  \param theNodes - list of groups of nodes to be doubled
  \param theModifiedElems - list of groups of elements to be updated.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNode(), DoubleNodeGroup(), DoubleNodes()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeGroups(const SMESH::ListOfGroups& theNodes,
                                                    const SMESH::ListOfGroups& theModifiedElems )
{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  std::list< int > aNodes;
  int i, n, j, m;
  for ( i = 0, n = theNodes.length(); i < n; i++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theNodes[ i ];
    if ( !CORBA::is_nil( aGrp ) && aGrp->GetType() == SMESH::NODE )
    {
      SMESH::long_array_var aCurr = aGrp->GetListOfID();
      for ( j = 0, m = aCurr->length(); j < m; j++ )
        aNodes.push_back( aCurr[ j ] );
    }
  }

  std::list< int > anElems;
  for ( i = 0, n = theModifiedElems.length(); i < n; i++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theModifiedElems[ i ];
    if ( !CORBA::is_nil( aGrp ) && aGrp->GetType() != SMESH::NODE )
    {
      SMESH::long_array_var aCurr = aGrp->GetListOfID();
      for ( j = 0, m = aCurr->length(); j < m; j++ )
        anElems.push_back( aCurr[ j ] );
    }
  }

  bool aResult = aMeshEditor.DoubleNodes( aNodes, anElems );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  if ( aResult )
    myMesh->SetIsModified( true );


  TPythonDump() << this << ".DoubleNodeGroups( " << theNodes << ", " << theModifiedElems << " )";

  return aResult;
}

//================================================================================
/*!
 * \brief Creates a hole in a mesh by doubling the nodes of some particular elements.
 * Works as DoubleNodeGroups(), but returns a new group with newly created nodes.
 * \param theNodes - group of nodes to be doubled.
 * \param theModifiedElems - group of elements to be updated.
 * \return a new group with newly created nodes
 * \sa DoubleNodeGroups()
 */
//================================================================================

SMESH::SMESH_Group_ptr
SMESH_MeshEditor_i::DoubleNodeGroupsNew( const SMESH::ListOfGroups& theNodes,
                                         const SMESH::ListOfGroups& theModifiedElems )
{
  SMESH::SMESH_Group_var aNewGroup;

  TPythonDump pyDump; // suppress dump by the next line

  bool aResult = DoubleNodeGroups( theNodes, theModifiedElems );

  if ( aResult )
  {
    // Create group with newly created nodes
    SMESH::long_array_var anIds = GetLastCreatedNodes();
    if (anIds->length() > 0) {
      string anUnindexedName (theNodes[0]->GetName());
      string aNewName = generateGroupName(anUnindexedName + "_double");
      aNewGroup = myMesh_i->CreateGroup(SMESH::NODE, aNewName.c_str());
      aNewGroup->Add(anIds);
      pyDump << aNewGroup << " = ";
    }
  }

  pyDump << this << ".DoubleNodeGroupsNew( " << theNodes << ", "
         << theModifiedElems << " )";

  return aNewGroup._retn();
}


//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  \param theElems - the list of elements (edges or faces) to be replicated
  The nodes for duplication could be found from these elements
  \param theNodesNot - list of nodes to NOT replicate
  \param theAffectedElems - the list of elements (cells and edges) to which the
  replicated nodes should be associated to.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodeGroup(), DoubleNodeGroups()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeElem( const SMESH::long_array& theElems,
                                                   const SMESH::long_array& theNodesNot,
                                                   const SMESH::long_array& theAffectedElems )

{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  arrayToSet(theElems, aMeshDS, anElems, SMDSAbs_All);
  arrayToSet(theNodesNot, aMeshDS, aNodes, SMDSAbs_Node);
  arrayToSet(theAffectedElems, aMeshDS, anAffected, SMDSAbs_All);

  bool aResult = aMeshEditor.DoubleNodes( anElems, aNodes, anAffected );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  if ( aResult )
    myMesh->SetIsModified( true );

  // Update Python script
  TPythonDump() << this << ".DoubleNodeElem( " << theElems << ", "
                << theNodesNot << ", " << theAffectedElems << " )";
  return aResult;
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  \param theElems - the list of elements (edges or faces) to be replicated
  The nodes for duplication could be found from these elements
  \param theNodesNot - list of nodes to NOT replicate
  \param theShape - shape to detect affected elements (element which geometric center
  located on or inside shape).
  The replicated nodes should be associated to affected elements.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodeGroupInRegion(), DoubleNodeGroupsInRegion()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeElemInRegion ( const SMESH::long_array& theElems,
                                                            const SMESH::long_array& theNodesNot,
                                                            GEOM::GEOM_Object_ptr    theShape )

{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes;
  arrayToSet(theElems, aMeshDS, anElems, SMDSAbs_All);
  arrayToSet(theNodesNot, aMeshDS, aNodes, SMDSAbs_Node);

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theShape );
  bool aResult = aMeshEditor.DoubleNodesInRegion( anElems, aNodes, aShape );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  if ( aResult )
    myMesh->SetIsModified( true );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemInRegion( " << theElems << ", "
                << theNodesNot << ", " << theShape << " )";
  return aResult;
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  \param theElems - group of of elements (edges or faces) to be replicated
  \param theNodesNot - group of nodes not to replicated
  \param theAffectedElems - group of elements to which the replicated nodes
  should be associated to.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodes(), DoubleNodeGroups()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeElemGroup(SMESH::SMESH_GroupBase_ptr theElems,
                                                       SMESH::SMESH_GroupBase_ptr theNodesNot,
                                                       SMESH::SMESH_GroupBase_ptr theAffectedElems)
{
  if ( CORBA::is_nil( theElems ) && theElems->GetType() == SMESH::NODE )
    return false;

  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  idSourceToSet( theElems, aMeshDS, anElems, SMDSAbs_All );
  idSourceToSet( theNodesNot, aMeshDS, aNodes, SMDSAbs_Node );
  idSourceToSet( theAffectedElems, aMeshDS, anAffected, SMDSAbs_All );

  bool aResult = aMeshEditor.DoubleNodes( anElems, aNodes, anAffected );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  if ( aResult )
    myMesh->SetIsModified( true );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroup( " << theElems << ", "
                << theNodesNot << ", " << theAffectedElems << " )";
  return aResult;
}

/*!
 * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
 * Works as DoubleNodeElemGroup(), but returns a new group with newly created elements.
 * \param theElems - group of of elements (edges or faces) to be replicated
 * \param theNodesNot - group of nodes not to replicated
 * \param theAffectedElems - group of elements to which the replicated nodes
 *        should be associated to.
 * \return a new group with newly created elements
 * \sa DoubleNodeElemGroup()
 */
SMESH::SMESH_Group_ptr
SMESH_MeshEditor_i::DoubleNodeElemGroupNew(SMESH::SMESH_GroupBase_ptr theElems,
                                           SMESH::SMESH_GroupBase_ptr theNodesNot,
                                           SMESH::SMESH_GroupBase_ptr theAffectedElems)
{
  TPythonDump pyDump;
  SMESH::ListOfGroups_var twoGroups = DoubleNodeElemGroup2New( theElems,
                                                               theNodesNot,
                                                               theAffectedElems,
                                                               true, false );
  SMESH::SMESH_GroupBase_var baseGroup = twoGroups[0].in();
  SMESH::SMESH_Group_var     elemGroup = SMESH::SMESH_Group::_narrow( baseGroup );

  pyDump << elemGroup << " = " << this << ".DoubleNodeElemGroupNew( "
         << theElems         << ", "
         << theNodesNot      << ", "
         << theAffectedElems << " )";

  return elemGroup._retn();
}

SMESH::ListOfGroups*
SMESH_MeshEditor_i::DoubleNodeElemGroup2New(SMESH::SMESH_GroupBase_ptr theElems,
                                            SMESH::SMESH_GroupBase_ptr theNodesNot,
                                            SMESH::SMESH_GroupBase_ptr theAffectedElems,
                                            CORBA::Boolean             theElemGroupNeeded,
                                            CORBA::Boolean             theNodeGroupNeeded)
{
  SMESH::SMESH_Group_var aNewElemGroup, aNewNodeGroup;
  SMESH::ListOfGroups_var aTwoGroups = new SMESH::ListOfGroups();
  aTwoGroups->length( 2 );

  if ( CORBA::is_nil( theElems ) && theElems->GetType() == SMESH::NODE )
    return aTwoGroups._retn();

  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  idSourceToSet( theElems, aMeshDS, anElems, SMDSAbs_All );
  idSourceToSet( theNodesNot, aMeshDS, aNodes, SMDSAbs_Node );
  idSourceToSet( theAffectedElems, aMeshDS, anAffected, SMDSAbs_All );


  bool aResult = aMeshEditor.DoubleNodes( anElems, aNodes, anAffected );

  storeResult( aMeshEditor) ;
  myMesh->GetMeshDS()->Modified();

  TPythonDump pyDump;

  if ( aResult )
  {
    myMesh->SetIsModified( true );

    // Create group with newly created elements
    CORBA::String_var elemGroupName = theElems->GetName();
    string aNewName = generateGroupName( string(elemGroupName.in()) + "_double");
    if ( !aMeshEditor.GetLastCreatedElems().IsEmpty() && theElemGroupNeeded )
    {
      SMESH::long_array_var anIds = GetLastCreatedElems();
      SMESH::ElementType aGroupType = myMesh_i->GetElementType(anIds[0], true);
      aNewElemGroup = myMesh_i->CreateGroup(aGroupType, aNewName.c_str());
      aNewElemGroup->Add(anIds);
    }
    if ( !aMeshEditor.GetLastCreatedNodes().IsEmpty() && theNodeGroupNeeded )
    {
      SMESH::long_array_var anIds = GetLastCreatedNodes();
      aNewNodeGroup = myMesh_i->CreateGroup(SMESH::NODE, aNewName.c_str());
      aNewNodeGroup->Add(anIds);
    }
  }

  // Update Python script

  pyDump << "[ ";
  if ( aNewElemGroup->_is_nil() ) pyDump << "nothing, ";
  else                            pyDump << aNewElemGroup << ", ";
  if ( aNewNodeGroup->_is_nil() ) pyDump << "nothing ] = ";
  else                            pyDump << aNewNodeGroup << " ] = ";

  pyDump << this << ".DoubleNodeElemGroup2New( " << theElems << ", "
         << theNodesNot        << ", "
         << theAffectedElems   << ", "
         << theElemGroupNeeded << ", "
         << theNodeGroupNeeded <<" )";

  aTwoGroups[0] = aNewElemGroup._retn();
  aTwoGroups[1] = aNewNodeGroup._retn();
  return aTwoGroups._retn();
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  \param theElems - group of of elements (edges or faces) to be replicated
  \param theNodesNot - group of nodes not to replicated
  \param theShape - shape to detect affected elements (element which geometric center
  located on or inside shape).
  The replicated nodes should be associated to affected elements.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodesInRegion(), DoubleNodeGroupsInRegion()
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeElemGroupInRegion(SMESH::SMESH_GroupBase_ptr theElems,
                                                               SMESH::SMESH_GroupBase_ptr theNodesNot,
                                                               GEOM::GEOM_Object_ptr      theShape )

{
  if ( CORBA::is_nil( theElems ) && theElems->GetType() == SMESH::NODE )
    return false;

  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  idSourceToSet( theElems, aMeshDS, anElems, SMDSAbs_All );
  idSourceToSet( theNodesNot, aMeshDS, aNodes, SMDSAbs_Node );

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theShape );
  bool aResult = aMeshEditor.DoubleNodesInRegion( anElems, aNodes, aShape );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  if ( aResult )
    myMesh->SetIsModified( true );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroupInRegion( " << theElems << ", "
                << theNodesNot << ", " << theShape << " )";
  return aResult;
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  This method provided for convenience works as DoubleNodes() described above.
  \param theElems - list of groups of elements (edges or faces) to be replicated
  \param theNodesNot - list of groups of nodes not to replicated
  \param theAffectedElems - group of elements to which the replicated nodes
  should be associated to.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodeGroup(), DoubleNodes(), DoubleNodeElemGroupsNew()
*/
//================================================================================

static void listOfGroupToSet(const SMESH::ListOfGroups& theGrpList,
                             SMESHDS_Mesh*              theMeshDS,
                             TIDSortedElemSet&          theElemSet,
                             const bool                 theIsNodeGrp)
{
  for ( int i = 0, n = theGrpList.length(); i < n; i++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theGrpList[ i ];
    if ( !CORBA::is_nil( aGrp ) && (theIsNodeGrp ? aGrp->GetType() == SMESH::NODE
                                    : aGrp->GetType() != SMESH::NODE ) )
    {
      SMESH::long_array_var anIDs = aGrp->GetIDs();
      arrayToSet( anIDs, theMeshDS, theElemSet, theIsNodeGrp ? SMDSAbs_Node : SMDSAbs_All );
    }
  }
}

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeElemGroups(const SMESH::ListOfGroups& theElems,
                                                        const SMESH::ListOfGroups& theNodesNot,
                                                        const SMESH::ListOfGroups& theAffectedElems)
{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  listOfGroupToSet(theElems, aMeshDS, anElems, false );
  listOfGroupToSet(theNodesNot, aMeshDS, aNodes, true );
  listOfGroupToSet(theAffectedElems, aMeshDS, anAffected, false );

  bool aResult = aMeshEditor.DoubleNodes( anElems, aNodes, anAffected );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  if ( aResult )
    myMesh->SetIsModified( true );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroups( " << &theElems << ", "
                << &theNodesNot << ", " << &theAffectedElems << " )";
  return aResult;
}

//================================================================================
/*!
 * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
 * Works as DoubleNodeElemGroups(), but returns a new group with newly created elements.
  \param theElems - list of groups of elements (edges or faces) to be replicated
  \param theNodesNot - list of groups of nodes not to replicated
  \param theAffectedElems - group of elements to which the replicated nodes
  should be associated to.
 * \return a new group with newly created elements
 * \sa DoubleNodeElemGroups()
 */
//================================================================================

SMESH::SMESH_Group_ptr
SMESH_MeshEditor_i::DoubleNodeElemGroupsNew(const SMESH::ListOfGroups& theElems,
                                            const SMESH::ListOfGroups& theNodesNot,
                                            const SMESH::ListOfGroups& theAffectedElems)
{
  TPythonDump pyDump;
  SMESH::ListOfGroups_var twoGroups = DoubleNodeElemGroups2New( theElems,
                                                                theNodesNot,
                                                                theAffectedElems,
                                                                true, false );
  SMESH::SMESH_GroupBase_var baseGroup = twoGroups[0].in();
  SMESH::SMESH_Group_var     elemGroup = SMESH::SMESH_Group::_narrow( baseGroup );

  pyDump << elemGroup << " = " << this << ".DoubleNodeElemGroupsNew( "
         << theElems         << ", "
         << theNodesNot      << ", "
         << theAffectedElems << " )";

  return elemGroup._retn();
}

SMESH::ListOfGroups*
SMESH_MeshEditor_i::DoubleNodeElemGroups2New(const SMESH::ListOfGroups& theElems,
                                             const SMESH::ListOfGroups& theNodesNot,
                                             const SMESH::ListOfGroups& theAffectedElems,
                                             CORBA::Boolean             theElemGroupNeeded,
                                             CORBA::Boolean             theNodeGroupNeeded)
{
  SMESH::SMESH_Group_var aNewElemGroup, aNewNodeGroup;
  SMESH::ListOfGroups_var aTwoGroups = new SMESH::ListOfGroups();
  aTwoGroups->length( 2 );
  
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  listOfGroupToSet(theElems, aMeshDS, anElems, false );
  listOfGroupToSet(theNodesNot, aMeshDS, aNodes, true );
  listOfGroupToSet(theAffectedElems, aMeshDS, anAffected, false );

  bool aResult = aMeshEditor.DoubleNodes( anElems, aNodes, anAffected );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  TPythonDump pyDump;
  if ( aResult )
  {
    myMesh->SetIsModified( true );

    // Create group with newly created elements
    CORBA::String_var elemGroupName = theElems[0]->GetName();
    string aNewName = generateGroupName( string(elemGroupName.in()) + "_double");
    if ( !aMeshEditor.GetLastCreatedElems().IsEmpty() && theElemGroupNeeded )
    {
      SMESH::long_array_var anIds = GetLastCreatedElems();
      SMESH::ElementType aGroupType = myMesh_i->GetElementType(anIds[0], true);
      aNewElemGroup = myMesh_i->CreateGroup(aGroupType, aNewName.c_str());
      aNewElemGroup->Add(anIds);
    }
    if ( !aMeshEditor.GetLastCreatedNodes().IsEmpty() && theNodeGroupNeeded )
    {
      SMESH::long_array_var anIds = GetLastCreatedNodes();
      aNewNodeGroup = myMesh_i->CreateGroup(SMESH::NODE, aNewName.c_str());
      aNewNodeGroup->Add(anIds);
    }
  }

  // Update Python script

  pyDump << "[ ";
  if ( aNewElemGroup->_is_nil() ) pyDump << "nothing, ";
  else                            pyDump << aNewElemGroup << ", ";
  if ( aNewNodeGroup->_is_nil() ) pyDump << "nothing ] = ";
  else                            pyDump << aNewNodeGroup << " ] = ";

  pyDump << this << ".DoubleNodeElemGroups2New( " << &theElems << ", "
         << &theNodesNot       << ", "
         << &theAffectedElems  << ", "
         << theElemGroupNeeded << ", "
         << theNodeGroupNeeded << " )";

  aTwoGroups[0] = aNewElemGroup._retn();
  aTwoGroups[1] = aNewNodeGroup._retn();
  return aTwoGroups._retn();
}

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements
  This method provided for convenience works as DoubleNodes() described above.
  \param theElems - list of groups of elements (edges or faces) to be replicated
  \param theNodesNot - list of groups of nodes not to replicated
  \param theShape - shape to detect affected elements (element which geometric center
  located on or inside shape).
  The replicated nodes should be associated to affected elements.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodeGroupInRegion(), DoubleNodesInRegion()
*/
//================================================================================

CORBA::Boolean
SMESH_MeshEditor_i::DoubleNodeElemGroupsInRegion(const SMESH::ListOfGroups& theElems,
                                                 const SMESH::ListOfGroups& theNodesNot,
                                                 GEOM::GEOM_Object_ptr      theShape )
{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();
  TIDSortedElemSet anElems, aNodes;
  listOfGroupToSet(theElems, aMeshDS, anElems,false );
  listOfGroupToSet(theNodesNot, aMeshDS, aNodes, true );

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theShape );
  bool aResult = aMeshEditor.DoubleNodesInRegion( anElems, aNodes, aShape );

  storeResult( aMeshEditor) ;

  myMesh->GetMeshDS()->Modified();
  if ( aResult )
    myMesh->SetIsModified( true );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroupsInRegion( " << &theElems << ", "
                << &theNodesNot << ", " << theShape << " )";
  return aResult;
}

//================================================================================
/*!
  \brief Generated skin mesh (containing 2D cells) from 3D mesh
   The created 2D mesh elements based on nodes of free faces of boundary volumes
  \return TRUE if operation has been completed successfully, FALSE otherwise
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::Make2DMeshFrom3D()
{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );
  bool aResult = aMeshEditor.Make2DMeshFrom3D();
  storeResult( aMeshEditor) ;
  myMesh->GetMeshDS()->Modified();
  TPythonDump() << "isDone = " << this << ".Make2DMeshFrom3D()";
  return aResult;
}

//================================================================================
/*!
 * \brief Double nodes on shared faces between groups of volumes and create flat elements on demand.
 * The list of groups must describe a partition of the mesh volumes.
 * The nodes of the internal faces at the boundaries of the groups are doubled.
 * In option, the internal faces are replaced by flat elements.
 * Triangles are transformed in prisms, and quadrangles in hexahedrons.
 * The flat elements are stored in groups of volumes.
 * @param theDomains - list of groups of volumes
 * @param createJointElems - if TRUE, create the elements
 * @return TRUE if operation has been completed successfully, FALSE otherwise
 */
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodesOnGroupBoundaries( const SMESH::ListOfGroups& theDomains,
                                                                 CORBA::Boolean createJointElems )
  throw (SALOME::SALOME_Exception)
{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();

  vector<TIDSortedElemSet> domains;
  domains.clear();

  for ( int i = 0, n = theDomains.length(); i < n; i++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theDomains[ i ];
    if ( !CORBA::is_nil( aGrp ) /*&& ( aGrp->GetType() != SMESH::NODE )*/ )
    {
//      if ( aGrp->GetType() != SMESH::VOLUME )
//        THROW_SALOME_CORBA_EXCEPTION("Not a volume group", SALOME::BAD_PARAM);
      TIDSortedElemSet domain;
      domain.clear();
      domains.push_back(domain);
      SMESH::long_array_var anIDs = aGrp->GetIDs();
      arrayToSet( anIDs, aMeshDS, domains[ i ], SMDSAbs_All );
    }
  }

  bool aResult = aMeshEditor.DoubleNodesOnGroupBoundaries( domains, createJointElems );
  // TODO publish the groups of flat elements in study

  storeResult( aMeshEditor) ;
  myMesh->GetMeshDS()->Modified();

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodesOnGroupBoundaries( " << &theDomains
      << ", " << createJointElems << " )";
  return aResult;
}

//================================================================================
/*!
 * \brief Double nodes on some external faces and create flat elements.
 * Flat elements are mainly used by some types of mechanic calculations.
 *
 * Each group of the list must be constituted of faces.
 * Triangles are transformed in prisms, and quadrangles in hexahedrons.
 * @param theGroupsOfFaces - list of groups of faces
 * @return TRUE if operation has been completed successfully, FALSE otherwise
 */
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::CreateFlatElementsOnFacesGroups( const SMESH::ListOfGroups& theGroupsOfFaces )
{
  initData();

  ::SMESH_MeshEditor aMeshEditor( myMesh );

  SMESHDS_Mesh* aMeshDS = GetMeshDS();

  vector<TIDSortedElemSet> faceGroups;
  faceGroups.clear();

  for ( int i = 0, n = theGroupsOfFaces.length(); i < n; i++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theGroupsOfFaces[ i ];
    if ( !CORBA::is_nil( aGrp ) && ( aGrp->GetType() != SMESH::NODE ) )
    {
      TIDSortedElemSet faceGroup;
      faceGroup.clear();
      faceGroups.push_back(faceGroup);
      SMESH::long_array_var anIDs = aGrp->GetIDs();
      arrayToSet( anIDs, aMeshDS, faceGroups[ i ], SMDSAbs_All );
    }
  }

  bool aResult = aMeshEditor.CreateFlatElementsOnFacesGroups( faceGroups );
  // TODO publish the groups of flat elements in study

  storeResult( aMeshEditor) ;
  myMesh->GetMeshDS()->Modified();

  // Update Python script
  TPythonDump() << "isDone = " << this << ".CreateFlatElementsOnFacesGroups( " << &theGroupsOfFaces << " )";
  return aResult;
}

// issue 20749 ===================================================================
/*!
 * \brief Creates missing boundary elements
 *  \param elements - elements whose boundary is to be checked
 *  \param dimension - defines type of boundary elements to create
 *  \param groupName - a name of group to store created boundary elements in,
 *                     "" means not to create the group
 *  \param meshName - a name of new mesh to store created boundary elements in,
 *                     "" means not to create the new mesh
 *  \param toCopyElements - if true, the checked elements will be copied into the new mesh
 *  \param toCopyExistingBondary - if true, not only new but also pre-existing
 *                                boundary elements will be copied into the new mesh
 *  \param group - returns the create group, if any
 *  \retval SMESH::SMESH_Mesh - the mesh where elements were added to
 */
// ================================================================================

SMESH::SMESH_Mesh_ptr
SMESH_MeshEditor_i::MakeBoundaryMesh(SMESH::SMESH_IDSource_ptr idSource,
                                     SMESH::Bnd_Dimension      dim,
                                     const char*               groupName,
                                     const char*               meshName,
                                     CORBA::Boolean            toCopyElements,
                                     CORBA::Boolean            toCopyExistingBondary,
                                     SMESH::SMESH_Group_out    group)
{
  initData();

  if ( dim > SMESH::BND_1DFROM2D )
    THROW_SALOME_CORBA_EXCEPTION("Invalid boundary dimension", SALOME::BAD_PARAM);

  SMESHDS_Mesh* aMeshDS = GetMeshDS();

  SMESH::SMESH_Mesh_var mesh_var;
  SMESH::SMESH_Group_var group_var;

  TPythonDump pyDump;

  TIDSortedElemSet elements;
  SMDSAbs_ElementType elemType = (dim == SMESH::BND_1DFROM2D) ? SMDSAbs_Face : SMDSAbs_Volume;
  if ( idSourceToSet( idSource, aMeshDS, elements, elemType,/*emptyIfIsMesh=*/true ))
  {
    // mesh to fill in
    mesh_var =
      strlen(meshName) ? makeMesh(meshName) : SMESH::SMESH_Mesh::_duplicate(myMesh_i->_this());
    SMESH_Mesh_i* mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh_var );
    // other mesh
    SMESH_Mesh* smesh_mesh = (mesh_i==myMesh_i) ? (SMESH_Mesh*)0 : &mesh_i->GetImpl();

    // group of new boundary elements
    SMESH_Group* smesh_group = 0;
    if ( strlen(groupName) )
    {
      group_var = mesh_i->CreateGroup( SMESH::ElementType(int(elemType)-1),groupName);
      if ( SMESH_GroupBase_i* group_i = SMESH::DownCast<SMESH_GroupBase_i*>( group_var ))
        smesh_group = group_i->GetSmeshGroup();
    }

    // do it
    ::SMESH_MeshEditor aMeshEditor( myMesh );
    aMeshEditor.MakeBoundaryMesh( elements,
                                  ::SMESH_MeshEditor::Bnd_Dimension(dim),
                                  smesh_group,
                                  smesh_mesh,
                                  toCopyElements,
                                  toCopyExistingBondary);
    storeResult( aMeshEditor );

    if ( smesh_mesh )
      smesh_mesh->GetMeshDS()->Modified();
  }

  const char* dimName[] = { "BND_2DFROM3D", "BND_1DFROM3D", "BND_1DFROM2D" };

  // result of MakeBoundaryMesh() is a tuple (mesh, group)
  if ( mesh_var->_is_nil() )
    pyDump << myMesh_i->_this() << ", ";
  else
    pyDump << mesh_var << ", ";
  if ( group_var->_is_nil() )
    pyDump << "_NoneGroup = "; // assignment to None is forbiden
  else
    pyDump << group_var << " = ";
  pyDump << this << ".MakeBoundaryMesh( "
         << idSource << ", "
         << "SMESH." << dimName[int(dim)] << ", "
         << "'" << groupName << "', "
         << "'" << meshName<< "', "
         << toCopyElements << ", "
         << toCopyExistingBondary << ")";

  group = group_var._retn();
  return mesh_var._retn();
}

//================================================================================
/*!
 * \brief Creates missing boundary elements
 *  \param dimension - defines type of boundary elements to create
 *  \param groupName - a name of group to store all boundary elements in,
 *    "" means not to create the group
 *  \param meshName - a name of a new mesh, which is a copy of the initial 
 *    mesh + created boundary elements; "" means not to create the new mesh
 *  \param toCopyAll - if true, the whole initial mesh will be copied into
 *    the new mesh else only boundary elements will be copied into the new mesh
 *  \param groups - optional groups of elements to make boundary around
 *  \param mesh - returns the mesh where elements were added to
 *  \param group - returns the created group, if any
 *  \retval long - number of added boundary elements
 */
//================================================================================

CORBA::Long SMESH_MeshEditor_i::MakeBoundaryElements(SMESH::Bnd_Dimension dim,
                                                     const char* groupName,
                                                     const char* meshName,
                                                     CORBA::Boolean toCopyAll,
                                                     const SMESH::ListOfIDSources& groups,
                                                     SMESH::SMESH_Mesh_out mesh,
                                                     SMESH::SMESH_Group_out group)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  initData();

  if ( dim > SMESH::BND_1DFROM2D )
    THROW_SALOME_CORBA_EXCEPTION("Invalid boundary dimension", SALOME::BAD_PARAM);

  // separate groups belonging to this and other mesh
  SMESH::ListOfIDSources_var groupsOfThisMesh = new SMESH::ListOfIDSources;
  SMESH::ListOfIDSources_var groupsOfOtherMesh = new SMESH::ListOfIDSources;
  groupsOfThisMesh->length( groups.length() );
  groupsOfOtherMesh->length( groups.length() );
  int nbGroups = 0, nbGroupsOfOtherMesh = 0;
  for ( int i = 0; i < groups.length(); ++i )
  {
    SMESH::SMESH_Mesh_var m = groups[i]->GetMesh();
    if ( myMesh_i != SMESH::DownCast<SMESH_Mesh_i*>( m ))
      groupsOfOtherMesh[ nbGroupsOfOtherMesh++ ] = groups[i];
    else
      groupsOfThisMesh[ nbGroups++ ] = groups[i];
    if ( SMESH::DownCast<SMESH_Mesh_i*>( groups[i] ))
      THROW_SALOME_CORBA_EXCEPTION("expect a group but recieve a mesh", SALOME::BAD_PARAM);
  }
  groupsOfThisMesh->length( nbGroups );
  groupsOfOtherMesh->length( nbGroupsOfOtherMesh );

  int nbAdded = 0;
  TPythonDump pyDump;

  if ( nbGroupsOfOtherMesh > 0 )
  {
    // process groups belonging to another mesh
    SMESH::SMESH_Mesh_var    otherMesh = groupsOfOtherMesh[0]->GetMesh();
    SMESH::SMESH_MeshEditor_var editor = otherMesh->GetMeshEditor();
    nbAdded += editor->MakeBoundaryElements( dim, groupName, meshName, toCopyAll,
                                             groupsOfOtherMesh, mesh, group );
  }

  SMESH::SMESH_Mesh_var mesh_var;
  SMESH::SMESH_Group_var group_var;

  // get mesh to fill
  mesh_var = SMESH::SMESH_Mesh::_duplicate( myMesh_i->_this() );
  const bool toCopyMesh = ( strlen( meshName ) > 0 );
  if ( toCopyMesh )
  {
    if ( toCopyAll )
      mesh_var = SMESH_Gen_i::GetSMESHGen()->CopyMesh(mesh_var,
                                                      meshName,
                                                      /*toCopyGroups=*/false,
                                                      /*toKeepIDs=*/true);
    else
      mesh_var = makeMesh(meshName);
  }
  SMESH_Mesh_i* mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh_var );
  SMESH_Mesh*  tgtMesh = &mesh_i->GetImpl();

  // source mesh
  SMESH_Mesh*     srcMesh = ( toCopyMesh && !toCopyAll ) ? myMesh : tgtMesh;
  SMESHDS_Mesh* srcMeshDS = srcMesh->GetMeshDS();

  // group of boundary elements
  SMESH_Group* smesh_group = 0;
  SMDSAbs_ElementType elemType = (dim == SMESH::BND_2DFROM3D) ? SMDSAbs_Volume : SMDSAbs_Face;
  if ( strlen(groupName) )
  {
    SMESH::ElementType groupType = SMESH::ElementType( int(elemType)-1 );
    group_var = mesh_i->CreateGroup( groupType, groupName );
    if ( SMESH_GroupBase_i* group_i = SMESH::DownCast<SMESH_GroupBase_i*>( group_var ))
      smesh_group = group_i->GetSmeshGroup();
  }

  TIDSortedElemSet elements;

  if ( groups.length() > 0 )
  {
    for ( int i = 0; i < nbGroups; ++i )
    {
      elements.clear();
      if ( idSourceToSet( groupsOfThisMesh[i], srcMeshDS, elements, elemType,/*emptyIfIsMesh=*/0 ))
      {
        SMESH::Bnd_Dimension bdim = 
          ( elemType == SMDSAbs_Volume ) ? SMESH::BND_2DFROM3D : SMESH::BND_1DFROM2D;
        ::SMESH_MeshEditor aMeshEditor( srcMesh );
        nbAdded += aMeshEditor.MakeBoundaryMesh( elements,
                                                 ::SMESH_MeshEditor::Bnd_Dimension(bdim),
                                                 smesh_group,
                                                 tgtMesh,
                                                 /*toCopyElements=*/false,
                                                 /*toCopyExistingBondary=*/srcMesh != tgtMesh,
                                                 /*toAddExistingBondary=*/true,
                                                 /*aroundElements=*/true);
        storeResult( aMeshEditor );
      }
    }
  }
  else
  {
    ::SMESH_MeshEditor aMeshEditor( srcMesh );
    nbAdded += aMeshEditor.MakeBoundaryMesh( elements,
                                             ::SMESH_MeshEditor::Bnd_Dimension(dim),
                                             smesh_group,
                                             tgtMesh,
                                             /*toCopyElements=*/false,
                                             /*toCopyExistingBondary=*/srcMesh != tgtMesh,
                                             /*toAddExistingBondary=*/true);
    storeResult( aMeshEditor );
  }
  tgtMesh->GetMeshDS()->Modified();

  const char* dimName[] = { "BND_2DFROM3D", "BND_1DFROM3D", "BND_1DFROM2D" };

  // result of MakeBoundaryElements() is a tuple (nb, mesh, group)
  pyDump << "nbAdded, ";
  if ( mesh_var->_is_nil() )
    pyDump << myMesh_i->_this() << ", ";
  else
    pyDump << mesh_var << ", ";
  if ( group_var->_is_nil() )
    pyDump << "_NoneGroup = "; // assignment to None is forbiden
  else
    pyDump << group_var << " = ";
  pyDump << this << ".MakeBoundaryElements( "
         << "SMESH." << dimName[int(dim)] << ", "
         << "'" << groupName << "', "
         << "'" << meshName<< "', "
         << toCopyAll << ", "
         << groups << ")";

  mesh  = mesh_var._retn();
  group = group_var._retn();
  return nbAdded;
}
