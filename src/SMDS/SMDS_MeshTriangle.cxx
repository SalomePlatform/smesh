//  SMESH SMDS : implementaion of Salome mesh data structure
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
//  File   : SMDS_MeshTriangle.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
#include "SMDS_MeshTriangle.ixx"


//=======================================================================
//function : SMDS_MeshTriangle
//purpose  : 
//=======================================================================

SMDS_MeshTriangle::SMDS_MeshTriangle(const Standard_Integer ID,
				     const Standard_Integer idnode1, 
				     const Standard_Integer idnode2, 
				     const Standard_Integer idnode3):SMDS_MeshFace(ID,3)
{
  SetConnections(idnode1,idnode2,idnode3);
  ComputeKey();
}


//=======================================================================
//function : SetConnections
//purpose  : 
//=======================================================================
void SMDS_MeshTriangle::SetConnections(const Standard_Integer idnode1, 
				       const Standard_Integer idnode2,
				       const Standard_Integer idnode3)
{
  Standard_Integer idmin = (idnode1 < idnode2 ? idnode1 : idnode2);
  idmin = (idmin < idnode3 ? idmin : idnode3);
  
  myNodes[0] = idmin;
  if (idmin == idnode1) {
    myNodes[1] = idnode2;
    myNodes[2] = idnode3;
  } else if (idmin == idnode2) {
    myNodes[1] = idnode3;
    myNodes[2] = idnode1;
  } else {
    myNodes[1] = idnode1;
    myNodes[2] = idnode2;
  }

}

