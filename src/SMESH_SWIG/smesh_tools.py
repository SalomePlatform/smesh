#!/usr/bin/env python3

import sys
import salome
import medcoupling as mc
from math import pi
import numpy as np

#salome.salome_init()

import GEOM
from salome.geom import geomBuilder

geompy = geomBuilder.New()

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

from salome.kernel.logger import Logger
logger = Logger("salome.smesh.smesh_tools")
logger.setLevel("DEBUG")

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

    # Creating output file
    logger.debug("Creating file with mesh: "+mesh_name)
    myfile = mc.MEDFileUMesh()
    myfile.setName(mesh_name)


    # We got a meshProxy so we need to convert pointer to MEDCoupling
    int_ptr = mesh.ExportMEDCoupling(True, True)
    dab = mc.FromPyIntPtrToDataArrayByte(int_ptr)
    mc_mesh_file = mc.MEDFileMesh.New(dab)
    tetras = mc_mesh_file[0]
    # End of SMESH -> MEDCoupling part for dualmesh

    tetras = mc.MEDCoupling1SGTUMesh(tetras)
    polyh = tetras.computeDualMesh()

    ## Adding skin + transfering groups on faces from tetras mesh
    mesh2d = polyh.buildUnstructured().computeSkin()
    mesh2d.setName(mesh_name)
    myfile.setMeshAtLevel(-1, mesh2d)


    for grp_name in mc_mesh_file.getGroupsOnSpecifiedLev(-1):
        # This group is created by the export
        if grp_name == "Group_Of_All_Faces":
            logger.debug("Skipping group: "+ grp_name)
            continue
        logger.debug("Transferring group: "+ grp_name)

        grp_tria = mc_mesh_file.getGroup(-1, grp_name)
        # Retrieve the nodes in group
        grp_nodes = grp_tria.computeFetchedNodeIds()
        # Find all the cells lying on one of the nodes
        id_grp_poly = mesh2d.getCellIdsLyingOnNodes(grp_nodes, False)

        grp_poly = mesh2d[id_grp_poly]

        # find the extra face cells, on the border of the group (lying on nodes, but outside the group)
        id_poly_border = grp_poly.findCellIdsOnBoundary()

        # ids of the cells in grp_poly
        id_poly=mc.DataArrayInt64.New(grp_poly.getNumberOfCells(), 1)
        id_poly.iota()

        # cells that are really in the group
        id_to_keep = id_poly.buildSubstraction(id_poly_border)

        id_grp_poly = id_grp_poly[id_to_keep]
        id_grp_poly.setName(grp_name.strip())

        myfile.addGroup(-1, id_grp_poly)

    # Getting list of new points added on the skin
    skin = tetras.buildUnstructured().computeSkin()
    skin_polyh = polyh.buildUnstructured().computeSkin()
    allNodesOnSkinPolyh = skin_polyh.computeFetchedNodeIds()
    allNodesOnSkin = skin.computeFetchedNodeIds()
    ptsAdded = allNodesOnSkinPolyh.buildSubstraction(allNodesOnSkin)
    ptsAddedMesh = mc.MEDCouplingUMesh.Build0DMeshFromCoords( skin_polyh.getCoords()[ptsAdded] )

    if adapt_to_shape:
        logger.debug("Adapting to shape")
        ptsAddedCoo = ptsAddedMesh.getCoords()
        ptsAddedCooModified = ptsAddedCoo[:]

        # Matching faces with their ids
        faces = geompy.ExtractShapes(shape, geompy.ShapeType["FACE"], True)
        id2face = {}
        for face in faces:
            id2face[face.GetSubShapeIndices()[0]] = face

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
    myfile.setMeshAtLevel(0, polyh)

    logger.debug("Writting dual mesh in :"+output_file)
    myfile.write(output_file, 2)
