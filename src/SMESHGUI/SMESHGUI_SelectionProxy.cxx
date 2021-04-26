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

#include "SMESHGUI_SelectionProxy.h"

#include "SMDS_Mesh.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"
#include "SMESH_ControlsDef.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)
#include CORBA_SERVER_HEADER(SMESH_Group)

#include <SALOMEDSClient_Study.hxx>
#include <SUIT_ResourceMgr.h>

////////////////////////////////////////////////////////////////////////////////
/// \class SMESH::SelectionProxy
/// \brief Provide operations over the selected object.
///
/// The selection proxy class is aimed to use in dialogs to access mesh object
/// data either from actor or directly from CORBA reference, depending on what
/// of them is accessible.
/// This is useful in situations when some information is needed, but an actor
/// was not created yet.
/// 
/// Selection proxy can be constructed in two ways:
/// - From interactive object: this performs full proxy initialization including
///   pick-up of an actor from the current viewer if it exists.
/// - From mesh source object (CORBA reference); for performance reasons in this
///   case full initialization is not immediately done and performed only when
///   needed.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Default constructor. Creates null proxy.
*/
SMESH::SelectionProxy::SelectionProxy(): myActor(0), myDirty(false)
{
}

/*!
  \brief Constructor.
  \param io Interactive object.
*/
SMESH::SelectionProxy::SelectionProxy( const Handle(SALOME_InteractiveObject)& io ): myActor(0), myDirty(true)
{
  myObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( io );
  init();
}

/*!
  \brief Constructor.
  \param object Mesh source.
*/
SMESH::SelectionProxy::SelectionProxy( SMESH::SMESH_IDSource_ptr object ): myActor(0), myDirty(true)
{
  if ( !CORBA::is_nil( object ) )
    myObject = SMESH::SMESH_IDSource::_duplicate( object );
}

/*!
  \brief Copy constructor.
  \param other Proxy being copied.
*/
SMESH::SelectionProxy::SelectionProxy( const SelectionProxy& other )
{
  myIO = other.myIO;
  myObject = other.myObject;
  myActor = other.myActor;
  myDirty = other.myDirty;
}

/*!
  \brief Perform internal initialization.
  \internal
*/
void SMESH::SelectionProxy::init()
{
  if ( myIO.IsNull() )
    myIO = new SALOME_InteractiveObject(); // create dummy IO to avoid crashes when accessing it

  if ( !CORBA::is_nil( myObject ) )
  {
    if ( !myIO->hasEntry() )
    {
      _PTR(SObject) sobj = SMESH::ObjectToSObject( myObject.in() );
      if ( sobj )
        myIO = new SALOME_InteractiveObject( sobj->GetID().c_str(), "SMESH", sobj->GetName().c_str() );
    }
    if ( !myActor && myIO->hasEntry() )
      myActor = SMESH::FindActorByEntry( myIO->getEntry() );
  }
  myDirty = false;
}

/*!
  \brief Assignment operator.
  \param other Proxy being copied.
*/
SMESH::SelectionProxy& SMESH::SelectionProxy::operator= ( const SMESH::SelectionProxy& other )
{
  myIO = other.myIO;
  myObject = other.myObject;
  myActor = other.myActor;
  myDirty = other.myDirty;
  return *this;
}

/*!
  \brief Equality comparison operator.
  \param other Proxy to compare with.
  \return \c true if two proxies are equal; \c false otherwise.
*/
bool SMESH::SelectionProxy::operator== ( const SMESH::SelectionProxy& other )
{
  return !CORBA::is_nil( myObject ) && !CORBA::is_nil( other.myObject ) && 
    myObject->_is_equivalent( other.myObject );
}

/*!
  \brief Re-initialize proxy.
*/
void SMESH::SelectionProxy::refresh()
{
  init();
}

/*!
  \brief Check if proxy is null.
  \return \c true if proxy is null; \c false otherwise.
*/
bool SMESH::SelectionProxy::isNull() const
{
  return CORBA::is_nil( myObject );
}

/*!
  \brief Boolean conversion operator.
  \return \c true if proxy is not null; \c false otherwise.
*/
SMESH::SelectionProxy::operator bool() const
{
  return !isNull();
}

/*!
  \brief Get interactive object.
  \return Interactive object referenced by proxy.
*/
const Handle(SALOME_InteractiveObject)& SMESH::SelectionProxy::io() const
{
  if ( myDirty )
    const_cast<SMESH::SelectionProxy*>(this)->init();
  return myIO;
}

/*!
  \brief Get mesh object.
  \return Mesh object (mesh, sub-mesh, group, etc.) referenced by proxy.
*/
SMESH::SMESH_IDSource_ptr SMESH::SelectionProxy::object() const
{
  return SMESH::SMESH_IDSource::_duplicate( myObject );
}

/*!
  \brief Get actor.
  \return Actor referenced by proxy.
*/
SMESH_Actor* SMESH::SelectionProxy::actor() const
{
  if ( myDirty )
    const_cast<SMESH::SelectionProxy*>(this)->init();
  return myActor;
}

/*!
  \brief Get object's validity.

  Mesh object is valid if it is not null and information stored in it is valid.

  \return \c true if object is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::isValid() const
{
  return !isNull() && myObject->IsMeshInfoCorrect();
}

/*!
  \brief Load mesh object from study file.
*/
void SMESH::SelectionProxy::load()
{
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
      mesh->Load();
  }
}

/*!
  \brief Get name.
  \return Mesh object's name.
*/
QString SMESH::SelectionProxy::name() const
{
  QString value;
  if ( !isNull() )
    value = io()->getName();
  return value;
}

/*!
  \brief Get type.
  \return Mesh object's type.
*/
SMESH::SelectionProxy::Type SMESH::SelectionProxy::type() const
{
  Type value = Unknown;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow( myObject );
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    SMESH::SMESH_Group_var sGroup = SMESH::SMESH_Group::_narrow( myObject );
    SMESH::SMESH_GroupOnGeom_var gGroup = SMESH::SMESH_GroupOnGeom::_narrow( myObject );
    SMESH::SMESH_GroupOnFilter_var fGroup = SMESH::SMESH_GroupOnFilter::_narrow( myObject );
    
    if ( !CORBA::is_nil( mesh ) )
      value = Mesh;
    else if ( !CORBA::is_nil( submesh ) )
      value = Submesh;
    else if ( !CORBA::is_nil( sGroup ) )
      value = GroupStd;
    else if ( !CORBA::is_nil( gGroup ) )
      value = GroupGeom;
    else if ( !CORBA::is_nil( fGroup ) )
      value = GroupFilter;
    else if ( !CORBA::is_nil( group ) )
      value = Group;
  }
  return value;
}

/*!
  \brief Get mesh information.
  \return Statistics on stored mesh object.
*/
SMESH::MeshInfo SMESH::SelectionProxy::meshInfo() const
{
  SMESH::MeshInfo info;
  if ( !isNull() )
  {
    SMESH::smIdType_array_var data = myObject->GetMeshInfo();
    for ( uint type = SMESH::Entity_Node; type < SMESH::Entity_Last; type++ )
    {
      if ( type < data->length() )
        info.addInfo( type, data[ type ] );
    }
  }
  return info;
}

/*!
  \brief Get parent mesh.
  \return Proxy object that stores parent mesh object.
  \note For proxy that stores a mesh, returns its copy.
*/
SMESH::SelectionProxy SMESH::SelectionProxy::mesh() const
{
  SMESH::SelectionProxy parent;
  if ( !isNull() )
    parent = SMESH::SelectionProxy( (SMESH::SMESH_Mesh_var) myObject->GetMesh() ); // cast to var to avoid leaks
  return parent;
}

/*!
  \brief Check if parent mesh has shape to mesh.
  \return \c true if Parent mesh is built on a shape; \c false otherwise.
  \note For group or submesh, this method checks that parent mesh has a shape.
*/
bool SMESH::SelectionProxy::hasShapeToMesh() const
{
  bool result = false;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
      result = mesh->HasShapeToMesh();
  }
  return result;
}

/*!
  \brief Get referenced GEOM object.
  \return GEOM object referenced by selection proxy.

  The result contains valid pointer only if proxy refers to mesh, sub-mesh
  or group created on a geometry.
*/
GEOM::GEOM_Object_ptr SMESH::SelectionProxy::shape() const
{
  GEOM::GEOM_Object_var shape;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow( myObject );
    SMESH::SMESH_GroupOnGeom_var group = SMESH::SMESH_GroupOnGeom::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
      shape = mesh->GetShapeToMesh();
    else if ( !CORBA::is_nil( submesh ) )
      shape = submesh->GetSubShape();
    else if ( !CORBA::is_nil( group ) )
      shape = group->GetShape();
  }
  return shape._retn();
}

/*!
  \brief Get name of referenced GEOM object.
  \return Name of GEOM object referenced by selection proxy.

  The result contains non-empty name only if proxy refers to mesh, sub-mesh
  or group created on a geometry, and if that geometry has valid name.
*/
QString SMESH::SelectionProxy::shapeName() const
{
  QString name;
  if ( !isNull() )
  {
    GEOM::GEOM_Object_var gobj = shape();
    _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
    if ( sobj )
      name = QString::fromStdString( sobj->GetName() );
  }
  return name;
}

/*!
  \brief Get type of referenced GEOM object.
  \return Type of GEOM object referenced by selection proxy.

  The result contains valid type only if proxy refers to mesh, sub-mesh
  or group created on a geometry.
*/
int SMESH::SelectionProxy::shapeType() const
{
  int type = -1;
  if ( !isNull() )
  {
    GEOM::GEOM_Object_var gobj = shape();
    if ( !CORBA::is_nil( gobj ) )
      type = gobj->GetShapeType();
  }
  return type;
}

/*!
  \brief Check if mesh has been loaded from study file.
  \return \c true if mesh was loaded; \c false otherwise.
*/
bool SMESH::SelectionProxy::isMeshLoaded() const
{
  bool result = true; // set default to true to avoid side effects
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
      result = mesh->IsLoaded();
  }
  return result;
}

/*!
  \brief Check if node with given ID is present in the mesh.
  \param id Mesh node ID.
  \return \c true if mesh contains node; \c false otherwise.
*/
bool SMESH::SelectionProxy::hasNode( int id )
{
  bool result = false;
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshNode* node = actor()->GetObject()->GetMesh()->FindNode( id );
      result = node != 0;
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        SMESH::double_array_var coords = mesh->GetNodeXYZ( id );
        result = coords->length() >= 3;
      }
    }
  }
  return result; 
}

/*!
  \brief Get node coordinates.
  \param id Mesh node ID.
  \param xyz Returned node coordinates.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::nodeCoordinates( int id, SMESH::XYZ& xyz )
{
  bool result = false;
  xyz = SMESH::XYZ();
  
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshNode* node = actor()->GetObject()->GetMesh()->FindNode( id );
      if ( node )
      {
        xyz = SMESH::XYZ( node->X(), node->Y(), node->Z() );
        result = true;
      }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        SMESH::double_array_var coords = mesh->GetNodeXYZ( id );
        if ( coords->length() >= 3 )
        {
          xyz = SMESH::XYZ( coords[0], coords[1], coords[2] );
          result = true;
        }
      }
    }
  }
  return result;
}

/*!
  \brief Get node connectivity.
  \param id Mesh node ID.
  \param connectivity Returned node connectivity.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::nodeConnectivity( int id, SMESH::Connectivity& connectivity )
{
  bool result = false;
  connectivity.clear();
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshNode* node = actor()->GetObject()->GetMesh()->FindNode( id );
      if ( node )
      {
        SMDS_ElemIteratorPtr it = node->GetInverseElementIterator();
        while ( it && it->more() )
        {
          const SMDS_MeshElement* ne = it->next();
          connectivity[ ne->GetType() ] << ne->GetID();
        }
        result = true;
      }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        SMESH::smIdType_array_var elements = mesh->GetNodeInverseElements( id, SMESH::ALL );
        for ( int i = 0, n = elements->length(); i < n; i++ )
        {
          SMESH::ElementType type = mesh->GetElementType( elements[i], true );
          connectivity[ type ] << elements[i];
        }
        result = true;
      }
    }
  }
  return result;
}

/*!
  \brief Get node position on a shape.
  \param id Mesh node ID.
  \param position Returned node position.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::nodePosition( int id, Position& position )
{
  bool result = false;
  position = Position();
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::NodePosition_var pos = mesh->GetNodePosition( id );
      position.setShapeId( pos->shapeID );
      if ( pos->shapeID > 0 )
      {
        position.setShapeType( pos->shapeType );
        if ( pos->shapeType == GEOM::EDGE && pos->params.length() > 0 )
        {
          position.setU( pos->params[0] );
        }
        if ( pos->shapeType == GEOM::FACE && pos->params.length() > 1 )
        {
          position.setU( pos->params[0] );
          position.setV( pos->params[1] );
        }
        result = true;
      }
    }
  }
  return result;
}

/*!
  \brief Get groups given node belongs to.
  \param id Mesh node ID.
  \return List of groups containing given node.
*/
QList<SMESH::SelectionProxy> SMESH::SelectionProxy::nodeGroups( int id ) const
{
  QList<SMESH::SelectionProxy> result;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::ListOfGroups_var groups = mesh->GetGroups();
      for ( uint i = 0 ; i < groups->length(); i++ )
      {
        if ( groups[i]->GetType() == SMESH::NODE && groups[i]->Contains( id ) )
        {
          result << SMESH::SelectionProxy( groups[i].in() );
        }
      }
    }
  }
  return result;
}

/*!
  \brief Check if element with given ID is present in the mesh.
  \param id Mesh element ID.
  \return \c true if mesh contains element; \c false otherwise.
*/
bool SMESH::SelectionProxy::hasElement( int id )
{
  bool result = false;
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      result = element != 0;
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        SMESH::smIdType_array_var nodes = mesh->GetElemNodes( id );
        result = nodes->length() > 0;
      }
    }
  }
  return result; 
}

/*!
  \brief Get type of given mesh element.
  \param id Mesh element ID.
  \return Element type.
*/
SMESH::ElementType SMESH::SelectionProxy::elementType( int id ) const
{
  SMESH::ElementType value = SMESH::ALL;
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      if ( element )
      {
        value = (SMESH::ElementType)element->GetType();
      }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        value = mesh->GetElementType( id, true );
      }
    }
  }
  return value;
}

/*!
  \brief Get entity type of given mesh element.
  \param id Mesh element ID.
  \return Entity type.
*/
int SMESH::SelectionProxy::elementEntityType( int id ) const
{
  SMESH::EntityType value = SMESH::Entity_Last;
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      if ( element )
      {
        value = (SMESH::EntityType)element->GetEntityType();
      }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        value = mesh->GetElementGeomType( id );
      }
    }
  }
  return value;
}

/*!
  \brief Get element connectivity.
  \param id Mesh element ID.
  \param connectivity Return element connectivity.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::elementConnectivity( SMESH::smIdType id, Connectivity& connectivity )
{
  bool result = false;
  connectivity.clear();
  if ( !isNull() )
  {
    QSet<SMESH::smIdType> nodes; // order of nodes is important
    if ( actor() )
    {
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      if ( element )
        if ( SMDS_NodeIteratorPtr it = element->nodeIterator())
        {
          while ( it->more() )
          {
            SMESH::smIdType n = it->next()->GetID();
            if ( !nodes.contains( n ))
            {
              connectivity[ SMDSAbs_Node ] << n;
              nodes << n;
            }
          }
          result = true;
        }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        SMESH::smIdType_array_var nn = mesh->GetElemNodes( id );
        for ( int i = 0, nb = nn->length(); i < nb; i++ )
        {
          if ( !nodes.contains( nn[i] ))
          {
            connectivity[ SMDSAbs_Node ] << nn[i];
            nodes << nn[i];
          }
        }
        result = true;
      }
    }
  }
  return result;
}

/*!
  \brief Get volume element connectivity.
  \param id Mesh element ID.
  \param connectivity Return nodal connectivity of each facet numbered from 1
  \param nbNodes Return number of unique nodes.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::perFaceConnectivity( int id, Connectivity& connectivity, int& nbNodes )
{
  bool result = false;
  connectivity.clear();
  nbNodes = 0;
  if ( !isNull() )
  {
    QSet<int> nodeSet;
    if ( actor() )
    {
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      SMDS_VolumeTool volTool;
      if ( volTool.Set( element ))
      {
        for ( int iF = 0; iF < volTool.NbFaces(); ++iF )
        {
          const SMDS_MeshNode** nn = volTool.GetFaceNodes( iF );
          int nbN = volTool.NbFaceNodes( iF );
          for ( int iN = 0; iN < nbN; ++iN )
          {
            connectivity[ iF+1 ] << ( nn[ iN ]->GetID() );
            nodeSet << ( nn[ iN ]->GetID() );
          }
        }
        result = true;
      }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        CORBA::Long nbFaces = mesh->ElemNbFaces( id );
        for ( CORBA::Long iF = 0; iF < nbFaces; ++iF )
        {
          SMESH::smIdType_array_var nodes = mesh->GetElemFaceNodes( id, iF );
          for ( CORBA::ULong iN = 0; iN < nodes->length(); ++iN )
          {
            connectivity[ iF+1 ] << nodes[iN];
            nodeSet << nodes[iN];
          }
        }
        result = ( nbFaces > 0 );
      }
    }
    nbNodes = nodeSet.size();
  }
  return result;
}

/*!
  \brief Get element position on a shape.
  \param id Mesh element ID.
  \param position Returned element position.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::elementPosition( int id, Position& position )
{
  bool result = false;
  position = Position();
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::ElementPosition_var pos = mesh->GetElementPosition( id );
      position.setShapeId( pos->shapeID );
      if ( pos->shapeID > 0 )
      {
        position.setShapeType( pos->shapeType );
        result = true;
      }
    }
  }
  return result;
}

/*!
  \brief Get gravity center of given element.
  \param id Mesh element ID.
  \param xyz Returned gravity center.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::elementGravityCenter( int id, SMESH::XYZ& xyz )
{
  bool result = false;
  xyz = SMESH::XYZ();
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      if ( element )
      {
        SMDS_ElemIteratorPtr it = element->nodesIterator();
        while ( it->more() )
        {
          const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( it->next() );
          xyz.add( node->X(), node->Y(), node->Z() );
        }
        xyz.divide( element->NbNodes() );
        result = true;
      }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        SMESH::smIdType_array_var nodes = mesh->GetElemNodes( id );
        for ( int i = 0, n = nodes->length(); i < n; i++ )
        {
          SMESH::double_array_var coords = mesh->GetNodeXYZ( nodes[i]  );
          if ( coords->length() >= 3 )
          {
            xyz.add( coords[0], coords[1], coords[2] );
          }
        }
        xyz.divide( nodes->length() );
        result = true;
      }
    }
  }
  return result;
}

/*!
  \brief Get normal vector to given element (face).
  \param id Mesh element ID.
  \param xyz Returned normal vector.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::elementNormal( int id, SMESH::XYZ& xyz )
{
  bool result = false;
  xyz = SMESH::XYZ();
  if ( !isNull() )
  {
    if ( actor() )
    {
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      if ( element && element->GetType() == SMDSAbs_Face )
      {
        xyz = SMESH::getNormale( SMDS_Mesh::DownCast<SMDS_MeshFace>( element ) );
        result = true;
      }
    }
    else
    {
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !CORBA::is_nil( mesh ) )
      {
        SMESH::double_array_var normal = mesh->GetFaceNormal( id, true );
        if ( normal->length() >= 3 )
        {
          xyz = SMESH::XYZ( normal[0], normal[1], normal[2] );
          result = true;
        }
      }
    }
  }
  return result;
}

/*!
  \brief Get quality control's value for given element.
  \param id Mesh element ID.
  \param control Quality control type.
  \param precision Precision for control's value.
  \param value Returned quality control's value.
  \return \c true if result is valid; \c false otherwise.
*/
bool SMESH::SelectionProxy::elementControl( int id, int control, double precision, double& value ) const
{
  bool result = false;
  value = 0.;
  if ( !isNull() )
  {
    if ( actor() )
    {
      SMESH::Controls::NumericalFunctorPtr functor;
      switch ( control )
      {
      case SMESH::FT_AspectRatio:
        functor.reset( new SMESH::Controls::AspectRatio() );
        break;
      case SMESH::FT_AspectRatio3D:
        functor.reset( new SMESH::Controls::AspectRatio3D() );
        break;
      case SMESH::FT_Warping:
        functor.reset( new SMESH::Controls::Warping() );
        break;
      case SMESH::FT_MinimumAngle:
        functor.reset( new SMESH::Controls::MinimumAngle() );
        break;
      case SMESH::FT_Taper:
        functor.reset( new SMESH::Controls::Taper() );
        break;
      case SMESH::FT_Skew:
        functor.reset( new SMESH::Controls::Skew() );
        break;
      case SMESH::FT_Area:
        functor.reset(  new SMESH::Controls::Area() );
        break;
      case SMESH::FT_Volume3D:
        functor.reset(  new SMESH::Controls::Volume() );
        break;
      case SMESH::FT_MaxElementLength2D:
        functor.reset( new SMESH::Controls::MaxElementLength2D() );
        break;
      case SMESH::FT_MaxElementLength3D:
        functor.reset(  new SMESH::Controls::MaxElementLength3D() );
        break;
      case SMESH::FT_Length:
        functor.reset( new SMESH::Controls::Length() );
        break;
      case SMESH::FT_Length2D:
        functor.reset( new SMESH::Controls::Length2D() );
        break;
      case SMESH::FT_Length3D:
        functor.reset( new SMESH::Controls::Length3D() );
        break;
      case SMESH::FT_BallDiameter:
        functor.reset( new SMESH::Controls::BallDiameter() );
        break;
      default:
        break;
      } 
      const SMDS_MeshElement* element = actor()->GetObject()->GetMesh()->FindElement( id );
      if ( element && functor && element->GetType() == functor->GetType() )
      {
        functor->SetMesh( actor()->GetObject()->GetMesh() );
        if ( functor->IsApplicable( element ))
        {
          functor->SetPrecision( precision );
          value = functor->GetValue( id );
          result = true;
        }
      }
    }
    else
    {
      SMESH::SMESH_Gen_var smeshGen = SMESHGUI::GetSMESHGen();
      if ( smeshGen->_is_nil() )
        return false;
      SMESH::FilterManager_var manager = smeshGen->CreateFilterManager();
      if ( manager->_is_nil() )
        return false;
      SMESH::NumericalFunctor_var functor;
      switch ( control )
      {
      case SMESH::FT_AspectRatio:
        functor = manager->CreateAspectRatio();
        break;
      case SMESH::FT_AspectRatio3D:
        functor = manager->CreateAspectRatio3D();
        break;
      case SMESH::FT_Warping:
        functor = manager->CreateWarping();
        break;
      case SMESH::FT_MinimumAngle:
        functor = manager->CreateMinimumAngle();
        break;
      case SMESH::FT_Taper:
        functor = manager->CreateTaper();
        break;
      case SMESH::FT_Skew:
        functor = manager->CreateSkew();
        break;
      case SMESH::FT_Area:
        functor = manager->CreateArea();
        break;
      case SMESH::FT_Volume3D:
        functor = manager->CreateVolume3D();
        break;
      case SMESH::FT_MaxElementLength2D:
        functor = manager->CreateMaxElementLength2D();
        break;
      case SMESH::FT_MaxElementLength3D:
        functor = manager->CreateMaxElementLength3D();
        break;
      case SMESH::FT_Length:
        functor = manager->CreateLength();
        break;
      case SMESH::FT_Length2D:
        functor = manager->CreateLength2D();
        break;
      case SMESH::FT_Length3D:
        functor = manager->CreateLength3D();
        break;
      case SMESH::FT_BallDiameter:
        functor = manager->CreateBallDiameter();
        break;
      default:
        break;
      }
      SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
      if ( !functor->_is_nil() && !mesh->_is_nil() )
      {
        functor->SetMesh( mesh );
        if ( functor->IsApplicable( id ))
        {
          functor->SetPrecision( precision );
          value = functor->GetValue( id );
          result = true;
        }
      }
      manager->UnRegister();
    }
  }
  return result;
}

/*!
  \brief Get groups given element belongs to.
  \param id Mesh element ID.
  \return List of groups containing element.
*/
QList<SMESH::SelectionProxy> SMESH::SelectionProxy::elementGroups( int id ) const
{
  QList<SMESH::SelectionProxy> result;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::ListOfGroups_var groups = mesh->GetGroups();
      for ( uint i = 0 ; i < groups->length(); i++ )
      {
        if ( groups[i]->GetType() != SMESH::NODE && groups[i]->Contains( id ) )
        {
          result << SMESH::SelectionProxy( groups[i].in() );
        }
      }
    }
  }
  return result;
}

/*!
  \brief Get MED file information.
  \return MED file information.

  The result contains valid data only if proxy refers to mesh object
  which has been imported from the MED file.
*/
SMESH::MedInfo SMESH::SelectionProxy::medFileInfo() const
{
  SMESH::MedInfo info;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::MedFileInfo_var inf = mesh->GetMEDFileInfo();
      info.setFileName( inf->fileName.in() );
      info.setSize( inf->fileSize );
      info.setVersion( inf->major, inf->minor, inf->release );
    }
  }
  return info;
}

/*!
  \brief Get child sub-meshes.
  \return List of sub-meshes for mesh object; empty list for other types.
*/
QList<SMESH::SelectionProxy> SMESH::SelectionProxy::submeshes() const
{
  QList<SMESH::SelectionProxy> lst;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::submesh_array_var array = mesh->GetSubMeshes();
      for ( uint i = 0 ; i < array->length(); i++ )
        lst << SMESH::SelectionProxy( array[i].in() );
    }
  }
  return lst;
}

/*!
  \brief Get child groups.
  \return List of groups for mesh object; empty list for other types.
*/
QList<SMESH::SelectionProxy> SMESH::SelectionProxy::groups() const
{
  QList<SMESH::SelectionProxy> lst;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::ListOfGroups_var array = mesh->GetGroups();
      for ( uint i = 0 ; i < array->length(); i++ )
        lst << SMESH::SelectionProxy( array[i].in() );
    }
  }
  return lst;
}

/*!
  \brief Get element type (for group). For other mesh objects result is undefined.
  \return Group's element type.
*/
SMESH::ElementType SMESH::SelectionProxy::groupElementType() const
{
  SMESH::ElementType value = SMESH::ALL;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
      value = group->GetType();
  }
  return value;
}

/*!
  \brief Get color (for group). For other mesh objects result is undefined.
  \return Group's color.
*/
QColor SMESH::SelectionProxy::color() const
{
  QColor result;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
    {
      SALOMEDS::Color c = group->GetColor();
      result = QColor::fromRgbF( c.R, c.G, c.B );
    }
  }
  return result;
}

/*!
  \brief Get size (for group). For other mesh objects result is undefined.
  \param autoCompute Compute size if it is unavailable. Defaults to \c false.
  \return Group's size.
*/
SMESH::smIdType SMESH::SelectionProxy::size( bool autoCompute ) const
{
  // note: size is not computed for group on filter for performance reasons, see IPAL52831
  SMESH::smIdType result = -1;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
    {
      if ( type() == GroupFilter )
        // for group on filter we check if value is already computed and cached
        autoCompute |= group->IsMeshInfoCorrect();
      else
        // for other groups we force autoCompute to true
        autoCompute = true;
      if ( autoCompute )
        result = group->Size();
    }
  }
  return result;
}

/*!
  \brief Get number of underlying nodes (for group of elements). For other
  mesh objects result is undefined.
  \param autoCompute Compute size if it is unavailable. Defaults to \c false.
  \return Number of nodes contained in group.
*/
SMESH::smIdType SMESH::SelectionProxy::nbNodes( bool autoCompute ) const
{
  // note: nb of nodes is not computed automatically for performance reasons
  SMESH::smIdType result = -1;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) && ( autoCompute || isMeshLoaded() ) )
    {
      int groupSize = size( autoCompute );
      if ( groupSize >= 0 )
      {
        int limit = SMESHGUI::resourceMgr()->integerValue( "SMESH", "info_groups_nodes_limit", 100000 );
        if ( group->IsNodeInfoAvailable() || limit <= 0 || groupSize <= limit )
          result = group->GetNumberOfNodes();
      }
    }
  }
  return result;
}

/*!
  \brief Get list of nodes / elements IDs for the mesh group.
  \return List of IDs for group object; empty list for other types.
*/
QSet<uint> SMESH::SelectionProxy::ids() const
{
  QSet<uint> result;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
    {
      SMESH::smIdType_array_var seq = group->GetListOfID();
      for ( int i = 0, n = seq->length(); i < n; i++ )
        result << (uint)seq[i];
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESH::MeshInfo
/// \brief Store statistics on mesh object.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
*/
SMESH::MeshInfo::MeshInfo()
{
}

/*!
  \brief Add information on given entity type.
  \param type Entity type.
  \param value Number of entities.
*/
void SMESH::MeshInfo::addInfo( int type, long value )
{
  myInfo[type] = value;
}

/*!
  \brief Get number of entities of given type.
  \param type Entity type.
  \return Number of entities.
*/
uint SMESH::MeshInfo::info( int type ) const
{
  return myInfo.value( type, 0U );
}

/*!
  \brief Access operator.
  \param type Entity type.
  \return Number of entities.
*/
uint SMESH::MeshInfo::operator[] ( int type )
{
  return (uint)info( type );
}

/*!
  \brief Get total number of entities for types in given range.
  \param fromType Lower entity type.
  \param toType Upper entity type.
  \return Number of entities.
*/
uint SMESH::MeshInfo::count( int fromType, int toType ) const
{
  uint value = 0;
  for ( int type = fromType; type <= toType; type++ )
    value += info( type );
  return value;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESH::MedInfo
/// \brief Provide operations over the selected object
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
*/
SMESH::MedInfo::MedInfo()
{
}

/*!
  \brief Get validity status.

  MED file information is valid if at least stored file name is not null.

  \return \c true if MED file info is valid; \c false otherwise.
*/
bool SMESH::MedInfo::isValid() const
{
  return !myFileName.isEmpty();
}

/*!
  \brief Get file name for mesh imported from MED file.
  \return MED file name.
*/
QString SMESH::MedInfo::fileName() const
{
  return myFileName;
}

/*!
  \brief Set file name for mesh imported from MED file.
  \param fileName MED file name.
*/
void SMESH::MedInfo::setFileName( const QString& fileName )
{
  myFileName = fileName;
}

/*!
  \brief Get file size for mesh imported from MED file.
  \return MED file size.
*/
uint SMESH::MedInfo::size() const
{
  return mySize;
}

/*!
  \brief Set file size for mesh imported from MED file.
  \param size MED file size.
*/
void SMESH::MedInfo::setSize( uint size )
{
  mySize = size;
}

/*!
  \brief Get version of format for mesh imported from MED file.
  \return MED file format version.
*/
QString SMESH::MedInfo::version() const
{
  QString v = QString( "%1" ).arg( myMajor );
  if ( myMinor > 0 || myRelease > 0 ) v += QString( ".%1" ).arg( myMinor );
  else if ( myMajor > 0 ) v += ".0";
  if ( myRelease > 0 ) v += QString( ".%1" ).arg( myRelease );
  return v;
}

/*!
  \brief Set version of format for mesh imported from MED file.
  \param major Major version number.
  \param minor Minor version number.
  \param release Release version number.
*/
void SMESH::MedInfo::setVersion( uint major, uint minor, uint release )
{
  myMajor = major;
  myMinor = minor;
  myRelease = release;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESH::Position
/// \brief Describes position of mesh node or element on a reference shape.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor. Creates invalid position.
*/
SMESH::Position::Position():
  myShapeId(-1), myShapeType(-1), myU(0), myV(0), myHasU(false), myHasV(false)
{
}

/*!
  \brief Check if position is valid.
  \return \c true if position is valid; \c false otherwise.
*/
bool SMESH::Position::isValid() const
{
  return myShapeId > 0 && myShapeType != -1;
}

/*!
  \brief Get reference shape ID.
  \return Shape / sub-shape ID.
*/
int SMESH::Position::shapeId() const
{
  return myShapeId;
}

/*!
  \brief Set reference shape ID.
  \param id Shape / sub-shape ID.
*/
void SMESH::Position::setShapeId( int id )
{
  myShapeId = id;
}

/*!
  \brief Get reference shape type.
  \return Shape type.
*/
int SMESH::Position::shapeType() const
{
  return myShapeType;
}

/*!
  \brief Set reference shape type.
  \param type Shape type.
*/
void SMESH::Position::setShapeType( int type )
{
  myShapeType = type;
}

/*!
  \brief Check if there is U position on a shape.
  \return \c true if U position is valid; \c false otherwise.
*/
bool SMESH::Position::hasU() const
{
  return myHasU;
}

/*!
  \brief Get U position on a shape.
  \return U position.
*/
double SMESH::Position::u() const
{
  return myU;
}

/*!
  \brief Set U position on a shape.
  \parm u U position.
*/
void SMESH::Position::setU( double u )
{
  myU = u;
  myHasU = true;
}

/*!
  \brief Check if there is V position on a shape.
  \return \c true if V position is valid; \c false otherwise.
*/
bool SMESH::Position::hasV() const
{
  return myHasV;
}

/*!
  \brief Get V position on a shape.
  \return V position.
*/
double SMESH::Position::v() const
{
  return myV;
}

/*!
  \brief Set V position on a shape.
  \parm v V position.
*/
void SMESH::Position::setV( double v)
{
  myV = v;
  myHasV = true;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESH::XYZ
/// \brief Simple structure to manage 3D coordinate.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Default constructor.
*/
SMESH::XYZ::XYZ()
{
  myX = myY = myZ = 0.0;
}

/*!
  \brief Base constructor.
  \param parameter x X coordinate.
  \param parameter y Y coordinate.
  \param parameter z Z coordinate.
*/
SMESH::XYZ::XYZ( double x, double y, double z )
{
  myX = x;
  myY = y;
  myZ = z;
}

/*!
  \brief Construction from geometrical point.
  \param parameter p Geometrical point.
*/
SMESH::XYZ::XYZ( const gp_XYZ& p )
{
  myX = p.X();
  myY = p.Y();
  myZ = p.Z();
}

/*!
  \brief Add a point.
  \param parameter x X coordinate.
  \param parameter y Y coordinate.
  \param parameter z Z coordinate.
*/
void SMESH::XYZ::add( double x, double y, double z )
{
  myX += x;
  myY += y;
  myZ += z;
}

/*!
  \brief Divide point to given coefficient.
  \param parameter a Divider coefficient.
*/
void SMESH::XYZ::divide( double a )
{
  if ( a != 0.)
  {
    myX /= a;
    myY /= a;
    myZ /= a;
  }
}

/*!
  \brief Get X coordinate.
  \return X coordinate.
*/
double SMESH::XYZ::x() const
{
  return myX;
}

/*!
  \brief Get Y coordinate.
  \return Y coordinate.
*/
double SMESH::XYZ::y() const
{
  return myY;
}

/*!
  \brief Get Z coordinate.
  \return Z coordinate.
*/
double SMESH::XYZ::z() const
{
  return myZ;
}

/*!
  \brief Conversion to geometrical point.
  \return Geometrical point.
*/
SMESH::XYZ::operator gp_XYZ() const
{
  return gp_XYZ( myX, myY, myZ );
}
