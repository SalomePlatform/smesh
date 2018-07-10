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

#include "MED_Factory.hxx"
#include "MED_Utilities.hxx"
#include "MED_Wrapper.hxx"

#include <stdio.h>
#include <errno.h>
#include <sstream>

#include <med.h>
extern "C"
{
#ifndef WIN32
  #include <unistd.h>
#endif
}

namespace MED
{
  bool exists(const std::string& fileName)
  {
#ifdef WIN32
    return (GetFileAttributes(xmlPath.c_str()) != INVALID_FILE_ATTRIBUTES);
#else
    return (access(fileName.c_str(), F_OK) == 0);
#endif
  }

  bool CheckCompatibility(const std::string& fileName)
  {
    bool ok = false;
    // check that file is accessible
    if ( exists(fileName) ) {
      // check HDF5 && MED compatibility
      med_bool hdfok, medok;
      MEDfileCompatibility(fileName.c_str(), &hdfok, &medok);
      if ( hdfok && medok ) {
        med_idt aFid = MEDfileOpen(fileName.c_str(), MED_ACC_RDONLY);
        if (aFid >= 0) {
          med_int major, minor, release;
          med_err ret = MEDfileNumVersionRd(aFid, &major, &minor, &release);
          if (ret >= 0) {
            int version = 100*major + minor;
            if (version >= 202)
              ok = true;
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
    return new MED::TWrapper(fileName);
  }

  PWrapper CrWrapperW(const std::string& fileName)
  {
    if (!CheckCompatibility(fileName))
      remove(fileName.c_str());
    return new MED::TWrapper(fileName);
  }
}