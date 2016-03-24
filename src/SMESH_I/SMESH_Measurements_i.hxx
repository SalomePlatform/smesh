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
//  File   : SMESH_Measurements_i.hxx
//  Author : Pavel TELKOV, Open CASCADE S.A.S. (pavel.telkov@opencascade.com)

#ifndef _SMESH_MEASUREMENTS_I_HXX_
#define _SMESH_MEASUREMENTS_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Measurements)

#include "SALOME_GenericObj_i.hh"

class SMESHDS_Mesh;

namespace SMESH
{

  /*
    Measurements
  */
  class SMESH_I_EXPORT Measurements_i: public virtual POA_SMESH::Measurements,
                                       public virtual SALOME::GenericObj_i
  {
  public:
    Measurements_i();
    ~Measurements_i();

    /*!
     * minimal distance between two given entities
     */
    SMESH::Measure MinDistance(SMESH::SMESH_IDSource_ptr theSource1,
                               SMESH::SMESH_IDSource_ptr theSource2);

    /*!
     * common bounding box of entities
     */
    SMESH::Measure BoundingBox(const SMESH::ListOfIDSources& theSources);


    /*!
     * sum of length of 1D elements of the source
     */
    double Length(SMESH::SMESH_IDSource_ptr theSource);

    /*!
     * sum of area of 2D elements of the source
     */
    double Area(SMESH::SMESH_IDSource_ptr);

    /*!
     * sum of volume of 3D elements of the source
     */
    double Volume(SMESH::SMESH_IDSource_ptr);
  };
}

#endif
