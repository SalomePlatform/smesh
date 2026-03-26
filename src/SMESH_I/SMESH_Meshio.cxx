// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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
#include <QDir>

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
void SMESH_Meshio::Convert(const QString& sourceFileName, const QString& targetFileName, bool isImport, const QString& converter) const
{ 
  auto appendIfNotEmpty = [&](QStringList &parts, const QString &opt, const QString &val) {
      if (!opt.isEmpty() && !val.isEmpty())
          parts << opt << val;
  };
  // Execute meshio convert command
  SMESHIOConverter::ExternalConverter externalConverter = SMESHIOConverter::ExternalConverter::Unknown;
  QString cmdExecutable;
  QString cmdConvertOpt = "";
  QString cmdInputOpt   = "";
  QString cmdOutputOpt  = "";     // for output format
  QString cmdOutputFileOpt = "";  // for output file name
  QString cmdSaveOpts   = "";
  QString cmdOpts   = "";
  QString inputFmt  = "";
  QString outputFmt = "";

  if (isImport) {
      externalConverter = SMESH_Meshio::GetConverterForExtension(mySelectedFilter, sourceFileName, converter);
  } else {
      externalConverter = SMESH_Meshio::GetConverterForExtension(mySelectedFilter, targetFileName, converter);
  }

  
  switch (externalConverter)
  {
    case SMESHIOConverter::ExternalConverter::Gmsh:
      cmdExecutable = "gmsh";
      cmdOutputOpt = "-format";
      cmdOutputFileOpt  = "-o";
      cmdOpts = "-save_all";
      cmdSaveOpts = "-save";
      if (mySelectedFilter.contains("Gmsh", Qt::CaseInsensitive))
      {
        if (mySelectedFilter.contains("binary", Qt::CaseInsensitive))
          cmdOutputOpt.prepend("-bin "); // for Gmsh binary format
        
        if (mySelectedFilter.contains("2.2", Qt::CaseInsensitive))
          cmdOpts = "";
      }
      break;

    case SMESHIOConverter::ExternalConverter::MeshIo:
      cmdExecutable = IsModernMeshioVersion() ? "meshio convert" : "meshio-convert";
      cmdInputOpt = "--input-format";
      cmdOutputOpt = "--output-format";
      break;

    case SMESHIOConverter::ExternalConverter::Unknown:
    default:
      MESSAGE("Unknown library for conversion");
      return;
  }

  // Decide input/output formats based on import/export
  if (isImport) {
      // Import case -> output format is always "med"
      inputFmt = GetConvertOptArgs(externalConverter, sourceFileName); // detect actual input format
      outputFmt = "med";
  } else {
      // Export case -> input format is always "med"
      inputFmt = "med";
      outputFmt = GetConvertOptArgs(externalConverter, targetFileName); // detect actual output format
  }

  // Build command parts
  QStringList cmdParts;
  cmdParts << cmdExecutable;

  // Add convert option right after executable (if any)
  if (!cmdConvertOpt.isEmpty())
      cmdParts << cmdConvertOpt;

  // Input option + format
  appendIfNotEmpty(cmdParts, cmdInputOpt, inputFmt);

  // Source file
  cmdParts << sourceFileName;

  // Output option + format
  appendIfNotEmpty(cmdParts, cmdOutputOpt, outputFmt);

  // Output file option + target file
  if (!cmdOutputFileOpt.isEmpty())
      cmdParts << cmdOutputFileOpt << targetFileName;
  else
      cmdParts << targetFileName;

  // Additional options 
  if (!cmdOpts.isEmpty())
      cmdParts << cmdOpts;

  // Save options (gmsh only)
  if (!cmdSaveOpts.isEmpty())
      cmdParts << cmdSaveOpts;

  // Redirect errors
  cmdParts << "2>" << myErrorFileName;

  const std::string cmd = cmdParts.join(" ").toStdString();
  MESSAGE("Call system(\"" << cmd << "\") ...");

  const int status = system(cmd.c_str());
  MESSAGE("status: " << status);

  // Check results
  if (status != 0 || !Kernel_Utils::IsExists(targetFileName.toStdString()))
  {
    // Get all the output from an error file
    const std::string meshioErrors = ReadErrorsFromFile();
    MESSAGE("External Converter Errors: \n" << meshioErrors);

    SALOME_CMOD::ExceptionStruct es;
    es.type = SALOME_CMOD::ExceptionType::BAD_PARAM;
    es.lineNumber = -1;
    es.text = CORBA::string_dup(
      ("External Converter\nFailed system(\"" + cmd + "\").\n\n" + meshioErrors + "\nOperation canceled.").c_str());

    throw SALOME_CMOD::SALOME_Exception(es);
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
    const QFileInfo fileInfo{myTempFileName};
    const QString path{fileInfo.path()};
    QDir dir{path};
    dir.remove(myTempFileName);
    dir.rmdir(path);
    MESSAGE("Temp file " << myTempFileName.toStdString() << " was removed");
  }
}

/*!
  Returns meshio version string that has valid integer at least in the first position.
*/
QString SMESH_Meshio::GetMeshioVersion()
{
  auto IsVersionStringValid = [](const QString& version) -> bool
  {
    if (version.isEmpty())
    {
      return false;
    }

    // Check if we have an integer at least at the first position
    const QStringList curVersionNums = version.split('.');

    bool ok;
    const int firstNum = curVersionNums[0].toInt(&ok);
    if (!ok)
    {
      ERROR_MESSAGE("meshio version value is not valid!");
      return false;
    }

    MESSAGE("meshio version first number: " << firstNum);
    return true;
  };

  auto GetMeshioVersionFromEnv = []() -> QString
  {
    const char *envVar = std::getenv("MESHIO_VERSION");
    if (envVar && (envVar[0] != '\0'))
    {
      MESSAGE("MESHIO_VERSION: " << envVar);
      return envVar;
    }

    MESSAGE("MESHIO_VERSION is not set!");
    return {};
  };

  auto GetMeshioVersionHelper = [&]() -> QString
  {
    // Check if we can get a version from environment
    const QString meshioVersionEnv = GetMeshioVersionFromEnv();
    if (IsVersionStringValid(meshioVersionEnv))
    {
      return meshioVersionEnv;
    }

    // Try to guess a version by installed Python version
    const QString meshioVersionByPython = IsModernPythonVersion() ? "5" : "4";
    MESSAGE("meshio version was defined by Python version: " << meshioVersionByPython.toStdString());

    return meshioVersionByPython;
  };

  static const QString meshioVersion = GetMeshioVersionHelper();
  return meshioVersion;
};

/*!
  Returns true if we're going to use meshio version of 5.0 or greater.
*/
bool SMESH_Meshio::IsModernMeshioVersion()
{
  // It's a version when meshio commands were changed from using
  // many executables for each operation to one for everything (meshio 5.0).
  // For example, from
  // meshio-convert input.msh output.vtk
  // to
  // meshio convert input.msh output.vtk

  auto IsModernVersion = [&]() -> bool
  {
    const QString curVersion = GetMeshioVersion();

    const int minReqVersion = 5;
    const QStringList curVersionNums = curVersion.split('.');
    if (minReqVersion > curVersionNums[0].toInt())
    {
      return false;
    }

    return true;
  };

  static const bool isModern = IsModernVersion();
  return isModern;
}

/*!
  Returns true if meshio is installed
*/
bool SMESH_Meshio::IsMeshioInstalled()
{
  auto IsAllowedFromEnvironment = []() -> bool
  {
    const QString curVersion = GetMeshioVersion();

    // Check if we explicitly set off using of meshio from environment
    const QStringList curVersionNums = curVersion.split('.');
    const int firstNum = curVersionNums[0].toInt();
    if (firstNum <= 0)
    {
      MESSAGE("meshio was set as not installed from an environment");
      return false;
    }

    return true;
  };

  auto IsAbleToCallMeshio = []() -> bool
  {
    // Try to call meshio to check if it's present
    const std::string cmd =
      SMESH_Meshio::IsModernMeshioVersion() ? "meshio --version" : "meshio-info --version";

    const int status = system(cmd.c_str());
    MESSAGE("status: " << status);

    return status == 0;
  };

  static const bool isInstalled = IsAllowedFromEnvironment() && IsAbleToCallMeshio();
  return isInstalled;
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
    const QFileInfo fileInfo{myErrorFileName};
    const QString path{fileInfo.path()};
    QDir dir{path};
    dir.remove(myErrorFileName);
    dir.rmdir(path);
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
QString SMESH_Meshio::GetConvertOptArgs(SMESHIOConverter::ExternalConverter externalConverter, const QString& fileName) const
{
    SMESHIOConverter::Extension fmt = SMESHIOConverter::Extension::Unknown;
    if (!mySelectedFilter.isEmpty())
    {
      fmt = SMESHIOConverter::fromLabel(mySelectedFilter);
    }

    if (fmt == SMESHIOConverter::Extension::Unknown && !fileName.isEmpty())
    {
      const QString ext = QFileInfo(fileName).completeSuffix().toLower();
      for (const auto& [f, info] : SMESHIOConverter::ExtensionMap)
      {
        if (info.extension.compare(ext, Qt::CaseInsensitive) == 0)
        {
          fmt = f;
          break;
        }
      }
    }

    if (fmt == SMESHIOConverter::Extension::Unknown)
    {
      return "";
    }
    
    // Chercher les infos du format
    auto itFmt = SMESHIOConverter::ExtensionMap.find(fmt);
    if (itFmt == SMESHIOConverter::ExtensionMap.end())
        return "";

    const auto& info = itFmt->second;

    // Find the converter-specific option string
    auto convIt = info.converters.find(externalConverter);
    if (convIt != info.converters.end())
        return convIt->second;

    return "";
}



/*!
  find library for extension
*/
SMESHIOConverter::ExternalConverter SMESH_Meshio::GetConverterForExtension(const QString& selectedFilter, const QString& sourceFileName, const QString& converter)
{
    if (!converter.isEmpty())
    {
      if (converter.compare("gmsh", Qt::CaseInsensitive) == 0)
        return SMESHIOConverter::ExternalConverter::Gmsh;
      else if (converter.compare("meshio", Qt::CaseInsensitive) == 0)
        return SMESHIOConverter::ExternalConverter::MeshIo;
      else
        return SMESHIOConverter::ExternalConverter::Unknown;
    }

    if (!selectedFilter.isEmpty())
    {
      for (const auto& [fmt, info] : SMESHIOConverter::ExtensionMap)
      {
        // Compare against the full label
        if (SMESHIOConverter::GetFilterLabel(info.label) == SMESHIOConverter::GetFilterLabel(selectedFilter))
        {
          // If multiple converters exist, return the first one
          if (!info.converters.empty())
            return info.converters.begin()->first;
        }
      }
    }

    if (!sourceFileName.isEmpty()){
      const QString ext = QFileInfo(sourceFileName).completeSuffix().toLower();
      for (const auto& [fmt, info] : SMESHIOConverter::ExtensionMap)
      {
        if (info.extension.compare(ext, Qt::CaseInsensitive) == 0)
        {
          // If multiple converters exist, return the first one
          if (!info.converters.empty())
            return info.converters.begin()->first;
        }
      }
    }
      
    return SMESHIOConverter::ExternalConverter::Unknown;
}

/*!
  Check if a given converter library is installed and allowed for use.
  
  This function performs two verifications:
   1. Environment check:
      - Reads the corresponding environment variable (e.g. GMSH_VERSION, MESHIO_VERSION).
      - If the variable is set to 0 or invalid, the converter is considered disabled.
   2. Executable check:
      - Attempts to start the corresponding executable with "--version" (or "meshio-info --version" for legacy MeshIo).
      - If the process starts and exits normally, the converter is considered available in PATH.

  A converter is considered installed only if both checks succeed.

  \param converter The converter to check (Gmsh, MeshIo).
  \return true if the converter is allowed by environment and its executable is available, false otherwise.
*/
bool SMESH_Meshio::IsConverterInstalled(SMESHIOConverter::ExternalConverter converter)
{
  auto IsAllowedFromEnvironment = [&](SMESHIOConverter::ExternalConverter conv) -> bool
  {
    const QString curVersion = GetConverterVersion(conv);

    // Check if we explicitly set off using of converter from environment
    const QStringList curVersionNums = curVersion.split('.');
    bool ok = false;
    int firstNum = curVersionNums[0].toInt(&ok);
    if (!ok || firstNum <= 0)
    {
      MESSAGE(SMESHIOConverter::toString(conv) << " was set as not installed from environment");
      return false;
    }

    return true;
  };

  auto checkExecutable = [](const std::string& program) -> bool {
    std::string cmd = program + " --version";
    int ret = std::system(cmd.c_str());
    MESSAGE("status: " << ret);
    return (ret == 0);
  };

  // Decide executable name depending on converter
  std::string program = SMESHIOConverter::toString(converter);

  // Special case: meshio old versions use meshio-info
  if (converter == SMESHIOConverter::ExternalConverter::MeshIo &&
      !SMESH_Meshio::IsModernMeshioVersion())
  {
    program = "meshio-info";
  }

  static const bool isInstalled =
      IsAllowedFromEnvironment(converter) && checkExecutable(program);

  return isInstalled;
}


/*!
  Returns lib version string that has valid integer at least in the first position.
*/
QString SMESH_Meshio::GetConverterVersion(SMESHIOConverter::ExternalConverter converter)
{
  auto IsVersionStringValid = [](const QString& version) -> bool
  {
    if (version.isEmpty())
    {
      return false;
    }

    // Check if we have an integer at least at the first position
    const QStringList curVersionNums = version.split('.');

    bool ok;
    const int firstNum = curVersionNums[0].toInt(&ok);
    if (!ok)
    {
      ERROR_MESSAGE("Converter version value is not valid!");
      return false;
    }

    MESSAGE("Converter version first number: " << firstNum);
    return true;
  };

  auto GetConverterVersionFromEnv = [&](SMESHIOConverter::ExternalConverter conv) -> QString
  {
    // Build env var name: uppercase + "_VERSION"
    std::string envName = SMESHIOConverter::toString(conv);
    std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
    envName += "_VERSION";

    // Read environment variable
    const char* envVar = std::getenv(envName.c_str());

    if (envVar && (envVar[0] != '\0'))
    {
      MESSAGE(envName << ": " << envVar);
      return envVar;
    }

    MESSAGE("MESHIO_VERSION is not set!");
    return {};
  };

  auto GetConverterVersionHelper = [&](SMESHIOConverter::ExternalConverter conv) -> QString
  {
    // Try environment variable first
    const QString versionEnv = GetConverterVersionFromEnv(conv);
    if (IsVersionStringValid(versionEnv))
      return versionEnv;

    // Special fallback for MeshIo: guess by Python version
    if (conv == SMESHIOConverter::ExternalConverter::MeshIo)
    {
      const QString meshioVersionByPython = IsModernPythonVersion() ? "5" : "4";
      MESSAGE("meshio version was defined by Python version: " << meshioVersionByPython.toStdString());
      return meshioVersionByPython;
    }

    // For other converters, just return "unknown"
    return "unknown";
  };

  static const QString converterVersion = GetConverterVersionHelper(converter);
  return converterVersion;
};

