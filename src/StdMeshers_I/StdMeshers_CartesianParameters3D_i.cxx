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
//  File   : StdMeshers_CartesianParameters3D_i.cxx
//  Module : SMESH
//
#include "StdMeshers_CartesianParameters3D_i.hxx"

#include "StdMeshers_CartesianParameters3D.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#define _vec2array( v, a,conversion )           \
  {                                             \
    a->length( v.size() );                      \
    for ( size_t i = 0; i < v.size(); ++i )     \
      a[i] = conversion( v[i] );                \
  }
#define _array2vec(a,v,conversion)              \
  {                                             \
    v.resize( a.length() );                     \
    for ( size_t i = 0; i < v.size(); ++i )     \
      v[i] = conversion ( a[i] );               \
  }
namespace
{
  const char* _string2chars(const std::string& s ) { return s.c_str(); }
}

//=============================================================================
/*!
 *  StdMeshers_CartesianParameters3D_i::StdMeshers_CartesianParameters3D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_CartesianParameters3D_i::
StdMeshers_CartesianParameters3D_i( PortableServer::POA_ptr thePOA,
                                    int                     theStudyId,
                                    ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_CartesianParameters3D_i::StdMeshers_CartesianParameters3D_i" );
  myBaseImpl = new ::StdMeshers_CartesianParameters3D( theGenImpl->GetANewId(),
                                                       theStudyId,
                                                       theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_CartesianParameters3D_i::~StdMeshers_CartesianParameters3D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_CartesianParameters3D_i::~StdMeshers_CartesianParameters3D_i()
{
  MESSAGE( "StdMeshers_CartesianParameters3D_i::~StdMeshers_CartesianParameters3D_i" );
}

//=============================================================================
/*!
 * SetGrid
 */
//=============================================================================

void StdMeshers_CartesianParameters3D_i::SetGrid(const SMESH::double_array& coords,
                                                 CORBA::Short               axis)
  throw (SALOME::SALOME_Exception)
{
  std::vector<double> coordVec;//, yCoords, zCoords;
  _array2vec( coords, coordVec, );

  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetGrid( coordVec, axis );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetGrid( " << coords << ", " << axis << " )";
}

//=============================================================================
/*!
 *  GetGrid
 */
//=============================================================================

SMESH::double_array* StdMeshers_CartesianParameters3D_i::GetGrid(CORBA::Short axis)
  throw (SALOME::SALOME_Exception)
{
  std::vector<double> coordVec;
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->GetGrid(coordVec, axis);
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  SMESH::double_array_var coords = new SMESH::double_array();
  _vec2array( coordVec, coords, );

  return coords._retn();
}

//=============================================================================
/*!
 *  SetSizeThreshold
 */
//=============================================================================

void StdMeshers_CartesianParameters3D_i::SetSizeThreshold(CORBA::Double threshold)
  throw (SALOME::SALOME_Exception)
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetSizeThreshold(threshold);
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetSizeThreshold( " << SMESH::TVar(threshold) << " )";
}

//=============================================================================
/*!
 *  GetSizeThreshold
 */
//=============================================================================

CORBA::Double StdMeshers_CartesianParameters3D_i::GetSizeThreshold()
{
  return this->GetImpl()->GetSizeThreshold();
}

//=======================================================================
//function : SetGridSpacing
//\brief Set grid spacing along the three axes
// \param spaceFunctions - functions defining spacing values at given point on axis
// \param internalPoints - points dividing a grid into parts along each direction
// Parameter t of spaceFunction f(t) is a position [0,1] withing bounding box of
// the shape to mesh or withing an interval defined by internal points
//=======================================================================

void StdMeshers_CartesianParameters3D_i::SetGridSpacing(const SMESH::string_array& spaceFunctions,
                                                        const SMESH::double_array& internalPoints,
                                                        CORBA::Short               axis)
  throw (SALOME::SALOME_Exception)
{
  vector<string> funVec;
  vector<double> pointVec;
  _array2vec( spaceFunctions, funVec, (const char*) );
  _array2vec( internalPoints, pointVec, );

  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetGridSpacing( funVec, pointVec, axis );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetGridSpacing( "
                       << spaceFunctions << ", "
                       << internalPoints << ", "
                       << axis << " )";
}

//=======================================================================
//function : GetGridSpacing
//=======================================================================

void StdMeshers_CartesianParameters3D_i::GetGridSpacing(SMESH::string_array_out xSpaceFunctions,
                                                        SMESH::double_array_out xInternalPoints,
                                                        CORBA::Short            axis)
  throw (SALOME::SALOME_Exception)
{
  ASSERT( myBaseImpl );
  try {
    vector<string> funVec;
    vector<double> pointVec;
    this->GetImpl()->GetGridSpacing( funVec, pointVec, axis );

    xSpaceFunctions = new SMESH::string_array();
    xInternalPoints = new SMESH::double_array();

    _vec2array( funVec, xSpaceFunctions, _string2chars );
    _vec2array( pointVec, xInternalPoints, );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
}

//=======================================================================
//function : IsGridBySpacing
//purpose  : Return true if the grid is defined by spacing functions and 
//           not by node coordinates
//=======================================================================

CORBA::Boolean StdMeshers_CartesianParameters3D_i::IsGridBySpacing(CORBA::Short axis)
{
  return this->GetImpl()->IsGridBySpacing(axis);
}

//=======================================================================
//function : ComputeCoordinates
//purpose  : Computes node coordinates by spacing functions
//=======================================================================

SMESH::double_array*
StdMeshers_CartesianParameters3D_i::ComputeCoordinates(CORBA::Double              x0,
                                                       CORBA::Double              x1,
                                                       const SMESH::string_array& spaceFuns,
                                                       const SMESH::double_array& points,
                                                       const char*                axisName )
    throw (SALOME::SALOME_Exception)
{
  vector<string> xFuns;
  vector<double> xPoints, coords;
  _array2vec( spaceFuns, xFuns, (const char*) );
  _array2vec( points, xPoints, );
  
  try {
    this->GetImpl()->ComputeCoordinates( x0, x1, xFuns, xPoints, coords, axisName );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  SMESH::double_array_var res = new SMESH::double_array;
  _vec2array( coords, res,  );

  return res._retn();
}

//=============================================================================
/*!
 *  Get implementation
 */
//=============================================================================

::StdMeshers_CartesianParameters3D* StdMeshers_CartesianParameters3D_i::GetImpl()
{
  MESSAGE( "StdMeshers_CartesianParameters3D_i::GetImpl" );
  return ( ::StdMeshers_CartesianParameters3D* )myBaseImpl;
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

CORBA::Boolean StdMeshers_CartesianParameters3D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}
