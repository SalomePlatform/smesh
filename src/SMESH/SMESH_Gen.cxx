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

//  SMESH SMESH : implementaion of SMESH idl descriptions
//  File   : SMESH_Gen.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//

//#define CHRONODEF

#include "SMESH_Gen.hxx"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESHDS_Document.hxx"
#include "SMESH_HypoFilter.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"

#include "utilities.h"
#include "OpUtil.hxx"
#include "Utils_ExceptHandlers.hxx"

#include <TopoDS_Iterator.hxx>
#include <LDOMParser.hxx>

#include "memoire.h"

#ifdef WNT
  #include <windows.h>
#endif

using namespace std;

//=============================================================================
/*!
 *  Constructor
 */
//=============================================================================

SMESH_Gen::SMESH_Gen()
{
        MESSAGE("SMESH_Gen::SMESH_Gen");
        _localId = 0;
        _hypId = 0;
        _segmentation = _nbSegments = 10;
        SMDS_Mesh::_meshList.clear();
        MESSAGE(SMDS_Mesh::_meshList.size());
        _counters = new counters(100);
#ifdef WITH_SMESH_CANCEL_COMPUTE
        _compute_canceled = false;
        _sm_current = NULL;
#endif
}

//=============================================================================
/*!
 * Destructor
 */
//=============================================================================

SMESH_Gen::~SMESH_Gen()
{
  MESSAGE("SMESH_Gen::~SMESH_Gen");
}

//=============================================================================
/*!
 * Creates a mesh in a study.
 * if (theIsEmbeddedMode) { mesh modification commands are not logged }
 */
//=============================================================================

SMESH_Mesh* SMESH_Gen::CreateMesh(int theStudyId, bool theIsEmbeddedMode)
  throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  MESSAGE("SMESH_Gen::CreateMesh");

  // Get studyContext, create it if it does'nt exist, with a SMESHDS_Document
  StudyContextStruct *aStudyContext = GetStudyContext(theStudyId);

  // create a new SMESH_mesh object
  SMESH_Mesh *aMesh = new SMESH_Mesh(_localId++,
                                     theStudyId,
                                     this,
                                     theIsEmbeddedMode,
                                     aStudyContext->myDocument);
  aStudyContext->mapMesh[_localId-1] = aMesh;

  return aMesh;
}

//=============================================================================
/*!
 * Compute a mesh
 */
//=============================================================================

bool SMESH_Gen::Compute(SMESH_Mesh &          aMesh,
                        const TopoDS_Shape &  aShape,
                        const bool            anUpward,
                        const ::MeshDimension aDim,
                        TSetOfInt*            aShapesId)
{
  MESSAGE("SMESH_Gen::Compute");
  MEMOSTAT;

  bool ret = true;

  SMESH_subMesh *sm = aMesh.GetSubMesh(aShape);

  const bool includeSelf = true;
  const bool complexShapeFirst = true;
  const int  globalAlgoDim = 100;

  SMESH_subMeshIteratorPtr smIt;

  if ( anUpward ) // is called from below code here
  {
    // -----------------------------------------------
    // mesh all the sub-shapes starting from vertices
    // -----------------------------------------------
    smIt = sm->getDependsOnIterator(includeSelf, !complexShapeFirst);
    while ( smIt->more() )
    {
      SMESH_subMesh* smToCompute = smIt->next();

      // do not mesh vertices of a pseudo shape
      const TopAbs_ShapeEnum aShType = smToCompute->GetSubShape().ShapeType();
      if ( !aMesh.HasShapeToMesh() && aShType == TopAbs_VERTEX )
        continue;

      // check for preview dimension limitations
      if ( aShapesId && GetShapeDim( aShType ) > (int)aDim )
      {
        // clear compute state to not show previous compute errors
        //  if preview invoked less dimension less than previous
        smToCompute->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
        continue;
      }

      if (smToCompute->GetComputeState() == SMESH_subMesh::READY_TO_COMPUTE)
      {
#ifdef WITH_SMESH_CANCEL_COMPUTE
        if (_compute_canceled)
          return false;
        _sm_current = smToCompute;
#endif
        smToCompute->ComputeStateEngine( SMESH_subMesh::COMPUTE );
#ifdef WITH_SMESH_CANCEL_COMPUTE
        _sm_current = NULL;
#endif
      }

      // we check all the submeshes here and detect if any of them failed to compute
      if (smToCompute->GetComputeState() == SMESH_subMesh::FAILED_TO_COMPUTE)
        ret = false;
      else if ( aShapesId )
        aShapesId->insert( smToCompute->GetId() );
    }
    //aMesh.GetMeshDS()->Modified();
    return ret;
  }
  else
  {
    // -----------------------------------------------------------------
    // apply algos that DO NOT require Discreteized boundaries and DO NOT
    // support submeshes, starting from the most complex shapes
    // and collect submeshes with algos that DO support submeshes
    // -----------------------------------------------------------------
    list< SMESH_subMesh* > smWithAlgoSupportingSubmeshes;

    // map to sort sm with same dim algos according to dim of
    // the shape the algo assigned to (issue 0021217)
    multimap< int, SMESH_subMesh* > shDim2sm;
    multimap< int, SMESH_subMesh* >::reverse_iterator shDim2smIt;
    TopoDS_Shape algoShape;
    int prevShapeDim = -1;

    smIt = sm->getDependsOnIterator(includeSelf, complexShapeFirst);
    while ( smIt->more() )
    {
      SMESH_subMesh* smToCompute = smIt->next();
      if ( smToCompute->GetComputeState() != SMESH_subMesh::READY_TO_COMPUTE )
        continue;

      const TopoDS_Shape& aSubShape = smToCompute->GetSubShape();
      int aShapeDim = GetShapeDim( aSubShape );
      if ( aShapeDim < 1 ) break;
      
      // check for preview dimension limitations
      if ( aShapesId && aShapeDim > (int)aDim )
        continue;

      SMESH_Algo* algo = GetAlgo( aMesh, aSubShape, &algoShape );
      if ( algo && !algo->NeedDiscreteBoundary() )
      {
        if ( algo->SupportSubmeshes() )
        {
          // reload sub-meshes from shDim2sm into smWithAlgoSupportingSubmeshes
          // so that more local algos to go first
          if ( prevShapeDim != aShapeDim )
          {
            prevShapeDim = aShapeDim;
            for ( shDim2smIt = shDim2sm.rbegin(); shDim2smIt != shDim2sm.rend(); ++shDim2smIt )
              if ( shDim2smIt->first == globalAlgoDim )
                smWithAlgoSupportingSubmeshes.push_back( shDim2smIt->second );
              else
                smWithAlgoSupportingSubmeshes.push_front( shDim2smIt->second );
            shDim2sm.clear();
          }
          // add smToCompute to shDim2sm map
          if ( algoShape.IsSame( aMesh.GetShapeToMesh() ))
          {
            aShapeDim = globalAlgoDim; // to compute last
          }
          else
          {
            aShapeDim = GetShapeDim( algoShape );
            if ( algoShape.ShapeType() == TopAbs_COMPOUND )
            {
              TopoDS_Iterator it( algoShape );
              aShapeDim += GetShapeDim( it.Value() );
            }
          }
          shDim2sm.insert( make_pair( aShapeDim, smToCompute ));
        }
        else
        {
#ifdef WITH_SMESH_CANCEL_COMPUTE
          if (_compute_canceled)
            return false;
          _sm_current = smToCompute;
#endif
          smToCompute->ComputeStateEngine( SMESH_subMesh::COMPUTE );
#ifdef WITH_SMESH_CANCEL_COMPUTE
          _sm_current = NULL;
#endif
          if ( aShapesId )
            aShapesId->insert( smToCompute->GetId() );
        }
      }
    }
    // reload sub-meshes from shDim2sm into smWithAlgoSupportingSubmeshes
    for ( shDim2smIt = shDim2sm.rbegin(); shDim2smIt != shDim2sm.rend(); ++shDim2smIt )
      if ( shDim2smIt->first == globalAlgoDim )
        smWithAlgoSupportingSubmeshes.push_back( shDim2smIt->second );
      else
        smWithAlgoSupportingSubmeshes.push_front( shDim2smIt->second );

    // ------------------------------------------------------------
    // sort list of submeshes according to mesh order
    // ------------------------------------------------------------
    aMesh.SortByMeshOrder( smWithAlgoSupportingSubmeshes );

    // ------------------------------------------------------------
    // compute submeshes under shapes with algos that DO NOT require
    // Discreteized boundaries and DO support submeshes
    // ------------------------------------------------------------
    list< SMESH_subMesh* >::iterator subIt, subEnd;
    subIt  = smWithAlgoSupportingSubmeshes.begin();
    subEnd = smWithAlgoSupportingSubmeshes.end();
    // start from lower shapes
    for ( ; subIt != subEnd; ++subIt )
    {
      sm = *subIt;

      // get a shape the algo is assigned to
      if ( !GetAlgo( aMesh, sm->GetSubShape(), & algoShape ))
        continue; // strange...

      // look for more local algos
      smIt = sm->getDependsOnIterator(!includeSelf, !complexShapeFirst);
      while ( smIt->more() )
      {
        SMESH_subMesh* smToCompute = smIt->next();

        const TopoDS_Shape& aSubShape = smToCompute->GetSubShape();
        const int aShapeDim = GetShapeDim( aSubShape );
        //if ( aSubShape.ShapeType() == TopAbs_VERTEX ) continue;
        if ( aShapeDim < 1 ) continue;

        // check for preview dimension limitations
        if ( aShapesId && GetShapeDim( aSubShape.ShapeType() ) > (int)aDim )
          continue;
        
        SMESH_HypoFilter filter( SMESH_HypoFilter::IsAlgo() );
        filter
          .And( SMESH_HypoFilter::IsApplicableTo( aSubShape ))
          .And( SMESH_HypoFilter::IsMoreLocalThan( algoShape, aMesh ));

        if ( SMESH_Algo* subAlgo = (SMESH_Algo*) aMesh.GetHypothesis( aSubShape, filter, true )) {
          SMESH_Hypothesis::Hypothesis_Status status;
          if ( subAlgo->CheckHypothesis( aMesh, aSubShape, status ))
            // mesh a lower smToCompute starting from vertices
            Compute( aMesh, aSubShape, /*anUpward=*/true, aDim, aShapesId );
        }
      }
    }
    // ----------------------------------------------------------
    // apply the algos that do not require Discreteized boundaries
    // ----------------------------------------------------------
    for ( subIt = smWithAlgoSupportingSubmeshes.begin(); subIt != subEnd; ++subIt )
    {
      sm = *subIt;
      if ( sm->GetComputeState() == SMESH_subMesh::READY_TO_COMPUTE)
      {
        const TopAbs_ShapeEnum aShType = sm->GetSubShape().ShapeType();
        // check for preview dimension limitations
        if ( aShapesId && GetShapeDim( aShType ) > (int)aDim )
          continue;

#ifdef WITH_SMESH_CANCEL_COMPUTE
        if (_compute_canceled)
          return false;
        _sm_current = sm;
#endif
        sm->ComputeStateEngine( SMESH_subMesh::COMPUTE );
#ifdef WITH_SMESH_CANCEL_COMPUTE
        _sm_current = NULL;
#endif
        if ( aShapesId )
          aShapesId->insert( sm->GetId() );
      }
    }
    // -----------------------------------------------
    // mesh the rest sub-shapes starting from vertices
    // -----------------------------------------------
    ret = Compute( aMesh, aShape, /*anUpward=*/true, aDim, aShapesId );
  }

  MESSAGE( "VSR - SMESH_Gen::Compute() finished, OK = " << ret);
  MEMOSTAT;

  SMESHDS_Mesh *myMesh = aMesh.GetMeshDS();
  myMesh->adjustStructure();
  MESSAGE("*** compactMesh after compute");
  myMesh->compactMesh();
  //myMesh->adjustStructure();
  list<int> listind = myMesh->SubMeshIndices();
  list<int>::iterator it = listind.begin();
  int total = 0;
  for(; it != listind.end(); ++it)
    {
      ::SMESHDS_SubMesh *subMesh = myMesh->MeshElements(*it);
      total +=  subMesh->getSize();
    }
  MESSAGE("total elements and nodes in submesh sets:" << total);
  MESSAGE("Number of node objects " << SMDS_MeshNode::nbNodes);
  MESSAGE("Number of cell objects " << SMDS_MeshCell::nbCells);
  //myMesh->dumpGrid();
  //aMesh.GetMeshDS()->Modified();

  // fix quadratic mesh by bending iternal links near concave boundary
  if ( aShape.IsSame( aMesh.GetShapeToMesh() ) &&
       !aShapesId ) // not preview
  {
    SMESH_MesherHelper aHelper( aMesh );
    if ( aHelper.IsQuadraticMesh() != SMESH_MesherHelper::LINEAR )
    {
      aHelper.FixQuadraticElements( sm->GetComputeError() );
    }
  }
  return ret;
}


#ifdef WITH_SMESH_CANCEL_COMPUTE
//=============================================================================
/*!
 * Prepare Compute a mesh
 */
//=============================================================================
void SMESH_Gen::PrepareCompute(SMESH_Mesh &          aMesh,
                               const TopoDS_Shape &  aShape)
{
  _compute_canceled = false;
  _sm_current = NULL;
}
//=============================================================================
/*!
 * Cancel Compute a mesh
 */
//=============================================================================
void SMESH_Gen::CancelCompute(SMESH_Mesh &          aMesh,
                              const TopoDS_Shape &  aShape)
{
  _compute_canceled = true;
  if(_sm_current)
    {
      _sm_current->ComputeStateEngine( SMESH_subMesh::COMPUTE_CANCELED );
    }
}
#endif

//=============================================================================
/*!
 * Evaluate a mesh
 */
//=============================================================================

bool SMESH_Gen::Evaluate(SMESH_Mesh &          aMesh,
                         const TopoDS_Shape &  aShape,
                         MapShapeNbElems&      aResMap,
                         const bool            anUpward,
                         TSetOfInt*            aShapesId)
{
  MESSAGE("SMESH_Gen::Evaluate");

  bool ret = true;

  SMESH_subMesh *sm = aMesh.GetSubMesh(aShape);

  const bool includeSelf = true;
  const bool complexShapeFirst = true;
  SMESH_subMeshIteratorPtr smIt;

  if ( anUpward ) { // is called from below code here
    // -----------------------------------------------
    // mesh all the sub-shapes starting from vertices
    // -----------------------------------------------
    smIt = sm->getDependsOnIterator(includeSelf, !complexShapeFirst);
    while ( smIt->more() ) {
      SMESH_subMesh* smToCompute = smIt->next();

      // do not mesh vertices of a pseudo shape
      const TopAbs_ShapeEnum aShType = smToCompute->GetSubShape().ShapeType();
      //if ( !aMesh.HasShapeToMesh() && aShType == TopAbs_VERTEX )
      //  continue;
      if ( !aMesh.HasShapeToMesh() ) {
        if( aShType == TopAbs_VERTEX || aShType == TopAbs_WIRE ||
            aShType == TopAbs_SHELL )
          continue;
      }

      smToCompute->Evaluate(aResMap);
      if( aShapesId )
        aShapesId->insert( smToCompute->GetId() );
    }
    return ret;
  }
  else {
    // -----------------------------------------------------------------
    // apply algos that DO NOT require Discreteized boundaries and DO NOT
    // support submeshes, starting from the most complex shapes
    // and collect submeshes with algos that DO support submeshes
    // -----------------------------------------------------------------
    list< SMESH_subMesh* > smWithAlgoSupportingSubmeshes;
    smIt = sm->getDependsOnIterator(includeSelf, complexShapeFirst);
    while ( smIt->more() ) {
      SMESH_subMesh* smToCompute = smIt->next();
      const TopoDS_Shape& aSubShape = smToCompute->GetSubShape();
      const int aShapeDim = GetShapeDim( aSubShape );
      if ( aShapeDim < 1 ) break;
      
      SMESH_Algo* algo = GetAlgo( aMesh, aSubShape );
      if ( algo && !algo->NeedDiscreteBoundary() ) {
        if ( algo->SupportSubmeshes() ) {
          smWithAlgoSupportingSubmeshes.push_front( smToCompute );
        }
        else {
          smToCompute->Evaluate(aResMap);
          if ( aShapesId )
            aShapesId->insert( smToCompute->GetId() );
        }
      }
    }

    // ------------------------------------------------------------
    // sort list of meshes according to mesh order
    // ------------------------------------------------------------
    aMesh.SortByMeshOrder( smWithAlgoSupportingSubmeshes );

    // ------------------------------------------------------------
    // compute submeshes under shapes with algos that DO NOT require
    // Discreteized boundaries and DO support submeshes
    // ------------------------------------------------------------
    list< SMESH_subMesh* >::iterator subIt, subEnd;
    subIt  = smWithAlgoSupportingSubmeshes.begin();
    subEnd = smWithAlgoSupportingSubmeshes.end();
    // start from lower shapes
    for ( ; subIt != subEnd; ++subIt ) {
      sm = *subIt;

      // get a shape the algo is assigned to
      TopoDS_Shape algoShape;
      if ( !GetAlgo( aMesh, sm->GetSubShape(), & algoShape ))
        continue; // strange...

      // look for more local algos
      smIt = sm->getDependsOnIterator(!includeSelf, !complexShapeFirst);
      while ( smIt->more() ) {
        SMESH_subMesh* smToCompute = smIt->next();

        const TopoDS_Shape& aSubShape = smToCompute->GetSubShape();
        const int aShapeDim = GetShapeDim( aSubShape );
        if ( aShapeDim < 1 ) continue;

        //const TopAbs_ShapeEnum aShType = smToCompute->GetSubShape().ShapeType();

        SMESH_HypoFilter filter( SMESH_HypoFilter::IsAlgo() );
        filter
          .And( SMESH_HypoFilter::IsApplicableTo( aSubShape ))
          .And( SMESH_HypoFilter::IsMoreLocalThan( algoShape, aMesh ));

        if ( SMESH_Algo* subAlgo = (SMESH_Algo*) aMesh.GetHypothesis( aSubShape, filter, true )) {
          SMESH_Hypothesis::Hypothesis_Status status;
          if ( subAlgo->CheckHypothesis( aMesh, aSubShape, status ))
            // mesh a lower smToCompute starting from vertices
            Evaluate( aMesh, aSubShape, aResMap, /*anUpward=*/true, aShapesId );
        }
      }
    }
    // ----------------------------------------------------------
    // apply the algos that do not require Discreteized boundaries
    // ----------------------------------------------------------
    for ( subIt = smWithAlgoSupportingSubmeshes.begin(); subIt != subEnd; ++subIt )
    {
      sm = *subIt;
      sm->Evaluate(aResMap);
      if ( aShapesId )
        aShapesId->insert( sm->GetId() );
    }

    // -----------------------------------------------
    // mesh the rest sub-shapes starting from vertices
    // -----------------------------------------------
    ret = Evaluate( aMesh, aShape, aResMap, /*anUpward=*/true, aShapesId );
  }

  MESSAGE( "VSR - SMESH_Gen::Evaluate() finished, OK = " << ret);
  return ret;
}


//=======================================================================
//function : checkConformIgnoredAlgos
//purpose  :
//=======================================================================

static bool checkConformIgnoredAlgos(SMESH_Mesh&               aMesh,
                                     SMESH_subMesh*            aSubMesh,
                                     const SMESH_Algo*         aGlobIgnoAlgo,
                                     const SMESH_Algo*         aLocIgnoAlgo,
                                     bool &                    checkConform,
                                     set<SMESH_subMesh*>&      aCheckedMap,
                                     list< SMESH_Gen::TAlgoStateError > & theErrors)
{
  ASSERT( aSubMesh );
  if ( aSubMesh->GetSubShape().ShapeType() == TopAbs_VERTEX)
    return true;


  bool ret = true;

  const list<const SMESHDS_Hypothesis*>& listHyp =
    aMesh.GetMeshDS()->GetHypothesis( aSubMesh->GetSubShape() );
  list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();
  for ( ; it != listHyp.end(); it++)
  {
    const SMESHDS_Hypothesis * aHyp = *it;
    if (aHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
      continue;

    const SMESH_Algo* algo = dynamic_cast<const SMESH_Algo*> (aHyp);
    ASSERT ( algo );

    if ( aLocIgnoAlgo ) // algo is hidden by a local algo of upper dim
    {
      INFOS( "Local <" << algo->GetName() << "> is hidden by local <"
            << aLocIgnoAlgo->GetName() << ">");
    }
    else
    {
      bool isGlobal = (aMesh.IsMainShape( aSubMesh->GetSubShape() ));
      int dim = algo->GetDim();
      int aMaxGlobIgnoDim = ( aGlobIgnoAlgo ? aGlobIgnoAlgo->GetDim() : -1 );

      if ( dim < aMaxGlobIgnoDim )
      {
        // algo is hidden by a global algo
        INFOS( ( isGlobal ? "Global" : "Local" )
              << " <" << algo->GetName() << "> is hidden by global <"
              << aGlobIgnoAlgo->GetName() << ">");
      }
      else if ( !algo->NeedDiscreteBoundary() && !isGlobal)
      {
        // local algo is not hidden and hides algos on sub-shapes
        if (checkConform && !aSubMesh->IsConform( algo ))
        {
          ret = false;
          checkConform = false; // no more check conformity
          INFOS( "ERROR: Local <" << algo->GetName() <<
                "> would produce not conform mesh: "
                "<Not Conform Mesh Allowed> hypotesis is missing");
          theErrors.push_back( SMESH_Gen::TAlgoStateError() );
          theErrors.back().Set( SMESH_Hypothesis::HYP_NOTCONFORM, algo, false );
        }

        // sub-algos will be hidden by a local <algo>
        SMESH_subMeshIteratorPtr revItSub =
          aSubMesh->getDependsOnIterator( /*includeSelf=*/false, /*complexShapeFirst=*/true);
        bool checkConform2 = false;
        while ( revItSub->more() )
        {
          SMESH_subMesh* sm = revItSub->next();
          checkConformIgnoredAlgos (aMesh, sm, aGlobIgnoAlgo,
                                    algo, checkConform2, aCheckedMap, theErrors);
          aCheckedMap.insert( sm );
        }
      }
    }
  }

  return ret;
}

//=======================================================================
//function : checkMissing
//purpose  : notify on missing hypothesis
//           Return false if algo or hipothesis is missing
//=======================================================================

static bool checkMissing(SMESH_Gen*                aGen,
                         SMESH_Mesh&               aMesh,
                         SMESH_subMesh*            aSubMesh,
                         const int                 aTopAlgoDim,
                         bool*                     globalChecked,
                         const bool                checkNoAlgo,
                         set<SMESH_subMesh*>&      aCheckedMap,
                         list< SMESH_Gen::TAlgoStateError > & theErrors)
{
  if ( aSubMesh->GetSubShape().ShapeType() == TopAbs_VERTEX ||
       aCheckedMap.count( aSubMesh ))
    return true;

  //MESSAGE("=====checkMissing");

  int ret = true;
  SMESH_Algo* algo = 0;

  switch (aSubMesh->GetAlgoState())
  {
  case SMESH_subMesh::NO_ALGO: {
    if (checkNoAlgo)
    {
      // should there be any algo?
      int shapeDim = SMESH_Gen::GetShapeDim( aSubMesh->GetSubShape() );
      if (aTopAlgoDim > shapeDim)
      {
        MESSAGE( "ERROR: " << shapeDim << "D algorithm is missing" );
        ret = false;
        theErrors.push_back( SMESH_Gen::TAlgoStateError() );
        theErrors.back().Set( SMESH_Hypothesis::HYP_MISSING, shapeDim, true );
      }
    }
    return ret;
  }
  case SMESH_subMesh::MISSING_HYP: {
    // notify if an algo missing hyp is attached to aSubMesh
    algo = aGen->GetAlgo( aMesh, aSubMesh->GetSubShape() );
    ASSERT( algo );
    bool IsGlobalHypothesis = aGen->IsGlobalHypothesis( algo, aMesh );
    if (!IsGlobalHypothesis || !globalChecked[ algo->GetDim() ])
    {
      TAlgoStateErrorName errName = SMESH_Hypothesis::HYP_MISSING;
      SMESH_Hypothesis::Hypothesis_Status status;
      algo->CheckHypothesis( aMesh, aSubMesh->GetSubShape(), status );
      if ( status == SMESH_Hypothesis::HYP_BAD_PARAMETER ) {
        MESSAGE( "ERROR: hypothesis of " << (IsGlobalHypothesis ? "Global " : "Local ")
                 << "<" << algo->GetName() << "> has a bad parameter value");
        errName = status;
      } else if ( status == SMESH_Hypothesis::HYP_BAD_GEOMETRY ) {
        MESSAGE( "ERROR: " << (IsGlobalHypothesis ? "Global " : "Local ")
                 << "<" << algo->GetName() << "> assigned to mismatching geometry");
        errName = status;
      } else {
        MESSAGE( "ERROR: " << (IsGlobalHypothesis ? "Global " : "Local ")
                 << "<" << algo->GetName() << "> misses some hypothesis");
      }
      if (IsGlobalHypothesis)
        globalChecked[ algo->GetDim() ] = true;
      theErrors.push_back( SMESH_Gen::TAlgoStateError() );
      theErrors.back().Set( errName, algo, IsGlobalHypothesis );
    }
    ret = false;
    break;
  }
  case SMESH_subMesh::HYP_OK:
    algo = aSubMesh->GetAlgo();
    ret = true;
    if (!algo->NeedDiscreteBoundary())
    {
      SMESH_subMeshIteratorPtr itsub = aSubMesh->getDependsOnIterator( /*includeSelf=*/false,
                                                                       /*complexShapeFirst=*/false);
      while ( itsub->more() )
        aCheckedMap.insert( itsub->next() );
    }
    break;
  default: ASSERT(0);
  }

  // do not check under algo that hides sub-algos or
  // re-start checking NO_ALGO state
  ASSERT (algo);
  bool isTopLocalAlgo =
    ( aTopAlgoDim <= algo->GetDim() && !aGen->IsGlobalHypothesis( algo, aMesh ));
  if (!algo->NeedDiscreteBoundary() || isTopLocalAlgo)
  {
    bool checkNoAlgo2 = ( algo->NeedDiscreteBoundary() );
    SMESH_subMeshIteratorPtr itsub = aSubMesh->getDependsOnIterator( /*includeSelf=*/false,
                                                                     /*complexShapeFirst=*/false);
    while ( itsub->more() )
    {
      // sub-meshes should not be checked further more
      SMESH_subMesh* sm = itsub->next();

      if (isTopLocalAlgo)
      {
        //check algo on sub-meshes
        int aTopAlgoDim2 = algo->GetDim();
        if (!checkMissing (aGen, aMesh, sm, aTopAlgoDim2,
                           globalChecked, checkNoAlgo2, aCheckedMap, theErrors))
        {
          ret = false;
          if (sm->GetAlgoState() == SMESH_subMesh::NO_ALGO )
            checkNoAlgo2 = false;
        }
      }
      aCheckedMap.insert( sm );
    }
  }
  return ret;
}

//=======================================================================
//function : CheckAlgoState
//purpose  : notify on bad state of attached algos, return false
//           if Compute() would fail because of some algo bad state
//=======================================================================

bool SMESH_Gen::CheckAlgoState(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape)
{
  list< TAlgoStateError > errors;
  return GetAlgoState( aMesh, aShape, errors );
}

//=======================================================================
//function : GetAlgoState
//purpose  : notify on bad state of attached algos, return false
//           if Compute() would fail because of some algo bad state
//           theErrors list contains problems description
//=======================================================================

bool SMESH_Gen::GetAlgoState(SMESH_Mesh&               theMesh,
                             const TopoDS_Shape&       theShape,
                             list< TAlgoStateError > & theErrors)
{
  //MESSAGE("SMESH_Gen::CheckAlgoState");

  bool ret = true;
  bool hasAlgo = false;

  SMESH_subMesh*          sm = theMesh.GetSubMesh(theShape);
  const SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();
  TopoDS_Shape     mainShape = meshDS->ShapeToMesh();

  // -----------------
  // get global algos
  // -----------------

  const SMESH_Algo* aGlobAlgoArr[] = {0,0,0,0};

  const list<const SMESHDS_Hypothesis*>& listHyp = meshDS->GetHypothesis( mainShape );
  list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();
  for ( ; it != listHyp.end(); it++)
  {
    const SMESHDS_Hypothesis * aHyp = *it;
    if (aHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
      continue;

    const SMESH_Algo* algo = dynamic_cast<const SMESH_Algo*> (aHyp);
    ASSERT ( algo );

    int dim = algo->GetDim();
    aGlobAlgoArr[ dim ] = algo;

    hasAlgo = true;
  }

  // --------------------------------------------------------
  // info on algos that will be ignored because of ones that
  // don't NeedDiscreteBoundary() attached to super-shapes,
  // check that a conform mesh will be produced
  // --------------------------------------------------------


  // find a global algo possibly hiding sub-algos
  int dim;
  const SMESH_Algo* aGlobIgnoAlgo = 0;
  for (dim = 3; dim > 0; dim--)
  {
    if (aGlobAlgoArr[ dim ] &&
        !aGlobAlgoArr[ dim ]->NeedDiscreteBoundary())
    {
      aGlobIgnoAlgo = aGlobAlgoArr[ dim ];
      break;
    }
  }

  set<SMESH_subMesh*> aCheckedSubs;
  bool checkConform = ( !theMesh.IsNotConformAllowed() );

  // loop on theShape and its sub-shapes
  SMESH_subMeshIteratorPtr revItSub = sm->getDependsOnIterator( /*includeSelf=*/true,
                                                                /*complexShapeFirst=*/true);
  while ( revItSub->more() )
  {
    SMESH_subMesh* smToCheck = revItSub->next();
    if ( smToCheck->GetSubShape().ShapeType() == TopAbs_VERTEX)
      break;

    if ( aCheckedSubs.insert( smToCheck ).second ) // not yet checked
      if (!checkConformIgnoredAlgos (theMesh, smToCheck, aGlobIgnoAlgo,
                                     0, checkConform, aCheckedSubs, theErrors))
        ret = false;

    if ( smToCheck->GetAlgoState() != SMESH_subMesh::NO_ALGO )
      hasAlgo = true;
  }

  // ----------------------------------------------------------------
  // info on missing hypothesis and find out if all needed algos are
  // well defined
  // ----------------------------------------------------------------

  //MESSAGE( "---info on missing hypothesis and find out if all needed algos are");

  // find max dim of global algo
  int aTopAlgoDim = 0;
  for (dim = 3; dim > 0; dim--)
  {
    if (aGlobAlgoArr[ dim ])
    {
      aTopAlgoDim = dim;
      break;
    }
  }
  bool checkNoAlgo = theMesh.HasShapeToMesh() ? bool( aTopAlgoDim ) : false;
  bool globalChecked[] = { false, false, false, false };

  // loop on theShape and its sub-shapes
  aCheckedSubs.clear();
  revItSub = sm->getDependsOnIterator( /*includeSelf=*/true, /*complexShapeFirst=*/true);
  while ( revItSub->more() )
  {
    SMESH_subMesh* smToCheck = revItSub->next();
    if ( smToCheck->GetSubShape().ShapeType() == TopAbs_VERTEX)
      break;

    if ( aCheckedSubs.insert( smToCheck ).second ) // not yet checked
      if (!checkMissing (this, theMesh, smToCheck, aTopAlgoDim,
                         globalChecked, checkNoAlgo, aCheckedSubs, theErrors))
      {
        ret = false;
        if (smToCheck->GetAlgoState() == SMESH_subMesh::NO_ALGO )
          checkNoAlgo = false;
      }
  }

  if ( !hasAlgo ) {
    ret = false;
    INFOS( "None algorithm attached" );
    theErrors.push_back( TAlgoStateError() );
    theErrors.back().Set( SMESH_Hypothesis::HYP_MISSING, 1, true );
  }

  return ret;
}

//=======================================================================
//function : IsGlobalHypothesis
//purpose  : check if theAlgo is attached to the main shape
//=======================================================================

bool SMESH_Gen::IsGlobalHypothesis(const SMESH_Hypothesis* theHyp, SMESH_Mesh& aMesh)
{
  SMESH_HypoFilter filter( SMESH_HypoFilter::Is( theHyp ));
  return aMesh.GetHypothesis( aMesh.GetMeshDS()->ShapeToMesh(), filter, false );
}

//================================================================================
/*!
 * \brief Return paths to xml files of plugins
 */
//================================================================================

std::vector< std::string > SMESH_Gen::GetPluginXMLPaths()
{
  // Get paths to xml files of plugins
  vector< string > xmlPaths;
  string sep;
  if ( const char* meshersList = getenv("SMESH_MeshersList") )
  {
    string meshers = meshersList, plugin;
    string::size_type from = 0, pos;
    while ( from < meshers.size() )
    {
      // cut off plugin name
      pos = meshers.find( ':', from );
      if ( pos != string::npos )
        plugin = meshers.substr( from, pos-from );
      else
        plugin = meshers.substr( from ), pos = meshers.size();
      from = pos + 1;

      // get PLUGIN_ROOT_DIR path
      string rootDirVar, pluginSubDir = plugin;
      if ( plugin == "StdMeshers" )
        rootDirVar = "SMESH", pluginSubDir = "smesh";
      else
        for ( pos = 0; pos < plugin.size(); ++pos )
          rootDirVar += toupper( plugin[pos] );
      rootDirVar += "_ROOT_DIR";

      const char* rootDir = getenv( rootDirVar.c_str() );
      if ( !rootDir || strlen(rootDir) == 0 )
      {
        rootDirVar = plugin + "_ROOT_DIR"; // HexoticPLUGIN_ROOT_DIR
        rootDir = getenv( rootDirVar.c_str() );
        if ( !rootDir || strlen(rootDir) == 0 ) continue;
      }

      // get a separator from rootDir
      for ( pos = strlen( rootDir )-1; pos >= 0 && sep.empty(); --pos )
        if ( rootDir[pos] == '/' || rootDir[pos] == '\\' )
        {
          sep = rootDir[pos];
          break;
        }
#ifdef WNT
      if (sep.empty() ) sep = "\\";
#else
      if (sep.empty() ) sep = "/";
#endif

      // get a path to resource file
      string xmlPath = rootDir;
      if ( xmlPath[ xmlPath.size()-1 ] != sep[0] )
        xmlPath += sep;
      xmlPath += "share" + sep + "salome" + sep + "resources" + sep;
      for ( pos = 0; pos < pluginSubDir.size(); ++pos )
        xmlPath += tolower( pluginSubDir[pos] );
      xmlPath += sep + plugin + ".xml";
      bool fileOK;
#ifdef WNT
      fileOK = (GetFileAttributes(xmlPath.c_str()) != INVALID_FILE_ATTRIBUTES);
#else
      fileOK = (access(xmlPath.c_str(), F_OK) == 0);
#endif
      if ( fileOK )
        xmlPaths.push_back( xmlPath );
    }
  }

  return xmlPaths;
}

//=======================================================================
namespace // Access to type of input and output of an algorithm
//=======================================================================
{
  struct AlgoData
  {
    int                       _dim;
    set<SMDSAbs_GeometryType> _inElemTypes; // acceptable types of input mesh element
    set<SMDSAbs_GeometryType> _outElemTypes; // produced types of mesh elements

    bool IsCompatible( const AlgoData& algo2 ) const
    {
      if ( _dim > algo2._dim ) return algo2.IsCompatible( *this );
      // algo2 is of highter dimension
      if ( _outElemTypes.empty() || algo2._inElemTypes.empty() )
        return false;
      bool compatible = true;
      set<SMDSAbs_GeometryType>::const_iterator myOutType = _outElemTypes.begin();
      for ( ; myOutType != _outElemTypes.end() && compatible; ++myOutType )
        compatible = algo2._inElemTypes.count( *myOutType );
      return compatible;
    }
  };

  //================================================================================
  /*!
   * \brief Return AlgoData of the algorithm
   */
  //================================================================================

  const AlgoData& getAlgoData( const SMESH_Algo* algo )
  {
    static map< string, AlgoData > theDataByName;
    if ( theDataByName.empty() )
    {
      // Read Plugin.xml files
      vector< string > xmlPaths = SMESH_Gen::GetPluginXMLPaths();
      LDOMParser xmlParser;
      for ( size_t iXML = 0; iXML < xmlPaths.size(); ++iXML )
      {
        bool error = xmlParser.parse( xmlPaths[iXML].c_str() );
        if ( error )
        {
          TCollection_AsciiString data;
          INFOS( xmlParser.GetError(data) );
          continue;
        }
        // <algorithm type="Regular_1D"
        //            ...
        //            input="EDGE"
        //            output="QUAD,TRIA">
        //
        LDOM_Document xmlDoc = xmlParser.getDocument();
        LDOM_NodeList algoNodeList = xmlDoc.getElementsByTagName( "algorithm" );
        for ( int i = 0; i < algoNodeList.getLength(); ++i )
        {
          LDOM_Node     algoNode           = algoNodeList.item( i );
          LDOM_Element& algoElem           = (LDOM_Element&) algoNode;
          TCollection_AsciiString algoType = algoElem.getAttribute("type");
          TCollection_AsciiString input    = algoElem.getAttribute("input");
          TCollection_AsciiString output   = algoElem.getAttribute("output");
          TCollection_AsciiString dim      = algoElem.getAttribute("dim");
          if ( algoType.IsEmpty() ) continue;
          AlgoData & data                  = theDataByName[ algoType.ToCString() ];
          data._dim = dim.IntegerValue();
          for ( int isInput = 0; isInput < 2; ++isInput )
          {
            TCollection_AsciiString&   typeStr = isInput ? input : output;
            set<SMDSAbs_GeometryType>& typeSet = isInput ? data._inElemTypes : data._outElemTypes;
            int beg = 1, end;
            while ( beg <= typeStr.Length() )
            {
              while ( beg < typeStr.Length() && !isalpha( typeStr.Value( beg ) ))
                ++beg;
              end = beg;
              while ( end < typeStr.Length() && isalpha( typeStr.Value( end + 1 ) ))
                ++end;
              if ( end > beg )
              {
                TCollection_AsciiString typeName = typeStr.SubString( beg, end );
                if      ( typeName == "EDGE" ) typeSet.insert( SMDSGeom_EDGE );
                else if ( typeName == "TRIA" ) typeSet.insert( SMDSGeom_TRIANGLE );
                else if ( typeName == "QUAD" ) typeSet.insert( SMDSGeom_QUADRANGLE );
              }
              beg = end + 1;
            }
          }
        }
      }
    }
    return theDataByName[ algo->GetName() ];
  }
}

//=============================================================================
/*!
 * Finds algo to mesh a shape. Optionally returns a shape the found algo is bound to
 */
//=============================================================================

SMESH_Algo *SMESH_Gen::GetAlgo(SMESH_Mesh &         aMesh,
                               const TopoDS_Shape & aShape,
                               TopoDS_Shape*        assignedTo)
{
  SMESH_HypoFilter filter( SMESH_HypoFilter::IsAlgo() );
  filter.And( filter.IsApplicableTo( aShape ));

  TopoDS_Shape assignedToShape;
  SMESH_Algo* algo =
    (SMESH_Algo*) aMesh.GetHypothesis( aShape, filter, true, &assignedToShape );

  if ( algo &&
       aShape.ShapeType() == TopAbs_FACE &&
       !aShape.IsSame( assignedToShape ) &&
       SMESH_MesherHelper::NbAncestors( aShape, aMesh, TopAbs_SOLID ) > 1 )
  {
    // Issue 0021559. If there is another 2D algo with different types of output
    // elements that can be used to mesh aShape, and 3D algos on adjacent SOLIDs
    // have different types of input elements, we choose a most appropriate 2D algo.

    // try to find a concurrent 2D algo
    filter.AndNot( filter.Is( algo ));
    TopoDS_Shape assignedToShape2;
    SMESH_Algo* algo2 =
      (SMESH_Algo*) aMesh.GetHypothesis( aShape, filter, true, &assignedToShape2 );
    if ( algo2 &&                                                  // algo found
         !assignedToShape2.IsSame( aMesh.GetShapeToMesh() ) &&     // algo is local
         ( SMESH_MesherHelper::GetGroupType( assignedToShape2 ) == // algo of the same level
           SMESH_MesherHelper::GetGroupType( assignedToShape )) &&
         aMesh.IsOrderOK( aMesh.GetSubMesh( assignedToShape2 ),    // no forced order
                          aMesh.GetSubMesh( assignedToShape  )))
    {
      // get algos on the adjacent SOLIDs
      filter.Init( filter.IsAlgo() ).And( filter.HasDim( 3 ));
      vector< SMESH_Algo* > algos3D;
      PShapeIteratorPtr solidIt = SMESH_MesherHelper::GetAncestors( aShape, aMesh,
                                                                    TopAbs_SOLID );
      while ( const TopoDS_Shape* solid = solidIt->next() )
        if ( SMESH_Algo* algo3D = (SMESH_Algo*) aMesh.GetHypothesis( *solid, filter, true ))
        {
          algos3D.push_back( algo3D );
          filter.AndNot( filter.HasName( algo3D->GetName() ));
        }
      // check compatibility of algos
      if ( algos3D.size() > 1 )
      {
        const AlgoData& algoData    = getAlgoData( algo );
        const AlgoData& algoData2   = getAlgoData( algo2 );
        const AlgoData& algoData3d0 = getAlgoData( algos3D[0] );
        const AlgoData& algoData3d1 = getAlgoData( algos3D[1] );
        if (( algoData2.IsCompatible( algoData3d0 ) &&
              algoData2.IsCompatible( algoData3d1 ))
            &&
            !(algoData.IsCompatible( algoData3d0 ) &&
              algoData.IsCompatible( algoData3d1 )))
          algo = algo2;
      }
    }
  }

  if ( assignedTo && algo )
    * assignedTo = assignedToShape;

  return algo;
}

//=============================================================================
/*!
 * Returns StudyContextStruct for a study
 */
//=============================================================================

StudyContextStruct *SMESH_Gen::GetStudyContext(int studyId)
{
  // Get studyContext, create it if it does'nt exist, with a SMESHDS_Document

  if (_mapStudyContext.find(studyId) == _mapStudyContext.end())
  {
    _mapStudyContext[studyId] = new StudyContextStruct;
    _mapStudyContext[studyId]->myDocument = new SMESHDS_Document(studyId);
  }
  StudyContextStruct *myStudyContext = _mapStudyContext[studyId];
  return myStudyContext;
}

//================================================================================
/*!
 * \brief Return shape dimension by TopAbs_ShapeEnum
 */
//================================================================================

int SMESH_Gen::GetShapeDim(const TopAbs_ShapeEnum & aShapeType)
{
  static vector<int> dim;
  if ( dim.empty() )
  {
    dim.resize( TopAbs_SHAPE, -1 );
    dim[ TopAbs_COMPOUND ]  = MeshDim_3D;
    dim[ TopAbs_COMPSOLID ] = MeshDim_3D;
    dim[ TopAbs_SOLID ]     = MeshDim_3D;
    dim[ TopAbs_SHELL ]     = MeshDim_2D;
    dim[ TopAbs_FACE  ]     = MeshDim_2D;
    dim[ TopAbs_WIRE ]      = MeshDim_1D;
    dim[ TopAbs_EDGE ]      = MeshDim_1D;
    dim[ TopAbs_VERTEX ]    = MeshDim_0D;
  }
  return dim[ aShapeType ];
}

//=============================================================================
/*!
 * Genarate a new id unique withing this Gen
 */
//=============================================================================

int SMESH_Gen::GetANewId()
{
  return _hypId++;
}
