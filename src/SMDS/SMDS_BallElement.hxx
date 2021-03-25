// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMDS_BallElement.hxx
//  Module : SMESH
//
#ifndef _SMDS_BallElement_HeaderFile
#define _SMDS_BallElement_HeaderFile

#include "SMESH_SMDS.hxx"
#include "SMDS_MeshCell.hxx"

/*!
 * \brief Ball element. This type is not allocated.
 *        It is only used as function argument type to provide more clear semantic
 *        and to provide API specific to ball element
 */
class SMDS_EXPORT SMDS_BallElement: public SMDS_MeshCell
{
  void init(const SMDS_MeshNode * node, double diameter);

  friend class SMDS_Mesh;

 public:

  double GetDiameter() const;
  void   SetDiameter(double diameter);

  static SMDSAbs_ElementType Type() { return SMDSAbs_Ball; }
};

#endif
