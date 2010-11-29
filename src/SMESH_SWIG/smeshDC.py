#  -*- coding: iso-8859-1 -*-
#  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  File   : smesh.py
#  Author : Francis KLOSS, OCC
#  Module : SMESH
#
"""
 \namespace smesh
 \brief Module smesh
"""

## @defgroup l1_auxiliary Auxiliary methods and structures
## @defgroup l1_creating  Creating meshes
## @{
##   @defgroup l2_impexp     Importing and exporting meshes
##   @defgroup l2_construct  Constructing meshes
##   @defgroup l2_algorithms Defining Algorithms
##   @{
##     @defgroup l3_algos_basic   Basic meshing algorithms
##     @defgroup l3_algos_proj    Projection Algorithms
##     @defgroup l3_algos_radialp Radial Prism
##     @defgroup l3_algos_segmarv Segments around Vertex
##     @defgroup l3_algos_3dextr  3D extrusion meshing algorithm

##   @}
##   @defgroup l2_hypotheses Defining hypotheses
##   @{
##     @defgroup l3_hypos_1dhyps 1D Meshing Hypotheses
##     @defgroup l3_hypos_2dhyps 2D Meshing Hypotheses
##     @defgroup l3_hypos_maxvol Max Element Volume hypothesis
##     @defgroup l3_hypos_netgen Netgen 2D and 3D hypotheses
##     @defgroup l3_hypos_ghs3dh GHS3D Parameters hypothesis
##     @defgroup l3_hypos_blsurf BLSURF Parameters hypothesis
##     @defgroup l3_hypos_hexotic Hexotic Parameters hypothesis
##     @defgroup l3_hypos_quad Quadrangle Parameters hypothesis
##     @defgroup l3_hypos_additi Additional Hypotheses

##   @}
##   @defgroup l2_submeshes Constructing submeshes
##   @defgroup l2_compounds Building Compounds
##   @defgroup l2_editing   Editing Meshes

## @}
## @defgroup l1_meshinfo  Mesh Information
## @defgroup l1_controls  Quality controls and Filtering
## @defgroup l1_grouping  Grouping elements
## @{
##   @defgroup l2_grps_create Creating groups
##   @defgroup l2_grps_edit   Editing groups
##   @defgroup l2_grps_operon Using operations on groups
##   @defgroup l2_grps_delete Deleting Groups

## @}
## @defgroup l1_modifying Modifying meshes
## @{
##   @defgroup l2_modif_add      Adding nodes and elements
##   @defgroup l2_modif_del      Removing nodes and elements
##   @defgroup l2_modif_edit     Modifying nodes and elements
##   @defgroup l2_modif_renumber Renumbering nodes and elements
##   @defgroup l2_modif_trsf     Transforming meshes (Translation, Rotation, Symmetry, Sewing, Merging)
##   @defgroup l2_modif_movenode Moving nodes
##   @defgroup l2_modif_throughp Mesh through point
##   @defgroup l2_modif_invdiag  Diagonal inversion of elements
##   @defgroup l2_modif_unitetri Uniting triangles
##   @defgroup l2_modif_changori Changing orientation of elements
##   @defgroup l2_modif_cutquadr Cutting quadrangles
##   @defgroup l2_modif_smooth   Smoothing
##   @defgroup l2_modif_extrurev Extrusion and Revolution
##   @defgroup l2_modif_patterns Pattern mapping
##   @defgroup l2_modif_tofromqu Convert to/from Quadratic Mesh

## @}
## @defgroup l1_measurements Measurements

import salome
import geompyDC

import SMESH # This is necessary for back compatibility
from   SMESH import *

import StdMeshers

import SALOME
import SALOMEDS

# import NETGENPlugin module if possible
noNETGENPlugin = 0
try:
    import NETGENPlugin
except ImportError:
    noNETGENPlugin = 1
    pass

# import GHS3DPlugin module if possible
noGHS3DPlugin = 0
try:
    import GHS3DPlugin
except ImportError:
    noGHS3DPlugin = 1
    pass

# import GHS3DPRLPlugin module if possible
noGHS3DPRLPlugin = 0
try:
    import GHS3DPRLPlugin
except ImportError:
    noGHS3DPRLPlugin = 1
    pass

# import HexoticPlugin module if possible
noHexoticPlugin = 0
try:
    import HexoticPlugin
except ImportError:
    noHexoticPlugin = 1
    pass

# import BLSURFPlugin module if possible
noBLSURFPlugin = 0
try:
    import BLSURFPlugin
except ImportError:
    noBLSURFPlugin = 1
    pass

## @addtogroup l1_auxiliary
## @{

# Types of algorithms
REGULAR    = 1
PYTHON     = 2
COMPOSITE  = 3
SOLE       = 0
SIMPLE     = 1

MEFISTO       = 3
NETGEN        = 4
GHS3D         = 5
FULL_NETGEN   = 6
NETGEN_2D     = 7
NETGEN_1D2D   = NETGEN
NETGEN_1D2D3D = FULL_NETGEN
NETGEN_FULL   = FULL_NETGEN
Hexa    = 8
Hexotic = 9
BLSURF  = 10
GHS3DPRL = 11
QUADRANGLE = 0
RADIAL_QUAD = 1

# MirrorType enumeration
POINT = SMESH_MeshEditor.POINT
AXIS =  SMESH_MeshEditor.AXIS
PLANE = SMESH_MeshEditor.PLANE

# Smooth_Method enumeration
LAPLACIAN_SMOOTH = SMESH_MeshEditor.LAPLACIAN_SMOOTH
CENTROIDAL_SMOOTH = SMESH_MeshEditor.CENTROIDAL_SMOOTH

# Fineness enumeration (for NETGEN)
VeryCoarse = 0
Coarse     = 1
Moderate   = 2
Fine       = 3
VeryFine   = 4
Custom     = 5

# Optimization level of GHS3D
# V3.1
None_Optimization, Light_Optimization, Medium_Optimization, Strong_Optimization = 0,1,2,3
# V4.1 (partialy redefines V3.1). Issue 0020574
None_Optimization, Light_Optimization, Standard_Optimization, StandardPlus_Optimization, Strong_Optimization = 0,1,2,3,4

# Topology treatment way of BLSURF
FromCAD, PreProcess, PreProcessPlus = 0,1,2

# Element size flag of BLSURF
DefaultSize, DefaultGeom, Custom = 0,0,1

PrecisionConfusion = 1e-07

# TopAbs_State enumeration
[TopAbs_IN, TopAbs_OUT, TopAbs_ON, TopAbs_UNKNOWN] = range(4)

# Methods of splitting a hexahedron into tetrahedra
Hex_5Tet, Hex_6Tet, Hex_24Tet = 1, 2, 3

# import items of enum QuadType
for e in StdMeshers.QuadType._items: exec('%s = StdMeshers.%s'%(e,e))

## Converts an angle from degrees to radians
def DegreesToRadians(AngleInDegrees):
    from math import pi
    return AngleInDegrees * pi / 180.0

# Salome notebook variable separator
var_separator = ":"

# Parametrized substitute for PointStruct
class PointStructStr:

    x = 0
    y = 0
    z = 0
    xStr = ""
    yStr = ""
    zStr = ""

    def __init__(self, xStr, yStr, zStr):
        self.xStr = xStr
        self.yStr = yStr
        self.zStr = zStr
        if isinstance(xStr, str) and notebook.isVariable(xStr):
            self.x = notebook.get(xStr)
        else:
            self.x = xStr
        if isinstance(yStr, str) and notebook.isVariable(yStr):
            self.y = notebook.get(yStr)
        else:
            self.y = yStr
        if isinstance(zStr, str) and notebook.isVariable(zStr):
            self.z = notebook.get(zStr)
        else:
            self.z = zStr

# Parametrized substitute for PointStruct (with 6 parameters)
class PointStructStr6:

    x1 = 0
    y1 = 0
    z1 = 0
    x2 = 0
    y2 = 0
    z2 = 0
    xStr1 = ""
    yStr1 = ""
    zStr1 = ""
    xStr2 = ""
    yStr2 = ""
    zStr2 = ""

    def __init__(self, x1Str, x2Str, y1Str, y2Str, z1Str, z2Str):
        self.x1Str = x1Str
        self.x2Str = x2Str
        self.y1Str = y1Str
        self.y2Str = y2Str
        self.z1Str = z1Str
        self.z2Str = z2Str
        if isinstance(x1Str, str) and notebook.isVariable(x1Str):
            self.x1 = notebook.get(x1Str)
        else:
            self.x1 = x1Str
        if isinstance(x2Str, str) and notebook.isVariable(x2Str):
            self.x2 = notebook.get(x2Str)
        else:
            self.x2 = x2Str
        if isinstance(y1Str, str) and notebook.isVariable(y1Str):
            self.y1 = notebook.get(y1Str)
        else:
            self.y1 = y1Str
        if isinstance(y2Str, str) and notebook.isVariable(y2Str):
            self.y2 = notebook.get(y2Str)
        else:
            self.y2 = y2Str
        if isinstance(z1Str, str) and notebook.isVariable(z1Str):
            self.z1 = notebook.get(z1Str)
        else:
            self.z1 = z1Str
        if isinstance(z2Str, str) and notebook.isVariable(z2Str):
            self.z2 = notebook.get(z2Str)
        else:
            self.z2 = z2Str

# Parametrized substitute for AxisStruct
class AxisStructStr:

    x = 0
    y = 0
    z = 0
    dx = 0
    dy = 0
    dz = 0
    xStr = ""
    yStr = ""
    zStr = ""
    dxStr = ""
    dyStr = ""
    dzStr = ""

    def __init__(self, xStr, yStr, zStr, dxStr, dyStr, dzStr):
        self.xStr = xStr
        self.yStr = yStr
        self.zStr = zStr
        self.dxStr = dxStr
        self.dyStr = dyStr
        self.dzStr = dzStr
        if isinstance(xStr, str) and notebook.isVariable(xStr):
            self.x = notebook.get(xStr)
        else:
            self.x = xStr
        if isinstance(yStr, str) and notebook.isVariable(yStr):
            self.y = notebook.get(yStr)
        else:
            self.y = yStr
        if isinstance(zStr, str) and notebook.isVariable(zStr):
            self.z = notebook.get(zStr)
        else:
            self.z = zStr
        if isinstance(dxStr, str) and notebook.isVariable(dxStr):
            self.dx = notebook.get(dxStr)
        else:
            self.dx = dxStr
        if isinstance(dyStr, str) and notebook.isVariable(dyStr):
            self.dy = notebook.get(dyStr)
        else:
            self.dy = dyStr
        if isinstance(dzStr, str) and notebook.isVariable(dzStr):
            self.dz = notebook.get(dzStr)
        else:
            self.dz = dzStr

# Parametrized substitute for DirStruct
class DirStructStr:

    def __init__(self, pointStruct):
        self.pointStruct = pointStruct

# Returns list of variable values from salome notebook
def ParsePointStruct(Point):
    Parameters = 2*var_separator
    if isinstance(Point, PointStructStr):
        Parameters = str(Point.xStr) + var_separator + str(Point.yStr) + var_separator + str(Point.zStr)
        Point = PointStruct(Point.x, Point.y, Point.z)
    return Point, Parameters

# Returns list of variable values from salome notebook
def ParseDirStruct(Dir):
    Parameters = 2*var_separator
    if isinstance(Dir, DirStructStr):
        pntStr = Dir.pointStruct
        if isinstance(pntStr, PointStructStr6):
            Parameters = str(pntStr.x1Str) + var_separator + str(pntStr.x2Str) + var_separator
            Parameters += str(pntStr.y1Str) + var_separator + str(pntStr.y2Str) + var_separator 
            Parameters += str(pntStr.z1Str) + var_separator + str(pntStr.z2Str)
            Point = PointStruct(pntStr.x2 - pntStr.x1, pntStr.y2 - pntStr.y1, pntStr.z2 - pntStr.z1)
        else:
            Parameters = str(pntStr.xStr) + var_separator + str(pntStr.yStr) + var_separator + str(pntStr.zStr)
            Point = PointStruct(pntStr.x, pntStr.y, pntStr.z)
        Dir = DirStruct(Point)
    return Dir, Parameters

# Returns list of variable values from salome notebook
def ParseAxisStruct(Axis):
    Parameters = 5*var_separator
    if isinstance(Axis, AxisStructStr):
        Parameters = str(Axis.xStr) + var_separator + str(Axis.yStr) + var_separator + str(Axis.zStr) + var_separator
        Parameters += str(Axis.dxStr) + var_separator + str(Axis.dyStr) + var_separator + str(Axis.dzStr)
        Axis = AxisStruct(Axis.x, Axis.y, Axis.z, Axis.dx, Axis.dy, Axis.dz)
    return Axis, Parameters

## Return list of variable values from salome notebook
def ParseAngles(list):
    Result = []
    Parameters = ""
    for parameter in list:
        if isinstance(parameter,str) and notebook.isVariable(parameter):
            Result.append(DegreesToRadians(notebook.get(parameter)))
            pass
        else:
            Result.append(parameter)
            pass
        
        Parameters = Parameters + str(parameter)
        Parameters = Parameters + var_separator
        pass
    Parameters = Parameters[:len(Parameters)-1]
    return Result, Parameters
    
def IsEqual(val1, val2, tol=PrecisionConfusion):
    if abs(val1 - val2) < tol:
        return True
    return False

NO_NAME = "NoName"

## Gets object name
def GetName(obj):
    if obj:
        # object not null
        if isinstance(obj, SALOMEDS._objref_SObject):
            # study object
            return obj.GetName()
        ior  = salome.orb.object_to_string(obj)
        if ior:
            # CORBA object
            studies = salome.myStudyManager.GetOpenStudies()
            for sname in studies:
                s = salome.myStudyManager.GetStudyByName(sname)
                if not s: continue
                sobj = s.FindObjectIOR(ior)
                if not sobj: continue
                return sobj.GetName()
            if hasattr(obj, "GetName"):
                # unknown CORBA object, having GetName() method
                return obj.GetName()
            else:
                # unknown CORBA object, no GetName() method
                return NO_NAME
            pass
        if hasattr(obj, "GetName"):
            # unknown non-CORBA object, having GetName() method
            return obj.GetName()
        pass
    raise RuntimeError, "Null or invalid object"

## Prints error message if a hypothesis was not assigned.
def TreatHypoStatus(status, hypName, geomName, isAlgo):
    if isAlgo:
        hypType = "algorithm"
    else:
        hypType = "hypothesis"
        pass
    if status == HYP_UNKNOWN_FATAL :
        reason = "for unknown reason"
    elif status == HYP_INCOMPATIBLE :
        reason = "this hypothesis mismatches the algorithm"
    elif status == HYP_NOTCONFORM :
        reason = "a non-conform mesh would be built"
    elif status == HYP_ALREADY_EXIST :
        if isAlgo: return # it does not influence anything
        reason = hypType + " of the same dimension is already assigned to this shape"
    elif status == HYP_BAD_DIM :
        reason = hypType + " mismatches the shape"
    elif status == HYP_CONCURENT :
        reason = "there are concurrent hypotheses on sub-shapes"
    elif status == HYP_BAD_SUBSHAPE :
        reason = "the shape is neither the main one, nor its subshape, nor a valid group"
    elif status == HYP_BAD_GEOMETRY:
        reason = "geometry mismatches the expectation of the algorithm"
    elif status == HYP_HIDDEN_ALGO:
        reason = "it is hidden by an algorithm of an upper dimension, which generates elements of all dimensions"
    elif status == HYP_HIDING_ALGO:
        reason = "it hides algorithms of lower dimensions by generating elements of all dimensions"
    elif status == HYP_NEED_SHAPE:
        reason = "Algorithm can't work without shape"
    else:
        return
    hypName = '"' + hypName + '"'
    geomName= '"' + geomName+ '"'
    if status < HYP_UNKNOWN_FATAL and not geomName =='""':
        print hypName, "was assigned to",    geomName,"but", reason
    elif not geomName == '""':
        print hypName, "was not assigned to",geomName,":", reason
    else:
        print hypName, "was not assigned:", reason
        pass

## Check meshing plugin availability
def CheckPlugin(plugin):
    if plugin == NETGEN and noNETGENPlugin:
        print "Warning: NETGENPlugin module unavailable"
        return False
    elif plugin == GHS3D and noGHS3DPlugin:
        print "Warning: GHS3DPlugin module unavailable"
        return False
    elif plugin == GHS3DPRL and noGHS3DPRLPlugin:
        print "Warning: GHS3DPRLPlugin module unavailable"
        return False
    elif plugin == Hexotic and noHexoticPlugin:
        print "Warning: HexoticPlugin module unavailable"
        return False
    elif plugin == BLSURF and noBLSURFPlugin:
        print "Warning: BLSURFPlugin module unavailable"
        return False
    return True
    
# end of l1_auxiliary
## @}

# All methods of this class are accessible directly from the smesh.py package.
class smeshDC(SMESH._objref_SMESH_Gen):

    ## Sets the current study and Geometry component
    #  @ingroup l1_auxiliary
    def init_smesh(self,theStudy,geompyD):
        self.SetCurrentStudy(theStudy,geompyD)

    ## Creates an empty Mesh. This mesh can have an underlying geometry.
    #  @param obj the Geometrical object on which the mesh is built. If not defined,
    #             the mesh will have no underlying geometry.
    #  @param name the name for the new mesh.
    #  @return an instance of Mesh class.
    #  @ingroup l2_construct
    def Mesh(self, obj=0, name=0):
        if isinstance(obj,str):
            obj,name = name,obj
        return Mesh(self,self.geompyD,obj,name)

    ## Returns a long value from enumeration
    #  Should be used for SMESH.FunctorType enumeration
    #  @ingroup l1_controls
    def EnumToLong(self,theItem):
        return theItem._v

    ## Returns a string representation of the color.
    #  To be used with filters.
    #  @param c color value (SALOMEDS.Color)
    #  @ingroup l1_controls
    def ColorToString(self,c):
        val = ""
        if isinstance(c, SALOMEDS.Color):
            val = "%s;%s;%s" % (c.R, c.G, c.B)
        elif isinstance(c, str):
            val = c
        else:
            raise ValueError, "Color value should be of string or SALOMEDS.Color type"
        return val

    ## Gets PointStruct from vertex
    #  @param theVertex a GEOM object(vertex)
    #  @return SMESH.PointStruct
    #  @ingroup l1_auxiliary
    def GetPointStruct(self,theVertex):
        [x, y, z] = self.geompyD.PointCoordinates(theVertex)
        return PointStruct(x,y,z)

    ## Gets DirStruct from vector
    #  @param theVector a GEOM object(vector)
    #  @return SMESH.DirStruct
    #  @ingroup l1_auxiliary
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

    ## Makes DirStruct from a triplet
    #  @param x,y,z vector components
    #  @return SMESH.DirStruct
    #  @ingroup l1_auxiliary
    def MakeDirStruct(self,x,y,z):
        pnt = PointStruct(x,y,z)
        return DirStruct(pnt)

    ## Get AxisStruct from object
    #  @param theObj a GEOM object (line or plane)
    #  @return SMESH.AxisStruct
    #  @ingroup l1_auxiliary
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

    ## Sets the given name to the object
    #  @param obj the object to rename
    #  @param name a new object name
    #  @ingroup l1_auxiliary
    def SetName(self, obj, name):
        if isinstance( obj, Mesh ):
            obj = obj.GetMesh()
        elif isinstance( obj, Mesh_Algorithm ):
            obj = obj.GetAlgorithm()
        ior  = salome.orb.object_to_string(obj)
        SMESH._objref_SMESH_Gen.SetName(self, ior, name)

    ## Sets the current mode
    #  @ingroup l1_auxiliary
    def SetEmbeddedMode( self,theMode ):
        #self.SetEmbeddedMode(theMode)
        SMESH._objref_SMESH_Gen.SetEmbeddedMode(self,theMode)

    ## Gets the current mode
    #  @ingroup l1_auxiliary
    def IsEmbeddedMode(self):
        #return self.IsEmbeddedMode()
        return SMESH._objref_SMESH_Gen.IsEmbeddedMode(self)

    ## Sets the current study
    #  @ingroup l1_auxiliary
    def SetCurrentStudy( self, theStudy, geompyD = None ):
        #self.SetCurrentStudy(theStudy)
        if not geompyD:
            import geompy
            geompyD = geompy.geom
            pass
        self.geompyD=geompyD
        self.SetGeomEngine(geompyD)
        SMESH._objref_SMESH_Gen.SetCurrentStudy(self,theStudy)

    ## Gets the current study
    #  @ingroup l1_auxiliary
    def GetCurrentStudy(self):
        #return self.GetCurrentStudy()
        return SMESH._objref_SMESH_Gen.GetCurrentStudy(self)

    ## Creates a Mesh object importing data from the given UNV file
    #  @return an instance of Mesh class
    #  @ingroup l2_impexp
    def CreateMeshesFromUNV( self,theFileName ):
        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromUNV(self,theFileName)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh)
        return aMesh

    ## Creates a Mesh object(s) importing data from the given MED file
    #  @return a list of Mesh class instances
    #  @ingroup l2_impexp
    def CreateMeshesFromMED( self,theFileName ):
        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromMED(self,theFileName)
        aMeshes = []
        for iMesh in range(len(aSmeshMeshes)) :
            aMesh = Mesh(self, self.geompyD, aSmeshMeshes[iMesh])
            aMeshes.append(aMesh)
        return aMeshes, aStatus

    ## Creates a Mesh object importing data from the given STL file
    #  @return an instance of Mesh class
    #  @ingroup l2_impexp
    def CreateMeshesFromSTL( self, theFileName ):
        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromSTL(self,theFileName)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh)
        return aMesh

    ## From SMESH_Gen interface
    #  @return the list of integer values
    #  @ingroup l1_auxiliary
    def GetSubShapesId( self, theMainObject, theListOfSubObjects ):
        return SMESH._objref_SMESH_Gen.GetSubShapesId(self,theMainObject, theListOfSubObjects)

    ## From SMESH_Gen interface. Creates a pattern
    #  @return an instance of SMESH_Pattern
    #
    #  <a href="../tui_modifying_meshes_page.html#tui_pattern_mapping">Example of Patterns usage</a>
    #  @ingroup l2_modif_patterns
    def GetPattern(self):
        return SMESH._objref_SMESH_Gen.GetPattern(self)

    ## Sets number of segments per diagonal of boundary box of geometry by which
    #  default segment length of appropriate 1D hypotheses is defined.
    #  Default value is 10
    #  @ingroup l1_auxiliary
    def SetBoundaryBoxSegmentation(self, nbSegments):
        SMESH._objref_SMESH_Gen.SetBoundaryBoxSegmentation(self,nbSegments)

    ## Concatenate the given meshes into one mesh.
    #  @return an instance of Mesh class
    #  @param meshes the meshes to combine into one mesh
    #  @param uniteIdenticalGroups if true, groups with same names are united, else they are renamed
    #  @param mergeNodesAndElements if true, equal nodes and elements aremerged
    #  @param mergeTolerance tolerance for merging nodes
    #  @param allGroups forces creation of groups of all elements
    def Concatenate( self, meshes, uniteIdenticalGroups,
                     mergeNodesAndElements = False, mergeTolerance = 1e-5, allGroups = False):
        mergeTolerance,Parameters = geompyDC.ParseParameters(mergeTolerance)
        for i,m in enumerate(meshes):
            if isinstance(m, Mesh):
                meshes[i] = m.GetMesh()
        if allGroups:
            aSmeshMesh = SMESH._objref_SMESH_Gen.ConcatenateWithGroups(
                self,meshes,uniteIdenticalGroups,mergeNodesAndElements,mergeTolerance)
        else:
            aSmeshMesh = SMESH._objref_SMESH_Gen.Concatenate(
                self,meshes,uniteIdenticalGroups,mergeNodesAndElements,mergeTolerance)
        aSmeshMesh.SetParameters(Parameters)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh)
        return aMesh

    # Filtering. Auxiliary functions:
    # ------------------------------

    ## Creates an empty criterion
    #  @return SMESH.Filter.Criterion
    #  @ingroup l1_controls
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

    ## Creates a criterion by the given parameters
    #  @param elementType the type of elements(NODE, EDGE, FACE, VOLUME)
    #  @param CritType the type of criterion (FT_Taper, FT_Area, FT_RangeOfIds, FT_LyingOnGeom etc.)
    #  @param Compare  belongs to {FT_LessThan, FT_MoreThan, FT_EqualTo}
    #  @param Treshold the threshold value (range of ids as string, shape, numeric)
    #  @param UnaryOp  FT_LogicalNOT or FT_Undefined
    #  @param BinaryOp a binary logical operation FT_LogicalAND, FT_LogicalOR or
    #                  FT_Undefined (must be for the last criterion of all criteria)
    #  @return SMESH.Filter.Criterion
    #  @ingroup l1_controls
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
            # Checks the treshold
            if isinstance(aTreshold, geompyDC.GEOM._objref_GEOM_Object):
                aCriterion.ThresholdStr = GetName(aTreshold)
                aCriterion.ThresholdID = salome.ObjectToID(aTreshold)
            else:
                print "Error: The treshold should be a shape."
                return None
        elif CritType == FT_RangeOfIds:
            # Checks the treshold
            if isinstance(aTreshold, str):
                aCriterion.ThresholdStr = aTreshold
            else:
                print "Error: The treshold should be a string."
                return None
        elif CritType == FT_CoplanarFaces:
            # Checks the treshold
            if isinstance(aTreshold, int):
                aCriterion.ThresholdID = "%s"%aTreshold
            elif isinstance(aTreshold, str):
                ID = int(aTreshold)
                if ID < 1:
                    raise ValueError, "Invalid ID of mesh face: '%s'"%aTreshold
                aCriterion.ThresholdID = aTreshold
            else:
                raise ValueError,\
                      "The treshold should be an ID of mesh face and not '%s'"%aTreshold
        elif CritType == FT_ElemGeomType:
            # Checks the treshold
            try:
                aCriterion.Threshold = self.EnumToLong(aTreshold)
            except:
                if isinstance(aTreshold, int):
                    aCriterion.Threshold = aTreshold
                else:
                    print "Error: The treshold should be an integer or SMESH.GeometryType."
                    return None
                pass
            pass
        elif CritType == FT_GroupColor:
            # Checks the treshold
            try:
                aCriterion.ThresholdStr = self.ColorToString(aTreshold)
            except:
                print "Error: The threshold value should be of SALOMEDS.Color type"
                return None
            pass
        elif CritType in [FT_FreeBorders, FT_FreeEdges, FT_BadOrientedVolume, FT_FreeNodes,
                          FT_FreeFaces, FT_LinearOrQuadratic]:
            # At this point the treshold is unnecessary
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
                print "Error: The treshold should be a number."
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

    ## Creates a filter with the given parameters
    #  @param elementType the type of elements in the group
    #  @param CritType the type of criterion ( FT_Taper, FT_Area, FT_RangeOfIds, FT_LyingOnGeom etc. )
    #  @param Compare  belongs to {FT_LessThan, FT_MoreThan, FT_EqualTo}
    #  @param Treshold the threshold value (range of id ids as string, shape, numeric)
    #  @param UnaryOp  FT_LogicalNOT or FT_Undefined
    #  @return SMESH_Filter
    #  @ingroup l1_controls
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
        aFilterMgr.Destroy()
        return aFilter

    ## Creates a numerical functor by its type
    #  @param theCriterion FT_...; functor type
    #  @return SMESH_NumericalFunctor
    #  @ingroup l1_controls
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
        elif theCriterion == FT_MaxElementLength2D:
            return aFilterMgr.CreateMaxElementLength2D()
        elif theCriterion == FT_MaxElementLength3D:
            return aFilterMgr.CreateMaxElementLength3D()
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

    ## Creates hypothesis
    #  @param theHType mesh hypothesis type (string)
    #  @param theLibName mesh plug-in library name
    #  @return created hypothesis instance
    def CreateHypothesis(self, theHType, theLibName="libStdMeshersEngine.so"):
        return SMESH._objref_SMESH_Gen.CreateHypothesis(self, theHType, theLibName )

    ## Gets the mesh stattistic
    #  @return dictionary type element - count of elements
    #  @ingroup l1_meshinfo
    def GetMeshInfo(self, obj):
        if isinstance( obj, Mesh ):
            obj = obj.GetMesh()
        d = {}
        if hasattr(obj, "_narrow") and obj._narrow(SMESH.SMESH_IDSource):
            values = obj.GetMeshInfo() 
            for i in range(SMESH.Entity_Last._v):
                if i < len(values): d[SMESH.EntityType._item(i)]=values[i]
            pass
        return d

    ## Get minimum distance between two objects
    #
    #  If @a src2 is None, and @a id2 = 0, distance from @a src1 / @a id1 to the origin is computed.
    #  If @a src2 is None, and @a id2 != 0, it is assumed that both @a id1 and @a id2 belong to @a src1.
    #
    #  @param src1 first source object 
    #  @param src2 second source object
    #  @param id1 node/element id from the first source
    #  @param id2 node/element id from the second (or first) source
    #  @param isElem1 @c True if @a id1 is element id, @c False if it is node id
    #  @param isElem2 @c True if @a id2 is element id, @c False if it is node id
    #  @return minimum distance value
    #  @sa GetMinDistance()
    #  @ingroup l1_measurements
    def MinDistance(self, src1, src2=None, id1=0, id2=0, isElem1=False, isElem2=False):
        result = self.GetMinDistance(src1, src2, id1, id2, isElem1, isElem2)
        if result is None:
            result = 0.0
        else:
            result = result.value
        return result
    
    ## Get measure structure specifying minimum distance data between two objects
    #
    #  If @a src2 is None, and @a id2 = 0, distance from @a src1 / @a id1 to the origin is computed.
    #  If @a src2 is None, and @a id2 != 0, it is assumed that both @a id1 and @a id2 belong to @a src1.
    #
    #  @param src1 first source object 
    #  @param src2 second source object
    #  @param id1 node/element id from the first source
    #  @param id2 node/element id from the second (or first) source
    #  @param isElem1 @c True if @a id1 is element id, @c False if it is node id
    #  @param isElem2 @c True if @a id2 is element id, @c False if it is node id
    #  @return Measure structure or None if input data is invalid
    #  @sa MinDistance()
    #  @ingroup l1_measurements
    def GetMinDistance(self, src1, src2=None, id1=0, id2=0, isElem1=False, isElem2=False):
        if isinstance(src1, Mesh): src1 = src1.mesh
        if isinstance(src2, Mesh): src2 = src2.mesh
        if src2 is None and id2 != 0: src2 = src1
        if not hasattr(src1, "_narrow"): return None
        src1 = src1._narrow(SMESH.SMESH_IDSource)
        if not src1: return None
        if id1 != 0:
            m = src1.GetMesh()
            e = m.GetMeshEditor()
            if isElem1:
                src1 = e.MakeIDSource([id1], SMESH.FACE)
            else:
                src1 = e.MakeIDSource([id1], SMESH.NODE)
            pass
        if hasattr(src2, "_narrow"):
            src2 = src2._narrow(SMESH.SMESH_IDSource)
            if src2 and id2 != 0:
                m = src2.GetMesh()
                e = m.GetMeshEditor()
                if isElem2:
                    src2 = e.MakeIDSource([id2], SMESH.FACE)
                else:
                    src2 = e.MakeIDSource([id2], SMESH.NODE)
                pass
            pass
        aMeasurements = self.CreateMeasurements()
        result = aMeasurements.MinDistance(src1, src2)
        aMeasurements.Destroy()
        return result
    
    ## Get bounding box of the specified object(s)
    #  @param objects single source object or list of source objects
    #  @return tuple of six values (minX, minY, minZ, maxX, maxY, maxZ)
    #  @sa GetBoundingBox()
    #  @ingroup l1_measurements
    def BoundingBox(self, objects):
        result = self.GetBoundingBox(objects)
        if result is None:
            result = (0.0,)*6
        else:
            result = (result.minX, result.minY, result.minZ, result.maxX, result.maxY, result.maxZ)
        return result

    ## Get measure structure specifying bounding box data of the specified object(s)
    #  @param objects single source object or list of source objects
    #  @return Measure structure
    #  @sa BoundingBox()
    #  @ingroup l1_measurements
    def GetBoundingBox(self, objects):
        if isinstance(objects, tuple):
            objects = list(objects)
        if not isinstance(objects, list):
            objects = [objects]
        srclist = []
        for o in objects:
            if isinstance(o, Mesh):
                srclist.append(o.mesh)
            elif hasattr(o, "_narrow"):
                src = o._narrow(SMESH.SMESH_IDSource)
                if src: srclist.append(src)
                pass
            pass
        aMeasurements = self.CreateMeasurements()
        result = aMeasurements.BoundingBox(srclist)
        aMeasurements.Destroy()
        return result

import omniORB
#Registering the new proxy for SMESH_Gen
omniORB.registerObjref(SMESH._objref_SMESH_Gen._NP_RepositoryId, smeshDC)


# Public class: Mesh
# ==================

## This class allows defining and managing a mesh.
#  It has a set of methods to build a mesh on the given geometry, including the definition of sub-meshes.
#  It also has methods to define groups of mesh elements, to modify a mesh (by addition of
#  new nodes and elements and by changing the existing entities), to get information
#  about a mesh and to export a mesh into different formats.
class Mesh:

    geom = 0
    mesh = 0
    editor = 0

    ## Constructor
    #
    #  Creates a mesh on the shape \a obj (or an empty mesh if \a obj is equal to 0) and
    #  sets the GUI name of this mesh to \a name.
    #  @param smeshpyD an instance of smeshDC class
    #  @param geompyD an instance of geompyDC class
    #  @param obj Shape to be meshed or SMESH_Mesh object
    #  @param name Study name of the mesh
    #  @ingroup l2_construct
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
            self.smeshpyD.SetName(self.mesh, name)
        elif obj != 0:
            self.smeshpyD.SetName(self.mesh, GetName(obj))

        if not self.geom:
            self.geom = self.mesh.GetShapeToMesh()

        self.editor = self.mesh.GetMeshEditor()

    ## Initializes the Mesh object from an instance of SMESH_Mesh interface
    #  @param theMesh a SMESH_Mesh object
    #  @ingroup l2_construct
    def SetMesh(self, theMesh):
        self.mesh = theMesh
        self.geom = self.mesh.GetShapeToMesh()

    ## Returns the mesh, that is an instance of SMESH_Mesh interface
    #  @return a SMESH_Mesh object
    #  @ingroup l2_construct
    def GetMesh(self):
        return self.mesh

    ## Gets the name of the mesh
    #  @return the name of the mesh as a string
    #  @ingroup l2_construct
    def GetName(self):
        name = GetName(self.GetMesh())
        return name

    ## Sets a name to the mesh
    #  @param name a new name of the mesh
    #  @ingroup l2_construct
    def SetName(self, name):
        self.smeshpyD.SetName(self.GetMesh(), name)

    ## Gets the subMesh object associated to a \a theSubObject geometrical object.
    #  The subMesh object gives access to the IDs of nodes and elements.
    #  @param theSubObject a geometrical object (shape)
    #  @param theName a name for the submesh
    #  @return an object of type SMESH_SubMesh, representing a part of mesh, which lies on the given shape
    #  @ingroup l2_submeshes
    def GetSubMesh(self, theSubObject, theName):
        submesh = self.mesh.GetSubMesh(theSubObject, theName)
        return submesh

    ## Returns the shape associated to the mesh
    #  @return a GEOM_Object
    #  @ingroup l2_construct
    def GetShape(self):
        return self.geom

    ## Associates the given shape to the mesh (entails the recreation of the mesh)
    #  @param geom the shape to be meshed (GEOM_Object)
    #  @ingroup l2_construct
    def SetShape(self, geom):
        self.mesh = self.smeshpyD.CreateMesh(geom)

    ## Returns true if the hypotheses are defined well
    #  @param theSubObject a subshape of a mesh shape
    #  @return True or False
    #  @ingroup l2_construct
    def IsReadyToCompute(self, theSubObject):
        return self.smeshpyD.IsReadyToCompute(self.mesh, theSubObject)

    ## Returns errors of hypotheses definition.
    #  The list of errors is empty if everything is OK.
    #  @param theSubObject a subshape of a mesh shape
    #  @return a list of errors
    #  @ingroup l2_construct
    def GetAlgoState(self, theSubObject):
        return self.smeshpyD.GetAlgoState(self.mesh, theSubObject)

    ## Returns a geometrical object on which the given element was built.
    #  The returned geometrical object, if not nil, is either found in the
    #  study or published by this method with the given name
    #  @param theElementID the id of the mesh element
    #  @param theGeomName the user-defined name of the geometrical object
    #  @return GEOM::GEOM_Object instance
    #  @ingroup l2_construct
    def GetGeometryByMeshElement(self, theElementID, theGeomName):
        return self.smeshpyD.GetGeometryByMeshElement( self.mesh, theElementID, theGeomName )

    ## Returns the mesh dimension depending on the dimension of the underlying shape
    #  @return mesh dimension as an integer value [0,3]
    #  @ingroup l1_auxiliary
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
    #  If the optional \a algo parameter is not set, this algorithm is REGULAR.
    #  \n If the optional \a geom parameter is not set, this algorithm is global.
    #  Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param algo the type of the required algorithm. Possible values are:
    #     - smesh.REGULAR,
    #     - smesh.PYTHON for discretization via a python function,
    #     - smesh.COMPOSITE for meshing a set of edges on one face side as a whole.
    #  @param geom If defined is the subshape to be meshed
    #  @return an instance of Mesh_Segment or Mesh_Segment_Python, or Mesh_CompositeSegment class
    #  @ingroup l3_algos_basic
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

    ## Creates 1D algorithm importing segments conatined in groups of other mesh.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom If defined the subshape is to be meshed
    #  @return an instance of Mesh_UseExistingElements class
    #  @ingroup l3_algos_basic
    def UseExisting1DElements(self, geom=0):
        return Mesh_UseExistingElements(1,self, geom)

    ## Creates 2D algorithm importing faces conatined in groups of other mesh.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom If defined the subshape is to be meshed
    #  @return an instance of Mesh_UseExistingElements class
    #  @ingroup l3_algos_basic
    def UseExisting2DElements(self, geom=0):
        return Mesh_UseExistingElements(2,self, geom)

    ## Enables creation of nodes and segments usable by 2D algoritms.
    #  The added nodes and segments must be bound to edges and vertices by
    #  SetNodeOnVertex(), SetNodeOnEdge() and SetMeshElementOnShape()
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  \n Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom the subshape to be manually meshed
    #  @return StdMeshers_UseExisting_1D algorithm that generates nothing
    #  @ingroup l3_algos_basic
    def UseExistingSegments(self, geom=0):
        algo = Mesh_UseExisting(1,self,geom)
        return algo.GetAlgorithm()

    ## Enables creation of nodes and faces usable by 3D algoritms.
    #  The added nodes and faces must be bound to geom faces by SetNodeOnFace()
    #  and SetMeshElementOnShape()
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  \n Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom the subshape to be manually meshed
    #  @return StdMeshers_UseExisting_2D algorithm that generates nothing
    #  @ingroup l3_algos_basic
    def UseExistingFaces(self, geom=0):
        algo = Mesh_UseExisting(2,self,geom)
        return algo.GetAlgorithm()

    ## Creates a triangle 2D algorithm for faces.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  \n Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param algo values are: smesh.MEFISTO || smesh.NETGEN_1D2D || smesh.NETGEN_2D || smesh.BLSURF
    #  @param geom If defined, the subshape to be meshed (GEOM_Object)
    #  @return an instance of Mesh_Triangle algorithm
    #  @ingroup l3_algos_basic
    def Triangle(self, algo=MEFISTO, geom=0):
        ## if Triangle(geom) is called by mistake
        if (isinstance(algo, geompyDC.GEOM._objref_GEOM_Object)):
            geom = algo
            algo = MEFISTO
        return Mesh_Triangle(self, algo, geom)

    ## Creates a quadrangle 2D algorithm for faces.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  \n Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom If defined, the subshape to be meshed (GEOM_Object)
    #  @param algo values are: smesh.QUADRANGLE || smesh.RADIAL_QUAD
    #  @return an instance of Mesh_Quadrangle algorithm
    #  @ingroup l3_algos_basic
    def Quadrangle(self, geom=0, algo=QUADRANGLE):
        if algo==RADIAL_QUAD:
            return Mesh_RadialQuadrangle1D2D(self,geom)
        else:
            return Mesh_Quadrangle(self, geom)

    ## Creates a tetrahedron 3D algorithm for solids.
    #  The parameter \a algo permits to choose the algorithm: NETGEN or GHS3D
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  \n Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param algo values are: smesh.NETGEN, smesh.GHS3D, smesh.GHS3DPRL, smesh.FULL_NETGEN
    #  @param geom If defined, the subshape to be meshed (GEOM_Object)
    #  @return an instance of Mesh_Tetrahedron algorithm
    #  @ingroup l3_algos_basic
    def Tetrahedron(self, algo=NETGEN, geom=0):
        ## if Tetrahedron(geom) is called by mistake
        if ( isinstance( algo, geompyDC.GEOM._objref_GEOM_Object)):
            algo, geom = geom, algo
            if not algo: algo = NETGEN
            pass
        return Mesh_Tetrahedron(self,  algo, geom)

    ## Creates a hexahedron 3D algorithm for solids.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  \n Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param algo possible values are: smesh.Hexa, smesh.Hexotic
    #  @param geom If defined, the subshape to be meshed (GEOM_Object)
    #  @return an instance of Mesh_Hexahedron algorithm
    #  @ingroup l3_algos_basic
    def Hexahedron(self, algo=Hexa, geom=0):
        ## if Hexahedron(geom, algo) or Hexahedron(geom) is called by mistake
        if ( isinstance(algo, geompyDC.GEOM._objref_GEOM_Object) ):
            if   geom in [Hexa, Hexotic]: algo, geom = geom, algo
            elif geom == 0:               algo, geom = Hexa, algo
        return Mesh_Hexahedron(self, algo, geom)

    ## Deprecated, used only for compatibility!
    #  @return an instance of Mesh_Netgen algorithm
    #  @ingroup l3_algos_basic
    def Netgen(self, is3D, geom=0):
        return Mesh_Netgen(self,  is3D, geom)

    ## Creates a projection 1D algorithm for edges.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom If defined, the subshape to be meshed
    #  @return an instance of Mesh_Projection1D algorithm
    #  @ingroup l3_algos_proj
    def Projection1D(self, geom=0):
        return Mesh_Projection1D(self,  geom)

    ## Creates a projection 2D algorithm for faces.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom If defined, the subshape to be meshed
    #  @return an instance of Mesh_Projection2D algorithm
    #  @ingroup l3_algos_proj
    def Projection2D(self, geom=0):
        return Mesh_Projection2D(self,  geom)

    ## Creates a projection 3D algorithm for solids.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom If defined, the subshape to be meshed
    #  @return an instance of Mesh_Projection3D algorithm
    #  @ingroup l3_algos_proj
    def Projection3D(self, geom=0):
        return Mesh_Projection3D(self,  geom)

    ## Creates a 3D extrusion (Prism 3D) or RadialPrism 3D algorithm for solids.
    #  If the optional \a geom parameter is not set, this algorithm is global.
    #  Otherwise, this algorithm defines a submesh based on \a geom subshape.
    #  @param geom If defined, the subshape to be meshed
    #  @return an instance of Mesh_Prism3D or Mesh_RadialPrism3D algorithm
    #  @ingroup l3_algos_radialp l3_algos_3dextr
    def Prism(self, geom=0):
        shape = geom
        if shape==0:
            shape = self.geom
        nbSolids = len( self.geompyD.SubShapeAll( shape, geompyDC.ShapeType["SOLID"] ))
        nbShells = len( self.geompyD.SubShapeAll( shape, geompyDC.ShapeType["SHELL"] ))
        if nbSolids == 0 or nbSolids == nbShells:
            return Mesh_Prism3D(self,  geom)
        return Mesh_RadialPrism3D(self,  geom)

    ## Evaluates size of prospective mesh on a shape
    #  @return a list where i-th element is a number of elements of i-th SMESH.EntityType
    #  To know predicted number of e.g. edges, inquire it this way
    #  Evaluate()[ EnumToLong( Entity_Edge )]
    def Evaluate(self, geom=0):
        if geom == 0 or not isinstance(geom, geompyDC.GEOM._objref_GEOM_Object):
            if self.geom == 0:
                geom = self.mesh.GetShapeToMesh()
            else:
                geom = self.geom
        return self.smeshpyD.Evaluate(self.mesh, geom)


    ## Computes the mesh and returns the status of the computation
    #  @param geom geomtrical shape on which mesh data should be computed
    #  @param discardModifs if True and the mesh has been edited since
    #         a last total re-compute and that may prevent successful partial re-compute,
    #         then the mesh is cleaned before Compute()
    #  @return True or False
    #  @ingroup l2_construct
    def Compute(self, geom=0, discardModifs=False):
        if geom == 0 or not isinstance(geom, geompyDC.GEOM._objref_GEOM_Object):
            if self.geom == 0:
                geom = self.mesh.GetShapeToMesh()
            else:
                geom = self.geom
        ok = False
        try:
            if discardModifs and self.mesh.HasModificationsToDiscard(): # issue 0020693
                self.mesh.Clear()
            ok = self.smeshpyD.Compute(self.mesh, geom)
        except SALOME.SALOME_Exception, ex:
            print "Mesh computation failed, exception caught:"
            print "    ", ex.details.text
        except:
            import traceback
            print "Mesh computation failed, exception caught:"
            traceback.print_exc()
        if True:#not ok:
            allReasons = ""

            # Treat compute errors
            computeErrors = self.smeshpyD.GetComputeErrors( self.mesh, geom )
            for err in computeErrors:
                shapeText = ""
                if self.mesh.HasShapeToMesh():
                    try:
                        mainIOR  = salome.orb.object_to_string(geom)
                        for sname in salome.myStudyManager.GetOpenStudies():
                            s = salome.myStudyManager.GetStudyByName(sname)
                            if not s: continue
                            mainSO = s.FindObjectIOR(mainIOR)
                            if not mainSO: continue
                            if err.subShapeID == 1:
                                shapeText = ' on "%s"' % mainSO.GetName()
                            subIt = s.NewChildIterator(mainSO)
                            while subIt.More():
                                subSO = subIt.Value()
                                subIt.Next()
                                obj = subSO.GetObject()
                                if not obj: continue
                                go = obj._narrow( geompyDC.GEOM._objref_GEOM_Object )
                                if not go: continue
                                ids = go.GetSubShapeIndices()
                                if len(ids) == 1 and ids[0] == err.subShapeID:
                                    shapeText = ' on "%s"' % subSO.GetName()
                                    break
                        if not shapeText:
                            shape = self.geompyD.GetSubShape( geom, [err.subShapeID])
                            if shape:
                                shapeText = " on %s #%s" % (shape.GetShapeType(), err.subShapeID)
                            else:
                                shapeText = " on subshape #%s" % (err.subShapeID)
                    except:
                        shapeText = " on subshape #%s" % (err.subShapeID)
                errText = ""
                stdErrors = ["OK",                 #COMPERR_OK            
                             "Invalid input mesh", #COMPERR_BAD_INPUT_MESH
                             "std::exception",     #COMPERR_STD_EXCEPTION 
                             "OCC exception",      #COMPERR_OCC_EXCEPTION 
                             "SALOME exception",   #COMPERR_SLM_EXCEPTION 
                             "Unknown exception",  #COMPERR_EXCEPTION     
                             "Memory allocation problem", #COMPERR_MEMORY_PB     
                             "Algorithm failed",   #COMPERR_ALGO_FAILED   
                             "Unexpected geometry"]#COMPERR_BAD_SHAPE
                if err.code > 0:
                    if err.code < len(stdErrors): errText = stdErrors[err.code]
                else:
                    errText = "code %s" % -err.code
                if errText: errText += ". "
                errText += err.comment
                if allReasons != "":allReasons += "\n"
                allReasons += '"%s" failed%s. Error: %s' %(err.algoName, shapeText, errText)
                pass

            # Treat hyp errors
            errors = self.smeshpyD.GetAlgoState( self.mesh, geom )
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
                    reason = ('%s %sD algorithm "%s" is assigned to mismatching'
                              'geometry' % ( glob, dim, name ))
                else:
                    reason = "For unknown reason."+\
                             " Revise Mesh.Compute() implementation in smeshDC.py!"
                    pass
                if allReasons != "":allReasons += "\n"
                allReasons += reason
                pass
            if allReasons != "":
                print '"' + GetName(self.mesh) + '"',"has not been computed:"
                print allReasons
                ok = False
            elif not ok:
                print '"' + GetName(self.mesh) + '"',"has not been computed."
                pass
            pass
        if salome.sg.hasDesktop():
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), ok, (self.NbNodes()==0) )
            salome.sg.updateObjBrowser(1)
            pass
        return ok

    ## Return submesh objects list in meshing order
    #  @return list of list of submesh objects
    #  @ingroup l2_construct
    def GetMeshOrder(self):
        return self.mesh.GetMeshOrder()

    ## Return submesh objects list in meshing order
    #  @return list of list of submesh objects
    #  @ingroup l2_construct
    def SetMeshOrder(self, submeshes):
        return self.mesh.SetMeshOrder(submeshes)

    ## Removes all nodes and elements
    #  @ingroup l2_construct
    def Clear(self):
        self.mesh.Clear()
        if salome.sg.hasDesktop():
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), False, True )
            salome.sg.updateObjBrowser(1)

    ## Removes all nodes and elements of indicated shape
    #  @ingroup l2_construct
    def ClearSubMesh(self, geomId):
        self.mesh.ClearSubMesh(geomId)
        if salome.sg.hasDesktop():
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), False, True )
            salome.sg.updateObjBrowser(1)

    ## Computes a tetrahedral mesh using AutomaticLength + MEFISTO + NETGEN
    #  @param fineness [0,-1] defines mesh fineness
    #  @return True or False
    #  @ingroup l3_algos_basic
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

    ## Computes an hexahedral mesh using AutomaticLength + Quadrangle + Hexahedron
    #  @param fineness [0,-1] defines mesh fineness
    #  @return True or False
    #  @ingroup l3_algos_basic
    def AutomaticHexahedralization(self, fineness=0):
        dim = self.MeshDimension()
        # assign the hypotheses
        self.RemoveGlobalHypotheses()
        self.Segment().AutomaticLength(fineness)
        if dim > 1 :
            self.Quadrangle()
            pass
        if dim > 2 :
            self.Hexahedron()
            pass
        return self.Compute()

    ## Assigns a hypothesis
    #  @param hyp a hypothesis to assign
    #  @param geom a subhape of mesh geometry
    #  @return SMESH.Hypothesis_Status
    #  @ingroup l2_hypotheses
    def AddHypothesis(self, hyp, geom=0):
        if isinstance( hyp, Mesh_Algorithm ):
            hyp = hyp.GetAlgorithm()
            pass
        if not geom:
            geom = self.geom
            if not geom:
                geom = self.mesh.GetShapeToMesh()
            pass
        status = self.mesh.AddHypothesis(geom, hyp)
        isAlgo = hyp._narrow( SMESH_Algo )
        hyp_name = GetName( hyp )
        geom_name = ""
        if geom:
            geom_name = GetName( geom )
        TreatHypoStatus( status, hyp_name, geom_name, isAlgo )
        return status

    ## Unassigns a hypothesis
    #  @param hyp a hypothesis to unassign
    #  @param geom a subshape of mesh geometry
    #  @return SMESH.Hypothesis_Status
    #  @ingroup l2_hypotheses
    def RemoveHypothesis(self, hyp, geom=0):
        if isinstance( hyp, Mesh_Algorithm ):
            hyp = hyp.GetAlgorithm()
            pass
        if not geom:
            geom = self.geom
            pass
        status = self.mesh.RemoveHypothesis(geom, hyp)
        return status

    ## Gets the list of hypotheses added on a geometry
    #  @param geom a subshape of mesh geometry
    #  @return the sequence of SMESH_Hypothesis
    #  @ingroup l2_hypotheses
    def GetHypothesisList(self, geom):
        return self.mesh.GetHypothesisList( geom )

    ## Removes all global hypotheses
    #  @ingroup l2_hypotheses
    def RemoveGlobalHypotheses(self):
        current_hyps = self.mesh.GetHypothesisList( self.geom )
        for hyp in current_hyps:
            self.mesh.RemoveHypothesis( self.geom, hyp )
            pass
        pass

    ## Creates a mesh group based on the geometric object \a grp
    #  and gives a \a name, \n if this parameter is not defined
    #  the name is the same as the geometric group name \n
    #  Note: Works like GroupOnGeom().
    #  @param grp  a geometric group, a vertex, an edge, a face or a solid
    #  @param name the name of the mesh group
    #  @return SMESH_GroupOnGeom
    #  @ingroup l2_grps_create
    def Group(self, grp, name=""):
        return self.GroupOnGeom(grp, name)

    ## Deprecated, used only for compatibility! Please, use ExportToMEDX() method instead.
    #  Exports the mesh in a file in MED format and chooses the \a version of MED format
    ## allowing to overwrite the file if it exists or add the exported data to its contents
    #  @param f the file name
    #  @param version values are SMESH.MED_V2_1, SMESH.MED_V2_2
    #  @param opt boolean parameter for creating/not creating
    #  the groups Group_On_All_Nodes, Group_On_All_Faces, ...
    #  @param overwrite boolean parameter for overwriting/not overwriting the file
    #  @ingroup l2_impexp
    def ExportToMED(self, f, version, opt=0, overwrite=1):
        self.mesh.ExportToMEDX(f, opt, version, overwrite)

    ## Exports the mesh in a file in MED format and chooses the \a version of MED format
    ## allowing to overwrite the file if it exists or add the exported data to its contents
    #  @param f is the file name
    #  @param auto_groups boolean parameter for creating/not creating
    #  the groups Group_On_All_Nodes, Group_On_All_Faces, ... ;
    #  the typical use is auto_groups=false.
    #  @param version MED format version(MED_V2_1 or MED_V2_2)
    #  @param overwrite boolean parameter for overwriting/not overwriting the file
    #  @ingroup l2_impexp
    def ExportMED(self, f, auto_groups=0, version=MED_V2_2, overwrite=1):
        self.mesh.ExportToMEDX(f, auto_groups, version, overwrite)

    ## Exports the mesh in a file in DAT format
    #  @param f the file name
    #  @ingroup l2_impexp
    def ExportDAT(self, f):
        self.mesh.ExportDAT(f)

    ## Exports the mesh in a file in UNV format
    #  @param f the file name
    #  @ingroup l2_impexp
    def ExportUNV(self, f):
        self.mesh.ExportUNV(f)

    ## Export the mesh in a file in STL format
    #  @param f the file name
    #  @param ascii defines the file encoding
    #  @ingroup l2_impexp
    def ExportSTL(self, f, ascii=1):
        self.mesh.ExportSTL(f, ascii)


    # Operations with groups:
    # ----------------------

    ## Creates an empty mesh group
    #  @param elementType the type of elements in the group
    #  @param name the name of the mesh group
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
    def CreateEmptyGroup(self, elementType, name):
        return self.mesh.CreateGroup(elementType, name)

    ## Creates a mesh group based on the geometrical object \a grp
    #  and gives a \a name, \n if this parameter is not defined
    #  the name is the same as the geometrical group name
    #  @param grp  a geometrical group, a vertex, an edge, a face or a solid
    #  @param name the name of the mesh group
    #  @param typ  the type of elements in the group. If not set, it is
    #              automatically detected by the type of the geometry
    #  @return SMESH_GroupOnGeom
    #  @ingroup l2_grps_create
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
                try: # it raises on a compound of compounds
                    if len( self.geompyD.GetObjectIDs( grp )) == 0:
                        print "Mesh.Group: empty geometric group", GetName( grp )
                        return 0
                    pass
                except:
                    pass
                if grp.GetType() == 37: # GEOMImpl_Types.hxx: #define GEOM_GROUP 37
                    # group
                    tgeo = self.geompyD.GetType(grp)
                    if tgeo == geompyDC.ShapeType["VERTEX"]:
                        typ = NODE
                    elif tgeo == geompyDC.ShapeType["EDGE"]:
                        typ = EDGE
                    elif tgeo == geompyDC.ShapeType["FACE"]:
                        typ = FACE
                    elif tgeo == geompyDC.ShapeType["SOLID"]:
                        typ = VOLUME
                        pass
                    pass
                else:
                    # just a compound
                    for elemType, shapeType in [[VOLUME,"SOLID"],[FACE,"FACE"],
                                                [EDGE,"EDGE"],[NODE,"VERTEX"]]:
                        if self.geompyD.SubShapeAll(grp,geompyDC.ShapeType[shapeType]):
                            typ = elemType
                            break
                        pass
                    pass
                pass
            pass
        if typ == None:
            print "Mesh.Group: bad first argument: expected a group, a vertex, an edge, a face or a solid"
            return 0
        else:
            return self.mesh.CreateGroupFromGEOM(typ, name, grp)

    ## Creates a mesh group by the given ids of elements
    #  @param groupName the name of the mesh group
    #  @param elementType the type of elements in the group
    #  @param elemIDs the list of ids
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
    def MakeGroupByIds(self, groupName, elementType, elemIDs):
        group = self.mesh.CreateGroup(elementType, groupName)
        group.Add(elemIDs)
        return group

    ## Creates a mesh group by the given conditions
    #  @param groupName the name of the mesh group
    #  @param elementType the type of elements in the group
    #  @param CritType the type of criterion( FT_Taper, FT_Area, FT_RangeOfIds, FT_LyingOnGeom etc. )
    #  @param Compare belongs to {FT_LessThan, FT_MoreThan, FT_EqualTo}
    #  @param Treshold the threshold value (range of id ids as string, shape, numeric)
    #  @param UnaryOp FT_LogicalNOT or FT_Undefined
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
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

    ## Creates a mesh group by the given criterion
    #  @param groupName the name of the mesh group
    #  @param Criterion the instance of Criterion class
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
    def MakeGroupByCriterion(self, groupName, Criterion):
        aFilterMgr = self.smeshpyD.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aCriteria = []
        aCriteria.append(Criterion)
        aFilter.SetCriteria(aCriteria)
        group = self.MakeGroupByFilter(groupName, aFilter)
        aFilterMgr.Destroy()
        return group

    ## Creates a mesh group by the given criteria (list of criteria)
    #  @param groupName the name of the mesh group
    #  @param theCriteria the list of criteria
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
    def MakeGroupByCriteria(self, groupName, theCriteria):
        aFilterMgr = self.smeshpyD.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aFilter.SetCriteria(theCriteria)
        group = self.MakeGroupByFilter(groupName, aFilter)
        aFilterMgr.Destroy()
        return group

    ## Creates a mesh group by the given filter
    #  @param groupName the name of the mesh group
    #  @param theFilter the instance of Filter class
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
    def MakeGroupByFilter(self, groupName, theFilter):
        group = self.CreateEmptyGroup(theFilter.GetElementType(), groupName)
        theFilter.SetMesh( self.mesh )
        group.AddFrom( theFilter )
        return group

    ## Passes mesh elements through the given filter and return IDs of fitting elements
    #  @param theFilter SMESH_Filter
    #  @return a list of ids
    #  @ingroup l1_controls
    def GetIdsFromFilter(self, theFilter):
        theFilter.SetMesh( self.mesh )
        return theFilter.GetIDs()

    ## Verifies whether a 2D mesh element has free edges (edges connected to one face only)\n
    #  Returns a list of special structures (borders).
    #  @return a list of SMESH.FreeEdges.Border structure: edge id and ids of two its nodes.
    #  @ingroup l1_controls
    def GetFreeBorders(self):
        aFilterMgr = self.smeshpyD.CreateFilterManager()
        aPredicate = aFilterMgr.CreateFreeEdges()
        aPredicate.SetMesh(self.mesh)
        aBorders = aPredicate.GetBorders()
        aFilterMgr.Destroy()
        return aBorders

    ## Removes a group
    #  @ingroup l2_grps_delete
    def RemoveGroup(self, group):
        self.mesh.RemoveGroup(group)

    ## Removes a group with its contents
    #  @ingroup l2_grps_delete
    def RemoveGroupWithContents(self, group):
        self.mesh.RemoveGroupWithContents(group)

    ## Gets the list of groups existing in the mesh
    #  @return a sequence of SMESH_GroupBase
    #  @ingroup l2_grps_create
    def GetGroups(self):
        return self.mesh.GetGroups()

    ## Gets the number of groups existing in the mesh
    #  @return the quantity of groups as an integer value
    #  @ingroup l2_grps_create
    def NbGroups(self):
        return self.mesh.NbGroups()

    ## Gets the list of names of groups existing in the mesh
    #  @return list of strings
    #  @ingroup l2_grps_create
    def GetGroupNames(self):
        groups = self.GetGroups()
        names = []
        for group in groups:
            names.append(group.GetName())
        return names

    ## Produces a union of two groups
    #  A new group is created. All mesh elements that are
    #  present in the initial groups are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def UnionGroups(self, group1, group2, name):
        return self.mesh.UnionGroups(group1, group2, name)
        
    ## Produces a union list of groups
    #  New group is created. All mesh elements that are present in 
    #  initial groups are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def UnionListOfGroups(self, groups, name):
      return self.mesh.UnionListOfGroups(groups, name)
      
    ## Prodices an intersection of two groups
    #  A new group is created. All mesh elements that are common
    #  for the two initial groups are added to the new one.
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def IntersectGroups(self, group1, group2, name):
        return self.mesh.IntersectGroups(group1, group2, name)
        
    ## Produces an intersection of groups
    #  New group is created. All mesh elements that are present in all 
    #  initial groups simultaneously are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def IntersectListOfGroups(self, groups, name):
      return self.mesh.IntersectListOfGroups(groups, name)

    ## Produces a cut of two groups
    #  A new group is created. All mesh elements that are present in
    #  the main group but are not present in the tool group are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def CutGroups(self, main_group, tool_group, name):
        return self.mesh.CutGroups(main_group, tool_group, name)
        
    ## Produces a cut of groups
    #  A new group is created. All mesh elements that are present in main groups 
    #  but do not present in tool groups are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def CutListOfGroups(self, main_groups, tool_groups, name):
      return self.mesh.CutListOfGroups(main_groups, tool_groups, name)
      
    ## Produces a group of elements with specified element type using list of existing groups
    #  A new group is created. System 
    #  1) extract all nodes on which groups elements are built
    #  2) combine all elements of specified dimension laying on these nodes
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def CreateDimGroup(self, groups, elem_type, name):
      return self.mesh.CreateDimGroup(groups, elem_type, name)


    ## Convert group on geom into standalone group
    #  @ingroup l2_grps_delete
    def ConvertToStandalone(self, group):
        return self.mesh.ConvertToStandalone(group)

    # Get some info about mesh:
    # ------------------------

    ## Returns the log of nodes and elements added or removed
    #  since the previous clear of the log.
    #  @param clearAfterGet log is emptied after Get (safe if concurrents access)
    #  @return list of log_block structures:
    #                                        commandType
    #                                        number
    #                                        coords
    #                                        indexes
    #  @ingroup l1_auxiliary
    def GetLog(self, clearAfterGet):
        return self.mesh.GetLog(clearAfterGet)

    ## Clears the log of nodes and elements added or removed since the previous
    #  clear. Must be used immediately after GetLog if clearAfterGet is false.
    #  @ingroup l1_auxiliary
    def ClearLog(self):
        self.mesh.ClearLog()

    ## Toggles auto color mode on the object.
    #  @param theAutoColor the flag which toggles auto color mode.
    #  @ingroup l1_auxiliary
    def SetAutoColor(self, theAutoColor):
        self.mesh.SetAutoColor(theAutoColor)

    ## Gets flag of object auto color mode.
    #  @return True or False
    #  @ingroup l1_auxiliary
    def GetAutoColor(self):
        return self.mesh.GetAutoColor()

    ## Gets the internal ID
    #  @return integer value, which is the internal Id of the mesh
    #  @ingroup l1_auxiliary
    def GetId(self):
        return self.mesh.GetId()

    ## Get the study Id
    #  @return integer value, which is the study Id of the mesh
    #  @ingroup l1_auxiliary
    def GetStudyId(self):
        return self.mesh.GetStudyId()

    ## Checks the group names for duplications.
    #  Consider the maximum group name length stored in MED file.
    #  @return True or False
    #  @ingroup l1_auxiliary
    def HasDuplicatedGroupNamesMED(self):
        return self.mesh.HasDuplicatedGroupNamesMED()

    ## Obtains the mesh editor tool
    #  @return an instance of SMESH_MeshEditor
    #  @ingroup l1_modifying
    def GetMeshEditor(self):
        return self.mesh.GetMeshEditor()

    ## Gets MED Mesh
    #  @return an instance of SALOME_MED::MESH
    #  @ingroup l1_auxiliary
    def GetMEDMesh(self):
        return self.mesh.GetMEDMesh()


    # Get informations about mesh contents:
    # ------------------------------------

    ## Gets the mesh stattistic
    #  @return dictionary type element - count of elements
    #  @ingroup l1_meshinfo
    def GetMeshInfo(self, obj = None):
        if not obj: obj = self.mesh
        return self.smeshpyD.GetMeshInfo(obj)

    ## Returns the number of nodes in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbNodes(self):
        return self.mesh.NbNodes()

    ## Returns the number of elements in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbElements(self):
        return self.mesh.NbElements()

    ## Returns the number of 0d elements in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def Nb0DElements(self):
        return self.mesh.Nb0DElements()

    ## Returns the number of edges in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbEdges(self):
        return self.mesh.NbEdges()

    ## Returns the number of edges with the given order in the mesh
    #  @param elementOrder the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbEdgesOfOrder(self, elementOrder):
        return self.mesh.NbEdgesOfOrder(elementOrder)

    ## Returns the number of faces in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbFaces(self):
        return self.mesh.NbFaces()

    ## Returns the number of faces with the given order in the mesh
    #  @param elementOrder the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbFacesOfOrder(self, elementOrder):
        return self.mesh.NbFacesOfOrder(elementOrder)

    ## Returns the number of triangles in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbTriangles(self):
        return self.mesh.NbTriangles()

    ## Returns the number of triangles with the given order in the mesh
    #  @param elementOrder is the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbTrianglesOfOrder(self, elementOrder):
        return self.mesh.NbTrianglesOfOrder(elementOrder)

    ## Returns the number of quadrangles in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbQuadrangles(self):
        return self.mesh.NbQuadrangles()

    ## Returns the number of quadrangles with the given order in the mesh
    #  @param elementOrder the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbQuadranglesOfOrder(self, elementOrder):
        return self.mesh.NbQuadranglesOfOrder(elementOrder)

    ## Returns the number of polygons in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPolygons(self):
        return self.mesh.NbPolygons()

    ## Returns the number of volumes in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbVolumes(self):
        return self.mesh.NbVolumes()

    ## Returns the number of volumes with the given order in the mesh
    #  @param elementOrder  the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbVolumesOfOrder(self, elementOrder):
        return self.mesh.NbVolumesOfOrder(elementOrder)

    ## Returns the number of tetrahedrons in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbTetras(self):
        return self.mesh.NbTetras()

    ## Returns the number of tetrahedrons with the given order in the mesh
    #  @param elementOrder  the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbTetrasOfOrder(self, elementOrder):
        return self.mesh.NbTetrasOfOrder(elementOrder)

    ## Returns the number of hexahedrons in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbHexas(self):
        return self.mesh.NbHexas()

    ## Returns the number of hexahedrons with the given order in the mesh
    #  @param elementOrder  the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbHexasOfOrder(self, elementOrder):
        return self.mesh.NbHexasOfOrder(elementOrder)

    ## Returns the number of pyramids in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPyramids(self):
        return self.mesh.NbPyramids()

    ## Returns the number of pyramids with the given order in the mesh
    #  @param elementOrder  the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPyramidsOfOrder(self, elementOrder):
        return self.mesh.NbPyramidsOfOrder(elementOrder)

    ## Returns the number of prisms in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPrisms(self):
        return self.mesh.NbPrisms()

    ## Returns the number of prisms with the given order in the mesh
    #  @param elementOrder  the order of elements:
    #         ORDER_ANY, ORDER_LINEAR or ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPrismsOfOrder(self, elementOrder):
        return self.mesh.NbPrismsOfOrder(elementOrder)

    ## Returns the number of polyhedrons in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPolyhedrons(self):
        return self.mesh.NbPolyhedrons()

    ## Returns the number of submeshes in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbSubMesh(self):
        return self.mesh.NbSubMesh()

    ## Returns the list of mesh elements IDs
    #  @return the list of integer values
    #  @ingroup l1_meshinfo
    def GetElementsId(self):
        return self.mesh.GetElementsId()

    ## Returns the list of IDs of mesh elements with the given type
    #  @param elementType  the required type of elements (SMESH.NODE, SMESH.EDGE, SMESH.FACE or SMESH.VOLUME)
    #  @return list of integer values
    #  @ingroup l1_meshinfo
    def GetElementsByType(self, elementType):
        return self.mesh.GetElementsByType(elementType)

    ## Returns the list of mesh nodes IDs
    #  @return the list of integer values
    #  @ingroup l1_meshinfo
    def GetNodesId(self):
        return self.mesh.GetNodesId()

    # Get the information about mesh elements:
    # ------------------------------------

    ## Returns the type of mesh element
    #  @return the value from SMESH::ElementType enumeration
    #  @ingroup l1_meshinfo
    def GetElementType(self, id, iselem):
        return self.mesh.GetElementType(id, iselem)

    ## Returns the geometric type of mesh element
    #  @return the value from SMESH::EntityType enumeration
    #  @ingroup l1_meshinfo
    def GetElementGeomType(self, id):
        return self.mesh.GetElementGeomType(id)

    ## Returns the list of submesh elements IDs
    #  @param Shape a geom object(subshape) IOR
    #         Shape must be the subshape of a ShapeToMesh()
    #  @return the list of integer values
    #  @ingroup l1_meshinfo
    def GetSubMeshElementsId(self, Shape):
        if ( isinstance( Shape, geompyDC.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshElementsId(ShapeID)

    ## Returns the list of submesh nodes IDs
    #  @param Shape a geom object(subshape) IOR
    #         Shape must be the subshape of a ShapeToMesh()
    #  @param all If true, gives all nodes of submesh elements, otherwise gives only submesh nodes
    #  @return the list of integer values
    #  @ingroup l1_meshinfo
    def GetSubMeshNodesId(self, Shape, all):
        if ( isinstance( Shape, geompyDC.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshNodesId(ShapeID, all)

    ## Returns type of elements on given shape
    #  @param Shape a geom object(subshape) IOR
    #         Shape must be a subshape of a ShapeToMesh()
    #  @return element type
    #  @ingroup l1_meshinfo
    def GetSubMeshElementType(self, Shape):
        if ( isinstance( Shape, geompyDC.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshElementType(ShapeID)

    ## Gets the mesh description
    #  @return string value
    #  @ingroup l1_meshinfo
    def Dump(self):
        return self.mesh.Dump()


    # Get the information about nodes and elements of a mesh by its IDs:
    # -----------------------------------------------------------

    ## Gets XYZ coordinates of a node
    #  \n If there is no nodes for the given ID - returns an empty list
    #  @return a list of double precision values
    #  @ingroup l1_meshinfo
    def GetNodeXYZ(self, id):
        return self.mesh.GetNodeXYZ(id)

    ## Returns list of IDs of inverse elements for the given node
    #  \n If there is no node for the given ID - returns an empty list
    #  @return a list of integer values
    #  @ingroup l1_meshinfo
    def GetNodeInverseElements(self, id):
        return self.mesh.GetNodeInverseElements(id)

    ## @brief Returns the position of a node on the shape
    #  @return SMESH::NodePosition
    #  @ingroup l1_meshinfo
    def GetNodePosition(self,NodeID):
        return self.mesh.GetNodePosition(NodeID)

    ## If the given element is a node, returns the ID of shape
    #  \n If there is no node for the given ID - returns -1
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def GetShapeID(self, id):
        return self.mesh.GetShapeID(id)

    ## Returns the ID of the result shape after
    #  FindShape() from SMESH_MeshEditor for the given element
    #  \n If there is no element for the given ID - returns -1
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def GetShapeIDForElem(self,id):
        return self.mesh.GetShapeIDForElem(id)

    ## Returns the number of nodes for the given element
    #  \n If there is no element for the given ID - returns -1
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def GetElemNbNodes(self, id):
        return self.mesh.GetElemNbNodes(id)

    ## Returns the node ID the given index for the given element
    #  \n If there is no element for the given ID - returns -1
    #  \n If there is no node for the given index - returns -2
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def GetElemNode(self, id, index):
        return self.mesh.GetElemNode(id, index)

    ## Returns the IDs of nodes of the given element
    #  @return a list of integer values
    #  @ingroup l1_meshinfo
    def GetElemNodes(self, id):
        return self.mesh.GetElemNodes(id)

    ## Returns true if the given node is the medium node in the given quadratic element
    #  @ingroup l1_meshinfo
    def IsMediumNode(self, elementID, nodeID):
        return self.mesh.IsMediumNode(elementID, nodeID)

    ## Returns true if the given node is the medium node in one of quadratic elements
    #  @ingroup l1_meshinfo
    def IsMediumNodeOfAnyElem(self, nodeID, elementType):
        return self.mesh.IsMediumNodeOfAnyElem(nodeID, elementType)

    ## Returns the number of edges for the given element
    #  @ingroup l1_meshinfo
    def ElemNbEdges(self, id):
        return self.mesh.ElemNbEdges(id)

    ## Returns the number of faces for the given element
    #  @ingroup l1_meshinfo
    def ElemNbFaces(self, id):
        return self.mesh.ElemNbFaces(id)

    ## Returns nodes of given face (counted from zero) for given volumic element.
    #  @ingroup l1_meshinfo
    def GetElemFaceNodes(self,elemId, faceIndex):
        return self.mesh.GetElemFaceNodes(elemId, faceIndex)

    ## Returns an element based on all given nodes.
    #  @ingroup l1_meshinfo
    def FindElementByNodes(self,nodes):
        return self.mesh.FindElementByNodes(nodes)

    ## Returns true if the given element is a polygon
    #  @ingroup l1_meshinfo
    def IsPoly(self, id):
        return self.mesh.IsPoly(id)

    ## Returns true if the given element is quadratic
    #  @ingroup l1_meshinfo
    def IsQuadratic(self, id):
        return self.mesh.IsQuadratic(id)

    ## Returns XYZ coordinates of the barycenter of the given element
    #  \n If there is no element for the given ID - returns an empty list
    #  @return a list of three double values
    #  @ingroup l1_meshinfo
    def BaryCenter(self, id):
        return self.mesh.BaryCenter(id)


    # Get mesh measurements information:
    # ------------------------------------

    ## Get minimum distance between two nodes, elements or distance to the origin
    #  @param id1 first node/element id
    #  @param id2 second node/element id (if 0, distance from @a id1 to the origin is computed)
    #  @param isElem1 @c True if @a id1 is element id, @c False if it is node id
    #  @param isElem2 @c True if @a id2 is element id, @c False if it is node id
    #  @return minimum distance value
    #  @sa GetMinDistance()
    def MinDistance(self, id1, id2=0, isElem1=False, isElem2=False):
        aMeasure = self.GetMinDistance(id1, id2, isElem1, isElem2)
        return aMeasure.value
    
    ## Get measure structure specifying minimum distance data between two objects
    #  @param id1 first node/element id
    #  @param id2 second node/element id (if 0, distance from @a id1 to the origin is computed)
    #  @param isElem1 @c True if @a id1 is element id, @c False if it is node id
    #  @param isElem2 @c True if @a id2 is element id, @c False if it is node id
    #  @return Measure structure
    #  @sa MinDistance()
    def GetMinDistance(self, id1, id2=0, isElem1=False, isElem2=False):
        if isElem1:
            id1 = self.editor.MakeIDSource([id1], SMESH.FACE)
        else:
            id1 = self.editor.MakeIDSource([id1], SMESH.NODE)
        if id2 != 0:
            if isElem2:
                id2 = self.editor.MakeIDSource([id2], SMESH.FACE)
            else:
                id2 = self.editor.MakeIDSource([id2], SMESH.NODE)
            pass
        else:
            id2 = None
        
        aMeasurements = self.smeshpyD.CreateMeasurements()
        aMeasure = aMeasurements.MinDistance(id1, id2)
        aMeasurements.Destroy()
        return aMeasure
    
    ## Get bounding box of the specified object(s)
    #  @param objects single source object or list of source objects or list of nodes/elements IDs
    #  @param isElem if @a objects is a list of IDs, @c True value in this parameters specifies that @a objects are elements,
    #  @c False specifies that @a objects are nodes
    #  @return tuple of six values (minX, minY, minZ, maxX, maxY, maxZ)
    #  @sa GetBoundingBox()
    def BoundingBox(self, objects=None, isElem=False):
        result = self.GetBoundingBox(objects, isElem)
        if result is None:
            result = (0.0,)*6
        else:
            result = (result.minX, result.minY, result.minZ, result.maxX, result.maxY, result.maxZ)
        return result

    ## Get measure structure specifying bounding box data of the specified object(s)
    #  @param objects single source object or list of source objects or list of nodes/elements IDs
    #  @param isElem if @a objects is a list of IDs, @c True value in this parameters specifies that @a objects are elements,
    #  @c False specifies that @a objects are nodes
    #  @return Measure structure
    #  @sa BoundingBox()
    def GetBoundingBox(self, IDs=None, isElem=False):
        if IDs is None:
            IDs = [self.mesh]
        elif isinstance(IDs, tuple):
            IDs = list(IDs)
        if not isinstance(IDs, list):
            IDs = [IDs]
        if len(IDs) > 0 and isinstance(IDs[0], int):
            IDs = [IDs]
        srclist = []
        for o in IDs:
            if isinstance(o, Mesh):
                srclist.append(o.mesh)
            elif hasattr(o, "_narrow"):
                src = o._narrow(SMESH.SMESH_IDSource)
                if src: srclist.append(src)
                pass
            elif isinstance(o, list):
                if isElem:
                    srclist.append(self.editor.MakeIDSource(o, SMESH.FACE))
                else:
                    srclist.append(self.editor.MakeIDSource(o, SMESH.NODE))
                pass
            pass
        aMeasurements = self.smeshpyD.CreateMeasurements()
        aMeasure = aMeasurements.BoundingBox(srclist)
        aMeasurements.Destroy()
        return aMeasure
    
    # Mesh edition (SMESH_MeshEditor functionality):
    # ---------------------------------------------

    ## Removes the elements from the mesh by ids
    #  @param IDsOfElements is a list of ids of elements to remove
    #  @return True or False
    #  @ingroup l2_modif_del
    def RemoveElements(self, IDsOfElements):
        return self.editor.RemoveElements(IDsOfElements)

    ## Removes nodes from mesh by ids
    #  @param IDsOfNodes is a list of ids of nodes to remove
    #  @return True or False
    #  @ingroup l2_modif_del
    def RemoveNodes(self, IDsOfNodes):
        return self.editor.RemoveNodes(IDsOfNodes)

    ## Removes all orphan (free) nodes from mesh
    #  @return number of the removed nodes
    #  @ingroup l2_modif_del
    def RemoveOrphanNodes(self):
        return self.editor.RemoveOrphanNodes()

    ## Add a node to the mesh by coordinates
    #  @return Id of the new node
    #  @ingroup l2_modif_add
    def AddNode(self, x, y, z):
        x,y,z,Parameters = geompyDC.ParseParameters(x,y,z)
        self.mesh.SetParameters(Parameters)
        return self.editor.AddNode( x, y, z)

    ## Creates a 0D element on a node with given number.
    #  @param IDOfNode the ID of node for creation of the element.
    #  @return the Id of the new 0D element
    #  @ingroup l2_modif_add
    def Add0DElement(self, IDOfNode):
        return self.editor.Add0DElement(IDOfNode)

    ## Creates a linear or quadratic edge (this is determined
    #  by the number of given nodes).
    #  @param IDsOfNodes the list of node IDs for creation of the element.
    #  The order of nodes in this list should correspond to the description
    #  of MED. \n This description is located by the following link:
    #  http://www.code-aster.org/outils/med/html/modele_de_donnees.html#3.
    #  @return the Id of the new edge
    #  @ingroup l2_modif_add
    def AddEdge(self, IDsOfNodes):
        return self.editor.AddEdge(IDsOfNodes)

    ## Creates a linear or quadratic face (this is determined
    #  by the number of given nodes).
    #  @param IDsOfNodes the list of node IDs for creation of the element.
    #  The order of nodes in this list should correspond to the description
    #  of MED. \n This description is located by the following link:
    #  http://www.code-aster.org/outils/med/html/modele_de_donnees.html#3.
    #  @return the Id of the new face
    #  @ingroup l2_modif_add
    def AddFace(self, IDsOfNodes):
        return self.editor.AddFace(IDsOfNodes)

    ## Adds a polygonal face to the mesh by the list of node IDs
    #  @param IdsOfNodes the list of node IDs for creation of the element.
    #  @return the Id of the new face
    #  @ingroup l2_modif_add
    def AddPolygonalFace(self, IdsOfNodes):
        return self.editor.AddPolygonalFace(IdsOfNodes)

    ## Creates both simple and quadratic volume (this is determined
    #  by the number of given nodes).
    #  @param IDsOfNodes the list of node IDs for creation of the element.
    #  The order of nodes in this list should correspond to the description
    #  of MED. \n This description is located by the following link:
    #  http://www.code-aster.org/outils/med/html/modele_de_donnees.html#3.
    #  @return the Id of the new volumic element
    #  @ingroup l2_modif_add
    def AddVolume(self, IDsOfNodes):
        return self.editor.AddVolume(IDsOfNodes)

    ## Creates a volume of many faces, giving nodes for each face.
    #  @param IdsOfNodes the list of node IDs for volume creation face by face.
    #  @param Quantities the list of integer values, Quantities[i]
    #         gives the quantity of nodes in face number i.
    #  @return the Id of the new volumic element
    #  @ingroup l2_modif_add
    def AddPolyhedralVolume (self, IdsOfNodes, Quantities):
        return self.editor.AddPolyhedralVolume(IdsOfNodes, Quantities)

    ## Creates a volume of many faces, giving the IDs of the existing faces.
    #  @param IdsOfFaces the list of face IDs for volume creation.
    #
    #  Note:  The created volume will refer only to the nodes
    #         of the given faces, not to the faces themselves.
    #  @return the Id of the new volumic element
    #  @ingroup l2_modif_add
    def AddPolyhedralVolumeByFaces (self, IdsOfFaces):
        return self.editor.AddPolyhedralVolumeByFaces(IdsOfFaces)


    ## @brief Binds a node to a vertex
    #  @param NodeID a node ID
    #  @param Vertex a vertex or vertex ID
    #  @return True if succeed else raises an exception
    #  @ingroup l2_modif_add
    def SetNodeOnVertex(self, NodeID, Vertex):
        if ( isinstance( Vertex, geompyDC.GEOM._objref_GEOM_Object)):
            VertexID = Vertex.GetSubShapeIndices()[0]
        else:
            VertexID = Vertex
        try:
            self.editor.SetNodeOnVertex(NodeID, VertexID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True


    ## @brief Stores the node position on an edge
    #  @param NodeID a node ID
    #  @param Edge an edge or edge ID
    #  @param paramOnEdge a parameter on the edge where the node is located
    #  @return True if succeed else raises an exception
    #  @ingroup l2_modif_add
    def SetNodeOnEdge(self, NodeID, Edge, paramOnEdge):
        if ( isinstance( Edge, geompyDC.GEOM._objref_GEOM_Object)):
            EdgeID = Edge.GetSubShapeIndices()[0]
        else:
            EdgeID = Edge
        try:
            self.editor.SetNodeOnEdge(NodeID, EdgeID, paramOnEdge)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    ## @brief Stores node position on a face
    #  @param NodeID a node ID
    #  @param Face a face or face ID
    #  @param u U parameter on the face where the node is located
    #  @param v V parameter on the face where the node is located
    #  @return True if succeed else raises an exception
    #  @ingroup l2_modif_add
    def SetNodeOnFace(self, NodeID, Face, u, v):
        if ( isinstance( Face, geompyDC.GEOM._objref_GEOM_Object)):
            FaceID = Face.GetSubShapeIndices()[0]
        else:
            FaceID = Face
        try:
            self.editor.SetNodeOnFace(NodeID, FaceID, u, v)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    ## @brief Binds a node to a solid
    #  @param NodeID a node ID
    #  @param Solid  a solid or solid ID
    #  @return True if succeed else raises an exception
    #  @ingroup l2_modif_add
    def SetNodeInVolume(self, NodeID, Solid):
        if ( isinstance( Solid, geompyDC.GEOM._objref_GEOM_Object)):
            SolidID = Solid.GetSubShapeIndices()[0]
        else:
            SolidID = Solid
        try:
            self.editor.SetNodeInVolume(NodeID, SolidID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    ## @brief Bind an element to a shape
    #  @param ElementID an element ID
    #  @param Shape a shape or shape ID
    #  @return True if succeed else raises an exception
    #  @ingroup l2_modif_add
    def SetMeshElementOnShape(self, ElementID, Shape):
        if ( isinstance( Shape, geompyDC.GEOM._objref_GEOM_Object)):
            ShapeID = Shape.GetSubShapeIndices()[0]
        else:
            ShapeID = Shape
        try:
            self.editor.SetMeshElementOnShape(ElementID, ShapeID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True


    ## Moves the node with the given id
    #  @param NodeID the id of the node
    #  @param x  a new X coordinate
    #  @param y  a new Y coordinate
    #  @param z  a new Z coordinate
    #  @return True if succeed else False
    #  @ingroup l2_modif_movenode
    def MoveNode(self, NodeID, x, y, z):
        x,y,z,Parameters = geompyDC.ParseParameters(x,y,z)
        self.mesh.SetParameters(Parameters)
        return self.editor.MoveNode(NodeID, x, y, z)

    ## Finds the node closest to a point and moves it to a point location
    #  @param x  the X coordinate of a point
    #  @param y  the Y coordinate of a point
    #  @param z  the Z coordinate of a point
    #  @param NodeID if specified (>0), the node with this ID is moved,
    #  otherwise, the node closest to point (@a x,@a y,@a z) is moved
    #  @return the ID of a node
    #  @ingroup l2_modif_throughp
    def MoveClosestNodeToPoint(self, x, y, z, NodeID):
        x,y,z,Parameters = geompyDC.ParseParameters(x,y,z)
        self.mesh.SetParameters(Parameters)
        return self.editor.MoveClosestNodeToPoint(x, y, z, NodeID)

    ## Finds the node closest to a point
    #  @param x  the X coordinate of a point
    #  @param y  the Y coordinate of a point
    #  @param z  the Z coordinate of a point
    #  @return the ID of a node
    #  @ingroup l2_modif_throughp
    def FindNodeClosestTo(self, x, y, z):
        #preview = self.mesh.GetMeshEditPreviewer()
        #return preview.MoveClosestNodeToPoint(x, y, z, -1)
        return self.editor.FindNodeClosestTo(x, y, z)

    ## Finds the elements where a point lays IN or ON
    #  @param x  the X coordinate of a point
    #  @param y  the Y coordinate of a point
    #  @param z  the Z coordinate of a point
    #  @param elementType type of elements to find (SMESH.ALL type
    #         means elements of any type excluding nodes and 0D elements)
    #  @return list of IDs of found elements
    #  @ingroup l2_modif_throughp
    def FindElementsByPoint(self, x, y, z, elementType = SMESH.ALL):
        return self.editor.FindElementsByPoint(x, y, z, elementType)
        
    # Return point state in a closed 2D mesh in terms of TopAbs_State enumeration.
    # TopAbs_UNKNOWN state means that either mesh is wrong or the analysis fails.
     
    def GetPointState(self, x, y, z):
        return self.editor.GetPointState(x, y, z)

    ## Finds the node closest to a point and moves it to a point location
    #  @param x  the X coordinate of a point
    #  @param y  the Y coordinate of a point
    #  @param z  the Z coordinate of a point
    #  @return the ID of a moved node
    #  @ingroup l2_modif_throughp
    def MeshToPassThroughAPoint(self, x, y, z):
        return self.editor.MoveClosestNodeToPoint(x, y, z, -1)

    ## Replaces two neighbour triangles sharing Node1-Node2 link
    #  with the triangles built on the same 4 nodes but having other common link.
    #  @param NodeID1  the ID of the first node
    #  @param NodeID2  the ID of the second node
    #  @return false if proper faces were not found
    #  @ingroup l2_modif_invdiag
    def InverseDiag(self, NodeID1, NodeID2):
        return self.editor.InverseDiag(NodeID1, NodeID2)

    ## Replaces two neighbour triangles sharing Node1-Node2 link
    #  with a quadrangle built on the same 4 nodes.
    #  @param NodeID1  the ID of the first node
    #  @param NodeID2  the ID of the second node
    #  @return false if proper faces were not found
    #  @ingroup l2_modif_unitetri
    def DeleteDiag(self, NodeID1, NodeID2):
        return self.editor.DeleteDiag(NodeID1, NodeID2)

    ## Reorients elements by ids
    #  @param IDsOfElements if undefined reorients all mesh elements
    #  @return True if succeed else False
    #  @ingroup l2_modif_changori
    def Reorient(self, IDsOfElements=None):
        if IDsOfElements == None:
            IDsOfElements = self.GetElementsId()
        return self.editor.Reorient(IDsOfElements)

    ## Reorients all elements of the object
    #  @param theObject mesh, submesh or group
    #  @return True if succeed else False
    #  @ingroup l2_modif_changori
    def ReorientObject(self, theObject):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.ReorientObject(theObject)

    ## Fuses the neighbouring triangles into quadrangles.
    #  @param IDsOfElements The triangles to be fused,
    #  @param theCriterion  is FT_...; used to choose a neighbour to fuse with.
    #  @param MaxAngle      is the maximum angle between element normals at which the fusion
    #                       is still performed; theMaxAngle is mesured in radians.
    #                       Also it could be a name of variable which defines angle in degrees.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_unitetri
    def TriToQuad(self, IDsOfElements, theCriterion, MaxAngle):
        flag = False
        if isinstance(MaxAngle,str):
            flag = True
        MaxAngle,Parameters = geompyDC.ParseParameters(MaxAngle)
        if flag:
            MaxAngle = DegreesToRadians(MaxAngle)
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        self.mesh.SetParameters(Parameters)
        Functor = 0
	if ( isinstance( theCriterion, SMESH._objref_NumericalFunctor ) ):
            Functor = theCriterion
        else:
            Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.TriToQuad(IDsOfElements, Functor, MaxAngle)

    ## Fuses the neighbouring triangles of the object into quadrangles
    #  @param theObject is mesh, submesh or group
    #  @param theCriterion is FT_...; used to choose a neighbour to fuse with.
    #  @param MaxAngle   a max angle between element normals at which the fusion
    #                   is still performed; theMaxAngle is mesured in radians.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_unitetri
    def TriToQuadObject (self, theObject, theCriterion, MaxAngle):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.TriToQuadObject(theObject, self.smeshpyD.GetFunctor(theCriterion), MaxAngle)

    ## Splits quadrangles into triangles.
    #  @param IDsOfElements the faces to be splitted.
    #  @param theCriterion   FT_...; used to choose a diagonal for splitting.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_cutquadr
    def QuadToTri (self, IDsOfElements, theCriterion):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.QuadToTri(IDsOfElements, self.smeshpyD.GetFunctor(theCriterion))

    ## Splits quadrangles into triangles.
    #  @param theObject  the object from which the list of elements is taken, this is mesh, submesh or group
    #  @param theCriterion   FT_...; used to choose a diagonal for splitting.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_cutquadr
    def QuadToTriObject (self, theObject, theCriterion):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.QuadToTriObject(theObject, self.smeshpyD.GetFunctor(theCriterion))

    ## Splits quadrangles into triangles.
    #  @param IDsOfElements the faces to be splitted
    #  @param Diag13        is used to choose a diagonal for splitting.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_cutquadr
    def SplitQuad (self, IDsOfElements, Diag13):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.SplitQuad(IDsOfElements, Diag13)

    ## Splits quadrangles into triangles.
    #  @param theObject the object from which the list of elements is taken, this is mesh, submesh or group
    #  @param Diag13    is used to choose a diagonal for splitting.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_cutquadr
    def SplitQuadObject (self, theObject, Diag13):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SplitQuadObject(theObject, Diag13)

    ## Finds a better splitting of the given quadrangle.
    #  @param IDOfQuad   the ID of the quadrangle to be splitted.
    #  @param theCriterion  FT_...; a criterion to choose a diagonal for splitting.
    #  @return 1 if 1-3 diagonal is better, 2 if 2-4
    #          diagonal is better, 0 if error occurs.
    #  @ingroup l2_modif_cutquadr
    def BestSplit (self, IDOfQuad, theCriterion):
        return self.editor.BestSplit(IDOfQuad, self.smeshpyD.GetFunctor(theCriterion))

    ## Splits volumic elements into tetrahedrons
    #  @param elemIDs either list of elements or mesh or group or submesh
    #  @param method  flags passing splitting method: Hex_5Tet, Hex_6Tet, Hex_24Tet
    #         Hex_5Tet - split the hexahedron into 5 tetrahedrons, etc
    #  @ingroup l2_modif_cutquadr
    def SplitVolumesIntoTetra(self, elemIDs, method=Hex_5Tet ):
        if isinstance( elemIDs, Mesh ):
            elemIDs = elemIDs.GetMesh()
        if ( isinstance( elemIDs, list )):
            elemIDs = self.editor.MakeIDSource(elemIDs, SMESH.VOLUME)
        self.editor.SplitVolumesIntoTetra(elemIDs, method)

    ## Splits quadrangle faces near triangular facets of volumes
    #
    #  @ingroup l1_auxiliary
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

    ## @brief Splits hexahedrons into tetrahedrons.
    #
    #  This operation uses pattern mapping functionality for splitting.
    #  @param theObject the object from which the list of hexahedrons is taken; this is mesh, submesh or group.
    #  @param theNode000,theNode001 within the range [0,7]; gives the orientation of the
    #         pattern relatively each hexahedron: the (0,0,0) key-point of the pattern
    #         will be mapped into <VAR>theNode000</VAR>-th node of each volume, the (0,0,1)
    #         key-point will be mapped into <VAR>theNode001</VAR>-th node of each volume.
    #         The (0,0,0) key-point of the used pattern corresponds to a non-split corner.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l1_auxiliary
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
    #  Uses the pattern mapping functionality for splitting.
    #  @param theObject the object (mesh, submesh or group) from where the list of hexahedrons is taken;
    #  @param theNode000,theNode001 (within the range [0,7]) gives the orientation of the
    #         pattern relatively each hexahedron: keypoint (0,0,0) of the pattern
    #         will be mapped into the <VAR>theNode000</VAR>-th node of each volume, keypoint (0,0,1)
    #         will be mapped into the <VAR>theNode001</VAR>-th node of each volume.
    #         Edge (0,0,0)-(0,0,1) of used pattern connects two not split corners.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l1_auxiliary
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

        # Splits quafrangle faces near triangular facets of volumes
        self.SplitQuadsNearTriangularFacets()

        return isDone

    ## Smoothes elements
    #  @param IDsOfElements the list if ids of elements to smooth
    #  @param IDsOfFixedNodes the list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations the maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_smooth
    def Smooth(self, IDsOfElements, IDsOfFixedNodes,
               MaxNbOfIterations, MaxAspectRatio, Method):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        MaxNbOfIterations,MaxAspectRatio,Parameters = geompyDC.ParseParameters(MaxNbOfIterations,MaxAspectRatio)
        self.mesh.SetParameters(Parameters)
        return self.editor.Smooth(IDsOfElements, IDsOfFixedNodes,
                                  MaxNbOfIterations, MaxAspectRatio, Method)

    ## Smoothes elements which belong to the given object
    #  @param theObject the object to smooth
    #  @param IDsOfFixedNodes the list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations the maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_smooth
    def SmoothObject(self, theObject, IDsOfFixedNodes,
                     MaxNbOfIterations, MaxAspectRatio, Method):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SmoothObject(theObject, IDsOfFixedNodes,
                                        MaxNbOfIterations, MaxAspectRatio, Method)

    ## Parametrically smoothes the given elements
    #  @param IDsOfElements the list if ids of elements to smooth
    #  @param IDsOfFixedNodes the list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations the maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_smooth
    def SmoothParametric(self, IDsOfElements, IDsOfFixedNodes,
                         MaxNbOfIterations, MaxAspectRatio, Method):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        MaxNbOfIterations,MaxAspectRatio,Parameters = geompyDC.ParseParameters(MaxNbOfIterations,MaxAspectRatio)
        self.mesh.SetParameters(Parameters)
        return self.editor.SmoothParametric(IDsOfElements, IDsOfFixedNodes,
                                            MaxNbOfIterations, MaxAspectRatio, Method)

    ## Parametrically smoothes the elements which belong to the given object
    #  @param theObject the object to smooth
    #  @param IDsOfFixedNodes the list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations the maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method Laplacian(LAPLACIAN_SMOOTH) or Centroidal(CENTROIDAL_SMOOTH)
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_smooth
    def SmoothParametricObject(self, theObject, IDsOfFixedNodes,
                               MaxNbOfIterations, MaxAspectRatio, Method):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SmoothParametricObject(theObject, IDsOfFixedNodes,
                                                  MaxNbOfIterations, MaxAspectRatio, Method)

    ## Converts the mesh to quadratic, deletes old elements, replacing
    #  them with quadratic with the same id.
    #  @param theForce3d new node creation method:
    #         0 - the medium node lies at the geometrical edge from which the mesh element is built
    #         1 - the medium node lies at the middle of the line segments connecting start and end node of a mesh element
    #  @ingroup l2_modif_tofromqu
    def ConvertToQuadratic(self, theForce3d):
        self.editor.ConvertToQuadratic(theForce3d)

    ## Converts the mesh from quadratic to ordinary,
    #  deletes old quadratic elements, \n replacing
    #  them with ordinary mesh elements with the same id.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_tofromqu
    def ConvertFromQuadratic(self):
        return self.editor.ConvertFromQuadratic()

    ## Creates 2D mesh as skin on boundary faces of a 3D mesh
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def  Make2DMeshFrom3D(self):
        return self.editor. Make2DMeshFrom3D()

    ## Creates missing boundary elements
    #  @param elements - elements whose boundary is to be checked:
    #                    mesh, group, sub-mesh or list of elements
    #  @param dimension - defines type of boundary elements to create:
    #                     SMESH.BND_2DFROM3D, SMESH.BND_1DFROM3D, SMESH.BND_1DFROM2D
    #  @param groupName - a name of group to store created boundary elements in,
    #                     "" means not to create the group
    #  @param meshName - a name of new mesh to store created boundary elements in,
    #                     "" means not to create the new mesh
    #  @param toCopyElements - if true, the checked elements will be copied into the new mesh
    #  @param toCopyExistingBondary - if true, not only new but also pre-existing 
    #                                boundary elements will be copied into the new mesh
    #  @return tuple (mesh, group) where bondary elements were added to
    #  @ingroup l2_modif_edit
    def MakeBoundaryMesh(self, elements, dimension=SMESH.BND_2DFROM3D, groupName="", meshName="",
                         toCopyElements=False, toCopyExistingBondary=False):
        if isinstance( elements, Mesh ):
            elements = elements.GetMesh()
        if ( isinstance( elements, list )):
            elemType = SMESH.ALL
            if elements: elemType = self.GetElementType( elements[0], iselem=True)
            elements = self.editor.MakeIDSource(elements, elemType)
        mesh, group = self.editor.MakeBoundaryMesh(elements,dimension,groupName,meshName,
                                                   toCopyElements,toCopyExistingBondary)
        if mesh: mesh = self.smeshpyD.Mesh(mesh)
        return mesh, group

    ## Renumber mesh nodes
    #  @ingroup l2_modif_renumber
    def RenumberNodes(self):
        self.editor.RenumberNodes()

    ## Renumber mesh elements
    #  @ingroup l2_modif_renumber
    def RenumberElements(self):
        self.editor.RenumberElements()

    ## Generates new elements by rotation of the elements around the axis
    #  @param IDsOfElements the list of ids of elements to sweep
    #  @param Axis the axis of rotation, AxisStruct or line(geom object)
    #  @param AngleInRadians the angle of Rotation (in radians) or a name of variable which defines angle in degrees
    #  @param NbOfSteps the number of steps
    #  @param Tolerance tolerance
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param TotalAngle gives meaning of AngleInRadians: if True then it is an angular size
    #                    of all steps, else - size of each step
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def RotationSweep(self, IDsOfElements, Axis, AngleInRadians, NbOfSteps, Tolerance,
                      MakeGroups=False, TotalAngle=False):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,AngleParameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        if TotalAngle and NbOfSteps:
            AngleInRadians /= NbOfSteps
        NbOfSteps,Tolerance,Parameters = geompyDC.ParseParameters(NbOfSteps,Tolerance)
        Parameters = AxisParameters + var_separator + AngleParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.RotationSweepMakeGroups(IDsOfElements, Axis,
                                                       AngleInRadians, NbOfSteps, Tolerance)
        self.editor.RotationSweep(IDsOfElements, Axis, AngleInRadians, NbOfSteps, Tolerance)
        return []

    ## Generates new elements by rotation of the elements of object around the axis
    #  @param theObject object which elements should be sweeped
    #  @param Axis the axis of rotation, AxisStruct or line(geom object)
    #  @param AngleInRadians the angle of Rotation
    #  @param NbOfSteps number of steps
    #  @param Tolerance tolerance
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param TotalAngle gives meaning of AngleInRadians: if True then it is an angular size
    #                    of all steps, else - size of each step
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def RotationSweepObject(self, theObject, Axis, AngleInRadians, NbOfSteps, Tolerance,
                            MakeGroups=False, TotalAngle=False):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,AngleParameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        if TotalAngle and NbOfSteps:
            AngleInRadians /= NbOfSteps
        NbOfSteps,Tolerance,Parameters = geompyDC.ParseParameters(NbOfSteps,Tolerance)
        Parameters = AxisParameters + var_separator + AngleParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.RotationSweepObjectMakeGroups(theObject, Axis, AngleInRadians,
                                                             NbOfSteps, Tolerance)
        self.editor.RotationSweepObject(theObject, Axis, AngleInRadians, NbOfSteps, Tolerance)
        return []

    ## Generates new elements by rotation of the elements of object around the axis
    #  @param theObject object which elements should be sweeped
    #  @param Axis the axis of rotation, AxisStruct or line(geom object)
    #  @param AngleInRadians the angle of Rotation
    #  @param NbOfSteps number of steps
    #  @param Tolerance tolerance
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param TotalAngle gives meaning of AngleInRadians: if True then it is an angular size
    #                    of all steps, else - size of each step
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def RotationSweepObject1D(self, theObject, Axis, AngleInRadians, NbOfSteps, Tolerance,
                              MakeGroups=False, TotalAngle=False):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,AngleParameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        if TotalAngle and NbOfSteps:
            AngleInRadians /= NbOfSteps
        NbOfSteps,Tolerance,Parameters = geompyDC.ParseParameters(NbOfSteps,Tolerance)
        Parameters = AxisParameters + var_separator + AngleParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.RotationSweepObject1DMakeGroups(theObject, Axis, AngleInRadians,
                                                               NbOfSteps, Tolerance)
        self.editor.RotationSweepObject1D(theObject, Axis, AngleInRadians, NbOfSteps, Tolerance)
        return []

    ## Generates new elements by rotation of the elements of object around the axis
    #  @param theObject object which elements should be sweeped
    #  @param Axis the axis of rotation, AxisStruct or line(geom object)
    #  @param AngleInRadians the angle of Rotation
    #  @param NbOfSteps number of steps
    #  @param Tolerance tolerance
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param TotalAngle gives meaning of AngleInRadians: if True then it is an angular size
    #                    of all steps, else - size of each step
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def RotationSweepObject2D(self, theObject, Axis, AngleInRadians, NbOfSteps, Tolerance,
                              MakeGroups=False, TotalAngle=False):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,AngleParameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        if TotalAngle and NbOfSteps:
            AngleInRadians /= NbOfSteps
        NbOfSteps,Tolerance,Parameters = geompyDC.ParseParameters(NbOfSteps,Tolerance)
        Parameters = AxisParameters + var_separator + AngleParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.RotationSweepObject2DMakeGroups(theObject, Axis, AngleInRadians,
                                                             NbOfSteps, Tolerance)
        self.editor.RotationSweepObject2D(theObject, Axis, AngleInRadians, NbOfSteps, Tolerance)
        return []

    ## Generates new elements by extrusion of the elements with given ids
    #  @param IDsOfElements the list of elements ids for extrusion
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweep(self, IDsOfElements, StepVector, NbOfSteps, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        StepVector,StepVectorParameters = ParseDirStruct(StepVector)
        NbOfSteps,Parameters = geompyDC.ParseParameters(NbOfSteps)
        Parameters = StepVectorParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionSweepMakeGroups(IDsOfElements, StepVector, NbOfSteps)
        self.editor.ExtrusionSweep(IDsOfElements, StepVector, NbOfSteps)
        return []

    ## Generates new elements by extrusion of the elements with given ids
    #  @param IDsOfElements is ids of elements
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param ExtrFlags sets flags for extrusion
    #  @param SewTolerance uses for comparing locations of nodes if flag
    #         EXTRUSION_FLAG_SEW is set
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def AdvancedExtrusion(self, IDsOfElements, StepVector, NbOfSteps,
                          ExtrFlags, SewTolerance, MakeGroups=False):
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if MakeGroups:
            return self.editor.AdvancedExtrusionMakeGroups(IDsOfElements, StepVector, NbOfSteps,
                                                           ExtrFlags, SewTolerance)
        self.editor.AdvancedExtrusion(IDsOfElements, StepVector, NbOfSteps,
                                      ExtrFlags, SewTolerance)
        return []

    ## Generates new elements by extrusion of the elements which belong to the object
    #  @param theObject the object which elements should be processed
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweepObject(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        StepVector,StepVectorParameters = ParseDirStruct(StepVector)
        NbOfSteps,Parameters = geompyDC.ParseParameters(NbOfSteps)
        Parameters = StepVectorParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionSweepObjectMakeGroups(theObject, StepVector, NbOfSteps)
        self.editor.ExtrusionSweepObject(theObject, StepVector, NbOfSteps)
        return []

    ## Generates new elements by extrusion of the elements which belong to the object
    #  @param theObject object which elements should be processed
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups to generate new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweepObject1D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        StepVector,StepVectorParameters = ParseDirStruct(StepVector)
        NbOfSteps,Parameters = geompyDC.ParseParameters(NbOfSteps)
        Parameters = StepVectorParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionSweepObject1DMakeGroups(theObject, StepVector, NbOfSteps)
        self.editor.ExtrusionSweepObject1D(theObject, StepVector, NbOfSteps)
        return []

    ## Generates new elements by extrusion of the elements which belong to the object
    #  @param theObject object which elements should be processed
    #  @param StepVector vector, defining the direction and value of extrusion
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweepObject2D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( StepVector, geompyDC.GEOM._objref_GEOM_Object)):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        StepVector,StepVectorParameters = ParseDirStruct(StepVector)
        NbOfSteps,Parameters = geompyDC.ParseParameters(NbOfSteps)
        Parameters = StepVectorParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionSweepObject2DMakeGroups(theObject, StepVector, NbOfSteps)
        self.editor.ExtrusionSweepObject2D(theObject, StepVector, NbOfSteps)
        return []



    ## Generates new elements by extrusion of the given elements
    #  The path of extrusion must be a meshed edge.
    #  @param Base mesh or list of ids of elements for extrusion
    #  @param Path - 1D mesh or 1D sub-mesh, along which proceeds the extrusion
    #  @param NodeStart the start node from Path. Defines the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path
    #                   to get the resulting mesh in a helical fashion
    #  @param Angles list of angles in radians
    #  @param LinearVariation forces the computation of rotation angles as linear
    #                         variation of the given Angles along path steps
    #  @param HasRefPoint allows using the reference point
    #  @param RefPoint the point around which the shape is rotated (the mass center of the shape by default).
    #         The User can specify any point as the Reference Point.
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param ElemType type of elements for extrusion (if param Base is a mesh)
    #  @return list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
    #          only SMESH::Extrusion_Error otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionAlongPathX(self, Base, Path, NodeStart,
                            HasAngles, Angles, LinearVariation,
                            HasRefPoint, RefPoint, MakeGroups, ElemType):
        Angles,AnglesParameters = ParseAngles(Angles)
        RefPoint,RefPointParameters = ParsePointStruct(RefPoint)
        if ( isinstance( RefPoint, geompyDC.GEOM._objref_GEOM_Object)):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
            pass
        Parameters = AnglesParameters + var_separator + RefPointParameters
        self.mesh.SetParameters(Parameters)

        if isinstance(Base,list):
            IDsOfElements = []
            if Base == []: IDsOfElements = self.GetElementsId()
            else: IDsOfElements = Base
            return self.editor.ExtrusionAlongPathX(IDsOfElements, Path, NodeStart,
                                                   HasAngles, Angles, LinearVariation,
                                                   HasRefPoint, RefPoint, MakeGroups, ElemType)
        else:
            if isinstance(Base,Mesh):
                return self.editor.ExtrusionAlongPathObjX(Base, Path, NodeStart,
                                                          HasAngles, Angles, LinearVariation,
                                                          HasRefPoint, RefPoint, MakeGroups, ElemType)
            else:
                raise RuntimeError, "Invalid Base for ExtrusionAlongPathX"


    ## Generates new elements by extrusion of the given elements
    #  The path of extrusion must be a meshed edge.
    #  @param IDsOfElements ids of elements
    #  @param PathMesh mesh containing a 1D sub-mesh on the edge, along which proceeds the extrusion
    #  @param PathShape shape(edge) defines the sub-mesh for the path
    #  @param NodeStart the first or the last node on the edge. Defines the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path
    #                   to get the resulting mesh in a helical fashion
    #  @param Angles list of angles in radians
    #  @param HasRefPoint allows using the reference point
    #  @param RefPoint the point around which the shape is rotated (the mass center of the shape by default).
    #         The User can specify any point as the Reference Point.
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param LinearVariation forces the computation of rotation angles as linear
    #                         variation of the given Angles along path steps
    #  @return list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
    #          only SMESH::Extrusion_Error otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionAlongPath(self, IDsOfElements, PathMesh, PathShape, NodeStart,
                           HasAngles, Angles, HasRefPoint, RefPoint,
                           MakeGroups=False, LinearVariation=False):
        Angles,AnglesParameters = ParseAngles(Angles)
        RefPoint,RefPointParameters = ParsePointStruct(RefPoint)
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( RefPoint, geompyDC.GEOM._objref_GEOM_Object)):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
            pass
        if ( isinstance( PathMesh, Mesh )):
            PathMesh = PathMesh.GetMesh()
        if HasAngles and Angles and LinearVariation:
            Angles = self.editor.LinearAnglesVariation( PathMesh, PathShape, Angles )
            pass
        Parameters = AnglesParameters + var_separator + RefPointParameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionAlongPathMakeGroups(IDsOfElements, PathMesh,
                                                            PathShape, NodeStart, HasAngles,
                                                            Angles, HasRefPoint, RefPoint)
        return self.editor.ExtrusionAlongPath(IDsOfElements, PathMesh, PathShape,
                                              NodeStart, HasAngles, Angles, HasRefPoint, RefPoint)

    ## Generates new elements by extrusion of the elements which belong to the object
    #  The path of extrusion must be a meshed edge.
    #  @param theObject the object which elements should be processed
    #  @param PathMesh mesh containing a 1D sub-mesh on the edge, along which the extrusion proceeds
    #  @param PathShape shape(edge) defines the sub-mesh for the path
    #  @param NodeStart the first or the last node on the edge. Defines the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path
    #                   to get the resulting mesh in a helical fashion
    #  @param Angles list of angles
    #  @param HasRefPoint allows using the reference point
    #  @param RefPoint the point around which the shape is rotated (the mass center of the shape by default).
    #         The User can specify any point as the Reference Point.
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param LinearVariation forces the computation of rotation angles as linear
    #                         variation of the given Angles along path steps
    #  @return list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
    #          only SMESH::Extrusion_Error otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionAlongPathObject(self, theObject, PathMesh, PathShape, NodeStart,
                                 HasAngles, Angles, HasRefPoint, RefPoint,
                                 MakeGroups=False, LinearVariation=False):
        Angles,AnglesParameters = ParseAngles(Angles)
        RefPoint,RefPointParameters = ParsePointStruct(RefPoint)
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( RefPoint, geompyDC.GEOM._objref_GEOM_Object)):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
        if ( isinstance( PathMesh, Mesh )):
            PathMesh = PathMesh.GetMesh()
        if HasAngles and Angles and LinearVariation:
            Angles = self.editor.LinearAnglesVariation( PathMesh, PathShape, Angles )
            pass
        Parameters = AnglesParameters + var_separator + RefPointParameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionAlongPathObjectMakeGroups(theObject, PathMesh,
                                                                  PathShape, NodeStart, HasAngles,
                                                                  Angles, HasRefPoint, RefPoint)
        return self.editor.ExtrusionAlongPathObject(theObject, PathMesh, PathShape,
                                                    NodeStart, HasAngles, Angles, HasRefPoint,
                                                    RefPoint)

    ## Generates new elements by extrusion of the elements which belong to the object
    #  The path of extrusion must be a meshed edge.
    #  @param theObject the object which elements should be processed
    #  @param PathMesh mesh containing a 1D sub-mesh on the edge, along which the extrusion proceeds
    #  @param PathShape shape(edge) defines the sub-mesh for the path
    #  @param NodeStart the first or the last node on the edge. Defines the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path
    #                   to get the resulting mesh in a helical fashion
    #  @param Angles list of angles
    #  @param HasRefPoint allows using the reference point
    #  @param RefPoint the point around which the shape is rotated (the mass center of the shape by default).
    #         The User can specify any point as the Reference Point.
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param LinearVariation forces the computation of rotation angles as linear
    #                         variation of the given Angles along path steps
    #  @return list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
    #          only SMESH::Extrusion_Error otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionAlongPathObject1D(self, theObject, PathMesh, PathShape, NodeStart,
                                   HasAngles, Angles, HasRefPoint, RefPoint,
                                   MakeGroups=False, LinearVariation=False):
        Angles,AnglesParameters = ParseAngles(Angles)
        RefPoint,RefPointParameters = ParsePointStruct(RefPoint)
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( RefPoint, geompyDC.GEOM._objref_GEOM_Object)):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
        if ( isinstance( PathMesh, Mesh )):
            PathMesh = PathMesh.GetMesh()
        if HasAngles and Angles and LinearVariation:
            Angles = self.editor.LinearAnglesVariation( PathMesh, PathShape, Angles )
            pass
        Parameters = AnglesParameters + var_separator + RefPointParameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionAlongPathObject1DMakeGroups(theObject, PathMesh,
                                                                    PathShape, NodeStart, HasAngles,
                                                                    Angles, HasRefPoint, RefPoint)
        return self.editor.ExtrusionAlongPathObject1D(theObject, PathMesh, PathShape,
                                                      NodeStart, HasAngles, Angles, HasRefPoint,
                                                      RefPoint)

    ## Generates new elements by extrusion of the elements which belong to the object
    #  The path of extrusion must be a meshed edge.
    #  @param theObject the object which elements should be processed
    #  @param PathMesh mesh containing a 1D sub-mesh on the edge, along which the extrusion proceeds
    #  @param PathShape shape(edge) defines the sub-mesh for the path
    #  @param NodeStart the first or the last node on the edge. Defines the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path
    #                   to get the resulting mesh in a helical fashion
    #  @param Angles list of angles
    #  @param HasRefPoint allows using the reference point
    #  @param RefPoint the point around which the shape is rotated (the mass center of the shape by default).
    #         The User can specify any point as the Reference Point.
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param LinearVariation forces the computation of rotation angles as linear
    #                         variation of the given Angles along path steps
    #  @return list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
    #          only SMESH::Extrusion_Error otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionAlongPathObject2D(self, theObject, PathMesh, PathShape, NodeStart,
                                   HasAngles, Angles, HasRefPoint, RefPoint,
                                   MakeGroups=False, LinearVariation=False):
        Angles,AnglesParameters = ParseAngles(Angles)
        RefPoint,RefPointParameters = ParsePointStruct(RefPoint)
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( RefPoint, geompyDC.GEOM._objref_GEOM_Object)):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
        if ( isinstance( PathMesh, Mesh )):
            PathMesh = PathMesh.GetMesh()
        if HasAngles and Angles and LinearVariation:
            Angles = self.editor.LinearAnglesVariation( PathMesh, PathShape, Angles )
            pass
        Parameters = AnglesParameters + var_separator + RefPointParameters
        self.mesh.SetParameters(Parameters)
        if MakeGroups:
            return self.editor.ExtrusionAlongPathObject2DMakeGroups(theObject, PathMesh,
                                                                    PathShape, NodeStart, HasAngles,
                                                                    Angles, HasRefPoint, RefPoint)
        return self.editor.ExtrusionAlongPathObject2D(theObject, PathMesh, PathShape,
                                                      NodeStart, HasAngles, Angles, HasRefPoint,
                                                      RefPoint)

    ## Creates a symmetrical copy of mesh elements
    #  @param IDsOfElements list of elements ids
    #  @param Mirror is AxisStruct or geom object(point, line, plane)
    #  @param theMirrorType is  POINT, AXIS or PLANE
    #  If the Mirror is a geom object this parameter is unnecessary
    #  @param Copy allows to copy element (Copy is 1) or to replace with its mirroring (Copy is 0)
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def Mirror(self, IDsOfElements, Mirror, theMirrorType, Copy=0, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Mirror, geompyDC.GEOM._objref_GEOM_Object)):
            Mirror = self.smeshpyD.GetAxisStruct(Mirror)
        Mirror,Parameters = ParseAxisStruct(Mirror)
        self.mesh.SetParameters(Parameters)
        if Copy and MakeGroups:
            return self.editor.MirrorMakeGroups(IDsOfElements, Mirror, theMirrorType)
        self.editor.Mirror(IDsOfElements, Mirror, theMirrorType, Copy)
        return []

    ## Creates a new mesh by a symmetrical copy of mesh elements
    #  @param IDsOfElements the list of elements ids
    #  @param Mirror is AxisStruct or geom object (point, line, plane)
    #  @param theMirrorType is  POINT, AXIS or PLANE
    #  If the Mirror is a geom object this parameter is unnecessary
    #  @param MakeGroups to generate new groups from existing ones
    #  @param NewMeshName a name of the new mesh to create
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def MirrorMakeMesh(self, IDsOfElements, Mirror, theMirrorType, MakeGroups=0, NewMeshName=""):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Mirror, geompyDC.GEOM._objref_GEOM_Object)):
            Mirror = self.smeshpyD.GetAxisStruct(Mirror)
        Mirror,Parameters = ParseAxisStruct(Mirror)
        mesh = self.editor.MirrorMakeMesh(IDsOfElements, Mirror, theMirrorType,
                                          MakeGroups, NewMeshName)
        mesh.SetParameters(Parameters)
        return Mesh(self.smeshpyD,self.geompyD,mesh)

    ## Creates a symmetrical copy of the object
    #  @param theObject mesh, submesh or group
    #  @param Mirror AxisStruct or geom object (point, line, plane)
    #  @param theMirrorType is  POINT, AXIS or PLANE
    #  If the Mirror is a geom object this parameter is unnecessary
    #  @param Copy allows copying the element (Copy is 1) or replacing it with its mirror (Copy is 0)
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def MirrorObject (self, theObject, Mirror, theMirrorType, Copy=0, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Mirror, geompyDC.GEOM._objref_GEOM_Object)):
            Mirror = self.smeshpyD.GetAxisStruct(Mirror)
        Mirror,Parameters = ParseAxisStruct(Mirror)
        self.mesh.SetParameters(Parameters)
        if Copy and MakeGroups:
            return self.editor.MirrorObjectMakeGroups(theObject, Mirror, theMirrorType)
        self.editor.MirrorObject(theObject, Mirror, theMirrorType, Copy)
        return []

    ## Creates a new mesh by a symmetrical copy of the object
    #  @param theObject mesh, submesh or group
    #  @param Mirror AxisStruct or geom object (point, line, plane)
    #  @param theMirrorType POINT, AXIS or PLANE
    #  If the Mirror is a geom object this parameter is unnecessary
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the new mesh to create
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def MirrorObjectMakeMesh (self, theObject, Mirror, theMirrorType,MakeGroups=0, NewMeshName=""):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if (isinstance(Mirror, geompyDC.GEOM._objref_GEOM_Object)):
            Mirror = self.smeshpyD.GetAxisStruct(Mirror)
        Mirror,Parameters = ParseAxisStruct(Mirror)
        mesh = self.editor.MirrorObjectMakeMesh(theObject, Mirror, theMirrorType,
                                                MakeGroups, NewMeshName)
        mesh.SetParameters(Parameters)
        return Mesh( self.smeshpyD,self.geompyD,mesh )

    ## Translates the elements
    #  @param IDsOfElements list of elements ids
    #  @param Vector the direction of translation (DirStruct or vector)
    #  @param Copy allows copying the translated elements
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def Translate(self, IDsOfElements, Vector, Copy, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Vector, geompyDC.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        Vector,Parameters = ParseDirStruct(Vector)
        self.mesh.SetParameters(Parameters)
        if Copy and MakeGroups:
            return self.editor.TranslateMakeGroups(IDsOfElements, Vector)
        self.editor.Translate(IDsOfElements, Vector, Copy)
        return []

    ## Creates a new mesh of translated elements
    #  @param IDsOfElements list of elements ids
    #  @param Vector the direction of translation (DirStruct or vector)
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the newly created mesh
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def TranslateMakeMesh(self, IDsOfElements, Vector, MakeGroups=False, NewMeshName=""):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Vector, geompyDC.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        Vector,Parameters = ParseDirStruct(Vector)
        mesh = self.editor.TranslateMakeMesh(IDsOfElements, Vector, MakeGroups, NewMeshName)
        mesh.SetParameters(Parameters)
        return Mesh ( self.smeshpyD, self.geompyD, mesh )

    ## Translates the object
    #  @param theObject the object to translate (mesh, submesh, or group)
    #  @param Vector direction of translation (DirStruct or geom vector)
    #  @param Copy allows copying the translated elements
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def TranslateObject(self, theObject, Vector, Copy, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Vector, geompyDC.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        Vector,Parameters = ParseDirStruct(Vector)
        self.mesh.SetParameters(Parameters)
        if Copy and MakeGroups:
            return self.editor.TranslateObjectMakeGroups(theObject, Vector)
        self.editor.TranslateObject(theObject, Vector, Copy)
        return []

    ## Creates a new mesh from the translated object
    #  @param theObject the object to translate (mesh, submesh, or group)
    #  @param Vector the direction of translation (DirStruct or geom vector)
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the newly created mesh
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def TranslateObjectMakeMesh(self, theObject, Vector, MakeGroups=False, NewMeshName=""):
        if (isinstance(theObject, Mesh)):
            theObject = theObject.GetMesh()
        if (isinstance(Vector, geompyDC.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        Vector,Parameters = ParseDirStruct(Vector)
        mesh = self.editor.TranslateObjectMakeMesh(theObject, Vector, MakeGroups, NewMeshName)
        mesh.SetParameters(Parameters)
        return Mesh( self.smeshpyD, self.geompyD, mesh )



    ## Scales the object
    #  @param theObject - the object to translate (mesh, submesh, or group)
    #  @param thePoint - base point for scale
    #  @param theScaleFact - list of 1-3 scale factors for axises
    #  @param Copy - allows copying the translated elements
    #  @param MakeGroups - forces the generation of new groups from existing
    #                      ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True,
    #          empty list otherwise
    def Scale(self, theObject, thePoint, theScaleFact, Copy, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( theObject, list )):
            theObject = self.editor.MakeIDSource(theObject, SMESH.ALL)

        thePoint, Parameters = ParsePointStruct(thePoint)
        self.mesh.SetParameters(Parameters)

        if Copy and MakeGroups:
            return self.editor.ScaleMakeGroups(theObject, thePoint, theScaleFact)
        self.editor.Scale(theObject, thePoint, theScaleFact, Copy)
        return []

    ## Creates a new mesh from the translated object
    #  @param theObject - the object to translate (mesh, submesh, or group)
    #  @param thePoint - base point for scale
    #  @param theScaleFact - list of 1-3 scale factors for axises
    #  @param MakeGroups - forces the generation of new groups from existing ones
    #  @param NewMeshName - the name of the newly created mesh
    #  @return instance of Mesh class
    def ScaleMakeMesh(self, theObject, thePoint, theScaleFact, MakeGroups=False, NewMeshName=""):
        if (isinstance(theObject, Mesh)):
            theObject = theObject.GetMesh()
        if ( isinstance( theObject, list )):
            theObject = self.editor.MakeIDSource(theObject,SMESH.ALL)

        mesh = self.editor.ScaleMakeMesh(theObject, thePoint, theScaleFact,
                                         MakeGroups, NewMeshName)
        #mesh.SetParameters(Parameters)
        return Mesh( self.smeshpyD, self.geompyD, mesh )



    ## Rotates the elements
    #  @param IDsOfElements list of elements ids
    #  @param Axis the axis of rotation (AxisStruct or geom line)
    #  @param AngleInRadians the angle of rotation (in radians) or a name of variable which defines angle in degrees
    #  @param Copy allows copying the rotated elements
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def Rotate (self, IDsOfElements, Axis, AngleInRadians, Copy, MakeGroups=False):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,Parameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        Parameters = AxisParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if Copy and MakeGroups:
            return self.editor.RotateMakeGroups(IDsOfElements, Axis, AngleInRadians)
        self.editor.Rotate(IDsOfElements, Axis, AngleInRadians, Copy)
        return []

    ## Creates a new mesh of rotated elements
    #  @param IDsOfElements list of element ids
    #  @param Axis the axis of rotation (AxisStruct or geom line)
    #  @param AngleInRadians the angle of rotation (in radians) or a name of variable which defines angle in degrees
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the newly created mesh
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def RotateMakeMesh (self, IDsOfElements, Axis, AngleInRadians, MakeGroups=0, NewMeshName=""):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,Parameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        Parameters = AxisParameters + var_separator + Parameters
        mesh = self.editor.RotateMakeMesh(IDsOfElements, Axis, AngleInRadians,
                                          MakeGroups, NewMeshName)
        mesh.SetParameters(Parameters)
        return Mesh( self.smeshpyD, self.geompyD, mesh )

    ## Rotates the object
    #  @param theObject the object to rotate( mesh, submesh, or group)
    #  @param Axis the axis of rotation (AxisStruct or geom line)
    #  @param AngleInRadians the angle of rotation (in radians) or a name of variable which defines angle in degrees
    #  @param Copy allows copying the rotated elements
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def RotateObject (self, theObject, Axis, AngleInRadians, Copy, MakeGroups=False):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,Parameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if (isinstance(theObject, Mesh)):
            theObject = theObject.GetMesh()
        if (isinstance(Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        Parameters = AxisParameters + ":" + Parameters
        self.mesh.SetParameters(Parameters)
        if Copy and MakeGroups:
            return self.editor.RotateObjectMakeGroups(theObject, Axis, AngleInRadians)
        self.editor.RotateObject(theObject, Axis, AngleInRadians, Copy)
        return []

    ## Creates a new mesh from the rotated object
    #  @param theObject the object to rotate (mesh, submesh, or group)
    #  @param Axis the axis of rotation (AxisStruct or geom line)
    #  @param AngleInRadians the angle of rotation (in radians)  or a name of variable which defines angle in degrees
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the newly created mesh
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def RotateObjectMakeMesh(self, theObject, Axis, AngleInRadians, MakeGroups=0,NewMeshName=""):
        flag = False
        if isinstance(AngleInRadians,str):
            flag = True
        AngleInRadians,Parameters = geompyDC.ParseParameters(AngleInRadians)
        if flag:
            AngleInRadians = DegreesToRadians(AngleInRadians)
        if (isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if (isinstance(Axis, geompyDC.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        Axis,AxisParameters = ParseAxisStruct(Axis)
        Parameters = AxisParameters + ":" + Parameters
        mesh = self.editor.RotateObjectMakeMesh(theObject, Axis, AngleInRadians,
                                                       MakeGroups, NewMeshName)
        mesh.SetParameters(Parameters)
        return Mesh( self.smeshpyD, self.geompyD, mesh )

    ## Finds groups of ajacent nodes within Tolerance.
    #  @param Tolerance the value of tolerance
    #  @return the list of groups of nodes
    #  @ingroup l2_modif_trsf
    def FindCoincidentNodes (self, Tolerance):
        return self.editor.FindCoincidentNodes(Tolerance)

    ## Finds groups of ajacent nodes within Tolerance.
    #  @param Tolerance the value of tolerance
    #  @param SubMeshOrGroup SubMesh or Group
    #  @param exceptNodes list of either SubMeshes, Groups or node IDs to exclude from search
    #  @return the list of groups of nodes
    #  @ingroup l2_modif_trsf
    def FindCoincidentNodesOnPart (self, SubMeshOrGroup, Tolerance, exceptNodes=[]):
        if (isinstance( SubMeshOrGroup, Mesh )):
            SubMeshOrGroup = SubMeshOrGroup.GetMesh()
        if not isinstance( exceptNodes, list):
            exceptNodes = [ exceptNodes ]
        if exceptNodes and isinstance( exceptNodes[0], int):
            exceptNodes = [ self.editor.MakeIDSource( exceptNodes, SMESH.NODE)]
        return self.editor.FindCoincidentNodesOnPartBut(SubMeshOrGroup, Tolerance,exceptNodes)

    ## Merges nodes
    #  @param GroupsOfNodes the list of groups of nodes
    #  @ingroup l2_modif_trsf
    def MergeNodes (self, GroupsOfNodes):
        self.editor.MergeNodes(GroupsOfNodes)

    ## Finds the elements built on the same nodes.
    #  @param MeshOrSubMeshOrGroup Mesh or SubMesh, or Group of elements for searching
    #  @return a list of groups of equal elements
    #  @ingroup l2_modif_trsf
    def FindEqualElements (self, MeshOrSubMeshOrGroup):
        if ( isinstance( MeshOrSubMeshOrGroup, Mesh )):
            MeshOrSubMeshOrGroup = MeshOrSubMeshOrGroup.GetMesh()
        return self.editor.FindEqualElements(MeshOrSubMeshOrGroup)

    ## Merges elements in each given group.
    #  @param GroupsOfElementsID groups of elements for merging
    #  @ingroup l2_modif_trsf
    def MergeElements(self, GroupsOfElementsID):
        self.editor.MergeElements(GroupsOfElementsID)

    ## Leaves one element and removes all other elements built on the same nodes.
    #  @ingroup l2_modif_trsf
    def MergeEqualElements(self):
        self.editor.MergeEqualElements()

    ## Sews free borders
    #  @return SMESH::Sew_Error
    #  @ingroup l2_modif_trsf
    def SewFreeBorders (self, FirstNodeID1, SecondNodeID1, LastNodeID1,
                        FirstNodeID2, SecondNodeID2, LastNodeID2,
                        CreatePolygons, CreatePolyedrs):
        return self.editor.SewFreeBorders(FirstNodeID1, SecondNodeID1, LastNodeID1,
                                          FirstNodeID2, SecondNodeID2, LastNodeID2,
                                          CreatePolygons, CreatePolyedrs)

    ## Sews conform free borders
    #  @return SMESH::Sew_Error
    #  @ingroup l2_modif_trsf
    def SewConformFreeBorders (self, FirstNodeID1, SecondNodeID1, LastNodeID1,
                               FirstNodeID2, SecondNodeID2):
        return self.editor.SewConformFreeBorders(FirstNodeID1, SecondNodeID1, LastNodeID1,
                                                 FirstNodeID2, SecondNodeID2)

    ## Sews border to side
    #  @return SMESH::Sew_Error
    #  @ingroup l2_modif_trsf
    def SewBorderToSide (self, FirstNodeIDOnFreeBorder, SecondNodeIDOnFreeBorder, LastNodeIDOnFreeBorder,
                         FirstNodeIDOnSide, LastNodeIDOnSide, CreatePolygons, CreatePolyedrs):
        return self.editor.SewBorderToSide(FirstNodeIDOnFreeBorder, SecondNodeIDOnFreeBorder, LastNodeIDOnFreeBorder,
                                           FirstNodeIDOnSide, LastNodeIDOnSide, CreatePolygons, CreatePolyedrs)

    ## Sews two sides of a mesh. The nodes belonging to Side1 are
    #  merged with the nodes of elements of Side2.
    #  The number of elements in theSide1 and in theSide2 must be
    #  equal and they should have similar nodal connectivity.
    #  The nodes to merge should belong to side borders and
    #  the first node should be linked to the second.
    #  @return SMESH::Sew_Error
    #  @ingroup l2_modif_trsf
    def SewSideElements (self, IDsOfSide1Elements, IDsOfSide2Elements,
                         NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                         NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge):
        return self.editor.SewSideElements(IDsOfSide1Elements, IDsOfSide2Elements,
                                           NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                                           NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge)

    ## Sets new nodes for the given element.
    #  @param ide the element id
    #  @param newIDs nodes ids
    #  @return If the number of nodes does not correspond to the type of element - returns false
    #  @ingroup l2_modif_edit
    def ChangeElemNodes(self, ide, newIDs):
        return self.editor.ChangeElemNodes(ide, newIDs)

    ## If during the last operation of MeshEditor some nodes were
    #  created, this method returns the list of their IDs, \n
    #  if new nodes were not created - returns empty list
    #  @return the list of integer values (can be empty)
    #  @ingroup l1_auxiliary
    def GetLastCreatedNodes(self):
        return self.editor.GetLastCreatedNodes()

    ## If during the last operation of MeshEditor some elements were
    #  created this method returns the list of their IDs, \n
    #  if new elements were not created - returns empty list
    #  @return the list of integer values (can be empty)
    #  @ingroup l1_auxiliary
    def GetLastCreatedElems(self):
        return self.editor.GetLastCreatedElems()

     ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  @param theNodes identifiers of nodes to be doubled
    #  @param theModifiedElems identifiers of elements to be updated by the new (doubled) 
    #         nodes. If list of element identifiers is empty then nodes are doubled but 
    #         they not assigned to elements
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodes(self, theNodes, theModifiedElems):
        return self.editor.DoubleNodes(theNodes, theModifiedElems)
        
    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  This method provided for convenience works as DoubleNodes() described above.
    #  @param theNodeId identifiers of node to be doubled
    #  @param theModifiedElems identifiers of elements to be updated
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNode(self, theNodeId, theModifiedElems):
        return self.editor.DoubleNode(theNodeId, theModifiedElems)
        
    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  This method provided for convenience works as DoubleNodes() described above.
    #  @param theNodes group of nodes to be doubled
    #  @param theModifiedElems group of elements to be updated.
    #  @param theMakeGroup forces the generation of a group containing new nodes.
    #  @return TRUE or a created group if operation has been completed successfully,
    #          FALSE or None otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeGroup(self, theNodes, theModifiedElems, theMakeGroup=False):
        if theMakeGroup:
            return self.editor.DoubleNodeGroupNew(theNodes, theModifiedElems)
        return self.editor.DoubleNodeGroup(theNodes, theModifiedElems)

    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  This method provided for convenience works as DoubleNodes() described above.
    #  @param theNodes list of groups of nodes to be doubled
    #  @param theModifiedElems list of groups of elements to be updated.
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeGroups(self, theNodes, theModifiedElems):
        return self.editor.DoubleNodeGroups(theNodes, theModifiedElems)
    
    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  @param theElems - the list of elements (edges or faces) to be replicated
    #         The nodes for duplication could be found from these elements
    #  @param theNodesNot - list of nodes to NOT replicate
    #  @param theAffectedElems - the list of elements (cells and edges) to which the 
    #         replicated nodes should be associated to.
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeElem(self, theElems, theNodesNot, theAffectedElems):
        return self.editor.DoubleNodeElem(theElems, theNodesNot, theAffectedElems)
        
    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  @param theElems - the list of elements (edges or faces) to be replicated
    #         The nodes for duplication could be found from these elements
    #  @param theNodesNot - list of nodes to NOT replicate
    #  @param theShape - shape to detect affected elements (element which geometric center
    #         located on or inside shape).
    #         The replicated nodes should be associated to affected elements.
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeElemInRegion(self, theElems, theNodesNot, theShape):
        return self.editor.DoubleNodeElemInRegion(theElems, theNodesNot, theShape)
    
    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  This method provided for convenience works as DoubleNodes() described above.
    #  @param theElems - group of of elements (edges or faces) to be replicated
    #  @param theNodesNot - group of nodes not to replicated
    #  @param theAffectedElems - group of elements to which the replicated nodes
    #         should be associated to.
    #  @param theMakeGroup forces the generation of a group containing new elements.
    #  @ingroup l2_modif_edit
    def DoubleNodeElemGroup(self, theElems, theNodesNot, theAffectedElems, theMakeGroup=False):
        if theMakeGroup:
            return self.editor.DoubleNodeElemGroupNew(theElems, theNodesNot, theAffectedElems)
        return self.editor.DoubleNodeElemGroup(theElems, theNodesNot, theAffectedElems)

    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  This method provided for convenience works as DoubleNodes() described above.
    #  @param theElems - group of of elements (edges or faces) to be replicated
    #  @param theNodesNot - group of nodes not to replicated
    #  @param theShape - shape to detect affected elements (element which geometric center
    #         located on or inside shape).
    #         The replicated nodes should be associated to affected elements.
    #  @ingroup l2_modif_edit
    def DoubleNodeElemGroupInRegion(self, theElems, theNodesNot, theShape):
        return self.editor.DoubleNodeElemGroupInRegion(theElems, theNodesNot, theShape)
        
    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  This method provided for convenience works as DoubleNodes() described above.
    #  @param theElems - list of groups of elements (edges or faces) to be replicated
    #  @param theNodesNot - list of groups of nodes not to replicated
    #  @param theAffectedElems - group of elements to which the replicated nodes
    #         should be associated to.
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeElemGroups(self, theElems, theNodesNot, theAffectedElems):
        return self.editor.DoubleNodeElemGroups(theElems, theNodesNot, theAffectedElems)

    ## Creates a hole in a mesh by doubling the nodes of some particular elements
    #  This method provided for convenience works as DoubleNodes() described above.
    #  @param theElems - list of groups of elements (edges or faces) to be replicated
    #  @param theNodesNot - list of groups of nodes not to replicated
    #  @param theShape - shape to detect affected elements (element which geometric center
    #         located on or inside shape).
    #         The replicated nodes should be associated to affected elements.
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeElemGroupsInRegion(self, theElems, theNodesNot, theShape):
        return self.editor.DoubleNodeElemGroupsInRegion(theElems, theNodesNot, theShape)
        
    ## Double nodes on shared faces between groups of volumes and create flat elements on demand.
    # The list of groups must describe a partition of the mesh volumes.
    # The nodes of the internal faces at the boundaries of the groups are doubled.
    # In option, the internal faces are replaced by flat elements.
    # Triangles are transformed in prisms, and quadrangles in hexahedrons.
    # @param theDomains - list of groups of volumes
    # @param createJointElems - if TRUE, create the elements
    # @return TRUE if operation has been completed successfully, FALSE otherwise
    def DoubleNodesOnGroupBoundaries(self, theDomains, createJointElems ):
       return self.editor.DoubleNodesOnGroupBoundaries( theDomains, createJointElems )

    def _valueFromFunctor(self, funcType, elemId):
        fn = self.smeshpyD.GetFunctor(funcType)
        fn.SetMesh(self.mesh)
        if fn.GetElementType() == self.GetElementType(elemId, True):
            val = fn.GetValue(elemId)
        else:
            val = 0
        return val
        
    ## Get length of 1D element.
    #  @param elemId mesh element ID
    #  @return element's length value
    #  @ingroup l1_measurements
    def GetLength(self, elemId):
        return self._valueFromFunctor(SMESH.FT_Length, elemId)    

    ## Get area of 2D element.
    #  @param elemId mesh element ID
    #  @return element's area value
    #  @ingroup l1_measurements
    def GetArea(self, elemId):
        return self._valueFromFunctor(SMESH.FT_Area, elemId)    

    ## Get volume of 3D element.
    #  @param elemId mesh element ID
    #  @return element's volume value
    #  @ingroup l1_measurements
    def GetVolume(self, elemId):
        return self._valueFromFunctor(SMESH.FT_Volume3D, elemId)    

    ## Get maximum element length.
    #  @param elemId mesh element ID
    #  @return element's maximum length value
    #  @ingroup l1_measurements
    def GetMaxElementLength(self, elemId):
        if self.GetElementType(elemId, True) == SMESH.VOLUME:
            ftype = SMESH.FT_MaxElementLength3D
        else:
            ftype = SMESH.FT_MaxElementLength2D
        return self._valueFromFunctor(ftype, elemId)    

    ## Get aspect ratio of 2D or 3D element.
    #  @param elemId mesh element ID
    #  @return element's aspect ratio value
    #  @ingroup l1_measurements
    def GetAspectRatio(self, elemId):
        if self.GetElementType(elemId, True) == SMESH.VOLUME:
            ftype = SMESH.FT_AspectRatio3D
        else:
            ftype = SMESH.FT_AspectRatio
        return self._valueFromFunctor(ftype, elemId)    

    ## Get warping angle of 2D element.
    #  @param elemId mesh element ID
    #  @return element's warping angle value
    #  @ingroup l1_measurements
    def GetWarping(self, elemId):
        return self._valueFromFunctor(SMESH.FT_Warping, elemId)

    ## Get minimum angle of 2D element.
    #  @param elemId mesh element ID
    #  @return element's minimum angle value
    #  @ingroup l1_measurements
    def GetMinimumAngle(self, elemId):
        return self._valueFromFunctor(SMESH.FT_MinimumAngle, elemId)

    ## Get taper of 2D element.
    #  @param elemId mesh element ID
    #  @return element's taper value
    #  @ingroup l1_measurements
    def GetTaper(self, elemId):
        return self._valueFromFunctor(SMESH.FT_Taper, elemId)

    ## Get skew of 2D element.
    #  @param elemId mesh element ID
    #  @return element's skew value
    #  @ingroup l1_measurements
    def GetSkew(self, elemId):
        return self._valueFromFunctor(SMESH.FT_Skew, elemId)

## The mother class to define algorithm, it is not recommended to use it directly.
#
#  More details.
#  @ingroup l2_algorithms
class Mesh_Algorithm:
    #  @class Mesh_Algorithm
    #  @brief Class Mesh_Algorithm

    #def __init__(self,smesh):
    #    self.smesh=smesh
    def __init__(self):
        self.mesh = None
        self.geom = None
        self.subm = None
        self.algo = None

    ## Finds a hypothesis in the study by its type name and parameters.
    #  Finds only the hypotheses created in smeshpyD engine.
    #  @return SMESH.SMESH_Hypothesis
    def FindHypothesis (self, hypname, args, CompareMethod, smeshpyD):
        study = smeshpyD.GetCurrentStudy()
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
        GetName(self.algo)

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
        if geom is None:
            raise RuntimeError, "Attemp to create " + algo + " algoritm on None shape"
        self.mesh = mesh
        piece = mesh.geom
        name = ""
        if not geom:
            self.geom = piece
        else:
            self.geom = geom
            try:
                name = GetName(geom)
                pass
            except:
                name = mesh.geompyD.SubShapeName(geom, piece)
                if not name:
                    name = "%s_%s"%(geom.GetShapeType(), id(geom%1000))
                pass
            self.subm = mesh.mesh.GetSubMesh(geom, algo.GetName())

        self.algo = algo
        status = mesh.mesh.AddHypothesis(self.geom, self.algo)
        TreatHypoStatus( status, algo.GetName(), name, True )

    def CompareHyp (self, hyp, args):
        print "CompareHyp is not implemented for ", self.__class__.__name__, ":", hyp.GetName()
        return False

    def CompareEqualHyp (self, hyp, args):
        return True

    ## Private method
    def Hypothesis (self, hyp, args=[], so="libStdMeshersEngine.so",
                    UseExisting=0, CompareMethod=""):
        hypo = None
        if UseExisting:
            if CompareMethod == "": CompareMethod = self.CompareHyp
            hypo = self.FindHypothesis(hyp, args, CompareMethod, self.mesh.smeshpyD)
            pass
        if hypo is None:
            hypo = self.mesh.smeshpyD.CreateHypothesis(hyp, so)
            a = ""
            s = "="
            i = 0
            n = len(args)
            while i<n:
                a = a + s + str(args[i])
                s = ","
                i = i + 1
                pass
            self.mesh.smeshpyD.SetName(hypo, hyp + a)
            pass
        status = self.mesh.mesh.AddHypothesis(self.geom, hypo)
        TreatHypoStatus( status, GetName(hypo), GetName(self.geom), 0 )
        return hypo

    ## Returns entry of the shape to mesh in the study
    def MainShapeEntry(self):
        entry = ""
        if not self.mesh or not self.mesh.GetMesh(): return entry
        if not self.mesh.GetMesh().HasShapeToMesh(): return entry
        study = self.mesh.smeshpyD.GetCurrentStudy()
        ior  = salome.orb.object_to_string( self.mesh.GetShape() )
        sobj = study.FindObjectIOR(ior)
        if sobj: entry = sobj.GetID()
        if not entry: return ""
        return entry

# Public class: Mesh_Segment
# --------------------------

## Class to define a segment 1D algorithm for discretization
#
#  More details.
#  @ingroup l3_algos_basic
class Mesh_Segment(Mesh_Algorithm):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "Regular_1D")

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
        hyp = self.Hypothesis("LocalLength", [l,p], UseExisting=UseExisting,
                              CompareMethod=self.CompareLocalLength)
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    ## Private method
    ## Checks if the given "LocalLength" hypothesis has the same parameters as the given arguments
    def CompareLocalLength(self, hyp, args):
        if IsEqual(hyp.GetLength(), args[0]):
            return IsEqual(hyp.GetPrecision(), args[1])
        return False

    ## Defines "MaxSize" hypothesis to cut an edge into segments not longer than given value
    #  @param length is optional maximal allowed length of segment, if it is omitted
    #                the preestimated length is used that depends on geometry size
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - create a new one
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
    #  @param reversedEdges is a list of edges to mesh using reversed orientation
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - create a new one
    #  @return an instance of StdMeshers_NumberOfSegments hypothesis
    #  @ingroup l3_hypos_1dhyps
    def NumberOfSegments(self, n, s=[], reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        entry = self.MainShapeEntry()
        if reversedEdges and isinstance(reversedEdges[0],geompyDC.GEOM._objref_GEOM_Object):
            reversedEdges = [ self.mesh.geompyD.GetSubShapeID(self.mesh.geom, e) for e in reversedEdges ]
        if s == []:
            hyp = self.Hypothesis("NumberOfSegments", [n, reversedEdges, entry],
                                  UseExisting=UseExisting,
                                  CompareMethod=self.CompareNumberOfSegments)
        else:
            hyp = self.Hypothesis("NumberOfSegments", [n,s, reversedEdges, entry],
                                  UseExisting=UseExisting,
                                  CompareMethod=self.CompareNumberOfSegments)
            hyp.SetDistrType( 1 )
            hyp.SetScaleFactor(s)
        hyp.SetNumberOfSegments(n)
        hyp.SetReversedEdges( reversedEdges )
        hyp.SetObjectEntry( entry )
        return hyp

    ## Private method
    ## Checks if the given "NumberOfSegments" hypothesis has the same parameters as the given arguments
    def CompareNumberOfSegments(self, hyp, args):
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
    #  @param reversedEdges is a list of edges to mesh using reversed orientation
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @return an instance of StdMeshers_Arithmetic1D hypothesis
    #  @ingroup l3_hypos_1dhyps
    def Arithmetic1D(self, start, end, reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        if reversedEdges and isinstance(reversedEdges[0],geompyDC.GEOM._objref_GEOM_Object):
            reversedEdges = [ self.mesh.geompyD.GetSubShapeID(self.mesh.geom, e) for e in reversedEdges ]
        entry = self.MainShapeEntry()
        hyp = self.Hypothesis("Arithmetic1D", [start, end, reversedEdges, entry],
                              UseExisting=UseExisting,
                              CompareMethod=self.CompareArithmetic1D)
        hyp.SetStartLength(start)
        hyp.SetEndLength(end)
        hyp.SetReversedEdges( reversedEdges )
        hyp.SetObjectEntry( entry )
        return hyp

    ## Private method
    ## Check if the given "Arithmetic1D" hypothesis has the same parameters as the given arguments
    def CompareArithmetic1D(self, hyp, args):
        if IsEqual(hyp.GetLength(1), args[0]):
            if IsEqual(hyp.GetLength(0), args[1]):
                if hyp.GetReversedEdges() == args[2]:
                    if not args[2] or hyp.GetObjectEntry() == args[3]:
                        return True
        return False


    ## Defines "FixedPoints1D" hypothesis to cut an edge using parameter
    # on curve from 0 to 1 (additionally it is neecessary to check
    # orientation of edges and create list of reversed edges if it is
    # needed) and sets numbers of segments between given points (default
    # values are equals 1
    #  @param points defines the list of parameters on curve
    #  @param nbSegs defines the list of numbers of segments
    #  @param reversedEdges is a list of edges to mesh using reversed orientation
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @return an instance of StdMeshers_Arithmetic1D hypothesis
    #  @ingroup l3_hypos_1dhyps
    def FixedPoints1D(self, points, nbSegs=[1], reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        if reversedEdges and isinstance(reversedEdges[0],geompyDC.GEOM._objref_GEOM_Object):
            reversedEdges = [ self.mesh.geompyD.GetSubShapeID(self.mesh.geom, e) for e in reversedEdges ]
        entry = self.MainShapeEntry()
        hyp = self.Hypothesis("FixedPoints1D", [points, nbSegs, reversedEdges, entry],
                              UseExisting=UseExisting,
                              CompareMethod=self.CompareFixedPoints1D)
        hyp.SetPoints(points)
        hyp.SetNbSegments(nbSegs)
        hyp.SetReversedEdges(reversedEdges)
        hyp.SetObjectEntry(entry)
        return hyp

    ## Private method
    ## Check if the given "FixedPoints1D" hypothesis has the same parameters
    ## as the given arguments
    def CompareFixedPoints1D(self, hyp, args):
        if hyp.GetPoints() == args[0]:
            if hyp.GetNbSegments() == args[1]:
                if hyp.GetReversedEdges() == args[2]:
                    if not args[2] or hyp.GetObjectEntry() == args[3]:
                        return True
        return False



    ## Defines "StartEndLength" hypothesis to cut an edge in several segments with increasing geometric length
    #  @param start defines the length of the first segment
    #  @param end   defines the length of the last  segment
    #  @param reversedEdges is a list of edges to mesh using reversed orientation
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @return an instance of StdMeshers_StartEndLength hypothesis
    #  @ingroup l3_hypos_1dhyps
    def StartEndLength(self, start, end, reversedEdges=[], UseExisting=0):
        if not isinstance(reversedEdges,list): #old version script, before adding reversedEdges
            reversedEdges, UseExisting = [], reversedEdges
        if reversedEdges and isinstance(reversedEdges[0],geompyDC.GEOM._objref_GEOM_Object):
            reversedEdges = [ self.mesh.geompyD.GetSubShapeID(self.mesh.geom, e) for e in reversedEdges ]
        entry = self.MainShapeEntry()
        hyp = self.Hypothesis("StartEndLength", [start, end, reversedEdges, entry],
                              UseExisting=UseExisting,
                              CompareMethod=self.CompareStartEndLength)
        hyp.SetStartLength(start)
        hyp.SetEndLength(end)
        hyp.SetReversedEdges( reversedEdges )
        hyp.SetObjectEntry( entry )
        return hyp

    ## Check if the given "StartEndLength" hypothesis has the same parameters as the given arguments
    def CompareStartEndLength(self, hyp, args):
        if IsEqual(hyp.GetLength(1), args[0]):
            if IsEqual(hyp.GetLength(0), args[1]):
                if hyp.GetReversedEdges() == args[2]:
                    if not args[2] or hyp.GetObjectEntry() == args[3]:
                        return True
        return False

    ## Defines "Deflection1D" hypothesis
    #  @param d for the deflection
    #  @param UseExisting if ==true - searches for an existing hypothesis created with
    #                     the same parameters, else (default) - create a new one
    #  @ingroup l3_hypos_1dhyps
    def Deflection1D(self, d, UseExisting=0):
        hyp = self.Hypothesis("Deflection1D", [d], UseExisting=UseExisting,
                              CompareMethod=self.CompareDeflection1D)
        hyp.SetDeflection(d)
        return hyp

    ## Check if the given "Deflection1D" hypothesis has the same parameters as the given arguments
    def CompareDeflection1D(self, hyp, args):
        return IsEqual(hyp.GetDeflection(), args[0])

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
        hyp = self.Hypothesis("AutomaticLength",[fineness],UseExisting=UseExisting,
                              CompareMethod=self.CompareAutomaticLength)
        hyp.SetFineness( fineness )
        return hyp

    ## Checks if the given "AutomaticLength" hypothesis has the same parameters as the given arguments
    def CompareAutomaticLength(self, hyp, args):
        return IsEqual(hyp.GetFineness(), args[0])

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
                vertex = self.mesh.geompyD.SubShapeAllSorted(self.geom, geompyDC.ShapeType["VERTEX"])[vertex]
                self.geom = vertex
                pass
            pass
        else:
            self.geom = vertex
            pass
        ### 0D algorithm
        if self.geom is None:
            raise RuntimeError, "Attemp to create SegmentAroundVertex_0D algoritm on None shape"
        try:
            name = GetName(self.geom)
            pass
        except:
            piece = self.mesh.geom
            name = self.mesh.geompyD.SubShapeName(self.geom, piece)
            self.mesh.geompyD.addToStudyInFather(piece, self.geom, name)
            pass
        algo = self.FindAlgorithm("SegmentAroundVertex_0D", self.mesh.smeshpyD)
        if algo is None:
            algo = self.mesh.smeshpyD.CreateHypothesis("SegmentAroundVertex_0D", "libStdMeshersEngine.so")
            pass
        status = self.mesh.mesh.AddHypothesis(self.geom, algo)
        TreatHypoStatus(status, "SegmentAroundVertex_0D", name, True)
        ###
        hyp = self.Hypothesis("SegmentLengthAroundVertex", [length], UseExisting=UseExisting,
                              CompareMethod=self.CompareLengthNearVertex)
        self.geom = store_geom
        hyp.SetLength( length )
        return hyp

    ## Checks if the given "LengthNearVertex" hypothesis has the same parameters as the given arguments
    #  @ingroup l3_algos_segmarv
    def CompareLengthNearVertex(self, hyp, args):
        return IsEqual(hyp.GetLength(), args[0])

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

## Defines a segment 1D algorithm for discretization
#
#  @ingroup l3_algos_basic
class Mesh_CompositeSegment(Mesh_Segment):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        self.Create(mesh, geom, "CompositeSegment_1D")


# Public class: Mesh_Segment_Python
# ---------------------------------

## Defines a segment 1D algorithm for discretization with python function
#
#  @ingroup l3_algos_basic
class Mesh_Segment_Python(Mesh_Segment):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        import Python1dPlugin
        self.Create(mesh, geom, "Python_1D", "libPython1dEngine.so")

    ## Defines "PythonSplit1D" hypothesis
    #  @param n for the number of segments that cut an edge
    #  @param func for the python function that calculates the length of all segments
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_1dhyps
    def PythonSplit1D(self, n, func, UseExisting=0):
        hyp = self.Hypothesis("PythonSplit1D", [n], "libPython1dEngine.so",
                              UseExisting=UseExisting, CompareMethod=self.ComparePythonSplit1D)
        hyp.SetNumberOfSegments(n)
        hyp.SetPythonLog10RatioFunction(func)
        return hyp

    ## Checks if the given "PythonSplit1D" hypothesis has the same parameters as the given arguments
    def ComparePythonSplit1D(self, hyp, args):
        #if hyp.GetNumberOfSegments() == args[0]:
        #    if hyp.GetPythonLog10RatioFunction() == args[1]:
        #        return True
        return False

# Public class: Mesh_Triangle
# ---------------------------

## Defines a triangle 2D algorithm
#
#  @ingroup l3_algos_basic
class Mesh_Triangle(Mesh_Algorithm):

    # default values
    algoType = 0
    params = 0

    _angleMeshS = 8
    _gradation  = 1.1

    ## Private constructor.
    def __init__(self, mesh, algoType, geom=0):
        Mesh_Algorithm.__init__(self)

        self.algoType = algoType
        if algoType == MEFISTO:
            self.Create(mesh, geom, "MEFISTO_2D")
            pass
        elif algoType == BLSURF:
            CheckPlugin(BLSURF)
            self.Create(mesh, geom, "BLSURF", "libBLSURFEngine.so")
            #self.SetPhysicalMesh() - PAL19680
        elif algoType == NETGEN:
            CheckPlugin(NETGEN)
            self.Create(mesh, geom, "NETGEN_2D", "libNETGENEngine.so")
            pass
        elif algoType == NETGEN_2D:
            CheckPlugin(NETGEN)
            self.Create(mesh, geom, "NETGEN_2D_ONLY", "libNETGENEngine.so")
            pass

    ## Defines "MaxElementArea" hypothesis basing on the definition of the maximum area of each triangle
    #  @param area for the maximum area of each triangle
    #  @param UseExisting if ==true - searches for an  existing hypothesis created with the
    #                     same parameters, else (default) - creates a new one
    #
    #  Only for algoType == MEFISTO || NETGEN_2D
    #  @ingroup l3_hypos_2dhyps
    def MaxElementArea(self, area, UseExisting=0):
        if self.algoType == MEFISTO or self.algoType == NETGEN_2D:
            hyp = self.Hypothesis("MaxElementArea", [area], UseExisting=UseExisting,
                                  CompareMethod=self.CompareMaxElementArea)
        elif self.algoType == NETGEN:
            hyp = self.Parameters(SIMPLE)
        hyp.SetMaxElementArea(area)
        return hyp

    ## Checks if the given "MaxElementArea" hypothesis has the same parameters as the given arguments
    def CompareMaxElementArea(self, hyp, args):
        return IsEqual(hyp.GetMaxElementArea(), args[0])

    ## Defines "LengthFromEdges" hypothesis to build triangles
    #  based on the length of the edges taken from the wire
    #
    #  Only for algoType == MEFISTO || NETGEN_2D
    #  @ingroup l3_hypos_2dhyps
    def LengthFromEdges(self):
        if self.algoType == MEFISTO or self.algoType == NETGEN_2D:
            hyp = self.Hypothesis("LengthFromEdges", UseExisting=1, CompareMethod=self.CompareEqualHyp)
            return hyp
        elif self.algoType == NETGEN:
            hyp = self.Parameters(SIMPLE)
            hyp.LengthFromEdges()
            return hyp

    ## Sets a way to define size of mesh elements to generate.
    #  @param thePhysicalMesh is: DefaultSize or Custom.
    #  @ingroup l3_hypos_blsurf
    def SetPhysicalMesh(self, thePhysicalMesh=DefaultSize):
        # Parameter of BLSURF algo
        self.Parameters().SetPhysicalMesh(thePhysicalMesh)

    ## Sets size of mesh elements to generate.
    #  @ingroup l3_hypos_blsurf
    def SetPhySize(self, theVal):
        # Parameter of BLSURF algo
        self.SetPhysicalMesh(1) #Custom - else why to set the size?
        self.Parameters().SetPhySize(theVal)

    ## Sets lower boundary of mesh element size (PhySize).
    #  @ingroup l3_hypos_blsurf
    def SetPhyMin(self, theVal=-1):
        #  Parameter of BLSURF algo
        self.Parameters().SetPhyMin(theVal)

    ## Sets upper boundary of mesh element size (PhySize).
    #  @ingroup l3_hypos_blsurf
    def SetPhyMax(self, theVal=-1):
        #  Parameter of BLSURF algo
        self.Parameters().SetPhyMax(theVal)

    ## Sets a way to define maximum angular deflection of mesh from CAD model.
    #  @param theGeometricMesh is: 0 (None) or 1 (Custom)
    #  @ingroup l3_hypos_blsurf
    def SetGeometricMesh(self, theGeometricMesh=0):
        #  Parameter of BLSURF algo
        if self.Parameters().GetPhysicalMesh() == 0: theGeometricMesh = 1
        self.params.SetGeometricMesh(theGeometricMesh)

    ## Sets angular deflection (in degrees) of a mesh face from CAD surface.
    #  @ingroup l3_hypos_blsurf
    def SetAngleMeshS(self, theVal=_angleMeshS):
        #  Parameter of BLSURF algo
        if self.Parameters().GetGeometricMesh() == 0: theVal = self._angleMeshS
        self.params.SetAngleMeshS(theVal)

    ## Sets angular deflection (in degrees) of a mesh edge from CAD curve.
    #  @ingroup l3_hypos_blsurf
    def SetAngleMeshC(self, theVal=_angleMeshS):
        #  Parameter of BLSURF algo
        if self.Parameters().GetGeometricMesh() == 0: theVal = self._angleMeshS
        self.params.SetAngleMeshC(theVal)

    ## Sets lower boundary of mesh element size computed to respect angular deflection.
    #  @ingroup l3_hypos_blsurf
    def SetGeoMin(self, theVal=-1):
        #  Parameter of BLSURF algo
        self.Parameters().SetGeoMin(theVal)

    ## Sets upper boundary of mesh element size computed to respect angular deflection.
    #  @ingroup l3_hypos_blsurf
    def SetGeoMax(self, theVal=-1):
        #  Parameter of BLSURF algo
        self.Parameters().SetGeoMax(theVal)

    ## Sets maximal allowed ratio between the lengths of two adjacent edges.
    #  @ingroup l3_hypos_blsurf
    def SetGradation(self, theVal=_gradation):
        #  Parameter of BLSURF algo
        if self.Parameters().GetGeometricMesh() == 0: theVal = self._gradation
        self.params.SetGradation(theVal)

    ## Sets topology usage way.
    # @param way defines how mesh conformity is assured <ul>
    # <li>FromCAD - mesh conformity is assured by conformity of a shape</li>
    # <li>PreProcess or PreProcessPlus - by pre-processing a CAD model</li></ul>
    #  @ingroup l3_hypos_blsurf
    def SetTopology(self, way):
        #  Parameter of BLSURF algo
        self.Parameters().SetTopology(way)

    ## To respect geometrical edges or not.
    #  @ingroup l3_hypos_blsurf
    def SetDecimesh(self, toIgnoreEdges=False):
        #  Parameter of BLSURF algo
        self.Parameters().SetDecimesh(toIgnoreEdges)

    ## Sets verbosity level in the range 0 to 100.
    #  @ingroup l3_hypos_blsurf
    def SetVerbosity(self, level):
        #  Parameter of BLSURF algo
        self.Parameters().SetVerbosity(level)

    ## Sets advanced option value.
    #  @ingroup l3_hypos_blsurf
    def SetOptionValue(self, optionName, level):
        #  Parameter of BLSURF algo
        self.Parameters().SetOptionValue(optionName,level)

    ## Sets QuadAllowed flag.
    #  Only for algoType == NETGEN || NETGEN_2D || BLSURF
    #  @ingroup l3_hypos_netgen l3_hypos_blsurf
    def SetQuadAllowed(self, toAllow=True):
        if self.algoType == NETGEN_2D:
            if toAllow: # add QuadranglePreference
                self.Hypothesis("QuadranglePreference", UseExisting=1, CompareMethod=self.CompareEqualHyp)
            else:       # remove QuadranglePreference
                for hyp in self.mesh.GetHypothesisList( self.geom ):
                    if hyp.GetName() == "QuadranglePreference":
                        self.mesh.RemoveHypothesis( self.geom, hyp )
                        pass
                    pass
                pass
            return
        if self.Parameters():
            self.params.SetQuadAllowed(toAllow)
            return

    ## Defines hypothesis having several parameters
    #
    #  @ingroup l3_hypos_netgen
    def Parameters(self, which=SOLE):
        if self.params:
            return self.params
        if self.algoType == NETGEN:
            if which == SIMPLE:
                self.params = self.Hypothesis("NETGEN_SimpleParameters_2D", [],
                                              "libNETGENEngine.so", UseExisting=0)
            else:
                self.params = self.Hypothesis("NETGEN_Parameters_2D", [],
                                              "libNETGENEngine.so", UseExisting=0)
            return self.params
        elif self.algoType == MEFISTO:
            print "Mefisto algo support no multi-parameter hypothesis"
            return None
        elif self.algoType == NETGEN_2D:
            print "NETGEN_2D_ONLY algo support no multi-parameter hypothesis"
            print "NETGEN_2D_ONLY uses 'MaxElementArea' and 'LengthFromEdges' ones"
            return None
        elif self.algoType == BLSURF:
            self.params = self.Hypothesis("BLSURF_Parameters", [],
                                          "libBLSURFEngine.so", UseExisting=0)
            return self.params
        else:
            print "Mesh_Triangle with algo type %s does not have such a parameter, check algo type"%self.algoType
        return None

    ## Sets MaxSize
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetMaxSize(self, theSize):
        if self.Parameters():
            self.params.SetMaxSize(theSize)

    ## Sets SecondOrder flag
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetSecondOrder(self, theVal):
        if self.Parameters():
            self.params.SetSecondOrder(theVal)

    ## Sets Optimize flag
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetOptimize(self, theVal):
        if self.Parameters():
            self.params.SetOptimize(theVal)

    ## Sets Fineness
    #  @param theFineness is:
    #  VeryCoarse, Coarse, Moderate, Fine, VeryFine or Custom
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetFineness(self, theFineness):
        if self.Parameters():
            self.params.SetFineness(theFineness)

    ## Sets GrowthRate
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetGrowthRate(self, theRate):
        if self.Parameters():
            self.params.SetGrowthRate(theRate)

    ## Sets NbSegPerEdge
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetNbSegPerEdge(self, theVal):
        if self.Parameters():
            self.params.SetNbSegPerEdge(theVal)

    ## Sets NbSegPerRadius
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetNbSegPerRadius(self, theVal):
        if self.Parameters():
            self.params.SetNbSegPerRadius(theVal)

    ## Sets number of segments overriding value set by SetLocalLength()
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetNumberOfSegments(self, theVal):
        self.Parameters(SIMPLE).SetNumberOfSegments(theVal)

    ## Sets number of segments overriding value set by SetNumberOfSegments()
    #
    #  Only for algoType == NETGEN
    #  @ingroup l3_hypos_netgen
    def SetLocalLength(self, theVal):
        self.Parameters(SIMPLE).SetLocalLength(theVal)

    pass


# Public class: Mesh_Quadrangle
# -----------------------------

## Defines a quadrangle 2D algorithm
#
#  @ingroup l3_algos_basic
class Mesh_Quadrangle(Mesh_Algorithm):

    params=0

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "Quadrangle_2D")
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
        vertexID = triangleVertex
        if isinstance( triangleVertex, geompyDC.GEOM._objref_GEOM_Object ):
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


# Public class: Mesh_Tetrahedron
# ------------------------------

## Defines a tetrahedron 3D algorithm
#
#  @ingroup l3_algos_basic
class Mesh_Tetrahedron(Mesh_Algorithm):

    params = 0
    algoType = 0

    ## Private constructor.
    def __init__(self, mesh, algoType, geom=0):
        Mesh_Algorithm.__init__(self)

        if algoType == NETGEN:
            CheckPlugin(NETGEN)
            self.Create(mesh, geom, "NETGEN_3D", "libNETGENEngine.so")
            pass

        elif algoType == FULL_NETGEN:
            CheckPlugin(NETGEN)
            self.Create(mesh, geom, "NETGEN_2D3D", "libNETGENEngine.so")
            pass

        elif algoType == GHS3D:
            CheckPlugin(GHS3D)
            self.Create(mesh, geom, "GHS3D_3D" , "libGHS3DEngine.so")
            pass

        elif algoType == GHS3DPRL:
            CheckPlugin(GHS3DPRL)
            self.Create(mesh, geom, "GHS3DPRL_3D" , "libGHS3DPRLEngine.so")
            pass

        self.algoType = algoType

    ## Defines "MaxElementVolume" hypothesis to give the maximun volume of each tetrahedron
    #  @param vol for the maximum volume of each tetrahedron
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                   the same parameters, else (default) - creates a new one
    #  @ingroup l3_hypos_maxvol
    def MaxElementVolume(self, vol, UseExisting=0):
        if self.algoType == NETGEN:
            hyp = self.Hypothesis("MaxElementVolume", [vol], UseExisting=UseExisting,
                                  CompareMethod=self.CompareMaxElementVolume)
            hyp.SetMaxElementVolume(vol)
            return hyp
        elif self.algoType == FULL_NETGEN:
            self.Parameters(SIMPLE).SetMaxElementVolume(vol)
        return None

    ## Checks if the given "MaxElementVolume" hypothesis has the same parameters as the given arguments
    def CompareMaxElementVolume(self, hyp, args):
        return IsEqual(hyp.GetMaxElementVolume(), args[0])

    ## Defines hypothesis having several parameters
    #
    #  @ingroup l3_hypos_netgen
    def Parameters(self, which=SOLE):
        if self.params:
            return self.params

        if self.algoType == FULL_NETGEN:
            if which == SIMPLE:
                self.params = self.Hypothesis("NETGEN_SimpleParameters_3D", [],
                                              "libNETGENEngine.so", UseExisting=0)
            else:
                self.params = self.Hypothesis("NETGEN_Parameters", [],
                                              "libNETGENEngine.so", UseExisting=0)
            return self.params

        if self.algoType == GHS3D:
            self.params = self.Hypothesis("GHS3D_Parameters", [],
                                          "libGHS3DEngine.so", UseExisting=0)
            return self.params

        if self.algoType == GHS3DPRL:
            self.params = self.Hypothesis("GHS3DPRL_Parameters", [],
                                          "libGHS3DPRLEngine.so", UseExisting=0)
            return self.params

        print "Algo supports no multi-parameter hypothesis"
        return None

    ## Sets MaxSize
    #  Parameter of FULL_NETGEN
    #  @ingroup l3_hypos_netgen
    def SetMaxSize(self, theSize):
        self.Parameters().SetMaxSize(theSize)

    ## Sets SecondOrder flag
    #  Parameter of FULL_NETGEN
    #  @ingroup l3_hypos_netgen
    def SetSecondOrder(self, theVal):
        self.Parameters().SetSecondOrder(theVal)

    ## Sets Optimize flag
    #  Parameter of FULL_NETGEN
    #  @ingroup l3_hypos_netgen
    def SetOptimize(self, theVal):
        self.Parameters().SetOptimize(theVal)

    ## Sets Fineness
    #  @param theFineness is:
    #  VeryCoarse, Coarse, Moderate, Fine, VeryFine or Custom
    #  Parameter of FULL_NETGEN
    #  @ingroup l3_hypos_netgen
    def SetFineness(self, theFineness):
        self.Parameters().SetFineness(theFineness)

    ## Sets GrowthRate
    #  Parameter of FULL_NETGEN
    #  @ingroup l3_hypos_netgen
    def SetGrowthRate(self, theRate):
        self.Parameters().SetGrowthRate(theRate)

    ## Sets NbSegPerEdge
    #  Parameter of FULL_NETGEN
    #  @ingroup l3_hypos_netgen
    def SetNbSegPerEdge(self, theVal):
        self.Parameters().SetNbSegPerEdge(theVal)

    ## Sets NbSegPerRadius
    #  Parameter of FULL_NETGEN
    #  @ingroup l3_hypos_netgen
    def SetNbSegPerRadius(self, theVal):
        self.Parameters().SetNbSegPerRadius(theVal)

    ## Sets number of segments overriding value set by SetLocalLength()
    #  Only for algoType == NETGEN_FULL
    #  @ingroup l3_hypos_netgen
    def SetNumberOfSegments(self, theVal):
        self.Parameters(SIMPLE).SetNumberOfSegments(theVal)

    ## Sets number of segments overriding value set by SetNumberOfSegments()
    #  Only for algoType == NETGEN_FULL
    #  @ingroup l3_hypos_netgen
    def SetLocalLength(self, theVal):
        self.Parameters(SIMPLE).SetLocalLength(theVal)

    ## Defines "MaxElementArea" parameter of NETGEN_SimpleParameters_3D hypothesis.
    #  Overrides value set by LengthFromEdges()
    #  Only for algoType == NETGEN_FULL
    #  @ingroup l3_hypos_netgen
    def MaxElementArea(self, area):
        self.Parameters(SIMPLE).SetMaxElementArea(area)

    ## Defines "LengthFromEdges" parameter of NETGEN_SimpleParameters_3D hypothesis
    #  Overrides value set by MaxElementArea()
    #  Only for algoType == NETGEN_FULL
    #  @ingroup l3_hypos_netgen
    def LengthFromEdges(self):
        self.Parameters(SIMPLE).LengthFromEdges()

    ## Defines "LengthFromFaces" parameter of NETGEN_SimpleParameters_3D hypothesis
    #  Overrides value set by MaxElementVolume()
    #  Only for algoType == NETGEN_FULL
    #  @ingroup l3_hypos_netgen
    def LengthFromFaces(self):
        self.Parameters(SIMPLE).LengthFromFaces()

    ## To mesh "holes" in a solid or not. Default is to mesh.
    #  @ingroup l3_hypos_ghs3dh
    def SetToMeshHoles(self, toMesh):
        #  Parameter of GHS3D
        self.Parameters().SetToMeshHoles(toMesh)

    ## Set Optimization level:
    #   None_Optimization, Light_Optimization, Standard_Optimization, StandardPlus_Optimization,
    #   Strong_Optimization.
    # Default is Standard_Optimization
    #  @ingroup l3_hypos_ghs3dh
    def SetOptimizationLevel(self, level):
        #  Parameter of GHS3D
        self.Parameters().SetOptimizationLevel(level)

    ## Maximal size of memory to be used by the algorithm (in Megabytes).
    #  @ingroup l3_hypos_ghs3dh
    def SetMaximumMemory(self, MB):
        #  Advanced parameter of GHS3D
        self.Parameters().SetMaximumMemory(MB)

    ## Initial size of memory to be used by the algorithm (in Megabytes) in
    #  automatic memory adjustment mode.
    #  @ingroup l3_hypos_ghs3dh
    def SetInitialMemory(self, MB):
        #  Advanced parameter of GHS3D
        self.Parameters().SetInitialMemory(MB)

    ## Path to working directory.
    #  @ingroup l3_hypos_ghs3dh
    def SetWorkingDirectory(self, path):
        #  Advanced parameter of GHS3D
        self.Parameters().SetWorkingDirectory(path)

    ## To keep working files or remove them. Log file remains in case of errors anyway.
    #  @ingroup l3_hypos_ghs3dh
    def SetKeepFiles(self, toKeep):
        #  Advanced parameter of GHS3D and GHS3DPRL
        self.Parameters().SetKeepFiles(toKeep)

    ## To set verbose level [0-10]. <ul>
    #<li> 0 - no standard output,
    #<li> 2 - prints the data, quality statistics of the skin and final meshes and
    #     indicates when the final mesh is being saved. In addition the software
    #     gives indication regarding the CPU time.
    #<li>10 - same as 2 plus the main steps in the computation, quality statistics
    #     histogram of the skin mesh, quality statistics histogram together with
    #     the characteristics of the final mesh.</ul>
    #  @ingroup l3_hypos_ghs3dh
    def SetVerboseLevel(self, level):
        #  Advanced parameter of GHS3D
        self.Parameters().SetVerboseLevel(level)

    ## To create new nodes.
    #  @ingroup l3_hypos_ghs3dh
    def SetToCreateNewNodes(self, toCreate):
        #  Advanced parameter of GHS3D
        self.Parameters().SetToCreateNewNodes(toCreate)

    ## To use boundary recovery version which tries to create mesh on a very poor
    #  quality surface mesh.
    #  @ingroup l3_hypos_ghs3dh
    def SetToUseBoundaryRecoveryVersion(self, toUse):
        #  Advanced parameter of GHS3D
        self.Parameters().SetToUseBoundaryRecoveryVersion(toUse)

    ## Sets command line option as text.
    #  @ingroup l3_hypos_ghs3dh
    def SetTextOption(self, option):
        #  Advanced parameter of GHS3D
        self.Parameters().SetTextOption(option)

    ## Sets MED files name and path.
    def SetMEDName(self, value):
        self.Parameters().SetMEDName(value)

    ## Sets the number of partition of the initial mesh
    def SetNbPart(self, value):
        self.Parameters().SetNbPart(value)

    ## When big mesh, start tepal in background
    def SetBackground(self, value):
        self.Parameters().SetBackground(value)

# Public class: Mesh_Hexahedron
# ------------------------------

## Defines a hexahedron 3D algorithm
#
#  @ingroup l3_algos_basic
class Mesh_Hexahedron(Mesh_Algorithm):

    params = 0
    algoType = 0

    ## Private constructor.
    def __init__(self, mesh, algoType=Hexa, geom=0):
        Mesh_Algorithm.__init__(self)

        self.algoType = algoType

        if algoType == Hexa:
            self.Create(mesh, geom, "Hexa_3D")
            pass

        elif algoType == Hexotic:
            CheckPlugin(Hexotic)
            self.Create(mesh, geom, "Hexotic_3D", "libHexoticEngine.so")
            pass

    ## Defines "MinMaxQuad" hypothesis to give three hexotic parameters
    #  @ingroup l3_hypos_hexotic
    def MinMaxQuad(self, min=3, max=8, quad=True):
        self.params = self.Hypothesis("Hexotic_Parameters", [], "libHexoticEngine.so",
                                      UseExisting=0)
        self.params.SetHexesMinLevel(min)
        self.params.SetHexesMaxLevel(max)
        self.params.SetHexoticQuadrangles(quad)
        return self.params

# Deprecated, only for compatibility!
# Public class: Mesh_Netgen
# ------------------------------

## Defines a NETGEN-based 2D or 3D algorithm
#  that needs no discrete boundary (i.e. independent)
#
#  This class is deprecated, only for compatibility!
#
#  More details.
#  @ingroup l3_algos_basic
class Mesh_Netgen(Mesh_Algorithm):

    is3D = 0

    ## Private constructor.
    def __init__(self, mesh, is3D, geom=0):
        Mesh_Algorithm.__init__(self)

        CheckPlugin(NETGEN)

        self.is3D = is3D
        if is3D:
            self.Create(mesh, geom, "NETGEN_2D3D", "libNETGENEngine.so")
            pass

        else:
            self.Create(mesh, geom, "NETGEN_2D", "libNETGENEngine.so")
            pass

    ## Defines the hypothesis containing parameters of the algorithm
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

## Defines a projection 1D algorithm
#  @ingroup l3_algos_proj
#
class Mesh_Projection1D(Mesh_Algorithm):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "Projection_1D")

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
        hyp = self.Hypothesis("ProjectionSource1D", [edge,mesh,srcV,tgtV],
                              UseExisting=0)
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceEdge)
        hyp.SetSourceEdge( edge )
        if not mesh is None and isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV, tgtV )
        return hyp

    ## Checks if the given "SourceEdge" hypothesis has the same parameters as the given arguments
    #def CompareSourceEdge(self, hyp, args):
    #    # it does not seem to be useful to reuse the existing "SourceEdge" hypothesis
    #    return False


# Public class: Mesh_Projection2D
# ------------------------------

## Defines a projection 2D algorithm
#  @ingroup l3_algos_proj
#
class Mesh_Projection2D(Mesh_Algorithm):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "Projection_2D")

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
        hyp = self.Hypothesis("ProjectionSource2D", [face,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=0)
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceFace)
        hyp.SetSourceFace( face )
        if not mesh is None and isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        hyp.SetSourceMesh( mesh )
        hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        return hyp

    ## Checks if the given "SourceFace" hypothesis has the same parameters as the given arguments
    #def CompareSourceFace(self, hyp, args):
    #    # it does not seem to be useful to reuse the existing "SourceFace" hypothesis
    #    return False

# Public class: Mesh_Projection3D
# ------------------------------

## Defines a projection 3D algorithm
#  @ingroup l3_algos_proj
#
class Mesh_Projection3D(Mesh_Algorithm):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "Projection_3D")

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
        hyp = self.Hypothesis("ProjectionSource3D",
                              [solid,mesh,srcV1,tgtV1,srcV2,tgtV2],
                              UseExisting=0)
                              #UseExisting=UseExisting, CompareMethod=self.CompareSourceShape3D)
        hyp.SetSource3DShape( solid )
        if not mesh is None and isinstance(mesh, Mesh):
            mesh = mesh.GetMesh()
        hyp.SetSourceMesh( mesh )
        if srcV1 and srcV2 and tgtV1 and tgtV2:
            hyp.SetVertexAssociation( srcV1, srcV2, tgtV1, tgtV2 )
        #elif srcV1 or srcV2 or tgtV1 or tgtV2:
        return hyp

    ## Checks if the given "SourceShape3D" hypothesis has the same parameters as given arguments
    #def CompareSourceShape3D(self, hyp, args):
    #    # seems to be not really useful to reuse existing "SourceShape3D" hypothesis
    #    return False


# Public class: Mesh_Prism
# ------------------------

## Defines a 3D extrusion algorithm
#  @ingroup l3_algos_3dextr
#
class Mesh_Prism3D(Mesh_Algorithm):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "Prism_3D")

# Public class: Mesh_RadialPrism
# -------------------------------

## Defines a Radial Prism 3D algorithm
#  @ingroup l3_algos_radialp
#
class Mesh_RadialPrism3D(Mesh_Algorithm):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "RadialPrism_3D")

        self.distribHyp = self.Hypothesis("LayerDistribution", UseExisting=0)
        self.nbLayers = None

    ## Return 3D hypothesis holding the 1D one
    def Get3DHypothesis(self):
        return self.distribHyp

    ## Private method creating a 1D hypothesis and storing it in the LayerDistribution
    #  hypothesis. Returns the created hypothesis
    def OwnHypothesis(self, hypType, args=[], so="libStdMeshersEngine.so"):
        #print "OwnHypothesis",hypType
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
        self.mesh.GetMesh().RemoveHypothesis( self.geom, self.distribHyp )
        self.nbLayers = self.Hypothesis("NumberOfLayers", [n], UseExisting=UseExisting,
                                        CompareMethod=self.CompareNumberOfLayers)
        self.nbLayers.SetNumberOfLayers( n )
        return self.nbLayers

    ## Checks if the given "NumberOfLayers" hypothesis has the same parameters as the given arguments
    def CompareNumberOfLayers(self, hyp, args):
        return IsEqual(hyp.GetNumberOfLayers(), args[0])

    ## Defines "LocalLength" hypothesis, specifying the segment length
    #  to build between the inner and the outer shells
    #  @param l the length of segments
    #  @param p the precision of rounding
    def LocalLength(self, l, p=1e-07):
        hyp = self.OwnHypothesis("LocalLength", [l,p])
        hyp.SetLength(l)
        hyp.SetPrecision(p)
        return hyp

    ## Defines "NumberOfSegments" hypothesis, specifying the number of layers of
    #  prisms to build between the inner and the outer shells.
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
    #  to build between the inner and the outer shells with a length that changes in arithmetic progression
    #  @param start  the length of the first segment
    #  @param end    the length of the last  segment
    def Arithmetic1D(self, start, end ):
        hyp = self.OwnHypothesis("Arithmetic1D", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    ## Defines "StartEndLength" hypothesis, specifying distribution of segments
    #  to build between the inner and the outer shells as geometric length increasing
    #  @param start for the length of the first segment
    #  @param end   for the length of the last  segment
    def StartEndLength(self, start, end):
        hyp = self.OwnHypothesis("StartEndLength", [start, end])
        hyp.SetLength(start, 1)
        hyp.SetLength(end  , 0)
        return hyp

    ## Defines "AutomaticLength" hypothesis, specifying the number of segments
    #  to build between the inner and outer shells
    #  @param fineness defines the quality of the mesh within the range [0-1]
    def AutomaticLength(self, fineness=0):
        hyp = self.OwnHypothesis("AutomaticLength")
        hyp.SetFineness( fineness )
        return hyp

# Public class: Mesh_RadialQuadrangle1D2D
# -------------------------------

## Defines a Radial Quadrangle 1D2D algorithm
#  @ingroup l2_algos_radialq
#
class Mesh_RadialQuadrangle1D2D(Mesh_Algorithm):

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        self.Create(mesh, geom, "RadialQuadrangle_1D2D")

        self.distribHyp = None #self.Hypothesis("LayerDistribution2D", UseExisting=0)
        self.nbLayers = None

    ## Return 2D hypothesis holding the 1D one
    def Get2DHypothesis(self):
        return self.distribHyp

    ## Private method creating a 1D hypothesis and storing it in the LayerDistribution
    #  hypothesis. Returns the created hypothesis
    def OwnHypothesis(self, hypType, args=[], so="libStdMeshersEngine.so"):
        #print "OwnHypothesis",hypType
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
        self.nbLayers = self.Hypothesis("NumberOfLayers2D", [n], UseExisting=UseExisting,
                                        CompareMethod=self.CompareNumberOfLayers)
        self.nbLayers.SetNumberOfLayers( n )
        return self.nbLayers

    ## Checks if the given "NumberOfLayers" hypothesis has the same parameters as the given arguments
    def CompareNumberOfLayers(self, hyp, args):
        return IsEqual(hyp.GetNumberOfLayers(), args[0])

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
#  @ingroup l3_algos_basic
#
class Mesh_UseExistingElements(Mesh_Algorithm):

    def __init__(self, dim, mesh, geom=0):
        if dim == 1:
            self.Create(mesh, geom, "Import_1D")
        else:
            self.Create(mesh, geom, "Import_1D2D")
        return

    ## Defines "Source edges" hypothesis, specifying groups of edges to import
    #  @param groups list of groups of edges
    #  @param toCopyMesh if True, the whole mesh \a groups belong to is imported
    #  @param toCopyGroups if True, all groups of the mesh \a groups belong to are imported
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def SourceEdges(self, groups, toCopyMesh=False, toCopyGroups=False, UseExisting=False):
        if self.algo.GetName() == "Import_2D":
            raise ValueError, "algoritm dimension mismatch"
        hyp = self.Hypothesis("ImportSource1D", [groups, toCopyMesh, toCopyGroups],
                              UseExisting=UseExisting, CompareMethod=self._compareHyp)
        hyp.SetSourceEdges(groups)
        hyp.SetCopySourceMesh(toCopyMesh, toCopyGroups)
        return hyp

    ## Defines "Source faces" hypothesis, specifying groups of faces to import
    #  @param groups list of groups of faces
    #  @param toCopyMesh if True, the whole mesh \a groups belong to is imported
    #  @param toCopyGroups if True, all groups of the mesh \a groups belong to are imported
    #  @param UseExisting if ==true - searches for the existing hypothesis created with
    #                     the same parameters, else (default) - creates a new one
    def SourceFaces(self, groups, toCopyMesh=False, toCopyGroups=False, UseExisting=False):
        if self.algo.GetName() == "Import_1D":
            raise ValueError, "algoritm dimension mismatch"
        hyp = self.Hypothesis("ImportSource2D", [groups, toCopyMesh, toCopyGroups],
                              UseExisting=UseExisting, CompareMethod=self._compareHyp)
        hyp.SetSourceFaces(groups)
        hyp.SetCopySourceMesh(toCopyMesh, toCopyGroups)
        return hyp

    def _compareHyp(self,hyp,args):
        if hasattr( hyp, "GetSourceEdges"):
            entries = hyp.GetSourceEdges()
        else:
            entries = hyp.GetSourceFaces()
        groups = args[0]
        toCopyMesh,toCopyGroups = hyp.GetCopySourceMesh()
        if len(entries)==len(groups) and toCopyMesh==args[1] and toCopyGroups==args[2]:
            entries2 = []
            study = self.mesh.smeshpyD.GetCurrentStudy()
            if study:
                for g in groups:
                    ior  = salome.orb.object_to_string(g)
                    sobj = study.FindObjectIOR(ior)
                    if sobj: entries2.append( sobj.GetID() )
                    pass
                pass
            entries.sort()
            entries2.sort()
            return entries == entries2
        return False


# Private class: Mesh_UseExisting
# -------------------------------
class Mesh_UseExisting(Mesh_Algorithm):

    def __init__(self, dim, mesh, geom=0):
        if dim == 1:
            self.Create(mesh, geom, "UseExisting_1D")
        else:
            self.Create(mesh, geom, "UseExisting_2D")


import salome_notebook
notebook = salome_notebook.notebook

##Return values of the notebook variables
def ParseParameters(last, nbParams,nbParam, value):
    result = None
    strResult = ""
    counter = 0
    listSize = len(last)
    for n in range(0,nbParams):
        if n+1 != nbParam:
            if counter < listSize:
                strResult = strResult + last[counter]
            else:
                strResult = strResult + ""
        else:
            if isinstance(value, str):
                if notebook.isVariable(value):
                    result = notebook.get(value)
                    strResult=strResult+value
                else:
                    raise RuntimeError, "Variable with name '" + value + "' doesn't exist!!!"
            else:
                strResult=strResult+str(value)
                result = value
        if nbParams - 1 != counter:
            strResult=strResult+var_separator #":"
        counter = counter+1
    return result, strResult

#Wrapper class for StdMeshers_LocalLength hypothesis
class LocalLength(StdMeshers._objref_StdMeshers_LocalLength):

    ## Set Length parameter value
    #  @param length numerical value or name of variable from notebook
    def SetLength(self, length):
        length,parameters = ParseParameters(StdMeshers._objref_StdMeshers_LocalLength.GetLastParameters(self),2,1,length)
        StdMeshers._objref_StdMeshers_LocalLength.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_LocalLength.SetLength(self,length)

   ## Set Precision parameter value
   #  @param precision numerical value or name of variable from notebook
    def SetPrecision(self, precision):
        precision,parameters = ParseParameters(StdMeshers._objref_StdMeshers_LocalLength.GetLastParameters(self),2,2,precision)
        StdMeshers._objref_StdMeshers_LocalLength.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_LocalLength.SetPrecision(self, precision)

#Registering the new proxy for LocalLength
omniORB.registerObjref(StdMeshers._objref_StdMeshers_LocalLength._NP_RepositoryId, LocalLength)


#Wrapper class for StdMeshers_LayerDistribution hypothesis
class LayerDistribution(StdMeshers._objref_StdMeshers_LayerDistribution):
    
    def SetLayerDistribution(self, hypo):
        StdMeshers._objref_StdMeshers_LayerDistribution.SetParameters(self,hypo.GetParameters())
        hypo.ClearParameters();
        StdMeshers._objref_StdMeshers_LayerDistribution.SetLayerDistribution(self,hypo)

#Registering the new proxy for LayerDistribution
omniORB.registerObjref(StdMeshers._objref_StdMeshers_LayerDistribution._NP_RepositoryId, LayerDistribution)

#Wrapper class for StdMeshers_SegmentLengthAroundVertex hypothesis
class SegmentLengthAroundVertex(StdMeshers._objref_StdMeshers_SegmentLengthAroundVertex):
    
    ## Set Length parameter value
    #  @param length numerical value or name of variable from notebook    
    def SetLength(self, length):
        length,parameters = ParseParameters(StdMeshers._objref_StdMeshers_SegmentLengthAroundVertex.GetLastParameters(self),1,1,length)
        StdMeshers._objref_StdMeshers_SegmentLengthAroundVertex.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_SegmentLengthAroundVertex.SetLength(self,length)

#Registering the new proxy for SegmentLengthAroundVertex
omniORB.registerObjref(StdMeshers._objref_StdMeshers_SegmentLengthAroundVertex._NP_RepositoryId, SegmentLengthAroundVertex)


#Wrapper class for StdMeshers_Arithmetic1D hypothesis
class Arithmetic1D(StdMeshers._objref_StdMeshers_Arithmetic1D):
    
    ## Set Length parameter value
    #  @param length   numerical value or name of variable from notebook
    #  @param isStart  true is length is Start Length, otherwise false
    def SetLength(self, length, isStart):
        nb = 2
        if isStart:
            nb = 1
        length,parameters = ParseParameters(StdMeshers._objref_StdMeshers_Arithmetic1D.GetLastParameters(self),2,nb,length)
        StdMeshers._objref_StdMeshers_Arithmetic1D.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_Arithmetic1D.SetLength(self,length,isStart)
        
#Registering the new proxy for Arithmetic1D
omniORB.registerObjref(StdMeshers._objref_StdMeshers_Arithmetic1D._NP_RepositoryId, Arithmetic1D)

#Wrapper class for StdMeshers_Deflection1D hypothesis
class Deflection1D(StdMeshers._objref_StdMeshers_Deflection1D):
    
    ## Set Deflection parameter value
    #  @param deflection numerical value or name of variable from notebook    
    def SetDeflection(self, deflection):
        deflection,parameters = ParseParameters(StdMeshers._objref_StdMeshers_Deflection1D.GetLastParameters(self),1,1,deflection)
        StdMeshers._objref_StdMeshers_Deflection1D.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_Deflection1D.SetDeflection(self,deflection)

#Registering the new proxy for Deflection1D
omniORB.registerObjref(StdMeshers._objref_StdMeshers_Deflection1D._NP_RepositoryId, Deflection1D)

#Wrapper class for StdMeshers_StartEndLength hypothesis
class StartEndLength(StdMeshers._objref_StdMeshers_StartEndLength):
    
    ## Set Length parameter value
    #  @param length  numerical value or name of variable from notebook
    #  @param isStart true is length is Start Length, otherwise false
    def SetLength(self, length, isStart):
        nb = 2
        if isStart:
            nb = 1
        length,parameters = ParseParameters(StdMeshers._objref_StdMeshers_StartEndLength.GetLastParameters(self),2,nb,length)
        StdMeshers._objref_StdMeshers_StartEndLength.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_StartEndLength.SetLength(self,length,isStart)
        
#Registering the new proxy for StartEndLength
omniORB.registerObjref(StdMeshers._objref_StdMeshers_StartEndLength._NP_RepositoryId, StartEndLength)

#Wrapper class for StdMeshers_MaxElementArea hypothesis
class MaxElementArea(StdMeshers._objref_StdMeshers_MaxElementArea):
    
    ## Set Max Element Area parameter value
    #  @param area  numerical value or name of variable from notebook
    def SetMaxElementArea(self, area):
        area ,parameters = ParseParameters(StdMeshers._objref_StdMeshers_MaxElementArea.GetLastParameters(self),1,1,area)
        StdMeshers._objref_StdMeshers_MaxElementArea.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_MaxElementArea.SetMaxElementArea(self,area)
        
#Registering the new proxy for MaxElementArea
omniORB.registerObjref(StdMeshers._objref_StdMeshers_MaxElementArea._NP_RepositoryId, MaxElementArea)


#Wrapper class for StdMeshers_MaxElementVolume hypothesis
class MaxElementVolume(StdMeshers._objref_StdMeshers_MaxElementVolume):
    
    ## Set Max Element Volume parameter value
    #  @param volume numerical value or name of variable from notebook
    def SetMaxElementVolume(self, volume):
        volume ,parameters = ParseParameters(StdMeshers._objref_StdMeshers_MaxElementVolume.GetLastParameters(self),1,1,volume)
        StdMeshers._objref_StdMeshers_MaxElementVolume.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_MaxElementVolume.SetMaxElementVolume(self,volume)
        
#Registering the new proxy for MaxElementVolume
omniORB.registerObjref(StdMeshers._objref_StdMeshers_MaxElementVolume._NP_RepositoryId, MaxElementVolume)


#Wrapper class for StdMeshers_NumberOfLayers hypothesis
class NumberOfLayers(StdMeshers._objref_StdMeshers_NumberOfLayers):
    
    ## Set Number Of Layers parameter value
    #  @param nbLayers  numerical value or name of variable from notebook
    def SetNumberOfLayers(self, nbLayers):
        nbLayers ,parameters = ParseParameters(StdMeshers._objref_StdMeshers_NumberOfLayers.GetLastParameters(self),1,1,nbLayers)
        StdMeshers._objref_StdMeshers_NumberOfLayers.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_NumberOfLayers.SetNumberOfLayers(self,nbLayers)
        
#Registering the new proxy for NumberOfLayers
omniORB.registerObjref(StdMeshers._objref_StdMeshers_NumberOfLayers._NP_RepositoryId, NumberOfLayers)

#Wrapper class for StdMeshers_NumberOfSegments hypothesis
class NumberOfSegments(StdMeshers._objref_StdMeshers_NumberOfSegments):
    
    ## Set Number Of Segments parameter value
    #  @param nbSeg numerical value or name of variable from notebook
    def SetNumberOfSegments(self, nbSeg):
        lastParameters = StdMeshers._objref_StdMeshers_NumberOfSegments.GetLastParameters(self)
        nbSeg , parameters = ParseParameters(lastParameters,1,1,nbSeg)
        StdMeshers._objref_StdMeshers_NumberOfSegments.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_NumberOfSegments.SetNumberOfSegments(self,nbSeg)
        
    ## Set Scale Factor parameter value
    #  @param factor numerical value or name of variable from notebook
    def SetScaleFactor(self, factor):
        factor, parameters = ParseParameters(StdMeshers._objref_StdMeshers_NumberOfSegments.GetLastParameters(self),2,2,factor)
        StdMeshers._objref_StdMeshers_NumberOfSegments.SetParameters(self,parameters)
        StdMeshers._objref_StdMeshers_NumberOfSegments.SetScaleFactor(self,factor)
        
#Registering the new proxy for NumberOfSegments
omniORB.registerObjref(StdMeshers._objref_StdMeshers_NumberOfSegments._NP_RepositoryId, NumberOfSegments)

if not noNETGENPlugin:
    #Wrapper class for NETGENPlugin_Hypothesis hypothesis
    class NETGENPlugin_Hypothesis(NETGENPlugin._objref_NETGENPlugin_Hypothesis):

        ## Set Max Size parameter value
        #  @param maxsize numerical value or name of variable from notebook
        def SetMaxSize(self, maxsize):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_Hypothesis.GetLastParameters(self)
            maxsize, parameters = ParseParameters(lastParameters,4,1,maxsize)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetMaxSize(self,maxsize)

        ## Set Growth Rate parameter value
        #  @param value  numerical value or name of variable from notebook
        def SetGrowthRate(self, value):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_Hypothesis.GetLastParameters(self)
            value, parameters = ParseParameters(lastParameters,4,2,value)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetGrowthRate(self,value)

        ## Set Number of Segments per Edge parameter value
        #  @param value  numerical value or name of variable from notebook
        def SetNbSegPerEdge(self, value):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_Hypothesis.GetLastParameters(self)
            value, parameters = ParseParameters(lastParameters,4,3,value)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetNbSegPerEdge(self,value)

        ## Set Number of Segments per Radius parameter value
        #  @param value  numerical value or name of variable from notebook
        def SetNbSegPerRadius(self, value):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_Hypothesis.GetLastParameters(self)
            value, parameters = ParseParameters(lastParameters,4,4,value)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_Hypothesis.SetNbSegPerRadius(self,value)

    #Registering the new proxy for NETGENPlugin_Hypothesis
    omniORB.registerObjref(NETGENPlugin._objref_NETGENPlugin_Hypothesis._NP_RepositoryId, NETGENPlugin_Hypothesis)


    #Wrapper class for NETGENPlugin_Hypothesis_2D hypothesis
    class NETGENPlugin_Hypothesis_2D(NETGENPlugin_Hypothesis,NETGENPlugin._objref_NETGENPlugin_Hypothesis_2D):
        pass

    #Registering the new proxy for NETGENPlugin_Hypothesis_2D
    omniORB.registerObjref(NETGENPlugin._objref_NETGENPlugin_Hypothesis_2D._NP_RepositoryId, NETGENPlugin_Hypothesis_2D)

    #Wrapper class for NETGENPlugin_SimpleHypothesis_2D hypothesis
    class NETGEN_SimpleParameters_2D(NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D):

        ## Set Number of Segments parameter value
        #  @param nbSeg numerical value or name of variable from notebook
        def SetNumberOfSegments(self, nbSeg):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.GetLastParameters(self)
            nbSeg, parameters = ParseParameters(lastParameters,2,1,nbSeg)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.SetNumberOfSegments(self, nbSeg)

        ## Set Local Length parameter value
        #  @param length numerical value or name of variable from notebook
        def SetLocalLength(self, length):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.GetLastParameters(self)
            length, parameters = ParseParameters(lastParameters,2,1,length)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.SetLocalLength(self, length)

        ## Set Max Element Area parameter value
        #  @param area numerical value or name of variable from notebook    
        def SetMaxElementArea(self, area):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.GetLastParameters(self)
            area, parameters = ParseParameters(lastParameters,2,2,area)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.SetMaxElementArea(self, area)

        def LengthFromEdges(self):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.GetLastParameters(self)
            value = 0;
            value, parameters = ParseParameters(lastParameters,2,2,value)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D.LengthFromEdges(self)

    #Registering the new proxy for NETGEN_SimpleParameters_2D
    omniORB.registerObjref(NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_2D._NP_RepositoryId, NETGEN_SimpleParameters_2D)


    #Wrapper class for NETGENPlugin_SimpleHypothesis_3D hypothesis
    class NETGEN_SimpleParameters_3D(NETGEN_SimpleParameters_2D,NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D):
        ## Set Max Element Volume parameter value
        #  @param volume numerical value or name of variable from notebook    
        def SetMaxElementVolume(self, volume):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D.GetLastParameters(self)
            volume, parameters = ParseParameters(lastParameters,3,3,volume)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D.SetMaxElementVolume(self, volume)

        def LengthFromFaces(self):
            lastParameters = NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D.GetLastParameters(self)
            value = 0;
            value, parameters = ParseParameters(lastParameters,3,3,value)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D.SetParameters(self,parameters)
            NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D.LengthFromFaces(self)

    #Registering the new proxy for NETGEN_SimpleParameters_3D
    omniORB.registerObjref(NETGENPlugin._objref_NETGENPlugin_SimpleHypothesis_3D._NP_RepositoryId, NETGEN_SimpleParameters_3D)

    pass # if not noNETGENPlugin:

class Pattern(SMESH._objref_SMESH_Pattern):

    def ApplyToMeshFaces(self, theMesh, theFacesIDs, theNodeIndexOnKeyPoint1, theReverse):
        flag = False
        if isinstance(theNodeIndexOnKeyPoint1,str):
            flag = True
        theNodeIndexOnKeyPoint1,Parameters = geompyDC.ParseParameters(theNodeIndexOnKeyPoint1)
        if flag:
            theNodeIndexOnKeyPoint1 -= 1
        theMesh.SetParameters(Parameters)
        return SMESH._objref_SMESH_Pattern.ApplyToMeshFaces( self, theMesh, theFacesIDs, theNodeIndexOnKeyPoint1, theReverse )

    def ApplyToHexahedrons(self, theMesh, theVolumesIDs, theNode000Index, theNode001Index):
        flag0 = False
        flag1 = False
        if isinstance(theNode000Index,str):
            flag0 = True
        if isinstance(theNode001Index,str):
            flag1 = True
        theNode000Index,theNode001Index,Parameters = geompyDC.ParseParameters(theNode000Index,theNode001Index)
        if flag0:
            theNode000Index -= 1
        if flag1:
            theNode001Index -= 1
        theMesh.SetParameters(Parameters)
        return SMESH._objref_SMESH_Pattern.ApplyToHexahedrons( self, theMesh, theVolumesIDs, theNode000Index, theNode001Index )

#Registering the new proxy for Pattern
omniORB.registerObjref(SMESH._objref_SMESH_Pattern._NP_RepositoryId, Pattern)
