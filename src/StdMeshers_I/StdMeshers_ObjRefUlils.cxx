// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH : implementaion of SMESH idl descriptions
// File      : StdMeshers_ObjRefUlils.cxx
// Created   : Wed Oct 18 15:38:22 2006
// Author    : Edward AGAPOV (eap)
//
#include "StdMeshers_ObjRefUlils.hxx"

#include <SALOMEDS_wrap.hxx>
#include <TopoDS_Shape.hxx>

using namespace std;

//=======================================================================
//function : GeomObjectToEntry
//purpose  : Return study entry of GEOM Object
//=======================================================================

std::string StdMeshers_ObjRefUlils::GeomObjectToEntry(GEOM::GEOM_Object_ptr theGeomObject)
{
  if ( CORBA::is_nil( theGeomObject ))
    return "NULL_OBJECT";

  CORBA::String_var entry = theGeomObject->GetStudyEntry();
  return entry.in();
}

//=======================================================================
//function : EntryOrShapeToGeomObject
//purpose  :  Return GEOM Object by its sytudy entry or TopoDS_Shape
//=======================================================================

GEOM::GEOM_Object_ptr
StdMeshers_ObjRefUlils::EntryOrShapeToGeomObject (const std::string&  theEntry,
                                                  const TopoDS_Shape& theShape)
{
  GEOM::GEOM_Object_var geom = GEOM::GEOM_Object::_nil();

  // try by entry
  if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen()) {
    SALOMEDS::Study_var study = gen->GetCurrentStudy();
    if ( ! theEntry.empty() && ! study->_is_nil() ) {
      SALOMEDS::SObject_wrap sobj = study->FindObjectID( theEntry.c_str() );
      CORBA::Object_var       obj = gen->SObjectToObject( sobj );
      geom = GEOM::GEOM_Object::_narrow( obj );
    }
  }
  // try by TopoDS_Shape
  if ( geom->_is_nil() )
    geom = ShapeToGeomObject( theShape );

  return geom._retn();
}

//================================================================================
  /*!
   * \brief Store the shape in the stream
    * \param theShape - shape to store
    * \param stream - the stream
   */
//================================================================================

void StdMeshers_ObjRefUlils::SaveToStream( const TopoDS_Shape& theShape, ostream & stream)
{
  bool ok = false;
  if ( !theShape.IsNull() ) {
    if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen()) {
      GEOM::GEOM_Object_var geom = gen->ShapeToGeomObject( theShape );
      if ( ! geom->_is_nil() ) {
        SALOMEDS::SObject_wrap sobj = gen->ObjectToSObject( gen->GetCurrentStudy(), geom );
        if ( !sobj->_is_nil() ) {
          CORBA::String_var entry = sobj->GetID();
          stream << " " << entry.in();
          ok = true;
        }
      }
    }
  }
  if ( ! ok )
    stream << " NULL_SHAPE ";
}

//================================================================================
  /*!
   * \brief Retrieve a shape from the stream
    * \param stream - the stream
    * \retval TopoDS_Shape - resulting shape
   */
//================================================================================

TopoDS_Shape StdMeshers_ObjRefUlils::LoadFromStream( istream &    stream,
                                                     std::string* entry)
{
  string str;
  if (stream >> str) {
    if ( entry )
      * entry = str;
    if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen()) {
      SALOMEDS::Study_var study = gen->GetCurrentStudy();
      if ( ! study->_is_nil() ) {
        SALOMEDS::SObject_wrap sobj = study->FindObjectID( str.c_str() );
        CORBA::Object_var       obj = gen->SObjectToObject( sobj );
        GEOM::GEOM_Object_var  geom = GEOM::GEOM_Object::_narrow( obj );
        return gen->GeomObjectToShape( geom.in() );
      }
    }
  }
  if ( entry )
    entry->clear();
  return TopoDS_Shape();
}

//================================================================================
  /*!
   * \brief Store the CORBA object in the stream
    * \param obj - object to store
    * \param stream - the stream
   */
//================================================================================

void StdMeshers_ObjRefUlils::SaveToStream( CORBA::Object_ptr obj,
                                           std::ostream & stream)
{
  bool ok = false;
  if ( !CORBA::is_nil( obj ) ) {
    if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen()) {
      stream << " " << gen->GetObjectId( obj );
      ok = true;
    }
  }
  if ( ! ok )
    stream << " NULL_OBJECT ";
}

//=======================================================================
//function : SaveToStream
//purpose  : Store the study entry of object in the stream
//=======================================================================

void StdMeshers_ObjRefUlils::SaveToStream( const std::string& studyEntry,
                                           std::ostream &     stream)
{
  if ( studyEntry.find_first_not_of( ' ' ) == std::string::npos )
    stream << " NULL_OBJECT ";
  else
    stream << " " << studyEntry;
}
