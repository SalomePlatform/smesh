// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMDS_CellOfNodes.hxx
//  Module : SMESH
//
#ifndef _SMDS_CellOfNodes_HeaderFile
#define _SMDS_CellOfNodes_HeaderFile

#include "SMESH_SMDS.hxx"
        
#include "SMDS_MeshElement.hxx"

#include <smIdType.hxx>

// ============================================================
/*!
 * \brief Base class for elements not contained in the mesh
 */
// ============================================================


class SMDS_EXPORT SMDS_CellOfNodes : public SMDS_MeshElement
{
public:

  virtual smIdType GetID() const;
  virtual int      GetShapeID() const;

  virtual void setIsMarked( bool is ) const;
  virtual bool isMarked() const;

  virtual VTKCellType GetVtkType() const { return VTK_EMPTY_CELL; }

 protected:

  SMDS_CellOfNodes( smIdType id = -1, int shapeID = 0);

  virtual void setID( const smIdType id);
  virtual void setShapeID( const int shapeID );

  smIdType  myID;
  int       myShapeID;

  enum Bits { // use the 1st right bit of myShapeId to set/unset a mark
    BIT_IS_MARKED = 1,
    BITS_SHIFT = 1
  };
};
#endif
