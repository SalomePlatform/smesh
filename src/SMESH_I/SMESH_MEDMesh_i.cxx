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
//  File   : SMESH_MEDMesh_i.cxx
//  Module : SMESH
//
#include "SMESH_MEDMesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"

#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_MapOfShape.hxx>

#include "utilities.h"
#include "Utils_CorbaException.hxx"

#include "SMESH_MEDSupport_i.hxx"
#include "SMESH_MEDFamily_i.hxx"

# include "Utils_ORB_INIT.hxx"
# include "Utils_SINGLETON.hxx"
# include "Utils_ExceptHandlers.hxx"

extern "C"
{
#include <stdio.h>
}

using namespace std;

//=============================================================================
/*!
 * Default constructor
 */
//=============================================================================
// PN Est-ce un const ?
SMESH_MEDMesh_i::SMESH_MEDMesh_i()
{
  BEGIN_OF("Default Constructor SMESH_MEDMesh_i");
  END_OF("Default Constructor SMESH_MEDMesh_i");
}

//=============================================================================
/*!
 * Destructor
 */
//=============================================================================
SMESH_MEDMesh_i::~SMESH_MEDMesh_i()
{
}

//=============================================================================
/*!
 * Constructor
 */
//=============================================================================
SMESH_MEDMesh_i::SMESH_MEDMesh_i(::SMESH_Mesh_i * m_i):_meshId(""),
                                                       _compte(false),
                                                       _creeFamily(false),
                                                       _famIdent(0),
                                                       _indexElts(0),
                                                       _indexEnts(0)
{
  BEGIN_OF("Constructor SMESH_MEDMesh_i");

  _mesh_i = m_i;
  _meshDS = _mesh_i->GetImpl().GetMeshDS();

  END_OF("Constructor SMESH_MEDMesh_i");
}

//=============================================================================
/*!
 * CORBA: Accessor for Name
 */
//=============================================================================
char *SMESH_MEDMesh_i::getName() throw(SALOME::SALOME_Exception)
{
  if (_meshDS == NULL)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);

  try
  {
    SMESH_Gen_i*             gen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_var    study = gen->GetCurrentStudy();
    SALOMEDS::SObject_var meshSO = gen->ObjectToSObject( study, _mesh_i->_this());
    if ( meshSO->_is_nil() )
      return CORBA::string_dup("toto");

    CORBA::String_var name = meshSO->GetName();
    return CORBA::string_dup( name.in() );
  }
  catch(...)
  {
    MESSAGE("Exception en accedant au nom");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
  return 0;
}

//=============================================================================
/*!
 * CORBA: Accessor for corbaindex cuisine interne 
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getCorbaIndex()throw(SALOME::SALOME_Exception)
{
  MESSAGE("Non Implemente");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
}

//=============================================================================
/*!
 * CORBA: Accessor for Space Dimension
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getSpaceDimension()throw(SALOME::SALOME_Exception)
{
  // PN : Il semblerait que la dimension soit fixee a 3
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  return 3;
}

//=============================================================================
/*!
 * CORBA: Accessor for Mesh Dimension
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getMeshDimension()throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  // PN : Il semblerait que la dimension soit fixee a 3
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  return 3;
}
//=============================================================================
/*!
 * CORBA: Accessor for the boolean _isAGrid
 */
//=============================================================================
CORBA::Boolean SMESH_MEDMesh_i::getIsAGrid() throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!!!! NOT YET IMPLEMENTED !!!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return false;
}
//=============================================================================
/*!
 * CORBA: Accessor for the connectivities, to see if they exist
 */
//=============================================================================
CORBA::Boolean
SMESH_MEDMesh_i::existConnectivity(SALOME_MED::medConnectivity connectivityType,
                                   SALOME_MED::medEntityMesh entity)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!!!! IMPLEMENTED BUT ONLY PARTIALLY !!!!!!");


  return false;

}
//=============================================================================
/*!
 * CORBA: Accessor for Coordinate
 */
//=============================================================================
CORBA::Double SMESH_MEDMesh_i::getCoordinate(CORBA::Long Number, CORBA::Long Axis)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!!!! NOT YET IMPLEMENTED !!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return 0.0;
}
//=============================================================================
/*!
 * CORBA: Accessor for Coordinates System
 */
//=============================================================================
char *SMESH_MEDMesh_i::getCoordinatesSystem() throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  // PN : En dur. Non encore prevu
  try
  {
    string systcoo = "CARTESIEN";
    return CORBA::string_dup(systcoo.c_str());
  }
  catch(...)
  {
    MESSAGE("Exception en accedant au maillage");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
}

//=============================================================================
/*!
 * CORBA: Accessor for Coordinates
 */
//=============================================================================
SALOME_TYPES::ListOfDouble * SMESH_MEDMesh_i::getCoordinates
(SALOME_MED::medModeSwitch typeSwitch) throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  SALOME_TYPES::ListOfDouble_var myseq = new SALOME_TYPES::ListOfDouble;
  try
  {
    // PN  : En dur
    int spaceDimension = 3;
    int nbNodes = _meshDS->NbNodes();
    SCRUTE(nbNodes);
    myseq->length(nbNodes * spaceDimension);
    int i = 0;

    SMDS_NodeIteratorPtr itNodes=_meshDS->nodesIterator();
    while(itNodes->more())
    {
      const SMDS_MeshNode* node = itNodes->next();

      if (typeSwitch == SALOME_MED::MED_FULL_INTERLACE)
      {
        myseq[i * 3] = node->X();
        myseq[i * 3 + 1] = node->Y();
        myseq[i * 3 + 2] = node->Z();
        SCRUTE(myseq[i * 3]);
        SCRUTE(myseq[i * 3 + 1]);
        SCRUTE(myseq[i * 3 + 2]);
      }
      else
      {
        ASSERT(typeSwitch == SALOME_MED::MED_NO_INTERLACE);
        myseq[i] = node->X();
        myseq[i + nbNodes] = node->Y();
        myseq[i + (nbNodes * 2)] = node->Z();
        SCRUTE(myseq[i]);
        SCRUTE(myseq[i + nbNodes]);
        SCRUTE(myseq[i + (nbNodes * 2)]);
      }
      i++;
    }
  }
  catch(...)
  {
    MESSAGE("Exception en accedant aux coordonnees");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
  return myseq._retn();
}

//=============================================================================
/*!
 * CORBA: Accessor for Coordinates Names
 */
//=============================================================================
SALOME_TYPES::ListOfString *
SMESH_MEDMesh_i::getCoordinatesNames()throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  SALOME_TYPES::ListOfString_var myseq = new SALOME_TYPES::ListOfString;
  try
  {
    // PN : en dur
    int spaceDimension = 3;
    myseq->length(spaceDimension);
    myseq[0] = CORBA::string_dup("x");
    myseq[1] = CORBA::string_dup("y");
    myseq[2] = CORBA::string_dup("z");
  }
  catch(...)
  {
    MESSAGE("Exception en accedant aux noms des coordonnees");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
  return myseq._retn();

}

//=============================================================================
/*!
 * CORBA: Accessor for Coordinates Units
 */
//=============================================================================
SALOME_TYPES::ListOfString *
SMESH_MEDMesh_i::getCoordinatesUnits()throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  SALOME_TYPES::ListOfString_var myseq = new SALOME_TYPES::ListOfString;
  try
  {
    // PN : en dur
    int spaceDimension = 3;
    myseq->length(spaceDimension);
    myseq[0] = CORBA::string_dup("m");
    myseq[1] = CORBA::string_dup("m");
    myseq[2] = CORBA::string_dup("m");
  }
  catch(...)
  {
    MESSAGE("Exception en accedant aux unites des coordonnees");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
  return myseq._retn();
}

//=============================================================================
/*!
 * CORBA: Accessor for Number of Nodes
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getNumberOfNodes()throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  try
  {
    return _meshDS->NbNodes();
  }
  catch(...)
  {
    MESSAGE("Exception en accedant au nombre de noeuds");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
}

//=============================================================================
/*!
 * CORBA: Accessor for number of Types
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getNumberOfTypes(SALOME_MED::medEntityMesh entity)
  throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  try
  {
    if (!_compte)
      calculeNbElts();
    int retour = 0;
    if (_mapNbTypes.find(entity) != _mapNbTypes.end())
      retour = _mapNbTypes[entity];
    return retour;
  }
  catch(...)
  {
    MESSAGE("Exception en accedant au nombre de Types");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
}

//=============================================================================
/*!
 * CORBA: Accessor for existing geometry element types 
 *        Not implemented for MED_ALL_ENTITIES
 */
//=============================================================================
SALOME_MED::medGeometryElement_array *
SMESH_MEDMesh_i::getTypes(SALOME_MED::medEntityMesh entity) throw(SALOME::
                                                                  SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  if (entity == SALOME_MED::MED_ALL_ENTITIES)
    THROW_SALOME_CORBA_EXCEPTION("Not implemented for MED_ALL_ENTITIES",
                                 SALOME::BAD_PARAM);
  if (!_compte)
    calculeNbElts();
  SALOME_MED::medGeometryElement_array_var myseq =
    new SALOME_MED::medGeometryElement_array;
  try
  {
    if (_mapNbTypes.find(entity) == _mapNbTypes.end())
      THROW_SALOME_CORBA_EXCEPTION("No Such Entity in the mesh",
                                   SALOME::BAD_PARAM);
    int nbTypes = _mapNbTypes[entity];

    myseq->length(nbTypes);

    if (_mapIndToVectTypes.find(entity) == _mapIndToVectTypes.end())
      THROW_SALOME_CORBA_EXCEPTION("No Such Entity in the mesh",
                                   SALOME::INTERNAL_ERROR);

    int index = _mapIndToVectTypes[entity];
    ASSERT(_TypesId[index].size() != 0);
    int i = 0;
    vector < SALOME_MED::medGeometryElement >::iterator it;
    for (it = _TypesId[index].begin(); it != _TypesId[index].end(); it++)
    {
      myseq[i++] = *it;
    };
  }
  catch(...)
  {
    MESSAGE("Exception en accedant aux differents types");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
  return myseq._retn();
}

//=============================================================================
/*!
 * CORBA: Returns number of elements of type medGeometryElement
 *        Not implemented for MED_ALL_ELEMENTS 
 *        implemented for MED_ALL_ENTITIES
 *
 * Dans cette implementation, il n est pas prevu de tenir compte du entity
 * qui ne doit pas pouvoir avoir deux valeurs differentes pour un geomElement 
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getNumberOfElements(SALOME_MED::
                                                 medEntityMesh entity,
                                                 SALOME_MED::medGeometryElement geomElement) throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  if (geomElement == SALOME_MED::MED_ALL_ELEMENTS)
    THROW_SALOME_CORBA_EXCEPTION("Not implemented for MED_ALL_ELEMENTS",
                                 SALOME::BAD_PARAM);
  if (!_compte)
    calculeNbElts();

  try
  {
    int retour = 0;
    if (_mapIndToSeqElts.find(geomElement) != _mapIndToSeqElts.end())
    {
      int index = _mapIndToSeqElts[geomElement];

      retour = _seq_elemId[index]->length();
    }
    return retour;
  }
  catch(...)
  {
    MESSAGE("Exception en accedant au nombre d élements");
    THROW_SALOME_CORBA_EXCEPTION("Unable to acces Mesh C++ Object",
                                 SALOME::INTERNAL_ERROR);
  }
}

//=============================================================================
/*!
 * CORBA: Accessor for connectivities
 */
//=============================================================================
SALOME_TYPES::ListOfLong *
SMESH_MEDMesh_i::getConnectivity(SALOME_MED::medConnectivity mode,
                                 SALOME_MED::medEntityMesh entity,
                                 SALOME_MED::medGeometryElement geomElement)
  throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  if (mode != SALOME_MED::MED_NODAL)
    THROW_SALOME_CORBA_EXCEPTION("Not Implemented", SALOME::BAD_PARAM);
  /*if (typeSwitch == SALOME_MED::MED_NO_INTERLACE)
    THROW_SALOME_CORBA_EXCEPTION("Not Yet Implemented", SALOME::BAD_PARAM);*/
  if (!_compte)
    calculeNbElts();

  // Faut-il renvoyer un pointeur vide ???
  if (_mapIndToSeqElts.find(geomElement) != _mapIndToSeqElts.end())
    THROW_SALOME_CORBA_EXCEPTION("No Such Element in the mesh",
                                 SALOME::BAD_PARAM);

  int index = _mapIndToSeqElts[geomElement];

  return _seq_elemId[index]._retn();
}

//=============================================================================
/*!
 * CORBA: Accessor for connectivities
 */
//=============================================================================
SALOME_TYPES::ListOfLong *
SMESH_MEDMesh_i::getConnectivityIndex(SALOME_MED::medConnectivity mode,
                                      SALOME_MED::medEntityMesh entity)
  throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: Find an element corresponding to the given connectivity
 */
//=============================================================================
CORBA::Long
SMESH_MEDMesh_i::getElementNumber(SALOME_MED::medConnectivity mode,
                                  SALOME_MED::medEntityMesh entity,
                                  SALOME_MED::medGeometryElement type,
                                  const SALOME_TYPES::ListOfLong & connectivity)
  throw(SALOME::SALOME_Exception)
{
  const char *LOC = "getElementNumber ";
  MESSAGE(LOC << "Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return -1;
}

//=============================================================================
/*!
 * CORBA: Accessor for Ascendant connectivities
 * not implemented for MED_ALL_ENTITIES and MED_MAILLE
 */
//=============================================================================
SALOME_TYPES::ListOfLong *
SMESH_MEDMesh_i::getReverseConnectivity(SALOME_MED::
                                        medConnectivity mode) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: Accessor for connectivities
 */
//=============================================================================
SALOME_TYPES::ListOfLong *
SMESH_MEDMesh_i::getReverseConnectivityIndex(SALOME_MED::
                                             medConnectivity mode) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: Returns number of families within the mesh
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getNumberOfFamilies(SALOME_MED::
                                                 medEntityMesh entity) throw(SALOME::SALOME_Exception)
{
  if (_creeFamily == false)
    createFamilies();
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  return _families.size();
}

//=============================================================================
/*!
 * CORBA: Returns number of groups within the mesh
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::getNumberOfGroups(SALOME_MED::medEntityMesh entity)
  throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  MESSAGE(" Pas d implementation des groupes dans SMESH");
  return 0;
}

//=============================================================================
/*!
 * CORBA: Returns references for families within the mesh
 */
//=============================================================================
SALOME_MED::Family_array *
SMESH_MEDMesh_i::getFamilies(SALOME_MED::
                             medEntityMesh entity) throw(SALOME::SALOME_Exception)
{
  if (_creeFamily == false)
    createFamilies();
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  SALOME_MED::Family_array_var myseq = new SALOME_MED::Family_array;
  int nbfam = _families.size();
  myseq->length(nbfam);
  int i = 0;
  vector < SALOME_MED::FAMILY_ptr >::iterator it;
  for (it = _families.begin(); it != _families.end(); it++)
  {
    myseq[i++] = *it;
  };
  return myseq._retn();
}

//=============================================================================
/*!
 * CORBA: Returns references for family i within the mesh
 */
//=============================================================================
SALOME_MED::FAMILY_ptr SMESH_MEDMesh_i::getFamily(SALOME_MED::
                                                  medEntityMesh entity, CORBA::Long i) throw(SALOME::SALOME_Exception)
{
  if (_creeFamily == false)
    createFamilies();
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);

  SCRUTE(_families[i]->getName());
  MESSAGE(" SMESH_MEDMesh_i::getFamily " << i) return _families[i];
}

//=============================================================================
/*!
 * CORBA: Returns references for groups within the mesh
 */
//=============================================================================
SALOME_MED::Group_array *
SMESH_MEDMesh_i::getGroups(SALOME_MED::medEntityMesh entity) throw(SALOME::
                                                                   SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  MESSAGE(" Pas d implementation des groupes dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("No group implementation", SALOME::BAD_PARAM);
}

//=============================================================================
/*!
 * CORBA: Returns references for group i within the mesh
 */
//=============================================================================
SALOME_MED::GROUP_ptr SMESH_MEDMesh_i::getGroup(SALOME_MED::
                                                medEntityMesh entity, CORBA::Long i) throw(SALOME::SALOME_Exception)
{
  if (_mesh_i == 0)
    THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                                 SALOME::INTERNAL_ERROR);
  MESSAGE(" Pas d implementation des groupes dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("No group implementation", SALOME::BAD_PARAM);
}
//=============================================================================
/*!
 * CORBA: Returns references for the global numbering index
 */
//=============================================================================
SALOME_TYPES::ListOfLong*
SMESH_MEDMesh_i::getGlobalNumberingIndex(SALOME_MED::medEntityMesh entity)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}
//=============================================================================
/*!
 * CORBA: Returns references for the support of boundary elements of type
 * entity
 */
//=============================================================================
SALOME_MED::SUPPORT_ptr
SMESH_MEDMesh_i::getBoundaryElements(SALOME_MED::medEntityMesh entity)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}
//=============================================================================
/*!
 * CORBA:  Method return a reference on a support define on all the element of
 *         an entity.
 */
//=============================================================================
SALOME_MED::SUPPORT_ptr
SMESH_MEDMesh_i::getSupportOnAll(SALOME_MED::medEntityMesh entity)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}
//=============================================================================
/*!
 * CORBA: Returns references for the support of the skin of the support
 * mySupport3D
 */
//=============================================================================
SALOME_MED::SUPPORT_ptr
SMESH_MEDMesh_i::getSkin(SALOME_MED::SUPPORT_ptr mySupport3D)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}
//=============================================================================
/*!
 * CORBA: 
 */
//=============================================================================
SALOME_MED::FIELD_ptr SMESH_MEDMesh_i::getVolume(SALOME_MED::
                                                 SUPPORT_ptr mySupport) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: 
 */
//=============================================================================
SALOME_MED::FIELD_ptr SMESH_MEDMesh_i::getArea(SALOME_MED::
                                               SUPPORT_ptr mySupport) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: 
 */
//=============================================================================
SALOME_MED::FIELD_ptr SMESH_MEDMesh_i::getLength(SALOME_MED::
                                                 SUPPORT_ptr mySupport) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: 
 */
//=============================================================================
SALOME_MED::FIELD_ptr SMESH_MEDMesh_i::getNormal(SALOME_MED::
                                                 SUPPORT_ptr mySupport) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: 
 */
//=============================================================================
SALOME_MED::FIELD_ptr SMESH_MEDMesh_i::getBarycenter(SALOME_MED::
                                                     SUPPORT_ptr mySupport) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Pas Implemente dans SMESH");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: 
 */
//=============================================================================
SALOME_MED::FIELD_ptr SMESH_MEDMesh_i::getNeighbourhood(SALOME_MED::
                                                        SUPPORT_ptr mySupport) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Non Implemente");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * CORBA: add the Mesh in the StudyManager 
 * PN Pas Implemente
 */
//=============================================================================
void SMESH_MEDMesh_i::addInStudy(SALOMEDS::Study_ptr myStudy,
                                 SALOME_MED::GMESH_ptr myIor) throw(SALOME::SALOME_Exception)
{
  BEGIN_OF("MED_Mesh_i::addInStudy");
  if (_meshId != "")
  {
    MESSAGE("Mesh already in Study");
    THROW_SALOME_CORBA_EXCEPTION("Mesh already in Study",
                                 SALOME::BAD_PARAM);
  };

  /*
   * SALOMEDS::StudyBuilder_var myBuilder = myStudy->NewBuilder();
   * 
   * // Create SComponent labelled 'MED' if it doesn't already exit
   * SALOMEDS::SComponent_var medfather = myStudy->FindComponent("MED");
   * if ( CORBA::is_nil(medfather) ) 
   * {
   * MESSAGE("Add Component MED");
   * medfather = myBuilder->NewComponent("MED");
   * //myBuilder->AddAttribute (medfather,SALOMEDS::Name,"MED");
   * SALOMEDS::AttributeName_var aName = SALOMEDS::AttributeName::_narrow(
   * myBuilder->FindOrCreateAttribute(medfather, "AttributeName"));
   * aName->SetValue("MED");
   * 
   * myBuilder->DefineComponentInstance(medfather,myIor);
   * 
   * } ;
   * 
   * MESSAGE("Add a mesh Object under MED");
   * myBuilder->NewCommand();
   * SALOMEDS::SObject_var newObj = myBuilder->NewObject(medfather);
   * 
   * ORB_INIT &init = *SINGLETON_<ORB_INIT>::Instance() ;
   * ASSERT(SINGLETON_<ORB_INIT>::IsAlreadyExisting()) ;
   * CORBA::ORB_var &orb = init(0,0);
   * CORBA::String_var iorStr = orb->object_to_string(myIor);
   * //myBuilder->AddAttribute(newObj,SALOMEDS::IOR,iorStr.in());
   * SALOMEDS::AttributeIOR_var aIOR = SALOMEDS::AttributeIOR::_narrow(
   * myBuilder->FindOrCreateAttribute(newObj, "AttributeIOR"));
   * aIOR->SetValue(iorStr.c_str());
   * 
   * //myBuilder->AddAttribute(newObj,SALOMEDS::Name,_mesh_i->getName().c_str());
   * SALOMEDS::AttributeName_var aName = SALOMEDS::AttributeName::_narrow(
   * myBuilder->FindOrCreateAttribute(newObj, "AttributeName"));
   * aName->SetValue(_mesh_i->getName().c_str());
   * 
   * _meshId = newObj->GetID();
   * myBuilder->CommitCommand();
   * 
   */
  END_OF("Mesh_i::addInStudy(SALOMEDS::Study_ptr myStudy)");
}

//=============================================================================
/*!
 * CORBA: write mesh in a med file
 */
//=============================================================================
void SMESH_MEDMesh_i::write(CORBA::Long i, const char *driverMeshName)
  throw(SALOME::SALOME_Exception)
{
  MESSAGE("Non Implemente");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
}

//=============================================================================
/*!
 * CORBA: read mesh in a med file
 */
//=============================================================================
void SMESH_MEDMesh_i::read(CORBA::Long i) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Non Implemente");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
}

//=============================================================================
/*!
 * CORBA : release driver
 */
//=============================================================================
void SMESH_MEDMesh_i::rmDriver(CORBA::Long i) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Non Implemente");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
}

//=============================================================================
/*!
 * CORBA : attach driver
 */
//=============================================================================
CORBA::Long SMESH_MEDMesh_i::addDriver(SALOME_MED::medDriverTypes driverType,
                                       const char *fileName, const char *meshName) throw(SALOME::SALOME_Exception)
{
  MESSAGE("Non Implemente");
  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);
  return 0;
}

//=============================================================================
/*!
 * Calcule le Nb d'elements par entite geometrique
 */
//=============================================================================
void SMESH_MEDMesh_i::calculeNbElts() throw(SALOME::SALOME_Exception)
{
  if (!_compte)
  {
    _compte = true;

    _mapNbTypes[SALOME_MED::MED_NODE] = 1;
    // On compte les aretes MED_SEG2 ou MED_SEG3
    // On range les elements dans  les vecteurs correspondants 

    _mapIndToSeqElts[SALOME_MED::MED_SEG2] = _indexElts++;
    _mapIndToSeqElts[SALOME_MED::MED_SEG3] = _indexElts++;
    _mapIndToVectTypes[SALOME_MED::MED_EDGE] = _indexEnts++;

    int trouveSeg2 = 0;
    int trouveSeg3 = 0;
    SALOME_MED::medGeometryElement medElement;

    SMDS_EdgeIteratorPtr itEdges=_meshDS->edgesIterator();
    while(itEdges->more())
    {
      const SMDS_MeshEdge* elem = itEdges->next();
      int nb_of_nodes = elem->NbNodes();

      switch (nb_of_nodes)
      {
      case 2:
        {
          medElement = SALOME_MED::MED_SEG2;
          if (trouveSeg2 == 0)
          {
            trouveSeg2 = 1;
            _TypesId[SALOME_MED::MED_EDGE].
              push_back(SALOME_MED::MED_SEG2);
          }
          break;
        }
      case 3:
        {
          medElement = SALOME_MED::MED_SEG3;
          if (trouveSeg3 == 0)
          {
            trouveSeg3 = 1;
            _TypesId[SALOME_MED::MED_EDGE].
              push_back(SALOME_MED::MED_SEG3);
          }
          break;
        }
      }
      int index = _mapIndToSeqElts[medElement];
      SCRUTE(index);
      // Traitement de l arete

      int longueur = _seq_elemId[index]->length();
      _seq_elemId[index]->length(longueur + nb_of_nodes);

      SMDS_NodeIteratorPtr itn=_meshDS->nodesIterator();

      for(int k=0; itn->more(); k++)
        _seq_elemId[index][longueur + k] = itn->next()->GetID()+1;
    }

    _mapNbTypes[SALOME_MED::MED_EDGE] = trouveSeg2 + trouveSeg3;

    // On compte les faces MED_TRIA3, MED_HEXA8, MED_TRIA6
    // On range les elements dans  les vecteurs correspondants 
    int trouveTria3 = 0;
    int trouveTria6 = 0;
    int trouveQuad4 = 0;

    _mapIndToSeqElts[SALOME_MED::MED_TRIA3] = _indexElts++;
    _mapIndToSeqElts[SALOME_MED::MED_TRIA6] = _indexElts++;
    _mapIndToSeqElts[SALOME_MED::MED_QUAD4] = _indexElts++;
    _mapIndToVectTypes[SALOME_MED::MED_FACE] = _indexEnts++;

    SMDS_FaceIteratorPtr itFaces=_meshDS->facesIterator();
    while(itFaces->more())
    {
      const SMDS_MeshFace * elem = itFaces->next();
      int nb_of_nodes = elem->NbNodes();

      switch (nb_of_nodes)
      {
      case 3:
        {
          medElement = SALOME_MED::MED_TRIA3;
          if (trouveTria3 == 0)
          {
            trouveTria3 = 1;
            _TypesId[SALOME_MED::MED_FACE].
              push_back(SALOME_MED::MED_TRIA3);
          }
          break;
        }
      case 4:
        {
          medElement = SALOME_MED::MED_QUAD4;
          if (trouveQuad4 == 0)
          {
            trouveQuad4 = 1;
            _TypesId[SALOME_MED::MED_FACE].
              push_back(SALOME_MED::MED_QUAD4);
          }
          break;
        }
      case 6:
        {
          medElement = SALOME_MED::MED_TRIA6;
          if (trouveTria6 == 0)
          {
            trouveTria6 = 1;
            _TypesId[SALOME_MED::MED_FACE].
              push_back(SALOME_MED::MED_TRIA6);
          }
          break;
        }
      }
      int index = _mapIndToSeqElts[medElement];
      SCRUTE(index);

      // Traitement de la face
      // Attention La numérotation des noeuds Med commence a 1

      int longueur = _seq_elemId[index]->length();
      _seq_elemId[index]->length(longueur + nb_of_nodes);

      SMDS_NodeIteratorPtr itn=_meshDS->nodesIterator();

      for(int k=0; itn->more(); k++)
        _seq_elemId[index][longueur + k] = itn->next()->GetID()+1;
    } //itFaces

    _mapNbTypes[SALOME_MED::MED_FACE] =
      trouveTria3 + trouveTria6 + trouveQuad4;

    _mapIndToSeqElts[SALOME_MED::MED_HEXA8] = _indexElts++;
    _mapIndToVectTypes[SALOME_MED::MED_CELL] = _indexEnts++;
    int index = _mapIndToSeqElts[medElement];

    int trouveHexa8 = 0;

    SMDS_VolumeIteratorPtr itVolumes=_meshDS->volumesIterator();
    while(itVolumes->more())
    {
      const SMDS_MeshVolume * elem = itVolumes->next();

      int nb_of_nodes = elem->NbNodes();
      medElement = SALOME_MED::MED_HEXA8;
      ASSERT(nb_of_nodes == 8);

      if (trouveHexa8 == 0)
      {
        trouveHexa8 = 1;
        _TypesId[SALOME_MED::MED_CELL].push_back(SALOME_MED::MED_HEXA8);
      };
      // Traitement de la maille
      int longueur = _seq_elemId[index]->length();
      _seq_elemId[index]->length(longueur + nb_of_nodes);

      SMDS_NodeIteratorPtr itn=_meshDS->nodesIterator();
      for(int k=0; itn->more(); k++)
        _seq_elemId[index][longueur + k] = itn->next()->GetID()+1;
    }

    _mapNbTypes[SALOME_MED::MED_CELL] = trouveHexa8;
    _mapNbTypes[SALOME_MED::MED_ALL_ENTITIES]
      =
      trouveHexa8 + trouveTria3 + trouveTria6 + trouveQuad4 + trouveSeg2 +
      trouveSeg3;
  }// fin du _compte
};

//=============================================================================
/*!
 * Creation des familles
 */
//=============================================================================
void SMESH_MEDMesh_i::createFamilies() throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  string famDes = ("Je ne sais pas");
  string famName0 = "Famille_";
  string famName;
  char numero[10];

  if (_creeFamily == false)
  {
    _creeFamily = true;
    //SMESH_subMesh_i *subMeshServant;

    map < int, SMESH_subMesh_i * >::iterator it;
    for (it = _mesh_i->_mapSubMesh_i.begin();
         it != _mesh_i->_mapSubMesh_i.end(); it++)
    {
      SMESH_subMesh_i *submesh_i = (*it).second;
      int famIdent = (*it).first;

      ASSERT(famIdent < 999999999);
      sprintf(numero, "%d\n", famIdent);
      famName = famName0 + numero;

      SMESH_MEDFamily_i *famservant =
        new SMESH_MEDFamily_i(famIdent, submesh_i,
                              famName, famDes, SALOME_MED::MED_NODE);
#ifdef WNT
      SALOME_MED::FAMILY_ptr famille = SALOME_MED::FAMILY::_nil();
      POA_SALOME_MED::FAMILY* servantbase = dynamic_cast<POA_SALOME_MED::FAMILY*>(famservant);
      if ( servantbase )
        famille = SALOME_MED::FAMILY::_narrow( servantbase->_this() );
#else 
      SALOME_MED::FAMILY_ptr famille = 
        SALOME_MED::FAMILY::_narrow( famservant->POA_SALOME_MED::FAMILY::_this() );
#endif
      _families.push_back(famille);
    }
  }
};
//=============================================================================
/*!
 * Gives informations of the considered mesh.
 */
//=============================================================================
SALOME_MED::GMESH::meshInfos * SMESH_MEDMesh_i::getMeshGlobal()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}

//================================================================================
/*!
 * \brief Converts this GMESH into MESH
 */
//================================================================================

SALOME_MED::MESH_ptr SMESH_MEDMesh_i::convertInMESH() throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}

//=============================================================================
/*!
 * Gives informations on coordinates of the considered mesh.
 */
//=============================================================================
SALOME_MED::GMESH::coordinateInfos * SMESH_MEDMesh_i::getCoordGlobal()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}
//=============================================================================
/*!
 * Gives informations on connectivities of the considered mesh for the entity
 * entity.
 */
//=============================================================================
SALOME_MED::MESH::connectivityInfos *
SMESH_MEDMesh_i::getConnectGlobal(SALOME_MED::medEntityMesh entity)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return NULL;
}
//=============================================================================
/*!
 * Gives the type of the element number of entity entity
 */
//=============================================================================
SALOME_MED::medGeometryElement
SMESH_MEDMesh_i::getElementType(SALOME_MED::medEntityMesh entity,
                                CORBA::Long number)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!!! NOT YET IMPLEMENTED !!!!!");

  THROW_SALOME_CORBA_EXCEPTION("Unimplemented Method", SALOME::BAD_PARAM);

  return (SALOME_MED::medGeometryElement) 0;
}
