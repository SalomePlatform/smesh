#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
#
#  File   : smesh.py
#  Author : Francis KLOSS, OCC
#  Module : SMESH

"""
 \namespace smesh
 \brief Module smesh
"""

import salome
import geompy
import StdMeshers
import SMESH

# Public variables
# ----------------

REGULAR = 1
PYTHON  = 2

NETGEN  = 3
GHS3D   = 4

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

# Private functions
# -----------------

NO_NAME = "NoName"

def GetName(obj):
    ior  = salome.orb.object_to_string(obj)
    sobj = salome.myStudy.FindObjectIOR(ior)
    if sobj is None:
        return NO_NAME
    else:
        attr = sobj.FindAttribute("AttributeName")[1]
        return attr.Value()

def SetName(obj, name):
    ior  = salome.orb.object_to_string(obj)
    sobj = salome.myStudy.FindObjectIOR(ior)
    attr = sobj.FindAttribute("AttributeName")[1]
    attr.SetValue(name)

# Algorithms and hypothesis
# =========================

# Private class: Mesh_Algorithm
# -----------------------------

class Mesh_Algorithm:
    """
    Mother class to define algorithm, recommended to don't use directly
    """

    mesh = 0
    geom = 0
    subm = 0

    def GetSubMesh(self):
        """
         If the algorithm is global, return 0
         else return the submesh associated to this algorithm
        """
        return self.subm

    def Create(self, mesh, geom, hypo, so="libStdMeshersEngine.so"):
        """
         Private method
        """
        self.mesh = mesh
        piece = mesh.geom
        if geom==0:
            self.geom = piece
            name = GetName(piece)
        else:
            self.geom = geom
            name = GetName(geom)
            if name==NO_NAME:
                name = geompy.SubShapeName(geom, piece)
                geompy.addToStudyInFather(piece, geom, name)
            self.subm = mesh.mesh.GetSubMesh(geom, hypo)

        algo = smesh.CreateHypothesis(hypo, so)
        SetName(algo, name + "/" + hypo)
        mesh.mesh.AddHypothesis(self.geom, algo)

    def Hypothesis(self, hyp, args=[], so="libStdMeshersEngine.so"):
        """
         Private method
        """
        hypo = smesh.CreateHypothesis(hyp, so)
        a = ""
        s = "="
        i = 0
        n = len(args)
        while i<n:
            a = a + s + str(args[i])
            s = ","
            i = i + 1
        SetName(hypo, GetName(self.geom) + "/" + hyp + a)
        self.mesh.mesh.AddHypothesis(self.geom, hypo)
        return hypo

# Public class: Mesh_Segment
# --------------------------

class Mesh_Segment(Mesh_Algorithm):
    """
    Class to define a segment 1D algorithm for discretization
    """

    def __init__(self, mesh, geom=0):
        """
         Private constructor
        """
        self.Create(mesh, geom, "Regular_1D")

    def LocalLength(self, l):
        """
         Define "LocalLength" hypothesis to cut an edge in several segments with the same length
         \param l for the length of segments that cut an edge
        """
        hyp = self.Hypothesis("LocalLength", [l])
        hyp.SetLength(l)
        return hyp

    def NumberOfSegments(self, n, s=[]):
        """
         Define "NumberOfSegments" hypothesis to cut an edge in several fixed number of segments
         \param n for the number of segments that cut an edge
         \param s for the scale factor (optional)
        """
        if s == []:
            hyp = self.Hypothesis("NumberOfSegments", [n])
        else:
            hyp = self.Hypothesis("NumberOfSegments", [n,s])
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        return hyp

    def Arithmetic1D(self, start, end):
        """
         Define "Arithmetic1D" hypothesis to cut an edge in several segments with arithmetic length increasing
         \param start for the length of the first segment
         \param end   for the length of the last  segment
        """
        hyp = self.Hypothesis("Arithmetic1D", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    def StartEndLength(self, start, end):
        """
         Define "StartEndLength" hypothesis to cut an edge in several segments with geometric length increasing
         \param start for the length of the first segment
         \param end   for the length of the last  segment
        """
        hyp = self.Hypothesis("StartEndLength", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    def Deflection1D(self, d):
        """
         Define "Deflection1D" hypothesis
         \param d for the deflection
        """
        hyp = self.Hypothesis("Deflection1D", [d])
        hyp.SetDeflection(d)
        return hyp

    def Propagation(self):
        """
         Define "Propagation" hypothesis that propagate all other hypothesis on all others edges that are in
         the opposite side in the case of quadrangular faces
        """
        return self.Hypothesis("Propagation")

# Public class: Mesh_Segment_Python
# ---------------------------------

class Mesh_Segment_Python(Mesh_Segment):
    """
    Class to define a segment 1D algorithm for discretization with python function
    """

    def __init__(self, mesh, geom=0):
        """
         Private constructor
        """
        import Python1dPlugin
        self.Create(mesh, geom, "Python_1D", "libPython1dEngine.so")

    def PythonSplit1D(self, n, func):
        """
         Define "PythonSplit1D" hypothesis based on the Erwan Adam patch, awaiting equivalent SALOME functionality
         \param n for the number of segments that cut an edge
         \param func for the python function that calculate the length of all segments
        """
        hyp = self.Hypothesis("PythonSplit1D", [n], "libPython1dEngine.so")
        hyp.SetNumberOfSegments(n)
        hyp.SetPythonLog10RatioFunction(func)
        return hyp

# Public class: Mesh_Triangle
# ---------------------------

class Mesh_Triangle(Mesh_Algorithm):
    """
    Class to define a triangle 2D algorithm
    """

    def __init__(self, mesh, geom=0):
        """
         Private constructor
        """
        self.Create(mesh, geom, "MEFISTO_2D")

    def MaxElementArea(self, area):
        """
         Define "MaxElementArea" hypothesis to give the maximun area of each triangles
         \param area for the maximum area of each triangles
        """
        hyp = self.Hypothesis("MaxElementArea", [area])
        hyp.SetMaxElementArea(area)
        return hyp

    def LengthFromEdges(self):
        """
         Define "LengthFromEdges" hypothesis to build triangles based on the length of the edges taken from the wire
        """
        return self.Hypothesis("LengthFromEdges")

# Public class: Mesh_Quadrangle
# -----------------------------

class Mesh_Quadrangle(Mesh_Algorithm):
    """
    Class to define a quadrangle 2D algorithm
    """

    def __init__(self, mesh, geom=0):
        """
         Private constructor
        """
        self.Create(mesh, geom, "Quadrangle_2D")

# Public class: Mesh_Tetrahedron
# ------------------------------

class Mesh_Tetrahedron(Mesh_Algorithm):
    """
    Class to define a tetrahedron 3D algorithm
    """

    def __init__(self, mesh, algo, geom=0):
        """
         Private constructor
        """
        if algo == NETGEN:
            self.Create(mesh, geom, "NETGEN_3D", "libNETGENEngine.so")
        elif algo == GHS3D:
            import GHS3DPlugin
            self.Create(mesh, geom, "GHS3D_3D" , "libGHS3DEngine.so")

    def MaxElementVolume(self, vol):
        """
         Define "MaxElementVolume" hypothesis to give the maximun volume of each tetrahedral
         \param vol for the maximum volume of each tetrahedral
        """
        hyp = self.Hypothesis("MaxElementVolume", [vol])
        hyp.SetMaxElementVolume(vol)
        return hyp

# Public class: Mesh_Hexahedron
# ------------------------------

class Mesh_Hexahedron(Mesh_Algorithm):
    """
    Class to define a hexahedron 3D algorithm
    """

    def __init__(self, mesh, geom=0):
        """
         Private constructor
        """
        self.Create(mesh, geom, "Hexa_3D")

# Public class: Mesh
# ==================

class Mesh:
    """
    Class to define a mesh
    """

    geom = 0
    mesh = 0

    def __init__(self, geom, name=0):
        """
         Constructor

         Creates mesh on the shape \a geom,
         sets GUI name of this mesh to \a name.
         \param geom Shape to be meshed
         \param name Study name of the mesh
        """
        self.geom = geom
        self.mesh = smesh.CreateMesh(geom)
        if name == 0:
            SetName(self.mesh, GetName(geom))
        else:
            SetName(self.mesh, name)

    def GetMesh(self):
        """
         Method that returns the mesh
        """
        return self.mesh

    def GetShape(self):
        """
         Method that returns the shape associated to the mesh
        """
        return self.geom

    def Segment(self, algo=REGULAR, geom=0):
        """
         Creates a segment discretization 1D algorithm.
         If the optional \a algo parameter is not sets, this algorithm is REGULAR.
         If the optional \a geom parameter is not sets, this algorithm is global.
         Otherwise, this algorithm define a submesh based on \a geom subshape.
         \param algo values are smesh.REGULAR or smesh.PYTHON for discretization via python function
         \param geom If defined, subshape to be meshed
        """
        if algo == REGULAR:
            return Mesh_Segment(self, geom)
        elif algo == PYTHON:
            return Mesh_Segment_Python(self, geom)
        else:
            return Mesh_Segment(self, algo)

    def Triangle(self, geom=0):
        """
         Creates a triangle 2D algorithm for faces.
         If the optional \a geom parameter is not sets, this algorithm is global.
         Otherwise, this algorithm define a submesh based on \a geom subshape.
         \param geom If defined, subshape to be meshed
        """
        return Mesh_Triangle(self, geom)

    def Quadrangle(self, geom=0):
        """
         Creates a quadrangle 2D algorithm for faces.
         If the optional \a geom parameter is not sets, this algorithm is global.
         Otherwise, this algorithm define a submesh based on \a geom subshape.
         \param geom If defined, subshape to be meshed
        """
        return Mesh_Quadrangle(self, geom)

    def Tetrahedron(self, algo, geom=0):
        """
         Creates a tetrahedron 3D algorithm for solids.
         The parameter \a algo permits to choice the algorithm: NETGEN or GHS3D
         If the optional \a geom parameter is not sets, this algorithm is global.
         Otherwise, this algorithm define a submesh based on \a geom subshape.
         \param algo values are: smesh.NETGEN, smesh.GHS3D
         \param geom If defined, subshape to be meshed
        """
        return Mesh_Tetrahedron(self, algo, geom)

    def Hexahedron(self, geom=0):
        """
         Creates a hexahedron 3D algorithm for solids.
         If the optional \a geom parameter is not sets, this algorithm is global.
         Otherwise, this algorithm define a submesh based on \a geom subshape.
         \param geom If defined, subshape to be meshed
        """
        return Mesh_Hexahedron(self, geom)

    def Compute(self):
        """
         Compute the mesh and return the status of the computation
        """
        b = smesh.Compute(self.mesh, self.geom)
        if salome.sg.hasDesktop():
            salome.sg.updateObjBrowser(1)
        return b

    def Group(self, grp, name=""):
        """
         Create a mesh group based on geometric object \a grp
         and give a \a name, if this parameter is not defined
         the name is the same as the geometric group name
         \param grp  is a geometric group, a vertex, an edge, a face or a solid
         \param name is the name of the mesh group
        """
        if name == "":
            name = grp.GetName()

        type = []
        tgeo = str(grp.GetShapeType())
        if tgeo == "VERTEX":
            type = SMESH.NODE
        elif tgeo == "EDGE":
            type = SMESH.EDGE
        elif tgeo == "FACE":
            type = SMESH.FACE
        elif tgeo == "SOLID":
            type = SMESH.VOLUME
        elif tgeo == "COMPOUND":
            tgeo = geompy.GetType(grp)
            if tgeo == geompy.ShapeType["VERTEX"]:
                type = SMESH.NODE
            elif tgeo == geompy.ShapeType["EDGE"]:
                type = SMESH.EDGE
            elif tgeo == geompy.ShapeType["FACE"]:
                type = SMESH.FACE
            elif tgeo == geompy.ShapeType["SOLID"]:
                type = SMESH.VOLUME

        if type == []:
            print "Mesh.Group: bad first argument: expected a group, a vertex, an edge, a face or a solid"
            return 0
        else:
            return self.mesh.CreateGroupFromGEOM(type, name, grp)

    def ExportToMED(self, f, version, opt=0):
        """
         Export the mesh in a file with the MED format and choice the \a version of MED format
         \param f is the file name
         \param version values are smesh.MED_V2_1, smesh.MED_V2_2
        """
        self.mesh.ExportToMED(f, opt, version)

    def ExportMED(self, f, opt=0):
        """
         Export the mesh in a file with the MED format
         \param f is the file name
        """
        self.mesh.ExportMED(f, opt)

    def ExportDAT(self, f):
        """
         Export the mesh in a file with the DAT format
         \param f is the file name
        """
        self.mesh.ExportDAT(f)

    def ExportUNV(self, f):
        """
         Export the mesh in a file with the UNV format
         \param f is the file name
        """
        self.mesh.ExportUNV(f)

    def ExportSTL(self, f, ascii=1):
        """
         Export the mesh in a file with the STL format
         \param f is the file name
         \param ascii defined the kind of file contents
        """
        self.mesh.ExportSTL(f, ascii)
