# -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

# Author: Guillaume Boulant (EDF/R&D)

# ==================================================================
# This file provides helper functions to drive some SMESH gui features
# of a SALOME Application (the selection of mesh objects in the object
# browser for now, further development coming soon).
# ==================================================================

from salome.gui import helper

#
# ==================================================================
# Special functions to deal with SMESH objects
# ==================================================================
#
import salome
salome.salome_init()
import SMESH

def getSmeshObjectFromSObject(sobject):
    '''
    Returns the SMESH object associated to the specified SObject,
    (the SObject is an item in the objects browser).
    '''
    if sobject is None:
        return None

    #theObject = sobject.GetObject()
    #if theObject is not None:
    #    return theObject

    # The engine must be loaded
    theStudy   = helper.getActiveStudy()
    theBuilder = theStudy.NewBuilder()
    strContainer, strComponentName = "FactoryServer", "SMESH"
    engine     = salome.lcc.FindOrLoadComponent( strContainer, strComponentName )
    scomponent = theStudy.FindComponent( strComponentName )
    theBuilder.LoadWith( scomponent , engine  )

    # The servant can be retrieved from the sobject
    theObject = sobject.GetObject()
    # Then cast to the smesh desired class (supposed to be a mesh in
    # this example).
    smeshObject = theObject._narrow(SMESH.SMESH_Mesh)
    return smeshObject

import smesh
def getSmeshObjectFromEntry(entry):
    '''
    Returns the SMESH object associated to the specified entry,
    (the entry is the identifier of an item in the objects browser).
    '''
    if entry is None:
        return None
    theStudy = helper.getActiveStudy()
    smesh.SetCurrentStudy(theStudy)
    smeshObject=smesh.IDToObject(entry)
    return smeshObject

def getSmeshObjectSelected():
    '''
    Returns the SMESH object currently selected in the objects browser.
    '''
    sobject, entry = helper.getSObjectSelected()
    # You can retrieve the smesh object either from the sobject or
    # from the entry. From the entry is quicker.
    #smeshObject = getSmeshObjectFromSObject(sobject)
    smeshObject = getSmeshObjectFromEntry(entry)
    return smeshObject

SmeshGUI = salome.ImportComponentGUI("SMESH")
def displaySmeshObject(entry):
    '''
    Display the SMESH object associated to the specified entry
    (the entry is the identifier of an item in the objects browser).    
    '''
    if not SalomeGUI.hasDesktop():
        print "displayGeomObjects: no desktop available"
        return
    SmeshGUI.CreateAndDisplayActor(entry)

#
# ==================================================================
# Use cases and demo functions
# ==================================================================
#

# CAUTION: Before running this test functions, you first have to
# create (or import) an smesh object and select this object in the
# objects browser. You can run the box mesh creation procedure below
# instead.

def TEST_createBoxMesh():
    theStudy = helper.getActiveStudy()
    
    import geompy
    geompy.init_geom(theStudy)
    box = geompy.MakeBoxDXDYDZ(200, 200, 200)

    import smesh, SMESH, SALOMEDS    
    smesh.SetCurrentStudy(theStudy)
    import StdMeshers
    boxmesh = smesh.Mesh(box)
    Regular_1D = boxmesh.Segment()
    Nb_Segments_1 = Regular_1D.NumberOfSegments(15)
    Nb_Segments_1.SetDistrType( 0 )
    Quadrangle_2D = boxmesh.Quadrangle()
    Hexa_3D = smesh.CreateHypothesis('Hexa_3D')
    status = boxmesh.AddHypothesis(Hexa_3D)
    isDone = boxmesh.Compute()

    smesh.SetName(boxmesh.GetMesh(), 'boxmesh')
    if salome.sg.hasDesktop():
        salome.sg.updateObjBrowser(1)

#
# Definitions:
# - the SObject is an item in the study (Study Object).
# - the entry is the identifier of an item.
# - the object (geom object or smesh object) is a CORBA servant
#   embedded in the SALOME component container and with a reference in
#   the SALOME study, so that it can be retrieved.
#

def TEST_selectAndExport_01():
    myMesh = getSmeshObjectSelected()
    myMesh.ExportUNV("/tmp/myMesh.unv")

def TEST_selectAndExport_02():
    # In this case, we want to retrieve the name of the mesh in the
    # object browser. Note that in SALOME, a mesh object has no
    # name. Only the SObject in the object browser has a name
    # attribute.
    mySObject, myEntry = helper.getSObjectSelected()
    myName = mySObject.GetName()

    myMesh = getSmeshObjectFromEntry(myEntry)
    exportFileName = "/tmp/"+myName+".unv"
    myMesh.ExportUNV(exportFileName)

if __name__ == "__main__":
    TEST_selectAndExport_01()
    TEST_selectAndExport_02()
