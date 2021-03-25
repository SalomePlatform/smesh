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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : StdMeshers_BlockRenumber_i.cxx
//  Author : Edward AGAPOV
//  Module : SMESH
//  $Header$
//
#include "StdMeshers_ObjRefUlils.hxx"
#include "StdMeshers_BlockRenumber_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"

//=============================================================================
/*!
 *  StdMeshers_BlockRenumber_i::StdMeshers_BlockRenumber_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_BlockRenumber_i::StdMeshers_BlockRenumber_i( PortableServer::POA_ptr thePOA,
                                                        ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_BlockRenumber( theGenImpl->GetANewId(),
                                               theGenImpl );
}

//================================================================================
/*!
 * \brief Set orientation of blocks
 */
//================================================================================

void StdMeshers_BlockRenumber_i::SetBlocksOrientation( const StdMeshers::blockcs_array& blockCS )
{
  try {
    SMESH_Comment dump;
    CORBA::String_var entry;
    std::vector< StdMeshers_BlockCS > bcsVec( blockCS.length() );
    for ( size_t i = 0; i < bcsVec.size(); i++ )
    {
      StdMeshers_BlockCS& bcs = bcsVec[i];
      if ( !CORBA::is_nil( blockCS[i].solid )    &&
           !CORBA::is_nil( blockCS[i].vertex000 )&&
           !CORBA::is_nil( blockCS[i].vertex001 ))
      {
        entry          = blockCS[i].solid->GetStudyEntry();
        bcs._solid     = entry.in();
        entry          = blockCS[i].vertex000->GetStudyEntry();
        bcs._vertex000 = entry.in();
        entry          = blockCS[i].vertex001->GetStudyEntry();
        bcs._vertex001 = entry.in();

        if ( !dump.empty() ) dump << ",\n    ";
        dump << "StdMeshers.BlockCS( "
             << bcs._solid << ", " << bcs._vertex000 << ", " << bcs._vertex001
             << " )";
      }
    }
    this->GetImpl()->SetBlocksOrientation( bcsVec );

    // Update Python script
    SMESH::TPythonDump() << _this() << ".SetBlocksOrientation([ " << dump << " ])";
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
}

//================================================================================
/*!
 * \brief Return orientation of blocks
 */
//================================================================================

StdMeshers::blockcs_array*  StdMeshers_BlockRenumber_i::GetBlocksOrientation()
{
  const std::vector< StdMeshers_BlockCS >& bcsVec =  this->GetImpl()->GetBlocksOrientation();
  StdMeshers::blockcs_array_var          bcsArray = new StdMeshers::blockcs_array();
  bcsArray->length( bcsVec.size() );
  TopoDS_Shape nullShape;
  for ( size_t i = 0; i < bcsVec.size(); ++i )
  {
    bcsArray[i].solid =
      StdMeshers_ObjRefUlils::EntryOrShapeToGeomObject( bcsVec[i]._solid, nullShape );
    bcsArray[i].vertex000 =
      StdMeshers_ObjRefUlils::EntryOrShapeToGeomObject( bcsVec[i]._vertex000, nullShape );
    bcsArray[i].vertex001 =
      StdMeshers_ObjRefUlils::EntryOrShapeToGeomObject( bcsVec[i]._vertex001, nullShape );
  }
  return bcsArray._retn();
}

//================================================================================
/*!
 * \brief Return geom entries
 */
//================================================================================

bool
StdMeshers_BlockRenumber_i::getObjectsDependOn( std::vector< std::string > & entryArray,
                                                std::vector< int >         & /*subIDArray*/ ) const
{
  const std::vector< StdMeshers_BlockCS >& bcsVec =
    const_cast<StdMeshers_BlockRenumber_i*>(this)->GetImpl()->GetBlocksOrientation();
  entryArray.reserve( entryArray.capacity() + 3 * bcsVec.size());
  for ( size_t i = 0; i < bcsVec.size(); ++i )
  {
    entryArray.push_back( bcsVec[i]._solid     );
    entryArray.push_back( bcsVec[i]._vertex000 );
    entryArray.push_back( bcsVec[i]._vertex001 );
  }
  return !bcsVec.empty();
}

//================================================================================
/*!
 * \brief Update geom entries for a new geometry
 */
//================================================================================

bool StdMeshers_BlockRenumber_i::setObjectsDependOn( std::vector< std::string > & entryArray,
                                                     std::vector< int >         & /*subIDArray*/ )
{
  std::vector< StdMeshers_BlockCS > bcsVec( entryArray.size() / 3 );
  for ( size_t i = 0; i + 2 < entryArray.size(); i += 3 )
  {
    StdMeshers_BlockCS& bcs = bcsVec[i];
    bcs._solid     = entryArray[ i ];
    bcs._vertex000 = entryArray[ i + 1 ];
    bcs._vertex001 = entryArray[ i + 2 ];
  }
  this->GetImpl()->SetBlocksOrientation( bcsVec );
  return true;
}


//=============================================================================
/*!
 *  StdMeshers_BlockRenumber_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_BlockRenumber* StdMeshers_BlockRenumber_i::GetImpl()
{
  return ( ::StdMeshers_BlockRenumber* )myBaseImpl;
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
CORBA::Boolean StdMeshers_BlockRenumber_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}

