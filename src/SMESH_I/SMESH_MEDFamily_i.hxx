// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_MEDFamily_i.hxx
//  Module : SMESH
//
#ifndef SMESH_MED_FAMILY_I_HXX_
#define SMESH_MED_FAMILY_I_HXX_

#include "SMESH.hxx"

#include "SMESH_MEDSupport_i.hxx"

#include<string>

class SMESH_I_EXPORT SMESH_MEDFamily_i:
  public virtual POA_SALOME_MED::FAMILY,
  public virtual SMESH_MEDSupport_i
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
  std::string *  _attributeDescription;
  int       _numberOfGroup ;
  std::string *  _groupName ;
  

public :
  
  // Constructors and associated internal methods
  SMESH_MEDFamily_i(int identifier, SMESH_subMesh_i* sm,
                    std::string name, std::string description, SALOME_MED::medEntityMesh entity );
  SMESH_MEDFamily_i(const SMESH_MEDFamily_i & f);
  
  // IDL Methods
  void setProtocol(SALOME::TypeOfCommunication typ) {}
  void release() {}
  SALOME::SenderInt_ptr getSenderForNumber(SALOME_MED::medGeometryElement) {return SALOME::SenderInt::_nil();}
  SALOME::SenderInt_ptr getSenderForNumberIndex() {return SALOME::SenderInt::_nil();}
  
  CORBA::Long            getIdentifier()      
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getNumberOfAttributes() 
    throw (SALOME::SALOME_Exception);
  SALOME_TYPES::ListOfLong*   getAttributesIdentifiers() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getAttributeIdentifier(CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  SALOME_TYPES::ListOfLong*   getAttributesValues() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getAttributeValue(CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  SALOME_TYPES::ListOfString* getAttributesDescriptions() 
    throw (SALOME::SALOME_Exception);
  char*                  getAttributeDescription( CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  CORBA::Long               getNumberOfGroups()
    throw (SALOME::SALOME_Exception);
  char *                    getGroupName( CORBA::Long i)
    throw (SALOME::SALOME_Exception);
  SALOME_TYPES::ListOfString* getGroupsNames()
    throw (SALOME::SALOME_Exception);  
};
#endif /* MED_FAMILY_I_HXX_ */
