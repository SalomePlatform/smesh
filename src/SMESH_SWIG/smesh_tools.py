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

def create_dual_mesh(mesh_ior, output_file, adapt_to_shape=True, mesh_name="MESH"):
    """ Create a dual of the mesh in input_file into output_file

    Args:
        mesh_ior (string): corba Id of the Tetrahedron mesh
        output_file (string): dual mesh file
    """
    # Import mesh from file
    # mesh = salome.orb.string_to_object(salome.salome_study.myStudy.FindObjectID(mesh_id).GetIOR())
    mesh = salome.orb.string_to_object(mesh_ior)
    if not mesh:
        raise Exception("Could not find mesh using id: ", mesh_ior)

    shape = mesh.GetShapeToMesh()

    # We got a meshProxy so we need to convert pointer to MEDCoupling
    int_ptr = mesh.ExportMEDCoupling(True, True)
    dab = mc.FromPyIntPtrToDataArrayByte(int_ptr)
    tetras =  mc.MEDFileMesh.New(dab)[0]
    # End of SMESH -> MEDCoupling part for dualmesh

    tetras = mc.MEDCoupling1SGTUMesh(tetras)
    polyh = tetras.computeDualMesh()
    skin = tetras.buildUnstructured().computeSkin()
    skin_polyh = polyh.buildUnstructured().computeSkin()
    allNodesOnSkinPolyh = skin_polyh.computeFetchedNodeIds()
    allNodesOnSkin = skin.computeFetchedNodeIds()
    ptsAdded = allNodesOnSkinPolyh.buildSubstraction(allNodesOnSkin)
    ptsAddedMesh = mc.MEDCouplingUMesh.Build0DMeshFromCoords( skin_polyh.getCoords()[ptsAdded] )

    if adapt_to_shape:
        ptsAddedCoo = ptsAddedMesh.getCoords()
        ptsAddedCooModified = ptsAddedCoo[:]

        # We need the geometry for that
        # TODO : Loop on faces identify points associated to which face
        faces = geompy.ExtractShapes(shape, geompy.ShapeType["FACE"], True)
        #assert( len(faces) == 1 )
        ## projection des points ajoutés par le dual sur la surface
        #for i,tup in enumerate(ptsAddedCooModified):
        #    vertex = geompy.MakeVertex(*tuple(tup))
        #    prj = geompy.MakeProjection(vertex, faces)
        #    newCoor = geompy.PointCoordinates( prj )
        #    ptsAddedCooModified[i] = newCoor
        ## assign coordinates with projected ones
        #polyh.getCoords()[ptsAdded] = ptsAddedCooModified

    print("Writing dual mesh in ", output_file)
    polyh.setName(mesh_name)
    polyh.write(output_file)





