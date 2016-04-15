// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshers_MaxLength_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_MaxLength_I_HXX_
#define _SMESH_MaxLength_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_MaxLength.hxx"

class SMESH_Gen;

// ======================================================
// Local Length hypothesis
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_MaxLength_i:
  public virtual POA_StdMeshers::StdMeshers_MaxLength,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_MaxLength_i( PortableServer::POA_ptr thePOA,
                       int                     theStudyId,
                       ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_MaxLength_i();

  // Set length
  void SetLength( CORBA::Double theLength )
    throw ( SALOME::SALOME_Exception );
  // Set precision

  // Sets preestimation flag
  void SetUsePreestimatedLength( CORBA::Boolean toUse)
    throw ( SALOME::SALOME_Exception );

  // Get length
  CORBA::Double GetLength();

  // Returns true if preestemated length is defined
  CORBA::Boolean HavePreestimatedLength();

  CORBA::Double GetPreestimatedLength();

  // Sets preestemated length
  void SetPreestimatedLength(CORBA::Double theLength);

  // Returns preestimation flag
  CORBA::Boolean GetUsePreestimatedLength();

  // Get implementation
  ::StdMeshers_MaxLength* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

 protected:
  virtual std::string getMethodOfParameter(const int paramIndex, int nbVars) const;
};

#endif

