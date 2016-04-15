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
//  File   : StdMeshers_Adaptive1D_i.cxx
//  Module : SMESH
//

#include "StdMeshers_Adaptive1D_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "StdMeshers_Adaptive1D.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=======================================================================
//function : StdMeshers_Adaptive1D_i
//purpose  : Constructor
//=======================================================================

StdMeshers_Adaptive1D_i::StdMeshers_Adaptive1D_i( PortableServer::POA_ptr thePOA,
                                                  int                     theStudyId,
                                                  ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_Adaptive1D( theGenImpl->GetANewId(),
                                            theStudyId,
                                            theGenImpl );
}

//=======================================================================
//function : ~StdMeshers_Adaptive1D_i
//purpose  : Destructor
//=======================================================================

StdMeshers_Adaptive1D_i::~StdMeshers_Adaptive1D_i()
{
  MESSAGE( "StdMeshers_Adaptive1D_i::~StdMeshers_Adaptive1D_i" );
}

//=======================================================================
//function : SetMinSize
//purpose  : Sets minimal allowed segment length
//=======================================================================

void StdMeshers_Adaptive1D_i::SetMinSize( CORBA::Double minSegLen )
  throw (SALOME::SALOME_Exception)
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetMinSize( minSegLen );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetMinSize( " << SMESH::TVar(minSegLen) << " )";
}

//=======================================================================
//function : GetMinSize
//purpose  : Returns minimal allowed segment length
//=======================================================================

CORBA::Double StdMeshers_Adaptive1D_i::GetMinSize()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetMinSize();
}

//=======================================================================
//function : SetMaxSize
//purpose  : Sets maximal allowed segment length
//=======================================================================

void StdMeshers_Adaptive1D_i::SetMaxSize( CORBA::Double maxSegLen )
  throw (SALOME::SALOME_Exception)
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetMaxSize( maxSegLen );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetMaxSize( " << SMESH::TVar(maxSegLen) << " )";
}

//=======================================================================
//function : GetMaxSize
//purpose  : Returns maximal allowed segment length
//=======================================================================

CORBA::Double StdMeshers_Adaptive1D_i::GetMaxSize()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetMaxSize();
}

//=======================================================================
//function : SetDeflection
//purpose  : Sets a maximal allowed distance between a segment and an edge.
//=======================================================================

void StdMeshers_Adaptive1D_i::SetDeflection( CORBA::Double theValue )
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetDeflection( theValue );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetDeflection( " << SMESH::TVar(theValue) << " )";
}

//=======================================================================
//function : GetDeflection
//purpose  : Returns deflection
//=======================================================================

CORBA::Double StdMeshers_Adaptive1D_i::GetDeflection()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetDeflection();
}

//=======================================================================
//function : GetImpl
//purpose  : Get implementation
//=======================================================================

::StdMeshers_Adaptive1D* StdMeshers_Adaptive1D_i::GetImpl()
{
  return ( ::StdMeshers_Adaptive1D* )myBaseImpl;
}

//=======================================================================
//function : IsDimSupported
//purpose  : Verify whether hypothesis supports given entity type
//=======================================================================

CORBA::Boolean StdMeshers_Adaptive1D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}
