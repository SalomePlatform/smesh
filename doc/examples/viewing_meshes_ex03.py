#!/usr/bin/env python

###
### This file shows how to get the information that are displayed when using Mesh Information for elements/nodes
###

import sys
import salome

salome.salome_init()
import salome_notebook
notebook = salome_notebook.NoteBook()
sys.path.insert(0, r'/local00/home/B61570/work_in_progress/mesh_info')

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS


geompy = geomBuilder.New()

Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( Box_1, 'Box_1' )
bottom = geompy.CreateGroup(Box_1, geompy.ShapeType["FACE"])
geompy.UnionIDs(bottom, [31])

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()


## Tetra
NETGEN_3D_Parameters_1 = smesh.CreateHypothesisByAverageLength( 'NETGEN_Parameters', 'NETGENEngine', 25, 0 )
Mesh_tetra = smesh.Mesh(Box_1,'Mesh_tetra')
status = Mesh_tetra.AddHypothesis( Box_1, NETGEN_3D_Parameters_1 )
NETGEN_1D_2D_3D = Mesh_tetra.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
isDone = Mesh_tetra.Compute()
if not isDone:
    raise ("Could not compute mesh: "+Mesh_tetra.GetName())

## Tetra
Mesh_quadratic = smesh.Mesh(Box_1,'Mesh_quadratic')
NETGEN_1D_2D_3D_1 = Mesh_quadratic.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
NETGEN_3D_Parameters_2 = NETGEN_1D_2D_3D_1.Parameters()
NETGEN_3D_Parameters_2.SetMaxSize( 34.641 )
NETGEN_3D_Parameters_2.SetMinSize( 0.34641 )
NETGEN_3D_Parameters_2.SetSecondOrder( 1 )
isDone = Mesh_quadratic.Compute()
if not isDone:
    raise ("Could not compute mesh: "+Mesh_quadratic.GetName())


# Hexa mesh
Mesh_hexa = smesh.Mesh(Box_1,'Mesh_hexa')
Regular_1D = Mesh_hexa.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(15)
Quadrangle_2D = Mesh_hexa.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = Mesh_hexa.Hexahedron(algo=smeshBuilder.Hexa)
isDone = Mesh_hexa.Compute()
if not isDone:
    raise ("Could not compute mesh: "+Mesh_hexa.GetName())

# Poly Mesh
Mesh_poly = smesh.CreateDualMesh(Mesh_tetra, 'dual_Mesh_1', True)

# Prism mesh

Mesh_prism = smesh.Mesh(Box_1,'Mesh_prism')
Regular_1D_1 = Mesh_prism.Segment()
Number_of_Segments_2 = Regular_1D_1.NumberOfSegments(15)
NETGEN_1D_2D = Mesh_prism.Triangle(algo=smeshBuilder.NETGEN_1D2D,geom=bottom)
NETGEN_2D_Parameters_1 = NETGEN_1D_2D.Parameters()
NETGEN_2D_Parameters_1.SetMaxSize(35)
NETGEN_2D_Parameters_1.SetMinSize(0.3)
Prism_3D = Mesh_prism.Prism()
isDone = Mesh_prism.Compute()
if not isDone:
    raise ("Could not compute mesh: "+Mesh_prism.GetName())

# Pyramid mesh
Mesh_pyramids = smesh.Mesh(Box_1,'Mesh_pyramids')
Regular_1D_2 = Mesh_pyramids.Segment()
Number_of_Segments_3 = Regular_1D_2.NumberOfSegments(15)
Quadrangle_2D_1 = Mesh_pyramids.Quadrangle(algo=smeshBuilder.QUADRANGLE)
NETGEN_3D = Mesh_pyramids.Tetrahedron()
bottom_1 = Mesh_pyramids.GroupOnGeom(bottom,'bottom',SMESH.FACE)
isDone = Mesh_pyramids.Compute()
if not isDone:
    raise ("Could not compute mesh: "+Mesh_pyramids.GetName())


## Set names of Mesh objects
smesh.SetName(NETGEN_1D_2D_3D.GetAlgorithm(), 'NETGEN 1D-2D-3D')
smesh.SetName(NETGEN_3D_Parameters_1, 'NETGEN 3D Parameters_1')
smesh.SetName(Mesh_tetra.GetMesh(), 'Mesh_tetra')
smesh.SetName(Mesh_hexa.GetMesh(), 'Mesh_hexa')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()

# Look in SMESH_GUI/SMESHGUI_MeshInfo.cxx +1666 for list of what is

def face_info(mesh, elem_id):
    """
    Print equivalent of Mesh Information for a face
    """
    elem_type = mesh.GetElementGeomType(elem_id)

    conn = mesh.GetElemNodes(elem_id)

    nb_nodes = len(conn)

    position = mesh.GetElementPosition(elem_id)
    pos = f"{position.shapeType} #{position.shapeID}"

    grav_center = mesh.BaryCenter(elem_id)

    normal = mesh.GetFaceNormal(elem_id, normalized=True)

    aspect_ratio = mesh.GetAspectRatio(elem_id)
    #aspect_ratio = mesh.FunctorValue(SMESH.FT_AspectRatio, elem_id, isElem=True)

    warping = mesh.GetWarping(elem_id)
    #warping = mesh.FunctorValue(SMESH.FT_Warping, elem_id, isElem=True)

    min_angle = mesh.GetMinimumAngle(elem_id)
    #min_angle = mesh.FunctorValue(SMESH.FT_MinimumAngle, elem_id, isElem=True)

    taper = mesh.GetTaper(elem_id)
    #taper = mesh.FunctorValue(SMESH.FT_Taper, elem_id, isElem=True)

    skew = mesh.GetSkew(elem_id)
    #skew = mesh.FunctorValue(SMESH.FT_Skew, elem_id, isElem=True)

    area = mesh.GetArea(elem_id)
    #area = mesh.FunctorValue(SMESH.FT_Area, elem_id, isElem=True)

    diameter = mesh.GetMaxElementLength(elem_id)
    #diameter = mesh.FunctorValue(SMESH.FT_MaxElementLength2D, elem_id, isElem=True)

    min_length = mesh.FunctorValue(SMESH.FT_Length2D, elem_id, isElem=True)

    string = f"""
Id: {elem_id}
Type: {elem_type}
Nb Nodes: {nb_nodes}
Connectivity: {conn}
Position: {pos}
Gravity center:
 - X: {grav_center[0]}
 - Y: {grav_center[1]}
 - Z: {grav_center[2]}
Normal:
 - X: {normal[0]}
 - Y: {normal[1]}
 - Z: {normal[2]}
Quality:
 - Aspect Ratio: {aspect_ratio}
 - Warping: {warping}
 - Minimum Angle: {min_angle}
 - Taper: {taper}
 - Skew: {skew}
 - Area: {area}
 - Element Diameter 2D: {diameter}
 - Minimum Edge Length: {min_length}
"""
    print(string)

def volume_info(mesh, elem_id):
    """
    Print equivalent of Mesh Information for a volume
    """
    elem_type = mesh.GetElementGeomType(elem_id)

    if elem_type in [SMESH.Entity_Polyhedra, SMESH.Entity_Quad_Polyhedra]:
        iface = 1
        face_conn = [12]
        conn = []
        while face_conn != []:
            face_conn = mesh.GetElemFaceNodes(elem_id, iface)
            iface += 1
            conn.append(face_conn)
            nb_nodes = len(mesh.GetElemNodes(elem_id))
    else:
        conn = mesh.GetElemNodes(elem_id)
        nb_nodes = len(conn)


    position = mesh.GetElementPosition(elem_id)
    pos = f"{position.shapeType} #{position.shapeID}"

    grav_center = mesh.BaryCenter(elem_id)

    aspect_ratio = mesh.GetAspectRatio(elem_id)
    #aspect_ratio = mesh.FunctorValue(SMESH.FT_AspectRatio3D, elem_id, isElem=True)

    volume = mesh.GetVolume(elem_id)
    #volume = mesh.FunctorValue(SMESH.FT_Volume3D, elem_id, isElem=True)

    jacob = mesh.GetScaledJacobian(elem_id)
    #jacob = mesh.FunctorValue(SMESH.FT_ScaledJacobian, elem_id, isElem=True)

    diameter = mesh.GetMaxElementLength(elem_id)
    #diameter = mesh.FunctorValue(SMESH.FT_MaxElementLength3D, elem_id, isElem=True)

    min_length = mesh.FunctorValue(SMESH.FT_Length3D, elem_id, isElem=True)

    string = f"""
Id: {elem_id}
Type: {elem_type}
Nb Nodes: {nb_nodes}
Connectivity: {conn}
Position: {pos}
Gravity center:
 - X: {grav_center[0]}
 - Y: {grav_center[1]}
 - Z: {grav_center[2]}
Quality:
 - Aspect Ratio 3D: {aspect_ratio}
 - Volume: {volume}
 - Scaled Jacobian: {jacob}
 - Element Diameter 3D: {diameter}
 - Minimum Edge Length: {min_length}
"""
    print(string)

def node_info(mesh, node_id):

    coord = mesh.GetNodeXYZ(node_id)

    conn_edge = mesh.GetNodeInverseElements(node_id, SMESH.EDGE)
    conn_face = mesh.GetNodeInverseElements(node_id, SMESH.FACE)
    conn_vol = mesh.GetNodeInverseElements(node_id, SMESH.VOLUME)

    position = mesh.GetNodePosition(node_id)
    pos = f"{position.shapeType} #{position.shapeID}"

    vec = [None, None]
    vec[0:len(position.params)] = position.params

    string = f"""
Id: {node_id}
Coordinates:
- X: {coord[0]}
- Y: {coord[1]}
- Z: {coord[2]}
Connectivity
- Edges: {conn_edge}
- Faces: {conn_face}
- Volumes: {conn_vol}
Position: {pos}
- U: {vec[0]}
- V: {vec[1]}
"""
    print(string)

###
# Volume
##

# Tetrahedron
volume_info(Mesh_tetra, 3000)
# Hexahedron
volume_info(Mesh_hexa, 3000)
# Polyhedron
volume_info(Mesh_poly, 3000)
# Prism
volume_info(Mesh_prism, 1400)
# Pyramids
volume_info(Mesh_pyramids, 8176)
# Quadratic tetra
volume_info(Mesh_quadratic, 1180)

###
# Face
##

## Triangle
face_info(Mesh_tetra, 147)
#Quadrangle
face_info(Mesh_hexa, 1464)
# Polygon
face_info(Mesh_poly, 771)
# Quadratic triangle
face_info(Mesh_quadratic, 138)


###
# Node
###
# U & V
node_info(Mesh_tetra, 152)
# U
node_info(Mesh_tetra, 32)
# None
node_info(Mesh_tetra, 2)
