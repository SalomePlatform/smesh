// Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

//  SMESH SMDS : implementaion of Salome mesh data structure
//  File   : SMDS_MeshElementIDFactory.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_MeshElementIDFactory.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_Mesh.hxx"

#include "utilities.h"

#include "SMDS_UnstructuredGrid.hxx"
#include <vtkCellType.h>

using namespace std;

//=======================================================================
//function : SMDS_MeshElementIDFactory
//purpose  : 
//=======================================================================
SMDS_MeshElementIDFactory::SMDS_MeshElementIDFactory():
  SMDS_MeshNodeIDFactory()
{
//    myIDElements.clear();
//    myVtkIndex.clear();
    myVtkCellTypes.clear();
    myVtkCellTypes.reserve(SMDSEntity_Last);
    myVtkCellTypes[SMDSEntity_Node]            = VTK_VERTEX;
    myVtkCellTypes[SMDSEntity_0D]              = VTK_VERTEX;
    myVtkCellTypes[SMDSEntity_Edge]            = VTK_LINE;
    myVtkCellTypes[SMDSEntity_Quad_Edge]       = VTK_QUADRATIC_EDGE;
    myVtkCellTypes[SMDSEntity_Triangle]        = VTK_TRIANGLE;
    myVtkCellTypes[SMDSEntity_Quad_Triangle]   = VTK_QUADRATIC_TRIANGLE;
    myVtkCellTypes[SMDSEntity_Quadrangle]      = VTK_QUAD;
    myVtkCellTypes[SMDSEntity_Quad_Quadrangle] = VTK_QUADRATIC_TRIANGLE;
    myVtkCellTypes[SMDSEntity_Polygon]         = VTK_POLYGON;
    myVtkCellTypes[SMDSEntity_Quad_Polygon]    = VTK_POLYGON; // -PR- verifer
    myVtkCellTypes[SMDSEntity_Tetra]           = VTK_TETRA;
    myVtkCellTypes[SMDSEntity_Quad_Tetra]      = VTK_QUADRATIC_TETRA;
    myVtkCellTypes[SMDSEntity_Pyramid]         = VTK_PYRAMID;
    myVtkCellTypes[SMDSEntity_Quad_Pyramid]    = VTK_CONVEX_POINT_SET;
    myVtkCellTypes[SMDSEntity_Hexa]            = VTK_HEXAHEDRON;
    myVtkCellTypes[SMDSEntity_Quad_Hexa]       = VTK_QUADRATIC_HEXAHEDRON;
    myVtkCellTypes[SMDSEntity_Penta]           = VTK_WEDGE;
    myVtkCellTypes[SMDSEntity_Quad_Penta]      = VTK_QUADRATIC_WEDGE;
//#ifdef VTK_HAVE_POLYHEDRON
    myVtkCellTypes[SMDSEntity_Polyhedra]       = VTK_POLYHEDRON;
//#else
//    myVtkCellTypes[SMDSEntity_Polyhedra]       = VTK_CONVEX_POINT_SET;
//#endif
    myVtkCellTypes[SMDSEntity_Quad_Polyhedra]  = VTK_CONVEX_POINT_SET;
}

int SMDS_MeshElementIDFactory::SetInVtkGrid(SMDS_MeshElement * elem)
{
   // --- retrieve nodes ID

  SMDS_MeshCell *cell = dynamic_cast<SMDS_MeshCell*>(elem);
  assert(cell);
  vector<vtkIdType> nodeIds;
  SMDS_ElemIteratorPtr it = elem->nodesIterator();
  while(it->more())
  {
      int nodeId = (static_cast<const SMDS_MeshNode*>(it->next()))->getVtkId();
      MESSAGE("   node in cell " << cell->getVtkId() << " : " << nodeId)
      nodeIds.push_back(nodeId);
  }

  // --- insert cell in vtkUnstructuredGrid

  vtkUnstructuredGrid * grid = myMesh->getGrid();
  //int locType = elem->GetType();
  int typ = VTK_VERTEX;//GetVtkCellType(locType);
  int cellId = grid->InsertNextLinkedCell(typ, nodeIds.size(), &nodeIds[0]);
  cell->setVtkId(cellId); 
  //MESSAGE("SMDS_MeshElementIDFactory::SetInVtkGrid " << cellId);
  return cellId;
}

//=======================================================================
//function : BindID
//purpose  : 
//=======================================================================

bool SMDS_MeshElementIDFactory::BindID(int ID, SMDS_MeshElement * elem)
{
  MESSAGE("SMDS_MeshElementIDFactory::BindID " << ID);
  SetInVtkGrid(elem);
  return myMesh->registerElement(ID, elem);
}

//=======================================================================
//function : MeshElement
//purpose  : 
//=======================================================================
SMDS_MeshElement* SMDS_MeshElementIDFactory::MeshElement(int ID)
{
  if ((ID<1) || (ID>=myMesh->myCells.size()))
    return NULL;
  const SMDS_MeshElement* elem = GetMesh()->FindElement(ID);
  return (SMDS_MeshElement*)(elem);
}

//=======================================================================
//function : GetFreeID
//purpose  : 
//=======================================================================

int SMDS_MeshElementIDFactory::GetFreeID()
{
  int ID;
  do {
    ID = SMDS_MeshIDFactory::GetFreeID();
  } while ( MeshElement( ID ));
  return ID;
}

//=======================================================================
//function : ReleaseID
//purpose  : 
//=======================================================================
void SMDS_MeshElementIDFactory::ReleaseID(int ID, int vtkId)
{
  if (ID < 1) // TODO check case ID == O
    {
      MESSAGE("~~~~~~~~~~~~~~ SMDS_MeshElementIDFactory::ReleaseID ID = " << ID);
      return;
    }
  //MESSAGE("~~~~~~~~~~~~~~ SMDS_MeshElementIDFactory::ReleaseID smdsId vtkId " << ID << " " << vtkId);
  if (vtkId >= 0)
    {
      assert(vtkId < myMesh->myCellIdVtkToSmds.size());
      myMesh->myCellIdVtkToSmds[vtkId] = -1;
      myMesh->setMyModified();
    }
  SMDS_MeshIDFactory::ReleaseID(ID);
  if (ID == myMax)
    myMax = 0;
  if (ID == myMin)
    myMax = 0;
}

//=======================================================================
//function : updateMinMax
//purpose  : 
//=======================================================================

void SMDS_MeshElementIDFactory::updateMinMax() const
{
  myMin = IntegerLast();
  myMax = 0;
  for (int i = 0; i < myMesh->myCells.size(); i++)
    {
      if (myMesh->myCells[i])
        {
          int id = myMesh->myCells[i]->GetID();
          if (id > myMax)
            myMax = id;
          if (id < myMin)
            myMin = id;
        }
    }
  if (myMin == IntegerLast())
    myMin = 0;
}

//=======================================================================
//function : elementsIterator
//purpose  : Return an iterator on elements of the factory
//=======================================================================

SMDS_ElemIteratorPtr SMDS_MeshElementIDFactory::elementsIterator() const
{
    return myMesh->elementsIterator(SMDSAbs_All);
}

void SMDS_MeshElementIDFactory::Clear()
{
  //myMesh->myCellIdSmdsToVtk.clear();
  myMesh->myCellIdVtkToSmds.clear();
  myMin = myMax = 0;
  SMDS_MeshIDFactory::Clear();
}

int SMDS_MeshElementIDFactory::GetVtkCellType(int SMDSType)
{
    assert((SMDSType >=0) && (SMDSType< SMDSEntity_Last));
    return myVtkCellTypes[SMDSType];
}
