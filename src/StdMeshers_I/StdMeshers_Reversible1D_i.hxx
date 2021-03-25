// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : StdMeshers_Reversible1D_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_Reversible1D_I_HXX_
#define _SMESH_Reversible1D_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_Reversible1D.hxx"

// ======================================================
// Common methods of Reversible 1D hypotheses
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_Reversible1D_i:
  public virtual POA_StdMeshers::Reversible1D
{
 public:
  StdMeshers_Reversible1D_i( SMESH_Hypothesis_i* reversible );

  //Set Reversed Edges
  void SetReversedEdges( const SMESH::long_array& theIDs);

  //Get Reversed Edges
  SMESH::long_array*  GetReversedEdges();

  //Set the Entry of the Object
  void SetObjectEntry( const char* theEntry);

  //Get Object Entry
  char* GetObjectEntry();

  // Get implementation
  ::StdMeshers_Reversible1D* GetImpl();


  // Methods for copying mesh definition to other geometry

  // Return geometry this hypothesis depends on. Return false if there is no geometry parameter
  virtual bool getObjectsDependOn( std::vector< std::string > & entryArray,
                                   std::vector< int >         & subIDArray ) const;

  // Set new geometry instead of that returned by getObjectsDependOn()
  virtual bool setObjectsDependOn( std::vector< std::string > & entryArray,
                                   std::vector< int >         & subIDArray );
 private:
  SMESH_Hypothesis_i* myHyp;
};
#endif
