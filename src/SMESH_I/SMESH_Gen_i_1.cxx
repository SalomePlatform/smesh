// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File      : SMESH_Gen_i_1.cxx
//  Created   : Thu Oct 21 17:24:06 2004
//  Author    : Edward AGAPOV (eap)
//  Module    : SMESH

#include "SMESH_Gen_i.hxx"

#include "SMESHDS_Mesh.hxx"
#include "SMESH_Algo_i.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_subMesh_i.hxx"

#include <utilities.h>
#include <Utils_ExceptHandlers.hxx>
#include <SALOMEDS_wrap.hxx>
#include <SALOMEDS_Attributes_wrap.hxx>
#include "SALOME_KernelServices.hxx"
#include "SALOME_ModuleCatalog_impl.hxx"

#include <TCollection_AsciiString.hxx>
#include <TopoDS_Solid.hxx>

#include <cctype>

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;

//=============================================================================
/*!
 *  Get...Tag [ static ]
 *
 *  Methods which determine SMESH data model structure
 */
//=============================================================================

long SMESH_Gen_i::GetHypothesisRootTag()
{
  return SMESH::Tag_HypothesisRoot;
}

long SMESH_Gen_i::GetAlgorithmsRootTag()
{
  return SMESH::Tag_AlgorithmsRoot;
}

long SMESH_Gen_i::GetRefOnShapeTag()
{
  return SMESH::Tag_RefOnShape;
}

long SMESH_Gen_i::GetRefOnAppliedHypothesisTag()
{
  return SMESH::Tag_RefOnAppliedHypothesis;
}

long SMESH_Gen_i::GetRefOnAppliedAlgorithmsTag()
{
  return SMESH::Tag_RefOnAppliedAlgorithms;
}

long SMESH_Gen_i::GetSubMeshOnVertexTag()
{
  return SMESH::Tag_SubMeshOnVertex;
}

long SMESH_Gen_i::GetSubMeshOnEdgeTag()
{
  return SMESH::Tag_SubMeshOnEdge;
}

long SMESH_Gen_i::GetSubMeshOnFaceTag()
{
  return SMESH::Tag_SubMeshOnFace;
}

long SMESH_Gen_i::GetSubMeshOnSolidTag()
{
  return SMESH::Tag_SubMeshOnSolid;
}

long SMESH_Gen_i::GetSubMeshOnCompoundTag()
{
  return SMESH::Tag_SubMeshOnCompound;
}

long SMESH_Gen_i::GetSubMeshOnWireTag()
{
  return SMESH::Tag_SubMeshOnWire;
}

long SMESH_Gen_i::GetSubMeshOnShellTag()
{
  return SMESH::Tag_SubMeshOnShell;
}

long SMESH_Gen_i::GetNodeGroupsTag()
{
  return SMESH::Tag_NodeGroups;
}

long SMESH_Gen_i::GetEdgeGroupsTag()
{
  return SMESH::Tag_EdgeGroups;
}

long SMESH_Gen_i::GetFaceGroupsTag()
{
  return SMESH::Tag_FaceGroups;
}

long SMESH_Gen_i::GetVolumeGroupsTag()
{
  return SMESH::Tag_VolumeGroups;
}

long SMESH_Gen_i::Get0DElementsGroupsTag()
{
  return SMESH::Tag_0DElementsGroups;
}

long SMESH_Gen_i::GetBallElementsGroupsTag()
{
  return SMESH::Tag_BallElementsGroups;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CanPublishInStudy
 *
 *  Returns true if object can be published in the study
 */
//=============================================================================

bool SMESH_Gen_i::CanPublishInStudy(CORBA::Object_ptr theIOR)
{
  if(MYDEBUG) MESSAGE("CanPublishInStudy - "<<!CORBA::is_nil(getStudyServant()));
  
  if ( !myIsEnablePublish )
    return false;

  SMESH::SMESH_Mesh_var aMesh       = SMESH::SMESH_Mesh::_narrow(theIOR);
  if( !aMesh->_is_nil() )
    return true;

  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(theIOR);
  if( !aSubMesh->_is_nil() )
    return true;

  SMESH::SMESH_Hypothesis_var aHyp  = SMESH::SMESH_Hypothesis::_narrow(theIOR);
  if( !aHyp->_is_nil() )
    return true;

  SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(theIOR);
  if( !aGroup->_is_nil() )
    return true;

  if(MYDEBUG) MESSAGE("CanPublishInStudy--CANT");
  return false;
}

//=======================================================================
//function : ObjectToSObject
//purpose  : Put a result into a SALOMEDS::SObject_wrap or call UnRegister()!
//=======================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::ObjectToSObject(CORBA::Object_ptr theObject)
{
  SALOMEDS::SObject_wrap aSO;
  try {
    if ( !CORBA::is_nil( theObject ))
    {
      CORBA::String_var objStr = SMESH_Gen_i::GetORB()->object_to_string( theObject );
      aSO = getStudyServant()->FindObjectIOR( objStr.in() );
    }
  }
  catch (...)
  {
  }
  return aSO._retn();
}

//=======================================================================
//function : objectToServant
//purpose  : 
//=======================================================================

template<typename T> static inline T* objectToServant( CORBA::Object_ptr theIOR )
{
  return dynamic_cast<T*>( SMESH_Gen_i::GetServant( theIOR ).in() );
}

//=======================================================================
//function : ShapeToGeomObject
//purpose  : 
//=======================================================================

GEOM::GEOM_Object_ptr SMESH_Gen_i::ShapeToGeomObject (const TopoDS_Shape& theShape )
{
  GEOM::GEOM_Object_var aShapeObj;
  if ( !theShape.IsNull() && mySMESHGen ) {
    GEOM_Client* aClient = mySMESHGen->GetShapeReader();
    TCollection_AsciiString IOR;
    if ( aClient && aClient->Find( theShape, IOR ))
    {
      CORBA::Object_var obj = GetORB()->string_to_object( IOR.ToCString() );
      if ( !obj->_non_existent() )
        aShapeObj = GEOM::GEOM_Object::_narrow ( obj );
    }
  }
  return aShapeObj._retn();
}

//=======================================================================
//function : GeomObjectToShape
//purpose  : 
//=======================================================================

TopoDS_Shape SMESH_Gen_i::GeomObjectToShape(GEOM::GEOM_Object_ptr theGeomObject)
{
  TopoDS_Shape S;
  if ( mySMESHGen && !theGeomObject->_is_nil() && !theGeomObject->_non_existent() )
  {
    GEOM_Client*           aClient = mySMESHGen->GetShapeReader();
    GEOM::GEOM_Gen_var aGeomEngine = GetGeomEngine( theGeomObject );
    if ( aClient && !aGeomEngine->_is_nil () )
      S = aClient->GetShape( aGeomEngine, theGeomObject );
  }
  return S;
}

//================================================================================
/*!
 * \brief Get GEOM Object by its study entry
 */
//================================================================================

GEOM::GEOM_Object_ptr SMESH_Gen_i::GetGeomObjectByEntry( const std::string& entry )
{
  GEOM::GEOM_Object_var go;
  if ( !entry.empty() && mySMESHGen )
  {
    SALOMEDS::SObject_wrap so = mySMESHGen->getStudyServant()->FindObjectID( entry.c_str() );
    CORBA::Object_var     obj = SObjectToObject( so );
    go = GEOM::GEOM_Object::_narrow( obj );
  }
  return go._retn();
}


//=======================================================================
//function : publish
//purpose  :
//=======================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::publish(CORBA::Object_ptr     theIOR,
                                     SALOMEDS::SObject_ptr theFatherObject,
                                     const int             theTag,
                                     const char*           thePixMap,
                                     const bool            theSelectable)
{
  SALOMEDS::Study_var theStudy = getStudyServant();
  SALOMEDS::SObject_wrap SO = ObjectToSObject( theIOR );
  SALOMEDS::StudyBuilder_var     aStudyBuilder = theStudy->NewBuilder();
  SALOMEDS::UseCaseBuilder_wrap useCaseBuilder = theStudy->GetUseCaseBuilder();
  bool isNewSO = false, isInUseCaseTree = false;
  if ( SO->_is_nil() )
  {
    if ( theTag == 0 ) {
      SO = aStudyBuilder->NewObject( theFatherObject );
      isNewSO = true;
    }
    else if ( !theFatherObject->FindSubObject( theTag, SO.inout() ))
    {
      SO = aStudyBuilder->NewObjectToTag( theFatherObject, theTag );
      isNewSO = true;
    }
    else
    {
      isInUseCaseTree = useCaseBuilder->IsUseCaseNode( SO );
    }
  }
  else
  {
    isInUseCaseTree = useCaseBuilder->IsUseCaseNode( SO );
  }

  SALOMEDS::GenericAttribute_wrap anAttr;
  if ( !CORBA::is_nil( theIOR ))
  {
    anAttr = aStudyBuilder->FindOrCreateAttribute( SO, "AttributeIOR" );
    CORBA::String_var objStr = SMESH_Gen_i::GetORB()->object_to_string( theIOR );
    SALOMEDS::AttributeIOR_wrap iorAttr = anAttr;
    CORBA::String_var objStrCur = iorAttr->Value();
    bool sameIOR = ( objStrCur.in() && strcmp( objStr.in(), objStrCur.in() ) == 0 );
    if ( !sameIOR )
    {
      iorAttr->SetValue( objStr.in() );
      // UnRegister() !!! --> random problems when meshing in parallel (yacs foreach) in
      // distributed python scripts, because simultaneously created meshes are
      // published into the same SO; as a result the mesh published first dies
      SALOME::GenericObj_var genObj = SALOME::GenericObj::_narrow( theIOR );
      if ( !genObj->_is_nil() )
        genObj->UnRegister();
    }
  }

  if ( thePixMap ) {
    anAttr  = aStudyBuilder->FindOrCreateAttribute( SO, "AttributePixMap" );
    SALOMEDS::AttributePixMap_wrap pm = anAttr;
    pm->SetPixMap( thePixMap );
  }

  if ( !theSelectable ) {
    anAttr = aStudyBuilder->FindOrCreateAttribute( SO, "AttributeSelectable" );
    SALOMEDS::AttributeSelectable_wrap selAttr = anAttr;
    selAttr->SetSelectable( false );
  }

  if ( !isNewSO )
    aStudyBuilder->RemoveReference( SO );// remove garbage reference (23336)

  // add object to the use case tree
  // (to support tree representation customization and drag-n-drop)
  if ( isNewSO || !isInUseCaseTree )
  {
    // define the next tag after given one in the data tree to insert SObject
    SALOMEDS::SObject_wrap curObj, objAfter;
    if ( theFatherObject->GetLastChildTag() > theTag && theTag > 0 )
    {
      SALOMEDS::UseCaseIterator_wrap
        anUseCaseIter = useCaseBuilder->GetUseCaseIterator(theFatherObject);
      for ( ; anUseCaseIter->More(); anUseCaseIter->Next() ) {
        curObj = anUseCaseIter->Value();
        if ( curObj->Tag() > theTag  ) {
          objAfter = curObj;
          break;
        }
      }
    }
    if ( !CORBA::is_nil( objAfter ))
      useCaseBuilder->InsertBefore( SO, objAfter );    // insert at given tag
    else if ( !isInUseCaseTree )
      useCaseBuilder->AppendTo( theFatherObject, SO ); // append to the end of list
  }
  return SO._retn();
}

//=======================================================================
//function : setName
//purpose  :
//=======================================================================

void SMESH_Gen_i::SetName(SALOMEDS::SObject_ptr theSObject,
                          const char*           theName,
                          const char*           theDefaultName)
{
  if ( !theSObject->_is_nil() ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
    SALOMEDS::GenericAttribute_wrap   anAttr =
      aStudyBuilder->FindOrCreateAttribute( theSObject, "AttributeName" );
    SALOMEDS::AttributeName_wrap aNameAttr = anAttr;
    if ( theName && theName[0] ) {
      std::string name( theName ); // trim trailing white spaces
      for ( size_t i = name.size()-1; i > 0; --i )
        if ( isspace( name[i] )) name[i] = '\0';
        else                     break;
      aNameAttr->SetValue( name.c_str() );
    }
    else
    {
      CORBA::String_var curName = aNameAttr->Value();
      if ( strlen( curName.in() ) == 0 ) {
        SMESH_Comment aName(theDefaultName);
        aNameAttr->SetValue( ( aName<< "_" << theSObject->Tag()).c_str() );
      }
    }
  }
}

//=======================================================================
//function : SetPixMap
//purpose  :
//=======================================================================

void SMESH_Gen_i::SetPixMap(SALOMEDS::SObject_ptr theSObject,
                            const char*           thePixMap)
{
  if ( !theSObject->_is_nil() && thePixMap && thePixMap && thePixMap[0] )
  {
    SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
    SALOMEDS::GenericAttribute_wrap anAttr =
      aStudyBuilder->FindOrCreateAttribute( theSObject, "AttributePixMap" );
    SALOMEDS::AttributePixMap_wrap aPMAttr = anAttr;
    aPMAttr->SetPixMap( thePixMap );
  }
}

//=======================================================================
//function : addReference
//purpose  : 
//=======================================================================

void SMESH_Gen_i::addReference (SALOMEDS::SObject_ptr theSObject, CORBA::Object_ptr theToObject, int theTag)
{
  SALOMEDS::Study_var aStudy = SMESH_Gen_i::getStudyServant();
  SALOMEDS::SObject_wrap aToObjSO = SMESH_Gen_i::ObjectToSObject( theToObject );
  if ( !aToObjSO->_is_nil() && !theSObject->_is_nil() ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();
    SALOMEDS::SObject_wrap aReferenceSO;
    if ( !theTag ) {
      // check if the reference to theToObject already exists
      // and find a free label for the reference object
      bool isReferred = false;
      int tag = 1;
      SALOMEDS::ChildIterator_wrap anIter = aStudy->NewChildIterator( theSObject );
      for ( ; !isReferred && anIter->More(); anIter->Next(), ++tag ) {
        SALOMEDS::SObject_wrap curSO = anIter->Value();
        if ( curSO->ReferencedObject( aReferenceSO.inout() )) {
          CORBA::String_var refEntry = aReferenceSO->GetID();
          CORBA::String_var  toEntry = aToObjSO->GetID();
          if ( strcmp( refEntry, toEntry ) == 0 )
            isReferred = true;
        }
        else if ( !theTag ) {
          SALOMEDS::GenericAttribute_wrap anAttr;
          if ( !curSO->FindAttribute( anAttr.inout(), "AttributeIOR" ))
            theTag = tag;
        }
      }
      if ( isReferred )
        return;
      if ( !theTag )
        theTag = tag;
    }
    if ( !theSObject->FindSubObject( theTag, aReferenceSO.inout() ))
      aReferenceSO = aStudyBuilder->NewObjectToTag( theSObject, theTag );

    aStudyBuilder->Addreference( aReferenceSO, aToObjSO );

    // make the reference visible (invisible ref is created by createInvalidSubMesh())
    aStudyBuilder->RemoveAttribute( aReferenceSO, "AttributeDrawable" );

    // add reference to the use case tree
    // (to support tree representation customization and drag-n-drop)
    SALOMEDS::UseCaseBuilder_wrap  useCaseBuilder = aStudy->GetUseCaseBuilder();
    SALOMEDS::UseCaseIterator_wrap    useCaseIter = useCaseBuilder->GetUseCaseIterator(theSObject);
    for ( ; useCaseIter->More(); useCaseIter->Next() )
    {
      SALOMEDS::SObject_wrap curSO = useCaseIter->Value();
      if ( curSO->Tag() == theTag )
        return;
    }
    useCaseBuilder->AppendTo( theSObject, aReferenceSO );
  }
}

//=============================================================================
/*!
 *  SMESH_Gen_i::PublishInStudy
 *
 *  Publish object in the study
 */
//=============================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::PublishInStudy(SALOMEDS::SObject_ptr /*theSObject*/,
                                                  CORBA::Object_ptr     theIOR,
                                                  const char*           theName)
{
  Unexpect aCatch(SALOME_SalomeException);
  SALOMEDS::SObject_wrap aSO;
  if ( !myIsEnablePublish )
    return aSO._retn();
  if ( CORBA::is_nil( theIOR ))
    return aSO._retn();
  if(MYDEBUG) MESSAGE("PublishInStudy");

  // Publishing a mesh
  SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( theIOR );
  if( !aMesh->_is_nil() )
    aSO = PublishMesh( aMesh, theName );

  // Publishing a sub-mesh
  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( theIOR );
  if( aSO->_is_nil() && !aSubMesh->_is_nil() ) {
    GEOM::GEOM_Object_var aShapeObject = aSubMesh->GetSubShape();
    aMesh = aSubMesh->GetFather();
    aSO = PublishSubMesh( aMesh, aSubMesh, aShapeObject, theName );
  }

  // Publishing a hypothesis or algorithm
  SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow( theIOR );
  if ( aSO->_is_nil() && !aHyp->_is_nil() )
    aSO = PublishHypothesis( aHyp, theName );

  // Publishing a group
  SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(theIOR);
  if ( aSO->_is_nil() && !aGroup->_is_nil() ) {
    GEOM::GEOM_Object_var aShapeObject;
    aMesh = aGroup->GetMesh();
    aSO = PublishGroup( aMesh, aGroup, aShapeObject, theName );
  }
  if(MYDEBUG) MESSAGE("PublishInStudy_END");

  return aSO._retn();
}

//=======================================================================
//function : PublishComponent
//purpose  : 
//=======================================================================

SALOMEDS::SComponent_ptr SMESH_Gen_i::PublishComponent()
{
  if(MYDEBUG) MESSAGE("PublishComponent");
  if ( !myIsEnablePublish )
    return SALOMEDS::SComponent::_nil();

  SALOMEDS::StudyBuilder_var    aStudyBuilder  = getStudyServant()->NewBuilder();
  SALOMEDS::UseCaseBuilder_wrap useCaseBuilder = getStudyServant()->GetUseCaseBuilder();

  std::string compDataType = ComponentDataType(); // SMESH module's data type
  std::string ior = SMESH_Gen_i::GetORB()->object_to_string( SMESH_Gen::_this() ); // IOR of this SMESH engine

  // Find study component which corresponds to this SMESH engine

  SALOMEDS::SComponent_wrap father;
  SALOMEDS::SComponentIterator_wrap citer = getStudyServant()->NewComponentIterator();
  for ( ; citer->More(); citer->Next()) {
    SALOMEDS::SComponent_wrap f_i = citer->Value();
    CORBA::String_var ior_i;
    bool ok = f_i->ComponentIOR(ior_i.out());
    if ( ok && compDataType == f_i->ComponentDataType() && ior == ior_i.in()) {
      father = f_i;
      break;
    }
  }
  
  if ( !CORBA::is_nil( father ) ) {
    // check that the component is added to the use case browser
    if ( !useCaseBuilder->IsUseCaseNode( father ) ) {
      useCaseBuilder->SetRootCurrent();
      useCaseBuilder->Append( father ); // component object is added as the top level item
    }
    return father._retn();
  }

  // If component for this SMESH engine does not exist in the study, create it

  SALOME_ModuleCatalog::ModuleCatalog_var aCat = this->getModuleCatalog();
  if ( CORBA::is_nil( aCat ) )
    return father._retn();

  SALOME_ModuleCatalog::Acomponent_var aComp = aCat->GetComponent( compDataType.c_str() );
  if ( CORBA::is_nil( aComp ) )
    return father._retn();

  SALOMEDS::GenericAttribute_wrap anAttr;
  SALOMEDS::AttributePixMap_wrap  aPixmap;

  father  = aStudyBuilder->NewComponent( compDataType.c_str() );
  aStudyBuilder->DefineComponentInstance( father, SMESH_Gen::_this() );
  anAttr  = aStudyBuilder->FindOrCreateAttribute( father, "AttributePixMap" );
  aPixmap = anAttr;
  aPixmap->SetPixMap( "ICON_OBJBROWSER_SMESH" );
  CORBA::String_var userName = aComp->componentusername();
  SetName( father, userName.in(), "MESH" );
  // add component to the use case tree
  // (to support tree representation customization and drag-n-drop)
  useCaseBuilder->SetRootCurrent();
  useCaseBuilder->Append( father ); // component object is added as the top level item
  if(MYDEBUG) MESSAGE("PublishComponent--END");

  return father._retn();
}

//=======================================================================
//function : PublishMesh
//purpose  : 
//=======================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::PublishMesh (SMESH::SMESH_Mesh_ptr theMesh,
                                                const char*           theName)
{
  if ( !myIsEnablePublish )
    return SALOMEDS::SObject::_nil();
  if ( CORBA::is_nil( theMesh ))
    return SALOMEDS::SObject::_nil();
  if(MYDEBUG) MESSAGE("PublishMesh--IN");

  // find or publish a mesh

  SALOMEDS::SObject_wrap aMeshSO = ObjectToSObject( theMesh );
  if ( aMeshSO->_is_nil() )
  {
    SALOMEDS::SComponent_wrap father = PublishComponent();
    if ( father->_is_nil() )
      return aMeshSO._retn();

    // Find correct free tag
    long aTag = father->GetLastChildTag();
    if ( aTag <= GetAlgorithmsRootTag() )
      aTag = GetAlgorithmsRootTag() + 1;
    else
      aTag++;

    aMeshSO = publish( theMesh, father, aTag, "ICON_SMESH_TREE_MESH_WARN" );
    if ( aMeshSO->_is_nil() )
      return aMeshSO._retn();
  }
  SetName( aMeshSO, theName, "Mesh" );

  // Add shape reference

  GEOM::GEOM_Object_var aShapeObject = theMesh->GetShapeToMesh();
  if ( !CORBA::is_nil( aShapeObject )) {
    addReference( aMeshSO, aShapeObject, GetRefOnShapeTag() );

    // Publish global hypotheses

    SMESH::ListOfHypothesis_var hypList = theMesh->GetHypothesisList( aShapeObject );
    for ( CORBA::ULong i = 0; i < hypList->length(); i++ )
    {
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow( hypList[ i ]);
      SALOMEDS::SObject_wrap so = PublishHypothesis( aHyp );
      AddHypothesisToShape( theMesh, aShapeObject, aHyp );
    }
  }

  // Publish submeshes

  SMESH_Mesh_i* mesh_i = objectToServant<SMESH_Mesh_i>( theMesh );
  if ( !mesh_i )
    return aMeshSO._retn();
  map<int, SMESH_subMesh_i*>& subMap = mesh_i->_mapSubMesh_i;
  map<int, SMESH_subMesh_i*>::iterator subIt = subMap.begin();
  for ( ; subIt != subMap.end(); subIt++ ) {
    SMESH::SMESH_subMesh_ptr aSubMesh = (*subIt).second->_this();
    if ( !CORBA::is_nil( aSubMesh )) {
      aShapeObject = aSubMesh->GetSubShape();
      SALOMEDS::SObject_wrap( PublishSubMesh( theMesh, aSubMesh, aShapeObject ));
    }
  }

  // Publish groups
  const map<int, SMESH::SMESH_GroupBase_ptr>& grMap = mesh_i->getGroups();
  map<int, SMESH::SMESH_GroupBase_ptr>::const_iterator it = grMap.begin();
  for ( ; it != grMap.end(); it++ )
  {
    SMESH::SMESH_GroupBase_ptr aGroup = (*it).second;
    if ( !aGroup->_is_nil() ) {
      GEOM::GEOM_Object_var aShapeObj;
      SMESH::SMESH_GroupOnGeom_var aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGroup );
      if ( !aGeomGroup->_is_nil() )
        aShapeObj = aGeomGroup->GetShape();
      SALOMEDS::SObject_wrap( PublishGroup( theMesh, aGroup, aShapeObj ));
    }
  }

  if(MYDEBUG) MESSAGE("PublishMesh_END");
  return aMeshSO._retn();
}

//=======================================================================
//function : PublishSubMesh
//purpose  : 
//=======================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::PublishSubMesh (SMESH::SMESH_Mesh_ptr    theMesh,
                                                   SMESH::SMESH_subMesh_ptr theSubMesh,
                                                   GEOM::GEOM_Object_ptr    theShapeObject,
                                                   const char*              theName)
{
  if ( !myIsEnablePublish )
    return SALOMEDS::SObject::_nil();
  if ( theMesh->_is_nil() || theSubMesh->_is_nil() || theShapeObject->_is_nil() )
    return SALOMEDS::SObject::_nil();

  std::string newName( theName ? theName : "" );

  SALOMEDS::SObject_wrap aSubMeshSO = ObjectToSObject( theSubMesh );
  if ( aSubMeshSO->_is_nil() )
  {
    SALOMEDS::SObject_wrap aMeshSO = ObjectToSObject( theMesh );
    if ( aMeshSO->_is_nil() ) {
      aMeshSO = PublishMesh( theMesh );
      if ( aMeshSO->_is_nil())
        return SALOMEDS::SObject::_nil();
    }
    // Find submesh sub-tree tag
    long aRootTag;
    const char* aRootName = "";
    switch ( theShapeObject->GetShapeType() ) {
    case GEOM::VERTEX:
      aRootTag  = GetSubMeshOnVertexTag();
      aRootName = "SubMeshes on Vertex";
      break;
    case GEOM::EDGE:
      aRootTag  = GetSubMeshOnEdgeTag();
      aRootName = "SubMeshes on Edge";
      break;
    case GEOM::WIRE:
      aRootTag  = GetSubMeshOnWireTag();
      aRootName = "SubMeshes on Wire";
      break;
    case GEOM::FACE:
      aRootTag  = GetSubMeshOnFaceTag();
      aRootName = "SubMeshes on Face";    
      break;
    case GEOM::SHELL:
      aRootTag  = GetSubMeshOnShellTag();
      aRootName = "SubMeshes on Shell";   
      break;
    case GEOM::SOLID:
      aRootTag  = GetSubMeshOnSolidTag();
      aRootName = "SubMeshes on Solid";
      break;
    default:
      aRootTag  = GetSubMeshOnCompoundTag();
      aRootName = "SubMeshes on Compound";
      break;
    }

    // Find or create submesh root
    SALOMEDS::SObject_wrap aRootSO = publish ( CORBA::Object::_nil(),
                                               aMeshSO, aRootTag, 0, false );
    if ( aRootSO->_is_nil() )
      return aSubMeshSO._retn();

    SetName( aRootSO, aRootName );

    // Add new sub-mesh to corresponding sub-tree
    int tag = 0; // to use a new SObject
    if ( theName && theName[0] == '0' )
    {
      // theName can be an entry of an invalid sub-mesh which theSubMesh should replace
      SALOMEDS::SObject_wrap aSObj = getStudyServant()->FindObjectID( theName );
      if ( aSObj )
      {
        CORBA::String_var oldName = aSObj->GetName();
        newName = oldName.in();
        SALOMEDS::SObject_wrap aRootSO2 = aSObj->GetFather();
        if ( aRootSO->Tag() == aRootSO2->Tag() ) // same parent
          tag = aSObj->Tag(); // to use the same SObject
        else
        {
          CORBA::Object_var anObj = SObjectToObject( aSObj );
          SMESH::SMESH_subMesh_var sm = SMESH::SMESH_subMesh::_narrow( anObj );
          theMesh->RemoveSubMesh( sm );
        }
      }
    }
    SMESH::array_of_ElementType_var elemTypes = theSubMesh->GetTypes();
    const int isEmpty = ( elemTypes->length() == 0 );
    const char* pm[2] = { "ICON_SMESH_TREE_MESH", "ICON_SMESH_TREE_MESH_WARN" };
    aSubMeshSO = publish ( theSubMesh, aRootSO, tag, pm[isEmpty] );
    if ( aSubMeshSO->_is_nil() )
      return aSubMeshSO._retn();

    highLightInvalid( aSubMeshSO, false ); // now it is valid
  }
  SetName( aSubMeshSO, newName.c_str(), "SubMesh" );

  // Add reference to theShapeObject

  addReference( aSubMeshSO, theShapeObject, GetRefOnShapeTag() );

  // Publish hypothesis

  SMESH::ListOfHypothesis_var hypList = theMesh->GetHypothesisList( theShapeObject );
  for ( CORBA::ULong i = 0; i < hypList->length(); i++ )
  {
    SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow( hypList[ i ]);
    SALOMEDS::SObject_wrap so = PublishHypothesis( aHyp );
    AddHypothesisToShape( theMesh, theShapeObject, aHyp );
  }

  return aSubMeshSO._retn();
}

//=======================================================================
//function : PublishGroup
//purpose  : 
//=======================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::PublishGroup (SMESH::SMESH_Mesh_ptr  theMesh,
                                                 SMESH::SMESH_GroupBase_ptr theGroup,
                                                 GEOM::GEOM_Object_ptr  theShapeObject,
                                                 const char*            theName)
{
  if ( !myIsEnablePublish )
    return SALOMEDS::SObject::_nil();
  if (theMesh->_is_nil() || theGroup->_is_nil() )
    return SALOMEDS::SObject::_nil();

  SALOMEDS::SObject_wrap aGroupSO = ObjectToSObject( theGroup );
  if ( aGroupSO->_is_nil() )
  {
    SALOMEDS::SObject_wrap aMeshSO = ObjectToSObject( theMesh );
    if ( aMeshSO->_is_nil() ) {
      aMeshSO = PublishInStudy( SALOMEDS::SObject::_nil(), theMesh, "");
      if ( aMeshSO->_is_nil())
        return SALOMEDS::SObject::_nil();
    }
    size_t aType = (int)theGroup->GetType();
    const char* aRootNames[] = {
      "Compound Groups", "Groups of Nodes", "Groups of Edges",
      "Groups of Faces", "Groups of Volumes", "Groups of 0D Elements",
      "Groups of Balls" };

    // Currently, groups with heterogeneous content are not supported
    if ( aType != SMESH::ALL )
    {
      long aRootTag = GetNodeGroupsTag() + aType - 1;

      // Find or create groups root
      SALOMEDS::SObject_wrap aRootSO = publish ( CORBA::Object::_nil(),
                                                 aMeshSO, aRootTag, 0, false );
      if ( aRootSO->_is_nil() ) return SALOMEDS::SObject::_nil();

      if ( aType < sizeof(aRootNames)/sizeof(char*) )
        SetName( aRootSO, aRootNames[aType] );

      // Add new group to corresponding sub-tree
      int isEmpty = ( theMesh->NbNodes() == 0 );
      std::string pm[2] = { "ICON_SMESH_TREE_GROUP", "ICON_SMESH_TREE_MESH_WARN" };
      if ( !isEmpty )
      {
        if ( SMESH::DownCast< SMESH_GroupOnFilter_i* > ( theGroup )) // on filter
        {
          pm[0] = "ICON_SMESH_TREE_GROUP_ON_FILTER";

          if ( theGroup->GetType() != SMESH::NODE )
          {
            isEmpty = true;
            SMESH::array_of_ElementType_var allElemTypes = theMesh->GetTypes();
            for ( size_t i =0; i < allElemTypes->length() && isEmpty; ++i )
              isEmpty = ( allElemTypes[i] != theGroup->GetType() );
          }
        }
        else // standalone or on geometry
        {
          isEmpty = ( theGroup->Size() == 0 );
        }
      }
      aGroupSO = publish ( theGroup, aRootSO, 0, pm[isEmpty].c_str() );
    }
    if ( aGroupSO->_is_nil() )
      return aGroupSO._retn();
  }

  SetName( aGroupSO, theName, "Group" );

  //Add reference to geometry
  if ( !theShapeObject->_is_nil() )
    addReference( aGroupSO, theShapeObject, GetRefOnShapeTag() );

  return aGroupSO._retn();
}

//=======================================================================
//function : PublishHypothesis
//purpose  :
//=======================================================================

SALOMEDS::SObject_ptr
SMESH_Gen_i::PublishHypothesis (SMESH::SMESH_Hypothesis_ptr theHyp,
                                const char*                 theName)
{
  if(MYDEBUG) MESSAGE("PublishHypothesis");
  if ( !myIsEnablePublish )
    return SALOMEDS::SObject::_nil();
  if (theHyp->_is_nil())
    return SALOMEDS::SObject::_nil();

  CORBA::String_var hypType = theHyp->GetName();

  SALOMEDS::SObject_wrap aHypSO = ObjectToSObject( theHyp );
  if ( aHypSO->_is_nil() )
  {
    SALOMEDS::SComponent_wrap father = PublishComponent();
    if ( father->_is_nil() )
      return aHypSO._retn();

    //Find or Create Hypothesis root
    bool isAlgo = ( !SMESH::SMESH_Algo::_narrow( theHyp )->_is_nil() );
    int aRootTag = isAlgo ? GetAlgorithmsRootTag() : GetHypothesisRootTag();
    SALOMEDS::SObject_wrap aRootSO =
      publish (CORBA::Object::_nil(),father, aRootTag,
               isAlgo ? "ICON_SMESH_TREE_ALGO" : "ICON_SMESH_TREE_HYPO", false);
    SetName( aRootSO, isAlgo ?  "Algorithms" : "Hypotheses" );

    // Add New Hypothesis
    string aPmName = isAlgo ? "ICON_SMESH_TREE_ALGO_" : "ICON_SMESH_TREE_HYPO_";
    aPmName += hypType.in();
    // prepend plugin name to pixmap name
    string pluginName = myHypCreatorMap[ hypType.in() ]->GetModuleName();
    if ( pluginName != "StdMeshers" )
      aPmName = pluginName + "::" + aPmName;
    aHypSO = publish( theHyp, aRootSO, 0, aPmName.c_str() );
  }

  SetName( aHypSO, theName, hypType.in() );

  if(MYDEBUG) MESSAGE("PublishHypothesis--END")
  return aHypSO._retn();
}

//=======================================================================
//function : UpdateIcons
//purpose  : update icons of a mesh and its children upon mesh modification
//=======================================================================

void SMESH_Gen_i::UpdateIcons( SMESH::SMESH_Mesh_ptr theMesh )
{
  SMESH_Mesh_i* mesh_i = SMESH::DownCast< SMESH_Mesh_i* >( theMesh );
  if ( ! mesh_i )
    return;

  SALOMEDS::SObject_wrap so = ObjectToSObject( theMesh );
  if ( so->_is_nil() )
    return;

  // set icon of the mesh
  if ( mesh_i->NbNodes() == 0 )
    SetPixMap( so, "ICON_SMESH_TREE_MESH_WARN" );
  else if ( mesh_i->IsComputedOK() )
    SetPixMap( so, "ICON_SMESH_TREE_MESH" );
  else if ( mesh_i->HasShapeToMesh() )
    SetPixMap( so, "ICON_SMESH_TREE_MESH_PARTIAL" );
  else
    SetPixMap( so, "ICON_SMESH_TREE_MESH_IMPORTED" );

  // set icons of sub-objects
  SALOMEDS::Study_var         study = getStudyServant();
  SALOMEDS::ChildIterator_wrap iter = study->NewChildIterator( so );
  for ( ; iter->More(); iter->Next() )
  {
    so = iter->Value(); // 1st level child - root of algos, hyps, sub-meshes or groups
    if ( so->Tag() < SMESH::Tag_FirstSubMesh )
      continue;

    SALOMEDS::ChildIterator_wrap subIter = study->NewChildIterator( so );
    for ( ; subIter->More(); subIter->Next() )
    {
      so = subIter->Value(); // 2nd level child - a sub-mesh or group

      CORBA::Object_var           obj = SObjectToObject( so );
      SMESH::SMESH_IDSource_var idSrc = SMESH::SMESH_IDSource::_narrow( obj );
      if ( idSrc->_is_nil() )
        continue;

      SMESH::SMESH_GroupBase_var     grp = SMESH::SMESH_GroupBase::_narrow( obj );
      SMESH::SMESH_GroupOnFilter_var gof = SMESH::SMESH_GroupOnFilter::_narrow( obj );
      const bool         isGroup = !grp->_is_nil();
      const bool isGroupOnFilter = !gof->_is_nil();

      bool isEmpty = ( mesh_i->NbNodes() == 0 );
      if ( !isEmpty )
      {
        if ( isGroupOnFilter ) // GetTypes() can be very long on GroupOnFilter!
        {
          SMESH::smIdType_array_var nbByType = mesh_i->GetNbElementsByType();
          isEmpty = ( nbByType[ grp->GetType() ] == 0 );
        }
        else
        {
          SMESH::array_of_ElementType_var elemTypes = idSrc->GetTypes();
          isEmpty = ( elemTypes->length() == 0 );

          if ( !isEmpty )
          {
            // check if all sub-shapes of sub-mesh on group are computed
            // (pb: "Compute sub-mesh" menu is missing if a sub-mesh is partially computed)
            SMESH::SMESH_subMesh_var subMesh = SMESH::SMESH_subMesh::_narrow( obj );
            if ( !subMesh->_is_nil() )
              if ( SMESH_subMesh* sm = mesh_i->GetImpl().GetSubMeshContaining( subMesh->GetId() ))
                if ( sm->IsComputedPartially() )
                {
                  SetPixMap( so, "ICON_SMESH_TREE_MESH_PARTIAL" );
                  continue;
                }
          }
        }
      }

      if ( isEmpty )
        SetPixMap( so, "ICON_SMESH_TREE_MESH_WARN");
      else if ( !isGroup )
        SetPixMap( so, "ICON_SMESH_TREE_MESH" );
      else if ( isGroupOnFilter )
        SetPixMap( so, "ICON_SMESH_TREE_GROUP_ON_FILTER" );
      else
        SetPixMap( so, "ICON_SMESH_TREE_GROUP" );

    } // loop on sub-meshes or groups
  } // loop on roots
}

//=======================================================================
//function : HighLightInvalid
//purpose  : change font color of a object in the Object Browser
//=======================================================================

void SMESH_Gen_i::HighLightInvalid( CORBA::Object_ptr theObject, bool isInvalid )
{
  SALOMEDS::SObject_wrap so = ObjectToSObject(theObject);
  highLightInvalid( so.in(), isInvalid );
}

//=======================================================================
//function : highLightInvalid
//purpose  : change font color of a object in the Object Browser
//=======================================================================

void SMESH_Gen_i::highLightInvalid( SALOMEDS::SObject_ptr theSObject, bool isInvalid )
{
  if ( !theSObject->_is_nil() )
  {
    SALOMEDS::StudyBuilder_var studyBuilder = getStudyServant()->NewBuilder();
    if ( isInvalid )
    {
      SALOMEDS::Color red = { 178,34,34 }; // to differ from reference color
      SALOMEDS::GenericAttribute_wrap attr =
        studyBuilder->FindOrCreateAttribute( theSObject, "AttributeTextColor" );
      SALOMEDS::AttributeTextColor_wrap colorAttr = attr;
      colorAttr->SetTextColor( red );
    }
    else
    {
      studyBuilder->RemoveAttribute( theSObject, "AttributeTextColor" );
    }
  }
}

//=======================================================================
//function : IsInvalid
//purpose  : Check object validity == absence of AttributeTextColor=(178,34,34)
//=======================================================================

bool SMESH_Gen_i::IsInvalid( SALOMEDS::SObject_ptr theSObject )
{
  bool isValid = true;
  if ( !theSObject->_is_nil() )
  {
    SALOMEDS::GenericAttribute_wrap attr;
    SALOMEDS::StudyBuilder_var studyBuilder = getStudyServant()->NewBuilder();
    if ( studyBuilder->FindAttribute( theSObject, attr.inout(), "AttributeTextColor" ))
    {
      SALOMEDS::AttributeTextColor_wrap colorAttr = attr;
      SALOMEDS::Color color = colorAttr->TextColor();
      isValid = ( color.R != 178 || color.G != 34 || color.B != 34 );
    }
  }
  return isValid;
}

//=======================================================================
//function : GetMeshOrSubmeshByShape
//purpose  : 
//=======================================================================

SALOMEDS::SObject_ptr
SMESH_Gen_i::GetMeshOrSubmeshByShape (SMESH::SMESH_Mesh_ptr theMesh,
                                      GEOM::GEOM_Object_ptr theShape)
{
  if(MYDEBUG) MESSAGE("GetMeshOrSubmeshByShape")
  SALOMEDS::SObject_wrap aMeshOrSubMesh;
  if (theMesh->_is_nil() || ( theShape->_is_nil() && theMesh->HasShapeToMesh()))
    return aMeshOrSubMesh._retn();
  
  TopoDS_Shape aShape;
  if(theMesh->HasShapeToMesh())
    aShape = GeomObjectToShape( theShape );
  else
    aShape = SMESH_Mesh::PseudoShape();

  SMESH_Mesh_i* mesh_i = objectToServant<SMESH_Mesh_i>( theMesh );

  if ( !aShape.IsNull() && mesh_i && mesh_i->GetImpl().GetMeshDS() ) {
    SMESHDS_Mesh* meshDS = mesh_i->GetImpl().GetMeshDS();
    if ( aShape.IsSame( meshDS->ShapeToMesh() ))
      aMeshOrSubMesh = ObjectToSObject( theMesh );
    else {
      int shapeID = meshDS->ShapeToIndex( aShape );
      SMESH::SMESH_subMesh_var aSubMesh = mesh_i->getSubMesh(shapeID);
      if ( !aSubMesh->_is_nil() )
        aMeshOrSubMesh = ObjectToSObject( aSubMesh );
    }
  }
  if(MYDEBUG) MESSAGE("GetMeshOrSubmeshByShape--END")
  return aMeshOrSubMesh._retn();
}

//=======================================================================
//function : AddHypothesisToShape
//purpose  : 
//=======================================================================

bool SMESH_Gen_i::AddHypothesisToShape(SMESH::SMESH_Mesh_ptr       theMesh,
                                       GEOM::GEOM_Object_ptr       theShape,
                                       SMESH::SMESH_Hypothesis_ptr theHyp)
{
  if(MYDEBUG) MESSAGE("AddHypothesisToShape")
  if (theMesh->_is_nil() ||
      theHyp->_is_nil() || (theShape->_is_nil()
                            && theMesh->HasShapeToMesh()) )
    return false;

  SALOMEDS::SObject_wrap aMeshSO = ObjectToSObject( theMesh );
  if ( aMeshSO->_is_nil() )
    aMeshSO = PublishMesh( theMesh );
  SALOMEDS::SObject_wrap aHypSO = PublishHypothesis( theHyp );
  if ( aMeshSO->_is_nil() || aHypSO->_is_nil())
    return false;

  // Find a mesh or submesh referring to theShape
  SALOMEDS::SObject_wrap aMeshOrSubMesh =
    GetMeshOrSubmeshByShape( theMesh, theShape );
  if ( aMeshOrSubMesh->_is_nil() )
  {
    // publish submesh
    TopoDS_Shape aShape = GeomObjectToShape( theShape );
    SMESH_Mesh_i* mesh_i = objectToServant<SMESH_Mesh_i>( theMesh );
    if ( !aShape.IsNull() && mesh_i && mesh_i->GetImpl().GetMeshDS() ) {
      SMESHDS_Mesh* meshDS = mesh_i->GetImpl().GetMeshDS();
      int shapeID = meshDS->ShapeToIndex( aShape );
      SMESH::SMESH_subMesh_var aSubMesh = mesh_i->getSubMesh(shapeID);
      aMeshOrSubMesh = PublishSubMesh( theMesh, aSubMesh, theShape );
    }
    if ( aMeshOrSubMesh->_is_nil() )
      return false;
  }

  //Find or Create Applied Hypothesis root
  bool aIsAlgo = !SMESH::SMESH_Algo::_narrow( theHyp )->_is_nil();
  SALOMEDS::SObject_wrap AHR =
    publish (CORBA::Object::_nil(), aMeshOrSubMesh,
             aIsAlgo ? GetRefOnAppliedAlgorithmsTag() : GetRefOnAppliedHypothesisTag(),
             aIsAlgo ? "ICON_SMESH_TREE_ALGO" : "ICON_SMESH_TREE_HYPO", false);
  SetName( AHR, aIsAlgo ? "Applied algorithms" : "Applied hypotheses" );

  addReference( AHR, theHyp );

  if(MYDEBUG) MESSAGE("AddHypothesisToShape--END")
  return true;
}

//=======================================================================
//function : RemoveHypothesisFromShape
//purpose  : 
//=======================================================================

bool SMESH_Gen_i::RemoveHypothesisFromShape(SMESH::SMESH_Mesh_ptr       theMesh,
                                            GEOM::GEOM_Object_ptr       theShape,
                                            SMESH::SMESH_Hypothesis_ptr theHyp)
{
  if (theMesh->_is_nil() ||
      theHyp->_is_nil() || (theShape->_is_nil()
                            && theMesh->HasShapeToMesh()))
    return false;

  SALOMEDS::SObject_wrap aHypSO = ObjectToSObject( theHyp );
  if ( aHypSO->_is_nil() )
    return false;

  CORBA::String_var hypEntry = aHypSO->GetID();

  // Find a mesh or sub-mesh referring to theShape
  SALOMEDS::SObject_wrap aMeshOrSubMesh =
    GetMeshOrSubmeshByShape( theMesh, theShape );
  if ( aMeshOrSubMesh->_is_nil() )
    return false;

  // Find and remove a reference to aHypSO
  SALOMEDS::SObject_wrap aRef, anObj;
  SALOMEDS::ChildIterator_wrap it = getStudyServant()->NewChildIterator( aMeshOrSubMesh );
  bool found = false;
  for ( it->InitEx( true ); ( it->More() && !found ); it->Next() ) {
    anObj = it->Value();
    if (anObj->ReferencedObject( aRef.inout() ))
    {
      CORBA::String_var refEntry = aRef->GetID();
      found = ( strcmp( refEntry, hypEntry ) == 0 );
    }
    if ( found )
    {
      SALOMEDS::StudyBuilder_var builder = getStudyServant()->NewBuilder();
      builder->RemoveObject( anObj );
    }
  }

  return true;
}

//================================================================================
/*!
 * \brief Stores names of variables that WILL be passes as parameters when calling
 *        some method of a given object.
 *  \param [in] theObject - the object whose a method WILL be called with \a theParameters.
 *  \param [in] theParameters - a string containing parameters separated by ':'.
 */
//================================================================================

void SMESH_Gen_i::UpdateParameters(CORBA::Object_ptr theObject, const char* theParameters)
{
  // find variable names within theParameters

  myLastObj.clear();
  myLastParameters.clear();
  myLastParamIndex.clear(); /* vector holding indices of virables within the string
                               of all variables used for theObject */ 
  int nbVars = 0;
  int pos = 0, prevPos = 0, len = strlen( theParameters );
  if ( len == 0 ) return;
  while ( pos <= len )
  {
    if ( pos == len || theParameters[pos] == ':' )
    {
      if ( prevPos < pos )
      {
        string val( theParameters + prevPos, theParameters + pos );
        if ( !getStudyServant()->IsVariable( val.c_str() ))
          val.clear();
        myLastParameters.push_back( val );
        nbVars += (! myLastParameters.back().empty() );
      }
      else
      {
        myLastParameters.push_back("");
      }
      prevPos = pos+1;
    }
    ++pos;
  }

  if ( nbVars < 1 )
    return;

  // store
  // (1) variable names in the string of all variables used for theObject and
  // (2) indices of found variables in myLastParamIndex.

  // remember theObject
  SALOMEDS::SObject_wrap aSObj =  ObjectToSObject(theObject);
  if ( aSObj->_is_nil() )
    return;
  CORBA::String_var anObjEntry = aSObj->GetID();
  myLastObj = anObjEntry.in();

  // get a string of variable names
  SALOMEDS::StudyBuilder_var   aStudyBuilder = getStudyServant()->NewBuilder();
  SALOMEDS::GenericAttribute_wrap     anAttr =
    aStudyBuilder->FindOrCreateAttribute( aSObj, "AttributeString" );
  SALOMEDS::AttributeString_wrap aStringAttr = anAttr;
  CORBA::String_var                  oldVars = aStringAttr->Value();
  std::string                         varStr = oldVars.in();

  // add new variables and find indices of variables
  for ( size_t i = 0; i < myLastParameters.size(); ++i )
  {
    int varIndex = -1;
    if ( !myLastParameters[i].empty() )
    {
      // find index of myLastParameters[i] in varStr
      int curIndex  = 0;
      bool varFound = false;
      size_t pos    = 0;
      // varStr can be "A|B::C;*=2|D"
      const std::string separators(":|;*=");
      while ( pos < varStr.size() && !varFound )
      {
        // skip separators
        while ( separators.find( varStr[ pos ]) != std::string::npos )
          if ( ++pos >= varStr.size() )
            break;
        // skip repetition number following '='
        if ( varStr[ pos-1 ] == '=' )
        {
          while ( '0' <= varStr[ pos ] && varStr[ pos ] <= '9' )
            ++pos;
          continue; // to skip next separator
        }
        // compare variable name
        if ( pos < varStr.size() )
        {
          varFound = ( varStr.compare( pos, myLastParameters[i].size(), myLastParameters[i] ) == 0 &&
                       // same string beginning but is length same?
                       ( pos + myLastParameters[i].size() >= varStr.size() ||
                         separators.find( varStr[ pos+1 ]) != std::string::npos ));
          if ( varFound )
            varIndex = curIndex;
          else
            pos = varStr.find_first_of( separators, pos ); // goto the next separator
          ++curIndex;
        }
      }
      // add new variable
      if ( !varFound )
      {
        varStr += ":" + myLastParameters[i];
        varIndex = curIndex;
      }
    }
    myLastParamIndex.push_back( varIndex );
  }
  aStringAttr->SetValue( varStr.c_str() );
}

//================================================================================
/*!
 * \brief Return all variables used to create an object
 *  \param [in] theObjectEntry - an object entry in the current study
 *  \return std::vector< std::string > - all variable names (or values of removed variables)
 */
//================================================================================

std::vector< std::string > SMESH_Gen_i::GetAllParameters(const std::string& theObjectEntry) const
{
  std::vector< std::string > varNames;

  SALOMEDS::SObject_wrap aSObj = getStudyServant()->FindObjectID( theObjectEntry.c_str() );

  // get a string of variable names
  SALOMEDS::StudyBuilder_var   aStudyBuilder = getStudyServant()->NewBuilder();
  SALOMEDS::GenericAttribute_wrap     anAttr =
    aStudyBuilder->FindOrCreateAttribute( aSObj, "AttributeString" );
  SALOMEDS::AttributeString_wrap aStringAttr = anAttr;
  CORBA::String_var                  oldVars = aStringAttr->Value();
  std::string                         varStr = oldVars.in();

  // separate variables within varStr;
  // varStr can be "A|B::C;*=2|D"
  size_t pos = 0;
  const std::string separators(":|;*=");
  while ( pos < varStr.size() )
  {
    // skip separators
    pos = varStr.find_first_not_of( separators, pos );
    // while ( separators.find( varStr[ pos ]) != std::string::npos )
    //   if ( ++pos >= varStr.size() )
    //     break;
    // skip repetition number following '='
    if ( varStr[ pos-1 ] == '=' )
    {
      while ( '0' <= varStr[ pos ] && varStr[ pos ] <= '9' )
        ++pos;
      continue; // to skip next separator
    }
    // store variable name
    if ( pos < varStr.size() )
    {
      size_t pos2 = varStr.find_first_of( separators, pos );
      varNames.push_back( varStr.substr( pos, pos2 - pos));
      pos = pos2;
    }
  }
  return varNames;
}

//=======================================================================
//function : ParseParameters
//purpose  : Replace variables by their values
//=======================================================================
// char* SMESH_Gen_i::ParseParameters(const char* theParameters)
// {
//   //const char* aParameters = theParameters;
// //   const char* aParameters = CORBA::string_dup(theParameters);
//   TCollection_AsciiString anInputParams;
//   SALOMEDS::Study_var aStudy = getStudyServant();
//   if( !aStudy->_is_nil() ) {
// //     SALOMEDS::ListOfListOfStrings_var aSections = aStudy->ParseVariables(theParameters);
// //     for(int j=0;j<aSections->length();j++) {
// //       SALOMEDS::ListOfStrings aVars= aSections[j];
// //       for(int i=0;i<aVars.length();i++ ) {
// //         anInputParams += aStudy->IsVariable(aVars[i].in()) ? 
// //           TCollection_AsciiString(aVars[i].in()) : TCollection_AsciiString("");
// //         if(i != aVars.length()-1)
// //           anInputParams+=":";
// //       }
// //       if(j!=aSections->length()-1)
// //         anInputParams+="|";
// //     }
//     TCollection_AsciiString paramStr( theParameters );
//     int beg = 0, end;
//     char sep, *pParams = (char*)paramStr.ToCString();
//     while ( beg < paramStr.Length() )
//     {
//       end = beg-1;
//       while ( ++end < paramStr.Length() )
//         if ( pParams[end] == ':' || pParams[end] == '|')
//           break;
//       if ( end < paramStr.Length())
//       {
//         sep = pParams[end];
//         pParams[end] = '\0';
//       }
//       if ( aStudy->IsVariable( pParams+beg ))
//         anInputParams += pParams+beg;
//       if ( end < paramStr.Length() )
//         anInputParams += sep;
//       else
//         break;
//       beg = end + 1;
//     }
//   }
//   return CORBA::string_dup(anInputParams.ToCString());
// }

//=======================================================================
//function : GetParameters
//purpose  : 
//=======================================================================

char* SMESH_Gen_i::GetParameters(CORBA::Object_ptr theObject)
{
  CORBA::String_var aResult("");

  SALOMEDS::SObject_wrap aSObj = ObjectToSObject( theObject );
  if ( !aSObj->_is_nil() )
  {
    SALOMEDS::GenericAttribute_wrap attr;
    if ( aSObj->FindAttribute( attr.inout(), "AttributeString"))
    {
      SALOMEDS::AttributeString_wrap strAttr = attr;
      aResult = strAttr->Value();
    }
  }
  return aResult._retn();
}

//================================================================================
/*!
 * \brief Create a sub-mesh on a geometry that is not a sub-shape of the main shape
 * for the case where a valid sub-shape not found by CopyMeshWithGeom().
 * The invalid sub-mesh has GetId() < 0.
 */
//================================================================================

SMESH::SMESH_subMesh_ptr
SMESH_Gen_i::createInvalidSubMesh(SMESH::SMESH_Mesh_ptr theMesh,
                                  GEOM::GEOM_Object_ptr theStrangerGeom,
                                  const char*           theName)
{
  SMESH::SMESH_subMesh_var subMesh;

  try
  {
    SMESH_Mesh_i* mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
    subMesh = mesh_i->createSubMesh( theStrangerGeom );

    if ( !subMesh->_is_nil() && CanPublishInStudy( subMesh ))
    {
      SALOMEDS::SObject_wrap so = PublishSubMesh( theMesh, subMesh, theStrangerGeom, theName );

      // hide a reference to geometry
      if ( !so->_is_nil() )
      {
        SALOMEDS::SObject_wrap refSO;
        if ( so->FindSubObject( GetRefOnShapeTag(), refSO.inout() ))
        {
          SALOMEDS::StudyBuilder_var studyBuilder = getStudyServant()->NewBuilder();
          SALOMEDS::GenericAttribute_wrap attr =
            studyBuilder->FindOrCreateAttribute( refSO, "AttributeDrawable" );
          SALOMEDS::AttributeDrawable_wrap ga = attr;
          ga->SetDrawable( false );
        }
      }
    }
  }
  catch (...) {
  }

  return subMesh._retn();
}


// ==============
// Study context
// ==============

//=======================================================================
//function : addObject
//purpose  : register object in the internal map and return its id
//=======================================================================

int StudyContext::addObject( const std::string& theIOR )
{
  int nextId = getNextId();
  mapIdToIOR.Bind( nextId, theIOR );
  return nextId;
}

//=======================================================================
//function : findId
//purpose  : find the object id in the internal map by the IOR
//=======================================================================

int StudyContext::findId( const std::string& theIOR )
{
  TInt2StringMap::iterator imap;
  for ( imap = mapIdToIOR.begin(); imap != mapIdToIOR.end(); ++imap ) {
    if ( *imap == theIOR )
      return imap.Iterator().Key();
  }
  return 0;
}

//=======================================================================
//function : getIORbyId
//purpose  : get object's IOR by id
//=======================================================================

std::string StudyContext::getIORbyId( const int theId )
{
  if ( mapIdToIOR.IsBound( theId ) )
    return mapIdToIOR( theId );
  return std::string();
}

//=======================================================================
//function : getIORbyOldId
//purpose  : get object's IOR by old id
//=======================================================================

std::string StudyContext::getIORbyOldId( const int theOldId )
{
  if ( mapIdToId.IsBound( theOldId ) )
    return getIORbyId( mapIdToId( theOldId ));
  return std::string();
}

//=======================================================================
//function : mapOldToNew
//purpose  : maps old object id to the new one (used when restoring data)
//=======================================================================

void StudyContext::mapOldToNew( const int oldId, const int newId )
{
  mapIdToId.Bind( oldId, newId );
}

//=======================================================================
//function : getOldId
//purpose  : get old id by a new one
//=======================================================================

int StudyContext::getOldId( const int newId )
{
  TInt2IntMap::iterator imap;
  for ( imap = mapIdToId.begin(); imap != mapIdToId.end(); ++imap ) {
    if ( *imap == newId )
      return imap.Iterator().Key();
  }
  return 0;
}

//=======================================================================
//function : Clear
//purpose  : clear data
//=======================================================================

void StudyContext::Clear()
{
  mapIdToIOR.Clear();
  mapIdToId.Clear();
}
