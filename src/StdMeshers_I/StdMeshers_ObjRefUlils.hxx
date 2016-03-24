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
// File      : StdMeshers_ObjRefUlils.hxx
// Created   : Wed Oct 18 15:15:27 2006
// Author    : Edward AGAPOV (eap)
//
#ifndef StdMeshers_ObjRefUlils_HeaderFile
#define StdMeshers_ObjRefUlils_HeaderFile

#include "SMESH_Gen_i.hxx"

/*!
 * \brief Class encapsulates methods
 *  - converting internal objects to CORBA objects and backward and
 *  - persistence methods for such objects
 *
 * These methods are useful for hypotheses referring to other objects
 * like meshes, geom objects, other hypotheses, etc.
 */
class StdMeshers_ObjRefUlils
{
public:
  /*!
   * \brief Return GEOM Object correspoding to TopoDS_Shape
    * \param theShape - input TopoDS_Shape
    * \retval GEOM::GEOM_Object_ptr - result object
   */
   static GEOM::GEOM_Object_ptr ShapeToGeomObject (const TopoDS_Shape& theShape ) {
     if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen())
       return gen->ShapeToGeomObject( theShape );
     else
       return GEOM::GEOM_Object::_nil();
   }

  /*!
   * \brief Return TopoDS_Shape correspoding to GEOM_Object
    * \param theGeomObject - input object
    * \retval TopoDS_Shape - result TopoDS_Shape
   */
  static TopoDS_Shape GeomObjectToShape(GEOM::GEOM_Object_ptr theGeomObject) {
     if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen())
       return gen->GeomObjectToShape( theGeomObject );
     else
       return TopoDS_Shape();
  }

  /*!
   * \brief Return study entry of GEOM Object
   */
  static std::string GeomObjectToEntry(GEOM::GEOM_Object_ptr theGeomObject);

  /*!
   * \brief Return GEOM Object by its study entry or TopoDS_Shape
   */
  static GEOM::GEOM_Object_ptr EntryOrShapeToGeomObject (const std::string&  theEntry,
                                                         const TopoDS_Shape& theShape);


  /*!
   * \brief Store the shape in the stream
    * \param theShape - shape to store
    * \param stream - the stream
   */
  static void SaveToStream( const TopoDS_Shape& theShape, std::ostream & stream);

  /*!
   * \brief Retrieve a shape from the stream
    * \param stream - the stream
    * \retval TopoDS_Shape - resulting shape
   */
  static TopoDS_Shape LoadFromStream( std::istream & stream, std::string* entry=NULL );

  /*!
   * \brief Store the CORBA object in the stream
    * \param obj - object to store
    * \param stream - the stream
   */
  static void SaveToStream( CORBA::Object_ptr obj, std::ostream & stream );

  /*!
   * \brief Retrieve a CORBA object from the stream 
    * \param stream - the stream
    * \retval CORBA::Object_ptr - result object
   */
  template<class TInterface> 
  static 
  typename TInterface::_var_type LoadObjectFromStream( std::istream & stream )
  {
    if (SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen()) {
      std::string str;
      if (stream >> str) {
        if ( StudyContext* myStudyContext = gen->GetCurrentStudyContext() ) {
          std::string ior = myStudyContext->getIORbyOldId( atoi( str.c_str() ));
          if ( !ior.empty() )
             return TInterface::_narrow(gen->GetORB()->string_to_object( ior.c_str() ));
        }
      }
    }
    return TInterface::_nil();
  }

  /*!
   * \brief Store the study entry of object in the stream
    * \param studyEntry - the study entry
    * \param stream - the stream
   */
  static void SaveToStream( const std::string& studyEntry, std::ostream & stream);

};

#endif
