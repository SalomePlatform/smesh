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
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#  File   : smesh.py
#  Author : Francis KLOSS, OCC
#  Module : SMESH

"""
 \namespace smesh
 \brief Module smesh
"""

import salome
import geompyDC

import SMESH # necessary for back compatibility
from   SMESH import *

import StdMeshers

import SALOME

# import NETGENPlugin module if possible
noNETGENPlugin = 0
try:
    import NETGENPlugin
except ImportError:
    noNETGENPlugin = 1
    pass

# Types of algo
REGULAR    = 1
PYTHON     = 2
COMPOSITE  = 3

MEFISTO       = 3
NETGEN        = 4
GHS3D         = 5
FULL_NETGEN   = 6
NETGEN_2D     = 7
NETGEN_1D2D   = NETGEN
NETGEN_1D2D3D = FULL_NETGEN
NETGEN_FULL   = FULL_NETGEN

# MirrorType enumeration
POINT = SMESH_MeshEditor.POINT
AXIS =  SMESH_MeshEditor.AXIS
PLANE = SMESH_MeshEditor.PLANE

# Smooth_Method enumeration
LAPLACIAN_SMOOTH = SMESH_MeshEditor.LAPLACIAN_SMOOTH
CENTROIDAL_SMOOTH = SMESH_MeshEditor.CENTROIDAL_SMOOTH

# Fineness enumeration(for NETGEN)
VeryCoarse = 0
Coarse = 1
Moderate = 2
Fine = 3
VeryFine = 4
Custom = 5


NO_NAME = "NoName"

## Gets object name
def GetName(obj):
    ior  = salome.orb.object_to_string(obj)
    sobj = salome.myStudy.FindObjectIOR(ior)
    if sobj is None:
        return NO_NAME
    else:
        attr = sobj.FindAttribute("AttributeName")[1]
        return attr.Value()

    ## Sets name to object
def SetName(obj, name):
    ior  = salome.orb.object_to_string(obj)
    sobj = salome.myStudy.FindObjectIOR(ior)
    if not sobj is None:
        attr = sobj.FindAttribute("AttributeName")[1]
        attr.SetValue(name)

    ## Print error message if a hypothesis was not assigned.
def TreatHypoStatus(status, hypName, geomName, isAlgo):
    if isAlgo:
        hypType = "algorithm"
    else:
        hypType = "hypothesis"
        pass
    if status == HYP_UNKNOWN_FATAL :
        reason = "for unknown reason"
    elif status == HYP_INCOMPATIBLE :
        reason = "this hypothesis mismatches algorithm"
    elif status == HYP_NOTCONFORM :
        reason = "not conform mesh would be built"
    elif status == HYP_ALREADY_EXIST :
        reason = hypType + " of the same dimension already assigned to this shape"
    elif status == HYP_BAD_DIM :
        reason = hypType + " mismatches shape"
    elif status == HYP_CONCURENT :
        reason = "there are concurrent hypotheses on sub-shapes"
    elif status == HYP_BAD_SUBSHAPE :
        reason = "shape is neither the main one, nor its subshape, nor a valid group"
    elif status == HYP_BAD_GEOMETRY:
        reason = "geometry mismatches algorithm's expectation"
    elif status == HYP_HIDDEN_ALGO:
        reason = "it is hidden by an algorithm of upper dimension generating all-dimensions elements"
    elif status == HYP_HIDING_ALGO:
        reason = "it hides algorithm(s) of lower dimension by generating all-dimensions elements"
    else:
        return
    hypName = '"' + hypName + '"'
    geomName= '"' + geomName+ '"'
    if status < HYP_UNKNOWN_FATAL:
        print hypName, "was assigned to",    geomName,"but", reason
    else:
        print hypName, "was not assigned to",geomName,":", reason
        pass

class smeshDC(SMESH._objref_SMESH_Gen):

    def init_smesh(self,theStudy,geompyD):
        self.geompyD=geompyD
        self.SetGeomEngine(geompyD)
        self.SetCurrentStudy(theStudy)

    def Mesh(self, obj=0, name=0):
      return Mesh(self,self.geompyD,obj,name)

    ## Returns long value from enumeration
    #  Uses for SMESH.FunctorType enumeration
    def EnumToLong(self,theItem):
        return theItem._v

    ## Get PointStruct from vertex
    #  @param theVertex is GEOM object(vertex)
    #  @return SMESH.PointStruct
    def GetPointStruct(self,theVertex):
        [x, y, z] = self.geompyD.PointCoordinates(theVertex)
        return PointStruct(x,y,z)

    ## Get DirStruct from vector
    #  @param theVector is GEOM object(vector)
    #  @return SMESH.DirStruct
    def GetDirStruct(self,theVector):
        vertices = self.geompyD.SubShapeAll( theVector, geompyDC.ShapeType["VERTEX"] )
        if(len(vertices) != 2):
            print "Error: vector object is incorrect."
            return None
        p1 = self.geompyD.PointCoordinates(vertices[0])
        p2 = self.geompyD.PointCoordinates(vertices[1])
        pnt = PointStruct(p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2])
        dirst = DirStruct(pnt)
        return dirst
    
    ## Make DirStruct from a triplet
    #  @param x,y,z are vector components
    #  @return SMESH.DirStruct
    def MakeDirStruct(x,y,z):
        pnt = PointStruct(x,y,z)
        return DirStruct(pnt)

    ## Get AxisStruct from object
    #  @param theObj is GEOM object(line or plane)
    #  @return SMESH.AxisStruct
    def GetAxisStruct(self,theObj):
        edges = self.geompyD.SubShapeAll( theObj, geompyDC.ShapeType["EDGE"] )
        if len(edges) > 1:
            vertex1, vertex2 = self.geompyD.SubShapeAll( edges[0], geompyDC.ShapeType["VERTEX"] )
            vertex3, vertex4 = self.geompyD.SubShapeAll( edges[1], geompyDC.ShapeType["VERTEX"] )
            vertex1 = self.geompyD.PointCoordinates(vertex1)
            vertex2 = self.geompyD.PointCoordinates(vertex2)
            vertex3 = self.geompyD.PointCoordinates(vertex3)
            vertex4 = self.geompyD.PointCoordinates(vertex4)
            v1 = [vertex2[0]-vertex1[0], vertex2[1]-vertex1[1], vertex2[2]-vertex1[2]]
            v2 = [vertex4[0]-vertex3[0], vertex4[1]-vertex3[1], vertex4[2]-vertex3[2]]
            normal = [ v1[1]*v2[2]-v2[1]*v1[2], v1[2]*v2[0]-v2[2]*v1[0], v1[0]*v2[1]-v2[0]*v1[1] ]
            axis = AxisStruct(vertex1[0], vertex1[1], vertex1[2], normal[0], normal[1], normal[2])
            return axis
        elif len(edges) == 1:
            vertex1, vertex2 = self.geompyD.SubShapeAll( edges[0], geompyDC.ShapeType["VERTEX"] )
            p1 = self.geompyD.PointCoordinates( vertex1 )
            p2 = self.geompyD.PointCoordinates( vertex2 )
            axis = AxisStruct(p1[0], p1[1], p1[2], p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2])
            return axis
        return None

    # From SMESH_Gen interface:
    # ------------------------
    
    ## Set the current mode
    def SetEmbeddedMode( self,theMode ):
        #self.SetEmbeddedMode(theMode)
        SMESH._objref_SMESH_Gen.SetEmbeddedMode(self,theMode)
        
    ## Get the current mode
    def IsEmbeddedMode(self):
        #return self.IsEmbeddedMode()
        return SMESH._objref_SMESH_Gen.IsEmbeddedMode(self)

    ## Set the current study
    def SetCurrentStudy( self, theStudy ):
        #self.SetCurrentStudy(theStudy)
        SMESH._objref_SMESH_Gen.SetCurrentStudy(self,theStudy)

    ## Get the current study
    def GetCurrentStudy(self):
        #return self.GetCurrentStudy()
        return SMESH._objref_SMESH_Gen.GetCurrentStudy(self)

    ## Create Mesh object importing data from given UNV file
    #  @return an instance of Mesh class
    def CreateMeshesFromUNV( self,theFileName ):
        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromUNV(self,theFileName)
        aMesh = Mesh(self,self.geompyD,aSmeshMesh)
        return aMesh

    ## Create Mesh object(s) importing data from given MED file
    #  @return a list of Mesh class instances
    def CreateMeshesFromMED( self,theFileName ):
        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromMED(self,theFileName)
        aMeshes = []
        for iMesh in range(len(aSmeshMeshes)) :
            aMesh = Mesh(self,self.geompyD,aSmeshMeshes[iMesh])
            aMeshes.append(aMesh)
        return aMeshes, aStatus
    
    ## Create Mesh object importing data from given STL file
    #  @return an instance of Mesh class
    def CreateMeshesFromSTL( self, theFileName ):
        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromSTL(self,theFileName)
        aMesh = Mesh(self,self.geompyD,aSmeshMesh)
        return aMesh
    
    ## From SMESH_Gen interface
    def GetSubShapesId( self, theMainObject, theListOfSubObjects ):
        return SMESH._objref_SMESH_Gen.GetSubShapesId(self,theMainObject, theListOfSubObjects)

    ## From SMESH_Gen interface. Creates pattern
    def GetPattern(self):
        return SMESH._objref_SMESH_Gen.GetPattern(self)
    
    
    
    # Filtering. Auxiliary functions:
    # ------------------------------
    
    ## Creates an empty criterion
    #  @return SMESH.Filter.Criterion
    def GetEmptyCriterion(self):
        Type = self.EnumToLong(FT_Undefined)
        Compare = self.EnumToLong(FT_Undefined)
        Threshold = 0
        ThresholdStr = ""
        ThresholdID = ""
        UnaryOp = self.EnumToLong(FT_Undefined)
        BinaryOp = self.EnumToLong(FT_Undefined)
        Tolerance = 1e-07
        TypeOfElement = ALL
        Precision = -1 ##@1e-07
        return Filter.Criterion(Type, Compare, Threshold, ThresholdStr, ThresholdID,
                                UnaryOp, BinaryOp, Tolerance, TypeOfElement, Precision)
      
    ## Creates a criterion by given parameters
    #  @param elementType is the type of elements(NODE, EDGE, FACE, VOLUME)
    #  @param CritType is type of criterion( FT_Taper, FT_Area, FT_RangeOfIds, FT_LyingOnGeom etc. )
    #  @param Compare belong to {FT_LessThan, FT_MoreThan, FT_EqualTo}
    #  @param Treshold is threshold value (range of ids as string, shape, numeric)
    #  @param UnaryOp is FT_LogicalNOT or FT_Undefined
    #  @param BinaryOp is binary logical operation FT_LogicalAND, FT_LogicalOR or
    #                  FT_Undefined(must be for the last criterion in criteria)
    #  @return SMESH.Filter.Criterion
    def GetCriterion(self,elementType,
                     CritType,
                     Compare = FT_EqualTo,
                     Treshold="",
                     UnaryOp=FT_Undefined,
                     BinaryOp=FT_Undefined):
        aCriterion = self.GetEmptyCriterion()
        aCriterion.TypeOfElement = elementType
        aCriterion.Type = self.EnumToLong(CritType)
        
        aTreshold = Treshold
        
        if Compare in [FT_LessThan, FT_MoreThan, FT_EqualTo]:
            aCriterion.Compare = self.EnumToLong(Compare)
        elif Compare == "=" or Compare == "==":
            aCriterion.Compare = self.EnumToLong(FT_EqualTo)
        elif Compare == "<":
            aCriterion.Compare = self.EnumToLong(FT_LessThan)
        elif Compare == ">":
            aCriterion.Compare = self.EnumToLong(FT_MoreThan)
        else:
            aCriterion.Compare = self.EnumToLong(FT_EqualTo)
            aTreshold = Compare
            
        if CritType in [FT_BelongToGeom,     FT_BelongToPlane, FT_BelongToGenSurface,
                        FT_BelongToCylinder, FT_LyingOnGeom]:
            # Check treshold
            if isinstance(aTreshold, geompyDC.GEOM._objref_GEOM_Object):
                aCriterion.ThresholdStr = GetName(aTreshold)
                aCriterion.ThresholdID = salome.ObjectToID(aTreshold)
            else:
                print "Error: Treshold should be a shape."
                return None
        elif CritType == FT_RangeOfIds:
            # Check treshold
            if isinstance(aTreshold, str):
                aCriterion.ThresholdStr = aTreshold
            else:
                print "Error: Treshold should be a string."
                return None
        elif CritType in [FT_FreeBorders, FT_FreeEdges, FT_BadOrientedVolume]:
            # Here we do not need treshold
            if aTreshold ==  FT_LogicalNOT:
                aCriterion.UnaryOp = self.EnumToLong(FT_LogicalNOT)
            elif aTreshold in [FT_LogicalAND, FT_LogicalOR]:
                aCriterion.BinaryOp = aTreshold
        else:
            # Check treshold
            try:
                aTreshold = float(aTreshold)
                aCriterion.Threshold = aTreshold
            except:
                print "Error: Treshold should be a number."
                return None
            
        if Treshold ==  FT_LogicalNOT or UnaryOp ==  FT_LogicalNOT:
            aCriterion.UnaryOp = self.EnumToLong(FT_LogicalNOT)
            
        if Treshold in [FT_LogicalAND, FT_LogicalOR]:
            aCriterion.BinaryOp = self.EnumToLong(Treshold)
            
        if UnaryOp in [FT_LogicalAND, FT_LogicalOR]:
            aCriterion.BinaryOp = self.EnumToLong(UnaryOp)

        if BinaryOp in [FT_LogicalAND, FT_LogicalOR]:
            aCriterion.BinaryOp = self.EnumToLong(BinaryOp)

        return aCriterion
    
    ## Creates filter by given parameters of criterion
    #  @param elementType is the type of elements in the group
    #  @param CritType is type of criterion( FT_Taper, FT_Area, FT_RangeOfIds, FT_LyingOnGeom etc. )
    #  @param Compare belong to {FT_LessThan, FT_MoreThan, FT_EqualTo}
    #  @param Treshold is threshold value (range of id ids as string, shape, numeric)
    #  @param UnaryOp is FT_LogicalNOT or FT_Undefined
    #  @return SMESH_Filter
    def GetFilter(self,elementType,
                  CritType=FT_Undefined,
                  Compare=FT_EqualTo,
                  Treshold="",
                  UnaryOp=FT_Undefined):
        aCriterion = self.GetCriterion(elementType, CritType, Compare, Treshold, UnaryOp, FT_Undefined)
        aFilterMgr = self.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aCriteria = []
        aCriteria.append(aCriterion)
        aFilter.SetCriteria(aCriteria)
        return aFilter
    
    ## Creates numerical functor by its type
    #  @param theCrierion is FT_...; functor type
    #  @return SMESH_NumericalFunctor
    def GetFunctor(self,theCriterion):
        aFilterMgr = self.CreateFilterManager()
        if theCriterion == FT_AspectRatio:
            return aFilterMgr.CreateAspectRatio()
        elif theCriterion == FT_AspectRatio3D:
            return aFilterMgr.CreateAspectRatio3D()
        elif theCriterion == FT_Warping:
            return aFilterMgr.CreateWarping()
        elif theCriterion == FT_MinimumAngle:
            return aFilterMgr.CreateMinimumAngle()
        elif theCriterion == FT_Taper:
            return aFilterMgr.CreateTaper()
        elif theCriterion == FT_Skew:
            return aFilterMgr.CreateSkew()
        elif theCriterion == FT_Area:
            return aFilterMgr.CreateArea()
        elif theCriterion == FT_Volume3D:
            return aFilterMgr.CreateVolume3D()
        elif theCriterion == FT_MultiConnection:
            return aFilterMgr.CreateMultiConnection()
        elif theCriterion == FT_MultiConnection2D:
            return aFilterMgr.CreateMultiConnection2D()
        elif theCriterion == FT_Length:
            return aFilterMgr.CreateLength()
        elif theCriterion == FT_Length2D:
            return aFilterMgr.CreateLength2D()
        else:
            print "Error: given parameter is not numerucal functor type."

import omniORB
#Register the new proxy for SMESH_Gen
omniORB.registerObjref(SMESH._objref_SMESH_Gen._NP_RepositoryId, smeshDC)
    
    
## Mother class to define algorithm, recommended to do not use directly.
#
#  More details.
class Mesh_Algorithm:
    #  @class Mesh_Algorithm
    #  @brief Class Mesh_Algorithm

    #17908#hypos = {}

    #def __init__(self,smesh):
    #    self.smesh=smesh
    def __init__(self):
        self.mesh = None
        self.geom = None
        self.subm = None
        self.algo = None

    #17908#def FindHypothesis(self,hypname, args):
    #17908#    key = "%s %s %s" % (self.__class__.__name__, hypname, args)
    #17908#    if Mesh_Algorithm.hypos.has_key( key ):
    #17908#        return Mesh_Algorithm.hypos[ key ]
    #17908#    return None

    ## If the algorithm is global, return 0; \n
    #  else return the submesh associated to this algorithm.
    def GetSubMesh(self):
        return self.subm

    ## Return the wrapped mesher.
    def GetAlgorithm(self):
        return self.algo

    ## Get list of hypothesis that can be used with this algorithm
    def GetCompatibleHypothesis(self):
        mylist = []
        if self.algo:
            mylist = self.algo.GetCompatibleHypothesis()
        return mylist

    ## Get name of algo
    def GetName(self):
        GetName(self.algo)

    ## Set name to algo
    def SetName(self, name):
        SetName(self.algo, name)

    ## Get id of algo
    def GetId(self):
        return self.algo.GetId()

    ## Private method.
    def Create(self, mesh, geom, hypo, so="libStdMeshersEngine.so"):
        if geom is None:
            raise RuntimeError, "Attemp to create " + hypo + " algoritm on None shape"
        algo = mesh.smeshpyD.CreateHypothesis(hypo, so)
        self.Assign(algo, mesh, geom)
        return self.algo

    ## Private method
    def Assign(self, algo, mesh, geom):
        if geom is None:
            raise RuntimeError, "Attemp to create " + hypo + " algoritm on None shape"
        self.mesh = mesh
        piece = mesh.geom
        if not geom:
            self.geom = piece
        else:
            self.geom = geom
            name = GetName(geom)
            if name==NO_NAME:
                name = mesh.geompyD.SubShapeName(geom, piece)
                mesh.geompyD.addToStudyInFather(piece, geom, name)
            self.subm = mesh.mesh.GetSubMesh(geom, algo.GetName())

        self.algo = algo
        status = mesh.mesh.AddHypothesis(self.geom, self.algo)
        TreatHypoStatus( status, algo.GetName(), GetName(self.geom), True )

    ## Private method
    def Hypothesis(self, hyp, args=[], so="libStdMeshersEngine.so", UseExisting=0):
        CreateNew = 1
        #17908#if UseExisting:
        #17908#    hypo = self.FindHypothesis(hyp, args)
        #17908#    if hypo: CreateNew = 0
        #17908#    pass
        if CreateNew:
            hypo = self.mesh.smeshpyD.CreateHypothesis(hyp, so)
            key = "%s %s %s" % (self.__class__.__name__, hyp, args)
            #17908#Mesh_Algorithm.hypos[key] = hypo
            a = ""
            s = "="
            i = 0
            n = len(args)
            while i<n:
                a = a + s + str(args[i])
                s = ","
                i = i + 1
                name = GetName(self.geom)
            #SetName(hypo, name + "/" + hyp + a) - NPAL16198
            SetName(hypo, hyp + a)
            pass
        status = self.mesh.mesh.AddHypothesis(self.geom, hypo)
        TreatHypoStatus( status, GetName(hypo), GetName(self.geom), 0 )
        return hypo


# Public class: Mesh_Segment
# --------------------------

## Class to define a segment 1D algorithm for discretization
#
#  More details.
class Mesh_Segment(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_Segments

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_Segment.algo:
        #17908#    Mesh_Segment.algo = self.Create(mesh, geom, "Regular_1D")
        #17908#else:
        #17908#    self.Assign( Mesh_Segment.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Regular_1D")

    ## Define "LocalLength" hypothesis to cut an edge in several segments with the same length
    #  @param l for the length of segments that cut an edge
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def LocalLength(self, l, UseExisting=0):
        hyp = self.Hypothesis("LocalLength", [l], UseExisting=UseExisting)
        hyp.SetLength(l)
        return hyp

    ## Define "NumberOfSegments" hypothesis to cut an edge in several fixed number of segments
    #  @param n for the number of segments that cut an edge
    #  @param s for the scale factor (optional)
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def NumberOfSegments(self, n, s=[], UseExisting=0):
        if s == []:
            hyp = self.Hypothesis("NumberOfSegments", [n], UseExisting=UseExisting)
        else:
            hyp = self.Hypothesis("NumberOfSegments", [n,s], UseExisting=UseExisting)
            hyp.SetDistrType( 1 )
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        return hyp

    ## Define "Arithmetic1D" hypothesis to cut an edge in several segments with arithmetic length increasing
    #  @param start for the length of the first segment
    #  @param end   for the length of the last  segment
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def Arithmetic1D(self, start, end, UseExisting=0):
        hyp = self.Hypothesis("Arithmetic1D", [start, end], UseExisting=UseExisting)
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp
        
    ## Define "StartEndLength" hypothesis to cut an edge in several segments with geometric length increasing
    #  @param start for the length of the first segment
    #  @param end   for the length of the last  segment
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def StartEndLength(self, start, end, UseExisting=0):
        hyp = self.Hypothesis("StartEndLength", [start, end], UseExisting=UseExisting)
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp
        
    ## Define "Deflection1D" hypothesis
    #  @param d for the deflection
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def Deflection1D(self, d, UseExisting=0):
        hyp = self.Hypothesis("Deflection1D", [d], UseExisting=UseExisting)
        hyp.SetDeflection(d)
        return hyp
        
    ## Define "Propagation" hypothesis that propagate all other hypothesis on all others edges that are in
    #  the opposite side in the case of quadrangular faces
    def Propagation(self):
        return self.Hypothesis("Propagation", UseExisting=1)

    ## Define "AutomaticLength" hypothesis
    #  @param fineness for the fineness [0-1]
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def AutomaticLength(self, fineness=0, UseExisting=0):
        hyp = self.Hypothesis("AutomaticLength",[fineness],UseExisting=UseExisting)
        hyp.SetFineness( fineness )
        return hyp

    ## Define "SegmentLengthAroundVertex" hypothesis
    #  @param length for the segment length
    #  @param vertex for the length localization: vertex index [0,1] | verext object
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def LengthNearVertex(self, length, vertex=0, UseExisting=0):
        import types
        store_geom = self.geom
        if vertex:
            if type(vertex) is types.IntType:
                vertex = self.mesh.geompyD.SubShapeAllSorted(self.geom, geompyDC.ShapeType["VERTEX"])[vertex]
                pass
            self.geom = vertex
            pass
        hyp = self.Hypothesis("SegmentAroundVertex_0D",[length],UseExisting=UseExisting)
        hyp = self.Hypothesis("SegmentLengthAroundVertex",[length],UseExisting=UseExisting)
        self.geom = store_geom
        hyp.SetLength( length )
        return hyp

    ## Define "QuadraticMesh" hypothesis, forcing construction of quadratic edges.
    #  If the 2D mesher sees that all boundary edges are quadratic ones,
    #  it generates quadratic faces, else it generates linear faces using
    #  medium nodes as if they were vertex ones.
    #  The 3D mesher generates quadratic volumes only if all boundary faces
    #  are quadratic ones, else it fails.
    def QuadraticMesh(self):
        hyp = self.Hypothesis("QuadraticMesh", UseExisting=1)
        return hyp

# Public class: Mesh_CompositeSegment
# --------------------------

## Class to define a segment 1D algorithm for discretization
#
#  More details.
class Mesh_CompositeSegment(Mesh_Segment):

    #17908#algo = 0 # algorithm object common for all Mesh_CompositeSegments

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        #17908#if not Mesh_CompositeSegment.algo:
        #17908#    Mesh_CompositeSegment.algo = self.Create(mesh, geom, "CompositeSegment_1D")
        #17908#else:
        #17908#    self.Assign( Mesh_CompositeSegment.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "CompositeSegment_1D")


# Public class: Mesh_Segment_Python
# ---------------------------------

## Class to define a segment 1D algorithm for discretization with python function
#
#  More details.
class Mesh_Segment_Python(Mesh_Segment):

    #17908#algo = 0 # algorithm object common for all Mesh_Segment_Pythons

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        import Python1dPlugin
        #17908#if not Mesh_Segment_Python.algo:
        #17908#    Mesh_Segment_Python.algo = self.Create(mesh, geom, "Python_1D", "libPython1dEngine.so")
        #17908#else:
        #17908#    self.Assign( Mesh_Segment_Python.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Python_1D", "libPython1dEngine.so")
    
    ## Define "PythonSplit1D" hypothesis based on the Erwan Adam patch, awaiting equivalent SALOME functionality
    #  @param n for the number of segments that cut an edge
    #  @param func for the python function that calculate the length of all segments
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def PythonSplit1D(self, n, func, UseExisting=0):
        hyp = self.Hypothesis("PythonSplit1D", [n], "libPython1dEngine.so", UseExisting=UseExisting)
        hyp.SetNumberOfSegments(n)
        hyp.SetPythonLog10RatioFunction(func)
        return hyp
        
# Public class: Mesh_Triangle
# ---------------------------

## Class to define a triangle 2D algorithm
#
#  More details.
class Mesh_Triangle(Mesh_Algorithm):

    # default values
    algoType = 0
    params = 0

    # algorithm objects common for all instances of Mesh_Triangle
    #17908#algoMEF = 0
    #17908#algoNET = 0
    #17908#algoNET_2D = 0

    ## Private constructor.
    def __init__(self, mesh, algoType, geom=0):
        Mesh_Algorithm.__init__(self)

        if algoType == MEFISTO:
            #17908#if not Mesh_Triangle.algoMEF:
            #17908#    Mesh_Triangle.algoMEF = self.Create(mesh, geom, "MEFISTO_2D")
            #17908#else:
            #17908#    self.Assign( Mesh_Triangle.algoMEF, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "MEFISTO_2D")
            pass
        elif algoType == NETGEN:
            if noNETGENPlugin:
                print "Warning: NETGENPlugin module unavailable"
                pass
            #17908#if not Mesh_Triangle.algoNET:
            #17908#    Mesh_Triangle.algoNET = self.Create(mesh, geom, "NETGEN_2D", "libNETGENEngine.so")
            #17908#else:
            #17908#    self.Assign( Mesh_Triangle.algoNET, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "NETGEN_2D", "libNETGENEngine.so")
            pass
        elif algoType == NETGEN_2D:
            if noNETGENPlugin:
                print "Warning: NETGENPlugin module unavailable"
                pass
            #17908#if not Mesh_Triangle.algoNET_2D:
            #17908#    Mesh_Triangle.algoNET_2D = self.Create(mesh, geom,
            #17908#                                          "NETGEN_2D_ONLY", "libNETGENEngine.so")
            #17908#else:
            #17908#    self.Assign( Mesh_Triangle.algoNET_2D, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "NETGEN_2D_ONLY", "libNETGENEngine.so")
            pass

        self.algoType = algoType

    ## Define "MaxElementArea" hypothesis to give the maximun area of each triangles
    #  @param area for the maximum area of each triangles
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    #
    #  Only for algoType == MEFISTO || NETGEN_2D
    def MaxElementArea(self, area, UseExisting=0):
        if self.algoType == MEFISTO or self.algoType == NETGEN_2D:
            hyp = self.Hypothesis("MaxElementArea", [area], UseExisting=UseExisting)
            hyp.SetMaxElementArea(area)
            return hyp
        elif self.algoType == NETGEN:
            print "Netgen 1D-2D algo doesn't support this hypothesis"
            return None

    ## Define "LengthFromEdges" hypothesis to build triangles based on the length of the edges taken from the wire
    #
    #  Only for algoType == MEFISTO || NETGEN_2D
    def LengthFromEdges(self):
        if self.algoType == MEFISTO or self.algoType == NETGEN_2D:
            hyp = self.Hypothesis("LengthFromEdges", UseExisting=1)
            return hyp
        elif self.algoType == NETGEN:
            print "Netgen 1D-2D algo doesn't support this hypothesis"
            return None

    ## Set QuadAllowed flag
    #
    #  Only for algoType == NETGEN || NETGEN_2D
    def SetQuadAllowed(self, toAllow=True):
        if self.algoType == NETGEN_2D:
            if toAllow: # add QuadranglePreference
                self.Hypothesis("QuadranglePreference", UseExisting=1)
            else:       # remove QuadranglePreference
                for hyp in self.mesh.GetHypothesisList( self.geom ):
                    if hyp.GetName() == "QuadranglePreference":
                        self.mesh.RemoveHypothesis( self.geom, hyp )
                        pass
                    pass
                pass
            return
        if self.params == 0 and self.Parameters():
            self.params.SetQuadAllowed(toAllow)
            return

    ## Define "Netgen 2D Parameters" hypothesis
    #
    #  Only for algoType == NETGEN
    def Parameters(self):
        if self.algoType == NETGEN:
            self.params = self.Hypothesis("NETGEN_Parameters_2D", [],
                                          "libNETGENEngine.so", UseExisting=0)
            return self.params
        elif self.algoType == MEFISTO:
            print "Mefisto algo doesn't support NETGEN_Parameters_2D hypothesis"
            return None
        elif self.algoType == NETGEN_2D:
            print "NETGEN_2D_ONLY algo doesn't support 'NETGEN_Parameters_2D' hypothesis"
            print "NETGEN_2D_ONLY uses 'MaxElementArea' and 'LengthFromEdges' ones"
            return None
        return None

    ## Set MaxSize
    #
    #  Only for algoType == NETGEN
    def SetMaxSize(self, theSize):
        if self.params == 0:
            self.Parameters()
        if self.params is not None:
            self.params.SetMaxSize(theSize)

    ## Set SecondOrder flag
    #
    #  Only for algoType == NETGEN
    def SetSecondOrder(self, theVal):
        if self.params == 0:
            self.Parameters()
        if self.params is not None:
            self.params.SetSecondOrder(theVal)

    ## Set Optimize flag
    #
    #  Only for algoType == NETGEN
    def SetOptimize(self, theVal):
        if self.params == 0:
            self.Parameters()
        if self.params is not None:
            self.params.SetOptimize(theVal)

    ## Set Fineness
    #  @param theFineness is:
    #  VeryCoarse, Coarse, Moderate, Fine, VeryFine or Custom
    #
    #  Only for algoType == NETGEN
    def SetFineness(self, theFineness):
        if self.params == 0:
            self.Parameters()
        if self.params is not None:
            self.params.SetFineness(theFineness)

    ## Set GrowthRate
    #
    #  Only for algoType == NETGEN
    def SetGrowthRate(self, theRate):
        if self.params == 0:
            self.Parameters()
        if self.params is not None:
            self.params.SetGrowthRate(theRate)

    ## Set NbSegPerEdge
    #
    #  Only for algoType == NETGEN
    def SetNbSegPerEdge(self, theVal):
        if self.params == 0:
            self.Parameters()
        if self.params is not None:
            self.params.SetNbSegPerEdge(theVal)

    ## Set NbSegPerRadius
    #
    #  Only for algoType == NETGEN
    def SetNbSegPerRadius(self, theVal):
        if self.params == 0:
            self.Parameters()
        if self.params is not None:
            self.params.SetNbSegPerRadius(theVal)

    pass


# Public class: Mesh_Quadrangle
# -----------------------------

## Class to define a quadrangle 2D algorithm
#
#  More details.
class Mesh_Quadrangle(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_Quadrangles

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_Quadrangle.algo:
        #17908#    Mesh_Quadrangle.algo = self.Create(mesh, geom, "Quadrangle_2D")
        #17908#else:
        #17908#    self.Assign( Mesh_Quadrangle.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Quadrangle_2D")
    
    ## Define "QuadranglePreference" hypothesis, forcing construction
    #  of quadrangles if the number of nodes on opposite edges is not the same
    #  in the case where the global number of nodes on edges is even
    def QuadranglePreference(self):
        hyp = self.Hypothesis("QuadranglePreference", UseExisting=1)
        return hyp
    
# Public class: Mesh_Tetrahedron
# ------------------------------

## Class to define a tetrahedron 3D algorithm
#
#  More details.
class Mesh_Tetrahedron(Mesh_Algorithm):

    params = 0
    algoType = 0

    #17908#algoNET = 0 # algorithm object common for all Mesh_Tetrahedrons
    #17908#algoGHS = 0 # algorithm object common for all Mesh_Tetrahedrons
    #17908#algoFNET = 0 # algorithm object common for all Mesh_Tetrahedrons

    ## Private constructor.
    def __init__(self, mesh, algoType, geom=0):
        Mesh_Algorithm.__init__(self)

        if algoType == NETGEN:
            #17908#if not Mesh_Tetrahedron.algoNET:
            #17908#    Mesh_Tetrahedron.algoNET = self.Create(mesh, geom, "NETGEN_3D", "libNETGENEngine.so")
            #17908#else:
            #17908#    self.Assign( Mesh_Tetrahedron.algoNET, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "NETGEN_3D", "libNETGENEngine.so")
            pass

        elif algoType == GHS3D:
            #17908#if not Mesh_Tetrahedron.algoGHS:
            #17908#    import GHS3DPlugin
            #17908#    Mesh_Tetrahedron.algoGHS = self.Create(mesh, geom, "GHS3D_3D" , "libGHS3DEngine.so")
            #17908#else:
            #17908#    self.Assign( Mesh_Tetrahedron.algoGHS, mesh, geom)
            #17908#    pass
            import GHS3DPlugin
            self.Create(mesh, geom, "GHS3D_3D" , "libGHS3DEngine.so")
            pass

        elif algoType == FULL_NETGEN:
            if noNETGENPlugin:
                print "Warning: NETGENPlugin module has not been imported."
            #17908#if not Mesh_Tetrahedron.algoFNET:
            #17908#    Mesh_Tetrahedron.algoFNET = self.Create(mesh, geom, "NETGEN_2D3D", "libNETGENEngine.so")
            #17908#else:
            #17908#    self.Assign( Mesh_Tetrahedron.algoFNET, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "NETGEN_2D3D", "libNETGENEngine.so")
            pass

        self.algoType = algoType

    ## Define "MaxElementVolume" hypothesis to give the maximun volume of each tetrahedral
    #  @param vol for the maximum volume of each tetrahedral
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def MaxElementVolume(self, vol, UseExisting=0):
        hyp = self.Hypothesis("MaxElementVolume", [vol], UseExisting=UseExisting)
        hyp.SetMaxElementVolume(vol)
        return hyp

    ## Define "Netgen 3D Parameters" hypothesis
    def Parameters(self):
        if (self.algoType == FULL_NETGEN):
            self.params = self.Hypothesis("NETGEN_Parameters", [],
                                          "libNETGENEngine.so", UseExisting=0)
            return self.params
        else:
            print "Algo doesn't support this hypothesis"
            return None

    ## Set MaxSize
    def SetMaxSize(self, theSize):
        if self.params == 0:
            self.Parameters()
        self.params.SetMaxSize(theSize)
        
    ## Set SecondOrder flag
    def SetSecondOrder(self, theVal):
        if self.params == 0:
            self.Parameters()
        self.params.SetSecondOrder(theVal)

    ## Set Optimize flag
    def SetOptimize(self, theVal):
        if self.params == 0:
            self.Parameters()
        self.params.SetOptimize(theVal)

    ## Set Fineness
    #  @param theFineness is:
    #  VeryCoarse, Coarse, Moderate, Fine, VeryFine or Custom
    def SetFineness(self, theFineness):
        if self.params == 0:
            self.Parameters()
        self.params.SetFineness(theFineness)

    ## Set GrowthRate
    def SetGrowthRate(self, theRate):
        if self.params == 0:
            self.Parameters()
        self.params.SetGrowthRate(theRate)

    ## Set NbSegPerEdge
    def SetNbSegPerEdge(self, theVal):
        if self.params == 0:
            self.Parameters()
        self.params.SetNbSegPerEdge(theVal)

    ## Set NbSegPerRadius
    def SetNbSegPerRadius(self, theVal):
        if self.params == 0:
            self.Parameters()
        self.params.SetNbSegPerRadius(theVal)

# Public class: Mesh_Hexahedron
# ------------------------------

## Class to define a hexahedron 3D algorithm
#
#  More details.
class Mesh_Hexahedron(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_Hexahedrons

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_Hexahedron.algo:
        #17908#    Mesh_Hexahedron.algo = self.Create(mesh, geom, "Hexa_3D")
        #17908#else:
        #17908#    self.Assign( Mesh_Hexahedron.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Hexa_3D")

# Deprecated, only for compatibility!
# Public class: Mesh_Netgen
# ------------------------------

## Class to define a NETGEN-based 2D or 3D algorithm
#  that need no discrete boundary (i.e. independent)
#
#  This class is deprecated, only for compatibility!
#
#  More details.
class Mesh_Netgen(Mesh_Algorithm):

    is3D = 0

    #17908#algoNET23 = 0 # algorithm object common for all Mesh_Netgens
    #17908#algoNET2 = 0 # algorithm object common for all Mesh_Netgens

    ## Private constructor.
    def __init__(self, mesh, is3D, geom=0):
        Mesh_Algorithm.__init__(self)

        if noNETGENPlugin:
            print "Warning: NETGENPlugin module has not been imported."
            
        self.is3D = is3D
        if is3D:
            #17908#if not Mesh_Netgen.algoNET23:
            #17908#    Mesh_Netgen.algoNET23 = self.Create(mesh, geom, "NETGEN_2D3D", "libNETGENEngine.so")
            #17908#else:
            #17908#    self.Assign( Mesh_Netgen.algoNET23, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "NETGEN_2D3D", "libNETGENEngine.so")
            pass

        else:
            #17908#if not Mesh_Netgen.algoNET2:
            #17908#    Mesh_Netgen.algoNET2 = self.Create(mesh, geom, "NETGEN_2D", "libNETGENEngine.so")
            #17908#else:
            #17908#    self.Assign( Mesh_Netgen.algoNET2, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "NETGEN_2D", "libNETGENEngine.so")
            pass

    ## Define hypothesis containing parameters of the algorithm
    def Parameters(self):
        if self.is3D:
            hyp = self.Hypothesis("NETGEN_Parameters", [],
                                  "libNETGENEngine.so", UseExisting=0)
        else:
            hyp = self.Hypothesis("NETGEN_Parameters_2D", [],
                                  "libNETGENEngine.so", UseExisting=0)
        return hyp

# Public class: Mesh_Projection1D
# ------------------------------

## Class to define a projection 1D algorithm
#
#  More details.
class Mesh_Projection1D(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_Projection1Ds

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_Projection1D.algo:
        #17908#    Mesh_Projection1D.algo = self.Create(mesh, geom, "Projection_1D")
        #17908#else:
        #17908#    self.Assign( Mesh_Projection1D.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Projection_1D")

    ## Define "Source Edge" hypothesis, specifying a meshed edge to
    #  take a mesh pattern from, and optionally association of vertices
    #  between the source edge and a target one (where a hipothesis is assigned to)
    #  @param edge to take nodes distribution from
    #  @param mesh to take nodes distribution from (optional)
    #  @param srcV is vertex of \a edge to associate with \a tgtV (optional)
    #  @param tgtV is vertex of \a the edge where the algorithm is assigned,
    #  to associate with \a srcV (optional)
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def SourceEdge(self, edge, mesh=None, srcV=None, tgtV=None, UseExisting=0):
        hyp = self.Hypothesis("ProjectionSource1D", [edge,mesh,srcV,tgtV], UseExisting=UseExisting)
        hyp.SetSourceEdge( edge )
        if not mesh is None and isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV, tgtV )
        return hyp


# Public class: Mesh_Projection2D
# ------------------------------

## Class to define a projection 2D algorithm
#
#  More details.
class Mesh_Projection2D(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_Projection2Ds

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_Projection2D.algo:
        #17908#    Mesh_Projection2D.algo = self.Create(mesh, geom, "Projection_2D")
        #17908#else:
        #17908#    self.Assign( Mesh_Projection2D.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Projection_2D")

    ## Define "Source Face" hypothesis, specifying a meshed face to
    #  take a mesh pattern from, and optionally association of vertices
    #  between the source face and a target one (where a hipothesis is assigned to)
    #  @param face to take mesh pattern from
    #  @param mesh to take mesh pattern from (optional)
    #  @param srcV1 is vertex of \a face to associate with \a tgtV1 (optional)
    #  @param tgtV1 is vertex of \a the face where the algorithm is assigned,
    #  to associate with \a srcV1 (optional)
    #  @param srcV2 is vertex of \a face to associate with \a tgtV1 (optional)
    #  @param tgtV2 is vertex of \a the face where the algorithm is assigned,
    #  to associate with \a srcV2 (optional)
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    #
    #  Note: association vertices must belong to one edge of a face
    def SourceFace(self, face, mesh=None, srcV1=None, tgtV1=None,
                   srcV2=None, tgtV2=None, UseExisting=0):
        hyp = self.Hypothesis("ProjectionSource2D", [face,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=UseExisting)
        hyp.SetSourceFace( face )
        if not mesh is None and isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        return hyp

# Public class: Mesh_Projection3D
# ------------------------------

## Class to define a projection 3D algorithm
#
#  More details.
class Mesh_Projection3D(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_Projection3Ds

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_Projection3D.algo:
        #17908#    Mesh_Projection3D.algo = self.Create(mesh, geom, "Projection_3D")
        #17908#else:
        #17908#    self.Assign( Mesh_Projection3D.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Projection_3D")

    ## Define "Source Shape 3D" hypothesis, specifying a meshed solid to
    #  take a mesh pattern from, and optionally association of vertices
    #  between the source solid and a target one (where a hipothesis is assigned to)
    #  @param solid to take mesh pattern from
    #  @param mesh to take mesh pattern from (optional)
    #  @param srcV1 is vertex of \a solid to associate with \a tgtV1 (optional)
    #  @param tgtV1 is vertex of \a the solid where the algorithm is assigned,
    #  to associate with \a srcV1 (optional)
    #  @param srcV2 is vertex of \a solid to associate with \a tgtV1 (optional)
    #  @param tgtV2 is vertex of \a the solid where the algorithm is assigned,
    #  to associate with \a srcV2 (optional)
    #  @param UseExisting - if ==true - search existing hypothesis created with
    #                       same parameters, else (default) - create new
    #
    #  Note: association vertices must belong to one edge of a solid
    def SourceShape3D(self, solid, mesh=0, srcV1=0, tgtV1=0,
                      srcV2=0, tgtV2=0, UseExisting=0):
        hyp = self.Hypothesis("ProjectionSource3D",
                              [solid,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=UseExisting)
        hyp.SetSource3DShape( solid )
        if not mesh is None and isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        return hyp


# Public class: Mesh_Prism
# ------------------------

## Class to define a 3D extrusion algorithm
#
#  More details.
class Mesh_Prism3D(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_Prism3Ds

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_Prism3D.algo:
        #17908#    Mesh_Prism3D.algo = self.Create(mesh, geom, "Prism_3D")
        #17908#else:
        #17908#    self.Assign( Mesh_Prism3D.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "Prism_3D")

# Public class: Mesh_RadialPrism
# -------------------------------

## Class to define a Radial Prism 3D algorithm
#
#  More details.
class Mesh_RadialPrism3D(Mesh_Algorithm):

    #17908#algo = 0 # algorithm object common for all Mesh_RadialPrism3Ds

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)

        #17908#if not Mesh_RadialPrism3D.algo:
        #17908#    Mesh_RadialPrism3D.algo = self.Create(mesh, geom, "RadialPrism_3D")
        #17908#else:
        #17908#    self.Assign( Mesh_RadialPrism3D.algo, mesh, geom)
        #17908#    pass
        self.Create(mesh, geom, "RadialPrism_3D")

        self.distribHyp = self.Hypothesis( "LayerDistribution", UseExisting=0)
        self.nbLayers = None

    ## Return 3D hypothesis holding the 1D one
    def Get3DHypothesis(self):
        return self.distribHyp

    ## Private method creating 1D hypothes and storing it in the LayerDistribution
    #  hypothes. Returns the created hypothes
    def OwnHypothesis(self, hypType, args=[], so="libStdMeshersEngine.so"):
        print "OwnHypothesis",hypType
        if not self.nbLayers is None:
            self.mesh.GetMesh().RemoveHypothesis( self.geom, self.nbLayers )
            self.mesh.GetMesh().AddHypothesis( self.geom, self.distribHyp )
        study = self.mesh.smeshpyD.GetCurrentStudy() # prevent publishing of own 1D hypothesis
        hyp = self.mesh.smeshpyD.CreateHypothesis(hypType, so)
        self.mesh.smeshpyD.SetCurrentStudy( study ) # anable publishing
        self.distribHyp.SetLayerDistribution( hyp )
        return hyp

    ## Define "NumberOfLayers" hypothesis, specifying a number of layers of
    #  prisms to build between the inner and outer shells
    #  @param UseExisting if ==true - search existing hypothesis created with
    #                     same parameters, else (default) - create new
    def NumberOfLayers(self, n, UseExisting=0):
        self.mesh.GetMesh().RemoveHypothesis( self.geom, self.distribHyp )
        self.nbLayers = self.Hypothesis("NumberOfLayers", [n], UseExisting=UseExisting)
        self.nbLayers.SetNumberOfLayers( n )
        return self.nbLayers

    ## Define "LocalLength" hypothesis, specifying segment length
    #  to build between the inner and outer shells
    #  @param l for the length of segments
    def LocalLength(self, l):
        hyp = self.OwnHypothesis("LocalLength", [l] )
        hyp.SetLength(l)
        return hyp
        
    ## Define "NumberOfSegments" hypothesis, specifying a number of layers of
    #  prisms to build between the inner and outer shells
    #  @param n for the number of segments
    #  @param s for the scale factor (optional)
    def NumberOfSegments(self, n, s=[]):
        if s == []:
            hyp = self.OwnHypothesis("NumberOfSegments", [n] )
        else:
            hyp = self.OwnHypothesis("NumberOfSegments", [n,s])
            hyp.SetDistrType( 1 )
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        return hyp
        
    ## Define "Arithmetic1D" hypothesis, specifying distribution of segments
    #  to build between the inner and outer shells as arithmetic length increasing
    #  @param start for the length of the first segment
    #  @param end   for the length of the last  segment
    def Arithmetic1D(self, start, end ):
        hyp = self.OwnHypothesis("Arithmetic1D", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp
        
    ## Define "StartEndLength" hypothesis, specifying distribution of segments
    #  to build between the inner and outer shells as geometric length increasing
    #  @param start for the length of the first segment
    #  @param end   for the length of the last  segment
    def StartEndLength(self, start, end):
        hyp = self.OwnHypothesis("StartEndLength", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp
        
    ## Define "AutomaticLength" hypothesis, specifying number of segments
    #  to build between the inner and outer shells
    #  @param fineness for the fineness [0-1]
    def AutomaticLength(self, fineness=0):
        hyp = self.OwnHypothesis("AutomaticLength")
        hyp.SetFineness( fineness )
        return hyp

# Private class: Mesh_UseExisting
# -------------------------------
class Mesh_UseExisting(Mesh_Algorithm):

    #17908#algo1D = 0 # StdMeshers_UseExisting_1D object common for all Mesh_UseExisting
    #17908#algo2D = 0 # StdMeshers_UseExisting_2D object common for all Mesh_UseExisting

    def __init__(self, dim, mesh, geom=0):
        if dim == 1:
            #17908#if not Mesh_UseExisting.algo1D:
            #17908#    Mesh_UseExisting.algo1D= self.Create(mesh, geom, "UseExisting_1D")
            #17908#else:
            #17908#    self.Assign( Mesh_UseExisting.algo1D, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "UseExisting_1D")
        else:
            #17908#if not Mesh_UseExisting.algo2D:
            #17908#    Mesh_UseExisting.algo2D= self.Create(mesh, geom, "UseExisting_2D")
            #17908#else:
            #17908#    self.Assign( Mesh_UseExisting.algo2D, mesh, geom)
            #17908#    pass
            self.Create(mesh, geom, "UseExisting_2D")

# Public class: Mesh
# ==================

## Class to define a mesh
#
#  The class contains mesh shape, SMESH_Mesh, SMESH_MeshEditor
#  More details.
class Mesh:

    geom = 0
    mesh = 0
    editor = 0

    ## Constructor
    #
    #  Creates mesh on the shape \a geom(or the empty mesh if geom equal to 0),
    #  sets GUI name of this mesh to \a name.
    #  @param obj Shape to be meshed or SMESH_Mesh object
    #  @param name Study name of the mesh
    def __init__(self, smeshpyD, geompyD, obj=0, name=0):
        self.smeshpyD=smeshpyD
        self.geompyD=geompyD
        if obj is None:
            obj = 0
        if obj != 0:
            if isinstance(obj, geompyDC.GEOM._objref_GEOM_Object):
                self.geom = obj
                self.mesh = self.smeshpyD.CreateMesh(self.geom)
            elif isinstance(obj, SMESH._objref_SMESH_Mesh):
                self.SetMesh(obj)
        else:
            self.mesh = self.smeshpyD.CreateEmptyMesh()
        if name != 0:
            SetName(self.mesh, name)
        elif obj != 0:
            SetName(self.mesh, GetName(obj))

        self.editor = self.mesh.GetMeshEditor()

    ## Method that inits the Mesh object from SMESH_Mesh interface
    #  @param theMesh is SMESH_Mesh object
    def SetMesh(self, theMesh):
        self.mesh = theMesh
        self.geom = self.mesh.GetShapeToMesh()
            
    ## Method that returns the mesh
    #  @return SMESH_Mesh object
    def GetMesh(self):
        return self.mesh

    ## Get mesh name
    def GetName(self):
        name = GetName(self.GetMesh())
        return name

    ## Set name to mesh
    def SetName(self, name):
        SetName(self.GetMesh(), name)
    
    ## Get the subMesh object associated to a subShape. The subMesh object
    #  gives access to nodes and elements IDs.
    #  \n SubMesh will be used instead of SubShape in a next idl version to
    #  adress a specific subMesh...
    def GetSubMesh(self, theSubObject, name):
        submesh = self.mesh.GetSubMesh(theSubObject, name)
        return submesh
        
    ## Method that returns the shape associated to the mesh
    #  @return GEOM_Object
    def GetShape(self):
        return self.geom

    ## Method that associates given shape to the mesh(entails the mesh recreation)
    #  @param geom shape to be meshed(GEOM_Object)
    def SetShape(self, geom):
        self.mesh = self.smeshpyD.CreateMesh(geom)

    ## Return true if hypotheses are defined well
    #  @param theMesh is an instance of Mesh class
    #  @param theSubObject subshape of a mesh shape
    def IsReadyToCompute(self, theSubObject):
        return self.smeshpyD.IsReadyToCompute(self.mesh, theSubObject)

    ## Return errors of hypotheses definintion
    #  error list is empty if everything is OK
    #  @param theMesh is an instance of Mesh class
    #  @param theSubObject subshape of a mesh shape
    #  @return a list of errors
    def GetAlgoState(self, theSubObject):
        return self.smeshpyD.GetAlgoState(self.mesh, theSubObject)
    
    ## Return geometrical object the given element is built on.
    #  The returned geometrical object, if not nil, is either found in the
    #  study or is published by this method with the given name
    #  @param theMesh is an instance of Mesh class
    #  @param theElementID an id of the mesh element
    #  @param theGeomName user defined name of geometrical object
    #  @return GEOM::GEOM_Object instance
    def GetGeometryByMeshElement(self, theElementID, theGeomName):
        return self.smeshpyD.GetGeometryByMeshElement( self.mesh, theElementID, theGeomName )
        
    ## Returns mesh dimension depending on shape one
    def MeshDimension(self):
        shells = self.geompyD.SubShapeAllIDs( self.geom, geompyDC.ShapeType["SHELL"] )
        if len( shells ) > 0 :
            return 3
        elif self.geompyD.NumberOfFaces( self.geom ) > 0 :
            return 2
        elif self.geompyD.NumberOfEdges( self.geom ) > 0 :
            return 1
        else:
            return 0;
        pass
        
    ## Creates a segment discretization 1D algorithm.
    #  If the optional \a algo parameter is not sets, this algorithm is REGULAR.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  \n Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param algo values are smesh.REGULAR or smesh.PYTHON for discretization via python function
    #  @param geom If defined, subshape to be meshed
    def Segment(self, algo=REGULAR, geom=0):
        ## if Segment(geom) is called by mistake
        if isinstance( algo, geompyDC.GEOM._objref_GEOM_Object):
            algo, geom = geom, algo
            if not algo: algo = REGULAR
            pass
        if algo == REGULAR:
            return Mesh_Segment(self,  geom)
        elif algo == PYTHON:
            return Mesh_Segment_Python(self, geom)
        elif algo == COMPOSITE:
            return Mesh_CompositeSegment(self, geom)
        else:
            return Mesh_Segment(self, geom)
        
    ## Enable creation of nodes and segments usable by 2D algoritms.
    #  Added nodes and segments must be bound to edges and vertices by
    #  SetNodeOnVertex(), SetNodeOnEdge() and SetMeshElementOnShape()
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  \n Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom subshape to be manually meshed
    #  @return StdMeshers_UseExisting_1D algorithm that generates nothing
    def UseExistingSegments(self, geom=0):
        algo = Mesh_UseExisting(1,self,geom)
        return algo.GetAlgorithm()

    ## Enable creation of nodes and faces usable by 3D algoritms.
    #  Added nodes and faces must be bound to geom faces by SetNodeOnFace()
    #  and SetMeshElementOnShape()
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  \n Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom subshape to be manually meshed
    #  @return StdMeshers_UseExisting_2D algorithm that generates nothing
    def UseExistingFaces(self, geom=0):
        algo = Mesh_UseExisting(2,self,geom)
        return algo.GetAlgorithm()

    ## Creates a triangle 2D algorithm for faces.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  \n Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param algo values are: smesh.MEFISTO || smesh.NETGEN_1D2D || smesh.NETGEN_2D
    #  @param geom If defined, subshape to be meshed
    def Triangle(self, algo=MEFISTO, geom=0):
        ## if Triangle(geom) is called by mistake
        if ( isinstance( algo, geompyDC.GEOM._objref_GEOM_Object)):
            geom = algo
            algo = MEFISTO
        
        return Mesh_Triangle(self,  algo, geom)
        
    ## Creates a quadrangle 2D algorithm for faces.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  \n Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom If defined, subshape to be meshed
    def Quadrangle(self, geom=0):
        return Mesh_Quadrangle(self,  geom)

    ## Creates a tetrahedron 3D algorithm for solids.
    #  The parameter \a algo permits to choice the algorithm: NETGEN or GHS3D
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  \n Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param algo values are: smesh.NETGEN, smesh.GHS3D, smesh.FULL_NETGEN
    #  @param geom If defined, subshape to be meshed
    def Tetrahedron(self, algo=NETGEN, geom=0):
        ## if Tetrahedron(geom) is called by mistake
        if ( isinstance( algo, geompyDC.GEOM._objref_GEOM_Object)):
            algo, geom = geom, algo
            if not algo: algo = NETGEN
            pass
        return Mesh_Tetrahedron(self,  algo, geom)
        
    ## Creates a hexahedron 3D algorithm for solids.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  \n Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom If defined, subshape to be meshed
    def Hexahedron(self, geom=0):
        return Mesh_Hexahedron(self,  geom)

    ## Deprecated, only for compatibility!
    def Netgen(self, is3D, geom=0):
        return Mesh_Netgen(self,  is3D, geom)

    ## Creates a projection 1D algorithm for edges.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom If defined, subshape to be meshed
    def Projection1D(self, geom=0):
        return Mesh_Projection1D(self,  geom)

    ## Creates a projection 2D algorithm for faces.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom If defined, subshape to be meshed
    def Projection2D(self, geom=0):
        return Mesh_Projection2D(self,  geom)

    ## Creates a projection 3D algorithm for solids.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom If defined, subshape to be meshed
    def Projection3D(self, geom=0):
        return Mesh_Projection3D(self,  geom)

    ## Creates a 3D extrusion (Prism 3D) or RadialPrism 3D algorithm for solids.
    #  If the optional \a geom parameter is not sets, this algorithm is global.
    #  Otherwise, this algorithm define a submesh based on \a geom subshape.
    #  @param geom If defined, subshape to be meshed
    def Prism(self, geom=0):
        shape = geom
        if shape==0:
            shape = self.geom
        nbSolids = len( self.geompyD.SubShapeAll( shape, geompyDC.ShapeType["SOLID"] ))
        nbShells = len( self.geompyD.SubShapeAll( shape, geompyDC.ShapeType["SHELL"] ))
        if nbSolids == 0 or nbSolids == nbShells:
            return Mesh_Prism3D(self,  geom)
        return Mesh_RadialPrism3D(self,  geom)

    ## Compute the mesh and return the status of the computation
    def Compute(self, geom=0):
        if geom == 0 or not isinstance(geom, geompyDC.GEOM._objref_GEOM_Object):
            if self.geom == 0:
                print "Compute impossible: mesh is not constructed on geom shape."
                return 0
            else:
                geom = self.geom
        ok = False
        try:
            ok = self.smeshpyD.Compute(self.mesh, geom)
        except SALOME.SALOME_Exception, ex:
            print "Mesh computation failed, exception caught:"
            print "    ", ex.details.text
        except:
            import traceback
            print "Mesh computation failed, exception caught:"
            traceback.print_exc()
        if not ok:
            errors = self.smeshpyD.GetAlgoState( self.mesh, geom )
            allReasons = ""
            for err in errors:
                if err.isGlobalAlgo:
                    glob = "global"
                else:
                    glob = "local"
                    pass
                dim = err.algoDim
                name = err.algoName
                if len(name) == 0:
                    reason = '%s %sD algorithm is missing' % (glob, dim)
                elif err.state == HYP_MISSING:
                    reason = ('%s %sD algorithm "%s" misses %sD hypothesis'
                              % (glob, dim, name, dim))
                elif err.state == HYP_NOTCONFORM:
                    reason = 'Global "Not Conform mesh allowed" hypothesis is missing'
                elif err.state == HYP_BAD_PARAMETER:
                    reason = ('Hypothesis of %s %sD algorithm "%s" has a bad parameter value'
                              % ( glob, dim, name ))
                elif err.state == HYP_BAD_GEOMETRY:
                    reason = ('%s %sD algorithm "%s" is assigned to geometry mismatching'
                              'its expectation' % ( glob, dim, name ))
                else:
                    reason = "For unknown reason."+\
                             " Revise Mesh.Compute() implementation in smesh.py!"
                    pass
                if allReasons != "":
                    allReasons += "\n"
                    pass
                allReasons += reason
                pass
            if allReasons != "":
                print '"' + GetName(self.mesh) + '"',"has not been computed:"
                print allReasons
            else:
                print '"' + GetName(self.mesh) + '"',"has not been computed."
                pass
            pass
        if salome.sg.hasDesktop():
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(salome.myStudyId)
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), ok, (self.NbNodes()==0) )
            salome.sg.updateObjBrowser(1)
            pass
        return ok

    ## Compute tetrahedral mesh using AutomaticLength + MEFISTO + NETGEN
    #  The parameter \a fineness [0,-1] defines mesh fineness
    def AutomaticTetrahedralization(self, fineness=0):
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
        
    ## Compute hexahedral mesh using AutomaticLength + Quadrangle + Hexahedron
    #  The parameter \a fineness [0,-1] defines mesh fineness
    def AutomaticHexahedralization(self, fineness=0):
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

    ## Assign hypothesis
    #  @param hyp is a hypothesis to assign
    #  @param geom is subhape of mesh geometry
    def AddHypothesis(self, hyp, geom=0 ):
        if isinstance( hyp, Mesh_Algorithm ):
            hyp = hyp.GetAlgorithm()
            pass
        if not geom:
            geom = self.geom
            pass
        status = self.mesh.AddHypothesis(geom, hyp)
        isAlgo = hyp._narrow( SMESH_Algo )
        TreatHypoStatus( status, GetName( hyp ), GetName( geom ), isAlgo )
        return status
    
    ## Unassign hypothesis
    #  @param hyp is a hypothesis to unassign
    #  @param geom is subhape of mesh geometry
    def RemoveHypothesis(self, hyp, geom=0 ):
        if isinstance( hyp, Mesh_Algorithm ):
            hyp = hyp.GetAlgorithm()
            pass
        if not geom:
            geom = self.geom
            pass
        status = self.mesh.RemoveHypothesis(geom, hyp)
        return status

    ## Get the list of hypothesis added on a geom
    #  @param geom is subhape of mesh geometry
    def GetHypothesisList(self, geom):
        return self.mesh.GetHypothesisList( geom )
                
    ## Removes all global hypotheses
    def RemoveGlobalHypotheses(self):
        current_hyps = self.mesh.GetHypothesisList( self.geom )
        for hyp in current_hyps:
            self.mesh.RemoveHypothesis( self.geom, hyp )
            pass
        pass
        
    ## Create a mesh group based on geometric object \a grp
    #  and give a \a name, \n if this parameter is not defined
    #  the name is the same as the geometric group name \n
    #  Note: Works like GroupOnGeom().
    #  @param grp  is a geometric group, a vertex, an edge, a face or a solid
    #  @param name is the name of the mesh group
    #  @return SMESH_GroupOnGeom
    def Group(self, grp, name=""):
        return self.GroupOnGeom(grp, name)
       
    ## Deprecated, only for compatibility! Please, use ExportMED() method instead.
    #  Export the mesh in a file with the MED format and choice the \a version of MED format
    #  @param f is the file name
    #  @param version values are SMESH.MED_V2_1, SMESH.MED_V2_2
    def ExportToMED(self, f, version, opt=0):
        self.mesh.ExportToMED(f, opt, version)
        
    ## Export the mesh in a file with the MED format
    #  @param f is the file name
    #  @param auto_groups boolean parameter for creating/not creating
    #  the groups Group_On_All_Nodes, Group_On_All_Faces, ... ;
    #  the typical use is auto_groups=false.
    #  @param version MED format version(MED_V2_1 or MED_V2_2)
    def ExportMED(self, f, auto_groups=0, version=MED_V2_2):
        self.mesh.ExportToMED(f, auto_groups, version)
        
    ## Export the mesh in a file with the DAT format
    #  @param f is the file name
    def ExportDAT(self, f):
        self.mesh.ExportDAT(f)
        
    ## Export the mesh in a file with the UNV format
    #  @param f is the file name
    def ExportUNV(self, f):
        self.mesh.ExportUNV(f)
        
    ## Export the mesh in a file with the STL format
    #  @param f is the file name
    #  @param ascii defined the kind of file contents
    def ExportSTL(self, f, ascii=1):
        self.mesh.ExportSTL(f, ascii)
   
        
    # Operations with groups:
    # ----------------------

    ## Creates an empty mesh group
    #  @param elementType is the type of elements in the group
    #  @param name is the name of the mesh group
    #  @return SMESH_Group
    def CreateEmptyGroup(self, elementType, name):
        return self.mesh.CreateGroup(elementType, name)
    
    ## Creates a mesh group based on geometric object \a grp
    #  and give a \a name, \n if this parameter is not defined
    #  the name is the same as the geometric group name
    #  @param grp  is a geometric group, a vertex, an edge, a face or a solid
    #  @param name is the name of the mesh group
    #  @return SMESH_GroupOnGeom
    def GroupOnGeom(self, grp, name="", typ=None):
        if name == "":
            name = grp.GetName()

        if typ == None:
            tgeo = str(grp.GetShapeType())
            if tgeo == "VERTEX":
                typ = NODE
            elif tgeo == "EDGE":
                typ = EDGE
            elif tgeo == "FACE":
                typ = FACE
            elif tgeo == "SOLID":
                typ = VOLUME
            elif tgeo == "SHELL":
                typ = VOLUME
            elif tgeo == "COMPOUND":
                if len( self.geompyD.GetObjectIDs( grp )) == 0:
                    print "Mesh.Group: empty geometric group", GetName( grp )
                    return 0
                tgeo = self.geompyD.GetType(grp)
                if tgeo == geompyDC.ShapeType["VERTEX"]:
                    typ = NODE
                elif tgeo == geompyDC.ShapeType["EDGE"]:
                    typ = EDGE
                elif tgeo == geompyDC.ShapeType["FACE"]:
                    typ = FACE
                elif tgeo == geompyDC.ShapeType["SOLID"]:
                    typ = VOLUME

        if typ == None:
            print "Mesh.Group: bad first argument: expected a group, a vertex, an edge, a face or a solid"
            return 0
        else:
            return self.mesh.CreateGroupFromGEOM(typ, name, grp)

    ## Create a mesh group by the given ids of elements
    #  @param groupName is the name of the mesh group
    #  @param elementType is the type of elements in the group
    #  @param elemIDs is the list of ids
    #  @return SMESH_Group
    def MakeGroupByIds(self, groupName, elementType, elemIDs):
        group = self.mesh.CreateGroup(elementType, groupName)
        group.Add(elemIDs)
        return group
    
    ## Create a mesh group by the given conditions
    #  @param groupName is the name of the mesh group
    #  @param elementType is the type of elements in the group
    #  @param CritType is type of criterion( FT_Taper, FT_Area, FT_RangeOfIds, FT_LyingOnGeom etc. )
    #  @param Compare belong to {FT_LessThan, FT_MoreThan, FT_EqualTo}
    #  @param Treshold is threshold value (range of id ids as string, shape, numeric)
    #  @param UnaryOp is FT_LogicalNOT or FT_Undefined
    #  @return SMESH_Group
    def MakeGroup(self,
                  groupName,
                  elementType,
                  CritType=FT_Undefined,
                  Compare=FT_EqualTo,
                  Treshold="",
                  UnaryOp=FT_Undefined):
        aCriterion = self.smeshpyD.GetCriterion(elementType, CritType, Compare, Treshold, UnaryOp, FT_Undefined)
        group = self.MakeGroupByCriterion(groupName, aCriterion)
        return group

    ## Create a mesh group by the given criterion
    #  @param groupName is the name of the mesh group
    #  @param Criterion is the instance of Criterion class
    #  @return SMESH_Group
    def MakeGroupByCriterion(self, groupName, Criterion):
        aFilterMgr = self.smeshpyD.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aCriteria = []
        aCriteria.append(Criterion)
        aFilter.SetCriteria(aCriteria)
        group = self.MakeGroupByFilter(groupName, aFilter)
        return group
    
    ## Create a mesh group by the given criteria(list of criterions)
    #  @param groupName is the name of the mesh group
    #  @param Criteria is the list of criterions
    #  @return SMESH_Group
    def MakeGroupByCriteria(self, groupName, theCriteria):
        aFilterMgr = self.smeshpyD.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aFilter.SetCriteria(theCriteria)
        group = self.MakeGroupByFilter(groupName, aFilter)
        return group
    
    ## Create a mesh group by the given filter
    #  @param groupName is the name of the mesh group
    #  @param Criterion is the instance of Filter class
    #  @return SMESH_Group
    def MakeGroupByFilter(self, groupName, theFilter):
        anIds = theFilter.GetElementsId(self.mesh)
        anElemType = theFilter.GetElementType()
        group = self.MakeGroupByIds(groupName, anElemType, anIds)
        return group

    ## Pass mesh elements through the given filter and return ids
    #  @param theFilter is SMESH_Filter
    #  @return list of ids
    def GetIdsFromFilter(self, theFilter):
        return theFilter.GetElementsId(self.mesh)

    ## Verify whether 2D mesh element has free edges(edges connected to one face only)\n
    #  Returns list of special structures(borders).
    #  @return list of SMESH.FreeEdges.Border structure: edge id and two its nodes ids.
    def GetFreeBorders(self):
        aFilterMgr = self.smeshpyD.CreateFilterManager()
        aPredicate = aFilterMgr.CreateFreeEdges()
        aPredicate.SetMesh(self.mesh)
        aBorders = aPredicate.GetBorders()
        return aBorders
                
    ## Remove a group
    def RemoveGroup(self, group):
        self.mesh.RemoveGroup(group)

    ## Remove group with its contents
    def RemoveGroupWithContents(self, group):
        self.mesh.RemoveGroupWithContents(group)
        
    ## Get the list of groups existing in the mesh
    def GetGroups(self):
        return self.mesh.GetGroups()

    ## Get number of groups existing in the mesh
    def NbGroups(self):
        return self.mesh.NbGroups()

    ## Get the list of names of groups existing in the mesh
    def GetGroupNames(self):
        groups = self.GetGroups()
        names = []
        for group in groups:
            names.append(group.GetName())
        return names

    ## Union of two groups
    #  New group is created. All mesh elements that are
    #  present in initial groups are added to the new one
    def UnionGroups(self, group1, group2, name):
        return self.mesh.UnionGroups(group1, group2, name)

    ## Intersection of two groups
    #  New group is created. All mesh elements that are
    #  present in both initial groups are added to the new one.
    def IntersectGroups(self, group1, group2, name):
        return self.mesh.IntersectGroups(group1, group2, name)
    
    ## Cut of two groups
    #  New group is created. All mesh elements that are present in
    #  main group but do not present in tool group are added to the new one
    def CutGroups(self, mainGroup, toolGroup, name):
        return self.mesh.CutGroups(mainGroup, toolGroup, name)
         
    
    # Get some info about mesh:
    # ------------------------

    ## Get the log of nodes and elements added or removed since previous
    #  clear of the log.
    #  @param clearAfterGet log is emptied after Get (safe if concurrents access)
    #  @return list of log_block structures:
    #                                        commandType
    #                                        number
    #                                        coords
    #                                        indexes
    def GetLog(self, clearAfterGet):
        return self.mesh.GetLog(clearAfterGet)

    ## Clear the log of nodes and elements added or removed since previous
    #  clear. Must be used immediately after GetLog if clearAfterGet is false.
    def ClearLog(self):
        self.mesh.ClearLog()

    def SetAutoColor(self, color):
        self.mesh.SetAutoColor(color)

    def GetAutoColor(self):
        return self.mesh.GetAutoColor()

    ## Get the internal Id
    def GetId(self):
        return self.mesh.GetId()

    ## Get the study Id
    def GetStudyId(self):
        return self.mesh.GetStudyId()

    ## Check group names for duplications.
    #  Consider maximum group name length stored in MED file.
    def HasDuplicatedGroupNamesMED(self):
        return self.mesh.HasDuplicatedGroupNamesMED()
        
    ## Obtain instance of SMESH_MeshEditor
    def GetMeshEditor(self):
        return self.mesh.GetMeshEditor()

    ## Get MED Mesh
    def GetMEDMesh(self):
        return self.mesh.GetMEDMesh()
    
    
    # Get informations about mesh contents:
    # ------------------------------------

    ## Returns number of nodes in mesh
    def NbNodes(self):
        return self.mesh.NbNodes()

    ## Returns number of elements in mesh
    def NbElements(self):
        return self.mesh.NbElements()

    ## Returns number of edges in mesh
    def NbEdges(self):
        return self.mesh.NbEdges()

    ## Returns number of edges with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbEdgesOfOrder(self, elementOrder):
        return self.mesh.NbEdgesOfOrder(elementOrder)
    
    ## Returns number of faces in mesh
    def NbFaces(self):
        return self.mesh.NbFaces()

    ## Returns number of faces with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbFacesOfOrder(self, elementOrder):
        return self.mesh.NbFacesOfOrder(elementOrder)

    ## Returns number of triangles in mesh
    def NbTriangles(self):
        return self.mesh.NbTriangles()

    ## Returns number of triangles with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbTrianglesOfOrder(self, elementOrder):
        return self.mesh.NbTrianglesOfOrder(elementOrder)

    ## Returns number of quadrangles in mesh
    def NbQuadrangles(self):
        return self.mesh.NbQuadrangles()

    ## Returns number of quadrangles with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbQuadranglesOfOrder(self, elementOrder):
        return self.mesh.NbQuadranglesOfOrder(elementOrder)

    ## Returns number of polygons in mesh
    def NbPolygons(self):
        return self.mesh.NbPolygons()

    ## Returns number of volumes in mesh
    def NbVolumes(self):
        return self.mesh.NbVolumes()

    ## Returns number of volumes with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbVolumesOfOrder(self, elementOrder):
        return self.mesh.NbVolumesOfOrder(elementOrder)

    ## Returns number of tetrahedrons in mesh
    def NbTetras(self):
        return self.mesh.NbTetras()

    ## Returns number of tetrahedrons with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbTetrasOfOrder(self, elementOrder):
        return self.mesh.NbTetrasOfOrder(elementOrder)

    ## Returns number of hexahedrons in mesh
    def NbHexas(self):
        return self.mesh.NbHexas()

    ## Returns number of hexahedrons with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbHexasOfOrder(self, elementOrder):
        return self.mesh.NbHexasOfOrder(elementOrder)

    ## Returns number of pyramids in mesh
    def NbPyramids(self):
        return self.mesh.NbPyramids()

    ## Returns number of pyramids with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbPyramidsOfOrder(self, elementOrder):
        return self.mesh.NbPyramidsOfOrder(elementOrder)

    ## Returns number of prisms in mesh
    def NbPrisms(self):
        return self.mesh.NbPrisms()

    ## Returns number of prisms with given order in mesh
    #  @param elementOrder is order of elements:
    #  ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    def NbPrismsOfOrder(self, elementOrder):
        return self.mesh.NbPrismsOfOrder(elementOrder)

    ## Returns number of polyhedrons in mesh
    def NbPolyhedrons(self):
        return self.mesh.NbPolyhedrons()

    ## Returns number of submeshes in mesh
    def NbSubMesh(self):
        return self.mesh.NbSubMesh()

    ## Returns list of mesh elements ids
    def GetElementsId(self):
        return self.mesh.GetElementsId()

    ## Returns list of ids of mesh elements with given type
    #  @param elementType is required type of elements
    def GetElementsByType(self, elementType):
        return self.mesh.GetElementsByType(elementType)

    ## Returns list of mesh nodes ids
    def GetNodesId(self):
        return self.mesh.GetNodesId()
    
    # Get informations about mesh elements:
    # ------------------------------------
    
    ## Returns type of mesh element
    def GetElementType(self, id, iselem):
        return self.mesh.GetElementType(id, iselem)

    ## Returns list of submesh elements ids
    #  @param Shape is geom object(subshape) IOR
    #  Shape must be subshape of a ShapeToMesh()
    def GetSubMeshElementsId(self, Shape):
        if ( isinstance( Shape, geompy.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshElementsId(ShapeID)

    ## Returns list of submesh nodes ids
    #  @param Shape is geom object(subshape) IOR
    #  Shape must be subshape of a ShapeToMesh()
    def GetSubMeshNodesId(self, Shape, all):
        if ( isinstance( Shape, geompy.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshNodesId(ShapeID, all)
    
    ## Returns list of ids of submesh elements with given type
    #  @param Shape is geom object(subshape) IOR
    #  Shape must be subshape of a ShapeToMesh()
    def GetSubMeshElementType(self, Shape):
        if ( isinstance( Shape, geompy.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshElementType(ShapeID)
      
    ## Get mesh description
    def Dump(self):
        return self.mesh.Dump()

    
    # Get information about nodes and elements of mesh by its ids:
    # -----------------------------------------------------------

    ## Get XYZ coordinates of node as list of double
    #  \n If there is not node for given ID - returns empty list
    def GetNodeXYZ(self, id):
        return self.mesh.GetNodeXYZ(id)

    ## For given node returns list of IDs of inverse elements
    #  \n If there is not node for given ID - returns empty list
    def GetNodeInverseElements(self, id):
        return self.mesh.GetNodeInverseElements(id)

    ## @brief Return position of a node on shape
    #  @return SMESH::NodePosition
    def GetNodePosition(self,NodeID):
        return self.mesh.GetNodePosition(NodeID)

    ## If given element is node returns IDs of shape from position
    #  \n If there is not node for given ID - returns -1
    def GetShapeID(self, id):
        return self.mesh.GetShapeID(id)

    ## For given element returns ID of result shape after
    #  FindShape() from SMESH_MeshEditor
    #  \n If there is not element for given ID - returns -1
    def GetShapeIDForElem(self,id):
        return self.mesh.GetShapeIDForElem(id)
    
    ## Returns number of nodes for given element
    #  \n If there is not element for given ID - returns -1
    def GetElemNbNodes(self, id):
        return self.mesh.GetElemNbNodes(id)

    ## Returns ID of node by given index for given element
    #  \n If there is not element for given ID - returns -1
    #  \n If there is not node for given index - returns -2
    def GetElemNode(self, id, index):
        return self.mesh.GetElemNode(id, index)

    ## Returns IDs of nodes of given element
    def GetElemNodes(self, id):
        return self.mesh.GetElemNodes(id)

    ## Returns true if given node is medium node
    #  in given quadratic element
    def IsMediumNode(self, elementID, nodeID):
        return self.mesh.IsMediumNode(elementID, nodeID)
    
    ## Returns true if given node is medium node
    #  in one of quadratic elements
    def IsMediumNodeOfAnyElem(self, nodeID, elementType):
        return self.mesh.IsMediumNodeOfAnyElem(nodeID, elementType)

    ## Returns number of edges for given element
    def ElemNbEdges(self, id):
        return self.mesh.ElemNbEdges(id)
        
    ## Returns number of faces for given element
    def ElemNbFaces(self, id):
        return self.mesh.ElemNbFaces(id)

    ## Returns true if given element is polygon
    def IsPoly(self, id):
        return self.mesh.IsPoly(id)

    ## Returns true if given element is quadratic
    def IsQuadratic(self, id):
        return self.mesh.IsQuadratic(id)

    ## Returns XYZ coordinates of bary center for given element
    #  as list of double
    #  \n If there is not element for given ID - returns empty list
    def BaryCenter(self, id):
        return self.mesh.BaryCenter(id)
    
    
    # Mesh edition (SMESH_MeshEditor functionality):
    # ---------------------------------------------

    ## Removes elements from mesh by ids
    #  @param IDsOfElements is list of ids of elements to remove
    def RemoveElements(self, IDsOfElements):
        return self.editor.RemoveElements(IDsOfElements)

    ## Removes nodes from mesh by ids
    #  @param IDsOfNodes is list of ids of nodes to remove
    def RemoveNodes(self, IDsOfNodes):
        return self.editor.RemoveNodes(IDsOfNodes)

    ## Add node to mesh by coordinates
    def AddNode(self, x, y, z):
        return self.editor.AddNode( x, y, z)

    
    ## Create edge both similar and quadratic (this is determed
    #  by number of given nodes).
    #  @param IdsOfNodes List of node IDs for creation of element.
    #  Needed order of nodes in this list corresponds to description
    #  of MED. \n This description is located by the following link:
    #  http://www.salome-platform.org/salome2/web_med_internet/logiciels/medV2.2.2_doc_html/html/modele_de_donnees.html#3.
    def AddEdge(self, IDsOfNodes):
        return self.editor.AddEdge(IDsOfNodes)

    ## Create face both similar and quadratic (this is determed
    #  by number of given nodes).
    #  @param IdsOfNodes List of node IDs for creation of element.
    #  Needed order of nodes in this list corresponds to description
    #  of MED. \n This description is located by the following link:
    #  http://www.salome-platform.org/salome2/web_med_internet/logiciels/medV2.2.2_doc_html/html/modele_de_donnees.html#3.
    def AddFace(self, IDsOfNodes):
        return self.editor.AddFace(IDsOfNodes)
    
    ## Add polygonal face to mesh by list of nodes ids
    def AddPolygonalFace(self, IdsOfNodes):
        return self.editor.AddPolygonalFace(IdsOfNodes)
    
    ## Create volume both similar and quadratic (this is determed
    #  by number of given nodes).
    #  @param IdsOfNodes List of node IDs for creation of element.
    #  Needed order of nodes in this list corresponds to description
    #  of MED. \n This description is located by the following link:
    #  http://www.salome-platform.org/salome2/web_med_internet/logiciels/medV2.2.2_doc_html/html/modele_de_donnees.html#3.
    def AddVolume(self, IDsOfNodes):
        return self.editor.AddVolume(IDsOfNodes)

    ## Create volume of many faces, giving nodes for each face.
    #  @param IdsOfNodes List of node IDs for volume creation face by face.
    #  @param Quantities List of integer values, Quantities[i]
    #         gives quantity of nodes in face number i.
    def AddPolyhedralVolume (self, IdsOfNodes, Quantities):
        return self.editor.AddPolyhedralVolume(IdsOfNodes, Quantities)

    ## Create volume of many faces, giving IDs of existing faces.
    #  @param IdsOfFaces List of face IDs for volume creation.
    #
    #  Note:  The created volume will refer only to nodes
    #         of the given faces, not to the faces itself.
    def AddPolyhedralVolumeByFaces (self, IdsOfFaces):
        return self.editor.AddPolyhedralVolumeByFaces(IdsOfFaces)
    

    ## @brief Bind a node to a vertex
    # @param NodeID - node ID
    # @param Vertex - vertex or vertex ID
    # @return True if succeed else raise an exception
    def SetNodeOnVertex(self, NodeID, Vertex):
        if ( isinstance( Vertex, geompy.GEOM._objref_GEOM_Object)):
            VertexID = Vertex.GetSubShapeIndices()[0]
        else:
            VertexID = Vertex
        try:
            self.editor.SetNodeOnVertex(NodeID, VertexID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True
        

    ## @brief Store node position on an edge
    # @param NodeID - node ID
    # @param Edge - edge or edge ID
    # @param paramOnEdge - parameter on edge where the node is located
    # @return True if succeed else raise an exception
    def SetNodeOnEdge(self, NodeID, Edge, paramOnEdge):
        if ( isinstance( Edge, geompy.GEOM._objref_GEOM_Object)):
            EdgeID = Edge.GetSubShapeIndices()[0]
        else:
            EdgeID = Edge
        try:
            self.editor.SetNodeOnEdge(NodeID, EdgeID, paramOnEdge)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    ## @brief Store node position on a face
    # @param NodeID - node ID
    # @param Face - face or face ID
    # @param u - U parameter on face where the node is located
    # @param v - V parameter on face where the node is located
    # @return True if succeed else raise an exception
    def SetNodeOnFace(self, NodeID, Face, u, v):
        if ( isinstance( Face, geompy.GEOM._objref_GEOM_Object)):
            FaceID = Face.GetSubShapeIndices()[0]
        else:
            FaceID = Face
        try:
            self.editor.SetNodeOnFace(NodeID, FaceID, u, v)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    ## @brief Bind a node to a solid
    # @param NodeID - node ID
    # @param Solid - solid or solid ID
    # @return True if succeed else raise an exception
    def SetNodeInVolume(self, NodeID, Solid):
        if ( isinstance( Solid, geompy.GEOM._objref_GEOM_Object)):
            SolidID = Solid.GetSubShapeIndices()[0]
        else:
            SolidID = Solid
        try:
            self.editor.SetNodeInVolume(NodeID, SolidID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    ## @brief Bind an element to a shape
    # @param ElementID - element ID
    # @param Shape - shape or shape ID
    # @return True if succeed else raise an exception
    def SetMeshElementOnShape(self, ElementID, Shape):
        if ( isinstance( Shape, geompy.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        try:
            self.editor.SetMeshElementOnShape(ElementID, ShapeID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True


    ## Move node with given id
    #  @param NodeID id of the node
    #  @param x new X coordinate
    #  @param y new Y coordinate
    #  @param z new Z coordinate
    def MoveNode(self, NodeID, x, y, z):
        return self.editor.MoveNode(NodeID, x, y, z)

    ## Find a node closest to a point
    #  @param x X coordinate of a point
    #  @param y Y coordinate of a point
    #  @param z Z coordinate of a point
    #  @return id of a node
    def FindNodeClosestTo(self, x, y, z):
        preview = self.mesh.GetMeshEditPreviewer()
        return preview.MoveClosestNodeToPoint(x, y, z, -1)

    ## Find a node closest to a point and move it to a point location
    #  @param x X coordinate of a point
    #  @param y Y coordinate of a point
    #  @param z Z coordinate of a point
    #  @return id of a moved node
    def MeshToPassThroughAPoint(self, x, y, z):
        return self.editor.MoveClosestNodeToPoint(x, y, z, -1)

    ## Replace two neighbour triangles sharing Node1-Node2 link
    #  with ones built on the same 4 nodes but having other common link.
    #  @param NodeID1 first node id
    #  @param NodeID2 second node id
    #  @return false if proper faces not found
    def InverseDiag(self, NodeID1, NodeID2):
        return self.editor.InverseDiag(NodeID1, NodeID2)

    ## Replace two neighbour triangles sharing Node1-Node2 link
    #  with a quadrangle built on the same 4 nodes.
    #  @param NodeID1 first node id
    #  @param NodeID2 second node id
    #  @return false if proper faces not found
    def DeleteDiag(self, NodeID1, NodeID2):
        return self.editor.DeleteDiag(NodeID1, NodeID2)

    ## Reorient elements by ids
    #  @param IDsOfElements if undefined reorient all mesh elements
    def Reorient(self, IDsOfElements=None):
        if IDsOfElements == None:
            IDsOfElements = self.GetElementsId()
        return self.editor.Reorient(IDsOfElements)

    ## Reorient all elements of the object
    #  @param theObject is mesh, submesh or group
    def ReorientObject(self, theObject):
        return self.editor.ReorientObject(theObject)

    ## Fuse neighbour triangles into quadrangles.
    #  @param IDsOfElements The triangles to be fused,
    #  @param theCriterion     is FT_...; used to choose a neighbour to fuse with.
    #  @param MaxAngle      is a max angle between element normals at which fusion
    #                       is still performed; theMaxAngle is mesured in radians.
    #  @return TRUE in case of success, FALSE otherwise.
    def TriToQuad(self, IDsOfElements, theCriterion, MaxAngle):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.TriToQuad(IDsOfElements, self.smeshpyD.GetFunctor(theCriterion), MaxAngle)

    ## Fuse neighbour triangles of the object into quadrangles
    #  @param theObject is mesh, submesh or group
    #  @param theCriterion is FT_...; used to choose a neighbour to fuse with.
    #  @param MaxAngle  is a max angle between element normals at which fusion
    #                   is still performed; theMaxAngle is mesured in radians.
    #  @return TRUE in case of success, FALSE otherwise.
    def TriToQuadObject (self, theObject, theCriterion, MaxAngle):
        return self.editor.TriToQuadObject(theObject, self.smeshpyD.GetFunctor(theCriterion), MaxAngle)

    ## Split quadrangles into triangles.
    #  @param IDsOfElements the faces to be splitted.
    #  @param theCriterion  is FT_...; used to choose a diagonal for splitting.
    #  @param @return TRUE in case of success, FALSE otherwise.
    def QuadToTri (self, IDsOfElements, theCriterion):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.QuadToTri(IDsOfElements, self.smeshpyD.GetFunctor(theCriterion))

    ## Split quadrangles into triangles.
    #  @param theObject object to taking list of elements from, is mesh, submesh or group
    #  @param theCriterion  is FT_...; used to choose a diagonal for splitting.
    def QuadToTriObject (self, theObject, theCriterion):
        return self.editor.QuadToTriObject(theObject, self.smeshpyD.GetFunctor(theCriterion))

    ## Split quadrangles into triangles.
    #  @param theElems  The faces to be splitted
    #  @param the13Diag is used to choose a diagonal for splitting.
    #  @return TRUE in case of success, FALSE otherwise.
    def SplitQuad (self, IDsOfElements, Diag13):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.SplitQuad(IDsOfElements, Diag13)

    ## Split quadrangles into triangles.
    #  @param theObject is object to taking list of elements from, is mesh, submesh or group
    def SplitQuadObject (self, theObject, Diag13):
        return self.editor.SplitQuadObject(theObject, Diag13)

    ## Find better splitting of the given quadrangle.
    #  @param IDOfQuad  ID of the quadrangle to be splitted.
    #  @param theCriterion is FT_...; a criterion to choose a diagonal for splitting.
    #  @return 1 if 1-3 diagonal is better, 2 if 2-4
    #          diagonal is better, 0 if error occurs.
    def BestSplit (self, IDOfQuad, theCriterion):
        return self.editor.BestSplit(IDOfQuad, self.smeshpyD.GetFunctor(theCriterion))

    ## Split quafrangle faces near triangular facets of volumes
    #
    def SplitQuadsNearTriangularFacets(self):
        faces_array = self.GetElementsByType(SMESH.FACE)
        for face_id in faces_array:
            if self.GetElemNbNodes(face_id) == 4: # quadrangle
                quad_nodes = self.mesh.GetElemNodes(face_id)
                node1_elems = self.GetNodeInverseElements(quad_nodes[1 -1])
                isVolumeFound = False
                for node1_elem in node1_elems:
                    if not isVolumeFound:
                        if self.GetElementType(node1_elem, True) == SMESH.VOLUME:
                            nb_nodes = self.GetElemNbNodes(node1_elem)
                            if 3 < nb_nodes and nb_nodes < 7: # tetra or penta, or prism
                                volume_elem = node1_elem
                                volume_nodes = self.mesh.GetElemNodes(volume_elem)
                                if volume_nodes.count(quad_nodes[2 -1]) > 0: # 1,2
                                    if volume_nodes.count(quad_nodes[4 -1]) > 0: # 1,2,4
                                        isVolumeFound = True
                                        if volume_nodes.count(quad_nodes[3 -1]) == 0: # 1,2,4 & !3
                                            self.SplitQuad([face_id], False) # diagonal 2-4
                                    elif volume_nodes.count(quad_nodes[3 -1]) > 0: # 1,2,3 & !4
                                        isVolumeFound = True
                                        self.SplitQuad([face_id], True) # diagonal 1-3
                                elif volume_nodes.count(quad_nodes[4 -1]) > 0: # 1,4 & !2
                                    if volume_nodes.count(quad_nodes[3 -1]) > 0: # 1,4,3 & !2
                                        isVolumeFound = True
                                        self.SplitQuad([face_id], True) # diagonal 1-3

    ## @brief Split hexahedrons into tetrahedrons.
    #
    #  Use pattern mapping functionality for splitting.
    #  @param theObject object to take list of hexahedrons from; is mesh, submesh or group.
    #  @param theNode000,theNode001 is in range [0,7]; give an orientation of the
    #         pattern relatively each hexahedron: the (0,0,0) key-point of pattern
    #         will be mapped into <theNode000>-th node of each volume, the (0,0,1)
    #         key-point will be mapped into <theNode001>-th node of each volume.
    #         The (0,0,0) key-point of used pattern corresponds to not split corner.
    #  @return TRUE in case of success, FALSE otherwise.
    def SplitHexaToTetras (self, theObject, theNode000, theNode001):
        # Pattern:     5.---------.6
        #              /|#*      /|
        #             / | #*    / |
        #            /  |  # * /  |
        #           /   |   # /*  |
        # (0,0,1) 4.---------.7 * |
        #          |#*  |1   | # *|
        #          | # *.----|---#.2
        #          |  #/ *   |   /
        #          |  /#  *  |  /
        #          | /   # * | /
        #          |/      #*|/
        # (0,0,0) 0.---------.3
        pattern_tetra = "!!! Nb of points: \n 8 \n\
        !!! Points: \n\
        0 0 0  !- 0 \n\
        0 1 0  !- 1 \n\
        1 1 0  !- 2 \n\
        1 0 0  !- 3 \n\
        0 0 1  !- 4 \n\
        0 1 1  !- 5 \n\
        1 1 1  !- 6 \n\
        1 0 1  !- 7 \n\
        !!! Indices of points of 6 tetras: \n\
        0 3 4 1 \n\
        7 4 3 1 \n\
        4 7 5 1 \n\
        6 2 5 7 \n\
        1 5 2 7 \n\
        2 3 1 7 \n"

        pattern = self.smeshpyD.GetPattern()
        isDone  = pattern.LoadFromFile(pattern_tetra)
        if not isDone:
            print 'Pattern.LoadFromFile :', pattern.GetErrorCode()
            return isDone

        pattern.ApplyToHexahedrons(self.mesh, theObject.GetIDs(), theNode000, theNode001)
        isDone = pattern.MakeMesh(self.mesh, False, False)
        if not isDone: print 'Pattern.MakeMesh :', pattern.GetErrorCode()

        # split quafrangle faces near triangular facets of volumes
        self.SplitQuadsNearTriangularFacets()

        return isDone

    ## @brief Split hexahedrons into prisms.
    #
    #  Use pattern mapping functionality for splitting.
    #  @param theObject object to take list of hexahedrons from; is mesh, submesh or group.
    #  @param theNode000,theNode001 is in range [0,7]; give an orientation of the
    #         pattern relatively each hexahedron: the (0,0,0) key-point of pattern
    #         will be mapped into <theNode000>-th node of each volume, the (0,0,1)
    #         key-point will be mapped into <theNode001>-th node of each volume.
    #         The edge (0,0,0)-(0,0,1) of used pattern connects two not split corners.
    #  @return TRUE in case of success, FALSE otherwise.
    def SplitHexaToPrisms (self, theObject, theNode000, theNode001):
        # Pattern:     5.---------.6
        #              /|#       /|
        #             / | #     / |
        #            /  |  #   /  |
        #           /   |   # /   |
        # (0,0,1) 4.---------.7   |
        #          |    |    |    |
        #          |   1.----|----.2
        #          |   / *   |   /
        #          |  /   *  |  /
        #          | /     * | /
        #          |/       *|/
        # (0,0,0) 0.---------.3
        pattern_prism = "!!! Nb of points: \n 8 \n\
        !!! Points: \n\
        0 0 0  !- 0 \n\
        0 1 0  !- 1 \n\
        1 1 0  !- 2 \n\
        1 0 0  !- 3 \n\
        0 0 1  !- 4 \n\
        0 1 1  !- 5 \n\
        1 1 1  !- 6 \n\
        1 0 1  !- 7 \n\
        !!! Indices of points of 2 prisms: \n\
        0 1 3 4 5 7 \n\
        2 3 1 6 7 5 \n"

        pattern = self.smeshpyD.GetPattern()
        isDone  = pattern.LoadFromFile(pattern_prism)
        if not isDone:
            print 'Pattern.LoadFromFile :', pattern.GetErrorCode()
            return isDone

        pattern.ApplyToHexahedrons(self.mesh, theObject.GetIDs(), theNode000, theNode001)
        isDone = pattern.MakeMesh(self.mesh, False, False)
        if not isDone: print 'Pattern.MakeMesh :', pattern.GetErrorCode()

        # split quafrangle faces near triangular facets of volumes
        self.SplitQuadsNearTriangularFacets()

        return isDone
    
    ## Smooth elements
    #  @param IDsOfElements list if ids of elements to smooth
    #  @param IDsOfFixedNodes list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    def Smooth(self, IDsOfElements, IDsOfFixedNodes,
               MaxNbOfIterations, MaxAspectRatio, Method):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.Smooth(IDsOfElements, IDsOfFixedNodes,
                                  MaxNbOfIterations, MaxAspectRatio, Method)
    
    ## Smooth elements belong to given object
    #  @param theObject object to smooth
    #  @param IDsOfFixedNodes list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    def SmoothObject(self, theObject, IDsOfFixedNodes,
                     MaxNbOfIterations, MaxxAspectRatio, Method):
        return self.editor.SmoothObject(theObject, IDsOfFixedNodes,
                                        MaxNbOfIterations, MaxxAspectRatio, Method)

    ## Parametric smooth the given elements
    #  @param IDsOfElements list if ids of elements to smooth
    #  @param IDsOfFixedNodes list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    def SmoothParametric(self, IDsOfElements, IDsOfFixedNodes,
                         MaxNbOfIterations, MaxAspectRatio, Method):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.SmoothParametric(IDsOfElements, IDsOfFixedNodes,
                                            MaxNbOfIterations, MaxAspectRatio, Method)
    
    ## Parametric smooth elements belong to given object
    #  @param theObject object to smooth
    #  @param IDsOfFixedNodes list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    def SmoothParametricObject(self, theObject, IDsOfFixedNodes,
                               MaxNbOfIterations, MaxAspectRatio, Method):
        return self.editor.SmoothParametricObject(theObject, IDsOfFixedNodes,
                                                  MaxNbOfIterations, MaxAspectRatio, Method)

    ## Converts all mesh to quadratic one, deletes old elements, replacing
    #  them with quadratic ones with the same id.
    def ConvertToQuadratic(self, theForce3d):
        self.editor.ConvertToQuadratic(theForce3d)

    ## Converts all mesh from quadratic to ordinary ones,
    #  deletes old quadratic elements, \n replacing
    #  them with ordinary mesh elements with the same id.
    def ConvertFromQuadratic(self):
        return self.editor.ConvertFromQuadratic()

    ## Renumber mesh nodes
    def RenumberNodes(self):
        self.editor.RenumberNodes()

    ## Renumber mesh elements
    def RenumberElements(self):
        self.editor.RenumberElements()

    ## Generate new elements by rotation of the elements around the axis
    #  @param IDsOfElements list of ids of elements to sweep
    #  @param Axix axis of rotation, AxisStruct or line(geom object)
    #  @param AngleInRadians angle of Rotation
    #  @param NbOfSteps number of steps
    #  @param Tolerance tolerance
    #  @param MakeGroups to generate new groups from existing ones
    def RotationSweep(self, IDsOfElements, Axix, AngleInRadians, NbOfSteps, Tolerance, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Axix, geompyDC.GEOM._objref_GEOM_Object)):
            Axix = self.smeshpyD.GetAxisStruct(Axix)
        if MakeGroups:
            return self.editor.RotationSweepMakeGroups(IDsOfElements, Axix,
                                                       AngleInRadians, NbOfSteps, Tolerance)
        self.editor.RotationSweep(IDsOfElements, Axix, AngleInRadians, NbOfSteps, Tolerance)
        return []

    ## Generate new elements by rotation of the elements of object around the axis
    #  @param theObject object wich elements should be sweeped
    #  @param Axix axis of rotation, AxisStruct or line(geom object)
    #  @param AngleInRadians angle of Rotation
    #  @param NbOfSteps number of steps
    #  @param Tolerance tolerance
    #  @param MakeGroups to generate new groups from existing ones
    def RotationSweepObject(self, theObject, Axix, AngleInRadians, NbOfSteps, Tolerance, MakeGroups=False):
        if ( isinstance( Axix, geompyDC.GEOM._objref_GEOM_Object)):
            Axix = self.smeshpyD.GetAxisStruct(Axix)
        if MakeGroups:
            return self.editor.RotationSweepObjectMakeGroups(theObject, Axix, AngleInRadians,
                                                             NbOfSteps, Tolerance)
        self.editor.RotationSweepObject(theObject, Axix, AngleInRadians, NbOfSteps, Tolerance)
        return []

    ## Generate new elements by extrusion of the elements with given ids
    #  @param IDsOfElements list of elements ids for extrusion
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups to generate new groups from existing ones
    def ExtrusionSweep(self, IDsOfElements, StepVector, NbOfSteps, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if MakeGroups:
            return self.editor.ExtrusionSweepMakeGroups(IDsOfElements, StepVector, NbOfSteps)
        self.editor.ExtrusionSweep(IDsOfElements, StepVector, NbOfSteps)
        return []

    ## Generate new elements by extrusion of the elements with given ids
    #  @param IDsOfElements is ids of elements
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param ExtrFlags set flags for performing extrusion
    #  @param SewTolerance uses for comparing locations of nodes if flag
    #         EXTRUSION_FLAG_SEW is set
    #  @param MakeGroups to generate new groups from existing ones
    def AdvancedExtrusion(self, IDsOfElements, StepVector, NbOfSteps, ExtrFlags, SewTolerance, MakeGroups=False):
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if MakeGroups:
            return self.editor.AdvancedExtrusionMakeGroups(IDsOfElements, StepVector, NbOfSteps,
                                                           ExtrFlags, SewTolerance)
        self.editor.AdvancedExtrusion(IDsOfElements, StepVector, NbOfSteps,
                                      ExtrFlags, SewTolerance)
        return []

    ## Generate new elements by extrusion of the elements belong to object
    #  @param theObject object wich elements should be processed
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups to generate new groups from existing ones
    def ExtrusionSweepObject(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if MakeGroups:
            return self.editor.ExtrusionSweepObjectMakeGroups(theObject, StepVector, NbOfSteps)
        self.editor.ExtrusionSweepObject(theObject, StepVector, NbOfSteps)
        return []

    ## Generate new elements by extrusion of the elements belong to object
    #  @param theObject object wich elements should be processed
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups to generate new groups from existing ones
    def ExtrusionSweepObject1D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if MakeGroups:
            return self.editor.ExtrusionSweepObject1DMakeGroups(theObject, StepVector, NbOfSteps)
        self.editor.ExtrusionSweepObject1D(theObject, StepVector, NbOfSteps)
        return []
    
    ## Generate new elements by extrusion of the elements belong to object
    #  @param theObject object wich elements should be processed
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups to generate new groups from existing ones
    def ExtrusionSweepObject2D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if MakeGroups:
            return self.editor.ExtrusionSweepObject2DMakeGroups(theObject, StepVector, NbOfSteps)
        self.editor.ExtrusionSweepObject2D(theObject, StepVector, NbOfSteps)
        return []

    ## Generate new elements by extrusion of the given elements
    #  A path of extrusion must be a meshed edge.
    #  @param IDsOfElements is ids of elements
    #  @param PathMesh mesh containing a 1D sub-mesh on the edge, along which proceeds the extrusion
    #  @param PathShape is shape(edge); as the mesh can be complex, the edge is used to define the sub-mesh for the path
    #  @param NodeStart the first or the last node on the edge. It is used to define the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path to get the resulting mesh in a helical fashion
    #  @param Angles list of angles
    #  @param HasRefPoint allows to use base point
    #  @param RefPoint point around which the shape is rotated(the mass center of the shape by default).
    #         User can specify any point as the Base Point and the shape will be rotated with respect to this point.
    #  @param MakeGroups to generate new groups from existing ones
    #  @param LinearVariation makes compute rotation angles as linear variation of given Angles along path steps
    def ExtrusionAlongPath(self, IDsOfElements, PathMesh, PathShape, NodeStart,
                           HasAngles, Angles, HasRefPoint, RefPoint,
                           MakeGroups=False, LinearVariation=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( RefPoint, geompyDC.GEOM._objref_GEOM_Object)):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
            pass
        if MakeGroups:
            return self.editor.ExtrusionAlongPathMakeGroups(IDsOfElements, PathMesh.GetMesh(),
                                                            PathShape, NodeStart, HasAngles,
                                                            Angles, HasRefPoint, RefPoint)
        return self.editor.ExtrusionAlongPath(IDsOfElements, PathMesh.GetMesh(), PathShape,
                                              NodeStart, HasAngles, Angles, HasRefPoint, RefPoint)

    ## Generate new elements by extrusion of the elements belong to object
    #  A path of extrusion must be a meshed edge.
    #  @param IDsOfElements is ids of elements
    #  @param PathMesh mesh containing a 1D sub-mesh on the edge, along which proceeds the extrusion
    #  @param PathShape is shape(edge); as the mesh can be complex, the edge is used to define the sub-mesh for the path
    #  @param NodeStart the first or the last node on the edge. It is used to define the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path to get the resulting mesh in a helical fashion
    #  @param Angles list of angles
    #  @param HasRefPoint allows to use base point
    #  @param RefPoint point around which the shape is rotated(the mass center of the shape by default).
    #         User can specify any point as the Base Point and the shape will be rotated with respect to this point.
    #  @param MakeGroups to generate new groups from existing ones
    #  @param LinearVariation makes compute rotation angles as linear variation of given Angles along path steps
    def ExtrusionAlongPathObject(self, theObject, PathMesh, PathShape, NodeStart,
                                 HasAngles, Angles, HasRefPoint, RefPoint,
                                 MakeGroups=False, LinearVariation=False):
        if ( isinstance( RefPoint, geompyDC.GEOM._objref_GEOM_Object)):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
        if MakeGroups:
            return self.editor.ExtrusionAlongPathObjectMakeGroups(theObject, PathMesh.GetMesh(),
                                                                  PathShape, NodeStart, HasAngles,
                                                                  Angles, HasRefPoint, RefPoint)
        return self.editor.ExtrusionAlongPathObject(theObject, PathMesh.GetMesh(), PathShape,
                                                    NodeStart, HasAngles, Angles, HasRefPoint,
                                                    RefPoint)
    
    ## Symmetrical copy of mesh elements
    #  @param IDsOfElements list of elements ids
    #  @param Mirror is AxisStruct or geom object(point, line, plane)
    #  @param theMirrorType is  POINT, AXIS or PLANE
    #  If the Mirror is geom object this parameter is unnecessary
    #  @param Copy allows to copy element(Copy is 1) or to replace with its mirroring(Copy is 0)
    #  @param MakeGroups to generate new groups from existing ones (if Copy)
    def Mirror(self, IDsOfElements, Mirror, theMirrorType, Copy=0, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Mirror, geompyDC.GEOM._objref_GEOM_Object)):
            Mirror = self.smeshpyD.GetAxisStruct(Mirror)
        if Copy and MakeGroups:
            return self.editor.MirrorMakeGroups(IDsOfElements, Mirror, theMirrorType)
        self.editor.Mirror(IDsOfElements, Mirror, theMirrorType, Copy)
        return []

    ## Symmetrical copy of object
    #  @param theObject mesh, submesh or group
    #  @param Mirror is AxisStruct or geom object(point, line, plane)
    #  @param theMirrorType is  POINT, AXIS or PLANE
    #  If the Mirror is geom object this parameter is unnecessary
    #  @param Copy allows to copy element(Copy is 1) or to replace with its mirroring(Copy is 0)
    #  @param MakeGroups to generate new groups from existing ones (if Copy)
    def MirrorObject (self, theObject, Mirror, theMirrorType, Copy=0, MakeGroups=False):
        if ( isinstance( Mirror, geompyDC.GEOM._objref_GEOM_Object)):
            Mirror = self.smeshpyD.GetAxisStruct(Mirror)
        if Copy and MakeGroups:
            return self.editor.MirrorObjectMakeGroups(theObject, Mirror, theMirrorType)
        self.editor.MirrorObject(theObject, Mirror, theMirrorType, Copy)
        return []

    ## Translates the elements
    #  @param IDsOfElements list of elements ids
    #  @param Vector direction of translation(DirStruct or vector)
    #  @param Copy allows to copy the translated elements
    #  @param MakeGroups to generate new groups from existing ones (if Copy)
    def Translate(self, IDsOfElements, Vector, Copy, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Vector, geompyDC.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if Copy and MakeGroups:
            return self.editor.TranslateMakeGroups(IDsOfElements, Vector)
        self.editor.Translate(IDsOfElements, Vector, Copy)
        return []

    ## Translates the object
    #  @param theObject object to translate(mesh, submesh, or group)
    #  @param Vector direction of translation(DirStruct or geom vector)
    #  @param Copy allows to copy the translated elements
    #  @param MakeGroups to generate new groups from existing ones (if Copy)
    def TranslateObject(self, theObject, Vector, Copy, MakeGroups=False):
        if ( isinstance( Vector, geompyDC.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if Copy and MakeGroups:
            return self.editor.TranslateObjectMakeGroups(theObject, Vector)
        self.editor.TranslateObject(theObject, Vector, Copy)
        return []

    ## Rotates the elements
    #  @param IDsOfElements list of elements ids
    #  @param Axis axis of rotation(AxisStruct or geom line)
    #  @param AngleInRadians angle of rotation(in radians)
    #  @param Copy allows to copy the rotated elements
    #  @param MakeGroups to generate new groups from existing ones (if Copy)
    def Rotate (self, IDsOfElements, Axis, AngleInRadians, Copy, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        if Copy and MakeGroups:
            return self.editor.RotateMakeGroups(IDsOfElements, Axis, AngleInRadians)
        self.editor.Rotate(IDsOfElements, Axis, AngleInRadians, Copy)
        return []

    ## Rotates the object
    #  @param theObject object to rotate(mesh, submesh, or group)
    #  @param Axis axis of rotation(AxisStruct or geom line)
    #  @param AngleInRadians angle of rotation(in radians)
    #  @param Copy allows to copy the rotated elements
    #  @param MakeGroups to generate new groups from existing ones (if Copy)
    def RotateObject (self, theObject, Axis, AngleInRadians, Copy, MakeGroups=False):
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        if Copy and MakeGroups:
            return self.editor.RotateObjectMakeGroups(theObject, Axis, AngleInRadians)
        self.editor.RotateObject(theObject, Axis, AngleInRadians, Copy)
        return []

    ## Find group of nodes close to each other within Tolerance.
    #  @param Tolerance tolerance value
    #  @param list of group of nodes
    def FindCoincidentNodes (self, Tolerance):
        return self.editor.FindCoincidentNodes(Tolerance)

    ## Find group of nodes close to each other within Tolerance.
    #  @param Tolerance tolerance value
    #  @param SubMeshOrGroup SubMesh or Group
    #  @param list of group of nodes
    def FindCoincidentNodesOnPart (self, SubMeshOrGroup, Tolerance):
        return self.editor.FindCoincidentNodesOnPart(SubMeshOrGroup, Tolerance)

    ## Merge nodes
    #  @param list of group of nodes
    def MergeNodes (self, GroupsOfNodes):
        self.editor.MergeNodes(GroupsOfNodes)

    ## Find elements built on the same nodes.
    #  @param MeshOrSubMeshOrGroup Mesh or SubMesh, or Group of elements for searching
    #  @return a list of groups of equal elements
    def FindEqualElements (self, MeshOrSubMeshOrGroup):
        return self.editor.FindEqualElements(MeshOrSubMeshOrGroup)

    ## Merge elements in each given group.
    #  @param GroupsOfElementsID groups of elements for merging
    def MergeElements(self, GroupsOfElementsID):
        self.editor.MergeElements(GroupsOfElementsID)

    ## Remove all but one of elements built on the same nodes.
    def MergeEqualElements(self):
        self.editor.MergeEqualElements()
        
    ## Sew free borders
    def SewFreeBorders (self, FirstNodeID1, SecondNodeID1, LastNodeID1,
                        FirstNodeID2, SecondNodeID2, LastNodeID2,
                        CreatePolygons, CreatePolyedrs):
        return self.editor.SewFreeBorders(FirstNodeID1, SecondNodeID1, LastNodeID1,
                                          FirstNodeID2, SecondNodeID2, LastNodeID2,
                                          CreatePolygons, CreatePolyedrs)

    ## Sew conform free borders
    def SewConformFreeBorders (self, FirstNodeID1, SecondNodeID1, LastNodeID1,
                               FirstNodeID2, SecondNodeID2):
        return self.editor.SewConformFreeBorders(FirstNodeID1, SecondNodeID1, LastNodeID1,
                                                 FirstNodeID2, SecondNodeID2)
    
    ## Sew border to side
    def SewBorderToSide (self, FirstNodeIDOnFreeBorder, SecondNodeIDOnFreeBorder, LastNodeIDOnFreeBorder,
                         FirstNodeIDOnSide, LastNodeIDOnSide, CreatePolygons, CreatePolyedrs):
        return self.editor.SewBorderToSide(FirstNodeIDOnFreeBorder, SecondNodeIDOnFreeBorder, LastNodeIDOnFreeBorder,
                                           FirstNodeIDOnSide, LastNodeIDOnSide, CreatePolygons, CreatePolyedrs)

    ## Sew two sides of a mesh. Nodes belonging to Side1 are
    #  merged with nodes of elements of Side2.
    #  Number of elements in theSide1 and in theSide2 must be
    #  equal and they should have similar node connectivity.
    #  The nodes to merge should belong to sides borders and
    #  the first node should be linked to the second.
    def SewSideElements (self, IDsOfSide1Elements, IDsOfSide2Elements,
                         NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                         NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge):
        return self.editor.SewSideElements(IDsOfSide1Elements, IDsOfSide2Elements,
                                           NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                                           NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge)

    ## Set new nodes for given element.
    #  @param ide the element id
    #  @param newIDs nodes ids
    #  @return If number of nodes is not corresponded to type of element - returns false
    def ChangeElemNodes(self, ide, newIDs):
        return self.editor.ChangeElemNodes(ide, newIDs)
    
    ## If during last operation of MeshEditor some nodes were
    #  created this method returns list of its IDs, \n
    #  if new nodes not created - returns empty list
    def GetLastCreatedNodes(self):
        return self.editor.GetLastCreatedNodes()

    ## If during last operation of MeshEditor some elements were
    #  created this method returns list of its IDs, \n
    #  if new elements not creared - returns empty list
    def GetLastCreatedElems(self):
        return self.editor.GetLastCreatedElems()
