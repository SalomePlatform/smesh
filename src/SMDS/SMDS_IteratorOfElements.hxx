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

#include "SMDS_Iterator.hxx"
#include "SMDS_MeshElement.hxx"
#include <set>

using namespace std;

class SMDS_IteratorOfElements:public SMDS_ElemIterator
{
  public:
/////////////////////////////////////////////////////////////////////////////
/// Create an iterator which look for elements of type type which are linked 
/// to the element element. it is the iterator to get connectivity of element
//////////////////////////////////////////////////////////////////////////////
	SMDS_IteratorOfElements(const SMDS_MeshElement * element,
                                SMDSAbs_ElementType type,
                                const SMDS_ElemIteratorPtr& it);
	bool more();
	const SMDS_MeshElement * next();

  private:
	SMDS_ElemIteratorPtr t2Iterator;
	SMDS_ElemIteratorPtr t1Iterator;
	SMDSAbs_ElementType myType;	
	const SMDS_MeshElement * myProxyElement;
	const SMDS_MeshElement * myElement;		
	bool myReverseIteration;

	set<const SMDS_MeshElement*> alreadyReturnedElements;
	set<const SMDS_MeshElement*>::iterator itAlreadyReturned;	
	bool subMore();
	const SMDS_MeshElement * subNext();
};
