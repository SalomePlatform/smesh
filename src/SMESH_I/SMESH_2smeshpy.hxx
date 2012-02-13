// Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include <list>
#include <map>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SALOMEDS)

// ===========================================================================================
/*!
 * This file was created in order to respond to requirement of bug PAL10494:
 * SMESH python dump uses idl interface.
 *
 * The creation reason is that smesh.py commands defining hypotheses encapsulate
 * several SMESH engine method calls. As well, the dependencies between smesh.py
 * classes differ from ones between corresponding SMESH IDL interfaces.
 * 
 * Everything here is for internal usage by SMESH_2smeshpy::ConvertScript()
 * declared in SMESH_PythonDump.hxx
 *
 * See comments to _pyHypothesis class to know how to assure convertion of a new
 * type of hypothesis
 */
// ===========================================================================================

class Resource_DataMapOfAsciiStringAsciiString;

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

DEFINE_STANDARD_HANDLE (_pyCommand   ,Standard_Transient);
DEFINE_STANDARD_HANDLE (_pyObject    ,Standard_Transient);
DEFINE_STANDARD_HANDLE (_pyGen       ,_pyObject);
DEFINE_STANDARD_HANDLE (_pyMesh      ,_pyObject);
DEFINE_STANDARD_HANDLE (_pySubMesh   ,_pyObject);
DEFINE_STANDARD_HANDLE (_pyGroup     ,_pySubMesh);
DEFINE_STANDARD_HANDLE (_pyMeshEditor,_pyObject);
DEFINE_STANDARD_HANDLE (_pyHypothesis,_pyObject);
DEFINE_STANDARD_HANDLE (_pyAlgorithm ,_pyHypothesis);

typedef TCollection_AsciiString _pyID;

// ===========================================================
/*!
 * \brief Class operating on a command string looking like
 *        ResultValue = Object.Method( Arg1, Arg2,...)
 */
// ===========================================================

class _pyCommand: public Standard_Transient
{
  int                             myOrderNb;            //!< position within the script
  TCollection_AsciiString         myString;             //!< command text
  TCollection_AsciiString         myRes, myObj, myMeth; //!< found parts of command
  TColStd_SequenceOfAsciiString   myArgs;               //!< found arguments
  TColStd_SequenceOfInteger       myBegPos;             //!< where myRes, myObj, ... begin
  std::list< Handle(_pyCommand) > myDependentCmds; //!< commands that sould follow me in the script

  enum { UNKNOWN=-1, EMPTY=0, RESULT_IND, OBJECT_IND, METHOD_IND, ARG1_IND };
  int GetBegPos( int thePartIndex );
  void SetBegPos( int thePartIndex, int thePosition );
  void SetPart( int thePartIndex, const TCollection_AsciiString& theNewPart,
                TCollection_AsciiString& theOldPart);
  void FindAllArgs() { GetArg(1); }

public:
  _pyCommand() {};
  _pyCommand( const TCollection_AsciiString& theString, int theNb )
    : myString( theString ), myOrderNb( theNb ) {};
  TCollection_AsciiString & GetString() { return myString; }
  int GetOrderNb() const { return myOrderNb; }
  void SetOrderNb( int theNb ) { myOrderNb = theNb; }
  typedef void* TAddr;
  TAddr GetAddress() const { return (void*) this; }
  int Length() { return myString.Length(); }
  void Clear() { myString.Clear(); myBegPos.Clear(); myArgs.Clear(); }
  bool IsEmpty() const { return myString.IsEmpty(); }
  TCollection_AsciiString GetIndentation();
  const TCollection_AsciiString & GetResultValue();
  const int GetNbResultValues();
  const TCollection_AsciiString & GetResultValue(int res);
  const TCollection_AsciiString & GetObject();
  const TCollection_AsciiString & GetMethod();
  const TCollection_AsciiString & GetArg( int index );
  int GetNbArgs() { FindAllArgs(); return myArgs.Length(); }
  bool MethodStartsFrom(const TCollection_AsciiString& beg)
  { GetMethod(); return ( myMeth.Location( beg, 1, myMeth.Length() ) == 1 ); }
  //Handle(TColStd_HSequenceOfAsciiString) GetArgs();
  void SetResultValue( const TCollection_AsciiString& theResult )
  { GetResultValue(); SetPart( RESULT_IND, theResult, myRes ); }
  void SetObject(const TCollection_AsciiString& theObject)
  { GetObject(); SetPart( OBJECT_IND, theObject, myObj ); }
  void SetMethod(const TCollection_AsciiString& theMethod)
  { GetMethod(); SetPart( METHOD_IND, theMethod, myMeth ); }
  void SetArg( int index, const TCollection_AsciiString& theArg);
  void RemoveArgs();
  void Comment();
  static bool SkipSpaces( const TCollection_AsciiString & theSring, int & thePos );
  static TCollection_AsciiString GetWord( const TCollection_AsciiString & theSring,
                                          int & theStartPos, const bool theForward,
                                          const bool dotIsWord = false);
  static bool IsStudyEntry( const TCollection_AsciiString& str );
  static std::list< _pyID > GetStudyEntries( const TCollection_AsciiString& str );
  void AddDependantCmd( Handle(_pyCommand) cmd, bool prepend = false)
  { if (prepend) myDependentCmds.push_front( cmd ); else myDependentCmds.push_back( cmd ); }
  bool SetDependentCmdsAfter() const;

  bool AddAccessorMethod( _pyID theObjectID, const char* theAcsMethod );

  DEFINE_STANDARD_RTTI (_pyCommand)
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
  bool                            myIsPublished;

  void setID(const _pyID& theID);
public:
  _pyObject(const Handle(_pyCommand)& theCreationCmd, const _pyID& theID=_pyID());
  const _pyID& GetID() { return myID.IsEmpty() ? myCreationCmd->GetResultValue() : myID; }
  static _pyID FatherID(const _pyID & childID);
  const Handle(_pyCommand)& GetCreationCmd() { return myCreationCmd; }
  int GetNbCalls() const { return myProcessedCmds.size(); }
  bool IsInStudy() const { return myIsPublished; }
  virtual void SetRemovedFromStudy(const bool isRemoved) { myIsPublished = !isRemoved; }
  void SetCreationCmd( Handle(_pyCommand) cmd ) { myCreationCmd = cmd; }
  int GetCommandNb() { return myCreationCmd->GetOrderNb(); }
  void AddProcessedCmd( const Handle(_pyCommand) & cmd )
  { if (myProcessedCmds.empty() || myProcessedCmds.back()!=cmd) myProcessedCmds.push_back( cmd );}
  std::list< Handle(_pyCommand) >& GetProcessedCmds() { return myProcessedCmds; }
  virtual void Process(const Handle(_pyCommand) & cmd) { AddProcessedCmd(cmd); }
  virtual void Flush() = 0;
  virtual const char* AccessorMethod() const;
  virtual bool CanClear() { return !myIsPublished; }
  virtual void ClearCommands();
  virtual void Free() {}

  DEFINE_STANDARD_RTTI (_pyObject)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Data used to restore cleared Compute() command of an exported mesh
 * when an imported mesh is created
 */
// -------------------------------------------------------------------------------------
struct ExportedMeshData
{
  Handle(_pyMesh)         myMesh;
  Handle(_pyCommand)      myLastComputeCmd;
  TCollection_AsciiString myLastComputeCmdString;
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
 * \brief Class corresponding to SMESH_Gen. It holds info on existing
 *        meshes and hypotheses
 */
// -------------------------------------------------------------------------------------
class _pyGen: public _pyObject
{
public:
  _pyGen(Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod,
         Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
         SALOMEDS::Study_ptr&                      theStudy,
         const bool                                theToKeepAllCommands);
  Handle(_pyCommand) AddCommand( const TCollection_AsciiString& theCommand );
  void ExchangeCommands( Handle(_pyCommand) theCmd1, Handle(_pyCommand) theCmd2 );
  void SetCommandAfter( Handle(_pyCommand) theCmd, Handle(_pyCommand) theAfterCmd );
  void SetCommandBefore( Handle(_pyCommand) theCmd, Handle(_pyCommand) theBeforeCmd );
  Handle(_pyCommand)& GetLastCommand();
  std::list< Handle(_pyCommand) >& GetCommands() { return myCommands; }

  _pyID GenerateNewID( const _pyID& theID );
  void AddObject( Handle(_pyObject)& theObj );
  Handle(_pyObject) FindObject( const _pyID& theObjID ) const;
  Handle(_pySubMesh) FindSubMesh( const _pyID& theSubMeshID );
  Handle(_pyHypothesis) FindHyp( const _pyID& theHypID );
  Handle(_pyHypothesis) FindAlgo( const _pyID& theGeom, const _pyID& theMesh,
                                  const Handle(_pyHypothesis)& theHypothesis);

  void SetAccessorMethod(const _pyID& theID, const char* theMethod );
  bool AddMeshAccessorMethod( Handle(_pyCommand) theCmd ) const;
  bool AddAlgoAccessorMethod( Handle(_pyCommand) theCmd ) const;
  virtual const char* AccessorMethod() const;

  bool IsGeomObject(const _pyID& theObjID) const;
  bool IsNotPublished(const _pyID& theObjID) const;
  bool IsToKeepAllCommands() const { return myToKeepAllCommands; }
  void AddExportedMesh(const TCollection_AsciiString& file, const ExportedMeshData& mesh )
  { myFile2ExportedMesh[ file ] = mesh; }
  ExportedMeshData& FindExportedMesh( const TCollection_AsciiString& file )
  { return myFile2ExportedMesh[ file ]; }

  virtual void Process( const Handle(_pyCommand)& theCommand );
  virtual void Flush();
  virtual void ClearCommands();
  virtual void Free();

private:
  void setNeighbourCommand( Handle(_pyCommand)& theCmd,
                            Handle(_pyCommand)& theOtherCmd,
                            const bool theIsAfter );
  
private:
  std::map< _pyID, Handle(_pyMesh) >       myMeshes;
  std::map< _pyID, Handle(_pyMeshEditor) > myMeshEditors;
  std::map< _pyID, Handle(_pyObject) >     myObjects;
  std::list< Handle(_pyHypothesis) >       myHypos;
  std::list< Handle(_pyCommand) >          myCommands;
  int                                      myNbCommands;
  Resource_DataMapOfAsciiStringAsciiString& myID2AccessorMethod;
  Resource_DataMapOfAsciiStringAsciiString& myObjectNames;
  Handle(_pyCommand)                       myLastCommand;
  int                                      myNbFilters;
  bool                                     myToKeepAllCommands;
  SALOMEDS::Study_var                      myStudy;
  int                                      myGeomIDNb, myGeomIDIndex;
  std::map< TCollection_AsciiString, ExportedMeshData > myFile2ExportedMesh;

  DEFINE_STANDARD_RTTI (_pyGen)
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
  std::list< Handle(_pyCommand) >    myAddHypCmds;
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

  DEFINE_STANDARD_RTTI (_pyMesh)
};
#undef _pyMesh_ACCESS_METHOD 

// -------------------------------------------------------------------------------------
/*!
 * \brief MeshEditor convert its commands to ones of mesh
 */
// -------------------------------------------------------------------------------------
class _pyMeshEditor: public _pyObject
{
  _pyID myMesh;
  TCollection_AsciiString myCreationCmdStr;
public:
  _pyMeshEditor(const Handle(_pyCommand)& theCreationCmd);
  _pyID GetMesh() const { return myMesh; }
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush() {}
  virtual bool CanClear();

  DEFINE_STANDARD_RTTI (_pyMesh)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Root class for hypothesis
 *
 * HOWTO assure convertion of a new type of hypothesis
 * In _pyHypothesis::NewHypothesis():
 * 1. add a case for the name of the new hypothesis
 * 2. use SetConvMethodAndType() to set
 *    . for algo: algorithm name and method of Mesh creating the algo
 *    . for hypo: name of the algorithm and method creating the hypothesis
 * 3. append to myArgMethods interface methods setting param values in the
 *    order they are used when creation method is called. If arguments of
 *    the creation method can't be easily got from calls of hypothesis methods, you are
 *    to derive a specific class from _pyHypothesis that would redefine Process(),
 *    see _pyComplexParamHypo for example
 */
// -------------------------------------------------------------------------------------
class _pyHypothesis: public _pyObject
{
protected:
  bool    myIsAlgo, myIsWrapped;
  _pyID   myGeom,   myMesh;
  // a hypothesis can be used and created by different algos by different methods
  std::map<TCollection_AsciiString, TCollection_AsciiString > myType2CreationMethod;
  TColStd_SequenceOfAsciiString myArgs;           // creation arguments
  TColStd_SequenceOfAsciiString myArgMethods;     // hypo methods setting myArgs
  TColStd_SequenceOfInteger     myNbArgsByMethod; // nb args set by each method
  std::list<Handle(_pyCommand)> myArgCommands;
  std::list<Handle(_pyCommand)> myUnknownCommands;
  std::list<Handle(_pyObject) > myReferredObjs;
  // maps used to clear commands setting parameters if result of setting is not
  // used (no mesh.Compute()) or discared (e.g. by mesh.Clear())
  std::map<TCollection_AsciiString, std::list<Handle(_pyCommand)> > myMeth2Commands;
  std::map< _pyCommand::TAddr, std::list<Handle(_pyCommand) > >     myComputeAddr2Cmds;
  std::list<Handle(_pyCommand) >                                    myComputeCmds;
  void rememberCmdOfParameter( const Handle(_pyCommand) & cmd );
  bool isCmdUsedForCompute( const Handle(_pyCommand) & cmd,
                            _pyCommand::TAddr avoidComputeAddr=NULL ) const;
public:
  _pyHypothesis(const Handle(_pyCommand)& theCreationCmd);
  void SetConvMethodAndType(const char* creationMethod, const char* type)
  { myType2CreationMethod[ (char*)type ] = (char*)creationMethod; }
  void AddArgMethod(const char* method, const int nbArgs = 1)
  { myArgMethods.Append( (char*)method ); myNbArgsByMethod.Append( nbArgs ); }
  const TColStd_SequenceOfAsciiString& GetArgs() const { return myArgs; }
  const std::list<Handle(_pyCommand)>& GetArgCommands() const { return myArgCommands; }
  void ClearAllCommands();
  virtual bool IsAlgo() const { return myIsAlgo; }
  bool IsValid() const { return !myType2CreationMethod.empty(); }
  bool IsWrapped() const { return myIsWrapped; }
  const _pyID & GetGeom() const { return myGeom; }
  void SetMesh( const _pyID& theMeshId) { if ( myMesh.IsEmpty() ) myMesh = theMeshId; }
  const _pyID & GetMesh() const { return myMesh; }
  const TCollection_AsciiString& GetAlgoType() const
  { return myType2CreationMethod.begin()->first; }
  const TCollection_AsciiString& GetAlgoCreationMethod() const
  { return myType2CreationMethod.begin()->second; }
  bool CanBeCreatedBy(const TCollection_AsciiString& algoType ) const
  { return myType2CreationMethod.find( algoType ) != myType2CreationMethod.end(); }
  const TCollection_AsciiString& GetCreationMethod(const TCollection_AsciiString& algoType) const
  { return myType2CreationMethod.find( algoType )->second; }
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


  DEFINE_STANDARD_RTTI (_pyHypothesis)
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

  DEFINE_STANDARD_RTTI (_pyAlgorithm)
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

  DEFINE_STANDARD_RTTI (_pyComplexParamHypo)
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
  TCollection_AsciiString myAlgoMethod;
public:
  _pyLayerDistributionHypo(const Handle(_pyCommand)& theCreationCmd, const char* algoMethod):
    _pyHypothesis(theCreationCmd), myAlgoMethod((char*)algoMethod) {}
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  virtual void Free() { my1dHyp.Nullify(); }

  DEFINE_STANDARD_RTTI (_pyLayerDistributionHypo)
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

  DEFINE_STANDARD_RTTI (_pyNumberOfSegmentsHyp)
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
  DEFINE_STANDARD_RTTI (_pySegmentLengthAroundVertexHyp)
};
DEFINE_STANDARD_HANDLE (_pySegmentLengthAroundVertexHyp, _pyHypothesis);

// -------------------------------------------------------------------------------------
/*!
 * \brief SelfEraser erases creation command if no more it's commands invoked
 */
// -------------------------------------------------------------------------------------
class _pySelfEraser: public _pyObject
{
public:
  _pySelfEraser(const Handle(_pyCommand)& theCreationCmd):_pyObject(theCreationCmd) {}
  virtual void Flush();

  DEFINE_STANDARD_RTTI (_pySelfEraser)
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
  _pySubMesh(const Handle(_pyCommand)& theCreationCmd);
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual Handle(_pyMesh) GetMesh() { return myMesh; }
  virtual void Free() { myCreator.Nullify(); myMesh.Nullify(); }
  void SetCreator( const Handle(_pyObject)& theCreator ) { myCreator = theCreator; }

  DEFINE_STANDARD_RTTI (_pySubMesh)
};
// -------------------------------------------------------------------------------------
/*!
 * \brief To convert creation of a group by filter
 */
// -------------------------------------------------------------------------------------
class _pyGroup:  public _pySubMesh
{
public:
  _pyGroup(const Handle(_pyCommand)& theCreationCmd, const _pyID & id=_pyID())
    :_pySubMesh(theCreationCmd) { setID( id ); }
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush() {}

  DEFINE_STANDARD_RTTI (_pyGroup)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief A filter sets a human readable name to self
 */
// -------------------------------------------------------------------------------------
class _pyFilter:  public _pyObject
{
  _pyID myNewID;
  std::list< Handle(_pyObject) > myUsers;
public:
  _pyFilter(const Handle(_pyCommand)& theCreationCmd, const _pyID& newID="");
  void AddUser( const Handle(_pyObject)& user) { myUsers.push_back( user ); }
  virtual void Process( const Handle(_pyCommand)& theCommand);
  virtual void Flush();
  virtual bool CanClear();
  virtual void Free() { myUsers.clear(); }
  const _pyID& GetNewID() const { return myNewID; }

  DEFINE_STANDARD_RTTI (_pyFilter)
};
DEFINE_STANDARD_HANDLE (_pyFilter, _pyObject);

#endif
