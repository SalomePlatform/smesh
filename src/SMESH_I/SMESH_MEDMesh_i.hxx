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
//  File   : SMESH_MEDMesh_i.hxx
//  Module : SMESH
//
#ifndef _MED_SMESH_MESH_I_HXX_
#define _MED_SMESH_MESH_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(MED)
#include <string>
#include <vector>
#include <map>

#include "SMESHDS_Mesh.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SALOME_GenericObj_i.hh"

#define MED_NBR_GEOMETRIE_MAILLE 15
#define MED_NBR_TYPE 5

class SMESH_Mesh_i;

class SMESH_I_EXPORT SMESH_MEDMesh_i:
  public virtual POA_SALOME_MED::MESH, 
  public virtual SALOME::GenericObj_i
{
protected:
  // C++ object containing values
  ::SMESH_Mesh_i * _mesh_i;
  SMESHDS_Mesh *_meshDS;
  
  std::string _meshId;
  bool _compte;
  bool _creeFamily;
  int _indexElts;
  int _indexEnts;
  int _famIdent;
  
  std::map < SALOME_MED::medGeometryElement, int >_mapIndToSeqElts;
  SALOME_TYPES::ListOfLong_var _seq_elemId[MED_NBR_GEOMETRIE_MAILLE];
  
  std::map < SALOME_MED::medEntityMesh, int >_mapNbTypes;
  std::map < SALOME_MED::medEntityMesh, int >_mapIndToVectTypes;
  std::vector < SALOME_MED::medGeometryElement >
  _TypesId[MED_NBR_GEOMETRIE_MAILLE];
  
  std::vector < SALOME_MED::FAMILY_ptr > _families;
public:
  
  // Constructors and associated internal methods
  SMESH_MEDMesh_i();
  SMESH_MEDMesh_i(SMESH_Mesh_i * m);
  ~SMESH_MEDMesh_i();
  
  // IDL Methods
  void setProtocol(SALOME::TypeOfCommunication typ) {}
  void release() {}
  SALOME::SenderDouble_ptr getSenderForCoordinates(SALOME_MED::medModeSwitch) {return SALOME::SenderDouble::_nil();}
  SALOME::SenderInt_ptr getSenderForConnectivity(SALOME_MED::medConnectivity, 
                                                 SALOME_MED::medEntityMesh, 
                                                 SALOME_MED::medGeometryElement) 
  {
    return SALOME::SenderInt::_nil();
  }  
  SALOME::SenderInt_ptr getSenderForConnectivityIndex(SALOME_MED::medConnectivity,
                                                      SALOME_MED::medEntityMesh,
                                                      SALOME_MED::medGeometryElement)
  {
    return SALOME::SenderInt::_nil();
  }  
  SALOME::SenderInt_ptr getSenderForPolygonsConnectivity(SALOME_MED::medConnectivity, SALOME_MED::medEntityMesh) {return SALOME::SenderInt::_nil();}
  SALOME::SenderInt_ptr getSenderForPolygonsConnectivityIndex(SALOME_MED::medConnectivity, SALOME_MED::medEntityMesh) {return SALOME::SenderInt::_nil();}
  SALOME::SenderInt_ptr getSenderForPolyhedronConnectivity(SALOME_MED::medConnectivity) {return SALOME::SenderInt::_nil();}
  SALOME::SenderInt_ptr getSenderForPolyhedronIndex(SALOME_MED::medConnectivity) {return SALOME::SenderInt::_nil();}
  SALOME::SenderInt_ptr getSenderForPolyhedronFacesIndex() {return SALOME::SenderInt::_nil();}
  
  char *getName() throw(SALOME::SALOME_Exception);
  CORBA::Long getSpaceDimension() throw(SALOME::SALOME_Exception);
  
  CORBA::Long getMeshDimension() throw(SALOME::SALOME_Exception);
  
  CORBA::Boolean  getIsAGrid() throw (SALOME::SALOME_Exception);
  
  CORBA::Boolean
  existConnectivity(SALOME_MED::medConnectivity connectivityType,
                    SALOME_MED::medEntityMesh entity)
    throw (SALOME::SALOME_Exception);
  
  char *getCoordinatesSystem() throw(SALOME::SALOME_Exception);
  
  CORBA::Double getCoordinate(CORBA::Long Number, CORBA::Long Axis)
    throw (SALOME::SALOME_Exception);
  
  SALOME_TYPES::ListOfDouble * getCoordinates(SALOME_MED::medModeSwitch typeSwitch)
    throw(SALOME::SALOME_Exception);

  SALOME_TYPES::ListOfString * getCoordinatesNames()
    throw(SALOME::SALOME_Exception);
  
  SALOME_TYPES::ListOfString * getCoordinatesUnits()
    throw(SALOME::SALOME_Exception);
  
  CORBA::Long getNumberOfNodes() throw(SALOME::SALOME_Exception);
  
  CORBA::Long getNumberOfTypes(SALOME_MED::medEntityMesh entity)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::medGeometryElement_array *
  getTypes(SALOME_MED::medEntityMesh entity) throw(SALOME::
                                                   SALOME_Exception);
  
  SALOME_MED::medGeometryElement
  getElementType(SALOME_MED::medEntityMesh entity,
                 CORBA::Long number)
    throw   (SALOME::SALOME_Exception);
  
  CORBA::Long getNumberOfElements(SALOME_MED::medEntityMesh entity,
                                  SALOME_MED::medGeometryElement geomElement)
    throw(SALOME::SALOME_Exception);
  
  SALOME_TYPES::ListOfLong *
  getConnectivity(SALOME_MED::medConnectivity mode,
                  SALOME_MED::medEntityMesh entity,
                  SALOME_MED::medGeometryElement geomElement)
    throw(SALOME::SALOME_Exception);
  
  SALOME_TYPES::ListOfLong *
  getConnectivityIndex(SALOME_MED::medConnectivity mode,
                       SALOME_MED::medEntityMesh entity)
    throw(SALOME::SALOME_Exception);
  
  SALOME_TYPES::ListOfLong*
  getGlobalNumberingIndex(SALOME_MED::medEntityMesh entity)
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long getElementNumber(SALOME_MED::medConnectivity mode,
                               SALOME_MED::medEntityMesh entity,
                               SALOME_MED::medGeometryElement type,
                               const SALOME_TYPES::ListOfLong & connectivity)
    throw(SALOME::SALOME_Exception);
  
  SALOME_TYPES::ListOfLong *
  getReverseConnectivity(SALOME_MED::medConnectivity mode)
    throw(SALOME::SALOME_Exception);
  
  SALOME_TYPES::ListOfLong *
  getReverseConnectivityIndex(SALOME_MED::medConnectivity mode) 
    throw(SALOME::SALOME_Exception);
  
  // Family and Group
  CORBA::Long getNumberOfFamilies(SALOME_MED::medEntityMesh entity)
    throw(SALOME::SALOME_Exception);
  
  CORBA::Long getNumberOfGroups(SALOME_MED::medEntityMesh entity)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::Family_array *
  getFamilies(SALOME_MED::medEntityMesh entity)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::FAMILY_ptr getFamily(SALOME_MED::medEntityMesh entity,
                                   CORBA::Long i) 
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::Group_array * getGroups(SALOME_MED::medEntityMesh entity)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::GROUP_ptr getGroup(SALOME_MED::medEntityMesh entity,
                                 CORBA::Long i) 
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::SUPPORT_ptr
  getBoundaryElements(SALOME_MED::medEntityMesh entity)
    throw (SALOME::SALOME_Exception);
  
  SALOME_MED::SUPPORT_ptr
  getSupportOnAll(SALOME_MED::medEntityMesh entity)
    throw (SALOME::SALOME_Exception);
  
  SALOME_MED::SUPPORT_ptr getSkin(SALOME_MED::SUPPORT_ptr mySupport3D)
    throw (SALOME::SALOME_Exception);
  
  SALOME_MED::FIELD_ptr getVolume(SALOME_MED::SUPPORT_ptr mySupport)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::FIELD_ptr getArea(SALOME_MED::SUPPORT_ptr mySupport)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::FIELD_ptr getLength(SALOME_MED::SUPPORT_ptr mySupport)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::FIELD_ptr getNormal(SALOME_MED::SUPPORT_ptr mySupport)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::FIELD_ptr getBarycenter(SALOME_MED::SUPPORT_ptr mySupport)
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::FIELD_ptr getNeighbourhood(SALOME_MED::SUPPORT_ptr mySupport)
    throw(SALOME::SALOME_Exception);
  
  // Others
  void addInStudy(SALOMEDS::Study_ptr myStudy,
                  SALOME_MED::GMESH_ptr myIor) 
    throw(SALOME::SALOME_Exception);
  CORBA::Long addDriver(SALOME_MED::medDriverTypes driverType,
                        const char *fileName, const char *meshName)
    throw(SALOME::SALOME_Exception);
  void rmDriver(CORBA::Long i) throw(SALOME::SALOME_Exception);
  void read(CORBA::Long i) throw(SALOME::SALOME_Exception);
  void write(CORBA::Long i, const char *driverMeshName)
    throw(SALOME::SALOME_Exception);
  
  //                    Cuisine interne
  CORBA::Long getCorbaIndex() 
    throw(SALOME::SALOME_Exception);
  
  SALOME_MED::GMESH::meshInfos * getMeshGlobal()
    throw (SALOME::SALOME_Exception);
  
  bool areEquals(SALOME_MED::GMESH_ptr other) { return false;};
  
  SALOME_MED::MESH_ptr convertInMESH() throw (SALOME::SALOME_Exception);
  
  SALOME_MED::GMESH::coordinateInfos * getCoordGlobal()
    throw (SALOME::SALOME_Exception);
  
  SALOME_MED::MESH::connectivityInfos *
  getConnectGlobal(SALOME_MED::medEntityMesh entity)
    throw (SALOME::SALOME_Exception);
  
  //
  void calculeNbElts() throw(SALOME::SALOME_Exception);
  void createFamilies() throw(SALOME::SALOME_Exception);
};

#endif /* _MED_MESH_I_HXX_ */
