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
#include "SMESHDriver.h"

#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>

#include "utilities.h"
#include "OpUtil.hxx"

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================

SMESH_Gen::SMESH_Gen()
{
	MESSAGE("SMESH_Gen::SMESH_Gen");
	_localId = 0;
	_hypothesisFactory.SetGen(this);
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

SMESH_Hypothesis *SMESH_Gen::CreateHypothesis(const char *anHyp, int studyId)
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
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Mesh *SMESH_Gen::Init(int studyId, const TopoDS_Shape & aShape)
throw(SALOME_Exception)
{
	MESSAGE("SMESH_Gen::Init");
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

	mesh->ShapeToMesh(aShape);
	return mesh;
}

//=============================================================================
/*!
 * @TODO Doing a full update after computation is not optimal when doing a local
 * remeshing.
 */
//=============================================================================

bool SMESH_Gen::Compute(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
throw(SALOME_Exception)
{
	MESSAGE("SMESH_Gen::Compute");
//   bool isDone = false;
/* 
Algo : s'appuie ou non sur une geometrie
Si geometrie:
Vertex : rien �faire (range le point)
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

	SMESH_subMesh *sm = aMesh.GetSubMesh(aShape);
//   SCRUTE(sm);
	SMESH_subMesh *smToCompute = sm->GetFirstToCompute();
	while (smToCompute)
	{
		TopoDS_Shape subShape = smToCompute->GetSubShape();
		int dim = GetShapeDim(subShape);
		//SCRUTE(dim);
		if (dim > 0)
		{
			bool ret1 = smToCompute->ComputeStateEngine(SMESH_subMesh::COMPUTE);
			ret = ret && ret1;
		}
		else
		{
			ASSERT(dim == 0);
			ASSERT(smToCompute->_vertexSet == false);
			TopoDS_Vertex V1 = TopoDS::Vertex(subShape);
			gp_Pnt P1 = BRep_Tool::Pnt(V1);
			SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
			//MESSAGE("point "<<nodeId<<" "<<P1.X()<<" "<<P1.Y()<<" "<<P1.Z());
			SMDS_MeshNode * node = meshDS->AddNode(P1.X(), P1.Y(), P1.Z());
			meshDS->SetNodeOnVertex(node, V1);
			smToCompute->GetSubMeshDS();
			smToCompute->_vertexSet = true;
			smToCompute->ComputeStateEngine(SMESH_subMesh::COMPUTE);
		}
		smToCompute = sm->GetFirstToCompute();
	}

	aMesh.GetMeshDS()->logFullUpdate();

	return ret;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Algo *SMESH_Gen::GetAlgo(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
	//MESSAGE("SMESH_Gen::GetAlgo");

	const SMESHDS_Hypothesis *theHyp = NULL;
	SMESH_Algo *algo = NULL;
	const SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
	int hypType;
	int hypId;
	int algoDim;

	// try shape first, then main shape

	TopoDS_Shape mainShape = meshDS->ShapeToMesh();
	const TopoDS_Shape *shapeToTry[2] = { &aShape, &mainShape };

	for (int iShape = 0; iShape < 2; iShape++)
	{
		TopoDS_Shape tryShape = (*shapeToTry[iShape]);

		const list<const SMESHDS_Hypothesis*>& listHyp =
			meshDS->GetHypothesis(tryShape);
		list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();
		
		int nb_algo = 0;
		int shapeDim = GetShapeDim(aShape);
		int typeOfShape = aShape.ShapeType();

		while (it!=listHyp.end())
		{
			const SMESHDS_Hypothesis *anHyp = *it;
			hypType = anHyp->GetType();
			//SCRUTE(hypType);
			if (hypType > SMESHDS_Hypothesis::PARAM_ALGO)
			{
				switch (hypType)
				{
				case SMESHDS_Hypothesis::ALGO_1D:
					algoDim = 1;
					break;
				case SMESHDS_Hypothesis::ALGO_2D:
					algoDim = 2;
					break;
				case SMESHDS_Hypothesis::ALGO_3D:
					algoDim = 3;
					break;
				default:
					algoDim = 0;
					break;
				}
				//SCRUTE(algoDim);
				//SCRUTE(shapeDim);
				//SCRUTE(typeOfShape);
				if (shapeDim == algoDim)	// count only algos of shape dim.
				{				// discard algos for subshapes
					hypId = anHyp->GetID();	// (of lower dim.)
					ASSERT(_mapAlgo.find(hypId) != _mapAlgo.end());
					SMESH_Algo *anAlgo = _mapAlgo[hypId];
					//SCRUTE(anAlgo->GetShapeType());
					//if (anAlgo->GetShapeType() == typeOfShape)
					if ((anAlgo->GetShapeType()) & (1 << typeOfShape))
					{			// only specific TopoDS_Shape
						nb_algo++;
						theHyp = anHyp;
					}
				}
			}
			if (nb_algo > 1) return NULL;	// more than one algo
			it++;
		}
		if (nb_algo == 1)		// one algo found : OK
			break;				// do not try a parent shape
	}

	if (!theHyp)
		return NULL;			// no algo found

	hypType = theHyp->GetType();
	hypId = theHyp->GetID();

	ASSERT(_mapAlgo.find(hypId) != _mapAlgo.end());
	algo = _mapAlgo[hypId];
	//MESSAGE("Algo found " << algo->GetName() << " Id " << hypId);
	return algo;
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

const char *SMESH_Gen::ComponentDataType()
{
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const char *SMESH_Gen::IORToLocalPersistentID(const char *IORString,
	bool & IsAFile)
{
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const char *SMESH_Gen::LocalPersistentIDToIOR(const char *aLocalPersistentID)
{
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESH_Gen::GetShapeDim(const TopoDS_Shape & aShape)
{
	int shapeDim = -1;			// Shape dimension: 0D, 1D, 2D, 3D
	int type = aShape.ShapeType();
	switch (type)
	{
//     case TopAbs_COMPOUND:
//       {
//  break;
//       }
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
//   SCRUTE(shapeDim);
	return shapeDim;
}

/**
 * Import a mesh from a file
 * @param fileName file name to be imported
 * @param fileType Currently it could be either "DAT", "UNV" or "MED".
 * @todo
 */
SMESH_Mesh * SMESH_Gen::Import(int studyId, const char * fileName,
	const char * fileType)
{
	MESSAGE("SMESH_Gen::Import("<<studyId<<","<<fileName<<","<<fileType<<")");

	// Get studyContext, create it if it does'nt exist, with a SMESHDS_Document
	StudyContextStruct *myStudyContext = GetStudyContext(studyId);

	// will be used with document
	/*Document_Reader * reader = SMESHDriver::GetDocumentReader(string(fileType));
	reader->SetDocument(myStudyContext->myDocument);
	reader->SetFile(string(fileName));
	reader->Read();*/
	// currently we only read one mesh from a file (limitation on MED files).

	// create a new SMESH_mesh object 
	SMESH_Mesh *mesh = new SMESH_Mesh(_localId++, studyId, this,
		myStudyContext->myDocument);
	myStudyContext->mapMesh[_localId] = mesh;
	
	Mesh_Reader * reader = SMESHDriver::GetMeshReader(string(fileType));
	reader->SetMesh(mesh->GetMeshDS());
	reader->SetFile(string(fileName));
	reader->Read();
	
	mesh->GetMeshDS()->logFullUpdate();
	
	return mesh;
}

