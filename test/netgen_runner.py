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
    param="""1
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
    Groupe_1 = geompy.CreateGroup(box, geompy.ShapeType["FACE"])
    geompy.UnionIDs(Groupe_1, [3, 13, 23, 27, 31, 33])

    # TODO: useful ?
    [_, _, _, _, _, _, Groupe_1] = geompy.GetExistingSubObjects(box, False)

    # Creating 2D mesh
    NETGEN_2D_Parameters_1 = smesh.CreateHypothesisByAverageLength(
        'NETGEN_Parameters_2D', 'NETGENEngine', 34.641, 0)
    Mesh2D = smesh.Mesh(Groupe_1, 'Maillage_1')
    status = Mesh2D.AddHypothesis(Groupe_1, NETGEN_2D_Parameters_1)
    NETGEN_1D_2D = Mesh2D.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    isDone = Mesh2D.Compute()
    smesh.SetName(Mesh2D, 'Maillage_1')

    # tmp_dir = tempfile.mkdtemp()
    with tempfile.TemporaryDirectory() as tmp_dir:
      mesh_file = path.join(tmp_dir, "mesh.med")
      shape_file = path.join(tmp_dir, "shape.step")
      param_file = path.join(tmp_dir, "param.txt")
      output_mesh = path.join(tmp_dir, "mesh3D.med")

      print("Running in folder: ", tmp_dir)
      create_param_file(param_file)

      Mesh2D.ExportMED(mesh_file, 0, 41, 1, Mesh2D, 1, [], '', -1, 1)
      geompy.ExportSTEP(box, shape_file, GEOM.LU_METER)

      runner = path.join("${NETGENPLUGIN_ROOT_DIR}",
                          "bin",
                          "salome",
                          "NETGENPlugin_Runner")

      cmd = "{runner} NETGEN3D {mesh_file} {shape_file} "\
            "{param_file} NONE 2 NONE {output_mesh}"\
            .format(runner=runner,
                    mesh_file=mesh_file,
                    shape_file=shape_file,
                    param_file=param_file,
                    output_mesh=output_mesh)
      print(cmd)
      subprocess.check_call(cmd, shell=True)

      meshRead = mc.ReadUMeshFromFile (output_mesh, "MESH", 0)

      nbTetras = meshRead.getNumberOfCellsWithType(mc.NORM_TETRA4)
      nbPoints = meshRead.getNumberOfNodes()

      meshRead = mc.ReadUMeshFromFile (output_mesh, "MESH", -1)
      nbTriangles = meshRead.getNumberOfCellsWithType(mc.NORM_TRI3)

      meshRead = mc.ReadUMeshFromFile (output_mesh, "MESH", -2)
      nbSegments = meshRead.getNumberOfCellsWithType(mc.NORM_SEG2)

      print("Nb Tetras:", nbTetras)
      print("Nb Triangles:", nbTriangles)
      print("Nb Segments:", nbSegments)
      print("Nb Points:", nbPoints)

      assert(nbPoints > 0)
      assert(nbSegments > 0)
      assert(nbTriangles > 0)
      assert(nbTetras > 0)

if __name__ == "__main__":
    test_netgen3d()
