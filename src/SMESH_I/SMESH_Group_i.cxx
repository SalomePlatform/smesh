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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : SMESH_Group_i.cxx
//  Author : Sergey ANIKIN, OCC
//  Module : SMESH
//
#include "SMESH_Group_i.hxx"

#include "SMDSAbs_ElementType.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnFilter.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_PreMeshInfo.hxx"

#include CORBA_SERVER_HEADER(SMESH_Filter)

#include "utilities.h"

using namespace SMESH;

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_GroupBase_i::SMESH_GroupBase_i( PortableServer::POA_ptr thePOA,
                                      SMESH_Mesh_i*           theMeshServant,
                                      const int               theLocalID )
: SALOME::GenericObj_i( thePOA ),
  myMeshServant( theMeshServant ), 
  myLocalID( theLocalID ),
  myNbNodes(-1),
  myGroupDSTic(0),
  myPreMeshInfo(NULL)
{
  // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i,
  // servant activation is performed by SMESH_Mesh_i::createGroup()
  // thePOA->activate_object( this );
}

SMESH_Group_i::SMESH_Group_i( PortableServer::POA_ptr thePOA,
                              SMESH_Mesh_i*           theMeshServant,
                              const int               theLocalID )
     : SALOME::GenericObj_i( thePOA ),
       SMESH_GroupBase_i( thePOA, theMeshServant, theLocalID )
{
  //MESSAGE("SMESH_Group_i; this = "<<this );
}

SMESH_GroupOnGeom_i::SMESH_GroupOnGeom_i( PortableServer::POA_ptr thePOA,
                                          SMESH_Mesh_i*           theMeshServant,
                                          const int               theLocalID )
     : SALOME::GenericObj_i( thePOA ),
       SMESH_GroupBase_i( thePOA, theMeshServant, theLocalID )
{
  //MESSAGE("SMESH_GroupOnGeom_i; this = "<<this );
}

SMESH_GroupOnFilter_i::SMESH_GroupOnFilter_i( PortableServer::POA_ptr thePOA,
                                              SMESH_Mesh_i*           theMeshServant,
                                              const int               theLocalID )
  : SALOME::GenericObj_i( thePOA ),
    SMESH_GroupBase_i( thePOA, theMeshServant, theLocalID )
{
  //MESSAGE("SMESH_GroupOnGeom_i; this = "<<this );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_GroupBase_i::~SMESH_GroupBase_i()
{
  MESSAGE("~SMESH_GroupBase_i; this = "<<this );
  if ( myMeshServant )
    myMeshServant->removeGroup(myLocalID);

  if ( myPreMeshInfo ) delete myPreMeshInfo; myPreMeshInfo = NULL;
}

//=======================================================================
//function : GetSmeshGroup
//purpose  : 
//=======================================================================

::SMESH_Group* SMESH_GroupBase_i::GetSmeshGroup() const
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    return aMesh.GetGroup(myLocalID);
  }
  return 0;
}

//=======================================================================
//function : GetGroupDS
//purpose  : 
//=======================================================================

SMESHDS_GroupBase* SMESH_GroupBase_i::GetGroupDS() const
{
  ::SMESH_Group* aGroup = GetSmeshGroup();
  if ( aGroup )
    return aGroup->GetGroupDS();
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_GroupBase_i::SetName( const char* theName )
{
  // Perform renaming
  ::SMESH_Group* aGroup = GetSmeshGroup();
  if (!aGroup) {
    MESSAGE("can't set name of a vague group");
    return;
  }

  if ( aGroup->GetName() && !strcmp( aGroup->GetName(), theName ) )
    return; // nothing to rename

  aGroup->SetName(theName);

  // Update group name in a study
  SMESH_Gen_i* aGen = myMeshServant->GetGen();
  aGen->SetName( aGen->ObjectToSObject( aGen->GetCurrentStudy(), _this() ), theName );
  
  // Update Python script
  TPythonDump() <<  _this() << ".SetName( '" << theName << "' )";
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

char* SMESH_GroupBase_i::GetName()
{
  ::SMESH_Group* aGroup = GetSmeshGroup();
  if (aGroup)
    return CORBA::string_dup (aGroup->GetName());
  MESSAGE("get name of a vague group");
  return CORBA::string_dup( "NO_NAME" );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::ElementType SMESH_GroupBase_i::GetType()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS) {
    SMDSAbs_ElementType aSMDSType = aGroupDS->GetType();
    SMESH::ElementType aType;
    switch (aSMDSType) {
    case SMDSAbs_Node:      aType = SMESH::NODE;   break;
    case SMDSAbs_Edge:      aType = SMESH::EDGE;   break;
    case SMDSAbs_Face:      aType = SMESH::FACE;   break;
    case SMDSAbs_Volume:    aType = SMESH::VOLUME; break;
    case SMDSAbs_0DElement: aType = SMESH::ELEM0D; break;
    default:                aType = SMESH::ALL;    break;
    }
    return aType;
  }
  MESSAGE("get type of a vague group");
  return SMESH::ALL;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_GroupBase_i::Size()
{
  if ( myPreMeshInfo )
    return GetType() == SMESH::NODE ? myPreMeshInfo->NbNodes() : myPreMeshInfo->NbElements();

  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->Extent();
  MESSAGE("get size of a vague group");
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_GroupBase_i::IsEmpty()
{
  if ( myPreMeshInfo )
    return Size() == 0;

  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->IsEmpty();
  MESSAGE("checking IsEmpty of a vague group");
  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Group_i::Clear()
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  // Update Python script
  TPythonDump() << _this() << ".Clear()";

  // Clear the group
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    aGroupDS->Clear();
    return;
  }
  MESSAGE("attempt to clear a vague group");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_GroupBase_i::Contains( CORBA::Long theID )
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->Contains(theID);
  MESSAGE("attempt to check contents of a vague group");
  return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Group_i::Add( const SMESH::long_array& theIDs )
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  // Update Python script
  TPythonDump() << "nbAdd = " << _this() << ".Add( " << theIDs << " )";

  // Add elements to the group
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    int nbAdd = 0;
    for (int i = 0; i < theIDs.length(); i++) {
      int anID = (int) theIDs[i];
      if (aGroupDS->Add(anID))
        nbAdd++;
    }
    return nbAdd;
  }
  MESSAGE("attempt to add elements to a vague group");
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Group_i::Remove( const SMESH::long_array& theIDs )
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  // Update Python script
  TPythonDump() << "nbDel = " << _this() << ".Remove( " << theIDs << " )";

  // Remove elements from the group
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    int nbDel = 0;
    for (int i = 0; i < theIDs.length(); i++) {
      int anID = (int) theIDs[i];
      if (aGroupDS->Remove(anID))
        nbDel++;
    }
    return nbDel;
  }
  MESSAGE("attempt to remove elements from a vague group");
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

typedef bool (SMESHDS_Group::*TFunChangeGroup)(const int);

CORBA::Long 
ChangeByPredicate( SMESH::Predicate_i* thePredicate,
                   SMESHDS_GroupBase*  theGroupBase,
                   TFunChangeGroup     theFun)
{
  CORBA::Long aNb = 0;
  if(SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>(theGroupBase)){
    SMESH::Controls::Filter::TIdSequence aSequence;
    const SMDS_Mesh* aMesh = theGroupBase->GetMesh();
    SMESH::Filter_i::GetElementsId(thePredicate,aMesh,aSequence);
    
    CORBA::Long i = 0, iEnd = aSequence.size();
    for(; i < iEnd; i++)
      if((aGroupDS->*theFun)(aSequence[i]))
        aNb++;
    return aNb;
  }
  return aNb;
}

CORBA::Long 
SMESH_Group_i::
AddByPredicate( SMESH::Predicate_ptr thePredicate )
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  if(SMESH::Predicate_i* aPredicate = SMESH::GetPredicate(thePredicate)){
    TPythonDump()<<_this()<<".AddByPredicate("<<aPredicate<<")";
    return ChangeByPredicate(aPredicate,GetGroupDS(),&SMESHDS_Group::Add);
  }
  return 0;
}

CORBA::Long 
SMESH_Group_i::
RemoveByPredicate( SMESH::Predicate_ptr thePredicate )
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  if(SMESH::Predicate_i* aPredicate = SMESH::GetPredicate(thePredicate)){
    TPythonDump()<<_this()<<".RemoveByPredicate("<<aPredicate<<")";
    return ChangeByPredicate(aPredicate,GetGroupDS(),&SMESHDS_Group::Remove);
  }
  return 0;
}

CORBA::Long SMESH_Group_i::AddFrom( SMESH::SMESH_IDSource_ptr theSource )
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  TPythonDump pd;
  long nbAdd = 0;
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    SMESH::long_array_var anIds;
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow(theSource);
    SMESH::SMESH_Mesh_var mesh       = SMESH::SMESH_Mesh::_narrow(theSource);
    SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow(theSource);
    SMESH::Filter_var filter         = SMESH::Filter::_narrow(theSource);
    if ( !group->_is_nil())
      anIds = group->GetType()==GetType() ? theSource->GetIDs() :  new SMESH::long_array();
    else if ( !mesh->_is_nil() )
      anIds = mesh->GetElementsByType( GetType() );
    else if ( !submesh->_is_nil())
      anIds = submesh->GetElementsByType( GetType() );
    else if ( !filter->_is_nil() ) {
      filter->SetMesh( GetMeshServant()->_this() );
      anIds = filter->GetElementType()==GetType() ? theSource->GetIDs() : new SMESH::long_array();
    }
    else 
      anIds = theSource->GetIDs();
    for ( int i = 0, total = anIds->length(); i < total; i++ ) {
      if ( aGroupDS->Add((int)anIds[i]) ) nbAdd++;
    }
  }

  // Update Python script
  pd << "nbAdd = " << _this() << ".AddFrom( " << theSource << " )";

  return nbAdd;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_GroupBase_i::GetID( CORBA::Long theIndex )
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->GetID(theIndex);
  MESSAGE("attempt to iterate on a vague group");
  return -1;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::long_array* SMESH_GroupBase_i::GetListOfID()
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  SMESH::long_array_var aRes = new SMESH::long_array();
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS) {
    int aSize = aGroupDS->Extent();
    aRes->length(aSize);
    for (int i = 0; i < aSize; i++)
      aRes[i] = aGroupDS->GetID(i+1);
    return aRes._retn();
  }
  MESSAGE("get list of IDs of a vague group");
  return aRes._retn();
}

namespace
{
  //================================================================================
  /*!
   * \brief return nodes of elements pointered by iterator
   */
  //================================================================================

  void getNodesOfElements(SMDS_ElemIteratorPtr        elemIt,
                          set<const SMDS_MeshNode* >& nodes)
  {
    while ( elemIt->more() )
    {
      const SMDS_MeshElement* e = elemIt->next();
      nodes.insert( e->begin_nodes(), e->end_nodes() );
    }
  }
}
  
//================================================================================
/*!
 * \brief return the number of nodes of cells included to the group
 */
//================================================================================

CORBA::Long SMESH_GroupBase_i::GetNumberOfNodes()
{
  if ( GetType() == SMESH::NODE )
    return Size();

  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  if ( SMESHDS_GroupBase* g = GetGroupDS())
  {
    if ( myNbNodes < 0 || g->GetTic() != myGroupDSTic )
    {      
      set<const SMDS_MeshNode* > nodes;
      getNodesOfElements( g->GetElements(), nodes );
      myNbNodes = nodes.size();
      myGroupDSTic = g->GetTic();
    }
  }
  return myNbNodes;
}

//================================================================================
/*!
 * \brief Return true if GetNumberOfNodes() won't take a long time for computation
 */
//================================================================================

CORBA::Boolean SMESH_GroupBase_i::IsNodeInfoAvailable()
{
  if ( GetType() == SMESH::NODE/* || Size() < 100000 */)
    return true;
  if ( myPreMeshInfo )
    return false;
  if ( SMESHDS_GroupBase* g = GetGroupDS())
    return ( myNbNodes > -1 && g->GetTic() == myGroupDSTic);
  return false;
}

//================================================================================
/*!
 * \brief Return IDs of nodes of cells included to the group
 */
//================================================================================

SMESH::long_array* SMESH_GroupBase_i::GetNodeIDs()
{
  if ( GetType() == SMESH::NODE )
    return GetListOfID();

  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  SMESH::long_array_var aRes = new SMESH::long_array();
  if ( SMESHDS_GroupBase* g = GetGroupDS())
  {
    set<const SMDS_MeshNode* > nodes;
    getNodesOfElements( g->GetElements(), nodes );
    aRes->length( nodes.size() );
    set<const SMDS_MeshNode*>::iterator nIt = nodes.begin(), nEnd = nodes.end();
    for ( int i = 0; nIt != nEnd; ++nIt, ++i )
      aRes[i] = (*nIt)->GetID();
  }
  return aRes._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESH_GroupBase_i::GetMesh()
{
  SMESH::SMESH_Mesh_var aMesh;
  if ( myMeshServant )
    aMesh = SMESH::SMESH_Mesh::_narrow( myMeshServant->_this() );
  return aMesh._retn();
}

//=======================================================================
//function : GetShape
//purpose  : 
//=======================================================================

GEOM::GEOM_Object_ptr SMESH_GroupOnGeom_i::GetShape()
{
  GEOM::GEOM_Object_var aGeomObj;
  SMESHDS_GroupOnGeom* aGroupDS = dynamic_cast<SMESHDS_GroupOnGeom*>( GetGroupDS() );
  if ( aGroupDS ) {
    SMESH_Gen_i* aGen = GetMeshServant()->GetGen();
    aGeomObj = aGen->ShapeToGeomObject( aGroupDS->GetShape() );
  }
  return aGeomObj._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SALOMEDS::Color SMESH_GroupBase_i::GetColor()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
  {
    Quantity_Color aQColor = aGroupDS->GetColor();
    SALOMEDS::Color aColor;
    aColor.R = aQColor.Red();
    aColor.G = aQColor.Green();
    aColor.B = aQColor.Blue();

    return aColor;
  }
  MESSAGE("get color of a group");
  return SALOMEDS::Color();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESH_GroupBase_i::SetColor(const SALOMEDS::Color& color)
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
  {
    Quantity_Color aQColor( color.R, color.G, color.B, Quantity_TOC_RGB );
    Quantity_Color oldColor = aGroupDS->GetColor();
    if ( oldColor != aQColor )
    {
      aGroupDS->SetColor(aQColor);
      TPythonDump()<<_this()<<".SetColor( SALOMEDS.Color( "<<color.R<<", "<<color.G<<", "<<color.B<<" ))";
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Long SMESH_GroupBase_i::GetColorNumber()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->GetColorGroup();
  MESSAGE("get color number of a group");
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESH_GroupBase_i::SetColorNumber(CORBA::Long color)
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
  {
    aGroupDS->SetColorGroup(color);
    TPythonDump()<<_this()<<".SetColorNumber( "<<color<<" )";
  }
  MESSAGE("set color number of a group");
  return ;
}

//=============================================================================
/*!
 * Returns statistic of mesh elements
 * Result array of number enityties
 * Inherited from SMESH_IDSource
 */
//=============================================================================
SMESH::long_array* SMESH_GroupBase_i::GetMeshInfo()
{
  if ( myPreMeshInfo )
    return myPreMeshInfo->GetMeshInfo();

  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::Entity_Last);
  for (int i = SMESH::Entity_Node; i < SMESH::Entity_Last; i++)
    aRes[i] = 0;

  if ( SMESHDS_GroupBase* g = GetGroupDS())
  {
    if ( g->GetType() == SMDSAbs_Node || ( myNbNodes > -1 && g->GetTic() == myGroupDSTic))
      aRes[ SMDSEntity_Node ] = GetNumberOfNodes();

    if ( g->GetType() != SMDSAbs_Node )
      SMESH_Mesh_i::CollectMeshInfo( g->GetElements(), aRes);
  }

  return aRes._retn();
}

//=======================================================================
//function : GetIDs
//purpose  : Returns ids of members
//=======================================================================

SMESH::long_array* SMESH_GroupBase_i::GetIDs()
{
  return GetListOfID();
}

//=======================================================================
//function : GetTypes
//purpose  : Returns types of elements it contains
//=======================================================================

SMESH::array_of_ElementType* SMESH_GroupBase_i::GetTypes()
{
  SMESH::array_of_ElementType_var types = new SMESH::array_of_ElementType;
  if ( !IsEmpty() )
  {
    types->length( 1 );
    types[0] = GetType();
  }
  return types._retn();
}

//=======================================================================
//function : IsMeshInfoCorrect
//purpose  : * Returns false if GetMeshInfo() returns incorrect information that may
//           * happen if mesh data is not yet fully loaded from the file of study.
//=======================================================================

bool SMESH_GroupBase_i::IsMeshInfoCorrect()
{
  return myPreMeshInfo ? myPreMeshInfo->IsMeshInfoCorrect() : true;
}

//================================================================================
/*!
 * \brief Retrieves the predicate from the filter
 */
//================================================================================

SMESH_PredicatePtr SMESH_GroupOnFilter_i::GetPredicate( SMESH::Filter_ptr filter )
{
  SMESH_PredicatePtr predicate;

  if ( SMESH::Filter_i* filt_i = SMESH::DownCast< SMESH::Filter_i* >( filter ))
    if ( SMESH::Predicate_i* predic_i= filt_i->GetPredicate_i() )
      predicate = predic_i->GetPredicate();

  return predicate;
}

//================================================================================
/*!
 * \brief Sets the filter defining group contents
 */
//================================================================================

void SMESH_GroupOnFilter_i::SetFilter(SMESH::Filter_ptr theFilter)
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  if ( ! myFilter->_is_nil() )
    myFilter->UnRegister();

  myFilter = SMESH::Filter::_duplicate( theFilter );

  if ( SMESHDS_GroupOnFilter* grDS = dynamic_cast< SMESHDS_GroupOnFilter*>( GetGroupDS() ))
    grDS->SetPredicate( GetPredicate( myFilter ));

  TPythonDump()<< _this() <<".SetFilter( "<<theFilter<<" )";

  if ( myFilter )
  {
    myFilter->Register();
    SMESH::DownCast< SMESH::Filter_i* >( myFilter )->AddWaiter( this );
  }
}

//================================================================================
/*!
 * \brief Returns the filter defining group contents
 */
//================================================================================

SMESH::Filter_ptr SMESH_GroupOnFilter_i::GetFilter()
{
  SMESH::Filter_var f = myFilter;
  TPythonDump() << f << " = " << _this() << ".GetFilter()";
  return f._retn();
}

#define SEPAR '^'

//================================================================================
/*!
 * \brief Return a string to be used to store group definition in the study
 */
//================================================================================

std::string SMESH_GroupOnFilter_i::FilterToString() const
{
  SMESH_Comment result;
  SMESH::Filter::Criteria_var criteria;
  if ( !myFilter->_is_nil() && myFilter->GetCriteria( criteria.out() ))
  {
    result << criteria->length() << SEPAR;
    for ( unsigned i = 0; i < criteria->length(); ++i )
    {
      // write FunctorType as string but not as number to assure correct
      // persistence if enum FunctorType is modified by insertion in the middle
      SMESH::Filter::Criterion& crit = criteria[ i ];
      result << SMESH::FunctorTypeToString( SMESH::FunctorType( crit.Type ))    << SEPAR;
      result << SMESH::FunctorTypeToString( SMESH::FunctorType( crit.Compare )) << SEPAR;
      result << crit.Threshold                                                  << SEPAR;
      result << crit.ThresholdStr                                               << SEPAR;
      result << crit.ThresholdID                                                << SEPAR;
      result << SMESH::FunctorTypeToString( SMESH::FunctorType( crit.UnaryOp )) << SEPAR;
      result << SMESH::FunctorTypeToString( SMESH::FunctorType( crit.BinaryOp ))<< SEPAR;
      result << crit.Tolerance                                                  << SEPAR;
      result << crit.TypeOfElement                                              << SEPAR;
      result << crit.Precision                                                  << SEPAR;
    }
  }
  return result;
}

//================================================================================
/*!
 * \brief Restore the filter by the persistent string
 */
//================================================================================

SMESH::Filter_ptr SMESH_GroupOnFilter_i::StringToFilter(const std::string& thePersistStr )
{
  SMESH::Filter_var filter;

  // divide thePersistStr into sub-strings
  std::vector< std::string > strVec;
  std::string::size_type from = 0, to;
  while ( from < thePersistStr.size() )
  {
    to = thePersistStr.find( SEPAR, from );
    if ( to == std::string::npos )
      break;
    strVec.push_back( thePersistStr.substr( from, to-from ));
    from = to+1;
  }
  if ( strVec.empty() || strVec[0] == "0" )
    return filter._retn();
#undef SEPAR

  // create Criteria
  int nbCrit = atoi( strVec[0].c_str() );
  SMESH::Filter::Criteria_var criteria = new SMESH::Filter::Criteria;
  criteria->length( nbCrit );
  int nbStrPerCrit = ( strVec.size() - 1 ) / nbCrit;
  for ( int i = 0; i < nbCrit; ++i )
  {
    SMESH::Filter::Criterion& crit = criteria[ i ];
    int iStr = 1 + i * nbStrPerCrit;
    crit.Type         = SMESH::StringToFunctorType( strVec[ iStr++ ].c_str() );
    crit.Compare      = SMESH::StringToFunctorType( strVec[ iStr++ ].c_str() );
    crit.Threshold    = atof(                       strVec[ iStr++ ].c_str() );
    crit.ThresholdStr =                             strVec[ iStr++ ].c_str();
    crit.ThresholdID  =                             strVec[ iStr++ ].c_str();
    crit.UnaryOp      = SMESH::StringToFunctorType( strVec[ iStr++ ].c_str() );
    crit.BinaryOp     = SMESH::StringToFunctorType( strVec[ iStr++ ].c_str() );
    crit.Tolerance    = atof(                       strVec[ iStr++ ].c_str() );
    crit.TypeOfElement= SMESH::ElementType( atoi(   strVec[ iStr++ ].c_str() ));
    crit.Precision    = atoi(                       strVec[ iStr++ ].c_str() );
  }

  // create a filter
  TPythonDump pd;
  SMESH::FilterManager_i* aFilterMgr = new SMESH::FilterManager_i();
  filter = aFilterMgr->CreateFilter();
  filter->SetCriteria( criteria.inout() );
  
  aFilterMgr->UnRegister();

  pd << ""; // to avoid optimizing pd out

  return filter._retn();
}

//================================================================================
/*!
 * \brief Destructor of SMESH_GroupOnFilter_i
 */
//================================================================================

SMESH_GroupOnFilter_i::~SMESH_GroupOnFilter_i()
{
  if ( ! myFilter->_is_nil() )
  {
    SMESH::DownCast< SMESH::Filter_i* >( myFilter )->RemoveWaiter( this );
    myFilter->UnRegister();
  }
}

//================================================================================
/*!
 * \brief Method calleds when a predicate of myFilter changes
 */
//================================================================================

void SMESH_GroupOnFilter_i::PredicateChanged()
{
  if ( myPreMeshInfo )
    myPreMeshInfo->FullLoadFromFile();

  if ( SMESHDS_GroupOnFilter* grDS = dynamic_cast< SMESHDS_GroupOnFilter*>( GetGroupDS() ))
    grDS->SetPredicate( GetPredicate( myFilter ));
}
