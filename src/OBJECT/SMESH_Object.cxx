//  SMESH OBJECT : interactive object for SMESH visualization
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
//  File   : SMESH_Grid.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH

#include "SMESH_ObjectDef.h"
#include "SMESH_ActorUtils.h"

#include "SMDS_Mesh.hxx"
#include "SMESH_Actor.h"
#include "SMESH_ControlsDef.hxx"
#include "SalomeApp_Application.h"
#include "VTKViewer_ExtractUnstructuredGrid.h"

#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SALOME_Exception)

#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>

#include <vtkUnstructuredGrid.h>

#include <memory>
#include <sstream>	
#include <stdexcept>
#include <set>

#include "utilities.h"

using namespace std;

#ifndef EXCEPTION
#define EXCEPTION(TYPE, MSG) {\
  std::ostringstream aStream;\
  aStream<<__FILE__<<"["<<__LINE__<<"]::"<<MSG;\
  throw TYPE(aStream.str());\
}
#endif

#ifdef _DEBUG_
static int MYDEBUG = 0;
static int MYDEBUGWITHFILES = 0;
#else
static int MYDEBUG = 0;
static int MYDEBUGWITHFILES = 0;
#endif


/*
  Class       : SMESH_VisualObjDef
  Description : Base class for all mesh objects to be visuilised
*/

//=================================================================================
// function : getCellType
// purpose  : Get type of VTK cell
//=================================================================================
static inline vtkIdType getCellType( const SMDSAbs_ElementType theType,
				     const bool thePoly,
                                     const int theNbNodes )
{
  switch( theType )
  {
    case SMDSAbs_Edge: 
      if( theNbNodes == 2 )         return VTK_LINE;
      else if ( theNbNodes == 3 )   return VTK_QUADRATIC_EDGE;
      else return VTK_EMPTY_CELL;

    case SMDSAbs_Face  :
      if (thePoly && theNbNodes>2 ) return VTK_POLYGON;
      else if ( theNbNodes == 3 )   return VTK_TRIANGLE;
      else if ( theNbNodes == 4 )   return VTK_QUAD;
      else if ( theNbNodes == 6 )   return VTK_QUADRATIC_TRIANGLE;
      else if ( theNbNodes == 8 )   return VTK_QUADRATIC_QUAD;
      else return VTK_EMPTY_CELL;
      
    case SMDSAbs_Volume:
      if (thePoly && theNbNodes>3 ) return VTK_CONVEX_POINT_SET;
      else if ( theNbNodes == 4 )   return VTK_TETRA;
      else if ( theNbNodes == 5 )   return VTK_PYRAMID;
      else if ( theNbNodes == 6 )   return VTK_WEDGE;
      else if ( theNbNodes == 8 )   return VTK_HEXAHEDRON;
      else if ( theNbNodes == 10 )  {
        return VTK_QUADRATIC_TETRA;
      }
      else if ( theNbNodes == 20 )  {
        return VTK_QUADRATIC_HEXAHEDRON;
      }
      else if ( theNbNodes==13 || theNbNodes==15 )  {
        return VTK_CONVEX_POINT_SET;
      }
      else return VTK_EMPTY_CELL;

    default: return VTK_EMPTY_CELL;
  }
}

//=================================================================================
// functions : SMESH_VisualObjDef
// purpose   : Constructor
//=================================================================================
SMESH_VisualObjDef::SMESH_VisualObjDef()
{
  myGrid = vtkUnstructuredGrid::New();
}
SMESH_VisualObjDef::~SMESH_VisualObjDef()
{
  if ( MYDEBUG )
    MESSAGE( "~SMESH_MeshObj - myGrid->GetReferenceCount() = " << myGrid->GetReferenceCount() );
  myGrid->Delete();
}

//=================================================================================
// functions : GetNodeObjId, GetNodeVTKId, GetElemObjId, GetElemVTKId
// purpose   : Methods for retrieving VTK IDs by SMDS IDs and  vice versa
//=================================================================================
vtkIdType SMESH_VisualObjDef::GetNodeObjId( int theVTKID )
{
  return myVTK2SMDSNodes.find(theVTKID) == myVTK2SMDSNodes.end() ? -1 : myVTK2SMDSNodes[theVTKID];
}

vtkIdType SMESH_VisualObjDef::GetNodeVTKId( int theObjID )
{
  return mySMDS2VTKNodes.find(theObjID) == mySMDS2VTKNodes.end() ? -1 : mySMDS2VTKNodes[theObjID];
}

vtkIdType SMESH_VisualObjDef::GetElemObjId( int theVTKID )
{
  return myVTK2SMDSElems.find(theVTKID) == myVTK2SMDSElems.end() ? -1 : myVTK2SMDSElems[theVTKID];
}

vtkIdType SMESH_VisualObjDef::GetElemVTKId( int theObjID )
{
  return mySMDS2VTKElems.find(theObjID) == mySMDS2VTKElems.end() ? -1 : mySMDS2VTKElems[theObjID];
}

//=================================================================================
// function : SMESH_VisualObjDef::createPoints
// purpose  : Create points from nodes
//=================================================================================
void SMESH_VisualObjDef::createPoints( vtkPoints* thePoints )
{
  if ( thePoints == 0 )
    return;

  TEntityList aNodes;
  vtkIdType nbNodes = GetEntities( SMDSAbs_Node, aNodes );
  thePoints->SetNumberOfPoints( nbNodes );
  
  int nbPoints = 0;

  TEntityList::const_iterator anIter;
  for ( anIter = aNodes.begin(); anIter != aNodes.end(); ++anIter )
  {
    const SMDS_MeshNode* aNode = ( const SMDS_MeshNode* )(*anIter);
    if ( aNode != 0 )
    {
      thePoints->SetPoint( nbPoints, aNode->X(), aNode->Y(), aNode->Z() );
      int anId = aNode->GetID();
      mySMDS2VTKNodes.insert( TMapOfIds::value_type( anId, nbPoints ) );
      myVTK2SMDSNodes.insert( TMapOfIds::value_type( nbPoints, anId ) );
      nbPoints++;
    }
  }

  if ( nbPoints != nbNodes )
    thePoints->SetNumberOfPoints( nbPoints );
}

//=================================================================================
// function : buildPrs
// purpose  : create VTK cells( fill unstructured grid )
//=================================================================================
void SMESH_VisualObjDef::buildPrs()
{
  try
  {
    mySMDS2VTKNodes.clear();
    myVTK2SMDSNodes.clear();
    mySMDS2VTKElems.clear();
    myVTK2SMDSElems.clear();
    
    if ( IsNodePrs() )
      buildNodePrs();
    else
      buildElemPrs();
  }
  catch( const std::exception& exc )
  {
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }
  catch(...)
  {
    INFOS("Unknown exception was cought !!!");
  }
  
  if( MYDEBUG ) MESSAGE( "Update - myGrid->GetNumberOfCells() = "<<myGrid->GetNumberOfCells() );
  if( MYDEBUGWITHFILES ) SMESH::WriteUnstructuredGrid( myGrid,"/tmp/buildPrs" );
}

//=================================================================================
// function : buildNodePrs
// purpose  : create VTK cells for nodes
//=================================================================================
void SMESH_VisualObjDef::buildNodePrs()
{
  vtkPoints* aPoints = vtkPoints::New();
  createPoints( aPoints );
  myGrid->SetPoints( aPoints );
  aPoints->Delete();

  myGrid->SetCells( 0, 0, 0 );
}

//=================================================================================
// function : buildElemPrs
// purpose  : Create VTK cells for elements
//=================================================================================

namespace{
  typedef std::vector<const SMDS_MeshElement*> TConnect;

  int GetConnect(const SMDS_ElemIteratorPtr& theNodesIter, 
		 TConnect& theConnect)
  {
    theConnect.clear();
    for(; theNodesIter->more();)
      theConnect.push_back(theNodesIter->next());
    return theConnect.size();
  }
  
  inline 
  void SetId(vtkIdList *theIdList, 
	     const SMESH_VisualObjDef::TMapOfIds& theSMDS2VTKNodes, 
	     const TConnect& theConnect, 
	     int thePosition,
	     int theId)
  {
    theIdList->SetId(thePosition,theSMDS2VTKNodes.find(theConnect[theId]->GetID())->second);
  }

}


void SMESH_VisualObjDef::buildElemPrs()
{
  // Create points
  
  vtkPoints* aPoints = vtkPoints::New();
  createPoints( aPoints );
  myGrid->SetPoints( aPoints );
  aPoints->Delete();
    
  if ( MYDEBUG )
    MESSAGE("Update - myGrid->GetNumberOfPoints() = "<<myGrid->GetNumberOfPoints());

  // Calculate cells size

  static SMDSAbs_ElementType aTypes[ 3 ] = { SMDSAbs_Edge, SMDSAbs_Face, SMDSAbs_Volume };

  // get entity data
  map<SMDSAbs_ElementType,int> nbEnts;
  map<SMDSAbs_ElementType,TEntityList> anEnts;

  for ( int i = 0; i <= 2; i++ )
    nbEnts[ aTypes[ i ] ] = GetEntities( aTypes[ i ], anEnts[ aTypes[ i ] ] );

  vtkIdType aCellsSize =  3 * nbEnts[ SMDSAbs_Edge ];

  for ( int i = 1; i <= 2; i++ ) // iterate through faces and volumes
  {
    if ( nbEnts[ aTypes[ i ] ] )
    {
      const TEntityList& aList = anEnts[ aTypes[ i ] ];
      TEntityList::const_iterator anIter;
      for ( anIter = aList.begin(); anIter != aList.end(); ++anIter )
        aCellsSize += (*anIter)->NbNodes() + 1;
    }
  }

  vtkIdType aNbCells = nbEnts[ SMDSAbs_Edge ] + nbEnts[ SMDSAbs_Face ] + nbEnts[ SMDSAbs_Volume ];
  
  if ( MYDEBUG )
    MESSAGE( "Update - aNbCells = "<<aNbCells<<"; aCellsSize = "<<aCellsSize );

  // Create cells
  
  vtkCellArray* aConnectivity = vtkCellArray::New();
  aConnectivity->Allocate( aCellsSize, 0 );
  
  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents( 1 );
  aCellTypesArray->Allocate( aNbCells * aCellTypesArray->GetNumberOfComponents() );
  
  vtkIdList *anIdList = vtkIdList::New();
  vtkIdType iElem = 0;

  TConnect aConnect;
  aConnect.reserve(VTK_CELL_SIZE);

  for ( int i = 0; i <= 2; i++ ) // iterate through edges, faces and volumes
  {
    if( nbEnts[ aTypes[ i ] ] > 0 )
    {
      const SMDSAbs_ElementType& aType = aTypes[ i ];
      const TEntityList& aList = anEnts[ aType ];
      TEntityList::const_iterator anIter;
      for ( anIter = aList.begin(); anIter != aList.end(); ++anIter )
      {
        const SMDS_MeshElement* anElem = *anIter;
        
        vtkIdType aNbNodes = anElem->NbNodes();
        anIdList->SetNumberOfIds( aNbNodes );

        int anId = anElem->GetID();

        mySMDS2VTKElems.insert( TMapOfIds::value_type( anId, iElem ) );
        myVTK2SMDSElems.insert( TMapOfIds::value_type( iElem, anId ) );

        SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	switch(aType){
	case SMDSAbs_Volume:{
	  std::vector<int> aConnectivities;
	  GetConnect(aNodesIter,aConnect);
	  // Convertions connectivities from SMDS to VTK
	  if (anElem->IsPoly() && aNbNodes > 3) { // POLYEDRE
	    for (int k = 0; k < aNbNodes; k++) {
	      aConnectivities.push_back(k);
	    }

	  } else if (aNbNodes == 4) {
	    static int anIds[] = {0,2,1,3};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);

	  } else if (aNbNodes == 5) {
	    static int anIds[] = {0,3,2,1,4};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);

	  } else if (aNbNodes == 6) {
	    static int anIds[] = {0,1,2,3,4,5};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);

	  }
          else if (aNbNodes == 8) {
	    static int anIds[] = {0,3,2,1,4,7,6,5};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);

	  }
          else if (aNbNodes == 10) {
	    static int anIds[] = {0,2,1,3,6,5,4,7,9,8};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);
	  }
          else if (aNbNodes == 13) {
	    static int anIds[] = {0,3,2,1,4,8,7,6,5,9,12,11,10};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);
	  }
          else if (aNbNodes == 15) {
	    static int anIds[] = {0,2,1,3,5,4,8,7,6,11,10,9,12,14,13};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);
	    //for (int k = 0; k < aNbNodes; k++) {
            //  int nn = aConnectivities[k];
            //  const SMDS_MeshNode* N = static_cast<const SMDS_MeshNode*> (aConnect[nn]);
            //  cout<<"k="<<k<<"  N("<<N->X()<<","<<N->Y()<<","<<N->Z()<<")"<<endl;
            //}
	  }
          else if (aNbNodes == 20) {
	    static int anIds[] = {0,3,2,1,4,7,6,5,11,10,9,8,15,14,13,12,16,19,18,17};
	    for (int k = 0; k < aNbNodes; k++) aConnectivities.push_back(anIds[k]);
	  }
          else {
          }

	  if (aConnectivities.size() > 0) {
	    for (vtkIdType aNodeId = 0; aNodeId < aNbNodes; aNodeId++)
	      SetId(anIdList,mySMDS2VTKNodes,aConnect,aNodeId,aConnectivities[aNodeId]);
	  }
	  break;
	}
	default:
	  for( vtkIdType aNodeId = 0; aNodesIter->more(); aNodeId++ ){
	    const SMDS_MeshElement* aNode = aNodesIter->next();
	    anIdList->SetId( aNodeId, mySMDS2VTKNodes[aNode->GetID()] );
	  }
	}

        aConnectivity->InsertNextCell( anIdList );
        aCellTypesArray->InsertNextValue( getCellType( aType, anElem->IsPoly(), aNbNodes ) );

        iElem++;
      }
    }
  }

  // Insert cells in grid
  
  vtkIntArray* aCellLocationsArray = vtkIntArray::New();
  aCellLocationsArray->SetNumberOfComponents( 1 );
  aCellLocationsArray->SetNumberOfTuples( aNbCells );
  
  aConnectivity->InitTraversal();
  for( vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell( npts, pts ); idType++ )
    aCellLocationsArray->SetValue( idType, aConnectivity->GetTraversalLocation( npts ) );

  myGrid->SetCells( aCellTypesArray, aCellLocationsArray,aConnectivity );
  
  aCellLocationsArray->Delete();
  aCellTypesArray->Delete();
  aConnectivity->Delete();
  anIdList->Delete();
}

//=================================================================================
// function : GetEdgeNodes
// purpose  : Retrieve ids of nodes from edge of elements ( edge is numbered from 1 )
//=================================================================================
bool SMESH_VisualObjDef::GetEdgeNodes( const int theElemId,
				       const int theEdgeNum,
				       int&      theNodeId1,
				       int&      theNodeId2 ) const
{
  const SMDS_Mesh* aMesh = GetMesh();
  if ( aMesh == 0 )
    return false;
    
  const SMDS_MeshElement* anElem = aMesh->FindElement( theElemId );
  if ( anElem == 0 )
    return false;
    
  int nbNodes = anElem->NbNodes();

  if ( theEdgeNum < 0 || theEdgeNum > 3 || nbNodes != 3 && nbNodes != 4 || theEdgeNum > nbNodes )
    return false;

  int anIds[ nbNodes ];
  SMDS_ElemIteratorPtr anIter = anElem->nodesIterator();
  int i = 0;
  while( anIter->more() )
    anIds[ i++ ] = anIter->next()->GetID();

  if ( theEdgeNum < nbNodes - 1 )
  {
    theNodeId1 = anIds[ theEdgeNum ];
    theNodeId2 = anIds[ theEdgeNum + 1 ];
  }
  else
  {
    theNodeId1 = anIds[ nbNodes - 1 ];
    theNodeId2 = anIds[ 0 ];
  }

  return true;
}

/*
  Class       : SMESH_MeshObj
  Description : Class for visualisation of mesh
*/

//=================================================================================
// function : SMESH_MeshObj
// purpose  : Constructor
//=================================================================================
SMESH_MeshObj::SMESH_MeshObj(SMESH::SMESH_Mesh_ptr theMesh):
  myClient(SalomeApp_Application::orb(),theMesh)
{
  if ( MYDEBUG ) 
    MESSAGE("SMESH_MeshObj - this = "<<this<<"; theMesh->_is_nil() = "<<theMesh->_is_nil());
}

//=================================================================================
// function : ~SMESH_MeshObj
// purpose  : Destructor
//=================================================================================
SMESH_MeshObj::~SMESH_MeshObj()
{
  if ( MYDEBUG ) 
    MESSAGE("SMESH_MeshObj - this = "<<this<<"\n");
}

//=================================================================================
// function : Update
// purpose  : Update mesh and fill grid with new values if necessary 
//=================================================================================
void SMESH_MeshObj::Update( int theIsClear )
{
  // Update SMDS_Mesh on client part
  if ( myClient.Update(theIsClear) )
    buildPrs();  // Fill unstructured grid
}

//=================================================================================
// function : GetElemDimension
// purpose  : Get dimension of element
//=================================================================================
int SMESH_MeshObj::GetElemDimension( const int theObjId )
{
  const SMDS_MeshElement* anElem = myClient->FindElement( theObjId );
  if ( anElem == 0 )
    return 0;

  int aType = anElem->GetType();
  switch ( aType )
  {
    case SMDSAbs_Edge  : return 1;
    case SMDSAbs_Face  : return 2;
    case SMDSAbs_Volume: return 3;
    default            : return 0;
  }
}

//=================================================================================
// function : GetEntities
// purpose  : Get entities of specified type. Return number of entities
//=================================================================================
int SMESH_MeshObj::GetNbEntities( const SMDSAbs_ElementType theType) const
{
  switch ( theType )
  {
    case SMDSAbs_Node:
    {
      return myClient->NbNodes();
    }
    break;
    case SMDSAbs_Edge:
    {
      return myClient->NbEdges();
    }
    break;
    case SMDSAbs_Face:
    {
      return myClient->NbFaces();
    }
    break;
    case SMDSAbs_Volume:
    {
      return myClient->NbVolumes();
    }
    break;
    default:
      return 0;
    break;
  }
}

int SMESH_MeshObj::GetEntities( const SMDSAbs_ElementType theType, TEntityList& theObjs ) const
{
  theObjs.clear();

  switch ( theType )
  {
    case SMDSAbs_Node:
    {
      SMDS_NodeIteratorPtr anIter = myClient->nodesIterator();
      while ( anIter->more() ) theObjs.push_back( anIter->next() );
    }
    break;
    case SMDSAbs_Edge:
    {
      SMDS_EdgeIteratorPtr anIter = myClient->edgesIterator();
      while ( anIter->more() ) theObjs.push_back( anIter->next() );
    }
    break;
    case SMDSAbs_Face:
    {
      SMDS_FaceIteratorPtr anIter = myClient->facesIterator();
      while ( anIter->more() ) theObjs.push_back( anIter->next() );
    }
    break;
    case SMDSAbs_Volume:
    {
      SMDS_VolumeIteratorPtr anIter = myClient->volumesIterator();
      while ( anIter->more() ) theObjs.push_back( anIter->next() );
    }
    break;
    default:
    break;
  }

  return theObjs.size();
}

//=================================================================================
// function : UpdateFunctor
// purpose  : Update functor in accordance with current mesh
//=================================================================================
void SMESH_MeshObj::UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor )
{
  theFunctor->SetMesh( GetMesh() );
}

//=================================================================================
// function : IsNodePrs
// purpose  : Return true if node presentation is used
//=================================================================================
bool SMESH_MeshObj::IsNodePrs() const
{
  return myClient->NbEdges() == 0 &&myClient->NbFaces() == 0 && myClient->NbVolumes() == 0 ;
}


/*
  Class       : SMESH_SubMeshObj
  Description : Base class for visualisation of submeshes and groups
*/

//=================================================================================
// function : SMESH_SubMeshObj
// purpose  : Constructor
//=================================================================================
SMESH_SubMeshObj::SMESH_SubMeshObj( SMESH_MeshObj* theMeshObj )
{
  if ( MYDEBUG ) MESSAGE( "SMESH_SubMeshObj - theMeshObj = " << theMeshObj );
  
  myMeshObj = theMeshObj;
}

SMESH_SubMeshObj::~SMESH_SubMeshObj()
{
}

//=================================================================================
// function : GetElemDimension
// purpose  : Get dimension of element
//=================================================================================
int SMESH_SubMeshObj::GetElemDimension( const int theObjId )
{
  return myMeshObj == 0 ? 0 : myMeshObj->GetElemDimension( theObjId );
}

//=================================================================================
// function : UpdateFunctor
// purpose  : Update functor in accordance with current mesh
//=================================================================================
void SMESH_SubMeshObj::UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor )
{
  theFunctor->SetMesh( myMeshObj->GetMesh() );
}

//=================================================================================
// function : Update
// purpose  : Update mesh object and fill grid with new values 
//=================================================================================
void SMESH_SubMeshObj::Update( int theIsClear )
{
  myMeshObj->Update( theIsClear );
  buildPrs();
}


/*
  Class       : SMESH_GroupObj
  Description : Class for visualisation of groups
*/

//=================================================================================
// function : SMESH_GroupObj
// purpose  : Constructor
//=================================================================================
SMESH_GroupObj::SMESH_GroupObj( SMESH::SMESH_GroupBase_ptr theGroup, 
                                SMESH_MeshObj*             theMeshObj )
: SMESH_SubMeshObj( theMeshObj ),
  myGroupServer( SMESH::SMESH_GroupBase::_duplicate(theGroup) )
{
  if ( MYDEBUG ) MESSAGE("SMESH_GroupObj - theGroup->_is_nil() = "<<theGroup->_is_nil());
  myGroupServer->Register();
}

SMESH_GroupObj::~SMESH_GroupObj()
{
  if ( MYDEBUG ) MESSAGE("~SMESH_GroupObj");
  myGroupServer->Destroy();
}

//=================================================================================
// function : IsNodePrs
// purpose  : Return true if node presentation is used
//=================================================================================
bool SMESH_GroupObj::IsNodePrs() const
{
  return myGroupServer->GetType() == SMESH::NODE;
}

//=================================================================================
// function : getNodesFromElems
// purpose  : Retrieve nodes from elements
//=================================================================================
static int getNodesFromElems( SMESH::long_array_var&              theElemIds,
                              const SMDS_Mesh*                    theMesh,
                              std::list<const SMDS_MeshElement*>& theResList )
{
  set<const SMDS_MeshElement*> aNodeSet;

  for ( CORBA::Long i = 0, n = theElemIds->length(); i < n; i++ )
  {
    const SMDS_MeshElement* anElem = theMesh->FindElement( theElemIds[ i ] );
    if ( anElem != 0 )
    {
      SMDS_ElemIteratorPtr anIter = anElem->nodesIterator();
      while ( anIter->more() )
      {
        const SMDS_MeshElement* aNode = anIter->next();
        if ( aNode != 0 )
          aNodeSet.insert( aNode );
      }
    }
  }

  set<const SMDS_MeshElement*>::const_iterator anIter;
  for ( anIter = aNodeSet.begin(); anIter != aNodeSet.end(); ++anIter )
    theResList.push_back( *anIter );

  return theResList.size();    
}

//=================================================================================
// function : getPointers
// purpose  : Get std::list<const SMDS_MeshElement*> from list of IDs
//=================================================================================
static int getPointers( const SMDSAbs_ElementType            theRequestType,
                        SMESH::long_array_var&              theElemIds,
                        const SMDS_Mesh*                    theMesh,
                        std::list<const SMDS_MeshElement*>& theResList )
{
  for ( CORBA::Long i = 0, n = theElemIds->length(); i < n; i++ )
  {
    const SMDS_MeshElement* anElem = theRequestType == SMDSAbs_Node
      ? theMesh->FindNode( theElemIds[ i ] ) : theMesh->FindElement( theElemIds[ i ] );

    if ( anElem != 0 )
      theResList.push_back( anElem );
  }

  return theResList.size();
}


//=================================================================================
// function : GetEntities
// purpose  : Get entities of specified type. Return number of entities
//=================================================================================
int SMESH_GroupObj::GetNbEntities( const SMDSAbs_ElementType theType) const
{
  if(SMDSAbs_ElementType(myGroupServer->GetType()) == theType){
    return myGroupServer->Size();
  }
  return 0;
}

int SMESH_GroupObj::GetEntities( const SMDSAbs_ElementType theType, TEntityList& theResList ) const
{
  theResList.clear();
  SMDS_Mesh* aMesh = myMeshObj->GetMesh();
  
  if ( myGroupServer->Size() == 0 || aMesh == 0 )
    return 0;

  SMDSAbs_ElementType aGrpType = SMDSAbs_ElementType(myGroupServer->GetType());
  SMESH::long_array_var anIds = myGroupServer->GetListOfID();

  if ( aGrpType == theType )
    return getPointers( theType, anIds, aMesh, theResList );
  else if ( theType == SMDSAbs_Node )
    return getNodesFromElems( anIds, aMesh, theResList );
  else
    return 0;
}    



/*
  Class       : SMESH_subMeshObj
  Description : Class for visualisation of submeshes
*/

//=================================================================================
// function : SMESH_subMeshObj
// purpose  : Constructor
//=================================================================================
SMESH_subMeshObj::SMESH_subMeshObj( SMESH::SMESH_subMesh_ptr theSubMesh,
                                    SMESH_MeshObj*           theMeshObj )
: SMESH_SubMeshObj( theMeshObj ),
  mySubMeshServer( SMESH::SMESH_subMesh::_duplicate( theSubMesh ) )
{
  if ( MYDEBUG ) MESSAGE( "SMESH_subMeshObj - theSubMesh->_is_nil() = " << theSubMesh->_is_nil() );
  
  mySubMeshServer->Register();
}

SMESH_subMeshObj::~SMESH_subMeshObj()
{
  if ( MYDEBUG ) MESSAGE( "~SMESH_subMeshObj" );
  mySubMeshServer->Destroy();
}

//=================================================================================
// function : GetEntities
// purpose  : Get entities of specified type. Return number of entities
//=================================================================================
int SMESH_subMeshObj::GetNbEntities( const SMDSAbs_ElementType theType) const
{
  switch ( theType )
  {
    case SMDSAbs_Node:
    {
      return mySubMeshServer->GetNumberOfNodes( false );
    }
    break;
    case SMDSAbs_Edge:
    case SMDSAbs_Face:
    case SMDSAbs_Volume:
    {
      SMESH::long_array_var anIds = 
	mySubMeshServer->GetElementsByType( SMESH::ElementType(theType) );
      return anIds->length();
    }
    default:
      return 0;
    break;
  }
}

int SMESH_subMeshObj::GetEntities( const SMDSAbs_ElementType theType, TEntityList& theResList ) const
{
  theResList.clear();

  SMDS_Mesh* aMesh = myMeshObj->GetMesh();
  if ( aMesh == 0 )
    return 0;

  bool isNodal = IsNodePrs();

  if ( isNodal )
  {
    if ( theType == SMDSAbs_Node )
    {
      SMESH::long_array_var anIds = mySubMeshServer->GetNodesId();
      return getPointers( SMDSAbs_Node, anIds, aMesh, theResList );
    }
  }
  else
  {
    if ( theType == SMDSAbs_Node )
    {
      SMESH::long_array_var anIds = mySubMeshServer->GetElementsId();
      return getNodesFromElems( anIds, aMesh, theResList );
    }
    else
    {
      SMESH::long_array_var anIds = 
	mySubMeshServer->GetElementsByType( SMESH::ElementType(theType) );
      return getPointers( theType, anIds, aMesh, theResList );
    }
  }

  return 0;
}

//=================================================================================
// function : IsNodePrs
// purpose  : Return true if node presentation is used
//=================================================================================
bool SMESH_subMeshObj::IsNodePrs() const
{
  return mySubMeshServer->GetNumberOfElements() == 0;
}
