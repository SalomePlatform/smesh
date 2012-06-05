# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

from smesh import Mesh_Algorithm, AssureGeomPublished, IsEqual, ParseParameters
from smesh import GetName, TreatHypoStatus
from smeshDC import Mesh

import StdMeshers

# Types of algorithms
REGULAR     = "Regular_1D"
PYTHON      = "Python_1D"
COMPOSITE   = "CompositeSegment_1D"
MEFISTO     = "MEFISTO_2D"
Hexa        = "Hexa_3D"
QUADRANGLE  = "Quadrangle_2D"
RADIAL_QUAD = "RadialQuadrangle_1D2D"


# import items of enum QuadType
for e in StdMeshers.QuadType._items: exec('%s = StdMeshers.%s'%(e,e))


# Public class: Mesh_Segment
# --------------------------

## Class to define a REGULAR 1D algorithm for discretization. It is created by
#  calling Mesh.Segment(geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_Segment(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Segment"
    ## Name of algorithm type
    algoType   = REGULAR
    isDefault  = True

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)

    ## Defines "LocalLength" hypothesis to cut an edge in several segments with the same length
    #  @param l for the length of segments that cut an edge
    #  @param UseExisting if ==true - searches for an  existing hypothesis created with
    #                    the same parameters, else (default) - creates a new one
    #  @param p precision, used for calculation of the number of segments.
    #           The precision should be a positive, meaningful value within the range [0,1].
    #           In general, the number of segments is calculated with the formula:
    #           nb = ceil((edge_length / l) - p)
    #           Function ceil rounds its argument to the higher integer.
    #           So, p=0 means rounding of (edge_length / l) to the higher integer,
    #               p=0.5 means rounding of (edge_length / l) to the nearest integer,
    #               p=1 means rounding of (edge_length / l) to the lower integer.
    #           Default value is 1e-07.
    #  @return an instance of StdMeshers_LocalLength hypothesis
    #  @ingroup l3_hypos_1dhyps
    def LocalLength(self, l, UseExisting=0, p=1e-07):
        comFun=lambda hyp, args: IsEqual(hyp.GetLength(), args[0]) and IsEqual(hyp.GetPrecision(), args[1])
        hyp = self.Hypothesis("LocalLength", [l,p], UseExisting=UseExisting, CompareMethod=comFun)
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    ## Defines "MaxSize" hypothesis to cut an edge into segments not longer than given value
    #  @param length is optional maximal allowed length of segment, if it is omitted
    #                the preestimated length is used that depends on geometry size
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @return an instance of StdMeshers_MaxLength hypothesis
    #  @ingroup l3_hypos_1dhyps
    def MaxSize(self, length=0.0, UseExisting=0):
        hyp = self.Hypothesis("MaxLength", [length], UseExisting=UseExisting)
        if length > 0.0:
            # set given length
            hyp.SetLength(length)
        if not UseExisting:
            # set preestimated length
            gen = self.mesh.smeshpyD
            initHyp = gen.GetHypothesisParameterValues("MaxLength", "libStdMeshersEngine.so",
                                                       self.mesh.GetMesh(), self.mesh.GetShape(),
                                                       False) # <- byMesh
            preHyp = initHyp._narrow(StdMeshers.StdMeshers_MaxLength)
            if preHyp:
                hyp.SetPreestimatedLength( preHyp.GetPreestimatedLength() )
                pass
            pass
        hyp.SetUsePreestimatedLength( length == 0.0 )
        return hyp

    ## Defines "NumberOfSegments" hypothesis to cut an edge in a fixed number of segments
    #  @param n for the number of segments that cut an edge
    #  @param s for the scale factor (optional)
    #  @param reversedEdges is a list of edges to mesh using reversed orientation.
    #                       A list item can also be a tuple (edge 1st_vertex_of_edge)
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - create a new one
    #  @return an instance of StdMeshers_NumberOfSegments hypothesis
    #  @ingroup l3_hypos_1dhyps
    def NumberOfSegments(self, n, s=[], reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        entry = self.MainShapeEntry()
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        if s == []:
            hyp = self.Hypothesis("NumberOfSegments", [n, reversedEdgeInd, entry],
                                  UseExisting=UseExisting,
                                  CompareMethod=self._compareNumberOfSegments)
        else:
            hyp = self.Hypothesis("NumberOfSegments", [n,s, reversedEdgeInd, entry],
                                  UseExisting=UseExisting,
                                  CompareMethod=self._compareNumberOfSegments)
            hyp.SetDistrType( 1 )
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        hyp.SetReversedEdges( reversedEdgeInd )
        hyp.SetObjectEntry( entry )
        return hyp

    ## Private method
    ## Checks if the given "NumberOfSegments" hypothesis has the same parameters as the given arguments
    def _compareNumberOfSegments(self, hyp, args):
        if hyp.GetNumberOfSegments() == args[0]:
            if len(args) == 3:
                if hyp.GetReversedEdges() == args[1]:
                    if not args[1] or hyp.GetObjectEntry() == args[2]:
                        return True
            else:
                if hyp.GetReversedEdges() == args[2]:
                    if not args[2] or hyp.GetObjectEntry() == args[3]:
                        if hyp.GetDistrType() == 1:
                            if IsEqual(hyp.GetScaleFactor(), args[1]):
                                return True
        return False

    ## Defines "Arithmetic1D" hypothesis to cut an edge in several segments with increasing arithmetic length
    #  @param start defines the length of the first segment
    #  @param end   defines the length of the last  segment
    #  @param reversedEdges is a list of edges to mesh using reversed orientation.
    #                       A list item can also be a tuple (edge 1st_vertex_of_edge)
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @return an instance of StdMeshers_Arithmetic1D hypothesis
    #  @ingroup l3_hypos_1dhyps
    def Arithmetic1D(self, start, end, reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        entry = self.MainShapeEntry()
        compFun = lambda hyp, args: ( IsEqual(hyp.GetLength(1), args[0]) and \
                                      IsEqual(hyp.GetLength(0), args[1]) and \
                                      hyp.GetReversedEdges() == args[2]  and \
                                      (not args[2] or hyp.GetObjectEntry() == args[3]))
        hyp = self.Hypothesis("Arithmetic1D", [start, end, reversedEdgeInd, entry],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetStartLength(start)
        hyp.SetEndLength(end)
        hyp.SetReversedEdges( reversedEdgeInd )
        hyp.SetObjectEntry( entry )
        return hyp

    ## Defines "FixedPoints1D" hypothesis to cut an edge using parameter
    # on curve from 0 to 1 (additionally it is neecessary to check
    # orientation of edges and create list of reversed edges if it is
    # needed) and sets numbers of segments between given points (default
    # values are equals 1
    #  @param points defines the list of parameters on curve
    #  @param nbSegs defines the list of numbers of segments
    #  @param reversedEdges is a list of edges to mesh using reversed orientation.
    #                       A list item can also be a tuple (edge 1st_vertex_of_edge)
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @return an instance of StdMeshers_Arithmetic1D hypothesis
    #  @ingroup l3_hypos_1dhyps
    def FixedPoints1D(self, points, nbSegs=[1], reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        entry = self.MainShapeEntry()
        compFun = lambda hyp, args: ( hyp.GetPoints() == args[0] and \
                                      hyp.GetNbSegments() == args[1] and \
                                      hyp.GetReversedEdges() == args[2] and \
                                      (not args[2] or hyp.GetObjectEntry() == args[3]))
        hyp = self.Hypothesis("FixedPoints1D", [points, nbSegs, reversedEdgeInd, entry],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetPoints(points)
        hyp.SetNbSegments(nbSegs)
        hyp.SetReversedEdges(reversedEdgeInd)
        hyp.SetObjectEntry(entry)
        return hyp

    ## Defines "StartEndLength" hypothesis to cut an edge in several segments with increasing geometric length
    #  @param start defines the length of the first segment
    #  @param end   defines the length of the last  segment
    #  @param reversedEdges is a list of edges to mesh using reversed orientation.
    #                       A list item can also be a tuple (edge 1st_vertex_of_edge)
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @return an instance of StdMeshers_StartEndLength hypothesis
    #  @ingroup l3_hypos_1dhyps
    def StartEndLength(self, start, end, reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        entry = self.MainShapeEntry()
        compFun = lambda hyp, args: ( IsEqual(hyp.GetLength(1), args[0]) and \
                                      IsEqual(hyp.GetLength(0), args[1]) and \
                                      hyp.GetReversedEdges() == args[2]  and \
                                      (not args[2] or hyp.GetObjectEntry() == args[3]))
        hyp = self.Hypothesis("StartEndLength", [start, end, reversedEdgeInd, entry],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetStartLength(start)
        hyp.SetEndLength(end)
        hyp.SetReversedEdges( reversedEdgeInd )
        hyp.SetObjectEntry( entry )
        return hyp

    ## Defines "Deflection1D" hypothesis
    #  @param d for the deflection
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - create a new one
    #  @ingroup l3_hypos_1dhyps
    def Deflection1D(self, d, UseExisting=0):
        compFun = lambda hyp, args: IsEqual(hyp.GetDeflection(), args[0])
        hyp = self.Hypothesis("Deflection1D", [d], UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetDeflection(d)
        return hyp

    ## Defines "Propagation" hypothesis that propagates all other hypotheses on all other edges that are at
    #  the opposite side in case of quadrangular faces
    #  @ingroup l3_hypos_additi
    def Propagation(self):
        return self.Hypothesis("Propagation", UseExisting=1, CompareMethod=self.CompareEqualHyp)

    ## Defines "AutomaticLength" hypothesis
    #  @param fineness for the fineness [0-1]
    #  @param UseExisting if ==true - searches for an existing hypothesis created with the
    #                     same parameters, else (default) - create a new one
    #  @ingroup l3_hypos_1dhyps
    def AutomaticLength(self, fineness=0, UseExisting=0):
        compFun = lambda hyp, args: IsEqual(hyp.GetFineness(), args[0])
        hyp = self.Hypothesis("AutomaticLength",[fineness],UseExisting=UseExisting,
                              CompareMethod=compFun)
        hyp.SetFineness( fineness )
        return hyp

    ## Defines "SegmentLengthAroundVertex" hypothesis
    #  @param length for the segment length
    #  @param vertex for the length localization: the vertex index [0,1] | vertex object.
    #         Any other integer value means that the hypothesis will be set on the
    #         whole 1D shape, where Mesh_Segment algorithm is assigned.
    #  @param UseExisting if ==true - searches for an  existing hypothesis created with
    #                   the same parameters, else (default) - creates a new one
    #  @ingroup l3_algos_segmarv
    def LengthNearVertex(self, length, vertex=0, UseExisting=0):
        import types
        store_geom = self.geom
        if type(vertex) is types.IntType:
            if vertex == 0 or vertex == 1:
                import geompyDC
                vertex = self.mesh.geompyD.ExtractShapes(self.geom, geompyDC.ShapeType["VERTEX"],True)[vertex]
                self.geom = vertex
                pass
            pass
        else:
            self.geom = vertex
            pass
        ### 0D algorithm
        if self.geom is None:
            raise RuntimeError, "Attemp to create SegmentAroundVertex_0D algoritm on None shape"
        AssureGeomPublished( self.mesh, self.geom )
        name = GetName(self.geom)

        algo = self.FindAlgorithm("SegmentAroundVertex_0D", self.mesh.smeshpyD)
        if algo is None:
            algo = self.mesh.smeshpyD.CreateHypothesis("SegmentAroundVertex_0D", "libStdMeshersEngine.so")
            pass
        status = self.mesh.mesh.AddHypothesis(self.geom, algo)
        TreatHypoStatus(status, "SegmentAroundVertex_0D", name, True)
        ###
        comFun = lambda hyp, args: IsEqual(hyp.GetLength(), args[0])
        hyp = self.Hypothesis("SegmentLengthAroundVertex", [length], UseExisting=UseExisting,
                              CompareMethod=comFun)
        self.geom = store_geom
        hyp.SetLength( length )
        return hyp

    ## Defines "QuadraticMesh" hypothesis, forcing construction of quadratic edges.
    #  If the 2D mesher sees that all boundary edges are quadratic,
    #  it generates quadratic faces, else it generates linear faces using
    #  medium nodes as if they are vertices.
    #  The 3D mesher generates quadratic volumes only if all boundary faces
    #  are quadratic, else it fails.
    #
    #  @ingroup l3_hypos_additi
    def QuadraticMesh(self):
        hyp = self.Hypothesis("QuadraticMesh", UseExisting=1, CompareMethod=self.CompareEqualHyp)
        return hyp

# Public class: Mesh_CompositeSegment
# --------------------------

## A regular 1D algorithm for discretization of a set of adjacent edges as one.
#  It is created by calling Mesh.Segment(COMPOSITE,geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_CompositeSegment(StdMeshersDC_Segment):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Segment"
    ## Name of algorithm type
    algoType   = COMPOSITE
    isDefault  = False

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        self.Create(mesh, geom, self.algoType)


# Public class: Mesh_Segment_Python
# ---------------------------------

## Defines a segment 1D algorithm for discretization with python function
#  It is created by calling Mesh.Segment(PYTHON,geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_Segment_Python(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Segment"
    ## Name of algorithm type
    algoType   = PYTHON

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        import Python1dPlugin
        self.Create(mesh, geom, self.algoType, "libPython1dEngine.so")

    ## Defines "PythonSplit1D" hypothesis
    #  @param n for the number of segments that cut an edge
    #  @param func for the python function that calculates the length of all segments
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_1dhyps
    def PythonSplit1D(self, n, func, UseExisting=0):
        compFun = lambda hyp, args: False
        hyp = self.Hypothesis("PythonSplit1D", [n], "libPython1dEngine.so",
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetNumberOfSegments(n)
        hyp.SetPythonLog10RatioFunction(func)
        return hyp

# Public class: Mesh_Triangle_MEFISTO
# -----------------------------------

## Triangle MEFISTO 2D algorithm
#  It is created by calling Mesh.Triangle(MEFISTO,geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_Triangle_MEFISTO(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Triangle"
    ## Name of algorithm type
    algoType   = MEFISTO
    isDefault  = True

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)

    ## Defines "MaxElementArea" hypothesis basing on the definition of the maximum area of each triangle
    #  @param area for the maximum area of each triangle
    #  @param UseExisting if ==true - searches for an  existing hypothesis created with the
    #                     same parameters, else (default) - creates a new one
    #
    #  @ingroup l3_hypos_2dhyps
    def MaxElementArea(self, area, UseExisting=0):
        comparator = lambda hyp, args: IsEqual(hyp.GetMaxElementArea(), args[0])
        hyp = self.Hypothesis("MaxElementArea", [area], UseExisting=UseExisting,
                              CompareMethod=comparator)
        hyp.SetMaxElementArea(area)
        return hyp

    ## Defines "LengthFromEdges" hypothesis to build triangles
    #  based on the length of the edges taken from the wire
    #
    #  @ingroup l3_hypos_2dhyps
    def LengthFromEdges(self):
        hyp = self.Hypothesis("LengthFromEdges", UseExisting=1, CompareMethod=self.CompareEqualHyp)
        return hyp

# Public class: Mesh_Quadrangle
# -----------------------------

## Defines a quadrangle 2D algorithm
#  It is created by calling Mesh.Quadrangle(geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_Quadrangle(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Quadrangle"
    ## Name of algorithm type
    algoType   = QUADRANGLE
    isDefault  = True

    params=0

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        return

    ## Defines "QuadrangleParameters" hypothesis
    #  @param quadType defines the algorithm of transition between differently descretized
    #                  sides of a geometrical face:
    #  - QUAD_STANDARD - both triangles and quadrangles are possible in the transition
    #                    area along the finer meshed sides.
    #  - QUAD_TRIANGLE_PREF - only triangles are built in the transition area along the
    #                    finer meshed sides.
    #  - QUAD_QUADRANGLE_PREF - only quadrangles are built in the transition area along
    #                    the finer meshed sides, iff the total quantity of segments on
    #                    all four sides of the face is even (divisible by 2).
    #  - QUAD_QUADRANGLE_PREF_REVERSED - same as QUAD_QUADRANGLE_PREF but the transition
    #                    area is located along the coarser meshed sides.
    #  - QUAD_REDUCED - only quadrangles are built and the transition between the sides
    #                    is made gradually, layer by layer. This type has a limitation on
    #                    the number of segments: one pair of opposite sides must have the
    #                    same number of segments, the other pair must have an even difference
    #                    between the numbers of segments on the sides.
    #  @param triangleVertex: vertex of a trilateral geometrical face, around which triangles
    #                  will be created while other elements will be quadrangles.
    #                  Vertex can be either a GEOM_Object or a vertex ID within the
    #                  shape to mesh
    #  @param UseExisting: if ==true - searches for the existing hypothesis created with
    #                  the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_quad
    def QuadrangleParameters(self, quadType=StdMeshers.QUAD_STANDARD, triangleVertex=0, UseExisting=0):
        import GEOM
        vertexID = triangleVertex
        if isinstance( triangleVertex, GEOM._objref_GEOM_Object ):
            vertexID = self.mesh.geompyD.GetSubShapeID( self.mesh.geom, triangleVertex )
        if not self.params:
            compFun = lambda hyp,args: \
                      hyp.GetQuadType() == args[0] and \
                      ( hyp.GetTriaVertex()==args[1] or ( hyp.GetTriaVertex()<1 and args[1]<1))
            self.params = self.Hypothesis("QuadrangleParams", [quadType,vertexID],
                                          UseExisting = UseExisting, CompareMethod=compFun)
            pass
        if self.params.GetQuadType() != quadType:
            self.params.SetQuadType(quadType)
        if vertexID > 0:
            self.params.SetTriaVertex( vertexID )
        return self.params

    ## Defines "QuadrangleParams" hypothesis with a type of quadrangulation that only
    #   quadrangles are built in the transition area along the finer meshed sides,
    #   iff the total quantity of segments on all four sides of the face is even.
    #  @param reversed if True, transition area is located along the coarser meshed sides.
    #  @param UseExisting: if ==true - searches for the existing hypothesis created with
    #                  the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_quad
    def QuadranglePreference(self, reversed=False, UseExisting=0):
        if reversed:
            return self.QuadrangleParameters(QUAD_QUADRANGLE_PREF_REVERSED,UseExisting=UseExisting)
        return self.QuadrangleParameters(QUAD_QUADRANGLE_PREF,UseExisting=UseExisting)

    ## Defines "QuadrangleParams" hypothesis with a type of quadrangulation that only
    #   triangles are built in the transition area along the finer meshed sides.
    #  @param UseExisting: if ==true - searches for the existing hypothesis created with
    #                  the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_quad
    def TrianglePreference(self, UseExisting=0):
        return self.QuadrangleParameters(QUAD_TRIANGLE_PREF,UseExisting=UseExisting)

    ## Defines "QuadrangleParams" hypothesis with a type of quadrangulation that only
    #   quadrangles are built and the transition between the sides is made gradually,
    #   layer by layer. This type has a limitation on the number of segments: one pair
    #   of opposite sides must have the same number of segments, the other pair must
    #   have an even difference between the numbers of segments on the sides.
    #  @param UseExisting: if ==true - searches for the existing hypothesis created with
    #                  the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_quad
    def Reduced(self, UseExisting=0):
        return self.QuadrangleParameters(QUAD_REDUCED,UseExisting=UseExisting)

    ## Defines "QuadrangleParams" hypothesis with QUAD_STANDARD type of quadrangulation
    #  @param vertex: vertex of a trilateral geometrical face, around which triangles
    #                 will be created while other elements will be quadrangles.
    #                 Vertex can be either a GEOM_Object or a vertex ID within the
    #                 shape to mesh
    #  @param UseExisting: if ==true - searches for the existing hypothesis created with
    #                   the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_quad
    def TriangleVertex(self, vertex, UseExisting=0):
        return self.QuadrangleParameters(QUAD_STANDARD,vertex,UseExisting)


# Public class: Mesh_Hexahedron
# ------------------------------

## Defines a hexahedron 3D algorithm
#  It is created by calling Mesh.Hexahedron(geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_Hexahedron(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Hexahedron"
    ## Name of algorithm type
    algoType   = Hexa
    isDefault  = True

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, Hexa)
        pass

# Public class: Mesh_Projection1D
# -------------------------------

## Defines a projection 1D algorithm
#  It is created by calling Mesh.Projection1D(geom=0)
#  @ingroup l3_algos_proj
#
class StdMeshersDC_Projection1D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Projection1D"
    ## Name of algorithm type
    algoType   = "Projection_1D"
    isDefault  = True

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)

    ## Defines "Source Edge" hypothesis, specifying a meshed edge, from where
    #  a mesh pattern is taken, and, optionally, the association of vertices
    #  between the source edge and a target edge (to which a hypothesis is assigned)
    #  @param edge from which nodes distribution is taken
    #  @param mesh from which nodes distribution is taken (optional)
    #  @param srcV a vertex of \a edge to associate with \a tgtV (optional)
    #  @param tgtV a vertex of \a the edge to which the algorithm is assigned,
    #  to associate with \a srcV (optional)
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def SourceEdge(self, edge, mesh=None, srcV=None, tgtV=None, UseExisting=0):
        AssureGeomPublished( self.mesh, edge )
        AssureGeomPublished( self.mesh, srcV )
        AssureGeomPublished( self.mesh, tgtV )
        hyp = self.Hypothesis("ProjectionSource1D", [edge,mesh,srcV,tgtV],
                              UseExisting=0)
        # it does not seem to be useful to reuse the existing "SourceEdge" hypothesis
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceEdge)
        hyp.SetSourceEdge( edge )
        if not mesh is None and isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV, tgtV )
        return hyp


# Public class: Mesh_Projection2D
# ------------------------------

## Defines a projection 2D algorithm
#  It is created by calling Mesh.Projection2D(geom=0)
#  @ingroup l3_algos_proj
#
class StdMeshersDC_Projection2D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Projection2D"
    ## Name of algorithm type
    algoType   = "Projection_2D"
    isDefault  = True

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)

    ## Defines "Source Face" hypothesis, specifying a meshed face, from where
    #  a mesh pattern is taken, and, optionally, the association of vertices
    #  between the source face and the target face (to which a hypothesis is assigned)
    #  @param face from which the mesh pattern is taken
    #  @param mesh from which the mesh pattern is taken (optional)
    #  @param srcV1 a vertex of \a face to associate with \a tgtV1 (optional)
    #  @param tgtV1 a vertex of \a the face to which the algorithm is assigned,
    #               to associate with \a srcV1 (optional)
    #  @param srcV2 a vertex of \a face to associate with \a tgtV1 (optional)
    #  @param tgtV2 a vertex of \a the face to which the algorithm is assigned,
    #               to associate with \a srcV2 (optional)
    #  @param UseExisting if ==true - forces the search for the existing hypothesis created with
    #                     the same parameters, else (default) - forces the creation a new one
    #
    #  Note: all association vertices must belong to one edge of a face
    def SourceFace(self, face, mesh=None, srcV1=None, tgtV1=None,
                   srcV2=None, tgtV2=None, UseExisting=0):
        from smeshDC import Mesh
        if isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        for geom in [ face, srcV1, tgtV1, srcV2, tgtV2 ]:
            AssureGeomPublished( self.mesh, geom )
        hyp = self.Hypothesis("ProjectionSource2D", [face,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=0)
        # it does not seem to be useful to reuse the existing "SourceFace" hypothesis
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceFace)
        hyp.SetSourceFace( face )
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        return hyp

# Public class: Mesh_Projection1D2D
# ---------------------------------

## Defines a projection 1D-2D algorithm
#  It is created by calling Mesh.Projection1D2D(geom=0)
#
#  @ingroup l3_algos_proj

class StdMeshersDC_Projection1D2D(StdMeshersDC_Projection2D):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Projection1D2D"
    ## Name of algorithm type
    algoType   = "Projection_1D2D"

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        StdMeshersDC_Projection2D.__init__(self, mesh, geom)

# Public class: Mesh_Projection3D
# ------------------------------

## Defines a projection 3D algorithm
#  It is created by calling Mesh.Projection3D(COMPOSITE)
#
#  @ingroup l3_algos_proj
#
class StdMeshersDC_Projection3D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Projection3D"
    ## Name of algorithm type
    algoType   = "Projection_3D"

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)

    ## Defines the "Source Shape 3D" hypothesis, specifying a meshed solid, from where
    #  the mesh pattern is taken, and, optionally, the  association of vertices
    #  between the source and the target solid  (to which a hipothesis is assigned)
    #  @param solid from where the mesh pattern is taken
    #  @param mesh from where the mesh pattern is taken (optional)
    #  @param srcV1 a vertex of \a solid to associate with \a tgtV1 (optional)
    #  @param tgtV1 a vertex of \a the solid where the algorithm is assigned,
    #  to associate with \a srcV1 (optional)
    #  @param srcV2 a vertex of \a solid to associate with \a tgtV1 (optional)
    #  @param tgtV2 a vertex of \a the solid to which the algorithm is assigned,
    #  to associate with \a srcV2 (optional)
    #  @param UseExisting - if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #
    #  Note: association vertices must belong to one edge of a solid
    def SourceShape3D(self, solid, mesh=0, srcV1=0, tgtV1=0,
                      srcV2=0, tgtV2=0, UseExisting=0):
        for geom in [ solid, srcV1, tgtV1, srcV2, tgtV2 ]:
            AssureGeomPublished( self.mesh, geom )
        hyp = self.Hypothesis("ProjectionSource3D",
                              [solid,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=0)
        # seems to be not really useful to reuse existing "SourceShape3D" hypothesis
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceShape3D)
        hyp.SetSource3DShape( solid )
        if isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        if mesh:
            hyp.SetSourceMesh( mesh )
        if srcV1 and srcV2 and tgtV1 and tgtV2:
            hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        #elif srcV1 or srcV2 or tgtV1 or tgtV2:
        return hyp

# Public class: Mesh_Prism
# ------------------------

## Defines a Prism 3D algorithm, which is either "Extrusion 3D" or "Radial Prism"
#  depending on geometry
#  It is created by calling Mesh.Prism(geom=0)
#
#  @ingroup l3_algos_3dextr
#
class StdMeshersDC_Prism3D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Prism"
    ## Name of algorithm type
    algoType   = "Prism_3D"

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        
        shape = geom
        if not shape:
            shape = mesh.geom
        from geompy import SubShapeAll, ShapeType
        nbSolids = len( SubShapeAll( shape, ShapeType["SOLID"] ))
        nbShells = len( SubShapeAll( shape, ShapeType["SHELL"] ))
        if nbSolids == 0 or nbSolids == nbShells:
            self.Create(mesh, geom, "Prism_3D")
        else:
            self.algoType = "RadialPrism_3D"
            self.Create(mesh, geom, "RadialPrism_3D")
            self.distribHyp = self.Hypothesis("LayerDistribution", UseExisting=0)
            self.nbLayers = None

    ## Return 3D hypothesis holding the 1D one
    def Get3DHypothesis(self):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        return self.distribHyp

    ## Private method creating a 1D hypothesis and storing it in the LayerDistribution
    #  hypothesis. Returns the created hypothesis
    def OwnHypothesis(self, hypType, args=[], so="libStdMeshersEngine.so"):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        if not self.nbLayers is None:
            self.mesh.GetMesh().RemoveHypothesis( self.geom, self.nbLayers )
            self.mesh.GetMesh().AddHypothesis( self.geom, self.distribHyp )
        study = self.mesh.smeshpyD.GetCurrentStudy() # prevents publishing own 1D hypothesis
        self.mesh.smeshpyD.SetCurrentStudy( None )
        hyp = self.mesh.smeshpyD.CreateHypothesis(hypType, so)
        self.mesh.smeshpyD.SetCurrentStudy( study ) # enables publishing
        self.distribHyp.SetLayerDistribution( hyp )
        return hyp

    ## Defines "NumberOfLayers" hypothesis, specifying the number of layers of
    #  prisms to build between the inner and outer shells
    #  @param n number of layers
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def NumberOfLayers(self, n, UseExisting=0):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        self.mesh.RemoveHypothesis( self.distribHyp, self.geom )
        compFun = lambda hyp, args: IsEqual(hyp.GetNumberOfLayers(), args[0])
        self.nbLayers = self.Hypothesis("NumberOfLayers", [n], UseExisting=UseExisting,
                                        CompareMethod=compFun)
        self.nbLayers.SetNumberOfLayers( n )
        return self.nbLayers

    ## Defines "LocalLength" hypothesis, specifying the segment length
    #  to build between the inner and the outer shells
    #  @param l the length of segments
    #  @param p the precision of rounding
    def LocalLength(self, l, p=1e-07):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        hyp = self.OwnHypothesis("LocalLength", [l,p])
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    ## Defines "NumberOfSegments" hypothesis, specifying the number of layers of
    #  prisms to build between the inner and the outer shells.
    #  @param n the number of layers
    #  @param s the scale factor (optional)
    def NumberOfSegments(self, n, s=[]):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        if s == []:
            hyp = self.OwnHypothesis("NumberOfSegments", [n])
        else:
            hyp = self.OwnHypothesis("NumberOfSegments", [n,s])
            hyp.SetDistrType( 1 )
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        return hyp

    ## Defines "Arithmetic1D" hypothesis, specifying the distribution of segments
    #  to build between the inner and the outer shells with a length that changes in arithmetic progression
    #  @param start  the length of the first segment
    #  @param end    the length of the last  segment
    def Arithmetic1D(self, start, end ):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        hyp = self.OwnHypothesis("Arithmetic1D", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    ## Defines "StartEndLength" hypothesis, specifying distribution of segments
    #  to build between the inner and the outer shells as geometric length increasing
    #  @param start for the length of the first segment
    #  @param end   for the length of the last  segment
    def StartEndLength(self, start, end):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        hyp = self.OwnHypothesis("StartEndLength", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    ## Defines "AutomaticLength" hypothesis, specifying the number of segments
    #  to build between the inner and outer shells
    #  @param fineness defines the quality of the mesh within the range [0-1]
    def AutomaticLength(self, fineness=0):
        if self.algoType != "RadialPrism_3D":
            print "Prism_3D algorith doesn't support any hyposesis"
            return None
        hyp = self.OwnHypothesis("AutomaticLength")
        hyp.SetFineness( fineness )
        return hyp


# Public class: Mesh_RadialQuadrangle1D2D
# -------------------------------

## Defines a Radial Quadrangle 1D2D algorithm
#  It is created by calling Mesh.Quadrangle(RADIAL_QUAD,geom=0)
#
#  @ingroup l2_algos_radialq
class StdMeshersDC_RadialQuadrangle1D2D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "Quadrangle"
    ## Name of algorithm type
    algoType   = RADIAL_QUAD

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)

        self.distribHyp = None #self.Hypothesis("LayerDistribution2D", UseExisting=0)
        self.nbLayers = None

    ## Return 2D hypothesis holding the 1D one
    def Get2DHypothesis(self):
        if not self.distribHyp:
            self.distribHyp = self.Hypothesis("LayerDistribution2D", UseExisting=0)
        return self.distribHyp

    ## Private method creating a 1D hypothesis and storing it in the LayerDistribution
    #  hypothesis. Returns the created hypothesis
    def OwnHypothesis(self, hypType, args=[], so="libStdMeshersEngine.so"):
        if self.nbLayers:
            self.mesh.GetMesh().RemoveHypothesis( self.geom, self.nbLayers )
        if self.distribHyp is None:
            self.distribHyp = self.Hypothesis("LayerDistribution2D", UseExisting=0)
        else:
            self.mesh.GetMesh().AddHypothesis( self.geom, self.distribHyp )
        study = self.mesh.smeshpyD.GetCurrentStudy() # prevents publishing own 1D hypothesis
        self.mesh.smeshpyD.SetCurrentStudy( None )
        hyp = self.mesh.smeshpyD.CreateHypothesis(hypType, so)
        self.mesh.smeshpyD.SetCurrentStudy( study ) # enables publishing
        self.distribHyp.SetLayerDistribution( hyp )
        return hyp

    ## Defines "NumberOfLayers" hypothesis, specifying the number of layers
    #  @param n number of layers
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def NumberOfLayers(self, n, UseExisting=0):
        if self.distribHyp:
            self.mesh.GetMesh().RemoveHypothesis( self.geom, self.distribHyp )
        compFun = lambda hyp, args: IsEqual(hyp.GetNumberOfLayers(), args[0])
        self.nbLayers = self.Hypothesis("NumberOfLayers2D", [n], UseExisting=UseExisting,
                                        CompareMethod=compFun)
        self.nbLayers.SetNumberOfLayers( n )
        return self.nbLayers

    ## Defines "LocalLength" hypothesis, specifying the segment length
    #  @param l the length of segments
    #  @param p the precision of rounding
    def LocalLength(self, l, p=1e-07):
        hyp = self.OwnHypothesis("LocalLength", [l,p])
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    ## Defines "NumberOfSegments" hypothesis, specifying the number of layers
    #  @param n the number of layers
    #  @param s the scale factor (optional)
    def NumberOfSegments(self, n, s=[]):
        if s == []:
            hyp = self.OwnHypothesis("NumberOfSegments", [n])
        else:
            hyp = self.OwnHypothesis("NumberOfSegments", [n,s])
            hyp.SetDistrType( 1 )
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        return hyp

    ## Defines "Arithmetic1D" hypothesis, specifying the distribution of segments
    #  with a length that changes in arithmetic progression
    #  @param start  the length of the first segment
    #  @param end    the length of the last  segment
    def Arithmetic1D(self, start, end ):
        hyp = self.OwnHypothesis("Arithmetic1D", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    ## Defines "StartEndLength" hypothesis, specifying distribution of segments
    #  as geometric length increasing
    #  @param start for the length of the first segment
    #  @param end   for the length of the last  segment
    def StartEndLength(self, start, end):
        hyp = self.OwnHypothesis("StartEndLength", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    ## Defines "AutomaticLength" hypothesis, specifying the number of segments
    #  @param fineness defines the quality of the mesh within the range [0-1]
    def AutomaticLength(self, fineness=0):
        hyp = self.OwnHypothesis("AutomaticLength")
        hyp.SetFineness( fineness )
        return hyp


# Public class: Mesh_UseExistingElements
# --------------------------------------
## Defines a Radial Quadrangle 1D2D algorithm
#  It is created by calling Mesh.UseExisting1DElements(geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_UseExistingElements_1D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "UseExisting1DElements"
    ## Name of algorithm type
    algoType   = "Import_1D"
    isDefault  = True

    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        return

    ## Defines "Source edges" hypothesis, specifying groups of edges to import
    #  @param groups list of groups of edges
    #  @param toCopyMesh if True, the whole mesh \a groups belong to is imported
    #  @param toCopyGroups if True, all groups of the mesh \a groups belong to are imported
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def SourceEdges(self, groups, toCopyMesh=False, toCopyGroups=False, UseExisting=False):
        for group in groups:
            AssureGeomPublished( self.mesh, group )
        compFun = lambda hyp, args: ( hyp.GetSourceEdges() == args[0] and \
                                      hyp.GetCopySourceMesh() == args[1], args[2] )
        hyp = self.Hypothesis("ImportSource1D", [groups, toCopyMesh, toCopyGroups],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetSourceEdges(groups)
        hyp.SetCopySourceMesh(toCopyMesh, toCopyGroups)
        return hyp

# Public class: Mesh_UseExistingElements
# --------------------------------------
## Defines a Radial Quadrangle 1D2D algorithm
#  It is created by calling Mesh.UseExisting2DElements(geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_UseExistingElements_1D2D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "UseExisting2DElements"
    ## Name of algorithm type
    algoType   = "Import_1D2D"
    isDefault  = True

    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        return

    ## Defines "Source faces" hypothesis, specifying groups of faces to import
    #  @param groups list of groups of faces
    #  @param toCopyMesh if True, the whole mesh \a groups belong to is imported
    #  @param toCopyGroups if True, all groups of the mesh \a groups belong to are imported
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def SourceFaces(self, groups, toCopyMesh=False, toCopyGroups=False, UseExisting=False):
        for group in groups:
            AssureGeomPublished( self.mesh, group )
        compFun = lambda hyp, args: ( hyp.GetSourceFaces() == args[0] and \
                                      hyp.GetCopySourceMesh() == args[1], args[2] )
        hyp = self.Hypothesis("ImportSource2D", [groups, toCopyMesh, toCopyGroups],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetSourceFaces(groups)
        hyp.SetCopySourceMesh(toCopyMesh, toCopyGroups)
        return hyp


# Public class: Mesh_Cartesian_3D
# --------------------------------------
## Defines a Body Fitting 3D algorithm
#  It is created by calling Mesh.BodyFitted(geom=0)
#
#  @ingroup l3_algos_basic
class StdMeshersDC_Cartesian_3D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "BodyFitted"
    ## Name of algorithm type
    algoType   = "Cartesian_3D"
    isDefault  = True

    def __init__(self, mesh, geom=0):
        self.Create(mesh, geom, self.algoType)
        self.hyp = None
        return

    ## Defines "Body Fitting parameters" hypothesis
    #  @param xGridDef is definition of the grid along the X asix.
    #  It can be in either of two following forms:
    #  - Explicit coordinates of nodes, e.g. [-1.5, 0.0, 3.1] or range( -100,200,10)
    #  - Functions f(t) defining grid spacing at each point on grid axis. If there are
    #    several functions, they must be accompanied by relative coordinates of
    #    points dividing the whole shape into ranges where the functions apply; points
    #    coodrinates should vary within (0.0, 1.0) range. Parameter \a t of the spacing
    #    function f(t) varies from 0.0 to 1.0 witin a shape range. 
    #    Examples:
    #    - "10.5" - defines a grid with a constant spacing
    #    - [["1", "1+10*t", "11"] [0.1, 0.6]] - defines different spacing in 3 ranges.
    #  @param yGridDef defines the grid along the Y asix the same way as \a xGridDef does
    #  @param zGridDef defines the grid along the Z asix the same way as \a xGridDef does
    #  @param sizeThreshold (> 1.0) defines a minimal size of a polyhedron so that
    #         a polyhedron of size less than hexSize/sizeThreshold is not created
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def SetGrid(self, xGridDef, yGridDef, zGridDef, sizeThreshold=4.0, UseExisting=False):
        if not self.hyp:
            compFun = lambda hyp, args: False
            self.hyp = self.Hypothesis("CartesianParameters3D",
                                       [xGridDef, yGridDef, zGridDef, sizeThreshold],
                                       UseExisting=UseExisting, CompareMethod=compFun)
        if not self.mesh.IsUsedHypothesis( self.hyp, self.geom ):
            self.mesh.AddHypothesis( self.hyp, self.geom )

        for axis, gridDef in enumerate( [xGridDef, yGridDef, zGridDef]):
            if not gridDef: raise ValueError, "Empty grid definition"
            if isinstance( gridDef, str ):
                self.hyp.SetGridSpacing( [gridDef], [], axis )
            elif isinstance( gridDef[0], str ):
                self.hyp.SetGridSpacing( gridDef, [], axis )
            elif isinstance( gridDef[0], int ) or \
                 isinstance( gridDef[0], float ):
                self.hyp.SetGrid(gridDef, axis )
            else:
                self.hyp.SetGridSpacing( gridDef[0], gridDef[1], axis )
        self.hyp.SetSizeThreshold( sizeThreshold )
        return self.hyp

# Public class: Mesh_UseExisting_1D
# ---------------------------------
## Defines a stub 1D algorithm, which enables "manual" creation of nodes and
#  segments usable by 2D algoritms
#  It is created by calling Mesh.UseExistingSegments(geom=0)
#
#  @ingroup l3_algos_basic

class StdMeshersDC_UseExisting_1D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "UseExistingSegments"
    ## Name of algorithm type
    algoType   = "UseExisting_1D"

    def __init__(self, mesh, geom=0):
        self.Create(mesh, geom, self.algoType)


# Public class: Mesh_UseExisting
# -------------------------------
## Defines a stub 2D algorithm, which enables "manual" creation of nodes and
#  faces usable by 3D algoritms
#  It is created by calling Mesh.UseExistingFaces(geom=0)
#
#  @ingroup l3_algos_basic

class StdMeshersDC_UseExisting_2D(Mesh_Algorithm):

    ## Name of method of class Mesh creating an instance of this class
    meshMethod = "UseExistingFaces"
    ## Name of algorithm type
    algoType   = "UseExisting_2D"

    def __init__(self, mesh, geom=0):
        self.Create(mesh, geom, self.algoType)
