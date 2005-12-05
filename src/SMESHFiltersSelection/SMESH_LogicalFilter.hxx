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
//  File   : SMESH_LogicalFilter.hxx
//  Module : SMESH

#ifndef _SMESH_LogicalFilter_HeaderFile
#define _SMESH_LogicalFilter_HeaderFile

#include <SUIT_SelectionFilter.h>

#include <qptrlist.h>

class SMESH_LogicalFilter : public SUIT_SelectionFilter
{
 public:
  enum { LO_OR, LO_AND, LO_NOT, LO_UNDEFINED };

 public:
  SMESH_LogicalFilter (const QPtrList<SUIT_SelectionFilter>&,
                       const int);
  virtual ~SMESH_LogicalFilter();

  virtual bool isOk (const SUIT_DataOwner*) const;

  void                                 setFilters (const QPtrList<SUIT_SelectionFilter>&);
  void                                 setOperation (const int);
  const QPtrList<SUIT_SelectionFilter> getFilters() const;
  int                                  getOperation() const;

private:
  QPtrList<SUIT_SelectionFilter>  myFilters;
  int                             myOperation;
};

#endif
