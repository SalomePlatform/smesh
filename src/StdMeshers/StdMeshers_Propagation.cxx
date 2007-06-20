//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_Propagation.cxx
//  Module : SMESH
//  $Header$

#include "StdMeshers_Propagation.hxx"

#include "utilities.h"

#include "SMESH_Mesh.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_HypoFilter.hxx"
#include "SMDS_SetIterator.hxx"

using namespace std;

namespace {

  // =======================================================================
  /*!
   * \brief Listener managing propagation of 1D hypotheses
   */
  // =======================================================================

  class PropagationMgr: public SMESH_subMeshEventListener
  {
  public:
    static PropagationMgr* GetListener();
    /*!
     * \brief Set listener on edge submesh
     */
    static void Set(SMESH_subMesh * submesh);
    /*!
     * \brief Return an edge from which hypotheses are propagated from
     */
    static TopoDS_Edge GetSource(SMESH_subMesh * submesh) { return TopoDS_Edge(); };
    /*!
     * \brief Does it's main job
     */
    void ProcessEvent(const int          event,
                      const int          eventType,
                      SMESH_subMesh*     subMesh,
                      SMESH_subMeshEventListenerData* data,
                      const SMESH_Hypothesis*         hyp = 0);
  private:
    PropagationMgr();
  };
}

//=============================================================================
/*!
 * StdMeshers_Propagation Implementation
 */
//=============================================================================

StdMeshers_Propagation::StdMeshers_Propagation (int hypId, int studyId, SMESH_Gen * gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
  _name = GetName();
  _param_algo_dim = -1; // 1D auxiliary
}
StdMeshers_Propagation::~StdMeshers_Propagation()                      {}
string StdMeshers_Propagation::GetName ()                              { return "Propagation"; }
ostream & StdMeshers_Propagation::SaveTo (ostream & save)              { return save; }
istream & StdMeshers_Propagation::LoadFrom (istream & load)            { return load; }
ostream & operator << (ostream & save, StdMeshers_Propagation & hyp)   { return hyp.SaveTo(save); }
istream & operator >> (istream & load, StdMeshers_Propagation & hyp)   { return hyp.LoadFrom(load); }
bool StdMeshers_Propagation::SetParametersByMesh(const SMESH_Mesh*,
                                                 const TopoDS_Shape& ) { return false; }
void StdMeshers_Propagation::SetPropagationMgr(SMESH_subMesh* subMesh) { PropagationMgr::Set( subMesh ); }
/*!
 * \brief Return an edge from which hypotheses are propagated from
 */
TopoDS_Edge StdMeshers_Propagation::GetPropagationSource(SMESH_Mesh& theMesh,
                                                         const TopoDS_Shape& theEdge)
{
  return PropagationMgr::GetSource(theMesh.GetSubMeshContaining( theEdge ));
}

//=============================================================================
//=============================================================================
// PROPAGATION MANAGEMENT
//=============================================================================
//=============================================================================

namespace {

  enum SubMeshState { WAIT_PROPAG_HYP, // no propagation hyp in chain
                      HAS_PROPAG_HYP,  // propag hyp on this submesh
                      IN_CHAIN,        // submesh is in propagation chain
                      LAST_IN_CHAIN,   // submesh with local 1D hyp breaking a chain
                      MEANINGLESS_LAST };          // meaningless

  struct PropagationMgrData : public EventListenerData
  {
    bool myForward; //!< true if a curve of edge in chain is codirected with one of source edge
    PropagationMgrData( SubMeshState state ): EventListenerData(true) {
      myType = state;
    }
    SubMeshState State() const {
      return (SubMeshState) myType;
    }
    void SetSource(SMESH_subMesh* sm ) {
      mySubMeshes.clear(); if ( sm ) mySubMeshes.push_back( sm );
    }
    void SetChain(list< SMESH_subMesh* >& chain ) {
      mySubMeshes.clear(); mySubMeshes.splice( mySubMeshes.end(), chain );
    }
    SMESH_subMeshIteratorPtr GetChain() const;
    SMESH_subMesh* GetSource() const;
  };

  //=============================================================================
  /*!
   * \brief return filter to find Propagation hypothesis
   */
  SMESH_HypoFilter & propagHypFilter()
  {
    static SMESH_HypoFilter propagHypFilter
      ( SMESH_HypoFilter::HasName( StdMeshers_Propagation::GetName ()));
    return propagHypFilter;
  }
  //=============================================================================
  /*!
   * \brief return static PropagationMgr
   */
  PropagationMgr* PropagationMgr::GetListener()
  {
    static PropagationMgr theListener;
    return &theListener;
  }
  PropagationMgr* getListener()
  {
    return PropagationMgr::GetListener();
  }
  //=============================================================================
  /*!
   * \brief return PropagationMgrData
   */
  PropagationMgrData* getData(SMESH_subMesh* sm)
  {
    if ( sm )
      return static_cast< PropagationMgrData* >( sm->GetEventListenerData( getListener() ));
    return 0;
  }
  //=============================================================================
  /*!
   * \brief return PropagationMgrData
   */
  PropagationMgrData* getData(SMESH_Mesh& theMesh, const TopoDS_Shape& theEdge)
  {
    if ( theEdge.ShapeType() == TopAbs_EDGE )
      return getData( theMesh.GetSubMeshContaining( theEdge ) );
    return 0;
  }
  //================================================================================
  /*!
   * \brief Return an iterator on a chain
   */
  SMESH_subMeshIteratorPtr PropagationMgrData::GetChain() const
  {
    typedef SMESH_subMesh* TsubMesh;
    typedef SMDS_SetIterator< TsubMesh, list< TsubMesh >::const_iterator > TIterator;
    switch ( State() ) {
    case HAS_PROPAG_HYP:
      return SMESH_subMeshIteratorPtr
        ( new TIterator( mySubMeshes.begin(), mySubMeshes.end() ));
    case IN_CHAIN:
    case LAST_IN_CHAIN:
      if ( mySubMeshes.empty() ) break;
      return getData( mySubMeshes.front() )->GetChain();
    default:;
    }
    return SMESH_subMeshIteratorPtr
      ( new TIterator( mySubMeshes.end(), mySubMeshes.end() ));
  }
  //================================================================================
  /*!
   * \brief Return a propagation source submesh
   */
  SMESH_subMesh* PropagationMgrData::GetSource() const
  {
    if ( myType == IN_CHAIN || myType == LAST_IN_CHAIN )
      if ( !mySubMeshes.empty() ) 
        return mySubMeshes.front();
    return 0;
  }
  //=============================================================================
  /*!
   * \brief Returns a local 1D hypothesis used for theEdge
   */
  const SMESH_Hypothesis* isLocal1DHypothesis (SMESH_Mesh& theMesh,
                                               const TopoDS_Shape& theEdge)
  {
    static SMESH_HypoFilter hypo ( SMESH_HypoFilter::HasDim( 1 ));
    hypo.AndNot( hypo.IsAlgo() ).AndNot( hypo.IsAssignedTo( theMesh.GetMeshDS()->ShapeToMesh() ));

    return theMesh.GetHypothesis( theEdge, hypo, true );
  }
  //================================================================================
  /*!
   * \brief Build propagation chain
    * \param theMainSubMesh - the submesh with Propagation hypothesis
   */
  bool buildPropagationChain ( SMESH_subMesh* theMainSubMesh )
  {
  //   const TopoDS_Shape& theMainEdge = theMainSubMesh->GetSubShape();
//     if (theMainEdge.ShapeType() != TopAbs_EDGE) return true;

//     SMESH_Mesh* mesh = theMainSubMesh->GetFather();

//     EventListenerData* chainData = new PropagationMgrData(HAS_PROPAG_HYP);
//     theMainSubMesh->SetEventListener( getListener(), chainData, theMainSubMesh );

//     // Edges submeshes, on which the 1D hypothesis will be propagated from <theMainEdge>
//     list<SMESH_subMesh*> & chain = chainData->mySubMeshes;

//     // List of edges, added to chain on the previous cycle pass
//     TopTools_ListOfShape listPrevEdges;
//     listPrevEdges.Append(theMainEdge.Oriented( TopAbs_FORWARD ));

//     //   4____3____2____3____4____5
//     //   |    |    |    |    |    |      Number in the each knot of
//     //   |    |    |    |    |    |      grid indicates cycle pass,
//     //   3____2____1____2____3____4      on which corresponding edge
//     //   |    |    |    |    |    |      (perpendicular to the plane
//     //   |    |    |    |    |    |      of view) will be found.
//     //   2____1____0____1____2____3
//     //   |    |    |    |    |    |
//     //   |    |    |    |    |    |
//     //   3____2____1____2____3____4

//     // Collect all edges pass by pass
//     while (listPrevEdges.Extent() > 0) {
//       // List of edges, added to chain on this cycle pass
//       TopTools_ListOfShape listCurEdges;

//       // Find the next portion of edges
//       TopTools_ListIteratorOfListOfShape itE (listPrevEdges);
//       for (; itE.More(); itE.Next()) {
//         TopoDS_Shape anE = itE.Value();

//         // Iterate on faces, having edge <anE>
//         TopTools_ListIteratorOfListOfShape itA (mesh->GetAncestors(anE));
//         for (; itA.More(); itA.Next()) {
//           TopoDS_Shape aW = itA.Value();

//           // There are objects of different type among the ancestors of edge
//           if (aW.ShapeType() == TopAbs_WIRE) {
//             TopoDS_Shape anOppE;

//             BRepTools_WireExplorer aWE (TopoDS::Wire(aW));
//             Standard_Integer nb = 1, found = 0;
//             TopTools_Array1OfShape anEdges (1,4);
//             for (; aWE.More(); aWE.Next(), nb++) {
//               if (nb > 4) {
//                 found = 0;
//                 break;
//               }
//               anEdges(nb) = aWE.Current();
//               if (!_mapAncestors.Contains(anEdges(nb))) {
//                 MESSAGE("WIRE EXPLORER HAVE GIVEN AN INVALID EDGE !!!");
//                 break;
//               }
//               if (anEdges(nb).IsSame(anE)) found = nb;
//             }

//             if (nb == 5 && found > 0) {
//               // Quadrangle face found, get an opposite edge
//               Standard_Integer opp = ( found + 2 ) % 4;
//               anOppE = anEdges(opp);

//               // add anOppE to aChain if ...
//               PropagationMgrData* data = getData( *mesh, anOppE );
//               if ( !data || data->State() == WAIT_PROPAG_HYP ) { // ... anOppE is not in any chain
//                 if ( !isLocal1DHypothesis( *mesh, anOppE )) { // ... no other 1d hyp on anOppE
//                   // Add found edge to the chain oriented so that to
//                   // have it co-directed with a forward MainEdge
//                     TopAbs_Orientation ori = anE.Orientation();
//                     if ( anEdges(opp).Orientation() == anEdges(found).Orientation() )
//                       ori = TopAbs::Reverse( ori );
//                     anOppE.Orientation( ori );
//                     aChain.Add(anOppE);
//                     listCurEdges.Append(anOppE);
//                   }
//                   else {
//                     // Collision!
//                     MESSAGE("Error: Collision between propagated hypotheses");
//                     CleanMeshOnPropagationChain(theMainEdge);
//                     aChain.Clear();
//                     return ( aMainHyp == isLocal1DHypothesis(aMainEdgeForOppEdge) );
//                   }
//                 }
//               }
//             } // if (nb == 5 && found > 0)
//           } // if (aF.ShapeType() == TopAbs_WIRE)
//         } // for (; itF.More(); itF.Next())
//       } // for (; itE.More(); itE.Next())

//       listPrevEdges = listCurEdges;
//     } // while (listPrevEdges.Extent() > 0)

//     CleanMeshOnPropagationChain(theMainEdge);
    return true;
  }
  //================================================================================
  /*!
   * \brief Clear propagation chain
   */
  //================================================================================

  bool clearPropagationChain( SMESH_subMesh* subMesh )
  {
    if ( PropagationMgrData* data = getData( subMesh )) {
      if ( data->State() == IN_CHAIN )
        return clearPropagationChain( data->GetSource() );
      return true;
    }
    return false;
  }  


  //================================================================================
  /*!
   * \brief Constructor
   */
  PropagationMgr::PropagationMgr()
    : SMESH_subMeshEventListener( false ) // won't be deleted by submesh
  {}
  //================================================================================
  /*!
   * \brief Set PropagationMgr on a submesh
   */
  void PropagationMgr::Set(SMESH_subMesh * submesh)
  {
    EventListenerData* data = EventListenerData::MakeData(submesh,WAIT_PROPAG_HYP);

    submesh->SetEventListener( getListener(), data, submesh );

    const SMESH_Hypothesis * propagHyp =
      submesh->GetFather()->GetHypothesis( submesh->GetSubShape(), propagHypFilter(), true );
    if ( propagHyp )
      getListener()->ProcessEvent( SMESH_subMesh::ADD_HYP,
                                   SMESH_subMesh::ALGO_EVENT,
                                   submesh,
                                   data,
                                   propagHyp);
  }

  //================================================================================
  /*!
   * \brief React on events on 1D submeshes
   */
  //================================================================================

  void PropagationMgr::ProcessEvent(const int          event,
                                    const int          eventType,
                                    SMESH_subMesh*     subMesh,
                                    SMESH_subMeshEventListenerData* data,
                                    const SMESH_Hypothesis*         hyp)
  {
    if ( !data )
      return;
    if ( !hyp || hyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO || hyp->GetDim() != 1 )
      return;
    if ( eventType != SMESH_subMesh::ALGO_EVENT )
      return;

    bool isPropagHyp = ( StdMeshers_Propagation::GetName() != hyp->GetName() );

    switch ( data->myType ) {

    case WAIT_PROPAG_HYP: { // no propagation hyp in chain
      // --------------------------------------------------------
      if ( !isPropagHyp )
        return;
      if ( !isLocal1DHypothesis( *subMesh->GetFather(), subMesh->GetSubShape()))
        return;
      if ( event == SMESH_subMesh::ADD_HYP ||
           event == SMESH_subMesh::ADD_FATHER_HYP ) // add propagation hyp
      {
        // build propagation chain
        clearPropagationChain( subMesh );
        buildPropagationChain( subMesh );
      }
      return;
    }
    case HAS_PROPAG_HYP: {  // propag hyp on this submesh
      // --------------------------------------------------------
      switch ( event ) {
      case SMESH_subMesh::REMOVE_HYP:
      case SMESH_subMesh::REMOVE_FATHER_HYP: // remove propagation hyp
        if ( isPropagHyp )
        {
          // clear propagation chain
        }
        return;
      case SMESH_subMesh::MODIF_HYP: // hyp modif
        // clear mesh in a chain
        return;
      }
      return;
    }
    case IN_CHAIN: {       // submesh is in propagation chain
      // --------------------------------------------------------
      if ( event == SMESH_subMesh::ADD_HYP ) // add local hypothesis
        if ( isPropagHyp )
          ; // collision
        else
          ; // rebuild propagation chain
        return;
    }
    case LAST_IN_CHAIN: { // submesh with local 1D hyp, breaking a chain
      // --------------------------------------------------------
      if ( event == SMESH_subMesh::REMOVE_HYP ) // remove local hyp
        ; // rebuild propagation chain
      return;
    }
    } // switch by SubMeshState
  }
} // namespace
