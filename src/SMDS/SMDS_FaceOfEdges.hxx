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

#ifndef _SMDS_FaceOfEdges_HeaderFile
#define _SMDS_FaceOfEdges_HeaderFile

#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_Iterator.hxx"

#include <iostream>


class SMDS_FaceOfEdges:public SMDS_MeshFace
{
  public:
	void Print(std::ostream & OS) const;
	SMDS_FaceOfEdges(const SMDS_MeshEdge* edge1,
                         const SMDS_MeshEdge* edge2,
                         const SMDS_MeshEdge* edge3);
	SMDS_FaceOfEdges(const SMDS_MeshEdge* edge1,
                         const SMDS_MeshEdge* edge2,
                         const SMDS_MeshEdge* edge3,
                         const SMDS_MeshEdge* edge4);
		
	SMDSAbs_ElementType GetType() const;
	int NbEdges() const;
	int NbFaces() const;
//	friend bool operator<(const SMDS_FaceOfEdges& e1, const SMDS_FaceOfEdges& e2);

  protected:
  	SMDS_ElemIteratorPtr
		elementsIterator(SMDSAbs_ElementType type) const;

  private:
	const SMDS_MeshEdge* myEdges[4];
        int                  myNbEdges;

};

#endif
