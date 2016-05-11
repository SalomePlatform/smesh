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
//  File   : StdMeshers_ViscousLayers_i.cxx
//  Module : SMESH
//
#include "StdMeshers_ViscousLayers_i.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_ViscousLayers_i::StdMeshers_ViscousLayers_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_ViscousLayers_i::StdMeshers_ViscousLayers_i( PortableServer::POA_ptr thePOA,
                                                        int                     theStudyId,
                                                        ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_ViscousLayers( theGenImpl->GetANewId(),
                                               theStudyId,
                                               theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_ViscousLayers_i::~StdMeshers_ViscousLayers_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_ViscousLayers_i::~StdMeshers_ViscousLayers_i()
{
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

void StdMeshers_ViscousLayers_i::SetFaces(const ::SMESH::long_array& faceIDs,
                                          CORBA::Boolean             toIgnore)
  throw ( SALOME::SALOME_Exception )
{
  vector<int> ids( faceIDs.length() );
  for ( unsigned i = 0; i < ids.size(); ++i )
    if (( ids[i] = faceIDs[i] ) < 1 )
      THROW_SALOME_CORBA_EXCEPTION( "Invalid face id", SALOME::BAD_PARAM );

  GetImpl()->SetBndShapes( ids, toIgnore );

  SMESH::TPythonDump() << _this() << ".SetFaces( " << faceIDs << ", " << toIgnore << " )";
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

SMESH::long_array* StdMeshers_ViscousLayers_i::GetFaces()
{
  vector<int> idsVec = GetImpl()->GetBndShapes();
  SMESH::long_array_var ids = new SMESH::long_array;
  ids->length( idsVec.size() );
  for ( unsigned i = 0; i < idsVec.size(); ++i )
    ids[i] = idsVec[i];
  return ids._retn();
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

SMESH::long_array* StdMeshers_ViscousLayers_i::GetIgnoreFaces()
{
  if ( GetImpl()->IsToIgnoreShapes() )
    return this->GetFaces();
  return new SMESH::long_array;
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

CORBA::Boolean StdMeshers_ViscousLayers_i::GetIsToIgnoreFaces()
{
  return GetImpl()->IsToIgnoreShapes();
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

void StdMeshers_ViscousLayers_i::SetIgnoreFaces(const ::SMESH::long_array& faceIDs)
throw ( SALOME::SALOME_Exception )
{
  vector<int> ids( faceIDs.length() );
  for ( unsigned i = 0; i < ids.size(); ++i )
    if (( ids[i] = faceIDs[i] ) < 1 )
      THROW_SALOME_CORBA_EXCEPTION( "Invalid face id", SALOME::BAD_PARAM );
  GetImpl()->SetBndShapes( ids, /*toIgnore=*/true );
  SMESH::TPythonDump() << _this() << ".SetIgnoreFaces( " << faceIDs << " )";
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

void StdMeshers_ViscousLayers_i::SetTotalThickness(::CORBA::Double thickness)
throw ( SALOME::SALOME_Exception )
{
  if ( thickness < 1e-100 )
    THROW_SALOME_CORBA_EXCEPTION( "Invalid thickness", SALOME::BAD_PARAM );
  GetImpl()->SetTotalThickness(thickness);
  SMESH::TPythonDump() << _this() << ".SetTotalThickness( " << SMESH::TVar(thickness) << " )";
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

::CORBA::Double StdMeshers_ViscousLayers_i::GetTotalThickness()
{
  return GetImpl()->GetTotalThickness();
}

//================================================================================
/*!
 * \brief 
 *  \param nb - 
 */
//================================================================================

void StdMeshers_ViscousLayers_i::SetNumberLayers(::CORBA::Short nb)
throw ( SALOME::SALOME_Exception )
{
  if ( nb < 1 )
    THROW_SALOME_CORBA_EXCEPTION( "Invalid number of layers", SALOME::BAD_PARAM );
  GetImpl()->SetNumberLayers( nb );
  SMESH::TPythonDump() << _this() << ".SetNumberLayers( " << SMESH::TVar(nb) << " )";
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

::CORBA::Short StdMeshers_ViscousLayers_i::GetNumberLayers()
{
  return CORBA::Short( GetImpl()->GetNumberLayers() );
}

//================================================================================
/*!
 * \brief 
 *  \param factor - 
 */
//================================================================================

void StdMeshers_ViscousLayers_i::SetStretchFactor(::CORBA::Double factor)
throw ( SALOME::SALOME_Exception )
{
  if ( factor < 1 )
    THROW_SALOME_CORBA_EXCEPTION( "Invalid stretch factor, it must be >= 1.0", SALOME::BAD_PARAM );
  GetImpl()->SetStretchFactor(factor);
  SMESH::TPythonDump() << _this() << ".SetStretchFactor( " << SMESH::TVar(factor) << " )";
}

//================================================================================
/*!
 * \brief 
 * 
 */
//================================================================================

::CORBA::Double StdMeshers_ViscousLayers_i::GetStretchFactor()
{
  return GetImpl()->GetStretchFactor();
}

//================================================================================
/*!
 * \brief Set Method of computing translation of a node
 */
//================================================================================

void StdMeshers_ViscousLayers_i::SetMethod( ::StdMeshers::VLExtrusionMethod how )
{
  GetImpl()->SetMethod( ::StdMeshers_ViscousLayers::ExtrusionMethod( how ));
  const char* methNames[3] = { "SURF_OFFSET_SMOOTH",
                               "FACE_OFFSET",
                               "NODE_OFFSET" };
  if ( how >= 0 && how < 3 )
    SMESH::TPythonDump() << _this() << ".SetMethod( StdMeshers." << methNames[ how ]<< " )";
}

//================================================================================
/*!
 * \brief Return Method of computing translation of a node
 */
//================================================================================

::StdMeshers::VLExtrusionMethod StdMeshers_ViscousLayers_i::GetMethod()
{
  return (::StdMeshers::VLExtrusionMethod) GetImpl()->GetMethod();
}

//=============================================================================
/*!
 *  Get implementation
 */
//=============================================================================

::StdMeshers_ViscousLayers* StdMeshers_ViscousLayers_i::GetImpl()
{
  return ( ::StdMeshers_ViscousLayers* )myBaseImpl;
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
CORBA::Boolean StdMeshers_ViscousLayers_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}

//================================================================================
/*!
 * \brief Return method name corresponding to index of variable parameter
 */
//================================================================================

std::string StdMeshers_ViscousLayers_i::getMethodOfParameter(const int paramIndex, int ) const
{
  // order of methods was defined by StdMeshersGUI_StdHypothesisCreator::storeParams()
  switch ( paramIndex )
  {
  case 0: return "SetTotalThickness";
  case 1: return "SetNumberLayers";
  case 2: return "SetStretchFactor";
  }
  return "";
}
