// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
// File    : SMESH_Gen_i_DumpPython.cxx
// Created : Thu Mar 24 17:17:59 2005
// Author  : Julia DOROVSKIKH
// Module  : SMESH
// $Header : $

#include "SMESH_PythonDump.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_2smeshpy.hxx"

#include <TColStd_HSequenceOfInteger.hxx>
#include <TCollection_AsciiString.hxx>


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

  TPythonDump::
  TPythonDump()
  {
    ++myCounter;
  }
  TPythonDump::
  ~TPythonDump()
  {
    if(--myCounter == 0){
      SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
      SALOMEDS::Study_ptr aStudy = aSMESHGen->GetCurrentStudy();
      if(!aStudy->_is_nil()){
	std::string aString = myStream.str();
	TCollection_AsciiString aCollection(Standard_CString(aString.c_str()));
	aSMESHGen->AddToPythonScript(aStudy->StudyId(),aCollection);
	if(MYDEBUG) MESSAGE(aString);
      }
    }
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
    }
    return *this;
  }

  template<class TArray>
  void DumpArray(const TArray& theArray, std::ostringstream & theStream)
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
    DumpArray( theArg, myStream );
    return *this;
  }

  TPythonDump& 
  TPythonDump::operator<<(const SMESH::double_array& theArg)
  {
    DumpArray( theArg, myStream );
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
    SALOMEDS::Study_ptr aStudy = aSMESHGen->GetCurrentStudy();
    SALOMEDS::SObject_var aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(!aSObject->_is_nil()) {
      myStream << aSObject->GetID();
    } else if ( !CORBA::is_nil(theArg)) {
      if ( aSMESHGen->CanPublishInStudy( theArg )) // not published SMESH object
        myStream << "smeshObj_" << (int) theArg;
      else
        myStream << NotPublishedObjectName();
    }
    else
      myStream << "None";
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
      case FT_AspectRatio:      myStream<< "anAspectRatio";     break;
      case FT_AspectRatio3D:    myStream<< "anAspectRatio3D";   break;
      case FT_Warping:          myStream<< "aWarping";          break;
      case FT_MinimumAngle:     myStream<< "aMinimumAngle";     break;
      case FT_Taper:            myStream<< "aTaper";            break;
      case FT_Skew:             myStream<< "aSkew";             break;
      case FT_Area:             myStream<< "aArea";             break;
      case FT_FreeBorders:      myStream<< "aFreeBorders";      break;
      case FT_FreeEdges:        myStream<< "aFreeEdges";        break;
      case FT_MultiConnection:  myStream<< "aMultiConnection";  break;
      case FT_MultiConnection2D:myStream<< "aMultiConnection2D";break;
      case FT_Length:           myStream<< "aLength";           break;
      case FT_Length2D:         myStream<< "aLength";           break;
      case FT_BelongToGeom:     myStream<< "aBelongToGeom";     break;
      case FT_BelongToPlane:    myStream<< "aBelongToPlane";    break;
      case FT_BelongToCylinder: myStream<< "aBelongToCylinder"; break;
      case FT_LyingOnGeom:      myStream<< "aLyingOnGeom";      break;
      case FT_RangeOfIds:       myStream<< "aRangeOfIds";       break;
      case FT_BadOrientedVolume:myStream<< "aBadOrientedVolume";break;
      case FT_LessThan:         myStream<< "aLessThan";         break;
      case FT_MoreThan:         myStream<< "aMoreThan";         break;
      case FT_EqualTo:          myStream<< "anEqualTo";         break;
      case FT_LogicalNOT:       myStream<< "aLogicalNOT";       break;
      case FT_LogicalAND:       myStream<< "aLogicalAND";       break;
      case FT_LogicalOR:        myStream<< "aLogicalOR";        break;
      case FT_Undefined:        myStream<< "anUndefined";       break;
      }
      myStream<<theArg;
    }
    return *this;
  }

  TPythonDump& TPythonDump:: operator<<(SMESH_Gen_i* theArg)
  {
    myStream << SMESHGenName(); return *this;
  }

  TPythonDump& TPythonDump::operator<<(SMESH_MeshEditor_i* theArg)
  {
    myStream << MeshEditorName(); return *this;
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
    myStream << "SMESH.AxisStruct( "
             << theAxis.x  << ", "
             << theAxis.y  << ", "
             << theAxis.z  << ", "
             << theAxis.vx << ", "
             << theAxis.vy << ", "
             << theAxis.vz << " )";
    return *this;
  }

  TPythonDump& TPythonDump::operator<<(const SMESH::DirStruct & theDir)
  {
    const SMESH::PointStruct & P = theDir.PS;
    myStream << "SMESH.DirStruct( SMESH.PointStruct ( "
             << P.x  << ", "
             << P.y  << ", "
             << P.z  << " ))";
    return *this;
  }
}

//=======================================================================
//function : DumpPython
//purpose  : 
//=======================================================================
Engines::TMPFile* SMESH_Gen_i::DumpPython (CORBA::Object_ptr theStudy,
                                           CORBA::Boolean isPublished,
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
  TCollection_AsciiString aScript =
    "### This file is generated by SALOME automatically by dump python functionality of SMESH component\n\n";
  aScript += DumpPython_impl(aStudy->StudyId(), aMap, aMapNames,
                             isPublished, isValidScript, aSavedTrace);

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

  char* arr = theString.ToCString();
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

//=============================================================================
/*!
 *  DumpPython
 */
//=============================================================================
TCollection_AsciiString SMESH_Gen_i::DumpPython_impl
                        (int theStudyID, 
                         Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
                         Resource_DataMapOfAsciiStringAsciiString& theNames,
                         bool isPublished, 
                         bool& aValidScript,
                         const TCollection_AsciiString& theSavedTrace)
{
  TCollection_AsciiString helper; // to comfortably concatenate C strings
  TCollection_AsciiString aSmeshpy( SMESH_2smeshpy::SmeshpyName() );
  TCollection_AsciiString aSMESHGen( SMESH_2smeshpy::GenName() );
  TCollection_AsciiString anOldGen( SMESH::TPythonDump::SMESHGenName() );

  TCollection_AsciiString aScript;
  aScript = "def RebuildData(theStudy):\n\t";
  aScript += helper + "aFilterManager = " + aSMESHGen + ".CreateFilterManager()\n\t";
  if ( isPublished )
    aScript += aSMESHGen + ".SetCurrentStudy(theStudy)";
  else
    aScript += aSMESHGen + ".SetCurrentStudy(None)";

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
        if ( isspace( charBefore ) || charBefore == '=' ) {
          aSavedTrace.Insert( beg + aSmeshCall.Length() - 1, gen );
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
  TCollection_AsciiString aNewLines = GetNewPythonLines(theStudyID);
  if (aNewLines.Length() > 0) {
    aScript += helper + "\n" + aNewLines;
  }

  // Convert IDL API calls into smesh.py API.
  // Some objects are wrapped with python classes and
  // Resource_DataMapOfAsciiStringAsciiString holds methods returning wrapped objects
  Resource_DataMapOfAsciiStringAsciiString anEntry2AccessorMethod;
  aScript = SMESH_2smeshpy::ConvertScript( aScript, anEntry2AccessorMethod );

  // Find entries to be replaced by names
  Handle(TColStd_HSequenceOfInteger) aSeq = FindEntries(aScript);
  Standard_Integer aLen = aSeq->Length();

  if (aLen == 0)
    return aScript;

  // Replace entries by the names
  GEOM::GEOM_Gen_ptr geom = GetGeomEngine();
  TColStd_SequenceOfAsciiString seqRemoved;
  Resource_DataMapOfAsciiStringAsciiString mapRemoved;
  Standard_Integer objectCounter = 0, aStart = 1, aScriptLength = aScript.Length();
  TCollection_AsciiString anUpdatedScript, anEntry, aName, aBaseName("smeshObj_"),
    allowedChars ("qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM0987654321_");

  // Collect names of GEOM objects to exclude same names for SMESH objects
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
        bool isValidName = true;
        int p=1; // replace not allowed chars with underscore
        while (p <= aName.Length() &&
               (p = aName.FirstLocationNotInSet(allowedChars, p, aName.Length())))
        {
          if ( p == 1 || p == aName.Length() || aName.Value(p-1) == '_')
            aName.Remove( p, 1 ); // remove double _ and from the start and the end
          else
            aName.SetValue(p, '_');
          isValidName = false;
        }
        if ( aName.IsIntegerValue() ) { // aName must not start with a digit
          aName.Insert( 1, 'a' );
          isValidName = false;
        }
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
  TCollection_AsciiString initPart = "import salome, SMESH\n";
  initPart += helper + "import " + aSmeshpy + "\n\n";
  if ( importGeom )
  {
    initPart += ("## import GEOM dump file ## \n"
                 "import string, os, sys, re\n"
                 "sys.path.insert( 0, os.path.dirname(__file__) )\n"
                 "exec(\"from \"+re.sub(\"SMESH$\",\"GEOM\",__name__)+\" import *\")\n\n");
  }
  anUpdatedScript.Insert ( 1, initPart );

  // add final part of aScript
  if (aSeq->Value(aLen) < aScriptLength)
    anUpdatedScript += aScript.SubString(aSeq->Value(aLen) + 1, aScriptLength);

  // Remove removed objects
  if ( seqRemoved.Length() > 0 ) {
    anUpdatedScript += "\n\t## some objects were removed";
    anUpdatedScript += "\n\taStudyBuilder = theStudy.NewBuilder()";
  }
  for (int ir = 1; ir <= seqRemoved.Length(); ir++) {
    anUpdatedScript += "\n\tSO = theStudy.FindObjectIOR(theStudy.ConvertObjectToIOR(";
    anUpdatedScript += seqRemoved.Value(ir);
    anUpdatedScript += "))\n\tif SO is not None: aStudyBuilder.RemoveObjectWithChildren(SO)";
  }

  // Set object names
  anUpdatedScript += "\n\t## set object names";
  anUpdatedScript += helper + "  \n\tisGUIMode = " + isPublished;
  anUpdatedScript += "\n\tif isGUIMode and salome.sg.hasDesktop():";
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
      anUpdatedScript += helper + "\n\t\t" + aSmeshpy + ".SetName(" + aName;
      if ( anEntry2AccessorMethod.IsBound( anEntry ) )
        anUpdatedScript += helper + "." + anEntry2AccessorMethod( anEntry );
      anUpdatedScript += helper + ", '" + aGUIName + "')";
    }
  }
  anUpdatedScript += "\n\n\t\tsalome.sg.updateObjBrowser(0)";

  anUpdatedScript += "\n\n\tpass\n";

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
