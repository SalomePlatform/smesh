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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : StdMeshers_ImportSource1D_i.cxx
//  Module : SMESH
//
#include "StdMeshers_ImportSource1D_i.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "StdMeshers_ObjRefUlils.hxx"

#include <Utils_CorbaException.hxx>
#include <utilities.h>
#include <SALOMEDS_wrap.hxx>

#include <TCollection_AsciiString.hxx>

#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_ImportSource1D_i::StdMeshers_ImportSource1D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_ImportSource1D_i::StdMeshers_ImportSource1D_i( PortableServer::POA_ptr thePOA,
                                                          int                     theStudyId,
                                                          ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_ImportSource1D_i::StdMeshers_ImportSource1D_i" );
  myBaseImpl = new ::StdMeshers_ImportSource1D( theGenImpl->GetANewId(),
                                                theStudyId,
                                                theGenImpl );
  _groupEntries = new SMESH::string_array();
}

//=============================================================================
/*!
 *  StdMeshers_ImportSource1D_i::~StdMeshers_ImportSource1D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_ImportSource1D_i::~StdMeshers_ImportSource1D_i()
{
  MESSAGE( "StdMeshers_ImportSource1D_i::~StdMeshers_ImportSource1D_i" );
}

//=============================================================================
/*!
 *  SetSourceEdges
 */
//=============================================================================

void StdMeshers_ImportSource1D_i::SetSourceEdges(const SMESH::ListOfGroups& groups)
{
  MESSAGE( "StdMeshers_ImportSource1D_i::SetSourceEdges" );
  ASSERT( myBaseImpl );
  try
  {
    std::vector<SMESH_Group*> smesh_groups;
    std::vector<string> entries;
    SALOMEDS::Study_var study = SMESH_Gen_i::GetSMESHGen()->GetCurrentStudy();
    for ( CORBA::ULong i = 0; i < groups.length(); ++i )
      if ( SMESH_GroupBase_i* gp_i = SMESH::DownCast<SMESH_GroupBase_i*>( groups[i] ))
      {
        if ( gp_i->GetType() != SMESH::EDGE )
          THROW_SALOME_CORBA_EXCEPTION("Wrong group type", SALOME::BAD_PARAM);
        smesh_groups.push_back( gp_i->GetSmeshGroup() );

        SALOMEDS::SObject_wrap so = SMESH_Gen_i::ObjectToSObject(study, groups[i]);
        if ( !so->_is_nil())
        {
          CORBA::String_var entry = so->GetID();
          entries.push_back( entry.in() );
        }
      }
    this->GetImpl()->SetGroups( smesh_groups );

    _groupEntries = new SMESH::string_array;
    _groupEntries->length( entries.size ());
    for ( size_t i = 0; i < entries.size(); ++i )
      _groupEntries[i] = entries[i].c_str();
  }
  catch ( SALOME_Exception& S_ex )
  {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetSourceEdges( " << groups << " )";
}

//=============================================================================
/*!
 * Return entries of groups
 */
//=============================================================================

SMESH::string_array*  StdMeshers_ImportSource1D_i::GetSourceEdges()
{
  MESSAGE( "StdMeshers_ImportSource1D_i::GetImportSource" );
  SMESH::string_array_var res = new SMESH::string_array( _groupEntries );
  return res._retn();
}

//================================================================================
/*!
 * \brief Set to copy mesh and groups
 */
//================================================================================

void StdMeshers_ImportSource1D_i::SetCopySourceMesh(CORBA::Boolean toCopyMesh,
                                                    CORBA::Boolean toCopyGroups)
{
  GetImpl()->SetCopySourceMesh(toCopyMesh,toCopyGroups);
  SMESH::TPythonDump() << _this() << ".SetCopySourceMesh( "
                       << toCopyMesh << ", " << toCopyGroups << " )";
}

//================================================================================
/*!
 * \brief Return "to copy mesh and groups"
 */
//================================================================================

void StdMeshers_ImportSource1D_i::GetCopySourceMesh(CORBA::Boolean& toCopyMesh,
                                                    CORBA::Boolean& toCopyGroups)
{
  GetImpl()->GetCopySourceMesh(toCopyMesh,toCopyGroups);
}

//================================================================================
/*!
 * \brief Write parameters in a string
  * \retval char* - resulting string
 */
//================================================================================

char* StdMeshers_ImportSource1D_i::SaveTo()
{
  std::ostringstream os;
  os << " " << _groupEntries->length();

  SALOMEDS::Study_var study = SMESH_Gen_i::GetSMESHGen()->GetCurrentStudy();
  for ( size_t i = 0; i < _groupEntries->length(); ++i )
  {
    // entry
    os << " " << _groupEntries[i];

    // id
    SALOMEDS::SObject_wrap groupSO = study->FindObjectID( _groupEntries[i] );
    CORBA::Object_var     groupObj;
    if ( !groupSO->_is_nil() )
      groupObj = groupSO->GetObject();
    StdMeshers_ObjRefUlils::SaveToStream( groupObj, os );
  }

  myBaseImpl->SaveTo( os );

  return CORBA::string_dup( os.str().c_str() );
}

//================================================================================
/*!
 * \brief Retrieve parameters from the string
  * \param theStream - the input string
 */
//================================================================================

void StdMeshers_ImportSource1D_i::LoadFrom( const char* theStream )
{
  std::istringstream is( theStream );

  int nbGroups;
  is >> nbGroups;

  _groupEntries = new SMESH::string_array;
  _groupEntries->length( nbGroups );
  std::string id, entry;
  for ( size_t i = 0; i < _groupEntries->length(); ++i )
  {
    if ( is >> entry )
      _groupEntries[i] = entry.c_str();
    else
    {
      _groupEntries->length( i );
      is.clear(ios::badbit | is.rdstate());
      break;
    }
    if ( is >> id )
      _groupIDs.push_back( id );
    else
    {
      is.clear(ios::badbit | is.rdstate());
      break;
    }
  }

  myBaseImpl->LoadFrom( is );
}

//================================================================================
/*!
 * \brief Retrieve groups by their ids loaded by LoadFrom()
 * This is possible only when all meshes are fully loaded
 */
//================================================================================

void StdMeshers_ImportSource1D_i::UpdateAsMeshesRestored()
{
  std::vector<SMESH_Group*> smesh_groups;
  for ( unsigned i = 0; i < _groupIDs.size(); ++i )
  {
    std::istringstream is( _groupIDs[i].c_str() );
    SMESH::SMESH_GroupBase_var group =
      StdMeshers_ObjRefUlils::LoadObjectFromStream<SMESH::SMESH_GroupBase>( is );
    if ( SMESH_GroupBase_i* gp_i = SMESH::DownCast<SMESH_GroupBase_i*>( group ))
      smesh_groups.push_back( gp_i->GetSmeshGroup() );
  }
  GetImpl()->RestoreGroups(smesh_groups);
}

//=============================================================================
/*!
 *  StdMeshers_ImportSource1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_ImportSource1D* StdMeshers_ImportSource1D_i::GetImpl()
{
  MESSAGE( "StdMeshers_ImportSource1D_i::GetImpl" );
  return ( ::StdMeshers_ImportSource1D* )myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type 
  * \param type - dimension (see SMESH::Dimension enumeration)
  * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 * 
 * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================  
CORBA::Boolean StdMeshers_ImportSource1D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

