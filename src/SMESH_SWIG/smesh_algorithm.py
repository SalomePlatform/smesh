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

## @package smesh_algorithm
#  Python API for base Mesh_Algorithm class.
#  This package is a part of SALOME %Mesh module Python API

import salome
import geompyDC
import SMESH

## The base class to define meshing algorithms
#
#  @note This class should not be used directly, it is supposed to be sub-classed
#  for implementing Python API for specific meshing algorithms
#
#  For each meshing algorithm, a python class inheriting from class %Mesh_Algorithm
#  should be defined. This descendant class should have two attributes defining the way
#  it is created by class Mesh (see e.g. class @ref StdMeshersDC.StdMeshersDC_Segment "StdMeshersDC_Segment"
#  in StdMeshersDC package):
#  - @c meshMethod attribute defines name of method of class smesh.Mesh by calling which the
#    python class of algorithm is created; this method is dynamically added to the smesh.Mesh class
#    in runtime. For example, if in @c class MyPlugin_Algorithm this attribute is defined as
#    @code
#    meshMethod = "MyAlgorithm"
#    @endcode
#    then an instance of @c MyPlugin_Algorithm can be created by the direct invokation of the function
#    of smesh.Mesh class:
#    @code
#    my_algo = mesh.MyAlgorithm()
#    @endcode
#  - @c algoType defines type of algorithm and is used mostly to discriminate
#    algorithms that are created by the same method of class smesh.Mesh. For example, if this attribute
#    is specified in @c MyPlugin_Algorithm class as
#    @code
#    algoType = "MyPLUGIN"
#    @endcode
#    then it's creation code can be:
#    @code
#    my_algo = mesh.MyAlgorithm(algo="MyPLUGIN")
#    @endcode
#  @ingroup l2_algorithms
class Mesh_Algorithm:
    
    ## Private constuctor
    def __init__(self):
        self.mesh = None
        self.geom = None
        self.subm = None
        self.algo = None
        pass

    ## Finds a hypothesis in the study by its type name and parameters.
    #  Finds only the hypotheses created in smeshpyD engine.
    #  @return SMESH.SMESH_Hypothesis
    def FindHypothesis (self, hypname, args, CompareMethod, smeshpyD):
        study = smeshpyD.GetCurrentStudy()
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

    ## Finds the algorithm in the study by its type name.
    #  Finds only the algorithms, which have been created in smeshpyD engine.
    #  @return SMESH.SMESH_Algo
    def FindAlgorithm (self, algoname, smeshpyD):
        study = smeshpyD.GetCurrentStudy()
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

    ## If the algorithm is global, returns 0; \n
    #  else returns the submesh associated to this algorithm.
    def GetSubMesh(self):
        return self.subm

    ## Returns the wrapped mesher.
    def GetAlgorithm(self):
        return self.algo

    ## Gets the list of hypothesis that can be used with this algorithm
    def GetCompatibleHypothesis(self):
        mylist = []
        if self.algo:
            mylist = self.algo.GetCompatibleHypothesis()
        return mylist

    ## Gets the name of the algorithm
    def GetName(self):
        from smesh import GetName
        return GetName(self.algo)

    ## Sets the name to the algorithm
    def SetName(self, name):
        self.mesh.smeshpyD.SetName(self.algo, name)

    ## Gets the id of the algorithm
    def GetId(self):
        return self.algo.GetId()

    ## Private method.
    def Create(self, mesh, geom, hypo, so="libStdMeshersEngine.so"):
        if geom is None:
            raise RuntimeError, "Attemp to create " + hypo + " algoritm on None shape"
        algo = self.FindAlgorithm(hypo, mesh.smeshpyD)
        if algo is None:
            algo = mesh.smeshpyD.CreateHypothesis(hypo, so)
            pass
        self.Assign(algo, mesh, geom)
        return self.algo

    ## Private method
    def Assign(self, algo, mesh, geom):
        from smesh import AssureGeomPublished, TreatHypoStatus, GetName
        if geom is None:
            raise RuntimeError, "Attemp to create " + algo + " algoritm on None shape"
        self.mesh = mesh
        name = ""
        if not geom or geom.IsSame( mesh.geom ):
            self.geom = mesh.geom
        else:
            self.geom = geom
            AssureGeomPublished( mesh, geom )
            try:
                name = GetName(geom)
                pass
            except:
                pass
            self.subm = mesh.mesh.GetSubMesh(geom, algo.GetName())
        self.algo = algo
        status = mesh.mesh.AddHypothesis(self.geom, self.algo)
        TreatHypoStatus( status, algo.GetName(), name, True )
        return

    def CompareHyp (self, hyp, args):
        print "CompareHyp is not implemented for ", self.__class__.__name__, ":", hyp.GetName()
        return False

    def CompareEqualHyp (self, hyp, args):
        return True

    ## Private method
    def Hypothesis (self, hyp, args=[], so="libStdMeshersEngine.so",
                    UseExisting=0, CompareMethod=""):
        from smesh import TreatHypoStatus, GetName
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
                if isinstance( arg, geompyDC.GEOM._objref_GEOM_Object ):
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
        status = self.mesh.mesh.AddHypothesis(self.geom, hypo)
        TreatHypoStatus( status, GetName(hypo), geomName, 0 )
        return hypo

    ## Returns entry of the shape to mesh in the study
    def MainShapeEntry(self):
        if not self.mesh or not self.mesh.GetMesh(): return ""
        if not self.mesh.GetMesh().HasShapeToMesh(): return ""
        shape = self.mesh.GetShape()
        return shape.GetStudyEntry()

    ## Defines "ViscousLayers" hypothesis to give parameters of layers of prisms to build
    #  near mesh boundary. This hypothesis can be used by several 3D algorithms:
    #  NETGEN 3D, GHS3D, Hexahedron(i,j,k)
    #  @param thickness total thickness of layers of prisms
    #  @param numberOfLayers number of layers of prisms
    #  @param stretchFactor factor (>1.0) of growth of layer thickness towards inside of mesh
    #  @param ignoreFaces list of geometrical faces (or their ids) not to generate layers on
    #  @ingroup l3_hypos_additi
    def ViscousLayers(self, thickness, numberOfLayers, stretchFactor, ignoreFaces=[]):
        if not isinstance(self.algo, SMESH._objref_SMESH_3D_Algo):
            raise TypeError, "ViscousLayers are supported by 3D algorithms only"
        if not "ViscousLayers" in self.GetCompatibleHypothesis():
            raise TypeError, "ViscousLayers are not supported by %s"%self.algo.GetName()
        if ignoreFaces and isinstance( ignoreFaces[0], geompyDC.GEOM._objref_GEOM_Object ):
            ignoreFaces = [ self.mesh.geompyD.GetSubShapeID(self.mesh.geom, f) for f in ignoreFaces ]
        hyp = self.Hypothesis("ViscousLayers",
                              [thickness, numberOfLayers, stretchFactor, ignoreFaces])
        hyp.SetTotalThickness(thickness)
        hyp.SetNumberLayers(numberOfLayers)
        hyp.SetStretchFactor(stretchFactor)
        hyp.SetIgnoreFaces(ignoreFaces)
        return hyp

    ## Defines "ViscousLayers2D" hypothesis to give parameters of layers of quadrilateral
    #  elements to build near mesh boundary. This hypothesis can be used by several 2D algorithms:
    #  NETGEN 2D, NETGEN 1D-2D, Quadrangle (mapping), MEFISTO, BLSURF
    #  @param thickness total thickness of layers of quadrilaterals
    #  @param numberOfLayers number of layers
    #  @param stretchFactor factor (>1.0) of growth of layer thickness towards inside of mesh
    #  @param ignoreEdges list of geometrical edge (or their ids) not to generate layers on
    #  @ingroup l3_hypos_additi
    def ViscousLayers2D(self, thickness, numberOfLayers, stretchFactor, ignoreEdges=[]):
        if not isinstance(self.algo, SMESH._objref_SMESH_2D_Algo):
            raise TypeError, "ViscousLayers2D are supported by 2D algorithms only"
        if not "ViscousLayers2D" in self.GetCompatibleHypothesis():
            raise TypeError, "ViscousLayers2D are not supported by %s"%self.algo.GetName()
        if ignoreEdges and isinstance( ignoreEdges[0], geompyDC.GEOM._objref_GEOM_Object ):
            ignoreEdges = [ self.mesh.geompyD.GetSubShapeID(self.mesh.geom, f) for f in ignoreEdges ]
        hyp = self.Hypothesis("ViscousLayers2D",
                              [thickness, numberOfLayers, stretchFactor, ignoreEdges])
        hyp.SetTotalThickness(thickness)
        hyp.SetNumberLayers(numberOfLayers)
        hyp.SetStretchFactor(stretchFactor)
        hyp.SetIgnoreEdges(ignoreEdges)
        return hyp

    ## Transform a list of ether edges or tuples (edge, 1st_vertex_of_edge)
    #  into a list acceptable to SetReversedEdges() of some 1D hypotheses
    #  @ingroup l3_hypos_1dhyps
    def ReversedEdgeIndices(self, reverseList):
        from smesh import FirstVertexOnCurve
        resList = []
        geompy = self.mesh.geompyD
        for i in reverseList:
            if isinstance( i, int ):
                s = geompy.SubShapes(self.mesh.geom, [i])[0]
                if s.GetShapeType() != geompyDC.GEOM.EDGE:
                    raise TypeError, "Not EDGE index given"
                resList.append( i )
            elif isinstance( i, geompyDC.GEOM._objref_GEOM_Object ):
                if i.GetShapeType() != geompyDC.GEOM.EDGE:
                    raise TypeError, "Not an EDGE given"
                resList.append( geompy.GetSubShapeID(self.mesh.geom, i ))
            elif len( i ) > 1:
                e = i[0]
                v = i[1]
                if not isinstance( e, geompyDC.GEOM._objref_GEOM_Object ) or \
                   not isinstance( v, geompyDC.GEOM._objref_GEOM_Object ):
                    raise TypeError, "A list item must be a tuple (edge, 1st_vertex_of_edge)"
                if v.GetShapeType() == geompyDC.GEOM.EDGE and \
                   e.GetShapeType() == geompyDC.GEOM.VERTEX:
                    v,e = e,v
                if e.GetShapeType() != geompyDC.GEOM.EDGE or \
                   v.GetShapeType() != geompyDC.GEOM.VERTEX:
                    raise TypeError, "A list item must be a tuple (edge, 1st_vertex_of_edge)"
                vFirst = FirstVertexOnCurve( e )
                tol    = geompy.Tolerance( vFirst )[-1]
                if geompy.MinDistance( v, vFirst ) > 1.5*tol:
                    resList.append( geompy.GetSubShapeID(self.mesh.geom, e ))
            else:
                raise TypeError, "Item must be either an edge or tuple (edge, 1st_vertex_of_edge)"
        return resList

