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
//  File   : SMDS_MeshVolume.hxx
//  Module : SMESH

#ifndef _SMDS_VolumeOfNodes_HeaderFile
#define _SMDS_VolumeOfNodes_HeaderFile

#include "SMDS_MeshVolume.hxx"

class SMDS_VolumeOfNodes:public SMDS_MeshVolume
{
	
  public:
	SMDS_VolumeOfNodes(
		SMDS_MeshNode * node1,
		SMDS_MeshNode * node2,
		SMDS_MeshNode * node3,
		SMDS_MeshNode * node4,
		SMDS_MeshNode * node5,
		SMDS_MeshNode * node6,
		SMDS_MeshNode * node7,
		SMDS_MeshNode * node8);

	void Print(ostream & OS) const;
	int NbFaces() const;
	int NbNodes() const;
	int NbEdges() const;
	SMDSAbs_ElementType GetType() const;	
  protected:
	SMDS_Iterator<const SMDS_MeshElement *> *
		elementsIterator(SMDSAbs_ElementType type) const;
	SMDS_MeshNode * myNodes[8];
};
#endif
