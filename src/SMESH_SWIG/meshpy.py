# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

import geompy
import salome

import StdMeshers
import SMESH

"""
 \namespace meshpy
 \brief Module meshpy
"""

# Variables
# ---------

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)
smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

class MeshHexaImpl:
"""
 Class MeshHexaImpl for Hexahedrical meshing

 Examples: cube2pyGibi.py, lines 270-295
           cube2partition.py, lines 72-83
"""
    piece = 0
    name  = 0
    mesh  = 0
    cpt   = 0

    def Mesh1D(self, shape, n, propagate=0):
    """
     Define Wires discretization.
     Sets algorithm and hypothesis for 1D discretization of \a shape:
       - algorithm  "Regular_1D"
       - hypothesis "NumberOfSegments" with number of segments \a n
     \param shape Main shape or sub-shape to define wire discretization of
     \param n Number of segments to split eash wire of the \a shape on
     \param propagate Boolean flag. If propagate = 1,
            "Propagation" hypothesis will be applied also to the \a shape
    """
        hyp1D=smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
        smeshgui.SetName(salome.ObjectToID(hyp1D), self.name+"/WireDiscretisation/"+str(self.cpt))
        self.mesh.AddHypothesis(shape, hyp1D)

        hyp=smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
        hyp.SetNumberOfSegments(n)
        smeshgui.SetName(salome.ObjectToID(hyp), self.name+"/Segments_"+str(n)+"/"+str(self.cpt))
        self.mesh.AddHypothesis(shape, hyp)

        if propagate:
            hypPro=smesh.CreateHypothesis("Propagation", "libStdMeshersEngine.so")
            smeshgui.SetName(salome.ObjectToID(hypPro), self.name+"/Propagation/"+str(self.cpt))
            self.mesh.AddHypothesis(shape, hypPro)

        self.cpt=self.cpt+1

    def __init__(self, piece, n, name):
    """
     Constructor

     Creates mesh on the shape \a piece,
     sets GUI name of this mesh to \a name. \n
     Sets the following global algorithms and hypotheses:
       - for 1D discretization:
           - algorithm  "Regular_1D"
           - hypothesis "NumberOfSegments" with number of segments \a n
       - for 2D discretization:
           - algorithm  "Quadrangle_2D"
       - for 3D discretization:
           - algorithm  "Hexa_3D"
     \param piece Shape to be meshed
     \param n Global number of segments for wires discretization
     \param name Name for mesh to be created
    """
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

    def local(self, edge, n):
    """
     Creates sub-mesh of the mesh, created by constructor.
     This sub-mesh will be created on edge \a edge.
     Set algorithm and hypothesis for 1D discretization of the \a edge:
       - algorithm  "Regular_1D"
       - hypothesis "NumberOfSegments" with number of segments \a n
     \param edge Sub-edge of the main shape
     \param n Number of segments to split the \a edge on
     \note: \a edge will be automatically published in study under the shape, given in constructor.
    """
        geompy.addToStudyInFather(self.piece, edge, geompy.SubShapeName(edge, self.piece))
        submesh = self.mesh.GetSubMesh(edge, self.name+"/SubMeshEdge/"+str(self.cpt))
        self.Mesh1D(edge, n)

    def Propagate(self, edge, n):
    """
     Creates sub-mesh of the mesh, created by constructor.
     This sub-mesh will be created on edge \a edge and propagate the hypothesis on all correspondant edges.
     Set algorithm and hypothesis for 1D discretization of the \a edge and all other propagate edges:
       - algorithm  "Regular_1D"
       - hypothesis "NumberOfSegments" with number of segments \a n
       - hypothesis "Propagation"
     \param edge Sub-edge of the main shape
     \param n Number of segments to split the \a edge and all other propagate edges on
     \note: \a edge will be automatically published in study under the shape, given in constructor.
    """
        geompy.addToStudyInFather(self.piece, edge, geompy.SubShapeName(edge, self.piece))
        submesh = self.mesh.GetSubMesh(edge, self.name+"/SubMeshEdge/"+str(self.cpt))
        self.Mesh1D(edge, n, 1)

    def Compute(self):
    """
     Computes mesh, created by constructor.
    """
        smesh.Compute(self.mesh, self.piece)
        salome.sg.updateObjBrowser(1)

    def Group(self, grp, name=""):
    """
     Creates mesh group based on a geometric group
     \param grp Geometric group
     \param name Name for mesh group to be created
    """
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

    def ExportMED(self, filename, groups=1):
    """
     Export mesh in a MED file
     \param filename Name for MED file to be created
     \param groups Boolean flag. If groups = 1, mesh groups will be also stored in file
    """
        self.mesh.ExportMED(filename, groups)

MeshHexa = MeshHexaImpl
