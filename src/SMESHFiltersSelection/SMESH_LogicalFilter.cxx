//  SALOME SALOMEGUI : implementation of desktop and GUI kernel
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
//  File   : SMESH_LogicalFilter.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESH_LogicalFilter.hxx"

/*
  Class       : SMESH_LogicalFilter
  Description : Filter for combaining several filters with logical operation (OR or AND)
*/

IMPLEMENT_STANDARD_HANDLE( SMESH_LogicalFilter, SALOME_Filter )
IMPLEMENT_STANDARD_RTTIEXT( SMESH_LogicalFilter, SALOME_Filter )

//=======================================================================
// name    : SMESH_LogicalFilter::SMESH_LogicalFilter
// Purpose : Constructor
//=======================================================================
SMESH_LogicalFilter::SMESH_LogicalFilter( const SMESH_ListOfFilter& theFilters,
                                        const int                theLogOp )
{
  myFilters = theFilters;
  myLogOp = theLogOp;
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
Standard_Boolean SMESH_LogicalFilter::IsOk( const Handle(SALOME_InteractiveObject)& theIO ) const
{
  SMESH_ListOfFilter::Iterator anIter( myFilters );
  for ( ; anIter.More(); anIter.Next() )
  {
    Handle(SALOME_Filter) aFilter = anIter.Value();
    if ( !aFilter.IsNull() )
    {
      if ( myLogOp == LO_OR &&  anIter.Value()->IsOk( theIO ) )
        return true;
      if ( myLogOp == LO_AND && !anIter.Value()->IsOk( theIO ) )
        return false;
    }
  }

  return myLogOp == LO_OR ? false : true;
}

//=======================================================================
// name    : SMESH_LogicalFilter::SetFilters
// Purpose : Set new list of filters. Old wilters are removed
//=======================================================================
void SMESH_LogicalFilter::SetFilters( const SMESH_ListOfFilter& theFilters )
{
  myFilters = theFilters;
}

//=======================================================================
// name    : SMESH_LogicalFilter::SetLogOp
// Purpose : Set logical operation
//=======================================================================
void SMESH_LogicalFilter::SetLogOp( const int theLogOp )
{
  myLogOp = theLogOp;
}

//=======================================================================
// name    : SMESH_LogicalFilter::GetFilters
// Purpose : Get list of filters
//=======================================================================
const SMESH_ListOfFilter& SMESH_LogicalFilter::GetFilters() const
{
  return myFilters;
}

//=======================================================================
// name    : SMESH_LogicalFilter::GetLogOp
// Purpose : Get logical operation
//=======================================================================
int SMESH_LogicalFilter::GetLogOp() const
{
  return myLogOp;
}



































