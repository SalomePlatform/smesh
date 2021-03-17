// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshers_FixedPoints1D_i.cxx
//  Author : Damien COQUERET, OCC
//  Module : SMESH
//  $Header$
//
#include "StdMeshers_FixedPoints1D_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_FixedPoints1D_i::StdMeshers_FixedPoints1D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_FixedPoints1D_i::StdMeshers_FixedPoints1D_i( PortableServer::POA_ptr thePOA,
                                                        ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA ),
    StdMeshers_Reversible1D_i( this )
{
  myBaseImpl = new ::StdMeshers_FixedPoints1D(theGenImpl->GetANewId(),
                                              theGenImpl);
}

//=============================================================================
/*!
 *  StdMeshers_FixedPoints1D_i::~StdMeshers_FixedPoints1D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_FixedPoints1D_i::~StdMeshers_FixedPoints1D_i()
{
}

//=============================================================================
/*!
 *  StdMeshers_FixedPoints1D_i::SetNbSegments
 */
//=============================================================================

void StdMeshers_FixedPoints1D_i::SetNbSegments(const SMESH::smIdType_array& listNbSeg)

{
  ASSERT( myBaseImpl );
  try {
    std::vector<smIdType> nbsegs( listNbSeg.length() );
    CORBA::ULong iEnd = listNbSeg.length();
    for ( CORBA::ULong i = 0; i < iEnd; i++ )
      nbsegs[ i ] = listNbSeg[ i ];
    this->GetImpl()->SetNbSegments( nbsegs );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetNbSegments( " << listNbSeg << " )";
}

//=============================================================================
/*!
 *  StdMeshers_FixedPoints1D_i::SetPoints
 */
//=============================================================================

void StdMeshers_FixedPoints1D_i::SetPoints(const SMESH::double_array& listParams) 
     
{
  ASSERT( myBaseImpl );
  try {
    std::vector<double> params( listParams.length() );
    CORBA::Long iEnd = listParams.length();
    for ( CORBA::Long i = 0; i < iEnd; i++ )
      params[ i ] = listParams[ i ];
    this->GetImpl()->SetPoints( params );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetPoints( " << listParams << " )";
}

//=============================================================================
/*!
 *  StdMeshers_FixedPoints1D_i::GetPoints
 *
 *  Get list of point's parameters
 */
//=============================================================================

SMESH::double_array* StdMeshers_FixedPoints1D_i::GetPoints()
{
  ASSERT( myBaseImpl );
  SMESH::double_array_var anArray = new SMESH::double_array;
  std::vector<double> params = this->GetImpl()->GetPoints();
  anArray->length( static_cast<CORBA::ULong>( params.size() ));
  for ( CORBA::ULong i = 0; i < params.size(); i++)
    anArray [ i ] = params [ i ];

  return anArray._retn();
}

//=============================================================================
/*!
 *  StdMeshers_FixedPoints1D_i::GetNbSegments
 *
 *  Get list of point's parameters
 */
//=============================================================================

SMESH::smIdType_array* StdMeshers_FixedPoints1D_i::GetNbSegments()
{
  ASSERT( myBaseImpl );
  SMESH::smIdType_array_var anArray = new SMESH::smIdType_array;
  std::vector<smIdType> nbsegs = this->GetImpl()->GetNbSegments();
  anArray->length( static_cast<CORBA::ULong>( nbsegs.size() ));
  for ( CORBA::ULong i = 0; i < nbsegs.size(); i++)
    anArray [ i ] = nbsegs [ i ];

  return anArray._retn();
}

//=============================================================================
/*!
 *  StdMeshers_FixedPoints1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_FixedPoints1D* StdMeshers_FixedPoints1D_i::GetImpl()
{
  return ( ::StdMeshers_FixedPoints1D* )myBaseImpl;
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
CORBA::Boolean StdMeshers_FixedPoints1D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

//================================================================================
/*!
 * \brief Return geometry this hypothesis depends on. Return false if there is no geometry parameter
 */
//================================================================================

bool
StdMeshers_FixedPoints1D_i::getObjectsDependOn( std::vector< std::string > & entryArray,
                                                std::vector< int >         & subIDArray ) const
{
  return StdMeshers_Reversible1D_i::getObjectsDependOn( entryArray, subIDArray );
}

//================================================================================
/*!
 * \brief Set new geometry instead of that returned by getObjectsDependOn()
 */
//================================================================================

bool
StdMeshers_FixedPoints1D_i::setObjectsDependOn( std::vector< std::string > & entryArray,
                                                std::vector< int >         & subIDArray )
{
  return StdMeshers_Reversible1D_i::setObjectsDependOn( entryArray, subIDArray );
}
