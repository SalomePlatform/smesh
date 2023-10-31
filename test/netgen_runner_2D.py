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

def create_param_file(param_file):
    """ Create a parameter file for runner """
    param = """1
34.641
0.14
16
0.15
1.5
0
0
1
5
1
1
-1
3
3
0.2
2
1
0
0
2
2
0

0
0
0"""
    with open(param_file, "w") as ffile:
        ffile.write(param)

def create_maxarea_param_file_2D(param_file,maxArea):
    """ Create a parameter file for runner """
    param = """1
{}
""".format(maxArea)
    with open(param_file, "w") as ffile:
        ffile.write(param)

def create_lenghtFromEdges_param_file_2D(param_file):
    """ Create a parameter file for runner """
    param = """1
"""

    with open(param_file, "w") as ffile:
        ffile.write(param)

def CommonFunction(netgen,case,numberOfEdges1D,hypo,maxArea):
    # Building geometry
    geometry = geompy.MakeBoxDXDYDZ(200, 200, 200)
    # Create 1D regular elements
    mesh_1d = smesh.Mesh(geometry, 'Maillage_1')
    mesh_1d.Segment().NumberOfSegments(numberOfEdges1D)
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

        if ( hypo == 0 ): 
            param_file = path.join(tmp_dir, "param.txt")
            create_param_file(param_file )

        if ( hypo == 1 ):
            param_file = path.join(tmp_dir, "param_lenghtfromedge.txt")
            create_lenghtFromEdges_param_file_2D(param_file ) 

        if ( hypo == 2 ):
            param_file = path.join(tmp_dir, "param_maxarea.txt")
            create_maxarea_param_file_2D(param_file,maxArea)            
        
        bin_file    =  path.join(tmp_dir, "mesh.bin")
        output_mesh = path.join(tmp_dir, "meshout.med")

        print("Running in folder: ", tmp_dir)
                
        mesh_1d.ExportMED(mesh_file, 0, 41, 1, mesh_1d, 1, [], '', -1, 1)
        geompy.ExportBREP( geometry, shape_file )

        runner = path.join("${NETGENPLUGIN_ROOT_DIR}",
                            "bin",
                            "salome",
                            "NETGENPlugin_Runner")

        if sys.platform == 'win32':
            runner += ".exe"

        cmd = "{runner} {NETGEN} {mesh_file} {shape_file} "\
                "{param_file} NONE {bin_file} {output_mesh}"\
                .format(runner=runner,
                        NETGEN=netgen,
                        mesh_file=mesh_file,
                        shape_file=shape_file,
                        param_file=param_file,
                        bin_file=bin_file,
                        output_mesh=output_mesh)
        print(cmd)
        subprocess.check_call(cmd, shell=True)   
        mesh_read = mc.ReadUMeshFromFile(output_mesh, "MESH", 0)
        nb_points = mesh_read.getNumberOfNodes()
        if (case == 2):
            nb_triangles = mesh_read.getNumberOfCellsWithType(mc.NORM_TRI3)
            mesh_read    = mc.ReadUMeshFromFile(output_mesh, "MESH", -1)
            nb_segments  = mesh_read.getNumberOfCellsWithType(mc.NORM_SEG2)
            
    return [nb_points,nb_segments,nb_triangles,nb_tetras] 
    

def test_netgen2d():
    """ Test netgen2d mesher """
    [nb_points,nb_segments,nb_triangles,nb_tetras] = CommonFunction( "NETGEN2D", 2, 3, 0, 0.0 )
    
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)

    assert nb_triangles > 12
    assert nb_points > 8
    assert nb_segments > 12

def test_netgen2dMaxArea():
    """ Test netgen2d mesher """
    [nb_points,nb_segments,nb_triangles,nb_tetras] = CommonFunction( "NETGEN2D", 2, 3, 2, 75.0 )
    
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)

    assert nb_triangles > 12
    assert nb_points > 8
    assert nb_segments > 12

def test_netgen2dLenghtFromEdge():
    """ Test netgen2d mesher """
    [nb_points,nb_segments,nb_triangles,nb_tetras] = CommonFunction( "NETGEN2D", 2, 1, 1, 0.0 )
    
    print("Nb Triangles:", nb_triangles)
    print("Nb Segments:", nb_segments)
    print("Nb Points:", nb_points)

    assert nb_triangles == 12
    assert nb_points == 8
    assert nb_segments == 12

if __name__ == "__main__":
    if sys.platform == "win32":
        print("Disabled on windows")
        sys.exit(0)
    test_netgen2d()
    test_netgen2dMaxArea()
    test_netgen2dLenghtFromEdge()
