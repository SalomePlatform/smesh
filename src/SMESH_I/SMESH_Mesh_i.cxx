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
//  File   : SMESH_Mesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_Mesh_i.hxx"
#include "SMESH_subMesh_i.hxx"
#include "SMESH_MEDMesh_i.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include "SALOME_NamingService.hxx"
#include "Utils_SINGLETON.hxx"
#include "OpUtil.hxx"

#include "TCollection_AsciiString.hxx"
#include "SMESHDS_Command.hxx"
#include "SMESHDS_CommandType.hxx"
#include "SMESH_MeshEditor_i.hxx"

#include <string>
#include <iostream>

//**** SMESHDS en champ

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Mesh_i::SMESH_Mesh_i()
{
	MESSAGE("SMESH_Mesh_i: default constructor, not for use");
	ASSERT(0);
};

//=============================================================================
/*!
 *  Constructor
 */
//=============================================================================

SMESH_Mesh_i::SMESH_Mesh_i(SMESH_Gen_i * gen_i,
	GEOM::GEOM_Gen_ptr geomEngine, CORBA::Long studyId, int localId)
{
	MESSAGE("SMESH_Mesh_i");
	_gen_i = gen_i;
	_id = localId;
	_geom = GEOM::GEOM_Gen::_narrow(geomEngine);
	_studyId = studyId;
}

//=============================================================================
/*!
 *  Destructor
 */
//=============================================================================

SMESH_Mesh_i::~SMESH_Mesh_i()
{
	MESSAGE("~SMESH_Mesh_i");
	// ****
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::AddHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
	SMESH::SMESH_Hypothesis_ptr anHyp) throw(SALOME::SALOME_Exception)
{
	MESSAGE("AddHypothesis");
	// **** proposer liste de subShape (selection multiple)

	GEOM::GEOM_Shape_var mySubShape = GEOM::GEOM_Shape::_narrow(aSubShape);
	if (CORBA::is_nil(mySubShape))
		THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
			SALOME::BAD_PARAM);

	SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
	if (CORBA::is_nil(myHyp))
		THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
			SALOME::BAD_PARAM);
	bool ret = false;
	try
	{
		TopoDS_Shape myLocSubShape =
			_gen_i->_ShapeReader->GetShape(_geom, mySubShape);
		int hypId = myHyp->GetId();
		ret = _impl->AddHypothesis(myLocSubShape, hypId);
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}
	return ret;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
	SMESH_Mesh_i::RemoveHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
	SMESH::SMESH_Hypothesis_ptr anHyp) throw(SALOME::SALOME_Exception)
{
	MESSAGE("RemoveHypothesis");
	// **** proposer liste de subShape (selection multiple)

	GEOM::GEOM_Shape_var mySubShape = GEOM::GEOM_Shape::_narrow(aSubShape);
	if (CORBA::is_nil(mySubShape))
		THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
			SALOME::BAD_PARAM);

	SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
	if (CORBA::is_nil(myHyp))
		THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
			SALOME::BAD_PARAM);

	bool ret = false;
	try
	{
		TopoDS_Shape myLocSubShape =
			_gen_i->_ShapeReader->GetShape(_geom, mySubShape);
		int hypId = myHyp->GetId();
		ret = _impl->RemoveHypothesis(myLocSubShape, hypId);
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}
	return ret;
};

/**
 *@TODO Not implemented
 */
SMESH::ListOfHypothesis *
	SMESH_Mesh_i::GetHypothesisList(GEOM::GEOM_Shape_ptr aSubShape)
throw(SALOME::SALOME_Exception)
{
	MESSAGE("GetHypothesisList: Not implemented");
	return NULL;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::GetElementsOnShape(GEOM::
	GEOM_Shape_ptr aSubShape) throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Mesh_i::GetElementsOnShape");
	GEOM::GEOM_Shape_var mySubShape = GEOM::GEOM_Shape::_narrow(aSubShape);
	if (CORBA::is_nil(mySubShape))
		THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
			SALOME::BAD_PARAM);

	int subMeshId = 0;
	try
	{
		SMESH_subMesh_i *subMeshServant;
		TopoDS_Shape myLocSubShape
			= _gen_i->_ShapeReader->GetShape(_geom, mySubShape);

		//Get or Create the SMESH_subMesh object implementation

		::SMESH_subMesh * mySubMesh = _impl->GetSubMesh(myLocSubShape);
		subMeshId = mySubMesh->GetId();

		// create a new subMesh object servant if there is none for the shape

		if (_mapSubMesh.find(subMeshId) != _mapSubMesh.end())
		{
			ASSERT(_mapSubMesh_i.find(subMeshId) != _mapSubMesh_i.end());
			subMeshServant = _mapSubMesh_i[subMeshId];
		}
		else
		{
			// create and activate the CORBA servant of Mesh
			subMeshServant = new SMESH_subMesh_i(_gen_i, this, subMeshId);
			SMESH::SMESH_subMesh_var subMesh
				= SMESH::SMESH_subMesh::_narrow(subMeshServant->_this());
			_mapSubMesh[subMeshId] = mySubMesh;
			_mapSubMesh_i[subMeshId] = subMeshServant;
			_mapSubMeshIor[subMeshId]
				= SMESH::SMESH_subMesh::_duplicate(subMesh);
		}
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}

	ASSERT(_mapSubMeshIor.find(subMeshId) != _mapSubMeshIor.end());
	return SMESH::SMESH_subMesh::_duplicate(_mapSubMeshIor[subMeshId]);
}

/**
 * Translate the UpdateAll SMESHDS_Command to a set of SMESH::log_command.
 * As the input log need to be resized, it is realocated.
 * @param logBlock The log where to insert created commands
 * @param index The place where to insert created commands in log. It is updated
 * with the place to put new elements.
 * @return The realocated and resized log.
 * @TODO Add support for other type of elements
 */
SMESH::log_array_var SMESH_Mesh_i::
	createUpdateAllCommand(SMESH::log_array_var log, int * index)
{
	MESSAGE("SMESH_Mesh_i::createUpdateAllCommand");
	SMESH::log_array_var aLog=new SMESH::log_array(log->length()+3);
	aLog->length(log->length()+3);
	
	for(int i=0;i<*index;i++)
	{		
		aLog[i]=log[i];
	}
	
	log->length(0);
	int id=*index;
		
	//Remove all elements
	aLog[id].commandType=SMESH::REMOVE_ALL;
	id++;
	
	//Export nodes
	aLog[id].commandType=SMESH::ADD_NODE;
	aLog[id].number=_impl->GetMeshDS()->NbNodes();

	double * nodesCoordinates=_impl->GetMeshDS()->getNodesCoordinates();
	aLog[id].coords=SMESH::double_array(
		aLog[id].number*3,
		aLog[id].number*3,
		nodesCoordinates);

	long * nodesID=_impl->GetMeshDS()->getNodesID();
	aLog[id].indexes=SMESH::long_array(
		aLog[id].number,
		aLog[id].number,
		nodesID);

	id++;
	
	MESSAGE("Export edges");
	//Export edges
	aLog[id].commandType=SMESH::ADD_EDGE;
	aLog[id].number=_impl->GetMeshDS()->NbEdges();	
	aLog[id].coords.length(0);

	long * edgesIndices=_impl->GetMeshDS()->getEdgesIndices();
	aLog[id].indexes=SMESH::long_array(
		aLog[id].number*3,
		aLog[id].number*3,
		edgesIndices);

	id++;
	
	MESSAGE("Export triangles");
	//Export triangles
	aLog[id].commandType=SMESH::ADD_TRIANGLE;
	aLog[id].number=_impl->GetMeshDS()->NbTriangles();	
	aLog[id].coords.length(0);

	long * triasIndices=_impl->GetMeshDS()->getTrianglesIndices();
	aLog[id].indexes=SMESH::long_array(
		aLog[id].number*4,
		aLog[id].number*4,
		triasIndices);

	(*index)=id;
	return aLog;
}

/**
 * Return the log of the current mesh. CORBA wrap of the SMESH::GetLog method
 * with a special treatment for SMESHDS_UpdateAll commands
 * @param clearAfterGet Tell if the log must be cleared after being returned
 * @return the log
 */
SMESH::log_array * SMESH_Mesh_i::GetLog(CORBA::Boolean clearAfterGet)
	throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Mesh_i::GetLog");

	SMESH::log_array_var aLog;
	/*try
	{*/
		list < SMESHDS_Command * >logDS = _impl->GetLog();
		aLog = new SMESH::log_array;
		int indexLog = 0;
		int lg = logDS.size();
		MESSAGE("Number of command in the log: "<<lg);
		aLog->length(lg);
		list < SMESHDS_Command * >::iterator its = logDS.begin();
		while (its != logDS.end())
		{
			SMESHDS_Command *com = *its;
			int comType = com->GetType();
			//SCRUTE(comType);
			int lgcom = com->GetNumber();
			//SCRUTE(lgcom);
			const list < int >&intList = com->GetIndexes();
			int inum = intList.size();
			//SCRUTE(inum);
			list < int >::const_iterator ii = intList.begin();
			const list < double >&coordList = com->GetCoords();
			int rnum = coordList.size();
			//SCRUTE(rnum);
			list < double >::const_iterator ir = coordList.begin();
			aLog[indexLog].commandType = comType;
			if(comType==SMESHDS_UpdateAll)
			{
				aLog=createUpdateAllCommand(aLog, &indexLog);
			}
			else
			{
				aLog[indexLog].number = lgcom;
				aLog[indexLog].coords.length(rnum);
				aLog[indexLog].indexes.length(inum);
				for (int i = 0; i < rnum; i++)
				{
					aLog[indexLog].coords[i] = *ir;
					//MESSAGE(" "<<i<<" "<<ir.Value());
					ir++;
				}
				for (int i = 0; i < inum; i++)
				{
					aLog[indexLog].indexes[i] = *ii;
					//MESSAGE(" "<<i<<" "<<ii.Value());
					ii++;
				}
				indexLog++;
			}
			its++;
		}
		if (clearAfterGet) _impl->ClearLog();
		return aLog._retn();
	/*}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}*/
}

//  SMESH::string_array* SMESH_Mesh_i::GetLog(CORBA::Boolean clearAfterGet)
//    throw (SALOME::SALOME_Exception)
//  {
//    MESSAGE("SMESH_Mesh_i::GetLog");

//    SMESH::string_array_var aLog;
//    try
//      {
//        const SMESHDS_ListOfCommand& logDS =_impl->GetLog();
//        aLog = new SMESH::string_array;
//        int logSize = 0;
//        int indexLog = 0;
//        int lg = logDS.Extent();
//        SCRUTE(lg);
//        SMESHDS_ListIteratorOfListOfCommand its(logDS);
//        while(its.More())
//        {
//      Handle(SMESHDS_Command) com = its.Value();
//      int comType = com->GetType();
//      SCRUTE(comType);
//      int lgcom = com->GetNumber();
//      SCRUTE(lgcom);
//      logSize += lgcom;
//      aLog->length(logSize);
//      SCRUTE(logSize);
//      const TColStd_ListOfInteger& intList = com->GetIndexes();
//      TColStd_ListIteratorOfListOfInteger ii(intList);
//      const TColStd_ListOfReal& coordList = com->GetCoords();
//      TColStd_ListIteratorOfListOfReal ir(coordList);
//      for (int icom = 1; icom <= lgcom; icom++)
//        {
//          ostringstream S;
//          switch (comType)
//            {
//            case SMESHDS_AddNode:
//          S << "AddNode " << ii.Value(); ii.Next();
//          S << " " << ir.Value(); ir.Next(); 
//          S << " " << ir.Value(); ir.Next();
//          S << " " << ir.Value(); ir.Next();
//          break;
//            case SMESHDS_AddEdge:
//          S << "AddEdge " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddTriangle:
//          S << "AddFace " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddQuadrangle:
//          S << "AddFace " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddTetrahedron:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddPyramid:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddPrism:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddHexahedron:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_RemoveNode:
//          S << "RemoveNode " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_RemoveElement:
//          S << "RemoveElement " << ii.Value(); ii.Next();
//          break;
//            default:
//          ASSERT(0);
//          break;
//            }
//          string ch = S.str();
//          SCRUTE(ch);
//          aLog[indexLog++] = CORBA::string_dup(ch.c_str());
//        }
//      its.Next();
//        }
//        if (clearAfterGet) _impl->ClearLog();
//      }
//    catch (SALOME_Exception& S_ex)
//      {
//        THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
//      }
//    return aLog._retn();
//  }

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::ClearLog() throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Mesh_i::ClearLog");
	// ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetId()throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Mesh_i::GetId");
	return _id;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetStudyId()throw(SALOME::SALOME_Exception)
{
	return _studyId;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::SetImpl(::SMESH_Mesh * impl)
{
	MESSAGE("SMESH_Mesh_i::SetImpl");
	_impl = impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

::SMESH_Mesh & SMESH_Mesh_i::GetImpl()
{
	MESSAGE("SMESH_Mesh_i::GetImpl()");
	return *_impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

GEOM::GEOM_Gen_ptr SMESH_Mesh_i::GetGeomEngine()
{
	MESSAGE("SMESH_Mesh_i::GetGeomEngine");
	return GEOM::GEOM_Gen::_duplicate(_geom);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::SetIor(SMESH::SMESH_Mesh_ptr myIor)
{
	MESSAGE("SMESH_Mesh_i::SetIor");
	_myIor = SMESH::SMESH_Mesh::_duplicate(myIor);
	ASSERT(!CORBA::is_nil(_myIor));
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Mesh_i::GetIor()
{
	MESSAGE("SMESH_Mesh_i::GetIor");
	ASSERT(!CORBA::is_nil(_myIor));
	return SMESH::SMESH_Mesh::_duplicate(_myIor);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_MeshEditor_ptr SMESH_Mesh_i::GetMeshEditor()
{
	SMESH_MeshEditor_i *aMeshEditor =
		new SMESH_MeshEditor_i(_impl->GetMeshDS());
	SMESH::SMESH_MeshEditor_var aMesh = aMeshEditor->_this();
	return aMesh._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SALOME_MED::MESH_ptr SMESH_Mesh_i::GetMEDMesh()throw(SALOME::SALOME_Exception)
{
	SMESH_MEDMesh_i *aMedMesh = new SMESH_MEDMesh_i(this);
	SALOME_MED::MESH_var aMesh = aMedMesh->_this();
	return aMesh._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbNodes()throw(SALOME::SALOME_Exception)
{
	return _impl->NbNodes();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbEdges()throw(SALOME::SALOME_Exception)
{
	return _impl->NbEdges();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbFaces()throw(SALOME::SALOME_Exception)
{
	return _impl->NbFaces();
}

CORBA::Long SMESH_Mesh_i::NbTriangles()throw(SALOME::SALOME_Exception)
{
	return _impl->NbTriangles();
}

CORBA::Long SMESH_Mesh_i::NbQuadrangles()throw(SALOME::SALOME_Exception)
{
	return _impl->NbQuadrangles();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbVolumes()throw(SALOME::SALOME_Exception)
{
	return _impl->NbVolumes();
}

CORBA::Long SMESH_Mesh_i::NbTetras()throw(SALOME::SALOME_Exception)
{
	return _impl->NbTetras();
}

CORBA::Long SMESH_Mesh_i::NbHexas()throw(SALOME::SALOME_Exception)
{
	return _impl->NbHexas();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbSubMesh()throw(SALOME::SALOME_Exception)
{
	return _impl->NbSubMesh();
}

/*!
 * Export mesh to a file
 * @param fileName file name where to export the file
 * @param fileType Currently it could be either "DAT", "UNV" or "MED".
 */
void SMESH_Mesh_i::Export(const char* fileName, const char* fileType)
	throw (SALOME::SALOME_Exception)
{
	_impl->Export(fileName, fileType);
}
