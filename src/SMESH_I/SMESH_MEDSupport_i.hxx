//=============================================================================
// File      : SMESH_MEDSupport_i.hxx
// Project   : SALOME
// Copyright : EDF 2001
//=============================================================================

#ifndef _MED_SMESH_MEDSUPPORT_I_HXX_
#define _MED_SMESH_MEDSUPPORT_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(MED)
#include <string>

#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"

#include "SMESH_MEDSupport_i.hxx"

class SMESH_subMesh_i;

class SMESH_MEDSupport_i: 
  public POA_SALOME_MED::SUPPORT,
  public PortableServer::RefCountServantBase 
{

protected :
    SMESH_MEDSupport_i();
    ~SMESH_MEDSupport_i();


public :
  
  Handle_SMESHDS_SubMesh  _subMeshDS;
  ::SMESH_subMesh_i*      _subMesh_i;

  Handle_SMESHDS_Mesh     _meshDS;
    string                   _name;
    string		     _description;
    bool		     _isOnAllElements;
    bool		     _seqNumber;
    int			     _seqLength;

    SALOME_MED::medEntityMesh         _entity;
    SALOME_MED::medGeometryElement *  _geometricType;
    int				      _numberOfGeometricType;


public:

    // Constructors and associated internal methods
  SMESH_MEDSupport_i(SMESH_subMesh_i* sm,
		     string name, 
		     string description,
		     SALOME_MED::medEntityMesh entity);
  SMESH_MEDSupport_i(const SMESH_MEDSupport_i & s);
  
  // IDL Methods
  char *               getName() 	 throw (SALOME::SALOME_Exception);
  char *               getDescription()  throw (SALOME::SALOME_Exception);
  SALOME_MED::MESH_ptr getMesh() 	 throw (SALOME::SALOME_Exception);
  CORBA::Boolean       isOnAllElements() throw (SALOME::SALOME_Exception);
  SALOME_MED::medEntityMesh getEntity()  throw (SALOME::SALOME_Exception);
  CORBA::Long          getNumberOfElements(SALOME_MED::medGeometryElement geomElement) 
    throw (SALOME::SALOME_Exception);
  Engines::long_array* getNumber(SALOME_MED::medGeometryElement geomElement) 
    throw (SALOME::SALOME_Exception);
  Engines::long_array* getNumberIndex() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long          getNumberOfGaussPoints(SALOME_MED::medGeometryElement geomElement) 
    throw (SALOME::SALOME_Exception);
  SALOME_MED::medGeometryElement_array* getTypes() 
    throw (SALOME::SALOME_Exception);
  CORBA::Long 	       getCorbaIndex()   throw (SALOME::SALOME_Exception);
  void 		       createSeq()       throw (SALOME::SALOME_Exception);
  

};

#endif /* _MED_MEDSUPPORT_I_HXX_ */
