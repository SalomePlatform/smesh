// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File      : SMESHUtils_MGLicenseKeyGen.cxx
// Created   : Sat Jul 31 18:54:16 2021
// Author    : Edward AGAPOV (OCC)

#include "SMESH_MGLicenseKeyGen.hxx"

#include "SMESH_Comment.hxx"
#include "SMESH_File.hxx"
#include "SMESH_TryCatch.hxx"

#include <Basics_DirUtils.hxx>
#include <Basics_Utils.hxx>

#include <regex>
#include <cstdlib> // getenv, system

#include <boost/filesystem.hpp>
namespace boofs = boost::filesystem;

#ifdef WIN32

#  include <windows.h>
#  include <process.h>

#  define LibHandle HMODULE
#  define LoadLib( name ) LoadLibrary( name )
#  define GetProc GetProcAddress
#  define UnLoadLib( handle ) FreeLibrary( handle );

#else // WIN32

#  include <dlfcn.h>

#  define LibHandle void*
#  define LoadLib( name ) dlopen( name, RTLD_LAZY | RTLD_LOCAL )
#  define GetProc dlsym
#  define UnLoadLib( handle ) dlclose( handle );

#endif // WIN32

// to retrieve description of exception caught by SMESH_TRY
#undef SMESH_CAUGHT
#define SMESH_CAUGHT error =


namespace
{
  static LibHandle theLibraryHandle = nullptr; //!< handle of a loaded library

  const char* theEnvVar = "SALOME_MG_KEYGEN_LIB_PATH"; /* var specifies either full file name
                                                          of libSalomeMeshGemsKeyGenerator or
                                                          URL to download the library from */

  const char* theTmpEnvVar = "SALOME_TMP_DIR"; // directory to download the library to

  //-----------------------------------------------------------------------------------
  /*!
   * \brief Remove library file at destruction in case if it was downloaded from server
   */
  //-----------------------------------------------------------------------------------

  struct LibraryFile
  {
    std::string _name; // full file name
    bool        _isURL;

    LibraryFile(): _isURL( false ) {}

    ~LibraryFile()
    {
      if ( _isURL )
      {
        if ( theLibraryHandle )
        {
          UnLoadLib( theLibraryHandle );
          theLibraryHandle = nullptr;
        }

        std::string tmpDir; // tmp dir that should not be removed
        if ( const char* libPath = getenv( theTmpEnvVar ))
        {
          tmpDir = libPath;
          while (( !tmpDir.empty() ) &&
                 ( tmpDir.back() == '/' || tmpDir.back() == '\\' ))
            tmpDir.pop_back();
        }

        while ( SMESH_File( _name ).remove() )
        {
          size_t length = _name.size();
          _name = boofs::path( _name ).parent_path().string(); // goto parent directory
          if ( _name.size() == length )
            break; // no more parents

          if ( _name == tmpDir )
            break; // don't remove tmp dir

          if ( !Kernel_Utils::IsEmptyDir( _name ))
            break;
        }
      }
    }
  };


  //================================================================================
  /*!
   * \brief Retrieve description of the last error
   *  \param [out] error - return the description
   *  \return bool - true if the description found
   */
  //================================================================================

  bool getLastError( std::string& error )
  {
#ifndef WIN32

    if ( const char* text = dlerror() )
    {
      error = text;
      return true;
    }
    return false;

#else

    DWORD dw = GetLastError();
    void* cstr;
    DWORD msgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 dw,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                 (LPTSTR) &cstr,
                                 0,
                                 NULL
                                 );
    if ( msgLen > 0 ) {
#  if defined( UNICODE )
      error = Kernel_Utils::encode_s((wchar_t*)cstr);
#  else
      error = (char*)cstr;
#  endif
      LocalFree(cstr);
    }

    return (bool)msgLen;

#endif
  }

  //================================================================================
  /*!
   * \brief Adjust file extension according to the platform
   */
  //================================================================================

  bool setExtension( std::string& fileName, std::string& error )
  {
    if ( fileName.empty() )
    {
      error = "Library file name is empty";
      return false;
    }
#if defined(WIN32)
    std::string ext = ".dll";
#elif defined(__APPLE__)
    std::string ext = ".dylib";
#else
    std::string ext = ".so";
#endif

    fileName = fileName.substr( 0, fileName.find_last_of('.')) + ext;
    return true;
  }

  //================================================================================
  /*!
   * \brief Check if library file name looks like an URL
   *  \param [in,out] libraryFile - holds file name and returns result in _isURL member field
   *  \return bool - true if the file name looks like an URL
   */
  //================================================================================

  bool isURL( LibraryFile & libraryFile )
  {
    enum { SCHEME = 2, AUTHORITY = 4, PATH = 5 }; // sub-strings
    std::regex urlRegex ( R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
                          std::regex::extended );
    std::smatch matchResult;

    libraryFile._isURL = false;
    if ( std::regex_match( libraryFile._name, matchResult, urlRegex ))
      libraryFile._isURL = ( !matchResult.str( SCHEME    ).empty() &&
                             !matchResult.str( AUTHORITY ).empty() &&
                             !matchResult.str( PATH      ).empty() );

    return libraryFile._isURL;
  }

  //================================================================================
  /*!
   * \brief Download libraryFile._name URL to SALOME_TMP_DIR
   *  \param [in,out] libraryFile - holds the URL and returns name of a downloaded file
   *  \param [out] error - return error description
   *  \return bool - is a success
   */
  //================================================================================

  bool downloadLib( LibraryFile& libraryFile, std::string & error )
  {
    // check if can write into SALOME_TMP_DIR

    std::string tmpDir = Kernel_Utils::GetTmpDirByEnv( theTmpEnvVar );
    if ( tmpDir.empty() ||
         !Kernel_Utils::IsExists( tmpDir ))
    {
      error = "Can't download " + libraryFile._name + " as SALOME_TMP_DIR is not correctly set";
      return false;
    }
    if ( !Kernel_Utils::IsWritable( tmpDir ))
    {
      error = "Can't download " + libraryFile._name + " as '" + tmpDir + "' is not writable. "
        "Check SALOME_TMP_DIR environment variable";
      return false;
    }

    // Download

    std::string url = libraryFile._name;

#ifdef WIN32

    std::string outFile = tmpDir + "MeshGemsKeyGenerator.dll";

    // use wget (== Invoke-WebRequest) PowerShell command available since Windows 7
    std::string psCmd = "wget -Uri " + url + " -OutFile " + outFile;
    std::string   cmd = "powershell.exe " + psCmd;

#else

    std::string outFile = tmpDir + "libMeshGemsKeyGenerator.so";

    std::string cmd = "wget " + url + " -O " + outFile;

#endif

    if ( Kernel_Utils::IsExists( outFile )) // remove existing file
    {
      SMESH_File lib( outFile, /*open=*/false );
      if ( !lib.remove() )
      {
        error = lib.error();
        return false;
      }
    }

    system( cmd.c_str() ); // download

    SMESH_File resultFile( outFile, /*open=*/false );
    bool ok = ( resultFile.exists() && resultFile.size() > 0 );

    if ( ok )
      libraryFile._name = outFile;
    else
      error = "Can't download file " + url;

    return ok;
  }

  //================================================================================
  /*!
   * \brief Load libMeshGemsKeyGenerator.so
   *  \param [out] error - return error description
   *  \param [out] libraryFile - return library file name and _isURL flag
   *  \return bool - is a success
   */
  //================================================================================

  bool loadLibrary( std::string& error, LibraryFile& libraryFile )
  {
    if ( theLibraryHandle )
      return true;

    const char* libPath = getenv( theEnvVar );
    if ( !libPath )
    {
      error = SMESH_Comment( "Environment variable ") <<  theEnvVar << " is not set";
      return false;
    }

    libraryFile._name = libPath;
    // if ( !setExtension( libraryFile._name, error )) // is it necessary?
    //   return false;

    if ( isURL( libraryFile ))
    {
      if ( !downloadLib( libraryFile, error ))
      {
        // try to fix extension
        std::string url = libraryFile._name;
        if ( !setExtension( libraryFile._name, error ))
          return false;
        if ( url == libraryFile._name )
          return false; // extension not changed

        if ( !downloadLib( libraryFile, error ))
          return false;
      }
    }

#if defined( WIN32 ) && defined( UNICODE )
    std::wstring encodePath = Kernel_Utils::utf8_decode_s( libraryFile._name );
    const wchar_t*     path = encodePath.c_str();
#else
    const char*        path = libraryFile._name.c_str();
#endif

    theLibraryHandle = LoadLib( path );
    if ( !theLibraryHandle )
    {
      if ( ! getLastError( error ))
        error = "Can't load library '" + libraryFile._name + "'";
    }

    return theLibraryHandle;
  }

} // anonymous namespace


namespace SMESHUtils_MGLicenseKeyGen // API implementation
{
  //================================================================================
  /*!
   * \brief Sign a CAD
   *  \param [in] meshgems_cad - pointer to a MG CAD object (meshgems_cad_t)
   *  \param [out] error - return error description
   *  \return bool - is a success
   */
  //================================================================================

  bool SignCAD( void* meshgems_cad, std::string& error )
  {
    LibraryFile libraryFile;
    if ( !loadLibrary( error, libraryFile ))
      return false;

    bool ok = false;
    typedef bool (*SignFun)(void* );
    SignFun signFun = (SignFun) GetProc( theLibraryHandle, "SignCAD" );
    if ( !signFun )
    {
      if ( ! getLastError( error ))
        error = SMESH_Comment( "Can't find symbol 'SignCAD' in '") << getenv( theEnvVar ) << "'";
    }
    else
    {

      SMESH_TRY;

      ok = signFun( meshgems_cad );

      SMESH_CATCH( SMESH::returnError );

      if ( !error.empty() )
        ok = false;
      else if ( !ok )
        error = "SignCAD() failed (located in '" + libraryFile._name + "')";
    }
    return ok;
  }

  //================================================================================
  /*!
   * \brief Sign a mesh
   *  \param [in] meshgems_mesh - pointer to a MG mesh (meshgems_mesh_t)
   *  \param [out] error - return error description
   *  \return bool - is a success
   */
  //================================================================================

  bool SignMesh( void* meshgems_mesh, std::string& error )
  {
    LibraryFile libraryFile;
    if ( !loadLibrary( error, libraryFile ))
      return false;

    typedef bool (*SignFun)(void* );
    SignFun signFun = (SignFun) GetProc( theLibraryHandle, "SignMesh" );
    if ( !signFun )
    {
      if ( ! getLastError( error ))
        error = SMESH_Comment( "Can't find symbol 'SignMesh' in '") << getenv( theEnvVar ) << "'";
    }
    bool ok;

    SMESH_TRY;

    ok = signFun( meshgems_mesh );

    SMESH_CATCH( SMESH::returnError );

    if ( !error.empty() )
      ok = false;
    else if ( !ok )
      error = "SignMesh() failed (located in '" + libraryFile._name + "')";

    return ok;
  }

  //================================================================================
  /*!
   * \brief Return a license key to pass as argument to a MG mesher executable
   *  \param [in] gmfFile - path to an input mesh file
   *  \param [in] nb* - nb of entities in the input mesh
   *  \param [out] error - return error description
   *  \return std::string - the key
   */
  //================================================================================

  std::string GetKey(const std::string& gmfFile,
                     int                nbVertex,
                     int                nbEdge,
                     int                nbFace,
                     int                nbVol,
                     std::string&       error)
  {
    std::string key;
    LibraryFile libraryFile;
    if ( !loadLibrary( error, libraryFile ))
      return key;

    typedef std::string (*GetKeyFun)(std::string const &, int, int, int, int );
    GetKeyFun keyFun = (GetKeyFun) GetProc( theLibraryHandle, "GetKey" );
    if ( !keyFun )
    {
      if ( ! getLastError( error ))
        error = SMESH_Comment( "Can't find symbol 'GetKey' in '") << getenv( theEnvVar ) << "'";
    }
    key = keyFun( gmfFile, nbVertex, nbEdge, nbFace, nbVol );

    if ( key.empty() )
      error = "GetKey() failed (located in '" + libraryFile._name + "')";

    return key;
  }

  //================================================================================
  /*!
   * \brief Return false if libMeshGemsKeyGenerator.so is not functional
   *  \param [out] error - return error description
   *  \return bool - is a success
   */
  //================================================================================

  bool CheckKeyGenLibrary( std::string& error )
  {
    return !GetKey("",4,0,2,0,error ).empty();
  }

  //================================================================================
  /*!
   * \brief Return KeyGenerator library name
   */
  //================================================================================

  std::string GetLibraryName()
  {
    std::string libName, error;
    if ( const char* libPath = getenv( theEnvVar ))
    {
      libName = Kernel_Utils::GetBaseName( libPath );
    }
    else
    {
      libName = "libSalomeMeshGemsKeyGenerator";
    }
    setExtension( libName, error );
    return libName;
  }
}
