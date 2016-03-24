// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File      : SMESH_NoteBook.cxx
// Author    : Roman NIKOLAEV
//
#include "SMESH_2smeshpy.hxx"
#include "SMESH_NoteBook.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_Hypothesis_i.hxx"

#include <Resource_DataMapOfAsciiStringAsciiString.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TColStd_HSequenceOfInteger.hxx>

#include <SALOMEDS_wrap.hxx>
#include <SALOMEDS_Attributes_wrap.hxx>

#include <vector>
#include <string>

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;


namespace
{
  /*!
   *  Set variable of the SMESH_ObjectStates from position to the _pyCommand
   *  method as nbArg argument
   */
  void SetVariable(Handle(_pyCommand) theCommand,
                   const SMESH_ObjectStates* theStates,
                   size_t position, int theArgNb)
  {
    if(theStates->GetCurrectState().size() > position)
      if(!theStates->GetCurrectState().at(position).IsEmpty())
        theCommand->SetArg(theArgNb,theStates->GetCurrectState().at(position));
  }
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================
SMESH_ObjectStates::SMESH_ObjectStates(TCollection_AsciiString theType)
{
  _type = theType;
  _dumpstate = 0;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================
SMESH_ObjectStates::~SMESH_ObjectStates()
{
}

//================================================================================
/*!
 * \brief Add new object state 
 * \param theState - Object state (vector of notebook variable)
 */
//================================================================================
void SMESH_ObjectStates::AddState(const TState &theState)
{
  _states.push_back(theState);
}

//================================================================================
/*!
 * \brief Return current object state
 * \\retval state - Object state (vector of notebook variable)
 */
//================================================================================
TState SMESH_ObjectStates::GetCurrectState() const
{
  if ( (int) _states.size() > _dumpstate)
    return _states[_dumpstate];
  TState empty;
  return empty;
}


//================================================================================
/*!
 *
 */
//================================================================================
TAllStates SMESH_ObjectStates::GetAllStates() const
{
  return _states;
}

//================================================================================
/*!
 *
 */
//================================================================================
void SMESH_ObjectStates::IncrementState()
{
  _dumpstate++;
}

//================================================================================
/*!
 *
 */
//================================================================================
TCollection_AsciiString SMESH_ObjectStates::GetObjectType() const{
  return _type;
}


//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================
LayerDistributionStates::LayerDistributionStates():
  SMESH_ObjectStates("LayerDistribution")
{
}
//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================
LayerDistributionStates::~LayerDistributionStates()
{
}


//================================================================================
/*!
 * \brief AddDistribution
 */
//================================================================================
void LayerDistributionStates::AddDistribution(const TCollection_AsciiString& theDistribution)
{
  _distributions.insert(pair<TCollection_AsciiString,TCollection_AsciiString>(theDistribution,""));
}

//================================================================================
/*!
 * \brief HasDistribution
 */
//================================================================================
bool LayerDistributionStates::HasDistribution(const TCollection_AsciiString& theDistribution) const
{
  return _distributions.find(theDistribution) != _distributions.end();
}

//================================================================================
/*!
 * \brief SetDistributionType
 */
//================================================================================
bool LayerDistributionStates::SetDistributionType(const TCollection_AsciiString& theDistribution,
                                                  const TCollection_AsciiString& theType)
{
  TDistributionMap::iterator it = _distributions.find(theDistribution);
  if(it == _distributions.end())
    return false;
  (*it).second = theType;
  return true;
}

//================================================================================
/*!
 * \brief GetDistributionType
 */
//================================================================================
TCollection_AsciiString LayerDistributionStates::
GetDistributionType(const TCollection_AsciiString& theDistribution) const
{
  TDistributionMap::const_iterator it = _distributions.find(theDistribution);
  return (it == _distributions.end()) ? TCollection_AsciiString() : (*it).second;
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================
SMESH_NoteBook::SMESH_NoteBook()
{
  InitObjectMap();
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================
SMESH_NoteBook::~SMESH_NoteBook()
{
  TVariablesMap::const_iterator it = _objectMap.begin();
  for(;it!=_objectMap.end();it++) {
    if((*it).second)
      delete (*it).second;
  }
}

//================================================================================
/*!
 * \brief Replace parameters of the functions on the Salome NoteBook Variables
 * \param theString - Input string
 * \retval TCollection_AsciiString - Convertion result
 */
//================================================================================
void SMESH_NoteBook::ReplaceVariables()
{
  for ( size_t i = 0 ; i < _commands.size(); i++ )
  {
    Handle(_pyCommand) aCmd = _commands[i];
    TCollection_AsciiString aMethod      = aCmd->GetMethod();
    TCollection_AsciiString aObject      = aCmd->GetObject();
    TCollection_AsciiString aResultValue = aCmd->GetResultValue();
    if(MYDEBUG) {
      cout<<"Command before : "<< aCmd->GetString()<<endl;
      cout<<"Method : "<< aMethod<<endl;
      cout<<"Object : "<< aObject<<endl;
      cout<<"Result : "<< aResultValue<<endl;
    }

    // NEW APPROACH
    // Names of variables are stored in the Study, in "StringAttribute". Python commands
    // store zero-based indices (as e.g.'$1$') of variables within "StringAttribute";
    // An entry of object storing "StringAttribute" is at the end of the command
    // after TVar::ObjPrefix().

    // Get the entry of object storing "StringAttribute"
    TCollection_AsciiString & cmdStr = aCmd->GetString();
    TEntry2VarVecMap::iterator ent2varVec;
    Standard_Integer fromIndex = 6;
    Standard_Integer cmdLen = cmdStr.Length();
    if ( int pos = (fromIndex <= cmdLen) ? cmdStr.Location( SMESH::TVar::ObjPrefix(), fromIndex, cmdLen ) : 0 )
    {
      TCollection_AsciiString varHolderEntry =
        cmdStr.SubString( pos + strlen( SMESH::TVar::ObjPrefix() ), cmdLen );
      ent2varVec = _entry2VarsMap.find( varHolderEntry );
      cmdStr.Split( pos - 1 );
    }
    else
    {
      ent2varVec = _entry2VarsMap.find( aObject );
    }
    // Set variables in cmdStr
    if ( ent2varVec != _entry2VarsMap.end() && !ent2varVec->second.empty() )
    {
      const std::vector< std::string >& vars = ent2varVec->second;
      int pos = 1, pos2;
      // look for '$VarIndex$' in cmdStr. TVar::Quote() == '$'
      while (( pos  = cmdStr.Location( 1, SMESH::TVar::Quote(), pos,   cmdStr.Length() )) &&
             ( pos2 = cmdStr.Location( 1, SMESH::TVar::Quote(), pos+1, cmdStr.Length() )) )
      {
        size_t varIndex = std::string::npos;
        const char* varIndexPtr = cmdStr.ToCString() + pos;
        if ( '0' <= *varIndexPtr && *varIndexPtr <= '9' )
          varIndex = atoi( varIndexPtr );
        if ( 0 <= varIndex && varIndex < vars.size() && !vars[varIndex].empty() )
        {
          // replace '$VarIndex$' either by var name of var value
          const char var0    = vars[varIndex][0];
          const bool isValue = (( '0' <= var0 && var0 <= '9' ) || var0 == '-');
          if ( isValue ) // remove TVar::Quote() as well
            pos2 += 2; // pos still points to '$'
          int indexLen = pos2 - pos - 1;
          int  lenDiff = int( vars[varIndex].size() ) - indexLen;
          if      ( lenDiff > 0 )
            cmdStr.InsertBefore( pos2, vars[varIndex].c_str() + vars[varIndex].size() - lenDiff );
          else if ( lenDiff < 0 )
            cmdStr.Remove( pos+1, -lenDiff );
          cmdStr.SetValue( pos+(!isValue), vars[varIndex].c_str() );
        }
        pos = pos2 + 1;
        if ( pos + 2 >= cmdStr.Length() )
          break;
      }
    }

    // OLD APPROACH
    // Variable names are stored historically in "StringAttribute",
    // i.e. for each command there is a set of either var names or separated empty places.

    // check if method modifies the object itself
    TVariablesMap::const_iterator it = _objectMap.find(aObject);
    if(it == _objectMap.end()) // check if method returns a new object
      it = _objectMap.find(aResultValue);
    
    if(it == _objectMap.end()) { // check if method modifies a mesh using mesh editor
      TMeshEditorMap::const_iterator meIt = _meshEditors.find(aObject);
      if(meIt != _meshEditors.end()) {
        TCollection_AsciiString aMesh = (*meIt).second;
        it = _objectMap.find(aMesh);
      }
    }
    
    if(it == _objectMap.end()) { // additional check for pattern mapping
      if(aMethod.IsEqual("ApplyToMeshFaces") ||
         aMethod.IsEqual("ApplyToHexahedrons"))
        it = _objectMap.find(aCmd->GetArg(1));
    }

    if(it != _objectMap.end()) {
      if(MYDEBUG)
        cout << "Found object : " << (*it).first << endl;
      SMESH_ObjectStates *aStates = (*it).second;
      // Case for LocalLength hypothesis
      if(aStates->GetObjectType().IsEqual("LocalLength") && aStates->GetCurrectState().size() >= 2)
      {
        if(aMethod.IsEqual("SetLength")) {
          if(!aStates->GetCurrectState().at(0).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("SetPrecision")) {
          if(!aStates->GetCurrectState().at(1).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(1));
          aStates->IncrementState();
        }
      }
      
      // Case for SegmentLengthAroundVertex hypothesis
      else if(aStates->GetObjectType().IsEqual("SegmentLengthAroundVertex")
              && aStates->GetCurrectState().size() >= 1) {
        if(aMethod == "SetLength") {
          if(!aStates->GetCurrectState().at(0).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          aStates->IncrementState();
        }
      }

      // Case for Arithmetic1D and StartEndLength hypothesis
      else if(aStates->GetObjectType().IsEqual("Arithmetic1D") || 
              aStates->GetObjectType().IsEqual("StartEndLength")) {
        if(aMethod == "SetLength" &&
           aStates->GetCurrectState().size() >= 2) {
          if(aCmd->GetArg(2) == "1" && !aStates->GetCurrectState().at(0).IsEmpty())
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          else if(!aStates->GetCurrectState().at(1).IsEmpty())
            aCmd->SetArg(1,aStates->GetCurrectState().at(1));
          aStates->IncrementState();
        }
      }
      
      //Case for Deflection1D hypothesis
      else if(aStates->GetObjectType().IsEqual("Deflection1D")){
        if(aMethod == "SetDeflection" && aStates->GetCurrectState().size() >= 1) {
          if(!aStates->GetCurrectState().at(0).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          aStates->IncrementState();
        }
      }
      
      // Case for LayerDistribution hypothesis (not finished yet)
      else if(aStates->GetObjectType() == "LayerDistribution") {
        if(aMethod == "SetLayerDistribution"){
          LayerDistributionStates* aLDStates = (LayerDistributionStates*)(aStates);
          aLDStates->AddDistribution(aCmd->GetArg(1));
          if(MYDEBUG)
            cout<<"Add Distribution :"<<aCmd->GetArg(1)<<endl;
        }
      }
      
      // Case for MaxElementArea hypothesis
      else if(aStates->GetObjectType().IsEqual("MaxElementArea")){
        if(aMethod == "SetMaxElementArea" && aStates->GetCurrectState().size() >= 1) {
          if(!aStates->GetCurrectState().at(0).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          aStates->IncrementState();
        }
      }

      // Case for MaxElementVolume hypothesis
      else if(aStates->GetObjectType().IsEqual("MaxElementVolume")){
        if(aMethod == "SetMaxElementVolume" && aStates->GetCurrectState().size() >= 1) {
          if(!aStates->GetCurrectState().at(0).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          aStates->IncrementState();
        }
      }
      // Case for NumberOfLayers hypothesis
      else if(aStates->GetObjectType().IsEqual("NumberOfLayers")){
        if(aMethod == "SetNumberOfLayers" && aStates->GetCurrectState().size() >= 1) {
          if(!aStates->GetCurrectState().at(0).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          aStates->IncrementState();
        }
      }

      // Case for NumberOfSegments hypothesis
      else if(aStates->GetObjectType().IsEqual("NumberOfSegments")){
        if(aMethod == "SetNumberOfSegments" && aStates->GetCurrectState().size() >= 1) {
          if(!aStates->GetCurrectState().at(0).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(0));
          if(aStates->GetCurrectState().size()==1)
            aStates->IncrementState();
        }
        else if (aMethod == "SetScaleFactor" && aStates->GetCurrectState().size() >= 2) {
          if(!aStates->GetCurrectState().at(1).IsEmpty() )
            aCmd->SetArg(1,aStates->GetCurrectState().at(1));
          aStates->IncrementState();
        }
      }
      
      else if(aStates->GetObjectType().IsEqual("Mesh")) {
        TState aCurrentState = aStates->GetCurrectState();
        int aCurrentStateSize = aCurrentState.size();
        if(aMethod.IsEqual("Translate")                  ||
           aMethod.IsEqual("TranslateMakeGroups")        ||
           aMethod.IsEqual("TranslateMakeMesh")          ||
           aMethod.IsEqual("TranslateObject")            ||
           aMethod.IsEqual("TranslateObjectMakeGroups")  ||
           aMethod.IsEqual("TranslateObjectMakeMesh")) {
          bool isVariableFound = false;
          int anArgIndex = 0;
          for(int i = 1, n = aCmd->GetNbArgs(); i <= n; i++) {
            if(aCmd->GetArg(i).IsEqual("SMESH.PointStruct")) {
              anArgIndex = i+1;
              break;
            }
          }
          if(anArgIndex > 0) {
            if(aCurrentStateSize == 3) { // translation by dx, dy, dz
              for(int j = 0; j < aCurrentStateSize; j++) {
                if(!aCurrentState.at(j).IsEmpty()) {
                  isVariableFound = true;
                  aCmd->SetArg(anArgIndex+j, aCurrentState.at(j));
                }
              }
            }
            else if(aCurrentStateSize == 6) { // translation by x1, x2, y1, y2, z1, z2
              // TODO: limitation until operations on the variables will be introduced
              /*
              isVariableFound = true;
              for(int j = 0; j < 3; j++) {
                TCollection_AsciiString anArg = aCmd->GetArg(anArgIndex+j);
                TCollection_AsciiString aValue1 = aCurrentState.at(2*j), aValue2 = aCurrentState.at(2*j+1);
                bool aV1 = !aValue1.IsEmpty();
                bool aV2 = !aValue2.IsEmpty();
                double aValue, aCurrentValue = anArg.IsRealValue() ? anArg.RealValue() : 0;
                if(aV1 && !aV2) {
                  if(!GetReal(aValue1, aValue))
                    aValue = 0;
                  aValue2 = TCollection_AsciiString( aValue + aCurrentValue );
                }
                else if(!aV1 && aV2) {
                  if(!GetReal(aValue2, aValue))
                    aValue = 0;
                  aValue1 = TCollection_AsciiString( aValue - aCurrentValue );
                }
                else if(!aV1 && !aV2) {
                  aValue1 = TCollection_AsciiString( 0 );
                  aValue2 = TCollection_AsciiString( aCurrentValue );
                }
                aCmd->SetArg(anArgIndex+j, aValue1 + ", " + aValue2 );
              }
              */
            }
          }
          if(isVariableFound) {
            TCollection_AsciiString aDim;
            if(aCurrentStateSize == 6)
              aDim = "6";
            aCmd->SetArg(anArgIndex - 1, TCollection_AsciiString(SMESH_2smeshpy::SmeshpyName())+".PointStructStr"+aDim);
            aCmd->SetArg(anArgIndex - 2, TCollection_AsciiString(SMESH_2smeshpy::SmeshpyName())+".DirStructStr");
          }
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("Rotate")                  ||
                aMethod.IsEqual("RotateMakeGroups")        ||
                aMethod.IsEqual("RotateMakeMesh")          ||
                aMethod.IsEqual("RotateObject")            ||
                aMethod.IsEqual("RotateObjectMakeGroups")  ||
                aMethod.IsEqual("RotateObjectMakeMesh")    ||
                aMethod.IsEqual("RotationSweep")           ||
                aMethod.IsEqual("RotationSweepObject")     ||
                aMethod.IsEqual("RotationSweepObject1D")   ||
                aMethod.IsEqual("RotationSweepObject2D")   ||
                aMethod.IsEqual("RotationSweepMakeGroups") ||
                aMethod.IsEqual("RotationSweepObjectMakeGroups") ||
                aMethod.IsEqual("RotationSweepObject1DMakeGroups") ||
                aMethod.IsEqual("RotationSweepObject2DMakeGroups") ||
                aMethod.IsEqual("Mirror")                  ||
                aMethod.IsEqual("MirrorMakeMesh")          ||
                aMethod.IsEqual("MirrorMakeGroups")        ||
                aMethod.IsEqual("MirrorObject")            || 
                aMethod.IsEqual("MirrorObjectMakeMesh")    ||
                aMethod.IsEqual("MirrorObjectMakeGroups")) {
          bool isSubstitute = false;
          int anArgIndex = 0;
          for(int i = 1, n = aCmd->GetNbArgs(); i <= n; i++) {
            if(aCmd->GetArg(i).IsEqual("SMESH.AxisStruct")) {
              anArgIndex = i+1;
              break;
            }
          }
          if(anArgIndex > 0) {
            for(int j = 0; j < aCurrentStateSize; j++) {
              if(!aCurrentState.at(j).IsEmpty()) {
                if(j < 6) // 0-5 - axis struct, 6 - angle (rotation & sweep), 7-8 - nbSteps and tolerance (sweep)
                  isSubstitute = true;
                aCmd->SetArg(anArgIndex+j, aCurrentState.at(j));
              }
            }
          }
          if(isSubstitute)
            aCmd->SetArg(anArgIndex - 1, TCollection_AsciiString(SMESH_2smeshpy::SmeshpyName())+".AxisStructStr");
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("AddNode") ||
                aMethod.IsEqual("MoveClosestNodeToPoint")) {
          for(int j = 0; j < aCurrentStateSize; j++) {
            if(!aCurrentState.at(j).IsEmpty())
              aCmd->SetArg(j+1, aCurrentState.at(j));
          }
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("MoveNode")) {
          for(int j = 0; j < aCurrentStateSize; j++) {
            if(!aCurrentState.at(j).IsEmpty())
              aCmd->SetArg(j+2, aCurrentState.at(j));
          }
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("ExtrusionSweep") ||
                aMethod.IsEqual("ExtrusionSweepObject") ||
                aMethod.IsEqual("ExtrusionSweepObject1D") ||
                aMethod.IsEqual("ExtrusionSweepObject2D") ||
                aMethod.IsEqual("ExtrusionSweepMakeGroups") ||
                aMethod.IsEqual("ExtrusionSweepObjectMakeGroups") ||
                aMethod.IsEqual("ExtrusionSweepObject1DMakeGroups") ||
                aMethod.IsEqual("ExtrusionSweepObject2DMakeGroups")) {
          bool isSubstitute = false;
          int anArgIndex = 0;
          for(int i = 1, n = aCmd->GetNbArgs(); i <= n; i++) {
            if(aCmd->GetArg(i).IsEqual("SMESH.PointStruct")) {
              anArgIndex = i+1;
              break;
            }
          }
          if(anArgIndex > 0) {
            for(int j = 0; j < aCurrentStateSize; j++) {
              if(!aCurrentState.at(j).IsEmpty()) {
                if(j < 3) // 0-2 - dir struct, 3 - number of steps
                  isSubstitute = true;
                aCmd->SetArg(anArgIndex+j, aCurrentState.at(j));
              }
            }
          }
          if(isSubstitute) {
            aCmd->SetArg(anArgIndex - 1, TCollection_AsciiString(SMESH_2smeshpy::SmeshpyName())+".PointStructStr");
            aCmd->SetArg(anArgIndex - 2, TCollection_AsciiString(SMESH_2smeshpy::SmeshpyName())+".DirStructStr");
          }
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("ExtrusionAlongPath") ||
                aMethod.IsEqual("ExtrusionAlongPathObject") ||
                aMethod.IsEqual("ExtrusionAlongPathObject1D") ||
                aMethod.IsEqual("ExtrusionAlongPathObject2D") ||
                aMethod.IsEqual("ExtrusionAlongPathMakeGroups") ||
                aMethod.IsEqual("ExtrusionAlongPathObjectMakeGroups") ||
                aMethod.IsEqual("ExtrusionAlongPathObject1DMakeGroups") ||
                aMethod.IsEqual("ExtrusionAlongPathObject2DMakeGroups") ||
                /* workaround for a bug in the command parsing algorithm */
                aCmd->GetString().Search("ExtrusionAlongPathMakeGroups") != -1 ||
                aCmd->GetString().Search("ExtrusionAlongPathObjectMakeGroups") != -1 ||
                aCmd->GetString().Search("ExtrusionAlongPathObject1DMakeGroups") != -1 ||
                aCmd->GetString().Search("ExtrusionAlongPathObject2DMakeGroups") != -1 ) {
          int aNbAngles = aCurrentStateSize-3; // State looks like "Angle1:...:AngleN:X:Y:Z"
          bool isSubstitute = false;
          int anArgIndex = 0;
          for(int i = 1, n = aCmd->GetNbArgs(); i <= n; i++) {
            if(aCmd->GetArg(i).IsEqual("SMESH.PointStruct")) {
              anArgIndex = i-1-aNbAngles;
              break;
            }
          }
          if(anArgIndex > 0) {
            int j = 0;
            for(; j < aNbAngles; j++) {
              if(!aCurrentState.at(j).IsEmpty()) {
                aCmd->SetArg(anArgIndex+j-1, aCurrentState.at(j));
              }
            }
            for(; j < aNbAngles+3; j++) {
              if(!aCurrentState.at(j).IsEmpty()) {
                isSubstitute = true;
                aCmd->SetArg(anArgIndex+j+2, aCurrentState.at(j));
              }
            }
          }
          if(isSubstitute)
            aCmd->SetArg(anArgIndex + aNbAngles + 1,
                         TCollection_AsciiString(SMESH_2smeshpy::SmeshpyName())+".PointStructStr");
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("TriToQuad") ||
                aMethod.IsEqual("Concatenate") ||
                aMethod.IsEqual("ConcatenateWithGroups")) {
          if(aCurrentStateSize && !aCurrentState.at(0).IsEmpty())
            aCmd->SetArg(aCmd->GetNbArgs(), aCurrentState.at(0));
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("Smooth") ||
                aMethod.IsEqual("SmoothObject") ||
                aMethod.IsEqual("SmoothParametric") ||
                aMethod.IsEqual("SmoothParametricObject")) {
          int anArgIndex = aCmd->GetNbArgs() - 2;
          for(int j = 0; j < aCurrentStateSize; j++) {
            if(!aCurrentState.at(j).IsEmpty())
              aCmd->SetArg(anArgIndex+j, aCurrentState.at(j));
          }
          aStates->IncrementState();
        }
        else if(aMethod.IsEqual("ApplyToMeshFaces") ||
                aMethod.IsEqual("ApplyToHexahedrons")) {
          int anArgIndex = aCmd->GetNbArgs()-1;
          for(int j = 0; j < aCurrentStateSize; j++)
            if(!aCurrentState.at(j).IsEmpty())
              aCmd->SetArg(anArgIndex+j, aCurrentState.at(j));
          aStates->IncrementState();
        }
      } // if ( aStates->GetObjectType().IsEqual("Mesh"))

      // Case for NETGEN_Parameters_2D or NETGEN_Parameters_2D hypothesis
      // else if(aStates->GetObjectType().IsEqual("NETGEN_Parameters_2D") ||
      //         aStates->GetObjectType().IsEqual("NETGEN_Parameters")){
      //   if(aMethod == "SetMaxSize" && aStates->GetCurrectState().size() >= 1) {
      //     if(!aStates->GetCurrectState().at(0).IsEmpty() )
      //       aCmd->SetArg(1,aStates->GetCurrectState().at(0));
      //     aStates->IncrementState();
      //   }
      //   else if(aMethod == "SetGrowthRate" && aStates->GetCurrectState().size() >= 2) {
      //     if(!aStates->GetCurrectState().at(1).IsEmpty() )
      //       aCmd->SetArg(1,aStates->GetCurrectState().at(1));
      //     aStates->IncrementState();
      //   }
      //   else if(aMethod == "SetNbSegPerEdge" && aStates->GetCurrectState().size() >= 3) {
      //     if(!aStates->GetCurrectState().at(2).IsEmpty() )
      //       aCmd->SetArg(1,aStates->GetCurrectState().at(2));
      //     aStates->IncrementState();
      //   }
      //   else if(aMethod == "SetNbSegPerRadius" && aStates->GetCurrectState().size() >= 4) {
      //     if(!aStates->GetCurrectState().at(3).IsEmpty() )
      //       aCmd->SetArg(1,aStates->GetCurrectState().at(3));
      //     aStates->IncrementState();
      //   }
      // }

      // // Case for NETGEN_SimpleParameters_3D or NETGEN_SimpleParameters_2D hypothesis
      // else if(aStates->GetObjectType().IsEqual("NETGEN_SimpleParameters_3D") ||
      //         aStates->GetObjectType().IsEqual("NETGEN_SimpleParameters_2D")) {

      //   if((aMethod == "SetNumberOfSegments" || aMethod == "SetLocalLength") && 
      //      aStates->GetCurrectState().size() >= 1) {
      //     if(!aStates->GetCurrectState().at(0).IsEmpty() )
      //       aCmd->SetArg(1,aStates->GetCurrectState().at(0));
      //     aStates->IncrementState();
      //   }
      //   else if(aMethod == "SetMaxElementArea" && aStates->GetCurrectState().size() >= 2) {
      //     if(!aStates->GetCurrectState().at(1).IsEmpty() )
      //       aCmd->SetArg(1,aStates->GetCurrectState().at(1));
      //     aStates->IncrementState();
      //   }
      //   else if(aMethod == "SetMaxElementVolume" && aStates->GetCurrectState().size() >= 3) {
      //     if(!aStates->GetCurrectState().at(2).IsEmpty() )
      //       aCmd->SetArg(1,aStates->GetCurrectState().at(2));
      //     aStates->IncrementState();
      //   }
      //   else if(aMethod == "LengthFromEdges" || aMethod == "LengthFromFaces"){
      //     aStates->IncrementState();
      //   }
      // }

      else
      {
        // treat Netgen hypotheses;
        // this (and above) code can work wrong since nb of states can differ from nb of
        // dumped calls due to the fix of
        // issue 0021364:: Dump of netgen parameters has duplicate lines
        SMESH_Gen_i *          aGen = SMESH_Gen_i::GetSMESHGen();
        SALOMEDS::Study_var  aStudy = aGen->GetCurrentStudy();
        SALOMEDS::SObject_wrap sobj = aStudy->FindObjectID( (*it).first.ToCString() );
        CORBA::Object_var       obj = aGen->SObjectToObject( sobj );
        if ( SMESH_Hypothesis_i* h = SMESH::DownCast< SMESH_Hypothesis_i*>( obj ))
        {
          TState aCurrentState = aStates->GetCurrectState();
          int         argIndex = h->getParamIndex( aMethod, aCurrentState.size() );
          if ( 0 <= argIndex && argIndex < (int)aCurrentState.size() &&
               !aCurrentState[argIndex].IsEmpty() )
            aCmd->SetArg( 1, aCurrentState[argIndex] );

          if ( argIndex >= 0 )
            aStates->IncrementState();
        }
      }
    }
    else {
      if(MYDEBUG)
        cout << "Object not found" << endl;
    }
    if(MYDEBUG) {
      cout<<"Command after: "<< aCmd->GetString()<<endl;
    }
  }
  
  ProcessLayerDistribution();
}
//================================================================================
/*!
 * \brief Private method
 */
//================================================================================
void SMESH_NoteBook::InitObjectMap()
{
  SMESH_Gen_i *aGen = SMESH_Gen_i::GetSMESHGen();
  if(!aGen)
    return;
  
  SALOMEDS::Study_var aStudy = aGen->GetCurrentStudy();
  if(aStudy->_is_nil())
    return;
  
  CORBA::String_var compDataType = aGen->ComponentDataType();
  SALOMEDS::SObject_wrap     aSO = aStudy->FindComponent( compDataType.in() );
  if(CORBA::is_nil(aSO))
    return;
  
  SALOMEDS::ChildIterator_wrap Itr = aStudy->NewChildIterator(aSO);
  for( Itr->InitEx(true); Itr->More(); Itr->Next())
  {
    SALOMEDS::SObject_wrap aSObject = Itr->Value();
    SALOMEDS::GenericAttribute_wrap anAttr;
    if ( aSObject->FindAttribute( anAttr.inout(), "AttributeString"))
    {
      SALOMEDS::AttributeString_wrap strAttr = anAttr;
      CORBA::String_var          aParameters = strAttr->Value();
      CORBA::String_var                 anID = aSObject->GetID();
      std::vector< std::string >     allVars = aGen->GetAllParameters( anID.in() );
      SALOMEDS::ListOfListOfStrings_var aSections = aStudy->ParseVariables(aParameters.in());
      _entry2VarsMap[ TCollection_AsciiString( anID.in() )] = allVars;
      if(MYDEBUG) {
        cout<<"Entry : "<< anID<<endl;
        cout<<"aParameters : "<<aParameters<<endl;
      }
      TCollection_AsciiString anObjType;
      CORBA::Object_var       anObject = SMESH_Gen_i::SObjectToObject(aSObject);
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow(anObject);
      SMESH::SMESH_Mesh_var      aMesh = SMESH::SMESH_Mesh::_narrow(anObject);
      if(!aHyp->_is_nil()) {
        CORBA::String_var hypName = aHyp->GetName();
        anObjType = hypName.in();
      }
      else if (!aMesh->_is_nil() ) {
        anObjType = "Mesh";
      }
      if(MYDEBUG)
        cout<<"The object Type : "<<anObjType<<endl;
      SMESH_ObjectStates *aState = NULL;
      if(anObjType == "LayerDistribution")
        aState = new LayerDistributionStates();
      else
        aState = new SMESH_ObjectStates(anObjType);

      for ( size_t i = 0; i < aSections->length(); i++ ) {
        TState aVars;
        SALOMEDS::ListOfStrings aListOfVars = aSections[i];
        for ( size_t j = 0; j < aListOfVars.length(); j++)
        {
          TCollection_AsciiString aVar(aListOfVars[j].in());
          if(!aVar.IsEmpty() && aStudy->IsVariable(aVar.ToCString())) {
            aVar.InsertBefore(1,            SMESH::TVar::Quote() );
            aVar.InsertAfter(aVar.Length(), SMESH::TVar::Quote() );
          }
          aVars.push_back(aVar);
          if(MYDEBUG) {
            cout<<"Variable: '"<<aVar<<"'"<<endl;
          }
        }
        aState->AddState(aVars);
      }
      if ( aState->GetAllStates().empty() )
      {
        delete aState;
      }
      else
      {
        CORBA::String_var objID = aSObject->GetID();
        _objectMap.insert( make_pair(TCollection_AsciiString( objID.in() ), aState ));
      }
    }
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================
void SMESH_NoteBook::AddCommand(const TCollection_AsciiString& theString)
{
  if(MYDEBUG)
    cout<<theString<<endl;
  Handle(_pyCommand) aCommand = new _pyCommand( theString, -1);
  _commands.push_back(aCommand);

  if ( aCommand->GetMethod() == "GetMeshEditor" ) { // MeshEditor creation
    _meshEditors.insert( make_pair( aCommand->GetResultValue(),
                                    aCommand->GetObject() ) );
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================
void SMESH_NoteBook::ProcessLayerDistribution()
{
  // 1) Find all LayerDistribution states
  vector<LayerDistributionStates*> aLDS;
  TVariablesMap::const_iterator it = _objectMap.begin();
  for(;it != _objectMap.end();it++) {
    LayerDistributionStates* aLDStates = dynamic_cast<LayerDistributionStates*>(((*it).second));
    if(aLDStates!=NULL) {
      aLDS.push_back(aLDStates);
    }
  }
  
  if(!aLDS.size())
    return;
  
  // 2) Initialize all type of 1D Distribution hypothesis
  for ( size_t i = 0; i < _commands.size(); i++ ) {
    for ( size_t j = 0; j < aLDS.size(); j++ ) {
      TCollection_AsciiString aResultValue = _commands[i]->GetResultValue();
      if(_commands[i]->GetMethod() == "CreateHypothesis" &&
         aLDS[j]->HasDistribution(aResultValue)){
        TCollection_AsciiString aType = _commands[i]->GetArg(1);
        aType.RemoveAll('\'');
        aLDS[j]->SetDistributionType(aResultValue,aType);
      }
    }
  }
  // 3) ... and replase variables ...

  for ( size_t i = 0; i < _commands.size(); i++ ) {
    for ( size_t j = 0; j < aLDS.size(); j++ ) {
      TCollection_AsciiString anObject = _commands[i]->GetObject();

      if(aLDS[j]->HasDistribution(anObject)) {
        TCollection_AsciiString aType = aLDS[j]->GetDistributionType(anObject);
        TCollection_AsciiString aMethod = _commands[i]->GetMethod();
        if(aType == "LocalLength") {
          if(aMethod == "SetLength") {
            SetVariable(_commands[i], aLDS[j],0,1);
            aLDS[j]->IncrementState();
          }
          else if(aMethod == "SetPrecision") {
            SetVariable(_commands[i], aLDS[j],1,1);
            aLDS[j]->IncrementState();
          }
        }

        // Case for NumberOfSegments hypothesis
        else if(aType == "NumberOfSegments"){
          if(aMethod == "SetNumberOfSegments") {
            SetVariable(_commands[i], aLDS[j],0,1);
            if(aLDS[j]->GetCurrectState().size()==1)
              aLDS[j]->IncrementState();
          }
          else if (aMethod == "SetScaleFactor") {
            SetVariable(_commands[i], aLDS[j],1,1);
            aLDS[j]->IncrementState();
          }
        }
        
        else if( aType == "Deflection1D" ){
          if(aMethod == "SetDeflection"){
            SetVariable(_commands[i], aLDS[j],0,1);
            aLDS[j]->IncrementState();
          }
        }
        // Case for Arithmetic1D and StartEndLength hypothesis
        else if(aType == "Arithmetic1D" || aType == "StartEndLength") {
          if(aMethod == "SetLength") {
            int anArgNb = (_commands[i]->GetArg(2) == "1") ? 0 : 1;
            SetVariable(_commands[i], aLDS[j],anArgNb,1);
            aLDS[j]->IncrementState();
          }
        }
      }
    }
  }
}
//================================================================================
/*!
 *  \brief Return result script
 */
//================================================================================
TCollection_AsciiString SMESH_NoteBook::GetResultScript() const
{
  TCollection_AsciiString aResult;
  for ( size_t i = 0; i < _commands.size(); i++ )
    aResult += _commands[i]->GetString() + "\n";
  return aResult;
}

//================================================================================
/*!
 *  \brief Return lines of the result script
 */
//================================================================================
void SMESH_NoteBook::GetResultLines(std::list< TCollection_AsciiString >& lines) const
{
  for ( size_t i = 0; i < _commands.size(); i++ )
    lines.push_back( _commands[i]->GetString() );
}

//================================================================================
/*!
 *  \brief Return value of the variable
 */
//================================================================================
bool SMESH_NoteBook::GetReal(const TCollection_AsciiString& theVarName, double& theValue)
{
  bool ok = false;

  SMESH_Gen_i *aGen = SMESH_Gen_i::GetSMESHGen();
  if(!aGen)
    return ok;

  SALOMEDS::Study_ptr aStudy = aGen->GetCurrentStudy();
  if(aStudy->_is_nil())
    return ok;

  TCollection_AsciiString aVarName = theVarName;
  aVarName.RemoveAll('\"');

  if(aVarName.IsEmpty())
    return ok;

  const char* aName = aVarName.ToCString();
  if(aStudy->IsVariable(aName) && (aStudy->IsReal(aName) || aStudy->IsInteger(aName))) {
    theValue = aStudy->GetReal(aName);
    ok = true;
  }

  return ok;
}
