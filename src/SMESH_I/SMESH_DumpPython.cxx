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
//  File    : SMESH_DumpPython.cxx
//  Created : Thu Mar 24 17:17:59 2005
//  Author  : Julia DOROVSKIKH
//  Module  : SMESH

#include "SMESH_PythonDump.hxx"

#include "SMESH_2smeshpy.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_MeshEditor_i.hxx"

#include <SALOMEDS_wrap.hxx>

#include <LDOMParser.hxx>
#include <Resource_DataMapIteratorOfDataMapOfAsciiStringAsciiString.hxx>
#include <TColStd_HSequenceOfInteger.hxx>
#include <TCollection_AsciiString.hxx>

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

#include "SMESH_TryCatch.hxx"

namespace SMESH
{

  size_t TPythonDump::myCounter = 0;
  const char theNotPublishedObjectName[] = "__NOT__Published__Object__";

  TVar::TVar(CORBA::Double value):myVals(1), myIsList(false) { myVals[0] = SMESH_Comment(value); }
  TVar::TVar(CORBA::Long   value):myVals(1), myIsList(false) { myVals[0] = SMESH_Comment(value); }
  TVar::TVar(CORBA::Short  value):myVals(1), myIsList(false) { myVals[0] = SMESH_Comment(value); }
  TVar::TVar(const SMESH::double_array& value):myVals(value.length()), myIsList(true)
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
      SALOMEDS::Study_var aStudy = aSMESHGen->GetCurrentStudy();
      if(!aStudy->_is_nil() && !aCollection.IsEmpty())
      {
        const std::string & objEntry = SMESH_Gen_i::GetSMESHGen()->GetLastObjEntry();
        if ( !objEntry.empty() )
          aCollection += (TVar::ObjPrefix() + objEntry ).c_str();
        aSMESHGen->AddToPythonScript(aStudy->StudyId(),aCollection);
        if(MYDEBUG) MESSAGE(aString);
        // prevent misuse of already treated variables
        aSMESHGen->UpdateParameters(CORBA::Object_var().in(),"");
      }
    }
  }

  TPythonDump& //!< store a variable value. Write either a value or '$varID$'
  TPythonDump::
  operator<<(const TVar& theVarValue)
  {
    const std::vector< int >& varIDs = SMESH_Gen_i::GetSMESHGen()->GetLastParamIndices();
    if ( theVarValue.myIsList )
    {
      myStream << "[ ";
      for ( size_t i = 1; i <= theVarValue.myVals.size(); ++i )
      {
        if ( myVarsCounter < (int)varIDs.size() && varIDs[ myVarsCounter ] >= 0 )
          myStream << TVar::Quote() << varIDs[ myVarsCounter ] << TVar::Quote();
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
      if ( myVarsCounter < (int)varIDs.size() && varIDs[ myVarsCounter ] >= 0 )
        myStream << TVar::Quote() << varIDs[ myVarsCounter ] << TVar::Quote();
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
  operator<<(const std::string& theArg){
    myStream<<theArg;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(const SMESH::ElementType& theArg)
  {
    myStream<<"SMESH.";
    switch(theArg){
    case ALL:    myStream<<"ALL";    break;
    case NODE:   myStream<<"NODE";   break;
    case EDGE:   myStream<<"EDGE";   break;
    case FACE:   myStream<<"FACE";   break;
    case VOLUME: myStream<<"VOLUME"; break;
    case ELEM0D: myStream<<"ELEM0D"; break;
    case BALL:   myStream<<"BALL";   break;
    default:     myStream<<"__UNKNOWN__ElementType: " << theArg;
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
    case Geom_BALL:       myStream<<"Geom_BALL";       break;
    default:    myStream<<"__UNKNOWN__GeometryType: " << theArg;
    }
    return *this;
  }
  TPythonDump&
  TPythonDump::
  operator<<(const SMESH::EntityType& theArg)
  {
    myStream<<"SMESH.";
    switch(theArg){
    case Entity_0D:                myStream<<"Entity_0D";                break;
    case Entity_Edge:              myStream<<"Entity_Edge";              break;
    case Entity_Quad_Edge:         myStream<<"Entity_Quad_Edge";         break;
    case Entity_Triangle:          myStream<<"Entity_Triangle";          break;
    case Entity_Quad_Triangle:     myStream<<"Entity_Quad_Triangle";     break;
    case Entity_BiQuad_Triangle:   myStream<<"Entity_BiQuad_Triangle";   break;
    case Entity_Quadrangle:        myStream<<"Entity_Quadrangle";        break;
    case Entity_Quad_Quadrangle:   myStream<<"Entity_Quad_Quadrangle";   break;
    case Entity_BiQuad_Quadrangle: myStream<<"Entity_BiQuad_Quadrangle"; break;
    case Entity_Polygon:           myStream<<"Entity_Polygon";           break;
    case Entity_Quad_Polygon:      myStream<<"Entity_Quad_Polygon";      break;
    case Entity_Tetra:             myStream<<"Entity_Tetra";             break;
    case Entity_Quad_Tetra:        myStream<<"Entity_Quad_Tetra";        break;
    case Entity_Pyramid:           myStream<<"Entity_Pyramid";           break;
    case Entity_Quad_Pyramid:      myStream<<"Entity_Quad_Pyramid";      break;
    case Entity_Hexa:              myStream<<"Entity_Hexa";              break;
    case Entity_Quad_Hexa:         myStream<<"Entity_Quad_Hexa";         break;
    case Entity_TriQuad_Hexa:      myStream<<"Entity_TriQuad_Hexa";      break;
    case Entity_Penta:             myStream<<"Entity_Penta";             break;
    case Entity_Quad_Penta:        myStream<<"Entity_Quad_Penta";        break;
    case Entity_Hexagonal_Prism:   myStream<<"Entity_Hexagonal_Prism";   break;
    case Entity_Polyhedra:         myStream<<"Entity_Polyhedra";         break;
    case Entity_Quad_Polyhedra:    myStream<<"Entity_Quad_Polyhedra";    break;
    case Entity_Ball:              myStream<<"Entity_Ball";              break;
    case Entity_Last:              myStream<<"Entity_Last";              break;
    default:    myStream<<"__UNKNOWN__EntityType: " << theArg;
    }
    return *this;
  }

  template<class TArray>
  void DumpArray(const TArray& theArray, TPythonDump & theStream)
  {
    if ( theArray.length() == 0 )
    {
      theStream << "[]";
    }
    else
    {
      theStream << "[ ";
      for (CORBA::ULong i = 1; i <= theArray.length(); i++) {
        theStream << theArray[i-1];
        if ( i < theArray.length() )
          theStream << ", ";
      }
      theStream << " ]";
    }
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
  TPythonDump::operator<<(const SMESH::nodes_array& theArg)
  {
    DumpArray( theArg, *this );
    return *this;
  }

  TPythonDump&
  TPythonDump::operator<<(const SMESH::string_array& theArray)
  {
    myStream << "[ ";
    for ( CORBA::ULong i = 1; i <= theArray.length(); i++ ) {
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
    if ( !aSObject->_is_nil() ) {
      CORBA::String_var entry = aSObject->GetID();
      myStream << entry.in();
    }
    else {
      myStream << theNotPublishedObjectName;
    }
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(CORBA::Object_ptr theArg)
  {
    SMESH_Gen_i*          aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var      aStudy = aSMESHGen->GetCurrentStudy();
    SALOMEDS::SObject_wrap aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(!aSObject->_is_nil()) {
      CORBA::String_var id = aSObject->GetID();
      myStream << id;
    } else if ( !CORBA::is_nil(theArg)) {
      if ( aSMESHGen->CanPublishInStudy( theArg )) // not published SMESH object
        myStream << "smeshObj_" << size_t(theArg);
      else
        myStream << theNotPublishedObjectName;
    }
    else
      myStream << "None";
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::SMESH_Hypothesis_ptr theArg)
  {
    SALOMEDS::Study_var     aStudy = SMESH_Gen_i::GetSMESHGen()->GetCurrentStudy();
    SALOMEDS::SObject_wrap aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(aSObject->_is_nil() && !CORBA::is_nil(theArg))
      myStream << "hyp_" << theArg->GetId();
    else
      *this << aSObject;
    return *this;
  }

  TPythonDump&
  TPythonDump::
  operator<<(SMESH::SMESH_IDSource_ptr theArg)
  {
    if ( CORBA::is_nil( theArg ) )
      return *this << "None";
    SMESH_Gen_i*          aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var      aStudy = aSMESHGen->GetCurrentStudy();
    SALOMEDS::SObject_wrap aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(!aSObject->_is_nil())
    {
      return *this << aSObject;
    }
    if ( SMESH::Filter_i* filter = SMESH::DownCast<SMESH::Filter_i*>( theArg ))
    {
      return *this << filter;
    }
    if ( SMESH_MeshEditor_i::IsTemporaryIDSource( theArg ))
    {
      SMESH::SMESH_Mesh_var            mesh = theArg->GetMesh();
      SMESH::long_array_var    anElementsId = theArg->GetIDs();
      SMESH::array_of_ElementType_var types = theArg->GetTypes();
      SMESH::ElementType               type = types->length() ? types[0] : SMESH::ALL;
      SALOMEDS::SObject_wrap         meshSO = SMESH_Gen_i::ObjectToSObject(aStudy,mesh);
      if ( meshSO->_is_nil() ) // don't waste memory for dumping not published objects
        return *this << mesh << ".GetIDSource([], " << type << ")";
      else
        return *this << mesh << ".GetIDSource(" << anElementsId << ", " << type << ")";
    }
    return *this << theNotPublishedObjectName;
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
      switch(aFunctorType) {
      case FT_AspectRatio:           myStream<< "aAspectRatio";           break;
      case FT_AspectRatio3D:         myStream<< "aAspectRatio3D";         break;
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
      case FT_EqualNodes:            myStream<< "aEqualNodes";            break;
      case FT_EqualEdges:            myStream<< "aEqualEdges";            break;
      case FT_EqualFaces:            myStream<< "aEqualFaces";            break;
      case FT_EqualVolumes:          myStream<< "aEqualVolumes";          break;
      case FT_MultiConnection:       myStream<< "aMultiConnection";       break;
      case FT_MultiConnection2D:     myStream<< "aMultiConnection2D";     break;
      case FT_Length:                myStream<< "aLength";                break;
      case FT_Length2D:              myStream<< "aLength2D";              break;
      case FT_NodeConnectivityNumber:myStream<< "aNodeConnectivityNumber";break;
      case FT_BelongToMeshGroup:     myStream<< "aBelongToMeshGroup";     break;
      case FT_BelongToGeom:          myStream<< "aBelongToGeom";          break;
      case FT_BelongToPlane:         myStream<< "aBelongToPlane";         break;
      case FT_BelongToCylinder:      myStream<< "aBelongToCylinder";      break;
      case FT_BelongToGenSurface:    myStream<< "aBelongToGenSurface";    break;
      case FT_LyingOnGeom:           myStream<< "aLyingOnGeom";           break;
      case FT_RangeOfIds:            myStream<< "aRangeOfIds";            break;
      case FT_BadOrientedVolume:     myStream<< "aBadOrientedVolume";     break;
      case FT_BareBorderVolume:      myStream<< "aBareBorderVolume";      break;
      case FT_BareBorderFace:        myStream<< "aBareBorderFace";        break;
      case FT_OverConstrainedVolume: myStream<< "aOverConstrainedVolume"; break;
      case FT_OverConstrainedFace:   myStream<< "aOverConstrainedFace";   break;
      case FT_LinearOrQuadratic:     myStream<< "aLinearOrQuadratic";     break;
      case FT_GroupColor:            myStream<< "aGroupColor";            break;
      case FT_ElemGeomType:          myStream<< "aElemGeomType";          break;
      case FT_EntityType:            myStream<< "aEntityType";            break;
      case FT_CoplanarFaces:         myStream<< "aCoplanarFaces";         break;
      case FT_BallDiameter:          myStream<< "aBallDiameter";          break;
      case FT_ConnectedElements:     myStream<< "aConnectedElements";     break;
      case FT_LessThan:              myStream<< "aLessThan";              break;
      case FT_MoreThan:              myStream<< "aMoreThan";              break;
      case FT_EqualTo:               myStream<< "aEqualTo";               break;
      case FT_LogicalNOT:            myStream<< "aLogicalNOT";            break;
      case FT_LogicalAND:            myStream<< "aLogicalAND";            break;
      case FT_LogicalOR:             myStream<< "aLogicalOR";             break;
      case FT_Undefined:             myStream<< "anUndefined";            break;
        //default: -- commented to have a compilation warning
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
  TPythonDump& TPythonDump::operator<<(const GEOM::ListOfGO& theList)
  {
    DumpArray( theList, *this );
    return *this;
  }
  TPythonDump& TPythonDump::operator<<(const GEOM::ListOfGBO& theList)
  {
    DumpArray( theList, *this );
    return *this;
  }
  TPythonDump& TPythonDump::operator<<(const SMESH::ListOfIDSources& theList)
  {
    DumpArray( theList, *this );
    return *this;
  }
  TPythonDump& TPythonDump::operator<<(const SMESH::CoincidentFreeBorders& theCFB)
  {
    // dump CoincidentFreeBorders as a list of lists, each enclosed list
    // contains node IDs of a group of coincident free borders where
    // each consequent triple of IDs describe a free border: (n1, n2, nLast)
    // For example [[1, 2, 10, 20, 21, 40], [11, 12, 15, 55, 54, 41]] describes
    // two groups of coincident free borders, each group including two borders

    myStream << "[";
    for ( CORBA::ULong i = 0; i < theCFB.coincidentGroups.length(); ++i )
    {
      const SMESH::FreeBordersGroup& aGRP = theCFB.coincidentGroups[ i ];
      if ( i ) myStream << ",";
      myStream << "[";
      for ( CORBA::ULong iP = 0; iP < aGRP.length(); ++iP )
      {
        const SMESH::FreeBorderPart& aPART = aGRP[ iP ];
        if ( 0 <= aPART.border && aPART.border < (CORBA::Long)theCFB.borders.length() )
        {
          if ( iP ) myStream << ", ";
          const SMESH::FreeBorder& aBRD = theCFB.borders[ aPART.border ];
          myStream << aBRD.nodeIDs[ aPART.node1    ] << ",";
          myStream << aBRD.nodeIDs[ aPART.node2    ] << ",";
          myStream << aBRD.nodeIDs[ aPART.nodeLast ];
        }
      }
      myStream << "]";
    }
    myStream << "]";

    return *this;
  }

  const char* TPythonDump::NotPublishedObjectName()
  {
    return theNotPublishedObjectName;
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

  void printException( const char* text )
  {
#ifdef _DEBUG_
    cout << "Exception in SMESH_Gen_i::DumpPython(): " << text << endl;
#endif
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

  CORBA::String_var compDataType = ComponentDataType();
  SALOMEDS::SObject_wrap aSO = aStudy->FindComponent( compDataType.in() );
  if (CORBA::is_nil(aSO))
    return new Engines::TMPFile(0);

  // Map study entries to object names
  Resource_DataMapOfAsciiStringAsciiString aMap;
  Resource_DataMapOfAsciiStringAsciiString aMapNames;

  SALOMEDS::ChildIterator_wrap Itr = aStudy->NewChildIterator(aSO);
  for (Itr->InitEx(true); Itr->More(); Itr->Next()) {
    SALOMEDS::SObject_wrap aValue = Itr->Value();
    CORBA::String_var anID = aValue->GetID();
    CORBA::String_var aName = aValue->GetName();
    TCollection_AsciiString aGUIName ( (char*) aName.in() );
    TCollection_AsciiString anEntry ( (char*) anID.in() );
    if (aGUIName.Length() > 0) {
      aMapNames.Bind( anEntry, aGUIName );
      aMap.Bind( anEntry, aGUIName );
    }
  }

  // Get trace of restored study
  SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();
  SALOMEDS::GenericAttribute_wrap anAttr =
    aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePythonObject");

  SALOMEDS::AttributePythonObject_var pyAttr =
    SALOMEDS::AttributePythonObject::_narrow(anAttr);
  CORBA::String_var oldValue = pyAttr->GetObject();
  TCollection_AsciiString aSavedTrace (oldValue.in());

  // Add trace of API methods calls and replace study entries by names
  TCollection_AsciiString aScript;
  aScript += DumpPython_impl(aStudy, aMap, aMapNames, isPublished, isMultiFile,
                             myIsHistoricalPythonDump, isValidScript, aSavedTrace);

  int aLen = aScript.Length();
  unsigned char* aBuffer = new unsigned char[aLen+1];
  strcpy((char*)aBuffer, aScript.ToCString());

  CORBA::Octet* anOctetBuf =  (CORBA::Octet*)aBuffer;
  Engines::TMPFile_var aStreamFile = new Engines::TMPFile(aLen+1, aLen+1, anOctetBuf, 1);

  bool hasNotPublishedObjects = aScript.Location( SMESH::theNotPublishedObjectName, 1, aLen);
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
  CORBA::String_var compDataType = ComponentDataType();
  SALOMEDS::SObject_wrap aSO = theStudy->FindComponent( compDataType.in() );
  SALOMEDS::StudyBuilder_var aStudyBuilder = theStudy->NewBuilder();
  SALOMEDS::GenericAttribute_wrap anAttr =
    aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePythonObject");

  SALOMEDS::AttributePythonObject_var pyAttr =
    SALOMEDS::AttributePythonObject::_narrow(anAttr);
  CORBA::String_var oldValue = pyAttr->GetObject();
  TCollection_AsciiString oldScript (oldValue.in());

  if (oldScript.Length() > 0) {
    oldScript += "\n";
    oldScript += aScript;
  } else {
    oldScript = aScript;
  }

  // Store in PythonObject attribute
  pyAttr->SetObject(oldScript.ToCString(), 1);

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

  bool fixPythonName(TCollection_AsciiString & aName)
  {
    bool isValidName = true;
    int nbUnderscore = 0;
    int p;
    // replace not allowed chars by underscore
    const char* name = aName.ToCString();
    for ( p = 0; name[p]; ++p ) {
      if ( !isalnum( name[p] ) && name[p] != '_' )
      {
        if ( p == 0 || p+1 == aName.Length() || name[p-1] == '_')
        {
          aName.Remove( p+1, 1 ); // remove __ and _ from the start and the end
          --p;
          name = aName.ToCString();
        }
        else
        {
          aName.SetValue( p+1, '_');
          nbUnderscore++;
        }
        isValidName = false;
      }
    }
    // aName must not start with a digit
    if ( aName.IsIntegerValue() ) {
      aName.Insert( 1, 'a' );
      isValidName = false;
    }
    // shorten names like CartesianParameters3D_400_400_400_1000000_1
    const int nbAllowedUnderscore = 3; /* changed from 2 to 3 by an user request
                                          posted to SALOME Forum */
    if ( aName.Length() > 20 && nbUnderscore > nbAllowedUnderscore )
    {
      p = aName.Location( "_", 20, aName.Length());
      if ( p > 1 )
        aName.Trunc( p-1 );
    }
    return isValidName;
  }

  //================================================================================
  /*!
   * \brief Return Python module names of available plug-ins.
   */
  //================================================================================

  std::vector<std::string> getPluginNames()
  {
    std::vector<std::string> pluginNames;
    std::vector< std::string > xmlPaths = SMESH_Gen::GetPluginXMLPaths();
    LDOMParser xmlParser;
    for ( size_t i = 0; i < xmlPaths.size(); ++i )
    {
      bool error = xmlParser.parse( xmlPaths[i].c_str() );
      if ( error )
      {
        TCollection_AsciiString data;
        INFOS( xmlParser.GetError(data) );
        continue;
      }
      // <meshers-group name="Standard Meshers"
      //                resources="StdMeshers"
      //                idl-module="StdMeshers"
      //                server-lib="StdMeshersEngine"
      //                gui-lib="StdMeshersGUI">
      LDOM_Document xmlDoc   = xmlParser.getDocument();
      LDOM_NodeList nodeList = xmlDoc.getElementsByTagName( "meshers-group" );
      for ( int i = 0; i < nodeList.getLength(); ++i )
      {
        LDOM_Node       node = nodeList.item( i );
        LDOM_Element&   elem = (LDOM_Element&) node;
        LDOMString idlModule = elem.getAttribute( "idl-module" );
        if ( strlen( idlModule.GetString() ) > 0 )
          pluginNames.push_back( idlModule.GetString() );
      }
    }
    return pluginNames;
  }
}

//================================================================================
/*!
 * \brief Createa a Dump Python script
 *  \param [in] theStudy - the study to dump
 *  \param [in,out] theObjectNames - map of an entry to a study and python name
 *  \param [in] theNames -  - map of an entry to a study name
 *  \param [in] isPublished - \c true if dump of object publication in study is needed
 *  \param [in] isMultiFile - \c true if dump of each module goes to a separate file
 *  \param [in] isHistoricalDump - \c true if removed object should be dumped
 *  \param [out] aValidScript - returns \c true if the returned script seems valid
 *  \param [in,out] theSavedTrace - the dump stored in the study. It's cleared to
 *         decrease memory usage.
 *  \return TCollection_AsciiString - the result dump script.
 */
//================================================================================

TCollection_AsciiString SMESH_Gen_i::DumpPython_impl
                        (SALOMEDS::Study_ptr                       theStudy,
                         Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
                         Resource_DataMapOfAsciiStringAsciiString& theNames,
                         bool                                      isPublished,
                         bool                                      isMultiFile,
                         bool                                      isHistoricalDump,
                         bool&                                     aValidScript,
                         TCollection_AsciiString&                  theSavedTrace)
{
  SMESH_TRY;
  const int aStudyID = theStudy->StudyId();

  const TCollection_AsciiString aSmeshpy ( SMESH_2smeshpy::SmeshpyName() );
  const TCollection_AsciiString aSMESHGen( SMESH_2smeshpy::GenName() );
  const TCollection_AsciiString anOldGen ( SMESH::TPythonDump::SMESHGenName() );
  const TCollection_AsciiString helper; // to comfortably append C strings to TCollection_AsciiString
  const TCollection_AsciiString tab( isMultiFile ? "\t" : "" ), nt = helper + "\n" + tab;

  std::list< TCollection_AsciiString > lines; // lines of a script
  std::list< TCollection_AsciiString >::iterator linesIt;
  
  if ( isPublished )
    lines.push_back(  aSMESHGen + " = smeshBuilder.New(theStudy)" );
   else
    lines.push_back(  aSMESHGen + " = smeshBuilder.New(None)" );
  lines.push_back( helper + "aFilterManager = " + aSMESHGen + ".CreateFilterManager()" );
  lines.push_back( helper + "aMeasurements = "  + aSMESHGen + ".CreateMeasurements()" );

  // Treat dump trace of restored study
  if (theSavedTrace.Length() > 0)
  {
    linesIt = --lines.end();
    // Split theSavedTrace into lines
    int from = 1, end = theSavedTrace.Length(), to;
    while ( from < end && ( to = theSavedTrace.Location( "\n", from, end )))
    {
      if ( theSavedTrace.ToCString()[from-1] == '\t' )
        ++from;
      if ( to != from )
        lines.push_back( theSavedTrace.SubString( from, to - 1 ));
      from = to + 1;
    }
    // For the convertion of IDL API calls -> smeshBuilder.py API, "smesh" standing for SMESH_Gen
    // was replaces with "smeshgen" (==TPythonDump::SMESHGenName()).
    // Change "smesh" -> "smeshgen" in the trace saved before passage to smeshBuilder.py API
    bool isNewVersion =
      theSavedTrace.Location( anOldGen + ".", 1, theSavedTrace.Length() );
    theSavedTrace.Clear();
    if ( !isNewVersion )
    {
      const TCollection_AsciiString aSmeshCall ( "smesh." ), gen( "gen" );
      int beg, end, from;
      for ( ++linesIt; linesIt != lines.end(); ++linesIt )
      {
        TCollection_AsciiString& aSavedLine = *linesIt;
        end = aSavedLine.Length(), from = 1;
        while ( from < end && ( beg = aSavedLine.Location( aSmeshCall, from, end )))
        {
          char charBefore = ( beg == 1 ) ? ' ' : aSavedLine.Value( beg - 1 );
          if ( isspace( charBefore ) || charBefore == '=' ) { // "smesh." is not a part of a long word
            aSavedLine.Insert( beg + aSmeshCall.Length() - 1, gen );// "smesh" -> "smeshgen"
            end += gen.Length();
          }
          from = beg + aSmeshCall.Length();
        }
      }
    }
  }

  // Add new dump trace of API methods calls to script lines
  if (myPythonScripts.find( aStudyID ) != myPythonScripts.end())
  {
    Handle(TColStd_HSequenceOfAsciiString) aPythonScript = myPythonScripts[ aStudyID ];
    Standard_Integer istr, aLen = aPythonScript->Length();
    for (istr = 1; istr <= aLen; istr++)
      lines.push_back( aPythonScript->Value( istr ));
  }

  // Convert IDL API calls into smeshBuilder.py API.
  // Some objects are wrapped with python classes and
  // Resource_DataMapOfAsciiStringAsciiString holds methods returning wrapped objects
  Resource_DataMapOfAsciiStringAsciiString anEntry2AccessorMethod;
  std::set< TCollection_AsciiString >      aRemovedObjIDs;
  if ( !getenv("NO_2smeshpy_conversion"))
    SMESH_2smeshpy::ConvertScript( lines, anEntry2AccessorMethod,
                                   theObjectNames, aRemovedObjIDs,
                                   theStudy, isHistoricalDump );

  bool importGeom = false;
  GEOM::GEOM_Gen_ptr geom = GetGeomEngine();
  {
    // Add names of GEOM objects to theObjectNames to exclude same names of SMESH objects
    GEOM::string_array_var aGeomNames = geom->GetAllDumpNames();
    int ign = 0, nbgn = aGeomNames->length();
    for (; ign < nbgn; ign++) {
      TCollection_AsciiString aName = aGeomNames[ign].in();
      theObjectNames.Bind(aName, "1");
    }
  }

  TCollection_AsciiString anUpdatedScript;

  Resource_DataMapOfAsciiStringAsciiString mapRemoved;
  Resource_DataMapOfAsciiStringAsciiString mapEntries; // names and entries present in anUpdatedScript
  Standard_Integer objectCounter = 0;
  TCollection_AsciiString anEntry, aName, aGUIName, aBaseName("smeshObj_");

  // Treat every script line and add it to anUpdatedScript
  for ( linesIt = lines.begin(); linesIt != lines.end(); ++linesIt )
  {
    TCollection_AsciiString& aLine = *linesIt;
    anUpdatedScript += tab;
    {
      //Replace characters used instead of quote marks to quote notebook variables
      int pos = 1;
      while (( pos = aLine.Location( 1, SMESH::TVar::Quote(), pos, aLine.Length() )))
        aLine.SetValue( pos, '"' );
    }
    // Find entries to be replaced by names
    Handle(TColStd_HSequenceOfInteger) aSeq = FindEntries(aLine);
    const Standard_Integer aSeqLen = aSeq->Length();
    Standard_Integer aStart = 1;
    for (Standard_Integer i = 1; i <= aSeqLen; i += 2)
    {
      if ( aStart < aSeq->Value(i) )
        anUpdatedScript += aLine.SubString( aStart, aSeq->Value(i) - 1 ); // line part before i-th entry
      anEntry = aLine.SubString( aSeq->Value(i), aSeq->Value(i + 1) );
      // is a GEOM object?
      CORBA::String_var geomName = geom->GetDumpName( anEntry.ToCString() );
      if ( !geomName.in() || !geomName.in()[0] ) {
        // is a SMESH object
        if ( theObjectNames.IsBound( anEntry )) {
          // The Object is in Study
          aName = theObjectNames.Find( anEntry );
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

          if ( aLine.Value(1) != '#' )
            mapEntries.Bind(anEntry, aName);
        }
        else
        {
          // Removed Object
          do {
            aName = aBaseName + (++objectCounter);
          } while (theObjectNames.IsBound(aName));

          if ( !aRemovedObjIDs.count( anEntry ) && aLine.Value(1) != '#')
            mapRemoved.Bind(anEntry, aName);

          theObjectNames.Bind(anEntry, aName);
        }
        theObjectNames.Bind(aName, anEntry); // to detect same name of diff objects
      }
      else
      {
        aName = geomName.in();
        importGeom = true;
      }
      anUpdatedScript += aName;
      aStart = aSeq->Value(i + 1) + 1;

    } // loop on entries within aLine

    if ( aSeqLen == 0 )
      anUpdatedScript += aLine;
    else if ( aSeq->Value( aSeqLen ) < aLine.Length() )
      anUpdatedScript += aLine.SubString( aSeq->Value(aSeqLen) + 1, aLine.Length() );

    anUpdatedScript += '\n';
  }

  // Make an initial part of aSript

  TCollection_AsciiString initPart = "import ";
  if ( isMultiFile )
    initPart += "salome, ";
  initPart += " SMESH, SALOMEDS\n";
  initPart += "from salome.smesh import smeshBuilder\n";
  if ( importGeom && isMultiFile )
  {
    initPart += ("\n## import GEOM dump file ## \n"
                 "import string, os, sys, re, inspect\n"
                 "thisFile   = inspect.getfile( inspect.currentframe() )\n"
                 "thisModule = os.path.splitext( os.path.basename( thisFile ))[0]\n"
                 "sys.path.insert( 0, os.path.dirname( thisFile ))\n"
                 "exec(\"from \"+re.sub(\"SMESH$\",\"GEOM\",thisModule)+\" import *\")\n\n");
  }
  // import python files corresponding to plugins if they are used in anUpdatedScript
  {
    TCollection_AsciiString importStr;
    std::vector<std::string> pluginNames = getPluginNames();
    for ( size_t i = 0; i < pluginNames.size(); ++i )
    {
      // Convert access to plugin members:
      // e.g. StdMeshers.QUAD_REDUCED -> StdMeshersBuilder.QUAD_REDUCED
      TCollection_AsciiString pluginAccess = (pluginNames[i] + ".").c_str() ;
      int iFrom = 1, iPos;
      while (( iPos = anUpdatedScript.Location( pluginAccess, iFrom, anUpdatedScript.Length() )))
      {
        anUpdatedScript.Insert( iPos + pluginNames[i].size(), "Builder" );
        iFrom = iPos + pluginNames[i].size() + 8;
      }
      // if any plugin member is used, import the plugin
      if ( iFrom > 1 )
        importStr += ( helper + "\n" "from salome." + pluginNames[i].c_str() +
                       " import " + pluginNames[i].c_str() +"Builder" );
    }
    if ( !importStr.IsEmpty() )
      initPart += importStr + "\n";
  }

  if ( isMultiFile )
    initPart += "def RebuildData(theStudy):";
  initPart += "\n";

  anUpdatedScript.Prepend( initPart );

  // Make a final part of aScript

  // Dump object removal
  TCollection_AsciiString removeObjPart;
  if ( !mapRemoved.IsEmpty() ) {
    removeObjPart += nt + "## some objects were removed";
    removeObjPart += nt + "aStudyBuilder = theStudy.NewBuilder()";
    Resource_DataMapIteratorOfDataMapOfAsciiStringAsciiString mapRemovedIt;
    for ( mapRemovedIt.Initialize( mapRemoved ); mapRemovedIt.More(); mapRemovedIt.Next() ) {
      aName   = mapRemovedIt.Value(); // python name
      anEntry = mapRemovedIt.Key();
      removeObjPart += nt + "SO = theStudy.FindObjectIOR(theStudy.ConvertObjectToIOR(";
      removeObjPart += aName;
      // for object wrapped by class of smeshBuilder.py
      if ( anEntry2AccessorMethod.IsBound( anEntry ) )
        removeObjPart += helper + "." + anEntry2AccessorMethod( anEntry );
      removeObjPart += helper + "))" + nt + "if SO: aStudyBuilder.RemoveObjectWithChildren(SO)";
    }
  }

  // Set object names
  TCollection_AsciiString setNamePart;
  Resource_DataMapIteratorOfDataMapOfAsciiStringAsciiString mapEntriesIt;
  for ( mapEntriesIt.Initialize( mapEntries ); mapEntriesIt.More(); mapEntriesIt.Next() )
  {
    anEntry = mapEntriesIt.Key();
    aName   = mapEntriesIt.Value(); // python name
    if ( theNames.IsBound( anEntry ))
    {
      aGUIName = theNames.Find(anEntry);
      aGUIName.RemoveAll('\''); // remove a quote from a name (issue 22360)
      setNamePart += nt + aSMESHGen + ".SetName(" + aName;
      if ( anEntry2AccessorMethod.IsBound( anEntry ) )
        setNamePart += helper + "." + anEntry2AccessorMethod( anEntry );
      setNamePart += helper + ", '" + aGUIName + "')";
    }
  }
  if ( !setNamePart.IsEmpty() )
  {
    setNamePart.Insert( 1, nt + "## Set names of Mesh objects" );
  }

  // Store visual properties of displayed objects

  TCollection_AsciiString visualPropertiesPart;
  if (isPublished)
  {
    //Output the script that sets up the visual parameters.
    CORBA::String_var compDataType = ComponentDataType();
    CORBA::String_var script = theStudy->GetDefaultScript( compDataType.in(), tab.ToCString() );
    if ( script.in() && script.in()[0] ) {
      visualPropertiesPart += nt + "### Store presentation parameters of displayed objects\n";
      visualPropertiesPart += script.in();
    }
  }

  anUpdatedScript += removeObjPart + '\n' + setNamePart + '\n' + visualPropertiesPart;

  if ( isMultiFile )
  {
    anUpdatedScript +=
      "\n\tpass"
      "\n"
      "\nif __name__ == '__main__':"
      "\n\tSMESH_RebuildData = RebuildData"
      "\n\texec('import '+re.sub('SMESH$','GEOM',thisModule)+' as GEOM_dump')"
      "\n\tGEOM_dump.RebuildData( salome.myStudy )"
      "\n\texec('from '+re.sub('SMESH$','GEOM',thisModule)+' import * ')"
      "\n\tSMESH_RebuildData( salome.myStudy )";
  }
  anUpdatedScript += "\n";

  // no need now as we use 'tab' and 'nt' variables depending on isMultiFile
  // if( !isMultiFile ) // remove unnecessary tabulation
  //   RemoveTabulation( anUpdatedScript );

  // -----------------------------------------------------------------
  // put string literals describing patterns into separate functions
  // -----------------------------------------------------------------

  TCollection_AsciiString aLongString, aFunctionType;
  int where = 1;
  std::set< std::string > functionNameSet;
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

  SMESH_CATCH( SMESH::printException );

  aValidScript = false;
  return "";
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
      aScript += "\n";
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
