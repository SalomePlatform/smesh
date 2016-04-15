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
  std::vector<std::string> funVec;
  std::vector<double>      pointVec;
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
    std::vector<std::string> funVec;
    std::vector<double>      pointVec;
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
//function : SetAxesDirs
//purpose  : Set custom direction of axes
//=======================================================================

void StdMeshers_CartesianParameters3D_i::SetAxesDirs(const SMESH::DirStruct& xDir,
                                                     const SMESH::DirStruct& yDir,
                                                     const SMESH::DirStruct& zDir)
  throw (SALOME::SALOME_Exception)
{
  double coords[9];
  coords[0] = xDir.PS.x;
  coords[1] = xDir.PS.y;
  coords[2] = xDir.PS.z;
  coords[3] = yDir.PS.x;
  coords[4] = yDir.PS.y;
  coords[5] = yDir.PS.z;
  coords[6] = zDir.PS.x;
  coords[7] = zDir.PS.y;
  coords[8] = zDir.PS.z;
  try {
    this->GetImpl()->SetAxisDirs(coords);

    SMESH::TPythonDump() << _this() << ".SetAxesDirs( "
                         << xDir << ", "
                         << yDir << ", "
                         << zDir << " )";
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
}

//=======================================================================
//function : GetAxesDirs
//purpose  : Returns direction of axes
//=======================================================================

void StdMeshers_CartesianParameters3D_i::GetAxesDirs(SMESH::DirStruct& xDir,
                                                     SMESH::DirStruct& yDir,
                                                     SMESH::DirStruct& zDir)
{
  const double* coords = GetImpl()->GetAxisDirs();
  xDir.PS.x = coords[0];
  xDir.PS.y = coords[1];
  xDir.PS.z = coords[2];
  yDir.PS.x = coords[3];
  yDir.PS.y = coords[4];
  yDir.PS.z = coords[5];
  zDir.PS.x = coords[6];
  zDir.PS.y = coords[7];
  zDir.PS.z = coords[8];
}

//=======================================================================
//function : SetFixedPoint
//purpose  : * Set/unset a fixed point, at which a node will be created provided that grid
//           * is defined by spacing in all directions
//=======================================================================

void StdMeshers_CartesianParameters3D_i::SetFixedPoint(const SMESH::PointStruct& ps,
                                                       CORBA::Boolean            toUnset)
{
  double p[3] = { ps.x, ps.y, ps.z };
  GetImpl()->SetFixedPoint( p, toUnset );

  SMESH::TPythonDump() << _this() << ".SetFixedPoint( " << ps << ", " << toUnset << " )";
}

//=======================================================================
//function : GetFixedPoint
//purpose  : Returns a fixed point
//=======================================================================

CORBA::Boolean StdMeshers_CartesianParameters3D_i::GetFixedPoint(SMESH::PointStruct& ps)
{
  double p[3];
  if ( GetImpl()->GetFixedPoint( p ) )
  {
    ps.x = p[0];
    ps.y = p[1];
    ps.z = p[2];
    return true;
  }
  else
  {
    ps.x = 0.;
    ps.y = 0.;
    ps.z = 0.;
  }
  return false;
}

//=======================================================================
//function : SetToAddEdges
//purpose  : Enables implementation of geometrical edges into the mesh.
//=======================================================================

void StdMeshers_CartesianParameters3D_i::SetToAddEdges(CORBA::Boolean toAdd)
{
  GetImpl()->SetToAddEdges( toAdd );
  SMESH::TPythonDump() << _this() << ".SetToAddEdges( " << toAdd << " )";
}

//=======================================================================
//function : GetToAddEdges
//purpose  : Returns true if implementation of geometrical edges into the
//           mesh is enabled
//=======================================================================

CORBA::Boolean StdMeshers_CartesianParameters3D_i::GetToAddEdges()
{
  return GetImpl()->GetToAddEdges();
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
//function : ComputeOptimalAxesDirs
//purpose  : Returns axes at which number of hexahedra is maximal
//=======================================================================

void StdMeshers_CartesianParameters3D_i::
ComputeOptimalAxesDirs(GEOM::GEOM_Object_ptr go,
                       CORBA::Boolean        isOrthogonal,
                       SMESH::DirStruct&     xDir,
                       SMESH::DirStruct&     yDir,
                       SMESH::DirStruct&     zDir)
  throw (SALOME::SALOME_Exception)
{
  TopoDS_Shape shape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( go );
  if ( shape.IsNull() )
    THROW_SALOME_CORBA_EXCEPTION( "Null shape", SALOME::BAD_PARAM );

  double c[9];
  ::StdMeshers_CartesianParameters3D::ComputeOptimalAxesDirs( shape, isOrthogonal, c );

  xDir.PS.x = c[0];
  xDir.PS.y = c[1];
  xDir.PS.z = c[2];
  yDir.PS.x = c[3];
  yDir.PS.y = c[4];
  yDir.PS.z = c[5];
  zDir.PS.x = c[6];
  zDir.PS.y = c[7];
  zDir.PS.z = c[8];
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
  std::vector<std::string> xFuns;
  std::vector<double>      xPoints, coords;
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
