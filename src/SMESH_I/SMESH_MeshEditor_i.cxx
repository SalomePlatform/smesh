//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_MeshEditor_i.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH

#include "SMESH_MeshEditor_i.hxx"

#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMDS_PolyhedralVolumeOfNodes.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_PythonDump.hxx"

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

#define cast2Node(elem) static_cast<const SMDS_MeshNode*>( elem )

using namespace std;
using SMESH::TPythonDump;

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
      _isShapeToMesh = _id =_studyId =_idDoc = 0;
      _myMeshDS  = new SMESHDS_Mesh( _id, true );
      myPreviewType = previewElements;
    }
    //!< Destructor
    virtual ~TPreviewMesh() { delete _myMeshDS; }
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

        if ( const SMDS_MeshElement* anElemCopy = Copy( anElem ))
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
        const SMDS_PolyhedralVolumeOfNodes* ph =
          dynamic_cast<const SMDS_PolyhedralVolumeOfNodes*> (anElem);
        if ( ph )
          anElemCopy = _myMeshDS->AddPolyhedralVolumeWithID
            (anElemNodesID, ph->GetQuanities(),anElem->GetID());
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

  static SMESH_NodeSearcher * myNodeSearcher = 0;

  //=============================================================================
  /*!
   * \brief Deleter of myNodeSearcher at any compute event occured
   */
  //=============================================================================

  struct TNodeSearcherDeleter : public SMESH_subMeshEventListener
  {
    SMESH_Mesh* myMesh;
    //!< Constructor
    TNodeSearcherDeleter(): SMESH_subMeshEventListener( false ), // won't be deleted by submesh
    myMesh(0) {}
    //!< Delete myNodeSearcher
    static void Delete()
    {
      if ( myNodeSearcher ) { delete myNodeSearcher; myNodeSearcher = 0; }
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
    //!< set self on all submeshes and delete myNodeSearcher if other mesh is set
    void Set(SMESH_Mesh* mesh)
    {
      if ( myMesh && myMesh != mesh ) {
        Delete();
        Unset( myMesh );
      }
      myMesh = mesh;
      if ( SMESH_subMesh* myMainSubMesh = mesh->GetSubMeshContaining(1) ) {
        const TDependsOnMap & subMeshes = myMainSubMesh->DependsOn();
        TDependsOnMap::const_iterator sm;
        for (sm = subMeshes.begin(); sm != subMeshes.end(); sm++)
          sm->second->SetEventListener( this, 0, sm->second );
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
    }
  };

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

void SMESH_MeshEditor_i::initData()
{
  if ( myPreviewMode ) {
    myPreviewData = new SMESH::MeshPreviewStruct();
  }
  else {
    myLastCreatedElems = new SMESH::long_array();
    myLastCreatedNodes = new SMESH::long_array();
    TNodeSearcherDeleter::Delete();
  }
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
#ifdef _DEBUG_
  TPythonDump() << "print 'RemoveElements: ', isDone";
#endif
  // Remove Elements
  return anEditor.Remove( IdList, false );
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
#ifdef _DEBUG_
  TPythonDump() << "print 'RemoveNodes: ', isDone";
#endif

  return anEditor.Remove( IdList, true );
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

  if(elem)
    return elem->GetID();

  return 0;
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
                << x << ", " << y << ", " << z << " )";

  return N->GetID();
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

  // Update Python script
  TPythonDump() << "faceID = " << this << ".AddFace( " << IDsOfNodes << " )";

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolygonalFace
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolygonalFace
                                   (const SMESH::long_array & IDsOfNodes)
{
  initData();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolygonalFace(nodes);
  
  // Update Python script
  TPythonDump() <<"faceID = "<<this<<".AddPolygonalFace( "<<IDsOfNodes<<" )";
#ifdef _DEBUG_
  TPythonDump() << "print 'AddPolygonalFace: ', faceID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
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
  }

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddVolume( " << IDsOfNodes << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'AddVolume: ', volID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolume
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolume
                                   (const SMESH::long_array & IDsOfNodes,
                                    const SMESH::long_array & Quantities)
{
  initData();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> n (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    n[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  int NbFaces = Quantities.length();
  std::vector<int> q (NbFaces);
  for (int j = 0; j < NbFaces; j++)
    q[j] = Quantities[j];

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolyhedralVolume(n, q);

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddPolyhedralVolume( "
                << IDsOfNodes << ", " << Quantities << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'AddPolyhedralVolume: ', volID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolumeByFaces
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolumeByFaces
                                   (const SMESH::long_array & IdsOfFaces)
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
#ifdef _DEBUG_
  TPythonDump() << "print 'AddPolyhedralVolume: ', volID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
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
}


//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::MoveNode(CORBA::Long   NodeID,
                                            CORBA::Double x,
                                            CORBA::Double y,
                                            CORBA::Double z)
{
  initData();

  const SMDS_MeshNode * node = GetMeshDS()->FindNode( NodeID );
  if ( !node )
    return false;

  GetMeshDS()->MoveNode(node, x, y, z);

  // Update Python script
  TPythonDump() << "isDone = " << this << ".MoveNode( "
                << NodeID << ", " << x << ", " << y << ", " << z << " )";

  return true;
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
  return aMeshEditor.InverseDiag ( n1, n2 );
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

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = Reorient(anElementsId);

  // Clear python line, created by Reorient()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TPythonDump() << "isDone = " << this << ".ReorientObject( " << theObject << " )";

  return isDone;
}

namespace
{
  //================================================================================
  /*!
   * \brief function for conversion long_array to TIDSortedElemSet
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
      const SMDS_MeshElement * elem = aMesh->FindElement(ind);
      if ( elem && ( aType == SMDSAbs_All || elem->GetType() == aType ))
        aMap.insert( elem );
    }
  }
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
                << IDsOfElements << ", " << aNumericalFunctor << ", " << MaxAngle << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'TriToQuad: ', isDone";
#endif

  ::SMESH_MeshEditor anEditor( myMesh );

  bool stat = anEditor.TriToQuad( faces, aCrit, MaxAngle );

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

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = TriToQuad(anElementsId, Criterion, MaxAngle);

  // Clear python line(s), created by TriToQuad()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    SMESH::DownCast<SMESH::NumericalFunctor_i*>( Criterion );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".TriToQuadObject("
                << theObject << ", " << aNumericalFunctor << ", " << MaxAngle << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'TriToQuadObject: ', isDone";
#endif

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
#ifdef _DEBUG_
  TPythonDump() << "print 'QuadToTri: ', isDone";
#endif

  ::SMESH_MeshEditor anEditor( myMesh );
  CORBA::Boolean stat = anEditor.QuadToTri( faces, aCrit );

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

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = QuadToTri(anElementsId, Criterion);

  // Clear python line(s), created by QuadToTri()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    SMESH::DownCast<SMESH::NumericalFunctor_i*>( Criterion );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".QuadToTriObject( " << theObject << ", " << aNumericalFunctor << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'QuadToTriObject: ', isDone";
#endif

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
#ifdef _DEBUG_
  TPythonDump() << "print 'SplitQuad: ', isDone";
#endif

  ::SMESH_MeshEditor anEditor( myMesh );
  CORBA::Boolean stat = anEditor.QuadToTri( faces, Diag13 );

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

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = SplitQuad(anElementsId, Diag13);

  // Clear python line(s), created by SplitQuad()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  // Update Python script
  TPythonDump() << "isDone = " << this << ".SplitQuadObject( "
                << theObject << ", " << Diag13 << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'SplitQuadObject: ', isDone";
#endif

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

  storeResult(anEditor);

  // Update Python script
  TPythonDump() << "isDone = " << this << "."
                << (IsParametric ? "SmoothParametric( " : "Smooth( ")
                << IDsOfElements << ", "     << IDsOfFixedNodes << ", "
                << MaxNbOfIterations << ", " << MaxAspectRatio << ", "
                << "SMESH.SMESH_MeshEditor."
                << ( Method == SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH ?
                     "CENTROIDAL_SMOOTH )" : "LAPLACIAN_SMOOTH )");
#ifdef _DEBUG_
  TPythonDump() << "print 'Smooth: ', isDone";
#endif

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

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = smooth (anElementsId, IDsOfFixedNodes, MaxNbOfIterations,
                                  MaxAspectRatio, Method, IsParametric);

  // Clear python line(s), created by Smooth()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  // Update Python script
  TPythonDump() << "isDone = " << this << "."
                << (IsParametric ? "SmoothParametricObject( " : "SmoothObject( ")
                << theObject << ", " << IDsOfFixedNodes << ", "
                << MaxNbOfIterations << ", " << MaxAspectRatio << ", "
                << "SMESH.SMESH_MeshEditor."
                << ( Method == SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH ?
                     "CENTROIDAL_SMOOTH )" : "LAPLACIAN_SMOOTH )");
#ifdef _DEBUG_
  TPythonDump() << "print 'SmoothObject: ', isDone";
#endif

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
                  << theIDsOfElements << ", "
                  << theAxis << ", "
                  << theAngleInRadians << ", "
                  << theNbOfSteps << ", "
                  << theTolerance << " )";
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
  SMESH::ListOfGroups *aGroups = rotationSweep(theIDsOfElements,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump<< this << ".RotationSweepMakeGroups( "
               << theIDsOfElements << ", "
               << theAxis << ", "
               << theAngleInRadians << ", "
               << theNbOfSteps << ", "
               << theTolerance << " )";
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups *aGroups = rotationSweep(anElementsId,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump<< this << ".RotationSweepObjectMakeGroups( "
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups *aGroups = rotationSweep(anElementsId,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true,
					       SMDSAbs_Edge);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump<< this << ".RotationSweepObject1DMakeGroups( "
               << theObject << ", "
	       << theAxis << ", "
               << theAngleInRadians << ", "
               << theNbOfSteps << ", "
               << theTolerance << " )";
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups *aGroups = rotationSweep(anElementsId,
                                               theAxis,
                                               theAngleInRadians,
                                               theNbOfSteps,
                                               theTolerance,
                                               true,
					       SMDSAbs_Face);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump<< this << ".RotationSweepObject2DMakeGroups( "
               << theObject << ", "
	       << theAxis << ", "
               << theAngleInRadians << ", "
               << theNbOfSteps << ", "
               << theTolerance << " )";
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
                                   const bool                theMakeGroups,
                                   const SMDSAbs_ElementType theElementType)
{
  initData();

  try {   
#ifdef NO_CAS_CATCH
    OCC_CATCH_SIGNALS;
#endif
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, GetMeshDS(), elements, theElementType);

    const SMESH::PointStruct * P = &theStepVector.PS;
    gp_Vec stepVec( P->x, P->y, P->z );

    TElemOfElemListMap aHystory;
    ::SMESH_MeshEditor anEditor( myMesh );
    ::SMESH_MeshEditor::PGroupIDs groupIds =
        anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps, aHystory, theMakeGroups);

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
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".ExtrusionSweep( "
                  << theIDsOfElements << ", " << theStepVector <<", " << theNbOfSteps << " )";
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
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".ExtrusionSweepObject( "
                  << theObject << ", " << theStepVector << ", " << theNbOfSteps << " )";
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
                  << theObject << ", " << theStepVector << ", " << theNbOfSteps << " )";
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
                  << theObject << ", " << theStepVector << ", " << theNbOfSteps << " )";
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
  SMESH::ListOfGroups* aGroups = extrusionSweep (theIDsOfElements, theStepVector, theNbOfSteps, true );
    
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump  << this << ".ExtrusionSweepMakeGroups( "
                 << theIDsOfElements << ", " << theStepVector <<", " << theNbOfSteps << " )";
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionSweep (anElementsId, theStepVector, theNbOfSteps, true );
  
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump<< this << ".ExtrusionSweepObjectMakeGroups( "
               << theObject << ", " << theStepVector << ", " << theNbOfSteps << " )";
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionSweep (anElementsId, theStepVector, theNbOfSteps, true, SMDSAbs_Edge );
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".ExtrusionSweepObject1DMakeGroups( "
		<< theObject << ", " << theStepVector << ", " << theNbOfSteps << " )";
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = extrusionSweep (anElementsId, theStepVector, theNbOfSteps, true, SMDSAbs_Face );
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".ExtrusionSweepObject2DMakeGroups( "
		<< theObject << ", " << theStepVector << ", " << theNbOfSteps << " )";
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
  SMESH::ListOfGroups * aGroups = advancedExtrusion( theIDsOfElements,
                                                    theStepVector,
                                                    theNbOfSteps,
                                                    theExtrFlags,
                                                    theSewTolerance,
                                                    true);
  
  if ( !myPreviewMode ) {
    TPythonDump() << "stepVector = " << theStepVector;
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
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
                                    theHasAngles, angles,
                                    theHasRefPoint, refPnt, theMakeGroups );
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
  if ( !myPreviewMode ) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    TPythonDump aPythonDump;
    if(isDumpGroups) {
      aPythonDump << "("<<aGroups;
    }
    if(isDumpGroups)
      aPythonDump << ", error)";
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
  
  if ( !myPreviewMode ) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    TPythonDump aPythonDump;
    if(isDumpGroups) {
      aPythonDump << "("<<aGroups;
    }
    if(isDumpGroups)
      aPythonDump << ", error)";
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
  
  if ( !myPreviewMode ) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    TPythonDump aPythonDump;
    if(isDumpGroups) {
      aPythonDump << "("<<aGroups;
    }
    if(isDumpGroups)
      aPythonDump << ", error)";
    else
      aPythonDump <<"error";

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
  
  if ( !myPreviewMode ) {
    bool isDumpGroups = aGroups && aGroups->length() > 0;
    TPythonDump aPythonDump;
    if(isDumpGroups) {
      aPythonDump << "("<<aGroups;
    }
    if(isDumpGroups)
      aPythonDump << ", error)";
    else
      aPythonDump <<"error";

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
SMESH_MeshEditor_i::mirror(const SMESH::long_array &           theIDsOfElements,
                           const SMESH::AxisStruct &           theAxis,
                           SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                           CORBA::Boolean                      theCopy,
                           const bool                          theMakeGroups,
                           ::SMESH_Mesh*                       theTargetMesh)
{
  initData();

  TIDSortedElemSet elements;
  arrayToSet(theIDsOfElements, GetMeshDS(), elements);

  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

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

  ::SMESH_MeshEditor anEditor( myMesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.Transform (elements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if(theCopy) {
    storeResult(anEditor);
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
                  << theIDsOfElements << ", "
                  << theAxis          << ", "
                  << mirrorTypeName(theMirrorType) << ", "
                  << theCopy          << " )";
  }
  mirror(theIDsOfElements, theAxis, theMirrorType, theCopy, false);
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
                  << theObject << ", "
                  << theAxis   << ", "
                  << mirrorTypeName(theMirrorType) << ", "
                  << theCopy   << " )";
  }
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  mirror(anElementsId, theAxis, theMirrorType, theCopy, false);
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
  SMESH::ListOfGroups * aGroups = mirror(theIDsOfElements, theMirror, theMirrorType, true, true);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".MirrorMakeGroups( "
                << theIDsOfElements << ", "
                << theMirror << ", "
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = mirror(anElementsId, theMirror, theMirrorType, true, true);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".MirrorObjectMakeGroups( "
                << theObject << ", "
                << theMirror << ", "
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
    if (mesh_i) {
      mirror(theIDsOfElements, theMirror, theMirrorType,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".MirrorMakeMesh( "
             << theIDsOfElements << ", "
             << theMirror   << ", "
             << mirrorTypeName(theMirrorType) << ", "
             << theCopyGroups << ", '"
             << theMeshName << "' )";
    }
  }

  //dump "GetGroups"
  if(!myPreviewMode && mesh_i)
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
    if ( mesh_i ) {
      SMESH::long_array_var anElementsId = theObject->GetIDs();
      mirror(anElementsId, theMirror, theMirrorType,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }

    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".MirrorObjectMakeMesh( "
             << theObject << ", "
             << theMirror   << ", "
             << mirrorTypeName(theMirrorType) << ", "
             << theCopyGroups << ", '"
             << theMeshName << "' )";
    }
  } 

  //dump "GetGroups"
  if(!myPreviewMode && mesh_i)
    mesh_i->GetGroups();
  
  return mesh._retn();
}

//=======================================================================
//function : translate
//purpose  : 
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::translate(const SMESH::long_array & theIDsOfElements,
                              const SMESH::DirStruct &  theVector,
                              CORBA::Boolean            theCopy,
                              const bool                theMakeGroups,
                              ::SMESH_Mesh*             theTargetMesh)
{
  initData();

  TIDSortedElemSet elements;
  arrayToSet(theIDsOfElements, GetMeshDS(), elements);

  gp_Trsf aTrsf;
  const SMESH::PointStruct * P = &theVector.PS;
  aTrsf.SetTranslation( gp_Vec( P->x, P->y, P->z ));

  ::SMESH_MeshEditor anEditor( myMesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.Transform (elements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if(theCopy)
    storeResult(anEditor);

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
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".Translate( "
                  << theIDsOfElements << ", "
                  << theVector << ", "
                  << theCopy << " )";
  }
  translate(theIDsOfElements,
            theVector,
            theCopy,
            false);
}

//=======================================================================
//function : TranslateObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::TranslateObject(SMESH::SMESH_IDSource_ptr theObject,
					 const SMESH::DirStruct &  theVector,
					 CORBA::Boolean            theCopy)
{
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".TranslateObject( "
                  << theObject << ", "
                  << theVector << ", "
                  << theCopy << " )";
  }
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  translate(anElementsId,
            theVector,
            theCopy,
            false);
}

//=======================================================================
//function : TranslateMakeGroups
//purpose  : 
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::TranslateMakeGroups(const SMESH::long_array& theIDsOfElements,
                                        const SMESH::DirStruct&  theVector)
{
  SMESH::ListOfGroups * aGroups = translate(theIDsOfElements,theVector,true,true);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".TranslateMakeGroups( "
                << theIDsOfElements << ", "
                << theVector << " )";
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups = translate(anElementsId, theVector, true, true);
  
  if ( !myPreviewMode ) {

    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
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
    
    if ( mesh_i ) {
      translate(theIDsOfElements, theVector,
                false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".TranslateMakeMesh( "
             << theIDsOfElements << ", "
             << theVector   << ", "
             << theCopyGroups << ", '"
             << theMeshName << "' )";
    }
  }
  
  //dump "GetGroups"
  if(!myPreviewMode && mesh_i)
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
    
    if ( mesh_i ) {
      SMESH::long_array_var anElementsId = theObject->GetIDs();
      translate(anElementsId, theVector,
                false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".TranslateObjectMakeMesh( "
             << theObject << ", "
             << theVector   << ", "
             << theCopyGroups << ", '"
             << theMeshName << "' )";
    }
  }
  
  //dump "GetGroups"
  if(!myPreviewMode && mesh_i)
    mesh_i->GetGroups();
  
  return mesh._retn();
}

//=======================================================================
//function : rotate
//purpose  : 
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::rotate(const SMESH::long_array & theIDsOfElements,
                           const SMESH::AxisStruct & theAxis,
                           CORBA::Double             theAngle,
                           CORBA::Boolean            theCopy,
                           const bool                theMakeGroups,
                           ::SMESH_Mesh*             theTargetMesh)
{
  initData();

  TIDSortedElemSet elements;
  arrayToSet(theIDsOfElements, GetMeshDS(), elements);

  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  gp_Trsf aTrsf;
  aTrsf.SetRotation( gp_Ax1( P, V ), theAngle);

  ::SMESH_MeshEditor anEditor( myMesh );
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      anEditor.Transform (elements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if(theCopy) {
    storeResult(anEditor);
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
  if ( !myPreviewMode ) {
    TPythonDump() << this << ".Rotate( "
                  << theIDsOfElements << ", "
                  << theAxis << ", "
                  << theAngle << ", "
                  << theCopy << " )";
  }
  rotate(theIDsOfElements,
         theAxis,
         theAngle,
         theCopy,
         false);
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
                  << theObject << ", "
                  << theAxis << ", "
                  << theAngle << ", "
                  << theCopy << " )";
  }
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  rotate(anElementsId,
         theAxis,
         theAngle,
         theCopy,
         false);
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
  SMESH::ListOfGroups * aGroups =  rotate(theIDsOfElements,theAxis,theAngle,true,true);
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".RotateMakeGroups( "
                << theIDsOfElements << ", "
		<< theAxis << ", "
                << theAngle << " )";
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
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::ListOfGroups * aGroups =  rotate(anElementsId,theAxis,theAngle,true,true);
 
  if ( !myPreviewMode ) {
    TPythonDump aPythonDump;
    DumpGroupsList(aPythonDump,aGroups);
    aPythonDump << this << ".RotateObjectMakeGroups( "
		<< theObject << ", "
		<< theAxis << ", "
		<< theAngle << " )";
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
    
    if ( mesh_i ) {
      rotate(theIDsOfElements, theAxis, theAngleInRadians,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".RotateMakeMesh( "
             << theIDsOfElements << ", "
             << theAxis << ", "
             << theAngleInRadians   << ", "
             << theCopyGroups << ", '"
             << theMeshName << "' )";
    }
  }
  
  //dump "GetGroups"
  if(!myPreviewMode && mesh_i)
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
    
    if (mesh_i ) {
      SMESH::long_array_var anElementsId = theObject->GetIDs();
      rotate(anElementsId, theAxis, theAngleInRadians,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myPreviewMode ) {
      pydump << mesh << " = " << this << ".RotateObjectMakeMesh( "
             << theObject << ", "
             << theAxis << ", "
             << theAngleInRadians   << ", "
             << theCopyGroups << ", '"
             << theMeshName << "' )";
    }
  }
  
  //dump "GetGroups"
  if(!myPreviewMode && mesh_i)
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
  set<const SMDS_MeshNode*> nodes; // no input nodes
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
  SMESH::long_array_var aElementsId = theObject->GetIDs();

  SMESHDS_Mesh* aMesh = GetMeshDS();
  set<const SMDS_MeshNode*> nodes;

  if ( !CORBA::is_nil(SMESH::SMESH_GroupBase::_narrow(theObject)) &&
      SMESH::SMESH_GroupBase::_narrow(theObject)->GetType() == SMESH::NODE) {
    for(int i = 0; i < aElementsId->length(); i++) {
      CORBA::Long ind = aElementsId[i];
      const SMDS_MeshNode * elem = aMesh->FindNode(ind);
      if(elem)
        nodes.insert(elem);
    }
  }
  else {
    for(int i = 0; i < aElementsId->length(); i++) {
      CORBA::Long ind = aElementsId[i];
      const SMDS_MeshElement * elem = aMesh->FindElement(ind);
      if(elem) {
        SMDS_ElemIteratorPtr nIt = elem->nodesIterator();
        while ( nIt->more() )
          nodes.insert( nodes.end(),static_cast<const SMDS_MeshNode*>(nIt->next()));
      }
    }
  }
    
  
  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  ::SMESH_MeshEditor anEditor( myMesh );
  if(!nodes.empty())
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
  TPythonDump() << "coincident_nodes_on_part = " << this << ".FindCoincidentNodesOnPart( "
                <<theObject<<", "
                << Tolerance << " )";
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
}

//=======================================================================
//function : FindEqualElements
//purpose  :
//=======================================================================
void SMESH_MeshEditor_i::FindEqualElements(SMESH::SMESH_IDSource_ptr      theObject,
					   SMESH::array_of_long_array_out GroupsOfElementsID)
{
  initData();
  if ( !(!CORBA::is_nil(SMESH::SMESH_GroupBase::_narrow(theObject)) &&
	 SMESH::SMESH_GroupBase::_narrow(theObject)->GetType() == SMESH::NODE) ) {
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

  TPythonDump() << this << ".MergeEqualElements()";
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
  // We keep myNodeSearcher until any mesh modification:
  // 1) initData() deletes myNodeSearcher at any edition,
  // 2) TNodeSearcherDeleter - at any mesh compute event and mesh change

  initData();

  int nodeID = theNodeID;
  const SMDS_MeshNode* node = GetMeshDS()->FindNode( nodeID );
  if ( !node )
  {
    static TNodeSearcherDeleter deleter;
    deleter.Set( myMesh );
    if ( !myNodeSearcher ) {
      ::SMESH_MeshEditor anEditor( myMesh );
      myNodeSearcher = anEditor.GetNodeSearcher();
    }
    gp_Pnt p( x,y,z );
    node = myNodeSearcher->FindClosestTo( p );
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
        SMDS_MeshEdge edge( node, cast2Node( *nIt ));
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
    else
    {
      GetMeshDS()->MoveNode(node, x, y, z);
    }
  }

  if ( !myPreviewMode ) {
    TPythonDump() << "nodeID = " << this
                  << ".MoveClosestNodeToPoint( "<< x << ", " << y << ", " << z
                  << ", " << nodeID << " )";
  }

  return nodeID;
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
#ifdef _DEBUG_
  TPythonDump() << "print 'ChangeElemNodes: ', isDone";
#endif

  return GetMeshDS()->ChangeElementNodes( elem, & aNodes[0], nbn1+1 );
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
    // add new nodes into myLastCreatedNodes
    const SMESH_SequenceOfElemPtr& aSeq = anEditor.GetLastCreatedNodes();
    myLastCreatedNodes->length(aSeq.Length());
    for(int i=0; i<aSeq.Length(); i++)
      myLastCreatedNodes[i] = aSeq.Value(i+1)->GetID();
  }
  {
    // add new elements into myLastCreatedElems
    const SMESH_SequenceOfElemPtr& aSeq = anEditor.GetLastCreatedElems();
    myLastCreatedElems->length(aSeq.Length());
    for(int i=0; i<aSeq.Length(); i++)
      myLastCreatedElems[i] = aSeq.Value(i+1)->GetID();
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
  return isDone;
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

  SALOMEDS::StudyBuilder_var builder = study->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr
    = builder->FindOrCreateAttribute( meshSO, "AttributePixMap" );
  SALOMEDS::AttributePixMap::_narrow( anAttr )->SetPixMap( "ICON_SMESH_TREE_MESH_IMPORTED" );

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

  storeResult( aMeshEditor) ;

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
  return DoubleNodes( aNodes, theModifiedElems );
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

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeGroup( 
  SMESH::SMESH_GroupBase_ptr theNodes,
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

  return DoubleNodes( aNodes, aModifiedElems );
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

CORBA::Boolean SMESH_MeshEditor_i::DoubleNodeGroups( 
  const SMESH::ListOfGroups& theNodes,
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

  return aResult;
}
