//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESH_2D_Algo_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

// File      : SMESH_2smeshpy.cxx
// Created   : Fri Nov 18 13:20:10 2005
// Author    : Edward AGAPOV (eap)

#include "SMESH_2smeshpy.hxx"

#include "utilities.h"
#include "SMESH_PythonDump.hxx"
#include "Resource_DataMapOfAsciiStringAsciiString.hxx"

#include "SMESH_Gen_i.hxx"
/* SALOME headers that include CORBA headers that include windows.h 
 * that defines GetObject symbol as GetObjectA should stand before SALOME headers
 * that declare methods named GetObject - to apply the same rules of GetObject renaming
 * and thus to avoid mess with GetObject symbol on Windows */

IMPLEMENT_STANDARD_HANDLE (_pyObject          ,Standard_Transient);
IMPLEMENT_STANDARD_HANDLE (_pyCommand         ,Standard_Transient);
IMPLEMENT_STANDARD_HANDLE (_pyGen             ,_pyObject);
IMPLEMENT_STANDARD_HANDLE (_pyMesh            ,_pyObject);
IMPLEMENT_STANDARD_HANDLE (_pyHypothesis      ,_pyObject);
IMPLEMENT_STANDARD_HANDLE (_pyAlgorithm       ,_pyHypothesis);
IMPLEMENT_STANDARD_HANDLE (_pyComplexParamHypo,_pyHypothesis);
IMPLEMENT_STANDARD_HANDLE (_pyNumberOfSegmentsHyp,_pyHypothesis);

IMPLEMENT_STANDARD_RTTIEXT(_pyObject          ,Standard_Transient);
IMPLEMENT_STANDARD_RTTIEXT(_pyCommand         ,Standard_Transient);
IMPLEMENT_STANDARD_RTTIEXT(_pyGen             ,_pyObject);
IMPLEMENT_STANDARD_RTTIEXT(_pyMesh            ,_pyObject);
IMPLEMENT_STANDARD_RTTIEXT(_pyHypothesis      ,_pyObject);
IMPLEMENT_STANDARD_RTTIEXT(_pyAlgorithm       ,_pyHypothesis);
IMPLEMENT_STANDARD_RTTIEXT(_pyComplexParamHypo,_pyHypothesis);
IMPLEMENT_STANDARD_RTTIEXT(_pyNumberOfSegmentsHyp,_pyHypothesis);
IMPLEMENT_STANDARD_RTTIEXT(_pyLayerDistributionHypo,_pyHypothesis);

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

//================================================================================
/*!
 * \brief Convert python script using commands of smesh.py
  * \param theScript - Input script
  * \retval TCollection_AsciiString - Convertion result
 */
//================================================================================

TCollection_AsciiString
SMESH_2smeshpy::ConvertScript(const TCollection_AsciiString& theScript,
                              Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod)
{
  theGen = new _pyGen( theEntry2AccessorMethod );

  // split theScript into separate commands
  int from = 1, end = theScript.Length(), to;
  while ( from < end && ( to = theScript.Location( "\n", from, end )))
  {
    if ( to != from )
      // cut out and store a command
      theGen->AddCommand( theScript.SubString( from, to - 1 ));
    from = to + 1;
  }
  // finish conversion
  theGen->Flush();
#ifdef DUMP_CONVERSION
  cout << endl << " ######## RESULT ######## " << endl<< endl;
#endif
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
  TCollection_AsciiString aScript;
  for ( cmd = theGen->GetCommands().begin(); cmd != theGen->GetCommands().end(); ++cmd )
  {
#ifdef DUMP_CONVERSION
    cout << "## COM " << (*cmd)->GetOrderNb() << ": "<< (*cmd)->GetString() << endl;
#endif
    if ( !(*cmd)->IsEmpty() ) {
      aScript += "\n";
      aScript += (*cmd)->GetString();
    }
  }
  aScript += "\n";

  theGen.Nullify();

  return aScript;
}

//================================================================================
/*!
 * \brief _pyGen constructor
 */
//================================================================================

_pyGen::_pyGen(Resource_DataMapOfAsciiStringAsciiString& theEntry2AccessorMethod)
  : _pyObject( new _pyCommand( TPythonDump::SMESHGenName(), 0 )),
    myID2AccessorMethod( theEntry2AccessorMethod )
{
  myNbCommands = 0;
  myHasPattern = false;
  // make that GetID() to return TPythonDump::SMESHGenName()
  GetCreationCmd()->GetString() += "=";
}

//================================================================================
/*!
 * \brief Convert a command using a specific converter
  * \param theCommand - the command to convert
 */
//================================================================================

void _pyGen::AddCommand( const TCollection_AsciiString& theCommand)
{
  // store theCommand in the sequence
  myCommands.push_back( new _pyCommand( theCommand, ++myNbCommands ));

  Handle(_pyCommand) aCommand = myCommands.back();
#ifdef DUMP_CONVERSION
  cout << "## COM " << myNbCommands << ": "<< aCommand->GetString() << endl;
#endif

  _pyID objID = aCommand->GetObject();

  if ( objID.IsEmpty() )
    return;

  // SMESH_Gen method?
  if ( objID == this->GetID() ) {
    this->Process( aCommand );
    return;
  }
  // SMESH_Mesh method?
  map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.find( objID );
  if ( id_mesh != myMeshes.end() ) {
    id_mesh->second->Process( aCommand );
    return;
  }
  // SMESH_Hypothesis method?
  list< Handle(_pyHypothesis) >::iterator hyp = myHypos.begin();
  for ( ; hyp != myHypos.end(); ++hyp )
    if ( !(*hyp)->IsAlgo() && objID == (*hyp)->GetID() ) {
      (*hyp)->Process( aCommand );
      return;
    }

  // Add access to a wrapped mesh
  AddMeshAccessorMethod( aCommand );

  // Add access to a wrapped algorithm
  AddAlgoAccessorMethod( aCommand );

  // PAL12227. PythonDump was not updated at proper time; result is
  //     aCriteria.append(SMESH.Filter.Criterion(17,26,0,'L1',26,25,1e-07,SMESH.EDGE,-1))
  // TypeError: __init__() takes exactly 11 arguments (10 given)
  char wrongCommand[] = "SMESH.Filter.Criterion(";
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
  }
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
  // CreateHypothesis( theHypType, theLibName )
  // Compute( mesh, geom )

  if ( theCommand->GetMethod() == "CreateMesh" )
  {
    Handle(_pyMesh) mesh = new _pyMesh( theCommand );
    myMeshes.insert( make_pair( mesh->GetID(), mesh ));
    return;
  }

  // CreateHypothesis()
  if ( theCommand->GetMethod() == "CreateHypothesis" )
  {
    myHypos.push_back( _pyHypothesis::NewHypothesis( theCommand ));
    return;
  }

  // smeshgen.Compute( mesh, geom ) --> mesh.Compute()
  if ( theCommand->GetMethod() == "Compute" )
  {
    const _pyID& meshID = theCommand->GetArg( 1 );
    map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.find( meshID );
    if ( id_mesh != myMeshes.end() ) {
      theCommand->SetObject( meshID );
      theCommand->RemoveArgs();
      id_mesh->second->Flush();
      return;
    }
  }

  // leave only one smeshgen.GetPattern() in the script
  if ( theCommand->GetMethod() == "GetPattern" ) {
    if ( myHasPattern ) {
      theCommand->Clear();
      return;
    }
    myHasPattern = true;
  }

  // smeshgen.Method() --> smesh.smesh.Method()
  theCommand->SetObject( SMESH_2smeshpy::GenName() );
}

//================================================================================
/*!
 * \brief Convert the remembered commands
 */
//================================================================================

void _pyGen::Flush()
{
  map< _pyID, Handle(_pyMesh) >::iterator id_mesh = myMeshes.begin();
  for ( ; id_mesh != myMeshes.end(); ++id_mesh )
    if ( ! id_mesh->second.IsNull() )
      id_mesh->second->Flush();

  list< Handle(_pyHypothesis) >::iterator hyp = myHypos.begin();
  for ( ; hyp != myHypos.end(); ++hyp )
    if ( !hyp->IsNull() ) {
      (*hyp)->Flush();
      // smeshgen.CreateHypothesis() --> smesh.smesh.CreateHypothesis()
      if ( !(*hyp)->IsWrapped() )
        (*hyp)->GetCreationCmd()->SetObject( SMESH_2smeshpy::GenName() );
    }
}

//================================================================================
/*!
 * \brief Add access method to mesh that is object or arg
  * \param theCmd - command to add access method
  * \retval bool - true if added
 */
//================================================================================

bool _pyGen::AddMeshAccessorMethod( Handle(_pyCommand) theCmd ) const
{
  map< _pyID, Handle(_pyMesh) >::const_iterator id_mesh = myMeshes.begin();
  for ( ; id_mesh != myMeshes.end(); ++id_mesh ) {
    if ( theCmd->AddAccessorMethod( id_mesh->first, id_mesh->second->AccessorMethod() ))
      return true;
  }
  return false;
}

//================================================================================
/*!
 * \brief Add access method to algo that is object or arg
  * \param theCmd - command to add access method
  * \retval bool - true if added
 */
//================================================================================

bool _pyGen::AddAlgoAccessorMethod( Handle(_pyCommand) theCmd ) const
{
  list< Handle(_pyHypothesis) >::const_iterator hyp = myHypos.begin();
  for ( ; hyp != myHypos.end(); ++hyp ) {
    if ( (*hyp)->IsAlgo() &&
         theCmd->AddAccessorMethod( (*hyp)->GetID(), (*hyp)->AccessorMethod() ))
      return true;
  }
  return false;
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
  list< Handle(_pyHypothesis) >::iterator hyp = myHypos.begin();
  for ( ; hyp != myHypos.end(); ++hyp )
    if ( !hyp->IsNull() && theHypID == (*hyp)->GetID() )
      return *hyp;
  return Handle(_pyHypothesis)();
}

//================================================================================
/*!
 * \brief Find algorithm the created algorithm
  * \param theGeom - The shape ID the algorithm was created on
  * \param theMesh - The mesh ID that created the algorithm
  * \param dim - The algo dimension
  * \retval Handle(_pyHypothesis) - The found algo
 */
//================================================================================

Handle(_pyHypothesis) _pyGen::FindAlgo( const _pyID& theGeom, const _pyID& theMesh,
                                      const TCollection_AsciiString& theAlgoType )
{
  list< Handle(_pyHypothesis) >::iterator hyp = myHypos.begin();
  for ( ; hyp != myHypos.end(); ++hyp )
    if ( !hyp->IsNull() &&
         (*hyp)->IsAlgo() &&
         (*hyp)->GetType() == theAlgoType &&
         (*hyp)->GetGeom() == theGeom &&
         (*hyp)->GetMesh() == theMesh )
      return *hyp;
  return 0;
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
//   cout << "SET\t" << theCmd->GetString() << endl << "AFTER\t" << theAfterCmd->GetString() << endl << endl;
  list< Handle(_pyCommand) >::iterator pos;
  pos = find( myCommands.begin(), myCommands.end(), theCmd );
  myCommands.erase( pos );
  pos = find( myCommands.begin(), myCommands.end(), theAfterCmd );
  myCommands.insert( ++pos, theCmd );

  int i = 1;
  for ( pos = myCommands.begin(); pos != myCommands.end(); ++pos)
    (*pos)->SetOrderNb( i++ );
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
 * \brief Find out type of geom group
  * \param grpID - The geom group entry
  * \retval int - The type
 */
//================================================================================

static bool sameGroupType( const _pyID&                   grpID,
                           const TCollection_AsciiString& theType)
{
  // define group type as smesh.Mesh.Group() does
  int type = -1;
  SALOMEDS::Study_var study = SMESH_Gen_i::GetSMESHGen()->GetCurrentStudy();
  SALOMEDS::SObject_var aSObj = study->FindObjectID( grpID.ToCString() );
  if ( !aSObj->_is_nil() ) {
    GEOM::GEOM_Object_var aGeomObj = GEOM::GEOM_Object::_narrow( aSObj->GetObject() );
    if ( !aGeomObj->_is_nil() ) {
      switch ( aGeomObj->GetShapeType() ) {
      case GEOM::VERTEX: type = SMESH::NODE; break;
      case GEOM::EDGE:   type = SMESH::EDGE; break;
      case GEOM::FACE:   type = SMESH::FACE; break;
      case GEOM::SOLID:
      case GEOM::SHELL:  type = SMESH::VOLUME; break;
      case GEOM::COMPOUND: {
        GEOM::GEOM_Gen_var aGeomGen = SMESH_Gen_i::GetSMESHGen()->GetGeomEngine();
        if ( !aGeomGen->_is_nil() ) {
          GEOM::GEOM_IGroupOperations_var aGrpOp =
            aGeomGen->GetIGroupOperations( study->StudyId() );
          if ( !aGrpOp->_is_nil() ) {
            switch ( aGrpOp->GetType( aGeomObj )) {
            case TopAbs_VERTEX: type = SMESH::NODE; break;
            case TopAbs_EDGE:   type = SMESH::EDGE; break;
            case TopAbs_FACE:   type = SMESH::FACE; break;
            case TopAbs_SOLID:  type = SMESH::VOLUME; break;
            default:;
            }
          }
        }
      }
      default:;
      }
    }
  }
  if ( type < 0 ) {
    MESSAGE("Type of the group " << grpID << " not found");
    return false;
  }
  if ( theType.IsIntegerValue() )
    return type == theType.IntegerValue();

  switch ( type ) {
  case SMESH::NODE:   return theType.Location( "NODE", 1, theType.Length() );
  case SMESH::EDGE:   return theType.Location( "EDGE", 1, theType.Length() );
  case SMESH::FACE:   return theType.Location( "FACE", 1, theType.Length() );
  case SMESH::VOLUME: return theType.Location( "VOLUME", 1, theType.Length() );
  default:;
  }
  return false;
}

//================================================================================
/*!
 * \brief 
  * \param theCreationCmd - 
 */
//================================================================================

_pyMesh::_pyMesh(const Handle(_pyCommand) theCreationCmd):
  _pyObject(theCreationCmd), myHasEditor(false)
{
  // convert my creation command
  Handle(_pyCommand) creationCmd = GetCreationCmd();
  creationCmd->SetObject( SMESH_2smeshpy::SmeshpyName() );
  creationCmd->SetMethod( "Mesh" );

  theGen->SetAccessorMethod( GetID(), "GetMesh()" );
}

//================================================================================
/*!
 * \brief Convert a IDL API command of SMESH::Mesh to a method call of python Mesh
  * \param theCommand - Engine method called for this mesh
 */
//================================================================================

void _pyMesh::Process( const Handle(_pyCommand)& theCommand )
{
  // smesh.py wraps the following methods:
  //
  // 1. GetSubMesh(geom, name) + AddHypothesis(geom, algo)
  //     --> in Mesh_Algorithm.Create(mesh, geom, hypo, so)
  // 2. AddHypothesis(geom, hyp)
  //     --> in Mesh_Algorithm.Hypothesis(hyp, args, so)
  // 3. CreateGroupFromGEOM(type, name, grp)
  //     --> in Mesh.Group(grp, name="")
  // 4. ExportToMED(f, opt, version)
  //     --> in Mesh.ExportToMED( f, version, opt=0 )
  // 5. ExportMED(f, opt)
  //     --> in Mesh.ExportMED( f,opt=0 )
  // 6. ExportDAT(f)
  //     --> in Mesh.ExportDAT( f )
  // 7. ExportUNV(f)
  //     --> in Mesh.ExportUNV(f)
  // 8. ExportSTL(f, ascii)
  //     --> in Mesh.ExportSTL(f, ascii=1)

  const TCollection_AsciiString method = theCommand->GetMethod();
  if ( method == "GetSubMesh" ) {
    mySubmeshes.push_back( theCommand );
  }
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
  else if ( method == "CreateGroupFromGEOM" ) {// (type, name, grp)
    _pyID grp = theCommand->GetArg( 3 );
    if ( sameGroupType( grp, theCommand->GetArg( 1 )) ) { // --> Group(grp)
      theCommand->SetMethod( "Group" );
      theCommand->RemoveArgs();
      theCommand->SetArg( 1, grp );
    }
    else {
      AddMeshAccess( theCommand );
    }
  }
  else if ( method == "ExportToMED" ) {//(f, opt, version)
    // --> (f, version, opt)
    _pyID opt = theCommand->GetArg( 2 );
    _pyID ver = theCommand->GetArg( 3 );
    theCommand->SetArg( 2, ver );
    theCommand->SetArg( 3, opt );
  }
  else if ( method == "RemoveHypothesis" ) // (geom, hyp)
  {
    const _pyID & hypID = theCommand->GetArg( 2 );

    // check if this mesh still has corresponding addition command
    bool hasAddCmd = false;
    list< Handle(_pyCommand) >::iterator cmd = myAddHypCmds.begin();
    while ( cmd != myAddHypCmds.end() )
    {
      // AddHypothesis(geom, hyp)
      if ( hypID == (*cmd)->GetArg( 2 )) { // erase both (add and remove) commands
        theCommand->Clear();
        (*cmd)->Clear();
        cmd = myAddHypCmds.erase( cmd );
        hasAddCmd = true;
      }
      else {
        ++cmd;
      }
    }
    Handle(_pyHypothesis) hyp = theGen->FindHyp( hypID );
    if ( ! hasAddCmd ) { // hypo addition already wrapped
      // access to wrapped mesh
      AddMeshAccess( theCommand );
      // access to wrapped algo
      if ( !hyp.IsNull() && hyp->IsAlgo() && hyp->IsWrapped() )
        theCommand->SetArg( 2, theCommand->GetArg( 2 ) + ".GetAlgorithm()" );
    }
    // remove hyp from myHypos
    myHypos.remove( hyp );
  }

  // leave only one "  mesh_editor_<nb> = mesh.GetMeshEditor()"
  else if ( theCommand->GetMethod() == "GetMeshEditor")
  {
    if ( myHasEditor )
      theCommand->Clear();
    else
      AddMeshAccess( theCommand );
    myHasEditor = true;
  }

  // apply theCommand to the mesh wrapped by smeshpy mesh
  else
  {
    AddMeshAccess( theCommand );
  }
}

//================================================================================
/*!
 * \brief Convert creation and addition of all algos and hypos
 */
//================================================================================

void _pyMesh::Flush()
{
  list < Handle(_pyCommand) >::iterator cmd, cmd2;

  // try to convert algo addition like this:
  // mesh.AddHypothesis(geom, ALGO ) --> ALGO = mesh.Algo()
  for ( cmd = myAddHypCmds.begin(); cmd != myAddHypCmds.end(); ++cmd )
  {
    Handle(_pyCommand) addCmd = *cmd;
    const _pyID& algoID = addCmd->GetArg( 2 );
    Handle(_pyHypothesis) algo = theGen->FindHyp( algoID );
    if ( algo.IsNull() || !algo->IsAlgo() )
      continue;
    // try to convert
    _pyID geom = addCmd->GetArg( 1 );
    if ( algo->Addition2Creation( addCmd, this->GetID() )) // OK
    {
      // wrapped algo is created atfer mesh creation
      GetCreationCmd()->AddDependantCmd( addCmd );

      if ( geom != GetGeom() ) // local algo
      {
        // mesh.AddHypothesis(geom, ALGO ) --> mesh.AlgoMethod(geom)
        addCmd->SetArg( addCmd->GetNbArgs() + 1,
                        TCollection_AsciiString( "geom=" ) + geom );
        // sm = mesh.GetSubMesh(geom, name) --> sm = ALGO.GetSubMesh()
        for ( cmd2 = mySubmeshes.begin(); cmd2 != mySubmeshes.end(); ++cmd2 ) {
          Handle(_pyCommand) subCmd = *cmd2;
          if ( geom == subCmd->GetArg( 1 )) {
            subCmd->SetObject( algo->GetID() );
            subCmd->RemoveArgs();
            addCmd->AddDependantCmd( subCmd );
          }
        }
      }
    }
    else // ALGO was already created
    {
      // mesh.AddHypothesis(geom, ALGO ) --> mesh.GetMesh().AddHypothesis(geom, ALGO )
      AddMeshAccess( addCmd );
      // mesh.GetMesh().AddHypothesis(geom, ALGO ) ->
      // mesh.GetMesh().AddHypothesis(geom, ALGO.GetAlgorithm() )
      addCmd->SetArg( 2, addCmd->GetArg( 2 ) + ".GetAlgorithm()" );
    }
  }

  // try to convert hypo addition like this:
  // mesh.AddHypothesis(geom, HYPO ) --> HYPO = algo.Hypo()
  for ( cmd = myAddHypCmds.begin(); cmd != myAddHypCmds.end(); ++cmd )
  {
    Handle(_pyCommand) addCmd = *cmd;
    const _pyID& hypID = addCmd->GetArg( 2 );
    Handle(_pyHypothesis) hyp = theGen->FindHyp( hypID );
    if ( hyp.IsNull() || hyp->IsAlgo() )
      continue;
    const _pyID& geom = addCmd->GetArg( 1 );
    // find algo created on <geom> for this mesh
    Handle(_pyHypothesis) algo = theGen->FindAlgo( geom, this->GetID(), hyp->GetType() );
    //_pyID algoID = algo.IsNull() ? "" : algo->GetID();
    if ( !algo.IsNull() && hyp->Addition2Creation( addCmd, this->GetID() )) // OK
    {
      addCmd->SetObject( algo->GetID() );
      algo->GetCreationCmd()->AddDependantCmd( addCmd );
    }
    else
    {
      AddMeshAccess( addCmd );
    }
  }

  // sm = mesh.GetSubMesh(geom, name) --> sm = mesh.GetMesh().GetSubMesh(geom, name)
  for ( cmd = mySubmeshes.begin(); cmd != mySubmeshes.end(); ++cmd ) {
    Handle(_pyCommand) subCmd = *cmd;
    if ( subCmd->GetNbArgs() > 0 )
      AddMeshAccess( subCmd );
  }
  myAddHypCmds.clear();
  mySubmeshes.clear();

  // flush hypotheses
  list< Handle(_pyHypothesis) >::iterator hyp = myHypos.begin();
  for ( ; hyp != myHypos.end(); ++hyp )
    (*hyp)->Flush();
}

//================================================================================
/*!
 * \brief _pyHypothesis constructor
  * \param theCreationCmd - 
 */
//================================================================================

_pyHypothesis::_pyHypothesis(const Handle(_pyCommand)& theCreationCmd):
  _pyObject( theCreationCmd )
{
  myDim = myIsAlgo = /*myIsLocal = */myIsWrapped = myIsConverted = false;
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

  // 1D Regular_1D ----------
  if ( hypType == "Regular_1D" ) {
    algo->SetDimMethodType( 1, "Segment");
  }
  else if ( hypType == "LocalLength" ) {
    hyp->SetDimMethodType( 1, "LocalLength", "Regular_1D");
    hyp->AddArgMethod( "SetLength" );
  }
  else if ( hypType == "NumberOfSegments" ) {
    hyp = new _pyNumberOfSegmentsHyp( theCreationCmd );
    hyp->SetDimMethodType( 1, "NumberOfSegments", "Regular_1D");
    hyp->AddArgMethod( "SetNumberOfSegments" );
    hyp->AddArgMethod( "SetScaleFactor" );
  }
  else if ( hypType == "Arithmetic1D" ) {
    hyp = new _pyComplexParamHypo( theCreationCmd );
    hyp->SetDimMethodType( 1, "Arithmetic1D", "Regular_1D");
  }
  else if ( hypType == "StartEndLength" ) {
    hyp = new _pyComplexParamHypo( theCreationCmd );
    hyp->SetDimMethodType( 1, "StartEndLength", "Regular_1D");
  }
  else if ( hypType == "Deflection1D" ) {
    hyp->SetDimMethodType( 1, "Deflection1D", "Regular_1D");
    hyp->AddArgMethod( "SetDeflection" );
  }
  else if ( hypType == "Propagation" ) {
    hyp->SetDimMethodType( 1, "Propagation", "Regular_1D");
  }
  else if ( hypType == "QuadraticMesh" ) {
    hyp->SetDimMethodType( 1, "QuadraticMesh", "Regular_1D");
  }
  else if ( hypType == "AutomaticLength" ) {
    hyp->SetDimMethodType( 1, "AutomaticLength", "Regular_1D");
    hyp->AddArgMethod( "SetFineness");
  }
  // 1D Python_1D ----------
  else if ( hypType == "Python_1D" ) {
    algo->SetDimMethodType( 1, "Segment");
    algo->myArgs.Append( "algo=smesh.PYTHON");
  }
  else if ( hypType == "PythonSplit1D" ) {
    hyp->SetDimMethodType( 1, "PythonSplit1D", "Python_1D");
    hyp->AddArgMethod( "SetNumberOfSegments");
    hyp->AddArgMethod( "SetPythonLog10RatioFunction");
  }
  // 2D ----------
  else if ( hypType == "MEFISTO_2D" ) {
    algo->SetDimMethodType( 2, "Triangle");
  }
  else if ( hypType == "MaxElementArea" ) {
    hyp->SetDimMethodType( 2, "MaxElementArea", "MEFISTO_2D");
    hyp->AddArgMethod( "SetMaxElementArea");
  }
  else if ( hypType == "LengthFromEdges" ) {
    hyp->SetDimMethodType( 2, "LengthFromEdges", "MEFISTO_2D");
  }
  else if ( hypType == "Quadrangle_2D" ) {
    algo->SetDimMethodType( 2, "Quadrangle" );
  }
  else if ( hypType == "QuadranglePreference" ) {
    hyp->SetDimMethodType( 2, "QuadranglePreference", "Quadrangle_2D");
  }
  // 3D ----------
  else if ( hypType == "NETGEN_3D") {
    algo->SetDimMethodType( 3, "Tetrahedron" );
    algo->myArgs.Append( "algo=smesh.NETGEN" );
  }
  else if ( hypType == "MaxElementVolume") {
    hyp->SetDimMethodType( 3, "MaxElementVolume", "NETGEN_3D");
    hyp->AddArgMethod( "SetMaxElementVolume" );
  }
  else if ( hypType == "GHS3D_3D" ) {
    algo->SetDimMethodType( 3, "Tetrahedron");
    algo->myArgs.Append( "algo=smesh.GHS3D" );
  }
  else if ( hypType == "Hexa_3D" ) {
    algo->SetDimMethodType( 3, "Hexahedron");
  }
  // Repetitive ---------
  else if ( hypType == "Projection_1D" ) {
    algo->SetDimMethodType( 1, "Projection1D");
  }
  else if ( hypType == "ProjectionSource1D" ) {
    hyp->SetDimMethodType( 1, "SourceEdge", "Projection_1D");
    hyp->AddArgMethod( "SetSourceEdge");
    hyp->AddArgMethod( "SetSourceMesh");
    hyp->AddArgMethod( "SetVertexAssociation", 2 );
  }
  else if ( hypType == "Projection_2D" ) {
    algo->SetDimMethodType( 2, "Projection2D");
  }
  else if ( hypType == "ProjectionSource2D" ) {
    hyp->SetDimMethodType( 2, "SourceFace", "Projection_2D");
    hyp->AddArgMethod( "SetSourceFace");
    hyp->AddArgMethod( "SetSourceMesh");
    hyp->AddArgMethod( "SetVertexAssociation", 4 );
  }
  else if ( hypType == "Projection_3D" ) {
    algo->SetDimMethodType( 3, "Projection3D");
  }
  else if ( hypType == "ProjectionSource3D" ) {
    hyp->SetDimMethodType( 3, "SourceShape3D", "Projection_3D");
    hyp->AddArgMethod( "SetSource3DShape");
    hyp->AddArgMethod( "SetSourceMesh");
    hyp->AddArgMethod( "SetVertexAssociation", 4 );
  }
  else if ( hypType == "Prism_3D" ) {
    algo->SetDimMethodType( 3, "Prism");
  }
  else if ( hypType == "RadialPrism_3D" ) {
    algo->SetDimMethodType( 3, "Prism");
  }
  else if ( hypType == "NumberOfLayers" ) {
    hyp->SetDimMethodType( 3, "NumberOfLayers", "RadialPrism_3D");
    hyp->AddArgMethod( "SetNumberOfLayers" );
  }
  else if ( hypType == "LayerDistribution" ) {
    hyp = new _pyLayerDistributionHypo( theCreationCmd );
    hyp->SetDimMethodType( 3, "LayerDistribution", "RadialPrism_3D");
//     hyp->AddArgMethod( "SetSource3DShape");
//     hyp->AddArgMethod( "SetSourceMesh");
//     hyp->AddArgMethod( "SetVertexAssociation", 4 );
  }

  if ( algo->GetDim() ) {
    algo->myType = hypType;
    return algo;
  }
  return hyp;
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

  myIsWrapped = true;

  if ( myIsWrapped )
  {
    // mesh.AddHypothesis(geom,hyp) --> hyp = theMesh.myCreationMethod(args)
    theCmd->SetResultValue( GetID() );
    theCmd->SetObject( theMesh );
    theCmd->SetMethod( myCreationMethod );
    // set args
    theCmd->RemoveArgs();
    for ( int i = 1; i <= myArgs.Length(); ++i ) {
      if ( !myArgs( i ).IsEmpty() )
        theCmd->SetArg( i, myArgs( i ));
      else
        theCmd->SetArg( i, "[]");
    }
    // set a new creation command
    GetCreationCmd()->Clear();
    SetCreationCmd( theCmd );

    // clear commands setting arg values
    list < Handle(_pyCommand) >::iterator argCmd = myArgCommands.begin();
    for ( ; argCmd != myArgCommands.end(); ++argCmd )
      (*argCmd)->Clear();
  }
  else
  {
//     // set arg commands after hypo creation
//     list<Handle(_pyCommand)>::iterator argCmd = myArgCommands.begin();
//     for ( ; argCmd != myArgCommands.end(); ++argCmd )
//       if ( !(*argCmd)->IsEmpty() && GetCommandNb() > (*argCmd)->GetOrderNb() )
//         theGen->ExchangeCommands( GetCreationCmd(), *argCmd );
  }

  // set unknown arg commands after hypo creation
  Handle(_pyCommand) afterCmd = myIsWrapped ? theCmd : GetCreationCmd();
  list<Handle(_pyCommand)>::iterator cmd = myUnknownCommands.begin();
  for ( ; cmd != myUnknownCommands.end(); ++cmd ) {
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
  // set args
  int nbArgs = 0;
  for ( int i = 1; i <= myArgMethods.Length(); ++i ) {
    if ( myArgMethods( i ) == theCommand->GetMethod() ) {
      while ( myArgs.Length() < nbArgs + myNbArgsByMethod( i ))
        myArgs.Append( "[]" );
      for ( int iArg = 1; iArg <= myNbArgsByMethod( i ); ++iArg )
        myArgs( nbArgs + iArg ) = theCommand->GetArg( iArg ); // arg value
      myArgCommands.push_back( theCommand );
      return;
    }
    nbArgs += myNbArgsByMethod( i );
  }
  myUnknownCommands.push_back( theCommand );
}

//================================================================================
/*!
 * \brief Finish conversion
 */
//================================================================================

void _pyHypothesis::Flush()
{
  if ( IsWrapped() ) {
  }
  else {
    list < Handle(_pyCommand) >::iterator cmd = myArgCommands.begin();
    for ( ; cmd != myArgCommands.end(); ++cmd ) {
      // Add access to a wrapped mesh
      theGen->AddMeshAccessorMethod( *cmd );
      // Add access to a wrapped algorithm
      theGen->AddAlgoAccessorMethod( *cmd );
    }
    cmd = myUnknownCommands.begin();
    for ( ; cmd != myUnknownCommands.end(); ++cmd ) {
      // Add access to a wrapped mesh
      theGen->AddMeshAccessorMethod( *cmd );
      // Add access to a wrapped algorithm
      theGen->AddAlgoAccessorMethod( *cmd );
    }
  }
  // forget previous hypothesis modifications
  myArgCommands.clear();
  myUnknownCommands.clear();
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
  cmd = myUnknownCommands.begin();
  for ( ; cmd != myUnknownCommands.end(); ++cmd )
    ( *cmd )->Clear();
}

//================================================================================
/*!
 * \brief Remember hypothesis parameter values
  * \param theCommand - The called hypothesis method
 */
//================================================================================

void _pyComplexParamHypo::Process( const Handle(_pyCommand)& theCommand)
{
  // ex: hyp.SetLength(start, 1)
  //     hyp.SetLength(end,   0)
  ASSERT(( theCommand->GetMethod() == "SetLength" ));
  ASSERT(( theCommand->GetArg( 2 ).IsIntegerValue() ));
  int i = 2 - theCommand->GetArg( 2 ).IntegerValue();
  while ( myArgs.Length() < i )
    myArgs.Append( "[]" );
  myArgs( i ) = theCommand->GetArg( 1 ); // arg value
  myArgCommands.push_back( theCommand );
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

  _pyID newName; // name for 1D hyp = "HypType" + "_Distribution"

  const _pyID& hyp1dID = theCommand->GetArg( 1 );
  Handle(_pyHypothesis) hyp1d = theGen->FindHyp( hyp1dID );
  if ( hyp1d.IsNull() ) // apparently hypId changed at study restoration
    hyp1d = my1dHyp;
  else if ( !my1dHyp.IsNull() && hyp1dID != my1dHyp->GetID() ) {
    // 1D hypo is already set, so distribution changes and the old
    // 1D hypo is thrown away
    my1dHyp->ClearAllCommands();
  }
  my1dHyp = hyp1d;
  if ( my1dHyp.IsNull() )
    return; // something wrong :(

  // make a new name for 1D hyp = "HypType" + "_Distribution"
  if ( my1dHyp->GetCreationCmd()->GetMethod() == "CreateHypothesis" ) {
    // not yet converted creation cmd
    TCollection_AsciiString hypTypeQuoted = my1dHyp->GetCreationCmd()->GetArg(1);
    TCollection_AsciiString hypType = hypTypeQuoted.SubString( 2, hypTypeQuoted.Length() - 1 );
    newName = hypType + "_Distribution";
    my1dHyp->GetCreationCmd()->SetResultValue( newName );
  }
  else {
    // already converted creation cmd
    newName = my1dHyp->GetCreationCmd()->GetResultValue();
  }

  // as creation of 1D hyp was written later then it's edition,
  // we need to find all it's edition calls and process them
  list< Handle(_pyCommand) >& cmds = theGen->GetCommands();
  list< Handle(_pyCommand) >::iterator cmdIt = cmds.begin();
  for ( ; cmdIt != cmds.end(); ++cmdIt ) {
    const _pyID& objID = (*cmdIt)->GetObject();
    if ( objID == hyp1dID ) {
      my1dHyp->Process( *cmdIt );
      my1dHyp->GetCreationCmd()->AddDependantCmd( *cmdIt );
      ( *cmdIt )->SetObject( newName );
    }
  }
  if ( !myArgCommands.empty() )
    myArgCommands.front()->Clear();
  theCommand->SetArg( 1, newName );
  myArgCommands.push_back( theCommand );
  // copy hyp1d's creation method and args
//   myCreationMethod = hyp1d->GetCreationMethod();
//   myArgs           = hyp1d->GetArgs();
//   // make them cleared at conversion
//   myArgCommands = hyp1d->GetArgCommands();

//   // to be cleared at convertion only
//   myArgCommands.push_back( theCommand );
}

//================================================================================
/*!
 * \brief 
  * \param theAdditionCmd - 
  * \param theMesh - 
  * \retval bool - 
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

  my1dHyp->SetMesh( theMesh );
  if ( !my1dHyp->Addition2Creation( theAdditionCmd, theMesh ))
    return false;

  // clear "SetLayerDistribution()" cmd
  myArgCommands.front()->Clear();

  // Convert my creation => me = RadialPrismAlgo.Get3DHypothesis()

  // find RadialPrism algo created on <geom> for theMesh
  Handle(_pyHypothesis) algo = theGen->FindAlgo( geom, theMesh, this->GetType() );
  if ( !algo.IsNull() ) {
    GetCreationCmd()->SetObject( algo->GetID() );
    GetCreationCmd()->SetMethod( "Get3DHypothesis" );
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
  //my1dHyp.Nullify();
  //_pyHypothesis::Flush();
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
  if ( IsWrappable( theMesh ) && myArgs.Length() > 1 ) {
    // scale factor (2-nd arg) is provided: clear SetDistrType(1) command
    bool scaleDistrType = false;
    list<Handle(_pyCommand)>::reverse_iterator cmd = myUnknownCommands.rbegin();
    for ( ; cmd != myUnknownCommands.rend(); ++cmd ) {
      if ( (*cmd)->GetMethod() == "SetDistrType" ) {
        if ( (*cmd)->GetArg( 1 ) == "1" ) {
          scaleDistrType = true;
          (*cmd)->Clear();
        }
        else if ( !scaleDistrType ) {
          // distribution type changed: remove scale factor from args
          myArgs.Remove( 2, myArgs.Length() );
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
  list<Handle(_pyCommand)>::reverse_iterator cmd = myUnknownCommands.rbegin();
  int distrTypeNb = 0;
  for ( ; !distrTypeNb && cmd != myUnknownCommands.rend(); ++cmd )
    if ( (*cmd)->GetMethod() == "SetDistrType" )
      distrTypeNb = (*cmd)->GetOrderNb();

  // clear commands before the last SetDistrType()
  list<Handle(_pyCommand)> * cmds[2] = { &myArgCommands, &myUnknownCommands };
  for ( int i = 0; i < 2; ++i ) {
    set<TCollection_AsciiString> uniqueMethods;
    list<Handle(_pyCommand)> & cmdList = *cmds[i];
    for ( cmd = cmdList.rbegin(); cmd != cmdList.rend(); ++cmd )
    {
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
  if ( IsWrappable( theMeshID )) {

    myGeom = theCmd->GetArg( 1 );

    // mesh.AddHypothesis(geom,algo) --> theMeshID.myCreationMethod()
    if ( _pyHypothesis::Addition2Creation( theCmd, theMeshID )) {
      theGen->SetAccessorMethod( GetID(), "GetAlgorithm()" );
      return true;
    }
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

int _pyCommand::GetBegPos( int thePartIndex )
{
  if ( IsEmpty() )
    return EMPTY;
  if ( myBegPos.Length() < thePartIndex )
    return UNKNOWN;
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
  myBegPos( thePartIndex ) = thePosition;
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
    int begPos = myString.Location( "=", 1, Length() );
    if ( begPos )
      myRes = GetWord( myString, begPos, false );
    else
      begPos = EMPTY;
    SetBegPos( RESULT_IND, begPos );
  }
  return myRes;
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
    int begPos = GetBegPos( RESULT_IND ) + myRes.Length();
    if ( begPos < 1 )
      begPos = myString.Location( "=", 1, Length() ) + 1;
    // store
    myObj = GetWord( myString, begPos, true );
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
    int begPos = GetBegPos( OBJECT_IND ) + myObj.Length();
    bool forward = true;
    if ( begPos < 1 ) {
      begPos = myString.Location( "(", 1, Length() ) - 1;
      forward = false;
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
 * \brief Return substring of python command looking like ResVal = Obj.Meth(Arg1,...)
  * \retval const TCollection_AsciiString & - Arg<index> substring
 */
//================================================================================

const TCollection_AsciiString & _pyCommand::GetArg( int index )
{
  if ( GetBegPos( ARG1_IND ) == UNKNOWN )
  {
    // find all args
    int begPos = GetBegPos( METHOD_IND ) + myMeth.Length();
    if ( begPos < 1 )
      begPos = myString.Location( "(", 1, Length() ) + 1;

    int i = 0, prevLen = 0;
    while ( begPos != EMPTY ) {
      begPos += prevLen;
      // check if we are looking at the closing parenthesis
      while ( begPos <= Length() && isspace( myString.Value( begPos )))
        ++begPos;
      if ( begPos > Length() || myString.Value( begPos ) == ')' )
        break;
      myArgs.Append( GetWord( myString, begPos, true, true ));
      SetBegPos( ARG1_IND + i, begPos );
      prevLen = myArgs.Last().Length();
      if ( prevLen == 0 )
        myArgs.Remove( myArgs.Length() ); // no more args
      i++;
    }
  }
  if ( myArgs.Length() < index )
    return theEmptyString;
  return myArgs( index );
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

TCollection_AsciiString _pyCommand::GetWord( const TCollection_AsciiString & theString,
                                            int &      theStartPos,
                                            const bool theForward,
                                            const bool dotIsWord )
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
    while ( end <= theString.Length() && isWord( theString.Value( end ), dotIsWord))
      ++end;
    --end;
  }
  else {  // search backward
    // end
    while ( end > 0 && !isWord( theString.Value( end ), dotIsWord))
      --end;
    if ( end == 0 )
      return theEmptyString; // no word found
    beg = end - 1;
    while ( beg > 0 && isWord( theString.Value( beg ), dotIsWord))
      --beg;
    ++beg;
  }
  theStartPos = beg;
  //cout << theString << " ---- " << beg << " - " << end << endl;
  return theString.SubString( beg, end );
}

//================================================================================
/*!
 * \brief Look for position where not space char is
  * \param theString - The string 
  * \param thePos - The position to search from and which returns result
  * \retval bool - false if there are only space after thePos in theString
 * 
 * 
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
    if ( int lastArgInd = GetNbArgs() ) {
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
  if ( int pos = myString.Location( '(', 1, Length() ))
    myString.Trunc( pos );
  myString += ")";
  myArgs.Clear();
  if ( myBegPos.Length() >= ARG1_IND )
    myBegPos.Remove( ARG1_IND, myBegPos.Length() );
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
  while (( beg = myString.Location( theObjectID, beg, Length() )))
  {
    // check that theObjectID is not just a part of a longer ID
    int afterEnd = beg + theObjectID.Length();
    Standard_Character c = myString.Value( afterEnd );
    if ( !isalnum( c ) && c != ':' ) {
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
      return true;
    }
    beg = afterEnd; // is a part - next search
  }
  return false;
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
