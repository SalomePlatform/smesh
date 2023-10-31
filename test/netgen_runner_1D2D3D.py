#!/usr/bin/env python
import sys
import salome

salome.salome_init()

from os import path
import tempfile
import subprocess

import GEOM, SMESH, SALOMEDS

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

import math

smesh = smeshBuilder.New()
geompy = geomBuilder.New()

import medcoupling as mc

def create_simple2d_param_file_1D(param_file,segments,segLenght,maxArea):
    """ Create a parameter file for runner """
    param = """1
{}
{}
{}
0""".format(segments,segLenght,maxArea)
    with open(param_file, "w") as ffile:
        ffile.write(param)

def create_simple2d3d_param_file_1D(param_file,segments,segLenght,maxArea,maxVolume):
    """ Create a parameter file for runner """
    param = """1
{}
{}
{}
{}
0""".format(segments,segLenght,maxArea,maxVolume)

    with open(param_file, "w") as ffile:
        ffile.write(param)

def CommonFunction(netgen,case,segments,segLenght,maxArea,maxVolume):
    # Building geometry
    box = geompy.MakeBoxDXDYDZ(200, 200, 200)

    # Create 1D regular elements
    mesh_1d = smesh.Mesh(box, 'Maillage_1')
    mesh_1d.Segment().NumberOfSegments(1)
    isDone = mesh_1d.Compute()

    if not isDone:
        raise Exception("Error when computing Mesh")

    smesh.SetName(mesh_1d, 'Maillage_1')
    nb_points = 0
    nb_segments = 0
    nb_triangles = 0
    nb_tetras = 0
    with tempfile.TemporaryDirectory() as tmp_dir:
        mesh_file = path.join(tmp_dir, "mesh.med")
        shape_file = path.join(tmp_dir, "shape.brep")
        if ( case <= 2 ):
            param_file = path.join(tmp_dir, "param_simple2D.txt")
        else: 
            param_file = path.join(tmp_dir, "param_simple3D.txt")

        output_mesh = path.join(tmp_dir, "meshout.med")

        print("Running in folder: ", tmp_dir)
        
        if ( case <= 2 ):
            create_simple2d_param_file_1D(param_file, segments, segLenght, maxArea )
        else:
            create_simple2d3d_param_file_1D(param_file, segments, segLenght, maxArea, maxVolume )
        
        mesh_1d.ExportMED(mesh_file, 0, 41, 1, mesh_1d, 1, [], '', -1, 1)
        geompy.ExportBREP( box, shape_file )

        runner = path.join("${NETGENPLUGIN_ROOT_DIR}",
                            "bin",
                            "salome",
                            "NETGENPlugin_Runner")

        if sys.platform == 'win32':
            runner += ".exe"

        cmd = "{runner} {NETGEN} {mesh_file} {shape_file} "\
                "{param_file} NONE NONE {output_mesh}"\
                .format(runner=runner,
                        NETGEN=netgen,
                        mesh_file=mesh_file,
                        shape_file=shape_file,
                        param_file=param_file,
                        output_mesh=output_mesh)
        print(cmd)
        subprocess.check_call(cmd, shell=True)   
        mesh_read = mc.ReadUMeshFromFile(output_mesh, "MESH", 0)
        nb_points = mesh_read.getNumberOfNodes()
        if (case == 1):
            nb_segments = mesh_read.getNumberOfCellsWithType(mc.NORM_SEG2)
        if (case == 2):
            nb_triangles = mesh_read.getNumberOfCellsWithType(mc.NORM_TRI3)
            mesh_read    = mc.ReadUMeshFromFile(output_mesh, "MESH", -1)
            nb_segments  = mesh_read.getNumberOfCellsWithType(mc.NORM_SEG2)
        if (case == 3):
            nb_tetras = mesh_read.getNumberOfCellsWithType(mc.NORM_TETRA4)
            mesh_read = mc.ReadUMeshFromFile(output_mesh, "MESH", -1)
            nb_triangles = mesh_read.getNumberOfCellsWithType(mc.NORM_TRI3)
            mesh_read = mc.ReadUMeshFromFile(output_mesh, "MESH", -2)
            nb_segments = mesh_read.getNumberOfCellsWithType(mc.NORM_SEG2)
            
    return [nb_points,nb_segments,nb_triangles,nb_tetras] 

def test_netgen1d():
    [nb_points,nb_segments,nb_triangles,nb_tetras] = CommonFunction( "NETGEN1D", 1, 1, 200, 0.0, 0.0 )

    print("Nb Points:", nb_points)
    print("Nb Segments:", nb_segments)
    assert nb_points > 0
    assert nb_segments > 0
    

def test_netgen1d2d():
    """ Test netgen1d2d mesher """
    [nb_points,nb_segments,nb_triangles,nb_tetras] = CommonFunction( "NETGEN1D2D", 2, 1, 200, 0.0, 0.0 )
    
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)

    assert nb_triangles == 12
    assert nb_points > 0
    assert nb_segments > 0

def test_netgen1d2dMaxArea():
    """ Test netgen1d2d mesher """
    [nb_points,nb_segments,nb_triangles, nb_tetras] = CommonFunction( "NETGEN1D2D", 2, 5, 200, 50.0, 0.0 )
    
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)

    assert nb_triangles > 12
    assert nb_points > 0
    assert nb_segments > 0

                
def test_netgen1d2d3d():
    """ Test netgen1d2d mesher """
    [nb_points,nb_segments,nb_triangles,nb_tetras] = CommonFunction( "NETGEN1D2D3D", 3, 1, 200, 0.0, 0.0 )

    print("Nb Tetras:", nb_tetras)
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)
    
    assert nb_triangles == 12
    assert nb_tetras == 5
    assert nb_points > 0
    assert nb_segments > 0

def test_netgen1d2dMaxVolume():
    """ Test netgen1d2d mesher """
    [nb_points,nb_segments,nb_triangles,nb_tetras] = CommonFunction( "NETGEN1D2D3D", 3, 1, 200, 500.0, 500.0 )

    print("Nb Tetras:", nb_tetras)
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)
    
    assert nb_triangles > 12
    assert nb_tetras > 5
    assert nb_points > 0
    assert nb_segments > 0

if __name__ == "__main__":
    if sys.platform == "win32":
        print("Disabled on windows")
        sys.exit(0)
    test_netgen1d()
    test_netgen1d2d()
    test_netgen1d2d3d()
    test_netgen1d2dMaxArea()
    test_netgen1d2dMaxVolume() 
