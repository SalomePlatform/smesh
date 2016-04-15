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
//  File   : StdMeshers_Geometric1D_i.cxx
//  Module : SMESH
//
#include "StdMeshers_Geometric1D_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include <Utils_CorbaException.hxx>
#include <utilities.h>

//=============================================================================
/*!
 *  StdMeshers_Geometric1D_i::StdMeshers_Geometric1D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_Geometric1D_i::StdMeshers_Geometric1D_i( PortableServer::POA_ptr thePOA,
                                                    int                     theStudyId,
                                                    ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA ),
    StdMeshers_Reversible1D_i( this )
{
  myBaseImpl = new ::StdMeshers_Geometric1D( theGenImpl->GetANewId(),
                                             theStudyId,
                                             theGenImpl );
}

//=============================================================================
/*!
 * Sets <start segment length> parameter value
 */
//=============================================================================

void StdMeshers_Geometric1D_i::SetStartLength( CORBA::Double theLength )
  throw (SALOME::SALOME_Exception)
{
  try {
    this->GetImpl()->SetStartLength( theLength );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump()
    << _this() << ".SetStartLength( " << SMESH::TVar(theLength) << " )";
}

//=============================================================================
/*!
 * Sets <common ratio> parameter value
 */
//=============================================================================

void StdMeshers_Geometric1D_i::SetCommonRatio( CORBA::Double factor )
  throw (SALOME::SALOME_Exception)
{
  try {
    this->GetImpl()->SetCommonRatio( factor );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump()
    << _this() << ".SetCommonRatio( " << SMESH::TVar(factor) << " )";
}

//=============================================================================
/*!
 *  Returns length of the first segment 
 */
//=============================================================================

CORBA::Double StdMeshers_Geometric1D_i::GetStartLength()
{
  return this->GetImpl()->GetStartLength();
}

//=============================================================================
/*!
 * Returns value of Common Ratio
 */
//=============================================================================

CORBA::Double StdMeshers_Geometric1D_i::GetCommonRatio()
{
  return this->GetImpl()->GetCommonRatio();
}

//=============================================================================
/*!
 *  StdMeshers_Geometric1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_Geometric1D* StdMeshers_Geometric1D_i::GetImpl()
{
  return ( ::StdMeshers_Geometric1D* )myBaseImpl;
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

CORBA::Boolean StdMeshers_Geometric1D_i::IsDimSupported(::SMESH::Dimension type)
{
  return type == SMESH::DIM_1D;
}
