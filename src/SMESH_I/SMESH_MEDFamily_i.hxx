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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_MEDFamily_i.hxx
//  Module : SMESH

#ifndef SMESH_MED_FAMILY_I_HXX_
#define SMESH_MED_FAMILY_I_HXX_

#include "SMESH_MEDSupport_i.hxx"

#include<string>

class SMESH_MEDFamily_i:
  public POA_SALOME_MED::FAMILY,
  public SMESH_MEDSupport_i
{
protected :
  SMESH_MEDFamily_i();
  ~SMESH_MEDFamily_i();
  
  ::SMESH_subMesh_i*      _subMesh_i;
  
  // Values
  int       _identifier;
  int       _numberOfAttribute;
  int    *  _attributeIdentifier;
  int    *  _attributeValue;
  string *  _attributeDescription;
  int       _numberOfGroup ;
  string *  _groupName ;
  

public :
  
  // Constructors and associated internal methods
  SMESH_MEDFamily_i(int identifier, SMESH_subMesh_i* sm,
		    string name, string description, SALOME_MED::medEntityMesh entity );
  SMESH_MEDFamily_i(const SMESH_MEDFamily_i & f);
  
  CORBA::Long            getIdentifier()      
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getNumberOfAttributes() 
    throw (SALOME::SALOME_Exception);
  SALOME_MED::long_array*   getAttributesIdentifiers() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getAttributeIdentifier(CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  SALOME_MED::long_array*   getAttributesValues() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getAttributeValue(CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  SALOME_MED::string_array* getAttributesDescriptions() 
    throw (SALOME::SALOME_Exception);
  char*                  getAttributeDescription( CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  CORBA::Long               getNumberOfGroups()
    throw (SALOME::SALOME_Exception);
  char *                    getGroupName( CORBA::Long i)
    throw (SALOME::SALOME_Exception);
  SALOME_MED::string_array* getGroupsNames()
    throw (SALOME::SALOME_Exception);  
};
#endif /* MED_FAMILY_I_HXX_ */
