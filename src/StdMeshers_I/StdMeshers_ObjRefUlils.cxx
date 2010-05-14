//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESH : implementaion of SMESH idl descriptions
// File      : StdMeshers_ObjRefUlils.cxx
// Created   : Wed Oct 18 15:38:22 2006
// Author    : Edward AGAPOV (eap)
//
#include "StdMeshers_ObjRefUlils.hxx"

#include <TopoDS_Shape.hxx>

using namespace std;

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
        SALOMEDS::SObject_var sobj = gen->ObjectToSObject( gen->GetCurrentStudy(), geom );
        if ( !sobj->_is_nil() ) {
          stream << " " << sobj->GetID();
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

TopoDS_Shape StdMeshers_ObjRefUlils::LoadFromStream( istream & stream)
{
  if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen()) {
    SALOMEDS::Study_var study = gen->GetCurrentStudy();
    if ( ! study->_is_nil() ) {
      string str;
      if (stream >> str) {
        SALOMEDS::SObject_var sobj= study->FindObjectID( str.c_str() );
        CORBA::Object_var obj = gen->SObjectToObject( sobj );
        GEOM::GEOM_Object_var geom = GEOM::GEOM_Object::_narrow( obj );
        return gen->GeomObjectToShape( geom.in() );
      }
    }
  }
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
