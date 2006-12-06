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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : SMESH_smesh.hxx
// Created   : Fri Nov 18 12:05:18 2005
// Author    : Edward AGAPOV (eap)

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

// ===========================================================================================
/*!
 * \brief Tool converting SMESH engine calls into commands defined in smesh.py
 *
 * This file was created in order to respond to requirement of bug PAL10494:
 * SMESH python dump uses idl interface.
 *
 * The creation reason is that smesh.py commands defining hypotheses encapsulate
 * several SMESH engine method calls. As well, the dependencies between smesh.py
 * classes differ from ones between SMESH IDL interfaces.
 * 
 * The only API method here is SMESH_2smeshpy::ConvertScript(), the rest ones are
 * for internal usage
 *
 * See comments to _pyHypothesis class to know how to assure convertion of a new hypothesis
 */
// ===========================================================================================

class Resource_DataMapOfAsciiStringAsciiString;

class SMESH_2smeshpy
{
public:
  /*!
   * \brief Convert a python script using commands of smesh.py
   * \param theScript - Input script
   * \param theEntry2AccessorMethod - The returning method names to access to
   *        objects wrapped with python class
   * \retval TCollection_AsciiString - Convertion result
   */
  static TCollection_AsciiString
  ConvertScript(const TCollection_AsciiString& theScript,
                Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod);

  /*!
   * \brief Return the name of the python file wrapping IDL API
    * \retval TCollection_AsciiString - The file name
   */
  static char* SmeshpyName() { return "smesh"; }
  static char* GenName() { return "smesh.smesh"; }
};

// ===========================================================================================
// =====================
//    INTERNAL STUFF
// =====================
// ===========================================================================================

class _pyCommand;
class _pyObject;
class _pyGen;
class _pyMesh;
class _pyHypothesis;
class _pyAlgorithm;

DEFINE_STANDARD_HANDLE (_pyCommand   ,Standard_Transient);
DEFINE_STANDARD_HANDLE (_pyObject    ,Standard_Transient);
DEFINE_STANDARD_HANDLE (_pyGen       ,_pyObject);
DEFINE_STANDARD_HANDLE (_pyMesh      ,_pyObject);
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
  int Length() { return myString.Length(); }
  void Clear() { myString.Clear(); myBegPos.Clear(); }
  bool IsEmpty() const { return myString.IsEmpty(); }
  const TCollection_AsciiString & GetResultValue();
  const TCollection_AsciiString & GetObject();
  const TCollection_AsciiString & GetMethod();
  const TCollection_AsciiString & GetArg( int index );
  int GetNbArgs() { FindAllArgs(); return myArgs.Length(); }
  //Handle(TColStd_HSequenceOfAsciiString) GetArgs();
  void SetResultValue( const TCollection_AsciiString& theResult )
  { GetResultValue(); SetPart( RESULT_IND, theResult, myRes ); }
  void SetObject(const TCollection_AsciiString& theObject)
  { GetObject(); SetPart( OBJECT_IND, theObject, myObj ); }
  void SetMethod(const TCollection_AsciiString& theMethod)
  { GetMethod(); SetPart( METHOD_IND, theMethod, myMeth ); }
  void SetArg( int index, const TCollection_AsciiString& theArg);
  void RemoveArgs();
  static bool SkipSpaces( const TCollection_AsciiString & theSring, int & thePos );
  static TCollection_AsciiString GetWord( const TCollection_AsciiString & theSring,
                                          int & theStartPos, const bool theForward,
                                          const bool dotIsWord = false);
  void AddDependantCmd( Handle(_pyCommand) cmd)
  { return myDependentCmds.push_back( cmd ); }
  bool SetDependentCmdsAfter() const;

  bool AddAccessorMethod( _pyID theObjectID, const char* theAcsMethod );

  DEFINE_STANDARD_RTTI (_pyCommand)
};

// -------------------------------------------------------------------------------------
/*!
 * \brief Root of all objects
 */
// -------------------------------------------------------------------------------------

class _pyObject: public Standard_Transient
{
  Handle(_pyCommand)              myCreationCmd;
public:
  _pyObject(const Handle(_pyCommand)& theCreationCmd): myCreationCmd(theCreationCmd) {}
  const _pyID& GetID() { return myCreationCmd->GetResultValue(); }
  const Handle(_pyCommand)& GetCreationCmd() { return myCreationCmd; }
  void  SetCreationCmd( Handle(_pyCommand) cmd ) { myCreationCmd = cmd; }
  int GetCommandNb() { return myCreationCmd->GetOrderNb(); }
  virtual void Process(const Handle(_pyCommand) & theCommand) = 0;
  virtual void Flush() = 0;
  virtual const char* AccessorMethod() const;

  DEFINE_STANDARD_RTTI (_pyObject)
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
  _pyGen(Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod);
  //~_pyGen();
  void AddCommand( const TCollection_AsciiString& theCommand );
  void Process( const Handle(_pyCommand)& theCommand );
  void Flush();
  Handle(_pyHypothesis) FindHyp( const _pyID& theHypID );
  Handle(_pyHypothesis) FindAlgo( const _pyID& theGeom, const _pyID& theMesh,
                                 const TCollection_AsciiString& theAlgoType);
  void ExchangeCommands( Handle(_pyCommand) theCmd1, Handle(_pyCommand) theCmd2 );
  void SetCommandAfter( Handle(_pyCommand) theCmd, Handle(_pyCommand) theAfterCmd );
  std::list< Handle(_pyCommand) >& GetCommands() { return myCommands; }
  void SetAccessorMethod(const _pyID& theID, const char* theMethod );
  const char* AccessorMethod() const { return SMESH_2smeshpy::GenName(); }
private:
  std::map< _pyID, Handle(_pyMesh) > myMeshes;
  std::list< Handle(_pyHypothesis) > myHypos;
  std::list< Handle(_pyCommand) >    myCommands;
  int                                myNbCommands;
  bool                               myHasPattern;
  Resource_DataMapOfAsciiStringAsciiString& myID2AccessorMethod;

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
  std::list< Handle(_pyCommand) > myAddHypCmds;
  std::list< Handle(_pyCommand) > mySubmeshes;
  bool                            myHasEditor;
public:
  _pyMesh(const Handle(_pyCommand) theCreationCmd);
  const _pyID& GetGeom() { return GetCreationCmd()->GetArg(1); }
  void Process( const Handle(_pyCommand)& theCommand);
  void Flush();
  const char* AccessorMethod() const { return _pyMesh_ACCESS_METHOD; }
private:
  static void AddMeshAccess( const Handle(_pyCommand)& theCommand )
  { theCommand->SetObject( theCommand->GetObject() + "." _pyMesh_ACCESS_METHOD ); }

  DEFINE_STANDARD_RTTI (_pyMesh)
};
#undef _pyMesh_ACCESS_METHOD 

// -------------------------------------------------------------------------------------
/*!
 * \brief Root class for hypothesis
 *
 * HOWTO assure convertion of a new hypothesis
 * In NewHypothesis():
 * 1. add a case for the name of the new hypothesis and
 * 2. initialize _pyHypothesis fields:
 *    . myDim - hypothesis dimention;
 *    . myType - type name of the algorithm creating the hypothesis;
 *    . myCreationMethod - method name of the algorithm creating the hypothesis;
 *    . append to myArgMethods interface methods setting param values in the
 *    order they are used when myCreationMethod is called. It is supposed that
 *    each interface method sets only one parameter, if it is not so, you are
 *    to derive a specific class from _pyHypothesis that would redefine Process(),
 *    see _pyComplexParamHypo for example
 */
// -------------------------------------------------------------------------------------
class _pyHypothesis: public _pyObject
{
protected:
  bool    myIsAlgo, /*myIsLocal, */myIsWrapped, myIsConverted;
  int     myDim, /*myAdditionCmdNb*/;
  _pyID    myGeom, myMesh;
  TCollection_AsciiString       myCreationMethod, myType;
  TColStd_SequenceOfAsciiString myArgs;
  TColStd_SequenceOfAsciiString myArgMethods;
  TColStd_SequenceOfInteger     myNbArgsByMethod;
  std::list<Handle(_pyCommand)>  myArgCommands;
  std::list<Handle(_pyCommand)>  myUnknownCommands;
public:
  _pyHypothesis(const Handle(_pyCommand)& theCreationCmd);
  void SetDimMethodType(const int dim, const char* creationMethod, const char* type=0)
  { myDim = dim; myCreationMethod = (char*)creationMethod; if ( type ) myType = (char*)type; }
  void AddArgMethod(const char* method, const int nbArgs = 1)
  { myArgMethods.Append( (char*)method ); myNbArgsByMethod.Append( nbArgs ); }
  const TColStd_SequenceOfAsciiString& GetArgs() const { return myArgs; }
  const TCollection_AsciiString& GetCreationMethod() const { return myCreationMethod; }
  const std::list<Handle(_pyCommand)>& GetArgCommands() const { return myArgCommands; }
  void ClearAllCommands();
  virtual bool IsAlgo() const { return myIsAlgo; }
  bool IsWrapped() const { return myIsWrapped; }
  bool & IsConverted() { return myIsConverted; }
  int GetDim() const { return myDim; }
  const _pyID & GetGeom() const { return myGeom; }
  void SetMesh( const _pyID& theMeshId) { if ( myMesh.IsEmpty() ) myMesh = theMeshId; }
  const _pyID & GetMesh() const { return myMesh; }
  const TCollection_AsciiString GetType() { return myType; }
  bool IsWrappable(const _pyID& theMesh) { return !myIsWrapped && myMesh == theMesh; }
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  static Handle(_pyHypothesis) NewHypothesis( const Handle(_pyCommand)& theCreationCmd);
  //     bool HasMesh() const { return !myMesh.IsEmpty(); }
  //     void SetGeom( const _pyID& theGeomID ) { myGeom = theGeomID; }
  void Process( const Handle(_pyCommand)& theCommand);
  void Flush();

  DEFINE_STANDARD_RTTI (_pyHypothesis)
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
  void Process( const Handle(_pyCommand)& theCommand);

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
public:
  _pyLayerDistributionHypo(const Handle(_pyCommand)& theCreationCmd):
    _pyHypothesis(theCreationCmd) {}
  void Process( const Handle(_pyCommand)& theCommand);
  void Flush();
  bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                          const _pyID&              theMesh);

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
 * \brief Class representing smesh.Mesh_Algorithm
 */
// -------------------------------------------------------------------------------------
class _pyAlgorithm: public _pyHypothesis
{
public:
  _pyAlgorithm(const Handle(_pyCommand)& theCreationCmd);
  virtual bool Addition2Creation( const Handle(_pyCommand)& theAdditionCmd,
                                  const _pyID&              theMesh);
  const char* AccessorMethod() const { return "GetAlgorithm()"; }

  DEFINE_STANDARD_RTTI (_pyAlgorithm)
};

#endif
