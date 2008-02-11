//  SMESH SMESH : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESH_Gen.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#include "SMESH_Gen.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_HypoFilter.hxx"
#include "SMESHDS_Document.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"

#include "utilities.h"
#include "OpUtil.hxx"
#include "Utils_ExceptHandlers.hxx"

#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

using namespace std;

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================

SMESH_Gen::SMESH_Gen()
{
	MESSAGE("SMESH_Gen::SMESH_Gen");
	_localId = 0;
	_hypId = 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_Gen::~SMESH_Gen()
{
	MESSAGE("SMESH_Gen::~SMESH_Gen");
}

//=============================================================================
/*!
 *
 */
//=============================================================================

/*SMESH_Hypothesis *SMESH_Gen::CreateHypothesis(const char *anHyp, int studyId)
	throw(SALOME_Exception)
{

	MESSAGE("CreateHypothesis("<<anHyp<<","<<studyId<<")");
	// Get studyContext, create it if it does'nt exist, with a SMESHDS_Document

	StudyContextStruct *myStudyContext = GetStudyContext(studyId);

	// create a new hypothesis object, store its ref. in studyContext

	SMESH_Hypothesis *myHypothesis = _hypothesisFactory.Create(anHyp, studyId);
	int hypId = myHypothesis->GetID();
	myStudyContext->mapHypothesis[hypId] = myHypothesis;
	SCRUTE(studyId);
	SCRUTE(hypId);

	// store hypothesis in SMESHDS document

	myStudyContext->myDocument->AddHypothesis(myHypothesis);
	return myHypothesis;
}*/

//=============================================================================
/*!
 *
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
  aStudyContext->mapMesh[_localId] = aMesh;

  return aMesh;
}

//=============================================================================
/*!
 * Compute a mesh
 */
//=============================================================================

bool SMESH_Gen::Compute(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
  MESSAGE("SMESH_Gen::Compute");

  bool ret = true;

  SMESH_subMesh *sm = aMesh.GetSubMesh(aShape);

  // -----------------------------------------------------------------
  // apply algos that do not require descretized boundaries, starting
  // from the most complex shapes
  // -----------------------------------------------------------------

  const bool includeSelf = true;
  const bool complexShapeFirst = true;

  SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator(includeSelf,
                                                           complexShapeFirst);
  while ( smIt->more() )
  {
    SMESH_subMesh* smToCompute = smIt->next();

    const TopoDS_Shape& aSubShape = smToCompute->GetSubShape();
    if ( GetShapeDim( aSubShape ) < 1 ) break;

    SMESH_Algo* algo = GetAlgo( aMesh, aSubShape );
    if (algo && !algo->NeedDescretBoundary())
    {
      if (smToCompute->GetComputeState() == SMESH_subMesh::READY_TO_COMPUTE)
        smToCompute->ComputeStateEngine( SMESH_subMesh::COMPUTE );

      if (smToCompute->GetComputeState() == SMESH_subMesh::FAILED_TO_COMPUTE)
        ret = false;;
    }
    if ((algo && !aMesh.HasShapeToMesh()))
    {
      if (smToCompute->GetComputeState() == SMESH_subMesh::READY_TO_COMPUTE)
        smToCompute->ComputeStateEngine( SMESH_subMesh::COMPUTE );
      
      if (smToCompute->GetComputeState() == SMESH_subMesh::FAILED_TO_COMPUTE)
        ret = false;;
    }
  }

  // -----------------------------------------------
  // mesh the rest subshapes starting from vertices
  // -----------------------------------------------
  smIt = sm->getDependsOnIterator(includeSelf, !complexShapeFirst);
  while ( smIt->more() )
  {
    SMESH_subMesh* smToCompute = smIt->next();

    if (smToCompute->GetComputeState() == SMESH_subMesh::READY_TO_COMPUTE)
      smToCompute->ComputeStateEngine( SMESH_subMesh::COMPUTE );

    if (smToCompute->GetComputeState() == SMESH_subMesh::FAILED_TO_COMPUTE)
      ret = false;
  }

  MESSAGE( "VSR - SMESH_Gen::Compute() finished, OK = " << ret);
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
                                     map<int, SMESH_subMesh*>& aCheckedMap,
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
      else if ( !algo->NeedDescretBoundary() && !isGlobal)
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
        const map<int, SMESH_subMesh*>& smMap = aSubMesh->DependsOn();
        map<int, SMESH_subMesh*>::const_reverse_iterator revItSub;
        bool checkConform2 = false;
        for ( revItSub = smMap.rbegin(); revItSub != smMap.rend(); revItSub++)
        {
          checkConformIgnoredAlgos (aMesh, (*revItSub).second, aGlobIgnoAlgo,
                                    algo, checkConform2, aCheckedMap, theErrors);
          int key = (*revItSub).first;
	  SMESH_subMesh* sm = (*revItSub).second;
          if ( aCheckedMap.find( key ) == aCheckedMap.end() )
          {
            aCheckedMap[ key ] = sm;
          }
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
                         map<int, SMESH_subMesh*>& aCheckedMap,
                         list< SMESH_Gen::TAlgoStateError > & theErrors)
{
  if ( aSubMesh->GetSubShape().ShapeType() == TopAbs_VERTEX)
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
    algo = aGen->GetAlgo( aMesh, aSubMesh->GetSubShape() );
    ret = true;
    break;
  default: ASSERT(0);
  }

  // do not check under algo that hides sub-algos or
  // re-start checking NO_ALGO state
  ASSERT (algo);
  bool isTopLocalAlgo =
    ( aTopAlgoDim <= algo->GetDim() && !aGen->IsGlobalHypothesis( algo, aMesh ));
  if (!algo->NeedDescretBoundary() || isTopLocalAlgo)
  {
    bool checkNoAlgo2 = ( algo->NeedDescretBoundary() );
    const map<int, SMESH_subMesh*>& subMeshes = aSubMesh->DependsOn();
    map<int, SMESH_subMesh*>::const_iterator itsub;
    for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
    {
      // sub-meshes should not be checked further more
      int key = (*itsub).first;
      SMESH_subMesh* sm = (*itsub).second;
      if ( aCheckedMap.find( key ) == aCheckedMap.end() )
        aCheckedMap[ key ] = sm;

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

  SMESH_subMesh* sm = theMesh.GetSubMesh(theShape);
  const SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();
  TopoDS_Shape mainShape = meshDS->ShapeToMesh();

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
  // don't NeedDescretBoundary() attached to super-shapes,
  // check that a conform mesh will be produced
  // --------------------------------------------------------


  // find a global algo possibly hiding sub-algos
  int dim;
  const SMESH_Algo* aGlobIgnoAlgo = 0;
  for (dim = 3; dim > 0; dim--)
  {
    if (aGlobAlgoArr[ dim ] &&
        !aGlobAlgoArr[ dim ]->NeedDescretBoundary())
    {
      aGlobIgnoAlgo = aGlobAlgoArr[ dim ];
      break;
    }
  }

  const map<int, SMESH_subMesh*>& smMap = sm->DependsOn();
  map<int, SMESH_subMesh*>::const_reverse_iterator revItSub = smMap.rbegin();
  map<int, SMESH_subMesh*> aCheckedMap;
  bool checkConform = ( !theMesh.IsNotConformAllowed() );
  int aKey = 1;
  SMESH_subMesh* smToCheck = sm;

  // loop on theShape and its sub-shapes
  while ( smToCheck )
  {
    if ( smToCheck->GetSubShape().ShapeType() == TopAbs_VERTEX)
      break;

    if ( aCheckedMap.find( aKey ) == aCheckedMap.end() )
      if (!checkConformIgnoredAlgos (theMesh, smToCheck, aGlobIgnoAlgo,
                                     0, checkConform, aCheckedMap, theErrors))
        ret = false;

    if ( smToCheck->GetAlgoState() != SMESH_subMesh::NO_ALGO )
      hasAlgo = true;

    // next subMesh
    if (revItSub != smMap.rend())
    {
      aKey = (*revItSub).first;
      smToCheck = (*revItSub).second;
      revItSub++;
    }
    else
    {
      smToCheck = 0;
    }

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
  aCheckedMap.clear();
  smToCheck = sm;
  revItSub = smMap.rbegin();
  bool checkNoAlgo = theMesh.HasShapeToMesh() ? bool( aTopAlgoDim ) : false;
  bool globalChecked[] = { false, false, false, false };

  // loop on theShape and its sub-shapes
  while ( smToCheck )
  {
    if ( smToCheck->GetSubShape().ShapeType() == TopAbs_VERTEX)
      break;

    if ( aCheckedMap.find( aKey ) == aCheckedMap.end() )
      if (!checkMissing (this, theMesh, smToCheck, aTopAlgoDim,
                         globalChecked, checkNoAlgo, aCheckedMap, theErrors))
      {
        ret = false;
        if (smToCheck->GetAlgoState() == SMESH_subMesh::NO_ALGO )
          checkNoAlgo = false;
      }

    // next subMesh
    if (revItSub != smMap.rend())
    {
      aKey = (*revItSub).first;
      smToCheck = (*revItSub).second;
      revItSub++;
    }
    else
      smToCheck = 0;
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

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_Algo *SMESH_Gen::GetAlgo(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{

  SMESH_HypoFilter filter( SMESH_HypoFilter::IsAlgo() );
  filter.And( filter.IsApplicableTo( aShape ));

  list <const SMESHDS_Hypothesis * > algoList;
  aMesh.GetHypotheses( aShape, filter, algoList, true );

  if ( algoList.empty() )
    return NULL;

  return const_cast<SMESH_Algo*> ( static_cast<const SMESH_Algo* >( algoList.front() ));
}

//=============================================================================
/*!
 *
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
//   ASSERT(_mapStudyContext.find(studyId) != _mapStudyContext.end());
	return myStudyContext;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_Gen::Save(int studyId, const char *aUrlOfFile)
{
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_Gen::Load(int studyId, const char *aUrlOfFile)
{
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_Gen::Close(int studyId)
{
}

//=============================================================================
/*!
 *
 */
//=============================================================================

int SMESH_Gen::GetShapeDim(const TopAbs_ShapeEnum & aShapeType)
{
  static vector<int> dim;
  if ( dim.empty() )
  {
    dim.resize( TopAbs_SHAPE, -1 );
    dim[ TopAbs_COMPOUND ]  = 3;
    dim[ TopAbs_COMPSOLID ] = 3;
    dim[ TopAbs_SOLID ]     = 3;
    dim[ TopAbs_SHELL ]     = 3;
    dim[ TopAbs_FACE  ]     = 2;
    dim[ TopAbs_WIRE ]      = 1;
    dim[ TopAbs_EDGE ]      = 1;
    dim[ TopAbs_VERTEX ]    = 0;
  }
  return dim[ aShapeType ];
}

//=============================================================================
/*!
 *
 */
//=============================================================================

int SMESH_Gen::GetANewId()
{
	//MESSAGE("SMESH_Gen::GetANewId");
	return _hypId++;
}
