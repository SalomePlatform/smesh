// Copyright (C) 2010-2011  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

#ifndef _SMDS_MESHCELL_HXX_
#define _SMDS_MESHCELL_HXX_

#include "SMDS_MeshElement.hxx"

/*!
 * \brief Base class for all cells
 */

class SMDS_EXPORT SMDS_MeshCell: public SMDS_MeshElement
{
public:
  SMDS_MeshCell();
  virtual ~SMDS_MeshCell();

  virtual bool ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes)= 0;
  virtual bool vtkOrder(const SMDS_MeshNode* nodes[], const int nbNodes) {return true; }

  static int nbCells;

protected:
  inline void exchange(const SMDS_MeshNode* nodes[],int a, int b)
  {
    const SMDS_MeshNode* noda = nodes[a];
    nodes[a] = nodes[b];
    nodes[b] = noda;
  }
};

#endif
