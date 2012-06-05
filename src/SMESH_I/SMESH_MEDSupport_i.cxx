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
//  File   : SMESH_MEDSupport_i.cxx
//  Module : SMESH
//
#include "SMESH_MEDSupport_i.hxx"
#include "utilities.h"
#include "Utils_CorbaException.hxx"
#include "Utils_ExceptHandlers.hxx"

#include "SMESHDS_Mesh.hxx"

#include "SMESH_subMesh.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_subMesh_i.hxx"
#include "SMESH_Gen_i.hxx"

#include <TopoDS_Iterator.hxx>

using namespace std;


//=============================================================================
/*!
 * Default constructor
 */
//=============================================================================
SMESH_MEDSupport_i::SMESH_MEDSupport_i()
{
        BEGIN_OF("Default Constructor SMESH_MEDSupport_i");
        END_OF("Default Constructor SMESH_MEDSupport_i");
}

//=============================================================================
/*!
 * Constructor
 */
//=============================================================================
SMESH_MEDSupport_i::SMESH_MEDSupport_i(SMESH_subMesh_i * sm, string name,
        string description, SALOME_MED::medEntityMesh entity)
  :_subMesh_i(sm), _name(name), _description(description), _entity(entity),
   _seqNumber(false), _seqLength(0)
{
        BEGIN_OF("Constructor SMESH_MEDSupport_i");

        int subMeshId = sm->GetId();

        MESSAGE(" subMeshId " << subMeshId);

        SMESH_Mesh_i* mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( sm->GetMesh() );
        _subMesh = mesh_i->GetImpl().GetSubMeshContaining( subMeshId );

        if (_entity == SALOME_MED::MED_NODE)
        {
                _numberOfGeometricType = 1;
                _geometricType = new SALOME_MED::medGeometryElement[1];
                _geometricType[0] = SALOME_MED::MED_NONE;
        }
        else
        {
                MESSAGE("Pas implemente dans cette version");
                THROW_SALOME_CORBA_EXCEPTION
                        ("Seules les familles de noeuds sont implementees ",
                        SALOME::BAD_PARAM);
        }

        END_OF("Constructor SMESH_MEDSupport_i");
}

//=============================================================================
/*!
 * Constructor
 */
//=============================================================================
SMESH_MEDSupport_i::
SMESH_MEDSupport_i(const SMESH_MEDSupport_i & s):_subMesh(s._subMesh),
_name(s._name), _description(s._description), _entity(s._entity),
_seqNumber(false), _seqLength(0)
{
        BEGIN_OF("Constructor SMESH_MEDSupport_i");

        END_OF("Constructor SMESH_MEDSupport_i");
}

//=============================================================================
/*!
 * Destructor
 */
//=============================================================================

SMESH_MEDSupport_i::~SMESH_MEDSupport_i()
{
}

//=============================================================================
/*!
 * CORBA: Accessor for Corba Index 
 */
//=============================================================================

CORBA::Long SMESH_MEDSupport_i::getCorbaIndex()throw(SALOME::SALOME_Exception)
{
        if (_subMesh == NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);
        MESSAGE("Not implemented for SMESH_i");
        THROW_SALOME_CORBA_EXCEPTION("Not Implemented ", SALOME::BAD_PARAM);

}

//=============================================================================
/*!
 * CORBA: Accessor for Name 
 */
//=============================================================================

char *SMESH_MEDSupport_i::getName() throw(SALOME::SALOME_Exception)
{
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);
        return CORBA::string_dup(_name.c_str());

}

//=============================================================================
/*!
 * CORBA: Accessor for Description 
 */
//=============================================================================

char *SMESH_MEDSupport_i::getDescription() throw(SALOME::SALOME_Exception)
{
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);
        return CORBA::string_dup(_description.c_str());
}

//=============================================================================
/*!
 * CORBA: Accessor for Mesh 
 */
//=============================================================================

SALOME_MED::GMESH_ptr SMESH_MEDSupport_i::getMesh()throw(SALOME::
        SALOME_Exception)
{
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);

        return _subMesh_i->GetMesh()->GetMEDMesh();
}

//=============================================================================
/*!
 * CORBA: boolean indicating if support concerns all elements 
 */
//=============================================================================

CORBA::Boolean SMESH_MEDSupport_i::isOnAllElements()throw(SALOME::
        SALOME_Exception)
{
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);
        if (_seqNumber == false)
        {
                if (_entity != SALOME_MED::MED_NONE)
                {
                        _seqLength = _subMesh_i->GetNumberOfNodes(/*all=*/false);
                        _seqNumber = true;
                }
                else
                {
                        MESSAGE("Only Node Families are implemented ");
                        THROW_SALOME_CORBA_EXCEPTION("Not implemented Yet ",
                                SALOME::BAD_PARAM);
                }
        }
        try
        {
          _isOnAllElements = (_seqLength == _subMesh->GetFather()->NbNodes());
        }
        catch(...)
        {
                MESSAGE("unable to acces related Mesh");
                THROW_SALOME_CORBA_EXCEPTION("No associated Mesh",
                        SALOME::INTERNAL_ERROR);
        };
        return _isOnAllElements;
}

//=============================================================================
/*!
 * CORBA: Accessor for type of support's entity 
 */
//=============================================================================

SALOME_MED::medEntityMesh SMESH_MEDSupport_i::getEntity()throw(SALOME::
        SALOME_Exception)
{
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);
        return _entity;
}

//=============================================================================
/*!
 * CORBA: Accessor for types of geometry elements 
 */
//=============================================================================

SALOME_MED::medGeometryElement_array *
        SMESH_MEDSupport_i::getTypes()throw(SALOME::SALOME_Exception)
{
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);
        SALOME_MED::medGeometryElement_array_var myseq =
                new SALOME_MED::medGeometryElement_array;
        try
        {
                int mySeqLength = _numberOfGeometricType;
                myseq->length(mySeqLength);
                for (int i = 0; i < mySeqLength; i++)
                {
                        myseq[i] = _geometricType[i];
                }
        }
        catch(...)
        {
                MESSAGE("Exception lors de la recherche des differents types");
                THROW_SALOME_CORBA_EXCEPTION("Unable to acces Support Types",
                        SALOME::INTERNAL_ERROR);
        }
        return myseq._retn();
}

//=============================================================================
/*!
 * CORBA: Number of different types of geometry elements
 *        existing in the support
 */
//=============================================================================
CORBA::Long SMESH_MEDSupport_i::getNumberOfElements(SALOME_MED::
        medGeometryElement geomElement) throw(SALOME::SALOME_Exception)
{
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);
        return _numberOfGeometricType;

}

//=============================================================================
/*!
 * CORBA: get Nodes 
 */
//=============================================================================

SALOME_TYPES::ListOfLong * SMESH_MEDSupport_i::getNumber(
        SALOME_MED::medGeometryElement geomElement) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
        if (_subMesh==NULL)
                THROW_SALOME_CORBA_EXCEPTION("No associated Support",
                        SALOME::INTERNAL_ERROR);

        // A changer s'il ne s agit plus seulement de famille de noeuds
        if (geomElement != SALOME_MED::MED_NONE)
                THROW_SALOME_CORBA_EXCEPTION("Not implemented", SALOME::BAD_PARAM);

        SALOME_TYPES::ListOfLong_var myseq = new SALOME_TYPES::ListOfLong;

        int i = 0;
        myseq->length(_subMesh_i->GetNumberOfNodes(/*all=*/false));

        if ( _subMesh->GetSubMeshDS() )
        {
          SMDS_NodeIteratorPtr it = _subMesh->GetSubMeshDS()->GetNodes();
          while(it->more())
          {
            myseq[i] = it->next()->GetID();
            i++;
          };
        }

        SCRUTE(myseq->length());
        MESSAGE("End of SMESH_MEDSupport_i::getNumber");
        return myseq._retn();

}

//=============================================================================
/*!
 * CORBA: get Nodes from file
 */
//=============================================================================

SALOME_TYPES::ListOfLong * SMESH_MEDSupport_i::getNumberFromFile(
        SALOME_MED::medGeometryElement geomElement) throw(SALOME::SALOME_Exception)
{
  return getNumber(geomElement);
}

//=============================================================================
/*!
 * CORBA: Global Nodes Index (optionnaly designed by the user)
 * CORBA:  ??????????????????????????????
 */
//=============================================================================

SALOME_TYPES::ListOfLong *
        SMESH_MEDSupport_i::getNumberIndex()throw(SALOME::SALOME_Exception)
{
        MESSAGE("Not implemented for SMESH_i");
        THROW_SALOME_CORBA_EXCEPTION("Not Implemented", SALOME::BAD_PARAM);
        return NULL;
}
//=============================================================================
/*!
 * CORBA: Array containing indexes for elements included in the support  
 */
//=============================================================================

CORBA::Long SMESH_MEDSupport_i::getNumberOfGaussPoint(SALOME_MED::
        medGeometryElement geomElement) throw(SALOME::SALOME_Exception)
{
        MESSAGE("Not implemented for SMESH_i");
        return 0;
}
//=============================================================================
/*!
 * Gives the number of types of elements included in the support 
 */
//=============================================================================
CORBA::Long SMESH_MEDSupport_i::getNumberOfTypes()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!! NOT YET IMPLEMENTED !!!!");
  THROW_SALOME_CORBA_EXCEPTION("Not Implemented", SALOME::BAD_PARAM);
  return 0;
}
//=============================================================================
/*!
 * Gives CORBA: Array containing the numbers of Gauss point of elements
 * included in the support 
 */
//=============================================================================
SALOME_TYPES::ListOfLong* SMESH_MEDSupport_i::getNumbersOfGaussPoint()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!! NOT YET IMPLEMENTED !!!!");
  THROW_SALOME_CORBA_EXCEPTION("Not Implemented", SALOME::BAD_PARAM);
  return NULL;
}
//=============================================================================
/*!
 * build the object which will contain all the boundary elements of the mesh.
 */
//=============================================================================
void SMESH_MEDSupport_i::getBoundaryElements()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!! NOT YET IMPLEMENTED !!!!");
  THROW_SALOME_CORBA_EXCEPTION("Not Implemented", SALOME::BAD_PARAM);
}
//=============================================================================
/*!
 * Gives information on the support
 */
//=============================================================================
SALOME_MED::SUPPORT::supportInfos * SMESH_MEDSupport_i::getSupportGlobal()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("!!! NOT YET IMPLEMENTED !!!!");
  THROW_SALOME_CORBA_EXCEPTION("Not Implemented", SALOME::BAD_PARAM);
  return NULL;
}
