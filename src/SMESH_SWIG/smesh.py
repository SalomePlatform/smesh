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
    algo = 0

    def GetSubMesh(self):
        """
         If the algorithm is global, return 0
         else return the submesh associated to this algorithm
        """
        return self.subm

    def GetAlgorithm(self):
        """
         Return the wrapped mesher
        """
        return self.algo

    def TreatHypoStatus(self, status, hypName, geomName, isAlgo):
        """
        Private method. Print error message if a hypothesis was not assigned
        """
        if isAlgo:
            hypType = "algorithm"
        else:
            hypType = "hypothesis"
        if status == SMESH.HYP_UNKNOWN_FATAL :
            reason = "for unknown reason"
        elif status == SMESH.HYP_INCOMPATIBLE :
            reason = "this hypothesis mismatches algorithm"
        elif status == SMESH.HYP_NOTCONFORM :
            reason = "not conform mesh would be built"
        elif status == SMESH.HYP_ALREADY_EXIST :
            reason = hypType + " of the same dimension already assigned to this shape"
        elif status == SMESH.HYP_BAD_DIM :
            reason = hypType + " mismatches shape"
        elif status == SMESH.HYP_CONCURENT :
            reason = "there are concurrent hypotheses on sub-shapes"
        elif status == SMESH.HYP_BAD_SUBSHAPE :
            reason = "shape is neither the main one, nor its subshape, nor a valid group"
        else:
            return
        hypName = '"' + hypName + '"'
        geomName= '"' + geomName+ '"'
        if status < SMESH.HYP_UNKNOWN_FATAL:
            print hypName, "was assigned to",    geomName,"but", reason
        else:
            print hypName, "was not assigned to",geomName,":", reason
        pass

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

        self.algo = smesh.CreateHypothesis(hypo, so)
        SetName(self.algo, name + "/" + hypo)
        status = mesh.mesh.AddHypothesis(self.geom, self.algo)
        self.TreatHypoStatus( status, hypo, name, 1 )

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
        name = GetName(self.geom)
        SetName(hypo, name + "/" + hyp + a)
        status = self.mesh.mesh.AddHypothesis(self.geom, hypo)
        self.TreatHypoStatus( status, hyp, name, 0 )
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
            hyp.SetDistrType( 1 )
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

    def AutomaticLength(self, fineness=0):
        """
         Define "AutomaticLength" hypothesis
         \param fineness for the fineness [0-1]
        """
        hyp = self.Hypothesis("AutomaticLength")
        hyp.SetFineness( fineness )
        return hyp

    def QuadraticMesh(self):
        """
         Define "QuadraticMesh" hypothesis, forcing construction of quadratic edges.
         If the 2D mesher sees that all boundary edges are quadratic ones,
         it generates quadratic faces, else it generates linear faces using
         medium nodes as if they were vertex ones.
         The 3D mesher generates quadratic volumes only if all boundary faces
         are quadratic ones, else it fails.
        """
        hyp = self.Hypothesis("QuadraticMesh")
        return hyp

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

    def QuadranglePreference(self):
        """
         Define "QuadranglePreference" hypothesis, forcing construction
         of quadrangles if the number of nodes on opposite edges is not the same
         in the case where the global number of nodes on edges is even
        """
        hyp = self.Hypothesis("QuadranglePreference")
        return hyp

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

    def MeshDimension(self):
        """
        Returns mesh dimension depending on shape one
        """
        shells = geompy.SubShapeAllIDs( self.geom, geompy.ShapeType["SHELL"] )
        if len( shells ) > 0 :
            return 3
        elif geompy.NumberOfFaces( self.geom ) > 0 :
            return 2
        elif geompy.NumberOfEdges( self.geom ) > 0 :
            return 1
        else:
            return 0;
        pass

    def Segment(self, algo=REGULAR, geom=0):
        """
         Creates a segment discretization 1D algorithm.
         If the optional \a algo parameter is not sets, this algorithm is REGULAR.
         If the optional \a geom parameter is not sets, this algorithm is global.
         Otherwise, this algorithm define a submesh based on \a geom subshape.
         \param algo values are smesh.REGULAR or smesh.PYTHON for discretization via python function
         \param geom If defined, subshape to be meshed
        """
        ## if Segment(geom) is called by mistake
        if ( isinstance( algo, geompy.GEOM._objref_GEOM_Object)):
            algo, geom = geom, algo
            pass
        if algo == REGULAR:
            return Mesh_Segment(self, geom)
        elif algo == PYTHON:
            return Mesh_Segment_Python(self, geom)
        else:
            return Mesh_Segment(self, geom)

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
        ## if Tetrahedron(geom) is called by mistake
        if ( isinstance( algo, geompy.GEOM._objref_GEOM_Object)):
            algo, geom = geom, algo
            pass
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
        ok = smesh.Compute(self.mesh, self.geom)
        if not ok:
            errors = smesh.GetAlgoState( self.mesh, self.geom )
            allReasons = ""
            for err in errors:
                if err.isGlobalAlgo:
                    glob = " global "
                else:
                    glob = " local "
                    pass
                dim = str(err.algoDim)
                if err.name == SMESH.MISSING_ALGO:
                    reason = glob + dim + "D algorithm is missing"
                elif err.name == SMESH.MISSING_HYPO:
                    name = '"' + err.algoName + '"'
                    reason = glob + dim + "D algorithm " + name + " misses " + dim + "D hypothesis"
                else:
                    reason = "Global \"Not Conform mesh allowed\" hypothesis is missing"
                    pass
                if allReasons != "":
                    allReasons += "\n"
                    pass
                allReasons += reason
                pass
            if allReasons != "":
                print '"' + GetName(self.mesh) + '"',"not computed:"
                print allReasons
                pass
            pass
        if salome.sg.hasDesktop():
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(salome.myStudyId)
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), ok )
            salome.sg.updateObjBrowser(1)
            pass
        return ok

    def AutomaticTetrahedralization(self, fineness=0):
        """
        Compute tetrahedral mesh using AutomaticLength + MEFISTO + NETGEN
        The parameter \a fineness [0.-1.] defines mesh fineness
        """
        dim = self.MeshDimension()
        # assign hypotheses
        self.RemoveGlobalHypotheses()
        self.Segment().AutomaticLength(fineness)
        if dim > 1 :
            self.Triangle().LengthFromEdges()
            pass
        if dim > 2 :
            self.Tetrahedron(NETGEN)
            pass
        return self.Compute()

    def AutomaticHexahedralization(self, fineness=0):
        """
        Compute hexahedral mesh using AutomaticLength + Quadrangle + Hexahedron
        The parameter \a fineness [0.-1.] defines mesh fineness
        """
        dim = self.MeshDimension()
        # assign hypotheses
        self.RemoveGlobalHypotheses()
        self.Segment().AutomaticLength(fineness)
        if dim > 1 :
            self.Quadrangle()
            pass
        if dim > 2 :
            self.Hexahedron()            
            pass
        return self.Compute()

    def RemoveGlobalHypotheses(self):
        """
        Removes all global hypotheses
        """
        current_hyps = self.mesh.GetHypothesisList( self.geom )
        for hyp in current_hyps:
            self.mesh.RemoveHypothesis( self.geom, hyp )
            pass
        pass

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
        elif tgeo == "SHELL":
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
         \param version values are SMESH.MED_V2_1, SMESH.MED_V2_2
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
