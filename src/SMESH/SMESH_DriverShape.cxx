// Copyright (C) 2007-2022  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  File   : SMESH_DriverShape.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//
#include <utilities.h>
#include <Utils_SALOME_Exception.hxx>
#include "SMESH_DriverShape.hxx"

// step include
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <Interface_Static.hxx>

// Brep include
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>

//Occ include
#include <TopoDS.hxx>

#ifndef DISABLE_PSMESH
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
namespace fs = boost::filesystem;
#endif

/**
 * @brief Import the content of a shape file (STEP) into a TopDS_Shape object
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int importSTEPShape(const std::string shape_file, TopoDS_Shape& aShape){

  MESSAGE("Importing STEP shape from " << shape_file);
  STEPControl_Reader reader;
  // Forcing Unit in meter
  Interface_Static::SetCVal("xstep.cascade.unit","M");
  Interface_Static::SetIVal("read.step.ideas", 1);
  Interface_Static::SetIVal("read.step.nonmanifold", 1);
  IFSelect_ReturnStatus aStat = reader.ReadFile(shape_file.c_str());
  if(aStat != IFSelect_RetDone){
    throw SALOME_Exception("Reading error for " + shape_file);
  }

  int NbTrans = reader.TransferRoots();
  // There should be only one shape within the file
  assert(NbTrans==1);
  aShape = reader.OneShape();

  return false;
}

/**
 * @brief Export the content of a TopoDS_Shape into a shape file (STEP)
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int exportSTEPShape(const std::string shape_file, const TopoDS_Shape& aShape){

  MESSAGE("Exporting STEP shape to " << shape_file);

  STEPControl_Writer aWriter;
  // Forcing Unit in meter
  Interface_Static::SetCVal("xstep.cascade.unit","M");
  Interface_Static::SetCVal("write.step.unit","M");
  Interface_Static::SetIVal("write.step.nonmanifold", 1);

  IFSelect_ReturnStatus aStat = aWriter.Transfer(aShape,STEPControl_AsIs);
  if(aStat != IFSelect_RetDone){
    throw SALOME_Exception("Reading error for " + shape_file);
  }

  aStat = aWriter.Write(shape_file.c_str());

  if(aStat != IFSelect_RetDone){
    throw SALOME_Exception("Writing error for " + shape_file);
  }
  return aStat;
}

/**
 * @brief Import the content of a shape file (BREP) into a TopDS_Shape object
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int importBREPShape(const std::string shape_file, TopoDS_Shape& aShape){

  MESSAGE("Importing BREP shape from " << shape_file);
  BRep_Builder builder;
  BRepTools::Read(aShape, shape_file.c_str(), builder);

  return false;
}

/**
 * @brief Export the content of a TopoDS_Shape into a shape file (BREP)
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int exportBREPShape(const std::string shape_file, const TopoDS_Shape& aShape){

  MESSAGE("Exporting BREP shape to " << shape_file);
  BRepTools::Write(aShape, shape_file.c_str());

  return false;
}

/**
 * @brief Import the content of a shape file into a TopDS_Shape object
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int SMESH_DriverShape::importShape(const std::string shape_file, TopoDS_Shape& aShape){
#ifndef DISABLE_PSMESH
  std::string type = fs::path(shape_file).extension().string();
  boost::algorithm::to_lower(type);
  if (type == ".brep"){
    return importBREPShape(shape_file, aShape);
  } else if (type == ".step"){
    return importSTEPShape(shape_file, aShape);
  } else {
    throw SALOME_Exception("Unknow format for importShape: " + type);
  }
#else
  return 0;
#endif
}

/**
 * @brief Import the content of a shape file into a TopDS_Shape object
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int SMESH_DriverShape::exportShape(const std::string shape_file, const TopoDS_Shape& aShape){
#ifndef DISABLE_PSMESH
  std::string type = fs::path(shape_file).extension().string();
  boost::algorithm::to_lower(type);
  if (type == ".brep"){
    return exportBREPShape(shape_file, aShape);
  } else if (type == ".step"){
    return exportSTEPShape(shape_file, aShape);
  } else {
    throw SALOME_Exception("Unknow format for exportShape: " + type);
  }
#else
  return 0;
#endif
}
