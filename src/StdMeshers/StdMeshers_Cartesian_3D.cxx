// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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
//  File   : StdMeshers_Cartesian_3D.cxx
//  Module : SMESH
//
#include "StdMeshers_Cartesian_3D.hxx"
#include "StdMeshers_CartesianParameters3D.hxx"
#include "StdMeshers_Cartesian_VL.hxx"
#include "StdMeshers_ViscousLayers.hxx"

// StdMeshersAlgos
#include"StdMeshers_Cartesian_3D_Hexahedron.hxx"

//STD
#include <limits>
#include <thread>

// BOOST
#include <boost/container/flat_map.hpp>

#ifdef WITH_TBB

#ifdef WIN32
// See https://docs.microsoft.com/en-gb/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019
// Windows 10 = 0x0A00  
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#endif

#include <tbb/parallel_for.h>
//#include <tbb/enumerable_thread_specific.h>
#endif

using namespace std;
using namespace SMESH;
using namespace StdMeshers::Cartesian3D;
// using namespace facegridintersector;

namespace 
{
  /*!
   * \brief Temporary mesh to hold 
   */
  struct TmpMesh: public SMESH_Mesh
  {
    TmpMesh() {
      _isShapeToMesh = (_id = 0);
      _meshDS  = new SMESHDS_Mesh( _id, true );
    }
  };  
} // namespace

//=============================================================================
/*!
 * Constructor
 */
//=============================================================================

StdMeshers_Cartesian_3D::StdMeshers_Cartesian_3D(int hypId, SMESH_Gen * gen)
  :SMESH_3D_Algo(hypId, gen)
{
  _name = "Cartesian_3D";
  _shapeType = (1 << TopAbs_SOLID);       // 1 bit /shape type
  _compatibleHypothesis.push_back( "CartesianParameters3D" );
  _compatibleHypothesis.push_back( StdMeshers_ViscousLayers::GetHypType() );

  _onlyUnaryInput = false;          // to mesh all SOLIDs at once
  _requireDiscreteBoundary = false; // 2D mesh not needed
  _supportSubmeshes = false;        // do not use any existing mesh
}

//=============================================================================
/*!
 * Check presence of a hypothesis
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::CheckHypothesis (SMESH_Mesh&          aMesh,
                                               const TopoDS_Shape&  aShape,
                                               Hypothesis_Status&   aStatus)
{
  aStatus = SMESH_Hypothesis::HYP_MISSING;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape, /*skipAux=*/false);
  list <const SMESHDS_Hypothesis* >::const_iterator h = hyps.begin();
  if ( h == hyps.end())
  {
    return false;
  }

  _hyp = nullptr;
  _hypViscousLayers = nullptr;
  _isComputeOffset = false;

  for ( ; h != hyps.end(); ++h )
  {
    if ( !_hyp && ( _hyp = dynamic_cast<const StdMeshers_CartesianParameters3D*>( *h )))
    {
      aStatus = _hyp->IsDefined() ? HYP_OK : HYP_BAD_PARAMETER;
    }
    else
    {
      _hypViscousLayers = dynamic_cast<const StdMeshers_ViscousLayers*>( *h );
    }
  }

  return aStatus == HYP_OK;
}



//=============================================================================
/*!
 * \brief Generates 3D structured Cartesian mesh in the internal part of
 * solid shapes and polyhedral volumes near the shape boundary.
 *  \param theMesh - mesh to fill in
 *  \param theShape - a compound of all SOLIDs to mesh
 *  \retval bool - true in case of success
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::Compute(SMESH_Mesh &         theMesh,
                                      const TopoDS_Shape & theShape)
{
  if ( _hypViscousLayers )
  {
    const StdMeshers_ViscousLayers* hypViscousLayers = _hypViscousLayers;
    _hypViscousLayers = nullptr;

    StdMeshers_Cartesian_VL::ViscousBuilder builder( hypViscousLayers, theMesh, theShape );

    std::string error;
    TopoDS_Shape offsetShape = builder.MakeOffsetShape( theShape, theMesh, error );
    if ( offsetShape.IsNull() )
      throw SALOME_Exception( error );

    SMESH_Mesh* offsetMesh = new TmpMesh(); 
    offsetMesh->ShapeToMesh( offsetShape );
    offsetMesh->GetSubMesh( offsetShape )->DependsOn();

    this->_isComputeOffset = true;
    if ( ! this->Compute( *offsetMesh, offsetShape ))
      return false;

    return builder.MakeViscousLayers( *offsetMesh, theMesh, theShape );
  }

  // The algorithm generates the mesh in following steps:

  // 1) Intersection of grid lines with the geometry boundary.
  // This step allows to find out if a given node of the initial grid is
  // inside or outside the geometry.

  // 2) For each cell of the grid, check how many of it's nodes are outside
  // of the geometry boundary. Depending on a result of this check
  // - skip a cell, if all it's nodes are outside
  // - skip a cell, if it is too small according to the size threshold
  // - add a hexahedron in the mesh, if all nodes are inside
  // - add a polyhedron in the mesh, if some nodes are inside and some outside

  _computeCanceled = false;

  SMESH_MesherHelper helper( theMesh );
  SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();

  try
  {
    Grid grid;
    grid._helper                         = &helper;
    grid._toAddEdges                     = _hyp->GetToAddEdges();
    grid._toCreateFaces                  = _hyp->GetToCreateFaces();
    grid._toConsiderInternalFaces        = _hyp->GetToConsiderInternalFaces();
    grid._toUseThresholdForInternalFaces = _hyp->GetToUseThresholdForInternalFaces();
    grid._sizeThreshold                  = _hyp->GetSizeThreshold();
    grid._toUseQuanta                    = _hyp->GetToUseQuanta();
    grid._quanta                         = _hyp->GetQuanta();

    if ( _isComputeOffset )
    {
      grid._toAddEdges = true;
      grid._toCreateFaces = true;
    }

    const auto numOfThreads = std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency();
    map< TGeomID, vector< TGeomID > > edge2faceIDsMap;
    grid.GridInitAndInterserctWithShape( theShape, edge2faceIDsMap, _hyp, numOfThreads, _computeCanceled );

    // create volume elements
    Hexahedron hex( &grid );
    
    int nbAdded = hex.MakeElements( helper, edge2faceIDsMap, numOfThreads );
    if ( nbAdded > 0 ) 
    {
      if ( !grid._toConsiderInternalFaces )
      {
        // make all SOLIDs computed
        TopExp_Explorer solidExp( theShape, TopAbs_SOLID );
        if ( SMESHDS_SubMesh* sm1 = meshDS->MeshElements( solidExp.Current()) )
        {
          SMDS_ElemIteratorPtr volIt = sm1->GetElements();
          for ( ; solidExp.More() && volIt->more(); solidExp.Next() )
          {
            const SMDS_MeshElement* vol = volIt->next();
            sm1->RemoveElement( vol );
            meshDS->SetMeshElementOnShape( vol, solidExp.Current() );
          }
        }
      }
      // make other sub-shapes computed
      setSubmeshesComputed( theMesh, theShape );
    }

    // remove free nodes
    if ( /*SMESHDS_SubMesh * smDS = */meshDS->MeshElements( helper.GetSubShapeID() ))
    {
      std::vector< const SMDS_MeshNode* > nodesToRemove;
      // get intersection nodes
      for ( int iDir = 0; iDir < 3; ++iDir )
      {
        vector< GridLine >& lines = grid._lines[ iDir ];
        for ( size_t i = 0; i < lines.size(); ++i )
        {
          multiset< F_IntersectPoint >::iterator ip = lines[i]._intPoints.begin();
          for ( ; ip != lines[i]._intPoints.end(); ++ip )
            if ( ip->_node &&
                 !ip->_node->IsNull() &&
                 ip->_node->NbInverseElements() == 0 &&
                 !ip->_node->isMarked() )
            {
              nodesToRemove.push_back( ip->_node );
              ip->_node->setIsMarked( true );
            }
        }
      }
      // get grid nodes
      for ( size_t i = 0; i < grid._nodes.size(); ++i )
        if ( grid._nodes[i] &&
             !grid._nodes[i]->IsNull() &&
             grid._nodes[i]->NbInverseElements() == 0 &&
             !grid._nodes[i]->isMarked() )
        {
          nodesToRemove.push_back( grid._nodes[i] );
          grid._nodes[i]->setIsMarked( true );
        }

      for ( size_t i = 0; i < grid._allBorderNodes.size(); ++i )
        if ( grid._allBorderNodes[i] &&
             !grid._allBorderNodes[i]->IsNull() &&
             grid._allBorderNodes[i]->NbInverseElements() == 0 )
        {
          nodesToRemove.push_back( grid._allBorderNodes[i] );
          grid._allBorderNodes[i]->setIsMarked( true );
        }

      // do remove
      for ( size_t i = 0; i < nodesToRemove.size(); ++i )
        meshDS->RemoveFreeNode( nodesToRemove[i], /*smD=*/0, /*fromGroups=*/false );
    }

    return nbAdded;

  }
  // SMESH_ComputeError is not caught at SMESH_submesh level for an unknown reason
  catch ( SMESH_ComputeError& e)
  {
    return error( SMESH_ComputeErrorPtr( new SMESH_ComputeError( e )));
  }
  return false;
}

//=============================================================================
/*!
 *  Evaluate
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::Evaluate(SMESH_Mesh &         /*theMesh*/,
                                       const TopoDS_Shape & /*theShape*/,
                                       MapShapeNbElems&     /*theResMap*/)
{
  return true;
}

//=============================================================================
namespace
{
  /*!
   * \brief Event listener setting/unsetting _alwaysComputed flag to
   *        submeshes of inferior levels to prevent their computing
   */
  struct _EventListener : public SMESH_subMeshEventListener
  {
    string _algoName;

    _EventListener(const string& algoName):
      SMESH_subMeshEventListener(/*isDeletable=*/true,"StdMeshers_Cartesian_3D::_EventListener"),
      _algoName(algoName)
    {}
    // --------------------------------------------------------------------------------
    // setting/unsetting _alwaysComputed flag to submeshes of inferior levels
    //
    static void setAlwaysComputed( const bool     isComputed,
                                   SMESH_subMesh* subMeshOfSolid)
    {
      SMESH_subMeshIteratorPtr smIt =
        subMeshOfSolid->getDependsOnIterator(/*includeSelf=*/false, /*complexShapeFirst=*/false);
      while ( smIt->more() )
      {
        SMESH_subMesh* sm = smIt->next();
        sm->SetIsAlwaysComputed( isComputed );
      }
      subMeshOfSolid->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
    }

    // --------------------------------------------------------------------------------
    // unsetting _alwaysComputed flag if "Cartesian_3D" was removed
    //
    virtual void ProcessEvent(const int          /*event*/,
                              const int          eventType,
                              SMESH_subMesh*     subMeshOfSolid,
                              SMESH_subMeshEventListenerData* /*data*/,
                              const SMESH_Hypothesis*         /*hyp*/ = 0)
    {
      if ( eventType == SMESH_subMesh::COMPUTE_EVENT )
      {
        setAlwaysComputed( subMeshOfSolid->GetComputeState() == SMESH_subMesh::COMPUTE_OK,
                           subMeshOfSolid );
      }
      else
      {
        SMESH_Algo* algo3D = subMeshOfSolid->GetAlgo();
        if ( !algo3D || _algoName != algo3D->GetName() )
          setAlwaysComputed( false, subMeshOfSolid );
      }
    }

    // --------------------------------------------------------------------------------
    // set the event listener
    //
    static void SetOn( SMESH_subMesh* subMeshOfSolid, const string& algoName )
    {
      subMeshOfSolid->SetEventListener( new _EventListener( algoName ),
                                        /*data=*/0,
                                        subMeshOfSolid );
    }

  }; // struct _EventListener

} // namespace

//================================================================================
/*!
 * \brief Sets event listener to submeshes if necessary
 *  \param subMesh - submesh where algo is set
 * This method is called when a submesh gets HYP_OK algo_state.
 * After being set, event listener is notified on each event of a submesh.
 */
//================================================================================

void StdMeshers_Cartesian_3D::SetEventListener(SMESH_subMesh* subMesh)
{
  _EventListener::SetOn( subMesh, GetName() );
}

//================================================================================
/*!
 * \brief Set _alwaysComputed flag to submeshes of inferior levels to avoid their computing
 */
//================================================================================

void StdMeshers_Cartesian_3D::setSubmeshesComputed(SMESH_Mesh&         theMesh,
                                                   const TopoDS_Shape& theShape)
{
  for ( TopExp_Explorer soExp( theShape, TopAbs_SOLID ); soExp.More(); soExp.Next() )
    _EventListener::setAlwaysComputed( true, theMesh.GetSubMesh( soExp.Current() ));
}
