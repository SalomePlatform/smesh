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
// File      : SMESH_MeshPartDS.hxx
// Created   : Thu Sep 13 20:26:47 2012
// Author    : Edward AGAPOV (eap)

#ifndef __SMESH_MeshPartDS_HXX__
#define __SMESH_MeshPartDS_HXX__

#include "SMESHDS_Mesh.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <list>

//=============================================================================
/*!
 * \brief Class providing SMESHDS_Mesh API to SMESH_IDSource. 
 *        It is used to export a part of mesh as a whole mesh.
 */
// Implementation is in SMESH_Mesh_i.cxx where this class was extracted from

class SMESH_MeshPartDS : public SMESHDS_Mesh
{
public:
  SMESH_MeshPartDS(SMESH::SMESH_IDSource_ptr                    meshPart);
  SMESH_MeshPartDS(const std::list< const SMDS_MeshElement* > & badElems );

  virtual SMDS_NodeIteratorPtr   nodesIterator     (bool idInceasingOrder=false) const;
  virtual SMDS_EdgeIteratorPtr   edgesIterator     (bool idInceasingOrder=false) const;
  virtual SMDS_FaceIteratorPtr   facesIterator     (bool idInceasingOrder=false) const;
  virtual SMDS_VolumeIteratorPtr volumesIterator   (bool idInceasingOrder=false) const;

  virtual SMDS_ElemIteratorPtr elementsIterator(SMDSAbs_ElementType type=SMDSAbs_All) const;
  virtual SMDS_ElemIteratorPtr elementGeomIterator(SMDSAbs_GeometryType type) const;
  virtual SMDS_ElemIteratorPtr elementEntityIterator(SMDSAbs_EntityType type) const;

private:
  TIDSortedElemSet _elements[ SMDSAbs_NbElementTypes ];
  SMESHDS_Mesh*    _meshDS;
  /*!
   * \brief Class used to access to protected data of SMDS_MeshInfo
   */
  struct TMeshInfo : public SMDS_MeshInfo
  {
    void Add(const SMDS_MeshElement* e) { SMDS_MeshInfo::addWithPoly( e ); }
  };
};

#endif
