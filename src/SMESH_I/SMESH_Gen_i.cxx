//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
//  File   : SMESH_Gen_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TCollection_AsciiString.hxx>

#include <fstream>
#include <stdio.h>

#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_LocalLength_i.hxx"
#include "SMESH_NumberOfSegments_i.hxx"
#include "SMESH_MaxElementArea_i.hxx"
#include "SMESH_MaxElementVolume_i.hxx"

#include "SMESHDriver.h"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include "SALOMEDS_Tool.hxx"
#include "SALOME_NamingService.hxx"
#include "SALOME_LifeCycleCORBA.hxx"
#include "Utils_SINGLETON.hxx"
#include "OpUtil.hxx"

//#include <TopAbs_ShapeEnum.hxx>

#include "GEOM_Client.hxx"

#include <map>

#define NUM_TMP_FILES 4

// Tags definition 
long Tag_HypothesisRoot = 1;
long Tag_AlgorithmsRoot = 2;

long Tag_RefOnShape = 1;
long Tag_RefOnAppliedHypothesis = 2;
long Tag_RefOnAppliedAlgorithms = 3;

long Tag_SubMeshOnVertex = 4;
long Tag_SubMeshOnEdge = 5;
long Tag_SubMeshOnFace = 6;
long Tag_SubMeshOnSolid = 7;
long Tag_SubMeshOnCompound = 8;

//=============================================================================
/*!
 *  default constructor: not for use
 */
//=============================================================================

SMESH_Gen_i::SMESH_Gen_i()
{
	MESSAGE("SMESH_Gen_i default constructor");
	// ****
}

//=============================================================================
/*!
 *  Standard constructor, used with Container.
 */
//=============================================================================

SMESH_Gen_i::SMESH_Gen_i(CORBA::ORB_ptr orb,
	PortableServer::POA_ptr poa,
	PortableServer::ObjectId * contId,
	const char *instanceName,
	const char *interfaceName):Engines_Component_i(orb, poa, contId,
	instanceName, interfaceName)
{
	MESSAGE("activate object");
	_thisObj = this;
	_id = _poa->activate_object(_thisObj);

	_ShapeReader = NULL;
	_localId = 0;				// number of created objects & local id 

}

//=============================================================================
/*!
 *  Standard destructor
 */
//=============================================================================

SMESH_Gen_i::~SMESH_Gen_i()
{
	MESSAGE("~SMESH_Gen_i");
	// ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_Hypothesis_ptr SMESH_Gen_i::CreateHypothesis(const char *anHyp,
	CORBA::Long studyId) throw(SALOME::SALOME_Exception)
{
	MESSAGE("CreateHypothesis");

	// create a new hypothesis object servant

	SMESH_Hypothesis_i *myHypothesis_i = 0;
	try
	{
		myHypothesis_i = _hypothesisFactory_i.Create(anHyp, studyId, &_impl);
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}

	// activate the CORBA servant of hypothesis

	SMESH::SMESH_Hypothesis_var hypothesis_i
		= SMESH::SMESH_Hypothesis::_narrow(myHypothesis_i->_this());
	return SMESH::SMESH_Hypothesis::_duplicate(hypothesis_i);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::Init(GEOM::GEOM_Gen_ptr geomEngine,
	CORBA::Long studyId,
	GEOM::GEOM_Shape_ptr aShape) throw(SALOME::SALOME_Exception)
{
	MESSAGE("Init");
	// _narrow() duplicates the reference and checks the type
	GEOM::GEOM_Gen_var geom = GEOM::GEOM_Gen::_narrow(geomEngine);
	GEOM::GEOM_Shape_var myShape = GEOM::GEOM_Shape::_narrow(aShape);

	if (CORBA::is_nil(geom))
		THROW_SALOME_CORBA_EXCEPTION("bad geom reference", SALOME::BAD_PARAM);
	if (CORBA::is_nil(myShape))
		THROW_SALOME_CORBA_EXCEPTION("bad shape reference", SALOME::BAD_PARAM);

	// Get or create the GEOM_Client instance

	SMESH_Mesh_i *meshServant = 0;
	try
	{
		if (!_ShapeReader)
			_ShapeReader = new GEOM_Client();
		ASSERT(_ShapeReader);

		// explore main Shape, get local TopoDS_Shapes of all subShapes
		//  SMESH_topo* myTopo = ExploreMainShape(geom, studyId, myShape);

		// Get studyContext_i, create it if it does'nt exist

		if (_mapStudyContext_i.find(studyId) == _mapStudyContext_i.end())
		{
			_mapStudyContext_i[studyId] = new StudyContext_iStruct;
		}
		StudyContext_iStruct *myStudyContext = _mapStudyContext_i[studyId];

		// create a new mesh object servant, store it in a map in study context

		meshServant = new SMESH_Mesh_i(this, geom, studyId, _localId);
		myStudyContext->mapMesh_i[_localId] = meshServant;
		_localId++;

		// create a new mesh object

		TopoDS_Shape myLocShape = _ShapeReader->GetShape(geom, myShape);
		meshServant->SetImpl(_impl.Init(studyId, myLocShape));
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}

	// activate the CORBA servant of Mesh

	SMESH::SMESH_Mesh_var mesh
		= SMESH::SMESH_Mesh::_narrow(meshServant->_this());

	meshServant->SetIor(mesh);

	return SMESH::SMESH_Mesh::_duplicate(mesh);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::IsReadyToCompute(SMESH::SMESH_Mesh_ptr aMesh,
	GEOM::GEOM_Shape_ptr aShape) throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Gen_i::IsReadyToCompute");
	return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::long_array *
	SMESH_Gen_i::GetSubShapesId(GEOM::GEOM_Gen_ptr geomEngine,
	CORBA::Long studyId,
	GEOM::GEOM_Shape_ptr mainShape,
	const SMESH::shape_array & listOfSubShape)throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Gen_i::GetSubShapesId");
	SMESH::long_array_var shapesId = new SMESH::long_array;
	set < int >setId;

	GEOM::GEOM_Gen_var geom = GEOM::GEOM_Gen::_narrow(geomEngine);
	GEOM::GEOM_Shape_var myShape = GEOM::GEOM_Shape::_narrow(mainShape);

	if (CORBA::is_nil(geom))
		THROW_SALOME_CORBA_EXCEPTION("bad geom reference", SALOME::BAD_PARAM);
	if (CORBA::is_nil(myShape))
		THROW_SALOME_CORBA_EXCEPTION("bad shape reference", SALOME::BAD_PARAM);

	try
	{
		if (!_ShapeReader)
			_ShapeReader = new GEOM_Client();
		ASSERT(_ShapeReader);
		TopoDS_Shape myMainShape = _ShapeReader->GetShape(geom, myShape);
		TopTools_IndexedMapOfShape myIndexToShape;
		TopExp::MapShapes(myMainShape, myIndexToShape);

		for (unsigned int i = 0; i < listOfSubShape.length(); i++)
		{
			GEOM::GEOM_Shape_var aShape
				= GEOM::GEOM_Shape::_narrow(listOfSubShape[i]);
			if (CORBA::is_nil(aShape))
				THROW_SALOME_CORBA_EXCEPTION("bad shape reference",
					SALOME::BAD_PARAM);
			TopoDS_Shape locShape = _ShapeReader->GetShape(geom, aShape);
			for (TopExp_Explorer exp(locShape, TopAbs_FACE); exp.More();
				exp.Next())
			{
				const TopoDS_Face & F = TopoDS::Face(exp.Current());
				setId.insert(myIndexToShape.FindIndex(F));
				SCRUTE(myIndexToShape.FindIndex(F));
			}
			for (TopExp_Explorer exp(locShape, TopAbs_EDGE); exp.More();
				exp.Next())
			{
				const TopoDS_Edge & E = TopoDS::Edge(exp.Current());
				setId.insert(myIndexToShape.FindIndex(E));
				SCRUTE(myIndexToShape.FindIndex(E));
			}
			for (TopExp_Explorer exp(locShape, TopAbs_VERTEX); exp.More();
				exp.Next())
			{
				const TopoDS_Vertex & V = TopoDS::Vertex(exp.Current());
				setId.insert(myIndexToShape.FindIndex(V));
				SCRUTE(myIndexToShape.FindIndex(V));
			}
		}
		shapesId->length(setId.size());
		set < int >::iterator iind;
		int i = 0;
		for (iind = setId.begin(); iind != setId.end(); iind++)
		{
			SCRUTE((*iind));
			shapesId[i] = (*iind);
			SCRUTE(shapesId[i]);
			i++;
		}
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}

	return shapesId._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::Compute(SMESH::SMESH_Mesh_ptr aMesh,
	GEOM::GEOM_Shape_ptr aShape) throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Gen_i::Compute");
	GEOM::GEOM_Shape_var myShape = GEOM::GEOM_Shape::_narrow(aShape);
	if (CORBA::is_nil(myShape))
		THROW_SALOME_CORBA_EXCEPTION("bad shape reference", SALOME::BAD_PARAM);

	SMESH::SMESH_Mesh_var myMesh = SMESH::SMESH_Mesh::_narrow(aMesh);
	if (CORBA::is_nil(myMesh))
		THROW_SALOME_CORBA_EXCEPTION("bad Mesh reference", SALOME::BAD_PARAM);

	bool ret = false;
	try
	{

		// get study context from studyId given by CORBA mesh object

		int studyId = myMesh->GetStudyId();
		ASSERT(_mapStudyContext_i.find(studyId) != _mapStudyContext_i.end());
		StudyContext_iStruct *myStudyContext = _mapStudyContext_i[studyId];

		// get local Mesh_i object with Id and study context 

		int meshId = myMesh->GetId();
		ASSERT(myStudyContext->mapMesh_i.find(meshId) !=
			myStudyContext->mapMesh_i.end());
		SMESH_Mesh_i *meshServant = myStudyContext->mapMesh_i[meshId];
		ASSERT(meshServant);

		// get local TopoDS_Shape

		GEOM::GEOM_Gen_var geom = meshServant->GetGeomEngine();
		TopoDS_Shape myLocShape = _ShapeReader->GetShape(geom, myShape);

		// implementation compute

		::SMESH_Mesh & myLocMesh = meshServant->GetImpl();
		ret = _impl.Compute(myLocMesh, myLocShape);
	}
	catch(SALOME_Exception & S_ex)
	{
		MESSAGE("catch exception " << S_ex.what());
		return false;
//       THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}

	return ret;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SALOMEDS::TMPFile * SMESH_Gen_i::Save(SALOMEDS::SComponent_ptr theComponent,
	const char *theURL, bool isMultiFile)
{
	MESSAGE("SMESH_Gen_i::SAVE");
	SALOMEDS::Study_var Study = theComponent->GetStudy();
	int studyId;

	// Declare a byte stream
	SALOMEDS::TMPFile_var aStreamFile;

	// Obtain a temporary dir
	TCollection_AsciiString tmpDir =
		(isMultiFile) ? TCollection_AsciiString((char *)theURL) :
		SALOMEDS_Tool::GetTmpDir();

	// Create a sequence of files processed
	SALOMEDS::ListOfFileNames_var aFileSeq = new SALOMEDS::ListOfFileNames;
	aFileSeq->length(NUM_TMP_FILES);

	TCollection_AsciiString aStudyName("");

	if (isMultiFile)
		aStudyName =
			(SALOMEDS_Tool::GetNameFromPath(theComponent->GetStudy()->URL()));

	// Set names of temporary files
	TCollection_AsciiString filename =
		aStudyName + TCollection_AsciiString("_SMESH.hdf");
	TCollection_AsciiString hypofile =
		aStudyName + TCollection_AsciiString("_SMESH_Hypo.txt");
	TCollection_AsciiString algofile =
		aStudyName + TCollection_AsciiString("_SMESH_Algo.txt");
	TCollection_AsciiString meshfile =
		aStudyName + TCollection_AsciiString("_SMESH_Mesh.med");
	aFileSeq[0] = CORBA::string_dup(filename.ToCString());
	aFileSeq[1] = CORBA::string_dup(hypofile.ToCString());
	aFileSeq[2] = CORBA::string_dup(algofile.ToCString());
	aFileSeq[3] = CORBA::string_dup(meshfile.ToCString());
	filename = tmpDir + filename;
	hypofile = tmpDir + hypofile;
	algofile = tmpDir + algofile;
	meshfile = tmpDir + meshfile;

	HDFfile *hdf_file;
	map < int, HDFgroup * >hdf_group, hdf_subgroup;
	map < int, HDFdataset * >hdf_dataset;
	FILE *destFile;

	SALOMEDS::ChildIterator_var itBig, it, itSM;
	SALOMEDS::SObject_var mySObject, myBranch, mySObjectChild;
	hdf_size size[1];
	int longueur, cmpt_ds = 0, cmpt_it;
	char *name_group, name_dataset[30], name_meshgroup[30];
	bool ok, _found;
	int cmpt_sm = 0, myTag;

//************* HDF file creation
	hdf_file = new HDFfile(filename.ToCString());
	hdf_file->CreateOnDisk();
//****************************

	itBig = Study->NewChildIterator(theComponent);
	SCRUTE(Tag_HypothesisRoot);
	SCRUTE(Tag_AlgorithmsRoot);
	for (; itBig->More(); itBig->Next())
	{
		SALOMEDS::SObject_var gotBranch = itBig->Value();
		SCRUTE(gotBranch->Name());
		SCRUTE(gotBranch->Tag());
		SCRUTE(gotBranch->GetID());

//************branch 1 : hypothesis
		if (gotBranch->Tag() == Tag_HypothesisRoot)
		{						//hypothesis = tag 1
			double length, maxElementsArea, maxElementsVolume;
			int numberOfSegments;

			destFile = fopen(hypofile.ToCString(), "w");
			it = Study->NewChildIterator(gotBranch);
			for (; it->More(); it->Next())
			{
				mySObject = it->Value();
				SALOMEDS::GenericAttribute_var anAttr;
				SALOMEDS::AttributeIOR_var anIOR;
				if (mySObject->FindAttribute(anAttr, "AttributeIOR"))
				{
					anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);

					SMESH::SMESH_Hypothesis_var myHyp =
						SMESH::SMESH_Hypothesis::_narrow(_orb->
						string_to_object(anIOR->Value()));
					SCRUTE(myHyp->GetName());
					fprintf(destFile, "%li\n", myHyp->GetId());
					fprintf(destFile, "%s\n", myHyp->GetName());
					if (strcmp(myHyp->GetName(), "LocalLength") == 0)
					{
						SMESH::SMESH_LocalLength_var LL =
							SMESH::SMESH_LocalLength::_narrow(myHyp);
						length = LL->GetLength();
						fprintf(destFile, "%f\n", length);
					}
					else if (strcmp(myHyp->GetName(), "NumberOfSegments") == 0)
					{
						SMESH::SMESH_NumberOfSegments_var NOS =
							SMESH::SMESH_NumberOfSegments::_narrow(myHyp);
						numberOfSegments = NOS->GetNumberOfSegments();
						fprintf(destFile, "%d\n", numberOfSegments);
					}
					else if (strcmp(myHyp->GetName(), "MaxElementArea") == 0)
					{
						SMESH::SMESH_MaxElementArea_var MEA =
							SMESH::SMESH_MaxElementArea::_narrow(myHyp);
						maxElementsArea = MEA->GetMaxElementArea();
						fprintf(destFile, "%f\n", maxElementsArea);
					}
					else if (strcmp(myHyp->GetName(), "MaxElementVolume") == 0)
					{
						SMESH::SMESH_MaxElementVolume_var MEV =
							SMESH::SMESH_MaxElementVolume::_narrow(myHyp);
						maxElementsVolume = MEV->GetMaxElementVolume();
						fprintf(destFile, "%f\n", maxElementsVolume);
					}
				}
			}
			fclose(destFile);

//writes the file name in the hdf file
			longueur = hypofile.Length() + 1;
			name_group = "Hypothesis";
			//SCRUTE(name_group);

			size[0] = longueur;
			hdf_group[1] = new HDFgroup(name_group, hdf_file);
			hdf_group[1]->CreateOnDisk();

			hdf_dataset[cmpt_ds] =
				new HDFdataset(name_group, hdf_group[1], HDF_STRING, size, 1);
			hdf_dataset[cmpt_ds]->CreateOnDisk();
			hdf_dataset[cmpt_ds]->WriteOnDisk(hypofile.ToCString());
			hdf_dataset[cmpt_ds]->CloseOnDisk();
			cmpt_ds++;

			hdf_group[1]->CloseOnDisk();
			MESSAGE("End of Hypothesis Save");

		}
//************branch 2 : algorithms
		else if (gotBranch->Tag() == Tag_AlgorithmsRoot)
		{						//algos = tag 2
			destFile = fopen(algofile.ToCString(), "w");
			it = Study->NewChildIterator(gotBranch);
			for (; it->More(); it->Next())
			{
				mySObject = it->Value();
				SALOMEDS::GenericAttribute_var anAttr;
				SALOMEDS::AttributeIOR_var anIOR;
				if (mySObject->FindAttribute(anAttr, "AttributeIOR"))
				{
					anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
					SMESH::SMESH_Algo_var myAlgo =
						SMESH::SMESH_Algo::_narrow(_orb->
						string_to_object(anIOR->Value()));
					SCRUTE(anIOR->Value());
					SCRUTE(myAlgo->_is_nil());
					fprintf(destFile, "%i\n", myAlgo->GetId());
					fprintf(destFile, "%s\n", myAlgo->GetName());
				}
			}

			fclose(destFile);

//writes the file name in the hdf file
			longueur = algofile.Length() + 1;
			name_group = "Algorithms";
			//SCRUTE(name_group);

			size[0] = longueur;
			hdf_group[2] = new HDFgroup(name_group, hdf_file);
			hdf_group[2]->CreateOnDisk();

			hdf_dataset[cmpt_ds] =
				new HDFdataset(name_group, hdf_group[2], HDF_STRING, size, 1);
			hdf_dataset[cmpt_ds]->CreateOnDisk();
			hdf_dataset[cmpt_ds]->WriteOnDisk(algofile.ToCString());
			hdf_dataset[cmpt_ds]->CloseOnDisk();
			cmpt_ds++;

			hdf_group[2]->CloseOnDisk();
			MESSAGE("End of Algos Save");

		}
//************branch 3 : meshes
		else if (gotBranch->Tag() >= 3)
		{						//meshes = tag > 3

			SALOMEDS::GenericAttribute_var anAttr;
			SALOMEDS::AttributeIOR_var anIOR;
			if (gotBranch->FindAttribute(anAttr, "AttributeIOR"))
			{
				anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);

				SMESH::SMESH_Mesh_var myMesh =
					SMESH::SMESH_Mesh::_narrow(_orb->string_to_object(anIOR->
						Value()));
				studyId = myMesh->GetStudyId();
				SCRUTE(studyId);

				StudyContext_iStruct *myStudyContext =
					_mapStudyContext_i[studyId];
				int meshId = myMesh->GetId();
				SMESH_Mesh_i *meshServant = myStudyContext->mapMesh_i[meshId];
				::SMESH_Mesh & myLocMesh = meshServant->GetImpl();
				SMESHDS_Mesh *mySMESHDSMesh = myLocMesh.GetMeshDS();

				SCRUTE(mySMESHDSMesh->NbNodes());
				if (mySMESHDSMesh->NbNodes() > 0)
				{
					//checks if the mesh is not empty   
					Mesh_Writer *myWriter = SMESHDriver::GetMeshWriter("MED");
					myWriter->SetFile(meshfile.ToCString());
					myWriter->SetMesh(mySMESHDSMesh);
					myWriter->SetMeshId(gotBranch->Tag());
					myWriter->Add();
				}
				else
					meshfile = "No data";

				//********** opening of the HDF group
				sprintf(name_meshgroup, "Mesh %d", gotBranch->Tag());
				SCRUTE(name_meshgroup);
				hdf_group[gotBranch->Tag()] =
					new HDFgroup(name_meshgroup, hdf_file);
				hdf_group[gotBranch->Tag()]->CreateOnDisk();
				//********** 

				//********** file where the data are stored
				longueur = strlen(meshfile.ToCString()) + 1;
				size[0] = longueur;
				strcpy(name_dataset, "Mesh data");
				hdf_dataset[cmpt_ds] =
					new HDFdataset(name_dataset, hdf_group[gotBranch->Tag()],
					HDF_STRING, size, 1);
				hdf_dataset[cmpt_ds]->CreateOnDisk();
				hdf_dataset[cmpt_ds]->WriteOnDisk(meshfile.ToCString());
				hdf_dataset[cmpt_ds]->CloseOnDisk();
				cmpt_ds++;
				//********** 

				//********** ref on shape
				Standard_CString myRefOnObject = "";
				SALOMEDS::SObject_var myRef, myShape;
				_found = gotBranch->FindSubObject(Tag_RefOnShape, myRef);
				if (_found)
				{
					ok = myRef->ReferencedObject(myShape);
					myRefOnObject = myShape->GetID();
					SCRUTE(myRefOnObject);

					longueur = strlen(myRefOnObject) + 1;
					if (longueur > 1)
					{
						size[0] = longueur;
						strcpy(name_dataset, "Ref on shape");
						hdf_dataset[cmpt_ds] =
							new HDFdataset(name_dataset,
							hdf_group[gotBranch->Tag()], HDF_STRING, size, 1);
						hdf_dataset[cmpt_ds]->CreateOnDisk();
						hdf_dataset[cmpt_ds]->WriteOnDisk(myRefOnObject);
						hdf_dataset[cmpt_ds]->CloseOnDisk();
						cmpt_ds++;

					}
				}
				//********** 

				//********** ref on applied hypothesis
				_found =
					gotBranch->FindSubObject(Tag_RefOnAppliedHypothesis,
					myBranch);
				if (_found)
				{

					strcpy(name_meshgroup, "Applied Hypothesis");
					hdf_subgroup[Tag_RefOnAppliedHypothesis] =
						new HDFgroup(name_meshgroup,
						hdf_group[gotBranch->Tag()]);
					hdf_subgroup[Tag_RefOnAppliedHypothesis]->CreateOnDisk();

					it = Study->NewChildIterator(myBranch);
					cmpt_it = 0;
					for (; it->More(); it->Next())
					{
						mySObject = it->Value();
						ok = mySObject->ReferencedObject(myRef);
						myRefOnObject = myRef->GetID();

						longueur = strlen(myRefOnObject) + 1;
						if (longueur > 1)
						{
							size[0] = longueur;
							sprintf(name_dataset, "Hyp %d", cmpt_it);
							hdf_dataset[cmpt_ds] =
								new HDFdataset(name_dataset,
								hdf_subgroup[Tag_RefOnAppliedHypothesis],
								HDF_STRING, size, 1);
							hdf_dataset[cmpt_ds]->CreateOnDisk();
							hdf_dataset[cmpt_ds]->WriteOnDisk(myRefOnObject);
							hdf_dataset[cmpt_ds]->CloseOnDisk();
						}
						cmpt_ds++;
						cmpt_it++;
					}
					hdf_subgroup[Tag_RefOnAppliedHypothesis]->CloseOnDisk();
				}
				//********** 

				//********** ref on applied algorithms
				_found =
					gotBranch->FindSubObject(Tag_RefOnAppliedAlgorithms,
					myBranch);
				if (_found)
				{

					strcpy(name_meshgroup, "Applied Algorithms");
					hdf_subgroup[Tag_RefOnAppliedAlgorithms] =
						new HDFgroup(name_meshgroup,
						hdf_group[gotBranch->Tag()]);
					hdf_subgroup[Tag_RefOnAppliedAlgorithms]->CreateOnDisk();

					it = Study->NewChildIterator(myBranch);
					cmpt_it = 0;
					for (; it->More(); it->Next())
					{
						mySObject = it->Value();
						ok = mySObject->ReferencedObject(myRef);
						myRefOnObject = myRef->GetID();

						longueur = strlen(myRefOnObject) + 1;
						if (longueur > 1)
						{
							size[0] = longueur;
							sprintf(name_dataset, "Algo %d", cmpt_it);
							hdf_dataset[cmpt_ds] =
								new HDFdataset(name_dataset,
								hdf_subgroup[Tag_RefOnAppliedAlgorithms],
								HDF_STRING, size, 1);
							hdf_dataset[cmpt_ds]->CreateOnDisk();
							hdf_dataset[cmpt_ds]->WriteOnDisk(myRefOnObject);
							hdf_dataset[cmpt_ds]->CloseOnDisk();
						}
						cmpt_ds++;
						cmpt_it++;
					}
					hdf_subgroup[Tag_RefOnAppliedAlgorithms]->CloseOnDisk();
				}
				MESSAGE("end of algo applied");
				//********** 

				//********** submeshes on subshapes
				int myLevel1Tag;
				for (int i = Tag_SubMeshOnVertex; i <= Tag_SubMeshOnCompound;
					i++)
				{
					_found = gotBranch->FindSubObject(i, myBranch);
					if (_found)
					{
						if (i == Tag_SubMeshOnVertex)
							strcpy(name_meshgroup, "SubMeshes On Vertex");
						else if (i == Tag_SubMeshOnEdge)
							strcpy(name_meshgroup, "SubMeshes On Edge");
						else if (i == Tag_SubMeshOnFace)
							strcpy(name_meshgroup, "SubMeshes On Face");
						else if (i == Tag_SubMeshOnSolid)
							strcpy(name_meshgroup, "SubMeshes On Solid");
						else if (i == Tag_SubMeshOnCompound)
							strcpy(name_meshgroup, "SubMeshes On Compound");

						cmpt_sm++;
						myLevel1Tag = 10 + cmpt_sm;
						hdf_subgroup[myLevel1Tag] =
							new HDFgroup(name_meshgroup,
							hdf_group[gotBranch->Tag()]);
						hdf_subgroup[myLevel1Tag]->CreateOnDisk();

						itSM = Study->NewChildIterator(myBranch);
						for (; itSM->More(); itSM->Next())
						{		//Loop on all submeshes
							mySObject = itSM->Value();
							cmpt_sm++;
							myTag = 10 + cmpt_sm;
							mySObject->FindAttribute(anAttr, "AttributeIOR");
							anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
							SMESH::SMESH_subMesh_var mySubMesh =
								SMESH::SMESH_subMesh::_narrow(_orb->
								string_to_object(anIOR->Value()));

							//sprintf(name_meshgroup,"SubMesh %d",myTag);
							sprintf(name_meshgroup, "SubMesh %ld",
								mySubMesh->GetId());
							SCRUTE(name_meshgroup);

							hdf_subgroup[myTag] =
								new HDFgroup(name_meshgroup,
								hdf_subgroup[myLevel1Tag]);
							hdf_subgroup[myTag]->CreateOnDisk();

							//********** ref on shape
							Standard_CString myRefOnObject = "";
							SALOMEDS::SObject_var myRef, myShape;
							bool _found2;
							_found2 =
								mySObject->FindSubObject(Tag_RefOnShape, myRef);
							if (_found2)
							{
								ok = myRef->ReferencedObject(myShape);
								myRefOnObject = myShape->GetID();
								SCRUTE(myRefOnObject);

								longueur = strlen(myRefOnObject) + 1;
								if (longueur > 1)
								{
									size[0] = longueur;
									strcpy(name_dataset, "Ref on shape");
									hdf_dataset[cmpt_ds] =
										new HDFdataset(name_dataset,
										hdf_subgroup[myTag], HDF_STRING, size,
										1);
									hdf_dataset[cmpt_ds]->CreateOnDisk();
									hdf_dataset[cmpt_ds]->
										WriteOnDisk(myRefOnObject);
									hdf_dataset[cmpt_ds]->CloseOnDisk();
									cmpt_ds++;
								}
							}
							//********** 

							//********** ref on applied hypothesis
							_found2 =
								mySObject->
								FindSubObject(Tag_RefOnAppliedHypothesis,
								myBranch);
							if (_found2)
							{

								strcpy(name_meshgroup, "Applied Hypothesis");
								cmpt_sm++;
								hdf_subgroup[10 + cmpt_sm] =
									new HDFgroup(name_meshgroup,
									hdf_subgroup[myTag]);
								hdf_subgroup[10 + cmpt_sm]->CreateOnDisk();

								it = Study->NewChildIterator(myBranch);
								cmpt_it = 0;
								for (; it->More(); it->Next())
								{
									mySObjectChild = it->Value();
									ok = mySObjectChild->
										ReferencedObject(myRef);
									myRefOnObject = myRef->GetID();

									longueur = strlen(myRefOnObject) + 1;
									if (longueur > 1)
									{
										size[0] = longueur;
										sprintf(name_dataset, "Hyp %d",
											cmpt_it);
										SCRUTE(cmpt_it);
										hdf_dataset[cmpt_ds] =
											new HDFdataset(name_dataset,
											hdf_subgroup[10 + cmpt_sm],
											HDF_STRING, size, 1);
										hdf_dataset[cmpt_ds]->CreateOnDisk();
										hdf_dataset[cmpt_ds]->
											WriteOnDisk(myRefOnObject);
										hdf_dataset[cmpt_ds]->CloseOnDisk();
									}
									cmpt_ds++;
									cmpt_it++;
								}
								hdf_subgroup[10 + cmpt_sm]->CloseOnDisk();
							}
							//********** 

							//********** ref on applied algorithms
							_found2 =
								mySObject->
								FindSubObject(Tag_RefOnAppliedAlgorithms,
								myBranch);
							SCRUTE(_found2);
							if (_found2)
							{

								strcpy(name_meshgroup, "Applied Algorithms");
								cmpt_sm++;
								hdf_subgroup[10 + cmpt_sm] =
									new HDFgroup(name_meshgroup,
									hdf_subgroup[myTag]);
								hdf_subgroup[10 + cmpt_sm]->CreateOnDisk();

								it = Study->NewChildIterator(myBranch);
								cmpt_it = 0;
								for (; it->More(); it->Next())
								{
									mySObjectChild = it->Value();
									ok = mySObjectChild->
										ReferencedObject(myRef);
									myRefOnObject = myRef->GetID();

									longueur = strlen(myRefOnObject) + 1;
									if (longueur > 1)
									{
										size[0] = longueur;
										sprintf(name_dataset, "Algo %d",
											cmpt_it);
										hdf_dataset[cmpt_ds] =
											new HDFdataset(name_dataset,
											hdf_subgroup[10 + cmpt_sm],
											HDF_STRING, size, 1);
										hdf_dataset[cmpt_ds]->CreateOnDisk();
										hdf_dataset[cmpt_ds]->
											WriteOnDisk(myRefOnObject);
										hdf_dataset[cmpt_ds]->CloseOnDisk();
									}
									cmpt_ds++;
									cmpt_it++;
								}
								hdf_subgroup[10 + cmpt_sm]->CloseOnDisk();
							}
							//MESSAGE("end of algo applied");
							//********** 

							hdf_subgroup[myTag]->CloseOnDisk();
						}

						hdf_subgroup[myLevel1Tag]->CloseOnDisk();
					}

				}
				//********** 

				//********** closing of the HDF group
				hdf_group[gotBranch->Tag()]->CloseOnDisk();
				MESSAGE("End of Mesh Save");
				//********** 
			}
		}
		MESSAGE("End of Meshes Save");
	}

	MESSAGE("hdf_file->CloseOnDisk()");
	hdf_file->CloseOnDisk();

	MESSAGE("delete hdf_file");
	delete hdf_file;
	hdf_file = 0;

	// Convert temporary files to stream
	MESSAGE("Convert temporary files to stream");
	aStreamFile =
		SALOMEDS_Tool::PutFilesToStream(tmpDir.ToCString(), aFileSeq.in(),
		isMultiFile);

	// Remove temporary files and directory
	MESSAGE("Remove temporary files and directory");
	if (!isMultiFile)
		SALOMEDS_Tool::RemoveTemporaryFiles(tmpDir.ToCString(), aFileSeq.in(),
			true);

	MESSAGE("End SMESH_Gen_i::Save");

	return aStreamFile._retn();
}

SALOMEDS::TMPFile *
	SMESH_Gen_i::SaveASCII(SALOMEDS::SComponent_ptr theComponent,
	const char *theURL, bool isMultiFile)
{
	SALOMEDS::TMPFile_var aStreamFile = Save(theComponent, theURL, isMultiFile);
	return aStreamFile._retn();
}

void SMESH_Gen_i::loadHypothesis(char * name, HDFfile * hdf_file,
	char * hypofile, int studyId)
{
	char name_of_group[HDF_NAME_MAX_LEN + 1];
	char objectId[10];
	double length, maxElementsArea, maxElementsVolume;
	int numberOfSegments;

	HDFgroup * hdfGroup = new HDFgroup(name, hdf_file);
	hdfGroup->OpenOnDisk();

	hdfGroup->InternalObjectIndentify(0, name_of_group);
	HDFdataset * dataset = new HDFdataset(name_of_group, hdfGroup);
	dataset->OpenOnDisk();
	char *name_of_file = new char[dataset->GetSize()];
	dataset->ReadFromDisk(name_of_file);
	SCRUTE(name_of_file);
	dataset->CloseOnDisk();
	hdfGroup->CloseOnDisk();
	delete[]name_of_file;
	
	ifstream loadedFile(hypofile);
	while (!loadedFile.eof())
	{
		int hypothesisID;
		string hypothesisName;
		loadedFile >> hypothesisID;
		loadedFile >> hypothesisName;
		if(hypothesisName.length()==0) break;
		SMESH_Hypothesis_i * corbaHyp =
			_hypothesisFactory_i.Create(hypothesisName.c_str(), studyId, &_impl);
		SMESH_Hypothesis * localHyp = corbaHyp->getImpl();
		localHyp->SetID(hypothesisID);
		localHyp->LoadFrom(loadedFile);
		
		SMESH::SMESH_Hypothesis_var varHyp = corbaHyp->_this();
		string iorString = _orb->object_to_string(varHyp);
		sprintf(objectId, "%ld", varHyp->GetId());
		_SMESHCorbaObj[string("Hypo_") + string(objectId)] =
			iorString;
	}
	MESSAGE("End of Hypos Load");
}

void SMESH_Gen_i::loadAlgorithms(char * name, HDFfile * hdf_file,
	char* algofile, int studyId)
{
	char name_of_group[HDF_NAME_MAX_LEN + 1];
	char objectId[10];
	HDFgroup * hdfGroup = new HDFgroup(name, hdf_file);
	hdfGroup->OpenOnDisk();

	hdfGroup->InternalObjectIndentify(0, name_of_group);
	HDFdataset * dataset =
		new HDFdataset(name_of_group, hdfGroup);
	dataset->OpenOnDisk();

	char *name_of_file = new char[dataset->GetSize()];
	dataset->ReadFromDisk(name_of_file);
	dataset->CloseOnDisk();
	hdfGroup->CloseOnDisk();	
	delete[]name_of_file;

	char * aLine = new char[100];
	FILE * loadedFile = fopen(algofile, "r");
	while (!feof(loadedFile))
	{
		int hypothesisID;
		fscanf(loadedFile, "%i", &hypothesisID);
		fscanf(loadedFile, "%s\n", aLine);
		//SCRUTE(aLine);
		if (strcmp(aLine, "") != 0)
		{
			SMESH_Hypothesis_i * corbaHyp =
				_hypothesisFactory_i.Create(aLine, studyId, &_impl);
			SMESH_Hypothesis * localHyp = corbaHyp->getImpl();
			localHyp->SetID(hypothesisID);
			
			SMESH::SMESH_Hypothesis_var myHyp = corbaHyp->_this();
			
			SMESH::SMESH_Algo_var myAlgo =
				SMESH::SMESH_Algo::_narrow(myHyp);
			string iorString = _orb->object_to_string(myAlgo);
			sprintf(objectId, "%ld", myAlgo->GetId());
			_SMESHCorbaObj[string("Hypo_") + string(objectId)] =
				iorString;
		}
	}
	fclose(loadedFile);
	delete[]aLine;
	aLine = 0;
	MESSAGE("End of Algos Load");
}

/**
 * @param hdfGroupMeshId The group where to read the hypothesis
 * @param _found ???
 * @param Study The study where to create the hypothesis
 * @param myNewMesh The mesh on which the hypothesis is applied
 * @param aShape The shape with which this mesh is linked
 */
void SMESH_Gen_i::loadAppliedHypothesis(HDFgroup * hdfGroupMeshId, 
	bool _found, SALOMEDS::Study_var Study, SMESH::SMESH_Mesh_var myNewMesh, 
	GEOM::GEOM_Shape_var aShape)
{
	HDFgroup * hdf_subgroup = new HDFgroup("Applied Hypothesis", hdfGroupMeshId);
	hdf_subgroup->OpenOnDisk();
	int nb_datasets = hdf_subgroup->nInternalObjects();
	char name_dataset[10];
	
	for (int j = 0; j < nb_datasets; j++)
	{
		sprintf(name_dataset, "Hyp %d", j);
		HDFdataset * hdf_dataset = new HDFdataset(name_dataset, hdf_subgroup);
		hdf_dataset->OpenOnDisk();

		char *refFromFile = new char[hdf_dataset->GetSize()];
		hdf_dataset->ReadFromDisk(refFromFile);
		hdf_dataset->CloseOnDisk();
		delete hdf_dataset;

		if (_found)
		{
			SALOMEDS::SObject_var HypSO =
				Study->FindObjectID(refFromFile);
			if (!CORBA::is_nil(HypSO))
			{
				SALOMEDS::GenericAttribute_var anAttr;
				HypSO->FindAttribute(anAttr, "AttributeIOR");
				SALOMEDS::AttributeIOR_var anIOR =
					SALOMEDS::AttributeIOR::_narrow(anAttr);
				if (!CORBA::is_nil(anIOR))
				{
					char *HypIOR = anIOR->Value();
					SMESH::SMESH_Hypothesis_var anHyp =
						SMESH::SMESH_Hypothesis::_narrow(_orb->
						string_to_object(HypIOR));
					if (!CORBA::is_nil(anHyp))
					{
						myNewMesh->AddHypothesis(aShape, anHyp);
						MESSAGE("Hypothesis added ...");
					}
				}
			}
		}
	}
	hdf_subgroup->CloseOnDisk();
}

/**
 * @param hdfGroupMeshId The group where to read the hypothesis
 * @param _found ???
 * @param Study The study where to create the hypothesis
 * @param myNewMesh The mesh on which the hypothesis is applied
 * @param aShape The shape with which this mesh is linked
 */
void SMESH_Gen_i::loadAppliedAlgorithms(HDFgroup * hdfGroupMeshId, 
	bool _found, SALOMEDS::Study_var Study, SMESH::SMESH_Mesh_var myNewMesh, 
	GEOM::GEOM_Shape_var aShape)
{
	HDFgroup * hdf_subgroup = new HDFgroup("Applied Algorithms", hdfGroupMeshId);
	hdf_subgroup->OpenOnDisk();

	int nb_datasets = hdf_subgroup->nInternalObjects();
	SCRUTE(nb_datasets);
	char name_dataset[10];
	
	for (int j = 0; j < nb_datasets; j++)
	{
		sprintf(name_dataset, "Algo %d", j);
		HDFdataset * dataset =
			new HDFdataset(name_dataset, hdf_subgroup);
		dataset->OpenOnDisk();

		char *refFromFile = new char[dataset->GetSize()];
		dataset->ReadFromDisk(refFromFile);
		dataset->CloseOnDisk();
		delete dataset;

		if (_found)
		{
			SALOMEDS::SObject_var AlgoSO =
				Study->FindObjectID(refFromFile);
			if (!CORBA::is_nil(AlgoSO))
			{
				SALOMEDS::GenericAttribute_var anAttr;
				AlgoSO->FindAttribute(anAttr, "AttributeIOR");
				SALOMEDS::AttributeIOR_var anIOR =
					SALOMEDS::AttributeIOR::_narrow(anAttr);
				if (!CORBA::is_nil(anIOR))
				{
					char *AlgoIOR = anIOR->Value();
					//SCRUTE(AlgoIOR);
					SMESH::SMESH_Hypothesis_var myHyp =
						SMESH::SMESH_Hypothesis::_narrow(_orb->
						string_to_object(AlgoIOR));
					SMESH::SMESH_Algo_var anAlgo =
						SMESH::SMESH_Algo::_narrow(myHyp);
											
					if (!CORBA::is_nil(anAlgo))
					{
						myNewMesh->AddHypothesis(aShape, anAlgo);	//essayer avec _SMESHCorbaObj
						MESSAGE("Algorithms added ...");
					}
				}
			}
		}
	}
	hdf_subgroup->CloseOnDisk();
}

/**
 * @param hdfGroupMeshId The group where to read the hypothesis
 * @param msgname ???
 * @param Study The study where to create the hypothesis
 * @param myNewMesh The mesh on which the hypothesis is applied
 */
void SMESH_Gen_i::loadSubMeshes(HDFgroup * hdfGroupMeshId, char * msgname,
	SALOMEDS::Study_var Study, SMESH::SMESH_Mesh_var myNewMesh)
{
	MESSAGE("SMESH_Gen_i::loadSubMeshes");
	HDFgroup * hdf_subgroupmyLevel1Tag = new HDFgroup(msgname, hdfGroupMeshId);
	hdf_subgroupmyLevel1Tag->OpenOnDisk();

	int nb_submeshes = hdf_subgroupmyLevel1Tag->nInternalObjects();
	char name_meshgroup[30];
	//SCRUTE(nb_submeshes);

	for (int j = 0; j < nb_submeshes; j++)
	{
		//cmpt_sm++;
		//myTag = 10 + cmpt_sm;
		hdf_subgroupmyLevel1Tag->InternalObjectIndentify(j, name_meshgroup);				

		HDFgroup * hdf_subgroupmyTag = new HDFgroup(name_meshgroup, 
			hdf_subgroupmyLevel1Tag);
		hdf_subgroupmyTag->OpenOnDisk();
		int subMeshId = atoi((string(name_meshgroup).substr(8, 18)).c_str());

		MESSAGE("Ref on shape");
		//********** ref on shape		
		HDFdataset * hdf_dataset =
			new HDFdataset("Ref on shape", hdf_subgroupmyTag);
		hdf_dataset->OpenOnDisk();

		char *refFromFile = new char[hdf_dataset->GetSize()];
		hdf_dataset->ReadFromDisk(refFromFile);
		hdf_dataset->CloseOnDisk();
		delete hdf_dataset;

		bool _found3 = false;
		SALOMEDS::SObject_var GSO = Study->FindObjectID(refFromFile);
		SMESH::SMESH_subMesh_var aSubMesh;
		GEOM::GEOM_Shape_var aSubShape;

		if (!CORBA::is_nil(GSO))
		{
			SALOMEDS::GenericAttribute_var anAttr;
			GSO->FindAttribute(anAttr, "AttributeIOR");
			SALOMEDS::AttributeIOR_var anIOR = 
				SALOMEDS::AttributeIOR::_narrow(anAttr);
				
			char *SubShapeIOR = anIOR->Value();
			aSubShape =
				GEOM::GEOM_Shape::_narrow(_orb-> string_to_object(SubShapeIOR));

			if (!CORBA::is_nil(aSubShape))
			{
				aSubMesh = myNewMesh->GetElementsOnShape(aSubShape);
				string iorString = _orb->object_to_string(aSubMesh);
				char objectId[10];
				sprintf(objectId, "%d", subMeshId);
				_SMESHCorbaObj[string("SubMesh_") + string(objectId)] =
					 iorString;
				_found3 = true;
			}
		}

		int nb_subgroup = hdf_subgroupmyTag->nInternalObjects();
		SCRUTE(nb_subgroup);
		char sgname[HDF_NAME_MAX_LEN + 1];
		for (int k = 0; k < nb_subgroup; k++)
		{
			hdf_subgroupmyTag->InternalObjectIndentify(k, sgname);
			if (strcmp(sgname, "Ref on shape") == 0)
			{
				//nothing
			}
			else if (strcmp(sgname, "Applied Hypothesis") == 0)
			{
				//********** ref on applied hypothesis
				MESSAGE("Applied Hypothesis");
				strcpy(name_meshgroup, "Applied Hypothesis");
				//cmpt_sm++;
				HDFgroup * hdf_subgroup10cmpt_sm = new HDFgroup(name_meshgroup,
					hdf_subgroupmyTag);
					
				hdf_subgroup10cmpt_sm->OpenOnDisk();
				int nb_datasets = hdf_subgroup10cmpt_sm->nInternalObjects();
				// SCRUTE(nb_datasets);

				char name_dataset[30];
				for (int l = 0; l < nb_datasets; l++)
				{
					sprintf(name_dataset, "Hyp %d", l);
					HDFdataset * hdf_datasetcmpt_ds =
						new HDFdataset(name_dataset, hdf_subgroup10cmpt_sm);
					hdf_datasetcmpt_ds->OpenOnDisk();
					int size = hdf_datasetcmpt_ds->GetSize();

					char *refFromFile = new char[size];
					hdf_datasetcmpt_ds->ReadFromDisk(refFromFile);
					hdf_datasetcmpt_ds->CloseOnDisk();
					//cmpt_ds++;

					if (_found3)
					{
						SALOMEDS::SObject_var HypSO =
							Study->FindObjectID(refFromFile);
						if (!CORBA::is_nil(HypSO))
						{
							SALOMEDS::GenericAttribute_var anAttr;
							SALOMEDS::AttributeIOR_var anIOR;
							HypSO->FindAttribute(anAttr, "AttributeIOR");
							anIOR = SALOMEDS::AttributeIOR:: _narrow(anAttr);
							if (!CORBA::is_nil(anIOR))
							{
								char *HypIOR = anIOR->Value();
								SMESH::SMESH_Hypothesis_var anHyp =
									SMESH::SMESH_Hypothesis::_narrow(_orb->
									string_to_object(HypIOR));
								if (!CORBA::is_nil(anHyp))
								{
									SMESH::SMESH_Mesh_var aMesh = 
										aSubMesh->GetFather();
									aMesh->AddHypothesis(aSubShape, anHyp); //essayer avec _SMESHCorbaObj
									MESSAGE("Hypothesis added ...");
								}
							}
						}
					}
				}
			}
			else if (strcmp(sgname, "Applied Algorithms") == 0)
			{
				//********** ref on applied algorithms
				MESSAGE("Applied Algorithms");
				strcpy(name_meshgroup, "Applied Algorithms");
				//cmpt_sm++;
				HDFgroup * hdf_subgroup10cmpt_sm = new HDFgroup(name_meshgroup,
					hdf_subgroupmyTag);
				hdf_subgroup10cmpt_sm->OpenOnDisk();
				int nb_datasets = hdf_subgroup10cmpt_sm->nInternalObjects();
				SCRUTE(nb_datasets);

				char name_dataset[30];
				for (int l = 0; l < nb_datasets; l++)
				{
					sprintf(name_dataset, "Algo %d", l);
					HDFdataset * hdf_datasetcmpt_ds = new HDFdataset(
						name_dataset, hdf_subgroup10cmpt_sm);
					hdf_datasetcmpt_ds->OpenOnDisk();
					int size = hdf_datasetcmpt_ds->GetSize();

					char *refFromFile = new char[size];
					hdf_datasetcmpt_ds->ReadFromDisk(refFromFile);
					hdf_datasetcmpt_ds->CloseOnDisk();
					delete hdf_datasetcmpt_ds;
					//cmpt_ds++;

					if (_found3)
					{
						SALOMEDS::SObject_var AlgoSO =
							Study->FindObjectID(refFromFile);
						if (!CORBA::is_nil(AlgoSO))
						{
							SALOMEDS::GenericAttribute_var anAttr;
							SALOMEDS::AttributeIOR_var anIOR;
							AlgoSO->FindAttribute(anAttr, "AttributeIOR");
							anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
							if (!CORBA::is_nil(anIOR))
							{
								char *AlgoIOR = anIOR->Value();
								//SCRUTE(AlgoIOR);
								SMESH::SMESH_Hypothesis_var myHyp =
									SMESH::SMESH_Hypothesis::_narrow(_orb->
									string_to_object(AlgoIOR));
								SMESH::SMESH_Algo_var anAlgo =
									SMESH::SMESH_Algo::_narrow(myHyp);
								//SMESH::SMESH_Algo_var anAlgo = SMESH::SMESH_Algo::_narrow(_orb->string_to_object(AlgoIOR));
								if (!CORBA::is_nil(anAlgo))
								{
									SMESH::SMESH_Mesh_var aMesh =
										aSubMesh->GetFather();
									aMesh->AddHypothesis(aSubShape, anAlgo);	//essayer avec _SMESHCorbaObj
									MESSAGE("Algorithms added ...");
								}
							}
						}
					}
				}
			}
		}
		hdf_subgroupmyTag->CloseOnDisk();
		delete hdf_subgroupmyTag;
	}
	hdf_subgroupmyLevel1Tag->CloseOnDisk();
	delete hdf_subgroupmyLevel1Tag;
}

GEOM::GEOM_Gen_var SMESH_Gen_i::getGeomEngine()
{
	MESSAGE("SMESH_Gen_i::getGeomEngine");
	SALOME_NamingService *_NS = SINGLETON_ < SALOME_NamingService >::Instance();
	ASSERT(SINGLETON_ < SALOME_NamingService >::IsAlreadyExisting());
	_NS->init_orb(_orb);
	SALOME_LifeCycleCORBA *myEnginesLifeCycle = new SALOME_LifeCycleCORBA(_NS);
	Engines::Component_var geomEngine =
	myEnginesLifeCycle->FindOrLoad_Component("FactoryServer", "GEOM");
	GEOM::GEOM_Gen_var myGeomEngine = GEOM::GEOM_Gen::_narrow(geomEngine);
	return myGeomEngine;	
}

GEOM::GEOM_Shape_var SMESH_Gen_i::getShape(SALOMEDS::Study_var Study, char * refFromFile)
{
	MESSAGE("SMESH_Gen_i::getShape("<<Study<<","<<refFromFile<<")");
	SCRUTE(CORBA::is_nil(Study));
	SALOMEDS::SObject_var CSO = Study->FindObjectID(refFromFile);
	GEOM::GEOM_Shape_var aShape;

	if (!CORBA::is_nil(CSO))
	{
		SALOMEDS::GenericAttribute_var anAttr;
		CSO->FindAttribute(anAttr, "AttributeIOR");
		if (!CORBA::is_nil(CSO))
		{
			MESSAGE("The shape was not loaded. Try to load it.");
			SALOMEDS::Driver_var driver = SALOMEDS::Driver::_narrow(getGeomEngine());
			SALOMEDS::SComponent_var SCO = SALOMEDS::SComponent::_narrow(Study->FindObject("Geometry"));
			SALOMEDS::StudyBuilder_var B = Study->NewBuilder();	
			B->LoadWith(SCO,driver);
			CSO->FindAttribute(anAttr, "AttributeIOR");
		}

		SALOMEDS::AttributeIOR_var anIOR =
			SALOMEDS::AttributeIOR::_narrow(anAttr);
		
		char *ShapeIOR = anIOR->Value();
		aShape =
			GEOM::GEOM_Shape::_narrow(_orb->string_to_object(ShapeIOR));
	}
	return aShape;
}

void SMESH_Gen_i::loadMesh(char * name, HDFfile * hdf_file,
	char* meshfile, SALOMEDS::Study_var Study)
{
	MESSAGE("in mesh load");
	char msgname[HDF_NAME_MAX_LEN + 1];	
	char objectId[10];
	char name_of_group[HDF_NAME_MAX_LEN + 1];
	
	int myMeshId =
		atoi((string(name).substr(5, 5)).c_str());
	SCRUTE(myMeshId);

	HDFgroup * hdfGroupMeshId = new HDFgroup(name, hdf_file);
	hdfGroupMeshId->OpenOnDisk();

	int nb_meshsubgroup = hdfGroupMeshId->nInternalObjects();
	SCRUTE(nb_meshsubgroup);

	//********** Loading of the file name where the data are stored
	MESSAGE("Mesh data file");
	strcpy(name_of_group, "Mesh data");
	HDFdataset * dataset =
		new HDFdataset(name_of_group, hdfGroupMeshId);
	dataset->OpenOnDisk();

	char *datafilename = new char[dataset->GetSize()];
	dataset->ReadFromDisk(datafilename);
	dataset->CloseOnDisk();	
	MESSAGE("datafilename="<<datafilename<<" but this is ignored. We will read from "<<meshfile);
	datafilename=meshfile;

	//********** Loading of the reference on the shape
	//********** and mesh initialization
	MESSAGE("Ref on shape");
	strcpy(name_of_group, "Ref on shape");
	dataset =
		new HDFdataset(name_of_group, hdfGroupMeshId);
	dataset->OpenOnDisk();
	
	char *refFromFile = new char[dataset->GetSize()];
	dataset->ReadFromDisk(refFromFile);
	dataset->CloseOnDisk();	

	bool _found = false;
	SCRUTE(refFromFile);
	SMESH::SMESH_Mesh_var myNewMesh;
	GEOM::GEOM_Shape_var aShape=getShape(Study, refFromFile);

	if (!CORBA::is_nil(aShape))
	{
		_found = true;
		myNewMesh = this->Init(getGeomEngine(), Study->StudyId(), aShape);
		string iorString = _orb->object_to_string(myNewMesh);
		sprintf(objectId, "%ld", myNewMesh->GetId());
		_SMESHCorbaObj[string("Mesh_") + string(objectId)] = iorString;

		
		//********** 
		//********** Loading of mesh data
		if (strcmp(datafilename, "No data") != 0)
		{			
			StudyContext_iStruct *myStudyContext =
				_mapStudyContext_i[Study->StudyId()];
			int meshId = myNewMesh->GetId();
			SMESH_Mesh_i *meshServant =
				myStudyContext->mapMesh_i[meshId];
			::SMESH_Mesh & myLocMesh = meshServant->GetImpl();
			SMESHDS_Mesh *mySMESHDSMesh = myLocMesh.GetMeshDS();

			Mesh_Reader *myReader = SMESHDriver::GetMeshReader("MED");
			myReader->SetMesh(mySMESHDSMesh);
			myReader->SetMeshId(myMeshId);
			myReader->SetFile(datafilename);
			myReader->Read();
			mySMESHDSMesh->logFullUpdate();
			MESSAGE("Loaded a mesh with " << mySMESHDSMesh->NbNodes() <<" nodes");
		}
	}
	//********** 
	//}
	//else if (strcmp(msgname,"Applied Hypothesis")==0) {
	for (int ii = 0; ii < nb_meshsubgroup; ii++)
	{
		hdfGroupMeshId->InternalObjectIndentify(ii, msgname);
		if (strcmp(msgname, "Mesh data") == 0)
		{
			//nothing
		}
		else if (strcmp(msgname, "Ref on shape") == 0)
		{
			//nothing
		}
		else if (strcmp(msgname, "Applied Hypothesis") == 0)
		{
			loadAppliedHypothesis(hdfGroupMeshId, _found, Study, myNewMesh,
				aShape);
		}
		else if (strcmp(msgname, "Applied Algorithms") == 0)
		{
			loadAppliedAlgorithms(hdfGroupMeshId, _found, Study, myNewMesh,
				aShape);
		}
		else if (string(msgname).substr(0, 9) == string("SubMeshes"))
		{
			loadSubMeshes(hdfGroupMeshId, msgname, Study, myNewMesh);
		}
	}
	hdfGroupMeshId->CloseOnDisk();
	MESSAGE("End of Meshes Load");
}

/**
 * Load the part of the study related to SMESH.
 */
bool SMESH_Gen_i::Load(SALOMEDS::SComponent_ptr theComponent,
	const SALOMEDS::TMPFile & theStream, const char *theURL, bool isMultiFile)
{
	MESSAGE("SMESH_Gen_i::Load");	
	// Get temporary files location
	TCollection_AsciiString tmpDir =
		isMultiFile ? TCollection_AsciiString((char *)theURL) : SALOMEDS_Tool::
		GetTmpDir();

	// Convert the stream into sequence of files to process
	SALOMEDS::ListOfFileNames_var aFileSeq =
		SALOMEDS_Tool::PutStreamToFiles(theStream, tmpDir.ToCString(),
		isMultiFile);

	TCollection_AsciiString aStudyName("");
	if (isMultiFile)
		aStudyName =
			(SALOMEDS_Tool::GetNameFromPath(theComponent->GetStudy()->URL()));

	// Set names of temporary files
	TCollection_AsciiString filename =
		tmpDir + aStudyName + TCollection_AsciiString("_SMESH.hdf");
	TCollection_AsciiString hypofile =
		tmpDir + aStudyName + TCollection_AsciiString("_SMESH_Hypo.txt");
	TCollection_AsciiString algofile =
		tmpDir + aStudyName + TCollection_AsciiString("_SMESH_Algo.txt");
	TCollection_AsciiString meshfile =
		tmpDir + aStudyName + TCollection_AsciiString("_SMESH_Mesh.med");

	SALOMEDS::Study_var Study = theComponent->GetStudy();
	int studyId = Study->StudyId();
	SCRUTE(studyId);

	SALOMEDS::AttributeName_var aName;
	
	SALOMEDS::SComponent_var fathergeom = Study->FindComponent("GEOM");
	SALOMEDS::SComponent_var myGeomSComp =
		SALOMEDS::SComponent::_narrow(fathergeom);
	SCRUTE(fathergeom);

	char name[HDF_NAME_MAX_LEN + 1];
	map < int, HDFgroup * > hdf_subgroup;	


	//************* HDF file opening
	HDFfile *hdf_file = new HDFfile(filename.ToCString());
	try
	{
		hdf_file->OpenOnDisk(HDF_RDONLY);
	}
	catch(HDFexception)
	{
		MESSAGE("Load(): " << filename << " not found!");
		return false;
	}

	//****************************

	int nb_group = hdf_file->nInternalObjects();
	SCRUTE(nb_group);
	for (int i = 0; i < nb_group; i++)
	{
		hdf_file->InternalObjectIndentify(i, name);
		if (strcmp(name, "Hypothesis") == 0)
			loadHypothesis(name, hdf_file, hypofile.ToCString(), studyId);
		else if (strcmp(name, "Algorithms") == 0)
			loadAlgorithms(name, hdf_file, algofile.ToCString(), studyId);
		else if (string(name).substr(0, 4) == string("Mesh"))
			loadMesh(name, hdf_file, meshfile.ToCString(), Study);
	}

	MESSAGE("End of SMESH_Gen::Load");

	hdf_file->CloseOnDisk();

	// Remove temporary files created from the stream
	if (isMultiFile)
		SALOMEDS_Tool::RemoveTemporaryFiles(tmpDir.ToCString(), aFileSeq.in(),
			true);

	return true;
}

bool SMESH_Gen_i::LoadASCII(SALOMEDS::SComponent_ptr theComponent,
	const SALOMEDS::TMPFile & theStream, const char *theURL, bool isMultiFile)
{
	return Load(theComponent, theStream, theURL, isMultiFile);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Gen_i::Close(SALOMEDS::SComponent_ptr theComponent)
{
	MESSAGE("Close");
	SALOMEDS::Study_var aStudy = theComponent->GetStudy();
	SALOMEDS::ChildIterator_var itBig = aStudy->NewChildIterator(theComponent);
	for (; itBig->More(); itBig->Next())
	{
		SALOMEDS::SObject_var gotBranch = itBig->Value();

		// branch 1 : hypothesis
		if (gotBranch->Tag() == Tag_HypothesisRoot ||
			gotBranch->Tag() == Tag_AlgorithmsRoot)
		{
			SALOMEDS::ChildIterator_var it =
				aStudy->NewChildIterator(gotBranch);
			for (; it->More(); it->Next())
			{
				SALOMEDS::SObject_var mySObject = it->Value();
				SALOMEDS::GenericAttribute_var anAttr;
				SALOMEDS::AttributeIOR_var anIOR;
				if (mySObject->FindAttribute(anAttr, "AttributeIOR"))
				{
					anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
					SMESH::SMESH_Hypothesis_var myHyp =
						SMESH::SMESH_Hypothesis::_narrow(_orb->
						string_to_object(anIOR->Value()));
					char objectId[10];
					sprintf(objectId, "%ld", myHyp->GetId());
//        cout<<"********** delete Hyp "<<objectId<<endl;
					_SMESHCorbaObj.erase(string("Hypo_") + string(objectId));
					myHyp =
						SMESH::SMESH_Hypothesis::_narrow(_orb->
						string_to_object(anIOR->Value()));
				}
			}
		}
		// branch 2 : algorithms
		else if (gotBranch->Tag() >= 3)
		{
			SALOMEDS::ChildIterator_var it =
				aStudy->NewChildIterator(gotBranch);
			for (; it->More(); it->Next())
			{
				SALOMEDS::SObject_var mySObject = it->Value();
				SALOMEDS::GenericAttribute_var anAttr;
				SALOMEDS::AttributeIOR_var anIOR;
				if (mySObject->FindAttribute(anAttr, "AttributeIOR"))
				{
					anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
					SMESH::SMESH_Mesh_var myMesh =
						SMESH::SMESH_Mesh::_narrow(_orb->
						string_to_object(anIOR->Value()));
					if (!myMesh->_is_nil())
					{
						char objectId[10];
						sprintf(objectId, "%ld", myMesh->GetId());
//          cout<<"********** delete Mesh "<<objectId<<endl;
						_SMESHCorbaObj.erase(string("Mesh_") +
							string(objectId));
						CORBA::release(myMesh);
					}
					else
					{
						SMESH::SMESH_subMesh_var mySubMesh =
							SMESH::SMESH_subMesh::_narrow(_orb->
							string_to_object(anIOR->Value()));
						if (!mySubMesh->_is_nil())
						{
							char objectId[10];
							sprintf(objectId, "%ld", mySubMesh->GetId());
//            cout<<"********** delete SubMesh "<<objectId<<endl;
							_SMESHCorbaObj.erase(string("SubMesh_") +
								string(objectId));
							CORBA::release(mySubMesh);
						}
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

char *SMESH_Gen_i::ComponentDataType()
{
	MESSAGE("SMESH_Gen_i::ComponentDataType");
	return strdup("SMESH");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

char *SMESH_Gen_i::IORToLocalPersistentID(SALOMEDS::SObject_ptr theSObject,
	const char *IORString, CORBA::Boolean isMultiFile, CORBA::Boolean isASCII)
{
	MESSAGE("SMESH_Gen_i::IORToLocalPersistentID");

	char objectId[10];

	SMESH::SMESH_Algo_var myAlgo =
		SMESH::SMESH_Algo::_narrow(_orb->string_to_object(IORString));
	if (!CORBA::is_nil(myAlgo))
	{
		string prefix = "Hypo_";
		sprintf(objectId, "%ld", myAlgo->GetId());
		string lpID = prefix + string(objectId);
		return CORBA::string_dup(lpID.c_str());
	}
	else
	{
		SMESH::SMESH_Hypothesis_var myHypo =
			SMESH::SMESH_Hypothesis::_narrow(_orb->string_to_object(IORString));
		if (!CORBA::is_nil(myHypo))
		{
			string prefix = "Hypo_";
			sprintf(objectId, "%ld", myHypo->GetId());
			string lpID = prefix + string(objectId);
			return CORBA::string_dup(lpID.c_str());
		}
		else
		{
			SMESH::SMESH_Mesh_var myMesh =
				SMESH::SMESH_Mesh::_narrow(_orb->string_to_object(IORString));
			if (!CORBA::is_nil(myMesh))
			{
				string prefix = "Mesh_";
				sprintf(objectId, "%ld", myMesh->GetId());
				string lpID = prefix + string(objectId);
				return CORBA::string_dup(lpID.c_str());
			}
			else
			{
				SMESH::SMESH_subMesh_var mySubMesh =
					SMESH::SMESH_subMesh::_narrow(_orb->
					string_to_object(IORString));
				if (!CORBA::is_nil(mySubMesh))
				{
					string prefix = "SubMesh_";
					sprintf(objectId, "%ld", mySubMesh->GetId());
					string lpID = prefix + string(objectId);
					return CORBA::string_dup(lpID.c_str());
				}
				else
					return (strdup("no object"));
			}
		}
	}
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

char *SMESH_Gen_i::LocalPersistentIDToIOR(SALOMEDS::SObject_ptr theSObject,
	const char *aLocalPersistentID,
	CORBA::Boolean isMultiFile, CORBA::Boolean isASCII)
{
	MESSAGE("SMESH_Gen_i::LocalPersistentIDToIOR");
	SCRUTE(aLocalPersistentID);
	string clef = string(aLocalPersistentID);
	SCRUTE(_SMESHCorbaObj[clef].c_str());
	return CORBA::string_dup(_SMESHCorbaObj[clef].c_str());
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_topo *SMESH_Gen_i::ExploreMainShape(GEOM::GEOM_Gen_ptr geomEngine,
	CORBA::Long studyId, GEOM::GEOM_Shape_ptr aShape)
{
	MESSAGE("SMESH_Mesh_i::ExploreMainShape");
	// _narrow() duplicates the reference and check the type
	GEOM::GEOM_Gen_var geom = GEOM::GEOM_Gen::_narrow(geomEngine);
	GEOM::GEOM_Shape_var myShape = GEOM::GEOM_Shape::_narrow(aShape);

	if (CORBA::is_nil(geom))
		THROW_SALOME_CORBA_EXCEPTION("bad geom reference", SALOME::BAD_PARAM);
	if (CORBA::is_nil(myShape))
		THROW_SALOME_CORBA_EXCEPTION("bad shape reference", SALOME::BAD_PARAM);
	MESSAGE("---");
	SCRUTE(myShape->Name());
	geom->GetCurrentStudy(studyId);
	SCRUTE(studyId);
	TopoDS_Shape mainShape = _ShapeReader->GetShape(geom, myShape);
	MESSAGE("---");

	// create an SMESH_topo object for the mainShape

	SMESH_topo *myTopo = new SMESH_topo();
	MESSAGE("---");

	// explore local TopoDS_Shape, store reference of local TopoDS subShapes

	for (TopExp_Explorer exp(mainShape, TopAbs_COMPOUND); exp.More();
		exp.Next())
	{
		const TopoDS_Compound & E = TopoDS::Compound(exp.Current());
		int i = myTopo->_myShapes[TopAbs_COMPOUND].Add(E);
		SCRUTE(i);
	}
	for (TopExp_Explorer exp(mainShape, TopAbs_COMPSOLID); exp.More();
		exp.Next())
	{
		const TopoDS_CompSolid & E = TopoDS::CompSolid(exp.Current());
		int i = myTopo->_myShapes[TopAbs_COMPSOLID].Add(E);
		SCRUTE(i);
	}
	for (TopExp_Explorer exp(mainShape, TopAbs_SOLID); exp.More(); exp.Next())
	{
		const TopoDS_Solid & E = TopoDS::Solid(exp.Current());
		int i = myTopo->_myShapes[TopAbs_SOLID].Add(E);
		SCRUTE(i);
	}
	for (TopExp_Explorer exp(mainShape, TopAbs_SHELL); exp.More(); exp.Next())
	{
		const TopoDS_Shell & E = TopoDS::Shell(exp.Current());
		int i = myTopo->_myShapes[TopAbs_SHELL].Add(E);
		SCRUTE(i);
	}
	for (TopExp_Explorer exp(mainShape, TopAbs_FACE); exp.More(); exp.Next())
	{
		const TopoDS_Face & E = TopoDS::Face(exp.Current());
		int i = myTopo->_myShapes[TopAbs_FACE].Add(E);
		SCRUTE(i);
	}
	for (TopExp_Explorer exp(mainShape, TopAbs_WIRE); exp.More(); exp.Next())
	{
		const TopoDS_Wire & E = TopoDS::Wire(exp.Current());
		int i = myTopo->_myShapes[TopAbs_WIRE].Add(E);
		SCRUTE(i);
	}
	for (TopExp_Explorer exp(mainShape, TopAbs_EDGE); exp.More(); exp.Next())
	{
		const TopoDS_Edge & E = TopoDS::Edge(exp.Current());
		int i = myTopo->_myShapes[TopAbs_EDGE].Add(E);
		SCRUTE(i);
	}
	for (TopExp_Explorer exp(mainShape, TopAbs_VERTEX); exp.More(); exp.Next())
	{
		const TopoDS_Vertex & E = TopoDS::Vertex(exp.Current());
		int i = myTopo->_myShapes[TopAbs_VERTEX].Add(E);
		SCRUTE(i);
	}

	// explore subShapes of distant CORBA object,
	// associate distant CORBA subShape references
	// with local reference to local TopoDS subShape

	string filenode = "toposhape.txt";
	ofstream fic(filenode.c_str());

	for (int shapeType = TopAbs_COMPOUND; shapeType < TopAbs_SHAPE; shapeType++)
	{
		fic << "shape type : " << SMESH_shapeTypeNames[shapeType];

		GEOM::GEOM_Gen::ListOfGeomShapes_var subShapes
			= geom->SubShapeAll(myShape, shapeType);
		int nbSubShapes = subShapes->length();
		int nbLocal = myTopo->_myShapes[shapeType].Extent();
		fic << " - number of elements: " << nbSubShapes << endl;
		ASSERT(nbSubShapes == nbLocal);

		for (int i = 0; i < nbSubShapes; i++)
		{
			GEOM::GEOM_Shape_var aSubShape = subShapes[i];
			string idShape = SMESH_topo::GetShapeLocalId(aSubShape);
			fic << "  " << idShape;
			SCRUTE(idShape);
			TopoDS_Shape aLocShape = _ShapeReader->GetShape(geom, aSubShape);
			for (int j = 1; j <= nbLocal; j++)
				if (aLocShape.IsSame(myTopo->_myShapes[shapeType].FindKey(j)))
				{
					MESSAGE("  --- trouve = " << j);
					myTopo->_mapIndShapes[shapeType][idShape] = j;
					fic << "  --- trouve = " << j;
					break;
				}
			fic << endl;
		}
	}
	fic.close();

	return myTopo;
}

/**
 * Import a mesh from a file
 * @param fileName file name to be imported
 * @param fileType Currently it could be either "DAT", "UNV" or "MED".
 */
SMESH::SMESH_Mesh_ptr SMESH_Gen_i::Import(CORBA::Long studyId,
	const char *fileName, const char *fileType)
{
	MESSAGE("SMESH_Gen_I::Import");
	SMESH_Mesh_i *meshServant;
	try
	{
		if (_mapStudyContext_i.find(studyId) == _mapStudyContext_i.end())
		{
			_mapStudyContext_i[studyId] = new StudyContext_iStruct;
		}
		StudyContext_iStruct *myStudyContext = _mapStudyContext_i[studyId];

		// create a new mesh object servant, store it in a map in study context
		meshServant = new SMESH_Mesh_i(this, NULL, studyId, _localId);
		myStudyContext->mapMesh_i[_localId] = meshServant;
		_localId++;

		// create a new mesh object
		meshServant->SetImpl(_impl.Import(studyId, fileName, fileType));
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}

	// activate the CORBA servant of Mesh

	SMESH::SMESH_Mesh_var mesh
		= SMESH::SMESH_Mesh::_narrow(meshServant->_this());

	meshServant->SetIor(mesh);
	return SMESH::SMESH_Mesh::_duplicate(mesh);
}

//=============================================================================
/*! 
 * C factory, accessible with dlsym, after dlopen  
 */
//=============================================================================

extern "C"
{
	PortableServer::ObjectId * SMESHEngine_factory(CORBA::ORB_ptr orb,
		PortableServer::POA_ptr poa,
		PortableServer::ObjectId * contId,
		const char *instanceName, const char *interfaceName)
	{
		MESSAGE("PortableServer::ObjectId * SMESHEngine_factory()");
		SCRUTE(interfaceName);
		SMESH_Gen_i *mySMESH_Gen
			= new SMESH_Gen_i(orb, poa, contId, instanceName, interfaceName);
		return mySMESH_Gen->getId();
	}
}
