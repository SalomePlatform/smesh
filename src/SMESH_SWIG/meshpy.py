# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

import geompy

import salome

import StdMeshers

import SMESH

# Variables
# ---------

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)
smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

# Hexahedrical meshing
#
# Examples: cube2pyGibi.py, lines 270-295
#           cube2partition.py, lines 72-83
# --------------------

class MeshHexaImpl:
    piece = 0
    name  = 0
    mesh  = 0
    cpt   = 0

    # Sets algorithm and hypothesis for 1D discretization of the <shape>:
    #   - algorithm  "Regular_1D"
    #   - hypothesis "NumberOfSegments" with number of segments <n>
    # --------------------

    def Mesh1D(self, shape, n):
        hyp1D=smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
        smeshgui.SetName(salome.ObjectToID(hyp1D), self.name+"/WireDiscretisation/"+str(self.cpt))
        self.mesh.AddHypothesis(shape, hyp1D)

        hyp=smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
        hyp.SetNumberOfSegments(n)
        smeshgui.SetName(salome.ObjectToID(hyp), self.name+"/Segments_"+str(n)+"/"+str(self.cpt))
        self.mesh.AddHypothesis(shape, hyp)

        self.cpt=self.cpt+1

    # Constructor
    #
    # Creates mesh on the shape <piece>,
    # sets GUI name of this mesh to <name>.
    # Sets the following global algorithms and hypotheses:
    #   - for 1D discretization:
    #       - algorithm  "Regular_1D"
    #       - hypothesis "NumberOfSegments" with number of segments <n>,
    #   - for 2D discretization:
    #       - algorithm  "Quadrangle_2D"
    #   - for 3D discretization:
    #       - algorithm  "Hexa_3D"
    # --------------------

    def __init__(self, piece, n, name):
        self.piece = piece
        self.name  = name

        self.mesh  = smesh.CreateMesh(piece)
        smeshgui.SetName(salome.ObjectToID(self.mesh), name)

        self.Mesh1D(piece, n)

        hyp2D=smesh.CreateHypothesis("Quadrangle_2D", "libStdMeshersEngine.so")
        smeshgui.SetName(salome.ObjectToID(hyp2D), name+"/Quadrangle")
        self.mesh.AddHypothesis(piece, hyp2D)

        hyp3D=smesh.CreateHypothesis("Hexa_3D", "libStdMeshersEngine.so")
        smeshgui.SetName(salome.ObjectToID(hyp3D), name+"/ijk")
        self.mesh.AddHypothesis(piece, hyp3D)

    # Creates sub-mesh of the mesh, created by constructor.
    # This sub-mesh will be created on edge <edge>.
    # Set algorithm and hypothesis for 1D discretization of the <edge>:
    #   - algorithm  "Regular_1D"
    #   - hypothesis "NumberOfSegments" with number of segments <n>
    # Note: the <edge> will be automatically published in study under the shape, given in constructor.
    # --------------------

    def local(self, edge, n):
        geompy.addToStudyInFather(self.piece, edge, geompy.SubShapeName(edge, self.piece))
        submesh = self.mesh.GetSubMesh(edge, self.name+"/SubMeshEdge/"+str(self.cpt))
        self.Mesh1D(edge, n)

    # Computes mesh, created by constructor.
    # --------------------

    def Compute(self):
        smesh.Compute(self.mesh, self.piece)
        salome.sg.updateObjBrowser(1)

    # Creates mesh group based on a geometric group
    # --------------------

    def Group(self, grp, name=""):
        if name == "":
            name = grp.GetName()
        tgeo = geompy.GroupOp.GetType(grp)
        if tgeo == geompy.ShapeType["VERTEX"]:
            type = SMESH.NODE
        elif tgeo == geompy.ShapeType["EDGE"]:
            type = SMESH.EDGE
        elif tgeo == geompy.ShapeType["FACE"]:
            type = SMESH.FACE
        elif tgeo == geompy.ShapeType["SOLID"]:
            type = SMESH.VOLUME
        return self.mesh.CreateGroupFromGEOM(type, name, grp)

    # Export mesh in a MED file
    # --------------------

    def ExportMED(self, filename, groups=1):
        self.mesh.ExportMED(filename, groups)

MeshHexa = MeshHexaImpl
