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
//  File    : SMESH_DumpPython.cxx
//  Created : Thu Mar 24 17:17:59 2005
//  Author  : Julia DOROVSKIKH
//  Module  : SMESH

#include "SMESH_PythonDump.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_MeshEditor_i.hxx"
#include "SMESH_2smeshpy.hxx"

#include <TColStd_HSequenceOfInteger.hxx>
#include <TCollection_AsciiString.hxx>
#include <SMESH_Comment.hxx>


#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

static TCollection_AsciiString NotPublishedObjectName()
{
  return "__NOT__Published__Object__";
}

namespace SMESH
{

  size_t TPythonDump::myCounter = 0;

  TVar::TVar(CORBA::Double value):myVals(1) { myVals[0] = SMESH_Comment(value); }
  TVar::TVar(CORBA::Long   value):myVals(1) { myVals[0] = SMESH_Comment(value); }
  TVar::TVar(CORBA::Short  value):myVals(1) { myVals[0] = SMESH_Comment(value); }
  TVar::TVar(const SMESH::double_array& value):myVals(value.length())
  {
    for ( size_t i = 0; i < value.length(); i++)
      myVals[i] = SMESH_Comment(value[i]);
  }

  TPythonDump::
  TPythonDump():myVarsCounter(0)
  {
    ++myCounter;
  }
  TPythonDump::
  ~TPythonDump()
  {
    if(--myCounter == 0){
      SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
      std::string aString = myStream.str();
      TCollection_AsciiString aCollection(Standard_CString(aString.c_str()));
      SALOMEDS::Study_ptr aStudy = aSMESHGen->GetCurrentStudy();
      if(!aStudy->_is_nil() && !aCollection.IsEmpty()){
        aSMESHGen->AddToPythonScript(aStudy->StudyId(),aCollection);
        if(MYDEBUG) MESSAGE(aString);
        aSMESHGen->UpdateParameters(""); // prevent misuse of already treated variables
      }
    }
  }

  TPythonDump& //!< to store a variable value
  TPythonDump::
  operator<<(const TVar& theVarValue)
  {
    if ( theVarValue.myVals.empty() ) return *this;

    const std::vector< std::string >& varNames = SMESH_Gen_i::GetSMESHGen()->GetLastParameters();
    if ( theVarValue.myVals.size() > 1 )
    {
      myStream << "[ ";
      for ( size_t i = 1; i <= theVarValue.myVals.size(); ++i )
      {
        if ( myVarsCounter < varNames.size() && !varNames[ myVarsCounter ].empty() )
          myStream << TVar::Quote() << varNames[ myVarsCounter ] << TVar::Quote();
        else
          myStream << theVarValue.myVals[i-1];
        if ( i < theVarValue.myVals.size() )
          myStream << ", ";
        ++myVarsCounter;
      }
      myStream << " ]";
    }
    else
    {
      if ( myVarsCounter < varNames.size() && !varNames[ myVarsCounter ].empty() )
        myStream << TVar::Quote() << varNames[ myVarsCounter ] << TVar::Quote();
      else
        myStream << theVarValue.myVals[0];
      ++myVarsCounter;
    }
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(long int theArg){
    myStream<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(int theArg){
    myStream<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(double theArg){
    myStream<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(float theArg){
    myStream<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(const void* theArg){
    myStream<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(const char* theArg){
    if ( theArg )
      myStream<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(const SMESH::ElementType& theArg)
  {
    myStream<<"SMESH.";
    switch(theArg){
    case ALL:   myStream<<"ALL";break;
    case NODE:  myStream<<"NODE";break;
    case EDGE:  myStream<<"EDGE";break;
    case FACE:  myStream<<"FACE";break;
    case VOLUME:myStream<<"VOLUME";break;
    case ELEM0D:myStream<<"ELEM0D";break;
    default:    myStream<<"__UNKNOWN__ElementType: " << theArg;
    }
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(const SMESH::GeometryType& theArg)
  {
    myStream<<"SMESH.";
    switch(theArg){
    case Geom_POINT:      myStream<<"Geom_POINT";      break;
    case Geom_EDGE:       myStream<<"Geom_EDGE";       break;
    case Geom_TRIANGLE:   myStream<<"Geom_TRIANGLE";   break;
    case Geom_QUADRANGLE: myStream<<"Geom_QUADRANGLE"; break;
    case Geom_POLYGON:    myStream<<"Geom_POLYGON";    break;
    case Geom_TETRA:      myStream<<"Geom_TETRA";      break;
    case Geom_PYRAMID:    myStream<<"Geom_PYRAMID";    break;
    case Geom_HEXA:       myStream<<"Geom_HEXA";       break;
    case Geom_PENTA:      myStream<<"Geom_PENTA";      break;
    case Geom_POLYHEDRA:  myStream<<"Geom_POLYHEDRA";  break;
    default:    myStream<<"__UNKNOWN__GeometryType: " << theArg;
    }
    return *this;
  }

  template<class TArray>
  void DumpArray(const TArray& theArray, TPythonDump & theStream)
  {
    theStream << "[ ";
    for (int i = 1; i <= theArray.length(); i++) {
      theStream << theArray[i-1];
      if ( i < theArray.length() )
        theStream << ", ";
    }
    theStream << " ]";
  }

  TPythonDump&
  TPythonDump::operator<<(const SMESH::long_array& theArg)
  {
    DumpArray( theArg, *this );
    return *this;
  }

  TPythonDump&
  TPythonDump::operator<<(const SMESH::double_array& theArg)
  {
    DumpArray( theArg, *this );
    return *this;
  }

  TPythonDump&
  TPythonDump::operator<<(const SMESH::string_array& theArray)
  {
    myStream << "[ ";
    for (int i = 1; i <= theArray.length(); i++) {
      myStream << "'" << theArray[i-1] << "'";
      if ( i < theArray.length() )
        myStream << ", ";
    }
    myStream << " ]";
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SALOMEDS::SObject_ptr aSObject)
  {
    if ( !aSObject->_is_nil() )
      myStream << aSObject->GetID();
    else
      myStream << NotPublishedObjectName();
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(CORBA::Object_ptr theArg)
  {
    SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var aStudy = aSMESHGen->GetCurrentStudy();
    SALOMEDS::SObject_var aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(!aSObject->_is_nil()) {
      CORBA::String_var id = aSObject->GetID();
      myStream << id;
    } else if ( !CORBA::is_nil(theArg)) {
      if ( aSMESHGen->CanPublishInStudy( theArg )) // not published SMESH object
        myStream << "smeshObj_" << size_t(theArg);
      else
        myStream << NotPublishedObjectName();
    }
    else
      myStream << "None";
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::SMESH_Hypothesis_ptr theArg)
  {
    SALOMEDS::Study_var aStudy = SMESH_Gen_i::GetSMESHGen()->GetCurrentStudy();
    SALOMEDS::SObject_var aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(aSObject->_is_nil() && !CORBA::is_nil(theArg))
      myStream << "hyp_" << theArg->GetId();
    else
      *this << CORBA::Object_ptr( theArg );
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::SMESH_IDSource_ptr theArg)
  {
    if ( CORBA::is_nil( theArg ) )
      return *this << "None";
    SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var aStudy = aSMESHGen->GetCurrentStudy();
    SALOMEDS::SObject_var aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(!aSObject->_is_nil())
      return *this << aSObject;
    if ( SMESH::Filter_i* filter = SMESH::DownCast<SMESH::Filter_i*>( theArg ))
      return *this << filter;
    SMESH::SMESH_Mesh_var mesh = theArg->GetMesh();
    if ( !theArg->_is_equivalent( mesh ))
    {
      SMESH::long_array_var anElementsId = theArg->GetIDs();
      SMESH::array_of_ElementType_var types =  theArg->GetTypes();
      SMESH::ElementType type = types->length() ? types[0] : SMESH::ALL;
      return *this << mesh << ".GetIDSource(" << anElementsId << ", " << type << ")";
    }
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::FilterLibrary_i* theArg)
  {
    myStream<<"aFilterLibrary"<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::FilterManager_i* theArg)
  {
    myStream<<"aFilterManager";
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::Filter_i* theArg)
  {
    myStream<<"aFilter"<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::Functor_i* theArg)
  {
    if ( theArg ) {
      FunctorType aFunctorType = theArg->GetFunctorType();
      switch(aFunctorType){
      case FT_AspectRatio:           myStream<< "anAspectRatio";          break;
      case FT_AspectRatio3D:         myStream<< "anAspectRatio3D";        break;
      case FT_Warping:               myStream<< "aWarping";               break;
      case FT_MinimumAngle:          myStream<< "aMinimumAngle";          break;
      case FT_Taper:                 myStream<< "aTaper";                 break;
      case FT_Skew:                  myStream<< "aSkew";                  break;
      case FT_Area:                  myStream<< "aArea";                  break;
      case FT_Volume3D:              myStream<< "aVolume3D";              break;
      case FT_MaxElementLength2D:    myStream<< "aMaxElementLength2D";    break;
      case FT_MaxElementLength3D:    myStream<< "aMaxElementLength3D";    break;
      case FT_FreeBorders:           myStream<< "aFreeBorders";           break;
      case FT_FreeEdges:             myStream<< "aFreeEdges";             break;
      case FT_FreeNodes:             myStream<< "aFreeNodes";             break;
      case FT_FreeFaces:             myStream<< "aFreeFaces";             break;
      case FT_MultiConnection:       myStream<< "aMultiConnection";       break;
      case FT_MultiConnection2D:     myStream<< "aMultiConnection2D";     break;
      case FT_Length:                myStream<< "aLength";                break;
      case FT_Length2D:              myStream<< "aLength2D";              break;
      case FT_BelongToGeom:          myStream<< "aBelongToGeom";          break;
      case FT_BelongToPlane:         myStream<< "aBelongToPlane";         break;
      case FT_BelongToCylinder:      myStream<< "aBelongToCylinder";      break;
      case FT_BelongToGenSurface:    myStream<< "aBelongToGenSurface";    break;
      case FT_LyingOnGeom:           myStream<< "aLyingOnGeom";           break;
      case FT_CoplanarFaces:         myStream<< "aCoplanarFaces";         break;
      case FT_RangeOfIds:            myStream<< "aRangeOfIds";            break;
      case FT_BadOrientedVolume:     myStream<< "aBadOrientedVolume";     break;
      case FT_BareBorderVolume:      myStream<< "aBareBorderVolume";      break;
      case FT_BareBorderFace:        myStream<< "aBareBorderFace";        break;
      case FT_OverConstrainedVolume: myStream<< "aOverConstrainedVolume"; break;
      case FT_OverConstrainedFace:   myStream<< "aOverConstrainedFace";   break;
      case FT_LinearOrQuadratic:     myStream<< "aLinearOrQuadratic";     break;
      case FT_GroupColor:            myStream<< "aGroupColor";            break;
      case FT_ElemGeomType:          myStream<< "anElemGeomType";         break;
      case FT_LessThan:              myStream<< "aLessThan";              break;
      case FT_MoreThan:              myStream<< "aMoreThan";              break;
      case FT_EqualTo:               myStream<< "anEqualTo";              break;
      case FT_LogicalNOT:            myStream<< "aLogicalNOT";            break;
      case FT_LogicalAND:            myStream<< "aLogicalAND";            break;
      case FT_LogicalOR:             myStream<< "aLogicalOR";             break;
      case FT_Undefined:
      default:                       myStream<< "anUndefined";            break;
      }
      myStream<<theArg;
    }
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::Measurements_i* theArg)
  {
    myStream<<"aMeasurements";
    return *this;
  }


  TPythonDump& TPythonDump:: operator<<(SMESH_Gen_i* theArg)
  {
    myStream << SMESHGenName(); return *this;
  }

  TPythonDump& TPythonDump::operator<<(SMESH_MeshEditor_i* theArg)
  {
    myStream << MeshEditorName() << "_" << ( theArg ? theArg->GetMeshId() : -1 ); return *this;
  }

  TPythonDump& TPythonDump::operator<<(const TCollection_AsciiString & theStr)
  {
    myStream << theStr; return *this;
  }


  TPythonDump& TPythonDump::operator<<(SMESH::MED_VERSION theVersion)
  {
    switch (theVersion) {
    case SMESH::MED_V2_1: myStream << "SMESH.MED_V2_1"; break;
    case SMESH::MED_V2_2: myStream << "SMESH.MED_V2_2"; break;
    default: myStream << theVersion;
    }
    return *this;
  }

  TPythonDump& TPythonDump::operator<<(const SMESH::AxisStruct & theAxis)
  {
    *this << "SMESH.AxisStruct( "
          << TVar( theAxis.x  ) << ", "
          << TVar( theAxis.y  ) << ", "
          << TVar( theAxis.z  ) << ", "
          << TVar( theAxis.vx ) << ", "
          << TVar( theAxis.vy ) << ", "
          << TVar( theAxis.vz ) << " )";
    return *this;
  }

  TPythonDump& TPythonDump::operator<<(const SMESH::DirStruct & theDir)
  {
    const SMESH::PointStruct & P = theDir.PS;
    *this << "SMESH.DirStruct( SMESH.PointStruct ( "
          << TVar( P.x ) << ", "
          << TVar( P.y ) << ", "
          << TVar( P.z ) << " ))";
    return *this;
  }

  TPythonDump& TPythonDump::operator<<(const SMESH::PointStruct & P)
  {
    *this << "SMESH.PointStruct ( "
          << TVar( P.x ) << ", "
          << TVar( P.y ) << ", "
          << TVar( P.z ) << " )";
    return *this;
  }

  TPythonDump& TPythonDump::operator<<(const SMESH::ListOfGroups& theList)
  {
    DumpArray( theList, *this );
    return *this;
  }
  TPythonDump& TPythonDump::operator<<(const SMESH::ListOfGroups * theList)
  {
    DumpArray( *theList, *this );
    return *this;
  }
  TPythonDump& TPythonDump::operator<<(const SMESH::ListOfIDSources& theList)
  {
    DumpArray( theList, *this );
    return *this;
  }

  TCollection_AsciiString myLongStringStart( "TPythonDump::LongStringStart" );
  TCollection_AsciiString myLongStringEnd  ( "TPythonDump::LongStringEnd" );

  //================================================================================
  /*!
   * \brief Return marker of long string literal beginning
   * \param type - a name of functionality producing the string literal
   * \retval TCollection_AsciiString - the marker string to be written into
   * a raw python script
   */
  //================================================================================

  TCollection_AsciiString TPythonDump::LongStringStart(const char* type)
  {
    return
      myLongStringStart +
      (Standard_Integer) strlen(type) +
      " " +
      (char*) type;
  }

  //================================================================================
  /*!
     * \brief Return marker of long string literal end
      * \retval TCollection_AsciiString - the marker string to be written into
      * a raw python script
   */
  //================================================================================

  TCollection_AsciiString TPythonDump::LongStringEnd()
  {
    return myLongStringEnd;
  }

  //================================================================================
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
  //================================================================================

  bool  TPythonDump::CutoutLongString( TCollection_AsciiString & theText,
                                       int                     & theFrom,
                                       TCollection_AsciiString & theLongString,
                                       TCollection_AsciiString & theStringType)
  {
    if ( theFrom < 1 || theFrom > theText.Length() )
      return false;

    // ...script \  beg marker    \ \ type \       literal              \  end marker  \ script...
    //  "theText myLongStringStart7 Pattern!!! SALOME Mesh Pattern file myLongStringEndtextEnd"
    //  012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    //  0         1         2         3         4         5         6         7         8

    theFrom = theText.Location( myLongStringStart, theFrom, theText.Length() ); // = 09
    if ( !theFrom )
      return false;

    // find where literal begins
    int literalBeg = theFrom + myLongStringStart.Length(); // = 26
    char* typeLenStr = (char*) theText.ToCString() + literalBeg - 1; // = "7 Pattern!!! SALO...."
    int typeLen = atoi ( typeLenStr ); // = 7
    while ( *typeLenStr != ' ' ) { // look for ' ' after typeLen
      literalBeg++; // 26 -> 27
      typeLenStr++;
    }
    literalBeg += typeLen + 1; // = 35
    if ( literalBeg > theText.Length() )
      return false;

    // where literal ends (i.e. end marker begins)
    int literalEnd = theText.Location( myLongStringEnd, literalBeg, theText.Length() ); // = 64
    if ( !literalEnd )
      literalEnd = theText.Length();

    // literal
    theLongString = theText.SubString( literalBeg, literalEnd - 1); // "!!! SALOME Mesh Pattern file "
    // type
    theStringType = theText.SubString( literalBeg - typeLen, literalBeg - 1 ); // "Pattern"
    // cut off literal
    literalEnd += myLongStringEnd.Length(); // = 79
    TCollection_AsciiString textEnd = theText.SubString( literalEnd, theText.Length() ); // "textE..."
    theText = theText.SubString( 1, theFrom - 1 ) + textEnd;

    return true;
  }
}

//=======================================================================
//function : RemoveTabulation
//purpose  : 
//=======================================================================
void RemoveTabulation( TCollection_AsciiString& theScript )
{
  std::string aString( theScript.ToCString() );
  std::string::size_type aPos = 0;
  while( aPos < aString.length() )
  {
    aPos = aString.find( "\n\t", aPos );
    if( aPos == std::string::npos )
      break;
    aString.replace( aPos, 2, "\n" );
    aPos++;
  }
  theScript = aString.c_str();
}

//=======================================================================
//function : DumpPython
//purpose  :
//=======================================================================
Engines::TMPFile* SMESH_Gen_i::DumpPython (CORBA::Object_ptr theStudy,
                                           CORBA::Boolean isPublished,
                                           CORBA::Boolean isMultiFile,
                                           CORBA::Boolean& isValidScript)
{
  SALOMEDS::Study_var aStudy = SALOMEDS::Study::_narrow(theStudy);
  if (CORBA::is_nil(aStudy))
    return new Engines::TMPFile(0);

  SALOMEDS::SObject_var aSO = aStudy->FindComponent(ComponentDataType());
  if (CORBA::is_nil(aSO))
    return new Engines::TMPFile(0);

  // Map study entries to object names
  Resource_DataMapOfAsciiStringAsciiString aMap;
  Resource_DataMapOfAsciiStringAsciiString aMapNames;
  //TCollection_AsciiString s ("qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM0987654321_");

  SALOMEDS::ChildIterator_var Itr = aStudy->NewChildIterator(aSO);
  for (Itr->InitEx(true); Itr->More(); Itr->Next()) {
    SALOMEDS::SObject_var aValue = Itr->Value();
    CORBA::String_var anID = aValue->GetID();
    CORBA::String_var aName = aValue->GetName();
    TCollection_AsciiString aGUIName ( (char*) aName.in() );
    TCollection_AsciiString anEnrty ( (char*) anID.in() );
    if (aGUIName.Length() > 0) {
      aMapNames.Bind( anEnrty, aGUIName );
      aMap.Bind( anEnrty, aGUIName );
    }
  }

  // Get trace of restored study
  //SALOMEDS::SObject_var aSO = SMESH_Gen_i::ObjectToSObject(theStudy, _this());
  SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr =
    aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePythonObject");

  char* oldValue = SALOMEDS::AttributePythonObject::_narrow(anAttr)->GetObject();
  TCollection_AsciiString aSavedTrace (oldValue);

  // Add trace of API methods calls and replace study entries by names
  TCollection_AsciiString aScript;
  aScript += DumpPython_impl(aStudy, aMap, aMapNames, isPublished, isMultiFile,
                             myIsHistoricalPythonDump, isValidScript, aSavedTrace);

  int aLen = aScript.Length();
  unsigned char* aBuffer = new unsigned char[aLen+1];
  strcpy((char*)aBuffer, aScript.ToCString());

  CORBA::Octet* anOctetBuf =  (CORBA::Octet*)aBuffer;
  Engines::TMPFile_var aStreamFile = new Engines::TMPFile(aLen+1, aLen+1, anOctetBuf, 1);

  bool hasNotPublishedObjects = aScript.Location( NotPublishedObjectName(), 1, aLen);
  isValidScript = isValidScript && !hasNotPublishedObjects;

  return aStreamFile._retn();
}

//=============================================================================
/*!
 *  AddToPythonScript
 */
//=============================================================================
void SMESH_Gen_i::AddToPythonScript (int theStudyID, const TCollection_AsciiString& theString)
{
  if (myPythonScripts.find(theStudyID) == myPythonScripts.end()) {
    myPythonScripts[theStudyID] = new TColStd_HSequenceOfAsciiString;
  }
  myPythonScripts[theStudyID]->Append(theString);
}

//=============================================================================
/*!
 *  RemoveLastFromPythonScript
 */
//=============================================================================
void SMESH_Gen_i::RemoveLastFromPythonScript (int theStudyID)
{
  if (myPythonScripts.find(theStudyID) != myPythonScripts.end()) {
    int aLen = myPythonScripts[theStudyID]->Length();
    myPythonScripts[theStudyID]->Remove(aLen);
  }
}

//=======================================================================
//function : SavePython
//purpose  :
//=======================================================================
void SMESH_Gen_i::SavePython (SALOMEDS::Study_ptr theStudy)
{
  // Dump trace of API methods calls
  TCollection_AsciiString aScript = GetNewPythonLines(theStudy->StudyId());

  // Check contents of PythonObject attribute
  SALOMEDS::SObject_var aSO = theStudy->FindComponent(ComponentDataType());
  //SALOMEDS::SObject_var aSO = SMESH_Gen_i::ObjectToSObject(theStudy, _this());
  SALOMEDS::StudyBuilder_var aStudyBuilder = theStudy->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr =
    aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePythonObject");

  char* oldValue = SALOMEDS::AttributePythonObject::_narrow(anAttr)->GetObject();
  TCollection_AsciiString oldScript (oldValue);

  if (oldScript.Length() > 0) {
    oldScript += "\n";
    oldScript += aScript;
  } else {
    oldScript = aScript;
  }

  // Store in PythonObject attribute
  SALOMEDS::AttributePythonObject::_narrow(anAttr)->SetObject(oldScript.ToCString(), 1);

  // Clean trace of API methods calls
  CleanPythonTrace(theStudy->StudyId());
}


// impl


//=============================================================================
/*!
 *  FindEntries: Returns a sequence of start/end positions of entries in the string
 */
//=============================================================================
Handle(TColStd_HSequenceOfInteger) FindEntries (TCollection_AsciiString& theString)
{
  Handle(TColStd_HSequenceOfInteger) aSeq = new TColStd_HSequenceOfInteger;
  Standard_Integer aLen = theString.Length();
  Standard_Boolean isFound = Standard_False;

  char* arr = (char*) theString.ToCString();
  Standard_Integer i = 0, j;

  while(i < aLen) {
    int c = (int)arr[i];
    j = i+1;
    if ( isdigit( c )) { //Is digit?

      isFound = Standard_False;
      while((j < aLen) && ( isdigit(c) || c == ':' )) { //Check if it is an entry
        c = (int)arr[j++];
        if(c == ':') isFound = Standard_True;
      }

      if (isFound) {
        int prev = (i < 1) ? 0 : (int)arr[i - 1];
        // to distinguish from a sketcher command:
        // last char should be a digit, not ":",
        // previous char should not be '"'.
        if (arr[j-2] != ':' && prev != '"') {
          aSeq->Append(i+1); // +1 because AsciiString starts from 1
          aSeq->Append(j-1);
        }
      }
    }

    i = j;
  }

  return aSeq;
}

namespace {

  //================================================================================
  /*!
   * \brief Make a string be a valid python name
    * \param aName - a string to fix
    * \retval bool - true if aName was not modified
   */
  //================================================================================

  bool fixPythonName(TCollection_AsciiString & aName )
  {
    const TCollection_AsciiString allowedChars =
      "qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM0987654321_";
    bool isValidName = true;
    int nbUnderscore = 0;
    int p=1; // replace not allowed chars by underscore
    while (p <= aName.Length() &&
           (p = aName.FirstLocationNotInSet(allowedChars, p, aName.Length())))
    {
      if ( p == 1 || p == aName.Length() || aName.Value(p-1) == '_')
        aName.Remove( p, 1 ); // remove double _ from the start and the end
      else
        aName.SetValue(p, '_'), nbUnderscore++;
      isValidName = false;
    }
    if ( aName.IsIntegerValue() ) { // aName must not start with a digit
      aName.Insert( 1, 'a' );
      isValidName = false;
    }
    // shorten names like CartesianParameters3D_400_400_400_1000000_1
    if ( aName.Length() > 20 && nbUnderscore > 2 )
    {
      p = aName.Location( "_", 20, aName.Length());
      if ( p > 1 )
        aName.Trunc( p-1 );
    }
    return isValidName;
  }
}

//=============================================================================
/*!
 *  DumpPython
 */
//=============================================================================
TCollection_AsciiString SMESH_Gen_i::DumpPython_impl
                        (SALOMEDS::Study_ptr theStudy,
                         Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
                         Resource_DataMapOfAsciiStringAsciiString& theNames,
                         bool isPublished,
                         bool isMultiFile,
                         bool isHistoricalDump,
                         bool& aValidScript,
                         const TCollection_AsciiString& theSavedTrace)
{
  int aStudyID = theStudy->StudyId();

  TCollection_AsciiString helper; // to comfortably concatenate C strings
  TCollection_AsciiString aSmeshpy( SMESH_2smeshpy::SmeshpyName() );
  TCollection_AsciiString aSMESHGen( SMESH_2smeshpy::GenName() );
  TCollection_AsciiString anOldGen( SMESH::TPythonDump::SMESHGenName() );

  TCollection_AsciiString aScript;
  if( isMultiFile )
    aScript += "def RebuildData(theStudy):";
  aScript += "\n\t";
  aScript += helper + "aFilterManager = " + aSMESHGen + ".CreateFilterManager()\n\t";
  aScript += helper + "aMeasurements = " + aSMESHGen + ".CreateMeasurements()\n\t";
  if ( isPublished )
    aScript += aSMESHGen + ".SetCurrentStudy(theStudy)";
  else
    aScript += aSMESHGen + ".SetCurrentStudy(None)";

  // import python files corresponding to plugins
  set<string> moduleNameSet;
  map<string, GenericHypothesisCreator_i*>::iterator hyp_creator = myHypCreatorMap.begin();
  for ( ; hyp_creator != myHypCreatorMap.end(); ++hyp_creator ) {
    string moduleName = hyp_creator->second->GetModuleName();
    bool newModule = moduleNameSet.insert( moduleName ).second;
    if ( newModule )
      aScript += helper + "\n\t" + "import " + (char*) moduleName.c_str();
  }

  // Dump trace of restored study
  if (theSavedTrace.Length() > 0) {
    // For the convertion of IDL API calls -> smesh.py API, "smesh" standing for SMESH_Gen
    // was replaces with "smeshgen" (==TPythonDump::SMESHGenName()).
    // Change "smesh" -> "smeshgen" in the trace saved before passage to smesh.py API
    bool isNewVersion =
      theSavedTrace.Location( anOldGen + ".", 1, theSavedTrace.Length() );
    if ( !isNewVersion ) {
      TCollection_AsciiString aSavedTrace( theSavedTrace );
      TCollection_AsciiString aSmeshCall ( "smesh." ), gen( "gen" );
      int beg, end = aSavedTrace.Length(), from = 1;
      while ( from < end && ( beg = aSavedTrace.Location( aSmeshCall, from, end ))) {
        char charBefore = ( beg == 1 ) ? ' ' : aSavedTrace.Value( beg - 1 );
        if ( isspace( charBefore ) || charBefore == '=' ) { // "smesh." is not a part of a long word
          aSavedTrace.Insert( beg + aSmeshCall.Length() - 1, gen );// "smesh" -> "smeshgen"
          end += gen.Length();
        }
        from = beg + aSmeshCall.Length();
      }
      aScript += helper + "\n" + aSavedTrace;
    }
    else
      // append a saved trace to the script
      aScript += helper + "\n" + theSavedTrace;
  }

  // Dump trace of API methods calls
  TCollection_AsciiString aNewLines = GetNewPythonLines(aStudyID);
  if (aNewLines.Length() > 0) {
    aScript += helper + "\n" + aNewLines;
  }

  // Convert IDL API calls into smesh.py API.
  // Some objects are wrapped with python classes and
  // Resource_DataMapOfAsciiStringAsciiString holds methods returning wrapped objects
  Resource_DataMapOfAsciiStringAsciiString anEntry2AccessorMethod;
  if ( !getenv("NO_2smeshpy_conversion"))
    aScript = SMESH_2smeshpy::ConvertScript( aScript, anEntry2AccessorMethod,
                                             theObjectNames, theStudy, isHistoricalDump );

  // Replace characters used instead of quote marks to quote notebook variables
  {
    int pos = 1;
    while (( pos = aScript.Location( 1, SMESH::TVar::Quote(), pos, aScript.Length() )))
      aScript.SetValue( pos, '"' );
  }

  // Find entries to be replaced by names
  Handle(TColStd_HSequenceOfInteger) aSeq = FindEntries(aScript);
  Standard_Integer aLen = aSeq->Length();

  if (aLen == 0 && isMultiFile)
    return aScript;

  // Replace entries by the names
  GEOM::GEOM_Gen_ptr geom = GetGeomEngine();
  TColStd_SequenceOfAsciiString seqRemoved;
  Resource_DataMapOfAsciiStringAsciiString mapRemoved;
  Standard_Integer objectCounter = 0, aStart = 1, aScriptLength = aScript.Length();
  TCollection_AsciiString anUpdatedScript, anEntry, aName, aBaseName("smeshObj_");

  // Collect names of GEOM objects to exclude same names of SMESH objects
  GEOM::string_array_var aGeomNames = geom->GetAllDumpNames();
  int ign = 0, nbgn = aGeomNames->length();
  for (; ign < nbgn; ign++) {
    aName = aGeomNames[ign];
    theObjectNames.Bind(aName, "1");
  }

  bool importGeom = false;
  for (Standard_Integer i = 1; i <= aLen; i += 2) {
    anUpdatedScript += aScript.SubString(aStart, aSeq->Value(i) - 1);
    anEntry = aScript.SubString(aSeq->Value(i), aSeq->Value(i + 1));
    // is a GEOM object?
    aName = geom->GetDumpName( anEntry.ToCString() );
    if (aName.IsEmpty()) {
      // is a SMESH object
      if (theObjectNames.IsBound(anEntry)) {
        // The Object is in Study
        aName = theObjectNames.Find(anEntry);
        // check validity of aName
        bool isValidName = fixPythonName( aName );
        if (theObjectNames.IsBound(aName) && anEntry != theObjectNames(aName)) {
          // diff objects have same name - make a new name by appending a digit
          TCollection_AsciiString aName2;
          Standard_Integer i = 0;
          do {
            aName2 = aName + "_" + ++i;
          } while (theObjectNames.IsBound(aName2) && anEntry != theObjectNames(aName2));
          aName = aName2;
          isValidName = false;
        }
        if ( !isValidName )
          theObjectNames(anEntry) = aName;

      } else {
        // Removed Object
        do {
          aName = aBaseName + (++objectCounter);
        } while (theObjectNames.IsBound(aName));
        seqRemoved.Append(aName);
        mapRemoved.Bind(anEntry, "1");
        theObjectNames.Bind(anEntry, aName);
      }
      theObjectNames.Bind(aName, anEntry); // to detect same name of diff objects
    }
    else
    {
      importGeom = true;
    }
    anUpdatedScript += aName;
    aStart = aSeq->Value(i + 1) + 1;
  }

  // set initial part of aSript
  TCollection_AsciiString initPart = "import ";
  if ( isMultiFile )
    initPart += helper + "salome, ";
  initPart += aSmeshpy + ", SMESH, SALOMEDS\n";
  if ( importGeom && isMultiFile )
  {
    initPart += ("\n## import GEOM dump file ## \n"
                 "import string, os, sys, re\n"
                 "sys.path.insert( 0, os.path.dirname(__file__) )\n"
                 "exec(\"from \"+re.sub(\"SMESH$\",\"GEOM\",__name__)+\" import *\")\n");
  }
  anUpdatedScript.Insert ( 1, initPart );

  // add final part of aScript
  if (aLen && aSeq->Value(aLen) < aScriptLength)
    anUpdatedScript += aScript.SubString(aSeq->Value(aLen) + 1, aScriptLength);

  // Remove removed objects
  if ( seqRemoved.Length() > 0 ) {
    anUpdatedScript += "\n\t## some objects were removed";
    anUpdatedScript += "\n\taStudyBuilder = theStudy.NewBuilder()";
  }
  for (int ir = 1; ir <= seqRemoved.Length(); ir++) {
    anUpdatedScript += "\n\tSO = theStudy.FindObjectIOR(theStudy.ConvertObjectToIOR(";
    anUpdatedScript += seqRemoved.Value(ir);
    // for object wrapped by class of smesh.py
    anEntry = theObjectNames( seqRemoved.Value(ir) );
    if ( anEntry2AccessorMethod.IsBound( anEntry ) )
      anUpdatedScript += helper + "." + anEntry2AccessorMethod( anEntry );
    anUpdatedScript += "))\n\tif SO is not None: aStudyBuilder.RemoveObjectWithChildren(SO)";
  }

  // Set object names
  anUpdatedScript += "\n\t## set object names";
//   anUpdatedScript += "\n\t\tsmeshgui = salome.ImportComponentGUI(\"SMESH\")";
//   anUpdatedScript += "\n\t\tsmeshgui.Init(theStudy._get_StudyId())";
//   anUpdatedScript += "\n";

  TCollection_AsciiString aGUIName;
  Resource_DataMapOfAsciiStringAsciiString mapEntries;
  for (Standard_Integer i = 1; i <= aLen; i += 2)
  {
    anEntry = aScript.SubString(aSeq->Value(i), aSeq->Value(i + 1));
    aName = geom->GetDumpName( anEntry.ToCString() );
    if (aName.IsEmpty() && // Not a GEOM object
        theNames.IsBound(anEntry) &&
        !mapEntries.IsBound(anEntry) && // Not yet processed
        !mapRemoved.IsBound(anEntry)) // Was not removed
    {
      aName = theObjectNames.Find(anEntry);
      aGUIName = theNames.Find(anEntry);
      mapEntries.Bind(anEntry, aName);
      anUpdatedScript += helper + "\n\t" + aSMESHGen + ".SetName(" + aName;
      if ( anEntry2AccessorMethod.IsBound( anEntry ) )
        anUpdatedScript += helper + "." + anEntry2AccessorMethod( anEntry );
      anUpdatedScript += helper + ", '" + aGUIName + "')";
    }
  }

  // Issue 0021249: removed (a similar block is dumped by SALOMEDSImpl_Study)
  //anUpdatedScript += "\n\tif salome.sg.hasDesktop():";
  //anUpdatedScript += "\n\t\tsalome.sg.updateObjBrowser(0)";

  // -----------------------------------------------------------------
  // store visual properties of displayed objects
  // -----------------------------------------------------------------

  if (isPublished)
  {
    //Output the script that sets up the visual parameters.
    char* script = theStudy->GetDefaultScript(ComponentDataType(), "\t");
    if (script && strlen(script) > 0) {
      anUpdatedScript += "\n\n\t### Store presentation parameters of displayed objects\n";
      anUpdatedScript += script;
      CORBA::string_free(script);
    }
  }

  if( isMultiFile )
    anUpdatedScript += "\n\tpass";
  anUpdatedScript += "\n";

  if( !isMultiFile ) // remove unnecessary tabulation
    RemoveTabulation( anUpdatedScript );

  // -----------------------------------------------------------------
  // put string literals describing patterns into separate functions
  // -----------------------------------------------------------------

  TCollection_AsciiString aLongString, aFunctionType;
  int where = 1;
  set< string > functionNameSet;
  while ( SMESH::TPythonDump::CutoutLongString( anUpdatedScript, where, aLongString, aFunctionType ))
  {
    // make a python string literal
    aLongString.Prepend(":\n\treturn '''\n");
    aLongString += "\n\t'''\n\tpass\n";

    TCollection_AsciiString functionName;

    // check if the function returning this literal is already defined
    int posAlready = anUpdatedScript.Location( aLongString, where, anUpdatedScript.Length() );
    if ( posAlready ) // already defined
    {
      // find the function name
      int functBeg = posAlready;
      char* script = (char*) anUpdatedScript.ToCString() + posAlready - 1; // look at ":" after "def fuction()"
      while ( *script != ' ' ) {
        script--;
        functBeg--;
      }
      functBeg++; // do not take ' '
      posAlready--; // do not take ':'
      functionName = anUpdatedScript.SubString( functBeg, posAlready );
    }
    else // not defined yet
    {
      // find a unique function name
      fixPythonName( aFunctionType );
      Standard_Integer nb = 0;
      do functionName = aFunctionType + "_" + ( nb++ ) + "()";
      while ( !functionNameSet.insert( functionName.ToCString() ).second );

      // define function
      TCollection_AsciiString funDef = helper + "def " + functionName + aLongString;
      if ( isMultiFile )
      {
        anUpdatedScript += helper + "\n\n" + funDef;
      }
      else
      {
        funDef += "\n\n";
        anUpdatedScript.Insert( 1, funDef);
        where += funDef.Length();
      }
    }
    anUpdatedScript.InsertBefore( where, functionName ); // call function
  }

  aValidScript = true;

  return anUpdatedScript;
}

//=============================================================================
/*!
 *  GetNewPythonLines
 */
//=============================================================================
TCollection_AsciiString SMESH_Gen_i::GetNewPythonLines (int theStudyID)
{
  TCollection_AsciiString aScript;

  // Dump trace of API methods calls
  if (myPythonScripts.find(theStudyID) != myPythonScripts.end()) {
    Handle(TColStd_HSequenceOfAsciiString) aPythonScript = myPythonScripts[theStudyID];
    Standard_Integer istr, aLen = aPythonScript->Length();
    for (istr = 1; istr <= aLen; istr++) {
      aScript += "\n\t";
      aScript += aPythonScript->Value(istr);
    }
    aScript += "\n";
  }

  return aScript;
}

//=============================================================================
/*!
 *  CleanPythonTrace
 */
//=============================================================================
void SMESH_Gen_i::CleanPythonTrace (int theStudyID)
{
  TCollection_AsciiString aScript;

  // Clean trace of API methods calls
  if (myPythonScripts.find(theStudyID) != myPythonScripts.end()) {
    myPythonScripts[theStudyID]->Clear();
  }
}
