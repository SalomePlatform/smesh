//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : StdMeshers_QuadrangleParams_i.hxx
//  Author : Sergey KUUL, OCC
//  Module : SMESH
//  $Header$

#ifndef _SMESH_QUADRANGLEPARAMS_I_HXX_
#define _SMESH_QUADRANGLEPARAMS_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_QuadrangleParams.hxx"

// ======================================================
// QuadrangleParams hypothesis
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_QuadrangleParams_i:
  public virtual POA_StdMeshers::StdMeshers_QuadrangleParams,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_QuadrangleParams_i (PortableServer::POA_ptr thePOA,
                                 int                     theStudyId,
                                 ::SMESH_Gen*            theGenImpl);
  // Destructor
  virtual ~StdMeshers_QuadrangleParams_i();

  // Set length
  //void SetLength( CORBA::Double theLength, CORBA::Boolean theIsStart )
  //  throw ( SALOME::SALOME_Exception );

  // Get length
  //CORBA::Double GetLength(CORBA::Boolean theIsStart);

  // Set base vertex for triangles
  void SetTriaVertex (CORBA::Long vertID);

  // Get base vertex for triangles
  CORBA::Long GetTriaVertex();
  
  // Set the Entry of the Object
  void SetObjectEntry (const char* theEntry);

  // Get Object Entry
  char* GetObjectEntry();

  // Set the type of quadrangulation
  void SetQuadType (StdMeshers::QuadType type);

  // Get the type of quadrangulation
  StdMeshers::QuadType GetQuadType();

  // Get implementation
  ::StdMeshers_QuadrangleParams* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif
