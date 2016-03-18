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
//  File   : StdMeshers_SegmentLengthAroundVertex_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_SegmentLengthAroundVertex_I_HXX_
#define _SMESH_SegmentLengthAroundVertex_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_SegmentLengthAroundVertex.hxx"

class SMESH_Gen;

/*!
 * \brief This hypothesis specifies length of segments adjacent to the vertex the
 * hypothesis is assigned to
 */
class StdMeshers_SegmentLengthAroundVertex_i:
  public virtual POA_StdMeshers::StdMeshers_SegmentLengthAroundVertex,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_SegmentLengthAroundVertex_i( PortableServer::POA_ptr thePOA,
                                          int                     theStudyId,
                                          ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_SegmentLengthAroundVertex_i();

  // Set length
  void SetLength( CORBA::Double theLength )
    throw ( SALOME::SALOME_Exception );

  // Get length
  CORBA::Double GetLength();

  // Get implementation
  ::StdMeshers_SegmentLengthAroundVertex* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

 protected:
  virtual std::string getMethodOfParameter(const int paramIndex, int nbVars) const;
};

#endif

