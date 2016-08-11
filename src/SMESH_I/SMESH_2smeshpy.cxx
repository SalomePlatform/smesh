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

// File      : SMESH_2smeshpy.cxx
// Created   : Fri Nov 18 13:20:10 2005
// Author    : Edward AGAPOV (eap)
//
#include "SMESH_2smeshpy.hxx"

#include "SMESH_PythonDump.hxx"
#include "SMESH_NoteBook.hxx"
#include "SMESH_Filter_i.hxx"

#include <SALOMEDS_wrap.hxx>
#include <utilities.h>

#include <Resource_DataMapOfAsciiStringAsciiString.hxx>
#include <Resource_DataMapIteratorOfDataMapOfAsciiStringAsciiString.hxx>

#include "SMESH_Gen_i.hxx"
/* SALOME headers that include CORBA headers that include windows.h
 * that defines GetObject symbol as GetObjectA should stand before SALOME headers
 * that declare methods named GetObject - to apply the same rules of GetObject renaming
 * and thus to avoid mess with GetObject symbol on Windows */

#include <LDOMParser.hxx>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyObject          ,Standard_Transient);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyCommand         ,Standard_Transient);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyHypothesisReader,Standard_Transient);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyGen             ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyMesh            ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pySubMesh         ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyMeshEditor      ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyHypothesis      ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pySelfEraser      ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyGroup           ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyFilter          ,_pyObject);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyAlgorithm       ,_pyHypothesis);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyComplexParamHypo,_pyHypothesis);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyNumberOfSegmentsHyp,_pyHypothesis);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pyLayerDistributionHypo,_pyHypothesis);
OCCT_IMPLEMENT_STANDARD_RTTIEXT(_pySegmentLengthAroundVertexHyp,_pyHypothesis);

using namespace std;
using SMESH::TPythonDump;

/*!
 * \brief Container of commands into which the initial script is split.
 *        It also contains data coresponding to SMESH_Gen contents
 */
static Handle(_pyGen) theGen;

static TCollection_AsciiString theEmptyString;

//#define DUMP_CONVERSION

#if !defined(_DEBUG_) && defined(DUMP_CONVERSION)
#undef DUMP_CONVERSION
#endif


namespace {

  //================================================================================
  /*!
   * \brief Set of TCollection_AsciiString initialized by C array of C strings
   */
  //================================================================================

  struct TStringSet: public set<TCollection_AsciiString>
  {
    /*!
     * \brief Filling. The last string must be ""
     */
    void Insert(const char* names[]) {
      for ( int i = 0; names[i][0] ; ++i )
        insert( (char*) names[i] );
    }
    /*!
     * \brief Check if a string is in
     */
    bool Contains(const TCollection_AsciiString& name ) {
      return find( name ) != end();
    }
  };

  //================================================================================
  /*!
   * \brief Map of TCollection_AsciiString initialized by C array of C strings.
   *        Odd items of the C array are map keys, and even items are values
   */
  //================================================================================

  struct TStringMap: public map<TCollection_AsciiString,TCollection_AsciiString>
  {
    /*!
     * \brief Filling. The last string must be ""
     */
    void Insert(const char* names_values[]) {
      for ( int i = 0; names_values[i][0] ; i += 2 )
        insert( make_pair( (char*) names_values[i], names_values[i+1] ));
    }
    /*!
     * \brief Check if a string is in
     */
    TCollection_AsciiString Value(const TCollection_AsciiString& name ) {
      map< _AString, _AString >::iterator it = find( name );
      return it == end() ? "" : it->second;
    }
  };

  //================================================================================
  /*!
   * \brief Returns a mesh by object
   */
  //================================================================================

  Handle(_pyMesh) ObjectToMesh( const Handle( _pyObject )& obj )
  {
    if ( !obj.IsNull() )
    {
      if ( obj->IsKind( STANDARD_TYPE( _pyMesh )))
        return Handle(_pyMesh)::DownCast( obj );
      else if ( obj->IsKind( STANDARD_TYPE( _pySubMesh )))
        return Handle(_pySubMesh)::DownCast( obj )->GetMesh();
      else if ( obj->IsKind( STANDARD_TYPE( _pyGroup )))
        return Handle(_pyGroup)::DownCast( obj )->GetMesh();
    }
    return Handle(_pyMesh)();
  }

  //================================================================================
  /*!
   * \brief Check if objects used as args have been created by previous commands
   */
  //================================================================================

  void CheckObjectPresence( const Handle(_pyCommand)& cmd, set<_pyID> & presentObjects)
  {
    // either comment or erase a command including NotPublishedObjectName()
    if ( cmd->GetString().Location( TPythonDump::NotPublishedObjectName(), 1, cmd->Length() ))
    {
      bool isResultPublished = false;
      const int nbRes = cmd->GetNbResultValues();
      for ( int i = 0; i < nbRes; i++ )
      {
        _pyID objID = cmd->GetResultValue( i+1 );
        if ( cmd->IsStudyEntry( objID ))
          isResultPublished = (! theGen->IsNotPublished( objID ));
        theGen->ObjectCreationRemoved( objID ); // objID.SetName( name ) is not needed
      }
      if ( isResultPublished )
        cmd->Comment();
      else
        cmd->Clear();
      return;
    }
    // check if an Object was created in the script
    _AString comment;

    _pyID obj = cmd->GetObject();
    if ( obj.Search( "print " ) == 1 )
      return; // print statement

    if ( !obj.IsEmpty() && obj.Value( obj.Length() ) == ')' )
      // remove an accessor method
      obj = _pyCommand( obj ).GetObject();

    const bool isMethodCall = cmd->IsMethodCall();
    if ( !obj.IsEmpty() && isMethodCall && !presentObjects.count( obj ) )
    {
      comment = "not created Object";
      theGen->ObjectCreationRemoved( obj );
    }
    // check if a command has not created args
    for ( int iArg = cmd->GetNbArgs(); iArg && comment.IsEmpty(); --iArg )
    {
      const _pyID& arg = cmd->GetArg( iArg );
      if ( arg.IsEmpty() || arg.Value( 1 ) == '"' || arg.Value( 1 ) == '\'' )
        continue;
      list< _pyID > idList = cmd->GetStudyEntries( arg );
      list< _pyID >::iterator id = idList.begin();
      for ( ; id != idList.end(); ++id )
        if ( !theGen->IsGeomObject( *id ) && !presentObjects.count( *id ))
        {
          comment += *id + " has not been yet created";
          break;
        }
      // if ( idList.empty() && cmd->IsID( arg ) && !presentObjects.count( arg ))
      //   comment += arg + " has not been yet created";
    }
    // treat result objects
    const _pyID& result = cmd->GetResultValue();
    if ( !result.IsEmpty() && result.Value( 1 ) != '"' && result.Value( 1 ) != '\'' )
    {
      list< _pyID > idList = cmd->GetStudyEntries( result );
      list< _pyID >::iterator id = idList.begin();
      for ( ; id != idList.end(); ++id )
      {
        if ( comment.IsEmpty() )
          presentObjects.insert( *id );
        else
          theGen->ObjectCreationRemoved( *id ); // objID.SetName( name ) is not needed
      }
      if ( idList.empty() && cmd->IsID( result ))
        presentObjects.insert( result );
    }
    // comment the command
    if ( !comment.IsEmpty() )
    {
      cmd->Comment();
      cmd->GetString() += " ### ";
      cmd->GetString() += comment;
    }
  }

  //================================================================================
  /*!
   * \brief Fix SMESH::FunctorType arguments of SMESH::Filter::Criterion()
   */
  //================================================================================

  void fixFunctorType( TCollection_AsciiString& Type,
                       TCollection_AsciiString& Compare,
                       TCollection_AsciiString& UnaryOp,
                       TCollection_AsciiString& BinaryOp )
  {
    // The problem is that dumps of old studies created using filters becomes invalid
    // when new items are inserted in the enum SMESH::FunctorType since values
    // of this enum are dumped as integer values.
    // This function corrects enum values of old studies given as args (Type,Compare,...)
    // We can find out how to correct them by value of BinaryOp which can have only two
    // values: FT_Undefined or FT_LogicalNOT.
    // Hereafter is the history of the enum SMESH::FunctorType since v3.0.0
    // where PythonDump appeared
    // v 3.0.0: FT_Undefined == 25
    // v 3.1.0: FT_Undefined == 26, new items:
    //   - FT_Volume3D              = 7
    // v 4.1.2: FT_Undefined == 27, new items:
    //   - FT_BelongToGenSurface    = 17
    // v 5.1.1: FT_Undefined == 32, new items:
    //   - FT_FreeNodes             = 10
    //   - FT_FreeFaces             = 11
    //   - FT_LinearOrQuadratic     = 23
    //   - FT_GroupColor            = 24
    //   - FT_ElemGeomType          = 25
    // v 5.1.5: FT_Undefined == 33, new items:
    //   - FT_CoplanarFaces         = 26
    // v 6.2.0: FT_Undefined == 39, new items:
    //   - FT_MaxElementLength2D    = 8
    //   - FT_MaxElementLength3D    = 9
    //   - FT_BareBorderVolume      = 25
    //   - FT_BareBorderFace        = 26
    //   - FT_OverConstrainedVolume = 27
    //   - FT_OverConstrainedFace   = 28
    // v 6.5.0: FT_Undefined == 43, new items:
    //   - FT_EqualNodes            = 14
    //   - FT_EqualEdges            = 15
    //   - FT_EqualFaces            = 16
    //   - FT_EqualVolumes          = 17
    // v 6.6.0: FT_Undefined == 44, new items:
    //   - FT_BallDiameter          = 37
    // v 6.7.1: FT_Undefined == 45, new items:
    //   - FT_EntityType            = 36
    // v 7.3.0: FT_Undefined == 46, new items:
    //   - FT_ConnectedElements     = 39
    // v 7.6.0: FT_Undefined == 47, new items:
    //   - FT_BelongToMeshGroup     = 22
    // v 8.1.0: FT_Undefined == 48, new items:
    //   - FT_NodeConnectivityNumber= 22
    //
    // It's necessary to continue recording this history and to fill
    // undef2newItems (see below) accordingly.

    typedef map< int, vector< int > > TUndef2newItems;
    static TUndef2newItems undef2newItems;
    if ( undef2newItems.empty() )
    {
      undef2newItems[ 26 ].push_back( 7 );
      undef2newItems[ 27 ].push_back( 17 );
      { int items[] = { 10, 11, 23, 24, 25 };
        undef2newItems[ 32 ].assign( items, items+5 ); }
      undef2newItems[ 33 ].push_back( 26 );
      { int items[] = { 8, 9, 25, 26, 27, 28 };
        undef2newItems[ 39 ].assign( items, items+6 ); }
      { int items[] = { 14, 15, 16, 17 };
        undef2newItems[ 43 ].assign( items, items+4 ); }
      undef2newItems[ 44 ].push_back( 37 );
      undef2newItems[ 45 ].push_back( 36 );
      undef2newItems[ 46 ].push_back( 39 );
      undef2newItems[ 47 ].push_back( 22 );
      undef2newItems[ 48 ].push_back( 22 );

      ASSERT( undef2newItems.rbegin()->first == SMESH::FT_Undefined );
    }

    int iType     = Type.IntegerValue();
    int iCompare  = Compare.IntegerValue();
    int iUnaryOp  = UnaryOp.IntegerValue();
    int iBinaryOp = BinaryOp.IntegerValue();

    // find out integer value of FT_Undefined at the moment of dump
    int oldUndefined = iBinaryOp;
    if ( iBinaryOp < iUnaryOp ) // BinaryOp was FT_LogicalNOT
      oldUndefined += 3;

    // apply history to args
    TUndef2newItems::const_iterator undef_items =
      undef2newItems.upper_bound( oldUndefined );
    if ( undef_items != undef2newItems.end() )
    {
      int* pArg[4] = { &iType, &iCompare, &iUnaryOp, &iBinaryOp };
      for ( ; undef_items != undef2newItems.end(); ++undef_items )
      {
        const vector< int > & addedItems = undef_items->second;
        for ( size_t i = 0; i < addedItems.size(); ++i )
          for ( int iArg = 0; iArg < 4; ++iArg )
          {
            int& arg = *pArg[iArg];
            if ( arg >= addedItems[i] )
              arg++;
          }
      }
      Type     = TCollection_AsciiString( iType     );
      Compare  = TCollection_AsciiString( iCompare  );
      UnaryOp  = TCollection_AsciiString( iUnaryOp  );
      BinaryOp = TCollection_AsciiString( iBinaryOp );
    }
  }

  //================================================================================
  /*!
   * \brief Replaces "SMESH.PointStruct(x,y,z)" and "SMESH.DirStruct( SMESH.PointStruct(x,y,z))"
   *        arguments of a given command by a list "[x,y,z]" if the list is accesible
   *        type of argument.
   */
  //================================================================================

  void StructToList( Handle( _pyCommand)& theCommand, const bool checkMethod=true )
  {
    static TStringSet methodsAcceptingList;
    if ( methodsAcceptingList.empty() ) {
      const char * methodNames[] = {
        "GetCriterion","Reorient2D","ExtrusionSweep","ExtrusionSweepMakeGroups0D",
        "ExtrusionSweepMakeGroups","ExtrusionSweep0D",
        "AdvancedExtrusion","AdvancedExtrusionMakeGroups",
        "ExtrusionSweepObject","ExtrusionSweepObject0DMakeGroups",
        "ExtrusionSweepObjectMakeGroups","ExtrusionSweepObject0D",
        "ExtrusionSweepObject1D","ExtrusionSweepObject1DMakeGroups",
        "ExtrusionSweepObject2D","ExtrusionSweepObject2DMakeGroups",
        "ExtrusionSweepObjects","RotationSweepObjects","ExtrusionAlongPathObjects",
        "Translate","TranslateMakeGroups","TranslateMakeMesh",
        "TranslateObject","TranslateObjectMakeGroups", "TranslateObjectMakeMesh",
        "ExtrusionAlongPathX","ExtrusionAlongPathObjX","SplitHexahedraIntoPrisms"
        ,"" }; // <- mark of the end
      methodsAcceptingList.Insert( methodNames );
    }
    if ( !checkMethod || methodsAcceptingList.Contains( theCommand->GetMethod() ))
    {
      for ( int i = theCommand->GetNbArgs(); i > 0; --i )
      {
        const _AString & arg = theCommand->GetArg( i );
        if ( arg.Search( "SMESH.PointStruct" ) == 1 ||
             arg.Search( "SMESH.DirStruct"   ) == 1 )
        {
          Handle(_pyCommand) workCmd = new _pyCommand( arg );
          if ( workCmd->GetNbArgs() == 1 ) // SMESH.DirStruct( SMESH.PointStruct(x,y,z))
          {
            workCmd = new _pyCommand( workCmd->GetArg( 1 ) );
          }
          if ( workCmd->GetNbArgs() == 3 ) // SMESH.PointStruct(x,y,z)
          {
            _AString newArg = "[ ";
            newArg += ( workCmd->GetArg( 1 ) + ", " +
                        workCmd->GetArg( 2 ) + ", " +
                        workCmd->GetArg( 3 ) + " ]");
            theCommand->SetArg( i, newArg );
          }
        }
      }
    }
  }
  //================================================================================
  /*!
   * \brief Replaces "mesh.GetIDSource([id1,id2])" argument of a given command by
   *        a list "[id1,id2]" if the list is an accesible type of argument.
   */
  //================================================================================

  void GetIDSourceToList( Handle( _pyCommand)& theCommand )
  {
    static TStringSet methodsAcceptingList;
    if ( methodsAcceptingList.empty() ) {
      const char * methodNames[] = {
        "ExportPartToMED","ExportPartToDAT","ExportPartToUNV","ExportPartToSTL",
        "ExportCGNS","ExportGMF",
        "Create0DElementsOnAllNodes","Reorient2D","QuadTo4Tri",
        "ScaleMakeGroups","Scale","ScaleMakeMesh",
        "FindCoincidentNodesOnPartBut","DoubleElements",
        "ExtrusionSweepObjects","RotationSweepObjects","ExtrusionAlongPathObjects"
        ,"" }; // <- mark of the end
      methodsAcceptingList.Insert( methodNames );
    }
    if ( methodsAcceptingList.Contains( theCommand->GetMethod() ))
    {
      for ( int i = theCommand->GetNbArgs(); i > 0; --i )
      {
        _pyCommand argCmd( theCommand->GetArg( i ));
        if ( argCmd.GetMethod() == "GetIDSource" &&
             argCmd.GetNbArgs() == 2 )
        {
          theCommand->SetArg( i, argCmd.GetArg( 1 ));
        }
      }
    }
  }
}

//================================================================================
/*!
 * \brief Convert a python script using commands of smeshBuilder.py
 *  \param theScriptLines - Lines of the input script
 *  \param theEntry2AccessorMethod - returns method names to access to
 *         objects wrapped with python class
 *  \param theObjectNames - names of objects
 *  \param theRemovedObjIDs - entries of objects whose created commands were removed
 *  \param theHistoricalDump - true means to keep all commands, false means
 *         to exclude commands relating to objects removed from study
 *  \retval TCollection_AsciiString - Convertion result
 */
//================================================================================

void
SMESH_2smeshpy::ConvertScript(std::list< TCollection_AsciiString >&     theScriptLines,
                              Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod,
                              Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
                              std::set< TCollection_AsciiString >&      theRemovedObjIDs,
                              SALOMEDS::Study_ptr&                      theStudy,
                              const bool                                theToKeepAllCommands)
{
  std::list< TCollection_AsciiString >::iterator lineIt;
  // process notebook variables
  {
    SMESH_NoteBook aNoteBook;

    for ( lineIt = theScriptLines.begin(); lineIt != theScriptLines.end(); ++lineIt )
      aNoteBook.AddCommand( *lineIt );

    theScriptLines.clear();

    aNoteBook.ReplaceVariables();

    aNoteBook.GetResultLines( theScriptLines );
  }

  // convert to smeshBuilder.py API

  theGen = new _pyGen( theEntry2AccessorMethod,
                       theObjectNames,
                       theRemovedObjIDs,
                       theStudy,
                       theToKeepAllCommands );

  for ( lineIt = theScriptLines.begin(); lineIt != theScriptLines.end(); ++lineIt )
    theGen->AddCommand( *lineIt );

  theScriptLines.clear();

  // finish conversion
  theGen->Flush();
#ifdef DUMP_CONVERSION
  MESSAGE_BEGIN ( std::endl << " ######## RESULT ######## " << std::endl<< std::endl );
#endif

  // clean commmands of removed objects depending on myIsPublished flag
  theGen->ClearCommands();

  // reorder commands after conversion
  list< Handle(_pyCommand) >::iterator cmd;
  bool orderChanges;
  do {
    orderChanges = false;
    for ( cmd = theGen->GetCommands().begin(); cmd != theGen->GetCommands().end(); ++cmd )
      if ( (*cmd)->SetDependentCmdsAfter() )
        orderChanges = true;
  } while ( orderChanges );

  // concat commands back into a script
  TCollection_AsciiString aPrevCmd;
  set<_pyID> createdObjects;
  createdObjects.insert( "smeshBuilder" );
  createdObjects.insert( "smesh" );
  createdObjects.insert( "theStudy" );
  for ( cmd = theGen->GetCommands().begin(); cmd != theGen->GetCommands().end(); ++cmd )
  {
#ifdef DUMP_CONVERSION
    MESSAGE_ADD ( "## COM " << (*cmd)->GetOrderNb() << ": "<< (*cmd)->GetString() << std::endl );
#endif
    if ( !(*cmd)->IsEmpty() && aPrevCmd != (*cmd)->GetString()) {
      CheckObjectPresence( *cmd, createdObjects );
      if ( !(*cmd)->IsEmpty() ) {
        aPrevCmd = (*cmd)->GetString();
        theScriptLines.push_back( aPrevCmd );
      }
    }
  }

  theGen->Free();
  theGen.Nullify();
}

//================================================================================
/*!
 * \brief _pyGen constructor
 */
//================================================================================

_pyGen::_pyGen(Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod,
               Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
               std::set< TCollection_AsciiString >&      theRemovedObjIDs,
               SALOMEDS::Study_ptr&                      theStudy,
               const bool                                theToKeepAllCommands)
  : _pyObject( new _pyCommand( "", 0 )),
    myNbCommands( 0 ),
    myID2AccessorMethod( theEntry2AccessorMethod ),
    myObjectNames( theObjectNames ),
    myRemovedObjIDs( theRemovedObjIDs ),
    myNbFilters( 0 ),
    myToKeepAllCommands( theToKeepAllCommands ),
    myStudy( SALOMEDS::Study::_duplicate( theStudy )),
    myGeomIDNb(0), myGeomIDIndex(-1)
{
  // make that GetID() to return TPythonDump::SMESHGenName()
  GetCreationCmd()->Clear();
  GetCreationCmd()->GetString() = TPythonDump::SMESHGenName();
  GetCreationCmd()->GetString() += "=";

  // Find 1st digit of study entry by which a GEOM object differs from a SMESH object
  if ( !theObjectNames.IsEmpty() && !CORBA::is_nil( theStudy ))
  {
    // find a GEOM entry
    _pyID geomID;
    SALOMEDS::SComponent_wrap geomComp = theStudy->FindComponent("GEOM");
    if ( geomComp->_is_nil() ) return;
    CORBA::String_var entry = geomComp->GetID();
    geomID = entry.in();

    // find a SMESH entry
    _pyID smeshID;
    Resource_DataMapIteratorOfDataMapOfAsciiStringAsciiString e2n( theObjectNames );
    for ( ; e2n.More() && smeshID.IsEmpty(); e2n.Next() )
      if ( _pyCommand::IsStudyEntry( e2n.Key() ))
        smeshID = e2n.Key();

    // find 1st difference between smeshID and geomID
    if ( !geomID.IsEmpty() && !smeshID.IsEmpty() )
      for ( int i = 1; i <= geomID.Length() && i <= smeshID.Length(); ++i )
        if ( geomID.Value( i ) != smeshID.Value( i ))
        {
          myGeomIDNb = geomID.Value( i );
          myGeomIDIndex = i;
        }
  }
}

//================================================================================
/*!
 * \brief name of SMESH_Gen in smeshBuilder.py
 */
//================================================================================

const char* _pyGen::AccessorMethod() const
{
  return SMESH_2smeshpy::GenName();
}

//================================================================================
/*!
 * \brief Convert a command using a specific converter
  * \param theCommand - the command to convert
 */
//================================================================================

Handle(_pyCommand) _pyGen::AddCommand( const TCollection_AsciiString& theCommand)
{
  // store theCommand in the sequence
  myCommands.push_back( new _pyCommand( theCommand, ++myNbCommands ));

  Handle(_pyCommand) aCommand = myCommands.back();
#ifdef DUMP_CONVERSION
  MESSAGE ( "## COM " << myNbCommands << ": "<< aCommand->GetString() );
#endif

  const _pyID& objID = aCommand->GetObject();

  if ( objID.IsEmpty() )
    return aCommand;

  // Prevent moving a command creating a sub-mesh to the end of the script
  // if the sub-mesh is used in theCommand as argument
  // if ( _pySubMesh::CanBeArgOfMethod( aCommand->GetMethod() ))
  // {
  //   PlaceSubmeshAfterItsCreation( aCommand );
  // }

  // Method( SMESH.PointStruct(x,y,z)... -> Method( [x,y,z]...
  StructToList( aCommand );

  const TCollection_AsciiString& method = aCommand->GetMethod();

  // not to erase _pySelfEraser's etc. used as args in some commands
  {
#ifdef USE_STRING_FAMILY
    std::list<_pyID>  objIDs;
    if ( myKeepAgrCmdsIDs.IsInArgs( aCommand, objIDs ))
    {
      std::list<_pyID>::iterator objID = objIDs.begin();
      for ( ; objID != objIDs.end(); ++objID )
      {
        Handle(_pyObject) obj = FindObject( *objID );
        if ( !obj.IsNull() )
        {
          obj->AddArgCmd( aCommand );
          //cout << objID << " found in " << theCommand << endl;
        }
      }
    }
#else
    std::list< _pyID >::const_iterator id = myKeepAgrCmdsIDs.begin();
    for ( ; id != myKeepAgrCmdsIDs.end(); ++id )
      if ( *id != objID && theCommand.Search( *id ) > id->Length() )
      {
        Handle(_pyObject) obj = FindObject( *id );
        if ( !obj.IsNull() )
          obj->AddArgCmd( aCommand );
      }
#endif
  }

  // Find an object to process theCommand

  // SMESH_Gen method?
  if ( objID == this->GetID() || objID == SMESH_2smeshpy::GenName())
  {
    this->Process( aCommand );
    //addFilterUser( aCommand, theGen ); // protect filters from clearing
    return aCommand;
  }

  // SMESH_Mesh method?
  map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.find( objID );
  if ( id_mesh != myMeshes.end() )
  {
    //id_mesh->second->AddProcessedCmd( aCommand );

    // Wrap Export*() into try-except
    if ( aCommand->MethodStartsFrom("Export"))
    {
      _AString    tab = "\t";
      _AString indent = aCommand->GetIndentation();
      _AString tryStr = indent + "try:";
      _AString newCmd = indent + tab + ( aCommand->GetString().ToCString() + indent.Length() );
      _AString excStr = indent + "except:";
      _AString msgStr = indent + "\tprint '"; msgStr += method + "() failed. Invalid file name?'";

      myCommands.insert( --myCommands.end(), new _pyCommand( tryStr, myNbCommands ));
      aCommand->Clear();
      aCommand->GetString() = newCmd;
      aCommand->SetOrderNb( ++myNbCommands );
      myCommands.push_back( new _pyCommand( excStr, ++myNbCommands ));
      myCommands.push_back( new _pyCommand( msgStr, ++myNbCommands ));
    }
    // check for mesh editor object
    if ( aCommand->GetMethod() == "GetMeshEditor" ) { // MeshEditor creation
      _pyID editorID = aCommand->GetResultValue();
      Handle(_pyMeshEditor) editor = new _pyMeshEditor( aCommand );
      myMeshEditors.insert( make_pair( editorID, editor ));
      return aCommand;
    }
    // check for SubMesh objects
    else if ( aCommand->GetMethod() == "GetSubMesh" ) { // SubMesh creation
      _pyID subMeshID = aCommand->GetResultValue();
      Handle(_pySubMesh) subMesh = new _pySubMesh( aCommand );
      AddObject( subMesh );
    }

    // Method( mesh.GetIDSource([id1,id2]) -> Method( [id1,id2]
    GetIDSourceToList( aCommand );

    //addFilterUser( aCommand, theGen ); // protect filters from clearing

    id_mesh->second->Process( aCommand );
    id_mesh->second->AddProcessedCmd( aCommand );
    return aCommand;
  }

  // SMESH_MeshEditor method?
  map< _pyID, Handle(_pyMeshEditor) >::iterator id_editor = myMeshEditors.find( objID );
  if ( id_editor != myMeshEditors.end() )
  {
    // Method( mesh.GetIDSource([id1,id2]) -> Method( [id1,id2]
    GetIDSourceToList( aCommand );

    //addFilterUser( aCommand, theGen ); // protect filters from clearing

    // some commands of SMESH_MeshEditor create meshes and groups
    _pyID meshID, groups;
    if ( method.Search("MakeMesh") != -1 )
      meshID = aCommand->GetResultValue();
    else if ( method == "MakeBoundaryMesh")
      meshID = aCommand->GetResultValue(1);
    else if ( method == "MakeBoundaryElements")
      meshID = aCommand->GetResultValue(2);

    if ( method.Search("MakeGroups") != -1  ||
         method == "ExtrusionAlongPathX"    ||
         method == "ExtrusionAlongPathObjX" ||
         method == "DoubleNodeGroupNew"     ||
         method == "DoubleNodeGroupsNew"    ||
         method == "DoubleNodeElemGroupNew" ||
         method == "DoubleNodeElemGroupsNew"||
         method == "DoubleNodeElemGroup2New"||
         method == "DoubleNodeElemGroups2New"
         )
      groups = aCommand->GetResultValue();
    else if ( method == "MakeBoundaryMesh" )
      groups = aCommand->GetResultValue(2);
    else if ( method == "MakeBoundaryElements")
      groups = aCommand->GetResultValue(3);
    else if ( method == "Create0DElementsOnAllNodes" &&
              aCommand->GetArg(2).Length() > 2 ) // group name != ''
      groups = aCommand->GetResultValue();

    id_editor->second->Process( aCommand );
    id_editor->second->AddProcessedCmd( aCommand );

    // create meshes
    if ( !meshID.IsEmpty() &&
         !myMeshes.count( meshID ) &&
         aCommand->IsStudyEntry( meshID ))
    {
      _AString processedCommand = aCommand->GetString();
      Handle(_pyMesh) mesh = new _pyMesh( aCommand, meshID );
      CheckObjectIsReCreated( mesh );
      myMeshes.insert( make_pair( meshID, mesh ));
      aCommand->Clear();
      aCommand->GetString() = processedCommand; // discard changes made by _pyMesh
    }
    // create groups
    if ( !groups.IsEmpty() )
    {
      if ( !aCommand->IsStudyEntry( meshID ))
        meshID = id_editor->second->GetMesh();
      Handle(_pyMesh) mesh = myMeshes[ meshID ];

      list< _pyID > idList = aCommand->GetStudyEntries( groups );
      list< _pyID >::iterator grID = idList.begin();
      for ( ; grID != idList.end(); ++grID )
        if ( !myObjects.count( *grID ))
        {
          Handle(_pyGroup) group = new _pyGroup( aCommand, *grID );
          AddObject( group );
          if ( !mesh.IsNull() ) mesh->AddGroup( group );
        }
    }
    return aCommand;
  } // SMESH_MeshEditor methods

  // SMESH_Hypothesis method?
  Handle(_pyHypothesis) hyp = FindHyp( objID );
  if ( !hyp.IsNull() && !hyp->IsAlgo() )
  {
    hyp->Process( aCommand );
    hyp->AddProcessedCmd( aCommand );
    return aCommand;
  }

  // aFilterManager.CreateFilter() ?
  if ( aCommand->GetMethod() == "CreateFilter" )
  {
    // Set a more human readable name to a filter
    // aFilter0x7fbf6c71cfb0 -> aFilter_nb
    _pyID newID, filterID = aCommand->GetResultValue();
    int pos = filterID.Search( "0x" );
    if ( pos > 1 )
      newID = (filterID.SubString(1,pos-1) + "_") + _pyID( ++myNbFilters );

    Handle(_pyObject) filter( new _pyFilter( aCommand, newID ));
    AddObject( filter );
  }
  // aFreeNodes0x5011f80 = aFilterManager.CreateFreeNodes() ## issue 0020976
  else if ( theCommand.Search( "aFilterManager.Create" ) > 0 )
  {
    // create _pySelfEraser for functors
    Handle(_pySelfEraser) functor = new _pySelfEraser( aCommand );
    functor->IgnoreOwnCalls(); // to erase if not used as an argument
    AddObject( functor );
  }

  // other object method?
  map< _pyID, Handle(_pyObject) >::iterator id_obj = myObjects.find( objID );
  if ( id_obj != myObjects.end() ) {
    id_obj->second->Process( aCommand );
    id_obj->second->AddProcessedCmd( aCommand );
    return aCommand;
  }

  // Add access to a wrapped mesh
  AddMeshAccessorMethod( aCommand );

  // Add access to a wrapped algorithm
  //  AddAlgoAccessorMethod( aCommand ); // ??? what if algo won't be wrapped at all ???

  // PAL12227. PythonDump was not updated at proper time; result is
  //     aCriteria.append(SMESH.Filter.Criterion(17,26,0,'L1',26,25,1e-07,SMESH.EDGE,-1))
  // TypeError: __init__() takes exactly 11 arguments (10 given)
  const char wrongCommand[] = "SMESH.Filter.Criterion(";
  if ( int beg = theCommand.Location( wrongCommand, 1, theCommand.Length() ))
  {
    _pyCommand tmpCmd( theCommand.SubString( beg, theCommand.Length() ), -1);
    // there must be 10 arguments, 5-th arg ThresholdID is missing,
    const int wrongNbArgs = 9, missingArg = 5;
    if ( tmpCmd.GetNbArgs() == wrongNbArgs )
    {
      for ( int i = wrongNbArgs; i > missingArg; --i )
        tmpCmd.SetArg( i + 1, tmpCmd.GetArg( i ));
      tmpCmd.SetArg(  missingArg, "''");
      aCommand->GetString().Trunc( beg - 1 );
      aCommand->GetString() += tmpCmd.GetString();
    }
    // IMP issue 0021014
    // set GetCriterion(elementType,CritType,Compare,Treshold,UnaryOp,BinaryOp,Tolerance)
    //                  1           2        3       4        5       6        7
    // instead of "SMESH.Filter.Criterion(
    // Type,Compare,Threshold,ThresholdStr,ThresholdID,UnaryOp,BinaryOp,Tolerance,TypeOfElement,Precision)
    // 1    2       3         4            5           6       7        8         9             10
    // in order to avoid the problem of type mismatch of long and FunctorType
    const TCollection_AsciiString
      SMESH("SMESH."), dfltFunctor("SMESH.FT_Undefined"), dfltTol("1e-07"), dfltPreci("-1");
    TCollection_AsciiString
      Type          = aCommand->GetArg(1),  // long
      Compare       = aCommand->GetArg(2),  // long
      Threshold     = aCommand->GetArg(3),  // double
      ThresholdStr  = aCommand->GetArg(4),  // string
      ThresholdID   = aCommand->GetArg(5),  // string
      UnaryOp       = aCommand->GetArg(6),  // long
      BinaryOp      = aCommand->GetArg(7),  // long
      Tolerance     = aCommand->GetArg(8),  // double
      TypeOfElement = aCommand->GetArg(9),  // ElementType
      Precision     = aCommand->GetArg(10); // long
    fixFunctorType( Type, Compare, UnaryOp, BinaryOp );
    Type     = SMESH + SMESH::FunctorTypeToString( SMESH::FunctorType( Type.IntegerValue() ));
    Compare  = SMESH + SMESH::FunctorTypeToString( SMESH::FunctorType( Compare.IntegerValue() ));
    UnaryOp  = SMESH + SMESH::FunctorTypeToString( SMESH::FunctorType( UnaryOp.IntegerValue() ));
    BinaryOp = SMESH + SMESH::FunctorTypeToString( SMESH::FunctorType( BinaryOp.IntegerValue() ));

    if ( Compare == "SMESH.FT_EqualTo" )
      Compare = "'='";

    aCommand->RemoveArgs();
    aCommand->SetObject( SMESH_2smeshpy::GenName() );
    aCommand->SetMethod( "GetCriterion" );

    aCommand->SetArg( 1, TypeOfElement );
    aCommand->SetArg( 2, Type );
    aCommand->SetArg( 3, Compare );

    if ( Threshold.IsIntegerValue() )
    {
      int iGeom = Threshold.IntegerValue();
      if ( Type == "SMESH.FT_ElemGeomType" )
      {
        // set SMESH.GeometryType instead of a numerical Threshold
        const int nbTypes = SMESH::Geom_LAST;
        const char* types[] = {
          "Geom_POINT", "Geom_EDGE", "Geom_TRIANGLE", "Geom_QUADRANGLE", "Geom_POLYGON",
          "Geom_TETRA", "Geom_PYRAMID", "Geom_HEXA", "Geom_PENTA", "Geom_HEXAGONAL_PRISM",
          "Geom_POLYHEDRA", "Geom_BALL" };
        if ( -1 < iGeom && iGeom < nbTypes )
          Threshold = SMESH + types[ iGeom ];
#ifdef _DEBUG_
        // is types complete? (compilation failure mains that enum GeometryType changed)
        int _asrt[( sizeof(types) / sizeof(const char*) == nbTypes ) ? 2 : -1 ]; _asrt[0]=_asrt[1];
#endif
      }
      if (Type == "SMESH.FT_EntityType")
      {
        // set SMESH.EntityType instead of a numerical Threshold
        const int nbTypes = SMESH::Entity_Last;
        const char* types[] = {
          "Entity_Node", "Entity_0D", "Entity_Edge", "Entity_Quad_Edge",
          "Entity_Triangle", "Entity_Quad_Triangle", "Entity_BiQuad_Triangle",
          "Entity_Quadrangle", "Entity_Quad_Quadrangle", "Entity_BiQuad_Quadrangle",
          "Entity_Polygon", "Entity_Quad_Polygon", "Entity_Tetra", "Entity_Quad_Tetra",
          "Entity_Pyramid", "Entity_Quad_Pyramid",
          "Entity_Hexa", "Entity_Quad_Hexa", "Entity_TriQuad_Hexa",
          "Entity_Penta", "Entity_Quad_Penta", "Entity_Hexagonal_Prism",
          "Entity_Polyhedra", "Entity_Quad_Polyhedra", "Entity_Ball" };
        if ( -1 < iGeom && iGeom < nbTypes )
          Threshold = SMESH + types[ iGeom ];
#ifdef _DEBUG_
        // is 'types' complete? (compilation failure mains that enum EntityType changed)
        int _asrt[( sizeof(types) / sizeof(const char*) == nbTypes ) ? 2 : -1 ]; _asrt[0]=_asrt[1];
#endif
      }
    }
    if ( ThresholdID.Length() != 2 ) // neither '' nor ""
      aCommand->SetArg( 4, ThresholdID.SubString( 2, ThresholdID.Length()-1 )); // shape entry
    else if ( ThresholdStr.Length() != 2 )
      aCommand->SetArg( 4, ThresholdStr );
    else if ( ThresholdID.Length() != 2 )
      aCommand->SetArg( 4, ThresholdID );
    else
      aCommand->SetArg( 4, Threshold );
    // find the last not default arg
    int lastDefault = 8;
    if ( Tolerance == dfltTol ) {
      lastDefault = 7;
      if ( BinaryOp == dfltFunctor ) {
        lastDefault = 6;
        if ( UnaryOp == dfltFunctor )
          lastDefault = 5;
      }
    }
    if ( 5 < lastDefault ) aCommand->SetArg( 5, UnaryOp );
    if ( 6 < lastDefault ) aCommand->SetArg( 6, BinaryOp );
    if ( 7 < lastDefault ) aCommand->SetArg( 7, Tolerance );
    if ( Precision != dfltPreci )
    {
      TCollection_AsciiString crit = aCommand->GetResultValue();
      aCommand->GetString() += "; ";
      aCommand->GetString() += crit + ".Precision = " + Precision;
    }
  }
  return aCommand;
}

//================================================================================
/*!
 * \brief Convert the command or remember it for later conversion
  * \param theCommand - The python command calling a method of SMESH_Gen
 */
//================================================================================

void _pyGen::Process( const Handle(_pyCommand)& theCommand )
{
  // there are methods to convert:
  // CreateMesh( shape )
  // Concatenate( [mesh1, ...], ... )
  // CreateHypothesis( theHypType, theLibName )
  // Compute( mesh, geom )
  // Evaluate( mesh, geom )
  // mesh creation
  TCollection_AsciiString method = theCommand->GetMethod();

  if ( method == "CreateMesh" || method == "CreateEmptyMesh")
  {
    Handle(_pyMesh) mesh = new _pyMesh( theCommand );
    AddObject( mesh );
    return;
  }
  if ( method == "CreateMeshesFromUNV" ||
       method == "CreateMeshesFromSTL" ||
       method == "CopyMesh" ) // command result is a mesh
  {
    Handle(_pyMesh) mesh = new _pyMesh( theCommand, theCommand->GetResultValue() );
    AddObject( mesh );
    return;
  }
  if( method == "CreateMeshesFromMED" ||
      method == "CreateMeshesFromSAUV"||
      method == "CreateMeshesFromCGNS" ||
      method == "CreateMeshesFromGMF" ) // command result is ( [mesh1,mesh2], status )
  {
    std::list< _pyID > meshIDs = theCommand->GetStudyEntries( theCommand->GetResultValue() );
    std::list< _pyID >::iterator meshID = meshIDs.begin();
    for ( ; meshID != meshIDs.end(); ++meshID )
    {
      Handle(_pyMesh) mesh = new _pyMesh( theCommand, *meshID );
      AddObject( mesh );
    }
    if ( method == "CreateMeshesFromGMF" )
    {
      // CreateMeshesFromGMF( theFileName, theMakeRequiredGroups ) ->
      // CreateMeshesFromGMF( theFileName )
      _AString file = theCommand->GetArg(1);
      theCommand->RemoveArgs();
      theCommand->SetArg( 1, file );
    }
  }

  // CreateHypothesis()
  if ( method == "CreateHypothesis" )
  {
    // issue 199929, remove standard library name (default parameter)
    const TCollection_AsciiString & aLibName = theCommand->GetArg( 2 );
    if ( aLibName.Search( "StdMeshersEngine" ) != -1 ) {
      // keep the first argument
      TCollection_AsciiString arg = theCommand->GetArg( 1 );
      theCommand->RemoveArgs();
      theCommand->SetArg( 1, arg );
    }

    Handle(_pyHypothesis) hyp = _pyHypothesis::NewHypothesis( theCommand );
    CheckObjectIsReCreated( hyp );
    myHypos.insert( make_pair( hyp->GetID(), hyp ));

    return;
  }

  // smeshgen.Compute( mesh, geom ) --> mesh.Compute()
  if ( method == "Compute" )
  {
    const _pyID& meshID = theCommand->GetArg( 1 );
    map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.find( meshID );
    if ( id_mesh != myMeshes.end() ) {
      theCommand->SetObject( meshID );
      theCommand->RemoveArgs();
      id_mesh->second->Process( theCommand );
      id_mesh->second->AddProcessedCmd( theCommand );
      return;
    }
  }

  // smeshgen.Evaluate( mesh, geom ) --> mesh.Evaluate(geom)
  if ( method == "Evaluate" )
  {
    const _pyID& meshID = theCommand->GetArg( 1 );
    map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.find( meshID );
    if ( id_mesh != myMeshes.end() ) {
      theCommand->SetObject( meshID );
      _pyID geom = theCommand->GetArg( 2 );
      theCommand->RemoveArgs();
      theCommand->SetArg( 1, geom );
      id_mesh->second->AddProcessedCmd( theCommand );
      return;
    }
  }

  // objects erasing creation command if no more its commands invoked:
  // SMESH_Pattern, FilterManager
  if ( method == "GetPattern" ||
       method == "CreateFilterManager" ||
       method == "CreateMeasurements" )
  {
    Handle(_pyObject) obj = new _pySelfEraser( theCommand );
    if ( !AddObject( obj ) )
      theCommand->Clear(); // already created
  }
  // Concatenate( [mesh1, ...], ... )
  else if ( method == "Concatenate" || method == "ConcatenateWithGroups")
  {
    if ( method == "ConcatenateWithGroups" ) {
      theCommand->SetMethod( "Concatenate" );
      theCommand->SetArg( theCommand->GetNbArgs() + 1, "True" );
    }
    Handle(_pyMesh) mesh = new _pyMesh( theCommand, theCommand->GetResultValue() );
    AddObject( mesh );
    AddMeshAccessorMethod( theCommand );
  }
  else if ( method == "SetName" ) // SetName(obj,name)
  {
    // store theCommand as one of object commands to erase it along with the object
    const _pyID& objID = theCommand->GetArg( 1 );
    Handle(_pyObject) obj = FindObject( objID );
    if ( !obj.IsNull() )
      obj->AddProcessedCmd( theCommand );
  }

  // Replace name of SMESH_Gen

  // names of SMESH_Gen methods fully equal to methods defined in smeshBuilder.py
  static TStringSet smeshpyMethods;
  if ( smeshpyMethods.empty() ) {
    const char * names[] =
      { "SetEmbeddedMode","IsEmbeddedMode","SetCurrentStudy","GetCurrentStudy",
        "GetPattern","GetSubShapesId",
        "" }; // <- mark of array end
    smeshpyMethods.Insert( names );
  }
  if ( smeshpyMethods.Contains( theCommand->GetMethod() ))
    // smeshgen.Method() --> smesh.Method()
    theCommand->SetObject( SMESH_2smeshpy::SmeshpyName() );
  else
    // smeshgen.Method() --> smesh.Method()
    theCommand->SetObject( SMESH_2smeshpy::GenName() );
}

//================================================================================
/*!
 * \brief Convert the remembered commands
 */
//================================================================================

void _pyGen::Flush()
{
  // create an empty command
  myLastCommand = new _pyCommand();

  map< _pyID, Handle(_pyMesh) >::iterator id_mesh;
  map< _pyID, Handle(_pyObject) >::iterator id_obj;
  map< _pyID, Handle(_pyHypothesis) >::iterator id_hyp;

  if ( IsToKeepAllCommands() ) // historical dump
  {
    // set myIsPublished = true to all objects
    for ( id_mesh = myMeshes.begin(); id_mesh != myMeshes.end(); ++id_mesh )
      id_mesh->second->SetRemovedFromStudy( false );
    for ( id_hyp = myHypos.begin(); id_hyp != myHypos.end(); ++id_hyp )
      id_hyp->second->SetRemovedFromStudy( false );
    for ( id_obj = myObjects.begin(); id_obj != myObjects.end(); ++id_obj )
      id_obj->second->SetRemovedFromStudy( false );
  }
  else
  {
    // let hypotheses find referred objects in order to prevent clearing
    // not published referred hyps (it's needed for hyps like "LayerDistribution")
    list< Handle(_pyMesh) > fatherMeshes;
    for ( id_hyp = myHypos.begin(); id_hyp != myHypos.end(); ++id_hyp )
      if ( !id_hyp->second.IsNull() )
        id_hyp->second->GetReferredMeshesAndGeom( fatherMeshes );
  }
  // set myIsPublished = false to all objects depending on
  // meshes built on a removed geometry
  for ( id_mesh = myMeshes.begin(); id_mesh != myMeshes.end(); ++id_mesh )
    if ( id_mesh->second->IsNotGeomPublished() )
      id_mesh->second->SetRemovedFromStudy( true );

  // Flush meshes
  for ( id_mesh = myMeshes.begin(); id_mesh != myMeshes.end(); ++id_mesh )
    if ( ! id_mesh->second.IsNull() )
      id_mesh->second->Flush();

  // Flush hyps
  for ( id_hyp = myHypos.begin(); id_hyp != myHypos.end(); ++id_hyp )
    if ( !id_hyp->second.IsNull() ) {
      id_hyp->second->Flush();
      // smeshgen.CreateHypothesis() --> smesh.CreateHypothesis()
      if ( !id_hyp->second->IsWrapped() )
        id_hyp->second->GetCreationCmd()->SetObject( SMESH_2smeshpy::GenName() );
    }

  // Flush other objects. 2 times, for objects depending on Flush() of later created objects
  std::list< Handle(_pyObject) >::reverse_iterator robj = myOrderedObjects.rbegin();
  for ( ; robj != myOrderedObjects.rend(); ++robj )
    if ( ! robj->IsNull() )
      (*robj)->Flush();
  std::list< Handle(_pyObject) >::iterator obj = myOrderedObjects.begin();
  for ( ; obj != myOrderedObjects.end(); ++obj )
    if ( ! obj->IsNull() )
      (*obj)->Flush();

  myLastCommand->SetOrderNb( ++myNbCommands );
  myCommands.push_back( myLastCommand );
}

//================================================================================
/*!
 * \brief Prevent moving a command creating a sub-mesh to the end of the script
 *        if the sub-mesh is used in theCmdUsingSubmesh as argument
 */
//================================================================================

void _pyGen::PlaceSubmeshAfterItsCreation( Handle(_pyCommand) theCmdUsingSubmesh ) const
{
  // map< _pyID, Handle(_pyObject) >::const_iterator id_obj = myObjects.begin();
  // for ( ; id_obj != myObjects.end(); ++id_obj )
  // {
  //   if ( !id_obj->second->IsKind( STANDARD_TYPE( _pySubMesh ))) continue;
  //   for ( int iArg = theCmdUsingSubmesh->GetNbArgs(); iArg; --iArg )
  //   {
  //     const _pyID& arg = theCmdUsingSubmesh->GetArg( iArg );
  //     if ( arg.IsEmpty() || arg.Value( 1 ) == '"' || arg.Value( 1 ) == '\'' )
  //       continue;
  //     list< _pyID > idList = theCmdUsingSubmesh->GetStudyEntries( arg );
  //     list< _pyID >::iterator id = idList.begin();
  //     for ( ; id != idList.end(); ++id )
  //       if ( id_obj->first == *id )
  //         // _pySubMesh::Process() does what we need
  //         Handle(_pySubMesh)::DownCast( id_obj->second )->Process( theCmdUsingSubmesh );
  //   }
  // }
}

//================================================================================
/*!
 * \brief Clean commmands of removed objects depending on myIsPublished flag
 */
//================================================================================

void _pyGen::ClearCommands()
{
  map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.begin();
  for ( ; id_mesh != myMeshes.end(); ++id_mesh )
    id_mesh->second->ClearCommands();

  map< _pyID, Handle(_pyHypothesis) >::iterator id_hyp = myHypos.begin();
  for ( ; id_hyp != myHypos.end(); ++id_hyp )
    if ( !id_hyp->second.IsNull() )
      id_hyp->second->ClearCommands();

  // Other objects. 2 times, for objects depending on ClearCommands() of later created objects
  std::list< Handle(_pyObject) >::reverse_iterator robj = myOrderedObjects.rbegin();
  for ( ; robj != myOrderedObjects.rend(); ++robj )
    if ( ! robj->IsNull() )
      (*robj)->ClearCommands();
  std::list< Handle(_pyObject) >::iterator obj = myOrderedObjects.begin();
  for ( ; obj != myOrderedObjects.end(); ++obj )
    if ( ! obj->IsNull() )
      (*obj)->ClearCommands();
}

//================================================================================
/*!
 * \brief Release mutual handles of objects
 */
//================================================================================

void _pyGen::Free()
{
  map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.begin();
  for ( ; id_mesh != myMeshes.end(); ++id_mesh )
    id_mesh->second->Free();
  myMeshes.clear();

  map< _pyID, Handle(_pyMeshEditor) >::iterator id_ed = myMeshEditors.begin();
  for ( ; id_ed != myMeshEditors.end(); ++id_ed )
    id_ed->second->Free();
  myMeshEditors.clear();

  map< _pyID, Handle(_pyObject) >::iterator id_obj = myObjects.begin();
  for ( ; id_obj != myObjects.end(); ++id_obj )
    id_obj->second->Free();
  myObjects.clear();

  map< _pyID, Handle(_pyHypothesis) >::iterator id_hyp = myHypos.begin();
  for ( ; id_hyp != myHypos.end(); ++id_hyp )
    if ( !id_hyp->second.IsNull() )
      id_hyp->second->Free();
  myHypos.clear();

  myFile2ExportedMesh.clear();

  //myKeepAgrCmdsIDs.Print();
}

//================================================================================
/*!
 * \brief Add access method to mesh that is an argument
  * \param theCmd - command to add access method
  * \retval bool - true if added
 */
//================================================================================

bool _pyGen::AddMeshAccessorMethod( Handle(_pyCommand) theCmd ) const
{
  bool added = false;
  map< _pyID, Handle(_pyMesh) >::const_iterator id_mesh = myMeshes.begin();
  for ( ; id_mesh != myMeshes.end(); ++id_mesh ) {
    if ( theCmd->AddAccessorMethod( id_mesh->first, id_mesh->second->AccessorMethod() ))
      added = true;
  }
  return added;
}

//================================================================================
/*!
 * \brief Add access method to algo that is an object or an argument
  * \param theCmd - command to add access method
  * \retval bool - true if added
 */
//================================================================================

bool _pyGen::AddAlgoAccessorMethod( Handle(_pyCommand) theCmd ) const
{
  bool added = false;
  map< _pyID, Handle(_pyHypothesis) >::const_iterator id_hyp = myHypos.begin();
  for ( ; id_hyp != myHypos.end(); ++id_hyp )
    if ( !id_hyp->second.IsNull() &&
         id_hyp->second->IsAlgo() && /*(*hyp)->IsWrapped() &&*/
         theCmd->AddAccessorMethod( id_hyp->second->GetID(),
                                    id_hyp->second->AccessorMethod() ))
      added = true;

  return added;
}

//================================================================================
/*!
 * \brief Find hypothesis by ID (entry)
  * \param theHypID - The hypothesis ID
  * \retval Handle(_pyHypothesis) - The found hypothesis
 */
//================================================================================

Handle(_pyHypothesis) _pyGen::FindHyp( const _pyID& theHypID )
{
  map< _pyID, Handle(_pyHypothesis) >::iterator id_hyp = myHypos.find( theHypID );
  if ( id_hyp != myHypos.end() &&
       !id_hyp->second.IsNull() &&
       theHypID == id_hyp->second->GetID() )
    return id_hyp->second;
  return Handle(_pyHypothesis)();
}

//================================================================================
/*!
 * \brief Find algorithm able to create a hypothesis
  * \param theGeom - The shape ID the algorithm was created on
  * \param theMesh - The mesh ID that created the algorithm
  * \param theHypothesis - The hypothesis the algorithm sould be able to create
  * \retval Handle(_pyHypothesis) - The found algo
 */
//================================================================================

Handle(_pyHypothesis) _pyGen::FindAlgo( const _pyID& theGeom, const _pyID& theMesh,
                                        const Handle(_pyHypothesis)& theHypothesis )
{
  map< _pyID, Handle(_pyHypothesis) >::iterator id_hyp = myHypos.begin();
  for ( ; id_hyp != myHypos.end(); ++id_hyp )
    if ( !id_hyp->second.IsNull() &&
         id_hyp->second->IsAlgo() &&
         theHypothesis->CanBeCreatedBy( id_hyp->second->GetAlgoType() ) &&
         id_hyp->second->GetGeom() == theGeom &&
         id_hyp->second->GetMesh() == theMesh )
      return id_hyp->second;
  return Handle(_pyHypothesis)();
}

//================================================================================
/*!
 * \brief Find subMesh by ID (entry)
  * \param theSubMeshID - The subMesh ID
  * \retval Handle(_pySubMesh) - The found subMesh
 */
//================================================================================

Handle(_pySubMesh) _pyGen::FindSubMesh( const _pyID& theSubMeshID )
{
  map< _pyID, Handle(_pyObject) >::iterator id_subMesh = myObjects.find(theSubMeshID);
  if ( id_subMesh != myObjects.end() )
    return Handle(_pySubMesh)::DownCast( id_subMesh->second );
  return Handle(_pySubMesh)();
}


//================================================================================
/*!
 * \brief Change order of commands in the script
  * \param theCmd1 - One command
  * \param theCmd2 - Another command
 */
//================================================================================

void _pyGen::ExchangeCommands( Handle(_pyCommand) theCmd1, Handle(_pyCommand) theCmd2 )
{
  list< Handle(_pyCommand) >::iterator pos1, pos2;
  pos1 = find( myCommands.begin(), myCommands.end(), theCmd1 );
  pos2 = find( myCommands.begin(), myCommands.end(), theCmd2 );
  myCommands.insert( pos1, theCmd2 );
  myCommands.insert( pos2, theCmd1 );
  myCommands.erase( pos1 );
  myCommands.erase( pos2 );

  int nb1 = theCmd1->GetOrderNb();
  theCmd1->SetOrderNb( theCmd2->GetOrderNb() );
  theCmd2->SetOrderNb( nb1 );
//   cout << "BECOME " << theCmd1->GetOrderNb() << "\t" << theCmd1->GetString() << endl
//        << "BECOME " << theCmd2->GetOrderNb() << "\t" << theCmd2->GetString() << endl << endl;
}

//================================================================================
/*!
 * \brief Set one command after the other
  * \param theCmd - Command to move
  * \param theAfterCmd - Command ater which to insert the first one
 */
//================================================================================

void _pyGen::SetCommandAfter( Handle(_pyCommand) theCmd, Handle(_pyCommand) theAfterCmd )
{
  setNeighbourCommand( theCmd, theAfterCmd, true );
}

//================================================================================
/*!
 * \brief Set one command before the other
  * \param theCmd - Command to move
  * \param theBeforeCmd - Command before which to insert the first one
 */
//================================================================================

void _pyGen::SetCommandBefore( Handle(_pyCommand) theCmd, Handle(_pyCommand) theBeforeCmd )
{
  setNeighbourCommand( theCmd, theBeforeCmd, false );
}

//================================================================================
/*!
 * \brief Set one command before or after the other
  * \param theCmd - Command to move
  * \param theOtherCmd - Command ater or before which to insert the first one
 */
//================================================================================

void _pyGen::setNeighbourCommand( Handle(_pyCommand)& theCmd,
                                  Handle(_pyCommand)& theOtherCmd,
                                  const bool theIsAfter )
{
  list< Handle(_pyCommand) >::iterator pos;
  pos = find( myCommands.begin(), myCommands.end(), theCmd );
  myCommands.erase( pos );
  pos = find( myCommands.begin(), myCommands.end(), theOtherCmd );
  myCommands.insert( (theIsAfter ? ++pos : pos), theCmd );

  int i = 1;
  for ( pos = myCommands.begin(); pos != myCommands.end(); ++pos)
    (*pos)->SetOrderNb( i++ );
}

//================================================================================
/*!
 * \brief Call _pyFilter.AddUser() if a filter is used as a command arg
 */
//================================================================================

// void _pyGen::addFilterUser( Handle(_pyCommand)& theCommand, const Handle(_pyObject)& user )
// {
  // No more needed after adding _pyObject::myArgCommands

//   const char filterPrefix[] = "aFilter0x";
//   if ( theCommand->GetString().Search( filterPrefix ) < 1 )
//     return;

//   for ( int i = theCommand->GetNbArgs(); i > 0; --i )
//   {
//     const _AString & arg = theCommand->GetArg( i );
//     // NOT TREATED CASE: arg == "[something, aFilter0x36a2f60]"
//     if ( arg.Search( filterPrefix ) != 1 )
//       continue;

//     Handle(_pyFilter) filter = Handle(_pyFilter)::DownCast( FindObject( arg ));
//     if ( !filter.IsNull() )
//     {
//       filter->AddUser( user );
//       if ( !filter->GetNewID().IsEmpty() )
//         theCommand->SetArg( i, filter->GetNewID() );
//     }
//   }
//}

//================================================================================
/*!
 * \brief Set command be last in list of commands
  * \param theCmd - Command to be last
 */
//================================================================================

Handle(_pyCommand)& _pyGen::GetLastCommand()
{
  return myLastCommand;
}

//================================================================================
/*!
 * \brief Set method to access to object wrapped with python class
  * \param theID - The wrapped object entry
  * \param theMethod - The accessor method
 */
//================================================================================

void _pyGen::SetAccessorMethod(const _pyID& theID, const char* theMethod )
{
  myID2AccessorMethod.Bind( theID, (char*) theMethod );
}

//================================================================================
/*!
 * \brief Generated new ID for object and assign with existing name
  * \param theID - ID of existing object
 */
//================================================================================

_pyID _pyGen::GenerateNewID( const _pyID& theID )
{
  int index = 1;
  _pyID aNewID;
  do {
    aNewID = theID + _pyID( ":" ) + _pyID( index++ );
  }
  while ( myObjectNames.IsBound( aNewID ) );

  if ( myObjectNames.IsBound( theID ) )
    myObjectNames.Bind( aNewID, ( myObjectNames.Find( theID ) + _pyID( "_" ) + _pyID( index-1 ) ) );
  else
    myObjectNames.Bind( aNewID, ( _pyID( "A" ) + aNewID ) );
  return aNewID;
}

//================================================================================
/*!
 * \brief Stores theObj in myObjects
 */
//================================================================================

bool _pyGen::AddObject( Handle(_pyObject)& theObj )
{
  if ( theObj.IsNull() ) return false;

  CheckObjectIsReCreated( theObj );

  bool add;

  if ( theObj->IsKind( STANDARD_TYPE( _pyMesh ))) {
    add = myMeshes.insert( make_pair( theObj->GetID(),
                                      Handle(_pyMesh)::DownCast( theObj ))).second;
  }
  else if ( theObj->IsKind( STANDARD_TYPE( _pyMeshEditor ))) {
    add = myMeshEditors.insert( make_pair( theObj->GetID(),
                                          Handle(_pyMeshEditor)::DownCast( theObj ))).second;
  }
  else {
    add = myObjects.insert( make_pair( theObj->GetID(), theObj )).second;
    if ( add ) myOrderedObjects.push_back( theObj );
  }
  return add;
}

//================================================================================
/*!
 * \brief Erases an existing object with the same ID. This method should be called
 *        before storing theObj in _pyGen
 */
//================================================================================

void _pyGen::CheckObjectIsReCreated( Handle(_pyObject)& theObj )
{
  if ( theObj.IsNull() || !_pyCommand::IsStudyEntry( theObj->GetID() ))
    return;

  const bool isHyp = theObj->IsKind( STANDARD_TYPE( _pyHypothesis ));
  Handle(_pyObject) existing;
  if( isHyp )
    existing = FindHyp( theObj->GetID() );
  else
    existing = FindObject( theObj->GetID() );
  if ( !existing.IsNull() && existing != theObj )
  {
    existing->SetRemovedFromStudy( true );
    existing->ClearCommands();
    if ( isHyp )
    {
      if ( myHypos.count( theObj->GetID() ))
        myHypos.erase( theObj->GetID() );
    }
    else if ( myMeshes.count( theObj->GetID() ))
    {
      myMeshes.erase( theObj->GetID() );
    }
    else if ( myObjects.count( theObj->GetID() ))
    {
      myObjects.erase( theObj->GetID() );
    }
  }
}

//================================================================================
/*!
 * \brief Re-register an object with other ID to make it Process() commands of
 * other object having this ID
 */
//================================================================================

void _pyGen::SetProxyObject( const _pyID& theID, Handle(_pyObject)& theObj )
{
  if ( theObj.IsNull() ) return;

  if ( theObj->IsKind( STANDARD_TYPE( _pyMesh )))
    myMeshes.insert( make_pair( theID, Handle(_pyMesh)::DownCast( theObj )));

  else if ( theObj->IsKind( STANDARD_TYPE( _pyMeshEditor )))
    myMeshEditors.insert( make_pair( theID, Handle(_pyMeshEditor)::DownCast( theObj )));

  else
    myObjects.insert( make_pair( theID, theObj ));
}

//================================================================================
/*!
 * \brief Finds a _pyObject by ID
 */
//================================================================================

Handle(_pyObject) _pyGen::FindObject( const _pyID& theObjID )  const
{
  {
    map< _pyID, Handle(_pyObject) >::const_iterator id_obj = myObjects.find( theObjID );
    if ( id_obj != myObjects.end() )
      return id_obj->second;
  }
  {
    _pyGen* me = const_cast< _pyGen* >( this );
    map< _pyID, Handle(_pyMesh) >::iterator id_obj = me->myMeshes.find( theObjID );
    if ( id_obj != myMeshes.end() )
      return id_obj->second;
  }
  // {
  //   map< _pyID, Handle(_pyMeshEditor) >::const_iterator id_obj = myMeshEditors.find( theObjID );
  //   if ( id_obj != myMeshEditors.end() )
  //     return id_obj->second;
  // }
  return Handle(_pyObject)();
}

//================================================================================
/*!
 * \brief Check if a study entry is under GEOM component
 */
//================================================================================

bool _pyGen::IsGeomObject(const _pyID& theObjID) const
{
  if ( myGeomIDNb )
  {
    return ( myGeomIDIndex <= theObjID.Length() &&
             int( theObjID.Value( myGeomIDIndex )) == myGeomIDNb &&
             _pyCommand::IsStudyEntry( theObjID ));
  }
  return false;
}

//================================================================================
/*!
 * \brief Returns true if an object is not present in a study
 */
//================================================================================

bool _pyGen::IsNotPublished(const _pyID& theObjID) const
{
  if ( theObjID.IsEmpty() ) return false;

  if ( myObjectNames.IsBound( theObjID ))
    return false; // SMESH object is in study

  // either the SMESH object is not in study or it is a GEOM object
  if ( IsGeomObject( theObjID ))
  {
    SALOMEDS::SObject_wrap so = myStudy->FindObjectID( theObjID.ToCString() );
    if ( so->_is_nil() ) return true;
    CORBA::Object_var obj = so->GetObject();
    return CORBA::is_nil( obj );
  }
  return true; // SMESH object not in study
}

//================================================================================
/*!
 * \brief Add an object to myRemovedObjIDs that leads to that SetName() for
 *        this object is not dumped
 *  \param [in] theObjID - entry of the object whose creation command was eliminated
 */
//================================================================================

void _pyGen::ObjectCreationRemoved(const _pyID& theObjID)
{
  myRemovedObjIDs.insert( theObjID );
}

//================================================================================
/*!
 * \brief Return reader of  hypotheses of plugins
 */
//================================================================================

Handle( _pyHypothesisReader ) _pyGen::GetHypothesisReader() const
{
  if (myHypReader.IsNull() )
    ((_pyGen*) this)->myHypReader = new _pyHypothesisReader;

  return myHypReader;
}


//================================================================================
/*!
 * \brief Mesh created by SMESH_Gen
 */
//================================================================================

_pyMesh::_pyMesh(const Handle(_pyCommand) theCreationCmd)
  : _pyObject( theCreationCmd ), myGeomNotInStudy( false )
{
  if ( theCreationCmd->GetMethod() == "CreateMesh" && theGen->IsNotPublished( GetGeom() ))
    myGeomNotInStudy = true;

  // convert my creation command --> smeshpy.Mesh(...)
  Handle(_pyCommand) creationCmd = GetCreationCmd();
  creationCmd->SetObject( SMESH_2smeshpy::SmeshpyName() );
  creationCmd->SetMethod( "Mesh" );
  theGen->SetAccessorMethod( GetID(), _pyMesh::AccessorMethod() );
}

//================================================================================
/*!
 * \brief Mesh created by SMESH_MeshEditor
 */
//================================================================================

_pyMesh::_pyMesh(const Handle(_pyCommand) theCreationCmd, const _pyID& meshId):
  _pyObject(theCreationCmd,meshId), myGeomNotInStudy(false )
{
  if ( theCreationCmd->MethodStartsFrom( "CreateMeshesFrom" ))
  {
    // this mesh depends on the exported mesh
    const TCollection_AsciiString& file = theCreationCmd->GetArg( 1 );
    if ( !file.IsEmpty() )
    {
      ExportedMeshData& exportData = theGen->FindExportedMesh( file );
      addFatherMesh( exportData.myMesh );
      if ( !exportData.myLastComputeCmd.IsNull() )
      {
        // restore cleared Compute() by which the exported mesh was generated
        exportData.myLastComputeCmd->GetString() = exportData.myLastComputeCmdString;
        // protect that Compute() cmd from clearing
        if ( exportData.myMesh->myLastComputeCmd == exportData.myLastComputeCmd )
          exportData.myMesh->myLastComputeCmd.Nullify();
      }
    }
  }
  else if ( theCreationCmd->MethodStartsFrom( "Concatenate" ))
  {
    // this mesh depends on concatenated meshes
    const TCollection_AsciiString& meshIDs = theCreationCmd->GetArg( 1 );
    list< _pyID > idList = theCreationCmd->GetStudyEntries( meshIDs );
    list< _pyID >::iterator meshID = idList.begin();
    for ( ; meshID != idList.end(); ++meshID )
      addFatherMesh( *meshID );
  }
  else if ( theCreationCmd->GetMethod() == "CopyMesh" )
  {
    // this mesh depends on a copied IdSource
    const _pyID& objID = theCreationCmd->GetArg( 1 );
    addFatherMesh( objID );
  }
  else if ( theCreationCmd->GetMethod().Search("MakeMesh") != -1 ||
            theCreationCmd->GetMethod() == "MakeBoundaryMesh" ||
            theCreationCmd->GetMethod() == "MakeBoundaryElements" )
  {
    // this mesh depends on a source mesh
    // (theCreationCmd is already Process()ed by _pyMeshEditor)
    const _pyID& meshID = theCreationCmd->GetObject();
    addFatherMesh( meshID );
  }
    
  // convert my creation command
  Handle(_pyCommand) creationCmd = GetCreationCmd();
  creationCmd->SetObject( SMESH_2smeshpy::SmeshpyName() );
  theGen->SetAccessorMethod( meshId, _pyMesh::AccessorMethod() );
}

//================================================================================
/*!
 * \brief Convert an IDL API command of SMESH::SMESH_Mesh to a method call of python Mesh
  * \param theCommand - Engine method called for this mesh
 */
//================================================================================

void _pyMesh::Process( const Handle(_pyCommand)& theCommand )
{
  // some methods of SMESH_Mesh interface needs special conversion
  // to methods of Mesh python class
  //
  // 1. GetSubMesh(geom, name) + AddHypothesis(geom, algo)
  //     --> in Mesh_Algorithm.Create(mesh, geom, hypo, so)
  // 2. AddHypothesis(geom, hyp)
  //     --> in Mesh_Algorithm.Hypothesis(hyp, args, so)
  // 3. CreateGroupFromGEOM(type, name, grp)
  //     --> in Mesh.Group(grp, name="")
  // 4. ExportToMED(f, auto_groups, version)
  //     --> in Mesh.ExportMED( f, auto_groups, version )
  // 5. etc

  const TCollection_AsciiString& method = theCommand->GetMethod();
  // ----------------------------------------------------------------------
  if ( method == "Compute" ) // in snapshot mode, clear the previous Compute()
  {
    if ( !theGen->IsToKeepAllCommands() ) // !historical
    {
      list< Handle(_pyHypothesis) >::iterator hyp;
      if ( !myLastComputeCmd.IsNull() )
      {
        // check if the previously computed mesh has been edited,
        // if so then we do not clear the previous Compute()
        bool toClear = true;
        if ( myLastComputeCmd->GetMethod() == "Compute" )
        {
          list< Handle(_pyMeshEditor)>::iterator e = myEditors.begin();
          for ( ; e != myEditors.end() && toClear; ++e )
          {
            list< Handle(_pyCommand)>& cmds = (*e)->GetProcessedCmds();
            list< Handle(_pyCommand) >::reverse_iterator cmd = cmds.rbegin();
            if ( cmd != cmds.rend() &&
                 (*cmd)->GetOrderNb() > myLastComputeCmd->GetOrderNb() )
              toClear = false;
          }
        }
        if ( toClear )
        {
          // clear hyp commands called before myLastComputeCmd
          for ( hyp = myHypos.begin(); hyp != myHypos.end(); ++hyp )
            (*hyp)->ComputeDiscarded( myLastComputeCmd );

          myLastComputeCmd->Clear();
        }
      }
      myLastComputeCmd = theCommand;

      for ( hyp = myHypos.begin(); hyp != myHypos.end(); ++hyp )
        (*hyp)->MeshComputed( myLastComputeCmd );
    }
    Flush();
  }
  // ----------------------------------------------------------------------
  else if ( method == "Clear" ) // in snapshot mode, clear all previous commands
  {
    if ( !theGen->IsToKeepAllCommands() ) // !historical
    {
      int untilCmdNb =
        myChildMeshes.empty() ? 0 : myChildMeshes.back()->GetCreationCmd()->GetOrderNb();
      // list< Handle(_pyCommand) >::reverse_iterator cmd = myProcessedCmds.rbegin();
      // for ( ; cmd != myProcessedCmds.rend() && (*cmd)->GetOrderNb() > untilCmdNb; ++cmd )
      //   (*cmd)->Clear();
      if ( !myLastComputeCmd.IsNull() )
      {
        list< Handle(_pyHypothesis) >::iterator hyp;
        for ( hyp = myHypos.begin(); hyp != myHypos.end(); ++hyp )
          (*hyp)->ComputeDiscarded( myLastComputeCmd );

        myLastComputeCmd->Clear();
      }

      list< Handle(_pyMeshEditor)>::iterator e = myEditors.begin();
      for ( ; e != myEditors.end(); ++e )
      {
        list< Handle(_pyCommand)>& cmds = (*e)->GetProcessedCmds();
        list< Handle(_pyCommand) >::reverse_iterator cmd = cmds.rbegin();
        for ( ; cmd != cmds.rend() && (*cmd)->GetOrderNb() > untilCmdNb; ++cmd )
          if ( !(*cmd)->IsEmpty() )
          {
            if ( (*cmd)->GetStudyEntries( (*cmd)->GetResultValue() ).empty() ) // no object created
              (*cmd)->Clear();
          }
      }
      myLastComputeCmd = theCommand; // to clear Clear() the same way as Compute()
    }
  }
  // ----------------------------------------------------------------------
  else if ( method == "GetSubMesh" ) { // collect sub-meshes of the mesh
    Handle(_pySubMesh) subMesh = theGen->FindSubMesh( theCommand->GetResultValue() );
    if ( !subMesh.IsNull() ) {
      subMesh->SetCreator( this );
      mySubmeshes.push_back( subMesh );
    }
  }
  // ----------------------------------------------------------------------
  else if ( method == "GetSubMeshes" ) { // clear as the command does nothing (0023156)
    theCommand->Clear();
  }
  // ----------------------------------------------------------------------
  else if ( method == "AddHypothesis" ) { // mesh.AddHypothesis(geom, HYPO )
    myAddHypCmds.push_back( theCommand );
    // set mesh to hypo
    const _pyID& hypID = theCommand->GetArg( 2 );
    Handle(_pyHypothesis) hyp = theGen->FindHyp( hypID );
    if ( !hyp.IsNull() ) {
      myHypos.push_back( hyp );
      if ( hyp->GetMesh().IsEmpty() )
        hyp->SetMesh( this->GetID() );
    }
  }
  // ----------------------------------------------------------------------
  else if ( method == "CreateGroup" ||
            method == "CreateGroupFromGEOM" ||
            method == "CreateGroupFromFilter" ||
            method == "CreateDimGroup" )
  {
    Handle(_pyGroup) group = new _pyGroup( theCommand );
    myGroups.push_back( group );
    theGen->AddObject( group );
  }
  // ----------------------------------------------------------------------
  // update list of groups
  else if ( method == "GetGroups" )
  {
    bool allGroupsRemoved = true;
    TCollection_AsciiString grIDs = theCommand->GetResultValue();
    list< _pyID >          idList = theCommand->GetStudyEntries( grIDs );
    list< _pyID >::iterator  grID = idList.begin();
    const size_t nbGroupsBefore = myGroups.size();
    Handle(_pyObject) obj;
    for ( ; grID != idList.end(); ++grID )
    {
      obj = theGen->FindObject( *grID );
      if ( obj.IsNull() )
      {
        Handle(_pyGroup) group = new _pyGroup( theCommand, *grID );
        theGen->AddObject( group );
        myGroups.push_back( group );
        obj = group;
      }
      if ( !obj->CanClear() )
        allGroupsRemoved = false;
    }
    if ( nbGroupsBefore == myGroups.size() ) // no new _pyGroup created
      obj->AddProcessedCmd( theCommand ); // to clear theCommand if all groups are removed

    if ( !allGroupsRemoved && !theGen->IsToKeepAllCommands() )
    {
      // check if the preceding command is Compute();
      // if GetGroups() is just after Compute(), this can mean that the groups
      // were created by some algorithm and hence Compute() should not be discarded
      std::list< Handle(_pyCommand) >& cmdList = theGen->GetCommands();
      std::list< Handle(_pyCommand) >::iterator cmd = cmdList.begin();
      while ( (*cmd)->GetMethod() == "GetGroups" )
        ++cmd;
      if ( myLastComputeCmd == (*cmd))
        // protect last Compute() from clearing by the next Compute()
        myLastComputeCmd.Nullify();
    }
  }
  // ----------------------------------------------------------------------
  // notify a group about full removal
  else if ( method == "RemoveGroupWithContents" ||
            method == "RemoveGroup")
  {
    if ( !theGen->IsToKeepAllCommands() ) { // snapshot mode
      const _pyID groupID = theCommand->GetArg( 1 );
      Handle(_pyGroup) grp = Handle(_pyGroup)::DownCast( theGen->FindObject( groupID ));
      if ( !grp.IsNull() )
      {
        if ( method == "RemoveGroupWithContents" )
          grp->RemovedWithContents();
        // to clear RemoveGroup() if the group creation is cleared
        grp->AddProcessedCmd( theCommand );
      }
    }
  }
  // ----------------------------------------------------------------------
  else if ( theCommand->MethodStartsFrom( "Export" ))
  {
    if ( method == "ExportToMED" ||  // ExportToMED()  --> ExportMED()
         method == "ExportToMEDX" )  // ExportToMEDX() --> ExportMED()
    {
      theCommand->SetMethod( "ExportMED" );
      if ( theCommand->GetNbArgs() == 5 )
      {
        // ExportToMEDX(...,autoDimension) -> ExportToMEDX(...,meshPart=None,autoDimension)
        _AString autoDimension = theCommand->GetArg( 5 );
        theCommand->SetArg( 5, "None" );
        theCommand->SetArg( 6, autoDimension );
      }
    }
    else if ( method == "ExportCGNS" )
    { // ExportCGNS(part, ...) -> ExportCGNS(..., part)
      _pyID partID = theCommand->GetArg( 1 );
      int nbArgs = theCommand->GetNbArgs();
      for ( int i = 2; i <= nbArgs; ++i )
        theCommand->SetArg( i-1, theCommand->GetArg( i ));
      theCommand->SetArg( nbArgs, partID );
    }
    else if ( method == "ExportGMF" )
    { // ExportGMF(part,file,bool) -> ExportCGNS(file, part)
      _pyID partID  = theCommand->GetArg( 1 );
      _AString file = theCommand->GetArg( 2 );
      theCommand->RemoveArgs();
      theCommand->SetArg( 1, file );
      theCommand->SetArg( 2, partID );
    }
    else if ( theCommand->MethodStartsFrom( "ExportPartTo" ))
    { // ExportPartTo*(part, ...) -> Export*(..., part)
      //
      // remove "PartTo" from the method
      TCollection_AsciiString newMethod = method;
      newMethod.Remove( /*where=*/7, /*howmany=*/6 );
      theCommand->SetMethod( newMethod );
      // make the 1st arg be the last one (or last but three for ExportMED())
      _pyID partID = theCommand->GetArg( 1 );
      int nbArgs = theCommand->GetNbArgs() - 3 * (newMethod == "ExportMED");
      for ( int i = 2; i <= nbArgs; ++i )
        theCommand->SetArg( i-1, theCommand->GetArg( i ));
      theCommand->SetArg( nbArgs, partID );
    }
    // remember file name
    theGen->AddExportedMesh( theCommand->GetArg( 1 ),
                             ExportedMeshData( this, myLastComputeCmd ));
  }
  // ----------------------------------------------------------------------
  else if ( method == "RemoveHypothesis" ) // (geom, hyp)
  {
    _pyID hypID  = theCommand->GetArg( 2 );
    _pyID geomID = theCommand->GetArg( 1 );
    bool isLocal = ( geomID != GetGeom() );

    // check if this mesh still has corresponding addition command
    Handle(_pyCommand) addCmd;
    list< Handle(_pyCommand) >::iterator cmd;
    list< Handle(_pyCommand) >* addCmds[2] = { &myAddHypCmds, &myNotConvertedAddHypCmds };
    for ( int i = 0; i < 2; ++i )
    {
      list< Handle(_pyCommand )> & addHypCmds = *(addCmds[i]);
      for ( cmd = addHypCmds.begin(); cmd != addHypCmds.end(); )
      {
        bool sameHyp = true;
        if ( hypID != (*cmd)->GetArg( 1 ) && hypID != (*cmd)->GetArg( 2 ))
          sameHyp = false; // other hyp
        if ( (*cmd)->GetNbArgs() == 2 &&
             geomID != (*cmd)->GetArg( 1 ) && geomID != (*cmd)->GetArg( 2 ))
          sameHyp = false; // other geom
        if ( (*cmd)->GetNbArgs() == 1 && isLocal )
          sameHyp = false; // other geom
        if ( sameHyp )
        {
          addCmd = *cmd;
          cmd    = addHypCmds.erase( cmd );
          if ( !theGen->IsToKeepAllCommands() && CanClear() ) {
            addCmd->Clear();
            theCommand->Clear();
          }
          else
          {
            // mesh.AddHypothesis(geom, hyp) --> mesh.AddHypothesis(hyp, geom=0)
            addCmd->RemoveArgs();
            addCmd->SetArg( 1, hypID );
            if ( isLocal )
              addCmd->SetArg( 2, geomID );
          }
        }
        else
        {
          ++cmd;
        }
      }
    }
    Handle(_pyHypothesis) hyp = theGen->FindHyp( hypID );
    if ( !theCommand->IsEmpty() && !hypID.IsEmpty() ) {
      // RemoveHypothesis(geom, hyp) --> RemoveHypothesis( hyp, geom=0 )
      _pyID geom = theCommand->GetArg( 1 );
      theCommand->RemoveArgs();
      theCommand->SetArg( 1, hypID );
      if ( geom != GetGeom() )
        theCommand->SetArg( 2, geom );
    }
    // remove hyp from myHypos
    myHypos.remove( hyp );
  }
  // check for SubMesh order commands
  else if ( method == "GetMeshOrder" || method == "SetMeshOrder" )
  {
    // make commands GetSubMesh() returning sub-meshes be before using sub-meshes
    // by GetMeshOrder() and SetMeshOrder(), since by defalut GetSubMesh()
    // commands are moved at the end of the script
    TCollection_AsciiString subIDs =
      ( method == "SetMeshOrder" ) ? theCommand->GetArg(1) : theCommand->GetResultValue();
    list< _pyID > idList = theCommand->GetStudyEntries( subIDs );
    list< _pyID >::iterator subID = idList.begin();
    for ( ; subID != idList.end(); ++subID )
    {
      Handle(_pySubMesh) subMesh = theGen->FindSubMesh( *subID );
      if ( !subMesh.IsNull() )
        subMesh->Process( theCommand ); // it moves GetSubMesh() before theCommand
    }
  }
  // add accessor method if necessary
  else
  {
    if ( NeedMeshAccess( theCommand ))
      // apply theCommand to the mesh wrapped by smeshpy mesh
      AddMeshAccess( theCommand );
  }
}

//================================================================================
/*!
 * \brief Return True if addition of accesor method is needed
 */
//================================================================================

bool _pyMesh::NeedMeshAccess( const Handle(_pyCommand)& theCommand )
{
  // names of SMESH_Mesh methods fully equal to methods of python class Mesh,
  // so no conversion is needed for them at all:
  static TStringSet sameMethods;
  if ( sameMethods.empty() ) {
    const char * names[] =
      { "ExportDAT","ExportUNV","ExportSTL","ExportSAUV", "RemoveGroup","RemoveGroupWithContents",
        "GetGroups","UnionGroups","IntersectGroups","CutGroups","CreateDimGroup","GetLog","GetId",
        "ClearLog","GetStudyId","HasDuplicatedGroupNamesMED","GetMEDMesh","NbNodes","NbElements",
        "NbEdges","NbEdgesOfOrder","NbFaces","NbFacesOfOrder","NbTriangles",
        "NbTrianglesOfOrder","NbQuadrangles","NbQuadranglesOfOrder","NbPolygons","NbVolumes",
        "NbVolumesOfOrder","NbTetras","NbTetrasOfOrder","NbHexas","NbHexasOfOrder",
        "NbPyramids","NbPyramidsOfOrder","NbPrisms","NbPrismsOfOrder","NbPolyhedrons",
        "NbSubMesh","GetElementsId","GetElementsByType","GetNodesId","GetElementType",
        "GetSubMeshElementsId","GetSubMeshNodesId","GetSubMeshElementType","Dump","GetNodeXYZ",
        "GetNodeInverseElements","GetShapeID","GetShapeIDForElem","GetElemNbNodes",
        "GetElemNode","IsMediumNode","IsMediumNodeOfAnyElem","ElemNbEdges","ElemNbFaces",
        "GetElemFaceNodes", "GetFaceNormal", "FindElementByNodes",
        "IsPoly","IsQuadratic","BaryCenter","GetHypothesisList", "SetAutoColor", "GetAutoColor",
        "Clear", "ConvertToStandalone", "GetMeshOrder", "SetMeshOrder"
        ,"" }; // <- mark of end
    sameMethods.Insert( names );
  }

  return !sameMethods.Contains( theCommand->GetMethod() );
}

//================================================================================
/*!
 * \brief Convert creation and addition of all algos and hypos
 */
//================================================================================

void _pyMesh::Flush()
{
  {
    // get the meshes this mesh depends on via hypotheses
    list< Handle(_pyMesh) > fatherMeshes;
    list< Handle(_pyHypothesis) >::iterator hyp = myHypos.begin();
    for ( ; hyp != myHypos.end(); ++hyp )
      if ( ! (*hyp)->GetReferredMeshesAndGeom( fatherMeshes ))
        myGeomNotInStudy = true;

    list< Handle(_pyMesh) >::iterator m = fatherMeshes.begin();
    for ( ; m != fatherMeshes.end(); ++m )
      addFatherMesh( *m );
    // if ( removedGeom )
    //     SetRemovedFromStudy(); // as reffered geometry not in study
  }
  if ( myGeomNotInStudy )
    return;

  list < Handle(_pyCommand) >::iterator cmd;

  // try to convert algo addition like this:
  // mesh.AddHypothesis(geom, ALGO ) --> ALGO = mesh.Algo()
  for ( cmd = myAddHypCmds.begin(); cmd != myAddHypCmds.end(); ++cmd )
  {
    Handle(_pyCommand) addCmd = *cmd;

    _pyID algoID = addCmd->GetArg( 2 );
    Handle(_pyHypothesis) algo = theGen->FindHyp( algoID );
    if ( algo.IsNull() || !algo->IsAlgo() )
      continue;

    // check and create new algorithm instance if it is already wrapped
    if ( algo->IsWrapped() ) {
      _pyID localAlgoID = theGen->GenerateNewID( algoID );
      TCollection_AsciiString aNewCmdStr = addCmd->GetIndentation() + localAlgoID +
        TCollection_AsciiString( " = " ) + theGen->GetID() +
        TCollection_AsciiString( ".CreateHypothesis( \"" ) + algo->GetAlgoType() +
        TCollection_AsciiString( "\" )" );

      Handle(_pyCommand) newCmd = theGen->AddCommand( aNewCmdStr );
      Handle(_pyAlgorithm) newAlgo = Handle(_pyAlgorithm)::DownCast(theGen->FindHyp( localAlgoID ));
      if ( !newAlgo.IsNull() ) {
        newAlgo->Assign( algo, this->GetID() );
        newAlgo->SetCreationCmd( newCmd );
        algo = newAlgo;
        // set algorithm creation
        theGen->SetCommandBefore( newCmd, addCmd );
        myHypos.push_back( newAlgo );
        if ( !myLastComputeCmd.IsNull() &&
             newCmd->GetOrderNb() == myLastComputeCmd->GetOrderNb() + 1)
          newAlgo->MeshComputed( myLastComputeCmd );
      }
      else
        newCmd->Clear();
    }
    _pyID geom = addCmd->GetArg( 1 );
    bool isLocalAlgo = ( geom != GetGeom() );

    // try to convert
    if ( algo->Addition2Creation( addCmd, this->GetID() )) // OK
    {
      // wrapped algo is created after mesh creation
      GetCreationCmd()->AddDependantCmd( addCmd );

      if ( isLocalAlgo ) {
        // mesh.AddHypothesis(geom, ALGO ) --> mesh.AlgoMethod(geom)
        addCmd->SetArg( addCmd->GetNbArgs() + 1,
                        TCollection_AsciiString( "geom=" ) + geom );
        // sm = mesh.GetSubMesh(geom, name) --> sm = ALGO.GetSubMesh()
        list < Handle(_pySubMesh) >::iterator smIt;
        for ( smIt = mySubmeshes.begin(); smIt != mySubmeshes.end(); ++smIt ) {
          Handle(_pySubMesh) subMesh = *smIt;
          Handle(_pyCommand) subCmd = subMesh->GetCreationCmd();
          if ( geom == subCmd->GetArg( 1 )) {
            subCmd->SetObject( algo->GetID() );
            subCmd->RemoveArgs();
            subMesh->SetCreator( algo );
          }
        }
      }
    }
    else // KO - ALGO was already created
    {
      // mesh.AddHypothesis(geom, ALGO) --> mesh.AddHypothesis(ALGO, geom=0)
      addCmd->RemoveArgs();
      addCmd->SetArg( 1, algoID );
      if ( isLocalAlgo )
        addCmd->SetArg( 2, geom );
      myNotConvertedAddHypCmds.push_back( addCmd );
    }
  }

  // try to convert hypo addition like this:
  // mesh.AddHypothesis(geom, HYPO ) --> HYPO = algo.Hypo()
  for ( cmd = myAddHypCmds.begin(); cmd != myAddHypCmds.end(); ++cmd )
  {
    Handle(_pyCommand) addCmd = *cmd;
    _pyID hypID = addCmd->GetArg( 2 );
    Handle(_pyHypothesis) hyp = theGen->FindHyp( hypID );
    if ( hyp.IsNull() || hyp->IsAlgo() )
      continue;
    bool converted = hyp->Addition2Creation( addCmd, this->GetID() );
    if ( !converted ) {
      // mesh.AddHypothesis(geom, HYP) --> mesh.AddHypothesis(HYP, geom=0)
      _pyID geom = addCmd->GetArg( 1 );
      addCmd->RemoveArgs();
      addCmd->SetArg( 1, hypID );
      if ( geom != GetGeom() )
        addCmd->SetArg( 2, geom );
      myNotConvertedAddHypCmds.push_back( addCmd );
    }
  }

  myAddHypCmds.clear();
  mySubmeshes.clear();

  // flush hypotheses
  list< Handle(_pyHypothesis) >::iterator hyp = myHypos.begin();
  for ( hyp = myHypos.begin(); hyp != myHypos.end(); ++hyp )
    (*hyp)->Flush();
}

//================================================================================
/*!
 * \brief Sets myIsPublished of me and of all objects depending on me.
 */
//================================================================================

void _pyMesh::SetRemovedFromStudy(const bool isRemoved)
{
  _pyObject::SetRemovedFromStudy(isRemoved);

  list< Handle(_pySubMesh) >::iterator sm = mySubmeshes.begin();
  for ( ; sm != mySubmeshes.end(); ++sm )
    (*sm)->SetRemovedFromStudy(isRemoved);

  list< Handle(_pyGroup) >::iterator gr = myGroups.begin();
  for ( ; gr != myGroups.end(); ++gr )
    (*gr)->SetRemovedFromStudy(isRemoved);

  list< Handle(_pyMesh) >::iterator m = myChildMeshes.begin();
  for ( ; m != myChildMeshes.end(); ++m )
    (*m)->SetRemovedFromStudy(isRemoved);

  list< Handle(_pyMeshEditor)>::iterator e = myEditors.begin();
  for ( ; e != myEditors.end(); ++e )
    (*e)->SetRemovedFromStudy(isRemoved);
}

//================================================================================
/*!
 * \brief Return true if none of myChildMeshes is in study
 */
//================================================================================

bool _pyMesh::CanClear()
{
  if ( IsInStudy() )
    return false;

  list< Handle(_pyMesh) >::iterator m = myChildMeshes.begin();
  for ( ; m != myChildMeshes.end(); ++m )
    if ( !(*m)->CanClear() )
      return false;

  return true;
}

//================================================================================
/*!
 * \brief Clear my commands and commands of mesh editor
 */
//================================================================================

void _pyMesh::ClearCommands()
{
  if ( !CanClear() )
  {
    if ( !IsInStudy() )
    {
      // mark all sub-objects as not removed, except child meshes
      list< Handle(_pyMesh) > children;
      children.swap( myChildMeshes );
      SetRemovedFromStudy( false );
      children.swap( myChildMeshes );
    }
    return;
  }
  _pyObject::ClearCommands();

  list< Handle(_pySubMesh) >::iterator sm = mySubmeshes.begin();
  for ( ; sm != mySubmeshes.end(); ++sm )
    (*sm)->ClearCommands();
  
  list< Handle(_pyGroup) >::iterator gr = myGroups.begin();
  for ( ; gr != myGroups.end(); ++gr )
    (*gr)->ClearCommands();

  list< Handle(_pyMeshEditor)>::iterator e = myEditors.begin();
  for ( ; e != myEditors.end(); ++e )
    (*e)->ClearCommands();
}

//================================================================================
/*!
 * \brief Add a father mesh by ID
 */
//================================================================================

void _pyMesh::addFatherMesh( const _pyID& meshID )
{
  if ( !meshID.IsEmpty() && meshID != GetID() )
    addFatherMesh( Handle(_pyMesh)::DownCast( theGen->FindObject( meshID )));
}

//================================================================================
/*!
 * \brief Add a father mesh
 */
//================================================================================

void _pyMesh::addFatherMesh( const Handle(_pyMesh)& mesh )
{
  if ( !mesh.IsNull() && mesh->GetID() != GetID() )
  {
    //myFatherMeshes.push_back( mesh );
    mesh->myChildMeshes.push_back( this );

    // protect last Compute() from clearing by the next Compute()
    mesh->myLastComputeCmd.Nullify();
  }
}

//================================================================================
/*!
 * \brief MeshEditor convert its commands to ones of mesh
 */
//================================================================================

_pyMeshEditor::_pyMeshEditor(const Handle(_pyCommand)& theCreationCmd):
  _pyObject( theCreationCmd )
{
  myMesh = theCreationCmd->GetObject();
  myCreationCmdStr = theCreationCmd->GetString();
  theCreationCmd->Clear();

  Handle(_pyMesh) mesh = ObjectToMesh( theGen->FindObject( myMesh ));
  if ( !mesh.IsNull() )
    mesh->AddEditor( this );
}

//================================================================================
/*!
 * \brief convert its commands to ones of mesh
 */
//================================================================================

void _pyMeshEditor::Process( const Handle(_pyCommand)& theCommand)
{
  // Names of SMESH_MeshEditor methods fully equal to methods of the python class Mesh, so
  // commands calling these methods are converted to calls of Mesh methods without
  // additional modifs, only object is changed from MeshEditor to Mesh.
  static TStringSet sameMethods;
  if ( sameMethods.empty() ) {
    const char * names[] = {
      "RemoveElements","RemoveNodes","RemoveOrphanNodes",
      "AddNode","Add0DElement","AddEdge","AddFace","AddPolygonalFace","AddBall",
      "AddVolume","AddPolyhedralVolume","AddPolyhedralVolumeByFaces",
      "MoveNode", "MoveClosestNodeToPoint",
      "InverseDiag","DeleteDiag","Reorient","ReorientObject","Reorient2DBy3D",
      "TriToQuad","TriToQuadObject", "QuadTo4Tri", "SplitQuad","SplitQuadObject",
      "BestSplit","Smooth","SmoothObject","SmoothParametric","SmoothParametricObject",
      "ConvertToQuadratic","ConvertFromQuadratic","RenumberNodes","RenumberElements",
      "RotationSweep","RotationSweepObject","RotationSweepObject1D","RotationSweepObject2D",
      "ExtrusionSweep","AdvancedExtrusion","ExtrusionSweepObject","ExtrusionSweepObject1D",
      "ExtrusionByNormal", "ExtrusionSweepObject2D","ExtrusionAlongPath","ExtrusionAlongPathObject",
      "ExtrusionAlongPathX","ExtrusionAlongPathObject1D","ExtrusionAlongPathObject2D",
      "ExtrusionSweepObjects","RotationSweepObjects","ExtrusionAlongPathObjects",
      "Mirror","MirrorObject","Translate","TranslateObject","Rotate","RotateObject",
      "FindCoincidentNodes","MergeNodes","FindEqualElements",
      "MergeElements","MergeEqualElements","SewFreeBorders","SewConformFreeBorders",
      "FindCoincidentFreeBorders", "SewCoincidentFreeBorders",
      "SewBorderToSide","SewSideElements","ChangeElemNodes","GetLastCreatedNodes",
      "GetLastCreatedElems",
      "MirrorMakeMesh","MirrorObjectMakeMesh","TranslateMakeMesh","TranslateObjectMakeMesh",
      "Scale","ScaleMakeMesh","RotateMakeMesh","RotateObjectMakeMesh","MakeBoundaryMesh",
      "MakeBoundaryElements", "SplitVolumesIntoTetra","SplitHexahedraIntoPrisms",
      "DoubleElements","DoubleNodes","DoubleNode","DoubleNodeGroup","DoubleNodeGroups",
      "DoubleNodeElem","DoubleNodeElemInRegion","DoubleNodeElemGroup",
      "DoubleNodeElemGroupInRegion","DoubleNodeElemGroups","DoubleNodeElemGroupsInRegion",
      "DoubleNodesOnGroupBoundaries","CreateFlatElementsOnFacesGroups","CreateHoleSkin"
      ,"" }; // <- mark of the end
    sameMethods.Insert( names );
  }

  // names of SMESH_MeshEditor commands in which only a method name must be replaced
  TStringMap diffMethods;
  if ( diffMethods.empty() ) {
    const char * orig2newName[] = {
      // original name --------------> new name
      "ExtrusionAlongPathObjX"      , "ExtrusionAlongPathX",
      "FindCoincidentNodesOnPartBut", "FindCoincidentNodesOnPart",
      "ConvertToQuadraticObject"    , "ConvertToQuadratic",
      "ConvertFromQuadraticObject"  , "ConvertFromQuadratic",
      "Create0DElementsOnAllNodes"  , "Add0DElementsToAllNodes",
      ""};// <- mark of the end
    diffMethods.Insert( orig2newName );
  }

  // names of SMESH_MeshEditor methods which differ from methods of Mesh class
  // only by last two arguments
  static TStringSet diffLastTwoArgsMethods;
  if (diffLastTwoArgsMethods.empty() ) {
    const char * names[] = {
      "MirrorMakeGroups","MirrorObjectMakeGroups",
      "TranslateMakeGroups","TranslateObjectMakeGroups","ScaleMakeGroups",
      "RotateMakeGroups","RotateObjectMakeGroups",
      ""};// <- mark of the end
    diffLastTwoArgsMethods.Insert( names );
  }

  // only a method name is to change?
  const TCollection_AsciiString & method = theCommand->GetMethod();
  bool isPyMeshMethod = sameMethods.Contains( method );
  if ( !isPyMeshMethod )
  {
    TCollection_AsciiString newMethod = diffMethods.Value( method );
    if (( isPyMeshMethod = ( newMethod.Length() > 0 )))
      theCommand->SetMethod( newMethod );
  }
  // ConvertToBiQuadratic(...) -> ConvertToQuadratic(...,True)
  if ( !isPyMeshMethod && (method == "ConvertToBiQuadratic" || method == "ConvertToBiQuadraticObject") )
  {
    isPyMeshMethod = true;
    theCommand->SetMethod( method.SubString( 1, 9) + method.SubString( 12, method.Length()));
    theCommand->SetArg( theCommand->GetNbArgs() + 1, "True" );
  }

  if ( !isPyMeshMethod )
  {
    // Replace SMESH_MeshEditor "*MakeGroups" functions by the Mesh
    // functions with the flag "theMakeGroups = True" like:
    // SMESH_MeshEditor.CmdMakeGroups => Mesh.Cmd(...,True)
    int pos = method.Search("MakeGroups");
    if( pos != -1)
    {
      isPyMeshMethod = true;
      bool is0DmethId  = ( method == "ExtrusionSweepMakeGroups0D" );
      bool is0DmethObj = ( method == "ExtrusionSweepObject0DMakeGroups");

      // 1. Remove "MakeGroups" from the Command
      TCollection_AsciiString aMethod = theCommand->GetMethod();
      int nbArgsToAdd = diffLastTwoArgsMethods.Contains(aMethod) ? 2 : 1;
      
      if(is0DmethObj)
        pos = pos-2;  //Remove "0D" from the Command too
      aMethod.Trunc(pos-1);
      theCommand->SetMethod(aMethod);

      // 2. And add last "True" argument(s)
      while(nbArgsToAdd--)
        theCommand->SetArg(theCommand->GetNbArgs()+1,"True");
      if( is0DmethId || is0DmethObj )
        theCommand->SetArg(theCommand->GetNbArgs()+1,"True");
    }
  }

  // ExtrusionSweep0D()       -> ExtrusionSweep()
  // ExtrusionSweepObject0D() -> ExtrusionSweepObject()
  if ( !isPyMeshMethod && ( method == "ExtrusionSweep0D"  ||
                            method == "ExtrusionSweepObject0D" ))
  {
    isPyMeshMethod = true;
    theCommand->SetMethod( method.SubString( 1, method.Length()-2));
    theCommand->SetArg(theCommand->GetNbArgs()+1,"False");  //sets flag "MakeGroups = False"
    theCommand->SetArg(theCommand->GetNbArgs()+1,"True");  //sets flag "IsNode = True"
  }

  // DoubleNode...New(...) -> DoubleNode...(...,True)
  if ( !isPyMeshMethod && ( method == "DoubleNodeElemGroupNew"  ||
                            method == "DoubleNodeElemGroupsNew" ||
                            method == "DoubleNodeGroupNew"      ||
                            method == "DoubleNodeGroupsNew"     ||
                            method == "DoubleNodeElemGroup2New" ||
                            method == "DoubleNodeElemGroups2New"))
  {
    isPyMeshMethod = true;
    const int excessLen = 3 + int( method.Value( method.Length()-3 ) == '2' );
    theCommand->SetMethod( method.SubString( 1, method.Length()-excessLen));
    if ( excessLen == 3 )
    {
      theCommand->SetArg(theCommand->GetNbArgs()+1,"True");
    }
    else if ( theCommand->GetArg(4) == "0" ||
              theCommand->GetArg(5) == "0" )
    {
      // [ nothing, Group ] = DoubleNodeGroup2New(,,,False, True) ->
      // Group = DoubleNodeGroup2New(,,,False, True)
      _pyID groupID = theCommand->GetResultValue( 1 + int( theCommand->GetArg(4) == "0"));
      theCommand->SetResultValue( groupID );
    }
  }
  // FindAmongElementsByPoint(meshPart, x, y, z, elementType) ->
  // FindElementsByPoint(x, y, z, elementType, meshPart)
  if ( !isPyMeshMethod && method == "FindAmongElementsByPoint" )
  {
    isPyMeshMethod = true;
    theCommand->SetMethod( "FindElementsByPoint" );
    // make the 1st arg be the last one
    _pyID partID = theCommand->GetArg( 1 );
    int nbArgs = theCommand->GetNbArgs();
    for ( int i = 2; i <= nbArgs; ++i )
      theCommand->SetArg( i-1, theCommand->GetArg( i ));
    theCommand->SetArg( nbArgs, partID );
  }
  // Reorient2D( mesh, dir, face, point ) -> Reorient2D( mesh, dir, faceORpoint )
  if ( !isPyMeshMethod && method == "Reorient2D" )
  {
    isPyMeshMethod = true;
    _AString mesh  = theCommand->GetArg( 1 );
    _AString dir   = theCommand->GetArg( 2 );
    _AString face  = theCommand->GetArg( 3 );
    _AString point = theCommand->GetArg( 4 );
    theCommand->RemoveArgs();
    theCommand->SetArg( 1, mesh );
    theCommand->SetArg( 2, dir );
    if ( face.Value(1) == '-' || face.Value(1) == '0' ) // invalid: face <= 0
      theCommand->SetArg( 3, point );
    else
      theCommand->SetArg( 3, face );
  }

  if ( method == "QuadToTri" || method == "QuadToTriObject" )
  {
    isPyMeshMethod = true;
    int crit_arg = theCommand->GetNbArgs();
    const _AString& crit = theCommand->GetArg(crit_arg);
    if (crit.Search("MaxElementLength2D") != -1)
      theCommand->SetArg(crit_arg, "");
  }

  if ( isPyMeshMethod )
  {
    theCommand->SetObject( myMesh );
  }
  else
  {
    // editor creation command is needed only if any editor function is called
    theGen->AddMeshAccessorMethod( theCommand ); // for *Object() methods
    if ( !myCreationCmdStr.IsEmpty() ) {
      GetCreationCmd()->GetString() = myCreationCmdStr;
      myCreationCmdStr.Clear();
    }
  }
}

//================================================================================
/*!
 * \brief Return true if my mesh can be removed
 */
//================================================================================

bool _pyMeshEditor::CanClear()
{
  Handle(_pyMesh) mesh = ObjectToMesh( theGen->FindObject( myMesh ));
  return mesh.IsNull() ? true : mesh->CanClear();
}

//================================================================================
/*!
 * \brief _pyHypothesis constructor
  * \param theCreationCmd -
 */
//================================================================================

_pyHypothesis::_pyHypothesis(const Handle(_pyCommand)& theCreationCmd):
  _pyObject( theCreationCmd ), myCurCrMethod(0)
{
  myIsAlgo = myIsWrapped = /*myIsConverted = myIsLocal = myDim = */false;
}

//================================================================================
/*!
 * \brief Creates algorithm or hypothesis
  * \param theCreationCmd - The engine command creating a hypothesis
  * \retval Handle(_pyHypothesis) - Result _pyHypothesis
 */
//================================================================================

Handle(_pyHypothesis) _pyHypothesis::NewHypothesis( const Handle(_pyCommand)& theCreationCmd)
{
  // theCreationCmd: CreateHypothesis( "theHypType", "theLibName" )
  ASSERT (( theCreationCmd->GetMethod() == "CreateHypothesis"));

  Handle(_pyHypothesis) hyp, algo;

  // "theHypType"
  const TCollection_AsciiString & hypTypeQuoted = theCreationCmd->GetArg( 1 );
  if ( hypTypeQuoted.IsEmpty() )
    return hyp;
  // theHypType
  TCollection_AsciiString  hypType =
    hypTypeQuoted.SubString( 2, hypTypeQuoted.Length() - 1 );

  algo = new _pyAlgorithm( theCreationCmd );
  hyp  = new _pyHypothesis( theCreationCmd );

  if ( hypType == "NumberOfSegments" ) {
    hyp = new _pyNumberOfSegmentsHyp( theCreationCmd );
    hyp->SetConvMethodAndType( "NumberOfSegments", "Regular_1D");
    // arg of SetNumberOfSegments() will become the 1-st arg of hyp creation command
    hyp->AddArgMethod( "SetNumberOfSegments" );
    // arg of SetScaleFactor() will become the 2-nd arg of hyp creation command
    hyp->AddArgMethod( "SetScaleFactor" );
    hyp->AddArgMethod( "SetReversedEdges" );
    // same for ""CompositeSegment_1D:
    hyp->SetConvMethodAndType( "NumberOfSegments", "CompositeSegment_1D");
    hyp->AddArgMethod( "SetNumberOfSegments" );
    hyp->AddArgMethod( "SetScaleFactor" );
    hyp->AddArgMethod( "SetReversedEdges" );
  }
  else if ( hypType == "SegmentLengthAroundVertex" ) {
    hyp = new _pySegmentLengthAroundVertexHyp( theCreationCmd );
    hyp->SetConvMethodAndType( "LengthNearVertex", "Regular_1D" );
    hyp->AddArgMethod( "SetLength" );
    // same for ""CompositeSegment_1D:
    hyp->SetConvMethodAndType( "LengthNearVertex", "CompositeSegment_1D");
    hyp->AddArgMethod( "SetLength" );
  }
  else if ( hypType == "LayerDistribution2D" ) {
    hyp = new _pyLayerDistributionHypo( theCreationCmd, "Get2DHypothesis" );
    hyp->SetConvMethodAndType( "LayerDistribution", "RadialQuadrangle_1D2D");
  }
  else if ( hypType == "LayerDistribution" ) {
    hyp = new _pyLayerDistributionHypo( theCreationCmd, "Get3DHypothesis" );
    hyp->SetConvMethodAndType( "LayerDistribution", "RadialPrism_3D");
  }
  else if ( hypType == "CartesianParameters3D" ) {
    hyp = new _pyComplexParamHypo( theCreationCmd );
    hyp->SetConvMethodAndType( "SetGrid", "Cartesian_3D");
    for ( int iArg = 0; iArg < 4; ++iArg )
      hyp->setCreationArg( iArg+1, "[]");
    hyp->AddAccumulativeMethod( "SetGrid" );
    hyp->AddAccumulativeMethod( "SetGridSpacing" );
  }
  else
  {
    hyp = theGen->GetHypothesisReader()->GetHypothesis( hypType, theCreationCmd );
  }

  return algo->IsValid() ? algo : hyp;
}

//================================================================================
/*!
 * \brief Returns true if addition of this hypothesis to a given mesh can be
 *        wrapped into hypothesis creation
 */
//================================================================================

bool _pyHypothesis::IsWrappable(const _pyID& theMesh) const
{
  if ( !myIsWrapped && myMesh == theMesh && IsInStudy() )
  {
    Handle(_pyObject) pyMesh = theGen->FindObject( myMesh );
    if ( !pyMesh.IsNull() && pyMesh->IsInStudy() )
      return true;
  }
  return false;
}

//================================================================================
/*!
 * \brief Convert the command adding a hypothesis to mesh into a smesh command
  * \param theCmd - The command like mesh.AddHypothesis( geom, hypo )
  * \param theAlgo - The algo that can create this hypo
  * \retval bool - false if the command cant be converted
 */
//================================================================================

bool _pyHypothesis::Addition2Creation( const Handle(_pyCommand)& theCmd,
                                       const _pyID&              theMesh)
{
  ASSERT(( theCmd->GetMethod() == "AddHypothesis" ));

  if ( !IsWrappable( theMesh ))
    return false;

  myGeom = theCmd->GetArg( 1 );

  Handle(_pyHypothesis) algo;
  if ( !IsAlgo() ) {
    // find algo created on myGeom in theMesh
    algo = theGen->FindAlgo( myGeom, theMesh, this );
    if ( algo.IsNull() )
      return false;
    // attach hypothesis creation command to be after algo creation command
    // because it can be new created instance of algorithm
    algo->GetCreationCmd()->AddDependantCmd( theCmd );
  }
  myIsWrapped = true;

  // mesh.AddHypothesis(geom,hyp) --> hyp = <theMesh or algo>.myCreationMethod(args)
  theCmd->SetResultValue( GetID() );
  theCmd->SetObject( IsAlgo() ? theMesh : algo->GetID());
  theCmd->SetMethod( IsAlgo() ? GetAlgoCreationMethod() : GetCreationMethod( algo->GetAlgoType() ));
  // set args (geom will be set by _pyMesh calling this method)
  theCmd->RemoveArgs();
  for ( size_t i = 0; i < myCurCrMethod->myArgs.size(); ++i ) {
    if ( !myCurCrMethod->myArgs[ i ].IsEmpty() )
      theCmd->SetArg( i+1, myCurCrMethod->myArgs[ i ]);
    else
      theCmd->SetArg( i+1, "[]");
  }
  // set a new creation command
  GetCreationCmd()->Clear();
  // replace creation command by wrapped instance
  // please note, that hypothesis attaches to algo creation command (see upper)
  SetCreationCmd( theCmd );


  // clear commands setting arg values
  list < Handle(_pyCommand) >::iterator argCmd = myArgCommands.begin();
  for ( ; argCmd != myArgCommands.end(); ++argCmd )
    (*argCmd)->Clear();

  // set unknown arg commands after hypo creation
  Handle(_pyCommand) afterCmd = myIsWrapped ? theCmd : GetCreationCmd();
  list<Handle(_pyCommand)>::iterator cmd = myUnusedCommands.begin();
  for ( ; cmd != myUnusedCommands.end(); ++cmd ) {
    afterCmd->AddDependantCmd( *cmd );
  }

  return myIsWrapped;
}

//================================================================================
/*!
 * \brief Remember hypothesis parameter values
 * \param theCommand - The called hypothesis method
 */
//================================================================================

void _pyHypothesis::Process( const Handle(_pyCommand)& theCommand)
{
  ASSERT( !myIsAlgo );
  if ( !theGen->IsToKeepAllCommands() )
    rememberCmdOfParameter( theCommand );
  // set args
  bool usedCommand = false;
  TType2CrMethod::iterator type2meth = myAlgoType2CreationMethod.begin();
  for ( ; type2meth != myAlgoType2CreationMethod.end(); ++type2meth )
  {
    CreationMethod& crMethod = type2meth->second;
    for ( size_t i = 0; i < crMethod.myArgMethods.size(); ++i ) {
      if ( crMethod.myArgMethods[ i ] == theCommand->GetMethod() ) {
        if ( !usedCommand )
          myArgCommands.push_back( theCommand );
        usedCommand = true;
        while ( crMethod.myArgs.size() < i+1 )
          crMethod.myArgs.push_back( "None" );
        crMethod.myArgs[ i ] = theCommand->GetArg( crMethod.myArgNb[i] );
      }
    }
  }
  if ( !usedCommand )
    myUnusedCommands.push_back( theCommand );
}

//================================================================================
/*!
 * \brief Finish conversion
 */
//================================================================================

void _pyHypothesis::Flush()
{
  if ( !IsAlgo() )
  {
    list < Handle(_pyCommand) >::iterator cmd = myArgCommands.begin();
    for ( ; cmd != myArgCommands.end(); ++cmd ) {
      // Add access to a wrapped mesh
      theGen->AddMeshAccessorMethod( *cmd );
      // Add access to a wrapped algorithm
      theGen->AddAlgoAccessorMethod( *cmd );
    }
    cmd = myUnusedCommands.begin();
    for ( ; cmd != myUnusedCommands.end(); ++cmd ) {
      // Add access to a wrapped mesh
      theGen->AddMeshAccessorMethod( *cmd );
      // Add access to a wrapped algorithm
      theGen->AddAlgoAccessorMethod( *cmd );
    }
  }
  // forget previous hypothesis modifications
  myArgCommands.clear();
  myUnusedCommands.clear();
}

//================================================================================
/*!
 * \brief clear creation, arg and unkown commands
 */
//================================================================================

void _pyHypothesis::ClearAllCommands()
{
  GetCreationCmd()->Clear();
  list<Handle(_pyCommand)>::iterator cmd = myArgCommands.begin();
  for ( ; cmd != myArgCommands.end(); ++cmd )
    ( *cmd )->Clear();
  cmd = myUnusedCommands.begin();
  for ( ; cmd != myUnusedCommands.end(); ++cmd )
    ( *cmd )->Clear();
}


//================================================================================
/*!
 * \brief Assign fields of theOther to me except myIsWrapped
 */
//================================================================================

void _pyHypothesis::Assign( const Handle(_pyHypothesis)& theOther,
                            const _pyID&                 theMesh )
{
  // myCreationCmd = theOther->myCreationCmd;
  myIsAlgo                  = theOther->myIsAlgo;
  myIsWrapped               = false;
  myGeom                    = theOther->myGeom;
  myMesh                    = theMesh;
  myAlgoType2CreationMethod = theOther->myAlgoType2CreationMethod;
  myAccumulativeMethods     = theOther->myAccumulativeMethods;
  //myUnusedCommands          = theOther->myUnusedCommands;
  // init myCurCrMethod
  GetCreationMethod( theOther->GetAlgoType() );
}

//================================================================================
/*!
 * \brief Analyze my erasability depending on myReferredObjs
 */
//================================================================================

bool _pyHypothesis::CanClear()
{
  if ( IsInStudy() )
  {
    list< Handle(_pyObject) >::iterator obj = myReferredObjs.begin();
    for ( ; obj != myReferredObjs.end(); ++obj )
      if ( (*obj)->CanClear() )
        return true;
    return false;
  }
  return true;
}

//================================================================================
/*!
 * \brief Clear my commands depending on usage by meshes
 */
//================================================================================

void _pyHypothesis::ClearCommands()
{
  // if ( !theGen->IsToKeepAllCommands() )
  // {
  //   bool isUsed = false;
  //   int lastComputeOrder = 0;
  //   list<Handle(_pyCommand) >::iterator cmd = myComputeCmds.begin();
  //   for ( ; cmd != myComputeCmds.end(); ++cmd )
  //     if ( ! (*cmd)->IsEmpty() )
  //     {
  //       isUsed = true;
  //       if ( (*cmd)->GetOrderNb() > lastComputeOrder )
  //         lastComputeOrder = (*cmd)->GetOrderNb();
  //     }
  //   if ( !isUsed )
  //   {
  //     SetRemovedFromStudy( true );
  //   }
  //   else
  //   {
  //     // clear my commands invoked after lastComputeOrder
  //     // map<TCollection_AsciiString, list< Handle(_pyCommand) > >::iterator m2c;
  //     // for ( m2c = myMeth2Commands.begin(); m2c != myMeth2Commands.end(); ++m2c )
  //     // {
  //     //   list< Handle(_pyCommand)> & cmds = m2c->second;
  //     //   if ( !cmds.empty() && cmds.back()->GetOrderNb() > lastComputeOrder )
  //     //     cmds.back()->Clear();
  //     // }
  //   }
  // }
  _pyObject::ClearCommands();
}

//================================================================================
/*!
 * \brief Find arguments that are objects like mesh, group, geometry
 *  \param meshes - referred meshes (directly or indirrectly)
 *  \retval bool - false if a referred geometry is not in the study
 */
//================================================================================

bool _pyHypothesis::GetReferredMeshesAndGeom( list< Handle(_pyMesh) >& meshes )
{
  if ( IsAlgo() ) return true;

  bool geomPublished = true;
  vector< _AString > args;
  TType2CrMethod::iterator type2meth = myAlgoType2CreationMethod.begin();
  for ( ; type2meth != myAlgoType2CreationMethod.end(); ++type2meth )
  {
    CreationMethod& crMethod = type2meth->second;
    args.insert( args.end(), crMethod.myArgs.begin(), crMethod.myArgs.end());
  }
  list<Handle(_pyCommand)>::iterator cmd = myUnusedCommands.begin();
  for ( ; cmd != myUnusedCommands.end(); ++cmd ) {
    for ( int nb = (*cmd)->GetNbArgs(); nb; --nb )
      args.push_back( (*cmd)->GetArg( nb ));
  }

  for ( size_t i = 0; i < args.size(); ++i )
  {
    list< _pyID > idList = _pyCommand::GetStudyEntries( args[ i ]);
    if ( idList.empty() && !args[ i ].IsEmpty() )
      idList.push_back( args[ i ]);
    list< _pyID >::iterator id = idList.begin();
    for ( ; id != idList.end(); ++id )
    {
      Handle(_pyObject)   obj = theGen->FindObject( *id );
      if ( obj.IsNull() ) obj = theGen->FindHyp( *id );
      if ( obj.IsNull() )
      {
        if ( theGen->IsGeomObject( *id ) && theGen->IsNotPublished( *id ))
          geomPublished = false;
      }
      else
      {
        myReferredObjs.push_back( obj );
        Handle(_pyMesh) mesh = ObjectToMesh( obj );
        if ( !mesh.IsNull() )
          meshes.push_back( mesh );
        // prevent clearing not published hyps referred e.g. by "LayerDistribution"
        else if ( obj->IsKind( STANDARD_TYPE( _pyHypothesis )) && this->IsInStudy() )
          obj->SetRemovedFromStudy( false );
      }
    }
  }
  return geomPublished;
}

//================================================================================
/*!
 * \brief Remember theCommand setting a parameter
 */
//================================================================================

void _pyHypothesis::rememberCmdOfParameter( const Handle(_pyCommand) & theCommand )
{
  // parameters are discriminated by method name
  _AString method = theCommand->GetMethod();
  if ( myAccumulativeMethods.count( method ))
    return; // this method adds values and not override the previus value

  // discriminate commands setting different parameters via one method
  // by passing parameter names like e.g. SetOption("size", "0.2")
  if ( theCommand->GetString().FirstLocationInSet( "'\"", 1, theCommand->Length() ) &&
       theCommand->GetNbArgs() > 1 )
  {
    // mangle method by appending a 1st textual arg
    for ( int iArg = 1; iArg <= theCommand->GetNbArgs(); ++iArg )
    {
      const TCollection_AsciiString& arg = theCommand->GetArg( iArg );
      if ( arg.Value(1) != '\"' && arg.Value(1) != '\'' ) continue;
      if ( !isalpha( arg.Value(2))) continue;
      method += arg;
      break;
    }
  }
  // parameters are discriminated by method name
  list< Handle(_pyCommand)>& cmds = myMeth2Commands[ method /*theCommand->GetMethod()*/ ];
  if ( !cmds.empty() && !isCmdUsedForCompute( cmds.back() ))
  {
    cmds.back()->Clear(); // previous parameter value has not been used
    cmds.back() = theCommand;
  }
  else
  {
    cmds.push_back( theCommand );
  }
}

//================================================================================
/*!
 * \brief Return true if a setting parameter command ha been used to compute mesh
 */
//================================================================================

bool _pyHypothesis::isCmdUsedForCompute( const Handle(_pyCommand) & cmd,
                                         _pyCommand::TAddr          avoidComputeAddr ) const
{
  bool isUsed = false;
  map< _pyCommand::TAddr, list<Handle(_pyCommand) > >::const_iterator addr2cmds =
    myComputeAddr2Cmds.begin();
  for ( ; addr2cmds != myComputeAddr2Cmds.end() && !isUsed; ++addr2cmds )
  {
    if ( addr2cmds->first == avoidComputeAddr ) continue;
    const list<Handle(_pyCommand)> & cmds = addr2cmds->second;
    isUsed = ( std::find( cmds.begin(), cmds.end(), cmd ) != cmds.end() );
  }
  return isUsed;
}

//================================================================================
/*!
 * \brief Save commands setting parameters as they are used for a mesh computation
 */
//================================================================================

void _pyHypothesis::MeshComputed( const Handle(_pyCommand)& theComputeCmd )
{
  myComputeCmds.push_back( theComputeCmd );
  list<Handle(_pyCommand)>& savedCmds = myComputeAddr2Cmds[ theComputeCmd->GetAddress() ];

  map<TCollection_AsciiString, list< Handle(_pyCommand) > >::iterator m2c;
  for ( m2c = myMeth2Commands.begin(); m2c != myMeth2Commands.end(); ++m2c )
    savedCmds.push_back( m2c->second.back() );
}

//================================================================================
/*!
 * \brief Clear commands setting parameters as a mesh computed using them is cleared
 */
//================================================================================

void _pyHypothesis::ComputeDiscarded( const Handle(_pyCommand)& theComputeCmd )
{
  list<Handle(_pyCommand)>& savedCmds = myComputeAddr2Cmds[ theComputeCmd->GetAddress() ];

  list<Handle(_pyCommand)>::iterator cmd = savedCmds.begin();
  for ( ; cmd != savedCmds.end(); ++cmd )
  {
    // check if a cmd has been used to compute another mesh
    if ( isCmdUsedForCompute( *cmd, theComputeCmd->GetAddress() ))
      continue;
    // check if a cmd is a sole command setting its parameter;
    // don't use method name for search as it can change
    map<TCollection_AsciiString, list<Handle(_pyCommand)> >::iterator
      m2cmds = myMeth2Commands.begin();
    for ( ; m2cmds != myMeth2Commands.end(); ++m2cmds )
    {
      list< Handle(_pyCommand)>& cmds = m2cmds->second;
      list< Handle(_pyCommand)>::iterator cmdIt = std::find( cmds.begin(), cmds.end(), *cmd );
      if ( cmdIt != cmds.end() )
      {
        if ( cmds.back() != *cmd )
        {
          cmds.erase( cmdIt );
          (*cmd)->Clear();
        }
        break;
      }
    }
  }
  myComputeAddr2Cmds.erase( theComputeCmd->GetAddress() );
}

//================================================================================
/*!
 * \brief Sets an argNb-th argument of current creation command
 *  \param argNb - argument index countered from 1
 */
//================================================================================

void _pyHypothesis::setCreationArg( const int argNb, const _AString& arg )
{
  if ( myCurCrMethod )
  {
    while ( (int) myCurCrMethod->myArgs.size() < argNb )
      myCurCrMethod->myArgs.push_back( "None" );
    if ( arg.IsEmpty() )
      myCurCrMethod->myArgs[ argNb-1 ] = "None";
    else
      myCurCrMethod->myArgs[ argNb-1 ] = arg;
  }
}


//================================================================================
/*!
 * \brief Remember hypothesis parameter values
 * \param theCommand - The called hypothesis method
 */
//================================================================================

void _pyComplexParamHypo::Process( const Handle(_pyCommand)& theCommand)
{
  if ( GetAlgoType() == "Cartesian_3D" )
  {
    // CartesianParameters3D hyp

    if ( theCommand->GetMethod() == "SetSizeThreshold"  ||
         theCommand->GetMethod() == "SetToAddEdges" )
    {
      int iEdges = ( theCommand->GetMethod().Value( 4 ) == 'T' );
      setCreationArg( 4+iEdges, theCommand->GetArg( 1 ));
      myArgCommands.push_back( theCommand );
      return;
    }
    if ( theCommand->GetMethod() == "SetGrid" ||
         theCommand->GetMethod() == "SetGridSpacing" )
    {
      TCollection_AsciiString axis = theCommand->GetArg( theCommand->GetNbArgs() );
      int iArg = axis.Value(1) - '0';
      if ( theCommand->GetMethod() == "SetGrid" )
      {
        setCreationArg( 1+iArg, theCommand->GetArg( 1 ));
      }
      else
      {
        myCurCrMethod->myArgs[ iArg ] = "[ ";
        myCurCrMethod->myArgs[ iArg ] += theCommand->GetArg( 1 );
        myCurCrMethod->myArgs[ iArg ] += ", ";
        myCurCrMethod->myArgs[ iArg ] += theCommand->GetArg( 2 );
        myCurCrMethod->myArgs[ iArg ] += "]";
      }
      myArgCommands.push_back( theCommand );
      //rememberCmdOfParameter( theCommand ); -- these commands are marked as
      //                                  accumulative, else, if the creation
      //                 is not converted, commands for axes 1 and 2 are lost
      return;
    }
  }

  if( theCommand->GetMethod() == "SetLength" )
  {
    // NOW it is OBSOLETE
    // ex: hyp.SetLength(start, 1)
    //     hyp.SetLength(end,   0)
    ASSERT(( theCommand->GetArg( 2 ).IsIntegerValue() ));
    int i = 1 - theCommand->GetArg( 2 ).IntegerValue();
    TType2CrMethod::iterator type2meth = myAlgoType2CreationMethod.begin();
    for ( ; type2meth != myAlgoType2CreationMethod.end(); ++type2meth )
    {
      CreationMethod& crMethod = type2meth->second;
      while ( (int) crMethod.myArgs.size() < i+1 )
          crMethod.myArgs.push_back( "[]" );
        crMethod.myArgs[ i ] = theCommand->GetArg( 1 ); // arg value
    }
    myArgCommands.push_back( theCommand );
  }
  else
  {
    _pyHypothesis::Process( theCommand );
  }
}
//================================================================================
/*!
 * \brief Clear SetObjectEntry() as it is called by methods of Mesh_Segment
 */
//================================================================================

void _pyComplexParamHypo::Flush()
{
  list < Handle(_pyCommand) >::iterator cmd;
  if ( IsWrapped() )
  {
    for ( cmd = myUnusedCommands.begin(); cmd != myUnusedCommands.end(); ++cmd )
      if ((*cmd)->GetMethod() == "SetObjectEntry" )
        (*cmd)->Clear();
  }

  // if ( GetAlgoType() == "Cartesian_3D" )
  // {
  //   _pyID algo = myCreationCmd->GetObject();
  //   for ( cmd = myProcessedCmds.begin(); cmd != myProcessedCmds.end(); ++cmd )
  //   {
  //     if ( IsWrapped() )
  //     {
  //       StructToList( *cmd, /*checkMethod=*/false );
  //       const _AString & method = (*cmd)->GetMethod();
  //       if ( method == "SetFixedPoint" )
  //         (*cmd)->SetObject( algo );
  //     }
  //   }
  // }
}

//================================================================================
/*!
 * \brief Convert methods of 1D hypotheses to my own methods
 * \param theCommand - The called hypothesis method
 */
//================================================================================

void _pyLayerDistributionHypo::Process( const Handle(_pyCommand)& theCommand)
{
  if ( theCommand->GetMethod() != "SetLayerDistribution" )
    return;

  const _pyID& hyp1dID = theCommand->GetArg( 1 );
  // Handle(_pyHypothesis) hyp1d = theGen->FindHyp( hyp1dID );
  // if ( hyp1d.IsNull() && ! my1dHyp.IsNull()) // apparently hypId changed at study restoration
  // {
  //   TCollection_AsciiString cmd =
  //     my1dHyp->GetCreationCmd()->GetIndentation() + hyp1dID + " = " + my1dHyp->GetID();
  //   Handle(_pyCommand) newCmd = theGen->AddCommand( cmd );
  //   theGen->SetCommandAfter( newCmd, my1dHyp->GetCreationCmd() );
  //   hyp1d = my1dHyp;
  // }
  // else if ( !my1dHyp.IsNull() && hyp1dID != my1dHyp->GetID() )
  // {
  //   // 1D hypo is already set, so distribution changes and the old
  //   // 1D hypo is thrown away
  //   my1dHyp->ClearAllCommands();
  // }
  // my1dHyp = hyp1d;
  // //my1dHyp->SetRemovedFromStudy( false );

  // if ( !myArgCommands.empty() )
  //   myArgCommands.back()->Clear();
  myCurCrMethod->myArgs.push_back( hyp1dID );
  myArgCommands.push_back( theCommand );
}

//================================================================================
/*!
 * \brief
  * \param theAdditionCmd - command to be converted
  * \param theMesh - mesh instance
  * \retval bool - status
 */
//================================================================================

bool _pyLayerDistributionHypo::Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                                  const _pyID&              theMesh)
{
  myIsWrapped = false;

  if ( my1dHyp.IsNull() )
    return false;

  // set "SetLayerDistribution()" after addition cmd
  theAdditionCmd->AddDependantCmd( myArgCommands.front() );

  _pyID geom = theAdditionCmd->GetArg( 1 );

  Handle(_pyHypothesis) algo = theGen->FindAlgo( geom, theMesh, this );
  if ( !algo.IsNull() )
  {
    my1dHyp->SetMesh( theMesh );
    my1dHyp->SetConvMethodAndType(my1dHyp->GetAlgoCreationMethod().ToCString(),
                                  algo->GetAlgoType().ToCString());
    if ( !my1dHyp->Addition2Creation( theAdditionCmd, theMesh ))
      return false;

    // clear "SetLayerDistribution()" cmd
    myArgCommands.back()->Clear();

    // Convert my creation => me = RadialPrismAlgo.Get3DHypothesis()

    // find RadialPrism algo created on <geom> for theMesh
    GetCreationCmd()->SetObject( algo->GetID() );
    GetCreationCmd()->SetMethod( myAlgoMethod );
    GetCreationCmd()->RemoveArgs();
    theAdditionCmd->AddDependantCmd( GetCreationCmd() );
    myIsWrapped = true;
  }
  return myIsWrapped;
}

//================================================================================
/*!
 * \brief
 */
//================================================================================

void _pyLayerDistributionHypo::Flush()
{
  // as creation of 1D hyp was written later then it's edition,
  // we need to find all it's edition calls and process them
  list< Handle(_pyCommand) >::iterator cmd = myArgCommands.begin();
  _pyID prevNewName;
  for ( cmd = myArgCommands.begin(); cmd != myArgCommands.end(); ++cmd )
  {    
    const _pyID& hyp1dID = (*cmd)->GetArg( 1 );
    if ( hyp1dID.IsEmpty() ) continue;

    Handle(_pyHypothesis) hyp1d = theGen->FindHyp( hyp1dID );

    // make a new name for 1D hyp = "HypType" + "_Distribution"
    _pyID newName;
    if ( hyp1d.IsNull() ) // apparently hypId changed at study restoration
    {
      if ( prevNewName.IsEmpty() ) continue;
      newName = prevNewName;
    }
    else
    {
      if ( hyp1d->IsWrapped() ) {
        newName = hyp1d->GetCreationCmd()->GetMethod();
      }
      else {
        TCollection_AsciiString hypTypeQuoted = hyp1d->GetCreationCmd()->GetArg(1);
        newName = hypTypeQuoted.SubString( 2, hypTypeQuoted.Length() - 1 );
      }
      newName += "_Distribution";
      prevNewName = newName;
    
      hyp1d->GetCreationCmd()->SetResultValue( newName );
    }
    list< Handle(_pyCommand) >& cmds = theGen->GetCommands();
    list< Handle(_pyCommand) >::iterator cmdIt = cmds.begin();
    for ( ; cmdIt != cmds.end(); ++cmdIt ) {
      const _pyID& objID = (*cmdIt)->GetObject();
      if ( objID == hyp1dID ) {
        if ( !hyp1d.IsNull() )
        {
          hyp1d->Process( *cmdIt );
          hyp1d->GetCreationCmd()->AddDependantCmd( *cmdIt );
        }
        ( *cmdIt )->SetObject( newName );
      }
    }
    // Set new hyp name to SetLayerDistribution(hyp1dID) cmd
    (*cmd)->SetArg( 1, newName );
  }
}

//================================================================================
/*!
 * \brief additionally to Addition2Creation, clears SetDistrType() command
  * \param theCmd - AddHypothesis() command
  * \param theMesh - mesh to which a hypothesis is added
  * \retval bool - convertion result
 */
//================================================================================

bool _pyNumberOfSegmentsHyp::Addition2Creation( const Handle(_pyCommand)& theCmd,
                                                const _pyID&              theMesh)
{
  if ( IsWrappable( theMesh ) && myCurCrMethod->myArgs.size() > 1 ) {
    // scale factor (2-nd arg) is provided: clear SetDistrType(1) command
    bool scaleDistrType = false;
    list<Handle(_pyCommand)>::reverse_iterator cmd = myUnusedCommands.rbegin();
    for ( ; cmd != myUnusedCommands.rend(); ++cmd ) {
      if ( (*cmd)->GetMethod() == "SetDistrType" ) {
        if ( (*cmd)->GetArg( 1 ) == "1" ) {
          scaleDistrType = true;
          (*cmd)->Clear();
        }
        else if ( !scaleDistrType ) {
          // distribution type changed: remove scale factor from args
          TType2CrMethod::iterator type2meth = myAlgoType2CreationMethod.begin();
          for ( ; type2meth != myAlgoType2CreationMethod.end(); ++type2meth )
          {
            CreationMethod& crMethod = type2meth->second;
            if ( crMethod.myArgs.size() == 2 )
              crMethod.myArgs.pop_back();
          }
          break;
        }
      }
    }
  }
  return _pyHypothesis::Addition2Creation( theCmd, theMesh );
}

//================================================================================
/*!
 * \brief remove repeated commands defining distribution
 */
//================================================================================

void _pyNumberOfSegmentsHyp::Flush()
{
  // find number of the last SetDistrType() command
  list<Handle(_pyCommand)>::reverse_iterator cmd = myUnusedCommands.rbegin();
  int distrTypeNb = 0;
  for ( ; !distrTypeNb && cmd != myUnusedCommands.rend(); ++cmd )
    if ( (*cmd)->GetMethod() == "SetDistrType" ) {
      if ( cmd != myUnusedCommands.rbegin() )
        distrTypeNb = (*cmd)->GetOrderNb();
    }
    else if (IsWrapped() && (*cmd)->GetMethod() == "SetObjectEntry" ) {
      (*cmd)->Clear();
    }
  // clear commands before the last SetDistrType()
  list<Handle(_pyCommand)> * cmds[2] = { &myArgCommands, &myUnusedCommands };
  set< int > treatedCmdNbs; // avoid treating same cmd twice
  for ( int i = 0; i < 2; ++i ) {
    set<TCollection_AsciiString> uniqueMethods;
    list<Handle(_pyCommand)> & cmdList = *cmds[i];
    for ( cmd = cmdList.rbegin(); cmd != cmdList.rend(); ++cmd )
    {
      if ( !treatedCmdNbs.insert( (*cmd)->GetOrderNb() ).second )
        continue;// avoid treating same cmd twice
      bool clear = ( (*cmd)->GetOrderNb() < distrTypeNb );
      const TCollection_AsciiString& method = (*cmd)->GetMethod();
      if ( !clear || method == "SetNumberOfSegments" ) {
        bool isNewInSet = uniqueMethods.insert( method ).second;
        clear = !isNewInSet;
      }
      if ( clear )
        (*cmd)->Clear();
    }
    cmdList.clear();
  }
}

//================================================================================
/*!
 * \brief Convert the command adding "SegmentLengthAroundVertex" to mesh
 * into regular1D.LengthNearVertex( length, vertex )
  * \param theCmd - The command like mesh.AddHypothesis( vertex, SegmentLengthAroundVertex )
  * \param theMesh - The mesh needing this hypo
  * \retval bool - false if the command cant be converted
 */
//================================================================================

bool _pySegmentLengthAroundVertexHyp::Addition2Creation( const Handle(_pyCommand)& theCmd,
                                                         const _pyID&              theMeshID)
{
  if ( IsWrappable( theMeshID )) {

    _pyID vertex = theCmd->GetArg( 1 );

    // the problem here is that segment algo will not be found
    // by pyHypothesis::Addition2Creation() for <vertex>, so we try to find
    // geometry where segment algorithm is assigned
    Handle(_pyHypothesis) algo;
    _pyID geom = vertex;
    while ( algo.IsNull() && !geom.IsEmpty()) {
      // try to find geom as a father of <vertex>
      geom = FatherID( geom );
      algo = theGen->FindAlgo( geom, theMeshID, this );
    }
    if ( algo.IsNull() )
      return false; // also possible to find geom as brother of veretex...
    // set geom instead of vertex
    theCmd->SetArg( 1, geom );

    // set vertex as a second arg
    if ( myCurCrMethod->myArgs.size() < 1) setCreationArg( 1, "1" ); // :(
    setCreationArg( 2, vertex );

    // mesh.AddHypothesis(vertex, SegmentLengthAroundVertex) -->
    // theMeshID.LengthNearVertex( length, vertex )
    return _pyHypothesis::Addition2Creation( theCmd, theMeshID );
  }
  return false;
}

//================================================================================
/*!
 * \brief _pyAlgorithm constructor
 * \param theCreationCmd - The command like "algo = smeshgen.CreateHypothesis(type,lib)"
 */
//================================================================================

_pyAlgorithm::_pyAlgorithm(const Handle(_pyCommand)& theCreationCmd)
  : _pyHypothesis( theCreationCmd )
{
  myIsAlgo = true;
}

//================================================================================
/*!
 * \brief Convert the command adding an algorithm to mesh
  * \param theCmd - The command like mesh.AddHypothesis( geom, algo )
  * \param theMesh - The mesh needing this algo
  * \retval bool - false if the command cant be converted
 */
//================================================================================

bool _pyAlgorithm::Addition2Creation( const Handle(_pyCommand)& theCmd,
                                      const _pyID&              theMeshID)
{
  // mesh.AddHypothesis(geom,algo) --> theMeshID.myCreationMethod()
  if ( _pyHypothesis::Addition2Creation( theCmd, theMeshID )) {
    theGen->SetAccessorMethod( GetID(), "GetAlgorithm()" );
    return true;
  }
  return false;
}

//================================================================================
/*!
 * \brief Return starting position of a part of python command
  * \param thePartIndex - The index of command part
  * \retval int - Part position
 */
//================================================================================

int _pyCommand::GetBegPos( int thePartIndex ) const
{
  if ( IsEmpty() )
    return EMPTY;
  if ( myBegPos.Length() < thePartIndex )
    return UNKNOWN;
  ASSERT( thePartIndex > 0 );
  return myBegPos( thePartIndex );
}

//================================================================================
/*!
 * \brief Store starting position of a part of python command
  * \param thePartIndex - The index of command part
  * \param thePosition - Part position
 */
//================================================================================

void _pyCommand::SetBegPos( int thePartIndex, int thePosition )
{
  while ( myBegPos.Length() < thePartIndex )
    myBegPos.Append( UNKNOWN );
  ASSERT( thePartIndex > 0 );
  myBegPos( thePartIndex ) = thePosition;
}

//================================================================================
/*!
 * \brief Returns whitespace symbols at the line beginning
  * \retval TCollection_AsciiString - result
 */
//================================================================================

TCollection_AsciiString _pyCommand::GetIndentation()
{
  int end = 1;
  //while ( end <= Length() && isblank( myString.Value( end )))
  //ANA: isblank() function isn't provided in VC2010 compiler
  while ( end <= Length() && ( myString.Value( end ) == ' ' || myString.Value( end ) == '\t') )
    ++end;
  return ( end == 1 ) ? _AString("") : myString.SubString( 1, end - 1 );
}

//================================================================================
/*!
 * \brief Return substring of python command looking like ResultValue = Obj.Meth()
  * \retval const TCollection_AsciiString & - ResultValue substring
 */
//================================================================================

const TCollection_AsciiString & _pyCommand::GetResultValue()
{
  if ( GetBegPos( RESULT_IND ) == UNKNOWN )
  {
    SetBegPos( RESULT_IND, EMPTY );
    int begPos, endPos = myString.Location( "=", 1, Length() );
    if ( endPos )
    {
      begPos = 1;
      while ( begPos < endPos && isspace( myString.Value( begPos ))) ++begPos;
      if ( begPos < endPos )
      {
        SetBegPos( RESULT_IND, begPos );
        --endPos;
        while ( begPos < endPos && isspace( myString.Value( endPos ))) --endPos;
        myRes = myString.SubString( begPos, endPos );
      }
    }
  }
  return myRes;
}

//================================================================================
/*!
 * \brief Return number of python command result value ResultValue = Obj.Meth()
 */
//================================================================================

int _pyCommand::GetNbResultValues()
{
  GetResultValue(1);
  return myResults.Length();
}


//================================================================================
/*!
 * \brief Return substring of python command looking like
 *  ResultValue1 , ResultValue2,... = Obj.Meth() with res index
 * \retval const TCollection_AsciiString & - ResultValue with res index substring
 */
//================================================================================
const _AString& _pyCommand::GetResultValue(int res)
{
  if ( GetResultValue().IsEmpty() )
    return theEmptyString;

  if ( myResults.IsEmpty() )
  {
    int begPos = 1;
    if ( SkipSpaces( myRes, begPos ) && myRes.Value( begPos ) == '[' )
      ++begPos; // skip [, else the whole list is returned
    while ( begPos < myRes.Length() ) {
      _AString result = GetWord( myRes, begPos, true );
      begPos += result.Length();
      // if(res == Nb) {
      //   result.RemoveAll('[');
      //   result.RemoveAll(']');
      //   return result;
      // }
      // if(Nb>res)
      //   break;
      myResults.Append( result );
    }
  }
  if ( res > 0 && res <= myResults.Length() )
    return myResults( res );
  return theEmptyString;
}

//================================================================================
/*!
 * \brief Return substring of python command looking like ResVal = Object.Meth()
 * \retval const TCollection_AsciiString & - Object substring
 */
//================================================================================

const TCollection_AsciiString & _pyCommand::GetObject()
{
  if ( GetBegPos( OBJECT_IND ) == UNKNOWN )
  {
    // beginning
    int begPos = GetBegPos( RESULT_IND );
    if ( begPos < 1 ) {
      begPos = myString.Location( "=", 1, Length() ) + 1;
      // is '=' in the string argument (for example, name) or not
      int nb1 = 0; // number of ' character at the left of =
      int nb2 = 0; // number of " character at the left of =
      for ( int i = 1; i < begPos-1; i++ ) {
        if ( myString.Value( i )=='\'' )
          nb1 += 1;
        else if ( myString.Value( i )=='"' )
          nb2 += 1;
      }
      // if number of ' or " is not divisible by 2,
      // then get an object at the start of the command
      if ( nb1 % 2 != 0 || nb2 % 2 != 0 )
        begPos = 1;
    }
    else {
      begPos += myRes.Length();
    }
    myObj = GetWord( myString, begPos, true );
    if ( begPos != EMPTY )
    {
      // check if object is complex,
      // so far consider case like "smesh.Method()"
      if ( int bracketPos = myString.Location( "(", begPos, Length() )) {
        //if ( bracketPos==0 ) bracketPos = Length();
        int dotPos = begPos+myObj.Length();
        while ( dotPos+1 < bracketPos ) {
          if ( int pos = myString.Location( ".", dotPos+1, bracketPos ))
            dotPos = pos;
          else
            break;
        }
        if ( dotPos > begPos+myObj.Length() )
          myObj = myString.SubString( begPos, dotPos-1 );
      }
    }
    // 1st word after '=' is an object
    // else // no method -> no object
    // {
    //   myObj.Clear();
    //   begPos = EMPTY;
    // }
    // store
    SetBegPos( OBJECT_IND, begPos );
  }
  //SCRUTE(myObj);
  return myObj;
}

//================================================================================
/*!
 * \brief Return substring of python command looking like ResVal = Obj.Method()
  * \retval const TCollection_AsciiString & - Method substring
 */
//================================================================================

const TCollection_AsciiString & _pyCommand::GetMethod()
{
  if ( GetBegPos( METHOD_IND ) == UNKNOWN )
  {
    // beginning
    int begPos = GetBegPos( OBJECT_IND );
    bool forward = true;
    if ( begPos < 1 ) {
      begPos = myString.Location( "(", 1, Length() ) - 1;
      forward = false;
    }
    else {
      begPos += myObj.Length();
    }
    // store
    myMeth = GetWord( myString, begPos, forward );
    SetBegPos( METHOD_IND, begPos );
  }
  //SCRUTE(myMeth);
  return myMeth;
}

//================================================================================
/*!
 * \brief Returns true if there are brackets after the method
 */
//================================================================================

bool _pyCommand::IsMethodCall()
{
  if ( GetMethod().IsEmpty() )
    return false;
  const char* s = myString.ToCString() + GetBegPos( METHOD_IND ) + myMeth.Length() - 1;
  return ( s[0] == '(' || s[1] == '(' );
}

//================================================================================
/*!
 * \brief Return substring of python command looking like ResVal = Obj.Meth(Arg1,...)
  * \retval const TCollection_AsciiString & - Arg<index> substring
 */
//================================================================================

const TCollection_AsciiString & _pyCommand::GetArg( int index )
{
  if ( GetBegPos( ARG1_IND ) == UNKNOWN )
  {
    // Find all args

    int pos = GetBegPos( METHOD_IND ) + myMeth.Length();
    if ( pos < 1 )
      pos = myString.Location( "(", 1, Length() );
    else
      --pos;

    // we are at or before '(', skip it if present
    if ( pos > 0 ) {
      while ( pos <= Length() && myString.Value( pos ) != '(' ) ++pos;
      if ( pos > Length() )
        pos = 0;
    }
    if ( pos < 1 ) {
      SetBegPos( ARG1_IND, 0 ); // even no '('
      return theEmptyString;
    }
    ++pos;

    list< TCollection_AsciiString > separatorStack( 1, ",)");
    bool ignoreNesting = false;
    int prevPos = pos;
    while ( pos <= Length() )
    {
      const char chr = myString.Value( pos );

      if ( separatorStack.back().Location( chr, 1, separatorStack.back().Length()))
      {
        if ( separatorStack.size() == 1 ) // a comma dividing args or a terminal ')' found
        {
          while ( pos-1 >= prevPos && isspace( myString.Value( prevPos )))
            ++prevPos;
          TCollection_AsciiString arg;
          if ( pos-1 >= prevPos ) {
            arg = myString.SubString( prevPos, pos-1 );
            arg.RightAdjust(); // remove spaces
            arg.LeftAdjust();
          }
          if ( !arg.IsEmpty() || chr == ',' )
          {
            SetBegPos( ARG1_IND + myArgs.Length(), prevPos );
            myArgs.Append( arg );
          }
          if ( chr == ')' )
            break;
          prevPos = pos+1;
        }
        else // end of nesting args found
        {
          separatorStack.pop_back();
          ignoreNesting = false;
        }
      }
      else if ( !ignoreNesting )
      {
        switch ( chr ) {
        case '(' : separatorStack.push_back(")"); break;
        case '[' : separatorStack.push_back("]"); break;
        case '\'': separatorStack.push_back("'");  ignoreNesting=true; break;
        case '"' : separatorStack.push_back("\""); ignoreNesting=true; break;
        default:;
        }
      }
      ++pos;
    }
  }
  if ( myArgs.Length() < index )
    return theEmptyString;
  return myArgs( index );
}

//================================================================================
/*!
 * \brief Return position where arguments begin
 */
//================================================================================

int _pyCommand::GetArgBeginning() const
{
  int pos = GetBegPos( ARG1_IND );
  if ( pos == UNKNOWN )
  {
    pos = GetBegPos( METHOD_IND ) + myMeth.Length();
    if ( pos < 1 )
      pos = myString.Location( "(", 4, Length() ); // 4 = strlen("b.c(")
  }
  return pos;
}

//================================================================================
/*!
 * \brief Check if char is a word part
  * \param c - The character to check
  * \retval bool - The check result
 */
//================================================================================

static inline bool isWord(const char c, const bool dotIsWord)
{
  return
    !isspace(c) && c != ',' && c != '=' && c != ')' && c != '(' && ( dotIsWord || c != '.');
}

//================================================================================
/*!
 * \brief Looks for a word in the string and returns word's beginning
  * \param theString - The input string
  * \param theStartPos - The position to start the search, returning word's beginning
  * \param theForward - The search direction
  * \retval TCollection_AsciiString - The found word
 */
//================================================================================

TCollection_AsciiString _pyCommand::GetWord( const _AString & theString,
                                             int &            theStartPos,
                                             const bool       theForward,
                                             const bool       dotIsWord )
{
  int beg = theStartPos, end = theStartPos;
  theStartPos = EMPTY;
  if ( beg < 1 || beg > theString.Length() )
    return theEmptyString;

  if ( theForward ) { // search forward
    // beg
    while ( beg <= theString.Length() && !isWord( theString.Value( beg ), dotIsWord))
      ++beg;
    if ( beg > theString.Length() )
      return theEmptyString; // no word found
    // end
    end = beg + 1;
    char begChar = theString.Value( beg );
    if ( begChar == '"' || begChar == '\'' || begChar == '[') {
      char endChar = ( begChar == '[' ) ? ']' : begChar;
      // end is at the corresponding quoting mark or bracket
      while ( end < theString.Length() &&
              ( theString.Value( end ) != endChar || theString.Value( end-1 ) == '\\'))
        ++end;
    }
    else {
      while ( end <= theString.Length() && isWord( theString.Value( end ), dotIsWord))
        ++end;
      --end;
    }
  }
  else {  // search backward
    // end
    while ( end > 0 && !isWord( theString.Value( end ), dotIsWord))
      --end;
    if ( end == 0 )
      return theEmptyString; // no word found
    beg = end - 1;
    char endChar = theString.Value( end );
    if ( endChar == '"' || endChar == '\'' || endChar == ']') {
      char begChar = ( endChar == ']' ) ? '[' : endChar;
      // beg is at the corresponding quoting mark
      while ( beg > 1 &&
              ( theString.Value( beg ) != begChar || theString.Value( beg-1 ) == '\\'))
        --beg;
    }
    else {
      while ( beg > 0 && isWord( theString.Value( beg ), dotIsWord))
        --beg;
      ++beg;
    }
  }
  theStartPos = beg;
  //cout << theString << " ---- " << beg << " - " << end << endl;
  return theString.SubString( beg, end );
}

//================================================================================
/*!
 * \brief Returns true if the string looks like a study entry
 */
//================================================================================

bool _pyCommand::IsStudyEntry( const TCollection_AsciiString& str )
{
  if ( str.Length() < 5 ) return false;

  int nbColons = 0, isColon;
  for ( int i = 1; i <= str.Length(); ++i )
  {
    char c = str.Value(i);
    if (!( isColon = (c == ':')) && ( c < '0' || c > '9' ))
      return false;
    nbColons += isColon;
  }
  return nbColons > 2 && str.Length()-nbColons > 2;
}

//================================================================================
/*!
 * \brief Returns true if the string looks like an object ID but not like a list,
 *        string, command etc.
 */
//================================================================================

bool _pyCommand::IsID( const TCollection_AsciiString& str )
{
  if ( str.Length() < 1 ) return false;

  const char* s = str.ToCString();

  for ( int i = 0; i < str.Length(); ++i )
    if ( !IsIDChar( s[i] ))
      return false;

  return true;
}

//================================================================================
/*!
 * \brief Finds entries in a sting
 */
//================================================================================

std::list< _pyID > _pyCommand::GetStudyEntries( const TCollection_AsciiString& str )
{
  std::list< _pyID > resList;
  int pos = 0;
  while ( ++pos <= str.Length() )
  {
    if ( !isdigit( str.Value( pos ))) continue;
    if ( pos != 1 && ( isalpha( str.Value( pos-1 ) || str.Value( pos-1 ) == ':'))) continue;

    int end = pos;
    while ( ++end <= str.Length() && ( isdigit( str.Value( end )) || str.Value( end ) == ':' ));
    _pyID entry = str.SubString( pos, end-1 );
    pos = end;
    if ( IsStudyEntry( entry ))
      resList.push_back( entry );
  }
  return resList;
}

//================================================================================
/*!
 * \brief Look for position where not space char is
  * \param theString - The string
  * \param thePos - The position to search from and which returns result
  * \retval bool - false if there are only space after thePos in theString
 */
//================================================================================

bool _pyCommand::SkipSpaces( const TCollection_AsciiString & theString, int & thePos )
{
  if ( thePos < 1 || thePos > theString.Length() )
    return false;

  while ( thePos <= theString.Length() && isspace( theString.Value( thePos )))
    ++thePos;

  return thePos <= theString.Length();
}

//================================================================================
/*!
 * \brief Modify a part of the command
  * \param thePartIndex - The index of the part
  * \param thePart - The new part string
  * \param theOldPart - The old part
 */
//================================================================================

void _pyCommand::SetPart(int thePartIndex, const TCollection_AsciiString& thePart,
                        TCollection_AsciiString& theOldPart)
{
  int pos = GetBegPos( thePartIndex );
  if ( pos <= Length() && theOldPart != thePart)
  {
    TCollection_AsciiString seperator;
    if ( pos < 1 ) {
      pos = GetBegPos( thePartIndex + 1 );
      if ( pos < 1 ) return;
      switch ( thePartIndex ) {
      case RESULT_IND: seperator = " = "; break;
      case OBJECT_IND: seperator = "."; break;
      case METHOD_IND: seperator = "()"; break;
      default:;
      }
    }
    myString.Remove( pos, theOldPart.Length() );
    if ( !seperator.IsEmpty() )
      myString.Insert( pos , seperator );
    myString.Insert( pos, thePart );
    // update starting positions of the following parts
    int posDelta = thePart.Length() + seperator.Length() - theOldPart.Length();
    for ( int i = thePartIndex + 1; i <= myBegPos.Length(); ++i ) {
      if ( myBegPos( i ) > 0 )
        myBegPos( i ) += posDelta;
    }
    theOldPart = thePart;
  }
}

//================================================================================
/*!
 * \brief Set agrument
  * \param index - The argument index, it counts from 1
  * \param theArg - The argument string
 */
//================================================================================

void _pyCommand::SetArg( int index, const TCollection_AsciiString& theArg)
{
  FindAllArgs();
  int argInd = ARG1_IND + index - 1;
  int pos = GetBegPos( argInd );
  if ( pos < 1 ) // no index-th arg exist, append inexistent args
  {
    // find a closing parenthesis
    if ( GetNbArgs() != 0 && index <= GetNbArgs() ) {
      int lastArgInd = GetNbArgs();
      pos = GetBegPos( ARG1_IND + lastArgInd  - 1 ) + GetArg( lastArgInd ).Length();
      while ( pos > 0 && pos <= Length() && myString.Value( pos ) != ')' )
        ++pos;
    }
    else {
      pos = Length();
      while ( pos > 0 && myString.Value( pos ) != ')' )
        --pos;
    }
    if ( pos < 1 || myString.Value( pos ) != ')' ) { // no parentheses at all
      myString += "()";
      pos = Length();
    }
    while ( myArgs.Length() < index ) {
      if ( myArgs.Length() )
        myString.Insert( pos++, "," );
      myArgs.Append("None");
      myString.Insert( pos, myArgs.Last() );
      SetBegPos( ARG1_IND + myArgs.Length() - 1, pos );
      pos += myArgs.Last().Length();
    }
  }
  SetPart( argInd, theArg, myArgs( index ));
}

//================================================================================
/*!
 * \brief Empty arg list
 */
//================================================================================

void _pyCommand::RemoveArgs()
{
  if ( int pos = myString.Location( '(', Max( 1, GetBegPos( METHOD_IND )), Length() ))
    myString.Trunc( pos );
  myString += ")";
  myArgs.Clear();
  if ( myBegPos.Length() >= ARG1_IND )
    myBegPos.Remove( ARG1_IND, myBegPos.Length() );
}

//================================================================================
/*!
 * \brief Comment a python command
 */
//================================================================================

void _pyCommand::Comment()
{
  if ( IsEmpty() ) return;

  int i = 1;
  while ( i <= Length() && isspace( myString.Value(i) )) ++i;
  if ( i <= Length() )
  {
    myString.Insert( i, "#" );
    for ( int iPart = 1; iPart <= myBegPos.Length(); ++iPart )
    {
      int begPos = GetBegPos( iPart );
      if ( begPos != UNKNOWN && begPos != EMPTY )
        SetBegPos( iPart, begPos + 1 );
    }
  }
}

//================================================================================
/*!
 * \brief Set dependent commands after this one
 */
//================================================================================

bool _pyCommand::SetDependentCmdsAfter() const
{
  bool orderChanged = false;
  list< Handle(_pyCommand)>::const_reverse_iterator cmd = myDependentCmds.rbegin();
  for ( ; cmd != myDependentCmds.rend(); ++cmd ) {
    if ( (*cmd)->GetOrderNb() < GetOrderNb() ) {
      orderChanged = true;
      theGen->SetCommandAfter( *cmd, this );
      (*cmd)->SetDependentCmdsAfter();
    }
  }
  return orderChanged;
}
//================================================================================
/*!
 * \brief Insert accessor method after theObjectID
  * \param theObjectID - id of the accessed object
  * \param theAcsMethod - name of the method giving access to the object
  * \retval bool - false if theObjectID is not found in the command string
 */
//================================================================================

bool _pyCommand::AddAccessorMethod( _pyID theObjectID, const char* theAcsMethod )
{
  if ( !theAcsMethod )
    return false;
  // start object search from the object, i.e. ignore result
  GetObject();
  int beg = GetBegPos( OBJECT_IND );
  if ( beg < 1 || beg > Length() )
    return false;
  bool added = false;
  while (( beg = myString.Location( theObjectID, beg, Length() )))
  {
    // check that theObjectID is not just a part of a longer ID
    int afterEnd = beg + theObjectID.Length();
    Standard_Character c = myString.Value( afterEnd );
    if ( !IsIDChar( c ))
    {
      // check if accessor method already present
      if ( c != '.' ||
           myString.Location( (char*) theAcsMethod, afterEnd, Length() ) != afterEnd+1) {
        // insertion
        int oldLen = Length();
        myString.Insert( afterEnd, (char*) theAcsMethod );
        myString.Insert( afterEnd, "." );
        // update starting positions of the parts following the modified one
        int posDelta = Length() - oldLen;
        for ( int i = 1; i <= myBegPos.Length(); ++i ) {
          if ( myBegPos( i ) > afterEnd )
            myBegPos( i ) += posDelta;
        }
        added = true;
      }
    }
    beg = afterEnd; // is a part -> next search
  }
  return added;
}

//================================================================================
/*!
 * \brief Creates pyObject
 */
//================================================================================

_pyObject::_pyObject(const Handle(_pyCommand)& theCreationCmd, const _pyID& theID)
  : myID(theID), myCreationCmd(theCreationCmd), myIsPublished(false)
{
  setID( theID );
}

//================================================================================
/*!
 * \brief Set up myID and myIsPublished
 */
//================================================================================

void _pyObject::setID(const _pyID& theID)
{
  myID = theID;
  myIsPublished = !theGen->IsNotPublished( GetID() );
}

//================================================================================
/*!
 * \brief Clear myCreationCmd and myProcessedCmds
 */
//================================================================================

void _pyObject::ClearCommands()
{
  if ( !CanClear() )
    return;

  if ( !myCreationCmd.IsNull() )
    myCreationCmd->Clear();

  list< Handle(_pyCommand) >::iterator cmd = myProcessedCmds.begin();
  for ( ; cmd != myProcessedCmds.end(); ++cmd )
    (*cmd)->Clear();
}

//================================================================================
/*!
 * \brief Return method name giving access to an interaface object wrapped by python class
  * \retval const char* - method name
 */
//================================================================================

const char* _pyObject::AccessorMethod() const
{
  return 0;
}
//================================================================================
/*!
 * \brief Return ID of a father
 */
//================================================================================

_pyID _pyObject::FatherID(const _pyID & childID)
{
  int colPos = childID.SearchFromEnd(':');
  if ( colPos > 0 )
    return childID.SubString( 1, colPos-1 );
  return "";
}

//================================================================================
/*!
 * \brief SelfEraser erases creation command if none of it's commands invoked
 *        (e.g. filterManager) or it's not used as a command argument (e.g. a filter)
 */
//================================================================================

_pySelfEraser::_pySelfEraser(const Handle(_pyCommand)& theCreationCmd)
  :_pyObject(theCreationCmd), myIgnoreOwnCalls(false)
{
  myIsPublished = true; // prevent clearing as a not published
  theGen->KeepAgrCmds( GetID() ); // ask to fill myArgCmds
}

//================================================================================
/*!
 * \brief SelfEraser erases creation command if none of it's commands invoked
 *        (e.g. filterManager) or it's not used as a command argument (e.g. a filter)
 */
//================================================================================

bool _pySelfEraser::CanClear()
{
  bool toErase = false;
  if ( myIgnoreOwnCalls ) // check if this obj is used as argument
  {
    int nbArgUses = 0;
    list< Handle(_pyCommand) >::iterator cmd = myArgCmds.begin();
    for ( ; cmd != myArgCmds.end(); ++cmd )
      nbArgUses += IsAliveCmd( *cmd );

    toErase = ( nbArgUses < 1 );
  }
  else
  {
    int nbCalls = 0;
    std::list< Handle(_pyCommand) >& cmds = GetProcessedCmds();
    std::list< Handle(_pyCommand) >::iterator cmd = cmds.begin();
    for ( ; cmd != cmds.end();  )
      // check of cmd emptiness is not enough as object can change
      if (( *cmd )->GetString().Search( GetID() ) > 0 )
        ++nbCalls, ++cmd;
      else
        cmd = cmds.erase( cmd ); // save the cmd from clearing

    toErase = ( nbCalls < 1 );
  }
  return toErase;
}

//================================================================================
/*!
 * \brief Check if a command is or can be cleared
 */
//================================================================================

bool _pySelfEraser::IsAliveCmd( const Handle(_pyCommand)& theCmd )
{
  if ( theCmd->IsEmpty() )
    return false;

  if ( !theGen->IsToKeepAllCommands() )
  {
    const _pyID& objID = theCmd->GetObject();
    Handle( _pyObject ) obj = theGen->FindObject( objID );
    if ( !obj.IsNull() )
      return !obj->CanClear();
  }
  return true;
}

//================================================================================
/*!
 * \brief SelfEraser erases creation command if none of it's commands invoked
 *        (e.g. filterManager) or it's not used as a command argument (e.g. a filter)
 */
//================================================================================

void _pySelfEraser::Flush()
{
  if ( CanClear() )
  {
    myIsPublished = false;
    _pyObject::ClearCommands();
  }
}

//================================================================================
/*!
 * \brief _pySubMesh constructor
 */
//================================================================================

_pySubMesh::_pySubMesh(const Handle(_pyCommand)& theCreationCmd, bool toKeepAgrCmds):
  _pyObject(theCreationCmd)
{
  myMesh = ObjectToMesh( theGen->FindObject( theCreationCmd->GetObject() ));
  if ( toKeepAgrCmds )
    theGen->KeepAgrCmds( GetID() ); // ask to fill myArgCmds
}

//================================================================================
/*!
 * \brief Return true if a sub-mesh can be used as argument of the given method
 */
//================================================================================

bool _pySubMesh::CanBeArgOfMethod(const _AString& theMethodName)
{
  return false;
//   // names of all methods where a sub-mesh can be used as argument
//   static TStringSet methods;
//   if ( methods.empty() ) {
//     const char * names[] = {
//       // methods of SMESH_Gen
//       "CopyMesh",
//       // methods of SMESH_Group
//       "AddFrom",
//       // methods of SMESH_Measurements
//       "MinDistance",
//       // methods of SMESH_Mesh
//       "ExportPartToMED","ExportCGNS","ExportPartToDAT","ExportPartToUNV","ExportPartToSTL",
//       "RemoveSubMesh",
//       // methods of SMESH_MeshEditor
//       "ReorientObject","Reorient2D","TriToQuadObject","QuadToTriObject","SplitQuadObject",
//       "SplitVolumesIntoTetra","SmoothObject","SmoothParametricObject","ConvertFromQuadraticObject",
//       "RotationSweepObject","RotationSweepObjectMakeGroups","RotationSweepObject1D",
//       "RotationSweepObject1DMakeGroups","RotationSweepObject2D","RotationSweepObject2DMakeGroups",
//       "ExtrusionSweepObject","ExtrusionSweepObjectMakeGroups","ExtrusionSweepObject0D",
//       "ExtrusionSweepObject0DMakeGroups","ExtrusionSweepObject1D","ExtrusionSweepObject2D",
//       "ExtrusionSweepObject1DMakeGroups","ExtrusionSweepObject2DMakeGroups",
//       "ExtrusionAlongPathObjX","ExtrusionAlongPathObject","ExtrusionAlongPathObjectMakeGroups",
//       "ExtrusionAlongPathObject1D","ExtrusionAlongPathObject1DMakeGroups",
//       "ExtrusionAlongPathObject2D","ExtrusionAlongPathObject2DMakeGroups","MirrorObject",
//       "MirrorObjectMakeGroups","MirrorObjectMakeMesh","TranslateObject","Scale",
//       "TranslateObjectMakeGroups","TranslateObjectMakeMesh","ScaleMakeGroups","ScaleMakeMesh",
//       "RotateObject","RotateObjectMakeGroups","RotateObjectMakeMesh","FindCoincidentNodesOnPart",
//       "FindCoincidentNodesOnPartBut","FindEqualElements","FindAmongElementsByPoint",
//       "MakeBoundaryMesh","Create0DElementsOnAllNodes",
//       "" }; // <- mark of end
//     methods.Insert( names );
//   }
//   return methods.Contains( theMethodName );
}

//================================================================================
/*!
 * \brief count invoked commands
 */
//================================================================================

void _pySubMesh::Process( const Handle(_pyCommand)& theCommand )
{
  _pyObject::Process(theCommand); // count calls of Process()
}

//================================================================================
/*!
 * \brief Move creation command depending on invoked commands
 */
//================================================================================

void _pySubMesh::Flush()
{
  if ( GetNbCalls() == 0 && myArgCmds.empty() ) // move to the end of all commands
    theGen->GetLastCommand()->AddDependantCmd( GetCreationCmd() );
  else if ( !myCreator.IsNull() )
    // move to be just after creator
    myCreator->GetCreationCmd()->AddDependantCmd( GetCreationCmd() );

  // move sub-mesh usage after creation cmd
  list< Handle(_pyCommand) >::iterator cmd = myArgCmds.begin();
  for ( ; cmd != myArgCmds.end(); ++cmd )
    if ( !(*cmd)->IsEmpty() )
      GetCreationCmd()->AddDependantCmd( *cmd );
}

//================================================================================
/*!
 * \brief Creates _pyGroup
 */
//================================================================================

_pyGroup::_pyGroup(const Handle(_pyCommand)& theCreationCmd, const _pyID & id)
  :_pySubMesh(theCreationCmd, /*toKeepAgrCmds=*/false)
{
  if ( !id.IsEmpty() )
    setID( id );

  myCanClearCreationCmd = true;

  const _AString& method = theCreationCmd->GetMethod();
  if ( method == "CreateGroup" ) // CreateGroup() --> CreateEmptyGroup()
  {
    theCreationCmd->SetMethod( "CreateEmptyGroup" );
  }
  // ----------------------------------------------------------------------
  else if ( method == "CreateGroupFromGEOM" ) // (type, name, grp)
  {
    _pyID geom = theCreationCmd->GetArg( 3 );
    // VSR 24/12/2010. PAL21106: always use GroupOnGeom() function on dump
    // next if(){...} section is commented
    //if ( sameGroupType( geom, theCreationCmd->GetArg( 1 )) ) { // --> Group(geom)
    //  theCreationCmd->SetMethod( "Group" );
    //  theCreationCmd->RemoveArgs();
    //  theCreationCmd->SetArg( 1, geom );
    //}
    //else {
    // ------------------------->>>>> GroupOnGeom( geom, name, typ )
      _pyID type = theCreationCmd->GetArg( 1 );
      _pyID name = theCreationCmd->GetArg( 2 );
      theCreationCmd->SetMethod( "GroupOnGeom" );
      theCreationCmd->RemoveArgs();
      theCreationCmd->SetArg( 1, geom );
      theCreationCmd->SetArg( 2, name );
      theCreationCmd->SetArg( 3, type );
    //}
  }
  else if ( method == "CreateGroupFromFilter" )
  {
    // -> GroupOnFilter(typ, name, aFilter0x4743dc0 -> aFilter_1)
    theCreationCmd->SetMethod( "GroupOnFilter" );

    _pyID filterID = theCreationCmd->GetArg(3);
    Handle(_pyFilter) filter = Handle(_pyFilter)::DownCast( theGen->FindObject( filterID ));
    if ( !filter.IsNull())
    {
      if ( !filter->GetNewID().IsEmpty() )
        theCreationCmd->SetArg( 3, filter->GetNewID() );
      //filter->AddUser( this );
    }
    myFilter = filter;
  }
  else if ( method == "GetGroups" )
  {
    myCanClearCreationCmd = ( theCreationCmd->GetNbResultValues() == 1 );
  }
  else
  {
    // theCreationCmd does something else apart from creation of this group
    // and thus it can't be cleared if this group is removed
    myCanClearCreationCmd = false;
  }
}

//================================================================================
/*!
 * \brief Check if "[ group1, group2 ] = mesh.GetGroups()" creation command 
 *        can be cleared
 */
//================================================================================

bool _pyGroup::CanClear()
{
  if ( IsInStudy() )
    return false;

  if ( !myCanClearCreationCmd &&
       !myCreationCmd.IsNull() &&
       myCreationCmd->GetMethod() == "GetGroups" )
  {
    TCollection_AsciiString grIDs = myCreationCmd->GetResultValue();
    list< _pyID >          idList = myCreationCmd->GetStudyEntries( grIDs );
    list< _pyID >::iterator  grID = idList.begin();
    if ( GetID() == *grID )
    {
      myCanClearCreationCmd = true;
      list< Handle(_pyGroup ) > groups;
      for ( ; grID != idList.end(); ++grID )
      {
        Handle(_pyGroup) group = Handle(_pyGroup)::DownCast( theGen->FindObject( *grID ));
        if ( group.IsNull() ) continue;
        groups.push_back( group );
        if ( group->IsInStudy() )
          myCanClearCreationCmd = false;
      }
      // set myCanClearCreationCmd == true to all groups
      list< Handle(_pyGroup ) >::iterator group = groups.begin();
      for ( ; group != groups.end(); ++group )
        (*group)->myCanClearCreationCmd = myCanClearCreationCmd;
    }
  }

  return myCanClearCreationCmd;
}

//================================================================================
/*!
 * \brief set myCanClearCreationCmd = true if the main action of the creation
 *        command is discarded
 */
//================================================================================

void _pyGroup::RemovedWithContents()
{
  // this code would be appropriate if Add0DElementsToAllNodes() returned only new nodes
  // via a created group
  //if ( GetCreationCmd()->GetMethod() == "Add0DElementsToAllNodes")
  // myCanClearCreationCmd = true;
}

//================================================================================
/*!
 * \brief To convert creation of a group by filter
 */
//================================================================================

void _pyGroup::Process( const Handle(_pyCommand)& theCommand)
{
  // Convert the following set of commands into mesh.MakeGroupByFilter(groupName, theFilter)
  // group = mesh.CreateEmptyGroup( elemType, groupName )
  // aFilter.SetMesh(mesh)
  // nbAdd = group.AddFrom( aFilter )
  Handle(_pyFilter) filter;
  if ( theCommand->GetMethod() == "AddFrom" )
  {
    _pyID idSource = theCommand->GetArg(1);
    // check if idSource is a filter
    filter = Handle(_pyFilter)::DownCast( theGen->FindObject( idSource ));
    if ( !filter.IsNull() )
    {
      // find aFilter.SetMesh(mesh) to clear it, it should be just before theCommand
      list< Handle(_pyCommand) >::reverse_iterator cmdIt = theGen->GetCommands().rbegin();
      while ( *cmdIt != theCommand ) ++cmdIt;
      while ( (*cmdIt)->GetOrderNb() != 1 )
      {
        const Handle(_pyCommand)& setMeshCmd = *(++cmdIt);
        if ((setMeshCmd->GetObject() == idSource ||
             setMeshCmd->GetObject() == filter->GetNewID() )
            &&
            setMeshCmd->GetMethod() == "SetMesh")
        {
          setMeshCmd->Clear();
          break;
        }
      }
      // replace 3 commands by one
      theCommand->Clear();
      const Handle(_pyCommand)& makeGroupCmd = GetCreationCmd();
      TCollection_AsciiString name = makeGroupCmd->GetArg( 2 );
      if ( !filter->GetNewID().IsEmpty() )
        idSource = filter->GetNewID();
      makeGroupCmd->SetMethod( "MakeGroupByFilter" );
      makeGroupCmd->SetArg( 1, name );
      makeGroupCmd->SetArg( 2, idSource );
      filter->AddArgCmd( makeGroupCmd );
    }
  }
  else if ( theCommand->GetMethod() == "SetFilter" )
  {
    // set new name of a filter or clear the command if the same filter is set
    _pyID filterID = theCommand->GetArg(1);
    filter = Handle(_pyFilter)::DownCast( theGen->FindObject( filterID ));
    if ( !myFilter.IsNull() && filter == myFilter )
      theCommand->Clear();
    else if ( !filter.IsNull() && !filter->GetNewID().IsEmpty() )
      theCommand->SetArg( 1, filter->GetNewID() );
    myFilter = filter;
  }
  else if ( theCommand->GetMethod() == "GetFilter" )
  {
    // GetFilter() returns a filter with other ID, make myFilter process
    // calls of the returned filter
    if ( !myFilter.IsNull() )
    {
      theGen->SetProxyObject( theCommand->GetResultValue(), myFilter );
      theCommand->Clear();
    }
  }

  // if ( !filter.IsNull() )
  //   filter->AddUser( this );

  theGen->AddMeshAccessorMethod( theCommand );
}

//================================================================================
/*!
 * \brief Prevent clearing "DoubleNode...() command if a group created by it is removed
 */
//================================================================================

void _pyGroup::Flush()
{
  if ( !theGen->IsToKeepAllCommands() &&
       !myCreationCmd.IsNull() && !myCanClearCreationCmd )
  {
    myCreationCmd.Nullify(); // this way myCreationCmd won't be cleared
  }
}

//================================================================================
/*!
 * \brief Constructor of _pyFilter
 */
//================================================================================

_pyFilter::_pyFilter(const Handle(_pyCommand)& theCreationCmd, const _pyID& newID/*=""*/)
  :_pyObject(theCreationCmd), myNewID( newID )
{
  //myIsPublished = true; // prevent clearing as a not published
  theGen->KeepAgrCmds( GetID() ); // ask to fill myArgCmds
}

//================================================================================
/*!
 * \brief To convert creation of a filter by criteria and
 * to replace an old name by a new one
 */
//================================================================================

void _pyFilter::Process( const Handle(_pyCommand)& theCommand)
{
  if ( theCommand->GetObject() == GetID() )
    _pyObject::Process(theCommand); // count commands

  if ( !myNewID.IsEmpty() )
    theCommand->SetObject( myNewID );
    
  // Convert the following set of commands into smesh.GetFilterFromCriteria(criteria)
  // aFilter0x2aaab0487080 = aFilterManager.CreateFilter()
  // aFilter0x2aaab0487080.SetCriteria(aCriteria)
  if ( GetNbCalls() == 1 && // none method was called before this SetCriteria() call
       theCommand->GetMethod() == "SetCriteria")
  {
    // aFilter.SetCriteria(aCriteria) ->
    // aFilter = smesh.GetFilterFromCriteria(criteria)
    if ( myNewID.IsEmpty() )
      theCommand->SetResultValue( GetID() );
    else
      theCommand->SetResultValue( myNewID );
    theCommand->SetObject( SMESH_2smeshpy::GenName() );
    theCommand->SetMethod( "GetFilterFromCriteria" );

    // Swap "aFilterManager.CreateFilter()" and "smesh.GetFilterFromCriteria(criteria)"
    GetCreationCmd()->Clear();
    GetCreationCmd()->GetString() = theCommand->GetString();
    theCommand->Clear();
    theCommand->AddDependantCmd( GetCreationCmd() );
    // why swap? -- it's needed
    //GetCreationCmd()->Clear();
  }
  else if ( theCommand->GetMethod() == "SetMesh" )
  {
    if ( myMesh == theCommand->GetArg( 1 ))
      theCommand->Clear();
    else
      myMesh = theCommand->GetArg( 1 );
    theGen->AddMeshAccessorMethod( theCommand );
  }
}

//================================================================================
/*!
 * \brief Set new filter name to the creation command and to myArgCmds
 */
//================================================================================

void _pyFilter::Flush()
{
  if ( myNewID.IsEmpty() ) return;
  
  list< Handle(_pyCommand) >::iterator cmd = myArgCmds.begin();
  for ( ; cmd != myArgCmds.end(); ++cmd )
    if ( !(*cmd)->IsEmpty() )
    {
      _AString cmdStr = (*cmd)->GetString();
      _AString id     = GetID();
      int pos = cmdStr.Search( id );
      if ( pos > 0 )
      {
        cmdStr.Remove( pos, id.Length() );
        cmdStr.Insert( pos, myNewID );
      }
      (*cmd)->Clear();
      (*cmd)->GetString() = cmdStr;
    }

  if ( !GetCreationCmd()->IsEmpty() )
    GetCreationCmd()->SetResultValue( myNewID );
}

//================================================================================
/*!
 * \brief Return true if all my users can be cleared
 */
//================================================================================

bool _pyObject::CanClear()
{
  list< Handle(_pyCommand) >::iterator cmd = myArgCmds.begin();
  for ( ; cmd != myArgCmds.end(); ++cmd )
    if ( !(*cmd)->IsEmpty() )
    {
      Handle(_pyObject) obj = theGen->FindObject( (*cmd)->GetObject() );
      if ( !obj.IsNull() && !obj->CanClear() )
        return false;
    }
  return ( !myIsPublished );
}

//================================================================================
/*!
 * \brief Reads _pyHypothesis'es from resource files of mesher Plugins
 */
//================================================================================

_pyHypothesisReader::_pyHypothesisReader()
{
  // Read xml files
  vector< string > xmlPaths = SMESH_Gen::GetPluginXMLPaths();
  LDOMParser xmlParser;
  for ( size_t i = 0; i < xmlPaths.size(); ++i )
  {
    bool error = xmlParser.parse( xmlPaths[i].c_str() );
    if ( error )
    {
      _AString data;
      INFOS( xmlParser.GetError(data) );
      continue;
    }
    // <algorithm type="Regular_1D"
    //            label-id="Wire discretisation"
    //            ...>
    //   <python-wrap>
    //     <algo>Regular_1D=Segment()</algo>
    //     <hypo>LocalLength=LocalLength(SetLength(1),,SetPrecision(1))</hypo>
    //
    LDOM_Document xmlDoc = xmlParser.getDocument();
    LDOM_NodeList algoNodeList = xmlDoc.getElementsByTagName( "algorithm" );
    for ( int i = 0; i < algoNodeList.getLength(); ++i )
    {
      LDOM_Node     algoNode = algoNodeList.item( i );
      LDOM_Element& algoElem = (LDOM_Element&) algoNode;
      LDOM_NodeList pyAlgoNodeList = algoElem.getElementsByTagName( "algo" );
      if ( pyAlgoNodeList.getLength() < 1 ) continue;

      _AString text, algoType, method, arg;
      for ( int iA = 0; iA < pyAlgoNodeList.getLength(); ++iA )
      {
        LDOM_Node pyAlgoNode = pyAlgoNodeList.item( iA );
        LDOM_Node textNode   = pyAlgoNode.getFirstChild();
        text = textNode.getNodeValue();
        Handle(_pyCommand) algoCmd = new _pyCommand( text );
        algoType = algoCmd->GetResultValue();
        method   = algoCmd->GetMethod();
        arg      = algoCmd->GetArg(1);
        if ( !algoType.IsEmpty() && !method.IsEmpty() )
        {
          Handle(_pyAlgorithm) algo = new _pyAlgorithm( algoCmd );
          algo->SetConvMethodAndType( method, algoType );
          if ( !arg.IsEmpty() )
            algo->setCreationArg( 1, arg );

          myType2Hyp[ algoType ] = algo;
          break;
        }
      }
      if ( algoType.IsEmpty() ) continue;

      LDOM_NodeList pyHypoNodeList = algoElem.getElementsByTagName( "hypo" );
      _AString hypType;
      Handle( _pyHypothesis ) hyp;
      for ( int iH = 0; iH < pyHypoNodeList.getLength(); ++iH )
      {
        LDOM_Node pyHypoNode = pyHypoNodeList.item( iH );
        LDOM_Node textNode   = pyHypoNode.getFirstChild();
        text = textNode.getNodeValue();
        Handle(_pyCommand) hypoCmd = new _pyCommand( text );
        hypType = hypoCmd->GetResultValue();
        method  = hypoCmd->GetMethod();
        if ( !hypType.IsEmpty() && !method.IsEmpty() )
        {
          map<_AString, Handle(_pyHypothesis)>::iterator type2hyp = myType2Hyp.find( hypType );
          if ( type2hyp == myType2Hyp.end() )
            hyp = new _pyHypothesis( hypoCmd );
          else
            hyp = type2hyp->second;
          hyp->SetConvMethodAndType( method, algoType );
          for ( int iArg = 1; iArg <= hypoCmd->GetNbArgs(); ++iArg )
          {
            _pyCommand argCmd( hypoCmd->GetArg( iArg ));
            _AString argMethod = argCmd.GetMethod();
            _AString argNbText = argCmd.GetArg( 1 );
            if ( argMethod.IsEmpty() && !argCmd.IsEmpty() )
              hyp->setCreationArg( 1, argCmd.GetString() ); // e.g. Parameters(smesh.SIMPLE)
            else
              hyp->AddArgMethod( argMethod,
                                 argNbText.IsIntegerValue() ? argNbText.IntegerValue() : 1 );
          }
          myType2Hyp[ hypType ] = hyp;
        }
      }
    }
    // <hypothesis type="BLSURF_Parameters"
    //          ...
    //          dim="2">
    //   <python-wrap>
    //     <accumulative-methods> 
    //       SetEnforcedVertex,
    //       SetEnforcedVertexNamed
    //     </accumulative-methods>
    //   </python-wrap>
    // </hypothesis>
    //
    LDOM_NodeList hypNodeList = xmlDoc.getElementsByTagName( "hypothesis" );
    for ( int i = 0; i < hypNodeList.getLength(); ++i )
    {
      LDOM_Node     hypNode      = hypNodeList.item( i );
      LDOM_Element& hypElem      = (LDOM_Element&) hypNode;
      _AString      hypType      = hypElem.getAttribute("type");
      LDOM_NodeList methNodeList = hypElem.getElementsByTagName( "accumulative-methods" );
      if ( methNodeList.getLength() != 1 || hypType.IsEmpty() ) continue;

      map<_AString, Handle(_pyHypothesis)>::const_iterator type2hyp = myType2Hyp.find( hypType );
      if ( type2hyp == myType2Hyp.end() ) continue;

      LDOM_Node methNode = methNodeList.item( 0 );
      LDOM_Node textNode = methNode.getFirstChild();
      _AString      text = textNode.getNodeValue();
      _AString method;
      int pos = 1;
      do {
        method = _pyCommand::GetWord( text, pos, /*forward= */true );
        pos += method.Length();
        type2hyp->second->AddAccumulativeMethod( method );
      }
      while ( !method.IsEmpty() );
    }

  } // loop on xmlPaths
}

//================================================================================
/*!
 * \brief Returns a new hypothesis initialized according to the read information
 */
//================================================================================

Handle(_pyHypothesis)
_pyHypothesisReader::GetHypothesis(const _AString&           hypType,
                                   const Handle(_pyCommand)& creationCmd) const
{
  Handle(_pyHypothesis) resHyp, sampleHyp;

  map<_AString, Handle(_pyHypothesis)>::const_iterator type2hyp = myType2Hyp.find( hypType );
  if ( type2hyp != myType2Hyp.end() )
    sampleHyp = type2hyp->second;

  if ( sampleHyp.IsNull() )
  {
    resHyp = new _pyHypothesis(creationCmd);
  }
  else
  {
    if ( sampleHyp->IsAlgo() )
      resHyp = new _pyAlgorithm( creationCmd );
    else
      resHyp = new _pyHypothesis(creationCmd);
    resHyp->Assign( sampleHyp, _pyID() );
  }
  return resHyp;
}

//================================================================================
/*!
 * \brief Adds an object ID to some family of IDs with a common prefix
 *  \param [in] str - the object ID
 *  \return bool - \c false if \a str does not have the same prefix as \a this family
 *          (for internal usage)
 */
//================================================================================

bool _pyStringFamily::Add( const char* str )
{
  if ( strncmp( str, _prefix.ToCString(), _prefix.Length() ) != 0 )
    return false; // expected prefix is missing

  str += _prefix.Length(); // skip _prefix

  // try to add to some of child falimies
  std::list< _pyStringFamily >::iterator itSub = _subFams.begin();
  for ( ; itSub != _subFams.end(); ++itSub )
    if ( itSub->Add( str ))
      return true;

  // no suitable family found - add str to _strings or create a new child family

  // look for a proper place within sorted _strings
  std::list< _AString >::iterator itStr = _strings.begin();
  while ( itStr != _strings.end() && itStr->IsLess( str ))
    ++itStr;
  if ( itStr != _strings.end() && itStr->IsEqual( str ))
    return true; // same ID already kept

  const int minPrefixSize = 4;

  // count "smaller" strings with the same prefix
  std::list< _AString >::iterator itLess = itStr; --itLess;
  int nbLess = 0;
  for ( ; itLess != _strings.end(); --itLess )
    if ( strncmp( str, itLess->ToCString(), minPrefixSize ) == 0 )
      ++nbLess;
    else
      break;
  ++itLess;
  // count "greater" strings with the same prefix
  std::list< _AString >::iterator itMore = itStr;
  int nbMore = 0;
  for ( ; itMore != _strings.end(); ++itMore )
    if ( strncmp( str, itMore->ToCString(), minPrefixSize ) == 0 )
      ++nbMore;
    else
      break;
  --itMore;
  if ( nbLess + nbMore > 1 ) // ------- ADD a NEW CHILD FAMILY -------------
  {
    // look for a maximal prefix length
    // int lessPrefSize = 3, morePrefSize = 3;
    // if ( nbLess > 0 )
    //   while( itLess->ToCString()[ lessPrefSize ] == str[ lessPrefSize ]  )
    //     ++lessPrefSize;
    // if ( nbMore > 0 )
    //   while ( itMore->ToCString()[ morePrefSize ] == str[ morePrefSize ] )
    //     ++morePrefSize;
    // int prefixSize = 3;
    // if ( nbLess == 0 )
    //   prefixSize = morePrefSize;
    // else if ( nbMore == 0 )
    //   prefixSize = lessPrefSize;
    // else
    //   prefixSize = Min( lessPrefSize, morePrefSize );
    int prefixSize = minPrefixSize;
    _AString newPrefix ( str, prefixSize );

    // look for a proper place within _subFams sorted by _prefix
    for ( itSub = _subFams.begin(); itSub != _subFams.end(); ++itSub )
      if ( !itSub->_prefix.IsLess( newPrefix ))
        break;

    // add the new _pyStringFamily
    itSub = _subFams.insert( itSub, _pyStringFamily());
    _pyStringFamily& newSubFam = *itSub;
    newSubFam._prefix = newPrefix;

    // pass this->_strings to newSubFam._strings
    for ( itStr = itLess; nbLess > 0; --nbLess, ++itStr )
      newSubFam._strings.push_back( itStr->ToCString() + prefixSize );
    newSubFam._strings.push_back( str + prefixSize );
    for ( ; nbMore > 0; --nbMore, ++itStr )
      newSubFam._strings.push_back( itStr->ToCString() + prefixSize );

    _strings.erase( itLess, ++itMore );
  }
  else // to few string to make a family fot them
  {
    _strings.insert( itStr, str );
  }
  return true;
}

//================================================================================
/*!
 * \brief Finds an object ID in the command
 *  \param [in] longStr - the command string
 *  \param [out] subStr - the found object ID
 *  \return bool - \c true if the object ID found
 */
//================================================================================

bool _pyStringFamily::IsInArgs( Handle( _pyCommand)& cmd, std::list<_AString>& subStr )
{
  const _AString& longStr = cmd->GetString();
  const char*           s = longStr.ToCString();

  // look in _subFams
  std::list< _pyStringFamily >::iterator itSub = _subFams.begin();
  int nbFound = 0, pos, len, from, argBeg = cmd->GetArgBeginning();
  if ( argBeg < 4 || argBeg > longStr.Length() )
    return false;
  for ( ; itSub != _subFams.end(); ++itSub )
  {
    from = argBeg;
    while (( pos = longStr.Location( itSub->_prefix, from, longStr.Length() )))
      if (( len = itSub->isIn( s + pos-1 + itSub->_prefix.Length() )) >= 0 )
      {
        subStr.push_back( _AString( s + pos-1, len + itSub->_prefix.Length() ));
        from = pos + len + itSub->_prefix.Length();
        nbFound++;
      }
      else
      {
        from += itSub->_prefix.Length();
      }
  }
  // look among _strings
  std::list< _AString >::iterator itStr = _strings.begin();
  for ( ; itStr != _strings.end(); ++itStr )
    if (( pos = longStr.Location( *itStr, argBeg, longStr.Length() )))
      // check that object ID does not continue after len
      if ( !cmd->IsIDChar( s[ pos + itStr->Length() - 1 ] ))
      {
        subStr.push_back( *itStr );
        nbFound++;
      }
  return nbFound;
}

//================================================================================
/*!
 * \brief Return remainder length of the object ID after my _prefix
 *  \param [in] str - remainder of the command after my _prefix
 *  \return int - length of the object ID or -1 if not found
 */
//================================================================================

int _pyStringFamily::isIn( const char* str )
{
  std::list< _pyStringFamily >::iterator itSub = _subFams.begin();
  int len = -1;
  for ( ; itSub != _subFams.end(); ++itSub )
  {
    int cmp = strncmp( str, itSub->_prefix.ToCString(), itSub->_prefix.Length() );
    if ( cmp == 0 )
    {
      if (( len = itSub->isIn( str + itSub->_prefix.Length() )) >= 0 )
        return itSub->_prefix.Length() + len;
    }
    else if ( cmp > 0 )
      break;
  }
  if ( !_strings.empty() )
  {
    std::list< _AString >::iterator itStr = _strings.begin();
    bool firstEmpty = itStr->IsEmpty();
    if ( firstEmpty )
      ++itStr, len = 0;
    for ( ; itStr != _strings.end(); ++itStr )
    {
      int cmp = strncmp( str, itStr->ToCString(), itStr->Length() );
      if ( cmp == 0 )
      {
        len = itStr->Length();
        break;
      }
      else if ( cmp < 0 )
      {
        break;
      }
    }

    // check that object ID does not continue after len
    if ( len >= 0 && _pyCommand::IsIDChar( str[len] ))
      len = -1;
  }

  return len;
}

//================================================================================
/*!
 * \brief DEBUG
 */
//================================================================================

void _pyStringFamily::Print( int level )
{
  cout << string( level, ' ' ) << "prefix = '" << _prefix << "' : ";
  std::list< _AString >::iterator itStr = _strings.begin();
  for ( ; itStr != _strings.end(); ++itStr )
    cout << *itStr << " | ";
  cout << endl;
  std::list< _pyStringFamily >::iterator itSub = _subFams.begin();
  for ( ; itSub != _subFams.end(); ++itSub )
    itSub->Print( level + 1 );
  if ( level == 0 )
    cout << string( 70, '-' ) << endl;
}

