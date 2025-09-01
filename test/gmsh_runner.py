#!/usr/bin/env python
import sys
from salome.kernel import salome

salome.salome_init()

import time
from os import path
import tempfile
import subprocess

from salome.kernel import GEOM
from salome.kernel import SMESH
from salome.kernel import SALOMEDS

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

import math

smesh = smeshBuilder.New()
geompy = geomBuilder.New()

import medcoupling as mc

def create_gmsh_empty_param_file(param_file):
    """ Create a parameter file for runner """
    param = """0"""
    with open(param_file, "w") as ffile:
        ffile.write(param)

def create_gmsh_param_file(param_file):
    """ Create a parameter file for runner """
    param = """1
0
0
0
0
0
0
2
1.0
0.0
1e22
0.0
0
0
0"""
    with open(param_file, "w") as ffile:
        ffile.write(param)


def runTheMesher( mesh_2d, geo, emptyParam=False ):
    nb_tetras = 0
    nb_points = 0
    nb_triangles = 0
    nb_segments = 0
    with tempfile.TemporaryDirectory() as tmp_dir:
        mesh_file = path.join(tmp_dir, "mesh.med")
        shape_file = path.join(tmp_dir, "shape.brep")
        param_file = path.join(tmp_dir, "gmsh_param.txt")
        new_elementsbinary = path.join(tmp_dir, "nodesAndElements.bin")
        output_mesh = path.join(tmp_dir, "mesh3D.med")

        print("Running in folder: ", tmp_dir)
        if not ( emptyParam ):
            create_gmsh_param_file(param_file)
        else:
            create_gmsh_empty_param_file(param_file)
        mesh_2d.ExportMED(mesh_file, 0, 41, 1, mesh_2d, 1, [], '', -1, 1)
        geompy.ExportBREP( geo, shape_file )
        
        runner = path.join("${GMSHPLUGIN_ROOT_DIR}",
                           "bin",
                           "salome",
                           "GMSHPlugin_Runner")

        if sys.platform == 'win32':
            runner += ".exe"

        cmd = "{runner} GMSH3D {mesh_file} {shape_file} "\
              "{param_file} NONE {new_elementsbinary} {output_mesh}"\
              .format(runner=runner,
                      mesh_file=mesh_file,
                      shape_file=shape_file,
                      param_file=param_file,
                      new_elementsbinary=new_elementsbinary,
                      output_mesh=output_mesh)
        print(cmd)
        subprocess.check_call(cmd, shell=True)

        mesh_read = mc.ReadUMeshFromFile(output_mesh, "MESH", 0)

        nb_tetras = mesh_read.getNumberOfCellsWithType(mc.NORM_TETRA4)
        nb_points = mesh_read.getNumberOfNodes()

        mesh_read = mc.ReadUMeshFromFile(output_mesh, "MESH", -1)
        nb_triangles = mesh_read.getNumberOfCellsWithType(mc.NORM_TRI3)

        mesh_read = mc.ReadUMeshFromFile(output_mesh, "MESH", -2)
        nb_segments = mesh_read.getNumberOfCellsWithType(mc.NORM_SEG2)

    return [nb_points, nb_segments, nb_triangles, nb_tetras]

def test_gmsh3dDefault():
    """ Test gmsh3d """
    # Building geometry
    box = geompy.MakeBoxDXDYDZ(200, 200, 200)

    geompy.ExtractShapes(box, geompy.ShapeType["FACE"], True)
    groupe_1 = geompy.CreateGroup(box, geompy.ShapeType["FACE"] )
    geompy.UnionIDs(groupe_1, [3, 13, 23, 27, 31, 33])

    [_, _, _, _, _, _, groupe_1] = geompy.GetExistingSubObjects(box, False)
    
    shape_faces = geompy.SubShapeAllSorted(box, geompy.ShapeType["FACE"])
      
    mesh_2d = smesh.Mesh(box, 'Maillage_1')

    mesh_2d.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    isDone = mesh_2d.Compute()
    premeshedTriangles  = mesh_2d.NbFaces()
    premeshedTetras     = mesh_2d.NbVolumes() 

    if not isDone:
        raise Exception("Error when computing Mesh")

    smesh.SetName(mesh_2d, 'Maillage_1')

    [nb_points, nb_segments, nb_triangles, nb_tetras] = runTheMesher( mesh_2d, box )
    

    print("Nb Tetras:", nb_tetras)
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)
    
    assert premeshedTetras == 0      
    assert nb_triangles == premeshedTriangles
    assert nb_tetras == 6 # number of tetras for normalized 2D premesh (update if default 2D netgen change!)
    assert nb_points == 8 # one node in each vertex of the cube
    assert nb_segments > 0

def test_gmsh3d():
    """ Test gmsh3d """
    # Building geometry
    box = geompy.MakeBoxDXDYDZ(200, 200, 200)

    geompy.ExtractShapes(box, geompy.ShapeType["FACE"], True)
    groupe_1 = geompy.CreateGroup(box, geompy.ShapeType["FACE"] )
    geompy.UnionIDs(groupe_1, [3, 13, 23, 27, 31, 33])

    [_, _, _, _, _, _, groupe_1] = geompy.GetExistingSubObjects(box, False)
    
    shape_faces = geompy.SubShapeAllSorted(box, geompy.ShapeType["FACE"])

    # Creating 2D mesh
    netgen_2d_parameters_1 = smesh.CreateHypothesisByAverageLength(
        'NETGEN_Parameters_2D', 'NETGENEngine', 34.641,  0 )
    
    mesh_2d = smesh.Mesh(box, 'Maillage_1')
    mesh_2d.AddHypothesis(box, netgen_2d_parameters_1)

    mesh_2d.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    isDone = mesh_2d.Compute()
    premeshedTriangles  = mesh_2d.NbFaces()
    premeshedTetras     = mesh_2d.NbVolumes() 

    if not isDone:
        raise Exception("Error when computing Mesh")

    smesh.SetName(mesh_2d, 'Maillage_1')

    [nb_points, nb_segments, nb_triangles, nb_tetras] = runTheMesher( mesh_2d, box )
    

    print("Nb Tetras:", nb_tetras)
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)
    
    assert premeshedTetras == 0      
    assert nb_triangles == premeshedTriangles
    assert nb_tetras > 6
    assert nb_points > 8
    assert nb_segments > 0
    
def test_gmsh3d_empty_parameters():
    """ Test gmsh3d """
    # Building geometry
    box = geompy.MakeBoxDXDYDZ(200, 200, 200)

    geompy.ExtractShapes(box, geompy.ShapeType["FACE"], True)
    groupe_1 = geompy.CreateGroup(box, geompy.ShapeType["FACE"] )
    geompy.UnionIDs(groupe_1, [3, 13, 23, 27, 31, 33])

    [_, _, _, _, _, _, groupe_1] = geompy.GetExistingSubObjects(box, False)
    
    shape_faces = geompy.SubShapeAllSorted(box, geompy.ShapeType["FACE"])

    # Creating 2D mesh
    netgen_2d_parameters_1 = smesh.CreateHypothesisByAverageLength(
        'NETGEN_Parameters_2D', 'NETGENEngine', 34.641,  0 )
    
    mesh_2d = smesh.Mesh(box, 'Maillage_1')
    mesh_2d.AddHypothesis(box, netgen_2d_parameters_1)

    mesh_2d.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    isDone = mesh_2d.Compute()
    premeshedTriangles  = mesh_2d.NbFaces()
    premeshedTetras     = mesh_2d.NbVolumes() 

    if not isDone:
        raise Exception("Error when computing Mesh")

    smesh.SetName(mesh_2d, 'Maillage_1')

    [nb_points, nb_segments, nb_triangles, nb_tetras] = runTheMesher( mesh_2d, box, True )
    

    print("Nb Tetras:", nb_tetras)
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)
    
    assert premeshedTetras == 0      
    assert nb_triangles == premeshedTriangles
    assert nb_tetras > 6
    assert nb_points > 8
    assert nb_segments > 0

if __name__ == "__main__":
    if sys.platform == "win32":
        print("Disabled on windows")
        sys.exit(0)

    test_gmsh3dDefault()
    test_gmsh3d()
    test_gmsh3d_empty_parameters()
    
