# Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

"""
Python API for the standard meshing plug-in module.
"""

LIBRARY = "libStdMeshersEngine.so"

from salome.smesh.smesh_algorithm import Mesh_Algorithm
import StdMeshers

#----------------------------
# Mesh algo type identifiers
#----------------------------

REGULAR     = "Regular_1D"
"""
Algorithm type: Regular 1D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_Segment`
"""

PYTHON      = "Python_1D"
"""
Algorithm type: Python 1D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_Segment_Python`
"""

COMPOSITE   = "CompositeSegment_1D"
"""

Algorithm type: Composite segment 1D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_CompositeSegment`
"""
MEFISTO     = "MEFISTO_2D"
"""
Algorithm type: Triangle MEFISTO 2D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_Triangle_MEFISTO`
"""

Hexa        = "Hexa_3D"
"""
Algorithm type: Hexahedron 3D (i-j-k) algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_Hexahedron`
"""

QUADRANGLE  = "Quadrangle_2D"
"""
Algorithm type: Quadrangle 2D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_Quadrangle`
"""

RADIAL_QUAD = "RadialQuadrangle_1D2D"
"""
Algorithm type: Radial Quadrangle 1D-2D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_RadialQuadrangle1D2D`
"""

QUAD_MA_PROJ = "QuadFromMedialAxis_1D2D"
"""
Algorithm type: Quadrangle (Medial Axis Projection) 1D-2D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_QuadMA_1D2D`
"""

POLYGON     = "PolygonPerFace_2D"
"""
Algorithm type: Polygon Per Face 2D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_PolygonPerFace`
"""

POLYHEDRON = "PolyhedronPerSolid_3D"
"""
Algorithm type: Polyhedron Per Solid 3D algorithm, see :class:`~StdMeshersBuilder.StdMeshersBuilder_PolyhedronPerSolid`
"""

# import items of enums
for e in StdMeshers.QuadType._items: exec('%s = StdMeshers.%s'%(e,e))
for e in StdMeshers.VLExtrusionMethod._items: exec('%s = StdMeshers.%s'%(e,e))

#----------------------
# Algorithms
#----------------------

class StdMeshersBuilder_Segment(Mesh_Algorithm):
    """
    Defines segment 1D algorithm for edges discretization.
    
    It can be created by calling smeshBuilder.Mesh.Segment(geom=0)
    """
    
    
    meshMethod = "Segment"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """

    algoType   = REGULAR
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """

    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
    of smeshBuilder.Mesh class    
    """

    docHelper  = "Create segment 1D algorithm for edges"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.
        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    def LocalLength(self, l, UseExisting=0, p=1e-07):
        """
        Defines "LocalLength" hypothesis to cut an edge in several segments with the same length
        
        Parameters:
            l : for the length of segments that cut an edge
            UseExisting : if == true - searches for an  existing hypothesis created with
                the same parameters, else (default) - Create a new one
            p : precision, used for calculation of the number of segments.
                The precision should be a positive, meaningful value within the range [0,1].
                In general, the number of segments is calculated with the formula:
                nb = ceil((edge_length / l) - p)
                Function ceil rounds its argument to the higher integer.
                So, p=0 means rounding of (edge_length / l) to the higher integer,
                p=0.5 means rounding of (edge_length / l) to the nearest integer,
                p=1 means rounding of (edge_length / l) to the lower integer.
                Default value is 1e-07.
                
        Returns: 
            an instance of StdMeshers_LocalLength hypothesis
        """

        from salome.smesh.smeshBuilder import IsEqual
        comFun=lambda hyp, args: IsEqual(hyp.GetLength(), args[0]) and IsEqual(hyp.GetPrecision(), args[1])
        hyp = self.Hypothesis("LocalLength", [l,p], UseExisting=UseExisting, CompareMethod=comFun)
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    def MaxSize(self, length=0.0, UseExisting=0):
        """
        Defines "MaxSize" hypothesis to cut an edge into segments not longer than given value

        Parameters:
            length : is optional maximal allowed length of segment, if it is omitted
                the preestimated length is used that depends on geometry size
            UseExisting : if ==true - searches for an existing hypothesis created with
                the same parameters, else (default) - Create a new one
        
        Returns:
            an instance of StdMeshers_MaxLength hypothesis
        """
    

        hyp = self.Hypothesis("MaxLength", [length], UseExisting=UseExisting)
        if isinstance(length,str) or length > 0:
            # set given length
            hyp.SetLength(length)
        if not UseExisting:
            # set preestimated length
            import SMESH
            gen = self.mesh.smeshpyD
            initHyp = gen.GetHypothesisParameterValues("MaxLength", "libStdMeshersEngine.so",
                                                       self.mesh.GetMesh(), self.mesh.GetShape(),
                                                       SMESH.HypInitParams( 1, 1.0, False ))
            preHyp = initHyp._narrow(StdMeshers.StdMeshers_MaxLength)
            if preHyp:
                hyp.SetPreestimatedLength( preHyp.GetPreestimatedLength() )
                pass
            pass
        hyp.SetUsePreestimatedLength( length == 0.0 )
        return hyp

    def NumberOfSegments(self, n, s=[], reversedEdges=[], UseExisting=0):
        """
        Defines "NumberOfSegments" hypothesis to cut an edge in a fixed number of segments

        Parameters:
            n: for the number of segments that cut an edge
            s: for the scale factor (optional)
            reversedEdges: is a list of edges to mesh using reversed orientation.
                    A list item can also be a tuple (edge, 1st_vertex_of_edge)
            UseExisting: if ==true - searches for an existing hypothesis created with
                    the same parameters, else (default) - create a new one
    
        Returns: 
            an instance of StdMeshers_NumberOfSegments hypothesis
        """
    
        
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        entry = self.MainShapeEntry()
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        if not s:
            hyp = self.Hypothesis("NumberOfSegments", [n, reversedEdgeInd, entry],
                                  UseExisting=UseExisting,
                                  CompareMethod=self._compareNumberOfSegments)
        else:
            hyp = self.Hypothesis("NumberOfSegments", [n,s, reversedEdgeInd, entry],
                                  UseExisting=UseExisting,
                                  CompareMethod=self._compareNumberOfSegments)
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        hyp.SetReversedEdges( reversedEdgeInd )
        hyp.SetObjectEntry( entry )
        return hyp

    def _compareNumberOfSegments(self, hyp, args):
        """
        Private method
        Checks if the given "NumberOfSegments" hypothesis has the same parameters as the given arguments
        """
        if hyp.GetNumberOfSegments() == args[0]:
            if len(args) == 3:
                if hyp.GetReversedEdges() == args[1]:
                    if not args[1] or hyp.GetObjectEntry() == args[2]:
                        return True
            else:
                from salome.smesh.smeshBuilder import IsEqual
                if hyp.GetReversedEdges() == args[2]:
                    if not args[2] or hyp.GetObjectEntry() == args[3]:
                        if hyp.GetDistrType() == 1:
                            if IsEqual(hyp.GetScaleFactor(), args[1]):
                                return True
        return False

    def Adaptive(self, minSize, maxSize, deflection, UseExisting=False):
        """
        Defines "Adaptive" hypothesis to cut an edge into segments keeping segment size
        within the given range and considering (1) deflection of segments from the edge
        and (2) distance from segments to closest edges and faces to have segment length
        not longer than two times shortest distances to edges and faces.

        Parameters:
            minSize: defines the minimal allowed segment length
            maxSize: defines the maximal allowed segment length
            deflection: defines the maximal allowed distance from a segment to an edge
            UseExisting: if ==true - searches for an existing hypothesis created with
                the same parameters, else (default) - Create a new one

        Returns:
            an instance of StdMeshers_Adaptive1D hypothesis
        """
        
        from salome.smesh.smeshBuilder import IsEqual
        compFun = lambda hyp, args: ( IsEqual(hyp.GetMinSize(), args[0]) and \
                                      IsEqual(hyp.GetMaxSize(), args[1]) and \
                                      IsEqual(hyp.GetDeflection(), args[2]))
        hyp = self.Hypothesis("Adaptive1D", [minSize, maxSize, deflection],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetMinSize(minSize)
        hyp.SetMaxSize(maxSize)
        hyp.SetDeflection(deflection)
        return hyp

    def Arithmetic1D(self, start, end, reversedEdges=[], UseExisting=0):
        """
        Defines "Arithmetic1D" hypothesis to cut an edge in several segments with a length
                that changes in arithmetic progression

        Parameters:
            start: defines the length of the first segment
            end:   defines the length of the last  segment
            reversedEdges: is a list of edges to mesh using reversed orientation.
                A list item can also be a tuple (edge, 1st_vertex_of_edge)
            UseExisting: if ==true - searches for an existing hypothesis created with
                the same parameters, else (default) - Create a new one

        Returns:
                an instance of StdMeshers_Arithmetic1D hypothesis
        """
        
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        entry = self.MainShapeEntry()
        from salome.smesh.smeshBuilder import IsEqual
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

    def GeometricProgression(self, start, ratio, reversedEdges=[], UseExisting=0):
        """
            Defines "GeometricProgression" hypothesis to cut an edge in several
                segments with a length that changes in Geometric progression

            Parameters:
                start: defines the length of the first segment
                ratio: defines the common ratio of the geometric progression
                reversedEdges: is a list of edges to mesh using reversed orientation.
                    A list item can also be a tuple (edge, 1st_vertex_of_edge)
                UseExisting: if ==true - searches for an existing hypothesis created with
                    the same parameters, else (default) - Create a new one

            Returns:
                an instance of StdMeshers_Geometric1D hypothesis
        """
        
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        entry = self.MainShapeEntry()
        from salome.smesh.smeshBuilder import IsEqual
        compFun = lambda hyp, args: ( IsEqual(hyp.GetLength(1), args[0]) and \
                                      IsEqual(hyp.GetLength(0), args[1]) and \
                                      hyp.GetReversedEdges() == args[2]  and \
                                      (not args[2] or hyp.GetObjectEntry() == args[3]))
        hyp = self.Hypothesis("GeometricProgression", [start, ratio, reversedEdgeInd, entry],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetStartLength( start )
        hyp.SetCommonRatio( ratio )
        hyp.SetReversedEdges( reversedEdgeInd )
        hyp.SetObjectEntry( entry )
        return hyp

    def FixedPoints1D(self, points, nbSegs=[1], reversedEdges=[], UseExisting=0):
        """
        Defines "FixedPoints1D" hypothesis to cut an edge using parameter
                on curve from 0 to 1 (additionally it is neecessary to check
                orientation of edges and create list of reversed edges if it is
                needed) and sets numbers of segments between given points (default
                values are 1)

        Parameters:
            points: defines the list of parameters on curve
            nbSegs: defines the list of numbers of segments
            reversedEdges: is a list of edges to mesh using reversed orientation.
                A list item can also be a tuple (edge, 1st_vertex_of_edge)
            UseExisting: if ==true - searches for an existing hypothesis created with
                the same parameters, else (default) - Create a new one

        Returns:
                an instance of StdMeshers_FixedPoints1D hypothesis
        """
        
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

    def StartEndLength(self, start, end, reversedEdges=[], UseExisting=0):
        """
        Defines "StartEndLength" hypothesis to cut an edge in several segments with increasing geometric length

        Parameters:
            start: defines the length of the first segment
            end:   defines the length of the last  segment
            reversedEdges: is a list of edges to mesh using reversed orientation.
                A list item can also be a tuple (edge, 1st_vertex_of_edge)
            UseExisting: if ==true - searches for an existing hypothesis created with
                the same parameters, else (default) - Create a new one

        Returns:
            an instance of StdMeshers_StartEndLength hypothesis
        """
        
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        reversedEdgeInd = self.ReversedEdgeIndices(reversedEdges)
        entry = self.MainShapeEntry()
        from salome.smesh.smeshBuilder import IsEqual
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

    def Deflection1D(self, d, UseExisting=0):
        """
        Defines "Deflection1D" hypothesis

        Parameters:
            d: for the deflection
            UseExisting: if ==true - searches for an existing hypothesis created with
                the same parameters, else (default) - create a new one
        """
        
        from salome.smesh.smeshBuilder import IsEqual
        compFun = lambda hyp, args: IsEqual(hyp.GetDeflection(), args[0])
        hyp = self.Hypothesis("Deflection1D", [d], UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetDeflection(d)
        return hyp

    def Propagation(self):
        """
        Defines "Propagation" hypothesis that propagates 1D hypotheses
                from an edge where this hypothesis is assigned to
                on all other edges that are at the opposite side in case of quadrangular faces
                This hypothesis should be assigned to an edge to propagate a hypothesis from.
        """
        
        return self.Hypothesis("Propagation", UseExisting=1, CompareMethod=self.CompareEqualHyp)

    def PropagationOfDistribution(self):
        """
        Defines "Propagation of Node Distribution" hypothesis that propagates
                distribution of nodes from an edge where this hypothesis is assigned to,
                to opposite edges of quadrangular faces, so that number of segments on all these
                edges will be the same, as well as relations between segment lengths. 
        """
        
        return self.Hypothesis("PropagOfDistribution", UseExisting=1,
                               CompareMethod=self.CompareEqualHyp)

    def AutomaticLength(self, fineness=0, UseExisting=0):
        """
        Defines "AutomaticLength" hypothesis

        Parameters:
            fineness: for the fineness [0-1]
            UseExisting: if ==true - searches for an existing hypothesis created with the
                same parameters, else (default) - create a new one
        """
        
        from salome.smesh.smeshBuilder import IsEqual
        compFun = lambda hyp, args: IsEqual(hyp.GetFineness(), args[0])
        hyp = self.Hypothesis("AutomaticLength",[fineness],UseExisting=UseExisting,
                              CompareMethod=compFun)
        hyp.SetFineness( fineness )
        return hyp

    def LengthNearVertex(self, length, vertex=-1, UseExisting=0):
        """
        Defines "SegmentLengthAroundVertex" hypothesis

        Parameters:
            length: for the segment length
            vertex: for the length localization: the vertex index [0,1] | vertex object.
                Any other integer value means that the hypothesis will be set on the
                whole 1D shape, where Mesh_Segment algorithm is assigned.
            UseExisting: if ==true - searches for an  existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        
        import types
        store_geom = self.geom
        if isinstance(vertex, int):
            if vertex == 0 or vertex == 1:
                from salome.geom import geomBuilder
                vertex = self.mesh.geompyD.ExtractShapes(self.geom, geomBuilder.geomBuilder.ShapeType["VERTEX"],True)[vertex]
                self.geom = vertex
                pass
            pass
        else:
            self.geom = vertex
            pass
        # 0D algorithm
        if self.geom is None:
            self.geom = store_geom
            raise RuntimeError("Attempt to create SegmentAroundVertex_0D algorithm on None shape")
        from salome.smesh.smeshBuilder import AssureGeomPublished, GetName, TreatHypoStatus
        AssureGeomPublished( self.mesh, self.geom )
        name = GetName(self.geom)

        algo = self.FindAlgorithm("SegmentAroundVertex_0D", self.mesh.smeshpyD)
        if algo is None:
            algo = self.mesh.smeshpyD.CreateHypothesis("SegmentAroundVertex_0D", "libStdMeshersEngine.so")
            pass
        status = self.mesh.mesh.AddHypothesis(self.geom, algo)
        TreatHypoStatus(status, "SegmentAroundVertex_0D", name, True, self.mesh)
        #
        from salome.smesh.smeshBuilder import IsEqual
        comFun = lambda hyp, args: IsEqual(hyp.GetLength(), args[0])
        hyp = self.Hypothesis("SegmentLengthAroundVertex", [length], UseExisting=UseExisting,
                              CompareMethod=comFun)
        self.geom = store_geom
        hyp.SetLength( length )
        return hyp

    def QuadraticMesh(self):
        """
        Defines "QuadraticMesh" hypothesis, forcing construction of quadratic edges.
        If the 2D mesher sees that all boundary edges are quadratic,
        it generates quadratic faces, else it generates linear faces using
        medium nodes as if they are vertices.
        The 3D mesher generates quadratic volumes only if all boundary faces
        are quadratic, else it fails.
        """
        
        hyp = self.Hypothesis("QuadraticMesh", UseExisting=1, CompareMethod=self.CompareEqualHyp)
        return hyp

    pass # end of StdMeshersBuilder_Segment class

class StdMeshersBuilder_CompositeSegment(StdMeshersBuilder_Segment):
    """
    Segment 1D algorithm for discretization of a set of adjacent edges as one edge.

    It is created by calling smeshBuilder.Mesh.Segment(smeshBuilder.COMPOSITE,geom=0)
    """
    

    meshMethod = "Segment"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """

    algoType   = COMPOSITE
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """

    isDefault  = False
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """

    docHelper  = "Create segment 1D algorithm for edges"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        self.Create(mesh, geom, self.algoType)
        pass

    pass # end of StdMeshersBuilder_CompositeSegment class

class StdMeshersBuilder_Segment_Python(Mesh_Algorithm):
    """
    Defines a segment 1D algorithm for discretization of edges with Python function.
    It is created by calling smeshBuilder.Mesh.Segment(smeshBuilder.PYTHON,geom=0)
    """
    

    meshMethod = "Segment"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = PYTHON
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create segment 1D algorithm for edges"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        import Python1dPlugin
        self.Create(mesh, geom, self.algoType, "libPython1dEngine.so")
        pass

    def PythonSplit1D(self, n, func, UseExisting=0):
        """
        Defines "PythonSplit1D" hypothesis

        Parameters:
            n: for the number of segments that cut an edge
            func: for the python function that calculates the length of all segments
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        
        compFun = lambda hyp, args: False
        hyp = self.Hypothesis("PythonSplit1D", [n], "libPython1dEngine.so",
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetNumberOfSegments(n)
        hyp.SetPythonLog10RatioFunction(func)
        return hyp

    pass # end of StdMeshersBuilder_Segment_Python class

class StdMeshersBuilder_Triangle_MEFISTO(Mesh_Algorithm):
    """
    Triangle MEFISTO 2D algorithm.
    It is created by calling smeshBuilder.Mesh.Triangle(smeshBuilder.MEFISTO,geom=0)
    """
    

    meshMethod = "Triangle"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = MEFISTO
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create triangle 2D algorithm for faces"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    def MaxElementArea(self, area, UseExisting=0):
        """
        Defines "MaxElementArea" hypothesis basing on the definition of the maximum area of each triangle

        Parameters:
            area: for the maximum area of each triangle
            UseExisting: if ==true - searches for an  existing hypothesis created with the
                same parameters, else (default) - Create a new one
        """
        
        from salome.smesh.smeshBuilder import IsEqual
        comparator = lambda hyp, args: IsEqual(hyp.GetMaxElementArea(), args[0])
        hyp = self.Hypothesis("MaxElementArea", [area], UseExisting=UseExisting,
                              CompareMethod=comparator)
        hyp.SetMaxElementArea(area)
        return hyp

    def LengthFromEdges(self):
        """
        Defines "LengthFromEdges" hypothesis to build triangles
            based on the length of the edges taken from the wire
        """
        
        hyp = self.Hypothesis("LengthFromEdges", UseExisting=1, CompareMethod=self.CompareEqualHyp)
        return hyp

    pass # end of StdMeshersBuilder_Triangle_MEFISTO class

class StdMeshersBuilder_Quadrangle(Mesh_Algorithm):
    """
    Defines a quadrangle 2D algorithm.
    It is created by calling smeshBuilder.Mesh.Quadrangle(geom=0)
    """
    

    meshMethod = "Quadrangle"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = QUADRANGLE
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create quadrangle 2D algorithm for faces"
    """
    doc string of the method
    """
    params     = 0
    """
    hypothesis associated with algorithm
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    def QuadrangleParameters(self, quadType=StdMeshers.QUAD_STANDARD, triangleVertex=0,
                             enfVertices=[],enfPoints=[],corners=[],UseExisting=0):
        """
        Defines "QuadrangleParameters" hypothesis
            quadType defines the algorithm of transition between differently descretized
            sides of a geometrical face:

            - QUAD_STANDARD - both triangles and quadrangles are possible in the transition
                area along the finer meshed sides.
            - QUAD_TRIANGLE_PREF - only triangles are built in the transition area along the
                finer meshed sides.
            - QUAD_QUADRANGLE_PREF - only quadrangles are built in the transition area along
                the finer meshed sides, iff the total quantity of segments on
                all four sides of the face is even (divisible by 2).
            - QUAD_QUADRANGLE_PREF_REVERSED - same as QUAD_QUADRANGLE_PREF but the transition
                area is located along the coarser meshed sides.
            - QUAD_REDUCED - only quadrangles are built and the transition between the sides
                is made gradually, layer by layer. This type has a limitation on
                the number of segments: one pair of opposite sides must have the
                same number of segments, the other pair must have an even difference
                between the numbers of segments on the sides.

        Parameters:
            triangleVertex: vertex of a trilateral geometrical face, around which triangles
                will be created while other elements will be quadrangles.
                Vertex can be either a GEOM_Object or a vertex ID within the
                shape to mesh
            enfVertices: list of shapes defining positions where nodes (enforced nodes)
                must be created by the mesher. Shapes can be of any type,
                vertices of given shapes define positions of enforced nodes.
                Only vertices successfully projected to the face are used.
            enfPoints: list of points giving positions of enforced nodes.
                Point can be defined either as SMESH.PointStruct's
                ([SMESH.PointStruct(x1,y1,z1), SMESH.PointStruct(x2,y2,z2),...])
                or triples of values ([[x1,y1,z1], [x2,y2,z2], ...]).
                In the case if the defined QuadrangleParameters() refer to a sole face,
                all given points must lie on this face, else the mesher fails.
            corners: list of vertices that should be used as quadrangle corners.
                The parameter can be useful for faces with more than four vertices,
                since in some cases Quadrangle Mapping algorithm chooses corner vertices
                differently than it is desired.
                A hypothesis can be global and define corners for all CAD faces that
                require it, but be sure that each specified vertex is a corner in all
                faces the hypothesis will be applied to.
            UseExisting: if *True* - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        

        import GEOM, SMESH
        vertexID = triangleVertex
        if isinstance( triangleVertex, GEOM._objref_GEOM_Object ):
            vertexID = self.mesh.geompyD.GetSubShapeID( self.mesh.geom, triangleVertex )
        if isinstance( enfVertices, int ) and not enfPoints and not UseExisting:
            # a call of old syntax, before inserting enfVertices and enfPoints before UseExisting
            UseExisting, enfVertices = enfVertices, []

        pStructs, xyz = [], []
        for p in enfPoints:
            if isinstance( p, SMESH.PointStruct ):
                xyz.append(( p.x, p.y, p.z ))
                pStructs.append( p )
            else:
                xyz.append(( p[0], p[1], p[2] ))
                pStructs.append( SMESH.PointStruct( p[0], p[1], p[2] ))
        if not self.params:
            compFun = lambda hyp,args: \
                      hyp.GetQuadType() == args[0] and \
                      (hyp.GetTriaVertex()==args[1] or ( hyp.GetTriaVertex()<1 and args[1]<1)) and \
                      ((hyp.GetEnforcedNodes()) == (args[2],args[3])) # True w/o enfVertices only
            entries = [ shape.GetStudyEntry() for shape in enfVertices ]
            self.params = self.Hypothesis("QuadrangleParams", [quadType,vertexID,entries,xyz],
                                          UseExisting = UseExisting, CompareMethod=compFun)
            pass

        if corners and isinstance( corners[0], GEOM._objref_GEOM_Object ):
            corners = [ self.mesh.geompyD.GetSubShapeID( self.mesh.geom, v ) for v in corners ]

        if self.params.GetQuadType() != quadType:
            self.params.SetQuadType(quadType)
        if vertexID > 0:
            self.params.SetTriaVertex( vertexID )
        from salome.smesh.smeshBuilder import AssureGeomPublished
        for v in enfVertices:
            AssureGeomPublished( self.mesh, v )
        self.params.SetEnforcedNodes( enfVertices, pStructs )
        self.params.SetCorners( corners )
        return self.params

    def QuadranglePreference(self, reversed=False, UseExisting=0):
        """
        Defines "QuadrangleParams" hypothesis with a type of quadrangulation that only
            quadrangles are built in the transition area along the finer meshed sides,
            if the total quantity of segments on all four sides of the face is even.

        Parameters:
            reversed: if True, transition area is located along the coarser meshed sides.
        UseExisting: if ==true - searches for the existing hypothesis created with
            the same parameters, else (default) - Create a new one
        """
        
        if reversed:
            return self.QuadrangleParameters(QUAD_QUADRANGLE_PREF_REVERSED,UseExisting=UseExisting)
        return self.QuadrangleParameters(QUAD_QUADRANGLE_PREF,UseExisting=UseExisting)

    def TrianglePreference(self, UseExisting=0):
        """
        Defines "QuadrangleParams" hypothesis with a type of quadrangulation that only
            triangles are built in the transition area along the finer meshed sides.

        Parameters:
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
    
        return self.QuadrangleParameters(QUAD_TRIANGLE_PREF,UseExisting=UseExisting)

    def Reduced(self, UseExisting=0):
        """
        Defines "QuadrangleParams" hypothesis with a type of quadrangulation that only
            quadrangles are built and the transition between the sides is made gradually,
            layer by layer. This type has a limitation on the number of segments: one pair
            of opposite sides must have the same number of segments, the other pair must
            have an even difference between the numbers of segments on the sides.

        Parameters:
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        
        return self.QuadrangleParameters(QUAD_REDUCED,UseExisting=UseExisting)

    def TriangleVertex(self, vertex, UseExisting=0):
        """
        Defines "QuadrangleParams" hypothesis with QUAD_STANDARD type of quadrangulation

        Parameters:
            vertex: vertex of a trilateral geometrical face, around which triangles
                will be created while other elements will be quadrangles.
                Vertex can be either a GEOM_Object or a vertex ID within the
                shape to mesh
             UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        
        return self.QuadrangleParameters(QUAD_STANDARD,vertex,UseExisting)

    pass # end of StdMeshersBuilder_Quadrangle class

class StdMeshersBuilder_Hexahedron(Mesh_Algorithm):
    """
    Defines a hexahedron 3D algorithm.
    It is created by calling smeshBuilder.Mesh.Hexahedron(geom=0)
    """
    

    meshMethod = "Hexahedron"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = Hexa
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create hexahedron 3D algorithm for volumes"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, Hexa)
        self.renumHypothesis = 0
        pass

    def Renumber(self, blockCSList=[] ):
        if isinstance( blockCSList, StdMeshers.BlockCS ):
            blockCSList = [blockCSList]
        if not self.renumHypothesis:
            self.renumHypothesis = self.Hypothesis("BlockRenumber", blockCSList, UseExisting=0)
        self.renumHypothesis.SetBlocksOrientation( blockCSList )
        return self.renumHypothesis

    pass # end of StdMeshersBuilder_Hexahedron class

class StdMeshersBuilder_Projection1D(Mesh_Algorithm):
    """
    Defines a projection 1D algorithm.
    It is created by calling smeshBuilder.Mesh.Projection1D(geom=0)
    """
    

    meshMethod = "Projection1D"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Projection_1D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create projection 1D algorithm for edges"
    """
    doc string of the method
    """
    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    def SourceEdge(self, edge, mesh=None, srcV=None, tgtV=None, UseExisting=0):
        """
        Defines "Source Edge" hypothesis, specifying a meshed edge, from where
            a mesh pattern is taken, and, optionally, the association of vertices
            between the source edge and a target edge (to which a hypothesis is assigned)

        Parameters:
            edge: from which nodes distribution is taken
            mesh: from which nodes distribution is taken (optional)
            srcV: a vertex of *edge* to associate with *tgtV* (optional)
            tgtV: a vertex of *the edge* to which the algorithm is assigned, to associate with *srcV* (optional)
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        from salome.smesh.smeshBuilder import AssureGeomPublished, Mesh
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

    pass # end of StdMeshersBuilder_Projection1D class

class StdMeshersBuilder_Projection2D(Mesh_Algorithm):
    """
    Defines a projection 2D algorithm.
    It is created by calling smeshBuilder.Mesh.Projection2D(geom=0)
    """
    

    meshMethod = "Projection2D"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Projection_2D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create projection 2D algorithm for faces"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
            if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass


    def SourceFace(self, face, mesh=None, srcV1=None, tgtV1=None,
                   srcV2=None, tgtV2=None, UseExisting=0):
        """
        Defines "Source Face" hypothesis, specifying a meshed face, from where
            a mesh pattern is taken, and, optionally, the association of vertices
            between the source face and the target face (to which a hypothesis is assigned)

        Parameters:
            face: from which the mesh pattern is taken
            mesh: from which the mesh pattern is taken (optional)
            srcV1: a vertex of *face* to associate with *tgtV1* (optional)
            tgtV1: a vertex of *the face* to which the algorithm is assigned, to associate with *srcV1* (optional)
            srcV2: a vertex of *face* to associate with *tgtV1* (optional)
            tgtV2: a vertex of *the face* to which the algorithm is assigned, to associate with *srcV2* (optional)
            UseExisting: if ==true - forces the search for the existing hypothesis created with
                he same parameters, else (default) - forces the creation a new one

        Note: 
            all association vertices must belong to one edge of a face
        """
        from salome.smesh.smeshBuilder import Mesh
        if isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        for geom in [ face, srcV1, tgtV1, srcV2, tgtV2 ]:
            from salome.smesh.smeshBuilder import AssureGeomPublished
            AssureGeomPublished( self.mesh, geom )
        hyp = self.Hypothesis("ProjectionSource2D", [face,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=0, toAdd=False)
        # it does not seem to be useful to reuse the existing "SourceFace" hypothesis
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceFace)
        hyp.SetSourceFace( face )
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        self.mesh.AddHypothesis(hyp, self.geom)
        return hyp

    pass # end of StdMeshersBuilder_Projection2D class

class StdMeshersBuilder_Projection1D2D(StdMeshersBuilder_Projection2D):
    """
    Defines a projection 1D-2D algorithm.
    It is created by calling smeshBuilder.Mesh.Projection1D2D(geom=0)
    """
    

    meshMethod = "Projection1D2D"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Projection_1D2D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create projection 1D-2D algorithm for faces"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        StdMeshersBuilder_Projection2D.__init__(self, mesh, geom)
        pass

    pass # end of StdMeshersBuilder_Projection1D2D class

class StdMeshersBuilder_Projection3D(Mesh_Algorithm):
    """
    Defines a projection 3D algorithm.
    It is created by calling smeshBuilder.Mesh.Projection3D(geom=0)
    """
    

    meshMethod = "Projection3D"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Projection_3D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create projection 3D algorithm for volumes"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom" geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    def SourceShape3D(self, solid, mesh=0, srcV1=0, tgtV1=0,
                      srcV2=0, tgtV2=0, UseExisting=0):
        """
        Defines the "Source Shape 3D" hypothesis, specifying a meshed solid, from where
            the mesh pattern is taken, and, optionally, the  association of vertices
            between the source and the target solid  (to which a hipothesis is assigned)

        Parameters:
            solid: from where the mesh pattern is taken
            mesh: from where the mesh pattern is taken (optional)
            srcV1: a vertex of *solid* to associate with *tgtV1* (optional)
            tgtV1: a vertex of *the solid* where the algorithm is assigned, to associate with *srcV1* (optional)
            srcV2: a vertex of *solid* to associate with *tgtV1* (optional)
            tgtV2: a vertex of *the solid* to which the algorithm is assigned,to associate with *srcV2* (optional)
            UseExisting: if ==true - searches for the existing hypothesis created with
                    the same parameters, else (default) - Create a new one

        Note: 
            association vertices must belong to one edge of a solid
        """
        for geom in [ solid, srcV1, tgtV1, srcV2, tgtV2 ]:
            from salome.smesh.smeshBuilder import AssureGeomPublished
            AssureGeomPublished( self.mesh, geom )
        hyp = self.Hypothesis("ProjectionSource3D",
                              [solid,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=0)
        # seems to be not really useful to reuse existing "SourceShape3D" hypothesis
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceShape3D)
        hyp.SetSource3DShape( solid )
        from salome.smesh.smeshBuilder import Mesh
        if isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        if mesh:
            hyp.SetSourceMesh( mesh )
        if srcV1 and srcV2 and tgtV1 and tgtV2:
            hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        #elif srcV1 or srcV2 or tgtV1 or tgtV2:
        return hyp

    pass # end of StdMeshersBuilder_Projection3D class

class StdMeshersBuilder_Prism3D(Mesh_Algorithm):
    """
    Defines a Prism 3D algorithm, which is either "Extrusion 3D" or "Radial Prism" depending on geometry.
    It is created by calling smeshBuilder.Mesh.Prism(geom=0)
    """
    

    meshMethod = "Prism"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Prism_3D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create prism 3D algorithm for volumes"
    """
    doc string of the method
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        
        shape = geom
        if not shape:
            shape = mesh.geom
        isRadial = mesh.smeshpyD.IsApplicable("RadialPrism_3D", LIBRARY, shape, False )
        if not isRadial:
            self.Create(mesh, geom, "Prism_3D")
            pass
        else:
            self.algoType = "RadialPrism_3D"
            self.Create(mesh, geom, "RadialPrism_3D")
            self.distribHyp = None #self.Hypothesis("LayerDistribution", UseExisting=0)
            self.nbLayers = None
            pass
        pass

    def Get3DHypothesis(self):
        """
        Returns: 
            3D hypothesis holding the 1D one
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        return self.distribHyp

    def OwnHypothesis(self, hypType, args=[], so="libStdMeshersEngine.so"):
        """
        Private method creating a 1D hypothesis and storing it in the LayerDistribution
            hypothesis. 

        Returns:
            the created hypothesis
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        if not self.nbLayers is None:
            self.mesh.GetMesh().RemoveHypothesis( self.geom, self.nbLayers )
            self.mesh.GetMesh().AddHypothesis( self.geom, self.distribHyp )
        self.mesh.smeshpyD.SetEnablePublish( False ) # prevents publishing own 1D hypothesis
        hyp = self.mesh.smeshpyD.CreateHypothesis(hypType, so)
        self.mesh.smeshpyD.SetEnablePublish( True ) # enables publishing
        if not self.distribHyp:
            self.distribHyp = self.Hypothesis("LayerDistribution", UseExisting=0)
        self.distribHyp.SetLayerDistribution( hyp )
        return hyp

    def NumberOfLayers(self, n, UseExisting=0):
        """
        Defines "NumberOfLayers" hypothesis, specifying the number of layers of
            prisms to build between the inner and outer shells

        Parameters:
            n: number of layers
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        self.mesh.RemoveHypothesis( self.distribHyp, self.geom )
        from salome.smesh.smeshBuilder import IsEqual
        compFun = lambda hyp, args: IsEqual(hyp.GetNumberOfLayers(), args[0])
        self.nbLayers = self.Hypothesis("NumberOfLayers", [n], UseExisting=UseExisting,
                                        CompareMethod=compFun)
        self.nbLayers.SetNumberOfLayers( n )
        return self.nbLayers

    def LocalLength(self, l, p=1e-07):
        """
        Defines "LocalLength" hypothesis, specifying the segment length
            to build between the inner and the outer shells

        Parameters:
            l: the length of segments
            p: the precision of rounding
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        hyp = self.OwnHypothesis("LocalLength", [l,p])
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    def NumberOfSegments(self, n, s=[]):
        """
        Defines "NumberOfSegments" hypothesis, specifying the number of layers of
            prisms to build between the inner and the outer shells.

        Parameters:
            n: the number of layers
            s: the scale factor (optional)
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        if not s:
            hyp = self.OwnHypothesis("NumberOfSegments", [n])
        else:
            hyp = self.OwnHypothesis("NumberOfSegments", [n,s])
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        return hyp

    def Arithmetic1D(self, start, end ):
        """
        Defines "Arithmetic1D" hypothesis, specifying the distribution of segments
            to build between the inner and the outer shells with a length that changes
            in arithmetic progression

        Parameters:
            start:  the length of the first segment
            end:    the length of the last  segment
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        hyp = self.OwnHypothesis("Arithmetic1D", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    def GeometricProgression(self, start, ratio ):
        """
        Defines "GeometricProgression" hypothesis, specifying the distribution of segments
            to build between the inner and the outer shells with a length that changes
            in Geometric progression

        Parameters:
            start:  the length of the first segment
            ratio:  the common ratio of the geometric progression
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        hyp = self.OwnHypothesis("GeometricProgression", [start, ratio])
        hyp.SetStartLength( start )
        hyp.SetCommonRatio( ratio )
        return hyp

    def StartEndLength(self, start, end):
        """
        Defines "StartEndLength" hypothesis, specifying distribution of segments
            to build between the inner and the outer shells as geometric length increasing

        Parameters:
            start: for the length of the first segment
        end:   for the length of the last  segment
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        hyp = self.OwnHypothesis("StartEndLength", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    def AutomaticLength(self, fineness=0):
        """
        Defines "AutomaticLength" hypothesis, specifying the number of segments
            to build between the inner and outer shells

        Parameters:
            fineness: defines the quality of the mesh within the range [0-1]
        """
        if self.algoType != "RadialPrism_3D":
            print("Prism_3D algorithm doesn't support any hypothesis")
            return None
        hyp = self.OwnHypothesis("AutomaticLength")
        hyp.SetFineness( fineness )
        return hyp

    pass # end of StdMeshersBuilder_Prism3D class

class StdMeshersBuilder_RadialPrism3D(StdMeshersBuilder_Prism3D):
    """
    Defines Radial Prism 3D algorithm.
    It is created by calling smeshBuilder.Mesh.Prism(geom=0).
    See :class:`StdMeshersBuilder_Prism3D` for methods defining distribution of mesh layers
    build between the inner and outer shells.
    """

    meshMethod = "Prism"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "RadialPrism_3D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create Raial Prism 3D algorithm for volumes"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
            if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        
        shape = geom
        if not shape:
            shape = mesh.geom
        self.Create(mesh, geom, "RadialPrism_3D")
        self.distribHyp = None
        self.nbLayers = None
        return

class StdMeshersBuilder_RadialAlgorithm(Mesh_Algorithm):
    """
    Base class for algorithms supporting radial distribution hypotheses
    """ 

    def __init__(self):
        Mesh_Algorithm.__init__(self)

        self.distribHyp = None #self.Hypothesis("LayerDistribution2D", UseExisting=0)
        self.nbLayers = None
        pass

    def Get2DHypothesis(self):
        """
        Returns:
            2D hypothesis holding the 1D one
        """
        if not self.distribHyp:
            self.distribHyp = self.Hypothesis("LayerDistribution2D", UseExisting=0)
        return self.distribHyp

    def OwnHypothesis(self, hypType, args=[], so="libStdMeshersEngine.so"):
        """
        Private method creating a 1D hypothesis and storing it in the LayerDistribution
            hypothesis. 

        Returns: 
            the created hypothesis
        """
        if self.nbLayers:
            self.mesh.GetMesh().RemoveHypothesis( self.geom, self.nbLayers )
        if self.distribHyp is None:
            self.distribHyp = self.Hypothesis("LayerDistribution2D", UseExisting=0)
        else:
            self.mesh.GetMesh().AddHypothesis( self.geom, self.distribHyp )
        self.mesh.smeshpyD.SetEnablePublish( False )
        hyp = self.mesh.smeshpyD.CreateHypothesis(hypType, so)
        self.mesh.smeshpyD.SetEnablePublish( True )
        self.distribHyp.SetLayerDistribution( hyp )
        return hyp

    def NumberOfLayers(self, n, UseExisting=0):
        """
        Defines "NumberOfLayers" hypothesis, specifying the number of layers

        Parameters:
            n: number of layers
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        if self.distribHyp:
            self.mesh.GetMesh().RemoveHypothesis( self.geom, self.distribHyp )
        from salome.smesh.smeshBuilder import IsEqual
        compFun = lambda hyp, args: IsEqual(hyp.GetNumberOfLayers(), args[0])
        self.nbLayers = self.Hypothesis("NumberOfLayers2D", [n], UseExisting=UseExisting,
                                        CompareMethod=compFun)
        self.nbLayers.SetNumberOfLayers( n )
        return self.nbLayers

    def LocalLength(self, l, p=1e-07):
        """
        Defines "LocalLength" hypothesis, specifying the segment length

        Parameters:
            l: the length of segments
            p: the precision of rounding
        """
        hyp = self.OwnHypothesis("LocalLength", [l,p])
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    def NumberOfSegments(self, n, s=[]):
        """
        Defines "NumberOfSegments" hypothesis, specifying the number of layers

        Parameters:
            n: the number of layers
            s: the scale factor (optional)
        """
        if s == []:
            hyp = self.OwnHypothesis("NumberOfSegments", [n])
        else:
            hyp = self.OwnHypothesis("NumberOfSegments", [n,s])
            hyp.SetDistrType( 1 )
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        return hyp

    def Arithmetic1D(self, start, end ):
        """
        Defines "Arithmetic1D" hypothesis, specifying the distribution of segments
            with a length that changes in arithmetic progression

        Parameters:
            start:  the length of the first segment
            end:    the length of the last  segment
        """
        hyp = self.OwnHypothesis("Arithmetic1D", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    def GeometricProgression(self, start, ratio ):
        """
        Defines "GeometricProgression" hypothesis, specifying the distribution of segments
            with a length that changes in Geometric progression

        Parameters:
            start:  the length of the first segment
            ratio:  the common ratio of the geometric progression
        """
        hyp = self.OwnHypothesis("GeometricProgression", [start, ratio])
        hyp.SetStartLength( start )
        hyp.SetCommonRatio( ratio )
        return hyp

    def StartEndLength(self, start, end):
        """
        Defines "StartEndLength" hypothesis, specifying distribution of segments
            as geometric length increasing

        Parameters:
            start: for the length of the first segment
            end:   for the length of the last  segment
        """
        hyp = self.OwnHypothesis("StartEndLength", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    def AutomaticLength(self, fineness=0):
        """
        Defines "AutomaticLength" hypothesis, specifying the number of segments

        Parameters:
            fineness: defines the quality of the mesh within the range [0-1]
        """
        hyp = self.OwnHypothesis("AutomaticLength")
        hyp.SetFineness( fineness )
        return hyp

    pass # end of StdMeshersBuilder_RadialQuadrangle1D2D class

class StdMeshersBuilder_RadialQuadrangle1D2D(StdMeshersBuilder_RadialAlgorithm):
    """
    Defines a Radial Quadrangle 1D-2D algorithm.
    It is created by calling smeshBuilder.Mesh.Quadrangle(smeshBuilder.RADIAL_QUAD,geom=0)
    """

    meshMethod = "Quadrangle"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = RADIAL_QUAD
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create quadrangle 1D-2D algorithm for faces having a shape of disk or a disk segment"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        StdMeshersBuilder_RadialAlgorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)

        self.distribHyp = None #self.Hypothesis("LayerDistribution2D", UseExisting=0)
        self.nbLayers = None
        pass


class StdMeshersBuilder_QuadMA_1D2D(StdMeshersBuilder_RadialAlgorithm):
    """
    Defines a Quadrangle (Medial Axis Projection) 1D-2D algorithm .
    It is created by calling smeshBuilder.Mesh.Quadrangle(smeshBuilder.QUAD_MA_PROJ,geom=0)
    """

    meshMethod = "Quadrangle"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = QUAD_MA_PROJ
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create quadrangle 1D-2D algorithm for faces"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        StdMeshersBuilder_RadialAlgorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    pass

class StdMeshersBuilder_PolygonPerFace(Mesh_Algorithm):
    """ Defines a Polygon Per Face 2D algorithm.
        It is created by calling smeshBuilder.Mesh.Polygon(geom=0)
    """

    meshMethod = "Polygon"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = POLYGON
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create polygon 2D algorithm for faces"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    pass

class StdMeshersBuilder_PolyhedronPerSolid(Mesh_Algorithm):
    """ Defines a Polyhedron Per Solid 3D algorithm.
        It is created by calling smeshBuilder.Mesh.Polyhedron(geom=0)
    """

    meshMethod = "Polyhedron"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = POLYHEDRON
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create polyhedron 3D algorithm for solids"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    pass

class StdMeshersBuilder_UseExistingElements_1D(Mesh_Algorithm):
    """ Defines a Use Existing Elements 1D algorithm.

    It is created by calling smeshBuilder.Mesh.UseExisting1DElements(geom=0)
    """
    

    meshMethod = "UseExisting1DElements"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Import_1D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create 1D algorithm for edges with reusing of existing mesh elements"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    def SourceEdges(self, groups, toCopyMesh=False, toCopyGroups=False, UseExisting=False):
        """
        Defines "Source edges" hypothesis, specifying groups of edges to import

        Parameters:
            groups: list of groups of edges
            toCopyMesh: if True, the whole mesh *groups* belong to is imported
            toCopyGroups: if True, all groups of the mesh *groups* belong to are imported
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        for group in groups:
            from salome.smesh.smeshBuilder import AssureGeomPublished
            AssureGeomPublished( self.mesh, group )
        compFun = lambda hyp, args: ( hyp.GetSourceEdges() == args[0] and \
                                      hyp.GetCopySourceMesh() == args[1], args[2] )
        hyp = self.Hypothesis("ImportSource1D", [groups, toCopyMesh, toCopyGroups],
                              UseExisting=UseExisting, CompareMethod=compFun)
        hyp.SetSourceEdges(groups)
        hyp.SetCopySourceMesh(toCopyMesh, toCopyGroups)
        return hyp

    pass # end of StdMeshersBuilder_UseExistingElements_1D class

class StdMeshersBuilder_UseExistingElements_1D2D(Mesh_Algorithm):
    """ Defines a Use Existing Elements 1D-2D algorithm.

    It is created by calling smeshBuilder.Mesh.UseExisting2DElements(geom=0)
    """
    

    meshMethod = "UseExisting2DElements"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Import_1D2D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create 1D-2D algorithm for faces with reusing of existing mesh elements"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, self.algoType)
        pass

    def SourceFaces(self, groups, toCopyMesh=False, toCopyGroups=False, UseExisting=False):
        """
        Defines "Source faces" hypothesis, specifying groups of faces to import

        Parameters:
            groups: list of groups of faces
            toCopyMesh: if True, the whole mesh *groups* belong to is imported
            toCopyGroups: if True, all groups of the mesh *groups* belong to are imported
            UseExisting: if ==true - searches for the existing hypothesis created with
                the same parameters, else (default) - Create a new one
        """
        import SMESH
        compFun = lambda hyp, args: ( hyp.GetSourceFaces() == args[0] and \
                                      hyp.GetCopySourceMesh() == args[1], args[2] )
        hyp = self.Hypothesis("ImportSource2D", [groups, toCopyMesh, toCopyGroups],
                              UseExisting=UseExisting, CompareMethod=compFun, toAdd=False)
        if groups and isinstance( groups, SMESH._objref_SMESH_GroupBase ):
            groups = [groups]
        hyp.SetSourceFaces(groups)
        hyp.SetCopySourceMesh(toCopyMesh, toCopyGroups)
        self.mesh.AddHypothesis(hyp, self.geom)
        return hyp

    pass # end of StdMeshersBuilder_UseExistingElements_1D2D class

class StdMeshersBuilder_Cartesian_3D(Mesh_Algorithm):
    """ Defines a Body Fitting 3D algorithm.

    It is created by calling smeshBuilder.Mesh.BodyFitted(geom=0)
    """
    

    meshMethod = "BodyFitted"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "Cartesian_3D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    isDefault  = True
    """
    flag pointing whether this algorithm should be used by default in dynamic method
        of smeshBuilder.Mesh class
    """
    docHelper  = "Create Body Fitting 3D algorithm for volumes"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        self.Create(mesh, geom, self.algoType)
        self.hyp = None
        pass

    def SetGrid(self, xGridDef, yGridDef, zGridDef, sizeThreshold=4.0, implEdges=False):
        """
        Defines "Body Fitting parameters" hypothesis

        Parameters:
            xGridDef: is definition of the grid along the X asix.
                It can be in either of two following forms:

                    - Explicit coordinates of nodes, e.g. [-1.5, 0.0, 3.1] or range( -100,200,10)
                    - Functions f(t) defining grid spacing at each point on grid axis. If there are
                        several functions, they must be accompanied by relative coordinates of
                        points dividing the whole shape into ranges where the functions apply; points
                        coordinates should vary within (0.0, 1.0) range. Parameter *t* of the spacing
                        function f(t) varies from 0.0 to 1.0 within a shape range. 
        Note: 
            The actual grid spacing can slightly differ from the defined one. This is done for the
            best fitting of polyhedrons and for a better mesh quality on the interval boundaries.
            For example, if a constant **Spacing** is defined along an axis, the actual grid will
            fill the shape's dimension L along this axis with round number of equal cells:
            Spacing_actual = L / round( L / Spacing_defined ).

        Examples:
            "10.5" - defines a grid with a constant spacing
            [["1", "1+10*t", "11"] [0.1, 0.6]] - defines different spacing in 3 ranges.

        Parameters:
            yGridDef: defines the grid along the Y asix the same way as *xGridDef* does.
            zGridDef: defines the grid along the Z asix the same way as *xGridDef* does.
            sizeThreshold: (> 1.0) defines a minimal size of a polyhedron so that
                a polyhedron of size less than hexSize/sizeThreshold is not created.
            implEdges: enables implementation of geometrical edges into the mesh.
        """
        if not self.hyp:
            compFun = lambda hyp, args: False
            self.hyp = self.Hypothesis("CartesianParameters3D",
                                       [xGridDef, yGridDef, zGridDef, sizeThreshold],
                                       UseExisting=False, CompareMethod=compFun)
        if not self.mesh.IsUsedHypothesis( self.hyp, self.geom ):
            self.mesh.AddHypothesis( self.hyp, self.geom )

        for axis, gridDef in enumerate( [xGridDef, yGridDef, zGridDef] ):
            if not gridDef: raise ValueError("Empty grid definition")
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
        self.hyp.SetToAddEdges( implEdges )
        return self.hyp

    def SetAxesDirs( self, xAxis, yAxis, zAxis ):
        """
        Defines custom directions of axes of the grid

        Parameters:
            xAxis: either SMESH.DirStruct or a vector, or 3 vector components
            yAxis: either SMESH.DirStruct or a vector, or 3 vector components
            zAxis: either SMESH.DirStruct or a vector, or 3 vector components
        """
        import GEOM
        if hasattr( xAxis, "__getitem__" ):
            xAxis = self.mesh.smeshpyD.MakeDirStruct( xAxis[0],xAxis[1],xAxis[2] )
        elif isinstance( xAxis, GEOM._objref_GEOM_Object ):
            xAxis = self.mesh.smeshpyD.GetDirStruct( xAxis )
        if hasattr( yAxis, "__getitem__" ):
            yAxis = self.mesh.smeshpyD.MakeDirStruct( yAxis[0],yAxis[1],yAxis[2] )
        elif isinstance( yAxis, GEOM._objref_GEOM_Object ):
            yAxis = self.mesh.smeshpyD.GetDirStruct( yAxis )
        if hasattr( zAxis, "__getitem__" ):
            zAxis = self.mesh.smeshpyD.MakeDirStruct( zAxis[0],zAxis[1],zAxis[2] )
        elif isinstance( zAxis, GEOM._objref_GEOM_Object ):
            zAxis = self.mesh.smeshpyD.GetDirStruct( zAxis )
        if not self.hyp:
            self.hyp = self.Hypothesis("CartesianParameters3D")
        if not self.mesh.IsUsedHypothesis( self.hyp, self.geom ):
            self.mesh.AddHypothesis( self.hyp, self.geom )
        self.hyp.SetAxesDirs( xAxis, yAxis, zAxis )
        return self.hyp

    def SetOptimalAxesDirs(self, isOrthogonal=True):
        """
        Automatically defines directions of axes of the grid at which
            a number of generated hexahedra is maximal

        Parameters:
            isOrthogonal: defines whether the axes mush be orthogonal
        """
        if not self.hyp:
            self.hyp = self.Hypothesis("CartesianParameters3D")
        if not self.mesh.IsUsedHypothesis( self.hyp, self.geom ):
            self.mesh.AddHypothesis( self.hyp, self.geom )
        x,y,z = self.hyp.ComputeOptimalAxesDirs( self.geom, isOrthogonal )
        self.hyp.SetAxesDirs( x,y,z )
        return self.hyp

    def SetFixedPoint( self, p, toUnset=False ):
        """
        Sets/unsets a fixed point. The algorithm makes a plane of the grid pass
            through the fixed point in each direction at which the grid is defined
            by spacing

        Parameters:
            p: coordinates of the fixed point. Either SMESH.PointStruct or
                a vertex or 3 components of coordinates.
            toUnset: defines whether the fixed point is defined or removed.
        """
        import SMESH, GEOM
        if toUnset:
            if not self.hyp: return
            p = SMESH.PointStruct(0,0,0)
        elif hasattr( p, "__getitem__" ):
            p = SMESH.PointStruct( p[0],p[1],p[2] )
        elif isinstance( p, GEOM._objref_GEOM_Object ):
            p = self.mesh.smeshpyD.GetPointStruct( p )
        if not self.hyp:
            self.hyp = self.Hypothesis("CartesianParameters3D")
        if not self.mesh.IsUsedHypothesis( self.hyp, self.geom ):
            self.mesh.AddHypothesis( self.hyp, self.geom )
        self.hyp.SetFixedPoint( p, toUnset )
        return self.hyp
        

    pass # end of StdMeshersBuilder_Cartesian_3D class

class StdMeshersBuilder_UseExisting_1D(Mesh_Algorithm):
    """ Defines a stub 1D algorithm, which enables "manual" creation of nodes and
        segments usable by 2D algorithms.

    It is created by calling smeshBuilder.Mesh.UseExistingSegments(geom=0)
    """


    meshMethod = "UseExistingSegments"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "UseExisting_1D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create 1D algorithm allowing batch meshing of edges"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
                if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        self.Create(mesh, geom, self.algoType)
        pass

    pass # end of StdMeshersBuilder_UseExisting_1D class

class StdMeshersBuilder_UseExisting_2D(Mesh_Algorithm):
    """ Defines a stub 2D algorithm, which enables "manual" creation of nodes and
    faces usable by 3D algorithms.

    It is created by calling smeshBuilder.Mesh.UseExistingFaces(geom=0)
    """
    

    meshMethod = "UseExistingFaces"
    """
    name of the dynamic method in smeshBuilder.Mesh class
    """
    algoType   = "UseExisting_2D"
    """
    type of algorithm used with helper function in smeshBuilder.Mesh class
    """
    docHelper  = "Create 2D algorithm allowing batch meshing of faces"
    """
    doc string of the method
    """

    def __init__(self, mesh, geom=0):
        """
        Private constructor.

        Parameters:
            mesh: parent mesh object algorithm is assigned to
            geom: geometry (shape/sub-shape) algorithm is assigned to;
            if it is :code:`0` (default), the algorithm is assigned to the main shape
        """
        self.Create(mesh, geom, self.algoType)
        pass

    pass # end of StdMeshersBuilder_UseExisting_2D class
