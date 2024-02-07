// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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

// SMESH SMESH_I : support of import / export with meshio library
// File   : SMESH_Meshio.h
// Author : Konstantin Leontev, Open CASCADE S.A.S.
//

#include "SMESH_Meshio.h"

// SALOME KERNEL includes
#include <SALOMEDS_Tool.hxx>
#include <Basics_DirUtils.hxx>
#include <SALOME_Exception.hh>
#include <utilities.h>

// Qt
#include <QFileInfo>

#include <fstream>


/*!
  Constructor
*/
SMESH_Meshio::SMESH_Meshio()
{
  CreateErrorFileName();
}

/*!
  Constructor
*/
SMESH_Meshio::SMESH_Meshio(const QString& selectedFilter)
: SMESH_Meshio()
{
  mySelectedFilter = selectedFilter;
}

/*!
  Destructor
*/
SMESH_Meshio::~SMESH_Meshio()
{
  RemoveTempFile();
  RemoveErrorFile();
}

/*!
  Convert file with meshio convert command
*/
void SMESH_Meshio::Convert(const QString& sourceFileName, const QString& targetFileName) const
{
  // Execute meshio convert command
  const QString convert = IsModernPythonVersion() ? "meshio convert " : "meshio-convert ";
  const QString optArgs = GetConvertOptArgs();

  const std::string cmd = 
    (convert + optArgs + " " + sourceFileName + " " + targetFileName + " 2> " + myErrorFileName).toStdString();
  MESSAGE("Call system(\"" << cmd << "\") ...");

  const int status = system(cmd.c_str());
  MESSAGE("status: " << status);

  // Check results
  if (status != 0 || !Kernel_Utils::IsExists(targetFileName.toStdString()))
  {
    // Get all the output from an error file
    const std::string meshioErrors = ReadErrorsFromFile();
    MESSAGE("meshioErrors: \n" << meshioErrors);

    SALOME::ExceptionStruct es;
    es.type = SALOME::ExceptionType::BAD_PARAM;
    es.lineNumber = -1;
    es.text = CORBA::string_dup(
      ("MESHIO\nFailed system(\"" + cmd + "\").\n\n" + meshioErrors + "\nOperation canceled.").c_str());

    throw SALOME::SALOME_Exception(es);
  }
}

/*!
  Returns a name for a file in default temp directory
  based on a target basename.
*/
QString SMESH_Meshio::CreateTempFileName(const QString& targetFileName)
{
  // Make a name for a temp file
  const QFileInfo fileInfo(targetFileName);
  const QString targetBaseName = fileInfo.baseName();
  const QString dirName(SALOMEDS_Tool::GetTmpDir().c_str());
  myTempFileName = dirName + targetBaseName + ".med";

  MESSAGE("targetFileName: " << targetFileName.toStdString());
  MESSAGE("myTempFileName: " << myTempFileName.toStdString());

  return myTempFileName;
}

/*!
  Removes the temporary file if we have any
*/
void SMESH_Meshio::RemoveTempFile()
{
  if (!myTempFileName.isEmpty())
  {
    std::remove(myTempFileName.toStdString().c_str());
    MESSAGE("Temp file " << myTempFileName.toStdString() << " was removed");
  }
}

/*!
  Returns true if current Python equal or newer than required version for
  meshio release from 5.0 and greater.
*/
bool SMESH_Meshio::IsModernPythonVersion()
{
  // It's a version when meshio commands were changed from using
  // many executables for each operation to one for everything (meshio 5.0).
  // For example, from
  // meshio-convert input.msh output.vtk
  // to
  // meshio convert input.msh output.vtk
 
  const std::vector<int> minReqVersionNums = { 3, 7 };

  auto GetCurrentVersion = []() -> QString
  {
    const char *envVar = std::getenv("PYTHON_VERSION");
    if (envVar && (envVar[0] != '\0'))
    {
      return envVar;
    }

    return {};
  };

  auto IsModernVersion = [&]() -> bool
  {
    const QString curVersion = GetCurrentVersion();
    MESSAGE("Current Python version: " << curVersion.toStdString())
    if (curVersion.length() < 3)
      return false;

    int index = 0;
    const QStringList curVersionNums = curVersion.split('.');
    for (const int minVersion : minReqVersionNums)
    {
      if (minVersion > curVersionNums[index].toInt())
      {
        return false;
      }

      ++index;
    }

    return true;
  };

  static const bool isModern = IsModernVersion();
  return isModern;
}

/*!
  Creates a temp file for error output from meshio command.
*/
void SMESH_Meshio::CreateErrorFileName()
{
  // Make a name for a temp file
  const QString fileName = "meshio_errors.txt";
  const QString dirName(SALOMEDS_Tool::GetTmpDir().c_str());
  myErrorFileName = dirName + fileName;
  
  MESSAGE("myErrorFileName: " << myErrorFileName.toStdString());
}

/*!
  Removes the temporary error output file if we have any
*/
void SMESH_Meshio::RemoveErrorFile()
{
  if (!myErrorFileName.isEmpty())
  {
    std::remove(myErrorFileName.toStdString().c_str());
    MESSAGE("Temp file " << myErrorFileName.toStdString() << " was removed");
  }
}

/*!
  Read all errors output from a temp file
*/
std::string SMESH_Meshio::ReadErrorsFromFile() const
{
  if (myErrorFileName.isEmpty())
    return {};

  std::ifstream fileStream(myErrorFileName.toStdString());
  if(fileStream)
  {
    std::ostringstream ss;
    ss << fileStream.rdbuf();

    return ss.str();
  }

  return {};
}

/*!
  Get optional arguments for meshio convert command
*/
QString SMESH_Meshio::GetConvertOptArgs() const
{
  if (mySelectedFilter.isEmpty())
    return mySelectedFilter;

  // Check what kind of option we must provide
  if (mySelectedFilter.startsWith("ANSYS"))
    return "-o ansys";
  else if (mySelectedFilter.startsWith("Gmsh 2"))
    return "-o gmsh22";
  else if (mySelectedFilter.startsWith("Gmsh 4"))
    return "-o gmsh";

  return {};
}
