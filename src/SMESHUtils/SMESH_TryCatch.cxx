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

// ------------------------------------------------------------------
#include "SMESH_TryCatch.hxx"

void SMESH::throwSalomeEx(const char* txt)
{
  throw SALOME_Exception( txt );
}

void SMESH::doNothing(const char* txt)
{
  (void)txt; // unused in release mode
  MESSAGE( txt << " " << __FILE__ << ": " << __LINE__ );
}

const char* SMESH::returnError(const char* txt)
{
  return txt;
}

// ------------------------------------------------------------------

#include "SMESH_ComputeError.hxx"
#include "SMDS_SetIterator.hxx"
#include <boost/make_shared.hpp>

#define _case2char(err) case err: return #err;

// Return SMESH_ComputeError::myName as text, to be used to dump errors in terminal
std::string SMESH_ComputeError::CommonName() const
{
  switch( myName ) {
  _case2char(COMPERR_OK              );
  _case2char(COMPERR_BAD_INPUT_MESH  );
  _case2char(COMPERR_STD_EXCEPTION   );
  _case2char(COMPERR_OCC_EXCEPTION   );
  _case2char(COMPERR_SLM_EXCEPTION   );
  _case2char(COMPERR_EXCEPTION       );
  _case2char(COMPERR_MEMORY_PB       );
  _case2char(COMPERR_ALGO_FAILED     );
  _case2char(COMPERR_BAD_SHAPE       );
  _case2char(COMPERR_WARNING         );
  _case2char(COMPERR_CANCELED        );
  _case2char(COMPERR_NO_MESH_ON_SHAPE);
  _case2char(COMPERR_BAD_PARMETERS   );
  default:;
  }
  return "";
}

// Return the most severe error
SMESH_ComputeErrorPtr SMESH_ComputeError::Worst( SMESH_ComputeErrorPtr er1,
                                                 SMESH_ComputeErrorPtr er2 )
{
  if ( !er1 ) return er2;
  if ( !er2 ) return er1;
  // both not NULL
  if ( er1->IsOK() ) return er2;
  if ( er2->IsOK() ) return er1;
  // both not OK
  if ( !er1->IsKO() ) return er2;
  if ( !er2->IsKO() ) return er1;
  // both KO
  bool hasInfo1 = er1->myComment.size() || er1->HasBadElems();
  bool hasInfo2 = er2->myComment.size() || er2->HasBadElems();
  if ( er1->myName == er2->myName ||
       hasInfo1    != hasInfo2 )
    return hasInfo1 < hasInfo2 ? er2 : er1;

  return er1->myName == COMPERR_CANCELED ? er2 : er1;
}

// Return bad elements
SMDS_ElemIteratorPtr SMESH_BadInputElements::getElements()
{
  typedef SMDS_SetIterator< const SMDS_MeshElement*,
                            std::list< const SMDS_MeshElement* >::const_iterator> TIterator;
  return boost::make_shared< TIterator >( myBadElements.begin(), myBadElements.end() );
}

// Temporary remove its elements before the mesh compacting
void SMESH_BadInputElements::tmpClear()
{
  myBadElements.clear();
}

// Re-add elements after the mesh compacting
void SMESH_BadInputElements::add( const SMDS_MeshElement* element )
{
  myBadElements.push_back( element );
}
