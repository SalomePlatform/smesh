// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

#ifndef _SMESH_PYTHONDUMP_HXX_
#define _SMESH_PYTHONDUMP_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SALOMEDS)

#include <sstream>
#include <vector>

class SMESH_Gen_i;
class SMESH_MeshEditor_i;
class TCollection_AsciiString;
class Resource_DataMapOfAsciiStringAsciiString;

// ===========================================================================================
/*!
 * \brief Tool converting SMESH engine calls into commands defined in smeshDC.py
 *
 * Implementation is in SMESH_2smeshpy.cxx
 */
// ===========================================================================================

class SMESH_2smeshpy
{
public:
  /*!
   * \brief Convert a python script using commands of smesh.py
   * \param theScript - Input script
   * \param theEntry2AccessorMethod - The returning method names to access to
   *        objects wrapped with python class
   * \param theHistoricalDump - true means to keep all commands, false means
   *        to exclude commands relating to objects removed from study
   * \retval TCollection_AsciiString - Convertion result
   */
  static TCollection_AsciiString
  ConvertScript(const TCollection_AsciiString&            theScript,
                Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod,
                Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
                SALOMEDS::Study_ptr&                      theStudy,
                const bool                                theHistoricalDump);

  /*!
   * \brief Return the name of the python file wrapping IDL API
    * \retval const char* - the file name
   */
  static const char* SmeshpyName() { return "smesh"; }
  static const char* GenName() { return "smesh"; }
};

namespace SMESH
{
  class FilterLibrary_i;
  class FilterManager_i;
  class Filter_i;
  class Functor_i;
  class Measurements_i;

  // ===========================================================================================
  /*!
   * \brief Object used to make TPythonDump know that its held value can be a varible
   *
   * TPythonDump substitute TVar with names of notebook variables if any.
   */
  // ===========================================================================================

  struct SMESH_I_EXPORT TVar
  {
    std::vector< std::string > myVals;
    TVar(CORBA::Double value);
    TVar(CORBA::Long   value);
    TVar(CORBA::Short  value);
    TVar(const SMESH::double_array& value);
    // string used to temporary quote variable names in order
    // not to confuse variables with string arguments
    static char Quote() { return '$'; }
  };

  // ===========================================================================================
  /*!
   * \brief Utility helping in storing SMESH engine calls as python commands
   */
  // ===========================================================================================

  class SMESH_I_EXPORT TPythonDump
  {
    std::ostringstream myStream;
    static size_t      myCounter;
    int                myVarsCounter; // counts stored TVar's
  public:
    TPythonDump();
    virtual ~TPythonDump();

    TPythonDump&
    operator<<(const TVar& theVariableValue);

    TPythonDump&
    operator<<(long int theArg);

    TPythonDump&
    operator<<(int theArg);

    TPythonDump&
    operator<<(double theArg);

    TPythonDump&
    operator<<(float theArg);

    TPythonDump&
    operator<<(const void* theArg);

    TPythonDump&
    operator<<(const char* theArg);

    TPythonDump&
    operator<<(const SMESH::ElementType& theArg);

    TPythonDump&
    operator<<(const SMESH::GeometryType& theArg);

    TPythonDump&
    operator<<(const SMESH::long_array& theArg);

    TPythonDump&
    operator<<(const SMESH::double_array& theArg);

    TPythonDump&
    operator<<(const SMESH::string_array& theArg);

    TPythonDump&
    operator<<(SMESH::SMESH_Hypothesis_ptr theArg);

    TPythonDump&
    operator<<(SMESH::SMESH_IDSource_ptr theArg);

    TPythonDump&
    operator<<(SALOMEDS::SObject_ptr theArg);

    TPythonDump&
    operator<<(CORBA::Object_ptr theArg);

    TPythonDump&
    operator<<(SMESH::FilterLibrary_i* theArg);

    TPythonDump&
    operator<<(SMESH::FilterManager_i* theArg);

    TPythonDump&
    operator<<(SMESH::Filter_i* theArg);

    TPythonDump&
    operator<<(SMESH::Functor_i* theArg);

    TPythonDump&
    operator<<(SMESH::Measurements_i* theArg);

    TPythonDump&
    operator<<(SMESH_Gen_i* theArg);

    TPythonDump&
    operator<<(SMESH_MeshEditor_i* theArg);

    TPythonDump&
    operator<<(SMESH::MED_VERSION theArg);

    TPythonDump&
    operator<<(const SMESH::AxisStruct & theAxis);

    TPythonDump&
    operator<<(const SMESH::DirStruct & theDir);

    TPythonDump&
    operator<<(const SMESH::PointStruct & P);

    TPythonDump&
    operator<<(const TCollection_AsciiString & theArg);

    TPythonDump&
    operator<<(const SMESH::ListOfGroups& theList);

    TPythonDump&
    operator<<(const SMESH::ListOfGroups * theList);

    TPythonDump&
    operator<<(const SMESH::ListOfIDSources& theList);

    static const char* SMESHGenName() { return "smeshgen"; }
    static const char* MeshEditorName() { return "mesh_editor"; }

    /*!
     * \brief Return marker of long string literal beginning
      * \param type - a name of functionality producing the string literal
      * \retval TCollection_AsciiString - the marker string to be written into
      * a raw python script
     */
    static TCollection_AsciiString LongStringStart(const char* type);
    /*!
     * \brief Return marker of long string literal end
      * \retval TCollection_AsciiString - the marker string to be written into
      * a raw python script
     */
    static TCollection_AsciiString LongStringEnd();
    /*!
     * \brief Cut out a long string literal from a string
      * \param theText - text possibly containing string literals
      * \param theFrom - position in the text to search from
      * \param theLongString - the retrieved literal
      * \param theStringType - a name of functionality produced the literal
      * \retval bool - true if a string literal found
     *
     * The literal is removed from theText; theFrom points position right after
     * the removed literal
     */
    static bool  CutoutLongString( TCollection_AsciiString & theText,
                                   int                     & theFrom,
                                   TCollection_AsciiString & theLongString,
                                   TCollection_AsciiString & theStringType);
  };
}

#endif
