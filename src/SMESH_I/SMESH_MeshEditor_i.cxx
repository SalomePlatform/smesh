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
//  File   : SMESH_MeshEditor_i.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH

#ifdef WIN32
#define NOMINMAX
#endif

// A macro used in SMESH_TryCatch.hxx,
// it re-raises a CORBA SALOME exception thrown by SMESH_MeshEditor_i and caught by SMESH_CATCH
#define SMY_OWN_CATCH \
  catch ( SALOME::SALOME_Exception & e ) { throw e; }

#include "SMESH_MeshEditor_i.hxx"

#include "SMDS_EdgePosition.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_IteratorOnIterators.hxx"
#include "SMDS_LinearEdge.hxx"
#include "SMDS_Mesh0DElement.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMDS_PolyhedralVolumeOfNodes.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_MeshPartDS.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "SMESH_subMesh_i.hxx"

#include <utilities.h>
#include <Utils_ExceptHandlers.hxx>
#include <Utils_CorbaException.hxx>
#include <SALOMEDS_wrap.hxx>
#include <SALOME_GenericObj_i.hh>
#include <Basics_OCCTVersion.hxx>

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

#include "SMESH_TryCatch.hxx" // include after OCCT headers!

#define cast2Node(elem) static_cast<const SMDS_MeshNode*>( elem )

using namespace std;
using SMESH::TPythonDump;
using SMESH::TVar;

namespace MeshEditor_I {

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
      _isShapeToMesh = (_id =_studyId = 0);
      _myMeshDS  = new SMESHDS_Mesh( _id, true );
      myPreviewType = previewElements;
    }
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
      ::SMESH_MeshEditor::ElemFeatures elemType;
      elemType.Init( anElem, /*basicOnly=*/false );
      elemType.SetID( anElem->GetID() );
      SMDS_MeshElement* anElemCopy =
        ::SMESH_MeshEditor(this).AddElement( anElemNodesID, elemType );
      return anElemCopy;
    }
    //!< Copy a node
    SMDS_MeshNode* Copy( const SMDS_MeshNode* anElemNode )
    {
      return _myMeshDS->AddNodeWithID(anElemNode->X(), anElemNode->Y(), anElemNode->Z(),
                                      anElemNode->GetID());
    }
    void RemoveAll()
    {
      GetMeshDS()->ClearMesh();
    }
    void Remove( SMDSAbs_ElementType type )
    {
      SMDS_ElemIteratorPtr eIt = GetMeshDS()->elementsIterator( type );
      while ( eIt->more() )
        GetMeshDS()->RemoveFreeElement( eIt->next(), /*sm=*/0, /*fromGroups=*/false );
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
        SMESH_subMesh* sm = mesh->GetSubMesh( mesh->GetShapeToMesh() );
        SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator( /*includeSelf=*/true );
        while ( smIt->more() )
        {
          sm = smIt->next();
          sm->SetEventListener( this, 0, sm );
        }
      }
    }
    //!<  delete self from all submeshes
    void Unset(SMESH_Mesh* mesh)
    {
      if ( SMESH_subMesh* sm = mesh->GetSubMeshContaining(1) ) {
        SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator( /*includeSelf=*/true );
        while ( smIt->more() )
          smIt->next()->DeleteEventListener( this );
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
                  const SMDSAbs_ElementType aType = SMDSAbs_All,
                  SMDS_MeshElement::Filter* aFilter = NULL)
  {
    SMDS_MeshElement::NonNullFilter filter1;
    SMDS_MeshElement::TypeFilter    filter2( aType );

    if ( aFilter == NULL )
      aFilter = ( aType == SMDSAbs_All ) ? (SMDS_MeshElement::Filter*) &filter1 : (SMDS_MeshElement::Filter*) &filter2;
    
    SMDS_MeshElement::Filter & filter = *aFilter;

    if ( aType == SMDSAbs_Node )
      for ( CORBA::ULong i = 0; i < IDs.length(); i++ ) {
        const SMDS_MeshElement * elem = aMesh->FindNode( IDs[i] );
        if ( filter( elem ))
          aMap.insert( aMap.end(), elem );
      }
    else
      for ( CORBA::ULong i = 0; i<IDs.length(); i++) {
        const SMDS_MeshElement * elem = aMesh->FindElement( IDs[i] );
        if ( filter( elem ))
          aMap.insert( aMap.end(), elem );
      }
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
      for ( CORBA::ULong i = 0; i < aElementsId->length(); i++ )
        if ( const SMDS_MeshNode * n = theMeshDS->FindNode( aElementsId[i] ))
          theNodeSet.insert( theNodeSet.end(), n);
    }
    else if ( SMESH::DownCast<SMESH_Mesh_i*>( theObject ))
    {
      SMDS_NodeIteratorPtr nIt = theMeshDS->nodesIterator();
      while ( nIt->more( ))
        if ( const SMDS_MeshElement * elem = nIt->next() )
          theNodeSet.insert( elem->begin_nodes(), elem->end_nodes());
    }
    else
    {
      for ( CORBA::ULong i = 0; i < aElementsId->length(); i++ )
        if ( const SMDS_MeshElement * elem = theMeshDS->FindElement( aElementsId[i] ))
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
         theMeshDS->GetMeshInfo().NbElements( elemType ) == (int) theElements.size() )
      return; // all the elements are in theElements

    if ( !sameElemType )
      elemType = SMDSAbs_All;

    vector<bool> isNodeChecked( theMeshDS->NbNodes(), false );

    TIDSortedElemSet::const_iterator elemIt = theElements.begin();
    for ( ; elemIt != theElements.end(); ++elemIt )
    {
      const SMDS_MeshElement* e = *elemIt;
      int i = e->NbCornerNodes();
      while ( --i != -1 )
      {
        const SMDS_MeshNode* n = e->GetNode( i );
        if ( !isNodeChecked[ n->GetID() ])
        {
          isNodeChecked[ n->GetID() ] = true;
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

  //================================================================================
  /*!
   * \brief Return a string used to detect change of mesh part on which theElementSearcher
   * is going to be used
   */
  //================================================================================

  string getPartIOR( SMESH::SMESH_IDSource_ptr theMeshPart, SMESH::ElementType type)
  {
    string partIOR = SMESH_Gen_i::GetORB()->object_to_string( theMeshPart );
    if ( SMESH_Group_i* group_i = SMESH::DownCast<SMESH_Group_i*>( theMeshPart ))
      // take into account passible group modification
      partIOR += SMESH_Comment( ((SMESHDS_Group*)group_i->GetGroupDS())->SMDSGroup().Tic() );
    partIOR += SMESH_Comment( type );
    return partIOR;
  }

} // namespace MeshEditor_I

using namespace MeshEditor_I;

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_MeshEditor_i::SMESH_MeshEditor_i(SMESH_Mesh_i* theMesh, bool isPreview):
  myMesh_i( theMesh ),
  myMesh( &theMesh->GetImpl() ),
  myEditor( myMesh ),
  myIsPreviewMode ( isPreview ),
  myPreviewMesh( 0 ),
  myPreviewEditor( 0 )
{
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESH_MeshEditor_i::~SMESH_MeshEditor_i()
{
  PortableServer::POA_var poa = SMESH_Gen_i::GetPOA();
  PortableServer::ObjectId_var anObjectId = poa->servant_to_id(this);
  poa->deactivate_object(anObjectId.in());

  //deleteAuxIDSources();
  delete myPreviewMesh;   myPreviewMesh = 0;
  delete myPreviewEditor; myPreviewEditor = 0;
}

//================================================================================
/*!
 * \brief Returns the mesh
 */
//================================================================================

SMESH::SMESH_Mesh_ptr SMESH_MeshEditor_i::GetMesh()
{
  return myMesh_i->_this();
}

//================================================================================
/*!
 * \brief Clear members
 */
//================================================================================

void SMESH_MeshEditor_i::initData(bool deleteSearchers)
{
  if ( myIsPreviewMode ) {
    if ( myPreviewMesh ) myPreviewMesh->RemoveAll();
  }
  else {
    if ( deleteSearchers )
      TSearchersDeleter::Delete();
  }
  getEditor().GetError().reset();
  getEditor().ClearLastCreated();
}

//================================================================================
/*!
 * \brief Increment mesh modif time and optionally record that the performed
 *        modification may influence futher mesh re-compute.
 *  \param [in] isReComputeSafe - true if the modification does not influence
 *              futher mesh re-compute
 */
//================================================================================

void SMESH_MeshEditor_i::declareMeshModified( bool isReComputeSafe )
{
  myMesh->GetMeshDS()->Modified();
  if ( !isReComputeSafe )
    myMesh->SetIsModified( true );
}

//================================================================================
/*!
 * \brief Return either myEditor or myPreviewEditor depending on myIsPreviewMode.
 *        WARNING: in preview mode call getPreviewMesh() before getEditor()!
 */
//================================================================================

::SMESH_MeshEditor& SMESH_MeshEditor_i::getEditor()
{
  if ( myIsPreviewMode && !myPreviewEditor ) {
    if ( !myPreviewMesh ) getPreviewMesh();
    myPreviewEditor = new ::SMESH_MeshEditor( myPreviewMesh );
  }
  return myIsPreviewMode ? *myPreviewEditor : myEditor;
}

//================================================================================
/*!
 * \brief Initialize and return myPreviewMesh
 *  \param previewElements - type of elements to show in preview
 *
 *  WARNING: call it once par a method!
 */
//================================================================================

TPreviewMesh * SMESH_MeshEditor_i::getPreviewMesh(SMDSAbs_ElementType previewElements)
{
  if ( !myPreviewMesh || myPreviewMesh->myPreviewType != previewElements )
  {
    delete myPreviewEditor;
    myPreviewEditor = 0;
    delete myPreviewMesh;
    myPreviewMesh = new TPreviewMesh( previewElements );
  }
  myPreviewMesh->Clear();
  return myPreviewMesh;
}

//================================================================================
/*!
 * Return data of mesh edition preview
 */
//================================================================================

SMESH::MeshPreviewStruct* SMESH_MeshEditor_i::GetPreviewData()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  const bool hasBadElems = ( getEditor().GetError() && getEditor().GetError()->HasBadElems() );

  if ( myIsPreviewMode || hasBadElems ) { // --- MeshPreviewStruct filling ---

    list<int> aNodesConnectivity;
    typedef map<int, int> TNodesMap;
    TNodesMap nodesMap;

    SMESHDS_Mesh* aMeshDS;
    std::auto_ptr< SMESH_MeshPartDS > aMeshPartDS;
    if ( hasBadElems ) {
      aMeshPartDS.reset( new SMESH_MeshPartDS( getEditor().GetError()->myBadElements ));
      aMeshDS = aMeshPartDS.get();
    }
    else {
      aMeshDS = getEditor().GetMeshDS();
    }
    myPreviewData = new SMESH::MeshPreviewStruct();
    myPreviewData->nodesXYZ.length(aMeshDS->NbNodes());


    SMDSAbs_ElementType previewType = SMDSAbs_All;
    if ( !hasBadElems )
      if (TPreviewMesh * aPreviewMesh = dynamic_cast< TPreviewMesh* >( getEditor().GetMesh() )) {
        previewType = aPreviewMesh->myPreviewType;
        switch ( previewType ) {
        case SMDSAbs_Edge  : break;
        case SMDSAbs_Face  : break;
        case SMDSAbs_Volume: break;
        default:;
          if ( aMeshDS->GetMeshInfo().NbElements() == 0 ) previewType = SMDSAbs_Node;
        }
      }

    myPreviewData->elementTypes.length( aMeshDS->GetMeshInfo().NbElements( previewType ));
    int i = 0, j = 0;
    SMDS_ElemIteratorPtr itMeshElems = aMeshDS->elementsIterator(previewType);

    while ( itMeshElems->more() ) {
      const SMDS_MeshElement* aMeshElem = itMeshElems->next();
      SMDS_NodeIteratorPtr itElemNodes = 
        (( aMeshElem->GetEntityType() == SMDSEntity_Quad_Polygon ) ?
         aMeshElem->interlacedNodesIterator() :
         aMeshElem->nodeIterator() );
      while ( itElemNodes->more() ) {
        const SMDS_MeshNode* aMeshNode = itElemNodes->next();
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
      SMDSAbs_ElementType aType = aMeshElem->GetType();
      bool               isPoly = aMeshElem->IsPoly();
      myPreviewData->elementTypes[i].SMDS_ElementType = (SMESH::ElementType) aType;
      myPreviewData->elementTypes[i].isPoly           = isPoly;
      myPreviewData->elementTypes[i].nbNodesInElement = aMeshElem->NbNodes();
      i++;
    }
    myPreviewData->nodesXYZ.length( j );

    // filling the elements connectivities
    list<int>::iterator aConnIter = aNodesConnectivity.begin();
    myPreviewData->elementConnectivities.length(aNodesConnectivity.size());
    for( int i = 0; aConnIter != aNodesConnectivity.end(); aConnIter++, i++ )
      myPreviewData->elementConnectivities[i] = *aConnIter;
  }
  return myPreviewData._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
 * \brief Returns list of it's IDs of created nodes
 * \retval SMESH::long_array* - list of node ID
 */
//================================================================================

SMESH::long_array* SMESH_MeshEditor_i::GetLastCreatedNodes()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::long_array_var myLastCreatedNodes = new SMESH::long_array();

  const SMESH_SequenceOfElemPtr& aSeq = getEditor().GetLastCreatedNodes();
  myLastCreatedNodes->length( aSeq.Length() );
  for (int i = 1; i <= aSeq.Length(); i++)
    myLastCreatedNodes[i-1] = aSeq.Value(i)->GetID();

  return myLastCreatedNodes._retn();
  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
 * \brief Returns list of it's IDs of created elements
 * \retval SMESH::long_array* - list of elements' ID
 */
//================================================================================

SMESH::long_array* SMESH_MeshEditor_i::GetLastCreatedElems()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::long_array_var myLastCreatedElems = new SMESH::long_array();

  const SMESH_SequenceOfElemPtr& aSeq = getEditor().GetLastCreatedElems();
  myLastCreatedElems->length( aSeq.Length() );
  for ( int i = 1; i <= aSeq.Length(); i++ )
    myLastCreatedElems[i-1] = aSeq.Value(i)->GetID();

  return myLastCreatedElems._retn();
  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : ClearLastCreated
//purpose  : Clears sequences of last created elements and nodes 
//=======================================================================

void SMESH_MeshEditor_i::ClearLastCreated() throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  getEditor().ClearLastCreated();
  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
/*
 * Returns description of an error/warning occured during the last operation
 * WARNING: ComputeError.code >= 100 and no corresponding enum in IDL API
 */
//=======================================================================

SMESH::ComputeError* SMESH_MeshEditor_i::GetLastError()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::ComputeError_var errOut = new SMESH::ComputeError;
  SMESH_ComputeErrorPtr&  errIn  = getEditor().GetError();
  if ( errIn && !errIn->IsOK() )
  {
    errOut->code       = -( errIn->myName < 0 ? errIn->myName + 1: errIn->myName ); // -1 -> 0
    errOut->comment    = errIn->myComment.c_str();
    errOut->subShapeID = -1;
    errOut->hasBadMesh = !errIn->myBadElements.empty();
  }
  else
  {
    errOut->code       = 0;
    errOut->subShapeID = -1;
    errOut->hasBadMesh = false;
  }

  return errOut._retn();
  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : MakeIDSource
//purpose  : Wrap a sequence of ids in a SMESH_IDSource.
//           Call UnRegister() as you fininsh using it!!
//=======================================================================

struct SMESH_MeshEditor_i::_IDSource : public virtual POA_SMESH::SMESH_IDSource,
                                       public virtual SALOME::GenericObj_i
{
  SMESH::long_array     _ids;
  SMESH::ElementType    _type;
  SMESH::SMESH_Mesh_ptr _mesh;
  SMESH::long_array* GetIDs()      { return new SMESH::long_array( _ids ); }
  SMESH::long_array* GetMeshInfo() { return 0; }
  SMESH::long_array* GetNbElementsByType()
  {
    SMESH::long_array_var aRes = new SMESH::long_array();
    aRes->length(SMESH::NB_ELEMENT_TYPES);
    for (int i = 0; i < SMESH::NB_ELEMENT_TYPES; i++)
      aRes[ i ] = ( i == _type ) ? _ids.length() : 0;
    return aRes._retn();  
  }
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
  SALOMEDS::TMPFile* GetVtkUgStream()
  {
    SALOMEDS::TMPFile_var SeqFile;
    return SeqFile._retn();
  }
};

SMESH::SMESH_IDSource_ptr SMESH_MeshEditor_i::MakeIDSource(const SMESH::long_array& ids,
                                                           SMESH::ElementType       type)
{
  _IDSource* idSrc = new _IDSource;
  idSrc->_mesh = myMesh_i->_this();
  idSrc->_ids  = ids;
  idSrc->_type = type;
  if ( type == SMESH::ALL && ids.length() > 0 )
    idSrc->_type = myMesh_i->GetElementType( ids[0], true );

  SMESH::SMESH_IDSource_var anIDSourceVar = idSrc->_this();

  return anIDSourceVar._retn();
}

bool SMESH_MeshEditor_i::IsTemporaryIDSource( SMESH::SMESH_IDSource_ptr& idSource )
{
  return SMESH::DownCast<SMESH_MeshEditor_i::_IDSource*>( idSource );
}

CORBA::Long* SMESH_MeshEditor_i::GetTemporaryIDs( SMESH::SMESH_IDSource_ptr& idSource,
                                                  int&                       nbIds)
{
  if ( _IDSource* tmpIdSource = SMESH::DownCast<SMESH_MeshEditor_i::_IDSource*>( idSource ))
  {
    nbIds = (int) tmpIdSource->_ids.length();
    return & tmpIdSource->_ids[0];
  }
  nbIds = 0;
  return 0;
}

// void SMESH_MeshEditor_i::deleteAuxIDSources()
// {
//   std::list< _IDSource* >::iterator idSrcIt = myAuxIDSources.begin();
//   for ( ; idSrcIt != myAuxIDSources.end(); ++idSrcIt )
//     delete *idSrcIt;
//   myAuxIDSources.clear();
// }

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean
SMESH_MeshEditor_i::RemoveElements(const SMESH::long_array & IDsOfElements)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  list< int > IdList;

  for ( CORBA::ULong i = 0; i < IDsOfElements.length(); i++ )
    IdList.push_back( IDsOfElements[i] );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".RemoveElements( " << IDsOfElements << " )";

  // Remove Elements
  bool ret = getEditor().Remove( IdList, false );

  declareMeshModified( /*isReComputeSafe=*/ IDsOfElements.length() == 0 ); // issue 0020693
  return ret;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveNodes(const SMESH::long_array & IDsOfNodes)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  list< int > IdList;
  for ( CORBA::ULong i = 0; i < IDsOfNodes.length(); i++)
    IdList.push_back( IDsOfNodes[i] );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".RemoveNodes( " << IDsOfNodes << " )";

  bool ret = getEditor().Remove( IdList, true );

  declareMeshModified( /*isReComputeSafe=*/ !ret ); // issue 0020693
  return ret;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::RemoveOrphanNodes()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  // Update Python script
  TPythonDump() << "nbRemoved = " << this << ".RemoveOrphanNodes()";

  // Create filter to find all orphan nodes
  SMESH::Controls::Filter::TIdSequence seq;
  SMESH::Controls::PredicatePtr predicate( new SMESH::Controls::FreeNodes() );
  SMESH::Controls::Filter::GetElementsId( getMeshDS(), predicate, seq );

  // remove orphan nodes (if there are any)
  list< int > IdList( seq.begin(), seq.end() );

  int nbNodesBefore = myMesh->NbNodes();
  getEditor().Remove( IdList, true );
  int nbNodesAfter = myMesh->NbNodes();

  declareMeshModified( /*isReComputeSafe=*/ IdList.size() == 0 ); // issue 0020693
  return nbNodesBefore - nbNodesAfter;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * Add a new node.
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddNode(CORBA::Double x,CORBA::Double y, CORBA::Double z)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  const SMDS_MeshNode* N = getMeshDS()->AddNode(x, y, z);

  // Update Python script
  TPythonDump() << "nodeID = " << this << ".AddNode( "
                << TVar( x ) << ", " << TVar( y ) << ", " << TVar( z )<< " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return N->GetID();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * Create 0D element on the given node.
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::Add0DElement(CORBA::Long    IDOfNode,
                                             CORBA::Boolean DuplicateElements)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  const SMDS_MeshNode* aNode = getMeshDS()->FindNode(IDOfNode);
  SMDS_ElemIteratorPtr it0D = aNode->GetInverseElementIterator( SMDSAbs_0DElement );
  
  SMDS_MeshElement* elem = 0;
  if ( DuplicateElements || !it0D->more() )
    elem = getMeshDS()->Add0DElement(aNode);

  // Update Python script
  TPythonDump() << "elem0d = " << this << ".Add0DElement( " << IDOfNode <<" )";

  declareMeshModified( /*isReComputeSafe=*/false );

  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * Create a ball element on the given node.
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddBall(CORBA::Long IDOfNode, CORBA::Double diameter)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  if ( diameter < std::numeric_limits<double>::min() )
    THROW_SALOME_CORBA_EXCEPTION("Invalid diameter", SALOME::BAD_PARAM);

  const SMDS_MeshNode* aNode = getMeshDS()->FindNode(IDOfNode);
  SMDS_MeshElement* elem = getMeshDS()->AddBall(aNode, diameter);

  // Update Python script
  TPythonDump() << "ballElem = "
                << this << ".AddBall( " << IDOfNode << ", " << diameter <<" )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * Create an edge, either linear and quadratic (this is determed
 *  by number of given nodes, two or three)
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddEdge(const SMESH::long_array & IDsOfNodes)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  int NbNodes = IDsOfNodes.length();
  SMDS_MeshElement* elem = 0;
  if (NbNodes == 2)
  {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    elem = getMeshDS()->AddEdge( getMeshDS()->FindNode(index1),
                                 getMeshDS()->FindNode(index2));

    // Update Python script
    TPythonDump() << "edge = " << this << ".AddEdge([ "
                  << index1 << ", " << index2 <<" ])";
  }
  if (NbNodes == 3) {
    CORBA::Long n1 = IDsOfNodes[0];
    CORBA::Long n2 = IDsOfNodes[1];
    CORBA::Long n12 = IDsOfNodes[2];
    elem = getMeshDS()->AddEdge( getMeshDS()->FindNode(n1),
                                 getMeshDS()->FindNode(n2),
                                 getMeshDS()->FindNode(n12));
    // Update Python script
    TPythonDump() << "edgeID = " << this << ".AddEdge([ "
                  <<n1<<", "<<n2<<", "<<n12<<" ])";
  }

  declareMeshModified( /*isReComputeSafe=*/false );
  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *  AddFace
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddFace(const SMESH::long_array & IDsOfNodes)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  int NbNodes = IDsOfNodes.length();
  if (NbNodes < 3)
  {
    return 0;
  }

  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = getMeshDS()->FindNode(IDsOfNodes[i]);

  SMDS_MeshElement* elem = 0;
  switch (NbNodes) {
  case 3: elem = getMeshDS()->AddFace(nodes[0], nodes[1], nodes[2]); break;
  case 4: elem = getMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3]); break;
  case 6: elem = getMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                      nodes[4], nodes[5]); break;
  case 7: elem = getMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                      nodes[4], nodes[5], nodes[6]); break;
  case 8: elem = getMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                      nodes[4], nodes[5], nodes[6], nodes[7]); break;
  case 9: elem = getMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                      nodes[4], nodes[5], nodes[6], nodes[7],
                                      nodes[8] ); break;
  default: elem = getMeshDS()->AddPolygonalFace(nodes);
  }

  // Update Python script
  TPythonDump() << "faceID = " << this << ".AddFace( " << IDsOfNodes << " )";

  declareMeshModified( /*isReComputeSafe=*/false );

  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *  AddPolygonalFace
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddPolygonalFace (const SMESH::long_array & IDsOfNodes)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    if ( ! ( nodes[i] = getMeshDS()->FindNode( IDsOfNodes[i] )))
      return 0;

  const SMDS_MeshElement* elem = getMeshDS()->AddPolygonalFace(nodes);

  // Update Python script
  TPythonDump() <<"faceID = "<<this<<".AddPolygonalFace( "<<IDsOfNodes<<" )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *  AddQuadPolygonalFace
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddQuadPolygonalFace (const SMESH::long_array & IDsOfNodes)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = getMeshDS()->FindNode(IDsOfNodes[i]);

  const SMDS_MeshElement* elem = getMeshDS()->AddQuadPolygonalFace(nodes);

  // Update Python script
  TPythonDump() <<"faceID = "<<this<<".AddPolygonalFace( "<<IDsOfNodes<<" )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * Create volume, either linear and quadratic (this is determed
 *  by number of given nodes)
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddVolume(const SMESH::long_array & IDsOfNodes)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  int NbNodes = IDsOfNodes.length();
  vector< const SMDS_MeshNode*> n(NbNodes);
  for(int i=0;i<NbNodes;i++)
    n[i]= getMeshDS()->FindNode(IDsOfNodes[i]);

  SMDS_MeshElement* elem = 0;
  switch(NbNodes)
  {
  case 4 :elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3]); break;
  case 5 :elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4]); break;
  case 6 :elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5]); break;
  case 8 :elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]); break;
  case 10:elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],
                                        n[6],n[7],n[8],n[9]);
    break;
  case 12:elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],
                                        n[6],n[7],n[8],n[9],n[10],n[11]);
    break;
  case 13:elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],
                                        n[7],n[8],n[9],n[10],n[11],n[12]);
    break;
  case 15:elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],
                                        n[9],n[10],n[11],n[12],n[13],n[14]);
    break;
  case 20:elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],
                                        n[8],n[9],n[10],n[11],n[12],n[13],n[14],
                                        n[15],n[16],n[17],n[18],n[19]);
    break;
  case 27:elem = getMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],
                                        n[8],n[9],n[10],n[11],n[12],n[13],n[14],
                                        n[15],n[16],n[17],n[18],n[19],
                                        n[20],n[21],n[22],n[23],n[24],n[25],n[26]);
    break;
  }

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddVolume( " << IDsOfNodes << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolume
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolume (const SMESH::long_array & IDsOfNodes,
                                                     const SMESH::long_array & Quantities)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> n (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    {
      const SMDS_MeshNode* aNode = getMeshDS()->FindNode(IDsOfNodes[i]);
      if (!aNode) return 0;
      n[i] = aNode;
    }

  int NbFaces = Quantities.length();
  std::vector<int> q (NbFaces);
  for (int j = 0; j < NbFaces; j++)
    q[j] = Quantities[j];

  const SMDS_MeshElement* elem = getMeshDS()->AddPolyhedralVolume(n, q);

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddPolyhedralVolume( "
                << IDsOfNodes << ", " << Quantities << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolumeByFaces
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolumeByFaces (const SMESH::long_array & IdsOfFaces)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  int NbFaces = IdsOfFaces.length();
  std::vector<const SMDS_MeshNode*> poly_nodes;
  std::vector<int> quantities (NbFaces);

  for (int i = 0; i < NbFaces; i++) {
    const SMDS_MeshElement* aFace = getMeshDS()->FindElement(IdsOfFaces[i]);
    quantities[i] = aFace->NbNodes();

    SMDS_ElemIteratorPtr It = aFace->nodesIterator();
    while (It->more()) {
      poly_nodes.push_back(static_cast<const SMDS_MeshNode *>(It->next()));
    }
  }

  const SMDS_MeshElement* elem = getMeshDS()->AddPolyhedralVolume(poly_nodes, quantities);

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddPolyhedralVolumeByFaces( "
                << IdsOfFaces << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return elem ? elem->GetID() : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
//
// \brief Create 0D elements on all nodes of the given object.
//  \param theObject object on whose nodes 0D elements will be created.
//  \param theGroupName optional name of a group to add 0D elements created
//         and/or found on nodes of \a theObject.
//  \param DuplicateElements to add one more 0D element to a node or not.
//  \return an object (a new group or a temporary SMESH_IDSource) holding
//          ids of new and/or found 0D elements.
//
//=============================================================================

SMESH::SMESH_IDSource_ptr
SMESH_MeshEditor_i::Create0DElementsOnAllNodes(SMESH::SMESH_IDSource_ptr theObject,
                                               const char*               theGroupName,
                                               CORBA::Boolean            theDuplicateElements)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESH::SMESH_IDSource_var result;
  TPythonDump pyDump;

  TIDSortedElemSet elements, elems0D;
  if ( idSourceToSet( theObject, getMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    getEditor().Create0DElementsOnAllNodes( elements, elems0D, theDuplicateElements );

  SMESH::long_array_var newElems = new SMESH::long_array;
  newElems->length( elems0D.size() );
  TIDSortedElemSet::iterator eIt = elems0D.begin();
  for ( size_t i = 0; i < elems0D.size(); ++i, ++eIt )
    newElems[ i ] = (*eIt)->GetID();

  SMESH::SMESH_GroupBase_var groupToFill;
  if ( theGroupName && strlen( theGroupName ))
  {
    // Get existing group named theGroupName
    SMESH::ListOfGroups_var groups = myMesh_i->GetGroups();
    for (int i = 0, nbGroups = groups->length(); i < nbGroups; i++ ) {
      SMESH::SMESH_GroupBase_var group = groups[i];
      if ( !group->_is_nil() ) {
        CORBA::String_var name = group->GetName();
        if ( strcmp( name.in(), theGroupName ) == 0 && group->GetType() == SMESH::ELEM0D ) {
          groupToFill = group;
          break;
        }
      }
    }
    if ( groupToFill->_is_nil() )
      groupToFill = myMesh_i->CreateGroup( SMESH::ELEM0D, theGroupName );
    else if ( !SMESH::DownCast< SMESH_Group_i* > ( groupToFill ))
      groupToFill = myMesh_i->ConvertToStandalone( groupToFill );
  }

  if ( SMESH_Group_i* group_i = SMESH::DownCast< SMESH_Group_i* > ( groupToFill ))
  {
    group_i->Add( newElems );
    result = SMESH::SMESH_IDSource::_narrow( groupToFill );
    pyDump << groupToFill;
  }
  else
  {
    result = MakeIDSource( newElems, SMESH::ELEM0D );
    pyDump << "elem0DIDs";
  }

  pyDump << " = " << this << ".Create0DElementsOnAllNodes( "
         << theObject << ", '" << theGroupName << "' )";

  return result._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
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
  SMESH_TRY;

  SMESHDS_Mesh * mesh = getMeshDS();
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

  SMESH_CATCH( SMESH::throwCorbaException );
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
  SMESH_TRY;

  SMESHDS_Mesh * mesh = getMeshDS();
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

  SMESH_CATCH( SMESH::throwCorbaException );
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
  SMESH_TRY;
  SMESHDS_Mesh * mesh = getMeshDS();
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

  SMESH_CATCH( SMESH::throwCorbaException );
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
  SMESH_TRY;
  SMESHDS_Mesh * mesh = getMeshDS();
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

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=============================================================================
/*!
 * \brief Bind an element to a shape
 * \param ElementID - element ID
 * \param ShapeID - shape ID available through GEOM_Object.GetSubShapeIndices()[0]
 */
//=============================================================================

void SMESH_MeshEditor_i::SetMeshElementOnShape(CORBA::Long ElementID,
                                               CORBA::Long ShapeID)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESHDS_Mesh * mesh = getMeshDS();
  SMDS_MeshElement* elem = const_cast<SMDS_MeshElement*>(mesh->FindElement(ElementID));
  if ( !elem )
    THROW_SALOME_CORBA_EXCEPTION("Invalid ElementID", SALOME::BAD_PARAM);

  if ( mesh->MaxShapeIndex() < ShapeID || ShapeID < 1 )
    THROW_SALOME_CORBA_EXCEPTION("Invalid ShapeID", SALOME::BAD_PARAM);

  TopoDS_Shape shape = mesh->IndexToShape( ShapeID );
  if ( shape.ShapeType() != TopAbs_EDGE &&
       shape.ShapeType() != TopAbs_FACE &&
       shape.ShapeType() != TopAbs_SOLID &&
       shape.ShapeType() != TopAbs_SHELL )
    THROW_SALOME_CORBA_EXCEPTION("Invalid shape type", SALOME::BAD_PARAM);

  mesh->SetMeshElementOnShape( elem, ShapeID );

  myMesh->SetIsModified( true );

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::InverseDiag(CORBA::Long NodeID1,
                                               CORBA::Long NodeID2)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  const SMDS_MeshNode * n1 = getMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = getMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TPythonDump() << "isDone = " << this << ".InverseDiag( "
                << NodeID1 << ", " << NodeID2 << " )";

  int ret =  getEditor().InverseDiag ( n1, n2 );

  declareMeshModified( /*isReComputeSafe=*/false );
  return ret;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::DeleteDiag(CORBA::Long NodeID1,
                                              CORBA::Long NodeID2)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  const SMDS_MeshNode * n1 = getMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = getMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DeleteDiag( "
                << NodeID1 << ", " << NodeID2 <<  " )";


  bool stat = getEditor().DeleteDiag ( n1, n2 );

  declareMeshModified( /*isReComputeSafe=*/!stat );

  return stat;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::Reorient(const SMESH::long_array & IDsOfElements)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  for ( CORBA::ULong i = 0; i < IDsOfElements.length(); i++ )
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = getMeshDS()->FindElement(index);
    if ( elem )
      getEditor().Reorient( elem );
  }
  // Update Python script
  TPythonDump() << "isDone = " << this << ".Reorient( " << IDsOfElements << " )";

  declareMeshModified( /*isReComputeSafe=*/ IDsOfElements.length() == 0 );
  return true;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::ReorientObject(SMESH::SMESH_IDSource_ptr theObject)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aTPythonDump; // suppress dump in Reorient()

  prepareIdSource( theObject );

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = Reorient(anElementsId);

  // Update Python script
  aTPythonDump << "isDone = " << this << ".ReorientObject( " << theObject << " )";

  declareMeshModified( /*isReComputeSafe=*/ anElementsId->length() == 0 );
  return isDone;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : Reorient2D
//purpose  : Reorient faces contained in \a the2Dgroup.
//           the2Dgroup   - the mesh or its part to reorient
//           theDirection - desired direction of normal of \a theFace
//           theFace      - ID of face whose orientation is checked.
//           It can be < 1 then \a thePoint is used to find a face.
//           thePoint     - is used to find a face if \a theFace < 1.
//           return number of reoriented elements.
//=======================================================================

CORBA::Long SMESH_MeshEditor_i::Reorient2D(SMESH::SMESH_IDSource_ptr the2Dgroup,
                                           const SMESH::DirStruct&   theDirection,
                                           CORBA::Long               theFace,
                                           const SMESH::PointStruct& thePoint)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData(/*deleteSearchers=*/false);

  TIDSortedElemSet elements;
  IDSource_Error error;
  idSourceToSet( the2Dgroup, getMeshDS(), elements, SMDSAbs_Face, /*emptyIfIsMesh=*/1, &error );
  if ( error == IDSource_EMPTY )
    return 0;
  if ( error == IDSource_INVALID )
    THROW_SALOME_CORBA_EXCEPTION("No faces in given group", SALOME::BAD_PARAM);


  const SMDS_MeshElement* face = 0;
  if ( theFace > 0 )
  {
    face = getMeshDS()->FindElement( theFace );
    if ( !face )
      THROW_SALOME_CORBA_EXCEPTION("Inexistent face given", SALOME::BAD_PARAM);
    if ( face->GetType() != SMDSAbs_Face )
      THROW_SALOME_CORBA_EXCEPTION("Wrong element type", SALOME::BAD_PARAM);
  }
  else
  {
    // create theElementSearcher if needed
    theSearchersDeleter.Set( myMesh, getPartIOR( the2Dgroup, SMESH::FACE ));
    if ( !theElementSearcher )
    {
      if ( elements.empty() ) // search in the whole mesh
      {
        if ( myMesh->NbFaces() == 0 )
          THROW_SALOME_CORBA_EXCEPTION("No faces in the mesh", SALOME::BAD_PARAM);

        theElementSearcher = SMESH_MeshAlgos::GetElementSearcher( *getMeshDS() );
      }
      else
      {
        typedef SMDS_SetIterator<const SMDS_MeshElement*, TIDSortedElemSet::const_iterator > TIter;
        SMDS_ElemIteratorPtr elemsIt( new TIter( elements.begin(), elements.end() ));

        theElementSearcher = SMESH_MeshAlgos::GetElementSearcher( *getMeshDS(), elemsIt);
      }
    }
    // find a face
    gp_Pnt p( thePoint.x, thePoint.y, thePoint.z );
    face = theElementSearcher->FindClosestTo( p, SMDSAbs_Face );

    if ( !face )
      THROW_SALOME_CORBA_EXCEPTION("No face found by point", SALOME::INTERNAL_ERROR );
    if ( !elements.empty() && !elements.count( face ))
      THROW_SALOME_CORBA_EXCEPTION("Found face is not in the group", SALOME::BAD_PARAM );
  }

  const SMESH::PointStruct * P = &theDirection.PS;
  gp_Vec dirVec( P->x, P->y, P->z );
  if ( dirVec.Magnitude() < std::numeric_limits< double >::min() )
    THROW_SALOME_CORBA_EXCEPTION("Zero size vector", SALOME::BAD_PARAM);

  int nbReori = getEditor().Reorient2D( elements, dirVec, face );

  if ( nbReori ) {
    declareMeshModified( /*isReComputeSafe=*/false );
  }
  TPythonDump() << this << ".Reorient2D( "
                << the2Dgroup << ", "
                << theDirection << ", "
                << theFace << ", "
                << thePoint << " )";

  return nbReori;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : Reorient2DBy3D
//purpose  : Reorient faces basing on orientation of adjacent volumes.
//=======================================================================

CORBA::Long SMESH_MeshEditor_i::Reorient2DBy3D(const SMESH::ListOfIDSources& faceGroups,
                                               SMESH::SMESH_IDSource_ptr     volumeGroup,
                                               CORBA::Boolean                outsideNormal)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedElemSet volumes;
  IDSource_Error volsError;
  idSourceToSet( volumeGroup, getMeshDS(), volumes, SMDSAbs_Volume, /*emptyIfMesh=*/1, &volsError);

  int nbReori = 0;
  for ( size_t i = 0; i < faceGroups.length(); ++i )
  {
    SMESH::SMESH_IDSource_ptr faceGrp = faceGroups[i].in();

    TIDSortedElemSet faces;
    IDSource_Error error;
    idSourceToSet( faceGrp, getMeshDS(), faces, SMDSAbs_Face, /*emptyIfIsMesh=*/1, &error );
    if ( error == IDSource_INVALID && faceGroups.length() == 1 )
      THROW_SALOME_CORBA_EXCEPTION("No faces in a given object", SALOME::BAD_PARAM);
    if ( error == IDSource_OK && volsError != IDSource_OK )
      THROW_SALOME_CORBA_EXCEPTION("No volumes in a given object", SALOME::BAD_PARAM);

    nbReori += getEditor().Reorient2DBy3D( faces, volumes, outsideNormal );

    if ( error != IDSource_EMPTY && faces.empty() ) // all faces in the mesh treated
      break;
  }

  if ( nbReori ) {
    declareMeshModified( /*isReComputeSafe=*/false );
  }
  TPythonDump() << this << ".Reorient2DBy3D( "
                << faceGroups << ", "
                << volumeGroup << ", "
                << outsideNormal << " )";

  return nbReori;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * \brief Fuse neighbour triangles into quadrangles.
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::TriToQuad (const SMESH::long_array &   IDsOfElements,
                                              SMESH::NumericalFunctor_ptr Criterion,
                                              CORBA::Double               MaxAngle)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();
  TIDSortedElemSet faces,copyFaces;
  SMDS_MeshElement::GeomFilter triaFilter(SMDSGeom_TRIANGLE);
  arrayToSet(IDsOfElements, aMesh, faces, SMDSAbs_Face, & triaFilter);
  TIDSortedElemSet* workElements = & faces;

  if ( myIsPreviewMode ) {
    SMDSAbs_ElementType select =  SMDSAbs_Face;
    getPreviewMesh( SMDSAbs_Face )->Copy( faces, copyFaces, select );
    workElements = & copyFaces;
  }

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    dynamic_cast<SMESH::NumericalFunctor_i*>( SMESH_Gen_i::GetServant( Criterion ).in() );
  SMESH::Controls::NumericalFunctorPtr aCrit;
  if ( !aNumericalFunctor )
    aCrit.reset( new SMESH::Controls::MaxElementLength2D() );
  else
    aCrit = aNumericalFunctor->GetNumericalFunctor();

  if ( !myIsPreviewMode ) {
    // Update Python script
    TPythonDump() << "isDone = " << this << ".TriToQuad( "
                  << IDsOfElements << ", " << aNumericalFunctor << ", " << TVar( MaxAngle ) << " )";
  }

  bool stat = getEditor().TriToQuad( *workElements, aCrit, MaxAngle );

  declareMeshModified( /*isReComputeSafe=*/!stat );
  return stat;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * \brief Fuse neighbour triangles into quadrangles.
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::TriToQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
                                                    SMESH::NumericalFunctor_ptr Criterion,
                                                    CORBA::Double               MaxAngle)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aTPythonDump;  // suppress dump in TriToQuad()

  prepareIdSource( theObject );
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = TriToQuad(anElementsId, Criterion, MaxAngle);

  if ( !myIsPreviewMode ) {
    SMESH::NumericalFunctor_i* aNumericalFunctor =
      SMESH::DownCast<SMESH::NumericalFunctor_i*>( Criterion );

    // Update Python script
    aTPythonDump << "isDone = " << this << ".TriToQuadObject("
                 << theObject << ", " << aNumericalFunctor << ", " << TVar( MaxAngle ) << " )";
  }

  return isDone;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * \brief Split quadrangles into triangles.
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::QuadToTri (const SMESH::long_array &   IDsOfElements,
                                              SMESH::NumericalFunctor_ptr Criterion)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();
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

  CORBA::Boolean stat = getEditor().QuadToTri( faces, aCrit );

  declareMeshModified( /*isReComputeSafe=*/false );
  return stat;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * \brief Split quadrangles into triangles.
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::QuadToTriObject (SMESH::SMESH_IDSource_ptr   theObject,
                                                    SMESH::NumericalFunctor_ptr Criterion)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aTPythonDump;  // suppress dump in QuadToTri()

  prepareIdSource( theObject );
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = QuadToTri(anElementsId, Criterion);

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    SMESH::DownCast<SMESH::NumericalFunctor_i*>( Criterion );

  // Update Python script
  aTPythonDump << "isDone = " << this << ".QuadToTriObject( " << theObject << ", " << aNumericalFunctor << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return isDone;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
 * \brief Split each of quadrangles into 4 triangles.
 *  \param [in] theObject - theQuads Container of quadrangles to split.
 */
//================================================================================

void SMESH_MeshEditor_i::QuadTo4Tri (SMESH::SMESH_IDSource_ptr theObject)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedElemSet faces;
  if ( !idSourceToSet( theObject, getMeshDS(), faces, SMDSAbs_Face, /*emptyIfIsMesh=*/true ) &&
       faces.empty() )
    THROW_SALOME_CORBA_EXCEPTION("No faces given", SALOME::BAD_PARAM);

  getEditor().QuadTo4Tri( faces );
  TPythonDump() << this << ".QuadTo4Tri( " << theObject << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=============================================================================
/*!
 * \brief Split quadrangles into triangles.
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::SplitQuad (const SMESH::long_array & IDsOfElements,
                                              CORBA::Boolean            Diag13)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();
  TIDSortedElemSet faces;
  arrayToSet(IDsOfElements, aMesh, faces, SMDSAbs_Face);

  // Update Python script
  TPythonDump() << "isDone = " << this << ".SplitQuad( "
                << IDsOfElements << ", " << Diag13 << " )";

  CORBA::Boolean stat = getEditor().QuadToTri( faces, Diag13 );

  declareMeshModified( /*isReComputeSafe=*/ !stat );
  return stat;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 * \brief Split quadrangles into triangles.
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::SplitQuadObject (SMESH::SMESH_IDSource_ptr theObject,
                                                    CORBA::Boolean            Diag13)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aTPythonDump;  // suppress dump in SplitQuad()

  prepareIdSource( theObject );
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = SplitQuad(anElementsId, Diag13);

  // Update Python script
  aTPythonDump << "isDone = " << this << ".SplitQuadObject( "
               << theObject << ", " << Diag13 << " )";

  declareMeshModified( /*isReComputeSafe=*/!isDone );
  return isDone;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}


//=============================================================================
/*!
 * Find better splitting of the given quadrangle.
 *  \param IDOfQuad  ID of the quadrangle to be splitted.
 *  \param Criterion A criterion to choose a diagonal for splitting.
 *  \return 1 if 1-3 diagonal is better, 2 if 2-4
 *          diagonal is better, 0 if error occurs.
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::BestSplit (CORBA::Long                 IDOfQuad,
                                           SMESH::NumericalFunctor_ptr Criterion)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  const SMDS_MeshElement* quad = getMeshDS()->FindElement(IDOfQuad);
  if (quad && quad->GetType() == SMDSAbs_Face && quad->NbNodes() == 4)
  {
    SMESH::NumericalFunctor_i* aNumericalFunctor =
      dynamic_cast<SMESH::NumericalFunctor_i*>(SMESH_Gen_i::GetServant(Criterion).in());
    SMESH::Controls::NumericalFunctorPtr aCrit;
    if (aNumericalFunctor)
      aCrit = aNumericalFunctor->GetNumericalFunctor();
    else
      aCrit.reset(new SMESH::Controls::AspectRatio());

    int id = getEditor().BestSplit(quad, aCrit);
    declareMeshModified( /*isReComputeSafe=*/ id < 1 );
    return id;
  }

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  SMESH_TRY;
  initData();

  ::SMESH_MeshEditor::TFacetOfElem elemSet;
  const int noneFacet = -1;
  SMDS_ElemIteratorPtr volIt = myMesh_i->GetElements( elems, SMESH::VOLUME );
  while( volIt->more() )
    elemSet.insert( elemSet.end(), make_pair( volIt->next(), noneFacet ));

  getEditor().SplitVolumes( elemSet, int( methodFlags ));
  declareMeshModified( /*isReComputeSafe=*/true ); // it does not influence Compute()

  TPythonDump() << this << ".SplitVolumesIntoTetra( "
                << elems << ", " << methodFlags << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Split hexahedra into triangular prisms
 *  \param elems - elements to split
 *  \param facetToSplitNormal - normal used to find a facet of hexahedron
 *         to split into triangles
 *  \param methodFlags - flags passing splitting method:
 *         1 - split the hexahedron into 2 prisms
 *         2 - split the hexahedron into 4 prisms
 */
//================================================================================

void SMESH_MeshEditor_i::SplitHexahedraIntoPrisms( SMESH::SMESH_IDSource_ptr  elems,
                                                   const SMESH::PointStruct & startHexPoint,
                                                   const SMESH::DirStruct&    facetToSplitNormal,
                                                   CORBA::Short               methodFlags,
                                                   CORBA::Boolean             allDomains)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();
  prepareIdSource( elems );

  gp_Ax1 facetNorm( gp_Pnt( startHexPoint.x,
                            startHexPoint.y,
                            startHexPoint.z ),
                    gp_Dir( facetToSplitNormal.PS.x,
                            facetToSplitNormal.PS.y,
                            facetToSplitNormal.PS.z ));
  TIDSortedElemSet elemSet;
  SMESH::long_array_var anElementsId = elems->GetIDs();
  SMDS_MeshElement::GeomFilter filter( SMDSGeom_HEXA );
  arrayToSet( anElementsId, getMeshDS(), elemSet, SMDSAbs_Volume, &filter );

  ::SMESH_MeshEditor::TFacetOfElem elemFacets;
  while ( !elemSet.empty() )
  {
    getEditor().GetHexaFacetsToSplit( elemSet, facetNorm, elemFacets );
    if ( !allDomains )
      break;

    ::SMESH_MeshEditor::TFacetOfElem::iterator ef = elemFacets.begin();
    for ( ; ef != elemFacets.end(); ++ef )
      elemSet.erase( ef->first );
  }

  if ( methodFlags == 2 )
    methodFlags = int( ::SMESH_MeshEditor::HEXA_TO_4_PRISMS );
  else
    methodFlags = int( ::SMESH_MeshEditor::HEXA_TO_2_PRISMS );

  getEditor().SplitVolumes( elemFacets, int( methodFlags ));
  declareMeshModified( /*isReComputeSafe=*/true ); // it does not influence Compute()

  TPythonDump() << this << ".SplitHexahedraIntoPrisms( "
                << elems << ", "
                << startHexPoint << ", "
                << facetToSplitNormal<< ", "
                << methodFlags<< ", "
                << allDomains << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Split bi-quadratic elements into linear ones without creation of additional nodes:
 *   - bi-quadratic triangle will be split into 3 linear quadrangles;
 *   - bi-quadratic quadrangle will be split into 4 linear quadrangles;
 *   - tri-quadratic hexahedron will be split into 8 linear hexahedra.
 *   Quadratic elements of lower dimension  adjacent to the split bi-quadratic element
 *   will be split in order to keep the mesh conformal.
 *  \param elems - elements to split
 */
//================================================================================

void SMESH_MeshEditor_i::SplitBiQuadraticIntoLinear(const SMESH::ListOfIDSources& theElems)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedElemSet elemSet;
  for ( size_t i = 0; i < theElems.length(); ++i )
  {
    SMESH::SMESH_IDSource_ptr elems = theElems[i].in();
    SMESH::SMESH_Mesh_var      mesh = elems->GetMesh();
    if ( mesh->GetId() != myMesh_i->GetId() )
      THROW_SALOME_CORBA_EXCEPTION("Wrong mesh of IDSource", SALOME::BAD_PARAM);

    idSourceToSet( elems, getMeshDS(), elemSet, SMDSAbs_All );
  }
  getEditor().SplitBiQuadraticIntoLinear( elemSet );

  declareMeshModified( /*isReComputeSafe=*/true ); // it does not influence Compute()

  TPythonDump() << this << ".SplitBiQuadraticIntoLinear( "
                << theElems << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
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
  throw (SALOME::SALOME_Exception)
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
  throw (SALOME::SALOME_Exception)
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
  throw (SALOME::SALOME_Exception)
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
  throw (SALOME::SALOME_Exception)
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();

  TIDSortedElemSet elements;
  arrayToSet(IDsOfElements, aMesh, elements, SMDSAbs_Face);

  set<const SMDS_MeshNode*> fixedNodes;
  for ( CORBA::ULong i = 0; i < IDsOfFixedNodes.length(); i++) {
    CORBA::Long index = IDsOfFixedNodes[i];
    const SMDS_MeshNode * node = aMesh->FindNode(index);
    if ( node )
      fixedNodes.insert( node );
  }
  ::SMESH_MeshEditor::SmoothMethod method = ::SMESH_MeshEditor::LAPLACIAN;
  if ( Method != SMESH::SMESH_MeshEditor::LAPLACIAN_SMOOTH )
    method = ::SMESH_MeshEditor::CENTROIDAL;

  getEditor().Smooth(elements, fixedNodes, method,
                  MaxNbOfIterations, MaxAspectRatio, IsParametric );

  declareMeshModified( /*isReComputeSafe=*/true ); // does not prevent re-compute

  // Update Python script
  TPythonDump() << "isDone = " << this << "."
                << (IsParametric ? "SmoothParametric( " : "Smooth( ")
                << IDsOfElements << ", "     << IDsOfFixedNodes << ", "
                << TVar( MaxNbOfIterations ) << ", " << TVar( MaxAspectRatio ) << ", "
                << "SMESH.SMESH_MeshEditor."
                << ( Method == SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH ?
                     "CENTROIDAL_SMOOTH )" : "LAPLACIAN_SMOOTH )");

  return true;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aTPythonDump;  // suppress dump in smooth()

  prepareIdSource( theObject );
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

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberNodes()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  // Update Python script
  TPythonDump() << this << ".RenumberNodes()";

  getMeshDS()->Renumber( true );

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberElements()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  // Update Python script
  TPythonDump() << this << ".RenumberElements()";

  getMeshDS()->Renumber( false );

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
/*!
 * \brief Return groups by their IDs
 */
//=======================================================================

SMESH::ListOfGroups* SMESH_MeshEditor_i::getGroups(const std::list<int>* groupIDs)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  if ( !groupIDs )
    return 0;
  myMesh_i->CreateGroupServants();
  return myMesh_i->GetGroups( *groupIDs );

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : RotationSweepObjects
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::RotationSweepObjects(const SMESH::ListOfIDSources & theNodes,
                                         const SMESH::ListOfIDSources & theEdges,
                                         const SMESH::ListOfIDSources & theFaces,
                                         const SMESH::AxisStruct &      theAxis,
                                         CORBA::Double                  theAngleInRadians,
                                         CORBA::Long                    theNbOfSteps,
                                         CORBA::Double                  theTolerance,
                                         const bool                     theMakeGroups)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedElemSet elemsNodes[2];
  for ( int i = 0, nb = theNodes.length(); i < nb; ++i ) {
    SMDS_ElemIteratorPtr nIt = myMesh_i->GetElements( theNodes[i], SMESH::NODE );
    while ( nIt->more() ) elemsNodes[1].insert( nIt->next() );
  }
  for ( int i = 0, nb = theEdges.length(); i < nb; ++i )
    idSourceToSet( theEdges[i], getMeshDS(), elemsNodes[0], SMDSAbs_Edge );
  for ( int i = 0, nb = theFaces.length(); i < nb; ++i )
    idSourceToSet( theFaces[i], getMeshDS(), elemsNodes[0], SMDSAbs_Face );

  TIDSortedElemSet* workElements = & elemsNodes[0], copyElements[2];
  bool              makeWalls=true;
  if ( myIsPreviewMode )
  {
    SMDSAbs_ElementType select = SMDSAbs_All, avoid = SMDSAbs_Volume;
    TPreviewMesh * tmpMesh = getPreviewMesh();
    tmpMesh->Copy( elemsNodes[0], copyElements[0], select, avoid );
    tmpMesh->Copy( elemsNodes[1], copyElements[1], select, avoid );
    workElements = & copyElements[0];
    //makeWalls = false; -- faces are needed for preview
  }

  TPythonDump aPythonDump; // it is here to prevent dump of getGroups()

  gp_Ax1 Ax1 (gp_Pnt( theAxis.x,  theAxis.y,  theAxis.z ),
              gp_Vec( theAxis.vx, theAxis.vy, theAxis.vz ));

  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().RotationSweep (workElements, Ax1, theAngleInRadians,
                                 theNbOfSteps, theTolerance, theMakeGroups, makeWalls);

  SMESH::ListOfGroups * aGroups = theMakeGroups ? getGroups( groupIds.get()) : 0;

  declareMeshModified( /*isReComputeSafe=*/true ); // does not influence Compute()

  if ( !myIsPreviewMode )
  {
    dumpGroupsList( aPythonDump, aGroups );
    aPythonDump << this<< ".RotationSweepObjects( "
                << theNodes                  << ", "
                << theEdges                  << ", "
                << theFaces                  << ", "
                << theAxis                   << ", "
                << TVar( theAngleInRadians ) << ", "
                << TVar( theNbOfSteps      ) << ", "
                << TVar( theTolerance      ) << ", "
                << theMakeGroups             << " )";
  }
  else
  {
    getPreviewMesh()->Remove( SMDSAbs_Volume );
  }

  return aGroups ? aGroups : new SMESH::ListOfGroups;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

namespace MeshEditor_I
{
  /*!
   * \brief Structure used to pass extrusion parameters to ::SMESH_MeshEditor
   */
  struct ExtrusionParams : public ::SMESH_MeshEditor::ExtrusParam
  {
    bool myIsExtrusionByNormal;

    static int makeFlags( CORBA::Boolean MakeGroups,
                          CORBA::Boolean LinearVariation = false,
                          CORBA::Boolean ByAverageNormal = false,
                          CORBA::Boolean UseInputElemsOnly = false,
                          CORBA::Long    Flags = 0,
                          CORBA::Boolean MakeBoundary = true )
    {
      if ( MakeGroups       ) Flags |= ::SMESH_MeshEditor::EXTRUSION_FLAG_GROUPS;
      if ( ByAverageNormal  ) Flags |= ::SMESH_MeshEditor::EXTRUSION_FLAG_BY_AVG_NORMAL;
      if ( UseInputElemsOnly) Flags |= ::SMESH_MeshEditor::EXTRUSION_FLAG_USE_INPUT_ELEMS_ONLY;
      if ( LinearVariation  ) Flags |= ::SMESH_MeshEditor::EXTRUSION_FLAG_SCALE_LINEAR_VARIATION;
      if ( MakeBoundary     ) Flags |= ::SMESH_MeshEditor::EXTRUSION_FLAG_BOUNDARY;
      return Flags;
    }
    // standard params
    ExtrusionParams(const SMESH::DirStruct &    theDir,
                    CORBA::Long                 theNbOfSteps,
                    const SMESH::double_array & theScaleFactors,
                    CORBA::Boolean              theLinearVariation,
                    const SMESH::double_array & theBasePoint,
                    CORBA::Boolean              theMakeGroups):
      ::SMESH_MeshEditor::ExtrusParam ( gp_Vec( theDir.PS.x,
                                                theDir.PS.y,
                                                theDir.PS.z ),
                                        theNbOfSteps,
                                        toList( theScaleFactors ),
                                        TBasePoint( theBasePoint ),
                                        makeFlags( theMakeGroups, theLinearVariation )),
      myIsExtrusionByNormal( false )
    {
    }
    // advanced params
    ExtrusionParams(const SMESH::DirStruct &  theDir,
                    CORBA::Long               theNbOfSteps,
                    CORBA::Boolean            theMakeGroups,
                    CORBA::Long               theExtrFlags,
                    CORBA::Double             theSewTolerance):
      ::SMESH_MeshEditor::ExtrusParam ( gp_Vec( theDir.PS.x,
                                                theDir.PS.y,
                                                theDir.PS.z ),
                                        theNbOfSteps,
                                        std::list<double>(),
                                        0,
                                        makeFlags( theMakeGroups, false, false, false,
                                                   theExtrFlags, false ),
                                        theSewTolerance ),
      myIsExtrusionByNormal( false )
    {
    }
    // params for extrusion by normal
    ExtrusionParams(CORBA::Double  theStepSize,
                    CORBA::Long    theNbOfSteps,
                    CORBA::Short   theDim,
                    CORBA::Boolean theByAverageNormal,
                    CORBA::Boolean theUseInputElemsOnly,
                    CORBA::Boolean theMakeGroups ):
      ::SMESH_MeshEditor::ExtrusParam ( theStepSize, 
                                        theNbOfSteps,
                                        makeFlags( theMakeGroups, false,
                                                   theByAverageNormal, theUseInputElemsOnly ),
                                        theDim),
      myIsExtrusionByNormal( true )
    {
    }

    void SetNoGroups()
    {
      Flags() &= ~(::SMESH_MeshEditor::EXTRUSION_FLAG_GROUPS);
    }

  private:

    static std::list<double> toList( const SMESH::double_array & theScaleFactors )
    {
      std::list<double> scales;
      for ( CORBA::ULong i = 0; i < theScaleFactors.length(); ++i )
        scales.push_back( theScaleFactors[i] );
      return scales;
    }

    // structure used to convert SMESH::double_array to gp_XYZ*
    struct TBasePoint
    {
      gp_XYZ *pp, p;
      TBasePoint( const SMESH::double_array & theBasePoint )
      {
        pp = 0;
        if ( theBasePoint.length() == 3 )
        {
          p.SetCoord( theBasePoint[0], theBasePoint[1], theBasePoint[2] );
          pp = &p;
        }
      }
      operator const gp_XYZ*() const { return pp; }
    };
  };
}

//=======================================================================
/*!
 * \brief Generate dim+1 elements by extrusion of elements along vector
 *  \param [in] edges - edges to extrude: a list including groups, sub-meshes or a mesh
 *  \param [in] faces - faces to extrude: a list including groups, sub-meshes or a mesh
 *  \param [in] nodes - nodes to extrude: a list including groups, sub-meshes or a mesh
 *  \param [in] stepVector - vector giving direction and distance of an extrusion step
 *  \param [in] nbOfSteps - number of elements to generate from one element
 *  \param [in] toMakeGroups - if true, new elements will be included into new groups
 *              corresponding to groups the input elements included in.
 *  \return ListOfGroups - new groups craeted if \a toMakeGroups is true
 */
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionSweepObjects(const SMESH::ListOfIDSources & theNodes,
                                          const SMESH::ListOfIDSources & theEdges,
                                          const SMESH::ListOfIDSources & theFaces,
                                          const SMESH::DirStruct &       theStepVector,
                                          CORBA::Long                    theNbOfSteps,
                                          const SMESH::double_array &    theScaleFactors,
                                          CORBA::Boolean                 theLinearVariation,
                                          const SMESH::double_array &    theBasePoint,
                                          CORBA::Boolean                 theToMakeGroups)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  ExtrusionParams params( theStepVector, theNbOfSteps, theScaleFactors,
                          theLinearVariation, theBasePoint, theToMakeGroups );

  TIDSortedElemSet elemsNodes[2];
  for ( int i = 0, nb = theNodes.length(); i < nb; ++i ) {
    SMDS_ElemIteratorPtr nIt = myMesh_i->GetElements( theNodes[i], SMESH::NODE );
    while ( nIt->more() ) elemsNodes[1].insert( nIt->next() );
  }
  for ( int i = 0, nb = theEdges.length(); i < nb; ++i )
    idSourceToSet( theEdges[i], getMeshDS(), elemsNodes[0], SMDSAbs_Edge );
  for ( int i = 0, nb = theFaces.length(); i < nb; ++i )
    idSourceToSet( theFaces[i], getMeshDS(), elemsNodes[0], SMDSAbs_Face );

  TIDSortedElemSet* workElements = & elemsNodes[0], copyElements[2];
  SMDSAbs_ElementType previewType = SMDSAbs_All; //SMDSAbs_Face;
  if ( myIsPreviewMode )
  {
    // if ( (*elemsNodes.begin())->GetType() == SMDSAbs_Node )
    //   previewType = SMDSAbs_Edge;

    SMDSAbs_ElementType select = SMDSAbs_All, avoid = SMDSAbs_Volume;
    TPreviewMesh * tmpMesh = getPreviewMesh( previewType );
    tmpMesh->Copy( elemsNodes[0], copyElements[0], select, avoid );
    tmpMesh->Copy( elemsNodes[1], copyElements[1], select, avoid );
    workElements = & copyElements[0];

    params.SetNoGroups();
  }
  TPythonDump aPythonDump; // it is here to prevent dump of getGroups()

  ::SMESH_MeshEditor::TTElemOfElemListMap aHistory;
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().ExtrusionSweep( workElements, params, aHistory );

  SMESH::ListOfGroups * aGroups = theToMakeGroups ? getGroups( groupIds.get()) : 0;

  declareMeshModified( /*isReComputeSafe=*/true ); // does not influence Compute()

  if ( !myIsPreviewMode )
  {
    dumpGroupsList( aPythonDump, aGroups );
    aPythonDump << this<< ".ExtrusionSweepObjects( "
                << theNodes             << ", "
                << theEdges             << ", "
                << theFaces             << ", "
                << theStepVector        << ", "
                << TVar( theNbOfSteps ) << ", "
                << theToMakeGroups      << " )";
  }
  else
  {
    getPreviewMesh( previewType )->Remove( SMDSAbs_Volume );
  }

  return aGroups ? aGroups : new SMESH::ListOfGroups;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : ExtrusionByNormal
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionByNormal(const SMESH::ListOfIDSources& objects,
                                      CORBA::Double                 stepSize,
                                      CORBA::Long                   nbOfSteps,
                                      CORBA::Boolean                byAverageNormal,
                                      CORBA::Boolean                useInputElemsOnly,
                                      CORBA::Boolean                makeGroups,
                                      CORBA::Short                  dim)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  ExtrusionParams params( stepSize, nbOfSteps, dim,
                          byAverageNormal, useInputElemsOnly, makeGroups );

  SMDSAbs_ElementType elemType = ( dim == 1 ? SMDSAbs_Edge : SMDSAbs_Face );
  if ( objects.length() > 0 && !SMESH::DownCast<SMESH_Mesh_i*>( objects[0] ))
  {
    SMESH::array_of_ElementType_var elemTypes = objects[0]->GetTypes();
    if (( elemTypes->length() == 1 ) &&
        ( elemTypes[0] == SMESH::EDGE || elemTypes[0] == SMESH::FACE ))
      elemType = ( SMDSAbs_ElementType ) elemTypes[0];
  }

  TIDSortedElemSet elemsNodes[2];
  for ( int i = 0, nb = objects.length(); i < nb; ++i )
    idSourceToSet( objects[i], getMeshDS(), elemsNodes[0], elemType );

  TIDSortedElemSet* workElements = & elemsNodes[0], copyElements[2];
  SMDSAbs_ElementType previewType = SMDSAbs_Face;
  if ( myIsPreviewMode )
  {
    SMDSAbs_ElementType select = SMDSAbs_All, avoid = SMDSAbs_Volume;
    TPreviewMesh * tmpMesh = getPreviewMesh( previewType );
    tmpMesh->Copy( elemsNodes[0], copyElements[0], select, avoid );
    workElements = & copyElements[0];

    params.SetNoGroups();
  }

  ::SMESH_MeshEditor::TTElemOfElemListMap aHistory;
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().ExtrusionSweep( workElements, params, aHistory );

  SMESH::ListOfGroups * aGroups = makeGroups ? getGroups( groupIds.get()) : 0;

  if (!myIsPreviewMode) {
    dumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".ExtrusionByNormal( " << objects
                << ", " << TVar( stepSize )
                << ", " << TVar( nbOfSteps )
                << ", " << byAverageNormal
                << ", " << useInputElemsOnly
                << ", " << makeGroups
                << ", " << dim
                << " )";
  }
  else
  {
    getPreviewMesh( previewType )->Remove( SMDSAbs_Volume );
  }

  declareMeshModified( /*isReComputeSafe=*/true ); // does not influence Compute()

  return aGroups ? aGroups : new SMESH::ListOfGroups;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : AdvancedExtrusion
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::AdvancedExtrusion(const SMESH::long_array & theIDsOfElements,
                                      const SMESH::DirStruct &  theStepVector,
                                      CORBA::Long               theNbOfSteps,
                                      CORBA::Long               theExtrFlags,
                                      CORBA::Double             theSewTolerance,
                                      CORBA::Boolean            theMakeGroups)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aPythonDump; // it is here to prevent dump of getGroups()

  ExtrusionParams params( theStepVector, theNbOfSteps, theMakeGroups,
                          theExtrFlags, theSewTolerance );

  TIDSortedElemSet elemsNodes[2];
  arrayToSet( theIDsOfElements, getMeshDS(), elemsNodes[0] );

  ::SMESH_MeshEditor::TTElemOfElemListMap aHistory;
  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().ExtrusionSweep( elemsNodes, params, aHistory );

  SMESH::ListOfGroups * aGroups = theMakeGroups ? getGroups( groupIds.get()) : 0;

  declareMeshModified( /*isReComputeSafe=*/true ); // does not influence Compute()

  if ( !myIsPreviewMode ) {
    dumpGroupsList(aPythonDump, aGroups);
    aPythonDump << this << ".AdvancedExtrusion( "
                << theIDsOfElements << ", "
                << theStepVector << ", "
                << theNbOfSteps << ", "
                << theExtrFlags << ", "
                << theSewTolerance << ", "
                << theMakeGroups << " )";
  }
  else
  {
    getPreviewMesh()->Remove( SMDSAbs_Volume );
  }

  return aGroups ? aGroups : new SMESH::ListOfGroups;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
 * \brief Convert extrusion error to IDL enum
 */
//================================================================================

namespace
{
#define RETCASE(enm) case ::SMESH_MeshEditor::enm: return SMESH::SMESH_MeshEditor::enm;

  SMESH::SMESH_MeshEditor::Extrusion_Error convExtrError( ::SMESH_MeshEditor::Extrusion_Error e )
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
}

//=======================================================================
//function : extrusionAlongPath
//purpose  :
//=======================================================================
SMESH::ListOfGroups*
SMESH_MeshEditor_i::ExtrusionAlongPathObjects(const SMESH::ListOfIDSources & theNodes,
                                              const SMESH::ListOfIDSources & theEdges,
                                              const SMESH::ListOfIDSources & theFaces,
                                              SMESH::SMESH_IDSource_ptr      thePathMesh,
                                              GEOM::GEOM_Object_ptr          thePathShape,
                                              CORBA::Long                    theNodeStart,
                                              CORBA::Boolean                 theHasAngles,
                                              const SMESH::double_array &    theAngles,
                                              CORBA::Boolean                 theLinearVariation,
                                              CORBA::Boolean                 theHasRefPoint,
                                              const SMESH::PointStruct &     theRefPoint,
                                              bool                           theMakeGroups,
                                              SMESH::SMESH_MeshEditor::Extrusion_Error& theError)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESH::ListOfGroups_var aGroups = new SMESH::ListOfGroups;

  theError = SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;
  if ( thePathMesh->_is_nil() )
    return aGroups._retn();

  // get a sub-mesh
  SMESH_subMesh* aSubMesh = 0;
  SMESH_Mesh_i* aMeshImp = SMESH::DownCast<SMESH_Mesh_i*>( thePathMesh );
  if ( thePathShape->_is_nil() )
  {
    // thePathMesh should be either a sub-mesh or a mesh with 1D elements only
    if ( SMESH_subMesh_i* sm = SMESH::DownCast<SMESH_subMesh_i*>( thePathMesh ))
    {
      SMESH::SMESH_Mesh_var mesh = thePathMesh->GetMesh();
      aMeshImp = SMESH::DownCast<SMESH_Mesh_i*>( mesh );
      if ( !aMeshImp ) return aGroups._retn();
      aSubMesh = aMeshImp->GetImpl().GetSubMeshContaining( sm->GetId() );
      if ( !aSubMesh ) return aGroups._retn();
    }
    else if ( !aMeshImp ||
              aMeshImp->NbEdges() != aMeshImp->NbElements() )
    {
      return aGroups._retn();
    }
  }
  else
  {
    if ( !aMeshImp ) return aGroups._retn();
    TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( thePathShape );
    aSubMesh = aMeshImp->GetImpl().GetSubMesh( aShape );
    if ( !aSubMesh /*|| !aSubMesh->GetSubMeshDS()*/ )
      return aGroups._retn();
  }

  SMDS_MeshNode* nodeStart =
    (SMDS_MeshNode*)aMeshImp->GetImpl().GetMeshDS()->FindNode(theNodeStart);
  if ( !nodeStart ) {
    theError = SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE;
    return aGroups._retn();
  }

  TIDSortedElemSet elemsNodes[2];
  for ( int i = 0, nb = theNodes.length(); i < nb; ++i ) {
    SMDS_ElemIteratorPtr nIt = myMesh_i->GetElements( theNodes[i], SMESH::NODE );
    while ( nIt->more() ) elemsNodes[1].insert( nIt->next() );
  }
  for ( int i = 0, nb = theEdges.length(); i < nb; ++i )
    idSourceToSet( theEdges[i], getMeshDS(), elemsNodes[0], SMDSAbs_Edge );
  for ( int i = 0, nb = theFaces.length(); i < nb; ++i )
    idSourceToSet( theFaces[i], getMeshDS(), elemsNodes[0], SMDSAbs_Face );

  list<double> angles;
  for ( CORBA::ULong i = 0; i < theAngles.length(); i++ ) {
    angles.push_back( theAngles[i] );
  }

  gp_Pnt refPnt( theRefPoint.x, theRefPoint.y, theRefPoint.z );

  int nbOldGroups = myMesh->NbGroup();

  TIDSortedElemSet* workElements = & elemsNodes[0], copyElements[2];
  if ( myIsPreviewMode )
  {
    SMDSAbs_ElementType select = SMDSAbs_All, avoid = SMDSAbs_Volume;
    TPreviewMesh * tmpMesh = getPreviewMesh();
    tmpMesh->Copy( elemsNodes[0], copyElements[0], select, avoid );
    tmpMesh->Copy( elemsNodes[1], copyElements[1], select, avoid );
    workElements = & copyElements[0];
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor::Extrusion_Error error;
  if ( !aSubMesh )
    error = getEditor().ExtrusionAlongTrack( workElements, &(aMeshImp->GetImpl()), nodeStart,
                                             theHasAngles, angles, theLinearVariation,
                                             theHasRefPoint, refPnt, theMakeGroups );
  else
    error = getEditor().ExtrusionAlongTrack( workElements, aSubMesh, nodeStart,
                                             theHasAngles, angles, theLinearVariation,
                                             theHasRefPoint, refPnt, theMakeGroups );

  declareMeshModified( /*isReComputeSafe=*/true );
  theError = convExtrError( error );

  TPythonDump aPythonDump; // it is here to prevent dump of getGroups()
  if ( theMakeGroups ) {
    list<int> groupIDs = myMesh->GetGroupIds();
    list<int>::iterator newBegin = groupIDs.begin();
    std::advance( newBegin, nbOldGroups ); // skip old groups
    groupIDs.erase( groupIDs.begin(), newBegin );
    aGroups = getGroups( & groupIDs );
    if ( ! &aGroups.in() ) aGroups = new SMESH::ListOfGroups;
  }

  if ( !myIsPreviewMode ) {
    if ( aGroups->length() > 0 ) aPythonDump << "(" << aGroups << ", error) = ";
    else                         aPythonDump << "(_noGroups, error) = ";
    aPythonDump << this << ".ExtrusionAlongPathObjects( "
                << theNodes            << ", "
                << theEdges            << ", "
                << theFaces            << ", "
                << thePathMesh         << ", "
                << thePathShape        << ", "
                << theNodeStart        << ", "
                << theHasAngles        << ", "
                << TVar( theAngles )   << ", "
                << theLinearVariation  << ", "
                << theHasRefPoint      << ", "
                << "SMESH.PointStruct( "
                << TVar( theHasRefPoint ? theRefPoint.x : 0 ) << ", "
                << TVar( theHasRefPoint ? theRefPoint.y : 0 ) << ", "
                << TVar( theHasRefPoint ? theRefPoint.z : 0 ) << " ), "
                << theMakeGroups       << " )";
  }
  else
  {
    getPreviewMesh()->Remove( SMDSAbs_Volume );
  }

  return aGroups._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
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
  TIDSortedElemSet* workElements = & theElements;

  if ( myIsPreviewMode )
  {
    TPreviewMesh * tmpMesh = getPreviewMesh();
    tmpMesh->Copy( theElements, copyElements);
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( theElements, getMeshDS(), elemsAround );
      tmpMesh->Copy( elemsAround, elemsAroundCopy);
    }
    workElements = & copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if ( theCopy && !myIsPreviewMode)
  {
    if ( theTargetMesh )
    {
      theTargetMesh->GetMeshDS()->Modified();
    }
    else
    {
      declareMeshModified( /*isReComputeSafe=*/false );
    }
  }
  return theMakeGroups ? getGroups(groupIds.get()) : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : Mirror
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Mirror(const SMESH::long_array &           theIDsOfElements,
                                const SMESH::AxisStruct &           theAxis,
                                SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                                CORBA::Boolean                      theCopy)
  throw (SALOME::SALOME_Exception)
{
  if ( !myIsPreviewMode ) {
    TPythonDump() << this << ".Mirror( "
                  << theIDsOfElements              << ", "
                  << theAxis                       << ", "
                  << mirrorTypeName(theMirrorType) << ", "
                  << theCopy                       << " )";
  }
  if ( theIDsOfElements.length() > 0 )
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, getMeshDS(), elements);
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
  throw (SALOME::SALOME_Exception)
{
  if ( !myIsPreviewMode ) {
    TPythonDump() << this << ".MirrorObject( "
                  << theObject                     << ", "
                  << theAxis                       << ", "
                  << mirrorTypeName(theMirrorType) << ", "
                  << theCopy                       << " )";
  }
  TIDSortedElemSet elements;

  bool emptyIfIsMesh = myIsPreviewMode ? false : true;

  if (idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
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
  throw (SALOME::SALOME_Exception)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  if ( theIDsOfElements.length() > 0 )
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, getMeshDS(), elements);
    aGroups = mirror(elements, theMirror, theMirrorType, true, true);
  }
  if (!myIsPreviewMode) {
    dumpGroupsList(aPythonDump, aGroups);
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
  throw (SALOME::SALOME_Exception)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  TIDSortedElemSet elements;
  if ( idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    aGroups = mirror(elements, theMirror, theMirrorType, true, true);

  if (!myIsPreviewMode)
  {
    dumpGroupsList(aPythonDump,aGroups);
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
  throw (SALOME::SALOME_Exception)
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
      arrayToSet(theIDsOfElements, getMeshDS(), elements);
      mirror(elements, theMirror, theMirrorType,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }

    if (!myIsPreviewMode) {
      pydump << mesh << " = " << this << ".MirrorMakeMesh( "
             << theIDsOfElements              << ", "
             << theMirror                     << ", "
             << mirrorTypeName(theMirrorType) << ", "
             << theCopyGroups                 << ", '"
             << theMeshName                   << "' )";
    }
  }

  //dump "GetGroups"
  if (!myIsPreviewMode && mesh_i)
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
  throw (SALOME::SALOME_Exception)
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
         idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    {
      mirror(elements, theMirror, theMirrorType,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if (!myIsPreviewMode) {
      pydump << mesh << " = " << this << ".MirrorObjectMakeMesh( "
             << theObject                     << ", "
             << theMirror                     << ", "
             << mirrorTypeName(theMirrorType) << ", "
             << theCopyGroups                 << ", '"
             << theMeshName                   << "' )";
    }
  }

  //dump "GetGroups"
  if (!myIsPreviewMode && mesh_i)
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  if ( theTargetMesh )
    theCopy = false;

  gp_Trsf aTrsf;
  const SMESH::PointStruct * P = &theVector.PS;
  aTrsf.SetTranslation( gp_Vec( P->x, P->y, P->z ));

  TIDSortedElemSet  copyElements;
  TIDSortedElemSet* workElements = &theElements;

  if ( myIsPreviewMode )
  {
    TPreviewMesh * tmpMesh = getPreviewMesh();
    tmpMesh->Copy( theElements, copyElements);
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( theElements, getMeshDS(), elemsAround );
      tmpMesh->Copy( elemsAround, elemsAroundCopy);
    }
    workElements = & copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if ( theCopy && !myIsPreviewMode )
  {
    if ( theTargetMesh )
    {
      theTargetMesh->GetMeshDS()->Modified();
    }
    else
    {
      declareMeshModified( /*isReComputeSafe=*/false );
    }
  }

  return theMakeGroups ? getGroups(groupIds.get()) : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : Translate
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Translate(const SMESH::long_array & theIDsOfElements,
                                   const SMESH::DirStruct &  theVector,
                                   CORBA::Boolean            theCopy)
  throw (SALOME::SALOME_Exception)
{
  if (!myIsPreviewMode) {
    TPythonDump() << this << ".Translate( "
                  << theIDsOfElements << ", "
                  << theVector        << ", "
                  << theCopy          << " )";
  }
  if (theIDsOfElements.length()) {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, getMeshDS(), elements);
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
  throw (SALOME::SALOME_Exception)
{
  if (!myIsPreviewMode) {
    TPythonDump() << this << ".TranslateObject( "
                  << theObject << ", "
                  << theVector << ", "
                  << theCopy   << " )";
  }
  TIDSortedElemSet elements;

  bool emptyIfIsMesh = myIsPreviewMode ? false : true;

  if (idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
    translate(elements, theVector, theCopy, false);
}

//=======================================================================
//function : TranslateMakeGroups
//purpose  :
//=======================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::TranslateMakeGroups(const SMESH::long_array& theIDsOfElements,
                                        const SMESH::DirStruct&  theVector)
  throw (SALOME::SALOME_Exception)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  if (theIDsOfElements.length()) {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, getMeshDS(), elements);
    aGroups = translate(elements,theVector,true,true);
  }
  if (!myIsPreviewMode) {
    dumpGroupsList(aPythonDump, aGroups);
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
  throw (SALOME::SALOME_Exception)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  TIDSortedElemSet elements;
  if (idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    aGroups = translate(elements, theVector, true, true);

  if (!myIsPreviewMode) {
    dumpGroupsList(aPythonDump, aGroups);
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
  throw (SALOME::SALOME_Exception)
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
      arrayToSet(theIDsOfElements, getMeshDS(), elements);
      translate(elements, theVector, false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }

    if ( !myIsPreviewMode ) {
      pydump << mesh << " = " << this << ".TranslateMakeMesh( "
             << theIDsOfElements << ", "
             << theVector        << ", "
             << theCopyGroups    << ", '"
             << theMeshName      << "' )";
    }
  }

  //dump "GetGroups"
  if (!myIsPreviewMode && mesh_i)
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH_Mesh_i* mesh_i;
  SMESH::SMESH_Mesh_var mesh;
  { // open new scope to dump "MakeMesh" command
    // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation
    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

    TIDSortedElemSet elements;
    if ( mesh_i &&
         idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    {
      translate(elements, theVector,false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myIsPreviewMode ) {
      pydump << mesh << " = " << this << ".TranslateObjectMakeMesh( "
             << theObject     << ", "
             << theVector     << ", "
             << theCopyGroups << ", '"
             << theMeshName   << "' )";
    }
  }

  // dump "GetGroups"
  if (!myIsPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  if ( theTargetMesh )
    theCopy = false;

  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  gp_Trsf aTrsf;
  aTrsf.SetRotation( gp_Ax1( P, V ), theAngle);

  TIDSortedElemSet  copyElements;
  TIDSortedElemSet* workElements = &theElements;
  if ( myIsPreviewMode ) {
    TPreviewMesh * tmpMesh = getPreviewMesh();
    tmpMesh->Copy( theElements, copyElements );
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( theElements, getMeshDS(), elemsAround );
      tmpMesh->Copy( elemsAround, elemsAroundCopy);
    }
    workElements = &copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if ( theCopy && !myIsPreviewMode)
  {
    if ( theTargetMesh ) theTargetMesh->GetMeshDS()->Modified();
    else                 declareMeshModified( /*isReComputeSafe=*/false );
  }

  return theMakeGroups ? getGroups(groupIds.get()) : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : Rotate
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Rotate(const SMESH::long_array & theIDsOfElements,
                                const SMESH::AxisStruct & theAxis,
                                CORBA::Double             theAngle,
                                CORBA::Boolean            theCopy)
  throw (SALOME::SALOME_Exception)
{
  if (!myIsPreviewMode) {
    TPythonDump() << this << ".Rotate( "
                  << theIDsOfElements << ", "
                  << theAxis          << ", "
                  << TVar( theAngle ) << ", "
                  << theCopy          << " )";
  }
  if (theIDsOfElements.length() > 0)
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, getMeshDS(), elements);
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
  throw (SALOME::SALOME_Exception)
{
  if ( !myIsPreviewMode ) {
    TPythonDump() << this << ".RotateObject( "
                  << theObject        << ", "
                  << theAxis          << ", "
                  << TVar( theAngle ) << ", "
                  << theCopy          << " )";
  }
  TIDSortedElemSet elements;
  bool emptyIfIsMesh = myIsPreviewMode ? false : true;
  if (idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
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
  throw (SALOME::SALOME_Exception)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  if (theIDsOfElements.length() > 0)
  {
    TIDSortedElemSet elements;
    arrayToSet(theIDsOfElements, getMeshDS(), elements);
    aGroups = rotate(elements,theAxis,theAngle,true,true);
  }
  if (!myIsPreviewMode) {
    dumpGroupsList(aPythonDump, aGroups);
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
  throw (SALOME::SALOME_Exception)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = 0;
  TIDSortedElemSet elements;
  if (idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    aGroups = rotate(elements, theAxis, theAngle, true, true);

  if (!myIsPreviewMode) {
    dumpGroupsList(aPythonDump, aGroups);
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
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
      arrayToSet(theIDsOfElements, getMeshDS(), elements);
      rotate(elements, theAxis, theAngleInRadians,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myIsPreviewMode ) {
      pydump << mesh << " = " << this << ".RotateMakeMesh( "
             << theIDsOfElements          << ", "
             << theAxis                   << ", "
             << TVar( theAngleInRadians ) << ", "
             << theCopyGroups             << ", '"
             << theMeshName               << "' )";
    }
  }

  // dump "GetGroups"
  if (!myIsPreviewMode && mesh_i && theIDsOfElements.length() > 0 )
    mesh_i->GetGroups();

  return mesh._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::SMESH_Mesh_var mesh;
  SMESH_Mesh_i* mesh_i;

  {// open new scope to dump "MakeMesh" command
   // and then "GetGroups" using SMESH_Mesh::GetGroups()

    TPythonDump pydump; // to prevent dump at mesh creation
    mesh = makeMesh( theMeshName );
    mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

    TIDSortedElemSet elements;
    if (mesh_i &&
        idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, /*emptyIfIsMesh=*/1))
    {
      rotate(elements, theAxis, theAngleInRadians,
             false, theCopyGroups, & mesh_i->GetImpl());
      mesh_i->CreateGroupServants();
    }
    if ( !myIsPreviewMode ) {
      pydump << mesh << " = " << this << ".RotateObjectMakeMesh( "
             << theObject                 << ", "
             << theAxis                   << ", "
             << TVar( theAngleInRadians ) << ", "
             << theCopyGroups             << ", '"
             << theMeshName               << "' )";
    }
  }

  // dump "GetGroups"
  if (!myIsPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();
  if ( theScaleFact.length() < 1 )
    THROW_SALOME_CORBA_EXCEPTION("Scale factor not given", SALOME::BAD_PARAM);
  if ( theScaleFact.length() == 2 )
    THROW_SALOME_CORBA_EXCEPTION("Invalid nb of scale factors : 2", SALOME::BAD_PARAM);

  if ( theTargetMesh )
    theCopy = false;

  TIDSortedElemSet elements;
  bool emptyIfIsMesh = myIsPreviewMode ? false : true;
  if ( !idSourceToSet(theObject, getMeshDS(), elements, SMDSAbs_All, emptyIfIsMesh))
    return 0;

  double S[3] = {
    theScaleFact[0],
    (theScaleFact.length() == 1) ? theScaleFact[0] : theScaleFact[1],
    (theScaleFact.length() == 1) ? theScaleFact[0] : theScaleFact[2],
  };
  gp_Trsf aTrsf;

#if OCC_VERSION_LARGE > 0x06070100
  // fight against orthogonalization
  // aTrsf.SetValues( S[0], 0,    0,    thePoint.x * (1-S[0]),
  //                  0,    S[1], 0,    thePoint.y * (1-S[1]),
  //                  0,    0,    S[2], thePoint.z * (1-S[2]) );
  aTrsf.SetScale( gp::Origin(), 1.0 ); // set form which is used to make group names
  gp_XYZ & loc = ( gp_XYZ& ) aTrsf.TranslationPart();
  gp_Mat & M   = ( gp_Mat& ) aTrsf.HVectorialPart();
  loc.SetCoord( thePoint.x * (1-S[0]),
                thePoint.y * (1-S[1]),
                thePoint.z * (1-S[2]));
  M.SetDiagonal( S[0], S[1], S[2] );

#else
  double tol = std::numeric_limits<double>::max();
  aTrsf.SetValues( S[0], 0,    0,    thePoint.x * (1-S[0]),
                   0,    S[1], 0,    thePoint.y * (1-S[1]),
                   0,    0,    S[2], thePoint.z * (1-S[2]),   tol, tol);
#endif

  TIDSortedElemSet  copyElements;
  TIDSortedElemSet* workElements = &elements;
  if ( myIsPreviewMode )
  {
    TPreviewMesh * tmpMesh = getPreviewMesh();
    tmpMesh->Copy( elements, copyElements);
    if ( !theCopy && !theTargetMesh )
    {
      TIDSortedElemSet elemsAround, elemsAroundCopy;
      getElementsAround( elements, getMeshDS(), elemsAround );
      tmpMesh->Copy( elemsAround, elemsAroundCopy);
    }
    workElements = & copyElements;
    theMakeGroups = false;
  }

  ::SMESH_MeshEditor::PGroupIDs groupIds =
      getEditor().Transform (*workElements, aTrsf, theCopy, theMakeGroups, theTargetMesh);

  if ( theCopy && !myIsPreviewMode )
  {
    if ( theTargetMesh ) theTargetMesh->GetMeshDS()->Modified();
    else                 declareMeshModified( /*isReComputeSafe=*/false );
  }
  return theMakeGroups ? getGroups(groupIds.get()) : 0;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : Scale
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Scale(SMESH::SMESH_IDSource_ptr  theObject,
                               const SMESH::PointStruct&  thePoint,
                               const SMESH::double_array& theScaleFact,
                               CORBA::Boolean             theCopy)
  throw (SALOME::SALOME_Exception)
{
  if ( !myIsPreviewMode ) {
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
  throw (SALOME::SALOME_Exception)
{
  TPythonDump aPythonDump; // it is here to prevent dump of GetGroups()

  SMESH::ListOfGroups * aGroups = scale(theObject, thePoint, theScaleFact, true, true);
  if (!myIsPreviewMode) {
    dumpGroupsList(aPythonDump, aGroups);
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
  throw (SALOME::SALOME_Exception)
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
    if ( !myIsPreviewMode )
      pydump << mesh << " = " << this << ".ScaleMakeMesh( "
             << theObject            << ", "
             << thePoint             << ", "
             << TVar( theScaleFact ) << ", "
             << theCopyGroups        << ", '"
             << theMeshName          << "' )";
  }

  // dump "GetGroups"
  if (!myIsPreviewMode && mesh_i)
    mesh_i->GetGroups();

  return mesh._retn();
}


//=======================================================================
//function : findCoincidentNodes
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::
findCoincidentNodes (TIDSortedNodeSet &             Nodes,
                     CORBA::Double                  Tolerance,
                     SMESH::array_of_long_array_out GroupsOfNodes,
                     CORBA::Boolean                 SeparateCornersAndMedium)
{
  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  getEditor().FindCoincidentNodes( Nodes, Tolerance, aListOfListOfNodes, SeparateCornersAndMedium );

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
}

//=======================================================================
//function : FindCoincidentNodes
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::
FindCoincidentNodes (CORBA::Double                  Tolerance,
                     SMESH::array_of_long_array_out GroupsOfNodes,
                     CORBA::Boolean                 SeparateCornersAndMedium)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedNodeSet nodes; // no input nodes
  findCoincidentNodes( nodes, Tolerance, GroupsOfNodes, SeparateCornersAndMedium );

  TPythonDump() << "coincident_nodes = " << this << ".FindCoincidentNodes( "
                << Tolerance << ", "
                << SeparateCornersAndMedium << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
//function : FindCoincidentNodesOnPart
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::
FindCoincidentNodesOnPart(SMESH::SMESH_IDSource_ptr      theObject,
                          CORBA::Double                  Tolerance,
                          SMESH::array_of_long_array_out GroupsOfNodes,
                          CORBA::Boolean                 SeparateCornersAndMedium)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedNodeSet nodes;
  idSourceToNodeSet( theObject, getMeshDS(), nodes );

  findCoincidentNodes( nodes, Tolerance, GroupsOfNodes, SeparateCornersAndMedium );

  TPythonDump() << "coincident_nodes_on_part = " << this << ".FindCoincidentNodesOnPart( "
                << theObject <<", "
                << Tolerance << ", "
                << SeparateCornersAndMedium << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
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
                             const SMESH::ListOfIDSources&  theExceptSubMeshOrGroups,
                             CORBA::Boolean                 theSeparateCornersAndMedium)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedNodeSet nodes;
  idSourceToNodeSet( theObject, getMeshDS(), nodes );

  for ( CORBA::ULong i = 0; i < theExceptSubMeshOrGroups.length(); ++i )
  {
    SMDS_ElemIteratorPtr nodeIt = myMesh_i->GetElements( theExceptSubMeshOrGroups[i],
                                                         SMESH::NODE );
    while ( nodeIt->more() )
      nodes.erase( cast2Node( nodeIt->next() ));
  }
  findCoincidentNodes( nodes, theTolerance, theGroupsOfNodes, theSeparateCornersAndMedium );

  TPythonDump() << "coincident_nodes_on_part = " << this << ".FindCoincidentNodesOnPartBut( "
                << theObject<<", "
                << theTolerance << ", "
                << theExceptSubMeshOrGroups << ", "
                << theSeparateCornersAndMedium << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
//function : MergeNodes
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes,
                                     const SMESH::ListOfIDSources&     NodesToKeep)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();

  TPythonDump aTPythonDump;
  aTPythonDump << this << ".MergeNodes([";

  TIDSortedNodeSet setOfNodesToKeep;
  for ( CORBA::ULong i = 0; i < NodesToKeep.length(); ++i )
  {
    prepareIdSource( NodesToKeep[i] );
    SMDS_ElemIteratorPtr nodeIt = myMesh_i->GetElements( NodesToKeep[i], SMESH::NODE );
    while ( nodeIt->more() )
      setOfNodesToKeep.insert( setOfNodesToKeep.end(), cast2Node( nodeIt->next() ));
  }

  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  for ( CORBA::ULong i = 0; i < GroupsOfNodes.length(); i++ )
  {
    const SMESH::long_array& aNodeGroup = GroupsOfNodes[ i ];
    aListOfListOfNodes.push_back( list< const SMDS_MeshNode* >() );
    list< const SMDS_MeshNode* >& aListOfNodes = aListOfListOfNodes.back();
    for ( CORBA::ULong j = 0; j < aNodeGroup.length(); j++ )
    {
      CORBA::Long index = aNodeGroup[ j ];
      if ( const SMDS_MeshNode * node = aMesh->FindNode( index ))
      {
        if ( setOfNodesToKeep.count( node ))
          aListOfNodes.push_front( node );
        else
          aListOfNodes.push_back( node );
      }
    }
    if ( aListOfNodes.size() < 2 )
      aListOfListOfNodes.pop_back();

    if ( i > 0 ) aTPythonDump << ", ";
    aTPythonDump << aNodeGroup;
  }

  getEditor().MergeNodes( aListOfListOfNodes );

  aTPythonDump << "], " << NodesToKeep << ")";

  declareMeshModified( /*isReComputeSafe=*/false );

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
//function : FindEqualElements
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::FindEqualElements(SMESH::SMESH_IDSource_ptr      theObject,
                                           SMESH::array_of_long_array_out GroupsOfElementsID)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow(theObject);
  if ( !(!group->_is_nil() && group->GetType() == SMESH::NODE) )
  {
    TIDSortedElemSet elems;
    idSourceToSet( theObject, getMeshDS(), elems, SMDSAbs_All, /*emptyIfIsMesh=*/true);

    ::SMESH_MeshEditor::TListOfListOfElementsID aListOfListOfElementsID;
    getEditor().FindEqualElements( elems, aListOfListOfElementsID );

    GroupsOfElementsID = new SMESH::array_of_long_array;
    GroupsOfElementsID->length( aListOfListOfElementsID.size() );

    ::SMESH_MeshEditor::TListOfListOfElementsID::iterator arraysIt =
        aListOfListOfElementsID.begin();
    for (CORBA::Long j = 0; arraysIt != aListOfListOfElementsID.end(); ++arraysIt, ++j)
    {
      SMESH::long_array& aGroup = (*GroupsOfElementsID)[ j ];
      list<int>&      listOfIDs = *arraysIt;
      aGroup.length( listOfIDs.size() );
      list<int>::iterator idIt = listOfIDs.begin();
      for (int k = 0; idIt != listOfIDs.end(); ++idIt, ++k )
        aGroup[ k ] = *idIt;
    }

    TPythonDump() << "equal_elements = " << this << ".FindEqualElements( "
                  <<theObject<<" )";
  }

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
//function : MergeElements
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeElements(const SMESH::array_of_long_array& GroupsOfElementsID)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump aTPythonDump;
  aTPythonDump << this << ".MergeElements( [";

  ::SMESH_MeshEditor::TListOfListOfElementsID aListOfListOfElementsID;

  for ( CORBA::ULong i = 0; i < GroupsOfElementsID.length(); i++ ) {
    const SMESH::long_array& anElemsIDGroup = GroupsOfElementsID[ i ];
    aListOfListOfElementsID.push_back( list< int >() );
    list< int >& aListOfElemsID = aListOfListOfElementsID.back();
    for ( CORBA::ULong j = 0; j < anElemsIDGroup.length(); j++ ) {
      CORBA::Long id = anElemsIDGroup[ j ];
      aListOfElemsID.push_back( id );
    }
    if ( aListOfElemsID.size() < 2 )
      aListOfListOfElementsID.pop_back();
    if ( i > 0 ) aTPythonDump << ", ";
    aTPythonDump << anElemsIDGroup;
  }

  getEditor().MergeElements(aListOfListOfElementsID);

  declareMeshModified( /*isReComputeSafe=*/true );

  aTPythonDump << "] )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
//function : MergeEqualElements
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeEqualElements()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  getEditor().MergeEqualElements();

  declareMeshModified( /*isReComputeSafe=*/true );

  TPythonDump() << this << ".MergeEqualElements()";

  SMESH_CATCH( SMESH::throwCorbaException );
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData(/*deleteSearchers=*/false);

  const SMDS_MeshNode * node = getMeshDS()->FindNode( NodeID );
  if ( !node )
    return false;

  if ( theNodeSearcher )
    theSearchersDeleter.Set( myMesh ); // remove theNodeSearcher if mesh is other

  if ( myIsPreviewMode ) // make preview data
  {
    // in a preview mesh, make edges linked to a node
    TPreviewMesh& tmpMesh = *getPreviewMesh();
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
  }
  else if ( theNodeSearcher ) // move node and update theNodeSearcher data accordingly
    theNodeSearcher->MoveNode(node, gp_Pnt( x,y,z ));
  else
    getMeshDS()->MoveNode(node, x, y, z);

  if ( !myIsPreviewMode )
  {
    // Update Python script
    TPythonDump() << "isDone = " << this << ".MoveNode( "
                  << NodeID << ", " << TVar(x) << ", " << TVar(y) << ", " << TVar(z) << " )";
    declareMeshModified( /*isReComputeSafe=*/false );
  }

  SMESH_CATCH( SMESH::throwCorbaException );

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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  theSearchersDeleter.Set( myMesh ); // remove theNodeSearcher if mesh is other

  if ( !theNodeSearcher ) {
    theNodeSearcher = SMESH_MeshAlgos::GetNodeSearcher( *getMeshDS() );
  }
  gp_Pnt p( x,y,z );
  if ( const SMDS_MeshNode* node = theNodeSearcher->FindClosestTo( p ))
    return node->GetID();

  SMESH_CATCH( SMESH::throwCorbaException );
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  // We keep theNodeSearcher until any mesh modification:
  // 1) initData() deletes theNodeSearcher at any edition,
  // 2) TSearchersDeleter - at any mesh compute event and mesh change

  initData(/*deleteSearchers=*/false);

  theSearchersDeleter.Set( myMesh ); // remove theNodeSearcher if mesh is other

  int nodeID = theNodeID;
  const SMDS_MeshNode* node = getMeshDS()->FindNode( nodeID );
  if ( !node ) // preview moving node
  {
    if ( !theNodeSearcher ) {
      theNodeSearcher = SMESH_MeshAlgos::GetNodeSearcher( *getMeshDS() );
    }
    gp_Pnt p( x,y,z );
    node = theNodeSearcher->FindClosestTo( p );
  }
  if ( node ) {
    nodeID = node->GetID();
    if ( myIsPreviewMode ) // make preview data
    {
      // in a preview mesh, make edges linked to a node
      TPreviewMesh tmpMesh = *getPreviewMesh();
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
    }
    else if ( theNodeSearcher ) // move node and update theNodeSearcher data accordingly
    {
      theNodeSearcher->MoveNode(node, gp_Pnt( x,y,z ));
    }
    else
    {
      getMeshDS()->MoveNode(node, x, y, z);
    }
  }

  if ( !myIsPreviewMode )
  {
    TPythonDump() << "nodeID = " << this
                  << ".MoveClosestNodeToPoint( "<< x << ", " << y << ", " << z
                  << ", " << nodeID << " )";

    declareMeshModified( /*isReComputeSafe=*/false );
  }

  return nodeID;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::long_array_var res = new SMESH::long_array;
  vector< const SMDS_MeshElement* > foundElems;

  theSearchersDeleter.Set( myMesh );
  if ( !theElementSearcher ) {
    theElementSearcher = SMESH_MeshAlgos::GetElementSearcher( *getMeshDS() );
  }
  theElementSearcher->FindElementsByPoint( gp_Pnt( x,y,z ),
                                           SMDSAbs_ElementType( type ),
                                           foundElems);
  res->length( foundElems.size() );
  for ( size_t i = 0; i < foundElems.size(); ++i )
    res[i] = foundElems[i]->GetID();

  return res._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::long_array_var res = new SMESH::long_array;

  if ( type != SMESH::NODE )
  {
    SMESH::array_of_ElementType_var types = elementIDs->GetTypes();
    if ( types->length() == 1 && // a part contains only nodes or 0D elements
         ( types[0] == SMESH::NODE || types[0] == SMESH::ELEM0D || types[0] == SMESH::BALL) &&
         type != types[0] ) // but search of elements of dim > 0
      return res._retn();
  }
  if ( SMESH::DownCast<SMESH_Mesh_i*>( elementIDs )) // elementIDs is the whole mesh 
    return FindElementsByPoint( x,y,z, type );

  TIDSortedElemSet elements; // elems should live until FindElementsByPoint() finishes

  theSearchersDeleter.Set( myMesh, getPartIOR( elementIDs, type ));
  if ( !theElementSearcher )
  {
    // create a searcher from elementIDs
    SMESH::SMESH_Mesh_var mesh = elementIDs->GetMesh();
    SMESHDS_Mesh* meshDS = SMESH::DownCast<SMESH_Mesh_i*>( mesh )->GetImpl().GetMeshDS();

    if ( !idSourceToSet( elementIDs, meshDS, elements,
                         ( type == SMESH::NODE ? SMDSAbs_All : (SMDSAbs_ElementType) type ),
                         /*emptyIfIsMesh=*/true))
      return res._retn();

    typedef SMDS_SetIterator<const SMDS_MeshElement*, TIDSortedElemSet::const_iterator > TIter;
    SMDS_ElemIteratorPtr elemsIt( new TIter( elements.begin(), elements.end() ));

    theElementSearcher = SMESH_MeshAlgos::GetElementSearcher( *getMeshDS(), elemsIt );
  }

  vector< const SMDS_MeshElement* > foundElems;

  theElementSearcher->FindElementsByPoint( gp_Pnt( x,y,z ),
                                           SMDSAbs_ElementType( type ),
                                           foundElems);
  res->length( foundElems.size() );
  for ( size_t i = 0; i < foundElems.size(); ++i )
    res[i] = foundElems[i]->GetID();

  return res._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
//function : GetPointState
//purpose  : Return point state in a closed 2D mesh in terms of TopAbs_State enumeration.
//           TopAbs_UNKNOWN state means that either mesh is wrong or the analysis fails.
//=======================================================================

CORBA::Short SMESH_MeshEditor_i::GetPointState(CORBA::Double x,
                                               CORBA::Double y,
                                               CORBA::Double z)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  theSearchersDeleter.Set( myMesh );
  if ( !theElementSearcher ) {
    theElementSearcher = SMESH_MeshAlgos::GetElementSearcher( *getMeshDS() );
  }
  return CORBA::Short( theElementSearcher->GetPointState( gp_Pnt( x,y,z )));

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
    RETCASE( SEW_INTERNAL_ERROR );
  }
  return SMESH::SMESH_MeshEditor::SEW_OK;
}

//=======================================================================
/*!
 * Returns groups of FreeBorder's coincident within the given tolerance.
 * If the tolerance <= 0.0 then one tenth of an average size of elements adjacent
 * to free borders being compared is used.
 */
//=======================================================================

SMESH::CoincidentFreeBorders*
SMESH_MeshEditor_i::FindCoincidentFreeBorders(CORBA::Double tolerance)
{
  SMESH::CoincidentFreeBorders_var aCFB = new SMESH::CoincidentFreeBorders;

  SMESH_TRY;

  SMESH_MeshAlgos::CoincidentFreeBorders cfb;
  SMESH_MeshAlgos::FindCoincidentFreeBorders( *getMeshDS(), tolerance, cfb );

  // copy free borders
  aCFB->borders.length( cfb._borders.size() );
  for ( size_t i = 0; i < cfb._borders.size(); ++i )
  {
    SMESH_MeshAlgos::TFreeBorder& nodes = cfb._borders[i];
    SMESH::FreeBorder&             aBRD = aCFB->borders[i];
    aBRD.nodeIDs.length( nodes.size() );
    for ( size_t iN = 0; iN < nodes.size(); ++iN )
      aBRD.nodeIDs[ iN ] = nodes[ iN ]->GetID();
  }

  // copy coincident parts
  aCFB->coincidentGroups.length( cfb._coincidentGroups.size() );
  for ( size_t i = 0; i < cfb._coincidentGroups.size(); ++i )
  {
    SMESH_MeshAlgos::TCoincidentGroup& grp = cfb._coincidentGroups[i];
    SMESH::FreeBordersGroup&          aGRP = aCFB->coincidentGroups[i];
    aGRP.length( grp.size() );
    for ( size_t iP = 0; iP < grp.size(); ++iP )
    {
      SMESH_MeshAlgos::TFreeBorderPart& part = grp[ iP ];
      SMESH::FreeBorderPart&           aPART = aGRP[ iP ];
      aPART.border   = part._border;
      aPART.node1    = part._node1;
      aPART.node2    = part._node2;
      aPART.nodeLast = part._nodeLast;
    }
  }
  SMESH_CATCH( SMESH::doNothing );

  TPythonDump() << "CoincidentFreeBorders = "
                << this << ".FindCoincidentFreeBorders( " << tolerance << " )";

  return aCFB._retn();
}

//=======================================================================
/*!
 * Sew FreeBorder's of each group
 */
//=======================================================================

CORBA::Short SMESH_MeshEditor_i::
SewCoincidentFreeBorders(const SMESH::CoincidentFreeBorders& freeBorders,
                         CORBA::Boolean                      createPolygons,
                         CORBA::Boolean                      createPolyhedra)
  throw (SALOME::SALOME_Exception)
{
  CORBA::Short nbSewed = 0;

  SMESH_MeshAlgos::TFreeBorderVec groups;
  SMESH_MeshAlgos::TFreeBorder    borderNodes; // triples of nodes for every FreeBorderPart

  // check the input and collect nodes
  for ( CORBA::ULong i = 0; i < freeBorders.coincidentGroups.length(); ++i )
  {
    borderNodes.clear();
    const SMESH::FreeBordersGroup& aGRP = freeBorders.coincidentGroups[ i ];
    for ( CORBA::ULong iP = 0; iP < aGRP.length(); ++iP )
    {
      const SMESH::FreeBorderPart& aPART = aGRP[ iP ];
      if ( aPART.border < 0 || aPART.border >= (int) freeBorders.borders.length() )
        THROW_SALOME_CORBA_EXCEPTION("Invalid FreeBorderPart::border index", SALOME::BAD_PARAM);

      const SMESH::FreeBorder& aBRD = freeBorders.borders[ aPART.border ];

      if ( aPART.node1 < 0 || aPART.node1 > (int) aBRD.nodeIDs.length() )
        THROW_SALOME_CORBA_EXCEPTION("Invalid FreeBorderPart::node1", SALOME::BAD_PARAM);
      if ( aPART.node2 < 0 || aPART.node2 > (int) aBRD.nodeIDs.length() )
        THROW_SALOME_CORBA_EXCEPTION("Invalid FreeBorderPart::node2", SALOME::BAD_PARAM);
      if ( aPART.nodeLast < 0 || aPART.nodeLast > (int) aBRD.nodeIDs.length() )
        THROW_SALOME_CORBA_EXCEPTION("Invalid FreeBorderPart::nodeLast", SALOME::BAD_PARAM);

      // do not keep these nodes for further sewing as nodes can be removed by the sewing
      const SMDS_MeshNode* n1 = getMeshDS()->FindNode( aBRD.nodeIDs[ aPART.node1    ]);
      const SMDS_MeshNode* n2 = getMeshDS()->FindNode( aBRD.nodeIDs[ aPART.node2    ]);
      const SMDS_MeshNode* n3 = getMeshDS()->FindNode( aBRD.nodeIDs[ aPART.nodeLast ]);
      if ( !n1)
        THROW_SALOME_CORBA_EXCEPTION("Nonexistent FreeBorderPart::node1", SALOME::BAD_PARAM);
      if ( !n2 )
        THROW_SALOME_CORBA_EXCEPTION("Nonexistent FreeBorderPart::node2", SALOME::BAD_PARAM);
      if ( !n3 )
        THROW_SALOME_CORBA_EXCEPTION("Nonexistent FreeBorderPart::nodeLast", SALOME::BAD_PARAM);

      borderNodes.push_back( n1 );
      borderNodes.push_back( n2 );
      borderNodes.push_back( n3 );
    }
    groups.push_back( borderNodes );
  }

  // SewFreeBorder() can merge nodes, thus nodes stored in 'groups' can become dead;
  // to get nodes that replace other nodes during merge we create 0D elements
  // on each node and MergeNodes() will replace underlying nodes of 0D elements by
  // new ones.

  vector< const SMDS_MeshElement* > tmp0Delems;
  for ( size_t i = 0; i < groups.size(); ++i )
  {
    SMESH_MeshAlgos::TFreeBorder& nodes = groups[i];
    for ( size_t iN = 0; iN < nodes.size(); ++iN )
    {
      SMDS_ElemIteratorPtr it0D = nodes[iN]->GetInverseElementIterator(SMDSAbs_0DElement);
      if ( it0D->more() )
        tmp0Delems.push_back( it0D->next() );
      else
        tmp0Delems.push_back( getMeshDS()->Add0DElement( nodes[iN] ));
    }
  }

  // cout << endl << "INIT" << endl;
  // for ( size_t i = 0; i < tmp0Delems.size(); ++i )
  // {
  //   cout << i << " ";
  //   if ( i % 3 == 0 ) cout << "^ ";
  //   tmp0Delems[i]->GetNode(0)->Print( cout );
  // }

  SMESH_TRY;

  ::SMESH_MeshEditor::Sew_Error res, ok = ::SMESH_MeshEditor::SEW_OK;
  int i0D = 0;
  for ( size_t i = 0; i < groups.size(); ++i )
  {
    bool isBordToBord = true;
    bool   groupSewed = false;
    SMESH_MeshAlgos::TFreeBorder& nodes = groups[i];
    for ( size_t iN = 3; iN+2 < nodes.size(); iN += 3 )
    {
      const SMDS_MeshNode* n0 = tmp0Delems[ i0D + 0 ]->GetNode( 0 );
      const SMDS_MeshNode* n1 = tmp0Delems[ i0D + 1 ]->GetNode( 0 );
      const SMDS_MeshNode* n2 = tmp0Delems[ i0D + 2 ]->GetNode( 0 );

      const SMDS_MeshNode* n3 = tmp0Delems[ i0D + 0 + iN ]->GetNode( 0 );
      const SMDS_MeshNode* n4 = tmp0Delems[ i0D + 1 + iN ]->GetNode( 0 );
      const SMDS_MeshNode* n5 = tmp0Delems[ i0D + 2 + iN ]->GetNode( 0 );

      if ( !n0 || !n1 || !n2 || !n3 || !n4 || !n5 )
        continue;

      // TIDSortedElemSet emptySet, avoidSet;
      // if ( !SMESH_MeshAlgos::FindFaceInSet( n0, n1, emptySet, avoidSet))
      // {
      //   cout << "WRONG 2nd 1" << endl;
      //   n0->Print( cout );
      //   n1->Print( cout );
      // }
      // if ( !SMESH_MeshAlgos::FindFaceInSet( n3, n4, emptySet, avoidSet))
      // {
      //   cout << "WRONG 2nd 2" << endl;
      //   n3->Print( cout );
      //   n4->Print( cout );
      // }

      if ( !isBordToBord )
      {
        n1 = n2; // at border-to-side sewing only last side node (n1) is needed
        n2 = 0;  //  and n2 is not used
      }
      // 1st border moves to 2nd
      res = getEditor().SewFreeBorder( n3, n4, n5 ,// 1st
                                       n0 ,n1 ,n2 ,// 2nd
                                       /*2ndIsFreeBorder=*/ isBordToBord,
                                       createPolygons, createPolyhedra);
      groupSewed = ( res == ok );

      isBordToBord = false;
      // cout << endl << "SEWED GROUP " << i << " PART " << iN / 3 << endl;
      // for ( size_t t = 0; t < tmp0Delems.size(); ++t )
      // {
      //   cout << t << " ";
      //   if ( t % 3 == 0 ) cout << "^ ";
      //   tmp0Delems[t]->GetNode(0)->Print( cout );
      // }
    }
    i0D += nodes.size();
    nbSewed += groupSewed;
  }

  TPythonDump() << "nbSewed = " << this << ".SewCoincidentFreeBorders( "
                << freeBorders     << ", "
                << createPolygons  << ", "
                << createPolyhedra << " )";

  SMESH_CATCH( SMESH::doNothing );

  declareMeshModified( /*isReComputeSafe=*/false );

  // remove tmp 0D elements
  SMESH_TRY;
  set< const SMDS_MeshElement* > removed0D;
  for ( size_t i = 0; i < tmp0Delems.size(); ++i )
  {
    if ( removed0D.insert( tmp0Delems[i] ).second )
      getMeshDS()->RemoveFreeElement( tmp0Delems[i], /*sm=*/0, /*fromGroups=*/false );
  }
  SMESH_CATCH( SMESH::throwCorbaException );

  return nbSewed;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();

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

  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( getEditor().SewFreeBorder (aBorderFirstNode,
                                          aBorderSecondNode,
                                          aBorderLastNode,
                                          aSide2FirstNode,
                                          aSide2SecondNode,
                                          aSide2ThirdNode,
                                          true,
                                          CreatePolygons,
                                          CreatePolyedrs) );


  declareMeshModified( /*isReComputeSafe=*/false );
  return error;

  SMESH_CATCH( SMESH::throwCorbaException );
  return SMESH::SMESH_MeshEditor::Sew_Error(0);
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();

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

  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( getEditor().SewFreeBorder (aBorderFirstNode,
                                          aBorderSecondNode,
                                          aBorderLastNode,
                                          aSide2FirstNode,
                                          aSide2SecondNode,
                                          aSide2ThirdNode,
                                          true,
                                          false, false) );

  declareMeshModified( /*isReComputeSafe=*/false );
  return error;

  SMESH_CATCH( SMESH::throwCorbaException );
  return SMESH::SMESH_MeshEditor::Sew_Error(0);
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();

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

  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( getEditor().SewFreeBorder (aBorderFirstNode,
                                          aBorderSecondNode,
                                          aBorderLastNode,
                                          aSide2FirstNode,
                                          aSide2SecondNode,
                                          aSide2ThirdNode,
                                          false,
                                          CreatePolygons,
                                          CreatePolyedrs) );

  declareMeshModified( /*isReComputeSafe=*/false );
  return error;

  SMESH_CATCH( SMESH::throwCorbaException );
  return SMESH::SMESH_MeshEditor::Sew_Error(0);
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMesh = getMeshDS();

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

  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( getEditor().SewSideElements (aSide1Elems, aSide2Elems,
                                         aFirstNode1ToMerge,
                                         aFirstNode2ToMerge,
                                         aSecondNode1ToMerge,
                                         aSecondNode2ToMerge));

  declareMeshModified( /*isReComputeSafe=*/false );
  return error;

  SMESH_CATCH( SMESH::throwCorbaException );
  return SMESH::SMESH_MeshEditor::Sew_Error(0);
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  const SMDS_MeshElement* elem = getMeshDS()->FindElement(ide);
  if(!elem) return false;

  int nbn = newIDs.length();
  int i=0;
  vector<const SMDS_MeshNode*> aNodes(nbn);
  int nbn1=-1;
  for(; i<nbn; i++) {
    const SMDS_MeshNode* aNode = getMeshDS()->FindNode(newIDs[i]);
    if(aNode) {
      nbn1++;
      aNodes[nbn1] = aNode;
    }
  }
  TPythonDump() << "isDone = " << this << ".ChangeElemNodes( "
                << ide << ", " << newIDs << " )";

  MESSAGE("ChangeElementNodes");
  bool res = getMeshDS()->ChangeElementNodes( elem, & aNodes[0], nbn1+1 );

  declareMeshModified( /*isReComputeSafe=*/ !res );

  return res;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//=======================================================================
/*!
 * \brief Makes a part of the mesh quadratic or bi-quadratic
 */
//=======================================================================

void SMESH_MeshEditor_i::convertToQuadratic(CORBA::Boolean            theForce3d,
                                            CORBA::Boolean            theToBiQuad,
                                            SMESH::SMESH_IDSource_ptr theObject)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TIDSortedElemSet elems;
  bool elemsOK;
  if ( !( elemsOK = CORBA::is_nil( theObject )))
  {
    elemsOK =  idSourceToSet( theObject, getMeshDS(), elems,
                              SMDSAbs_All, /*emptyIfIsMesh=*/true );
  }
  if ( elemsOK )
  {
    if ( !elems.empty() && (*elems.begin())->GetType() == SMDSAbs_Node )
      THROW_SALOME_CORBA_EXCEPTION("Group of nodes is not allowed", SALOME::BAD_PARAM);

    if ( elems.empty() ) getEditor().ConvertToQuadratic(theForce3d, theToBiQuad);
    else                 getEditor().ConvertToQuadratic(theForce3d, elems, theToBiQuad);

    declareMeshModified( /*isReComputeSafe=*/false );
  }

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
//function : ConvertFromQuadratic
//purpose  :
//=======================================================================

CORBA::Boolean SMESH_MeshEditor_i::ConvertFromQuadratic()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  CORBA::Boolean isDone = getEditor().ConvertFromQuadratic();
  TPythonDump() << this << ".ConvertFromQuadratic()";
  declareMeshModified( /*isReComputeSafe=*/!isDone );
  return isDone;

  SMESH_CATCH( SMESH::throwCorbaException );
  return false;
}

//=======================================================================
//function : ConvertToQuadratic
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ConvertToQuadratic(CORBA::Boolean theForce3d)
  throw (SALOME::SALOME_Exception)
{
  convertToQuadratic( theForce3d, false );
  TPythonDump() << this << ".ConvertToQuadratic("<<theForce3d<<")";
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
  convertToQuadratic( theForce3d, false, theObject );
  TPythonDump() << this << ".ConvertToQuadraticObject("<<theForce3d<<", "<<theObject<<")";
}

//================================================================================
/*!
 * \brief Makes a part of the mesh bi-quadratic
 */
//================================================================================

void SMESH_MeshEditor_i::ConvertToBiQuadratic(CORBA::Boolean            theForce3d,
                                              SMESH::SMESH_IDSource_ptr theObject)
  throw (SALOME::SALOME_Exception)
{
  convertToQuadratic( theForce3d, true, theObject );
  TPythonDump() << this << ".ConvertToBiQuadratic("<<theForce3d<<", "<<theObject<<")";
}

//================================================================================
/*!
 * \brief Makes a part of the mesh linear
 */
//================================================================================

void SMESH_MeshEditor_i::ConvertFromQuadraticObject(SMESH::SMESH_IDSource_ptr theObject)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  TPythonDump pyDump;

  TIDSortedElemSet elems;
  if ( idSourceToSet( theObject, getMeshDS(), elems, SMDSAbs_All, /*emptyIfIsMesh=*/true ))
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
      getEditor().ConvertFromQuadratic(elems);
    }
  }
  declareMeshModified( /*isReComputeSafe=*/false );

  pyDump << this << ".ConvertFromQuadraticObject( "<<theObject<<" )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=======================================================================
//function : makeMesh
//purpose  : create a named imported mesh
//=======================================================================

SMESH::SMESH_Mesh_ptr SMESH_MeshEditor_i::makeMesh(const char* theMeshName)
{
  SMESH_Gen_i*              gen = SMESH_Gen_i::GetSMESHGen();
  SMESH::SMESH_Mesh_var    mesh = gen->CreateEmptyMesh();
  SALOMEDS::Study_var     study = gen->GetCurrentStudy();
  SALOMEDS::SObject_wrap meshSO = gen->ObjectToSObject( study, mesh );
  gen->SetName( meshSO, theMeshName, "Mesh" );
  gen->SetPixMap( meshSO, "ICON_SMESH_TREE_MESH_IMPORTED");

  return mesh._retn();
}

//=======================================================================
//function : dumpGroupsList
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::dumpGroupsList(TPythonDump &               theDumpPython,
                                        const SMESH::ListOfGroups * theGroupList)
{
  bool isDumpGroupList = ( theGroupList && theGroupList->length() > 0 );
  if ( isDumpGroupList )
    theDumpPython << theGroupList << " = ";
}

//================================================================================
/*!
  \brief Generates the unique group name.
  \param thePrefix name prefix
  \return unique name
*/
//================================================================================

std::string SMESH_MeshEditor_i::generateGroupName(const std::string& thePrefix)
{
  SMESH::ListOfGroups_var groups = myMesh_i->GetGroups();
  set<std::string> groupNames;

  // Get existing group names
  for (int i = 0, nbGroups = groups->length(); i < nbGroups; i++ ) {
    SMESH::SMESH_GroupBase_var aGroup = groups[i];
    if (CORBA::is_nil(aGroup))
      continue;

    CORBA::String_var name = aGroup->GetName();
    groupNames.insert( name.in() );
  }

  // Find new name
  std::string name = thePrefix;
  int index = 0;

  while (!groupNames.insert(name).second)
    name = SMESH_Comment( thePrefix ) << "_" << index++;

  return name;
}

//================================================================================
/*!
 * \brief Prepare SMESH_IDSource for work
 */
//================================================================================

void SMESH_MeshEditor_i::prepareIdSource(SMESH::SMESH_IDSource_ptr theObject)
{
  if ( SMESH::Filter_i* filter = SMESH::DownCast<SMESH::Filter_i*>( theObject ))
  {
    SMESH::SMESH_Mesh_var mesh = myMesh_i->_this();
    filter->SetMesh( mesh );
  }
}
//================================================================================
/*!
 * \brief Retrieve elements of given type from SMESH_IDSource
 */
//================================================================================

bool SMESH_MeshEditor_i::idSourceToSet(SMESH::SMESH_IDSource_ptr  theIDSource,
                                       const SMESHDS_Mesh*        theMeshDS,
                                       TIDSortedElemSet&          theElemSet,
                                       const SMDSAbs_ElementType  theType,
                                       const bool                 emptyIfIsMesh,
                                       IDSource_Error*            error)

{
  if ( error ) *error = IDSource_OK;

  if ( CORBA::is_nil( theIDSource ))
  {
    if ( error ) *error = IDSource_INVALID;
    return false;
  }
  if ( emptyIfIsMesh && SMESH::DownCast<SMESH_Mesh_i*>( theIDSource ))
  {
    if ( error && getMeshDS()->GetMeshInfo().NbElements( theType ) == 0 )
      *error = IDSource_EMPTY;
    return true;
  }
  prepareIdSource( theIDSource );
  SMESH::long_array_var anIDs = theIDSource->GetIDs();
  if ( anIDs->length() == 0 )
  {
    if ( error ) *error = IDSource_EMPTY;
    return false;
  }
  SMESH::array_of_ElementType_var types = theIDSource->GetTypes();
  if ( types->length() == 1 && types[0] == SMESH::NODE ) // group of nodes
  {
    if ( theType == SMDSAbs_All || theType == SMDSAbs_Node )
    {
      arrayToSet( anIDs, getMeshDS(), theElemSet, SMDSAbs_Node );
    }
    else
    {
      if ( error ) *error = IDSource_INVALID;
      return false;
    }
  }
  else
  {
    arrayToSet( anIDs, getMeshDS(), theElemSet, theType);
    if ( bool(anIDs->length()) != bool(theElemSet.size()))
    {
      if ( error ) *error = IDSource_INVALID;
      return false;
    }
  }
  return true;
}

//================================================================================
/*!
 * \brief Duplicates given elements, i.e. creates new elements based on the
 *        same nodes as the given ones.
 * \param theElements - container of elements to duplicate.
 * \param theGroupName - a name of group to contain the generated elements.
 *                    If a group with such a name already exists, the new elements
 *                    are added to the existng group, else a new group is created.
 *                    If \a theGroupName is empty, new elements are not added 
 *                    in any group.
 * \return a group where the new elements are added. NULL if theGroupName == "".
 * \sa DoubleNode()
 */
//================================================================================

SMESH::SMESH_Group_ptr
SMESH_MeshEditor_i::DoubleElements(SMESH::SMESH_IDSource_ptr theElements,
                                   const char*               theGroupName)
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var newGroup;

  SMESH_TRY;
  initData();

  TPythonDump pyDump;

  TIDSortedElemSet elems;
  if ( idSourceToSet( theElements, getMeshDS(), elems, SMDSAbs_All, /*emptyIfIsMesh=*/true))
  {
    getEditor().DoubleElements( elems );

    if ( strlen( theGroupName ) && !getEditor().GetLastCreatedElems().IsEmpty() )
    {
      // group type
      SMESH::ElementType type =
        SMESH::ElementType( getEditor().GetLastCreatedElems().Value(1)->GetType() );
      // find existing group
      SMESH::ListOfGroups_var groups = myMesh_i->GetGroups();
      for ( size_t i = 0; i < groups->length(); ++i )
        if ( groups[i]->GetType() == type )
        {
          CORBA::String_var name = groups[i]->GetName();
          if ( strcmp( name, theGroupName ) == 0 ) {
            newGroup = SMESH::SMESH_Group::_narrow( groups[i] );
            break;
          }
        }
      // create a new group
      if ( newGroup->_is_nil() )
        newGroup = myMesh_i->CreateGroup( type, theGroupName );
      // fill newGroup
      if ( SMESH_Group_i* group_i = SMESH::DownCast< SMESH_Group_i* >( newGroup ))
      {
        SMESHDS_Group* groupDS = static_cast< SMESHDS_Group* >( group_i->GetGroupDS() );
        const SMESH_SequenceOfElemPtr& aSeq = getEditor().GetLastCreatedElems();
        for ( int i = 1; i <= aSeq.Length(); i++ )
          groupDS->SMDSGroup().Add( aSeq(i) );
      }
    }
  }
  // python dump
  if ( !newGroup->_is_nil() )
    pyDump << newGroup << " = ";
  pyDump << this << ".DoubleElements( "
         << theElements << ", " << "'" << theGroupName <<"')";

  SMESH_CATCH( SMESH::throwCorbaException );

  return newGroup._retn();
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  list< int > aListOfNodes;
  int i, n;
  for ( i = 0, n = theNodes.length(); i < n; i++ )
    aListOfNodes.push_back( theNodes[ i ] );

  list< int > aListOfElems;
  for ( i = 0, n = theModifiedElems.length(); i < n; i++ )
    aListOfElems.push_back( theModifiedElems[ i ] );

  bool aResult = getEditor().DoubleNodes( aListOfNodes, aListOfElems );

  declareMeshModified( /*isReComputeSafe=*/ !aResult );

  // Update Python script
  TPythonDump() << this << ".DoubleNodes( " << theNodes << ", "<< theModifiedElems << " )";

  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::long_array_var aNodes = new SMESH::long_array;
  aNodes->length( 1 );
  aNodes[ 0 ] = theNodeId;

  TPythonDump pyDump; // suppress dump by the next line

  CORBA::Boolean done = DoubleNodes( aNodes, theModifiedElems );

  pyDump << this << ".DoubleNode( " << theNodeId << ", " << theModifiedElems << " )";

  return done;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
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

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
 * \brief Creates a hole in a mesh by doubling the nodes of some particular elements.
 * Works as DoubleNodeGroup(), but returns a new group with newly created nodes.
 * \param theNodes - group of nodes to be doubled.
 * \param theModifiedElems - group of elements to be updated.
 * \return a new group with newly created nodes
 * \sa DoubleNodeGroup()
 */
//================================================================================

SMESH::SMESH_Group_ptr
SMESH_MeshEditor_i::DoubleNodeGroupNew( SMESH::SMESH_GroupBase_ptr theNodes,
                                        SMESH::SMESH_GroupBase_ptr theModifiedElems )
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
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
      std::string anUnindexedName (theNodes->GetName());
      std::string aNewName = generateGroupName(anUnindexedName + "_double");
      aNewGroup = myMesh_i->CreateGroup(SMESH::NODE, aNewName.c_str());
      aNewGroup->Add(anIds);
      pyDump << aNewGroup << " = ";
    }
  }

  pyDump << this << ".DoubleNodeGroupNew( " << theNodes << ", "
         << theModifiedElems << " )";

  return aNewGroup._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

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

  bool aResult = getEditor().DoubleNodes( aNodes, anElems );

  declareMeshModified( /*isReComputeSafe=*/false );

  TPythonDump() << this << ".DoubleNodeGroups( " << theNodes << ", " << theModifiedElems << " )";

  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aNewGroup;

  TPythonDump pyDump; // suppress dump by the next line

  bool aResult = DoubleNodeGroups( theNodes, theModifiedElems );

  if ( aResult )
  {
    // Create group with newly created nodes
    SMESH::long_array_var anIds = GetLastCreatedNodes();
    if (anIds->length() > 0) {
      std::string anUnindexedName (theNodes[0]->GetName());
      std::string aNewName = generateGroupName(anUnindexedName + "_double");
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  arrayToSet(theElems, aMeshDS, anElems, SMDSAbs_All);
  arrayToSet(theNodesNot, aMeshDS, aNodes, SMDSAbs_Node);
  arrayToSet(theAffectedElems, aMeshDS, anAffected, SMDSAbs_All);

  bool aResult = getEditor().DoubleNodes( anElems, aNodes, anAffected );

  // Update Python script
  TPythonDump() << this << ".DoubleNodeElem( " << theElems << ", "
                << theNodesNot << ", " << theAffectedElems << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();


  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes;
  arrayToSet(theElems, aMeshDS, anElems, SMDSAbs_All);
  arrayToSet(theNodesNot, aMeshDS, aNodes, SMDSAbs_Node);

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theShape );
  bool aResult = getEditor().DoubleNodesInRegion( anElems, aNodes, aShape );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemInRegion( " << theElems << ", "
                << theNodesNot << ", " << theShape << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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

CORBA::Boolean
SMESH_MeshEditor_i::DoubleNodeElemGroup(SMESH::SMESH_GroupBase_ptr theElems,
                                        SMESH::SMESH_GroupBase_ptr theNodesNot,
                                        SMESH::SMESH_GroupBase_ptr theAffectedElems)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  if ( CORBA::is_nil( theElems ) && theElems->GetType() == SMESH::NODE )
    return false;

  initData();


  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  idSourceToSet( theElems, aMeshDS, anElems, SMDSAbs_All );
  idSourceToSet( theNodesNot, aMeshDS, aNodes, SMDSAbs_Node );
  idSourceToSet( theAffectedElems, aMeshDS, anAffected, SMDSAbs_All );

  bool aResult = getEditor().DoubleNodes( anElems, aNodes, anAffected );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroup( " << theElems << ", "
                << theNodesNot << ", " << theAffectedElems << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
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
//================================================================================

SMESH::SMESH_Group_ptr
SMESH_MeshEditor_i::DoubleNodeElemGroupNew(SMESH::SMESH_GroupBase_ptr theElems,
                                           SMESH::SMESH_GroupBase_ptr theNodesNot,
                                           SMESH::SMESH_GroupBase_ptr theAffectedElems)
  throw (SALOME::SALOME_Exception)
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

//================================================================================
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
//================================================================================

SMESH::ListOfGroups*
SMESH_MeshEditor_i::DoubleNodeElemGroup2New(SMESH::SMESH_GroupBase_ptr theElems,
                                            SMESH::SMESH_GroupBase_ptr theNodesNot,
                                            SMESH::SMESH_GroupBase_ptr theAffectedElems,
                                            CORBA::Boolean             theElemGroupNeeded,
                                            CORBA::Boolean             theNodeGroupNeeded)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::SMESH_Group_var aNewElemGroup, aNewNodeGroup;
  SMESH::ListOfGroups_var aTwoGroups = new SMESH::ListOfGroups();
  aTwoGroups->length( 2 );

  if ( CORBA::is_nil( theElems ) && theElems->GetType() == SMESH::NODE )
    return aTwoGroups._retn();

  initData();


  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  idSourceToSet( theElems, aMeshDS, anElems, SMDSAbs_All );
  idSourceToSet( theNodesNot, aMeshDS, aNodes, SMDSAbs_Node );
  idSourceToSet( theAffectedElems, aMeshDS, anAffected, SMDSAbs_All );


  bool aResult = getEditor().DoubleNodes( anElems, aNodes, anAffected );

  declareMeshModified( /*isReComputeSafe=*/ !aResult );

  TPythonDump pyDump;

  if ( aResult )
  {
    // Create group with newly created elements
    CORBA::String_var elemGroupName = theElems->GetName();
    std::string aNewName = generateGroupName( std::string(elemGroupName.in()) + "_double");
    if ( !getEditor().GetLastCreatedElems().IsEmpty() && theElemGroupNeeded )
    {
      SMESH::long_array_var anIds = GetLastCreatedElems();
      SMESH::ElementType aGroupType = myMesh_i->GetElementType(anIds[0], true);
      aNewElemGroup = myMesh_i->CreateGroup(aGroupType, aNewName.c_str());
      aNewElemGroup->Add(anIds);
    }
    if ( !getEditor().GetLastCreatedNodes().IsEmpty() && theNodeGroupNeeded )
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

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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

CORBA::Boolean
SMESH_MeshEditor_i::DoubleNodeElemGroupInRegion(SMESH::SMESH_GroupBase_ptr theElems,
                                                SMESH::SMESH_GroupBase_ptr theNodesNot,
                                                GEOM::GEOM_Object_ptr      theShape )
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  if ( CORBA::is_nil( theElems ) && theElems->GetType() == SMESH::NODE )
    return false;

  initData();


  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  idSourceToSet( theElems, aMeshDS, anElems, SMDSAbs_All );
  idSourceToSet( theNodesNot, aMeshDS, aNodes, SMDSAbs_Node );

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theShape );
  bool aResult = getEditor().DoubleNodesInRegion( anElems, aNodes, aShape );


  declareMeshModified( /*isReComputeSafe=*/ !aResult );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroupInRegion( " << theElems << ", "
                << theNodesNot << ", " << theShape << " )";
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
 * \brief Re-load elements from a list of groups into a TIDSortedElemSet
 *  \param [in] theGrpList - groups
 *  \param [in] theMeshDS -  mesh
 *  \param [out] theElemSet - set of elements
 *  \param [in] theIsNodeGrp - is \a theGrpList includes goups of nodes
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

//================================================================================
/*!
  \brief Creates a hole in a mesh by doubling the nodes of some particular elements.
  This method provided for convenience works as DoubleNodes() described above.
  \param theElems - list of groups of elements (edges or faces) to be replicated
  \param theNodesNot - list of groups of nodes not to replicated
  \param theAffectedElems - group of elements to which the replicated nodes
  should be associated to.
  \return TRUE if operation has been completed successfully, FALSE otherwise
  \sa DoubleNodeGroup(), DoubleNodes(), DoubleNodeElemGroupsNew()
*/
//================================================================================

CORBA::Boolean
SMESH_MeshEditor_i::DoubleNodeElemGroups(const SMESH::ListOfGroups& theElems,
                                         const SMESH::ListOfGroups& theNodesNot,
                                         const SMESH::ListOfGroups& theAffectedElems)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();


  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  listOfGroupToSet(theElems, aMeshDS, anElems, false );
  listOfGroupToSet(theNodesNot, aMeshDS, aNodes, true );
  listOfGroupToSet(theAffectedElems, aMeshDS, anAffected, false );

  bool aResult = getEditor().DoubleNodes( anElems, aNodes, anAffected );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroups( " << &theElems << ", "
                << &theNodesNot << ", " << &theAffectedElems << " )";

  declareMeshModified( /*isReComputeSafe=*/false );
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
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

SMESH::ListOfGroups*
SMESH_MeshEditor_i::DoubleNodeElemGroups2New(const SMESH::ListOfGroups& theElems,
                                             const SMESH::ListOfGroups& theNodesNot,
                                             const SMESH::ListOfGroups& theAffectedElems,
                                             CORBA::Boolean             theElemGroupNeeded,
                                             CORBA::Boolean             theNodeGroupNeeded)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  SMESH::SMESH_Group_var aNewElemGroup, aNewNodeGroup;
  SMESH::ListOfGroups_var aTwoGroups = new SMESH::ListOfGroups();
  aTwoGroups->length( 2 );
  
  initData();


  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes, anAffected;
  listOfGroupToSet(theElems, aMeshDS, anElems, false );
  listOfGroupToSet(theNodesNot, aMeshDS, aNodes, true );
  listOfGroupToSet(theAffectedElems, aMeshDS, anAffected, false );

  bool aResult = getEditor().DoubleNodes( anElems, aNodes, anAffected );

  declareMeshModified( /*isReComputeSafe=*/ !aResult );

  TPythonDump pyDump;
  if ( aResult )
  {
    // Create group with newly created elements
    CORBA::String_var elemGroupName = theElems[0]->GetName();
    std::string aNewName = generateGroupName( std::string(elemGroupName.in()) + "_double");
    if ( !getEditor().GetLastCreatedElems().IsEmpty() && theElemGroupNeeded )
    {
      SMESH::long_array_var anIds = GetLastCreatedElems();
      SMESH::ElementType aGroupType = myMesh_i->GetElementType(anIds[0], true);
      aNewElemGroup = myMesh_i->CreateGroup(aGroupType, aNewName.c_str());
      aNewElemGroup->Add(anIds);
    }
    if ( !getEditor().GetLastCreatedNodes().IsEmpty() && theNodeGroupNeeded )
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

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();


  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes;
  listOfGroupToSet(theElems, aMeshDS, anElems,false );
  listOfGroupToSet(theNodesNot, aMeshDS, aNodes, true );

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theShape );
  bool aResult = getEditor().DoubleNodesInRegion( anElems, aNodes, aShape );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodeElemGroupsInRegion( " << &theElems << ", "
                << &theNodesNot << ", " << theShape << " )";

  declareMeshModified( /*isReComputeSafe=*/ !aResult );
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
  \brief Identify the elements that will be affected by node duplication (actual
         duplication is not performed.
  This method is the first step of DoubleNodeElemGroupsInRegion.
  \param theElems - list of groups of elements (edges or faces) to be replicated
  \param theNodesNot - list of groups of nodes not to replicated
  \param theShape - shape to detect affected elements (element which geometric center
         located on or inside shape).
         The replicated nodes should be associated to affected elements.
  \return groups of affected elements
  \sa DoubleNodeElemGroupsInRegion()
*/
//================================================================================
SMESH::ListOfGroups*
SMESH_MeshEditor_i::AffectedElemGroupsInRegion( const SMESH::ListOfGroups& theElems,
                                                const SMESH::ListOfGroups& theNodesNot,
                                                GEOM::GEOM_Object_ptr      theShape )
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  MESSAGE("AffectedElemGroupsInRegion");
  SMESH::ListOfGroups_var aListOfGroups = new SMESH::ListOfGroups();
  bool isEdgeGroup = false;
  bool isFaceGroup = false;
  bool isVolumeGroup = false;
  SMESH::SMESH_Group_var aNewEdgeGroup = myMesh_i->CreateGroup(SMESH::EDGE, "affectedEdges");
  SMESH::SMESH_Group_var aNewFaceGroup = myMesh_i->CreateGroup(SMESH::FACE, "affectedFaces");
  SMESH::SMESH_Group_var aNewVolumeGroup = myMesh_i->CreateGroup(SMESH::VOLUME, "affectedVolumes");

  initData();

  ::SMESH_MeshEditor aMeshEditor(myMesh);

  SMESHDS_Mesh* aMeshDS = getMeshDS();
  TIDSortedElemSet anElems, aNodes;
  listOfGroupToSet(theElems, aMeshDS, anElems, false);
  listOfGroupToSet(theNodesNot, aMeshDS, aNodes, true);

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape(theShape);
  TIDSortedElemSet anAffected;
  bool aResult = aMeshEditor.AffectedElemGroupsInRegion(anElems, aNodes, aShape, anAffected);


  declareMeshModified( /*isReComputeSafe=*/ !aResult );

  TPythonDump pyDump;
  if (aResult)
  {
    int lg = anAffected.size();
    MESSAGE("lg="<< lg);
    SMESH::long_array_var volumeIds = new SMESH::long_array;
    volumeIds->length(lg);
    SMESH::long_array_var faceIds = new SMESH::long_array;
    faceIds->length(lg);
    SMESH::long_array_var edgeIds = new SMESH::long_array;
    edgeIds->length(lg);
    int ivol = 0;
    int iface = 0;
    int iedge = 0;

    TIDSortedElemSet::const_iterator eIt = anAffected.begin();
    for (; eIt != anAffected.end(); ++eIt)
    {
      const SMDS_MeshElement* anElem = *eIt;
      if (!anElem)
        continue;
      int elemId = anElem->GetID();
      if (myMesh->GetElementType(elemId, true) == SMDSAbs_Volume)
        volumeIds[ivol++] = elemId;
      else if (myMesh->GetElementType(elemId, true) == SMDSAbs_Face)
        faceIds[iface++] = elemId;
      else if (myMesh->GetElementType(elemId, true) == SMDSAbs_Edge)
        edgeIds[iedge++] = elemId;
    }
    volumeIds->length(ivol);
    faceIds->length(iface);
    edgeIds->length(iedge);

    aNewVolumeGroup->Add(volumeIds);
    aNewFaceGroup->Add(faceIds);
    aNewEdgeGroup->Add(edgeIds);
    isVolumeGroup = (aNewVolumeGroup->Size() > 0);
    isFaceGroup = (aNewFaceGroup->Size() > 0);
    isEdgeGroup = (aNewEdgeGroup->Size() > 0);
  }

  int nbGroups = 0;
  if (isEdgeGroup)   nbGroups++;
  if (isFaceGroup)   nbGroups++;
  if (isVolumeGroup) nbGroups++;
  aListOfGroups->length(nbGroups);

  int i = 0;
  if (isEdgeGroup)   aListOfGroups[i++] = aNewEdgeGroup._retn();
  if (isFaceGroup)   aListOfGroups[i++] = aNewFaceGroup._retn();
  if (isVolumeGroup) aListOfGroups[i++] = aNewVolumeGroup._retn();

  // Update Python script

  pyDump << "[ ";
  if (isEdgeGroup)   pyDump << aNewEdgeGroup << ", ";
  if (isFaceGroup)   pyDump << aNewFaceGroup << ", ";
  if (isVolumeGroup) pyDump << aNewVolumeGroup << ", ";
  pyDump << "] = ";
  pyDump << this << ".AffectedElemGroupsInRegion( "
         << &theElems << ", " << &theNodesNot << ", " << theShape << " )";

  return aListOfGroups._retn();

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}

//================================================================================
/*!
  \brief Generated skin mesh (containing 2D cells) from 3D mesh
   The created 2D mesh elements based on nodes of free faces of boundary volumes
  \return TRUE if operation has been completed successfully, FALSE otherwise
*/
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::Make2DMeshFrom3D()
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  bool aResult = getEditor().Make2DMeshFrom3D();

  TPythonDump() << "isDone = " << this << ".Make2DMeshFrom3D()";

  declareMeshModified( /*isReComputeSafe=*/ !aResult );
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return false;
}

//================================================================================
/*!
 * \brief Double nodes on shared faces between groups of volumes and create flat elements on demand.
 * The list of groups must contain at least two groups. The groups have to be disjoint:
 * no common element into two different groups.
 * The nodes of the internal faces at the boundaries of the groups are doubled.
 * Optionally, the internal faces are replaced by flat elements.
 * Triangles are transformed into prisms, and quadrangles into hexahedrons.
 * The flat elements are stored in groups of volumes.
 * These groups are named according to the position of the group in the list:
 * the group j_n_p is the group of the flat elements that are built between the group #n and the group #p in the list.
 * If there is no shared faces between the group #n and the group #p in the list, the group j_n_p is not created.
 * All the flat elements are gathered into the group named "joints3D" (or "joints2D" in 2D situation).
 * The flat element of the multiple junctions between the simple junction are stored in a group named "jointsMultiples".
 * \param theDomains - list of groups of volumes
 * \param createJointElems - if TRUE, create the elements
 * \param onAllBoundaries - if TRUE, the nodes and elements are also created on
 *        the boundary between \a theDomains and the rest mesh
 * \return TRUE if operation has been completed successfully, FALSE otherwise
 */
//================================================================================

CORBA::Boolean
SMESH_MeshEditor_i::DoubleNodesOnGroupBoundaries( const SMESH::ListOfGroups& theDomains,
                                                  CORBA::Boolean             createJointElems,
                                                  CORBA::Boolean             onAllBoundaries )
  throw (SALOME::SALOME_Exception)
{
  bool isOK = false;

  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMeshDS = getMeshDS();

  // MESSAGE("theDomains.length = "<<theDomains.length());
  if ( theDomains.length() <= 1 && !onAllBoundaries )
    THROW_SALOME_CORBA_EXCEPTION("At least 2 groups are required.", SALOME::BAD_PARAM);

  vector<TIDSortedElemSet> domains;
  domains.resize( theDomains.length() );

  for ( int i = 0, n = theDomains.length(); i < n; i++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theDomains[ i ];
    if ( !CORBA::is_nil( aGrp ) /*&& ( aGrp->GetType() != SMESH::NODE )*/ )
    {
//      if ( aGrp->GetType() != SMESH::VOLUME )
//        THROW_SALOME_CORBA_EXCEPTION("Not a volume group", SALOME::BAD_PARAM);
      SMESH::long_array_var anIDs = aGrp->GetIDs();
      arrayToSet( anIDs, aMeshDS, domains[ i ], SMDSAbs_All );
    }
  }

  isOK = getEditor().DoubleNodesOnGroupBoundaries( domains, createJointElems, onAllBoundaries );
  // TODO publish the groups of flat elements in study

  declareMeshModified( /*isReComputeSafe=*/ !isOK );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DoubleNodesOnGroupBoundaries( " << &theDomains
                << ", " << createJointElems << ", " << onAllBoundaries << " )";

  SMESH_CATCH( SMESH::throwCorbaException );

  myMesh_i->CreateGroupServants(); // publish created groups if any

  return isOK;
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

CORBA::Boolean
SMESH_MeshEditor_i::CreateFlatElementsOnFacesGroups( const SMESH::ListOfGroups& theGroupsOfFaces )
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  SMESHDS_Mesh* aMeshDS = getMeshDS();

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

  bool aResult = getEditor().CreateFlatElementsOnFacesGroups( faceGroups );
  // TODO publish the groups of flat elements in study

  declareMeshModified( /*isReComputeSafe=*/ !aResult );

  // Update Python script
  TPythonDump() << this << ".CreateFlatElementsOnFacesGroups( " << &theGroupsOfFaces << " )";
  return aResult;

  SMESH_CATCH( SMESH::throwCorbaException );
  return false;
}

//================================================================================
/*!
 *  \brief Identify all the elements around a geom shape, get the faces delimiting
 *         the hole.
 *
 *  Build groups of volume to remove, groups of faces to replace on the skin of the
 *  object, groups of faces to remove inside the object, (idem edges).
 *  Build ordered list of nodes at the border of each group of faces to replace
 *  (to be used to build a geom subshape).
 */
//================================================================================

void SMESH_MeshEditor_i::CreateHoleSkin(CORBA::Double                  radius,
                                        GEOM::GEOM_Object_ptr          theShape,
                                        const char*                    groupName,
                                        const SMESH::double_array&     theNodesCoords,
                                        SMESH::array_of_long_array_out GroupsOfNodes)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;

  initData();
  std::vector<std::vector<int> > aListOfListOfNodes;
  ::SMESH_MeshEditor aMeshEditor( myMesh );

  theSearchersDeleter.Set( myMesh ); // remove theNodeSearcher if mesh is other
  if ( !theNodeSearcher )
    theNodeSearcher = SMESH_MeshAlgos::GetNodeSearcher( *getMeshDS() );

  vector<double> nodesCoords;
  for ( CORBA::ULong i = 0; i < theNodesCoords.length(); i++)
  {
    nodesCoords.push_back( theNodesCoords[i] );
  }

  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( theShape );
  aMeshEditor.CreateHoleSkin(radius, aShape, theNodeSearcher, groupName,
                             nodesCoords, aListOfListOfNodes);

  GroupsOfNodes = new SMESH::array_of_long_array;
  GroupsOfNodes->length( aListOfListOfNodes.size() );
  std::vector<std::vector<int> >::iterator llIt = aListOfListOfNodes.begin();
  for ( CORBA::Long i = 0; llIt != aListOfListOfNodes.end(); llIt++, i++ )
  {
    vector<int>& aListOfNodes = *llIt;
    vector<int>::iterator lIt = aListOfNodes.begin();;
    SMESH::long_array& aGroup = (*GroupsOfNodes)[ i ];
    aGroup.length( aListOfNodes.size() );
    for ( int j = 0; lIt != aListOfNodes.end(); lIt++, j++ )
      aGroup[ j ] = (*lIt);
  }
  TPythonDump() << "lists_nodes = " << this << ".CreateHoleSkin( "
                << radius << ", "
                << theShape
                << ", '" << groupName << "', "
                << theNodesCoords << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
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
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  initData();

  if ( dim > SMESH::BND_1DFROM2D )
    THROW_SALOME_CORBA_EXCEPTION("Invalid boundary dimension", SALOME::BAD_PARAM);

  SMESHDS_Mesh* aMeshDS = getMeshDS();

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
    getEditor().MakeBoundaryMesh( elements,
                                  ::SMESH_MeshEditor::Bnd_Dimension(dim),
                                  smesh_group,
                                  smesh_mesh,
                                  toCopyElements,
                                  toCopyExistingBondary);

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

  SMESH_CATCH( SMESH::throwCorbaException );
  return SMESH::SMESH_Mesh::_nil();
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
  SMESH_TRY;
  initData();

  if ( dim > SMESH::BND_1DFROM2D )
    THROW_SALOME_CORBA_EXCEPTION("Invalid boundary dimension", SALOME::BAD_PARAM);

  // separate groups belonging to this and other mesh
  SMESH::ListOfIDSources_var groupsOfThisMesh  = new SMESH::ListOfIDSources;
  SMESH::ListOfIDSources_var groupsOfOtherMesh = new SMESH::ListOfIDSources;
  groupsOfThisMesh ->length( groups.length() );
  groupsOfOtherMesh->length( groups.length() );
  int nbGroups = 0, nbGroupsOfOtherMesh = 0;
  for ( CORBA::ULong i = 0; i < groups.length(); ++i )
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
        nbAdded += getEditor().MakeBoundaryMesh( elements,
                                                 ::SMESH_MeshEditor::Bnd_Dimension(bdim),
                                                 smesh_group,
                                                 tgtMesh,
                                                 /*toCopyElements=*/false,
                                                 /*toCopyExistingBondary=*/srcMesh != tgtMesh,
                                                 /*toAddExistingBondary=*/true,
                                                 /*aroundElements=*/true);
      }
    }
  }
  else
  {
    nbAdded += getEditor().MakeBoundaryMesh( elements,
                                             ::SMESH_MeshEditor::Bnd_Dimension(dim),
                                             smesh_group,
                                             tgtMesh,
                                             /*toCopyElements=*/false,
                                             /*toCopyExistingBondary=*/srcMesh != tgtMesh,
                                             /*toAddExistingBondary=*/true);
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

  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}
