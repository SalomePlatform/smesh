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

// File      : SMESH_NoteBook.hxx
// Author    : Roman NIKOLAEV ()
//
#ifndef SMESH_NoteBook_HeaderFile
#define SMESH_NoteBook_HeaderFile

// All this stuff is obsolete since issue 0021308:
// "Remove hard-coded dependency of the external mesh plugins from the SMESH module"
// is implemented (Mar 2012). It is kept for backward compatibility only.

#include <TCollection_AsciiString.hxx>
#include <Resource_DataMapOfAsciiStringAsciiString.hxx>
 
class _pyCommand;

#include <vector>
#include <string>

typedef std::vector<TCollection_AsciiString>  TState;
typedef std::vector<TState>                   TAllStates;
typedef TCollection_AsciiString               _pyID;

class SMESH_ObjectStates{
  
public:
  
  SMESH_ObjectStates(TCollection_AsciiString theType);
  virtual ~SMESH_ObjectStates();

  void AddState(const TState &theState);

  TState GetCurrectState() const;
  TAllStates GetAllStates() const;
  void IncrementState();
  TCollection_AsciiString GetObjectType() const;

  

private:
  TCollection_AsciiString                   _type;
  TAllStates                                _states;
  int                                       _dumpstate;
};

class LayerDistributionStates : public SMESH_ObjectStates
{
public:
  typedef std::map<TCollection_AsciiString,TCollection_AsciiString> TDistributionMap;
  LayerDistributionStates();
  virtual ~LayerDistributionStates();

  void AddDistribution(const TCollection_AsciiString& theDistribution);
  bool HasDistribution(const TCollection_AsciiString& theDistribution) const;

  bool SetDistributionType(const TCollection_AsciiString& theDistribution,
                           const TCollection_AsciiString& theType);
  TCollection_AsciiString GetDistributionType(const TCollection_AsciiString& theDistribution) const;
  
private:
  
  TDistributionMap _distributions;
};


class SMESH_NoteBook
{
public:

  SMESH_NoteBook();
  ~SMESH_NoteBook();
  void ReplaceVariables();
  
  void AddCommand(const TCollection_AsciiString& theString);
  TCollection_AsciiString GetResultScript() const;
  void GetResultLines(std::list< TCollection_AsciiString >& lines) const;

private:

  void InitObjectMap();
  void ProcessLayerDistribution();

  bool GetReal(const TCollection_AsciiString& theVarName, double& theValue);
  
private:
  
  typedef std::map<TCollection_AsciiString,SMESH_ObjectStates*>         TVariablesMap;
  typedef std::map<TCollection_AsciiString,TCollection_AsciiString>     TMeshEditorMap;
  typedef std::map<TCollection_AsciiString,std::vector< std::string > > TEntry2VarVecMap;

  TVariablesMap                   _objectMap; // old approach - full states are kept
  TEntry2VarVecMap                _entry2VarsMap; // new approach - only var names are kept
  std::vector<Handle(_pyCommand)> _commands;
  TMeshEditorMap                  _meshEditors;
};

#endif //SMESH_NoteBook_HeaderFile
