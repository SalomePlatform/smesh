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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESH_Gen.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#include "SMESH_Gen.hxx"
#include "SMESH_subMesh.hxx"
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

SMESH_Mesh* SMESH_Gen::CreateMesh(int studyId)
throw(SALOME_Exception)
{
        Unexpect aCatch(SalomeException);
	MESSAGE("SMESH_Gen::CreateMesh");
//   if (aShape.ShapeType() == TopAbs_COMPOUND)
//     {
//       INFOS("Mesh Compound not yet implemented!");
//       throw(SALOME_Exception(LOCALIZED("Mesh Compound not yet implemented!")));
//     }

	// Get studyContext, create it if it does'nt exist, with a SMESHDS_Document

	StudyContextStruct *myStudyContext = GetStudyContext(studyId);

	// create a new SMESH_mesh object

	SMESH_Mesh *mesh = new SMESH_Mesh(_localId++,
		studyId,
		this,
		myStudyContext->myDocument);
	myStudyContext->mapMesh[_localId] = mesh;

	// associate a TopoDS_Shape to the mesh

//mesh->ShapeToMesh(aShape);
	return mesh;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

bool SMESH_Gen::Compute(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
  MESSAGE("SMESH_Gen::Compute");
  //   bool isDone = false;
  /*
     Algo : s'appuie ou non sur une geometrie
     Si geometrie:
     Vertex : rien à faire (range le point)
     Edge, Wire, collection d'edge et wire : 1D
     Face, Shell, collection de Face et Shells : 2D
     Solid, Collection de Solid : 3D
     */
  // *** corriger commentaires
  // check hypothesis associated to the mesh :
  // - only one algo : type compatible with the type of the shape
  // - hypothesis = compatible with algo
  //    - check if hypothesis are applicable to this algo
  //    - check contradictions within hypothesis
  //    (test if enough hypothesis is done further)

  bool ret = true;

//   if ( !CheckAlgoState( aMesh, aShape ))
//   {
//     INFOS( "ABORT MESHING: some algos or hypothesis are missing");
//     return false;
//   }

  SMESH_subMesh *sm = aMesh.GetSubMesh(aShape);

  // -----------------------------------------------------------------
  // apply algos that do not require descretized boundaries, starting
  // from the most complex shapes
  // -----------------------------------------------------------------

  // map containing all subshapes in the order: vertices, edges, faces...
  const map<int, SMESH_subMesh*>& smMap = sm->DependsOn();
  map<int, SMESH_subMesh*>::const_reverse_iterator revItSub = smMap.rbegin();

  SMESH_subMesh* smToCompute = sm;
  while ( smToCompute )
  {
    const TopoDS_Shape& aSubShape = smToCompute->GetSubShape();
    if ( GetShapeDim( aSubShape ) < 1 ) break;

    SMESH_Algo* algo = GetAlgo( aMesh, aSubShape );
    if (algo && !algo->NeedDescretBoundary()) {
      if (smToCompute->GetComputeState() == SMESH_subMesh::READY_TO_COMPUTE) {
        ret = smToCompute->ComputeStateEngine( SMESH_subMesh::COMPUTE );
      } else if (smToCompute->GetComputeState() == SMESH_subMesh::FAILED_TO_COMPUTE) {
        // JFA for PAL6524
        ret = false;
      } else {
      }
    }
    if (!ret)
      return false;

    // next subMesh
    if (revItSub != smMap.rend())
    {
      smToCompute = (*revItSub).second;
      revItSub++;
    }
    else
      smToCompute = 0;
  }

  // -----------------------------------------------
  // mesh the rest subshapes starting from vertices
  // -----------------------------------------------

  int i, nbSub = smMap.size();
  map<int, SMESH_subMesh*>::const_iterator itSub = smMap.begin();
  for ( i = 0; i <= nbSub; ++i ) // loop on the whole map plus <sm>
  {
    if ( itSub == smMap.end() )
      smToCompute = sm;
    else
      smToCompute = (itSub++)->second;
    if (smToCompute->GetComputeState() != SMESH_subMesh::READY_TO_COMPUTE) {
      if (smToCompute->GetComputeState() == SMESH_subMesh::FAILED_TO_COMPUTE)
        ret = false;
      continue;
    }
    TopoDS_Shape subShape = smToCompute->GetSubShape();
    if ( subShape.ShapeType() != TopAbs_VERTEX )
    {
      if ( !smToCompute->ComputeStateEngine(SMESH_subMesh::COMPUTE) )
        ret = false;
    }
    else
    {
      TopoDS_Vertex V1 = TopoDS::Vertex(subShape);
      gp_Pnt P1 = BRep_Tool::Pnt(V1);
      SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
      SMDS_MeshNode * node = meshDS->AddNode(P1.X(), P1.Y(), P1.Z());
      if ( node ) {  // san - increase robustness
        meshDS->SetNodeOnVertex(node, V1);
        smToCompute->ComputeStateEngine(SMESH_subMesh::COMPUTE);
      }
    }
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
                                     map<int, SMESH_subMesh*>& aCheckedMap)
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
        }

        // sub-algos will be hidden by a local <algo>
        const map<int, SMESH_subMesh*>& smMap = aSubMesh->DependsOn();
        map<int, SMESH_subMesh*>::const_reverse_iterator revItSub;
        bool checkConform2 = false;
          for ( revItSub = smMap.rbegin(); revItSub != smMap.rend(); revItSub++)
        {
          checkConformIgnoredAlgos (aMesh, (*revItSub).second, aGlobIgnoAlgo,
                                    algo, checkConform2, aCheckedMap);
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
                         map<int, SMESH_subMesh*>& aCheckedMap)
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
        INFOS( "ERROR: " << shapeDim << "D algorithm is missing" );
        ret = false;
      }
    }
    return ret;
  }
  case SMESH_subMesh::MISSING_HYP: {
    // notify if an algo missing hyp is attached to aSubMesh
    algo = aGen->GetAlgo( aMesh, aSubMesh->GetSubShape() );
    ASSERT( algo );
    bool isGlobalAlgo = aGen->IsGlobalAlgo( algo, aMesh );
    if (!isGlobalAlgo || !globalChecked[ algo->GetDim() ])
    {
      INFOS( "ERROR: " << (isGlobalAlgo ? "Global " : "Local ")
            << "<" << algo->GetName() << "> misses some hypothesis");
      if (isGlobalAlgo)
        globalChecked[ algo->GetDim() ] = true;
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
    ( aTopAlgoDim <= algo->GetDim() && !aGen->IsGlobalAlgo( algo, aMesh ));
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
                           globalChecked, checkNoAlgo2, aCheckedMap))
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
  //MESSAGE("SMESH_Gen::CheckAlgoState");

  bool ret = true;
  bool hasAlgo = false;

  SMESH_subMesh* sm = aMesh.GetSubMesh(aShape);
  const SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
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


  // find a global algo possibly hidding sub-algos
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
  bool checkConform = ( !aMesh.IsNotConformAllowed() );
  int aKey = 1;
  SMESH_subMesh* smToCheck = sm;

  // loop on aShape and its sub-shapes
  while ( smToCheck )
  {
    if ( smToCheck->GetSubShape().ShapeType() == TopAbs_VERTEX)
      break;

    if ( aCheckedMap.find( aKey ) == aCheckedMap.end() )
      if (!checkConformIgnoredAlgos (aMesh, smToCheck, aGlobIgnoAlgo,
                                     0, checkConform, aCheckedMap))
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
  bool checkNoAlgo = (bool) aTopAlgoDim;
  bool globalChecked[] = { false, false, false, false };

  // loop on aShape and its sub-shapes
  while ( smToCheck )
  {
    if ( smToCheck->GetSubShape().ShapeType() == TopAbs_VERTEX)
      break;

    if ( aCheckedMap.find( aKey ) == aCheckedMap.end() )
      if (!checkMissing (this, aMesh, smToCheck, aTopAlgoDim,
                         globalChecked, checkNoAlgo, aCheckedMap))
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

  if ( !hasAlgo )
    INFOS( "None algorithm attached" );

  return ( ret && hasAlgo );
}

//=======================================================================
//function : IsGlobalAlgo
//purpose  : check if theAlgo is attached to the main shape
//=======================================================================

bool SMESH_Gen::IsGlobalAlgo(const SMESH_Algo* theAlgo, SMESH_Mesh& aMesh)
{
  const SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  TopoDS_Shape mainShape = meshDS->ShapeToMesh();
  const list<const SMESHDS_Hypothesis*>& listHyp = meshDS->GetHypothesis( mainShape );
  list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();
  for ( ; it != listHyp.end(); it++)
    if ( *it == theAlgo )
      return true;

  return false;
}


//=======================================================================
//function : getAlgoId
//purpose  : return algo ID or -1 if not found
//=======================================================================

static int getAlgo(const list<const SMESHDS_Hypothesis*>& theHypList,
                   const int                              theAlgoDim,
                   const int                              theAlgoShapeType)
{
  list<const SMESHDS_Hypothesis*>::const_iterator it = theHypList.begin();

  int nb_algo = 0;
  int algo_id = -1;

  while (it!=theHypList.end())
  {
    const SMESH_Hypothesis *anHyp = static_cast< const SMESH_Hypothesis *>( *it );
    if (anHyp->GetType() > SMESHDS_Hypothesis::PARAM_ALGO &&
        anHyp->GetDim() == theAlgoDim &&
        ((anHyp->GetShapeType()) & (1 << theAlgoShapeType)))
    {
      nb_algo++;
      algo_id = anHyp->GetID();
      break;
    }

    //if (nb_algo > 1) return -1;	// more than one algo
    it++;
  }

  return algo_id;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_Algo *SMESH_Gen::GetAlgo(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
//  MESSAGE("SMESH_Gen::GetAlgo");

  const SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  int dim = GetShapeDim( aShape );
  int shape_type = aShape.ShapeType();
  int algo_id = -1;

  algo_id = getAlgo( meshDS->GetHypothesis( aShape ), dim, shape_type );

  if (algo_id < 0)
  {
    // try ansestors
    TopTools_ListIteratorOfListOfShape ancIt( aMesh.GetAncestors( aShape ));
    for (; ancIt.More(); ancIt.Next())
    {
      const TopoDS_Shape& ancestor = ancIt.Value();
      algo_id = getAlgo( meshDS->GetHypothesis( ancestor ), dim, shape_type );
      if ( algo_id >= 0 )
        break;
    }
    if (algo_id < 0) return NULL;
  }

  ASSERT(_mapAlgo.find(algo_id) != _mapAlgo.end());

  return _mapAlgo[algo_id];

// 	const SMESHDS_Hypothesis *theHyp = NULL;
// 	SMESH_Algo *algo = NULL;
// 	const SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
// 	int hypType;
// 	int hypId;
// 	int algoDim;

// 	// try shape first, then main shape

// 	TopoDS_Shape mainShape = meshDS->ShapeToMesh();
// 	const TopoDS_Shape *shapeToTry[2] = { &aShape, &mainShape };

// 	for (int iShape = 0; iShape < 2; iShape++)
// 	{
// 		TopoDS_Shape tryShape = (*shapeToTry[iShape]);

// 		const list<const SMESHDS_Hypothesis*>& listHyp =
// 			meshDS->GetHypothesis(tryShape);
// 		list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();

// 		int nb_algo = 0;
// 		int shapeDim = GetShapeDim(aShape);
// 		int typeOfShape = aShape.ShapeType();

// 		while (it!=listHyp.end())
// 		{
// 			const SMESHDS_Hypothesis *anHyp = *it;
// 			hypType = anHyp->GetType();
// 			//SCRUTE(hypType);
// 			if (hypType > SMESHDS_Hypothesis::PARAM_ALGO)
// 			{
// 				switch (hypType)
// 				{
// 				case SMESHDS_Hypothesis::ALGO_1D:
// 					algoDim = 1;
// 					break;
// 				case SMESHDS_Hypothesis::ALGO_2D:
// 					algoDim = 2;
// 					break;
// 				case SMESHDS_Hypothesis::ALGO_3D:
// 					algoDim = 3;
// 					break;
// 				default:
// 					algoDim = 0;
// 					break;
// 				}
// 				//SCRUTE(algoDim);
// 				//SCRUTE(shapeDim);
// 				//SCRUTE(typeOfShape);
// 				if (shapeDim == algoDim)	// count only algos of shape dim.
// 				{				// discard algos for subshapes
// 					hypId = anHyp->GetID();	// (of lower dim.)
// 					ASSERT(_mapAlgo.find(hypId) != _mapAlgo.end());
// 					SMESH_Algo *anAlgo = _mapAlgo[hypId];
// 					//SCRUTE(anAlgo->GetShapeType());
// 					//if (anAlgo->GetShapeType() == typeOfShape)
// 					if ((anAlgo->GetShapeType()) & (1 << typeOfShape))
// 					{			// only specific TopoDS_Shape
// 						nb_algo++;
// 						theHyp = anHyp;
// 					}
// 				}
// 			}
// 			if (nb_algo > 1) return NULL;	// more than one algo
// 			it++;
// 		}
// 		if (nb_algo == 1)		// one algo found : OK
// 			break;				// do not try a parent shape
// 	}

// 	if (!theHyp)
// 		return NULL;			// no algo found

// 	hypType = theHyp->GetType();
// 	hypId = theHyp->GetID();

// 	ASSERT(_mapAlgo.find(hypId) != _mapAlgo.end());
// 	algo = _mapAlgo[hypId];
// 	//MESSAGE("Algo found " << algo->GetName() << " Id " << hypId);
// 	return algo;
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
	int shapeDim = -1;			// Shape dimension: 0D, 1D, 2D, 3D
	int type = aShapeType;//.ShapeType();
	switch (type)
	{
	case TopAbs_COMPOUND:
	case TopAbs_COMPSOLID:
	case TopAbs_SOLID:
	case TopAbs_SHELL:
	{
		shapeDim = 3;
		break;
	}
		//    case TopAbs_SHELL:
	case TopAbs_FACE:
	{
		shapeDim = 2;
		break;
	}
	case TopAbs_WIRE:
	case TopAbs_EDGE:
	{
		shapeDim = 1;
		break;
	}
	case TopAbs_VERTEX:
	{
		shapeDim = 0;
		break;
	}
	}
	return shapeDim;
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
