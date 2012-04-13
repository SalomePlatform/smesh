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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_2D_Algo_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$
//
#ifndef _SMESH_2D_ALGO_I_HXX_
#define _SMESH_2D_ALGO_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include "SMESH_Algo_i.hxx"

// ======================================================
// Generic 2D algorithm
// ======================================================
class SMESH_I_EXPORT SMESH_2D_Algo_i:
  public virtual POA_SMESH::SMESH_2D_Algo,
  public virtual SMESH_Algo_i
{
protected:
  // Constructor : placed in protected section to prohibit creation of generic class instance
  SMESH_2D_Algo_i( PortableServer::POA_ptr thePOA );

public:
  // Destructor
  virtual ~SMESH_2D_Algo_i();
  
  // Verify whether algorithm supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif
