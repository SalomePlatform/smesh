# -*- coding: utf-8 -*-
#
#  Copyright (C) 2007-2009     EDF R&D
# 
#    This file is part of PAL_SRC.
#
#    PAL_SRC is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    PAL_SRC is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with PAL_SRC; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
#
"""
This module provides a new class :class:`SMeshStudyTools` to facilitate the
use of mesh objects in Salome study.
"""

import salome
SMESH = None    # SMESH module is loaded only when needed

from salome.kernel.studyedit import getStudyEditor

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
