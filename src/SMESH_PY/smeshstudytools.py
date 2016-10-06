# -*- coding: utf-8 -*-
#
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

## \package smeshstudytools Python API to access SMESH objects in the study.

## \defgroup smeshstudytools Accessing SMESH object in the study
#  \{ 
#  \details
#  Module \b smeshstudytools provides a new class SMeshStudyTools to facilitate the
#  use of mesh objects in Salome study.
#  \}

"""
This module provides a new class :class:`SMeshStudyTools` to facilitate the
use of mesh objects in Salome study.
"""

import salome
SMESH = None    # SMESH module is loaded only when needed

from salome.kernel.studyedit import getStudyEditor
from salome.kernel.deprecation import is_called_by_sphinx
if not is_called_by_sphinx():
  from salome.gui import helper

## This class provides several methods to manipulate mesh objects in Salome
#  study. The parameter \em studyEditor defines a \b StudyEditor
#  object used to access the study. If \b None, the method returns a 
#  \b StudyEditor object on the current study.
#
#  \b editor
#  This instance attribute contains the underlying \b StudyEditor object. 
#  It can be used to access the study but the attribute itself should not be modified.
#  \ingroup smeshstudytools
class SMeshStudyTools:
    """
    This class provides several methods to manipulate mesh objects in Salome
    study. The parameter `studyEditor` defines a
    :class:`~salome.kernel.studyedit.StudyEditor` object used to access the study. If
    :const:`None`, the method returns a :class:`~salome.kernel.studyedit.StudyEditor`
    object on the current study.

    .. attribute:: editor
    
       This instance attribute contains the underlying
       :class:`~salome.kernel.studyedit.StudyEditor` object. It can be used to access
       the study but the attribute itself should not be modified.

    """

    def __init__(self, studyEditor = None):
        global SMESH
        if SMESH is None:
            SMESH = __import__("SMESH")
        if studyEditor is None:
            studyEditor = getStudyEditor()
        self.editor = studyEditor
        self.smeshGui = None

    ## This function updates the tools so that it works on the
    #  specified study.
    def updateStudy(self, studyId=None):
        """
        This function updates the tools so that it works on the
        specified study.
        """
        self.editor = getStudyEditor(studyId)

    ## Get the mesh item owning the mesh group \em meshGroupItem.
    #  \param  meshGroupItem (SObject) mesh group belonging to the searched mesh.
    #  \return The SObject corresponding to the mesh, or None if it was not found.        
    def getMeshFromGroup(self, meshGroupItem):
        """
        Get the mesh item owning the mesh group `meshGroupItem`.

        :type   meshGroupItem: SObject
        :param  meshGroupItem: Mesh group belonging to the searched mesh.
        
        :return: The SObject corresponding to the mesh, or None if it was not
                 found.
        """
        meshItem = None
        obj = self.editor.getOrLoadObject(meshGroupItem)
        group = obj._narrow(SMESH.SMESH_GroupBase)
        if group is not None: # The type of the object is ok
            meshObj = group.GetMesh()
            meshItem = salome.ObjectToSObject(meshObj)
        return meshItem

    ## Returns the MESH object currently selected in the active study.
    def getMeshObjectSelected(self):
        """
        Returns the MESH object currently selected in the active study.
        """
        sobject, entry = helper.getSObjectSelected()
        meshObject = self.getMeshObjectFromEntry(entry)
        return meshObject

    ## Returns the MESH object associated to the specified entry,
    #  (the entry is the identifier of an item in the objects browser).
    def getMeshObjectFromEntry(self, entry):
        """
        Returns the MESH object associated to the specified entry,
        (the entry is the identifier of an item in the objects browser).
        """
        if entry is None:
            return None
        import SMESH
        from salome.smesh import smeshBuilder
        smesh = smeshBuilder.New(self.editor.study)

        meshObject=salome.IDToObject(entry)
        return smesh.Mesh( meshObject )
    
    ## Returns the SMESH object associated to the specified \em SObject,
    #  (the SObject is an item in the objects browser).
    def getMeshObjectFromSObject(self, sobject):
        """
        Returns the SMESH object associated to the specified SObject,
        (the SObject is an item in the objects browser).
        """
        if sobject is None:
            return None
        
        obj = self.editor.getOrLoadObject(sobject)
        meshObject = obj._narrow(SMESH.SMESH_Mesh)
        return meshObject

    ## Display the SMESH object associated to the specified \em entry
    #  (the entry is the identifier of an item in the objects browser).
    def displayMeshObjectFromEntry(self,entry):
        """
        Display the SMESH object associated to the specified entry
        (the entry is the identifier of an item in the objects browser).    
        """
        if self.smeshGui is None:
            self.smeshGui = salome.ImportComponentGUI("SMESH")

        if not helper.SalomeGUI.hasDesktop():
            print "displayMeshObject: no desktop available"
            return
        self.smeshGui.CreateAndDisplayActor(entry)

#
# ==================================================================
# Use cases and demo functions
# ==================================================================
#

# CAUTION: Before running this test functions, you first have to
# create (or import) an smesh object and select this object in the
# objects browser. You can run the box mesh creation procedure below
# instead.

# How to test?
# 1. Run a SALOME application including GEOM and SMESH, and create a new study
# 2. In the console, enter:
#    >>> from salome.smesh import smeshstudytools
#    >>> smeshstudytools.TEST_createBoxMesh()
# 3. Select the object named "boxmesh" in the browser
# 4. In the console, enter:
#    >>> smeshstudytools.TEST_selectAndExport_01()
#    >>> smeshstudytools.TEST_selectAndExport_02()
#    >>> smeshstudytools.TEST_display()


def TEST_createBoxMesh():
    theStudy = helper.getActiveStudy()
    
    import GEOM
    from salome.geom import geomBuilder
    geompy = geomBuilder.New(theStudy)
    
    box = geompy.MakeBoxDXDYDZ(200, 200, 200)

    import SMESH, SALOMEDS
    from salome.smesh import smeshBuilder
    smesh = smeshBuilder.New(theStudy) 

    from salome.StdMeshers import StdMeshersBuilder
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
        salome.sg.updateObjBrowser(True)

#
# Definitions:
# - the SObject is an item in the study (Study Object).
# - the entry is the identifier of an item.
# - the object (geom object or smesh object) is a CORBA servant
#   embedded in the SALOME component container and with a reference in
#   the SALOME study, so that it can be retrieved.
#

def TEST_selectAndExport_01():
    tool = SMeshStudyTools()
    myMesh = tool.getMeshObjectSelected()
    myMesh.ExportUNV("/tmp/myMesh.unv")

def TEST_selectAndExport_02():
    # In this case, we want to retrieve the name of the mesh in the
    # object browser. Note that in SALOME, a mesh object has no
    # name. Only the SObject in the object browser has a name
    # attribute.
    tool = SMeshStudyTools()

    mySObject, myEntry = helper.getSObjectSelected()
    myName = mySObject.GetName()

    myMesh = tool.getMeshObjectFromEntry(myEntry)
    exportFileName = "/tmp/"+myName+".unv"
    myMesh.ExportUNV(exportFileName)

def TEST_display():
    mySObject, myEntry = helper.getSObjectSelected()

    tool = SMeshStudyTools()
    tool.displayMeshObjectFromEntry(myEntry)

if __name__ == "__main__":
    TEST_selectAndExport_01()
    TEST_selectAndExport_02()
    TEST_display()
