//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshers_NumberOfSegments_i.hxx
//           Moved here from SMESH_NumberOfSegments_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_NUMBEROFSEGMENTS_I_HXX_
#define _SMESH_NUMBEROFSEGMENTS_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_NumberOfSegments.hxx"

// ======================================================
// Number of segments hypothesis
// ======================================================
class StdMeshers_NumberOfSegments_i:
  public virtual POA_StdMeshers::StdMeshers_NumberOfSegments,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_NumberOfSegments_i( PortableServer::POA_ptr thePOA,
                            int                     theStudyId,
			    ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_NumberOfSegments_i();

  // Set number of segments
  void SetNumberOfSegments( CORBA::Long theSegmentsNumber )
    throw ( SALOME::SALOME_Exception );
  // Get number of segments
  CORBA::Long GetNumberOfSegments();

  // Set scalar factor
  void SetScaleFactor( CORBA::Double theScaleFactor )
    throw ( SALOME::SALOME_Exception );
  // Get scalar factor
  CORBA::Double GetScaleFactor();

  // Get implementation
  ::StdMeshers_NumberOfSegments* GetImpl();
};

#endif
