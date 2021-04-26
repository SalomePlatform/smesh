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

## @package smesh_algorithm
#  Python API for base Mesh_Algorithm class.
#  This package is a part of SALOME %Mesh module Python API

import salome
from salome.geom import geomBuilder
import SMESH, StdMeshers

class Mesh_Algorithm:
    """
    The base class to define meshing algorithms

    Note:
        This class should not be used directly, it is supposed to be sub-classed
        for implementing Python API for specific meshing algorithms

    For each meshing algorithm, a python class inheriting from class *Mesh_Algorithm*
    should be defined. This descendant class should have two attributes defining the way
    it is created by class :class:`~smeshBuilder.Mesh` (see e.g. class :class:`~StdMeshersBuilder.StdMeshersBuilder_Segment`):

    - :code:`meshMethod` attribute defines name of method of class :class:`~smeshBuilder.Mesh` by calling which the
      python class of algorithm is created; this method is dynamically added to the :class:`~smeshBuilder.Mesh` class
      in runtime. For example, if in :code:`class MyPlugin_Algorithm` this attribute is defined as::

          meshMethod = "MyAlgorithm"

      then an instance of :code:`MyPlugin_Algorithm` can be created by the direct invocation of the function
      of :class:`~smeshBuilder.Mesh` class::
    
          my_algo = mesh.MyAlgorithm()
    
    - :code:`algoType` defines type of algorithm and is used mostly to discriminate
      algorithms that are created by the same method of class :class:`~smeshBuilder.Mesh`. For example, if this attribute
      is specified in :code:`MyPlugin_Algorithm` class as::
    
          algoType = "MyPLUGIN"

      then it's creation code can be::

          my_algo = mesh.MyAlgorithm(algo="MyPLUGIN")
    """
    
    
    def __init__(self):
        """
        Private constructor
        """
        self.mesh = None
        self.geom = None
        self.subm = None
        self.algo = None
        pass

    def FindHypothesis (self, hypname, args, CompareMethod, smeshpyD):
        """
        Finds a hypothesis in the study by its type name and parameters.
        Finds only the hypotheses created in smeshBuilder engine.

        Returns: 
                :class:`~SMESH.SMESH_Hypothesis`
        """
        study = salome.myStudy
        if not study: return None
        #to do: find component by smeshpyD object, not by its data type
        scomp = study.FindComponent(smeshpyD.ComponentDataType())
        if scomp is not None:
            res,hypRoot = scomp.FindSubObject(SMESH.Tag_HypothesisRoot)
            # Check if the root label of the hypotheses exists
            if res and hypRoot is not None:
                iter = study.NewChildIterator(hypRoot)
                # Check all published hypotheses
                while iter.More():
                    hypo_so_i = iter.Value()
                    attr = hypo_so_i.FindAttribute("AttributeIOR")[1]
                    if attr is not None:
                        anIOR = attr.Value()
                        if not anIOR: continue # prevent exception in orb.string_to_object()
                        hypo_o_i = salome.orb.string_to_object(anIOR)
                        if hypo_o_i is not None:
                            # Check if this is a hypothesis
                            hypo_i = hypo_o_i._narrow(SMESH.SMESH_Hypothesis)
                            if hypo_i is not None:
                                # Check if the hypothesis belongs to current engine
                                if smeshpyD.GetObjectId(hypo_i) > 0:
                                    # Check if this is the required hypothesis
                                    if hypo_i.GetName() == hypname:
                                        # Check arguments
                                        if CompareMethod(hypo_i, args):
                                            # found!!!
                                            return hypo_i
                                        pass
                                    pass
                                pass
                            pass
                        pass
                    iter.Next()
                    pass
                pass
            pass
        return None

    def FindAlgorithm (self, algoname, smeshpyD):
        """
        Finds the algorithm in the study by its type name.
        Finds only the algorithms, which have been created in smeshBuilder engine.

        Returns:
                SMESH.SMESH_Algo
        """
        study = salome.myStudy
        if not study: return None
        #to do: find component by smeshpyD object, not by its data type
        scomp = study.FindComponent(smeshpyD.ComponentDataType())
        if scomp is not None:
            res,hypRoot = scomp.FindSubObject(SMESH.Tag_AlgorithmsRoot)
            # Check if the root label of the algorithms exists
            if res and hypRoot is not None:
                iter = study.NewChildIterator(hypRoot)
                # Check all published algorithms
                while iter.More():
                    algo_so_i = iter.Value()
                    attr = algo_so_i.FindAttribute("AttributeIOR")[1]
                    if attr is not None:
                        anIOR = attr.Value()
                        if not anIOR: continue # prevent exception in orb.string_to_object()
                        algo_o_i = salome.orb.string_to_object(anIOR)
                        if algo_o_i is not None:
                            # Check if this is an algorithm
                            algo_i = algo_o_i._narrow(SMESH.SMESH_Algo)
                            if algo_i is not None:
                                # Checks if the algorithm belongs to the current engine
                                if smeshpyD.GetObjectId(algo_i) > 0:
                                    # Check if this is the required algorithm
                                    if algo_i.GetName() == algoname:
                                        # found!!!
                                        return algo_i
                                    pass
                                pass
                            pass
                        pass
                    iter.Next()
                    pass
                pass
            pass
        return None

    def GetSubMesh(self):
        """
        If the algorithm is global, returns 0; 
        else returns the :class:`~SMESH.SMESH_subMesh` associated to this algorithm.
        """
        return self.subm

    def GetAlgorithm(self):
        """
        Returns the wrapped mesher.
        """
        return self.algo

    def GetCompatibleHypothesis(self):
        """
        Gets the list of hypothesis that can be used with this algorithm
        """
        mylist = []
        if self.algo:
            mylist = self.algo.GetCompatibleHypothesis()
        return mylist

    def GetName(self):
        """
        Gets the name of the algorithm
        """
        from salome.smesh.smeshBuilder import GetName
        return GetName(self.algo)

    def SetName(self, name):
        """
        Sets the name to the algorithm
        """
        self.mesh.smeshpyD.SetName(self.algo, name)

    def GetId(self):
        """
        Gets the id of the algorithm
        """
        return self.algo.GetId()

    def Create(self, mesh, geom, hypo, so="libStdMeshersEngine.so"):
        """
        Private method.
        """
        if geom is None and mesh.mesh.HasShapeToMesh():
            raise RuntimeError("Attempt to create " + hypo + " algorithm on None shape")
        algo = self.FindAlgorithm(hypo, mesh.smeshpyD)
        if algo is None:
            algo = mesh.smeshpyD.CreateHypothesis(hypo, so)
            pass
        self.Assign(algo, mesh, geom)
        return self.algo

    def Assign(self, algo, mesh, geom):
        """
        Private method
        """
        from salome.smesh.smeshBuilder import AssureGeomPublished, TreatHypoStatus, GetName
        if geom is None and mesh.mesh.HasShapeToMesh():
            raise RuntimeError("Attempt to create " + algo + " algorithm on None shape")
        self.mesh = mesh
        if not geom or geom.IsSame( mesh.geom ):
            self.geom = mesh.geom
        else:
            self.geom = geom
            AssureGeomPublished( mesh, geom )
            self.subm = mesh.mesh.GetSubMesh(geom, algo.GetName())
        self.algo = algo
        status = mesh.AddHypothesis(self.algo, self.geom)
        return

    def CompareHyp (self, hyp, args):
        print("CompareHyp is not implemented for ", self.__class__.__name__, ":", hyp.GetName())
        return False

    def CompareEqualHyp (self, hyp, args):
        return True

    def Hypothesis (self, hyp, args=[], so="libStdMeshersEngine.so",
                    UseExisting=0, CompareMethod="", toAdd=True):
        """
        Private method
        """
        from salome.smesh.smeshBuilder import TreatHypoStatus, GetName
        hypo = None
        if UseExisting:
            if CompareMethod == "": CompareMethod = self.CompareHyp
            hypo = self.FindHypothesis(hyp, args, CompareMethod, self.mesh.smeshpyD)
            pass
        if hypo is None:
            hypo = self.mesh.smeshpyD.CreateHypothesis(hyp, so)
            a = ""
            s = "="
            for arg in args:
                argStr = str(arg)
                if isinstance( arg, geomBuilder.GEOM._objref_GEOM_Object ):
                    argStr = arg.GetStudyEntry()
                    if not argStr: argStr = "GEOM_Obj_%s", arg.GetEntry()
                if len( argStr ) > 10:
                    argStr = argStr[:7]+"..."
                    if argStr[0] == '[': argStr += ']'
                a = a + s + argStr
                s = ","
                pass
            if len(a) > 50:
                a = a[:47]+"..."
            self.mesh.smeshpyD.SetName(hypo, hyp + a)
            pass
        geomName=""
        if self.geom:
            geomName = GetName(self.geom)
        if toAdd:
            status = self.mesh.mesh.AddHypothesis(self.geom, hypo)
            TreatHypoStatus( status, GetName(hypo), geomName, 0, self.mesh )
        return hypo

    def MainShapeEntry(self):
        """
        Returns entry of the shape to mesh in the study
        """
        if not self.mesh or not self.mesh.GetMesh(): return ""
        if not self.mesh.GetMesh().HasShapeToMesh(): return ""
        shape = self.mesh.GetShape()
        return shape.GetStudyEntry()

    def ViscousLayers(self, thickness, numberOfLayers, stretchFactor,
                      faces=[], isFacesToIgnore=True,
                      extrMethod=StdMeshers.SURF_OFFSET_SMOOTH, groupName=""):
        """
        Defines "ViscousLayers" hypothesis to give parameters of layers of prisms to build
        near mesh boundary. This hypothesis can be used by several 3D algorithms:
        NETGEN 3D, MG-Tetra, Hexahedron(i,j,k)

        Parameters:
                thickness: total thickness of layers of prisms
                numberOfLayers: number of layers of prisms
                stretchFactor: factor (>1.0) of growth of layer thickness towards inside of mesh
                faces: list of geometrical faces (or their ids).
                        Viscous layers are either generated on these faces or not, depending on
                        the value of **isFacesToIgnore** parameter.
                isFacesToIgnore: if *True*, the Viscous layers are not generated on the
                        faces specified by the previous parameter (**faces**).
                extrMethod: extrusion method defines how position of new nodes are found during
                        prism construction and how creation of distorted and intersecting prisms is
                        prevented. Possible values are:

                        - StdMeshers.SURF_OFFSET_SMOOTH (default) method extrudes nodes along normal
                                to underlying geometrical surface. Smoothing of internal surface of
                                element layers can be used to avoid creation of invalid prisms.
                        - StdMeshers.FACE_OFFSET method extrudes nodes along average normal of
                                surrounding mesh faces till intersection with a neighbor mesh face
                                translated along its own normal by the layers thickness. Thickness
                                of layers can be limited to avoid creation of invalid prisms.
                        - StdMeshers.NODE_OFFSET method extrudes nodes along average normal of
                                surrounding mesh faces by the layers thickness. Thickness of
                                layers can be limited to avoid creation of invalid prisms.
                groupName: name of a group to contain elements of layers. If not provided,
                           no group is created. The group is created upon mesh generation.
                           It can be retrieved by calling
                           ::

                             group = mesh.GetGroupByName( groupName, SMESH.VOLUME )[0]

        Returns:
                StdMeshers.StdMeshers_ViscousLayers hypothesis
        """

        if not isinstance(self.algo, SMESH._objref_SMESH_3D_Algo):
            raise TypeError("ViscousLayers are supported by 3D algorithms only")
        if not "ViscousLayers" in self.GetCompatibleHypothesis():
            raise TypeError("ViscousLayers are not supported by %s"%self.algo.GetName())
        if faces and isinstance( faces, geomBuilder.GEOM._objref_GEOM_Object ):
            faces = [ faces ]
        if faces and isinstance( faces[0], geomBuilder.GEOM._objref_GEOM_Object ):
            faceIDs = []
            for shape in faces:
                try:
                  ff = self.mesh.geompyD.SubShapeAll( shape, self.mesh.geompyD.ShapeType["FACE"] )
                  for f in ff:
                    faceIDs.append( self.mesh.geompyD.GetSubShapeID(self.mesh.geom, f))
                except:
                  # try to get the SHAPERSTUDY engine directly, because GetGen does not work because of
                  # simplification of access in geomBuilder: omniORB.registerObjref
                  from SHAPERSTUDY_utils import getEngine
                  gen = getEngine()
                  if gen:
                    aShapeOp = gen.GetIShapesOperations()
                    ff = aShapeOp.ExtractSubShapes( shape, self.mesh.geompyD.ShapeType["FACE"], False)
                    for f in ff:
                      faceIDs.append( aShapeOp.GetSubShapeIndex( self.mesh.geom, f ))
            faces = faceIDs
        hyp = self.Hypothesis("ViscousLayers",
                              [thickness, numberOfLayers, stretchFactor, faces, isFacesToIgnore],
                              toAdd=False)
        hyp.SetTotalThickness( thickness )
        hyp.SetNumberLayers( numberOfLayers )
        hyp.SetStretchFactor( stretchFactor )
        hyp.SetFaces( faces, isFacesToIgnore )
        hyp.SetMethod( extrMethod )
        hyp.SetGroupName( groupName )
        self.mesh.AddHypothesis( hyp, self.geom )
        return hyp

    def ViscousLayers2D(self, thickness, numberOfLayers, stretchFactor,
                        edges=[], isEdgesToIgnore=True,  groupName="" ):
        """
        Defines "ViscousLayers2D" hypothesis to give parameters of layers of quadrilateral
        elements to build near mesh boundary. This hypothesis can be used by several 2D algorithms:
        NETGEN 2D, NETGEN 1D-2D, Quadrangle (mapping), MEFISTO, MG-CADSurf

        Parameters:
                thickness: total thickness of layers of quadrilaterals
                numberOfLayers: number of layers
                stretchFactor: factor (>1.0) of growth of layer thickness towards inside of mesh
                edges: list of geometrical edges (or their ids).
                        Viscous layers are either generated on these edges or not, depending on
                        the value of **isEdgesToIgnore** parameter.
                isEdgesToIgnore: if *True*, the Viscous layers are not generated on the
                        edges specified by the previous parameter (**edges**).
                groupName: name of a group to contain elements of layers. If not provided,
                        no group is created. The group is created upon mesh generation.
                        It can be retrieved by calling
                        ::

                          group = mesh.GetGroupByName( groupName, SMESH.FACE )[0]

        Returns:
                StdMeshers.StdMeshers_ViscousLayers2D hypothesis
        """
        
        if not isinstance(self.algo, SMESH._objref_SMESH_2D_Algo):
            raise TypeError("ViscousLayers2D are supported by 2D algorithms only")
        if not "ViscousLayers2D" in self.GetCompatibleHypothesis():
            raise TypeError("ViscousLayers2D are not supported by %s"%self.algo.GetName())
        if edges and not isinstance( edges, list ) and not isinstance( edges, tuple ):
            edges = [edges]
        if edges and isinstance( edges[0], geomBuilder.GEOM._objref_GEOM_Object ):
            edgeIDs = []
            for shape in edges:
              try:
                ee = self.mesh.geompyD.SubShapeAll( shape, self.mesh.geompyD.ShapeType["EDGE"])
                for e in ee:
                  edgeIDs.append( self.mesh.geompyD.GetSubShapeID( self.mesh.geom, e ))
              except:
                # try to get the SHAPERSTUDY engine directly, because GetGen does not work because of
                # simplification of access in geomBuilder: omniORB.registerObjref
                from SHAPERSTUDY_utils import getEngine
                gen = getEngine()
                if gen:
                  aShapeOp = gen.GetIShapesOperations()
                  ee = aShapeOp.ExtractSubShapes( shape, self.mesh.geompyD.ShapeType["EDGE"], False)
                  for e in ee:
                    edgeIDs.append( aShapeOp.GetSubShapeIndex( self.mesh.geom, e ))
            edges = edgeIDs
        hyp = self.Hypothesis("ViscousLayers2D",
                              [thickness, numberOfLayers, stretchFactor, edges, isEdgesToIgnore],
                              toAdd=False)
        hyp.SetTotalThickness(thickness)
        hyp.SetNumberLayers(numberOfLayers)
        hyp.SetStretchFactor(stretchFactor)
        hyp.SetEdges(edges, isEdgesToIgnore)
        hyp.SetGroupName( groupName )
        self.mesh.AddHypothesis( hyp, self.geom )
        return hyp

    def ReversedEdgeIndices(self, reverseList):
        """
        Transform a list of either edges or tuples (edge, 1st_vertex_of_edge)
        into a list acceptable to SetReversedEdges() of some 1D hypotheses
        """
        
        resList = []
        geompy = self.mesh.geompyD
        for i in reverseList:
            if isinstance( i, int ):
                s = geompy.GetSubShape(self.mesh.geom, [i])

                #bos #20082 begin:
                if s is None and type(self.geom) != geomBuilder.GEOM._objref_GEOM_Object:
                    # try to get the SHAPERSTUDY engine directly, as GetGen does not work because of
                    # simplification of access in geomBuilder: omniORB.registerObjref
                    from SHAPERSTUDY_utils import getEngine
                    gen = getEngine()
                    if gen:
                        aShapeOp = gen.GetIShapesOperations()
                        s = aShapeOp.GetSubShape(self.mesh.geom, i)
                #bos #20082 end

                if s.GetShapeType() != geomBuilder.GEOM.EDGE:
                    raise TypeError("Not EDGE index given")
                resList.append( i )
            elif isinstance( i, geomBuilder.GEOM._objref_GEOM_Object ):
                if i.GetShapeType() != geomBuilder.GEOM.EDGE:
                    raise TypeError("Not an EDGE given")
                resList.append( geompy.GetSubShapeID(self.mesh.geom, i ))
            elif len( i ) > 1:
                e = i[0]
                v = i[1]
                if not isinstance( e, geomBuilder.GEOM._objref_GEOM_Object ) or \
                   not isinstance( v, geomBuilder.GEOM._objref_GEOM_Object ):
                    raise TypeError("A list item must be a tuple (edge, 1st_vertex_of_edge)")
                if v.GetShapeType() == geomBuilder.GEOM.EDGE and \
                   e.GetShapeType() == geomBuilder.GEOM.VERTEX:
                    v,e = e,v
                if e.GetShapeType() != geomBuilder.GEOM.EDGE or \
                   v.GetShapeType() != geomBuilder.GEOM.VERTEX:
                    raise TypeError("A list item must be a tuple (edge, 1st_vertex_of_edge)")
                vFirst = geompy.GetVertexByIndex( e, 0, False )
                tol    = geompy.Tolerance( vFirst )[-1]
                if geompy.MinDistance( v, vFirst ) > 1.5*tol:
                    resList.append( geompy.GetSubShapeID(self.mesh.geom, e ))
            else:
                raise TypeError("Item must be either an edge or tuple (edge, 1st_vertex_of_edge)")
        return resList

