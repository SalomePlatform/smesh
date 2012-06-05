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

// SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
// File : SMESH_MEDSupport_i.hxx
// Module : SMESH
//
#ifndef _MED_SMESH_MEDSUPPORT_I_HXX_
#define _MED_SMESH_MEDSUPPORT_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(MED)
#include <string>

#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"

#include "SMESH_MEDSupport_i.hxx"
#include "SALOME_GenericObj_i.hh"
class SMESH_subMesh;
class SMESH_subMesh_i;

class SMESH_I_EXPORT SMESH_MEDSupport_i:
        public virtual POA_SALOME_MED::SUPPORT, public virtual SALOME::GenericObj_i
{
  public:

// Constructors and associated internal methods
        SMESH_MEDSupport_i(SMESH_subMesh_i * sm,
                std::string name, std::string description, SALOME_MED::medEntityMesh entity);
        SMESH_MEDSupport_i(const SMESH_MEDSupport_i & s);

// IDL Methods
        char *getName() throw(SALOME::SALOME_Exception);
        char *getDescription() throw(SALOME::SALOME_Exception);
        SALOME_MED::GMESH_ptr getMesh() throw(SALOME::SALOME_Exception);
        CORBA::Boolean isOnAllElements() throw(SALOME::SALOME_Exception);
        SALOME_MED::medEntityMesh getEntity() throw(SALOME::SALOME_Exception);
        CORBA::Long
        getNumberOfElements(SALOME_MED::medGeometryElement geomElement)
          throw(SALOME::SALOME_Exception);

        CORBA::Long getNumberOfTypes() throw (SALOME::SALOME_Exception);

        SALOME_TYPES::ListOfLong *
        getNumber(SALOME_MED::medGeometryElement geomElement)
          throw(SALOME::SALOME_Exception);

  /*!
   * Same function as getNumber.
   */
        SALOME_TYPES::ListOfLong *
        getNumberFromFile(SALOME_MED::medGeometryElement geomElement)
          throw(SALOME::SALOME_Exception);

        SALOME_TYPES::ListOfLong * getNumberIndex()
          throw(SALOME::SALOME_Exception);

        CORBA::Long
        getNumberOfGaussPoint(SALOME_MED::medGeometryElement geomElement)
          throw(SALOME::SALOME_Exception);

        SALOME_TYPES::ListOfLong* getNumbersOfGaussPoint()
          throw (SALOME::SALOME_Exception);

        SALOME_MED::medGeometryElement_array *getTypes()
          throw(SALOME::SALOME_Exception);

        void getBoundaryElements() throw (SALOME::SALOME_Exception);

        CORBA::Long getCorbaIndex() throw(SALOME::SALOME_Exception);

        SALOME_MED::SUPPORT::supportInfos * getSupportGlobal()
          throw (SALOME::SALOME_Exception);

        void createSeq() throw(SALOME::SALOME_Exception);

  public: //public field
        SMESH_subMesh_i * _subMesh_i;
        ::SMESH_subMesh * _subMesh;

        std::string _name;
        std::string _description;
        bool _isOnAllElements;
        bool _seqNumber;
        int _seqLength;

        SALOME_MED::medEntityMesh _entity;
        SALOME_MED::medGeometryElement * _geometricType;
        int _numberOfGeometricType;

  protected:
        SMESH_MEDSupport_i();
        ~SMESH_MEDSupport_i();
};

#endif /* _MED_MEDSUPPORT_I_HXX_ */
