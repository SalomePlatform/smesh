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

//  File   : StdMeshers_ViscousLayers_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_ViscousLayers_I_HXX_
#define _SMESH_ViscousLayers_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_ViscousLayers.hxx"

class SMESH_Gen;

class STDMESHERS_I_EXPORT StdMeshers_ViscousLayers_i:
  public virtual POA_StdMeshers::StdMeshers_ViscousLayers,
  public virtual SMESH_Hypothesis_i
{
 public:
  // Constructor
  StdMeshers_ViscousLayers_i( PortableServer::POA_ptr thePOA,
                              int                     theStudyId,
                              ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_ViscousLayers_i();

  void SetIgnoreFaces(const ::SMESH::long_array& faceIDs) throw ( SALOME::SALOME_Exception );
  SMESH::long_array* GetIgnoreFaces();

  void SetFaces(const SMESH::long_array& faceIDs,
                CORBA::Boolean           toIgnore) throw (SALOME::SALOME_Exception);
  SMESH::long_array* GetFaces();
  CORBA::Boolean     GetIsToIgnoreFaces();

  void SetTotalThickness(::CORBA::Double thickness) throw ( SALOME::SALOME_Exception );
  ::CORBA::Double GetTotalThickness();

  void SetNumberLayers(::CORBA::Short nb) throw ( SALOME::SALOME_Exception );
  ::CORBA::Short GetNumberLayers();

  void SetStretchFactor(::CORBA::Double factor) throw ( SALOME::SALOME_Exception );
  ::CORBA::Double GetStretchFactor();

  void SetMethod( ::StdMeshers::VLExtrusionMethod how );
  ::StdMeshers::VLExtrusionMethod GetMethod();

  // Get implementation
  ::StdMeshers_ViscousLayers* GetImpl();

  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

 protected:
  virtual std::string getMethodOfParameter(const int paramIndex, int nbVars) const;
};

#endif
