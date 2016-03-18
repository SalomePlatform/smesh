# Copyright (C) 2015-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#  File   : smesh_selection.py
#  Author : Roman NIKOLAEV, OPEN CASCADE ( roman.nikolaev@opencascade.com )
#  Module : SMESH

import salome
salome.salome_init()

import libSMESH_Swig
sm_gui = libSMESH_Swig.SMESH_Swig()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

import GEOM

# swig -> idl
_converter = { 
    libSMESH_Swig.EdgeOfCell    :  None, # TODO: check how to process it
    libSMESH_Swig.Node          :  SMESH.NODE,
    libSMESH_Swig.Edge          :  SMESH.EDGE,
    libSMESH_Swig.Face          :  SMESH.FACE,
    libSMESH_Swig.Volume        :  SMESH.VOLUME,
    libSMESH_Swig.Elem0D        :  SMESH.ELEM0D,
    libSMESH_Swig.Ball          :  SMESH.BALL,
    libSMESH_Swig.Cell          :  SMESH.ALL
}

# Converts swig to idl enumeration
def _swig2idl( type ):
    if _converter.has_key( type ) :
        return _converter[type]
    return None

def _getEntry(mesh):
    if isinstance( mesh, smeshBuilder.Mesh ) :
        return salome.ObjectToID( mesh.GetMesh() )
    else :
        if isinstance( mesh, str ) :
            return mesh
    return None

def _getMesh(mesh):
    if isinstance( mesh, smeshBuilder.Mesh ) :
        return mesh.GetMesh()
    else :
        if isinstance( mesh, str ) :
            return salome.IDToObject( mesh )
    return None

def _getGeom(geom):
    if isinstance( geom, GEOM._objref_GEOM_Object ) :
        return geom
    else :
        if isinstance( geom, str ) :
            return salome.IDToObject( geom )
    return None


# Selects an elements lst on the mesh
def select( mesh, lst, append = False ) :
    # Check mesh parameter
    entry = _getEntry(mesh)   
    if entry is None:
        print "Wrong 'mesh' parameter"
        return
    
    # Check lst parameter
    tmp = []
    if isinstance( lst, int ) :
        tmp.append( lst )
    else :
        if isinstance( lst,list ) :
            tmp = lst
        else :
            print "Wrong 'lst' parameter"
            return
    sm_gui.select( entry, tmp, append )


def _preProcess(mesh) :
    m = _getMesh(mesh);
    if m is None:
        print "Wrong 'mesh' parameter"
        return [None, None]
    
    elemType = _swig2idl(sm_gui.getSelectionMode())
    if elemType is None:
        return [None, None]
    return [m, elemType]



# Selects an elements on the mesh inside the sphere with radius r and center (x, y, z)
def selectInsideSphere( mesh, x, y, z, r, append = False ) :

    [m, elemType] = _preProcess(mesh)
    if m is None or elemType is None :
        return
    
    l = smesh.GetInsideSphere( m, elemType, x, y, z, r )
    if len(l) > 0:
        select(mesh, l, append)

# Selects an elements on the mesh inside the box
def selectInsideBox( mesh, x1, y1, z1, x2, y2, z2 , append = False ) :    

    [m, elemType] = _preProcess(mesh)
    if m is None or elemType is None :
        return

    l = smesh.GetInsideBox( m, elemType, x1, y1, z1, x2, y2, z2 )
    if len(l) > 0:
        select(mesh, l, append)

# Selects an elements on the mesh inside the cylinder
def selectInsideCylinder( mesh, x, y, z, dx, dy, dz, h, r, append = False ) :

    [m, elemType] = _preProcess(mesh)
    if m is None or elemType is None :
        return

    l = smesh.GetInsideCylinder( m, elemType, x, y, z, dx, dy, dz, h, r )
    if len(l) > 0:
        select(mesh, l, append)

# Selects an elements on the mesh inside the geometrical object
def selectInside( mesh, geom, tolerance , append = False ):

    [m, elemType] = _preProcess(mesh)
    if m is None or elemType is None :
        return    

    g = _getGeom(geom)

    l = smesh.GetInside( m, elemType, g ,tolerance )
    if len(l) > 0:
        select(mesh, l, append)
