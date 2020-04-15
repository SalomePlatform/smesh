// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMDS : implementation of Salome mesh data structure
//  File   : SMDS_MeshFace.hxx
//  Module : SMESH
//
#ifndef _SMDS_MeshFace_HeaderFile
#define _SMDS_MeshFace_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_MeshCell.hxx"

#include "Utils_SALOME_Exception.hxx"

/*!
 * \brief Mesh face. This type is not allocated.
 *        It is only used as function argument type to provide more clear semantic.
 */
class SMDS_EXPORT SMDS_MeshFace : public SMDS_MeshCell
{
  void init( const std::vector<vtkIdType>& vtkNodeIds )
  {
    SMDSAbs_EntityType entity = SMDSEntity_Triangle;
    switch ( vtkNodeIds.size())
    {
    case 3: entity = SMDSEntity_Triangle;          break;
    case 4: entity = SMDSEntity_Quadrangle;        break;
    case 6: entity = SMDSEntity_Quad_Triangle;     break;
    case 8: entity = SMDSEntity_Quad_Quadrangle;   break;
    case 7: entity = SMDSEntity_BiQuad_Triangle;   break;
    case 9: entity = SMDSEntity_BiQuad_Quadrangle; break;
    default: throw SALOME_Exception("wrong face nodes");
    }
    SMDS_MeshCell::init( entity, vtkNodeIds );
  }

  friend class SMDS_Mesh;

 public:

  virtual SMDSAbs_ElementType GetType() const { return SMDSAbs_Face; }

  static SMDSAbs_ElementType Type() { return SMDSAbs_Face; }
};

#endif
