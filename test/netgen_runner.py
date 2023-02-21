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
34.64
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


def test_netgen3d():
    """ Test netgen3d mesher """
    # Building geometry
    box = geompy.MakeBoxDXDYDZ(200, 200, 200)

    geompy.ExtractShapes(box, geompy.ShapeType["FACE"], True)
    groupe_1 = geompy.CreateGroup(box, geompy.ShapeType["FACE"])
    geompy.UnionIDs(groupe_1, [3, 13, 23, 27, 31, 33])

    [_, _, _, _, _, _, groupe_1] = geompy.GetExistingSubObjects(box, False)

    # Creating 2D mesh
    netgen_2d_parameters_1 = smesh.CreateHypothesisByAverageLength(
        'NETGEN_Parameters_2D', 'NETGENEngine', 34.641, 0)
    mesh_2d = smesh.Mesh(groupe_1, 'Maillage_1')
    mesh_2d.AddHypothesis(groupe_1, netgen_2d_parameters_1)
    mesh_2d.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    isDone = mesh_2d.Compute()
    if not isDone:
        raise Exception("Error when computing Mesh")

    smesh.SetName(mesh_2d, 'Maillage_1')

    with tempfile.TemporaryDirectory() as tmp_dir:
        mesh_file = path.join(tmp_dir, "mesh.med")
        shape_file = path.join(tmp_dir, "shape.step")
        param_file = path.join(tmp_dir, "param.txt")
        output_mesh = path.join(tmp_dir, "mesh3D.med")

        print("Running in folder: ", tmp_dir)
        create_param_file(param_file)

        mesh_2d.ExportMED(mesh_file, 0, 41, 1, mesh_2d, 1, [], '', -1, 1)
        geompy.ExportSTEP(box, shape_file, GEOM.LU_METER)

        runner = path.join("${NETGENPLUGIN_ROOT_DIR}",
                           "bin",
                           "salome",
                           "NETGENPlugin_Runner")

        if sys.platform == 'win32':
            runner += ".exe"

        cmd = "{runner} NETGEN3D {mesh_file} {shape_file} "\
              "{param_file} NONE NONE {output_mesh}"\
              .format(runner=runner,
                      mesh_file=mesh_file,
                      shape_file=shape_file,
                      param_file=param_file,
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

        print("Nb Tetras:", nb_tetras)
        print("Nb Triangles:", nb_triangles)
        print("Nb Segments:", nb_segments)
        print("Nb Points:", nb_points)

        assert nb_points > 0
        assert nb_segments > 0
        assert nb_triangles > 0
        assert nb_tetras > 0

if __name__ == "__main__":
    if sys.platform == "win32":
        print("Disabled on windows")
        sys.exit(0)
    test_netgen3d()
