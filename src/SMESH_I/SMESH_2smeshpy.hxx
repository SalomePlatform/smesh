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

// File      : SMESH_smesh.hxx
// Created   : Fri Nov 18 12:05:18 2005
// Author    : Edward AGAPOV (eap)
//
#ifndef SMESH_smesh_HeaderFile
#define SMESH_smesh_HeaderFile

#include <Standard_DefineHandle.hxx>
#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TColStd_SequenceOfInteger.hxx>
#include <Resource_DataMapOfAsciiStringAsciiString.hxx>

#include <list>
#include <map>
#include <vector>
#include <set>

#include <Basics_OCCTVersion.hxx>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SALOMEDS)

#define USE_STRING_FAMILY

// ===========================================================================================
/*!
 * This file was created in order to respond to requirement of bug PAL10494:
 * SMESH python dump uses idl interface.
 *
 * The creation reason is that smeshBuilder.py commands defining hypotheses encapsulate
 * several SMESH engine method calls. As well, the dependencies between smeshBuilder.py
 * classes differ from ones between corresponding SMESH IDL interfaces.
 * 
 * Everything here is for internal usage by SMESH_2smeshpy::ConvertScript()
 * declared in SMESH_PythonDump.hxx
 */
// ===========================================================================================

// ===========================================================================================
// =====================
//    INTERNAL STUFF
// =====================
// ===========================================================================================

class _pyCommand;
class _pyObject;
class _pyGen;
class _pyMesh;
class _pySubMesh;
class _pyHypothesis;
class _pyAlgorithm;
class _pyHypothesisReader;

DEFINE_STANDARD_HANDLE (_pyCommand         ,Standard_Transient);
DEFINE_STANDARD_HANDLE (_pyObject          ,Standard_Transient);
DEFINE_STANDARD_HANDLE (_pyHypothesisReader,Standard_Transient);
DEFINE_STANDARD_HANDLE (_pyGen             ,_pyObject);
DEFINE_STANDARD_HANDLE (_pyMesh            ,_pyObject);
DEFINE_STANDARD_HANDLE (_pySubMesh         ,_pyObject);
DEFINE_STANDARD_HANDLE (_pyGroup           ,_pySubMesh);
DEFINE_STANDARD_HANDLE (_pyMeshEditor      ,_pyObject);
DEFINE_STANDARD_HANDLE (_pyHypothesis      ,_pyObject);
DEFINE_STANDARD_HANDLE (_pyAlgorithm       ,_pyHypothesis);

typedef TCollection_AsciiString _pyID;
typedef TCollection_AsciiString _AString;

// ===========================================================
/*!
 * \brief Class operating on a command string looking like
 *        ResultValue = Object.Method( Arg1, Arg2,...)
 */
// ===========================================================

class _pyCommand: public Standard_Transient
{
  int                             myOrderNb;            //!< position within the script
  _AString                        myString;             //!< command text
  _AString                        myRes, myObj, myMeth; //!< found parts of command
  TColStd_SequenceOfAsciiString   myArgs;               //!< found arguments
  TColStd_SequenceOfAsciiString   myResults;            //!< found results
  TColStd_SequenceOfInteger       myBegPos;             //!< where myRes, myObj, ... begin
  std::list< Handle(_pyCommand) > myDependentCmds; //!< commands that sould follow me in the script

  enum { UNKNOWN=-1, EMPTY=0, RESULT_IND, OBJECT_IND, METHOD_IND, ARG1_IND };
  int  GetBegPos( int thePartIndex ) const;
  void SetBegPos( int thePartIndex, int thePosition );
  void SetPart( int thePartIndex, const _AString& theNewPart, _AString& theOldPart);
  void FindAllArgs() { GetArg(1); }

public:
  _pyCommand() {};
  _pyCommand( const _AString& theString, int theNb=-1 )
    : myOrderNb( theNb ), myString( theString ) {};
  _AString & GetString() { return myString; }
  int  GetOrderNb() const { return myOrderNb; }
  void SetOrderNb( int theNb ) { myOrderNb = theNb; }
  typedef void* TAddr;
  TAddr GetAddress() const { return (void*) this; }
  int  Length() const { return myString.Length(); }
  void Clear() { myString.Clear(); myBegPos.Clear(); myArgs.Clear(); }
  bool IsEmpty() const { return myString.IsEmpty(); }
  _AString GetIndentation();
  const _AString & GetResultValue();
  int GetNbResultValues();
  const _AString & GetResultValue(int res);
  const _AString & GetObject();
  const _AString & GetMethod();
  const _AString & GetArg( int index );
  int GetNbArgs() { FindAllArgs(); return myArgs.Length(); }
  int GetArgBeginning() const;
  bool IsMethodCall();
  bool MethodStartsFrom(const _AString& beg)
  { GetMethod(); return ( myMeth.Location( beg, 1, myMeth.Length() ) == 1 ); }
  void SetResultValue( const _AString& theResult )
  { GetResultValue(); SetPart( RESULT_IND, theResult, myRes ); }
  void SetObject(const _AString& theObject)
  { GetObject(); SetPart( OBJECT_IND, theObject, myObj ); }
  void SetMethod(const _AString& theMethod)
  { GetMethod(); SetPart( METHOD_IND, theMethod, myMeth ); }
  void SetArg( int index, const _AString& theArg);
  void RemoveArgs();
  void Comment();
  static bool SkipSpaces( const _AString & theSring, int & thePos );
  static _AString GetWord( const _AString & theSring, int & theStartPos,
                           const bool theForward, const bool dotIsWord = false);
  static bool IsStudyEntry( const _AString& str );
  static bool IsID( const _AString& str );
  static bool IsIDChar( char c )
  { return ( isalnum( c ) || c == '_' || c == ':' ); }
  static std::list< _pyID > GetStudyEntries( const _AString& str );
  void AddDependantCmd( Handle(_pyCommand) cmd, bool prepend = false)
  { if (prepend) myDependentCmds.push_front( cmd ); else myDependentCmds.push_back( cmd ); }
  bool SetDependentCmdsAfter() const;

  bool AddAccessorMethod( _pyID theObjectID, const char* theAcsMethod );

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyCommand,Standard_Transient)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Root of all objects. It counts calls of Process()
 */
// -------------------------------------------------------------------------------------

class _pyObject: public Standard_Transient
{
protected:
  _pyID                           myID;
  Handle(_pyCommand)              myCreationCmd;
  std::list< Handle(_pyCommand) > myProcessedCmds;
  std::list< Handle(_pyCommand) > myArgCmds; // where this obj is used as an argument
  bool                            myIsPublished;

  void setID(const _pyID& theID);
public:
  _pyObject(const Handle(_pyCommand)& theCreationCmd, const _pyID& theID=_pyID());
  const _pyID& GetID() { return myID.IsEmpty() ? myCreationCmd->GetResultValue() : myID; }
  static _pyID FatherID(const _pyID & childID);
  const Handle(_pyCommand)& GetCreationCmd() { return myCreationCmd; }
  int  GetNbCalls() const { return myProcessedCmds.size(); }
  bool IsInStudy() const { return myIsPublished; }
  virtual void SetRemovedFromStudy(const bool isRemoved) { myIsPublished = !isRemoved; }
  void SetCreationCmd( Handle(_pyCommand) cmd ) { myCreationCmd = cmd; }
  int  GetCommandNb() { return myCreationCmd->GetOrderNb(); }
  void AddProcessedCmd( const Handle(_pyCommand) & cmd )
  { if (myProcessedCmds.empty() || myProcessedCmds.back()!=cmd) myProcessedCmds.push_back( cmd );}
  std::list< Handle(_pyCommand) >& GetProcessedCmds() { return myProcessedCmds; }
  void AddArgCmd( const Handle(_pyCommand) & cmd ) { myArgCmds.push_back( cmd ); }
  virtual void Process(const Handle(_pyCommand) & cmd) { AddProcessedCmd(cmd); }
  virtual void Flush() = 0;
  virtual const char* AccessorMethod() const;
  virtual bool CanClear();
  virtual void ClearCommands();
  virtual void Free() {}

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyObject,Standard_Transient)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Data used to restore cleared Compute() command of an exported mesh
 * when an imported mesh is created
 */
// -------------------------------------------------------------------------------------
struct ExportedMeshData
{
  Handle(_pyMesh)    myMesh;
  Handle(_pyCommand) myLastComputeCmd;
  _AString           myLastComputeCmdString;
  ExportedMeshData() {}
  ExportedMeshData( const Handle(_pyMesh)& mesh, Handle(_pyCommand) computeCmd):
    myMesh( mesh ), myLastComputeCmd( computeCmd )
  {
    if ( !myLastComputeCmd.IsNull())
      myLastComputeCmdString = myLastComputeCmd->GetString();
  }
};

// -------------------------------------------------------------------------------------
/*!
 * \brief A container of strings groupped by prefix. It is used for a faster search of
 *        objects requiring to KeepAgrCmds() in commands. A speed up is gained because
 *        only a common prefix (e.g. "aArea") of many object IDs is searched in a command
 *        and not every object ID
 */
// -------------------------------------------------------------------------------------
class _pyStringFamily
{
  _AString                     _prefix;
  std::list< _pyStringFamily > _subFams;
  std::list< _AString >        _strings;
  int isIn( const char* str );
public:
  bool Add( const char* str );
  bool IsInArgs( Handle( _pyCommand)& cmd, std::list<_AString>& subStr );
  void Print( int level = 0 );
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Class corresponding to SMESH_Gen. It holds info on existing
 *        meshes and hypotheses
 */
// -------------------------------------------------------------------------------------
class _pyGen: public _pyObject
{
public:
  _pyGen(Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod,
         Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
         std::set< TCollection_AsciiString >&      theRemovedObjIDs,
         SALOMEDS::Study_ptr&                      theStudy,
         const bool                                theToKeepAllCommands);
  Handle(_pyCommand) AddCommand( const _AString& theCommand );
  void ExchangeCommands( Handle(_pyCommand) theCmd1, Handle(_pyCommand) theCmd2 );
  void SetCommandAfter ( Handle(_pyCommand) theCmd,  Handle(_pyCommand) theAfterCmd );
  void SetCommandBefore( Handle(_pyCommand) theCmd,  Handle(_pyCommand) theBeforeCmd );
  Handle(_pyCommand)& GetLastCommand();
  std::list< Handle(_pyCommand) >& GetCommands() { return myCommands; }
  void PlaceSubmeshAfterItsCreation( Handle(_pyCommand) theCmdUsingSubmesh ) const;

  _pyID GenerateNewID( const _pyID& theID );
  bool AddObject( Handle(_pyObject)& theObj );
  void CheckObjectIsReCreated( Handle(_pyObject)& theObj );
  void SetProxyObject( const _pyID& theID, Handle(_pyObject)& theObj );
  Handle(_pyObject)     FindObject( const _pyID& theObjID ) const;
  Handle(_pySubMesh)    FindSubMesh( const _pyID& theSubMeshID );
  Handle(_pyHypothesis) FindHyp( const _pyID& theHypID );
  Handle(_pyHypothesis) FindAlgo( const _pyID& theGeom, const _pyID& theMesh,
                                  const Handle(_pyHypothesis)& theHypothesis);

  void SetAccessorMethod(const _pyID& theID, const char* theMethod );
  bool AddMeshAccessorMethod( Handle(_pyCommand) theCmd ) const;
  bool AddAlgoAccessorMethod( Handle(_pyCommand) theCmd ) const;
  virtual const char* AccessorMethod() const;

  bool IsGeomObject(const _pyID& theObjID) const;
  bool IsNotPublished(const _pyID& theObjID) const;
  void ObjectCreationRemoved(const _pyID& theObjID);
#ifdef USE_STRING_FAMILY
  void KeepAgrCmds(const _pyID& theObjID) { myKeepAgrCmdsIDs.Add( theObjID.ToCString() ); }
#else
  void KeepAgrCmds(const _pyID& theObjID) { myKeepAgrCmdsIDs.push_back( theObjID ); }
#endif
  bool IsToKeepAllCommands() const { return myToKeepAllCommands; }
  void AddExportedMesh(const _AString& file, const ExportedMeshData& mesh )
  { myFile2ExportedMesh[ file ] = mesh; }
  ExportedMeshData& FindExportedMesh( const _AString& file )
  { return myFile2ExportedMesh[ file ]; }

  virtual void Process( const Handle(_pyCommand)& theCommand );
  virtual void Flush();
  virtual void ClearCommands();
  virtual void Free();

  Handle( _pyHypothesisReader ) GetHypothesisReader() const;

private:
  void setNeighbourCommand( Handle(_pyCommand)& theCmd,
                            Handle(_pyCommand)& theOtherCmd,
                            const bool theIsAfter );
  //void addFilterUser( Handle(_pyCommand)& theCmd, const Handle(_pyObject)& user );

private:
  std::map< _pyID, Handle(_pyMesh) >        myMeshes;
  std::map< _pyID, Handle(_pyMeshEditor) >  myMeshEditors;
  std::map< _pyID, Handle(_pyHypothesis) >  myHypos;
  std::map< _pyID, Handle(_pyObject) >      myObjects;
  std::list< Handle(_pyObject) >            myOrderedObjects; // to know order of myObjects creation
#ifdef USE_STRING_FAMILY
  _pyStringFamily                           myKeepAgrCmdsIDs;
#else
  std::list< _pyID >                        myKeepAgrCmdsIDs;
#endif
  std::list< Handle(_pyCommand) >           myCommands;
  int                                       myNbCommands;
  Resource_DataMapOfAsciiStringAsciiString& myID2AccessorMethod;
  Resource_DataMapOfAsciiStringAsciiString& myObjectNames;
  std::set< TCollection_AsciiString >&      myRemovedObjIDs;
  Handle(_pyCommand)                        myLastCommand;
  int                                       myNbFilters;
  bool                                      myToKeepAllCommands;
  SALOMEDS::Study_var                       myStudy;
  int                                       myGeomIDNb, myGeomIDIndex;
  std::map< _AString, ExportedMeshData >    myFile2ExportedMesh;
  Handle( _pyHypothesisReader )             myHypReader;

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyGen,_pyObject)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Contains commands concerning mesh substructures
 */
// -------------------------------------------------------------------------------------
#define _pyMesh_ACCESS_METHOD "GetMesh()"
class _pyMesh: public _pyObject
{
  std::list< Handle(_pyHypothesis) > myHypos;
  std::list< Handle(_pyCommand) >    myAddHypCmds, myNotConvertedAddHypCmds;
  std::list< Handle(_pySubMesh) >    mySubmeshes;
  std::list< Handle(_pyGroup) >      myGroups;
  std::list< Handle(_pyMeshEditor)>  myEditors;
  //d::list< Handle(_pyMesh) >       myFatherMeshes; // this mesh depends on
  std::list< Handle(_pyMesh) >       myChildMeshes; // depending on me
  bool                               myGeomNotInStudy;
  Handle(_pyCommand)                 myLastComputeCmd;
public:
  _pyMesh(const Handle(_pyCommand) creationCmd);
  _pyMesh(const Handle(_pyCommand) theCreationCmd, const _pyID & id);
  const _pyID& GetGeom() { return GetCreationCmd()->GetArg(1); }
  void AddGroup( const Handle(_pyGroup)& g ) { myGroups.push_back( g ); }
  void AddEditor( const Handle(_pyMeshEditor)& e ) { myEditors.push_back( e ); }
  bool IsNotGeomPublished() { return myGeomNotInStudy; }
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual void SetRemovedFromStudy(const bool isRemoved);
  virtual bool CanClear();
  virtual void ClearCommands();
  virtual void Free() { /*myFatherMeshes.clear();*/ myChildMeshes.clear(); }
  virtual const char* AccessorMethod() const { return _pyMesh_ACCESS_METHOD; }
private:
  void addFatherMesh( const Handle(_pyMesh)& mesh );
  void addFatherMesh( const _pyID& meshID );
  static bool NeedMeshAccess( const Handle(_pyCommand)& theCommand );
  static void AddMeshAccess( const Handle(_pyCommand)& theCommand )
  { theCommand->SetObject( theCommand->GetObject() + "." _pyMesh_ACCESS_METHOD ); }

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyMesh,_pyObject)
};
#undef _pyMesh_ACCESS_METHOD 

// -------------------------------------------------------------------------------------
/*!
 * \brief MeshEditor convert its commands to ones of mesh
 */
// -------------------------------------------------------------------------------------
class _pyMeshEditor: public _pyObject
{
  _pyID    myMesh;
  _AString myCreationCmdStr;
public:
  _pyMeshEditor(const Handle(_pyCommand)& theCreationCmd);
  _pyID GetMesh() const { return myMesh; }
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush() {}
  virtual bool CanClear();

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyMesh,_pyObject)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Root class for hypothesis
 */
// -------------------------------------------------------------------------------------
class _pyHypothesis: public _pyObject
{
  friend class _pyHypothesisReader;
protected:
  bool    myIsAlgo, myIsWrapped;
  _pyID   myGeom,   myMesh;
  struct CreationMethod {
    _AString              myMethod; // method of algo or mesh creating a hyp
    // myArgNb(i)-th arg of myArgMethods(i) of hyp becomes an i-th arg of myMethod
    std::vector<_AString> myArgMethods;
    std::vector<int>      myArgNb; // arg nb countered from 1
    std::vector<_AString> myArgs; // creation arguments
  };
  void setCreationArg( const int argNb, const _AString& arg );
  // a hypothesis can be created by different algos by different methods
  typedef std::map<_AString, CreationMethod > TType2CrMethod;
  TType2CrMethod                myAlgoType2CreationMethod;
  std::set< _AString >          myAccumulativeMethods;
  CreationMethod*               myCurCrMethod; // used for adding to myAlgoType2CreationMethod
  std::list<Handle(_pyCommand)> myArgCommands;
  std::list<Handle(_pyCommand)> myUnusedCommands;
  std::list<Handle(_pyObject) > myReferredObjs;
  // maps used to clear commands setting parameters if result of setting is
  // discared (e.g. by mesh.Clear())
  std::map<_AString, std::list<Handle(_pyCommand)> >            myMeth2Commands;
  std::map< _pyCommand::TAddr, std::list<Handle(_pyCommand) > > myComputeAddr2Cmds;
  std::list<Handle(_pyCommand) >                                myComputeCmds;
  void rememberCmdOfParameter( const Handle(_pyCommand) & cmd );
  bool isCmdUsedForCompute( const Handle(_pyCommand) & cmd,
                            _pyCommand::TAddr avoidComputeAddr=NULL ) const;
public:
  _pyHypothesis(const Handle(_pyCommand)& theCreationCmd);
  void SetConvMethodAndType(const _AString& creationMethod, const _AString& type)
  { myCurCrMethod = &myAlgoType2CreationMethod[ type ];
    myCurCrMethod->myMethod = creationMethod; }
  void AddArgMethod(const _AString& method, const int argNb = 1)
  { myCurCrMethod->myArgMethods.push_back( method );
    myCurCrMethod->myArgNb.push_back( argNb ); }
  void AddAccumulativeMethod( const _AString& method)
  { myAccumulativeMethods.insert( method ); }
  //const TColStd_SequenceOfAsciiString& GetArgs() const { return myArgs; }
  const std::list<Handle(_pyCommand)>& GetArgCommands() const { return myArgCommands; }
  void ClearAllCommands();
  virtual bool IsAlgo() const { return myIsAlgo; }
  bool IsValid() const { return !myAlgoType2CreationMethod.empty(); }
  bool IsWrapped() const { return myIsWrapped; }
  const _pyID & GetGeom() const { return myGeom; }
  void SetMesh( const _pyID& theMeshId) { if ( myMesh.IsEmpty() ) myMesh = theMeshId; }
  const _pyID & GetMesh() const { return myMesh; }
  const _AString& GetAlgoType() const
  { return myAlgoType2CreationMethod.begin()->first; }
  const _AString& GetAlgoCreationMethod() const
  { return myAlgoType2CreationMethod.begin()->second.myMethod; }
  bool CanBeCreatedBy(const _AString& algoType ) const
  { return myAlgoType2CreationMethod.find( algoType ) != myAlgoType2CreationMethod.end(); }
  const _AString& GetCreationMethod(const _AString& algoType)
  { return ( myCurCrMethod = & myAlgoType2CreationMethod[ algoType ])->myMethod; }
  static Handle(_pyHypothesis) NewHypothesis( const Handle(_pyCommand)& theCreationCmd);

  virtual bool IsWrappable(const _pyID& theMesh) const;
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual void Free() { myReferredObjs.clear(); }
  virtual void Assign( const Handle(_pyHypothesis)& theOther,
                       const _pyID&                 theMesh );
  virtual bool CanClear();
  virtual void ClearCommands();
  virtual bool GetReferredMeshesAndGeom( std::list< Handle(_pyMesh) >& meshes );

  void MeshComputed    ( const Handle(_pyCommand)& theComputeCommand );
  void ComputeDiscarded( const Handle(_pyCommand)& theComputeCommand );
  //void ComputeSaved    ( const Handle(_pyCommand)& theComputeCommand );


  OCCT_DEFINE_STANDARD_RTTIEXT(_pyHypothesis,_pyObject)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Class representing smesh.Mesh_Algorithm
 */
// -------------------------------------------------------------------------------------
class _pyAlgorithm: public _pyHypothesis
{
public:
  _pyAlgorithm(const Handle(_pyCommand)& theCreationCmd);
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  virtual const char* AccessorMethod() const { return "GetAlgorithm()"; }
  virtual bool IsWrappable(const _pyID& theMesh) { return !myIsWrapped; }

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyAlgorithm,_pyHypothesis)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Class for hypotheses having several parameters modified by one method
 */
// -------------------------------------------------------------------------------------
class _pyComplexParamHypo: public _pyHypothesis
{
public:
  _pyComplexParamHypo(const Handle(_pyCommand)& theCreationCmd): _pyHypothesis(theCreationCmd) {}
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyComplexParamHypo,_pyHypothesis)
};
DEFINE_STANDARD_HANDLE (_pyComplexParamHypo, _pyHypothesis);

// -------------------------------------------------------------------------------------
/*!
 * \brief Class for LayerDistribution hypothesis conversion
 */
// -------------------------------------------------------------------------------------
class _pyLayerDistributionHypo: public _pyHypothesis
{
  Handle(_pyHypothesis) my1dHyp;
  _AString              myAlgoMethod;
public:
  _pyLayerDistributionHypo(const Handle(_pyCommand)& theCreationCmd, const char* algoMethod):
    _pyHypothesis(theCreationCmd), myAlgoMethod((char*)algoMethod) {}
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  virtual void Free() { my1dHyp.Nullify(); }

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyLayerDistributionHypo,_pyHypothesis)
};
DEFINE_STANDARD_HANDLE (_pyLayerDistributionHypo, _pyHypothesis);

// -------------------------------------------------------------------------------------
/*!
 * \brief Class representing NumberOfSegments hypothesis
 */
// -------------------------------------------------------------------------------------
class _pyNumberOfSegmentsHyp: public _pyHypothesis
{
public:
  _pyNumberOfSegmentsHyp(const Handle(_pyCommand)& theCrCmd): _pyHypothesis(theCrCmd) {}
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  void Flush();

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyNumberOfSegmentsHyp,_pyHypothesis)
};
DEFINE_STANDARD_HANDLE (_pyNumberOfSegmentsHyp, _pyHypothesis);

// -------------------------------------------------------------------------------------
/*!
 * \brief Class representing SegmentLengthAroundVertex hypothesis
 */
// -------------------------------------------------------------------------------------
class _pySegmentLengthAroundVertexHyp: public _pyHypothesis
{
public:
  _pySegmentLengthAroundVertexHyp(const Handle(_pyCommand)& theCrCmd): _pyHypothesis(theCrCmd) {}
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  OCCT_DEFINE_STANDARD_RTTIEXT(_pySegmentLengthAroundVertexHyp,_pyHypothesis)
};
DEFINE_STANDARD_HANDLE (_pySegmentLengthAroundVertexHyp, _pyHypothesis);

// -------------------------------------------------------------------------------------
/*!
 * \brief SelfEraser erases creation command if no more it's commands invoked
 */
// -------------------------------------------------------------------------------------
class _pySelfEraser: public _pyObject
{
  bool myIgnoreOwnCalls; // not to erase only if this obj is used as argument
public:
  _pySelfEraser(const Handle(_pyCommand)& theCreationCmd);
  void IgnoreOwnCalls() { myIgnoreOwnCalls = true; }
  virtual void Flush();
  virtual bool CanClear();
  static bool IsAliveCmd( const Handle(_pyCommand)& theCmd );

  OCCT_DEFINE_STANDARD_RTTIEXT(_pySelfEraser,_pyObject)
};
DEFINE_STANDARD_HANDLE (_pySelfEraser, _pyObject);

// -------------------------------------------------------------------------------------
/*!
 * \brief SubMesh creation can be moved to the end of engine commands
 */
// -------------------------------------------------------------------------------------
class _pySubMesh:  public _pyObject
{
  Handle(_pyObject) myCreator;
  Handle(_pyMesh) myMesh;
public:
  _pySubMesh(const Handle(_pyCommand)& theCreationCmd, bool toKeepAgrCmds=true);
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual Handle(_pyMesh) GetMesh() { return myMesh; }
  virtual void Free() { myCreator.Nullify(); myMesh.Nullify(); }
  void SetCreator( const Handle(_pyObject)& theCreator ) { myCreator = theCreator; }
  static bool CanBeArgOfMethod(const _AString& theMethodName);

  OCCT_DEFINE_STANDARD_RTTIEXT(_pySubMesh,_pyObject)
};
// -------------------------------------------------------------------------------------
/*!
 * \brief A filter sets a human readable name to self
 */
// -------------------------------------------------------------------------------------
class _pyFilter:  public _pyObject
{
  _pyID myNewID, myMesh;
  //std::list< Handle(_pyObject) > myUsers;
public:
  _pyFilter(const Handle(_pyCommand)& theCreationCmd, const _pyID& newID="");
  //void AddUser( const Handle(_pyObject)& user) { myUsers.push_back( user ); }
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  //virtual bool CanClear();
  //virtual void Free() { myUsers.clear(); }
  const _pyID& GetNewID() const { return myNewID; }

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyFilter,_pyObject)
};
DEFINE_STANDARD_HANDLE (_pyFilter, _pyObject);

// -------------------------------------------------------------------------------------
/*!
 * \brief To convert creation of a group by filter
 */
// -------------------------------------------------------------------------------------
class _pyGroup:  public _pySubMesh // use myMesh of _pySubMesh
{
  Handle(_pyFilter) myFilter;
  bool              myCanClearCreationCmd;
public:
  _pyGroup(const Handle(_pyCommand)& theCreationCmd, const _pyID & id=_pyID());
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual void Free() { myFilter.Nullify(); }
  virtual bool CanClear();
  void RemovedWithContents();

  OCCT_DEFINE_STANDARD_RTTIEXT(_pyGroup,_pySubMesh)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Class reading _pyHypothesis'es from resource files of mesher Plugins
 */
// -------------------------------------------------------------------------------------
class _pyHypothesisReader: public Standard_Transient
{
  std::map<_AString, Handle(_pyHypothesis)> myType2Hyp;
public:
  _pyHypothesisReader();
  Handle(_pyHypothesis) GetHypothesis(const _AString&           hypType,
                                      const Handle(_pyCommand)& creationCmd) const;
  OCCT_DEFINE_STANDARD_RTTIEXT(_pyHypothesisReader,Standard_Transient)
};

#endif
