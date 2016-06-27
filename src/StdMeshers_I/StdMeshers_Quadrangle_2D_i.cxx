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
//  File   : StdMeshers_Quadrangle_2D_i.cxx
//           Moved here from SMESH_Quadrangle_2D_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "StdMeshers_Quadrangle_2D_i.hxx"
#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include "StdMeshers_QuadFromMedialAxis_1D2D.hxx"

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_Quadrangle_2D_i::StdMeshers_Quadrangle_2D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_Quadrangle_2D_i::StdMeshers_Quadrangle_2D_i( PortableServer::POA_ptr thePOA,
                                                        int                     theStudyId,
                                                        ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA ),
    SMESH_Algo_i( thePOA ),
    SMESH_2D_Algo_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_Quadrangle_2D( theGenImpl->GetANewId(),
                                               theStudyId,
                                               theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_Quadrangle_2D_i::~StdMeshers_Quadrangle_2D_i
 *
 *  Destructor
 *
 */
//=============================================================================

StdMeshers_Quadrangle_2D_i::~StdMeshers_Quadrangle_2D_i()
{
}

//=============================================================================
/*!
 *  StdMeshers_Quadrangle_2D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_Quadrangle_2D* StdMeshers_Quadrangle_2D_i::GetImpl()
{
  return ( ::StdMeshers_Quadrangle_2D* )myBaseImpl;
}

//=============================================================================
/*!
 *  StdMeshers_Quadrangle_2D_i::IsApplicable
 *
 *  Return true if the algorithm is applicable to a shape
 */
//=============================================================================

CORBA::Boolean StdMeshers_Quadrangle_2D_i::IsApplicable( const TopoDS_Shape &S,
                                                         CORBA::Boolean toCheckAll )
{
  return ::StdMeshers_Quadrangle_2D::IsApplicable( S, toCheckAll );
}

//=============================================================================
/*!
 *  StdMeshers_QuadFromMedialAxis_1D2D_i::StdMeshers_QuadFromMedialAxis_1D2D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_QuadFromMedialAxis_1D2D_i::
StdMeshers_QuadFromMedialAxis_1D2D_i( PortableServer::POA_ptr thePOA,
                                      int                     theStudyId,
                                      ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA ),
    SMESH_Algo_i( thePOA ),
    SMESH_2D_Algo_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_QuadFromMedialAxis_1D2D( theGenImpl->GetANewId(),
                                                         theStudyId,
                                                         theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_QuadFromMedialAxis_1D2D_i::~StdMeshers_QuadFromMedialAxis_1D2D_i
 *
 *  Destructor
 *
 */
//=============================================================================

StdMeshers_QuadFromMedialAxis_1D2D_i::~StdMeshers_QuadFromMedialAxis_1D2D_i()
{
}

//================================================================================
/*!
 * \brief Return true if the algorithm is applicable to a shape
 */
//================================================================================

CORBA::Boolean StdMeshers_QuadFromMedialAxis_1D2D_i::IsApplicable( const TopoDS_Shape &S,
                                                                   CORBA::Boolean toCheckAll )
{
  return ::StdMeshers_QuadFromMedialAxis_1D2D::IsApplicable( S, toCheckAll );
}
