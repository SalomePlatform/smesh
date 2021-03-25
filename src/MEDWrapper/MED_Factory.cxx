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

#include "MED_Factory.hxx"
#include "MED_Utilities.hxx"
#include "MED_Wrapper.hxx"

#include <Basics_Utils.hxx>

#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <fstream>

#include <med.h>
extern "C"
{
#ifndef WIN32
  #include <unistd.h>
#endif
}
#include <utilities.h>

#ifdef WIN32
#include <windows.h>
#endif

// -------------------------------------------------------------------------------------------------------------------
// --- MED file compatibility: write using a lower major version implies append on an empty file of the target version
//
// *******************************************************************************************************************
// ==> This file must be modified when MED version changes:
// MED_MAJOR_NUM and MED_MINOR_NUM are defined in an external include from MED prerequisite: med.h
// When MED_MAJOR_NUM or MED_MINOR_NUM changes, MED_MAJOR_EXPECTED and MED_MINOR_EXPECTED must be set accordingly
// and MED_VERSIONS_APPEND_COMPATIBLE must be updated: The lower compatible versions may change with a new MED version
// If the major version of MED change (for instance 4 --> 5) and if MED allows to append meshes in MED-4.x format,
// Empty file content for MED-4.x should be provided (EMPTY_FILE_4x) and method CreateEmptyMEDFile should be updated.
// *******************************************************************************************************************

#define MED_MAJOR_EXPECTED 4
#define MED_MINOR_EXPECTED 1
#if MED_MAJOR_NUM != MED_MAJOR_EXPECTED
  #error "MED major version does not correspond to the expected version, fix the minor and major compatibility values in CheckCompatibility method (MED_VERSIONS_APPEND_COMPATIBLE) and set the correct expected version (MED_MAJOR_EXPECTED, MED_MINOR_EXPECTED)"
#endif
#if MED_MINOR_NUM != MED_MINOR_EXPECTED
  #error "MED minor version does not correspond to the expected version, fix the minor and major compatibility values in CheckCompatibility method (MED_VERSIONS_APPEND_COMPATIBLE) and set the correct expected version above (MED_MAJOR_EXPECTED, MED_MINOR_EXPECTED)"
#endif
#define MED_VERSIONS_APPEND_COMPATIBLE {41, 40, 32, 33} // --- 10*major + minor (the 3rd digit, release, is not used here,
                                                        //                       med uses always the latest available)
                                                        // --- The first in the list should be the default: current version

namespace MED
{
  bool exists(const std::string& fileName)
  {
#ifdef WIN32
#ifdef UNICODE
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), strlen(fileName.c_str()), NULL, 0);
    wchar_t* path = new wchar_t[size_needed + 1];
    MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), strlen(fileName.c_str()), path, size_needed);
    path[size_needed] = '\0';
#else
    cosnt char* path = xmlPath.c_str();
#endif
    bool res = (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
#ifdef UNICODE
    delete path;
#endif
    return res;
#else
    return (access(fileName.c_str(), F_OK) == 0);
#endif
  }

  /*!
   *  Return the list of med versions compatibles for write/append,
   *  encoded in 10*major+minor (for instance, code for med 3.2.1 is 32)
   */
  std::vector<int> GetMEDVersionsAppendCompatible()
  {
    int mvok[] = MED_VERSIONS_APPEND_COMPATIBLE;
    std::vector<int> MEDVersionsOK(mvok, mvok + sizeof(mvok)/sizeof(int));
    int curVersion = MED_MAJOR_NUM * 10 + MED_MINOR_NUM;
    if ( MEDVersionsOK[0] != curVersion )
      MEDVersionsOK.insert( MEDVersionsOK.begin(), curVersion );
    return MEDVersionsOK;
  }

  /*!
   *  \brief: Check read or write(append) Compatibility of a med file
   *  \param [in] : fileName - the file to read or to append to
   *  \param [in] : isforAppend - when true, check if the med file version is OK to append a mesh,
   *                              when false, check if the med file is readable.
   */
  bool CheckCompatibility(const std::string& fileName, bool isForAppend)
  {
    bool ok = false;
    std::vector<int> medVersionsOK = GetMEDVersionsAppendCompatible();
    // check that file is accessible
    if ( exists(fileName) ) {
      // check HDF5 && MED compatibility
      med_bool hdfok, medok;
      med_err r0 = MEDfileCompatibility(fileName.c_str(), &hdfok, &medok);
      MESSAGE(r0 << " " << hdfok << " " << medok);
      if ( r0==0 && hdfok && medok ) {
        med_idt aFid = MEDfileOpen(fileName.c_str(), MED_ACC_RDONLY);
        if (aFid >= 0) {
          med_int major, minor, release;
          med_err ret = MEDfileNumVersionRd(aFid, &major, &minor, &release);
          MESSAGE(ret << " " << major << "." << minor << "." << release);
          if (ret >= 0) {
            bool isReadOnly = !isForAppend;
            if (isReadOnly)
              ok = true;
            else {
              int medVersion = 10*major + minor;
              for (size_t ii=0; ii < medVersionsOK.size(); ii++)
                if (medVersionsOK[ii] == medVersion) {
                  ok =true;
                  break;
                }
            }
          }
        }
        MEDfileClose(aFid);
      }
    }
    return ok;
  }

  bool GetMEDVersion(const std::string& fileName, int& major, int& minor, int& release)
  {
    bool ok = false;
    major = minor = release = 0;
    med_idt aFid = MEDfileOpen(fileName.c_str(), MED_ACC_RDONLY);
    if (aFid >= 0) {
      med_int _major, _minor, _release;
      med_err ret = MEDfileNumVersionRd(aFid, &_major, &_minor, &_release);
      if (ret == 0) {
        major = _major;
        minor = _minor;
        release = _release;
        ok = true;
      }
      MEDfileClose(aFid);
    }
    return ok;
  }

  std::string GetMEDVersion(const std::string& fileName)
  {
    std::string version;
    int major, minor, release;
    if (GetMEDVersion(fileName, major, minor, release)) {
      std::ostringstream os;
      os << major << "." << minor << "." << release;
      version = os.str();
    }
    return version;
  }

  PWrapper CrWrapperR(const std::string& fileName)
  {
    if (!CheckCompatibility(fileName)) {
      EXCEPTION(std::runtime_error, "Cannot open file '"<<fileName<<"'.");
    }
    return new MED::TWrapper(fileName, false);
  }

  PWrapper CrWrapperW(const std::string& fileName, int theVersion)
  {
    bool isCreated = false;
    if (!CheckCompatibility(fileName, true))
    {
      remove(fileName.c_str());
      isCreated = true;
    }
    med_int wantedMajor = MED_MAJOR_NUM;
    med_int wantedMinor = MED_MINOR_NUM;
    // when non managed version of file is requested : ignore it and take the latest version
    std::vector<int> versionsOK(GetMEDVersionsAppendCompatible());
    bool isVersionRequestedOK(std::find(versionsOK.begin(),versionsOK.end(),theVersion)!=versionsOK.end());
    if (isCreated && isVersionRequestedOK)
    {
      wantedMajor = theVersion/10;
      wantedMinor = theVersion%10;
    }
    return new MED::TWrapper(fileName, true, wantedMajor, wantedMinor);
  }
}
