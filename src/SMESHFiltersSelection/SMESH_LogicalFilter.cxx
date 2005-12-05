// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
//  File   : SMESH_LogicalFilter.cxx
//  Module : SMESH

#include "SMESH_LogicalFilter.hxx"

//=======================================================================
// name    : SMESH_LogicalFilter::SMESH_LogicalFilter
// Purpose : Constructor
//=======================================================================
SMESH_LogicalFilter::SMESH_LogicalFilter (const QPtrList<SUIT_SelectionFilter>& theFilters,
                                          const int                             theLogOp)
{
  setFilters(theFilters); 
  setOperation(theLogOp);
}

//=======================================================================
// name    : SMESH_LogicalFilter::~SMESH_LogicalFilter
// Purpose : Destructor
//=======================================================================
SMESH_LogicalFilter::~SMESH_LogicalFilter()
{
}

//=======================================================================
// name    : SMESH_LogicalFilter::IsOk
// Purpose : Verify validity of entry object
//=======================================================================
bool SMESH_LogicalFilter::isOk (const SUIT_DataOwner* owner) const
{
  bool res = true;
  QPtrListIterator<SUIT_SelectionFilter> it (myFilters);
  SUIT_SelectionFilter* filter;
  for (; ((filter = it.current()) != 0) && res; ++it)
  {
    if (myOperation == LO_OR && filter->isOk(owner))
      return true;
    if (myOperation == LO_AND && !filter->isOk(owner))
      return false;
    if (myOperation == LO_NOT)
      return !filter->isOk(owner);
  }

  return (myOperation != LO_OR);
}

//=======================================================================
// name    : SMESH_LogicalFilter::setFilters
// Purpose : Set new list of filters. Old wilters are removed
//=======================================================================
void SMESH_LogicalFilter::setFilters (const QPtrList<SUIT_SelectionFilter>& theFilters)
{
  myFilters = theFilters;
}

//=======================================================================
// name    : SMESH_LogicalFilter::setOperation
// Purpose : Set logical operation
//=======================================================================
void SMESH_LogicalFilter::setOperation (const int theLogOp)
{
  myOperation = theLogOp;
}

//=======================================================================
// name    : SMESH_LogicalFilter::getFilters
// Purpose : Get list of filters
//=======================================================================
const QPtrList<SUIT_SelectionFilter> SMESH_LogicalFilter::getFilters() const
{
  return myFilters;
}

//=======================================================================
// name    : SMESH_LogicalFilter::getOperation
// Purpose : Get logical operation
//=======================================================================
int SMESH_LogicalFilter::getOperation() const
{
  return myOperation;
}
