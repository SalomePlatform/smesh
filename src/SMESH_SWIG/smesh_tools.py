#!/usr/bin/env python3

import sys
import salome
import medcoupling as mc
from math import pi

#salome.salome_init()

import GEOM
from salome.geom import geomBuilder

geompy = geomBuilder.New()

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

def smesh_create_dual_mesh(mesh_ior, output_file, adapt_to_shape=True, mesh_name="MESH"):
    """ Create a dual of the mesh in input_file into output_file

    Args:
        mesh_ior (string): corba Id of the Tetrahedron mesh
        output_file (string): dual mesh file
    """
    # Import mesh from file
    mesh = salome.orb.string_to_object(mesh_ior)
    if not mesh:
        raise Exception("Could not find mesh using id: ", mesh_ior)

    shape = mesh.GetShapeToMesh()

    # We got a meshProxy so we need to convert pointer to MEDCoupling
    int_ptr = mesh.ExportMEDCoupling(True, True)
    dab = mc.FromPyIntPtrToDataArrayByte(int_ptr)
    tetras = mc.MEDFileMesh.New(dab)[0]
    # End of SMESH -> MEDCoupling part for dualmesh

    tetras = mc.MEDCoupling1SGTUMesh(tetras)
    polyh = tetras.computeDualMesh()
    dual_volume_raw = polyh.getMeasureField(True).accumulate()[0]

    # Getting list of new points added on the skin
    skin = tetras.buildUnstructured().computeSkin()
    skin_polyh = polyh.buildUnstructured().computeSkin()
    allNodesOnSkinPolyh = skin_polyh.computeFetchedNodeIds()
    allNodesOnSkin = skin.computeFetchedNodeIds()
    ptsAdded = allNodesOnSkinPolyh.buildSubstraction(allNodesOnSkin)
    ptsAddedMesh = mc.MEDCouplingUMesh.Build0DMeshFromCoords( skin_polyh.getCoords()[ptsAdded] )

    if adapt_to_shape:
        ptsAddedCoo = ptsAddedMesh.getCoords()
        ptsAddedCooModified = ptsAddedCoo[:]

        # Matching faces with their ids
        faces = geompy.ExtractShapes(shape, geompy.ShapeType["FACE"], True)
        id2face = {}
        for face in faces:
            id2face[face.GetSubShapeIndices()[0]] = face
        print(id2face)

        ## Projecting each points added by the dual mesh on the surface it is
        # associated with
        for i, tup in enumerate(ptsAddedCooModified):
            vertex = geompy.MakeVertex(*tuple(tup))
            shapes = geompy.GetShapesNearPoint(shape, vertex,
                                               geompy.ShapeType["FACE"])
            prj = geompy.MakeProjection(vertex,
                                        id2face[shapes.GetSubShapeIndices()[0]])
            new_coor = geompy.PointCoordinates(prj)
            ptsAddedCooModified[i] = new_coor

        polyh.getCoords()[ptsAdded] = ptsAddedCooModified

    polyh.setName(mesh_name)
    polyh.write(output_file)
