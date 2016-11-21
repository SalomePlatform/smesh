# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
#  File   : smeshBuilder.py
#  Author : Francis KLOSS, OCC
#  Module : SMESH

## @package smeshBuilder
#  Python API for SALOME %Mesh module

## @defgroup l1_auxiliary Auxiliary methods and structures
## @defgroup l1_creating  Creating meshes
## @{
##   @defgroup l2_impexp     Importing and exporting meshes
##   @defgroup l2_construct  Constructing meshes
##   @defgroup l2_algorithms Defining Algorithms
##   @{
##     @defgroup l3_algos_basic   Basic meshing algorithms
##     @defgroup l3_algos_proj    Projection Algorithms
##     @defgroup l3_algos_segmarv Segments around Vertex
##     @defgroup l3_algos_3dextr  3D extrusion meshing algorithm

##   @}
##   @defgroup l2_hypotheses Defining hypotheses
##   @{
##     @defgroup l3_hypos_1dhyps 1D Meshing Hypotheses
##     @defgroup l3_hypos_2dhyps 2D Meshing Hypotheses
##     @defgroup l3_hypos_maxvol Max Element Volume hypothesis
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
##   @defgroup l2_modif_unitetri Uniting triangles
##   @defgroup l2_modif_cutquadr Cutting elements
##   @defgroup l2_modif_changori Changing orientation of elements
##   @defgroup l2_modif_smooth   Smoothing
##   @defgroup l2_modif_extrurev Extrusion and Revolution
##   @defgroup l2_modif_patterns Pattern mapping
##   @defgroup l2_modif_tofromqu Convert to/from Quadratic Mesh

## @}
## @defgroup l1_measurements Measurements

import salome
from salome.geom import geomBuilder

import SMESH # This is necessary for back compatibility
from   SMESH import *
from   salome.smesh.smesh_algorithm import Mesh_Algorithm

import SALOME
import SALOMEDS
import os

class MeshMeta(type):
    def __instancecheck__(cls, inst):
        """Implement isinstance(inst, cls)."""
        return any(cls.__subclasscheck__(c)
                   for c in {type(inst), inst.__class__})

    def __subclasscheck__(cls, sub):
        """Implement issubclass(sub, cls)."""
        return type.__subclasscheck__(cls, sub) or (cls.__name__ == sub.__name__ and cls.__module__ == sub.__module__)

## @addtogroup l1_auxiliary
## @{

## Converts an angle from degrees to radians
def DegreesToRadians(AngleInDegrees):
    from math import pi
    return AngleInDegrees * pi / 180.0

import salome_notebook
notebook = salome_notebook.notebook
# Salome notebook variable separator
var_separator = ":"

## Return list of variable values from salome notebook.
#  The last argument, if is callable, is used to modify values got from notebook
def ParseParameters(*args):
    Result = []
    Parameters = ""
    hasVariables = False
    varModifFun=None
    if args and callable( args[-1] ):
        args, varModifFun = args[:-1], args[-1]
    for parameter in args:

        Parameters += str(parameter) + var_separator

        if isinstance(parameter,str):
            # check if there is an inexistent variable name
            if not notebook.isVariable(parameter):
                raise ValueError, "Variable with name '" + parameter + "' doesn't exist!!!"
            parameter = notebook.get(parameter)
            hasVariables = True
            if varModifFun:
                parameter = varModifFun(parameter)
                pass
            pass
        Result.append(parameter)

        pass
    Parameters = Parameters[:-1]
    Result.append( Parameters )
    Result.append( hasVariables )
    return Result

# Parse parameters converting variables to radians
def ParseAngles(*args):
    return ParseParameters( *( args + (DegreesToRadians, )))

# Substitute PointStruct.__init__() to create SMESH.PointStruct using notebook variables.
# Parameters are stored in PointStruct.parameters attribute
def __initPointStruct(point,*args):
    point.x, point.y, point.z, point.parameters,hasVars = ParseParameters(*args)
    pass
SMESH.PointStruct.__init__ = __initPointStruct

# Substitute AxisStruct.__init__() to create SMESH.AxisStruct using notebook variables.
# Parameters are stored in AxisStruct.parameters attribute
def __initAxisStruct(ax,*args):
    if len( args ) != 6:
        raise RuntimeError,\
              "Bad nb args (%s) passed in SMESH.AxisStruct(x,y,z,dx,dy,dz)"%(len( args ))
    ax.x, ax.y, ax.z, ax.vx, ax.vy, ax.vz, ax.parameters,hasVars = ParseParameters(*args)
    pass
SMESH.AxisStruct.__init__ = __initAxisStruct

smeshPrecisionConfusion = 1.e-07
def IsEqual(val1, val2, tol=smeshPrecisionConfusion):
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
        try:
            ior  = salome.orb.object_to_string(obj)
        except:
            ior = None
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
def TreatHypoStatus(status, hypName, geomName, isAlgo, mesh):
    if isAlgo:
        hypType = "algorithm"
    else:
        hypType = "hypothesis"
        pass
    reason = ""
    if hasattr( status, "__getitem__" ):
        status,reason = status[0],status[1]
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
        reason = "the shape is neither the main one, nor its sub-shape, nor a valid group"
    elif status == HYP_BAD_GEOMETRY:
        reason = "the algorithm is not applicable to this geometry"
    elif status == HYP_HIDDEN_ALGO:
        reason = "it is hidden by an algorithm of an upper dimension, which generates elements of all dimensions"
    elif status == HYP_HIDING_ALGO:
        reason = "it hides algorithms of lower dimensions by generating elements of all dimensions"
    elif status == HYP_NEED_SHAPE:
        reason = "algorithm can't work without shape"
    elif status == HYP_INCOMPAT_HYPS:
        pass
    else:
        return
    where = geomName
    if where:
        where = '"%s"' % geomName
        if mesh:
            meshName = GetName( mesh )
            if meshName and meshName != NO_NAME:
                where = '"%s" shape in "%s" mesh ' % ( geomName, meshName )
    if status < HYP_UNKNOWN_FATAL and where:
        print '"%s" was assigned to %s but %s' %( hypName, where, reason )
    elif where:
        print '"%s" was not assigned to %s : %s' %( hypName, where, reason )
    else:
        print '"%s" was not assigned : %s' %( hypName, reason )
        pass

## Private method. Add geom (sub-shape of the main shape) into the study if not yet there
def AssureGeomPublished(mesh, geom, name=''):
    if not isinstance( geom, geomBuilder.GEOM._objref_GEOM_Object ):
        return
    if not geom.GetStudyEntry() and \
           mesh.smeshpyD.GetCurrentStudy():
        ## set the study
        studyID = mesh.smeshpyD.GetCurrentStudy()._get_StudyId()
        if studyID != mesh.geompyD.myStudyId:
            mesh.geompyD.init_geom( mesh.smeshpyD.GetCurrentStudy())
        ## get a name
        if not name and geom.GetShapeType() != geomBuilder.GEOM.COMPOUND:
            # for all groups SubShapeName() returns "Compound_-1"
            name = mesh.geompyD.SubShapeName(geom, mesh.geom)
        if not name:
            name = "%s_%s"%(geom.GetShapeType(), id(geom)%10000)
        ## publish
        mesh.geompyD.addToStudyInFather( mesh.geom, geom, name )
    return

## Return the first vertex of a geometrical edge by ignoring orientation
def FirstVertexOnCurve(mesh, edge):
    vv = mesh.geompyD.SubShapeAll( edge, geomBuilder.geomBuilder.ShapeType["VERTEX"])
    if not vv:
        raise TypeError, "Given object has no vertices"
    if len( vv ) == 1: return vv[0]
    v0   = mesh.geompyD.MakeVertexOnCurve(edge,0.)
    xyz  = mesh.geompyD.PointCoordinates( v0 ) # coords of the first vertex
    xyz1 = mesh.geompyD.PointCoordinates( vv[0] )
    xyz2 = mesh.geompyD.PointCoordinates( vv[1] )
    dist1, dist2 = 0,0
    for i in range(3):
        dist1 += abs( xyz[i] - xyz1[i] )
        dist2 += abs( xyz[i] - xyz2[i] )
    if dist1 < dist2:
        return vv[0]
    else:
        return vv[1]

# end of l1_auxiliary
## @}


# Warning: smeshInst is a singleton
smeshInst = None
engine = None
doLcc = False
created = False

## This class allows to create, load or manipulate meshes.
#  It has a set of methods to create, load or copy meshes, to combine several meshes, etc.
#  It also has methods to get infos and measure meshes.
class smeshBuilder(object, SMESH._objref_SMESH_Gen):

    # MirrorType enumeration
    POINT = SMESH_MeshEditor.POINT
    AXIS =  SMESH_MeshEditor.AXIS
    PLANE = SMESH_MeshEditor.PLANE

    # Smooth_Method enumeration
    LAPLACIAN_SMOOTH = SMESH_MeshEditor.LAPLACIAN_SMOOTH
    CENTROIDAL_SMOOTH = SMESH_MeshEditor.CENTROIDAL_SMOOTH

    PrecisionConfusion = smeshPrecisionConfusion

    # TopAbs_State enumeration
    [TopAbs_IN, TopAbs_OUT, TopAbs_ON, TopAbs_UNKNOWN] = range(4)

    # Methods of splitting a hexahedron into tetrahedra
    Hex_5Tet, Hex_6Tet, Hex_24Tet, Hex_2Prisms, Hex_4Prisms = 1, 2, 3, 1, 2

    def __new__(cls):
        global engine
        global smeshInst
        global doLcc
        #print "==== __new__", engine, smeshInst, doLcc

        if smeshInst is None:
            # smesh engine is either retrieved from engine, or created
            smeshInst = engine
            # Following test avoids a recursive loop
            if doLcc:
                if smeshInst is not None:
                    # smesh engine not created: existing engine found
                    doLcc = False
                if doLcc:
                    doLcc = False
                    # FindOrLoadComponent called:
                    # 1. CORBA resolution of server
                    # 2. the __new__ method is called again
                    #print "==== smeshInst = lcc.FindOrLoadComponent ", engine, smeshInst, doLcc
                    smeshInst = salome.lcc.FindOrLoadComponent( "FactoryServer", "SMESH" )
            else:
                # FindOrLoadComponent not called
                if smeshInst is None:
                    # smeshBuilder instance is created from lcc.FindOrLoadComponent
                    #print "==== smeshInst = super(smeshBuilder,cls).__new__(cls) ", engine, smeshInst, doLcc
                    smeshInst = super(smeshBuilder,cls).__new__(cls)
                else:
                    # smesh engine not created: existing engine found
                    #print "==== existing ", engine, smeshInst, doLcc
                    pass
            #print "====1 ", smeshInst
            return smeshInst

        #print "====2 ", smeshInst
        return smeshInst

    def __init__(self):
        global created
        #print "--------------- smeshbuilder __init__ ---", created
        if not created:
          created = True
          SMESH._objref_SMESH_Gen.__init__(self)

    ## Dump component to the Python script
    #  This method overrides IDL function to allow default values for the parameters.
    def DumpPython(self, theStudy, theIsPublished=True, theIsMultiFile=True):
        return SMESH._objref_SMESH_Gen.DumpPython(self, theStudy, theIsPublished, theIsMultiFile)

    ## Set mode of DumpPython(), \a historical or \a snapshot.
    # In the \a historical mode, the Python Dump script includes all commands
    # performed by SMESH engine. In the \a snapshot mode, commands
    # relating to objects removed from the Study are excluded from the script
    # as well as commands not influencing the current state of meshes
    def SetDumpPythonHistorical(self, isHistorical):
        if isHistorical: val = "true"
        else:            val = "false"
        SMESH._objref_SMESH_Gen.SetOption(self, "historical_python_dump", val)

    ## Sets the current study and Geometry component
    #  @ingroup l1_auxiliary
    def init_smesh(self,theStudy,geompyD = None):
        #print "init_smesh"
        self.SetCurrentStudy(theStudy,geompyD)
        if theStudy:
            global notebook
            notebook.myStudy = theStudy

    ## Creates a mesh. This can be either an empty mesh, possibly having an underlying geometry,
    #  or a mesh wrapping a CORBA mesh given as a parameter.
    #  @param obj either (1) a CORBA mesh (SMESH._objref_SMESH_Mesh) got e.g. by calling
    #         salome.myStudy.FindObjectID("0:1:2:3").GetObject() or
    #         (2) a Geometrical object for meshing or
    #         (3) none.
    #  @param name the name for the new mesh.
    #  @return an instance of Mesh class.
    #  @ingroup l2_construct
    def Mesh(self, obj=0, name=0):
        if isinstance(obj,str):
            obj,name = name,obj
        return Mesh(self,self.geompyD,obj,name)

    ## Returns a long value from enumeration
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
        vertices = self.geompyD.SubShapeAll( theVector, geomBuilder.geomBuilder.ShapeType["VERTEX"] )
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
        import GEOM
        edges = self.geompyD.SubShapeAll( theObj, geomBuilder.geomBuilder.ShapeType["EDGE"] )
        axis = None
        if len(edges) > 1:
            vertex1, vertex2 = self.geompyD.SubShapeAll( edges[0], geomBuilder.geomBuilder.ShapeType["VERTEX"] )
            vertex3, vertex4 = self.geompyD.SubShapeAll( edges[1], geomBuilder.geomBuilder.ShapeType["VERTEX"] )
            vertex1 = self.geompyD.PointCoordinates(vertex1)
            vertex2 = self.geompyD.PointCoordinates(vertex2)
            vertex3 = self.geompyD.PointCoordinates(vertex3)
            vertex4 = self.geompyD.PointCoordinates(vertex4)
            v1 = [vertex2[0]-vertex1[0], vertex2[1]-vertex1[1], vertex2[2]-vertex1[2]]
            v2 = [vertex4[0]-vertex3[0], vertex4[1]-vertex3[1], vertex4[2]-vertex3[2]]
            normal = [ v1[1]*v2[2]-v2[1]*v1[2], v1[2]*v2[0]-v2[2]*v1[0], v1[0]*v2[1]-v2[0]*v1[1] ]
            axis = AxisStruct(vertex1[0], vertex1[1], vertex1[2], normal[0], normal[1], normal[2])
            axis._mirrorType = SMESH.SMESH_MeshEditor.PLANE
        elif len(edges) == 1:
            vertex1, vertex2 = self.geompyD.SubShapeAll( edges[0], geomBuilder.geomBuilder.ShapeType["VERTEX"] )
            p1 = self.geompyD.PointCoordinates( vertex1 )
            p2 = self.geompyD.PointCoordinates( vertex2 )
            axis = AxisStruct(p1[0], p1[1], p1[2], p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2])
            axis._mirrorType = SMESH.SMESH_MeshEditor.AXIS
        elif theObj.GetShapeType() == GEOM.VERTEX:
            x,y,z = self.geompyD.PointCoordinates( theObj )
            axis = AxisStruct( x,y,z, 1,0,0,)
            axis._mirrorType = SMESH.SMESH_MeshEditor.POINT
        return axis

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
        SMESH._objref_SMESH_Gen.SetEmbeddedMode(self,theMode)

    ## Gets the current mode
    #  @ingroup l1_auxiliary
    def IsEmbeddedMode(self):
        return SMESH._objref_SMESH_Gen.IsEmbeddedMode(self)

    ## Sets the current study. Calling SetCurrentStudy( None ) allows to
    #  switch OFF automatic pubilishing in the Study of mesh objects.
    #  @ingroup l1_auxiliary
    def SetCurrentStudy( self, theStudy, geompyD = None ):
        if not geompyD:
            from salome.geom import geomBuilder
            geompyD = geomBuilder.geom
            pass
        self.geompyD=geompyD
        self.SetGeomEngine(geompyD)
        SMESH._objref_SMESH_Gen.SetCurrentStudy(self,theStudy)
        global notebook
        if theStudy:
            notebook = salome_notebook.NoteBook( theStudy )
        else:
            notebook = salome_notebook.NoteBook( salome_notebook.PseudoStudyForNoteBook() )
        if theStudy:
            sb = theStudy.NewBuilder()
            sc = theStudy.FindComponent("SMESH")
            if sc: sb.LoadWith(sc, self)
            pass
        pass

    ## Gets the current study
    #  @ingroup l1_auxiliary
    def GetCurrentStudy(self):
        return SMESH._objref_SMESH_Gen.GetCurrentStudy(self)

    ## Creates a Mesh object importing data from the given UNV file
    #  @return an instance of Mesh class
    #  @ingroup l2_impexp
    def CreateMeshesFromUNV( self,theFileName ):
        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromUNV(self,theFileName)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh)
        return aMesh

    ## Creates a Mesh object(s) importing data from the given MED file
    #  @return a tuple ( list of Mesh class instances, SMESH.DriverMED_ReadStatus )
    #  @ingroup l2_impexp
    def CreateMeshesFromMED( self,theFileName ):
        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromMED(self,theFileName)
        aMeshes = [ Mesh(self, self.geompyD, m) for m in aSmeshMeshes ]
        return aMeshes, aStatus

    ## Creates a Mesh object(s) importing data from the given SAUV file
    #  @return a tuple ( list of Mesh class instances, SMESH.DriverMED_ReadStatus )
    #  @ingroup l2_impexp
    def CreateMeshesFromSAUV( self,theFileName ):
        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromSAUV(self,theFileName)
        aMeshes = [ Mesh(self, self.geompyD, m) for m in aSmeshMeshes ]
        return aMeshes, aStatus

    ## Creates a Mesh object importing data from the given STL file
    #  @return an instance of Mesh class
    #  @ingroup l2_impexp
    def CreateMeshesFromSTL( self, theFileName ):
        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromSTL(self,theFileName)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh)
        return aMesh

    ## Creates Mesh objects importing data from the given CGNS file
    #  @return a tuple ( list of Mesh class instances, SMESH.DriverMED_ReadStatus )
    #  @ingroup l2_impexp
    def CreateMeshesFromCGNS( self, theFileName ):
        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromCGNS(self,theFileName)
        aMeshes = [ Mesh(self, self.geompyD, m) for m in aSmeshMeshes ]
        return aMeshes, aStatus

    ## Creates a Mesh object importing data from the given GMF file.
    #  GMF files must have .mesh extension for the ASCII format and .meshb for
    #  the binary format.
    #  @return [ an instance of Mesh class, SMESH.ComputeError ]
    #  @ingroup l2_impexp
    def CreateMeshesFromGMF( self, theFileName ):
        aSmeshMesh, error = SMESH._objref_SMESH_Gen.CreateMeshesFromGMF(self,
                                                                        theFileName,
                                                                        True)
        if error.comment: print "*** CreateMeshesFromGMF() errors:\n", error.comment
        return Mesh(self, self.geompyD, aSmeshMesh), error

    ## Concatenate the given meshes into one mesh. All groups of input meshes will be
    #  present in the new mesh.
    #  @param meshes the meshes, sub-meshes and groups to combine into one mesh
    #  @param uniteIdenticalGroups if true, groups with same names are united, else they are renamed
    #  @param mergeNodesAndElements if true, equal nodes and elements are merged
    #  @param mergeTolerance tolerance for merging nodes
    #  @param allGroups forces creation of groups corresponding to every input mesh
    #  @param name name of a new mesh
    #  @return an instance of Mesh class
    #  @ingroup l2_compounds
    def Concatenate( self, meshes, uniteIdenticalGroups,
                     mergeNodesAndElements = False, mergeTolerance = 1e-5, allGroups = False,
                     name = ""):
        if not meshes: return None
        for i,m in enumerate(meshes):
            if isinstance(m, Mesh):
                meshes[i] = m.GetMesh()
        mergeTolerance,Parameters,hasVars = ParseParameters(mergeTolerance)
        meshes[0].SetParameters(Parameters)
        if allGroups:
            aSmeshMesh = SMESH._objref_SMESH_Gen.ConcatenateWithGroups(
                self,meshes,uniteIdenticalGroups,mergeNodesAndElements,mergeTolerance)
        else:
            aSmeshMesh = SMESH._objref_SMESH_Gen.Concatenate(
                self,meshes,uniteIdenticalGroups,mergeNodesAndElements,mergeTolerance)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh, name=name)
        return aMesh

    ## Create a mesh by copying a part of another mesh.
    #  @param meshPart a part of mesh to copy, either a Mesh, a sub-mesh or a group;
    #                  to copy nodes or elements not contained in any mesh object,
    #                  pass result of Mesh.GetIDSource( list_of_ids, type ) as meshPart
    #  @param meshName a name of the new mesh
    #  @param toCopyGroups to create in the new mesh groups the copied elements belongs to
    #  @param toKeepIDs to preserve order of the copied elements or not
    #  @return an instance of Mesh class
    def CopyMesh( self, meshPart, meshName, toCopyGroups=False, toKeepIDs=False):
        if (isinstance( meshPart, Mesh )):
            meshPart = meshPart.GetMesh()
        mesh = SMESH._objref_SMESH_Gen.CopyMesh( self,meshPart,meshName,toCopyGroups,toKeepIDs )
        return Mesh(self, self.geompyD, mesh)

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
    #  \n Criterion structures allow to define complex filters by combining them with logical operations (AND / OR) (see example below)
    #  @param elementType the type of elements(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
    #  @param CritType the type of criterion (SMESH.FT_Taper, SMESH.FT_Area, etc.)
    #          Type SMESH.FunctorType._items in the Python Console to see all values.
    #          Note that the items starting from FT_LessThan are not suitable for CritType.
    #  @param Compare  belongs to {SMESH.FT_LessThan, SMESH.FT_MoreThan, SMESH.FT_EqualTo}
    #  @param Threshold the threshold value (range of ids as string, shape, numeric)
    #  @param UnaryOp  SMESH.FT_LogicalNOT or SMESH.FT_Undefined
    #  @param BinaryOp a binary logical operation SMESH.FT_LogicalAND, SMESH.FT_LogicalOR or
    #                  SMESH.FT_Undefined
    #  @param Tolerance the tolerance used by SMESH.FT_BelongToGeom, SMESH.FT_BelongToSurface,
    #         SMESH.FT_LyingOnGeom, SMESH.FT_CoplanarFaces criteria
    #  @return SMESH.Filter.Criterion
    #
    #  <a href="../tui_filters_page.html#combining_filters">Example of Criteria usage</a>
    #  @ingroup l1_controls
    def GetCriterion(self,elementType,
                     CritType,
                     Compare = FT_EqualTo,
                     Threshold="",
                     UnaryOp=FT_Undefined,
                     BinaryOp=FT_Undefined,
                     Tolerance=1e-07):
        if not CritType in SMESH.FunctorType._items:
            raise TypeError, "CritType should be of SMESH.FunctorType"
        aCriterion               = self.GetEmptyCriterion()
        aCriterion.TypeOfElement = elementType
        aCriterion.Type          = self.EnumToLong(CritType)
        aCriterion.Tolerance     = Tolerance

        aThreshold = Threshold

        if Compare in [FT_LessThan, FT_MoreThan, FT_EqualTo]:
            aCriterion.Compare = self.EnumToLong(Compare)
        elif Compare == "=" or Compare == "==":
            aCriterion.Compare = self.EnumToLong(FT_EqualTo)
        elif Compare == "<":
            aCriterion.Compare = self.EnumToLong(FT_LessThan)
        elif Compare == ">":
            aCriterion.Compare = self.EnumToLong(FT_MoreThan)
        elif Compare != FT_Undefined:
            aCriterion.Compare = self.EnumToLong(FT_EqualTo)
            aThreshold = Compare

        if CritType in [FT_BelongToGeom,     FT_BelongToPlane, FT_BelongToGenSurface,
                        FT_BelongToCylinder, FT_LyingOnGeom]:
            # Check that Threshold is GEOM object
            if isinstance(aThreshold, geomBuilder.GEOM._objref_GEOM_Object):
                aCriterion.ThresholdStr = GetName(aThreshold)
                aCriterion.ThresholdID  = aThreshold.GetStudyEntry()
                if not aCriterion.ThresholdID:
                    name = aCriterion.ThresholdStr
                    if not name:
                        name = "%s_%s"%(aThreshold.GetShapeType(), id(aThreshold)%10000)
                    aCriterion.ThresholdID = self.geompyD.addToStudy( aThreshold, name )
            # or a name of GEOM object
            elif isinstance( aThreshold, str ):
                aCriterion.ThresholdStr = aThreshold
            else:
                raise TypeError, "The Threshold should be a shape."
            if isinstance(UnaryOp,float):
                aCriterion.Tolerance = UnaryOp
                UnaryOp = FT_Undefined
                pass
        elif CritType == FT_BelongToMeshGroup:
            # Check that Threshold is a group
            if isinstance(aThreshold, SMESH._objref_SMESH_GroupBase):
                if aThreshold.GetType() != elementType:
                    raise ValueError, "Group type mismatches Element type"
                aCriterion.ThresholdStr = aThreshold.GetName()
                aCriterion.ThresholdID  = salome.orb.object_to_string( aThreshold )
                study = self.GetCurrentStudy()
                if study:
                    so = study.FindObjectIOR( aCriterion.ThresholdID )
                    if so:
                        entry = so.GetID()
                        if entry:
                            aCriterion.ThresholdID = entry
            else:
                raise TypeError, "The Threshold should be a Mesh Group"
        elif CritType == FT_RangeOfIds:
            # Check that Threshold is string
            if isinstance(aThreshold, str):
                aCriterion.ThresholdStr = aThreshold
            else:
                raise TypeError, "The Threshold should be a string."
        elif CritType == FT_CoplanarFaces:
            # Check the Threshold
            if isinstance(aThreshold, int):
                aCriterion.ThresholdID = str(aThreshold)
            elif isinstance(aThreshold, str):
                ID = int(aThreshold)
                if ID < 1:
                    raise ValueError, "Invalid ID of mesh face: '%s'"%aThreshold
                aCriterion.ThresholdID = aThreshold
            else:
                raise TypeError,\
                      "The Threshold should be an ID of mesh face and not '%s'"%aThreshold
        elif CritType == FT_ConnectedElements:
            # Check the Threshold
            if isinstance(aThreshold, geomBuilder.GEOM._objref_GEOM_Object): # shape
                aCriterion.ThresholdID = aThreshold.GetStudyEntry()
                if not aCriterion.ThresholdID:
                    name = aThreshold.GetName()
                    if not name:
                        name = "%s_%s"%(aThreshold.GetShapeType(), id(aThreshold)%10000)
                    aCriterion.ThresholdID = self.geompyD.addToStudy( aThreshold, name )
            elif isinstance(aThreshold, int): # node id
                aCriterion.Threshold = aThreshold
            elif isinstance(aThreshold, list): # 3 point coordinates
                if len( aThreshold ) < 3:
                    raise ValueError, "too few point coordinates, must be 3"
                aCriterion.ThresholdStr = " ".join( [str(c) for c in aThreshold[:3]] )
            elif isinstance(aThreshold, str):
                if aThreshold.isdigit():
                    aCriterion.Threshold = aThreshold # node id
                else:
                    aCriterion.ThresholdStr = aThreshold # hope that it's point coordinates
            else:
                raise TypeError,\
                      "The Threshold should either a VERTEX, or a node ID, "\
                      "or a list of point coordinates and not '%s'"%aThreshold
        elif CritType == FT_ElemGeomType:
            # Check the Threshold
            try:
                aCriterion.Threshold = self.EnumToLong(aThreshold)
                assert( aThreshold in SMESH.GeometryType._items )
            except:
                if isinstance(aThreshold, int):
                    aCriterion.Threshold = aThreshold
                else:
                    raise TypeError, "The Threshold should be an integer or SMESH.GeometryType."
                pass
            pass
        elif CritType == FT_EntityType:
            # Check the Threshold
            try:
                aCriterion.Threshold = self.EnumToLong(aThreshold)
                assert( aThreshold in SMESH.EntityType._items )
            except:
                if isinstance(aThreshold, int):
                    aCriterion.Threshold = aThreshold
                else:
                    raise TypeError, "The Threshold should be an integer or SMESH.EntityType."
                pass
            pass
        
        elif CritType == FT_GroupColor:
            # Check the Threshold
            try:
                aCriterion.ThresholdStr = self.ColorToString(aThreshold)
            except:
                raise TypeError, "The threshold value should be of SALOMEDS.Color type"
            pass
        elif CritType in [FT_FreeBorders, FT_FreeEdges, FT_FreeNodes, FT_FreeFaces,
                          FT_LinearOrQuadratic, FT_BadOrientedVolume,
                          FT_BareBorderFace, FT_BareBorderVolume,
                          FT_OverConstrainedFace, FT_OverConstrainedVolume,
                          FT_EqualNodes,FT_EqualEdges,FT_EqualFaces,FT_EqualVolumes ]:
            # At this point the Threshold is unnecessary
            if aThreshold ==  FT_LogicalNOT:
                aCriterion.UnaryOp = self.EnumToLong(FT_LogicalNOT)
            elif aThreshold in [FT_LogicalAND, FT_LogicalOR]:
                aCriterion.BinaryOp = aThreshold
        else:
            # Check Threshold
            try:
                aThreshold = float(aThreshold)
                aCriterion.Threshold = aThreshold
            except:
                raise TypeError, "The Threshold should be a number."
                return None

        if Threshold ==  FT_LogicalNOT or UnaryOp ==  FT_LogicalNOT:
            aCriterion.UnaryOp = self.EnumToLong(FT_LogicalNOT)

        if Threshold in [FT_LogicalAND, FT_LogicalOR]:
            aCriterion.BinaryOp = self.EnumToLong(Threshold)

        if UnaryOp in [FT_LogicalAND, FT_LogicalOR]:
            aCriterion.BinaryOp = self.EnumToLong(UnaryOp)

        if BinaryOp in [FT_LogicalAND, FT_LogicalOR]:
            aCriterion.BinaryOp = self.EnumToLong(BinaryOp)

        return aCriterion

    ## Creates a filter with the given parameters
    #  @param elementType the type of elements (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
    #  @param CritType the type of criterion (SMESH.FT_Taper, SMESH.FT_Area, etc.)
    #          Type SMESH.FunctorType._items in the Python Console to see all values.
    #          Note that the items starting from FT_LessThan are not suitable for CritType.
    #  @param Compare  belongs to {SMESH.FT_LessThan, SMESH.FT_MoreThan, SMESH.FT_EqualTo}
    #  @param Threshold the threshold value (range of ids as string, shape, numeric)
    #  @param UnaryOp  SMESH.FT_LogicalNOT or SMESH.FT_Undefined
    #  @param Tolerance the tolerance used by SMESH.FT_BelongToGeom, SMESH.FT_BelongToSurface,
    #         SMESH.FT_LyingOnGeom, SMESH.FT_CoplanarFaces and SMESH.FT_EqualNodes criteria
    #  @param mesh the mesh to initialize the filter with
    #  @return SMESH_Filter
    #
    #  <a href="../tui_filters_page.html#tui_filters">Example of Filters usage</a>
    #  @ingroup l1_controls
    def GetFilter(self,elementType,
                  CritType=FT_Undefined,
                  Compare=FT_EqualTo,
                  Threshold="",
                  UnaryOp=FT_Undefined,
                  Tolerance=1e-07,
                  mesh=None):
        aCriterion = self.GetCriterion(elementType, CritType, Compare, Threshold, UnaryOp, FT_Undefined,Tolerance)
        aFilterMgr = self.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aCriteria = []
        aCriteria.append(aCriterion)
        aFilter.SetCriteria(aCriteria)
        if mesh:
            if isinstance( mesh, Mesh ): aFilter.SetMesh( mesh.GetMesh() )
            else                       : aFilter.SetMesh( mesh )
        aFilterMgr.UnRegister()
        return aFilter

    ## Creates a filter from criteria
    #  @param criteria a list of criteria
    #  @param binOp binary operator used when binary operator of criteria is undefined
    #  @return SMESH_Filter
    #
    #  <a href="../tui_filters_page.html#tui_filters">Example of Filters usage</a>
    #  @ingroup l1_controls
    def GetFilterFromCriteria(self,criteria, binOp=SMESH.FT_LogicalAND):
        for i in range( len( criteria ) - 1 ):
            if criteria[i].BinaryOp == self.EnumToLong( SMESH.FT_Undefined ):
                criteria[i].BinaryOp = self.EnumToLong( binOp )
        aFilterMgr = self.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aFilter.SetCriteria(criteria)
        aFilterMgr.UnRegister()
        return aFilter

    ## Creates a numerical functor by its type
    #  @param theCriterion functor type - an item of SMESH.FunctorType enumeration.
    #          Type SMESH.FunctorType._items in the Python Console to see all items.
    #          Note that not all items correspond to numerical functors.
    #  @return SMESH_NumericalFunctor
    #  @ingroup l1_controls
    def GetFunctor(self,theCriterion):
        if isinstance( theCriterion, SMESH._objref_NumericalFunctor ):
            return theCriterion
        aFilterMgr = self.CreateFilterManager()
        functor = None
        if theCriterion == FT_AspectRatio:
            functor = aFilterMgr.CreateAspectRatio()
        elif theCriterion == FT_AspectRatio3D:
            functor = aFilterMgr.CreateAspectRatio3D()
        elif theCriterion == FT_Warping:
            functor = aFilterMgr.CreateWarping()
        elif theCriterion == FT_MinimumAngle:
            functor = aFilterMgr.CreateMinimumAngle()
        elif theCriterion == FT_Taper:
            functor = aFilterMgr.CreateTaper()
        elif theCriterion == FT_Skew:
            functor = aFilterMgr.CreateSkew()
        elif theCriterion == FT_Area:
            functor = aFilterMgr.CreateArea()
        elif theCriterion == FT_Volume3D:
            functor = aFilterMgr.CreateVolume3D()
        elif theCriterion == FT_MaxElementLength2D:
            functor = aFilterMgr.CreateMaxElementLength2D()
        elif theCriterion == FT_MaxElementLength3D:
            functor = aFilterMgr.CreateMaxElementLength3D()
        elif theCriterion == FT_MultiConnection:
            functor = aFilterMgr.CreateMultiConnection()
        elif theCriterion == FT_MultiConnection2D:
            functor = aFilterMgr.CreateMultiConnection2D()
        elif theCriterion == FT_Length:
            functor = aFilterMgr.CreateLength()
        elif theCriterion == FT_Length2D:
            functor = aFilterMgr.CreateLength2D()
        elif theCriterion == FT_NodeConnectivityNumber:
            functor = aFilterMgr.CreateNodeConnectivityNumber()
        elif theCriterion == FT_BallDiameter:
            functor = aFilterMgr.CreateBallDiameter()
        else:
            print "Error: given parameter is not numerical functor type."
        aFilterMgr.UnRegister()
        return functor

    ## Creates hypothesis
    #  @param theHType mesh hypothesis type (string)
    #  @param theLibName mesh plug-in library name
    #  @return created hypothesis instance
    def CreateHypothesis(self, theHType, theLibName="libStdMeshersEngine.so"):
        hyp = SMESH._objref_SMESH_Gen.CreateHypothesis(self, theHType, theLibName )

        if isinstance( hyp, SMESH._objref_SMESH_Algo ):
            return hyp

        # wrap hypothesis methods
        #print "HYPOTHESIS", theHType
        for meth_name in dir( hyp.__class__ ):
            if not meth_name.startswith("Get") and \
               not meth_name in dir ( SMESH._objref_SMESH_Hypothesis ):
                method = getattr ( hyp.__class__, meth_name )
                if callable(method):
                    setattr( hyp, meth_name, hypMethodWrapper( hyp, method ))

        return hyp

    ## Gets the mesh statistic
    #  @return dictionary "element type" - "count of elements"
    #  @ingroup l1_meshinfo
    def GetMeshInfo(self, obj):
        if isinstance( obj, Mesh ):
            obj = obj.GetMesh()
        d = {}
        if hasattr(obj, "GetMeshInfo"):
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
        unRegister = genObjUnRegister()
        if id1 != 0:
            m = src1.GetMesh()
            e = m.GetMeshEditor()
            if isElem1:
                src1 = e.MakeIDSource([id1], SMESH.FACE)
            else:
                src1 = e.MakeIDSource([id1], SMESH.NODE)
            unRegister.set( src1 )
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
                unRegister.set( src2 )
                pass
            pass
        aMeasurements = self.CreateMeasurements()
        unRegister.set( aMeasurements )
        result = aMeasurements.MinDistance(src1, src2)
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
        aMeasurements.UnRegister()
        return result

    ## Get sum of lengths of all 1D elements in the mesh object.
    #  @param obj mesh, submesh or group
    #  @return sum of lengths of all 1D elements
    #  @ingroup l1_measurements
    def GetLength(self, obj):
        if isinstance(obj, Mesh): obj = obj.mesh
        if isinstance(obj, Mesh_Algorithm): obj = obj.GetSubMesh()
        aMeasurements = self.CreateMeasurements()
        value = aMeasurements.Length(obj)
        aMeasurements.UnRegister()
        return value

    ## Get sum of areas of all 2D elements in the mesh object.
    #  @param obj mesh, submesh or group
    #  @return sum of areas of all 2D elements
    #  @ingroup l1_measurements
    def GetArea(self, obj):
        if isinstance(obj, Mesh): obj = obj.mesh
        if isinstance(obj, Mesh_Algorithm): obj = obj.GetSubMesh()
        aMeasurements = self.CreateMeasurements()
        value = aMeasurements.Area(obj)
        aMeasurements.UnRegister()
        return value

    ## Get sum of volumes of all 3D elements in the mesh object.
    #  @param obj mesh, submesh or group
    #  @return sum of volumes of all 3D elements
    #  @ingroup l1_measurements
    def GetVolume(self, obj):
        if isinstance(obj, Mesh): obj = obj.mesh
        if isinstance(obj, Mesh_Algorithm): obj = obj.GetSubMesh()
        aMeasurements = self.CreateMeasurements()
        value = aMeasurements.Volume(obj)
        aMeasurements.UnRegister()
        return value

    pass # end of class smeshBuilder

import omniORB
#Registering the new proxy for SMESH_Gen
omniORB.registerObjref(SMESH._objref_SMESH_Gen._NP_RepositoryId, smeshBuilder)

## Create a new smeshBuilder instance.The smeshBuilder class provides the Python
#  interface to create or load meshes.
#
#  Typical use is:
#  \code
#    import salome
#    salome.salome_init()
#    from salome.smesh import smeshBuilder
#    smesh = smeshBuilder.New(salome.myStudy)
#  \endcode
#  @param  study     SALOME study, generally obtained by salome.myStudy.
#  @param  instance  CORBA proxy of SMESH Engine. If None, the default Engine is used.
#  @return smeshBuilder instance

def New( study, instance=None):
    """
    Create a new smeshBuilder instance.The smeshBuilder class provides the Python
    interface to create or load meshes.

    Typical use is:
        import salome
        salome.salome_init()
        from salome.smesh import smeshBuilder
        smesh = smeshBuilder.New(salome.myStudy)

    Parameters:
        study     SALOME study, generally obtained by salome.myStudy.
        instance  CORBA proxy of SMESH Engine. If None, the default Engine is used.
    Returns:
        smeshBuilder instance
    """
    global engine
    global smeshInst
    global doLcc
    engine = instance
    if engine is None:
      doLcc = True
    smeshInst = smeshBuilder()
    assert isinstance(smeshInst,smeshBuilder), "Smesh engine class is %s but should be smeshBuilder.smeshBuilder. Import salome.smesh.smeshBuilder before creating the instance."%smeshInst.__class__
    smeshInst.init_smesh(study)
    return smeshInst


# Public class: Mesh
# ==================

## This class allows defining and managing a mesh.
#  It has a set of methods to build a mesh on the given geometry, including the definition of sub-meshes.
#  It also has methods to define groups of mesh elements, to modify a mesh (by addition of
#  new nodes and elements and by changing the existing entities), to get information
#  about a mesh and to export a mesh into different formats.
class Mesh:
    __metaclass__ = MeshMeta

    geom = 0
    mesh = 0
    editor = 0

    ## Constructor
    #
    #  Creates a mesh on the shape \a obj (or an empty mesh if \a obj is equal to 0) and
    #  sets the GUI name of this mesh to \a name.
    #  @param smeshpyD an instance of smeshBuilder class
    #  @param geompyD an instance of geomBuilder class
    #  @param obj Shape to be meshed or SMESH_Mesh object
    #  @param name Study name of the mesh
    #  @ingroup l2_construct
    def __init__(self, smeshpyD, geompyD, obj=0, name=0):
        self.smeshpyD=smeshpyD
        self.geompyD=geompyD
        if obj is None:
            obj = 0
        objHasName = False
        if obj != 0:
            if isinstance(obj, geomBuilder.GEOM._objref_GEOM_Object):
                self.geom = obj
                objHasName = True
                # publish geom of mesh (issue 0021122)
                if not self.geom.GetStudyEntry() and smeshpyD.GetCurrentStudy():
                    objHasName = False
                    studyID = smeshpyD.GetCurrentStudy()._get_StudyId()
                    if studyID != geompyD.myStudyId:
                        geompyD.init_geom( smeshpyD.GetCurrentStudy())
                        pass
                    if name:
                        geo_name = name + " shape"
                    else:
                        geo_name = "%s_%s to mesh"%(self.geom.GetShapeType(), id(self.geom)%100)
                    geompyD.addToStudy( self.geom, geo_name )
                self.SetMesh( self.smeshpyD.CreateMesh(self.geom) )

            elif isinstance(obj, SMESH._objref_SMESH_Mesh):
                self.SetMesh(obj)
        else:
            self.SetMesh( self.smeshpyD.CreateEmptyMesh() )
        if name:
            self.smeshpyD.SetName(self.mesh, name)
        elif objHasName:
            self.smeshpyD.SetName(self.mesh, GetName(obj)) # + " mesh"

        if not self.geom:
            self.geom = self.mesh.GetShapeToMesh()

        self.editor   = self.mesh.GetMeshEditor()
        self.functors = [None] * SMESH.FT_Undefined._v

        # set self to algoCreator's
        for attrName in dir(self):
            attr = getattr( self, attrName )
            if isinstance( attr, algoCreator ):
                setattr( self, attrName, attr.copy( self ))
                pass
            pass
        pass

    ## Destructor. Clean-up resources
    def __del__(self):
        if self.mesh:
            #self.mesh.UnRegister()
            pass
        pass
        
    ## Initializes the Mesh object from an instance of SMESH_Mesh interface
    #  @param theMesh a SMESH_Mesh object
    #  @ingroup l2_construct
    def SetMesh(self, theMesh):
        # do not call Register() as this prevents mesh servant deletion at closing study
        #if self.mesh: self.mesh.UnRegister()
        self.mesh = theMesh
        if self.mesh:
            #self.mesh.Register()
            self.geom = self.mesh.GetShapeToMesh()
        pass

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
    #  @param geom a geometrical object (shape)
    #  @param name a name for the submesh
    #  @return an object of type SMESH_SubMesh, representing a part of mesh, which lies on the given shape
    #  @ingroup l2_submeshes
    def GetSubMesh(self, geom, name):
        AssureGeomPublished( self, geom, name )
        submesh = self.mesh.GetSubMesh( geom, name )
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

    ## Loads mesh from the study after opening the study
    def Load(self):
        self.mesh.Load()

    ## Returns true if the hypotheses are defined well
    #  @param theSubObject a sub-shape of a mesh shape
    #  @return True or False
    #  @ingroup l2_construct
    def IsReadyToCompute(self, theSubObject):
        return self.smeshpyD.IsReadyToCompute(self.mesh, theSubObject)

    ## Returns errors of hypotheses definition.
    #  The list of errors is empty if everything is OK.
    #  @param theSubObject a sub-shape of a mesh shape
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
    #  or, if the mesh is not based on any shape, basing on deimension of elements
    #  @return mesh dimension as an integer value [0,3]
    #  @ingroup l1_auxiliary
    def MeshDimension(self):
        if self.mesh.HasShapeToMesh():
            shells = self.geompyD.SubShapeAllIDs( self.geom, self.geompyD.ShapeType["SOLID"] )
            if len( shells ) > 0 :
                return 3
            elif self.geompyD.NumberOfFaces( self.geom ) > 0 :
                return 2
            elif self.geompyD.NumberOfEdges( self.geom ) > 0 :
                return 1
            else:
                return 0;
        else:
            if self.NbVolumes() > 0: return 3
            if self.NbFaces()   > 0: return 2
            if self.NbEdges()   > 0: return 1
        return 0

    ## Evaluates size of prospective mesh on a shape
    #  @return a list where i-th element is a number of elements of i-th SMESH.EntityType
    #  To know predicted number of e.g. edges, inquire it this way
    #  Evaluate()[ EnumToLong( Entity_Edge )]
    def Evaluate(self, geom=0):
        if geom == 0 or not isinstance(geom, geomBuilder.GEOM._objref_GEOM_Object):
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
    #  @param refresh if @c True, Object browser is automatically updated (when running in GUI)
    #  @return True or False
    #  @ingroup l2_construct
    def Compute(self, geom=0, discardModifs=False, refresh=False):
        if geom == 0 or not isinstance(geom, geomBuilder.GEOM._objref_GEOM_Object):
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
            shapeText = ""
            for err in computeErrors:
                if self.mesh.HasShapeToMesh():
                    shapeText = " on %s" % self.GetSubShapeName( err.subShapeID )
                errText = ""
                stdErrors = ["OK",                   #COMPERR_OK
                             "Invalid input mesh",   #COMPERR_BAD_INPUT_MESH
                             "std::exception",       #COMPERR_STD_EXCEPTION
                             "OCC exception",        #COMPERR_OCC_EXCEPTION
                             "..",                   #COMPERR_SLM_EXCEPTION
                             "Unknown exception",    #COMPERR_EXCEPTION
                             "Memory allocation problem", #COMPERR_MEMORY_PB
                             "Algorithm failed",     #COMPERR_ALGO_FAILED
                             "Unexpected geometry",  #COMPERR_BAD_SHAPE
                             "Warning",              #COMPERR_WARNING
                             "Computation cancelled",#COMPERR_CANCELED
                             "No mesh on sub-shape"] #COMPERR_NO_MESH_ON_SHAPE
                if err.code > 0:
                    if err.code < len(stdErrors): errText = stdErrors[err.code]
                else:
                    errText = "code %s" % -err.code
                if errText: errText += ". "
                errText += err.comment
                if allReasons != "":allReasons += "\n"
                if ok:
                    allReasons += '-  "%s"%s - %s' %(err.algoName, shapeText, errText)
                else:
                    allReasons += '-  "%s" failed%s. Error: %s' %(err.algoName, shapeText, errText)
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
                elif err.state == HYP_HIDDEN_ALGO:
                    reason = ('%s %sD algorithm "%s" is ignored due to presence of a %s '
                              'algorithm of upper dimension generating %sD mesh'
                              % ( glob, dim, name, glob, dim ))
                else:
                    reason = ("For unknown reason. "
                              "Developer, revise Mesh.Compute() implementation in smeshBuilder.py!")
                    pass
                if allReasons != "":allReasons += "\n"
                allReasons += "-  " + reason
                pass
            if not ok or allReasons != "":
                msg = '"' + GetName(self.mesh) + '"'
                if ok: msg += " has been computed with warnings"
                else:  msg += " has not been computed"
                if allReasons != "": msg += ":"
                else:                msg += "."
                print msg
                print allReasons
            pass
        if salome.sg.hasDesktop() and self.mesh.GetStudyId() >= 0:
            if not isinstance( refresh, list): # not a call from subMesh.Compute()
                smeshgui = salome.ImportComponentGUI("SMESH")
                smeshgui.Init(self.mesh.GetStudyId())
                smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), ok, (self.NbNodes()==0) )
                if refresh: salome.sg.updateObjBrowser(True)

        return ok

    ## Return a list of error messages (SMESH.ComputeError) of the last Compute()
    def GetComputeErrors(self, shape=0 ):
        if shape == 0:
            shape = self.mesh.GetShapeToMesh()
        return self.smeshpyD.GetComputeErrors( self.mesh, shape )

    ## Return a name of a sub-shape by its ID
    #  @param subShapeID a unique ID of a sub-shape
    #  @return a string describing the sub-shape; possible variants:
    #  - "Face_12"    (published sub-shape)
    #  - FACE #3      (not published sub-shape)
    #  - sub-shape #3 (invalid sub-shape ID)
    #  - #3           (error in this function)
    def GetSubShapeName(self, subShapeID ):
        if not self.mesh.HasShapeToMesh():
            return ""
        try:
            shapeText = ""
            mainIOR  = salome.orb.object_to_string( self.GetShape() )
            for sname in salome.myStudyManager.GetOpenStudies():
                s = salome.myStudyManager.GetStudyByName(sname)
                if not s: continue
                mainSO = s.FindObjectIOR(mainIOR)
                if not mainSO: continue
                if subShapeID == 1:
                    shapeText = '"%s"' % mainSO.GetName()
                subIt = s.NewChildIterator(mainSO)
                while subIt.More():
                    subSO = subIt.Value()
                    subIt.Next()
                    obj = subSO.GetObject()
                    if not obj: continue
                    go = obj._narrow( geomBuilder.GEOM._objref_GEOM_Object )
                    if not go: continue
                    try:
                        ids = self.geompyD.GetSubShapeID( self.GetShape(), go )
                    except:
                        continue
                    if ids == subShapeID:
                        shapeText = '"%s"' % subSO.GetName()
                        break
            if not shapeText:
                shape = self.geompyD.GetSubShape( self.GetShape(), [subShapeID])
                if shape:
                    shapeText = '%s #%s' % (shape.GetShapeType(), subShapeID)
                else:
                    shapeText = 'sub-shape #%s' % (subShapeID)
        except:
            shapeText = "#%s" % (subShapeID)
        return shapeText

    ## Return a list of sub-shapes meshing of which failed, grouped into GEOM groups by
    #  error of an algorithm
    #  @param publish if @c True, the returned groups will be published in the study
    #  @return a list of GEOM groups each named after a failed algorithm
    def GetFailedShapes(self, publish=False):

        algo2shapes = {}
        computeErrors = self.smeshpyD.GetComputeErrors( self.mesh, self.GetShape() )
        for err in computeErrors:
            shape = self.geompyD.GetSubShape( self.GetShape(), [err.subShapeID])
            if not shape: continue
            if err.algoName in algo2shapes:
                algo2shapes[ err.algoName ].append( shape )
            else:
                algo2shapes[ err.algoName ] = [ shape ]
            pass

        groups = []
        for algoName, shapes in algo2shapes.items():
            while shapes:
                groupType = self.smeshpyD.EnumToLong( shapes[0].GetShapeType() )
                otherTypeShapes = []
                sameTypeShapes  = []
                group = self.geompyD.CreateGroup( self.geom, groupType )
                for shape in shapes:
                    if shape.GetShapeType() == shapes[0].GetShapeType():
                        sameTypeShapes.append( shape )
                    else:
                        otherTypeShapes.append( shape )
                self.geompyD.UnionList( group, sameTypeShapes )
                if otherTypeShapes:
                    group.SetName( "%s %s" % ( algoName, shapes[0].GetShapeType() ))
                else:
                    group.SetName( algoName )
                groups.append( group )
                shapes = otherTypeShapes
            pass
        if publish:
            for group in groups:
                self.geompyD.addToStudyInFather( self.geom, group, group.GetName() )
        return groups

    ## Return sub-mesh objects list in meshing order
    #  @return list of lists of sub-meshes
    #  @ingroup l2_construct
    def GetMeshOrder(self):
        return self.mesh.GetMeshOrder()

    ## Set order in which concurrent sub-meshes sould be meshed
    #  @param submeshes list of lists of sub-meshes
    #  @ingroup l2_construct
    def SetMeshOrder(self, submeshes):
        return self.mesh.SetMeshOrder(submeshes)

    ## Removes all nodes and elements
    #  @param refresh if @c True, Object browser is automatically updated (when running in GUI)
    #  @ingroup l2_construct
    def Clear(self, refresh=False):
        self.mesh.Clear()
        if ( salome.sg.hasDesktop() and 
             salome.myStudyManager.GetStudyByID( self.mesh.GetStudyId() ) ):
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), False, True )
            if refresh: salome.sg.updateObjBrowser(True)

    ## Removes all nodes and elements of indicated shape
    #  @param refresh if @c True, Object browser is automatically updated (when running in GUI)
    #  @param geomId the ID of a sub-shape to remove elements on
    #  @ingroup l2_construct
    def ClearSubMesh(self, geomId, refresh=False):
        self.mesh.ClearSubMesh(geomId)
        if salome.sg.hasDesktop():
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), False, True )
            if refresh: salome.sg.updateObjBrowser(True)

    ## Computes a tetrahedral mesh using AutomaticLength + MEFISTO + Tetrahedron
    #  @param fineness [0.0,1.0] defines mesh fineness
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
            self.Tetrahedron()
            pass
        return self.Compute()

    ## Computes an hexahedral mesh using AutomaticLength + Quadrangle + Hexahedron
    #  @param fineness [0.0, 1.0] defines mesh fineness
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
        if isinstance( hyp, geomBuilder.GEOM._objref_GEOM_Object ):
            hyp, geom = geom, hyp
        if isinstance( hyp, Mesh_Algorithm ):
            hyp = hyp.GetAlgorithm()
            pass
        if not geom:
            geom = self.geom
            if not geom:
                geom = self.mesh.GetShapeToMesh()
            pass
        isApplicable = True
        if self.mesh.HasShapeToMesh():
            hyp_type     = hyp.GetName()
            lib_name     = hyp.GetLibName()
            # checkAll    = ( not geom.IsSame( self.mesh.GetShapeToMesh() ))
            # if checkAll and geom:
            #     checkAll = geom.GetType() == 37
            checkAll     = False
            isApplicable = self.smeshpyD.IsApplicable(hyp_type, lib_name, geom, checkAll)
        if isApplicable:
            AssureGeomPublished( self, geom, "shape for %s" % hyp.GetName())
            status = self.mesh.AddHypothesis(geom, hyp)
        else:
            status = HYP_BAD_GEOMETRY,""
        hyp_name = GetName( hyp )
        geom_name = ""
        if geom:
            geom_name = geom.GetName()
        isAlgo = hyp._narrow( SMESH_Algo )
        TreatHypoStatus( status, hyp_name, geom_name, isAlgo, self )
        return status

    ## Return True if an algorithm of hypothesis is assigned to a given shape
    #  @param hyp a hypothesis to check
    #  @param geom a subhape of mesh geometry
    #  @return True of False
    #  @ingroup l2_hypotheses
    def IsUsedHypothesis(self, hyp, geom):
        if not hyp: # or not geom
            return False
        if isinstance( hyp, Mesh_Algorithm ):
            hyp = hyp.GetAlgorithm()
            pass
        hyps = self.GetHypothesisList(geom)
        for h in hyps:
            if h.GetId() == hyp.GetId():
                return True
        return False

    ## Unassigns a hypothesis
    #  @param hyp a hypothesis to unassign
    #  @param geom a sub-shape of mesh geometry
    #  @return SMESH.Hypothesis_Status
    #  @ingroup l2_hypotheses
    def RemoveHypothesis(self, hyp, geom=0):
        if not hyp:
            return None
        if isinstance( hyp, Mesh_Algorithm ):
            hyp = hyp.GetAlgorithm()
            pass
        shape = geom
        if not shape:
            shape = self.geom
            pass
        if self.IsUsedHypothesis( hyp, shape ):
            return self.mesh.RemoveHypothesis( shape, hyp )
        hypName = GetName( hyp )
        geoName = GetName( shape )
        print "WARNING: RemoveHypothesis() failed as '%s' is not assigned to '%s' shape" % ( hypName, geoName )
        return None

    ## Gets the list of hypotheses added on a geometry
    #  @param geom a sub-shape of mesh geometry
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

   ## Exports the mesh in a file in MED format and chooses the \a version of MED format
    ## allowing to overwrite the file if it exists or add the exported data to its contents
    #  @param f is the file name
    #  @param auto_groups boolean parameter for creating/not creating
    #  the groups Group_On_All_Nodes, Group_On_All_Faces, ... ;
    #  the typical use is auto_groups=false.
    #  @param version MED format version(MED_V2_1 or MED_V2_2)
    #  @param overwrite boolean parameter for overwriting/not overwriting the file
    #  @param meshPart a part of mesh (group, sub-mesh) to export instead of the mesh
    #  @param autoDimension: if @c True (default), a space dimension of a MED mesh can be either
    #         - 1D if all mesh nodes lie on OX coordinate axis, or
    #         - 2D if all mesh nodes lie on XOY coordinate plane, or
    #         - 3D in the rest cases.
    #         If @a autoDimension is @c False, the space dimension is always 3.
    #  @param fields : list of GEOM fields defined on the shape to mesh.
    #  @param geomAssocFields : each character of this string means a need to export a 
    #         corresponding field; correspondence between fields and characters is following:
    #         - 'v' stands for _vertices_ field;
    #         - 'e' stands for _edges_ field;
    #         - 'f' stands for _faces_ field;
    #         - 's' stands for _solids_ field.
    #  @ingroup l2_impexp
    def ExportMED(self, f, auto_groups=0, version=MED_V2_2,
                  overwrite=1, meshPart=None, autoDimension=True, fields=[], geomAssocFields=''):
        if meshPart or fields or geomAssocFields:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToMED( meshPart, f, auto_groups, version, overwrite, autoDimension,
                                       fields, geomAssocFields)
        else:
            self.mesh.ExportToMEDX(f, auto_groups, version, overwrite, autoDimension)

    ## Exports the mesh in a file in SAUV format
    #  @param f is the file name
    #  @param auto_groups boolean parameter for creating/not creating
    #  the groups Group_On_All_Nodes, Group_On_All_Faces, ... ;
    #  the typical use is auto_groups=false.
    #  @ingroup l2_impexp
    def ExportSAUV(self, f, auto_groups=0):
        self.mesh.ExportSAUV(f, auto_groups)

    ## Exports the mesh in a file in DAT format
    #  @param f the file name
    #  @param meshPart a part of mesh (group, sub-mesh) to export instead of the mesh
    #  @ingroup l2_impexp
    def ExportDAT(self, f, meshPart=None):
        if meshPart:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToDAT( meshPart, f )
        else:
            self.mesh.ExportDAT(f)

    ## Exports the mesh in a file in UNV format
    #  @param f the file name
    #  @param meshPart a part of mesh (group, sub-mesh) to export instead of the mesh
    #  @ingroup l2_impexp
    def ExportUNV(self, f, meshPart=None):
        if meshPart:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToUNV( meshPart, f )
        else:
            self.mesh.ExportUNV(f)

    ## Export the mesh in a file in STL format
    #  @param f the file name
    #  @param ascii defines the file encoding
    #  @param meshPart a part of mesh (group, sub-mesh) to export instead of the mesh
    #  @ingroup l2_impexp
    def ExportSTL(self, f, ascii=1, meshPart=None):
        if meshPart:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToSTL( meshPart, f, ascii )
        else:
            self.mesh.ExportSTL(f, ascii)

    ## Exports the mesh in a file in CGNS format
    #  @param f is the file name
    #  @param overwrite boolean parameter for overwriting/not overwriting the file
    #  @param meshPart a part of mesh (group, sub-mesh) to export instead of the mesh
    #  @ingroup l2_impexp
    def ExportCGNS(self, f, overwrite=1, meshPart=None):
        unRegister = genObjUnRegister()
        if isinstance( meshPart, list ):
            meshPart = self.GetIDSource( meshPart, SMESH.ALL )
            unRegister.set( meshPart )
        if isinstance( meshPart, Mesh ):
            meshPart = meshPart.mesh
        elif not meshPart:
            meshPart = self.mesh
        self.mesh.ExportCGNS(meshPart, f, overwrite)

    ## Exports the mesh in a file in GMF format.
    #  GMF files must have .mesh extension for the ASCII format and .meshb for
    #  the bynary format. Other extensions are not allowed.
    #  @param f is the file name
    #  @param meshPart a part of mesh (group, sub-mesh) to export instead of the mesh
    #  @ingroup l2_impexp
    def ExportGMF(self, f, meshPart=None):
        unRegister = genObjUnRegister()
        if isinstance( meshPart, list ):
            meshPart = self.GetIDSource( meshPart, SMESH.ALL )
            unRegister.set( meshPart )
        if isinstance( meshPart, Mesh ):
            meshPart = meshPart.mesh
        elif not meshPart:
            meshPart = self.mesh
        self.mesh.ExportGMF(meshPart, f, True)

    ## Deprecated, used only for compatibility! Please, use ExportToMEDX() method instead.
    #  Exports the mesh in a file in MED format and chooses the \a version of MED format
    ## allowing to overwrite the file if it exists or add the exported data to its contents
    #  @param f the file name
    #  @param version values are SMESH.MED_V2_1, SMESH.MED_V2_2
    #  @param opt boolean parameter for creating/not creating
    #         the groups Group_On_All_Nodes, Group_On_All_Faces, ...
    #  @param overwrite boolean parameter for overwriting/not overwriting the file
    #  @param autoDimension: if @c True (default), a space dimension of a MED mesh can be either
    #         - 1D if all mesh nodes lie on OX coordinate axis, or
    #         - 2D if all mesh nodes lie on XOY coordinate plane, or
    #         - 3D in the rest cases.
    #
    #         If @a autoDimension is @c False, the space dimension is always 3.
    #  @ingroup l2_impexp
    def ExportToMED(self, f, version, opt=0, overwrite=1, autoDimension=True):
        self.mesh.ExportToMEDX(f, opt, version, overwrite, autoDimension)

    # Operations with groups:
    # ----------------------

    ## Creates an empty mesh group
    #  @param elementType the type of elements in the group; either of 
    #         (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
    #  @param name the name of the mesh group
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
    def CreateEmptyGroup(self, elementType, name):
        return self.mesh.CreateGroup(elementType, name)

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

    ## Creates a mesh group based on the geometrical object \a grp
    #  and gives a \a name, \n if this parameter is not defined
    #  the name is the same as the geometrical group name
    #  @param grp  a geometrical group, a vertex, an edge, a face or a solid
    #  @param name the name of the mesh group
    #  @param typ  the type of elements in the group; either of 
    #         (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME). If not set, it is
    #         automatically detected by the type of the geometry
    #  @return SMESH_GroupOnGeom
    #  @ingroup l2_grps_create
    def GroupOnGeom(self, grp, name="", typ=None):
        AssureGeomPublished( self, grp, name )
        if name == "":
            name = grp.GetName()
        if not typ:
            typ = self._groupTypeFromShape( grp )
        return self.mesh.CreateGroupFromGEOM(typ, name, grp)

    ## Pivate method to get a type of group on geometry
    def _groupTypeFromShape( self, shape ):
        tgeo = str(shape.GetShapeType())
        if tgeo == "VERTEX":
            typ = NODE
        elif tgeo == "EDGE":
            typ = EDGE
        elif tgeo == "FACE" or tgeo == "SHELL":
            typ = FACE
        elif tgeo == "SOLID" or tgeo == "COMPSOLID":
            typ = VOLUME
        elif tgeo == "COMPOUND":
            sub = self.geompyD.SubShapeAll( shape, self.geompyD.ShapeType["SHAPE"])
            if not sub:
                raise ValueError,"_groupTypeFromShape(): empty geometric group or compound '%s'" % GetName(shape)
            return self._groupTypeFromShape( sub[0] )
        else:
            raise ValueError, \
                  "_groupTypeFromShape(): invalid geometry '%s'" % GetName(shape)
        return typ

    ## Creates a mesh group with given \a name based on the \a filter which
    ## is a special type of group dynamically updating it's contents during
    ## mesh modification
    #  @param typ  the type of elements in the group; either of 
    #         (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME).
    #  @param name the name of the mesh group
    #  @param filter the filter defining group contents
    #  @return SMESH_GroupOnFilter
    #  @ingroup l2_grps_create
    def GroupOnFilter(self, typ, name, filter):
        return self.mesh.CreateGroupFromFilter(typ, name, filter)

    ## Creates a mesh group by the given ids of elements
    #  @param groupName the name of the mesh group
    #  @param elementType the type of elements in the group; either of 
    #         (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME).
    #  @param elemIDs either the list of ids, group, sub-mesh, or filter
    #  @return SMESH_Group
    #  @ingroup l2_grps_create
    def MakeGroupByIds(self, groupName, elementType, elemIDs):
        group = self.mesh.CreateGroup(elementType, groupName)
        if hasattr( elemIDs, "GetIDs" ):
            if hasattr( elemIDs, "SetMesh" ):
                elemIDs.SetMesh( self.GetMesh() )
            group.AddFrom( elemIDs )
        else:
            group.Add(elemIDs)
        return group

    ## Creates a mesh group by the given conditions
    #  @param groupName the name of the mesh group
    #  @param elementType the type of elements(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
    #  @param CritType the type of criterion (SMESH.FT_Taper, SMESH.FT_Area, etc.)
    #          Type SMESH.FunctorType._items in the Python Console to see all values.
    #          Note that the items starting from FT_LessThan are not suitable for CritType.
    #  @param Compare  belongs to {SMESH.FT_LessThan, SMESH.FT_MoreThan, SMESH.FT_EqualTo}
    #  @param Threshold the threshold value (range of ids as string, shape, numeric)
    #  @param UnaryOp  SMESH.FT_LogicalNOT or SMESH.FT_Undefined
    #  @param Tolerance the tolerance used by SMESH.FT_BelongToGeom, SMESH.FT_BelongToSurface,
    #         SMESH.FT_LyingOnGeom, SMESH.FT_CoplanarFaces criteria
    #  @return SMESH_GroupOnFilter
    #  @ingroup l2_grps_create
    def MakeGroup(self,
                  groupName,
                  elementType,
                  CritType=FT_Undefined,
                  Compare=FT_EqualTo,
                  Threshold="",
                  UnaryOp=FT_Undefined,
                  Tolerance=1e-07):
        aCriterion = self.smeshpyD.GetCriterion(elementType, CritType, Compare, Threshold, UnaryOp, FT_Undefined,Tolerance)
        group = self.MakeGroupByCriterion(groupName, aCriterion)
        return group

    ## Creates a mesh group by the given criterion
    #  @param groupName the name of the mesh group
    #  @param Criterion the instance of Criterion class
    #  @return SMESH_GroupOnFilter
    #  @ingroup l2_grps_create
    def MakeGroupByCriterion(self, groupName, Criterion):
        return self.MakeGroupByCriteria( groupName, [Criterion] )

    ## Creates a mesh group by the given criteria (list of criteria)
    #  @param groupName the name of the mesh group
    #  @param theCriteria the list of criteria
    #  @param binOp binary operator used when binary operator of criteria is undefined
    #  @return SMESH_GroupOnFilter
    #  @ingroup l2_grps_create
    def MakeGroupByCriteria(self, groupName, theCriteria, binOp=SMESH.FT_LogicalAND):
        aFilter = self.smeshpyD.GetFilterFromCriteria( theCriteria, binOp )
        group = self.MakeGroupByFilter(groupName, aFilter)
        return group

    ## Creates a mesh group by the given filter
    #  @param groupName the name of the mesh group
    #  @param theFilter the instance of Filter class
    #  @return SMESH_GroupOnFilter
    #  @ingroup l2_grps_create
    def MakeGroupByFilter(self, groupName, theFilter):
        #group = self.CreateEmptyGroup(theFilter.GetElementType(), groupName)
        #theFilter.SetMesh( self.mesh )
        #group.AddFrom( theFilter )
        group = self.GroupOnFilter( theFilter.GetElementType(), groupName, theFilter )
        return group

    ## Removes a group
    #  @ingroup l2_grps_delete
    def RemoveGroup(self, group):
        self.mesh.RemoveGroup(group)

    ## Removes a group with its contents
    #  @ingroup l2_grps_delete
    def RemoveGroupWithContents(self, group):
        self.mesh.RemoveGroupWithContents(group)

    ## Gets the list of groups existing in the mesh in the order
    #  of creation (starting from the oldest one)
    #  @param elemType type of elements the groups contain; either of 
    #         (SMESH.ALL, SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME);
    #         by default groups of elements of all types are returned
    #  @return a sequence of SMESH_GroupBase
    #  @ingroup l2_grps_create
    def GetGroups(self, elemType = SMESH.ALL):
        groups = self.mesh.GetGroups()
        if elemType == SMESH.ALL:
            return groups
        typedGroups = []
        for g in groups:
            if g.GetType() == elemType:
                typedGroups.append( g )
                pass
            pass
        return typedGroups

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

    ## Finds groups by name and type
    #  @param name name of the group of interest
    #  @param elemType type of elements the groups contain; either of 
    #         (SMESH.ALL, SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME);
    #         by default one group of any type of elements is returned
    #         if elemType == SMESH.ALL then all groups of any type are returned
    #  @return a list of SMESH_GroupBase's
    #  @ingroup l2_grps_create
    def GetGroupByName(self, name, elemType = None):
        groups = []
        for group in self.GetGroups():
            if group.GetName() == name:
                if elemType is None:
                    return [group]
                if ( elemType == SMESH.ALL or 
                     group.GetType() == elemType ):
                    groups.append( group )
        return groups

    ## Produces a union of two groups.
    #  A new group is created. All mesh elements that are
    #  present in the initial groups are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def UnionGroups(self, group1, group2, name):
        return self.mesh.UnionGroups(group1, group2, name)

    ## Produces a union list of groups.
    #  New group is created. All mesh elements that are present in
    #  initial groups are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def UnionListOfGroups(self, groups, name):
      return self.mesh.UnionListOfGroups(groups, name)

    ## Prodices an intersection of two groups.
    #  A new group is created. All mesh elements that are common
    #  for the two initial groups are added to the new one.
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def IntersectGroups(self, group1, group2, name):
        return self.mesh.IntersectGroups(group1, group2, name)

    ## Produces an intersection of groups.
    #  New group is created. All mesh elements that are present in all
    #  initial groups simultaneously are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def IntersectListOfGroups(self, groups, name):
      return self.mesh.IntersectListOfGroups(groups, name)

    ## Produces a cut of two groups.
    #  A new group is created. All mesh elements that are present in
    #  the main group but are not present in the tool group are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def CutGroups(self, main_group, tool_group, name):
        return self.mesh.CutGroups(main_group, tool_group, name)

    ## Produces a cut of groups.
    #  A new group is created. All mesh elements that are present in main groups
    #  but do not present in tool groups are added to the new one
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def CutListOfGroups(self, main_groups, tool_groups, name):
        return self.mesh.CutListOfGroups(main_groups, tool_groups, name)

    ##
    #  Create a standalone group of entities basing on nodes of other groups.
    #  \param groups - list of reference groups, sub-meshes or filters, of any type.
    #  \param elemType - a type of elements to include to the new group; either of 
    #         (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME).
    #  \param name - a name of the new group.
    #  \param nbCommonNodes - a criterion of inclusion of an element to the new group
    #         basing on number of element nodes common with reference \a groups.
    #         Meaning of possible values are:
    #         - SMESH.ALL_NODES - include if all nodes are common,
    #         - SMESH.MAIN - include if all corner nodes are common (meaningful for a quadratic mesh),
    #         - SMESH.AT_LEAST_ONE - include if one or more node is common,
    #         - SMEHS.MAJORITY - include if half of nodes or more are common.
    #  \param underlyingOnly - if \c True (default), an element is included to the
    #         new group provided that it is based on nodes of an element of \a groups;
    #         in this case the reference \a groups are supposed to be of higher dimension
    #         than \a elemType, which can be useful for example to get all faces lying on
    #         volumes of the reference \a groups.
    #  @return an instance of SMESH_Group
    #  @ingroup l2_grps_operon
    def CreateDimGroup(self, groups, elemType, name,
                       nbCommonNodes = SMESH.ALL_NODES, underlyingOnly = True):
        if isinstance( groups, SMESH._objref_SMESH_IDSource ):
            groups = [groups]
        return self.mesh.CreateDimGroup(groups, elemType, name, nbCommonNodes, underlyingOnly)


    ## Convert group on geom into standalone group
    #  @ingroup l2_grps_edit
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
        return self.editor

    ## Wrap a list of IDs of elements or nodes into SMESH_IDSource which
    #  can be passed as argument to a method accepting mesh, group or sub-mesh
    #  @param ids list of IDs
    #  @param elemType type of elements; this parameter is used to distinguish
    #         IDs of nodes from IDs of elements; by default ids are treated as
    #         IDs of elements; use SMESH.NODE if ids are IDs of nodes.
    #  @return an instance of SMESH_IDSource
    #  @warning call UnRegister() for the returned object as soon as it is no more useful:
    #          idSrc = mesh.GetIDSource( [1,3,5], SMESH.NODE )
    #          mesh.DoSomething( idSrc )
    #          idSrc.UnRegister()
    #  @ingroup l1_auxiliary
    def GetIDSource(self, ids, elemType = SMESH.ALL):
        if isinstance( ids, int ):
            ids = [ids]
        return self.editor.MakeIDSource(ids, elemType)


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

    ## Returns the number of ball discrete elements in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbBalls(self):
        return self.mesh.NbBalls()

    ## Returns the number of edges in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbEdges(self):
        return self.mesh.NbEdges()

    ## Returns the number of edges with the given order in the mesh
    #  @param elementOrder the order of elements:
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
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
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
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
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbTrianglesOfOrder(self, elementOrder):
        return self.mesh.NbTrianglesOfOrder(elementOrder)

    ## Returns the number of biquadratic triangles in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbBiQuadTriangles(self):
        return self.mesh.NbBiQuadTriangles()

    ## Returns the number of quadrangles in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbQuadrangles(self):
        return self.mesh.NbQuadrangles()

    ## Returns the number of quadrangles with the given order in the mesh
    #  @param elementOrder the order of elements:
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbQuadranglesOfOrder(self, elementOrder):
        return self.mesh.NbQuadranglesOfOrder(elementOrder)

    ## Returns the number of biquadratic quadrangles in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbBiQuadQuadrangles(self):
        return self.mesh.NbBiQuadQuadrangles()

    ## Returns the number of polygons of given order in the mesh
    #  @param elementOrder the order of elements:
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPolygons(self, elementOrder = SMESH.ORDER_ANY):
        return self.mesh.NbPolygonsOfOrder(elementOrder)

    ## Returns the number of volumes in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbVolumes(self):
        return self.mesh.NbVolumes()

    ## Returns the number of volumes with the given order in the mesh
    #  @param elementOrder  the order of elements:
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
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
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
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
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbHexasOfOrder(self, elementOrder):
        return self.mesh.NbHexasOfOrder(elementOrder)

    ## Returns the number of triquadratic hexahedrons in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbTriQuadraticHexas(self):
        return self.mesh.NbTriQuadraticHexas()

    ## Returns the number of pyramids in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPyramids(self):
        return self.mesh.NbPyramids()

    ## Returns the number of pyramids with the given order in the mesh
    #  @param elementOrder  the order of elements:
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
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
    #         SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbPrismsOfOrder(self, elementOrder):
        return self.mesh.NbPrismsOfOrder(elementOrder)

    ## Returns the number of hexagonal prisms in the mesh
    #  @return an integer value
    #  @ingroup l1_meshinfo
    def NbHexagonalPrisms(self):
        return self.mesh.NbHexagonalPrisms()

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
    #  @param elementType  the required type of elements, either of
    #         (SMESH.NODE, SMESH.EDGE, SMESH.FACE or SMESH.VOLUME)
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
    #          Type SMESH.ElementType._items in the Python Console to see all possible values.
    #  @ingroup l1_meshinfo
    def GetElementType(self, id, iselem=True):
        return self.mesh.GetElementType(id, iselem)

    ## Returns the geometric type of mesh element
    #  @return the value from SMESH::EntityType enumeration
    #          Type SMESH.EntityType._items in the Python Console to see all possible values.
    #  @ingroup l1_meshinfo
    def GetElementGeomType(self, id):
        return self.mesh.GetElementGeomType(id)

    ## Returns the shape type of mesh element
    #  @return the value from SMESH::GeometryType enumeration.
    #          Type SMESH.GeometryType._items in the Python Console to see all possible values.
    #  @ingroup l1_meshinfo
    def GetElementShape(self, id):
        return self.mesh.GetElementShape(id)

    ## Returns the list of submesh elements IDs
    #  @param Shape a geom object(sub-shape)
    #         Shape must be the sub-shape of a ShapeToMesh()
    #  @return the list of integer values
    #  @ingroup l1_meshinfo
    def GetSubMeshElementsId(self, Shape):
        if isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshElementsId(ShapeID)

    ## Returns the list of submesh nodes IDs
    #  @param Shape a geom object(sub-shape)
    #         Shape must be the sub-shape of a ShapeToMesh()
    #  @param all If true, gives all nodes of submesh elements, otherwise gives only submesh nodes
    #  @return the list of integer values
    #  @ingroup l1_meshinfo
    def GetSubMeshNodesId(self, Shape, all):
        if isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshNodesId(ShapeID, all)

    ## Returns type of elements on given shape
    #  @param Shape a geom object(sub-shape)
    #         Shape must be a sub-shape of a ShapeToMesh()
    #  @return element type
    #  @ingroup l1_meshinfo
    def GetSubMeshElementType(self, Shape):
        if isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
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

    ## @brief Returns the position of an element on the shape
    #  @return SMESH::ElementPosition
    #  @ingroup l1_meshinfo
    def GetElementPosition(self,ElemID):
        return self.mesh.GetElementPosition(ElemID)

    ## Returns the ID of the shape, on which the given node was generated.
    #  @return an integer value > 0 or -1 if there is no node for the given
    #          ID or the node is not assigned to any geometry
    #  @ingroup l1_meshinfo
    def GetShapeID(self, id):
        return self.mesh.GetShapeID(id)

    ## Returns the ID of the shape, on which the given element was generated.
    #  @return an integer value > 0 or -1 if there is no element for the given
    #          ID or the element is not assigned to any geometry
    #  @ingroup l1_meshinfo
    def GetShapeIDForElem(self,id):
        return self.mesh.GetShapeIDForElem(id)

    ## Returns the number of nodes of the given element
    #  @return an integer value > 0 or -1 if there is no element for the given ID
    #  @ingroup l1_meshinfo
    def GetElemNbNodes(self, id):
        return self.mesh.GetElemNbNodes(id)

    ## Returns the node ID the given (zero based) index for the given element
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
    #  @param nodeID ID of the node
    #  @param elementType  the type of elements to check a state of the node, either of
    #         (SMESH.ALL, SMESH.NODE, SMESH.EDGE, SMESH.FACE or SMESH.VOLUME)
    #  @ingroup l1_meshinfo
    def IsMediumNodeOfAnyElem(self, nodeID, elementType = SMESH.ALL ):
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

    ## Returns three components of normal of given mesh face
    #  (or an empty array in KO case)
    #  @ingroup l1_meshinfo
    def GetFaceNormal(self, faceId, normalized=False):
        return self.mesh.GetFaceNormal(faceId,normalized)

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

    ## Returns diameter of a ball discrete element or zero in case of an invalid \a id
    #  @ingroup l1_meshinfo
    def GetBallDiameter(self, id):
        return self.mesh.GetBallDiameter(id)

    ## Returns XYZ coordinates of the barycenter of the given element
    #  \n If there is no element for the given ID - returns an empty list
    #  @return a list of three double values
    #  @ingroup l1_meshinfo
    def BaryCenter(self, id):
        return self.mesh.BaryCenter(id)

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
        aFilterMgr.UnRegister()
        return aBorders


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
        genObjUnRegister([aMeasurements,id1, id2])
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
    #  @param IDs single source object or list of source objects or list of nodes/elements IDs
    #  @param isElem if @a IDs is a list of IDs, @c True value in this parameters specifies that @a objects are elements,
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
        unRegister = genObjUnRegister()
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
                unRegister.set( srclist[-1] )
                pass
            pass
        aMeasurements = self.smeshpyD.CreateMeasurements()
        unRegister.set( aMeasurements )
        aMeasure = aMeasurements.BoundingBox(srclist)
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
        x,y,z,Parameters,hasVars = ParseParameters(x,y,z)
        if hasVars: self.mesh.SetParameters(Parameters)
        return self.editor.AddNode( x, y, z)

    ## Creates a 0D element on a node with given number.
    #  @param IDOfNode the ID of node for creation of the element.
    #  @param DuplicateElements to add one more 0D element to a node or not
    #  @return the Id of the new 0D element
    #  @ingroup l2_modif_add
    def Add0DElement( self, IDOfNode, DuplicateElements=True ):
        return self.editor.Add0DElement( IDOfNode, DuplicateElements )

    ## Create 0D elements on all nodes of the given elements except those 
    #  nodes on which a 0D element already exists.
    #  @param theObject an object on whose nodes 0D elements will be created.
    #         It can be mesh, sub-mesh, group, list of element IDs or a holder
    #         of nodes IDs created by calling mesh.GetIDSource( nodes, SMESH.NODE )
    #  @param theGroupName optional name of a group to add 0D elements created
    #         and/or found on nodes of \a theObject.
    #  @param DuplicateElements to add one more 0D element to a node or not
    #  @return an object (a new group or a temporary SMESH_IDSource) holding
    #          IDs of new and/or found 0D elements. IDs of 0D elements 
    #          can be retrieved from the returned object by calling GetIDs()
    #  @ingroup l2_modif_add
    def Add0DElementsToAllNodes(self, theObject, theGroupName="", DuplicateElements=False):
        unRegister = genObjUnRegister()
        if isinstance( theObject, Mesh ):
            theObject = theObject.GetMesh()
        elif isinstance( theObject, list ):
            theObject = self.GetIDSource( theObject, SMESH.ALL )
            unRegister.set( theObject )
        return self.editor.Create0DElementsOnAllNodes( theObject, theGroupName, DuplicateElements )

    ## Creates a ball element on a node with given ID.
    #  @param IDOfNode the ID of node for creation of the element.
    #  @param diameter the bal diameter.
    #  @return the Id of the new ball element
    #  @ingroup l2_modif_add
    def AddBall(self, IDOfNode, diameter):
        return self.editor.AddBall( IDOfNode, diameter )

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

    ## Adds a quadratic polygonal face to the mesh by the list of node IDs
    #  @param IdsOfNodes the list of node IDs for creation of the element;
    #         corner nodes follow first.
    #  @return the Id of the new face
    #  @ingroup l2_modif_add
    def AddQuadPolygonalFace(self, IdsOfNodes):
        return self.editor.AddQuadPolygonalFace(IdsOfNodes)

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
        if ( isinstance( Vertex, geomBuilder.GEOM._objref_GEOM_Object)):
            VertexID = self.geompyD.GetSubShapeID( self.geom, Vertex )
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
        if ( isinstance( Edge, geomBuilder.GEOM._objref_GEOM_Object)):
            EdgeID = self.geompyD.GetSubShapeID( self.geom, Edge )
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
        if ( isinstance( Face, geomBuilder.GEOM._objref_GEOM_Object)):
            FaceID = self.geompyD.GetSubShapeID( self.geom, Face )
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
        if ( isinstance( Solid, geomBuilder.GEOM._objref_GEOM_Object)):
            SolidID = self.geompyD.GetSubShapeID( self.geom, Solid )
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
        if ( isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object)):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
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
        x,y,z,Parameters,hasVars = ParseParameters(x,y,z)
        if hasVars: self.mesh.SetParameters(Parameters)
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
        x,y,z,Parameters,hasVars = ParseParameters(x,y,z)
        if hasVars: self.mesh.SetParameters(Parameters)
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
    #  @param elementType type of elements to find; either of 
    #         (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME); SMESH.ALL type
    #         means elements of any type excluding nodes, discrete and 0D elements.
    #  @param meshPart a part of mesh (group, sub-mesh) to search within
    #  @return list of IDs of found elements
    #  @ingroup l2_modif_throughp
    def FindElementsByPoint(self, x, y, z, elementType = SMESH.ALL, meshPart=None):
        if meshPart:
            return self.editor.FindAmongElementsByPoint( meshPart, x, y, z, elementType );
        else:
            return self.editor.FindElementsByPoint(x, y, z, elementType)

    ## Return point state in a closed 2D mesh in terms of TopAbs_State enumeration:
    #  0-IN, 1-OUT, 2-ON, 3-UNKNOWN
    #  UNKNOWN state means that either mesh is wrong or the analysis fails.
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
    #  @ingroup l2_modif_cutquadr
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

    ## Reorient faces contained in \a the2DObject.
    #  @param the2DObject is a mesh, sub-mesh, group or list of IDs of 2D elements
    #  @param theDirection is a desired direction of normal of \a theFace.
    #         It can be either a GEOM vector or a list of coordinates [x,y,z].
    #  @param theFaceOrPoint defines a face of \a the2DObject whose normal will be
    #         compared with theDirection. It can be either ID of face or a point
    #         by which the face will be found. The point can be given as either
    #         a GEOM vertex or a list of point coordinates.
    #  @return number of reoriented faces
    #  @ingroup l2_modif_changori
    def Reorient2D(self, the2DObject, theDirection, theFaceOrPoint ):
        unRegister = genObjUnRegister()
        # check the2DObject
        if isinstance( the2DObject, Mesh ):
            the2DObject = the2DObject.GetMesh()
        if isinstance( the2DObject, list ):
            the2DObject = self.GetIDSource( the2DObject, SMESH.FACE )
            unRegister.set( the2DObject )
        # check theDirection
        if isinstance( theDirection, geomBuilder.GEOM._objref_GEOM_Object):
            theDirection = self.smeshpyD.GetDirStruct( theDirection )
        if isinstance( theDirection, list ):
            theDirection = self.smeshpyD.MakeDirStruct( *theDirection  )
        # prepare theFace and thePoint
        theFace = theFaceOrPoint
        thePoint = PointStruct(0,0,0)
        if isinstance( theFaceOrPoint, geomBuilder.GEOM._objref_GEOM_Object):
            thePoint = self.smeshpyD.GetPointStruct( theFaceOrPoint )
            theFace = -1
        if isinstance( theFaceOrPoint, list ):
            thePoint = PointStruct( *theFaceOrPoint )
            theFace = -1
        if isinstance( theFaceOrPoint, PointStruct ):
            thePoint = theFaceOrPoint
            theFace = -1
        return self.editor.Reorient2D( the2DObject, theDirection, theFace, thePoint )

    ## Reorient faces according to adjacent volumes.
    #  @param the2DObject is a mesh, sub-mesh, group or list of
    #         either IDs of faces or face groups.
    #  @param the3DObject is a mesh, sub-mesh, group or list of IDs of volumes.
    #  @param theOutsideNormal to orient faces to have their normals
    #         pointing either \a outside or \a inside the adjacent volumes.
    #  @return number of reoriented faces.
    #  @ingroup l2_modif_changori
    def Reorient2DBy3D(self, the2DObject, the3DObject, theOutsideNormal=True ):
        unRegister = genObjUnRegister()
        # check the2DObject
        if not isinstance( the2DObject, list ):
            the2DObject = [ the2DObject ]
        elif the2DObject and isinstance( the2DObject[0], int ):
            the2DObject = self.GetIDSource( the2DObject, SMESH.FACE )
            unRegister.set( the2DObject )
            the2DObject = [ the2DObject ]
        for i,obj2D in enumerate( the2DObject ):
            if isinstance( obj2D, Mesh ):
                the2DObject[i] = obj2D.GetMesh()
            if isinstance( obj2D, list ):
                the2DObject[i] = self.GetIDSource( obj2D, SMESH.FACE )
                unRegister.set( the2DObject[i] )
        # check the3DObject
        if isinstance( the3DObject, Mesh ):
            the3DObject = the3DObject.GetMesh()
        if isinstance( the3DObject, list ):
            the3DObject = self.GetIDSource( the3DObject, SMESH.VOLUME )
            unRegister.set( the3DObject )
        return self.editor.Reorient2DBy3D( the2DObject, the3DObject, theOutsideNormal )

    ## Fuses the neighbouring triangles into quadrangles.
    #  @param IDsOfElements The triangles to be fused.
    #  @param theCriterion  a numerical functor, in terms of enum SMESH.FunctorType, used to
    #          applied to possible quadrangles to choose a neighbour to fuse with.
    #          Type SMESH.FunctorType._items in the Python Console to see all items.
    #          Note that not all items correspond to numerical functors.
    #  @param MaxAngle      is the maximum angle between element normals at which the fusion
    #          is still performed; theMaxAngle is mesured in radians.
    #          Also it could be a name of variable which defines angle in degrees.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_unitetri
    def TriToQuad(self, IDsOfElements, theCriterion, MaxAngle):
        MaxAngle,Parameters,hasVars = ParseAngles(MaxAngle)
        self.mesh.SetParameters(Parameters)
        if not IDsOfElements:
            IDsOfElements = self.GetElementsId()
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.TriToQuad(IDsOfElements, Functor, MaxAngle)

    ## Fuses the neighbouring triangles of the object into quadrangles
    #  @param theObject is mesh, submesh or group
    #  @param theCriterion is a numerical functor, in terms of enum SMESH.FunctorType,
    #          applied to possible quadrangles to choose a neighbour to fuse with.
    #          Type SMESH.FunctorType._items in the Python Console to see all items.
    #          Note that not all items correspond to numerical functors.
    #  @param MaxAngle   a max angle between element normals at which the fusion
    #          is still performed; theMaxAngle is mesured in radians.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_unitetri
    def TriToQuadObject (self, theObject, theCriterion, MaxAngle):
        MaxAngle,Parameters,hasVars = ParseAngles(MaxAngle)
        self.mesh.SetParameters(Parameters)
        if isinstance( theObject, Mesh ):
            theObject = theObject.GetMesh()
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.TriToQuadObject(theObject, Functor, MaxAngle)

    ## Splits quadrangles into triangles.
    #  @param IDsOfElements the faces to be splitted.
    #  @param theCriterion is a numerical functor, in terms of enum SMESH.FunctorType, used to
    #         choose a diagonal for splitting. If @a theCriterion is None, which is a default
    #         value, then quadrangles will be split by the smallest diagonal.
    #         Type SMESH.FunctorType._items in the Python Console to see all items.
    #         Note that not all items correspond to numerical functors.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_cutquadr
    def QuadToTri (self, IDsOfElements, theCriterion = None):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if theCriterion is None:
            theCriterion = FT_MaxElementLength2D
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.QuadToTri(IDsOfElements, Functor)

    ## Splits quadrangles into triangles.
    #  @param theObject the object from which the list of elements is taken,
    #         this is mesh, submesh or group
    #  @param theCriterion is a numerical functor, in terms of enum SMESH.FunctorType, used to
    #         choose a diagonal for splitting. If @a theCriterion is None, which is a default
    #         value, then quadrangles will be split by the smallest diagonal.
    #         Type SMESH.FunctorType._items in the Python Console to see all items.
    #         Note that not all items correspond to numerical functors.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_cutquadr
    def QuadToTriObject (self, theObject, theCriterion = None):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if theCriterion is None:
            theCriterion = FT_MaxElementLength2D
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.QuadToTriObject(theObject, Functor)

    ## Splits each of given quadrangles into 4 triangles. A node is added at the center of
    #  a quadrangle.
    #  @param theElements the faces to be splitted. This can be either mesh, sub-mesh,
    #         group or a list of face IDs. By default all quadrangles are split
    #  @ingroup l2_modif_cutquadr
    def QuadTo4Tri (self, theElements=[]):
        unRegister = genObjUnRegister()
        if isinstance( theElements, Mesh ):
            theElements = theElements.mesh
        elif not theElements:
            theElements = self.mesh
        elif isinstance( theElements, list ):
            theElements = self.GetIDSource( theElements, SMESH.FACE )
            unRegister.set( theElements )
        return self.editor.QuadTo4Tri( theElements )

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
    #  @param theObject the object from which the list of elements is taken,
    #         this is mesh, submesh or group
    #  @param Diag13    is used to choose a diagonal for splitting.
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_cutquadr
    def SplitQuadObject (self, theObject, Diag13):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SplitQuadObject(theObject, Diag13)

    ## Finds a better splitting of the given quadrangle.
    #  @param IDOfQuad   the ID of the quadrangle to be splitted.
    #  @param theCriterion  is a numerical functor, in terms of enum SMESH.FunctorType, used to
    #         choose a diagonal for splitting.
    #         Type SMESH.FunctorType._items in the Python Console to see all items.
    #         Note that not all items correspond to numerical functors.
    #  @return 1 if 1-3 diagonal is better, 2 if 2-4
    #          diagonal is better, 0 if error occurs.
    #  @ingroup l2_modif_cutquadr
    def BestSplit (self, IDOfQuad, theCriterion):
        return self.editor.BestSplit(IDOfQuad, self.smeshpyD.GetFunctor(theCriterion))

    ## Splits volumic elements into tetrahedrons
    #  @param elems either a list of elements or a mesh or a group or a submesh or a filter
    #  @param method  flags passing splitting method:
    #         smesh.Hex_5Tet, smesh.Hex_6Tet, smesh.Hex_24Tet.
    #         smesh.Hex_5Tet - to split the hexahedron into 5 tetrahedrons, etc.
    #  @ingroup l2_modif_cutquadr
    def SplitVolumesIntoTetra(self, elems, method=smeshBuilder.Hex_5Tet ):
        unRegister = genObjUnRegister()
        if isinstance( elems, Mesh ):
            elems = elems.GetMesh()
        if ( isinstance( elems, list )):
            elems = self.editor.MakeIDSource(elems, SMESH.VOLUME)
            unRegister.set( elems )
        self.editor.SplitVolumesIntoTetra(elems, method)
        return

    ## Split bi-quadratic elements into linear ones without creation of additional nodes:
    #   - bi-quadratic triangle will be split into 3 linear quadrangles;
    #   - bi-quadratic quadrangle will be split into 4 linear quadrangles;
    #   - tri-quadratic hexahedron will be split into 8 linear hexahedra.
    #   Quadratic elements of lower dimension  adjacent to the split bi-quadratic element
    #   will be split in order to keep the mesh conformal.
    #  @param elems - elements to split: sub-meshes, groups, filters or element IDs;
    #         if None (default), all bi-quadratic elements will be split
    #  @ingroup l2_modif_cutquadr
    def SplitBiQuadraticIntoLinear(self, elems=None):
        unRegister = genObjUnRegister()
        if elems and isinstance( elems, list ) and isinstance( elems[0], int ):
            elems = self.editor.MakeIDSource(elems, SMESH.ALL)
            unRegister.set( elems )
        if elems is None:
            elems = [ self.GetMesh() ]
        if isinstance( elems, Mesh ):
            elems = [ elems.GetMesh() ]
        if not isinstance( elems, list ):
            elems = [elems]
        self.editor.SplitBiQuadraticIntoLinear( elems )

    ## Splits hexahedra into prisms
    #  @param elems either a list of elements or a mesh or a group or a submesh or a filter
    #  @param startHexPoint a point used to find a hexahedron for which @a facetNormal
    #         gives a normal vector defining facets to split into triangles.
    #         @a startHexPoint can be either a triple of coordinates or a vertex.
    #  @param facetNormal a normal to a facet to split into triangles of a
    #         hexahedron found by @a startHexPoint.
    #         @a facetNormal can be either a triple of coordinates or an edge.
    #  @param method  flags passing splitting method: smesh.Hex_2Prisms, smesh.Hex_4Prisms.
    #         smesh.Hex_2Prisms - to split the hexahedron into 2 prisms, etc.
    #  @param allDomains if @c False, only hexahedra adjacent to one closest
    #         to @a startHexPoint are split, else @a startHexPoint
    #         is used to find the facet to split in all domains present in @a elems.
    #  @ingroup l2_modif_cutquadr
    def SplitHexahedraIntoPrisms(self, elems, startHexPoint, facetNormal,
                                 method=smeshBuilder.Hex_2Prisms, allDomains=False ):
        # IDSource
        unRegister = genObjUnRegister()
        if isinstance( elems, Mesh ):
            elems = elems.GetMesh()
        if ( isinstance( elems, list )):
            elems = self.editor.MakeIDSource(elems, SMESH.VOLUME)
            unRegister.set( elems )
            pass
        # axis
        if isinstance( startHexPoint, geomBuilder.GEOM._objref_GEOM_Object):
            startHexPoint = self.smeshpyD.GetPointStruct( startHexPoint )
        elif isinstance( startHexPoint, list ):
            startHexPoint = SMESH.PointStruct( startHexPoint[0],
                                               startHexPoint[1],
                                               startHexPoint[2])
        if isinstance( facetNormal, geomBuilder.GEOM._objref_GEOM_Object):
            facetNormal = self.smeshpyD.GetDirStruct( facetNormal )
        elif isinstance( facetNormal, list ):
            facetNormal = self.smeshpyD.MakeDirStruct( facetNormal[0],
                                                       facetNormal[1],
                                                       facetNormal[2])
        self.mesh.SetParameters( startHexPoint.parameters + facetNormal.PS.parameters )

        self.editor.SplitHexahedraIntoPrisms(elems, startHexPoint, facetNormal, method, allDomains)

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
    #  @param Method is either Laplacian (smesh.LAPLACIAN_SMOOTH)
    #         or Centroidal (smesh.CENTROIDAL_SMOOTH)
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_smooth
    def Smooth(self, IDsOfElements, IDsOfFixedNodes,
               MaxNbOfIterations, MaxAspectRatio, Method):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        MaxNbOfIterations,MaxAspectRatio,Parameters,hasVars = ParseParameters(MaxNbOfIterations,MaxAspectRatio)
        self.mesh.SetParameters(Parameters)
        return self.editor.Smooth(IDsOfElements, IDsOfFixedNodes,
                                  MaxNbOfIterations, MaxAspectRatio, Method)

    ## Smoothes elements which belong to the given object
    #  @param theObject the object to smooth
    #  @param IDsOfFixedNodes the list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations the maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is either Laplacian (smesh.LAPLACIAN_SMOOTH)
    #         or Centroidal (smesh.CENTROIDAL_SMOOTH)
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
    #  @param Method is either Laplacian (smesh.LAPLACIAN_SMOOTH)
    #         or Centroidal (smesh.CENTROIDAL_SMOOTH)
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_smooth
    def SmoothParametric(self, IDsOfElements, IDsOfFixedNodes,
                         MaxNbOfIterations, MaxAspectRatio, Method):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        MaxNbOfIterations,MaxAspectRatio,Parameters,hasVars = ParseParameters(MaxNbOfIterations,MaxAspectRatio)
        self.mesh.SetParameters(Parameters)
        return self.editor.SmoothParametric(IDsOfElements, IDsOfFixedNodes,
                                            MaxNbOfIterations, MaxAspectRatio, Method)

    ## Parametrically smoothes the elements which belong to the given object
    #  @param theObject the object to smooth
    #  @param IDsOfFixedNodes the list of ids of fixed nodes.
    #  Note that nodes built on edges and boundary nodes are always fixed.
    #  @param MaxNbOfIterations the maximum number of iterations
    #  @param MaxAspectRatio varies in range [1.0, inf]
    #  @param Method is either Laplacian (smesh.LAPLACIAN_SMOOTH)
    #         or Centroidal (smesh.CENTROIDAL_SMOOTH)
    #  @return TRUE in case of success, FALSE otherwise.
    #  @ingroup l2_modif_smooth
    def SmoothParametricObject(self, theObject, IDsOfFixedNodes,
                               MaxNbOfIterations, MaxAspectRatio, Method):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SmoothParametricObject(theObject, IDsOfFixedNodes,
                                                  MaxNbOfIterations, MaxAspectRatio, Method)

    ## Converts the mesh to quadratic or bi-quadratic, deletes old elements, replacing
    #  them with quadratic with the same id.
    #  @param theForce3d new node creation method:
    #         0 - the medium node lies at the geometrical entity from which the mesh element is built
    #         1 - the medium node lies at the middle of the line segments connecting two nodes of a mesh element
    #  @param theSubMesh a group or a sub-mesh to convert; WARNING: in this case the mesh can become not conformal
    #  @param theToBiQuad If True, converts the mesh to bi-quadratic
    #  @ingroup l2_modif_tofromqu
    def ConvertToQuadratic(self, theForce3d=False, theSubMesh=None, theToBiQuad=False):
        if isinstance( theSubMesh, Mesh ):
            theSubMesh = theSubMesh.mesh
        if theToBiQuad:
            self.editor.ConvertToBiQuadratic(theForce3d,theSubMesh)
        else:
            if theSubMesh:
                self.editor.ConvertToQuadraticObject(theForce3d,theSubMesh)
            else:
                self.editor.ConvertToQuadratic(theForce3d)
        error = self.editor.GetLastError()
        if error and error.comment:
            print error.comment
            
    ## Converts the mesh from quadratic to ordinary,
    #  deletes old quadratic elements, \n replacing
    #  them with ordinary mesh elements with the same id.
    #  @param theSubMesh a group or a sub-mesh to convert; WARNING: in this case the mesh can become not conformal
    #  @ingroup l2_modif_tofromqu
    def ConvertFromQuadratic(self, theSubMesh=None):
        if theSubMesh:
            self.editor.ConvertFromQuadraticObject(theSubMesh)
        else:
            return self.editor.ConvertFromQuadratic()

    ## Creates 2D mesh as skin on boundary faces of a 3D mesh
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def Make2DMeshFrom3D(self):
        return self.editor.Make2DMeshFrom3D()

    ## Creates missing boundary elements
    #  @param elements - elements whose boundary is to be checked:
    #                    mesh, group, sub-mesh or list of elements
    #   if elements is mesh, it must be the mesh whose MakeBoundaryMesh() is called
    #  @param dimension - defines type of boundary elements to create, either of
    #                     { SMESH.BND_2DFROM3D, SMESH.BND_1DFROM3D, SMESH.BND_1DFROM2D }
    #    SMESH.BND_1DFROM3D creates mesh edges on all borders of free facets of 3D cells
    #  @param groupName - a name of group to store created boundary elements in,
    #                     "" means not to create the group
    #  @param meshName - a name of new mesh to store created boundary elements in,
    #                     "" means not to create the new mesh
    #  @param toCopyElements - if true, the checked elements will be copied into
    #     the new mesh else only boundary elements will be copied into the new mesh
    #  @param toCopyExistingBondary - if true, not only new but also pre-existing
    #     boundary elements will be copied into the new mesh
    #  @return tuple (mesh, group) where boundary elements were added to
    #  @ingroup l2_modif_edit
    def MakeBoundaryMesh(self, elements, dimension=SMESH.BND_2DFROM3D, groupName="", meshName="",
                         toCopyElements=False, toCopyExistingBondary=False):
        unRegister = genObjUnRegister()
        if isinstance( elements, Mesh ):
            elements = elements.GetMesh()
        if ( isinstance( elements, list )):
            elemType = SMESH.ALL
            if elements: elemType = self.GetElementType( elements[0], iselem=True)
            elements = self.editor.MakeIDSource(elements, elemType)
            unRegister.set( elements )
        mesh, group = self.editor.MakeBoundaryMesh(elements,dimension,groupName,meshName,
                                                   toCopyElements,toCopyExistingBondary)
        if mesh: mesh = self.smeshpyD.Mesh(mesh)
        return mesh, group

    ##
    # @brief Creates missing boundary elements around either the whole mesh or 
    #    groups of elements
    #  @param dimension - defines type of boundary elements to create, either of
    #                     { SMESH.BND_2DFROM3D, SMESH.BND_1DFROM3D, SMESH.BND_1DFROM2D }
    #  @param groupName - a name of group to store all boundary elements in,
    #    "" means not to create the group
    #  @param meshName - a name of a new mesh, which is a copy of the initial 
    #    mesh + created boundary elements; "" means not to create the new mesh
    #  @param toCopyAll - if true, the whole initial mesh will be copied into
    #    the new mesh else only boundary elements will be copied into the new mesh
    #  @param groups - groups of elements to make boundary around
    #  @retval tuple( long, mesh, groups )
    #                 long - number of added boundary elements
    #                 mesh - the mesh where elements were added to
    #                 group - the group of boundary elements or None
    #
    def MakeBoundaryElements(self, dimension=SMESH.BND_2DFROM3D, groupName="", meshName="",
                             toCopyAll=False, groups=[]):
        nb, mesh, group = self.editor.MakeBoundaryElements(dimension,groupName,meshName,
                                                           toCopyAll,groups)
        if mesh: mesh = self.smeshpyD.Mesh(mesh)
        return nb, mesh, group

    ## Renumber mesh nodes (Obsolete, does nothing)
    #  @ingroup l2_modif_renumber
    def RenumberNodes(self):
        self.editor.RenumberNodes()

    ## Renumber mesh elements (Obsole, does nothing)
    #  @ingroup l2_modif_renumber
    def RenumberElements(self):
        self.editor.RenumberElements()

    ## Private method converting \a arg into a list of SMESH_IdSource's
    def _getIdSourceList(self, arg, idType, unRegister):
        if arg and isinstance( arg, list ):
            if isinstance( arg[0], int ):
                arg = self.GetIDSource( arg, idType )
                unRegister.set( arg )
            elif isinstance( arg[0], Mesh ):
                arg[0] = arg[0].GetMesh()
        elif isinstance( arg, Mesh ):
            arg = arg.GetMesh()
        if arg and isinstance( arg, SMESH._objref_SMESH_IDSource ):
            arg = [arg]
        return arg

    ## Generates new elements by rotation of the given elements and nodes around the axis
    #  @param nodes - nodes to revolve: a list including ids, groups, sub-meshes or a mesh
    #  @param edges - edges to revolve: a list including ids, groups, sub-meshes or a mesh
    #  @param faces - faces to revolve: a list including ids, groups, sub-meshes or a mesh
    #  @param Axis the axis of rotation: AxisStruct, line (geom object) or [x,y,z,dx,dy,dz]
    #  @param AngleInRadians the angle of Rotation (in radians) or a name of variable
    #         which defines angle in degrees
    #  @param NbOfSteps the number of steps
    #  @param Tolerance tolerance
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param TotalAngle gives meaning of AngleInRadians: if True then it is an angular size
    #                    of all steps, else - size of each step
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def RotationSweepObjects(self, nodes, edges, faces, Axis, AngleInRadians, NbOfSteps, Tolerance,
                             MakeGroups=False, TotalAngle=False):
        unRegister = genObjUnRegister()
        nodes = self._getIdSourceList( nodes, SMESH.NODE, unRegister )
        edges = self._getIdSourceList( edges, SMESH.EDGE, unRegister )
        faces = self._getIdSourceList( faces, SMESH.FACE, unRegister )

        if isinstance( Axis, geomBuilder.GEOM._objref_GEOM_Object):
            Axis = self.smeshpyD.GetAxisStruct( Axis )
        if isinstance( Axis, list ):
            Axis = SMESH.AxisStruct( *Axis )

        AngleInRadians,AngleParameters,hasVars = ParseAngles(AngleInRadians)
        NbOfSteps,Tolerance,Parameters,hasVars = ParseParameters(NbOfSteps,Tolerance)
        Parameters = Axis.parameters + var_separator + AngleParameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        if TotalAngle and NbOfSteps:
            AngleInRadians /= NbOfSteps
        return self.editor.RotationSweepObjects( nodes, edges, faces,
                                                 Axis, AngleInRadians,
                                                 NbOfSteps, Tolerance, MakeGroups)

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
        return self.RotationSweepObjects([], IDsOfElements, IDsOfElements, Axis,
                                         AngleInRadians, NbOfSteps, Tolerance,
                                         MakeGroups, TotalAngle)

    ## Generates new elements by rotation of the elements of object around the axis
    #  @param theObject object which elements should be sweeped.
    #                   It can be a mesh, a sub mesh or a group.
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
        return self.RotationSweepObjects( [], theObject, theObject, Axis,
                                          AngleInRadians, NbOfSteps, Tolerance,
                                          MakeGroups, TotalAngle )

    ## Generates new elements by rotation of the elements of object around the axis
    #  @param theObject object which elements should be sweeped.
    #                   It can be a mesh, a sub mesh or a group.
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
        return self.RotationSweepObjects([],theObject,[], Axis,
                                         AngleInRadians, NbOfSteps, Tolerance,
                                         MakeGroups, TotalAngle)

    ## Generates new elements by rotation of the elements of object around the axis
    #  @param theObject object which elements should be sweeped.
    #                   It can be a mesh, a sub mesh or a group.
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
        return self.RotationSweepObjects([],[],theObject, Axis, AngleInRadians,
                                         NbOfSteps, Tolerance, MakeGroups, TotalAngle)

    ## Generates new elements by extrusion of the given elements and nodes
    #  @param nodes nodes to extrude: a list including ids, groups, sub-meshes or a mesh
    #  @param edges edges to extrude: a list including ids, groups, sub-meshes or a mesh
    #  @param faces faces to extrude: a list including ids, groups, sub-meshes or a mesh
    #  @param StepVector vector or DirStruct or 3 vector components, defining
    #         the direction and value of extrusion for one step (the total extrusion
    #         length will be NbOfSteps * ||StepVector||)
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param scaleFactors optional scale factors to apply during extrusion
    #  @param linearVariation if @c True, scaleFactors are spread over all @a scaleFactors,
    #         else scaleFactors[i] is applied to nodes at the i-th extrusion step
    #  @param basePoint optional scaling center; if not provided, a gravity center of
    #         nodes and elements being extruded is used as the scaling center.
    #         It can be either
    #         - a list of tree components of the point or
    #         - a node ID or
    #         - a GEOM point
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweepObjects(self, nodes, edges, faces, StepVector, NbOfSteps, MakeGroups=False,
                              scaleFactors=[], linearVariation=False, basePoint=[] ):
        unRegister = genObjUnRegister()
        nodes = self._getIdSourceList( nodes, SMESH.NODE, unRegister )
        edges = self._getIdSourceList( edges, SMESH.EDGE, unRegister )
        faces = self._getIdSourceList( faces, SMESH.FACE, unRegister )

        if isinstance( StepVector, geomBuilder.GEOM._objref_GEOM_Object):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if isinstance( StepVector, list ):
            StepVector = self.smeshpyD.MakeDirStruct(*StepVector)

        if isinstance( basePoint, int):
            xyz = self.GetNodeXYZ( basePoint )
            if not xyz:
                raise RuntimeError, "Invalid node ID: %s" % basePoint
            basePoint = xyz
        if isinstance( basePoint, geomBuilder.GEOM._objref_GEOM_Object ):
            basePoint = self.geompyD.PointCoordinates( basePoint )

        NbOfSteps,Parameters,hasVars = ParseParameters(NbOfSteps)
        Parameters = StepVector.PS.parameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)

        return self.editor.ExtrusionSweepObjects( nodes, edges, faces,
                                                  StepVector, NbOfSteps,
                                                  scaleFactors, linearVariation, basePoint,
                                                  MakeGroups)


    ## Generates new elements by extrusion of the elements with given ids
    #  @param IDsOfElements the list of ids of elements or nodes for extrusion
    #  @param StepVector vector or DirStruct or 3 vector components, defining
    #         the direction and value of extrusion for one step (the total extrusion
    #         length will be NbOfSteps * ||StepVector||)
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param IsNodes is True if elements with given ids are nodes
    #  @return the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweep(self, IDsOfElements, StepVector, NbOfSteps, MakeGroups=False, IsNodes = False):
        n,e,f = [],[],[]
        if IsNodes: n = IDsOfElements
        else      : e,f, = IDsOfElements,IDsOfElements
        return self.ExtrusionSweepObjects(n,e,f, StepVector, NbOfSteps, MakeGroups)

    ## Generates new elements by extrusion along the normal to a discretized surface or wire
    #  @param Elements elements to extrude - a list including ids, groups, sub-meshes or a mesh.
    #         Only faces can be extruded so far. A sub-mesh should be a sub-mesh on geom faces.
    #  @param StepSize length of one extrusion step (the total extrusion
    #         length will be \a NbOfSteps * \a StepSize ).
    #  @param NbOfSteps number of extrusion steps.
    #  @param ByAverageNormal if True each node is translated by \a StepSize
    #         along the average of the normal vectors to the faces sharing the node;
    #         else each node is translated along the same average normal till
    #         intersection with the plane got by translation of the face sharing
    #         the node along its own normal by \a StepSize.
    #  @param UseInputElemsOnly to use only \a Elements when computing extrusion direction
    #         for every node of \a Elements.
    #  @param MakeGroups forces generation of new groups from existing ones.
    #  @param Dim dimension of elements to extrude: 2 - faces or 1 - edges. Extrusion of edges
    #         is not yet implemented. This parameter is used if \a Elements contains
    #         both faces and edges, i.e. \a Elements is a Mesh.
    #  @return the list of created groups (SMESH_GroupBase) if \a MakeGroups=True,
    #          empty list otherwise.
    #  @ingroup l2_modif_extrurev
    def ExtrusionByNormal(self, Elements, StepSize, NbOfSteps,
                          ByAverageNormal=False, UseInputElemsOnly=True, MakeGroups=False, Dim = 2):
        unRegister = genObjUnRegister()
        if isinstance( Elements, Mesh ):
            Elements = [ Elements.GetMesh() ]
        if isinstance( Elements, list ):
            if not Elements:
                raise RuntimeError, "Elements empty!"
            if isinstance( Elements[0], int ):
                Elements = self.GetIDSource( Elements, SMESH.ALL )
                unRegister.set( Elements )
        if not isinstance( Elements, list ):
            Elements = [ Elements ]
        StepSize,NbOfSteps,Parameters,hasVars = ParseParameters(StepSize,NbOfSteps)
        self.mesh.SetParameters(Parameters)
        return self.editor.ExtrusionByNormal(Elements, StepSize, NbOfSteps,
                                             ByAverageNormal, UseInputElemsOnly, MakeGroups, Dim)

    ## Generates new elements by extrusion of the elements or nodes which belong to the object
    #  @param theObject the object whose elements or nodes should be processed.
    #                   It can be a mesh, a sub-mesh or a group.
    #  @param StepVector vector or DirStruct or 3 vector components, defining
    #         the direction and value of extrusion for one step (the total extrusion
    #         length will be NbOfSteps * ||StepVector||)
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param IsNodes is True if elements to extrude are nodes
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweepObject(self, theObject, StepVector, NbOfSteps, MakeGroups=False, IsNodes=False):
        n,e,f = [],[],[]
        if IsNodes: n    = theObject
        else      : e,f, = theObject,theObject
        return self.ExtrusionSweepObjects(n,e,f, StepVector, NbOfSteps, MakeGroups)

    ## Generates new elements by extrusion of edges which belong to the object
    #  @param theObject object whose 1D elements should be processed.
    #                   It can be a mesh, a sub-mesh or a group.
    #  @param StepVector vector or DirStruct or 3 vector components, defining
    #         the direction and value of extrusion for one step (the total extrusion
    #         length will be NbOfSteps * ||StepVector||)
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups to generate new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweepObject1D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        return self.ExtrusionSweepObjects([],theObject,[], StepVector, NbOfSteps, MakeGroups)

    ## Generates new elements by extrusion of faces which belong to the object
    #  @param theObject object whose 2D elements should be processed.
    #                   It can be a mesh, a sub-mesh or a group.
    #  @param StepVector vector or DirStruct or 3 vector components, defining
    #         the direction and value of extrusion for one step (the total extrusion
    #         length will be NbOfSteps * ||StepVector||)
    #  @param NbOfSteps the number of steps
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionSweepObject2D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        return self.ExtrusionSweepObjects([],[],theObject, StepVector, NbOfSteps, MakeGroups)

    ## Generates new elements by extrusion of the elements with given ids
    #  @param IDsOfElements is ids of elements
    #  @param StepVector vector or DirStruct or 3 vector components, defining
    #         the direction and value of extrusion for one step (the total extrusion
    #         length will be NbOfSteps * ||StepVector||)
    #  @param NbOfSteps the number of steps
    #  @param ExtrFlags sets flags for extrusion
    #  @param SewTolerance uses for comparing locations of nodes if flag
    #         EXTRUSION_FLAG_SEW is set
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_extrurev
    def AdvancedExtrusion(self, IDsOfElements, StepVector, NbOfSteps,
                          ExtrFlags, SewTolerance, MakeGroups=False):
        if isinstance( StepVector, geomBuilder.GEOM._objref_GEOM_Object):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if isinstance( StepVector, list ):
            StepVector = self.smeshpyD.MakeDirStruct(*StepVector)
        return self.editor.AdvancedExtrusion(IDsOfElements, StepVector, NbOfSteps,
                                             ExtrFlags, SewTolerance, MakeGroups)

    ## Generates new elements by extrusion of the given elements and nodes along the path.
    #  The path of extrusion must be a meshed edge.
    #  @param Nodes nodes to extrude: a list including ids, groups, sub-meshes or a mesh
    #  @param Edges edges to extrude: a list including ids, groups, sub-meshes or a mesh
    #  @param Faces faces to extrude: a list including ids, groups, sub-meshes or a mesh
    #  @param PathMesh 1D mesh or 1D sub-mesh, along which proceeds the extrusion
    #  @param PathShape shape (edge) defines the sub-mesh of PathMesh if PathMesh
    #         contains not only path segments, else it can be None
    #  @param NodeStart the first or the last node on the path. Defines the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path
    #                   to get the resulting mesh in a helical fashion
    #  @param Angles list of angles
    #  @param LinearVariation forces the computation of rotation angles as linear
    #                         variation of the given Angles along path steps
    #  @param HasRefPoint allows using the reference point
    #  @param RefPoint the point around which the shape is rotated (the mass center of the
    #         shape by default). The User can specify any point as the Reference Point.
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @return list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error
    #  @ingroup l2_modif_extrurev
    def ExtrusionAlongPathObjects(self, Nodes, Edges, Faces, PathMesh, PathShape=None,
                                  NodeStart=1, HasAngles=False, Angles=[], LinearVariation=False,
                                  HasRefPoint=False, RefPoint=[0,0,0], MakeGroups=False):
        unRegister = genObjUnRegister()
        Nodes = self._getIdSourceList( Nodes, SMESH.NODE, unRegister )
        Edges = self._getIdSourceList( Edges, SMESH.EDGE, unRegister )
        Faces = self._getIdSourceList( Faces, SMESH.FACE, unRegister )

        if isinstance( RefPoint, geomBuilder.GEOM._objref_GEOM_Object):
            RefPoint = self.smeshpyD.GetPointStruct(RefPoint)
        if isinstance( RefPoint, list ):
            if not RefPoint: RefPoint = [0,0,0]
            RefPoint = SMESH.PointStruct( *RefPoint )
        if isinstance( PathMesh, Mesh ):
            PathMesh = PathMesh.GetMesh()
        Angles,AnglesParameters,hasVars = ParseAngles(Angles)
        Parameters = AnglesParameters + var_separator + RefPoint.parameters
        self.mesh.SetParameters(Parameters)
        return self.editor.ExtrusionAlongPathObjects(Nodes, Edges, Faces,
                                                     PathMesh, PathShape, NodeStart,
                                                     HasAngles, Angles, LinearVariation,
                                                     HasRefPoint, RefPoint, MakeGroups)

    ## Generates new elements by extrusion of the given elements
    #  The path of extrusion must be a meshed edge.
    #  @param Base mesh or group, or sub-mesh, or list of ids of elements for extrusion
    #  @param Path - 1D mesh or 1D sub-mesh, along which proceeds the extrusion
    #  @param NodeStart the start node from Path. Defines the direction of extrusion
    #  @param HasAngles allows the shape to be rotated around the path
    #                   to get the resulting mesh in a helical fashion
    #  @param Angles list of angles in radians
    #  @param LinearVariation forces the computation of rotation angles as linear
    #                         variation of the given Angles along path steps
    #  @param HasRefPoint allows using the reference point
    #  @param RefPoint the point around which the elements are rotated (the mass
    #         center of the elements by default).
    #         The User can specify any point as the Reference Point.
    #         RefPoint can be either GEOM Vertex, [x,y,z] or SMESH.PointStruct
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param ElemType type of elements for extrusion (if param Base is a mesh)
    #  @return list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
    #          only SMESH::Extrusion_Error otherwise
    #  @ingroup l2_modif_extrurev
    def ExtrusionAlongPathX(self, Base, Path, NodeStart,
                            HasAngles=False, Angles=[], LinearVariation=False,
                            HasRefPoint=False, RefPoint=[0,0,0], MakeGroups=False,
                            ElemType=SMESH.FACE):
        n,e,f = [],[],[]
        if ElemType == SMESH.NODE: n = Base
        if ElemType == SMESH.EDGE: e = Base
        if ElemType == SMESH.FACE: f = Base
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, Path, None, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

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
                           HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                           MakeGroups=False, LinearVariation=False):
        n,e,f = [],IDsOfElements,IDsOfElements
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape,
                                               NodeStart, HasAngles, Angles,
                                               LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    ## Generates new elements by extrusion of the elements which belong to the object
    #  The path of extrusion must be a meshed edge.
    #  @param theObject the object whose elements should be processed.
    #                   It can be a mesh, a sub-mesh or a group.
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
                                 HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                                 MakeGroups=False, LinearVariation=False):
        n,e,f = [],theObject,theObject
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    ## Generates new elements by extrusion of mesh segments which belong to the object
    #  The path of extrusion must be a meshed edge.
    #  @param theObject the object whose 1D elements should be processed.
    #                   It can be a mesh, a sub-mesh or a group.
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
                                   HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                                   MakeGroups=False, LinearVariation=False):
        n,e,f = [],theObject,[]
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    ## Generates new elements by extrusion of faces which belong to the object
    #  The path of extrusion must be a meshed edge.
    #  @param theObject the object whose 2D elements should be processed.
    #                   It can be a mesh, a sub-mesh or a group.
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
                                   HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                                   MakeGroups=False, LinearVariation=False):
        n,e,f = [],[],theObject
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    ## Creates a symmetrical copy of mesh elements
    #  @param IDsOfElements list of elements ids
    #  @param Mirror is AxisStruct or geom object(point, line, plane)
    #  @param theMirrorType smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
    #         If the Mirror is a geom object this parameter is unnecessary
    #  @param Copy allows to copy element (Copy is 1) or to replace with its mirroring (Copy is 0)
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def Mirror(self, IDsOfElements, Mirror, theMirrorType=None, Copy=0, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Mirror, geomBuilder.GEOM._objref_GEOM_Object)):
            Mirror        = self.smeshpyD.GetAxisStruct(Mirror)
            theMirrorType = Mirror._mirrorType
        else:
            self.mesh.SetParameters(Mirror.parameters)
        if Copy and MakeGroups:
            return self.editor.MirrorMakeGroups(IDsOfElements, Mirror, theMirrorType)
        self.editor.Mirror(IDsOfElements, Mirror, theMirrorType, Copy)
        return []

    ## Creates a new mesh by a symmetrical copy of mesh elements
    #  @param IDsOfElements the list of elements ids
    #  @param Mirror is AxisStruct or geom object (point, line, plane)
    #  @param theMirrorType smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
    #         If the Mirror is a geom object this parameter is unnecessary
    #  @param MakeGroups to generate new groups from existing ones
    #  @param NewMeshName a name of the new mesh to create
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def MirrorMakeMesh(self, IDsOfElements, Mirror, theMirrorType=0, MakeGroups=0, NewMeshName=""):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Mirror, geomBuilder.GEOM._objref_GEOM_Object)):
            Mirror        = self.smeshpyD.GetAxisStruct(Mirror)
            theMirrorType = Mirror._mirrorType
        else:
            self.mesh.SetParameters(Mirror.parameters)
        mesh = self.editor.MirrorMakeMesh(IDsOfElements, Mirror, theMirrorType,
                                          MakeGroups, NewMeshName)
        return Mesh(self.smeshpyD,self.geompyD,mesh)

    ## Creates a symmetrical copy of the object
    #  @param theObject mesh, submesh or group
    #  @param Mirror AxisStruct or geom object (point, line, plane)
    #  @param theMirrorType smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
    #         If the Mirror is a geom object this parameter is unnecessary
    #  @param Copy allows copying the element (Copy is 1) or replacing it with its mirror (Copy is 0)
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def MirrorObject (self, theObject, Mirror, theMirrorType=None, Copy=0, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Mirror, geomBuilder.GEOM._objref_GEOM_Object)):
            Mirror        = self.smeshpyD.GetAxisStruct(Mirror)
            theMirrorType = Mirror._mirrorType
        else:
            self.mesh.SetParameters(Mirror.parameters)
        if Copy and MakeGroups:
            return self.editor.MirrorObjectMakeGroups(theObject, Mirror, theMirrorType)
        self.editor.MirrorObject(theObject, Mirror, theMirrorType, Copy)
        return []

    ## Creates a new mesh by a symmetrical copy of the object
    #  @param theObject mesh, submesh or group
    #  @param Mirror AxisStruct or geom object (point, line, plane)
    #  @param theMirrorType smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
    #         If the Mirror is a geom object this parameter is unnecessary
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the new mesh to create
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def MirrorObjectMakeMesh (self, theObject, Mirror, theMirrorType=0,MakeGroups=0,NewMeshName=""):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Mirror, geomBuilder.GEOM._objref_GEOM_Object)):
            Mirror        = self.smeshpyD.GetAxisStruct(Mirror)
            theMirrorType = Mirror._mirrorType
        else:
            self.mesh.SetParameters(Mirror.parameters)
        mesh = self.editor.MirrorObjectMakeMesh(theObject, Mirror, theMirrorType,
                                                MakeGroups, NewMeshName)
        return Mesh( self.smeshpyD,self.geompyD,mesh )

    ## Translates the elements
    #  @param IDsOfElements list of elements ids
    #  @param Vector the direction of translation (DirStruct or vector or 3 vector components)
    #  @param Copy allows copying the translated elements
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def Translate(self, IDsOfElements, Vector, Copy, MakeGroups=False):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Vector, geomBuilder.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if isinstance( Vector, list ):
            Vector = self.smeshpyD.MakeDirStruct(*Vector)
        self.mesh.SetParameters(Vector.PS.parameters)
        if Copy and MakeGroups:
            return self.editor.TranslateMakeGroups(IDsOfElements, Vector)
        self.editor.Translate(IDsOfElements, Vector, Copy)
        return []

    ## Creates a new mesh of translated elements
    #  @param IDsOfElements list of elements ids
    #  @param Vector the direction of translation (DirStruct or vector or 3 vector components)
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the newly created mesh
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def TranslateMakeMesh(self, IDsOfElements, Vector, MakeGroups=False, NewMeshName=""):
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Vector, geomBuilder.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if isinstance( Vector, list ):
            Vector = self.smeshpyD.MakeDirStruct(*Vector)
        self.mesh.SetParameters(Vector.PS.parameters)
        mesh = self.editor.TranslateMakeMesh(IDsOfElements, Vector, MakeGroups, NewMeshName)
        return Mesh ( self.smeshpyD, self.geompyD, mesh )

    ## Translates the object
    #  @param theObject the object to translate (mesh, submesh, or group)
    #  @param Vector direction of translation (DirStruct or geom vector or 3 vector components)
    #  @param Copy allows copying the translated elements
    #  @param MakeGroups forces the generation of new groups from existing ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
    #  @ingroup l2_modif_trsf
    def TranslateObject(self, theObject, Vector, Copy, MakeGroups=False):
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( Vector, geomBuilder.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if isinstance( Vector, list ):
            Vector = self.smeshpyD.MakeDirStruct(*Vector)
        self.mesh.SetParameters(Vector.PS.parameters)
        if Copy and MakeGroups:
            return self.editor.TranslateObjectMakeGroups(theObject, Vector)
        self.editor.TranslateObject(theObject, Vector, Copy)
        return []

    ## Creates a new mesh from the translated object
    #  @param theObject the object to translate (mesh, submesh, or group)
    #  @param Vector the direction of translation (DirStruct or geom vector or 3 vector components)
    #  @param MakeGroups forces the generation of new groups from existing ones
    #  @param NewMeshName the name of the newly created mesh
    #  @return instance of Mesh class
    #  @ingroup l2_modif_trsf
    def TranslateObjectMakeMesh(self, theObject, Vector, MakeGroups=False, NewMeshName=""):
        if isinstance( theObject, Mesh ):
            theObject = theObject.GetMesh()
        if isinstance( Vector, geomBuilder.GEOM._objref_GEOM_Object ):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if isinstance( Vector, list ):
            Vector = self.smeshpyD.MakeDirStruct(*Vector)
        self.mesh.SetParameters(Vector.PS.parameters)
        mesh = self.editor.TranslateObjectMakeMesh(theObject, Vector, MakeGroups, NewMeshName)
        return Mesh( self.smeshpyD, self.geompyD, mesh )



    ## Scales the object
    #  @param theObject - the object to translate (mesh, submesh, or group)
    #  @param thePoint - base point for scale (SMESH.PointStruct or list of 3 coordinates)
    #  @param theScaleFact - list of 1-3 scale factors for axises
    #  @param Copy - allows copying the translated elements
    #  @param MakeGroups - forces the generation of new groups from existing
    #                      ones (if Copy)
    #  @return list of created groups (SMESH_GroupBase) if MakeGroups=True,
    #          empty list otherwise
    def Scale(self, theObject, thePoint, theScaleFact, Copy, MakeGroups=False):
        unRegister = genObjUnRegister()
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if ( isinstance( theObject, list )):
            theObject = self.GetIDSource(theObject, SMESH.ALL)
            unRegister.set( theObject )
        if ( isinstance( thePoint, list )):
            thePoint = PointStruct( thePoint[0], thePoint[1], thePoint[2] )
        if ( isinstance( theScaleFact, float )):
             theScaleFact = [theScaleFact]
        if ( isinstance( theScaleFact, int )):
             theScaleFact = [ float(theScaleFact)]

        self.mesh.SetParameters(thePoint.parameters)

        if Copy and MakeGroups:
            return self.editor.ScaleMakeGroups(theObject, thePoint, theScaleFact)
        self.editor.Scale(theObject, thePoint, theScaleFact, Copy)
        return []

    ## Creates a new mesh from the translated object
    #  @param theObject - the object to translate (mesh, submesh, or group)
    #  @param thePoint - base point for scale (SMESH.PointStruct or list of 3 coordinates)
    #  @param theScaleFact - list of 1-3 scale factors for axises
    #  @param MakeGroups - forces the generation of new groups from existing ones
    #  @param NewMeshName - the name of the newly created mesh
    #  @return instance of Mesh class
    def ScaleMakeMesh(self, theObject, thePoint, theScaleFact, MakeGroups=False, NewMeshName=""):
        unRegister = genObjUnRegister()
        if (isinstance(theObject, Mesh)):
            theObject = theObject.GetMesh()
        if ( isinstance( theObject, list )):
            theObject = self.GetIDSource(theObject,SMESH.ALL)
            unRegister.set( theObject )
        if ( isinstance( thePoint, list )):
            thePoint = PointStruct( thePoint[0], thePoint[1], thePoint[2] )
        if ( isinstance( theScaleFact, float )):
             theScaleFact = [theScaleFact]
        if ( isinstance( theScaleFact, int )):
             theScaleFact = [ float(theScaleFact)]

        self.mesh.SetParameters(thePoint.parameters)
        mesh = self.editor.ScaleMakeMesh(theObject, thePoint, theScaleFact,
                                         MakeGroups, NewMeshName)
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
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Axis, geomBuilder.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        AngleInRadians,Parameters,hasVars = ParseAngles(AngleInRadians)
        Parameters = Axis.parameters + var_separator + Parameters
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
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Axis, geomBuilder.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        AngleInRadians,Parameters,hasVars = ParseAngles(AngleInRadians)
        Parameters = Axis.parameters + var_separator + Parameters
        self.mesh.SetParameters(Parameters)
        mesh = self.editor.RotateMakeMesh(IDsOfElements, Axis, AngleInRadians,
                                          MakeGroups, NewMeshName)
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
        if (isinstance(theObject, Mesh)):
            theObject = theObject.GetMesh()
        if (isinstance(Axis, geomBuilder.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        AngleInRadians,Parameters,hasVars = ParseAngles(AngleInRadians)
        Parameters = Axis.parameters + ":" + Parameters
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
        if (isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if (isinstance(Axis, geomBuilder.GEOM._objref_GEOM_Object)):
            Axis = self.smeshpyD.GetAxisStruct(Axis)
        AngleInRadians,Parameters,hasVars = ParseAngles(AngleInRadians)
        Parameters = Axis.parameters + ":" + Parameters
        mesh = self.editor.RotateObjectMakeMesh(theObject, Axis, AngleInRadians,
                                                       MakeGroups, NewMeshName)
        self.mesh.SetParameters(Parameters)
        return Mesh( self.smeshpyD, self.geompyD, mesh )

    ## Finds groups of adjacent nodes within Tolerance.
    #  @param Tolerance the value of tolerance
    #  @param SeparateCornerAndMediumNodes if @c True, in quadratic mesh puts
    #         corner and medium nodes in separate groups thus preventing
    #         their further merge.
    #  @return the list of groups of nodes IDs (e.g. [[1,12,13],[4,25]])
    #  @ingroup l2_modif_trsf
    def FindCoincidentNodes (self, Tolerance, SeparateCornerAndMediumNodes=False):
        return self.editor.FindCoincidentNodes( Tolerance, SeparateCornerAndMediumNodes )

    ## Finds groups of ajacent nodes within Tolerance.
    #  @param Tolerance the value of tolerance
    #  @param SubMeshOrGroup SubMesh, Group or Filter
    #  @param exceptNodes list of either SubMeshes, Groups or node IDs to exclude from search
    #  @param SeparateCornerAndMediumNodes if @c True, in quadratic mesh puts
    #         corner and medium nodes in separate groups thus preventing
    #         their further merge.
    #  @return the list of groups of nodes IDs (e.g. [[1,12,13],[4,25]])
    #  @ingroup l2_modif_trsf
    def FindCoincidentNodesOnPart (self, SubMeshOrGroup, Tolerance,
                                   exceptNodes=[], SeparateCornerAndMediumNodes=False):
        unRegister = genObjUnRegister()
        if (isinstance( SubMeshOrGroup, Mesh )):
            SubMeshOrGroup = SubMeshOrGroup.GetMesh()
        if not isinstance( exceptNodes, list ):
            exceptNodes = [ exceptNodes ]
        if exceptNodes and isinstance( exceptNodes[0], int ):
            exceptNodes = [ self.GetIDSource( exceptNodes, SMESH.NODE )]
            unRegister.set( exceptNodes )
        return self.editor.FindCoincidentNodesOnPartBut(SubMeshOrGroup, Tolerance,
                                                        exceptNodes, SeparateCornerAndMediumNodes)

    ## Merges nodes
    #  @param GroupsOfNodes a list of groups of nodes IDs for merging
    #         (e.g. [[1,12,13],[25,4]], then nodes 12, 13 and 4 will be removed and replaced
    #         by nodes 1 and 25 correspondingly in all elements and groups
    #  @param NodesToKeep nodes to keep in the mesh: a list of groups, sub-meshes or node IDs.
    #         If @a NodesToKeep does not include a node to keep for some group to merge,
    #         then the first node in the group is kept.
    #  @ingroup l2_modif_trsf
    def MergeNodes (self, GroupsOfNodes, NodesToKeep=[]):
        # NodesToKeep are converted to SMESH_IDSource in meshEditor.MergeNodes()
        self.editor.MergeNodes(GroupsOfNodes,NodesToKeep)

    ## Finds the elements built on the same nodes.
    #  @param MeshOrSubMeshOrGroup Mesh or SubMesh, or Group of elements for searching
    #  @return the list of groups of equal elements IDs (e.g. [[1,12,13],[4,25]])
    #  @ingroup l2_modif_trsf
    def FindEqualElements (self, MeshOrSubMeshOrGroup=None):
        if not MeshOrSubMeshOrGroup:
            MeshOrSubMeshOrGroup=self.mesh
        elif isinstance( MeshOrSubMeshOrGroup, Mesh ):
            MeshOrSubMeshOrGroup = MeshOrSubMeshOrGroup.GetMesh()
        return self.editor.FindEqualElements( MeshOrSubMeshOrGroup )

    ## Merges elements in each given group.
    #  @param GroupsOfElementsID a list of groups of elements IDs for merging
    #        (e.g. [[1,12,13],[25,4]], then elements 12, 13 and 4 will be removed and
    #        replaced by elements 1 and 25 in all groups)
    #  @ingroup l2_modif_trsf
    def MergeElements(self, GroupsOfElementsID):
        self.editor.MergeElements(GroupsOfElementsID)

    ## Leaves one element and removes all other elements built on the same nodes.
    #  @ingroup l2_modif_trsf
    def MergeEqualElements(self):
        self.editor.MergeEqualElements()

    ## Returns groups of FreeBorder's coincident within the given tolerance.
    #  @param tolerance the tolerance. If the tolerance <= 0.0 then one tenth of an average
    #         size of elements adjacent to free borders being compared is used.
    #  @return SMESH.CoincidentFreeBorders structure
    #  @ingroup l2_modif_trsf
    def FindCoincidentFreeBorders (self, tolerance=0.):
        return self.editor.FindCoincidentFreeBorders( tolerance )
        
    ## Sew FreeBorder's of each group
    #  @param freeBorders either a SMESH.CoincidentFreeBorders structure or a list of lists
    #         where each enclosed list contains node IDs of a group of coincident free
    #         borders such that each consequent triple of IDs within a group describes
    #         a free border in a usual way: n1, n2, nLast - i.e. 1st node, 2nd node and
    #         last node of a border.
    #         For example [[1, 2, 10, 20, 21, 40], [11, 12, 15, 55, 54, 41]] describes two
    #         groups of coincident free borders, each group including two borders.
    #  @param createPolygons if @c True faces adjacent to free borders are converted to
    #         polygons if a node of opposite border falls on a face edge, else such
    #         faces are split into several ones.
    #  @param createPolyhedra if @c True volumes adjacent to free borders are converted to
    #         polyhedra if a node of opposite border falls on a volume edge, else such
    #         volumes, if any, remain intact and the mesh becomes non-conformal.
    #  @return a number of successfully sewed groups
    #  @ingroup l2_modif_trsf
    def SewCoincidentFreeBorders (self, freeBorders, createPolygons=False, createPolyhedra=False):
        if freeBorders and isinstance( freeBorders, list ):
            # construct SMESH.CoincidentFreeBorders
            if isinstance( freeBorders[0], int ):
                freeBorders = [freeBorders]
            borders = []
            coincidentGroups = []
            for nodeList in freeBorders:
                if not nodeList or len( nodeList ) % 3:
                    raise ValueError, "Wrong number of nodes in this group: %s" % nodeList
                group = []
                while nodeList:
                    group.append  ( SMESH.FreeBorderPart( len(borders), 0, 1, 2 ))
                    borders.append( SMESH.FreeBorder( nodeList[:3] ))
                    nodeList = nodeList[3:]
                    pass
                coincidentGroups.append( group )
                pass
            freeBorders = SMESH.CoincidentFreeBorders( borders, coincidentGroups )

        return self.editor.SewCoincidentFreeBorders( freeBorders, createPolygons, createPolyhedra )

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

    ## Clears sequences of nodes and elements created by mesh edition oparations
    #  @ingroup l1_auxiliary
    def ClearLastCreated(self):
        self.editor.ClearLastCreated()

    ## Creates duplicates of given elements, i.e. creates new elements based on the 
    #  same nodes as the given ones.
    #  @param theElements - container of elements to duplicate. It can be a Mesh,
    #         sub-mesh, group, filter or a list of element IDs. If \a theElements is
    #         a Mesh, elements of highest dimension are duplicated
    #  @param theGroupName - a name of group to contain the generated elements.
    #                    If a group with such a name already exists, the new elements
    #                    are added to the existng group, else a new group is created.
    #                    If \a theGroupName is empty, new elements are not added 
    #                    in any group.
    # @return a group where the new elements are added. None if theGroupName == "".
    #  @ingroup l2_modif_edit
    def DoubleElements(self, theElements, theGroupName=""):
        unRegister = genObjUnRegister()
        if isinstance( theElements, Mesh ):
            theElements = theElements.mesh
        elif isinstance( theElements, list ):
            theElements = self.GetIDSource( theElements, SMESH.ALL )
            unRegister.set( theElements )
        return self.editor.DoubleElements(theElements, theGroupName)

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
    #  @param theMakeGroup forces the generation of a group containing new nodes.
    #  @return TRUE if operation has been completed successfully, FALSE otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeGroups(self, theNodes, theModifiedElems, theMakeGroup=False):
        if theMakeGroup:
            return self.editor.DoubleNodeGroupsNew(theNodes, theModifiedElems)
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
    #  @param theMakeNodeGroup forces the generation of a group containing new nodes.
    #  @return TRUE or created groups (one or two) if operation has been completed successfully,
    #          FALSE or None otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeElemGroup(self, theElems, theNodesNot, theAffectedElems,
                             theMakeGroup=False, theMakeNodeGroup=False):
        if theMakeGroup or theMakeNodeGroup:
            twoGroups = self.editor.DoubleNodeElemGroup2New(theElems, theNodesNot,
                                                            theAffectedElems,
                                                            theMakeGroup, theMakeNodeGroup)
            if theMakeGroup and theMakeNodeGroup:
                return twoGroups
            else:
                return twoGroups[ int(theMakeNodeGroup) ]
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
    #  @param theMakeGroup forces the generation of a group containing new elements.
    #  @param theMakeNodeGroup forces the generation of a group containing new nodes.
    #  @return TRUE or created groups (one or two) if operation has been completed successfully,
    #          FALSE or None otherwise
    #  @ingroup l2_modif_edit
    def DoubleNodeElemGroups(self, theElems, theNodesNot, theAffectedElems,
                             theMakeGroup=False, theMakeNodeGroup=False):
        if theMakeGroup or theMakeNodeGroup:
            twoGroups = self.editor.DoubleNodeElemGroups2New(theElems, theNodesNot,
                                                             theAffectedElems,
                                                             theMakeGroup, theMakeNodeGroup)
            if theMakeGroup and theMakeNodeGroup:
                return twoGroups
            else:
                return twoGroups[ int(theMakeNodeGroup) ]
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

    ## Identify the elements that will be affected by node duplication (actual duplication is not performed.
    #  This method is the first step of DoubleNodeElemGroupsInRegion.
    #  @param theElems - list of groups of elements (edges or faces) to be replicated
    #  @param theNodesNot - list of groups of nodes not to replicated
    #  @param theShape - shape to detect affected elements (element which geometric center
    #         located on or inside shape).
    #         The replicated nodes should be associated to affected elements.
    #  @return groups of affected elements
    #  @ingroup l2_modif_edit
    def AffectedElemGroupsInRegion(self, theElems, theNodesNot, theShape):
        return self.editor.AffectedElemGroupsInRegion(theElems, theNodesNot, theShape)

    ## Double nodes on shared faces between groups of volumes and create flat elements on demand.
    # The list of groups must describe a partition of the mesh volumes.
    # The nodes of the internal faces at the boundaries of the groups are doubled.
    # In option, the internal faces are replaced by flat elements.
    # Triangles are transformed in prisms, and quadrangles in hexahedrons.
    # @param theDomains - list of groups of volumes
    # @param createJointElems - if TRUE, create the elements
    # @param onAllBoundaries - if TRUE, the nodes and elements are also created on
    #        the boundary between \a theDomains and the rest mesh
    # @return TRUE if operation has been completed successfully, FALSE otherwise
    def DoubleNodesOnGroupBoundaries(self, theDomains, createJointElems, onAllBoundaries=False ):
       return self.editor.DoubleNodesOnGroupBoundaries( theDomains, createJointElems, onAllBoundaries )

    ## Double nodes on some external faces and create flat elements.
    # Flat elements are mainly used by some types of mechanic calculations.
    #
    # Each group of the list must be constituted of faces.
    # Triangles are transformed in prisms, and quadrangles in hexahedrons.
    # @param theGroupsOfFaces - list of groups of faces
    # @return TRUE if operation has been completed successfully, FALSE otherwise
    def CreateFlatElementsOnFacesGroups(self, theGroupsOfFaces ):
        return self.editor.CreateFlatElementsOnFacesGroups( theGroupsOfFaces )
    
    ## identify all the elements around a geom shape, get the faces delimiting the hole
    #
    def CreateHoleSkin(self, radius, theShape, groupName, theNodesCoords):
        return self.editor.CreateHoleSkin( radius, theShape, groupName, theNodesCoords )

    def _getFunctor(self, funcType ):
        fn = self.functors[ funcType._v ]
        if not fn:
            fn = self.smeshpyD.GetFunctor(funcType)
            fn.SetMesh(self.mesh)
            self.functors[ funcType._v ] = fn
        return fn

    ## Returns value of a functor for a given element
    #  @param funcType an item of SMESH.FunctorType enum
    #         Type "SMESH.FunctorType._items" in the Python Console to see all items.
    #  @param elemId element or node ID
    #  @param isElem @a elemId is ID of element or node
    #  @return the functor value or zero in case of invalid arguments
    def FunctorValue(self, funcType, elemId, isElem=True):
        fn = self._getFunctor( funcType )
        if fn.GetElementType() == self.GetElementType(elemId, isElem):
            val = fn.GetValue(elemId)
        else:
            val = 0
        return val

    ## Get length of 1D element or sum of lengths of all 1D mesh elements
    #  @param elemId mesh element ID (if not defined - sum of length of all 1D elements will be calculated)
    #  @return element's length value if \a elemId is specified or sum of all 1D mesh elements' lengths otherwise
    #  @ingroup l1_measurements
    def GetLength(self, elemId=None):
        length = 0
        if elemId == None:
            length = self.smeshpyD.GetLength(self)
        else:
            length = self.FunctorValue(SMESH.FT_Length, elemId)
        return length

    ## Get area of 2D element or sum of areas of all 2D mesh elements
    #  @param elemId mesh element ID (if not defined - sum of areas of all 2D elements will be calculated)
    #  @return element's area value if \a elemId is specified or sum of all 2D mesh elements' areas otherwise
    #  @ingroup l1_measurements
    def GetArea(self, elemId=None):
        area = 0
        if elemId == None:
            area = self.smeshpyD.GetArea(self)
        else:
            area = self.FunctorValue(SMESH.FT_Area, elemId)
        return area

    ## Get volume of 3D element or sum of volumes of all 3D mesh elements
    #  @param elemId mesh element ID (if not defined - sum of volumes of all 3D elements will be calculated)
    #  @return element's volume value if \a elemId is specified or sum of all 3D mesh elements' volumes otherwise
    #  @ingroup l1_measurements
    def GetVolume(self, elemId=None):
        volume = 0
        if elemId == None:
            volume = self.smeshpyD.GetVolume(self)
        else:
            volume = self.FunctorValue(SMESH.FT_Volume3D, elemId)
        return volume

    ## Get maximum element length.
    #  @param elemId mesh element ID
    #  @return element's maximum length value
    #  @ingroup l1_measurements
    def GetMaxElementLength(self, elemId):
        if self.GetElementType(elemId, True) == SMESH.VOLUME:
            ftype = SMESH.FT_MaxElementLength3D
        else:
            ftype = SMESH.FT_MaxElementLength2D
        return self.FunctorValue(ftype, elemId)

    ## Get aspect ratio of 2D or 3D element.
    #  @param elemId mesh element ID
    #  @return element's aspect ratio value
    #  @ingroup l1_measurements
    def GetAspectRatio(self, elemId):
        if self.GetElementType(elemId, True) == SMESH.VOLUME:
            ftype = SMESH.FT_AspectRatio3D
        else:
            ftype = SMESH.FT_AspectRatio
        return self.FunctorValue(ftype, elemId)

    ## Get warping angle of 2D element.
    #  @param elemId mesh element ID
    #  @return element's warping angle value
    #  @ingroup l1_measurements
    def GetWarping(self, elemId):
        return self.FunctorValue(SMESH.FT_Warping, elemId)

    ## Get minimum angle of 2D element.
    #  @param elemId mesh element ID
    #  @return element's minimum angle value
    #  @ingroup l1_measurements
    def GetMinimumAngle(self, elemId):
        return self.FunctorValue(SMESH.FT_MinimumAngle, elemId)

    ## Get taper of 2D element.
    #  @param elemId mesh element ID
    #  @return element's taper value
    #  @ingroup l1_measurements
    def GetTaper(self, elemId):
        return self.FunctorValue(SMESH.FT_Taper, elemId)

    ## Get skew of 2D element.
    #  @param elemId mesh element ID
    #  @return element's skew value
    #  @ingroup l1_measurements
    def GetSkew(self, elemId):
        return self.FunctorValue(SMESH.FT_Skew, elemId)

    ## Return minimal and maximal value of a given functor.
    #  @param funType a functor type, an item of SMESH.FunctorType enum
    #         (one of SMESH.FunctorType._items)
    #  @param meshPart a part of mesh (group, sub-mesh) to treat
    #  @return tuple (min,max)
    #  @ingroup l1_measurements
    def GetMinMax(self, funType, meshPart=None):
        unRegister = genObjUnRegister()
        if isinstance( meshPart, list ):
            meshPart = self.GetIDSource( meshPart, SMESH.ALL )
            unRegister.set( meshPart )
        if isinstance( meshPart, Mesh ):
            meshPart = meshPart.mesh
        fun = self._getFunctor( funType )
        if fun:
            if meshPart:
                if hasattr( meshPart, "SetMesh" ):
                    meshPart.SetMesh( self.mesh ) # set mesh to filter
                hist = fun.GetLocalHistogram( 1, False, meshPart )
            else:
                hist = fun.GetHistogram( 1, False )
            if hist:
                return hist[0].min, hist[0].max
        return None

    pass # end of Mesh class


## Class used to compensate change of CORBA API of SMESH_Mesh for backward compatibility
#  with old dump scripts which call SMESH_Mesh directly and not via smeshBuilder.Mesh
#
class meshProxy(SMESH._objref_SMESH_Mesh):
    def __init__(self):
        SMESH._objref_SMESH_Mesh.__init__(self)
    def __deepcopy__(self, memo=None):
        new = self.__class__()
        return new
    def CreateDimGroup(self,*args): # 2 args added: nbCommonNodes, underlyingOnly
        if len( args ) == 3:
            args += SMESH.ALL_NODES, True
        return SMESH._objref_SMESH_Mesh.CreateDimGroup( self, *args )
    pass
omniORB.registerObjref(SMESH._objref_SMESH_Mesh._NP_RepositoryId, meshProxy)


## Class wrapping SMESH_SubMesh in order to add Compute()
#
class submeshProxy(SMESH._objref_SMESH_subMesh):
    def __init__(self):
        SMESH._objref_SMESH_subMesh.__init__(self)
        self.mesh = None
    def __deepcopy__(self, memo=None):
        new = self.__class__()
        return new

    ## Computes the sub-mesh and returns the status of the computation
    #  @param refresh if @c True, Object browser is automatically updated (when running in GUI)
    #  @return True or False
    #  @ingroup l2_construct
    def Compute(self,refresh=False):
        if not self.mesh:
            self.mesh = Mesh( smeshBuilder(), None, self.GetMesh())

        ok = self.mesh.Compute( self.GetSubShape(),refresh=[] )

        if salome.sg.hasDesktop() and self.mesh.GetStudyId() >= 0:
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self ), ok, (self.GetNumberOfElements()==0) )
            if refresh: salome.sg.updateObjBrowser(True)
            pass

        return ok
    pass
omniORB.registerObjref(SMESH._objref_SMESH_subMesh._NP_RepositoryId, submeshProxy)


## Class used to compensate change of CORBA API of SMESH_MeshEditor for backward compatibility
#  with old dump scripts which call SMESH_MeshEditor directly and not via smeshBuilder.Mesh
#
class meshEditor(SMESH._objref_SMESH_MeshEditor):
    def __init__(self):
        SMESH._objref_SMESH_MeshEditor.__init__(self)
        self.mesh = None
    def __getattr__(self, name ): # method called if an attribute not found
        if not self.mesh:         # look for name() method in Mesh class
            self.mesh = Mesh( None, None, SMESH._objref_SMESH_MeshEditor.GetMesh(self))
        if hasattr( self.mesh, name ):
            return getattr( self.mesh, name )
        if name == "ExtrusionAlongPathObjX":
            return getattr( self.mesh, "ExtrusionAlongPathX" ) # other method name
        print "meshEditor: attribute '%s' NOT FOUND" % name
        return None
    def __deepcopy__(self, memo=None):
        new = self.__class__()
        return new
    def FindCoincidentNodes(self,*args): # a 2nd arg added (SeparateCornerAndMediumNodes)
        if len( args ) == 1: args += False,
        return SMESH._objref_SMESH_MeshEditor.FindCoincidentNodes( self, *args )
    def FindCoincidentNodesOnPart(self,*args): # a 3d arg added (SeparateCornerAndMediumNodes)
        if len( args ) == 2: args += False,
        return SMESH._objref_SMESH_MeshEditor.FindCoincidentNodesOnPart( self, *args )
    def MergeNodes(self,*args): # a 2nd arg added (NodesToKeep)
        if len( args ) == 1:
            return SMESH._objref_SMESH_MeshEditor.MergeNodes( self, args[0], [] )
        NodesToKeep = args[1]
        unRegister  = genObjUnRegister()
        if NodesToKeep:
            if isinstance( NodesToKeep, list ) and isinstance( NodesToKeep[0], int ):
                NodesToKeep = self.MakeIDSource( NodesToKeep, SMESH.NODE )
            if not isinstance( NodesToKeep, list ):
                NodesToKeep = [ NodesToKeep ]
        return SMESH._objref_SMESH_MeshEditor.MergeNodes( self, args[0], NodesToKeep )
    pass
omniORB.registerObjref(SMESH._objref_SMESH_MeshEditor._NP_RepositoryId, meshEditor)

## Helper class for wrapping of SMESH.SMESH_Pattern CORBA class
#
class Pattern(SMESH._objref_SMESH_Pattern):

    def LoadFromFile(self, patternTextOrFile ):
        text = patternTextOrFile
        if os.path.exists( text ):
            text = open( patternTextOrFile ).read()
            pass
        return SMESH._objref_SMESH_Pattern.LoadFromFile( self, text )

    def ApplyToMeshFaces(self, theMesh, theFacesIDs, theNodeIndexOnKeyPoint1, theReverse):
        decrFun = lambda i: i-1
        theNodeIndexOnKeyPoint1,Parameters,hasVars = ParseParameters(theNodeIndexOnKeyPoint1, decrFun)
        theMesh.SetParameters(Parameters)
        return SMESH._objref_SMESH_Pattern.ApplyToMeshFaces( self, theMesh, theFacesIDs, theNodeIndexOnKeyPoint1, theReverse )

    def ApplyToHexahedrons(self, theMesh, theVolumesIDs, theNode000Index, theNode001Index):
        decrFun = lambda i: i-1
        theNode000Index,theNode001Index,Parameters,hasVars = ParseParameters(theNode000Index,theNode001Index, decrFun)
        theMesh.SetParameters(Parameters)
        return SMESH._objref_SMESH_Pattern.ApplyToHexahedrons( self, theMesh, theVolumesIDs, theNode000Index, theNode001Index )

    def MakeMesh(self, mesh, CreatePolygons=False, CreatePolyhedra=False):
        if isinstance( mesh, Mesh ):
            mesh = mesh.GetMesh()
        return SMESH._objref_SMESH_Pattern.MakeMesh( self, mesh, CreatePolygons, CreatePolyhedra )

# Registering the new proxy for Pattern
omniORB.registerObjref(SMESH._objref_SMESH_Pattern._NP_RepositoryId, Pattern)

## Private class used to bind methods creating algorithms to the class Mesh
#
class algoCreator:
    def __init__(self):
        self.mesh = None
        self.defaultAlgoType = ""
        self.algoTypeToClass = {}

    # Stores a python class of algorithm
    def add(self, algoClass):
        if type( algoClass ).__name__ == 'classobj' and \
           hasattr( algoClass, "algoType"):
            self.algoTypeToClass[ algoClass.algoType ] = algoClass
            if not self.defaultAlgoType and \
               hasattr( algoClass, "isDefault") and algoClass.isDefault:
                self.defaultAlgoType = algoClass.algoType
            #print "Add",algoClass.algoType, "dflt",self.defaultAlgoType

    # creates a copy of self and assign mesh to the copy
    def copy(self, mesh):
        other = algoCreator()
        other.defaultAlgoType = self.defaultAlgoType
        other.algoTypeToClass  = self.algoTypeToClass
        other.mesh = mesh
        return other

    # creates an instance of algorithm
    def __call__(self,algo="",geom=0,*args):
        algoType = self.defaultAlgoType
        for arg in args + (algo,geom):
            if isinstance( arg, geomBuilder.GEOM._objref_GEOM_Object ):
                geom = arg
            if isinstance( arg, str ) and arg:
                algoType = arg
        if not algoType and self.algoTypeToClass:
            algoType = self.algoTypeToClass.keys()[0]
        if self.algoTypeToClass.has_key( algoType ):
            #print "Create algo",algoType
            return self.algoTypeToClass[ algoType ]( self.mesh, geom )
        raise RuntimeError, "No class found for algo type %s" % algoType
        return None

## Private class used to substitute and store variable parameters of hypotheses.
#
class hypMethodWrapper:
    def __init__(self, hyp, method):
        self.hyp    = hyp
        self.method = method
        #print "REBIND:", method.__name__
        return

    # call a method of hypothesis with calling SetVarParameter() before
    def __call__(self,*args):
        if not args:
            return self.method( self.hyp, *args ) # hypothesis method with no args

        #print "MethWrapper.__call__",self.method.__name__, args
        try:
            parsed = ParseParameters(*args)     # replace variables with their values
            self.hyp.SetVarParameter( parsed[-2], self.method.__name__ )
            result = self.method( self.hyp, *parsed[:-2] ) # call hypothesis method
        except omniORB.CORBA.BAD_PARAM: # raised by hypothesis method call
            # maybe there is a replaced string arg which is not variable
            result = self.method( self.hyp, *args )
        except ValueError, detail: # raised by ParseParameters()
            try:
                result = self.method( self.hyp, *args )
            except omniORB.CORBA.BAD_PARAM:
                raise ValueError, detail # wrong variable name

        return result
    pass

## A helper class that call UnRegister() of SALOME.GenericObj'es stored in it
#
class genObjUnRegister:

    def __init__(self, genObj=None):
        self.genObjList = []
        self.set( genObj )
        return

    def set(self, genObj):
        "Store one or a list of of SALOME.GenericObj'es"
        if isinstance( genObj, list ):
            self.genObjList.extend( genObj )
        else:
            self.genObjList.append( genObj )
        return

    def __del__(self):
        for genObj in self.genObjList:
            if genObj and hasattr( genObj, "UnRegister" ):
                genObj.UnRegister()


## Bind methods creating mesher plug-ins to the Mesh class
#
for pluginName in os.environ[ "SMESH_MeshersList" ].split( ":" ):
    #
    #print "pluginName: ", pluginName
    pluginBuilderName = pluginName + "Builder"
    try:
        exec( "from salome.%s.%s import *" % (pluginName, pluginBuilderName))
    except Exception, e:
	from salome_utils import verbose
	if verbose(): print "Exception while loading %s: %s" % ( pluginBuilderName, e )
        continue
    exec( "from salome.%s import %s" % (pluginName, pluginBuilderName))
    plugin = eval( pluginBuilderName )
    #print "  plugin:" , str(plugin)

    # add methods creating algorithms to Mesh
    for k in dir( plugin ):
        if k[0] == '_': continue
        algo = getattr( plugin, k )
        #print "             algo:", str(algo)
        if type( algo ).__name__ == 'classobj' and hasattr( algo, "meshMethod" ):
            #print "                     meshMethod:" , str(algo.meshMethod)
            if not hasattr( Mesh, algo.meshMethod ):
                setattr( Mesh, algo.meshMethod, algoCreator() )
                pass
            getattr( Mesh, algo.meshMethod ).add( algo )
            pass
        pass
    pass
del pluginName
