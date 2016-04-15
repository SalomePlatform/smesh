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

//  File   : StdMeshers_LayerDistribution_i.cxx
//  Author : Edward AGAPOV
//  Module : SMESH
//
#include "StdMeshers_LayerDistribution_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution_i::StdMeshers_LayerDistribution_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_LayerDistribution_i::StdMeshers_LayerDistribution_i( PortableServer::POA_ptr thePOA,
                                                                int                     theStudyId,
                                                                ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_LayerDistribution_i::StdMeshers_LayerDistribution_i" );
  myBaseImpl = new ::StdMeshers_LayerDistribution( theGenImpl->GetANewId(),
                                                   theStudyId,
                                                   theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution_i::~StdMeshers_LayerDistribution_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_LayerDistribution_i::~StdMeshers_LayerDistribution_i()
{
  MESSAGE( "StdMeshers_LayerDistribution_i::~StdMeshers_LayerDistribution_i" );
  if ( !myHyp->_is_nil() )
    myHyp->UnRegister();
}

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution_i::SetLayerDistribution
 *

 */
//=============================================================================

void StdMeshers_LayerDistribution_i::SetLayerDistribution(SMESH::SMESH_Hypothesis_ptr hyp1D)
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    SMESH_Hypothesis_i * hyp_i = SMESH::DownCast< SMESH_Hypothesis_i*>( hyp1D );
    bool isNewHyp = ( hyp_i->GetImpl() != this->GetImpl()->GetLayerDistribution() );
    this->GetImpl()->SetLayerDistribution( hyp_i->GetImpl() );
    myHyp = SMESH::SMESH_Hypothesis::_duplicate( hyp1D );
    myHyp->Register();
    // Remove SO of 1D hypothesis if it was published
    if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen())
    {
      SALOMEDS::Study_var study = gen->GetCurrentStudy();
      SALOMEDS::SObject_var  SO = gen->ObjectToSObject( study, hyp1D );
      if ( ! SO->_is_nil() )
      {
        SALOMEDS::StudyBuilder_var builder = study->NewBuilder();
        builder->RemoveObjectWithChildren( SO );
        SO->UnRegister();
      }
    }
    // Update Python script: write creation of 1D hyp as it is not published and
    // for this, SMESH_Gen does not write it's creation
    if ( isNewHyp )
      SMESH::TPythonDump() << hyp1D << " = "
                           << SMESH_Gen_i::GetSMESHGen() << ".CreateHypothesis('"
                           << hyp_i->GetName() << "', '" << hyp_i->GetLibName() << "')";
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetLayerDistribution( " << hyp1D << " )";
}

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution_i::GetLayerDistribution
 *
 *  Returns <number of layers> parameter value
 */
//=============================================================================

SMESH::SMESH_Hypothesis_ptr StdMeshers_LayerDistribution_i::GetLayerDistribution()
{
  SMESH::SMESH_Hypothesis_var hyp = myHyp;
  return hyp._retn();
}

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_LayerDistribution* StdMeshers_LayerDistribution_i::GetImpl()
{
  return ( ::StdMeshers_LayerDistribution* )myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type 
 *  \param type - dimension (see SMESH::Dimension enumeration)
 *  \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 * 
 * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================  

CORBA::Boolean StdMeshers_LayerDistribution_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}

//================================================================================
/*!
 * \brief Write parameters in a string
 *  \retval char* - resulting string
 */
//================================================================================

char* StdMeshers_LayerDistribution_i::SaveTo()
{
  ASSERT( myBaseImpl );
  std::ostringstream os;

  ::SMESH_Hypothesis* hyp1D = GetImpl()->GetLayerDistribution();
  SMESH_Hypothesis_i* hyp1D_i = SMESH::DownCast< SMESH_Hypothesis_i*>( myHyp );
  if ( !hyp1D || !hyp1D_i )
    os << "NULL_HYPO ";
  else {
    os << hyp1D->GetName() << " "
       << hyp1D->GetLibName() << " "
       << hyp1D_i->SaveTo() << " ";
  }
  os << SMESH_Hypothesis_i::SaveTo();  // to have a mark of storage version ("VARS...")

  return CORBA::string_dup( os.str().c_str() );
}

//================================================================================
/*!
 * \brief Retrieve parameters from the string
 *  \param theStream - the input string
 */
//================================================================================

void StdMeshers_LayerDistribution_i::LoadFrom( const char* theStream )
{
  ASSERT( myBaseImpl );
  std::istringstream is( theStream );

  string typeName, libName;
  if ( is >> typeName &&
       is >> libName )
  {
    SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var curStudy = gen->GetCurrentStudy();
    gen->SetCurrentStudy( SALOMEDS::Study::_nil() ); // prevent hypo publishing

    try {
      SMESH::SMESH_Hypothesis_var hyp1D =
        gen->CreateHypothesis( typeName.c_str(), libName.c_str() );
      SMESH_Hypothesis_i* hyp1D_i = SMESH::DownCast< SMESH_Hypothesis_i*>( hyp1D );
      if ( hyp1D_i ) {
        hyp1D_i->LoadFrom( & theStream[ (streamoff) is.tellg()+1 ]);
        this->GetImpl()->SetLayerDistribution( hyp1D_i->GetImpl() );
        myHyp = hyp1D;

        SMESH::SMESH_Hypothesis_var me = _this();
        hyp1D->SetHolderHypothesis( me );
        // as hyp1D is not published, its ID changes
        //SMESH::TPythonDump() << _this() << ".SetLayerDistribution( " << hyp1D << " )";

        // restore a mark of storage version ("VARS...")
        SMESH_Hypothesis_i::LoadFrom( & theStream[ (streamoff)is.tellg()+1 ]);
      }
    }
    catch (...) {
    }
    gen->SetCurrentStudy( curStudy );  // enable hypo publishing
  }
}

//================================================================================
/*!
 * \brief Restore myMethod2VarParams by parameters stored in an old study
 */
//================================================================================

void StdMeshers_LayerDistribution_i::setOldParameters (const char* theParameters)
{
  if ( SMESH_Hypothesis_i* hyp1D_i = SMESH::DownCast< SMESH_Hypothesis_i*>( myHyp ))
    hyp1D_i->setOldParameters( theParameters );
}
