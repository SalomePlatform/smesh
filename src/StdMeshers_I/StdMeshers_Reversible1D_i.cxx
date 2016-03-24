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
//  File   : StdMeshers_Reversible1D_i.cxx
//  Module : SMESH
//
#include "StdMeshers_Reversible1D_i.hxx"
#include "SMESH_PythonDump.hxx"

#include <Utils_CorbaException.hxx>
#include <utilities.h>

#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

StdMeshers_Reversible1D_i::StdMeshers_Reversible1D_i( SMESH_Hypothesis_i* reversible )
  : myHyp( reversible )
{
}

//=============================================================================
/*!
 *  StdMeshers_Reversible1D_i::SetReversedEdges
 *
 *  Set edges to reverse
 */
//=============================================================================

void StdMeshers_Reversible1D_i::SetReversedEdges( const SMESH::long_array& theIds )
{
  try {
    std::vector<int> ids( theIds.length() );
    CORBA::Long iEnd = theIds.length();
    for ( CORBA::Long i = 0; i < iEnd; i++ )
      ids[ i ] = theIds[ i ];

    this->GetImpl()->SetReversedEdges( ids );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << myHyp->_this() << ".SetReversedEdges( " << theIds << " )";
}

//=============================================================================
/*!
 *  StdMeshers_Reversible1D_i::SetObjectEntry
 *
 *  Set the Entry for the Main Object
 */
//=============================================================================

void StdMeshers_Reversible1D_i::SetObjectEntry( const char* theEntry )
{
  std::string entry(theEntry); // actually needed as theEntry is spoiled by moment of dumping
  try {
    this->GetImpl()->SetObjectEntry( entry.c_str() );
    // Update Python script
    SMESH::TPythonDump() << myHyp->_this() << ".SetObjectEntry( \"" << entry.c_str() << "\" )";
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),SALOME::BAD_PARAM );
  }
}

//=============================================================================
/*!
 *  StdMeshers_Reversible1D_i::GetObjectEntry
 *
 *  Set the Entry for the Main Object
 */
//=============================================================================

char* StdMeshers_Reversible1D_i::GetObjectEntry()
{
  const char* entry;
  try {
    entry = this->GetImpl()->GetObjectEntry();
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  return CORBA::string_dup( entry );
}

//=============================================================================
/*!
 *  StdMeshers_Reversible1D_i::GetReversedEdges
 *
 *  Get reversed edges
 */
//=============================================================================

SMESH::long_array* StdMeshers_Reversible1D_i::GetReversedEdges()
{
  SMESH::long_array_var anArray = new SMESH::long_array;
  std::vector<int> ids = this->GetImpl()->GetReversedEdges();
  anArray->length( ids.size() );
  for ( CORBA::ULong i = 0; i < ids.size(); i++)
    anArray [ i ] = ids [ i ];

  return anArray._retn();
}

//=============================================================================
/*!
 *  StdMeshers_Reversible1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_Reversible1D* StdMeshers_Reversible1D_i::GetImpl()
{
  return ( ::StdMeshers_Reversible1D* )myHyp->GetImpl();
}
