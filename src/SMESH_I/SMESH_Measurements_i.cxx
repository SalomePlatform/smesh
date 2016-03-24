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
//  File   : SMESH_Measurements_i.cxx
//  Author : Pavel TELKOV, Open CASCADE S.A.S. (pavel.telkov@opencascade.com)

#ifdef WIN32
#define NOMINMAX
#endif

#include "SMESH_Measurements_i.hxx"

#include "SMESH_Gen_i.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_PythonDump.hxx"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_ElemIterator.hxx"

#include "SMESHDS_Mesh.hxx"


using namespace SMESH;

/**
 * this local function to avoid uninitialized fields
 */
static void initMeasure( SMESH::Measure& theMeasure)
{

  theMeasure.minX = theMeasure.minY = theMeasure.minZ = 0.;
  theMeasure.maxX = theMeasure.maxY = theMeasure.maxZ = 0.;
  theMeasure.node1 = theMeasure.node2 = -1;
  theMeasure.elem1 = theMeasure.elem2 = -1;
  theMeasure.value = 0.;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeasurements
 *
 *  Create measurement instance
 */
//=============================================================================

SMESH::Measurements_ptr SMESH_Gen_i::CreateMeasurements()
{
  SMESH::Measurements_i* aMeasure = new SMESH::Measurements_i();
  SMESH::Measurements_var anObj = aMeasure->_this();
  return anObj._retn();
}

  
/*
  Class       : Measurements
  Description : make measure of mesh qunatities
*/

//=======================================================================
// name    : Measurements_i
// Purpose : Constructor
//=======================================================================
Measurements_i::Measurements_i()
: SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  //Base class Salome_GenericObject do it inmplicitly by overriding PortableServer::POA_ptr _default_POA() method
  //PortableServer::ObjectId_var anObjectId =
  //  SMESH_Gen_i::GetPOA()->activate_object( this );
}

//=======================================================================
// name    : ~Measurements_i
// Purpose : Destructor
//=======================================================================
Measurements_i::~Measurements_i()
{
  //TPythonDump()<<this<<".UnRegister()";
}

static bool getNodeNodeDistance (SMESH::Measure& theMeasure,
                                 const SMDS_MeshNode* theNode1,
                                 const SMDS_MeshNode* theNode2 = 0)
{
  double dist = 0., dd = 0.;

  if (!theNode1)
    return false;

  dd = theNode1->X(); if (theNode2) dd -= theNode2->X(); theMeasure.minX = dd; dd *= dd; dist += dd;
  dd = theNode1->Y(); if (theNode2) dd -= theNode2->Y(); theMeasure.minY = dd; dd *= dd; dist += dd;
  dd = theNode1->Z(); if (theNode2) dd -= theNode2->Z(); theMeasure.minZ = dd; dd *= dd; dist += dd;

  if (dist < 0)
    return false;
  
  theMeasure.value = sqrt(dist);
  theMeasure.node1 = theNode1->GetID();
  theMeasure.node2 = theNode2 ? theNode2->GetID() : 0;

  return true;
}

static SMESHDS_Mesh* getMesh(SMESH::SMESH_IDSource_ptr theSource)
{
  if (!CORBA::is_nil( theSource ))
  {
    SMESH_Mesh_i* anImplPtr = DownCast<SMESH_Mesh_i*>(theSource->GetMesh());
    if (anImplPtr)
      return anImplPtr->GetImpl().GetMeshDS();
  }
  return 0;
}

static bool isNodeType (SMESH::array_of_ElementType_var theTypes)
{
  return theTypes->length() > 0 && theTypes[0] == SMESH::NODE;
}

static double getNumericalValue(SMESH::SMESH_IDSource_ptr            theSource,
                                SMESH::Controls::NumericalFunctorPtr theFunctor)
{
  double value = 0;

  if ( !CORBA::is_nil( theSource ) ) {
    const SMESHDS_Mesh* aMesh = getMesh( theSource );
    if ( aMesh ) {
      theFunctor->SetMesh( aMesh );
      
      SMESH::long_array_var anElementsId = theSource->GetIDs();
      for ( CORBA::ULong i = 0; i < anElementsId->length(); i++) {
        value += theFunctor->GetValue( anElementsId[i] );
      }
    }
  }
  return value;
}

//=======================================================================
// name    : MinDistance
// Purpose : minimal distance between two given entities
//=======================================================================
SMESH::Measure Measurements_i::MinDistance
 (SMESH::SMESH_IDSource_ptr theSource1,
  SMESH::SMESH_IDSource_ptr theSource2)
{
  SMESH::Measure aMeasure;
  initMeasure(aMeasure);

  if (CORBA::is_nil( theSource1 ))
    return aMeasure;
  
  // if second source is null, min distance from theSource1 to the origin is calculated
  bool isOrigin =  CORBA::is_nil( theSource2 );

  // calculate minimal distance between two mesh entities
  SMESH::array_of_ElementType_var types1 = theSource1->GetTypes();
  SMESH::array_of_ElementType_var types2;
  if ( !isOrigin ) types2 = theSource2->GetTypes();

  // here we assume that type of all IDs defined by first type in array
  bool isNode1 = isNodeType(types1);
  bool isNode2 = isOrigin || isNodeType(types2);

  SMESH::long_array_var aElementsId1 = theSource1->GetIDs();
  SMESH::long_array_var aElementsId2;
  if ( !isOrigin ) aElementsId2 = theSource2->GetIDs();

  // compute distance between two entities
  /* NOTE: currently only node-to-node case is implemented
   * all other cases will be implemented later
   * below IF should be replaced by complete switch
   * on mesh entities types
   */
  if (isNode1 && isNode2)
  {
    // node - node
    const SMESHDS_Mesh* aMesh1 = getMesh( theSource1 );
    const SMESHDS_Mesh* aMesh2 = isOrigin ? 0 : getMesh( theSource2 );
    const SMDS_MeshNode* theNode1 = aMesh1 ? aMesh1->FindNode( aElementsId1[0] ) : 0;
    const SMDS_MeshNode* theNode2 = aMesh2 ? aMesh2->FindNode( aElementsId2[0] ) : 0;
    getNodeNodeDistance( aMeasure, theNode1, theNode2 );
  }
  else
  {
    // NOT_IMPLEMENTED
  }

  return aMeasure;
}

//=======================================================================
// name    : enlargeBoundingBox
// Purpose : 
//=======================================================================
static void enlargeBoundingBox(const SMDS_MeshNode* theNode,
                               SMESH::Measure&      theMeasure)
{
  if (!theNode)
    return;
  if ( theMeasure.node1 == -1 ) {
    // we use this attribute as a flag that it is the first node added to the bnd box 
    theMeasure.minX = theMeasure.maxX = theNode->X();
    theMeasure.minY = theMeasure.maxY = theNode->Y();
    theMeasure.minZ = theMeasure.maxZ = theNode->Z();
    theMeasure.node1 = theNode->GetID();
  }
  else {
    theMeasure.minX = std::min( theMeasure.minX, theNode->X() );
    theMeasure.maxX = std::max( theMeasure.maxX, theNode->X() );
    theMeasure.minY = std::min( theMeasure.minY, theNode->Y() );
    theMeasure.maxY = std::max( theMeasure.maxY, theNode->Y() );
    theMeasure.minZ = std::min( theMeasure.minZ, theNode->Z() );
    theMeasure.maxZ = std::max( theMeasure.maxZ, theNode->Z() );
  }
}

//=======================================================================
// name    : enlargeBoundingBox
// Purpose : 
//=======================================================================
static void enlargeBoundingBox(const SMESH::SMESH_IDSource_ptr theObject,
                               SMESH::Measure&                 theMeasure)
{
  if ( CORBA::is_nil( theObject ) )
    return;
  const SMESHDS_Mesh* aMesh = getMesh( theObject );
  if ( !aMesh )
    return;
  SMESH::array_of_ElementType_var types = theObject->GetTypes();
  SMESH::long_array_var     aElementsId = theObject->GetIDs();
  // here we assume that type of all IDs defined by first type in array
  const bool isNode = isNodeType( types );
  for(int i = 0, n = aElementsId->length(); i < n; i++)
  {
    if (isNode)
      enlargeBoundingBox( aMesh->FindNode( aElementsId[i] ), theMeasure);
    else
    {
      const SMDS_MeshElement* elem = aMesh->FindElement( aElementsId[i] );
      if (!elem)
        continue;
      SMDS_ElemIteratorPtr aNodeIter = elem->nodesIterator();
      while( aNodeIter->more() )
        enlargeBoundingBox( dynamic_cast<const SMDS_MeshNode*>( aNodeIter->next() ), theMeasure);
    }
  }
}
                               
//=======================================================================
// name    : BoundingBox
// Purpose : compute common bounding box of entities
//=======================================================================
SMESH::Measure Measurements_i::BoundingBox (const SMESH::ListOfIDSources& theSources)
{
  SMESH::Measure aMeasure;
  initMeasure(aMeasure);

  // calculate bounding box on sources
  for ( int i = 0, n = theSources.length(); i < n ; ++i )
    enlargeBoundingBox( theSources[i], aMeasure );

  return aMeasure;
}

//=======================================================================
// name    : Length
// Purpose : sum of length of 1D elements of the source
//=======================================================================
double Measurements_i::Length(SMESH::SMESH_IDSource_ptr theSource)
{
  return getNumericalValue( theSource, SMESH::Controls::NumericalFunctorPtr(new SMESH::Controls::Length()) );
}

//=======================================================================
// name    : Area
// Purpose : sum of area of 2D elements of the source
//=======================================================================
double Measurements_i::Area(SMESH::SMESH_IDSource_ptr theSource)
{
  return getNumericalValue( theSource, SMESH::Controls::NumericalFunctorPtr(new SMESH::Controls::Area()) );
}

//=======================================================================
// name    : Volume
// Purpose : sum of volume of 3D elements of the source
//=======================================================================
double Measurements_i::Volume(SMESH::SMESH_IDSource_ptr theSource)
{
  return getNumericalValue( theSource, SMESH::Controls::NumericalFunctorPtr(new SMESH::Controls::Volume()) );
}
