//=============================================================================
// File      : SMESH_MEDFamily_i.hxx
// Project   : SALOME
// Copyright : EDF 2001
//=============================================================================

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
  Engines::long_array*   getAttributesIdentifiers() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getAttributeIdentifier(CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  Engines::long_array*   getAttributesValues() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long            getAttributeValue(CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  Engines::string_array* getAttributesDescriptions() 
    throw (SALOME::SALOME_Exception);
  char*                  getAttributeDescription( CORBA::Long i) 
    throw (SALOME::SALOME_Exception);
  
};
#endif /* MED_FAMILY_I_HXX_ */
