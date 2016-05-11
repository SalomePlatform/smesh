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
//  File   : SMESH_Algo_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$
//
#include "SMESH_Algo_i.hxx"
#include "SMESH_Algo.hxx"

#include "utilities.h"

#include <string>
#include <vector>

using namespace std;

//=============================================================================
/*!
 *  SMESH_Algo_i::SMESH_Algo_i
 * 
 *  Constructor
 */
//=============================================================================

SMESH_Algo_i::SMESH_Algo_i( PortableServer::POA_ptr thePOA )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
}

//=============================================================================
/*!
 *  SMESH_Algo_i::~SMESH_Algo_i
 * 
 *  Destructor
 */
//=============================================================================

SMESH_Algo_i::~SMESH_Algo_i()
{
}

//=============================================================================
/*!
 *  SMESH_Algo_i::GetCompatibleHypothesis
 * 
 *  Gets list of compatible hypotheses
 */
//=============================================================================

SMESH::ListOfHypothesisName* SMESH_Algo_i::GetCompatibleHypothesis()
{
  SMESH::ListOfHypothesisName_var listOfHypothesis = new SMESH::ListOfHypothesisName;
  const vector<string>& hypList = ( ( ::SMESH_Algo* )myBaseImpl )->GetCompatibleHypothesis();
  int nbHyp = hypList.size();
  listOfHypothesis->length( nbHyp );
  for ( int i = 0; i < nbHyp; i++ ) {
    listOfHypothesis[ i ] = CORBA::string_dup( hypList[ i ].c_str() );
  }
  return listOfHypothesis._retn();
}
