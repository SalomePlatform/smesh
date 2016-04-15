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
//  File   : StdMeshers_Projection_1D_2D_3D_i.cxx
//  Module : SMESH
//
#include "StdMeshers_Projection_1D_2D_3D_i.hxx"

#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include "StdMeshers_ObjRefUlils.hxx"

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_Projection_3D_i::StdMeshers_Projection_3D_i
 */
//=============================================================================

StdMeshers_Projection_3D_i::StdMeshers_Projection_3D_i( PortableServer::POA_ptr thePOA,
                                                        int                     theStudyId,
                                                        ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA ), 
       SMESH_Algo_i( thePOA ),
       SMESH_3D_Algo_i( thePOA )
{
  MESSAGE( "StdMeshers_Projection_3D_i::StdMeshers_Projection_3D_i" );
  myBaseImpl = new ::StdMeshers_Projection_3D( theGenImpl->GetANewId(),
                                    theStudyId,
                                    theGenImpl );
}
//-----------------------------------------------------------------------------

StdMeshers_Projection_3D_i::~StdMeshers_Projection_3D_i()
{
  MESSAGE( "StdMeshers_Projection_3D_i::~StdMeshers_Projection_3D_i" );
}
//-----------------------------------------------------------------------------

::StdMeshers_Projection_3D* StdMeshers_Projection_3D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Projection_3D_i::GetImpl" );
  return ( ::StdMeshers_Projection_3D* )myBaseImpl;
}

//================================================================================
/*!
 * \brief Return true if the algorithm is applicable to a shape
 */
//================================================================================

CORBA::Boolean StdMeshers_Projection_3D_i::IsApplicable(const TopoDS_Shape &S,
                                                        CORBA::Boolean toCheckAll)
{
  return ::StdMeshers_Projection_3D::IsApplicable( S, toCheckAll );
}

//=============================================================================
/*!
 *  StdMeshers_Projection_2D_i::StdMeshers_Projection_2D_i
 */
//=============================================================================

StdMeshers_Projection_2D_i::StdMeshers_Projection_2D_i( PortableServer::POA_ptr thePOA,
                                                        int                     theStudyId,
                                                        ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA ), 
       SMESH_Algo_i( thePOA ),
       SMESH_2D_Algo_i( thePOA )
{
  MESSAGE( "StdMeshers_Projection_2D_i::StdMeshers_Projection_2D_i" );
  myBaseImpl = new ::StdMeshers_Projection_2D( theGenImpl->GetANewId(),
                                    theStudyId,
                                    theGenImpl );
}
//-----------------------------------------------------------------------------

StdMeshers_Projection_2D_i::~StdMeshers_Projection_2D_i()
{
  MESSAGE( "StdMeshers_Projection_2D_i::~StdMeshers_Projection_2D_i" );
}
//-----------------------------------------------------------------------------

::StdMeshers_Projection_2D* StdMeshers_Projection_2D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Projection_2D_i::GetImpl" );
  return ( ::StdMeshers_Projection_2D* )myBaseImpl;
}


//=============================================================================
/*!
 *  StdMeshers_Projection_1D2D_i::StdMeshers_Projection_1D2D_i
 */
//=============================================================================

StdMeshers_Projection_1D2D_i::StdMeshers_Projection_1D2D_i( PortableServer::POA_ptr thePOA,
                                                            int                     theStudyId,
                                                            ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ),
       SMESH_Hypothesis_i( thePOA ),
       SMESH_Algo_i( thePOA ),
       SMESH_2D_Algo_i( thePOA )
{
  MESSAGE( "StdMeshers_Projection_1D2D_i::StdMeshers_Projection_1D2D_i" );
  myBaseImpl = new ::StdMeshers_Projection_1D2D( theGenImpl->GetANewId(),
                                                 theStudyId,
                                                 theGenImpl );
}
//-----------------------------------------------------------------------------

StdMeshers_Projection_1D2D_i::~StdMeshers_Projection_1D2D_i()
{
  MESSAGE( "StdMeshers_Projection_1D2D_i::~StdMeshers_Projection_1D2D_i" );
}
//-----------------------------------------------------------------------------

::StdMeshers_Projection_1D2D* StdMeshers_Projection_1D2D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Projection_1D2D_i::GetImpl" );
  return ( ::StdMeshers_Projection_1D2D* )myBaseImpl;
}


//=============================================================================
/*!
 *  StdMeshers_Projection_1D_i::StdMeshers_Projection_1D_i
 */
//=============================================================================

StdMeshers_Projection_1D_i::StdMeshers_Projection_1D_i( PortableServer::POA_ptr thePOA,
                                                        int                     theStudyId,
                                                        ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA ), 
       SMESH_Algo_i( thePOA ),
       SMESH_1D_Algo_i( thePOA )
{
  MESSAGE( "StdMeshers_Projection_1D_i::StdMeshers_Projection_1D_i" );
  myBaseImpl = new ::StdMeshers_Projection_1D( theGenImpl->GetANewId(),
                                    theStudyId,
                                    theGenImpl );
}
//-----------------------------------------------------------------------------

StdMeshers_Projection_1D_i::~StdMeshers_Projection_1D_i()
{
  MESSAGE( "StdMeshers_Projection_1D_i::~StdMeshers_Projection_1D_i" );
}
//-----------------------------------------------------------------------------

::StdMeshers_Projection_1D* StdMeshers_Projection_1D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Projection_1D_i::GetImpl" );
  return ( ::StdMeshers_Projection_1D* )myBaseImpl;
}
