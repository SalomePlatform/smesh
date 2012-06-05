// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : StdMeshers_Import_1D2D_i.cxx
//  Module : SMESH
//
#include "StdMeshers_Import_1D2D_i.hxx"
#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

using namespace std;


//=============================================================================
/*!
 *  StdMeshers_Import_1D2D_i::StdMeshers_Import_1D2D_i
 */
//=============================================================================

StdMeshers_Import_1D2D_i::StdMeshers_Import_1D2D_i (PortableServer::POA_ptr thePOA,
                                                    int                     theStudyId,
                                                    ::SMESH_Gen*            theGenImpl)
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA ), 
    SMESH_Algo_i( thePOA ),
    SMESH_2D_Algo_i( thePOA )
{
  MESSAGE( "StdMeshers_Import_1D2D_i::StdMeshers_Import_1D2D_i" );
  myBaseImpl = new ::StdMeshers_Import_1D2D(theGenImpl->GetANewId(),
                                            theStudyId,
                                            theGenImpl );
}

//-----------------------------------------------------------------------------

StdMeshers_Import_1D2D_i::~StdMeshers_Import_1D2D_i()
{
  MESSAGE( "StdMeshers_Import_1D2D_i::~StdMeshers_Import_1D2D_i" );
}

//-----------------------------------------------------------------------------

::StdMeshers_Import_1D2D* StdMeshers_Import_1D2D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Import_1D2D_i::GetImpl" );
  return ( ::StdMeshers_Import_1D2D* )myBaseImpl;
}

