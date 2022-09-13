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

//  File   : SMESH_DriverStep.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#include "SMESH_DriverStep.hxx"

#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <Interface_Static.hxx>

//Occ include
#include <TopoDS.hxx>


/**
 * @brief Compares two shape file (STEP)
 *
 * @param file1 first file
 * @param file2 second file
 *
 * @return true if the files are the same
 */
bool diffStepFile(std::string file1, std::string file2){
  std::ifstream sfile1(file1);
  std::ifstream sfile2(file2);
  std::string line1, line2;
  int nb_lines = 0;

  while(!sfile1.eof() && !sfile2.eof()){
    std::getline(sfile1, line1);
    std::getline(sfile2, line2);
    nb_lines++;
    // Skipping 4th line contain date of creation
    if (nb_lines==4){
      std::cout << "Skipping line" << std::endl;
      continue;
    }

    // if lines are different end of read
    if(line1 != line2){
      return false;
    }

  }
  // True if we reached the end of both files
  return sfile1.eof() && sfile2.eof();
}

/**
 * @brief Import the content of a shape file (STEP) into a TopDS_Shape object
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int importShape(const std::string shape_file, TopoDS_Shape& aShape){

  std::cout << "Importing shape from " << shape_file << std::endl;
  STEPControl_Reader reader;
  // Forcing Unit in meter
  Interface_Static::SetCVal("xstep.cascade.unit","M");
  Interface_Static::SetIVal("read.step.ideas", 1);
  Interface_Static::SetIVal("read.step.nonmanifold", 1);
  IFSelect_ReturnStatus aStat = reader.ReadFile(shape_file.c_str());
  if(aStat != IFSelect_RetDone)
    std::cout << "Reading error for "  << shape_file << std::endl;

  int NbTrans = reader.TransferRoots();
  // There should be only one shape within the file
  assert(NbTrans==1);
  aShape = reader.OneShape();

  return true;
}

/**
 * @brief Export the content of a TopoDS_Shape into a shape file (STEP)
 *
 * @param shape_file the shape file
 * @param aShape the object
 *
 * @return error code
 */
int exportShape(const std::string shape_file, const TopoDS_Shape& aShape){

  std::cout << "Exporting shape to " << shape_file << std::endl;

  STEPControl_Writer aWriter;
  // Forcing Unit in meter
  Interface_Static::SetCVal("xstep.cascade.unit","M");
  Interface_Static::SetCVal("write.step.unit","M");
  Interface_Static::SetIVal("write.step.nonmanifold", 1);

  IFSelect_ReturnStatus aStat = aWriter.Transfer(aShape,STEPControl_AsIs);
  if(aStat != IFSelect_RetDone)
    std::cout << "Transfer error for "  << shape_file << std::endl;

  aStat = aWriter.Write(shape_file.c_str());

  if(aStat != IFSelect_RetDone)
    std::cout << "Writing error for "  << shape_file << std::endl;

  return aStat;
}
