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

import salome
from salome.geom import geomBuilder

import SMESH # This is necessary for back compatibility
from   SMESH import *
from   salome.smesh.smesh_algorithm import Mesh_Algorithm

import SALOME
import SALOMEDS
import os

class MeshMeta(type):
    """Private class used to workaround a problem that sometimes isinstance(m, Mesh) returns False
    """
    def __instancecheck__(cls, inst):
        """Implement isinstance(inst, cls)."""
        return any(cls.__subclasscheck__(c)
                   for c in {type(inst), inst.__class__})

    def __subclasscheck__(cls, sub):
        """Implement issubclass(sub, cls)."""
        return type.__subclasscheck__(cls, sub) or (cls.__name__ == sub.__name__ and cls.__module__ == sub.__module__)

def DegreesToRadians(AngleInDegrees):
    """Convert an angle from degrees to radians
    """
    from math import pi
    return AngleInDegrees * pi / 180.0

import salome_notebook
notebook = salome_notebook.notebook
# Salome notebook variable separator
var_separator = ":"

def ParseParameters(*args):
    """
    Return list of variable values from salome notebook.
    The last argument, if is callable, is used to modify values got from notebook
    """
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

def ParseAngles(*args):
    """
    Parse parameters while converting variables to radians
    """
    return ParseParameters( *( args + (DegreesToRadians, )))

def __initPointStruct(point,*args):
    """
    Substitute PointStruct.__init__() to create SMESH.PointStruct using notebook variables.
    Parameters are stored in PointStruct.parameters attribute
    """
    point.x, point.y, point.z, point.parameters,hasVars = ParseParameters(*args)
    pass
SMESH.PointStruct.__init__ = __initPointStruct

def __initAxisStruct(ax,*args):
    """
    Substitute AxisStruct.__init__() to create SMESH.AxisStruct using notebook variables.
    Parameters are stored in AxisStruct.parameters attribute
    """
    if len( args ) != 6:
        raise RuntimeError,\
              "Bad nb args (%s) passed in SMESH.AxisStruct(x,y,z,dx,dy,dz)"%(len( args ))
    ax.x, ax.y, ax.z, ax.vx, ax.vy, ax.vz, ax.parameters,hasVars = ParseParameters(*args)
    pass
SMESH.AxisStruct.__init__ = __initAxisStruct

smeshPrecisionConfusion = 1.e-07
def IsEqual(val1, val2, tol=smeshPrecisionConfusion):
    """Compare real values using smeshPrecisionConfusion as tolerance
    """
    if abs(val1 - val2) < tol:
        return True
    return False

NO_NAME = "NoName"

def GetName(obj):
    """
    Returns:
	object name
    """
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

def TreatHypoStatus(status, hypName, geomName, isAlgo, mesh):
    """
    Print error message if a hypothesis was not assigned.
    """
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
    elif status == HYP_CONCURRENT :
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

def AssureGeomPublished(mesh, geom, name=''):
    """
    Private method. Add geom (sub-shape of the main shape) into the study if not yet there
    """
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
            # for all groups SubShapeName() return "Compound_-1"
            name = mesh.geompyD.SubShapeName(geom, mesh.geom)
        if not name:
            name = "%s_%s"%(geom.GetShapeType(), id(geom)%10000)
        ## publish
        mesh.geompyD.addToStudyInFather( mesh.geom, geom, name )
    return

def FirstVertexOnCurve(mesh, edge):
    """
    Returns:
	the first vertex of a geometrical edge by ignoring orientation
    """
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

smeshInst = None
"""
Warning:
    smeshInst is a singleton
"""
engine = None
doLcc = False
created = False

class smeshBuilder(object, SMESH._objref_SMESH_Gen):
    """
    This class allows to create, load or manipulate meshes.
    It has a set of methods to create, load or copy meshes, to combine several meshes, etc.
    It also has methods to get infos and measure meshes.
    """

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

    def DumpPython(self, theStudy, theIsPublished=True, theIsMultiFile=True):
        """
        Dump component to the Python script
            This method overrides IDL function to allow default values for the parameters.
        """

        return SMESH._objref_SMESH_Gen.DumpPython(self, theStudy, theIsPublished, theIsMultiFile)

    def SetDumpPythonHistorical(self, isHistorical):
        """
        Set mode of DumpPython(), *historical* or *snapshot*.
            In the *historical* mode, the Python Dump script includes all commands
            performed by SMESH engine. In the *snapshot* mode, commands
            relating to objects removed from the Study are excluded from the script
            as well as commands not influencing the current state of meshes
        """

        if isHistorical: val = "true"
        else:            val = "false"
        SMESH._objref_SMESH_Gen.SetOption(self, "historical_python_dump", val)

    def init_smesh(self,theStudy,geompyD = None):
        """
        Set the current study and Geometry component
        """

        #print "init_smesh"
        self.SetCurrentStudy(theStudy,geompyD)
        if theStudy:
            global notebook
            notebook.myStudy = theStudy

    def Mesh(self, obj=0, name=0):
        """
        Create a mesh. This can be either an empty mesh, possibly having an underlying geometry,
        or a mesh wrapping a CORBA mesh given as a parameter.

        Parameters:
            obj: either (1) a CORBA mesh: (SMESH._objref_SMESH_Mesh) got e.g. by calling
                salome.myStudy.FindObjectID("0:1:2:3").GetObject() or
                (2) a Geometrical object: for meshing or (3) none:.
            name: the name for the new mesh.

        Returns:
            an instance of Mesh class.
        """

        if isinstance(obj,str):
            obj,name = name,obj
        return Mesh(self,self.geompyD,obj,name)

    def EnumToLong(self,theItem):
        """
        Return a long value from enumeration
        """

        return theItem._v

    def ColorToString(self,c):
        """
        Returns:
            a string representation of the color.
        To be used with filters.

        Parametrs:
            c: color value (SALOMEDS.Color)
        """

        val = ""
        if isinstance(c, SALOMEDS.Color):
            val = "%s;%s;%s" % (c.R, c.G, c.B)
        elif isinstance(c, str):
            val = c
        else:
            raise ValueError, "Color value should be of string or SALOMEDS.Color type"
        return val

    def GetPointStruct(self,theVertex):
        """
        Get PointStruct from vertex

	Parameters:
		theVertex: a GEOM object(vertex)

	Returns:
		SMESH.PointStruct
	"""

        [x, y, z] = self.geompyD.PointCoordinates(theVertex)
        return PointStruct(x,y,z)

    def GetDirStruct(self,theVector):
        """
	Get DirStruct from vector

	Parameters:
		theVector: a GEOM object(vector)

	Returns:
		SMESH.DirStruct
	"""

        vertices = self.geompyD.SubShapeAll( theVector, geomBuilder.geomBuilder.ShapeType["VERTEX"] )
        if(len(vertices) != 2):
            print "Error: vector object is incorrect."
            return None
        p1 = self.geompyD.PointCoordinates(vertices[0])
        p2 = self.geompyD.PointCoordinates(vertices[1])
        pnt = PointStruct(p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2])
        dirst = DirStruct(pnt)
        return dirst

    def MakeDirStruct(self,x,y,z):
	"""
	Make DirStruct from a triplet

	Parameters:
		x,y,z: vector components

	Returns:
		SMESH.DirStruct
	"""

        pnt = PointStruct(x,y,z)
        return DirStruct(pnt)

    def GetAxisStruct(self,theObj):
        """
        Get AxisStruct from object

        Parameters:
            theObj: a GEOM object (line or plane)

	Returns:
            SMESH.AxisStruct
        """
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

    def SetName(self, obj, name):
	"""
	Set the given name to the object

	Parameters:
		obj: the object to rename
		name: a new object name
	"""

        if isinstance( obj, Mesh ):
            obj = obj.GetMesh()
        elif isinstance( obj, Mesh_Algorithm ):
            obj = obj.GetAlgorithm()
        ior  = salome.orb.object_to_string(obj)
        SMESH._objref_SMESH_Gen.SetName(self, ior, name)

    def SetEmbeddedMode( self,theMode ):
	"""
	Set the current mode
	"""

        SMESH._objref_SMESH_Gen.SetEmbeddedMode(self,theMode)

    def IsEmbeddedMode(self):
	"""
	Get the current mode
	"""

        return SMESH._objref_SMESH_Gen.IsEmbeddedMode(self)

    def SetCurrentStudy( self, theStudy, geompyD = None ):
	"""
	Set the current study. Calling SetCurrentStudy( None ) allows to
		switch OFF automatic pubilishing in the Study of mesh objects.
	"""

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

    def GetCurrentStudy(self):
	"""
	Get the current study
	"""

        return SMESH._objref_SMESH_Gen.GetCurrentStudy(self)

    def CreateMeshesFromUNV( self,theFileName ):
	"""
	Create a Mesh object importing data from the given UNV file

	Returns:
		an instance of Mesh class
	"""

        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromUNV(self,theFileName)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh)
        return aMesh

    def CreateMeshesFromMED( self,theFileName ):
	"""
	Create a Mesh object(s) importing data from the given MED file

	Returns:
		a tuple ( list of Mesh class instances, SMESH.DriverMED_ReadStatus )
	"""

        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromMED(self,theFileName)
        aMeshes = [ Mesh(self, self.geompyD, m) for m in aSmeshMeshes ]
        return aMeshes, aStatus

    def CreateMeshesFromSAUV( self,theFileName ):
	"""
	Create a Mesh object(s) importing data from the given SAUV file

	Returns:
		a tuple ( list of Mesh class instances, SMESH.DriverMED_ReadStatus )
	"""

        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromSAUV(self,theFileName)
        aMeshes = [ Mesh(self, self.geompyD, m) for m in aSmeshMeshes ]
        return aMeshes, aStatus

    def CreateMeshesFromSTL( self, theFileName ):
	"""
	Create a Mesh object importing data from the given STL file

	Returns:
		an instance of Mesh class
	"""

        aSmeshMesh = SMESH._objref_SMESH_Gen.CreateMeshesFromSTL(self,theFileName)
        aMesh = Mesh(self, self.geompyD, aSmeshMesh)
        return aMesh

    def CreateMeshesFromCGNS( self, theFileName ):
	"""
	Create Mesh objects importing data from the given CGNS file

	Returns:
		a tuple ( list of Mesh class instances, SMESH.DriverMED_ReadStatus )
	"""

        aSmeshMeshes, aStatus = SMESH._objref_SMESH_Gen.CreateMeshesFromCGNS(self,theFileName)
        aMeshes = [ Mesh(self, self.geompyD, m) for m in aSmeshMeshes ]
        return aMeshes, aStatus

    def CreateMeshesFromGMF( self, theFileName ):
	"""
	Create a Mesh object importing data from the given GMF file.
	GMF files must have .mesh extension for the ASCII format and .meshb for
	the binary format.

	Returns:
		[ an instance of Mesh class, SMESH.ComputeError ]
	"""

        aSmeshMesh, error = SMESH._objref_SMESH_Gen.CreateMeshesFromGMF(self,
                                                                        theFileName,
                                                                        True)
        if error.comment: print "*** CreateMeshesFromGMF() errors:\n", error.comment
        return Mesh(self, self.geompyD, aSmeshMesh), error

    def Concatenate( self, meshes, uniteIdenticalGroups,
                     mergeNodesAndElements = False, mergeTolerance = 1e-5, allGroups = False,
                     name = ""):
	"""
	Concatenate the given meshes into one mesh. All groups of input meshes will be
		present in the new mesh.

	Parameters:
		meshes: the meshes, sub-meshes and groups to combine into one mesh
		uniteIdenticalGroups: if true, groups with same names are united, else they are renamed
		mergeNodesAndElements: if true, equal nodes and elements are merged
		mergeTolerance: tolerance for merging nodes
		allGroups: forces creation of groups corresponding to every input mesh
		name: name of a new mesh

	Returns:
		an instance of Mesh class
	"""

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

    def CopyMesh( self, meshPart, meshName, toCopyGroups=False, toKeepIDs=False):
	"""
	Create a mesh by copying a part of another mesh.

	Parameters:
		meshPart: a part of mesh to copy, either a Mesh, a sub-mesh or a group;
			to copy nodes or elements not contained in any mesh object,
			pass result of Mesh.GetIDSource( list_of_ids, type ) as meshPart
		meshName: a name of the new mesh
		toCopyGroups: to create in the new mesh groups the copied elements belongs to
		toKeepIDs: to preserve order of the copied elements or not

	Returns:
		an instance of Mesh class
	"""

        if (isinstance( meshPart, Mesh )):
            meshPart = meshPart.GetMesh()
        mesh = SMESH._objref_SMESH_Gen.CopyMesh( self,meshPart,meshName,toCopyGroups,toKeepIDs )
        return Mesh(self, self.geompyD, mesh)

    def GetSubShapesId( self, theMainObject, theListOfSubObjects ):
	"""
	Return IDs of sub-shapes

	Returns:
		the list of integer values
	"""

        return SMESH._objref_SMESH_Gen.GetSubShapesId(self,theMainObject, theListOfSubObjects)

    def GetPattern(self):
	"""
	Create a pattern mapper.

	Returns:
		an instance of SMESH_Pattern

        `Example of Patterns usage <../tui_modifying_meshes_page.html#tui_pattern_mapping>`_
	"""

        return SMESH._objref_SMESH_Gen.GetPattern(self)

    def SetBoundaryBoxSegmentation(self, nbSegments):
        """
        Set number of segments per diagonal of boundary box of geometry, by which
        default segment length of appropriate 1D hypotheses is defined in GUI.
        Default value is 10.
        """

        SMESH._objref_SMESH_Gen.SetBoundaryBoxSegmentation(self,nbSegments)

    # Filtering. Auxiliary functions:
    # ------------------------------

    def GetEmptyCriterion(self):
	"""
	Create an empty criterion

	Returns:
		SMESH.Filter.Criterion
	"""

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

    def GetCriterion(self,elementType,
                     CritType,
                     Compare = FT_EqualTo,
                     Threshold="",
                     UnaryOp=FT_Undefined,
                     BinaryOp=FT_Undefined,
                     Tolerance=1e-07):
	"""
	Create a criterion by the given parameters
	Criterion structures allow to define complex filters by combining them with logical operations (AND / OR) (see example below)

	Parameters:
		elementType: the type of elements(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
		CritType: the type of criterion (SMESH.FT_Taper, SMESH.FT_Area, etc.)
			Type SMESH.FunctorType._items in the Python Console to see all values.
			Note that the items starting from FT_LessThan are not suitable for CritType.
		Compare:  belongs to {SMESH.FT_LessThan, SMESH.FT_MoreThan, SMESH.FT_EqualTo}
		Threshold: the threshold value (range of ids as string, shape, numeric)
		UnaryOp:  SMESH.FT_LogicalNOT or SMESH.FT_Undefined
		BinaryOp: a binary logical operation SMESH.FT_LogicalAND, SMESH.FT_LogicalOR or
			SMESH.FT_Undefined
		Tolerance: the tolerance used by SMESH.FT_BelongToGeom, SMESH.FT_BelongToSurface,
			SMESH.FT_LyingOnGeom, SMESH.FT_CoplanarFaces criteria

	Returns:
		SMESH.Filter.Criterion

    	 href="../tui_filters_page.html#combining_filters"
	"""

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

    def GetFilter(self,elementType,
                  CritType=FT_Undefined,
                  Compare=FT_EqualTo,
                  Threshold="",
                  UnaryOp=FT_Undefined,
                  Tolerance=1e-07,
                  mesh=None):
	"""
	Create a filter with the given parameters

	Parameters:
		elementType: the type of elements (SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
		CritType: the type of criterion (SMESH.FT_Taper, SMESH.FT_Area, etc.)
			Type SMESH.FunctorType._items in the Python Console to see all values.
			Note that the items starting from FT_LessThan are not suitable for CritType.
		Compare: belongs to {SMESH.FT_LessThan, SMESH.FT_MoreThan, SMESH.FT_EqualTo}
		Threshold: the threshold value (range of ids as string, shape, numeric)
		UnaryOp:  SMESH.FT_LogicalNOT or SMESH.FT_Undefined
		Tolerance: the tolerance used by SMESH.FT_BelongToGeom, SMESH.FT_BelongToSurface,
			SMESH.FT_LyingOnGeom, SMESH.FT_CoplanarFaces and SMESH.FT_EqualNodes criteria
		mesh: the mesh to initialize the filter with

	Returns:
		SMESH_Filter

    	`Example of Filters usage <../tui_filters_page.html#tui_filters>`_
	"""

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

    def GetFilterFromCriteria(self,criteria, binOp=SMESH.FT_LogicalAND):
	"""
	Create a filter from criteria

	Parameters:
		criteria: a list of criteria
		binOp: binary operator used when binary operator of criteria is undefined

	Returns:
		SMESH_Filter

    	`Example of Filters usage <../tui_filters_page.html#tui_filters>`_
	"""

        for i in range( len( criteria ) - 1 ):
            if criteria[i].BinaryOp == self.EnumToLong( SMESH.FT_Undefined ):
                criteria[i].BinaryOp = self.EnumToLong( binOp )
        aFilterMgr = self.CreateFilterManager()
        aFilter = aFilterMgr.CreateFilter()
        aFilter.SetCriteria(criteria)
        aFilterMgr.UnRegister()
        return aFilter

    def GetFunctor(self,theCriterion):
	"""
	Create a numerical functor by its type

	Parameters:
		theCriterion: functor type - an item of SMESH.FunctorType enumeration.
			Type SMESH.FunctorType._items in the Python Console to see all items.
			Note that not all items correspond to numerical functors.

	Returns:
		SMESH_NumericalFunctor
	"""

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
        elif theCriterion == FT_Deflection2D:
            functor = aFilterMgr.CreateDeflection2D()
        elif theCriterion == FT_NodeConnectivityNumber:
            functor = aFilterMgr.CreateNodeConnectivityNumber()
        elif theCriterion == FT_BallDiameter:
            functor = aFilterMgr.CreateBallDiameter()
        else:
            print "Error: given parameter is not numerical functor type."
        aFilterMgr.UnRegister()
        return functor

    def CreateHypothesis(self, theHType, theLibName="libStdMeshersEngine.so"):
	"""
	Create hypothesis

	Parameters:
		theHType: mesh hypothesis type (string)
		theLibName: mesh plug-in library name

	Returns:
		created hypothesis instance
	"""
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

    def GetMeshInfo(self, obj):
	"""
	Get the mesh statistic

	Returns:
		dictionary "element type" - "count of elements"
	"""

        if isinstance( obj, Mesh ):
            obj = obj.GetMesh()
        d = {}
        if hasattr(obj, "GetMeshInfo"):
            values = obj.GetMeshInfo()
            for i in range(SMESH.Entity_Last._v):
                if i < len(values): d[SMESH.EntityType._item(i)]=values[i]
            pass
        return d

    def MinDistance(self, src1, src2=None, id1=0, id2=0, isElem1=False, isElem2=False):
	"""
	Get minimum distance between two objects

	If *src2* is None, and *id2* = 0, distance from *src1* / *id1* to the origin is computed.
	If *src2* None, and *id2* != 0, it is assumed that both *id1* and *id2* belong to *src1*.

	Parameters:
		src1: first source object
		src2: second source object
		id1: node/element id from the first source
		id2: node/element id from the second (or first) source
		isElem1: *True* if *id1* is element id, *False* if it is node id
		isElem2: *True* if *id2* is element id, *False* if it is node id

	Returns:
		minimum distance value *GetMinDistance()*
	"""

        result = self.GetMinDistance(src1, src2, id1, id2, isElem1, isElem2)
        if result is None:
            result = 0.0
        else:
            result = result.value
        return result

    def GetMinDistance(self, src1, src2=None, id1=0, id2=0, isElem1=False, isElem2=False):
	"""
	Get measure structure specifying minimum distance data between two objects

	If *src2* is None, and *id2*  = 0, distance from *src1* / *id1* to the origin is computed.
	If *src2* is None, and *id2* != 0, it is assumed that both *id1* and *id2* belong to *src1*.


	Parameters:
		src1: first source object
		src2: second source object
		id1: node/element id from the first source
		id2: node/element id from the second (or first) source
		isElem1: *True* if **id1** is element id, *False* if it is node id
		isElem2: *True* if **id2** is element id, *False* if it is node id

	Returns:
		Measure structure or None if input data is invalid **MinDistance()**
	"""

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

    def BoundingBox(self, objects):
	"""
	Get bounding box of the specified object(s)

	Parameters:
		objects: single source object or list of source objects

	Returns:
		tuple of six values (minX, minY, minZ, maxX, maxY, maxZ) **GetBoundingBox()**
	"""

        result = self.GetBoundingBox(objects)
        if result is None:
            result = (0.0,)*6
        else:
            result = (result.minX, result.minY, result.minZ, result.maxX, result.maxY, result.maxZ)
        return result

    def GetBoundingBox(self, objects):
	"""
	Get measure structure specifying bounding box data of the specified object(s)

	Parameters:
		objects: single source object or list of source objects

	Returns:
		Measure structure **BoundingBox()**
	"""

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

    def GetLength(self, obj):
	"""
	Get sum of lengths of all 1D elements in the mesh object.

	Parameters:
		obj: mesh, submesh or group

	Returns:
		sum of lengths of all 1D elements
	"""

        if isinstance(obj, Mesh): obj = obj.mesh
        if isinstance(obj, Mesh_Algorithm): obj = obj.GetSubMesh()
        aMeasurements = self.CreateMeasurements()
        value = aMeasurements.Length(obj)
        aMeasurements.UnRegister()
        return value

    def GetArea(self, obj):
	"""
	Get sum of areas of all 2D elements in the mesh object.

	Parameters:
		obj: mesh, submesh or group

	Returns:
		sum of areas of all 2D elements
	"""

        if isinstance(obj, Mesh): obj = obj.mesh
        if isinstance(obj, Mesh_Algorithm): obj = obj.GetSubMesh()
        aMeasurements = self.CreateMeasurements()
        value = aMeasurements.Area(obj)
        aMeasurements.UnRegister()
        return value

    def GetVolume(self, obj):
	"""
	Get sum of volumes of all 3D elements in the mesh object.

	Parameters:
		obj: mesh, submesh or group

	Returns:
		sum of volumes of all 3D elements
	"""

        if isinstance(obj, Mesh): obj = obj.mesh
        if isinstance(obj, Mesh_Algorithm): obj = obj.GetSubMesh()
        aMeasurements = self.CreateMeasurements()
        value = aMeasurements.Volume(obj)
        aMeasurements.UnRegister()
        return value

    def GetGravityCenter(self, obj):
        """
        Get gravity center of all nodes of the mesh object.
        
        Parameters:            
            obj: mesh, submesh or group

        Returns:        
            Three components of the gravity center: x,y,z
        """
        if isinstance(obj, Mesh): obj = obj.mesh
        if isinstance(obj, Mesh_Algorithm): obj = obj.GetSubMesh()
        aMeasurements = self.CreateMeasurements()
        pointStruct = aMeasurements.GravityCenter(obj)
        aMeasurements.UnRegister()
        return pointStruct.x, pointStruct.y, pointStruct.z

    pass # end of class smeshBuilder

import omniORB
omniORB.registerObjref(SMESH._objref_SMESH_Gen._NP_RepositoryId, smeshBuilder)
"""Registering the new proxy for SMESH_Gen"""


def New( study, instance=None, instanceGeom=None):
    """
    Create a new smeshBuilder instance.The smeshBuilder class provides the Python
    interface to create or load meshes.

    Typical use is:
        import salome
        salome.salome_init()
        from salome.smesh import smeshBuilder
        smesh = smeshBuilder.New(salome.myStudy)

    Parameters:
        study:         SALOME study, generally obtained by salome.myStudy.
        instance:      CORBA proxy of SMESH Engine. If None, the default Engine is used.
        instanceGeom:  CORBA proxy of GEOM  Engine. If None, the default Engine is used.
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
    smeshInst.init_smesh(study, instanceGeom)
    return smeshInst


# Public class: Mesh
# ==================

class Mesh:
    """
    This class allows defining and managing a mesh.
    It has a set of methods to build a mesh on the given geometry, including the definition of sub-meshes.
    It also has methods to define groups of mesh elements, to modify a mesh (by addition of
    new nodes and elements and by changing the existing entities), to get information
    about a mesh and to export a mesh in different formats.
    """
    __metaclass__ = MeshMeta

    geom = 0
    mesh = 0
    editor = 0

    def __init__(self, smeshpyD, geompyD, obj=0, name=0):
	"""
	Constructor

	Create a mesh on the shape *obj* (or an empty mesh if *obj* is equal to 0) and
		sets the GUI name of this mesh to *name*.

	Parameters:
		smeshpyD: an instance of smeshBuilder class
		geompyD: an instance of geomBuilder class
		obj: Shape to be meshed or SMESH_Mesh object
		name: Study name of the mesh
	"""

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

    def __del__(self):
	"""
	Destructor. Clean-up resources
	"""
        if self.mesh:
            #self.mesh.UnRegister()
            pass
        pass

    def SetMesh(self, theMesh):
	"""
	Initialize the Mesh object from an instance of SMESH_Mesh interface

	Parameters:
		theMesh: a SMESH_Mesh object
	"""


        # do not call Register() as this prevents mesh servant deletion at closing study
        #if self.mesh: self.mesh.UnRegister()
        self.mesh = theMesh
        if self.mesh:
            #self.mesh.Register()
            self.geom = self.mesh.GetShapeToMesh()
        pass

    def GetMesh(self):
	"""
	Return the mesh, that is an instance of SMESH_Mesh interface

	Returns:
		a SMESH_Mesh object
	"""

        return self.mesh

    def GetName(self):
	"""
	Get the name of the mesh

	Returns:
		the name of the mesh as a string
	"""

        name = GetName(self.GetMesh())
        return name

    def SetName(self, name):
	"""
	Set a name to the mesh

	Parameters:
		name: a new name of the mesh
	"""

        self.smeshpyD.SetName(self.GetMesh(), name)

    def GetSubMesh(self, geom, name):
	"""
	Get a sub-mesh object associated to a *geom* geometrical object.

	Parameters:
		geom: a geometrical object (shape)
		name: a name for the sub-mesh in the Object Browser

	Returns:
		an object of type SMESH.SMESH_subMesh, representing a part of mesh,
			which lies on the given shape

	The sub-mesh object gives access to the IDs of nodes and elements.
	The sub-mesh object has the following methods:

		- SMESH.SMESH_subMesh.GetNumberOfElements()
		- SMESH.SMESH_subMesh.GetNumberOfNodes( all )
		- SMESH.SMESH_subMesh.GetElementsId()
		- SMESH.SMESH_subMesh.GetElementsByType( ElementType )
		- SMESH.SMESH_subMesh.GetNodesId()
		- SMESH.SMESH_subMesh.GetSubShape()
		- SMESH.SMESH_subMesh.GetFather()
		- SMESH.SMESH_subMesh.GetId()

	Note:
		A sub-mesh is implicitly created when a sub-shape is specified at
			creating an algorithm, for example: algo1D = mesh.Segment(geom=Edge_1)
			creates a sub-mesh on *Edge_1* and assign Wire Discretization algorithm to it.
			The created sub-mesh can be retrieved from the algorithm:
			submesh = algo1D.GetSubMesh()
	"""

        AssureGeomPublished( self, geom, name )
        submesh = self.mesh.GetSubMesh( geom, name )
        return submesh

    def GetShape(self):
	"""
	Return the shape associated to the mesh

	Returns:
		a GEOM_Object
	"""

        return self.geom

    def SetShape(self, geom):
	"""
	Associate the given shape to the mesh (entails the recreation of the mesh)

	Parameters:
		geom: the shape to be meshed (GEOM_Object)
	"""

        self.mesh = self.smeshpyD.CreateMesh(geom)

    def Load(self):
	"""
	Load mesh from the study after opening the study
	"""
        self.mesh.Load()

    def IsReadyToCompute(self, theSubObject):
	"""
	Return true if the hypotheses are defined well

	Parameters:
		theSubObject: a sub-shape of a mesh shape

	Returns:
		True or False
	"""

        return self.smeshpyD.IsReadyToCompute(self.mesh, theSubObject)

    def GetAlgoState(self, theSubObject):
	"""
	Return errors of hypotheses definition.
	The list of errors is empty if everything is OK.

	Parameters:
		theSubObject: a sub-shape of a mesh shape

	Returns:
		a list of errors
	"""

        return self.smeshpyD.GetAlgoState(self.mesh, theSubObject)

    def GetGeometryByMeshElement(self, theElementID, theGeomName):
        """
        Return a geometrical object on which the given element was built.
        The returned geometrical object, if not nil, is either found in the
        study or published by this method with the given name

        Parameters:
            theElementID: the id of the mesh element
            theGeomName: the user-defined name of the geometrical object

        Returns:
            GEOM::GEOM_Object instance
        """

        return self.smeshpyD.GetGeometryByMeshElement( self.mesh, theElementID, theGeomName )

    def MeshDimension(self):
	"""
	Return the mesh dimension depending on the dimension of the underlying shape
		or, if the mesh is not based on any shape, basing on deimension of elements

	Returns:
		mesh dimension as an integer value [0,3]
	"""

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

    def Evaluate(self, geom=0):
	"""
	Evaluate size of prospective mesh on a shape

	Returns:
		a list where i-th element is a number of elements of i-th SMESH.EntityType
		To know predicted number of e.g. edges, inquire it this way
		Evaluate()[ EnumToLong( Entity_Edge )]
	"""

        if geom == 0 or not isinstance(geom, geomBuilder.GEOM._objref_GEOM_Object):
            if self.geom == 0:
                geom = self.mesh.GetShapeToMesh()
            else:
                geom = self.geom
        return self.smeshpyD.Evaluate(self.mesh, geom)


    def Compute(self, geom=0, discardModifs=False, refresh=False):
	"""
	Compute the mesh and return the status of the computation

	Parameters:
		geom: geomtrical shape on which mesh data should be computed
		discardModifs: if True and the mesh has been edited since
			a last total re-compute and that may prevent successful partial re-compute,
			then the mesh is cleaned before Compute()
		refresh: if *True*, Object browser is automatically updated (when running in GUI)

	Returns:
		True or False
	"""

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
                if allReasons: allReasons += "\n"
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
                if allReasons: allReasons += "\n"
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

    def GetComputeErrors(self, shape=0 ):
	"""
	Return a list of error messages (SMESH.ComputeError) of the last Compute()
	"""

        if shape == 0:
            shape = self.mesh.GetShapeToMesh()
        return self.smeshpyD.GetComputeErrors( self.mesh, shape )

    def GetSubShapeName(self, subShapeID ):
	"""
	Return a name of a sub-shape by its ID

	Parameters:
		subShapeID: a unique ID of a sub-shape

	Returns:
		a string describing the sub-shape; possible variants:

			- "Face_12"    (published sub-shape)
			- FACE #3      (not published sub-shape)
			- sub-shape #3 (invalid sub-shape ID)
			- #3           (error in this function)
	"""

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

    def GetFailedShapes(self, publish=False):
	"""
	Return a list of sub-shapes meshing of which failed, grouped into GEOM groups by
	error of an algorithm

	Parameters:
		publish: if *True*, the returned groups will be published in the study

	Returns:
		a list of GEOM groups each named after a failed algorithm
	"""


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

    def GetMeshOrder(self):
	"""
	Return sub-mesh objects list in meshing order

	Returns:
		list of lists of sub-meshes
	"""

        return self.mesh.GetMeshOrder()

    def SetMeshOrder(self, submeshes):
	"""
	Set order in which concurrent sub-meshes should be meshed

	Parameters:
		submeshes list of lists of sub-meshes
	"""

        return self.mesh.SetMeshOrder(submeshes)

    def Clear(self, refresh=False):
	"""
	Remove all nodes and elements generated on geometry. Imported elements remain.

	Parameters:
		refresh if *True*, Object browser is automatically updated (when running in GUI)
	"""

        self.mesh.Clear()
        if ( salome.sg.hasDesktop() and
             salome.myStudyManager.GetStudyByID( self.mesh.GetStudyId() ) ):
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), False, True )
            if refresh: salome.sg.updateObjBrowser(True)

    def ClearSubMesh(self, geomId, refresh=False):
	"""
	Remove all nodes and elements of indicated shape

	Parameters:
		refresh: if *True*, Object browser is automatically updated (when running in GUI)
		geomId: the ID of a sub-shape to remove elements on
	"""

        self.mesh.ClearSubMesh(geomId)
        if salome.sg.hasDesktop():
            smeshgui = salome.ImportComponentGUI("SMESH")
            smeshgui.Init(self.mesh.GetStudyId())
            smeshgui.SetMeshIcon( salome.ObjectToID( self.mesh ), False, True )
            if refresh: salome.sg.updateObjBrowser(True)

    def AutomaticTetrahedralization(self, fineness=0):
	"""
	Compute a tetrahedral mesh using AutomaticLength + MEFISTO + Tetrahedron

	Parameters:
		fineness: [0.0,1.0] defines mesh fineness

	Returns:
		True or False
	"""

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

    def AutomaticHexahedralization(self, fineness=0):
	"""
	Compute an hexahedral mesh using AutomaticLength + Quadrangle + Hexahedron

	Parameters:
		fineness [0.0, 1.0] defines mesh fineness

	Returns:
		True or False
	"""

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

    def AddHypothesis(self, hyp, geom=0):
	"""
	Assign a hypothesis

	Parameters:
		hyp: a hypothesis to assign
		geom: a subhape of mesh geometry

	Returns:
		SMESH.Hypothesis_Status
	"""

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

    def IsUsedHypothesis(self, hyp, geom):
	"""
	Return True if an algorithm of hypothesis is assigned to a given shape

	Parameters:
		hyp: a hypothesis to check
		geom: a subhape of mesh geometry

	Returns:
		True of False
	"""

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

    def RemoveHypothesis(self, hyp, geom=0):
	"""
	Unassign a hypothesis

	Parameters:
		hyp: a hypothesis to unassign
		geom: a sub-shape of mesh geometry

	Returns:
		SMESH.Hypothesis_Status
	"""

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

    def GetHypothesisList(self, geom):
	"""
	Get the list of hypotheses added on a geometry

	Parameters:
		geom: a sub-shape of mesh geometry

	Returns:
		the sequence of SMESH_Hypothesis
	"""

        return self.mesh.GetHypothesisList( geom )

    def RemoveGlobalHypotheses(self):
	"""
	Remove all global hypotheses
	"""

        current_hyps = self.mesh.GetHypothesisList( self.geom )
        for hyp in current_hyps:
            self.mesh.RemoveHypothesis( self.geom, hyp )
            pass
        pass

    def ExportMED(self, f, auto_groups=0, version=MED_V2_2,
                  overwrite=1, meshPart=None, autoDimension=True, fields=[], geomAssocFields=''):
	"""
	Export the mesh in a file in MED format
		allowing to overwrite the file if it exists or add the exported data to its contents

	Parameters:
		f: is the file name
		auto_groups: boolean parameter for creating/not creating
			the groups Group_On_All_Nodes, Group_On_All_Faces, ... ;
			the typical use is auto_groups=False.
		version: MED format version (MED_V2_1 or MED_V2_2,
			the latter meaning any current version). The parameter is
			obsolete since MED_V2_1 is no longer supported.
		overwrite: boolean parameter for overwriting/not overwriting the file
		meshPart: a part of mesh (group, sub-mesh) to export instead of the mesh
		autoDimension: if *True* (default), a space dimension of a MED mesh can be either

			- 1D if all mesh nodes lie on OX coordinate axis, or
			- 2D if all mesh nodes lie on XOY coordinate plane, or
			- 3D in the rest cases.

			If *autoDimension* is *False*, the space dimension is always 3.
		fields: list of GEOM fields defined on the shape to mesh.
		geomAssocFields: each character of this string means a need to export a
			corresponding field; correspondence between fields and characters is following:

				- 'v' stands for "_vertices _" field;
				- 'e' stands for "_edges _" field;
				- 'f' stands for "_faces _" field;
				- 's' stands for "_solids _" field.
	"""

        if meshPart or fields or geomAssocFields:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToMED( meshPart, f, auto_groups, version, overwrite, autoDimension,
                                       fields, geomAssocFields)
        else:
            self.mesh.ExportToMEDX(f, auto_groups, version, overwrite, autoDimension)

    def ExportSAUV(self, f, auto_groups=0):
	"""
	Export the mesh in a file in SAUV format


	Parameters:
		f: is the file name
		auto_groups: boolean parameter for creating/not creating
			the groups Group_On_All_Nodes, Group_On_All_Faces, ... ;
			the typical use is auto_groups=false.
	"""

        self.mesh.ExportSAUV(f, auto_groups)

    def ExportDAT(self, f, meshPart=None):
	"""
	Export the mesh in a file in DAT format

	Parameters:
		f: the file name
		meshPart: a part of mesh (group, sub-mesh) to export instead of the mesh
	"""

        if meshPart:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToDAT( meshPart, f )
        else:
            self.mesh.ExportDAT(f)

    def ExportUNV(self, f, meshPart=None):
	"""
	Export the mesh in a file in UNV format

	Parameters:
		f: the file name
		meshPart: a part of mesh (group, sub-mesh) to export instead of the mesh
	"""

        if meshPart:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToUNV( meshPart, f )
        else:
            self.mesh.ExportUNV(f)

    def ExportSTL(self, f, ascii=1, meshPart=None):
	"""
	Export the mesh in a file in STL format

	Parameters:
		f: the file name
		ascii: defines the file encoding
		meshPart: a part of mesh (group, sub-mesh) to export instead of the mesh
	"""

        if meshPart:
            unRegister = genObjUnRegister()
            if isinstance( meshPart, list ):
                meshPart = self.GetIDSource( meshPart, SMESH.ALL )
                unRegister.set( meshPart )
            self.mesh.ExportPartToSTL( meshPart, f, ascii )
        else:
            self.mesh.ExportSTL(f, ascii)

    def ExportCGNS(self, f, overwrite=1, meshPart=None, groupElemsByType=False):
	"""
	Export the mesh in a file in CGNS format

	Parameters:
		f: is the file name
		overwrite: boolean parameter for overwriting/not overwriting the file
		meshPart: a part of mesh (group, sub-mesh) to export instead of the mesh
		groupElemsByType: if true all elements of same entity type are exported at ones,
			else elements are exported in order of their IDs which can cause creation
			of multiple cgns sections
	"""

        unRegister = genObjUnRegister()
        if isinstance( meshPart, list ):
            meshPart = self.GetIDSource( meshPart, SMESH.ALL )
            unRegister.set( meshPart )
        if isinstance( meshPart, Mesh ):
            meshPart = meshPart.mesh
        elif not meshPart:
            meshPart = self.mesh
        self.mesh.ExportCGNS(meshPart, f, overwrite, groupElemsByType)

    def ExportGMF(self, f, meshPart=None):
	"""
	Export the mesh in a file in GMF format.
	GMF files must have .mesh extension for the ASCII format and .meshb for
	the bynary format. Other extensions are not allowed.

	Parameters:
		f: is the file name
		meshPart: a part of mesh (group, sub-mesh) to export instead of the mesh
	"""

        unRegister = genObjUnRegister()
        if isinstance( meshPart, list ):
            meshPart = self.GetIDSource( meshPart, SMESH.ALL )
            unRegister.set( meshPart )
        if isinstance( meshPart, Mesh ):
            meshPart = meshPart.mesh
        elif not meshPart:
            meshPart = self.mesh
        self.mesh.ExportGMF(meshPart, f, True)

    def ExportToMED(self, f, version=MED_V2_2, opt=0, overwrite=1, autoDimension=True):
	"""
	Deprecated, used only for compatibility! Please, use ExportMED() method instead.
	Export the mesh in a file in MED format
	allowing to overwrite the file if it exists or add the exported data to its contents

	Parameters:
		f: the file name
		version: MED format version (MED_V2_1 or MED_V2_2,
			the latter meaning any current version). The parameter is
			obsolete since MED_V2_1 is no longer supported.
		opt: boolean parameter for creating/not creating
			the groups Group_On_All_Nodes, Group_On_All_Faces, ...
		overwrite: boolean parameter for overwriting/not overwriting the file
		autoDimension: if *True* (default), a space dimension of a MED mesh can be either

			- 1D if all mesh nodes lie on OX coordinate axis, or
			- 2D if all mesh nodes lie on XOY coordinate plane, or
			- 3D in the rest cases.

			If **autoDimension** isc **False**, the space dimension is always 3.
	"""

        self.mesh.ExportToMEDX(f, opt, version, overwrite, autoDimension)

    # Operations with groups:
    # ----------------------

    def CreateEmptyGroup(self, elementType, name):
	"""
	Create an empty mesh group

	Parameters:
		elementType: the type of elements in the group; either of
			(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
		name: the name of the mesh group

	Returns:
		SMESH_Group
	"""

        return self.mesh.CreateGroup(elementType, name)

    def Group(self, grp, name=""):
	"""
	Create a mesh group based on the geometric object *grp*
	and gives a *name*,
	if this parameter is not defined
	the name is the same as the geometric group name

	Note:
		Works like GroupOnGeom().

	Parameters:
		grp:  a geometric group, a vertex, an edge, a face or a solid
		name: the name of the mesh group

	Returns:
		SMESH_GroupOnGeom
	"""

        return self.GroupOnGeom(grp, name)

    def GroupOnGeom(self, grp, name="", typ=None):
	"""
	Create a mesh group based on the geometrical object *grp*
	and gives a *name*,
	if this parameter is not defined the name is the same as the geometrical group name

	Parameters:
		grp:  a geometrical group, a vertex, an edge, a face or a solid
		name: the name of the mesh group
		typ:  the type of elements in the group; either of
			(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME). If not set, it is
			automatically detected by the type of the geometry

	Returns:
		SMESH_GroupOnGeom
	"""

        AssureGeomPublished( self, grp, name )
        if name == "":
            name = grp.GetName()
        if not typ:
            typ = self._groupTypeFromShape( grp )
        return self.mesh.CreateGroupFromGEOM(typ, name, grp)

    def _groupTypeFromShape( self, shape ):
	"""
	Pivate method to get a type of group on geometry
	"""
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

    def GroupOnFilter(self, typ, name, filter):
	"""
	Create a mesh group with given *name* based on the *filter* which
	is a special type of group dynamically updating it's contents during
	mesh modification

	Parameters:
		typ: the type of elements in the group; either of
			(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME).
		name: the name of the mesh group
		filter: the filter defining group contents

	Returns:
		SMESH_GroupOnFilter
	"""

        return self.mesh.CreateGroupFromFilter(typ, name, filter)

    def MakeGroupByIds(self, groupName, elementType, elemIDs):
	"""
	Create a mesh group by the given ids of elements

	Parameters:
		groupName: the name of the mesh group
		elementType: the type of elements in the group; either of
			(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME).
		elemIDs: either the list of ids, group, sub-mesh, or filter

	Returns:
		SMESH_Group
	"""

        group = self.mesh.CreateGroup(elementType, groupName)
        if isinstance( elemIDs, Mesh ):
            elemIDs = elemIDs.GetMesh()
        if hasattr( elemIDs, "GetIDs" ):
            if hasattr( elemIDs, "SetMesh" ):
                elemIDs.SetMesh( self.GetMesh() )
            group.AddFrom( elemIDs )
        else:
            group.Add(elemIDs)
        return group

    def MakeGroup(self,
                  groupName,
                  elementType,
                  CritType=FT_Undefined,
                  Compare=FT_EqualTo,
                  Threshold="",
                  UnaryOp=FT_Undefined,
                  Tolerance=1e-07):
	"""
	Create a mesh group by the given conditions

	Parameters:
		groupName: the name of the mesh group
		elementType: the type of elements(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME)
		CritType: the type of criterion (SMESH.FT_Taper, SMESH.FT_Area, etc.)
			Type SMESH.FunctorType._items in the Python Console to see all values.
			Note that the items starting from FT_LessThan are not suitable for CritType.
		Compare: belongs to {SMESH.FT_LessThan, SMESH.FT_MoreThan, SMESH.FT_EqualTo}
		Threshold: the threshold value (range of ids as string, shape, numeric)
		UnaryOp:  SMESH.FT_LogicalNOT or SMESH.FT_Undefined
		Tolerance: the tolerance used by SMESH.FT_BelongToGeom, SMESH.FT_BelongToSurface,
			SMESH.FT_LyingOnGeom, SMESH.FT_CoplanarFaces criteria

	Returns:
		SMESH_GroupOnFilter
	"""

        aCriterion = self.smeshpyD.GetCriterion(elementType, CritType, Compare, Threshold, UnaryOp, FT_Undefined,Tolerance)
        group = self.MakeGroupByCriterion(groupName, aCriterion)
        return group

    def MakeGroupByCriterion(self, groupName, Criterion):
	"""
	Create a mesh group by the given criterion

	Parameters:
		groupName: the name of the mesh group
		Criterion: the instance of Criterion class

	Returns:
		SMESH_GroupOnFilter
	"""

        return self.MakeGroupByCriteria( groupName, [Criterion] )

    def MakeGroupByCriteria(self, groupName, theCriteria, binOp=SMESH.FT_LogicalAND):
	"""
	Create a mesh group by the given criteria (list of criteria)

	Parameters:
		groupName: the name of the mesh group
		theCriteria: the list of criteria
		binOp: binary operator used when binary operator of criteria is undefined

	Returns:
		SMESH_GroupOnFilter
	"""

        aFilter = self.smeshpyD.GetFilterFromCriteria( theCriteria, binOp )
        group = self.MakeGroupByFilter(groupName, aFilter)
        return group

    def MakeGroupByFilter(self, groupName, theFilter):
	"""
	Create a mesh group by the given filter

	Parameters:
		groupName: the name of the mesh group
		theFilter: the instance of Filter class

	Returns:
		SMESH_GroupOnFilter
	"""

        #group = self.CreateEmptyGroup(theFilter.GetElementType(), groupName)
        #theFilter.SetMesh( self.mesh )
        #group.AddFrom( theFilter )
        group = self.GroupOnFilter( theFilter.GetElementType(), groupName, theFilter )
        return group

    def RemoveGroup(self, group):
	"""
	Remove a group
	"""

        self.mesh.RemoveGroup(group)

    def RemoveGroupWithContents(self, group):
	"""
	Remove a group with its contents
	"""

        self.mesh.RemoveGroupWithContents(group)

    def GetGroups(self, elemType = SMESH.ALL):
	"""
	Get the list of groups existing in the mesh in the order
	of creation (starting from the oldest one)

	Parameters:
		elemType: type of elements the groups contain; either of
			(SMESH.ALL, SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME);
			by default groups of elements of all types are returned

	Returns:
		a sequence of SMESH_GroupBase
	"""

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

    def NbGroups(self):
	"""
	Get the number of groups existing in the mesh

	Returns:
		the quantity of groups as an integer value
	"""

        return self.mesh.NbGroups()

    def GetGroupNames(self):
	"""
	Get the list of names of groups existing in the mesh

	Returns:
		list of strings
	"""

        groups = self.GetGroups()
        names = []
        for group in groups:
            names.append(group.GetName())
        return names

    def GetGroupByName(self, name, elemType = None):
	"""
	Find groups by name and type

	Parameters:
		name: name of the group of interest
		elemType: type of elements the groups contain; either of
			(SMESH.ALL, SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME);
			by default one group of any type of elements is returned
			if elemType == SMESH.ALL then all groups of any type are returned

	Returns:
		a list of SMESH_GroupBase's
	"""

        groups = []
        for group in self.GetGroups():
            if group.GetName() == name:
                if elemType is None:
                    return [group]
                if ( elemType == SMESH.ALL or
                     group.GetType() == elemType ):
                    groups.append( group )
        return groups

    def UnionGroups(self, group1, group2, name):
	"""
	Produce a union of two groups.
	A new group is created. All mesh elements that are
	present in the initial groups are added to the new one

	Parameters:
		an instance of SMESH_Group
	"""

        return self.mesh.UnionGroups(group1, group2, name)

    def UnionListOfGroups(self, groups, name):
	"""
	Produce a union list of groups.
	New group is created. All mesh elements that are present in
	initial groups are added to the new one


	Returns:
		an instance of SMESH_Group
	"""

        return self.mesh.UnionListOfGroups(groups, name)

    def IntersectGroups(self, group1, group2, name):
	"""
	Prodice an intersection of two groups.
	A new group is created. All mesh elements that are common
	for the two initial groups are added to the new one.

	Returns:
		an instance of SMESH_Group
	"""

        return self.mesh.IntersectGroups(group1, group2, name)

    def IntersectListOfGroups(self, groups, name):
	"""
	Produce an intersection of groups.
	New group is created. All mesh elements that are present in all
	initial groups simultaneously are added to the new one

	Returns:
		an instance of SMESH_Group
	"""

        return self.mesh.IntersectListOfGroups(groups, name)

    def CutGroups(self, main_group, tool_group, name):
	"""
	Produce a cut of two groups.
	A new group is created. All mesh elements that are present in
	the main group but are not present in the tool group are added to the new one

	Returns:
		an instance of SMESH_Group
	"""

        return self.mesh.CutGroups(main_group, tool_group, name)

    def CutListOfGroups(self, main_groups, tool_groups, name):
	"""
	Produce a cut of groups.
	A new group is created. All mesh elements that are present in main groups
	but do not present in tool groups are added to the new one

	Returns:
		an instance of SMESH_Group
	"""

        return self.mesh.CutListOfGroups(main_groups, tool_groups, name)

    def CreateDimGroup(self, groups, elemType, name,
                       nbCommonNodes = SMESH.ALL_NODES, underlyingOnly = True):
	"""
	Create a standalone group of entities basing on nodes of other groups.

	Parameters:
		groups: list of reference groups, sub-meshes or filters, of any type.
		elemType: a type of elements to include to the new group; either of
			(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME).
		name: a name of the new group.
		nbCommonNodes: a criterion of inclusion of an element to the new group
			basing on number of element nodes common with reference *groups*.
			Meaning of possible values are:

				- SMESH.ALL_NODES - include if all nodes are common,
				- SMESH.MAIN - include if all corner nodes are common (meaningful for a quadratic mesh),
				- SMESH.AT_LEAST_ONE - include if one or more node is common,
				- SMEHS.MAJORITY - include if half of nodes or more are common.
		underlyingOnly: if *True* (default), an element is included to the
			new group provided that it is based on nodes of an element of *groups*;
			in this case the reference *groups* are supposed to be of higher dimension
			than *elemType*, which can be useful for example to get all faces lying on
			volumes of the reference *groups*.

	Returns:
		an instance of SMESH_Group
	"""

        if isinstance( groups, SMESH._objref_SMESH_IDSource ):
            groups = [groups]
        return self.mesh.CreateDimGroup(groups, elemType, name, nbCommonNodes, underlyingOnly)


    def ConvertToStandalone(self, group):
	"""
	Convert group on geom into standalone group
	"""

        return self.mesh.ConvertToStandalone(group)

    # Get some info about mesh:
    # ------------------------

    def GetLog(self, clearAfterGet):
	"""
	Return the log of nodes and elements added or removed
	since the previous clear of the log.

	Parameters:
		clearAfterGet: log is emptied after Get (safe if concurrents access)

	Returns:
		list of log_block structures:
			commandType
			number
			coords
			indexes
	"""

        return self.mesh.GetLog(clearAfterGet)

    def ClearLog(self):
	"""
	Clear the log of nodes and elements added or removed since the previous
	clear. Must be used immediately after GetLog if clearAfterGet is false.
	"""

        self.mesh.ClearLog()

    def SetAutoColor(self, theAutoColor):
	"""
	Toggle auto color mode on the object.

	Parameters:
		theAutoColor: the flag which toggles auto color mode.
	If switched on, a default color of a new group in Create Group dialog is chosen randomly.
	"""

        self.mesh.SetAutoColor(theAutoColor)

    def GetAutoColor(self):
	"""
	Get flag of object auto color mode.

	Returns:
		True or False
	"""

        return self.mesh.GetAutoColor()

    def GetId(self):
	"""
	Get the internal ID

        Returns:
            integer value, which is the internal Id of the mesh
	"""

        return self.mesh.GetId()

    def GetStudyId(self):
	"""
	Get the study Id

        Returns:
            integer value, which is the study Id of the mesh
	"""

        return self.mesh.GetStudyId()

    def HasDuplicatedGroupNamesMED(self):
	"""
	Check the group names for duplications.
	Consider the maximum group name length stored in MED file.

        Returns:
            True or False
	"""

        return self.mesh.HasDuplicatedGroupNamesMED()

    def GetMeshEditor(self):
	"""
	Obtain the mesh editor tool

        Returns:
            an instance of SMESH_MeshEditor
	"""

        return self.editor

    def GetIDSource(self, ids, elemType = SMESH.ALL):
	"""
	Wrap a list of IDs of elements or nodes into SMESH_IDSource which
	can be passed as argument to a method accepting mesh, group or sub-mesh

	Parameters:
		ids: list of IDs
		lemType: type of elements; this parameter is used to distinguish
			IDs of nodes from IDs of elements; by default ids are treated as
			IDs of elements; use SMESH.NODE if ids are IDs of nodes.

        Returns:
            an instance of SMESH_IDSource

	Warning:
		call UnRegister() for the returned object as soon as it is no more useful:
			idSrc = mesh.GetIDSource( [1,3,5], SMESH.NODE )
			mesh.DoSomething( idSrc )
			idSrc.UnRegister()
	"""

        if isinstance( ids, int ):
            ids = [ids]
        return self.editor.MakeIDSource(ids, elemType)


    # Get information about mesh contents:
    # ------------------------------------

    def GetMeshInfo(self, obj = None):
	"""
	Get the mesh statistic

        Returns:
            dictionary type element - count of elements
	"""

        if not obj: obj = self.mesh
        return self.smeshpyD.GetMeshInfo(obj)

    def NbNodes(self):
	"""
	Return the number of nodes in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbNodes()

    def NbElements(self):
	"""
	Return the number of elements in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbElements()

    def Nb0DElements(self):
	"""
	Return the number of 0d elements in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.Nb0DElements()

    def NbBalls(self):
        """
        Return the number of ball discrete elements in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbBalls()

    def NbEdges(self):
        """
        Return the number of edges in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbEdges()

    def NbEdgesOfOrder(self, elementOrder):
        """
        Return the number of edges with the given order in the mesh

	Parameters:
		elementOrder: the order of elements:
		SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbEdgesOfOrder(elementOrder)

    def NbFaces(self):
        """
        Return the number of faces in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbFaces()

    def NbFacesOfOrder(self, elementOrder):
        """
        Return the number of faces with the given order in the mesh

	Parameters:
		elementOrder: the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbFacesOfOrder(elementOrder)

    def NbTriangles(self):
        """
        Return the number of triangles in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbTriangles()

    def NbTrianglesOfOrder(self, elementOrder):
        """
        Return the number of triangles with the given order in the mesh

	Parameters:
		elementOrder: is the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbTrianglesOfOrder(elementOrder)

    def NbBiQuadTriangles(self):
        """
        Return the number of biquadratic triangles in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbBiQuadTriangles()

    def NbQuadrangles(self):
        """
        Return the number of quadrangles in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbQuadrangles()

    def NbQuadranglesOfOrder(self, elementOrder):
        """
        Return the number of quadrangles with the given order in the mesh

	Parameters:
		elementOrder the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbQuadranglesOfOrder(elementOrder)

    def NbBiQuadQuadrangles(self):
        """
        Return the number of biquadratic quadrangles in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbBiQuadQuadrangles()

    def NbPolygons(self, elementOrder = SMESH.ORDER_ANY):
        """
        Return the number of polygons of given order in the mesh

	Parameters:
		elementOrder: the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbPolygonsOfOrder(elementOrder)

    def NbVolumes(self):
        """
        Return the number of volumes in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbVolumes()


    def NbVolumesOfOrder(self, elementOrder):
        """
        Return the number of volumes with the given order in the mesh

	Parameters:
		elementOrder:  the order of elements:
		SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
        """

        return self.mesh.NbVolumesOfOrder(elementOrder)

    def NbTetras(self):
        """
        Return the number of tetrahedrons in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbTetras()

    def NbTetrasOfOrder(self, elementOrder):
        """
        Return the number of tetrahedrons with the given order in the mesh

	Parameters:
		elementOrder:  the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbTetrasOfOrder(elementOrder)

    def NbHexas(self):
        """
        Return the number of hexahedrons in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbHexas()

    def NbHexasOfOrder(self, elementOrder):
        """
        Return the number of hexahedrons with the given order in the mesh

	Parameters:
		elementOrder:  the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbHexasOfOrder(elementOrder)

    def NbTriQuadraticHexas(self):
        """
        Return the number of triquadratic hexahedrons in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbTriQuadraticHexas()

    def NbPyramids(self):
        """
        Return the number of pyramids in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbPyramids()

    def NbPyramidsOfOrder(self, elementOrder):
        """
        Return the number of pyramids with the given order in the mesh

	Parameters:
		elementOrder:  the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbPyramidsOfOrder(elementOrder)

    def NbPrisms(self):
        """
        Return the number of prisms in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbPrisms()

    def NbPrismsOfOrder(self, elementOrder):
        """
        Return the number of prisms with the given order in the mesh

	Parameters:
		elementOrder:  the order of elements:
			SMESH.ORDER_ANY, SMESH.ORDER_LINEAR or SMESH.ORDER_QUADRATIC

        Returns:
            an integer value
	"""

        return self.mesh.NbPrismsOfOrder(elementOrder)

    def NbHexagonalPrisms(self):
        """
        Return the number of hexagonal prisms in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbHexagonalPrisms()

    def NbPolyhedrons(self):
        """
        Return the number of polyhedrons in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbPolyhedrons()

    def NbSubMesh(self):
        """
        Return the number of submeshes in the mesh

        Returns:
            an integer value
	"""

        return self.mesh.NbSubMesh()

    def GetElementsId(self):
        """
        Return the list of mesh elements IDs

        Returns:
            the list of integer values
	"""

        return self.mesh.GetElementsId()

    def GetElementsByType(self, elementType):
        """
        Return the list of IDs of mesh elements with the given type

	Parameters:
		elementType:  the required type of elements, either of
			(SMESH.NODE, SMESH.EDGE, SMESH.FACE or SMESH.VOLUME)

        Returns:
            list of integer values
	"""

        return self.mesh.GetElementsByType(elementType)

    def GetNodesId(self):
        """
        Return the list of mesh nodes IDs

        Returns:
            the list of integer values
	"""

        return self.mesh.GetNodesId()

    # Get the information about mesh elements:
    # ------------------------------------

    def GetElementType(self, id, iselem=True):
        """
        Return the type of mesh element

        Returns:
            the value from SMESH::ElementType enumeration
		Type SMESH.ElementType._items in the Python Console to see all possible values.
	"""

        return self.mesh.GetElementType(id, iselem)

    def GetElementGeomType(self, id):
        """
        Return the geometric type of mesh element

        Returns:
            the value from SMESH::EntityType enumeration
		Type SMESH.EntityType._items in the Python Console to see all possible values.
	"""

        return self.mesh.GetElementGeomType(id)

    def GetElementShape(self, id):
        """
        Return the shape type of mesh element

        Returns:
            the value from SMESH::GeometryType enumeration.
		Type SMESH.GeometryType._items in the Python Console to see all possible values.
	"""

        return self.mesh.GetElementShape(id)

    def GetSubMeshElementsId(self, Shape):
        """
        Return the list of submesh elements IDs

	Parameters:
		Shape: a geom object(sub-shape)
			Shape must be the sub-shape of a ShapeToMesh()

        Returns:
            the list of integer values
	"""

        if isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshElementsId(ShapeID)

    def GetSubMeshNodesId(self, Shape, all):
        """
        Return the list of submesh nodes IDs

	Parameters:
		Shape: a geom object(sub-shape)
			Shape must be the sub-shape of a ShapeToMesh()
		all: If true, gives all nodes of submesh elements, otherwise gives only submesh nodes

        Returns:
            the list of integer values
	"""

        if isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshNodesId(ShapeID, all)

    def GetSubMeshElementType(self, Shape):
        """
        Return type of elements on given shape

	Parameters:
		Shape: a geom object(sub-shape)
			Shape must be a sub-shape of a ShapeToMesh()

        Returns:
            element type
	"""

        if isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
        else:
            ShapeID = Shape
        return self.mesh.GetSubMeshElementType(ShapeID)

    def Dump(self):
        """
        Get the mesh description

        Returns:
            string value
	"""

        return self.mesh.Dump()


    # Get the information about nodes and elements of a mesh by its IDs:
    # -----------------------------------------------------------

    def GetNodeXYZ(self, id):
        """
        Get XYZ coordinates of a node
	If there is no nodes for the given ID - return an empty list

        Returns:
            a list of double precision values
	"""

        return self.mesh.GetNodeXYZ(id)

    def GetNodeInverseElements(self, id):
        """
        Return list of IDs of inverse elements for the given node
	If there is no node for the given ID - return an empty list

        Returns:
            a list of integer values
	"""

        return self.mesh.GetNodeInverseElements(id)

    def GetNodePosition(self,NodeID):
        """
        Return the position of a node on the shape

        Returns:
            SMESH::NodePosition
	"""

        return self.mesh.GetNodePosition(NodeID)

    def GetElementPosition(self,ElemID):
        """
        Return the position of an element on the shape

        Returns:
            SMESH::ElementPosition
	"""

        return self.mesh.GetElementPosition(ElemID)

    def GetShapeID(self, id):
        """
        Return the ID of the shape, on which the given node was generated.

        Returns:
            an integer value > 0 or -1 if there is no node for the given
		ID or the node is not assigned to any geometry
	"""

        return self.mesh.GetShapeID(id)

    def GetShapeIDForElem(self,id):
        """
        Return the ID of the shape, on which the given element was generated.

        Returns:
            an integer value > 0 or -1 if there is no element for the given
		ID or the element is not assigned to any geometry
	"""

        return self.mesh.GetShapeIDForElem(id)

    def GetElemNbNodes(self, id):
        """
        Return the number of nodes of the given element

        Returns:
            an integer value > 0 or -1 if there is no element for the given ID
	"""

        return self.mesh.GetElemNbNodes(id)

    def GetElemNode(self, id, index):
        """
        Return the node ID the given (zero based) index for the given element
	If there is no element for the given ID - return -1
	If there is no node for the given index - return -2

        Returns:
            an integer value
	"""

        return self.mesh.GetElemNode(id, index)

    def GetElemNodes(self, id):
        """
        Return the IDs of nodes of the given element

        Returns:
            a list of integer values
	"""

        return self.mesh.GetElemNodes(id)

    def IsMediumNode(self, elementID, nodeID):
        """
        Return true if the given node is the medium node in the given quadratic element
	"""

        return self.mesh.IsMediumNode(elementID, nodeID)

    def IsMediumNodeOfAnyElem(self, nodeID, elementType = SMESH.ALL ):
        """
        Return true if the given node is the medium node in one of quadratic elements

	Parameters:
		nodeID: ID of the node
		elementType:  the type of elements to check a state of the node, either of
			(SMESH.ALL, SMESH.NODE, SMESH.EDGE, SMESH.FACE or SMESH.VOLUME)
	"""

        return self.mesh.IsMediumNodeOfAnyElem(nodeID, elementType)

    def ElemNbEdges(self, id):
        """
        Return the number of edges for the given element
	"""

        return self.mesh.ElemNbEdges(id)

    def ElemNbFaces(self, id):
        """
        Return the number of faces for the given element
	"""

        return self.mesh.ElemNbFaces(id)

    def GetElemFaceNodes(self,elemId, faceIndex):
        """
        Return nodes of given face (counted from zero) for given volumic element.
	"""

        return self.mesh.GetElemFaceNodes(elemId, faceIndex)

    def GetFaceNormal(self, faceId, normalized=False):
        """
        Return three components of normal of given mesh face
		(or an empty array in KO case)
	"""

        return self.mesh.GetFaceNormal(faceId,normalized)

    def FindElementByNodes(self, nodes):
        """
        Return an element based on all given nodes.
	"""

        return self.mesh.FindElementByNodes(nodes)

    def GetElementsByNodes(self, nodes, elemType=SMESH.ALL):
        """
        Return elements including all given nodes.
	"""

        return self.mesh.GetElementsByNodes( nodes, elemType )

    def IsPoly(self, id):
        """
        Return true if the given element is a polygon
	"""

        return self.mesh.IsPoly(id)

    def IsQuadratic(self, id):
        """
        Return true if the given element is quadratic
	"""

        return self.mesh.IsQuadratic(id)

    def GetBallDiameter(self, id):
        """
        Return diameter of a ball discrete element or zero in case of an invalid *id*
	"""

        return self.mesh.GetBallDiameter(id)

    def BaryCenter(self, id):
        """
        Return XYZ coordinates of the barycenter of the given element
	If there is no element for the given ID - return an empty list

        Returns:
            a list of three double values
	"""

        return self.mesh.BaryCenter(id)

    def GetIdsFromFilter(self, theFilter):
        """
        Pass mesh elements through the given filter and return IDs of fitting elements

	Parameters:
		theFilter: SMESH_Filter

        Returns:
            a list of ids
	"""

        theFilter.SetMesh( self.mesh )
        return theFilter.GetIDs()

    # Get mesh measurements information:
    # ------------------------------------

    def GetFreeBorders(self):
        """
        Verify whether a 2D mesh element has free edges (edges connected to one face only)\n
	Return a list of special structures (borders).

        Returns:
            a list of SMESH.FreeEdges. Border structure:: edge id and ids of two its nodes.
	"""

        aFilterMgr = self.smeshpyD.CreateFilterManager()
        aPredicate = aFilterMgr.CreateFreeEdges()
        aPredicate.SetMesh(self.mesh)
        aBorders = aPredicate.GetBorders()
        aFilterMgr.UnRegister()
        return aBorders

    def MinDistance(self, id1, id2=0, isElem1=False, isElem2=False):
        """
        Get minimum distance between two nodes, elements or distance to the origin

	Parameters:
		id1: first node/element id
		id2: second node/element id (if 0, distance from *id1* to the origin is computed)
		isElem1: *True* if *id1* is element id, *False* if it is node id
		isElem2: *True* if *id2* is element id, *False* if it is node id

        Returns:
            minimum distance value **GetMinDistance()**
	"""

        aMeasure = self.GetMinDistance(id1, id2, isElem1, isElem2)
        return aMeasure.value

    def GetMinDistance(self, id1, id2=0, isElem1=False, isElem2=False):
        """
        Get measure structure specifying minimum distance data between two objects

	Parameters:
		id1: first node/element id
		id2: second node/element id (if 0, distance from *id1* to the origin is computed)
		isElem1: *True* if *id1* is element id, *False* if it is node id
		isElem2: *True* if *id2* is element id, *False* if it is node id

        Returns:
            Measure structure **MinDistance()**
	"""

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

    def BoundingBox(self, objects=None, isElem=False):
        """
        Get bounding box of the specified object(s)

	Parameters:
		objects: single source object or list of source objects or list of nodes/elements IDs
		isElem: if *objects* is a list of IDs, *True* value in this parameters specifies that *objects* are elements,
			*False* specifies that *objects* are nodes

        Returns:
            tuple of six values (minX, minY, minZ, maxX, maxY, maxZ) **GetBoundingBox()**
	"""

        result = self.GetBoundingBox(objects, isElem)
        if result is None:
            result = (0.0,)*6
        else:
            result = (result.minX, result.minY, result.minZ, result.maxX, result.maxY, result.maxZ)
        return result

    def GetBoundingBox(self, IDs=None, isElem=False):
        """
        Get measure structure specifying bounding box data of the specified object(s)

	Parameters:
		IDs: single source object or list of source objects or list of nodes/elements IDs
		isElem: if *IDs* is a list of IDs, *True* value in this parameters specifies that *objects* are elements,
			*False* specifies that *objects* are nodes

        Returns:
            Measure structure **BoundingBox()**
	"""

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

    def RemoveElements(self, IDsOfElements):
        """
        Remove the elements from the mesh by ids

	Parameters:
		IDsOfElements: is a list of ids of elements to remove

        Returns:
            True or False
	"""

        return self.editor.RemoveElements(IDsOfElements)

    def RemoveNodes(self, IDsOfNodes):
        """
        Remove nodes from mesh by ids

	Parameters:
		IDsOfNodes: is a list of ids of nodes to remove

        Returns:
            True or False
	"""

        return self.editor.RemoveNodes(IDsOfNodes)

    def RemoveOrphanNodes(self):
        """
        Remove all orphan (free) nodes from mesh

        Returns:
            number of the removed nodes
	"""

        return self.editor.RemoveOrphanNodes()

    def AddNode(self, x, y, z):
        """
        Add a node to the mesh by coordinates

        Returns:
            Id of the new node
	"""

        x,y,z,Parameters,hasVars = ParseParameters(x,y,z)
        if hasVars: self.mesh.SetParameters(Parameters)
        return self.editor.AddNode( x, y, z)

    def Add0DElement( self, IDOfNode, DuplicateElements=True ):
        """
        Create a 0D element on a node with given number.

	Parameters:
		IDOfNode: the ID of node for creation of the element.
		DuplicateElements: to add one more 0D element to a node or not

        Returns:
            the Id of the new 0D element
	"""

        return self.editor.Add0DElement( IDOfNode, DuplicateElements )

    def Add0DElementsToAllNodes(self, theObject, theGroupName="", DuplicateElements=False):
        """
        Create 0D elements on all nodes of the given elements except those
	nodes on which a 0D element already exists.

	Parameters:
		theObject: an object on whose nodes 0D elements will be created.
			It can be mesh, sub-mesh, group, list of element IDs or a holder
			of nodes IDs created by calling mesh.GetIDSource( nodes, SMESH.NODE )
		theGroupName: optional name of a group to add 0D elements created
			and/or found on nodes of *theObject*.
		DuplicateElements: to add one more 0D element to a node or not

        Returns:
            an object (a new group or a temporary SMESH_IDSource) holding
		IDs of new and/or found 0D elements. IDs of 0D elements
		can be retrieved from the returned object by calling GetIDs()
	"""

        unRegister = genObjUnRegister()
        if isinstance( theObject, Mesh ):
            theObject = theObject.GetMesh()
        elif isinstance( theObject, list ):
            theObject = self.GetIDSource( theObject, SMESH.ALL )
            unRegister.set( theObject )
        return self.editor.Create0DElementsOnAllNodes( theObject, theGroupName, DuplicateElements )

    def AddBall(self, IDOfNode, diameter):
        """
        Create a ball element on a node with given ID.

	Parameters:
		IDOfNode: the ID of node for creation of the element.
		diameter: the bal diameter.

        Returns:
            the Id of the new ball element
	"""

        return self.editor.AddBall( IDOfNode, diameter )

    def AddEdge(self, IDsOfNodes):
        """
        Create a linear or quadratic edge (this is determined
		by the number of given nodes).

	Parameters:
		IDsOfNodes: the list of node IDs for creation of the element.
			The order of nodes in this list should correspond to the description
			of MED.
			This description is located by the following link:
			http://www.code-aster.org/outils/med/html/modele_de_donnees.html#3.

        Returns:
            the Id of the new edge
	"""

        return self.editor.AddEdge(IDsOfNodes)

    def AddFace(self, IDsOfNodes):
        """
        Create a linear or quadratic face (this is determined
	by the number of given nodes).

	Parameters:
		IDsOfNodes: the list of node IDs for creation of the element.
			The order of nodes in this list should correspond to the description
			of MED.
			This description is located by the following link:
			http://www.code-aster.org/outils/med/html/modele_de_donnees.html#3.

        Returns:
            the Id of the new face
	"""

        return self.editor.AddFace(IDsOfNodes)

    def AddPolygonalFace(self, IdsOfNodes):
        """
        Add a polygonal face to the mesh by the list of node IDs

	Parameters:
		IdsOfNodes: the list of node IDs for creation of the element.

        Returns:
            the Id of the new face
	"""

        return self.editor.AddPolygonalFace(IdsOfNodes)

    def AddQuadPolygonalFace(self, IdsOfNodes):
        """
        Add a quadratic polygonal face to the mesh by the list of node IDs

	Parameters:
		IdsOfNodes: the list of node IDs for creation of the element;
			corner nodes follow first.

        Returns:
            the Id of the new face
	"""

        return self.editor.AddQuadPolygonalFace(IdsOfNodes)

    def AddVolume(self, IDsOfNodes):
        """
        Create both simple and quadratic volume (this is determined
		by the number of given nodes).

	Parameters:
		IDsOfNodes: the list of node IDs for creation of the element.
			The order of nodes in this list should correspond to the description
			of MED.
			This description is located by the following link:
			http://www.code-aster.org/outils/med/html/modele_de_donnees.html#3.

        Returns:
            the Id of the new volumic element
	"""

        return self.editor.AddVolume(IDsOfNodes)

    def AddPolyhedralVolume (self, IdsOfNodes, Quantities):
        """
        Create a volume of many faces, giving nodes for each face.

	Parameters:
		IdsOfNodes: the list of node IDs for volume creation face by face.
		Quantities: the list of integer values, Quantities[i]
			gives the quantity of nodes in face number i.

        Returns:
            the Id of the new volumic element
	"""

        return self.editor.AddPolyhedralVolume(IdsOfNodes, Quantities)

    def AddPolyhedralVolumeByFaces (self, IdsOfFaces):
        """
        Create a volume of many faces, giving the IDs of the existing faces.

	Parameters:
		IdsOfFaces: the list of face IDs for volume creation.

	Note:
		The created volume will refer only to the nodes
			of the given faces, not to the faces themselves.

        Returns:
            the Id of the new volumic element
	"""

        return self.editor.AddPolyhedralVolumeByFaces(IdsOfFaces)


    def SetNodeOnVertex(self, NodeID, Vertex):
        """
        **Binds** a node to a vertex

	Parameters:
		NodeID: a node ID
		Vertex: a vertex or vertex ID

        Returns:
            True if succeed else raises an exception
	"""

        if ( isinstance( Vertex, geomBuilder.GEOM._objref_GEOM_Object)):
            VertexID = self.geompyD.GetSubShapeID( self.geom, Vertex )
        else:
            VertexID = Vertex
        try:
            self.editor.SetNodeOnVertex(NodeID, VertexID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True


    def SetNodeOnEdge(self, NodeID, Edge, paramOnEdge):
        """
        **Stores** the node position on an edge

	Parameters:
		NodeID: a node ID
		Edge: an edge or edge ID
		paramOnEdge: a parameter on the edge where the node is located

        Returns:
            True if succeed else raises an exception
	"""

        if ( isinstance( Edge, geomBuilder.GEOM._objref_GEOM_Object)):
            EdgeID = self.geompyD.GetSubShapeID( self.geom, Edge )
        else:
            EdgeID = Edge
        try:
            self.editor.SetNodeOnEdge(NodeID, EdgeID, paramOnEdge)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    def SetNodeOnFace(self, NodeID, Face, u, v):
        """
        **Stores** node position on a face

	Parameters:
		NodeID: a node ID
		Face: a face or face ID
		u: U parameter on the face where the node is located
		v: V parameter on the face where the node is located

        Returns:
            True if succeed else raises an exception
	"""

        if ( isinstance( Face, geomBuilder.GEOM._objref_GEOM_Object)):
            FaceID = self.geompyD.GetSubShapeID( self.geom, Face )
        else:
            FaceID = Face
        try:
            self.editor.SetNodeOnFace(NodeID, FaceID, u, v)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    def SetNodeInVolume(self, NodeID, Solid):
        """
        **Binds** a node to a solid

	Parameters:
		NodeID: a node ID
		Solid:  a solid or solid ID

        Returns:
            True if succeed else raises an exception
	"""

        if ( isinstance( Solid, geomBuilder.GEOM._objref_GEOM_Object)):
            SolidID = self.geompyD.GetSubShapeID( self.geom, Solid )
        else:
            SolidID = Solid
        try:
            self.editor.SetNodeInVolume(NodeID, SolidID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True

    def SetMeshElementOnShape(self, ElementID, Shape):
        """
        **Bind** an element to a shape

	Parameters:
		ElementID: an element ID
		Shape: a shape or shape ID

        Returns:
            True if succeed else raises an exception
	"""

        if ( isinstance( Shape, geomBuilder.GEOM._objref_GEOM_Object)):
            ShapeID = self.geompyD.GetSubShapeID( self.geom, Shape )
        else:
            ShapeID = Shape
        try:
            self.editor.SetMeshElementOnShape(ElementID, ShapeID)
        except SALOME.SALOME_Exception, inst:
            raise ValueError, inst.details.text
        return True


    def MoveNode(self, NodeID, x, y, z):
        """
        Move the node with the given id

	Parameters:
		NodeID: the id of the node
		x:  a new X coordinate
		y:  a new Y coordinate
		z:  a new Z coordinate

        Returns:
            True if succeed else False
	"""

        x,y,z,Parameters,hasVars = ParseParameters(x,y,z)
        if hasVars: self.mesh.SetParameters(Parameters)
        return self.editor.MoveNode(NodeID, x, y, z)

    def MoveClosestNodeToPoint(self, x, y, z, NodeID):
        """
        Find the node closest to a point and moves it to a point location

	Parameters:
		x:  the X coordinate of a point
		y:  the Y coordinate of a point
		z:  the Z coordinate of a point
		NodeID: if specified (>0), the node with this ID is moved,
			otherwise, the node closest to point (*x*, *y*, *z*) is moved

        Returns:
            the ID of a node
	"""

        x,y,z,Parameters,hasVars = ParseParameters(x,y,z)
        if hasVars: self.mesh.SetParameters(Parameters)
        return self.editor.MoveClosestNodeToPoint(x, y, z, NodeID)

    def FindNodeClosestTo(self, x, y, z):
        """
        Find the node closest to a point

	Parameters:
		x:  the X coordinate of a point
		y:  the Y coordinate of a point
		z:  the Z coordinate of a point

        Returns:
            the ID of a node
	"""

        #preview = self.mesh.GetMeshEditPreviewer()
        #return preview.MoveClosestNodeToPoint(x, y, z, -1)
        return self.editor.FindNodeClosestTo(x, y, z)

    def FindElementsByPoint(self, x, y, z, elementType = SMESH.ALL, meshPart=None):
        """
        Find the elements where a point lays IN or ON

	Parameters:
		x:  the X coordinate of a point
		y:  the Y coordinate of a point
		z:  the Z coordinate of a point
		elementType: type of elements to find; either of
			(SMESH.NODE, SMESH.EDGE, SMESH.FACE, SMESH.VOLUME); SMESH.ALL type
			means elements of any type excluding nodes, discrete and 0D elements.
		meshPart: a part of mesh (group, sub-mesh) to search within

        Returns:
            list of IDs of found elements
	"""

        if meshPart:
            return self.editor.FindAmongElementsByPoint( meshPart, x, y, z, elementType );
        else:
            return self.editor.FindElementsByPoint(x, y, z, elementType)

    def GetPointState(self, x, y, z):
        """
        Return point state in a closed 2D mesh in terms of TopAbs_State enumeration:
	0-IN, 1-OUT, 2-ON, 3-UNKNOWN
	UNKNOWN state means that either mesh is wrong or the analysis fails.
	"""

        return self.editor.GetPointState(x, y, z)

    def IsManifold(self):
        """
        Check if a 2D mesh is manifold
	"""

        return self.editor.IsManifold()

    def IsCoherentOrientation2D(self):
        """
        Check if orientation of 2D elements is coherent
	"""

        return self.editor.IsCoherentOrientation2D()

    def MeshToPassThroughAPoint(self, x, y, z):
        """
        Find the node closest to a point and moves it to a point location

	Parameters:
		x:  the X coordinate of a point
		y:  the Y coordinate of a point
		z:  the Z coordinate of a point

        Returns:
            the ID of a moved node
	"""

        return self.editor.MoveClosestNodeToPoint(x, y, z, -1)

    def InverseDiag(self, NodeID1, NodeID2):
        """
        Replace two neighbour triangles sharing Node1-Node2 link
	with the triangles built on the same 4 nodes but having other common link.

	Parameters:
		NodeID1:  the ID of the first node
		NodeID2:  the ID of the second node

        Returns:
            false if proper faces were not found
	"""
        return self.editor.InverseDiag(NodeID1, NodeID2)

    def DeleteDiag(self, NodeID1, NodeID2):
        """
        Replace two neighbour triangles sharing Node1-Node2 link
	with a quadrangle built on the same 4 nodes.

	Parameters:
		NodeID1:  the ID of the first node
		NodeID2:  the ID of the second node

        Returns:
            false if proper faces were not found
	"""

        return self.editor.DeleteDiag(NodeID1, NodeID2)

    def Reorient(self, IDsOfElements=None):
        """
        Reorient elements by ids

	Parameters:
		IDsOfElements: if undefined reorients all mesh elements

        Returns:
            True if succeed else False
	"""

        if IDsOfElements == None:
            IDsOfElements = self.GetElementsId()
        return self.editor.Reorient(IDsOfElements)

    def ReorientObject(self, theObject):
        """
        Reorient all elements of the object

	Parameters:
		theObject: mesh, submesh or group

        Returns:
            True if succeed else False
	"""

        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.ReorientObject(theObject)

    def Reorient2D(self, the2DObject, theDirection, theFaceOrPoint ):
        """
        Reorient faces contained in *the2DObject*.

	Parameters:
		the2DObject: is a mesh, sub-mesh, group or list of IDs of 2D elements
		theDirection: is a desired direction of normal of *theFace*.
			It can be either a GEOM vector or a list of coordinates [x,y,z].
		theFaceOrPoint: defines a face of *the2DObject* whose normal will be
			compared with theDirection. It can be either ID of face or a point
			by which the face will be found. The point can be given as either
			a GEOM vertex or a list of point coordinates.

        Returns:
            number of reoriented faces
	"""

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

    def Reorient2DBy3D(self, the2DObject, the3DObject, theOutsideNormal=True ):
        """
        Reorient faces according to adjacent volumes.

	Parameters:
		the2DObject: is a mesh, sub-mesh, group or list of
			either IDs of faces or face groups.
		the3DObject: is a mesh, sub-mesh, group or list of IDs of volumes.
		theOutsideNormal: to orient faces to have their normals
			pointing either *outside* or *inside* the adjacent volumes.

        Returns:
            number of reoriented faces.
	"""

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

    def TriToQuad(self, IDsOfElements, theCriterion, MaxAngle):
        """
        Fuse the neighbouring triangles into quadrangles.

	Parameters:
		IDsOfElements: The triangles to be fused.
		theCriterion:  a numerical functor, in terms of enum SMESH.FunctorType, used to
			applied to possible quadrangles to choose a neighbour to fuse with.
			Type SMESH.FunctorType._items in the Python Console to see all items.
			Note that not all items correspond to numerical functors.
		MaxAngle: is the maximum angle between element normals at which the fusion
			is still performed; theMaxAngle is measured in radians.
			Also it could be a name of variable which defines angle in degrees.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""

        MaxAngle,Parameters,hasVars = ParseAngles(MaxAngle)
        self.mesh.SetParameters(Parameters)
        if not IDsOfElements:
            IDsOfElements = self.GetElementsId()
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.TriToQuad(IDsOfElements, Functor, MaxAngle)

    def TriToQuadObject (self, theObject, theCriterion, MaxAngle):
        """
        Fuse the neighbouring triangles of the object into quadrangles

	Parameters:
		theObject: is mesh, submesh or group
		theCriterion: is a numerical functor, in terms of enum SMESH.FunctorType,
			applied to possible quadrangles to choose a neighbour to fuse with.
			Type SMESH.FunctorType._items in the Python Console to see all items.
			Note that not all items correspond to numerical functors.
		MaxAngle: a max angle between element normals at which the fusion
			is still performed; theMaxAngle is measured in radians.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""

        MaxAngle,Parameters,hasVars = ParseAngles(MaxAngle)
        self.mesh.SetParameters(Parameters)
        if isinstance( theObject, Mesh ):
            theObject = theObject.GetMesh()
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.TriToQuadObject(theObject, Functor, MaxAngle)

    def QuadToTri (self, IDsOfElements, theCriterion = None):
        """
        Split quadrangles into triangles.

	Parameters:
		IDsOfElements: the faces to be splitted.
		theCriterion: is a numerical functor, in terms of enum SMESH.FunctorType, used to
			choose a diagonal for splitting. If *theCriterion* is None, which is a default
			value, then quadrangles will be split by the smallest diagonal.
			Type SMESH.FunctorType._items in the Python Console to see all items.
			Note that not all items correspond to numerical functors.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if theCriterion is None:
            theCriterion = FT_MaxElementLength2D
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.QuadToTri(IDsOfElements, Functor)

    def QuadToTriObject (self, theObject, theCriterion = None):
        """
        Split quadrangles into triangles.

	Parameters:
		theObject: the object from which the list of elements is taken,
			this is mesh, submesh or group
		theCriterion: is a numerical functor, in terms of enum SMESH.FunctorType, used to
			choose a diagonal for splitting. If *theCriterion* is None, which is a default
			value, then quadrangles will be split by the smallest diagonal.
			Type SMESH.FunctorType._items in the Python Console to see all items.
			Note that not all items correspond to numerical functors.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        if theCriterion is None:
            theCriterion = FT_MaxElementLength2D
        Functor = self.smeshpyD.GetFunctor(theCriterion)
        return self.editor.QuadToTriObject(theObject, Functor)

    def QuadTo4Tri (self, theElements=[]):
        """
        Split each of given quadrangles into 4 triangles. A node is added at the center of
		a quadrangle.

	Parameters:
		theElements: the faces to be splitted. This can be either mesh, sub-mesh,
			group or a list of face IDs. By default all quadrangles are split
	"""
        unRegister = genObjUnRegister()
        if isinstance( theElements, Mesh ):
            theElements = theElements.mesh
        elif not theElements:
            theElements = self.mesh
        elif isinstance( theElements, list ):
            theElements = self.GetIDSource( theElements, SMESH.FACE )
            unRegister.set( theElements )
        return self.editor.QuadTo4Tri( theElements )

    def SplitQuad (self, IDsOfElements, Diag13):
        """
        Split quadrangles into triangles.

	Parameters:
		IDsOfElements: the faces to be splitted
		Diag13:        is used to choose a diagonal for splitting.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""
        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        return self.editor.SplitQuad(IDsOfElements, Diag13)

    def SplitQuadObject (self, theObject, Diag13):
        """
        Split quadrangles into triangles.

	Parameters:
		theObject: the object from which the list of elements is taken,
			this is mesh, submesh or group
		Diag13:    is used to choose a diagonal for splitting.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""
        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SplitQuadObject(theObject, Diag13)

    def BestSplit (self, IDOfQuad, theCriterion):
        """
        Find a better splitting of the given quadrangle.

	Parameters:
		IDOfQuad:   the ID of the quadrangle to be splitted.
		theCriterion:  is a numerical functor, in terms of enum SMESH.FunctorType, used to
			choose a diagonal for splitting.
			Type SMESH.FunctorType._items in the Python Console to see all items.
			Note that not all items correspond to numerical functors.

        Returns:
            1 if 1-3 diagonal is better, 2 if 2-4
		diagonal is better, 0 if error occurs.
	"""
        return self.editor.BestSplit(IDOfQuad, self.smeshpyD.GetFunctor(theCriterion))

    def SplitVolumesIntoTetra(self, elems, method=smeshBuilder.Hex_5Tet ):
        """
        Split volumic elements into tetrahedrons

	Parameters:
		elems: either a list of elements or a mesh or a group or a submesh or a filter
		method:  flags passing splitting method:
			smesh.Hex_5Tet, smesh.Hex_6Tet, smesh.Hex_24Tet.
			smesh.Hex_5Tet - to split the hexahedron into 5 tetrahedrons, etc.
	"""
        unRegister = genObjUnRegister()
        if isinstance( elems, Mesh ):
            elems = elems.GetMesh()
        if ( isinstance( elems, list )):
            elems = self.editor.MakeIDSource(elems, SMESH.VOLUME)
            unRegister.set( elems )
        self.editor.SplitVolumesIntoTetra(elems, method)
        return

    def SplitBiQuadraticIntoLinear(self, elems=None):
        """
        Split bi-quadratic elements into linear ones without creation of additional nodes:

            - bi-quadratic triangle will be split into 3 linear quadrangles;
            - bi-quadratic quadrangle will be split into 4 linear quadrangles;
            - tri-quadratic hexahedron will be split into 8 linear hexahedra.

        Quadratic elements of lower dimension  adjacent to the split bi-quadratic element
        will be split in order to keep the mesh conformal.

        Parameters:
            elems: elements to split\: sub-meshes, groups, filters or element IDs;
                if None (default), all bi-quadratic elements will be split
        """
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

    def SplitHexahedraIntoPrisms(self, elems, startHexPoint, facetNormal,
                                 method=smeshBuilder.Hex_2Prisms, allDomains=False ):
        """
        Split hexahedra into prisms

	Parameters:
		elems: either a list of elements or a mesh or a group or a submesh or a filter
		startHexPoint: a point used to find a hexahedron for which *facetNormal*
			gives a normal vector defining facets to split into triangles.
			**startHexPoint** can be either a triple of coordinates or a vertex.
		facetNormal: a normal to a facet to split into triangles of a
			hexahedron found by *startHexPoint*.
			**facetNormal** can be either a triple of coordinates or an edge.
		method:  flags passing splitting method: smesh.Hex_2Prisms, smesh.Hex_4Prisms.
			smesh.Hex_2Prisms - to split the hexahedron into 2 prisms, etc.
		allDomains: if :code:`False`, only hexahedra adjacent to one closest
			to **startHexPoint** are split, else **startHexPoint**
			is used to find the facet to split in all domains present in *elems*.
	"""
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

    def SplitQuadsNearTriangularFacets(self):
        """
        Split quadrangle faces near triangular facets of volumes
	"""
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

    def SplitHexaToTetras (self, theObject, theNode000, theNode001):
        """
        **Splits** hexahedrons into tetrahedrons.

	This operation uses pattern mapping functionality for splitting.

	Parameters:
		theObject: the object from which the list of hexahedrons is taken; this is mesh, submesh or group.
		theNode000,theNode001: within the range [0,7]; gives the orientation of the
			pattern relatively each hexahedron: the (0,0,0) key-point of the pattern
			will be mapped into <VAR>theNode000</VAR>-th node of each volume, the (0,0,1)
			key-point will be mapped into <VAR>theNode001</VAR>-th node of each volume.
			The (0,0,0) key-point of the used pattern corresponds to a non-split corner.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""
#    Pattern:
#                     5.---------.6
#                    /|#*      /|
#                   / | #*    / |
#                  /  |  # * /  |
#                 /   |   # /*  |
#       (0,0,1) 4.---------.7 * |
#                |#*  |1   | # *|
#                | # *.----|---#.2
#                |  #/ *   |   /
#                |  /#  *  |  /
#                | /   # * | /
#                |/      #*|/
#        (0,0,0) 0.---------.3
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

    def SplitHexaToPrisms (self, theObject, theNode000, theNode001):
        """
        **Split** hexahedrons into prisms.

	Uses the pattern mapping functionality for splitting.

	Parameters:
		theObject: the object (mesh, submesh or group) from where the list of hexahedrons is taken;
		theNode000,theNode001: (within the range [0,7]) gives the orientation of the
			pattern relatively each hexahedron: keypoint (0,0,0) of the pattern
			will be mapped into the <VAR>theNode000</VAR>-th node of each volume, keypoint (0,0,1)
			will be mapped into the <VAR>theNode001</VAR>-th node of each volume.
			Edge (0,0,0)-(0,0,1) of used pattern connects two not split corners.

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""
#        Pattern:     5.---------.6
#                     /|#       /|
#                    / | #     / |
#                   /  |  #   /  |
#                  /   |   # /   |
#        (0,0,1) 4.---------.7   |
#                 |    |    |    |
#                 |   1.----|----.2
#                 |   / *   |   /
#                 |  /   *  |  /
#                 | /     * | /
#                 |/       *|/
#        (0,0,0) 0.---------.3
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

        # Split quafrangle faces near triangular facets of volumes
        self.SplitQuadsNearTriangularFacets()

        return isDone

    def Smooth(self, IDsOfElements, IDsOfFixedNodes,
               MaxNbOfIterations, MaxAspectRatio, Method):
        """
        Smooth elements

	Parameters:
		IDsOfElements: the list if ids of elements to smooth
		IDsOfFixedNodes: the list of ids of fixed nodes.
			Note that nodes built on edges and boundary nodes are always fixed.
		MaxNbOfIterations: the maximum number of iterations
		MaxAspectRatio: varies in range [1.0, inf]
		Method: is either Laplacian (smesh.LAPLACIAN_SMOOTH)
			or Centroidal (smesh.CENTROIDAL_SMOOTH)

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""

        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        MaxNbOfIterations,MaxAspectRatio,Parameters,hasVars = ParseParameters(MaxNbOfIterations,MaxAspectRatio)
        self.mesh.SetParameters(Parameters)
        return self.editor.Smooth(IDsOfElements, IDsOfFixedNodes,
                                  MaxNbOfIterations, MaxAspectRatio, Method)

    def SmoothObject(self, theObject, IDsOfFixedNodes,
                     MaxNbOfIterations, MaxAspectRatio, Method):
        """
        Smooth elements which belong to the given object

	Parameters:
		theObject: the object to smooth
		IDsOfFixedNodes: the list of ids of fixed nodes.
			Note that nodes built on edges and boundary nodes are always fixed.
		MaxNbOfIterations: the maximum number of iterations
		MaxAspectRatio: varies in range [1.0, inf]
		Method: is either Laplacian (smesh.LAPLACIAN_SMOOTH)
			or Centroidal (smesh.CENTROIDAL_SMOOTH)

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""

        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SmoothObject(theObject, IDsOfFixedNodes,
                                        MaxNbOfIterations, MaxAspectRatio, Method)

    def SmoothParametric(self, IDsOfElements, IDsOfFixedNodes,
                         MaxNbOfIterations, MaxAspectRatio, Method):
        """
        Parametrically smooth the given elements

	Parameters:
		IDsOfElements: the list if ids of elements to smooth
		IDsOfFixedNodes: the list of ids of fixed nodes.
			Note that nodes built on edges and boundary nodes are always fixed.
		MaxNbOfIterations: the maximum number of iterations
		MaxAspectRatio: varies in range [1.0, inf]
		Method: is either Laplacian (smesh.LAPLACIAN_SMOOTH)
			or Centroidal (smesh.CENTROIDAL_SMOOTH)

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""

        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        MaxNbOfIterations,MaxAspectRatio,Parameters,hasVars = ParseParameters(MaxNbOfIterations,MaxAspectRatio)
        self.mesh.SetParameters(Parameters)
        return self.editor.SmoothParametric(IDsOfElements, IDsOfFixedNodes,
                                            MaxNbOfIterations, MaxAspectRatio, Method)

    def SmoothParametricObject(self, theObject, IDsOfFixedNodes,
                               MaxNbOfIterations, MaxAspectRatio, Method):
        """
        Parametrically smooth the elements which belong to the given object

	Parameters:
		theObject: the object to smooth
		IDsOfFixedNodes: the list of ids of fixed nodes.
			Note that nodes built on edges and boundary nodes are always fixed.
		MaxNbOfIterations: the maximum number of iterations
		MaxAspectRatio: varies in range [1.0, inf]
		Method: is either Laplacian (smesh.LAPLACIAN_SMOOTH)
			or Centroidal (smesh.CENTROIDAL_SMOOTH)

        Returns:
            TRUE in case of success, FALSE otherwise.
	"""

        if ( isinstance( theObject, Mesh )):
            theObject = theObject.GetMesh()
        return self.editor.SmoothParametricObject(theObject, IDsOfFixedNodes,
                                                  MaxNbOfIterations, MaxAspectRatio, Method)

    def ConvertToQuadratic(self, theForce3d=False, theSubMesh=None, theToBiQuad=False):
        """
        Convert the mesh to quadratic or bi-quadratic, deletes old elements, replacing
		them with quadratic with the same id.

	Parameters:
		theForce3d: new node creation method:
			0 - the medium node lies at the geometrical entity from which the mesh element is built
			1 - the medium node lies at the middle of the line segments connecting two nodes of a mesh element
		theSubMesh: a group or a sub-mesh to convert; WARNING: in this case the mesh can become not conformal
		theToBiQuad: If True, converts the mesh to bi-quadratic

        Returns:
            SMESH.ComputeError which can hold a warning
	"""

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
        return error

    def ConvertFromQuadratic(self, theSubMesh=None):
        """
        Convert the mesh from quadratic to ordinary,
        deletes old quadratic elements,
        replacing them with ordinary mesh elements with the same id.

        Parameters:
            theSubMesh: a group or a sub-mesh to convert;

        Warning:
            in this case the mesh can become not conformal
	"""

        if theSubMesh:
            self.editor.ConvertFromQuadraticObject(theSubMesh)
        else:
            return self.editor.ConvertFromQuadratic()

    def Make2DMeshFrom3D(self):
        """
        Create 2D mesh as skin on boundary faces of a 3D mesh

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.Make2DMeshFrom3D()

    def MakeBoundaryMesh(self, elements, dimension=SMESH.BND_2DFROM3D, groupName="", meshName="",
                         toCopyElements=False, toCopyExistingBondary=False):
        """
        Create missing boundary elements

	Parameters:
		elements: elements whose boundary is to be checked:
			mesh, group, sub-mesh or list of elements
			if elements is mesh, it must be the mesh whose MakeBoundaryMesh() is called
		dimension: defines type of boundary elements to create, either of
			{ SMESH.BND_2DFROM3D, SMESH.BND_1DFROM3D, SMESH.BND_1DFROM2D }
			SMESH.BND_1DFROM3D create mesh edges on all borders of free facets of 3D cells
		groupName: a name of group to store created boundary elements in,
			"" means not to create the group
		meshName: a name of new mesh to store created boundary elements in,
			"" means not to create the new mesh
		toCopyElements: if true, the checked elements will be copied into
			the new mesh else only boundary elements will be copied into the new mesh
		toCopyExistingBondary: if true, not only new but also pre-existing
			boundary elements will be copied into the new mesh

        Returns:
            tuple (mesh, group) where boundary elements were added to
	"""

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

    def MakeBoundaryElements(self, dimension=SMESH.BND_2DFROM3D, groupName="", meshName="",
                             toCopyAll=False, groups=[]):
	"""
	**Create** missing boundary elements around either the whole mesh or
		groups of elements

	Parameters:
		dimension: defines type of boundary elements to create, either of
			{ SMESH.BND_2DFROM3D, SMESH.BND_1DFROM3D, SMESH.BND_1DFROM2D }
		groupName: a name of group to store all boundary elements in,
			"" means not to create the group
		meshName: a name of a new mesh, which is a copy of the initial
			mesh + created boundary elements; "" means not to create the new mesh
		toCopyAll: if true, the whole initial mesh will be copied into
			the new mesh else only boundary elements will be copied into the new mesh
		groups: groups of elements to make boundary around

	Returns:
		tuple( long, mesh, groups )
			long - number of added boundary elements
			mesh - the mesh where elements were added to
			group - the group of boundary elements or None
	"""

        nb, mesh, group = self.editor.MakeBoundaryElements(dimension,groupName,meshName,
                                                           toCopyAll,groups)
        if mesh: mesh = self.smeshpyD.Mesh(mesh)
        return nb, mesh, group

    def RenumberNodes(self):
        """
        Renumber mesh nodes (Obsolete, does nothing)
	"""
        self.editor.RenumberNodes()

    def RenumberElements(self):
        """
        Renumber mesh elements (Obsole, does nothing)
	"""
        self.editor.RenumberElements()

    def _getIdSourceList(self, arg, idType, unRegister):
        """
        Private method converting *arg* into a list of SMESH_IdSource's
	"""
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

    def RotationSweepObjects(self, nodes, edges, faces, Axis, AngleInRadians, NbOfSteps, Tolerance,
                             MakeGroups=False, TotalAngle=False):
        """
        Generate new elements by rotation of the given elements and nodes around the axis

	Parameters:
		nodes: nodes to revolve: a list including ids, groups, sub-meshes or a mesh
		edges: edges to revolve: a list including ids, groups, sub-meshes or a mesh
		faces: faces to revolve: a list including ids, groups, sub-meshes or a mesh
		Axis: the axis of rotation: AxisStruct, line (geom object) or [x,y,z,dx,dy,dz]
		AngleInRadians: the angle of Rotation (in radians) or a name of variable
			which defines angle in degrees
		NbOfSteps: the number of steps
		Tolerance: tolerance
		MakeGroups: forces the generation of new groups from existing ones
		TotalAngle: gives meaning of AngleInRadians: if True then it is an angular size
			of all steps, else - size of each step

        Returns:
            the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

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

    def RotationSweep(self, IDsOfElements, Axis, AngleInRadians, NbOfSteps, Tolerance,
                      MakeGroups=False, TotalAngle=False):
        """
        Generate new elements by rotation of the elements around the axis

	Parameters:
            IDsOfElements: the list of ids of elements to sweep
            Axis: the axis of rotation, AxisStruct or line(geom object)
            AngleInRadians: the angle of Rotation (in radians) or a name of variable which defines angle in degrees
            NbOfSteps: the number of steps
            Tolerance: tolerance
            MakeGroups: forces the generation of new groups from existing ones
            TotalAngle: gives meaning of AngleInRadians: if True then it is an angular size
		of all steps, else - size of each step

        Returns:
            the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

        return self.RotationSweepObjects([], IDsOfElements, IDsOfElements, Axis,
                                         AngleInRadians, NbOfSteps, Tolerance,
                                         MakeGroups, TotalAngle)

    def RotationSweepObject(self, theObject, Axis, AngleInRadians, NbOfSteps, Tolerance,
                            MakeGroups=False, TotalAngle=False):
        """
        Generate new elements by rotation of the elements of object around the axis
            theObject object which elements should be sweeped.
		It can be a mesh, a sub mesh or a group.

	Parameters:
            Axis: the axis of rotation, AxisStruct or line(geom object)
            AngleInRadians: the angle of Rotation
            NbOfSteps: number of steps
            Tolerance: tolerance
            MakeGroups: forces the generation of new groups from existing ones
            TotalAngle: gives meaning of AngleInRadians: if True then it is an angular size
		of all steps, else - size of each step

        Returns:
            the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

        return self.RotationSweepObjects( [], theObject, theObject, Axis,
                                          AngleInRadians, NbOfSteps, Tolerance,
                                          MakeGroups, TotalAngle )

    def RotationSweepObject1D(self, theObject, Axis, AngleInRadians, NbOfSteps, Tolerance,
                              MakeGroups=False, TotalAngle=False):
        """
        Generate new elements by rotation of the elements of object around the axis
            theObject object which elements should be sweeped.
	    It can be a mesh, a sub mesh or a group.

	Parameters:
            Axis: the axis of rotation, AxisStruct or line(geom object)
            AngleInRadians: the angle of Rotation
            NbOfSteps: number of steps
            Tolerance: tolerance
            MakeGroups: forces the generation of new groups from existing ones
            TotalAngle: gives meaning of AngleInRadians: if True then it is an angular size
		of all steps, else - size of each step

        Returns:
            the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

        return self.RotationSweepObjects([],theObject,[], Axis,
                                         AngleInRadians, NbOfSteps, Tolerance,
                                         MakeGroups, TotalAngle)

    def RotationSweepObject2D(self, theObject, Axis, AngleInRadians, NbOfSteps, Tolerance,
                              MakeGroups=False, TotalAngle=False):
        """
        Generate new elements by rotation of the elements of object around the axis
            theObject object which elements should be sweeped.
	    It can be a mesh, a sub mesh or a group.

	Parameters:
            Axis: the axis of rotation, AxisStruct or line(geom object)
            AngleInRadians: the angle of Rotation
            NbOfSteps: number of steps
            Tolerance: tolerance
            MakeGroups: forces the generation of new groups from existing ones
            TotalAngle: gives meaning of AngleInRadians: if True then it is an angular size
		of all steps, else - size of each step

        Returns:
            the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

        return self.RotationSweepObjects([],[],theObject, Axis, AngleInRadians,
                                         NbOfSteps, Tolerance, MakeGroups, TotalAngle)

    def ExtrusionSweepObjects(self, nodes, edges, faces, StepVector, NbOfSteps, MakeGroups=False,
                              scaleFactors=[], linearVariation=False, basePoint=[] ):
        """
        Generate new elements by extrusion of the given elements and nodes

	Parameters:
            nodes: nodes to extrude: a list including ids, groups, sub-meshes or a mesh
            edges: edges to extrude: a list including ids, groups, sub-meshes or a mesh
            faces: faces to extrude: a list including ids, groups, sub-meshes or a mesh
            StepVector: vector or DirStruct or 3 vector components, defining
		the direction and value of extrusion for one step (the total extrusion
		length will be NbOfSteps * ||StepVector||)
            NbOfSteps: the number of steps
            MakeGroups: forces the generation of new groups from existing ones
            scaleFactors: optional scale factors to apply during extrusion
            linearVariation: if *True*, scaleFactors are spread over all *scaleFactors*,
		else scaleFactors[i] is applied to nodes at the i-th extrusion step
            basePoint: optional scaling center; if not provided, a gravity center of
		nodes and elements being extruded is used as the scaling center.
		It can be either

			- a list of tree components of the point or
			- a node ID or
			- a GEOM point

        Returns:
            the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise

        :ref:`tui_extrusion` example
	"""
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


    def ExtrusionSweep(self, IDsOfElements, StepVector, NbOfSteps, MakeGroups=False, IsNodes = False):
        """
        Generate new elements by extrusion of the elements with given ids

	Parameters:
            IDsOfElements: the list of ids of elements or nodes for extrusion
            StepVector: vector or DirStruct or 3 vector components, defining
		the direction and value of extrusion for one step (the total extrusion
		length will be NbOfSteps * ||StepVector||)
            NbOfSteps: the number of steps
            MakeGroups: forces the generation of new groups from existing ones
            IsNodes: is True if elements with given ids are nodes

        Returns:
            the list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise

	:ref:`tui_extrusion` example
	"""
        n,e,f = [],[],[]
        if IsNodes: n = IDsOfElements
        else      : e,f, = IDsOfElements,IDsOfElements
        return self.ExtrusionSweepObjects(n,e,f, StepVector, NbOfSteps, MakeGroups)

    def ExtrusionByNormal(self, Elements, StepSize, NbOfSteps,
                          ByAverageNormal=False, UseInputElemsOnly=True, MakeGroups=False, Dim = 2):
        """
        Generate new elements by extrusion along the normal to a discretized surface or wire

	Parameters:
            Elements: elements to extrude - a list including ids, groups, sub-meshes or a mesh.
		Only faces can be extruded so far. A sub-mesh should be a sub-mesh on geom faces.
            StepSize: length of one extrusion step (the total extrusion
		length will be *NbOfSteps* *StepSize*).
            NbOfSteps: number of extrusion steps.
            ByAverageNormal: if True each node is translated by *StepSize*
		along the average of the normal vectors to the faces sharing the node;
		else each node is translated along the same average normal till
		intersection with the plane got by translation of the face sharing
		the node along its own normal by *StepSize*.
            UseInputElemsOnly: to use only *Elements* when computing extrusion direction
		for every node of *Elements*.
            MakeGroups: forces generation of new groups from existing ones.
            Dim: dimension of elements to extrude: 2 - faces or 1 - edges. Extrusion of edges
		is not yet implemented. This parameter is used if *Elements* contains
		both faces and edges, i.e. *Elements* is a Mesh.

        Returns:
            the list of created groups (SMESH_GroupBase) if *MakeGroups=True*,
		empty list otherwise.
	:ref:`tui_extrusion` example
	"""

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

    def ExtrusionSweepObject(self, theObject, StepVector, NbOfSteps, MakeGroups=False, IsNodes=False):
        """
        Generate new elements by extrusion of the elements or nodes which belong to the object

	Parameters:
            theObject: the object whose elements or nodes should be processed.
		It can be a mesh, a sub-mesh or a group.
            StepVector: vector or DirStruct or 3 vector components, defining
		the direction and value of extrusion for one step (the total extrusion
		length will be NbOfSteps * ||StepVector||)
            NbOfSteps: the number of steps
            MakeGroups: forces the generation of new groups from existing ones
            IsNodes: is True if elements to extrude are nodes

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	:ref:`tui_extrusion` example
	"""

        n,e,f = [],[],[]
        if IsNodes: n    = theObject
        else      : e,f, = theObject,theObject
        return self.ExtrusionSweepObjects(n,e,f, StepVector, NbOfSteps, MakeGroups)

    def ExtrusionSweepObject1D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        """
        Generate new elements by extrusion of edges which belong to the object

	Parameters:
            theObject: object whose 1D elements should be processed.
		It can be a mesh, a sub-mesh or a group.
            StepVector: vector or DirStruct or 3 vector components, defining
		the direction and value of extrusion for one step (the total extrusion
		length will be NbOfSteps * ||StepVector||)
            NbOfSteps: the number of steps
            MakeGroups: to generate new groups from existing ones

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	:ref:`tui_extrusion` example
	"""

        return self.ExtrusionSweepObjects([],theObject,[], StepVector, NbOfSteps, MakeGroups)

    def ExtrusionSweepObject2D(self, theObject, StepVector, NbOfSteps, MakeGroups=False):
        """
        Generate new elements by extrusion of faces which belong to the object

	Parameters:
            theObject: object whose 2D elements should be processed.
		It can be a mesh, a sub-mesh or a group.
            StepVector: vector or DirStruct or 3 vector components, defining
		the direction and value of extrusion for one step (the total extrusion
		length will be NbOfSteps * ||StepVector||)
            NbOfSteps: the number of steps
            MakeGroups: forces the generation of new groups from existing ones

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	:ref:`tui_extrusion` example
	"""

        return self.ExtrusionSweepObjects([],[],theObject, StepVector, NbOfSteps, MakeGroups)

    def AdvancedExtrusion(self, IDsOfElements, StepVector, NbOfSteps,
                          ExtrFlags, SewTolerance, MakeGroups=False):
        """
        Generate new elements by extrusion of the elements with given ids

	Parameters:
            IDsOfElements: is ids of elements
            StepVector: vector or DirStruct or 3 vector components, defining
		the direction and value of extrusion for one step (the total extrusion
		length will be NbOfSteps * ||StepVector||)
            NbOfSteps: the number of steps
            ExtrFlags: sets flags for extrusion
            SewTolerance: uses for comparing locations of nodes if flag
		EXTRUSION_FLAG_SEW is set
            MakeGroups: forces the generation of new groups from existing ones

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

        if isinstance( StepVector, geomBuilder.GEOM._objref_GEOM_Object):
            StepVector = self.smeshpyD.GetDirStruct(StepVector)
        if isinstance( StepVector, list ):
            StepVector = self.smeshpyD.MakeDirStruct(*StepVector)
        return self.editor.AdvancedExtrusion(IDsOfElements, StepVector, NbOfSteps,
                                             ExtrFlags, SewTolerance, MakeGroups)

    def ExtrusionAlongPathObjects(self, Nodes, Edges, Faces, PathMesh, PathShape=None,
                                  NodeStart=1, HasAngles=False, Angles=[], LinearVariation=False,
                                  HasRefPoint=False, RefPoint=[0,0,0], MakeGroups=False):
        """
        Generate new elements by extrusion of the given elements and nodes along the path.
        The path of extrusion must be a meshed edge.

        Parameters:
            Nodes: nodes to extrude: a list including ids, groups, sub-meshes or a mesh
            Edges: edges to extrude: a list including ids, groups, sub-meshes or a mesh
            Faces: faces to extrude: a list including ids, groups, sub-meshes or a mesh
            PathMesh: 1D mesh or 1D sub-mesh, along which proceeds the extrusion
            PathShape: shape (edge) defines the sub-mesh of PathMesh if PathMesh
                contains not only path segments, else it can be None
            NodeStart: the first or the last node on the path. Defines the direction of extrusion
            HasAngles: allows the shape to be rotated around the path
                to get the resulting mesh in a helical fashion
            Angles: list of angles
            LinearVariation: forces the computation of rotation angles as linear
                variation of the given Angles along path steps
            HasRefPoint: allows using the reference point
            RefPoint: the point around which the shape is rotated (the mass center of the
                shape by default). The User can specify any point as the Reference Point.
            MakeGroups: forces the generation of new groups from existing ones

        Returns:
            list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error
        :ref:`tui_extrusion_along_path` example
        """

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

    def ExtrusionAlongPathX(self, Base, Path, NodeStart,
                            HasAngles=False, Angles=[], LinearVariation=False,
                            HasRefPoint=False, RefPoint=[0,0,0], MakeGroups=False,
                            ElemType=SMESH.FACE):
        """
        Generate new elements by extrusion of the given elements
	The path of extrusion must be a meshed edge.

	Parameters:
            Base: mesh or group, or sub-mesh, or list of ids of elements for extrusion
            Path: 1D mesh or 1D sub-mesh, along which proceeds the extrusion
            NodeStart: the start node from Path. Defines the direction of extrusion
            HasAngles: allows the shape to be rotated around the path
		to get the resulting mesh in a helical fashion
            Angles: list of angles in radians
            LinearVariation: forces the computation of rotation angles as linear
		variation of the given Angles along path steps
            HasRefPoint: allows using the reference point
            RefPoint: the point around which the elements are rotated (the mass
		center of the elements by default).
		The User can specify any point as the Reference Point.
		RefPoint can be either GEOM Vertex, [x,y,z] or SMESH.PointStruct
            MakeGroups: forces the generation of new groups from existing ones
            ElemType: type of elements for extrusion (if param Base is a mesh)

        Returns:
            list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
		only SMESH::Extrusion_Error otherwise
	:ref:`tui_extrusion_along_path` example
	"""

        n,e,f = [],[],[]
        if ElemType == SMESH.NODE: n = Base
        if ElemType == SMESH.EDGE: e = Base
        if ElemType == SMESH.FACE: f = Base
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, Path, None, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    def ExtrusionAlongPath(self, IDsOfElements, PathMesh, PathShape, NodeStart,
                           HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                           MakeGroups=False, LinearVariation=False):
        """
        Generate new elements by extrusion of the given elements
	The path of extrusion must be a meshed edge.

	Parameters:
            IDsOfElements: ids of elements
            PathMesh: mesh containing a 1D sub-mesh on the edge, along which proceeds the extrusion
            PathShape: shape(edge) defines the sub-mesh for the path
            NodeStart: the first or the last node on the edge. Defines the direction of extrusion
            HasAngles: allows the shape to be rotated around the path
		to get the resulting mesh in a helical fashion
            Angles: list of angles in radians
            HasRefPoint: allows using the reference point
            RefPoint: the point around which the shape is rotated (the mass center of the shape by default).
		The User can specify any point as the Reference Point.
            MakeGroups: forces the generation of new groups from existing ones
            LinearVariation: forces the computation of rotation angles as linear
		variation of the given Angles along path steps

        Returns:
            list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
		only SMESH::Extrusion_Error otherwise
	:ref:`tui_extrusion_along_path` example
	"""

        n,e,f = [],IDsOfElements,IDsOfElements
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape,
                                               NodeStart, HasAngles, Angles,
                                               LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    def ExtrusionAlongPathObject(self, theObject, PathMesh, PathShape, NodeStart,
                                 HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                                 MakeGroups=False, LinearVariation=False):
        """
        Generate new elements by extrusion of the elements which belong to the object
	The path of extrusion must be a meshed edge.

	Parameters:
            theObject: the object whose elements should be processed.
		It can be a mesh, a sub-mesh or a group.
            PathMesh: mesh containing a 1D sub-mesh on the edge, along which the extrusion proceeds
            PathShape: shape(edge) defines the sub-mesh for the path
            NodeStart: the first or the last node on the edge. Defines the direction of extrusion
            HasAngles: allows the shape to be rotated around the path
		to get the resulting mesh in a helical fashion
            Angles: list of angles
            HasRefPoint: allows using the reference point
            RefPoint: the point around which the shape is rotated (the mass center of the shape by default).
		The User can specify any point as the Reference Point.
            MakeGroups: forces the generation of new groups from existing ones
            LinearVariation: forces the computation of rotation angles as linear
		variation of the given Angles along path steps

        Returns:
            list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
		only SMESH::Extrusion_Error otherwise
	:ref:`tui_extrusion_along_path` example
	"""

        n,e,f = [],theObject,theObject
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    def ExtrusionAlongPathObject1D(self, theObject, PathMesh, PathShape, NodeStart,
                                   HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                                   MakeGroups=False, LinearVariation=False):
        """
        Generate new elements by extrusion of mesh segments which belong to the object
	The path of extrusion must be a meshed edge.

	Parameters:
            theObject: the object whose 1D elements should be processed.
		It can be a mesh, a sub-mesh or a group.
            PathMesh: mesh containing a 1D sub-mesh on the edge, along which the extrusion proceeds
            PathShape: shape(edge) defines the sub-mesh for the path
            NodeStart: the first or the last node on the edge. Defines the direction of extrusion
            HasAngles: allows the shape to be rotated around the path
		to get the resulting mesh in a helical fashion
            Angles: list of angles
            HasRefPoint: allows using the reference point
            RefPoint the point: around which the shape is rotated (the mass center of the shape by default).
		The User can specify any point as the Reference Point.
            MakeGroups: forces the generation of new groups from existing ones
            LinearVariation: forces the computation of rotation angles as linear
		variation of the given Angles along path steps

        Returns:
            list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
		only SMESH::Extrusion_Error otherwise
	:ref:`tui_extrusion_along_path` example
	"""

        n,e,f = [],theObject,[]
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    def ExtrusionAlongPathObject2D(self, theObject, PathMesh, PathShape, NodeStart,
                                   HasAngles=False, Angles=[], HasRefPoint=False, RefPoint=[],
                                   MakeGroups=False, LinearVariation=False):
        """
        Generate new elements by extrusion of faces which belong to the object
	The path of extrusion must be a meshed edge.

	Parameters:
            theObject: the object whose 2D elements should be processed.
		It can be a mesh, a sub-mesh or a group.
            PathMesh: mesh containing a 1D sub-mesh on the edge, along which the extrusion proceeds
            PathShape: shape(edge) defines the sub-mesh for the path
            NodeStart: the first or the last node on the edge. Defines the direction of extrusion
            HasAngles: allows the shape to be rotated around the path
		to get the resulting mesh in a helical fashion
            Angles: list of angles
            HasRefPoint: allows using the reference point
            RefPoint: the point around which the shape is rotated (the mass center of the shape by default).
		The User can specify any point as the Reference Point.
            MakeGroups: forces the generation of new groups from existing ones
            LinearVariation: forces the computation of rotation angles as linear
		variation of the given Angles along path steps

        Returns:
            list of created groups (SMESH_GroupBase) and SMESH::Extrusion_Error if MakeGroups=True,
		only SMESH::Extrusion_Error otherwise
	:ref:`tui_extrusion_along_path` example
	"""

        n,e,f = [],[],theObject
        gr,er = self.ExtrusionAlongPathObjects(n,e,f, PathMesh, PathShape, NodeStart,
                                               HasAngles, Angles, LinearVariation,
                                               HasRefPoint, RefPoint, MakeGroups)
        if MakeGroups: return gr,er
        return er

    def Mirror(self, IDsOfElements, Mirror, theMirrorType=None, Copy=0, MakeGroups=False):
        """
        Create a symmetrical copy of mesh elements

	Parameters:
            IDsOfElements: list of elements ids
            Mirror: is AxisStruct or geom object(point, line, plane)
            theMirrorType: smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
		If the Mirror is a geom object this parameter is unnecessary
            Copy: allows to copy element (Copy is 1) or to replace with its mirroring (Copy is 0)
            MakeGroups: forces the generation of new groups from existing ones (if Copy)

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

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

    def MirrorMakeMesh(self, IDsOfElements, Mirror, theMirrorType=0, MakeGroups=0, NewMeshName=""):
        """
        Create a new mesh by a symmetrical copy of mesh elements

	Parameters:
            IDsOfElements: the list of elements ids
            Mirror: is AxisStruct or geom object (point, line, plane)
            theMirrorType: smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
		If the Mirror is a geom object this parameter is unnecessary
            MakeGroups: to generate new groups from existing ones
            NewMeshName: a name of the new mesh to create

        Returns:
            instance of Mesh class
	"""

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

    def MirrorObject (self, theObject, Mirror, theMirrorType=None, Copy=0, MakeGroups=False):
        """
        Create a symmetrical copy of the object

	Parameters:
            theObject: mesh, submesh or group
            Mirror: AxisStruct or geom object (point, line, plane)
            theMirrorType: smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
		If the Mirror is a geom object this parameter is unnecessary
            Copy: allows copying the element (Copy is 1) or replacing it with its mirror (Copy is 0)
            MakeGroups: forces the generation of new groups from existing ones (if Copy)

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

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

    def MirrorObjectMakeMesh (self, theObject, Mirror, theMirrorType=0,MakeGroups=0,NewMeshName=""):
        """
        Create a new mesh by a symmetrical copy of the object

	Parameters:
            theObject: mesh, submesh or group
            Mirror: AxisStruct or geom object (point, line, plane)
            theMirrorType: smeshBuilder.POINT, smeshBuilder.AXIS or smeshBuilder.PLANE
		If the Mirror is a geom object this parameter is unnecessary
            MakeGroups: forces the generation of new groups from existing ones
            NewMeshName: the name of the new mesh to create

        Returns:
            instance of Mesh class
	"""

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

    def Translate(self, IDsOfElements, Vector, Copy, MakeGroups=False):
        """
        Translate the elements

	Parameters:
            IDsOfElements: list of elements ids
            Vector: the direction of translation (DirStruct or vector or 3 vector components)
            Copy: allows copying the translated elements
            MakeGroups: forces the generation of new groups from existing ones (if Copy)

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

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

    def TranslateMakeMesh(self, IDsOfElements, Vector, MakeGroups=False, NewMeshName=""):
        """
        Create a new mesh of translated elements

	Parameters:
            IDsOfElements: list of elements ids
            Vector: the direction of translation (DirStruct or vector or 3 vector components)
            MakeGroups: forces the generation of new groups from existing ones
            NewMeshName: the name of the newly created mesh

        Returns:
            instance of Mesh class
	"""

        if IDsOfElements == []:
            IDsOfElements = self.GetElementsId()
        if ( isinstance( Vector, geomBuilder.GEOM._objref_GEOM_Object)):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if isinstance( Vector, list ):
            Vector = self.smeshpyD.MakeDirStruct(*Vector)
        self.mesh.SetParameters(Vector.PS.parameters)
        mesh = self.editor.TranslateMakeMesh(IDsOfElements, Vector, MakeGroups, NewMeshName)
        return Mesh ( self.smeshpyD, self.geompyD, mesh )

    def TranslateObject(self, theObject, Vector, Copy, MakeGroups=False):
        """
        Translate the object

	Parameters:
            theObject: the object to translate (mesh, submesh, or group)
            Vector: direction of translation (DirStruct or geom vector or 3 vector components)
            Copy: allows copying the translated elements
            MakeGroups: forces the generation of new groups from existing ones (if Copy)

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

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

    def TranslateObjectMakeMesh(self, theObject, Vector, MakeGroups=False, NewMeshName=""):
        """
        Create a new mesh from the translated object

	Parameters:
            theObject: the object to translate (mesh, submesh, or group)
            Vector: the direction of translation (DirStruct or geom vector or 3 vector components)
            MakeGroups: forces the generation of new groups from existing ones
            NewMeshName: the name of the newly created mesh

        Returns:
            instance of Mesh class
	"""

        if isinstance( theObject, Mesh ):
            theObject = theObject.GetMesh()
        if isinstance( Vector, geomBuilder.GEOM._objref_GEOM_Object ):
            Vector = self.smeshpyD.GetDirStruct(Vector)
        if isinstance( Vector, list ):
            Vector = self.smeshpyD.MakeDirStruct(*Vector)
        self.mesh.SetParameters(Vector.PS.parameters)
        mesh = self.editor.TranslateObjectMakeMesh(theObject, Vector, MakeGroups, NewMeshName)
        return Mesh( self.smeshpyD, self.geompyD, mesh )



    def Scale(self, theObject, thePoint, theScaleFact, Copy, MakeGroups=False):
        """
        Scale the object

	Parameters:
            theObject: the object to translate (mesh, submesh, or group)
            thePoint: base point for scale (SMESH.PointStruct or list of 3 coordinates)
            theScaleFact: list of 1-3 scale factors for axises
            Copy: allows copying the translated elements
            MakeGroups: forces the generation of new groups from existing
		ones (if Copy)

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True,
		empty list otherwise
	"""
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

    def ScaleMakeMesh(self, theObject, thePoint, theScaleFact, MakeGroups=False, NewMeshName=""):
        """
        Create a new mesh from the translated object

	Parameters:
            theObject: the object to translate (mesh, submesh, or group)
            thePoint: base point for scale (SMESH.PointStruct or list of 3 coordinates)
            theScaleFact: list of 1-3 scale factors for axises
            MakeGroups: forces the generation of new groups from existing ones
            NewMeshName: the name of the newly created mesh

        Returns:
            instance of Mesh class
	"""
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



    def Rotate (self, IDsOfElements, Axis, AngleInRadians, Copy, MakeGroups=False):
        """
        Rotate the elements

	Parameters:
            IDsOfElements: list of elements ids
            Axis: the axis of rotation (AxisStruct or geom line)
            AngleInRadians: the angle of rotation (in radians) or a name of variable which defines angle in degrees
            Copy: allows copying the rotated elements
            MakeGroups: forces the generation of new groups from existing ones (if Copy)

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""


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

    def RotateMakeMesh (self, IDsOfElements, Axis, AngleInRadians, MakeGroups=0, NewMeshName=""):
        """
        Create a new mesh of rotated elements

	Parameters:
            IDsOfElements: list of element ids
            Axis: the axis of rotation (AxisStruct or geom line)
            AngleInRadians: the angle of rotation (in radians) or a name of variable which defines angle in degrees
            MakeGroups: forces the generation of new groups from existing ones
            NewMeshName: the name of the newly created mesh

        Returns:
            instance of Mesh class
	"""

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

    def RotateObject (self, theObject, Axis, AngleInRadians, Copy, MakeGroups=False):
        """
        Rotate the object

	Parameters:
            theObject: the object to rotate( mesh, submesh, or group)
            Axis: the axis of rotation (AxisStruct or geom line)
            AngleInRadians: the angle of rotation (in radians) or a name of variable which defines angle in degrees
            Copy: allows copying the rotated elements
            MakeGroups: forces the generation of new groups from existing ones (if Copy)

        Returns:
            list of created groups (SMESH_GroupBase) if MakeGroups=True, empty list otherwise
	"""

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

    def RotateObjectMakeMesh(self, theObject, Axis, AngleInRadians, MakeGroups=0,NewMeshName=""):
        """
        Create a new mesh from the rotated object

	Parameters:
            theObject: the object to rotate (mesh, submesh, or group)
            Axis: the axis of rotation (AxisStruct or geom line)
            AngleInRadians: the angle of rotation (in radians)  or a name of variable which defines angle in degrees
            MakeGroups: forces the generation of new groups from existing ones
            NewMeshName: the name of the newly created mesh

        Returns:
            instance of Mesh class
	"""

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

    def Offset(self, theObject, theValue, MakeGroups=False, NewMeshName=''):
        """
        Create an offset mesh from the given 2D object

	Parameters:
            theObject: the source object (mesh, submesh, group or filter)
            theValue: signed offset size
            MakeGroups: forces the generation of new groups from existing ones
            NewMeshName: the name of a mesh to create. If empty, offset elements are added to this mesh

        Returns:
            A tuple (mesh, list_of_groups)
        """

        if isinstance( theObject, Mesh ):
            theObject = theObject.GetMesh()
        theValue,Parameters,hasVars = ParseParameters(theValue)
        mesh_groups = self.editor.Offset(theObject, theValue, MakeGroups, NewMeshName )
        self.mesh.SetParameters(Parameters)
        # if mesh_groups[0]:
        #     return Mesh( self.smeshpyD, self.geompyD, mesh_groups[0] ), mesh_groups[1]
        return mesh_groups

    def FindCoincidentNodes (self, Tolerance, SeparateCornerAndMediumNodes=False):
        """
        Find groups of adjacent nodes within Tolerance.

	Parameters:
            Tolerance: the value of tolerance
            SeparateCornerAndMediumNodes: if *True*, in quadratic mesh puts
		corner and medium nodes in separate groups thus preventing
		their further merge.

        Returns:
            the list of groups of nodes IDs (e.g. [[1,12,13],[4,25]])
	"""

        return self.editor.FindCoincidentNodes( Tolerance, SeparateCornerAndMediumNodes )

    def FindCoincidentNodesOnPart (self, SubMeshOrGroup, Tolerance,
                                   exceptNodes=[], SeparateCornerAndMediumNodes=False):
        """
        Find groups of ajacent nodes within Tolerance.

	Parameters:
            Tolerance: the value of tolerance
            SubMeshOrGroup: SubMesh, Group or Filter
            exceptNodes: list of either SubMeshes, Groups or node IDs to exclude from search
            SeparateCornerAndMediumNodes: if *True*, in quadratic mesh puts
		corner and medium nodes in separate groups thus preventing
		their further merge.

        Returns:
            the list of groups of nodes IDs (e.g. [[1,12,13],[4,25]])
	"""

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

    def MergeNodes (self, GroupsOfNodes, NodesToKeep=[], AvoidMakingHoles=False):
        """
        Merge nodes

	Parameters:
            GroupsOfNodes: a list of groups of nodes IDs for merging
		(e.g. [[1,12,13],[25,4]], then nodes 12, 13 and 4 will be removed and replaced
		by nodes 1 and 25 correspondingly in all elements and groups
            NodesToKeep: nodes to keep in the mesh: a list of groups, sub-meshes or node IDs.
		If *NodesToKeep* does not include a node to keep for some group to merge,
		then the first node in the group is kept.
            AvoidMakingHoles: prevent merging nodes which cause removal of elements becoming
		invalid
	"""


        # NodesToKeep are converted to SMESH_IDSource in meshEditor.MergeNodes()
        self.editor.MergeNodes( GroupsOfNodes, NodesToKeep, AvoidMakingHoles )

    def FindEqualElements (self, MeshOrSubMeshOrGroup=None):
        """
        Find the elements built on the same nodes.

	Parameters:
            MeshOrSubMeshOrGroup: Mesh or SubMesh, or Group of elements for searching

        Returns:
            the list of groups of equal elements IDs (e.g. [[1,12,13],[4,25]])
	"""

        if not MeshOrSubMeshOrGroup:
            MeshOrSubMeshOrGroup=self.mesh
        elif isinstance( MeshOrSubMeshOrGroup, Mesh ):
            MeshOrSubMeshOrGroup = MeshOrSubMeshOrGroup.GetMesh()
        return self.editor.FindEqualElements( MeshOrSubMeshOrGroup )

    def MergeElements(self, GroupsOfElementsID):
        """
        Merge elements in each given group.

	Parameters:
            GroupsOfElementsID: a list of groups of elements IDs for merging
		(e.g. [[1,12,13],[25,4]], then elements 12, 13 and 4 will be removed and
		replaced by elements 1 and 25 in all groups)
	"""

        self.editor.MergeElements(GroupsOfElementsID)

    def MergeEqualElements(self):
        """
        Leave one element and remove all other elements built on the same nodes.
	"""

        self.editor.MergeEqualElements()

    def FindFreeBorders(self, ClosedOnly=True):
        """
        Returns all or only closed free borders

        Returns:
            list of SMESH.FreeBorder's
	"""

        return self.editor.FindFreeBorders( ClosedOnly )

    def FillHole(self, holeNodes):
        """
        Fill with 2D elements a hole defined by a SMESH.FreeBorder.

	Parameters:
            FreeBorder: either a SMESH.FreeBorder or a list on node IDs. These nodes
		must describe all sequential nodes of the hole border. The first and the last
		nodes must be the same. Use FindFreeBorders() to get nodes of holes.
	"""


        if holeNodes and isinstance( holeNodes, list ) and isinstance( holeNodes[0], int ):
            holeNodes = SMESH.FreeBorder(nodeIDs=holeNodes)
        if not isinstance( holeNodes, SMESH.FreeBorder ):
            raise TypeError, "holeNodes must be either SMESH.FreeBorder or list of integer and not %s" % holeNodes
        self.editor.FillHole( holeNodes )

    def FindCoincidentFreeBorders (self, tolerance=0.):
        """
        Return groups of FreeBorder's coincident within the given tolerance.

	Parameters:
            tolerance: the tolerance. If the tolerance <= 0.0 then one tenth of an average
		size of elements adjacent to free borders being compared is used.

        Returns:
            SMESH.CoincidentFreeBorders structure
	"""

        return self.editor.FindCoincidentFreeBorders( tolerance )

    def SewCoincidentFreeBorders (self, freeBorders, createPolygons=False, createPolyhedra=False):
        """
        Sew FreeBorder's of each group

	Parameters:
            freeBorders: either a SMESH.CoincidentFreeBorders structure or a list of lists
		where each enclosed list contains node IDs of a group of coincident free
		borders such that each consequent triple of IDs within a group describes
		a free border in a usual way: n1, n2, nLast - i.e. 1st node, 2nd node and
		last node of a border.
		For example [[1, 2, 10, 20, 21, 40], [11, 12, 15, 55, 54, 41]] describes two
		groups of coincident free borders, each group including two borders.
            createPolygons: if :code:`True` faces adjacent to free borders are converted to
		polygons if a node of opposite border falls on a face edge, else such
		faces are split into several ones.
            createPolyhedra: if :code:`True` volumes adjacent to free borders are converted to
		polyhedra if a node of opposite border falls on a volume edge, else such
		volumes, if any, remain intact and the mesh becomes non-conformal.

        Returns:
            a number of successfully sewed groups
	"""

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

    def SewFreeBorders (self, FirstNodeID1, SecondNodeID1, LastNodeID1,
                        FirstNodeID2, SecondNodeID2, LastNodeID2,
                        CreatePolygons, CreatePolyedrs):
        """
        Sew free borders

        Returns:
            SMESH::Sew_Error
	"""

        return self.editor.SewFreeBorders(FirstNodeID1, SecondNodeID1, LastNodeID1,
                                          FirstNodeID2, SecondNodeID2, LastNodeID2,
                                          CreatePolygons, CreatePolyedrs)

    def SewConformFreeBorders (self, FirstNodeID1, SecondNodeID1, LastNodeID1,
                               FirstNodeID2, SecondNodeID2):
        """
        Sew conform free borders

        Returns:
            SMESH::Sew_Error
	"""

        return self.editor.SewConformFreeBorders(FirstNodeID1, SecondNodeID1, LastNodeID1,
                                                 FirstNodeID2, SecondNodeID2)

    def SewBorderToSide (self, FirstNodeIDOnFreeBorder, SecondNodeIDOnFreeBorder, LastNodeIDOnFreeBorder,
                         FirstNodeIDOnSide, LastNodeIDOnSide, CreatePolygons, CreatePolyedrs):
        """
        Sew border to side

        Returns:
            SMESH::Sew_Error
	"""

        return self.editor.SewBorderToSide(FirstNodeIDOnFreeBorder, SecondNodeIDOnFreeBorder, LastNodeIDOnFreeBorder,
                                           FirstNodeIDOnSide, LastNodeIDOnSide, CreatePolygons, CreatePolyedrs)

    def SewSideElements (self, IDsOfSide1Elements, IDsOfSide2Elements,
                         NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                         NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge):
        """
        Sew two sides of a mesh. The nodes belonging to Side1 are
		merged with the nodes of elements of Side2.
		The number of elements in theSide1 and in theSide2 must be
		equal and they should have similar nodal connectivity.
		The nodes to merge should belong to side borders and
		the first node should be linked to the second.

        Returns:
            SMESH::Sew_Error
	"""

        return self.editor.SewSideElements(IDsOfSide1Elements, IDsOfSide2Elements,
                                           NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                                           NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge)

    def ChangeElemNodes(self, ide, newIDs):
        """
        Set new nodes for the given element.

	Parameters:
            ide: the element id
            newIDs: nodes ids

        Returns:
            If the number of nodes does not correspond to the type of element - return false
	"""

        return self.editor.ChangeElemNodes(ide, newIDs)

    def GetLastCreatedNodes(self):
        """
        If during the last operation of MeshEditor some nodes were
		created, this method return the list of their IDs, \n
		if new nodes were not created - return empty list

        Returns:
            the list of integer values (can be empty)
	"""

        return self.editor.GetLastCreatedNodes()

    def GetLastCreatedElems(self):
        """
        If during the last operation of MeshEditor some elements were
		created this method return the list of their IDs, \n
		if new elements were not created - return empty list

        Returns:
            the list of integer values (can be empty)
	"""

        return self.editor.GetLastCreatedElems()

    def ClearLastCreated(self):
        """
        Forget what nodes and elements were created by the last mesh edition operation
	"""

        self.editor.ClearLastCreated()

    def DoubleElements(self, theElements, theGroupName=""):
        """
        Create duplicates of given elements, i.e. create new elements based on the
		same nodes as the given ones.

	Parameters:
            theElements: container of elements to duplicate. It can be a Mesh,
		sub-mesh, group, filter or a list of element IDs. If *theElements* is
		a Mesh, elements of highest dimension are duplicated
            theGroupName: a name of group to contain the generated elements.
		If a group with such a name already exists, the new elements
		are added to the existng group, else a new group is created.
		If *theGroupName* is empty, new elements are not added
		in any group.

	Returns:
		a group where the new elements are added. None if theGroupName == "".
	"""

        unRegister = genObjUnRegister()
        if isinstance( theElements, Mesh ):
            theElements = theElements.mesh
        elif isinstance( theElements, list ):
            theElements = self.GetIDSource( theElements, SMESH.ALL )
            unRegister.set( theElements )
        return self.editor.DoubleElements(theElements, theGroupName)

    def DoubleNodes(self, theNodes, theModifiedElems):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements

	Parameters:
            theNodes: identifiers of nodes to be doubled
            theModifiedElems: identifiers of elements to be updated by the new (doubled)
		nodes. If list of element identifiers is empty then nodes are doubled but
		they not assigned to elements

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.DoubleNodes(theNodes, theModifiedElems)

    def DoubleNode(self, theNodeId, theModifiedElems):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements
	This method provided for convenience works as DoubleNodes() described above.

	Parameters:
            theNodeId: identifiers of node to be doubled
            theModifiedElems: identifiers of elements to be updated

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.DoubleNode(theNodeId, theModifiedElems)

    def DoubleNodeGroup(self, theNodes, theModifiedElems, theMakeGroup=False):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements
	This method provided for convenience works as DoubleNodes() described above.

	Parameters:
            theNodes: group of nodes to be doubled
            theModifiedElems: group of elements to be updated.
            theMakeGroup: forces the generation of a group containing new nodes.

        Returns:
            TRUE or a created group if operation has been completed successfully,
		FALSE or None otherwise
	"""

        if theMakeGroup:
            return self.editor.DoubleNodeGroupNew(theNodes, theModifiedElems)
        return self.editor.DoubleNodeGroup(theNodes, theModifiedElems)

    def DoubleNodeGroups(self, theNodes, theModifiedElems, theMakeGroup=False):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements
	This method provided for convenience works as DoubleNodes() described above.

	Parameters:
            theNodes: list of groups of nodes to be doubled
            theModifiedElems: list of groups of elements to be updated.
            theMakeGroup: forces the generation of a group containing new nodes.

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        if theMakeGroup:
            return self.editor.DoubleNodeGroupsNew(theNodes, theModifiedElems)
        return self.editor.DoubleNodeGroups(theNodes, theModifiedElems)

    def DoubleNodeElem(self, theElems, theNodesNot, theAffectedElems):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements

	Parameters:
            theElems: the list of elements (edges or faces) to be replicated
		The nodes for duplication could be found from these elements
            theNodesNot: list of nodes to NOT replicate
            theAffectedElems: the list of elements (cells and edges) to which the
		replicated nodes should be associated to.

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.DoubleNodeElem(theElems, theNodesNot, theAffectedElems)

    def DoubleNodeElemInRegion(self, theElems, theNodesNot, theShape):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements

	Parameters:
            theElems: the list of elements (edges or faces) to be replicated
		The nodes for duplication could be found from these elements
            theNodesNot: list of nodes to NOT replicate
            theShape: shape to detect affected elements (element which geometric center
		located on or inside shape).
		The replicated nodes should be associated to affected elements.

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.DoubleNodeElemInRegion(theElems, theNodesNot, theShape)

    def DoubleNodeElemGroup(self, theElems, theNodesNot, theAffectedElems,
                             theMakeGroup=False, theMakeNodeGroup=False):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements
	This method provided for convenience works as DoubleNodes() described above.

	Parameters:
            theElems: group of of elements (edges or faces) to be replicated
            theNodesNot: group of nodes not to replicated
            theAffectedElems: group of elements to which the replicated nodes
		should be associated to.
            theMakeGroup: forces the generation of a group containing new elements.
            theMakeNodeGroup: forces the generation of a group containing new nodes.

        Returns:
            TRUE or created groups (one or two) if operation has been completed successfully,
		FALSE or None otherwise
	"""

        if theMakeGroup or theMakeNodeGroup:
            twoGroups = self.editor.DoubleNodeElemGroup2New(theElems, theNodesNot,
                                                            theAffectedElems,
                                                            theMakeGroup, theMakeNodeGroup)
            if theMakeGroup and theMakeNodeGroup:
                return twoGroups
            else:
                return twoGroups[ int(theMakeNodeGroup) ]
        return self.editor.DoubleNodeElemGroup(theElems, theNodesNot, theAffectedElems)

    def DoubleNodeElemGroupInRegion(self, theElems, theNodesNot, theShape):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements
	This method provided for convenience works as DoubleNodes() described above.

	Parameters:
            theElems: group of of elements (edges or faces) to be replicated
            theNodesNot: group of nodes not to replicated
            theShape: shape to detect affected elements (element which geometric center
		located on or inside shape).
		The replicated nodes should be associated to affected elements.
	"""

        return self.editor.DoubleNodeElemGroupInRegion(theElems, theNodesNot, theShape)

    def DoubleNodeElemGroups(self, theElems, theNodesNot, theAffectedElems,
                             theMakeGroup=False, theMakeNodeGroup=False):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements
	This method provided for convenience works as DoubleNodes() described above.

	Parameters:
            theElems: list of groups of elements (edges or faces) to be replicated
            theNodesNot: list of groups of nodes not to replicated
            theAffectedElems: group of elements to which the replicated nodes
		should be associated to.
            theMakeGroup: forces the generation of a group containing new elements.
            theMakeNodeGroup: forces the generation of a group containing new nodes.

        Returns:
            TRUE or created groups (one or two) if operation has been completed successfully,
		FALSE or None otherwise
	"""

        if theMakeGroup or theMakeNodeGroup:
            twoGroups = self.editor.DoubleNodeElemGroups2New(theElems, theNodesNot,
                                                             theAffectedElems,
                                                             theMakeGroup, theMakeNodeGroup)
            if theMakeGroup and theMakeNodeGroup:
                return twoGroups
            else:
                return twoGroups[ int(theMakeNodeGroup) ]
        return self.editor.DoubleNodeElemGroups(theElems, theNodesNot, theAffectedElems)

    def DoubleNodeElemGroupsInRegion(self, theElems, theNodesNot, theShape):
        """
        Create a hole in a mesh by doubling the nodes of some particular elements
	This method provided for convenience works as DoubleNodes() described above.

	Parameters:
            theElems: list of groups of elements (edges or faces) to be replicated
            theNodesNot: list of groups of nodes not to replicated
            theShape: shape to detect affected elements (element which geometric center
		located on or inside shape).
		The replicated nodes should be associated to affected elements.

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.DoubleNodeElemGroupsInRegion(theElems, theNodesNot, theShape)

    def AffectedElemGroupsInRegion(self, theElems, theNodesNot, theShape):
        """
        Identify the elements that will be affected by node duplication (actual duplication is not performed.
	This method is the first step of DoubleNodeElemGroupsInRegion.

	Parameters:
            theElems: list of groups of nodes or elements (edges or faces) to be replicated
            theNodesNot: list of groups of nodes not to replicated
            theShape: shape to detect affected elements (element which geometric center
		located on or inside shape).
		The replicated nodes should be associated to affected elements.

        Returns:
            groups of affected elements in order:: volumes, faces, edges
	"""

        return self.editor.AffectedElemGroupsInRegion(theElems, theNodesNot, theShape)

    def DoubleNodesOnGroupBoundaries(self, theDomains, createJointElems, onAllBoundaries=False ):
        """
        Double nodes on shared faces between groups of volumes and create flat elements on demand.
	The list of groups must describe a partition of the mesh volumes.
	The nodes of the internal faces at the boundaries of the groups are doubled.
	In option, the internal faces are replaced by flat elements.
	Triangles are transformed in prisms, and quadrangles in hexahedrons.

	Parameters:
            theDomains: list of groups of volumes
            createJointElems: if TRUE, create the elements
            onAllBoundaries: if TRUE, the nodes and elements are also created on
		the boundary between *theDomains* and the rest mesh

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.DoubleNodesOnGroupBoundaries( theDomains, createJointElems, onAllBoundaries )

    def CreateFlatElementsOnFacesGroups(self, theGroupsOfFaces ):
        """
        Double nodes on some external faces and create flat elements.
	Flat elements are mainly used by some types of mechanic calculations.

	Each group of the list must be constituted of faces.
	Triangles are transformed in prisms, and quadrangles in hexahedrons.

	Parameters:
            theGroupsOfFaces: list of groups of faces

        Returns:
            TRUE if operation has been completed successfully, FALSE otherwise
	"""

        return self.editor.CreateFlatElementsOnFacesGroups( theGroupsOfFaces )

    def CreateHoleSkin(self, radius, theShape, groupName, theNodesCoords):
        """
        identify all the elements around a geom shape, get the faces delimiting the hole
	"""
        return self.editor.CreateHoleSkin( radius, theShape, groupName, theNodesCoords )

    def MakePolyLine(self, segments, groupName='', isPreview=False ):
        """    
        Create a polyline consisting of 1D mesh elements each lying on a 2D element of
        the initial mesh. Positions of new nodes are found by cutting the mesh by the
        plane passing through pairs of points specified by each PolySegment structure.
        If there are several paths connecting a pair of points, the shortest path is
        selected by the module. Position of the cutting plane is defined by the two
        points and an optional vector lying on the plane specified by a PolySegment.
        By default the vector is defined by Mesh module as following. A middle point
        of the two given points is computed. The middle point is projected to the mesh.
        The vector goes from the middle point to the projection point. In case of planar
        mesh, the vector is normal to the mesh.

	Parameters:        
            segments - PolySegment's defining positions of cutting planes.
            groupName - optional name of a group where created mesh segments will
            be added.
            
        Returns:    
            The used vector which goes from the middle point to its projection.
	"""    
        editor = self.editor
        if isPreview:
            editor = self.mesh.GetMeshEditPreviewer()
        segmentsRes = editor.MakePolyLine( segments, groupName )
        for i, seg in enumerate( segmentsRes ):
            segments[i].vector = seg.vector
        if isPreview:
            return editor.GetPreviewData()
        return None        

    def GetFunctor(self, funcType ):
        """
        Return a cached numerical functor by its type.

	Parameters:
            theCriterion functor type: an item of SMESH.FunctorType enumeration.
		Type SMESH.FunctorType._items in the Python Console to see all items.
		Note that not all items correspond to numerical functors.

        Returns:
            SMESH_NumericalFunctor. The functor is already initialized
		with a mesh
	"""

        fn = self.functors[ funcType._v ]
        if not fn:
            fn = self.smeshpyD.GetFunctor(funcType)
            fn.SetMesh(self.mesh)
            self.functors[ funcType._v ] = fn
        return fn

    def FunctorValue(self, funcType, elemId, isElem=True):
        """
        Return value of a functor for a given element

	Parameters:
            funcType: an item of SMESH.FunctorType enum
		Type "SMESH.FunctorType._items" in the Python Console to see all items.
            elemId: element or node ID
            isElem: *elemId* is ID of element or node

        Returns:
            the functor value or zero in case of invalid arguments
	"""

        fn = self.GetFunctor( funcType )
        if fn.GetElementType() == self.GetElementType(elemId, isElem):
            val = fn.GetValue(elemId)
        else:
            val = 0
        return val

    def GetLength(self, elemId=None):
        """
        Get length of 1D element or sum of lengths of all 1D mesh elements

        Parameters:
            elemId mesh element ID (if not defined - sum of length of all 1D elements will be calculated)

        Returns:
            element's length value if *elemId* is specified or sum of all 1D mesh elements' lengths otherwise
	"""

        length = 0
        if elemId == None:
            length = self.smeshpyD.GetLength(self)
        else:
            length = self.FunctorValue(SMESH.FT_Length, elemId)
        return length

    def GetArea(self, elemId=None):
        """
        Get area of 2D element or sum of areas of all 2D mesh elements
            elemId mesh element ID (if not defined - sum of areas of all 2D elements will be calculated)

        Returns:
            element's area value if *elemId* is specified or sum of all 2D mesh elements' areas otherwise
	"""

        area = 0
        if elemId == None:
            area = self.smeshpyD.GetArea(self)
        else:
            area = self.FunctorValue(SMESH.FT_Area, elemId)
        return area

    def GetVolume(self, elemId=None):
        """
        Get volume of 3D element or sum of volumes of all 3D mesh elements
            elemId mesh element ID (if not defined - sum of volumes of all 3D elements will be calculated)

        Returns:
            element's volume value if *elemId* is specified or sum of all 3D mesh elements' volumes otherwise
	"""

        volume = 0
        if elemId == None:
            volume = self.smeshpyD.GetVolume(self)
        else:
            volume = self.FunctorValue(SMESH.FT_Volume3D, elemId)
        return volume

    def GetMaxElementLength(self, elemId):
        """
        Get maximum element length.

        Parameters:
            elemId mesh element ID

        Returns:
            element's maximum length value
	"""

        if self.GetElementType(elemId, True) == SMESH.VOLUME:
            ftype = SMESH.FT_MaxElementLength3D
        else:
            ftype = SMESH.FT_MaxElementLength2D
        return self.FunctorValue(ftype, elemId)

    def GetAspectRatio(self, elemId):
        """
        Get aspect ratio of 2D or 3D element.

        Parameters:
            elemId mesh element ID

        Returns:
            element's aspect ratio value
	"""

        if self.GetElementType(elemId, True) == SMESH.VOLUME:
            ftype = SMESH.FT_AspectRatio3D
        else:
            ftype = SMESH.FT_AspectRatio
        return self.FunctorValue(ftype, elemId)

    def GetWarping(self, elemId):
        """
        Get warping angle of 2D element.

        Parameters:
            elemId mesh element ID

        Returns:
            element's warping angle value
	"""

        return self.FunctorValue(SMESH.FT_Warping, elemId)

    def GetMinimumAngle(self, elemId):
        """
        Get minimum angle of 2D element.

        Parameters:
            elemId mesh element ID

        Returns:
            element's minimum angle value
	"""

        return self.FunctorValue(SMESH.FT_MinimumAngle, elemId)

    def GetTaper(self, elemId):
        """
        Get taper of 2D element.

        Parameters:
            elemId mesh element ID

        Returns:
            element's taper value
	"""

        return self.FunctorValue(SMESH.FT_Taper, elemId)

    def GetSkew(self, elemId):
        """
        Get skew of 2D element.

        Parameters:
            elemId mesh element ID

        Returns:
            element's skew value
	"""

        return self.FunctorValue(SMESH.FT_Skew, elemId)

    def GetMinMax(self, funType, meshPart=None):
        """
        Return minimal and maximal value of a given functor.

	Parameters:
            funType a functor type, an item of SMESH.FunctorType enum
		(one of SMESH.FunctorType._items)
            meshPart a part of mesh (group, sub-mesh) to treat

        Returns:
            tuple (min,max)
	"""

        unRegister = genObjUnRegister()
        if isinstance( meshPart, list ):
            meshPart = self.GetIDSource( meshPart, SMESH.ALL )
            unRegister.set( meshPart )
        if isinstance( meshPart, Mesh ):
            meshPart = meshPart.mesh
        fun = self.GetFunctor( funType )
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


class meshProxy(SMESH._objref_SMESH_Mesh):
    """
    Private class used to compensate change of CORBA API of SMESH_Mesh for backward compatibility
    with old dump scripts which call SMESH_Mesh directly and not via smeshBuilder.Mesh
    """
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


class submeshProxy(SMESH._objref_SMESH_subMesh):
    """
    Private class wrapping SMESH.SMESH_SubMesh in order to add Compute()
    """
    def __init__(self):
        SMESH._objref_SMESH_subMesh.__init__(self)
        self.mesh = None
    def __deepcopy__(self, memo=None):
        new = self.__class__()
        return new

    def Compute(self,refresh=False):
        """
        Compute the sub-mesh and return the status of the computation
            refresh if *True*, Object browser is automatically updated (when running in GUI)

        Returns:
            True or False
	This is a method of SMESH.SMESH_submesh that can be obtained via Mesh.GetSubMesh() or
	:meth:`smeshBuilder.Mesh.GetSubMesh`.
	"""

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


class meshEditor(SMESH._objref_SMESH_MeshEditor):
    """
    Private class used to compensate change of CORBA API of SMESH_MeshEditor for backward
    compatibility with old dump scripts which call SMESH_MeshEditor directly and not via
    smeshBuilder.Mesh
    """
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
    def MergeNodes(self,*args): # 2 args added (NodesToKeep,AvoidMakingHoles)
        if len( args ) == 1:
            return SMESH._objref_SMESH_MeshEditor.MergeNodes( self, args[0], [], False )
        NodesToKeep = args[1]
        AvoidMakingHoles = args[2] if len( args ) == 3 else False
        unRegister  = genObjUnRegister()
        if NodesToKeep:
            if isinstance( NodesToKeep, list ) and isinstance( NodesToKeep[0], int ):
                NodesToKeep = self.MakeIDSource( NodesToKeep, SMESH.NODE )
            if not isinstance( NodesToKeep, list ):
                NodesToKeep = [ NodesToKeep ]
        return SMESH._objref_SMESH_MeshEditor.MergeNodes( self, args[0], NodesToKeep, AvoidMakingHoles )
    pass
omniORB.registerObjref(SMESH._objref_SMESH_MeshEditor._NP_RepositoryId, meshEditor)

class Pattern(SMESH._objref_SMESH_Pattern):
    """
    Private class wrapping SMESH.SMESH_Pattern CORBA class in order to treat Notebook
    variables in some methods
    """

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

omniORB.registerObjref(SMESH._objref_SMESH_Pattern._NP_RepositoryId, Pattern)
"""
Registering the new proxy for Pattern
"""

class algoCreator:
    """
    Private class used to bind methods creating algorithms to the class Mesh
    """

    def __init__(self, method):
        self.mesh = None
        self.defaultAlgoType = ""
        self.algoTypeToClass = {}
        self.method = method

    def add(self, algoClass):
        """
	Store a python class of algorithm
	"""
        if type( algoClass ).__name__ == 'classobj' and \
           hasattr( algoClass, "algoType"):
            self.algoTypeToClass[ algoClass.algoType ] = algoClass
            if not self.defaultAlgoType and \
               hasattr( algoClass, "isDefault") and algoClass.isDefault:
                self.defaultAlgoType = algoClass.algoType
            #print "Add",algoClass.algoType, "dflt",self.defaultAlgoType

    def copy(self, mesh):
	"""
	Create a copy of self and assign mesh to the copy
	"""

        other = algoCreator( self.method )
        other.defaultAlgoType = self.defaultAlgoType
        other.algoTypeToClass = self.algoTypeToClass
        other.mesh = mesh
        return other

    def __call__(self,algo="",geom=0,*args):
	"""
	Create an instance of algorithm
	"""
        algoType = ""
        shape = 0
        if isinstance( algo, str ):
            algoType = algo
        elif ( isinstance( algo, geomBuilder.GEOM._objref_GEOM_Object ) and \
               not isinstance( geom, geomBuilder.GEOM._objref_GEOM_Object )):
            shape = algo
        elif algo:
            args += (algo,)

        if isinstance( geom, geomBuilder.GEOM._objref_GEOM_Object ):
            shape = geom
        elif not algoType and isinstance( geom, str ):
            algoType = geom
        elif geom:
            args += (geom,)
        for arg in args:
            if isinstance( arg, geomBuilder.GEOM._objref_GEOM_Object ) and not shape:
                shape = arg
            elif isinstance( arg, str ) and not algoType:
                algoType = arg
            else:
                import traceback, sys
                msg = "Warning. Unexpected argument in mesh.%s() --->  %s" % ( self.method, arg )
                sys.stderr.write( msg + '\n' )
                tb = traceback.extract_stack(None,2)
                traceback.print_list( [tb[0]] )
        if not algoType:
            algoType = self.defaultAlgoType
        if not algoType and self.algoTypeToClass:
            algoType = sorted( self.algoTypeToClass.keys() )[0]
        if self.algoTypeToClass.has_key( algoType ):
            #print "Create algo",algoType
            return self.algoTypeToClass[ algoType ]( self.mesh, shape )
        raise RuntimeError, "No class found for algo type %s" % algoType
        return None

class hypMethodWrapper:
    """
    Private class used to substitute and store variable parameters of hypotheses.
    """

    def __init__(self, hyp, method):
        self.hyp    = hyp
        self.method = method
        #print "REBIND:", method.__name__
        return

    def __call__(self,*args):
	"""
	call a method of hypothesis with calling SetVarParameter() before
	"""

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

class genObjUnRegister:
    """
    A helper class that calls UnRegister() of SALOME.GenericObj'es stored in it
    """

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

for pluginName in os.environ[ "SMESH_MeshersList" ].split( ":" ):
    """
    Bind methods creating mesher plug-ins to the Mesh class
    """

    # print "pluginName: ", pluginName
    pluginBuilderName = pluginName + "Builder"
    try:
        exec( "from salome.%s.%s import *" % (pluginName, pluginBuilderName))
    except Exception, e:
	from salome_utils import verbose
	if verbose(): print "Exception while loading %s: %s" % ( pluginBuilderName, e )
        continue
    exec( "from salome.%s import %s" % (pluginName, pluginBuilderName))
    plugin = eval( pluginBuilderName )
    # print "  plugin:" , str(plugin)

    # add methods creating algorithms to Mesh
    for k in dir( plugin ):
        if k[0] == '_': continue
        algo = getattr( plugin, k )
        # print "             algo:", str(algo)
        if type( algo ).__name__ == 'classobj' and hasattr( algo, "meshMethod" ):
            # print "                     meshMethod:" , str(algo.meshMethod)
            if not hasattr( Mesh, algo.meshMethod ):
                setattr( Mesh, algo.meshMethod, algoCreator( algo.meshMethod ))
                pass
            _mmethod = getattr( Mesh, algo.meshMethod )
            if hasattr(  _mmethod, "add" ):
                _mmethod.add(algo)
            pass
        pass
    pass
del pluginName
