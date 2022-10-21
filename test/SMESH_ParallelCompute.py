# contains function to compute a mesh in parallel
from platform import java_ver
import sys
from tkinter import W
import salome

import time


salome.salome_init()
import salome_notebook
notebook = salome_notebook.NoteBook()

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder
import math
import SALOMEDS

import numpy as np

geompy = geomBuilder.New()

smesh = smeshBuilder.New()


def build_seq_mesh(nbox, boxsize, offset):
    # Create 3D faces
    boxes = []
    # First creating all the boxes
    for i in range(nbox):
        for j in range(nbox):
            for k in range(nbox):

                x_orig = i*(boxsize+offset)
                y_orig = j*(boxsize+offset)
                z_orig = k*(boxsize+offset)

                tmp_box = geompy.MakeBoxDXDYDZ(boxsize, boxsize, boxsize)

                if not i == j == k == 0:
                    box = geompy.MakeTranslation(tmp_box, x_orig,
                                                 y_orig, z_orig)
                else:
                    box = tmp_box

                geompy.addToStudy(box, 'box_{}:{}:{}'.format(i, j, k))

                boxes.append(box)

    # Create fuse of all boxes
    all_boxes = geompy.MakeCompound(boxes)
    geompy.addToStudy(all_boxes, 'Compound_1')

    # Removing duplicates faces and edges
    all_boxes = geompy.MakeGlueFaces(all_boxes, 1e-07)
    geompy.addToStudy(all_boxes, 'Glued_Faces_1')

    all_boxes = geompy.MakeGlueEdges(all_boxes, 1e-07)
    geompy.addToStudy(all_boxes, 'rubik_cube')


    # Building sequetial mesh
    print("Creating mesh")
    all_box_mesh = smesh.Mesh(all_boxes, "seq_mesh")

    print("Adding algo")
    algo3d = all_box_mesh.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)

    netgen_parameters = algo3d.Parameters()
    netgen_parameters.SetMaxSize(34.641)
    netgen_parameters.SetMinSize(0.141421)
    netgen_parameters.SetOptimize(1)
    netgen_parameters.SetCheckOverlapping(0)
    netgen_parameters.SetCheckChartBoundary(0)
    netgen_parameters.SetFineness(5)
    netgen_parameters.SetNbSegPerEdge(16*(boxsize//100))
    netgen_parameters.SetNbSegPerRadius(1.5)
    netgen_parameters.SetGrowthRate(0.15)
    netgen_parameters.SetChordalError(-1)
    netgen_parameters.SetChordalErrorEnabled(0)
    netgen_parameters.SetUseSurfaceCurvature(1)
    netgen_parameters.SetQuadAllowed(0)
    netgen_parameters.SetCheckOverlapping(False)
    netgen_parameters.SetNbThreads(2)

    return all_boxes, all_box_mesh, netgen_parameters

def run_test(nbox=2, boxsize=100):
    """ Run sequential mesh and parallel version of it

    nbox: NUmber of boxes
    boxsize: Size of each box
    """
    geom, seq_mesh, netgen_parameters = build_seq_mesh(nbox, boxsize, 0)

    par_mesh = smesh.ParallelMesh(geom, netgen_parameters, 6, name="par_mesh")

    start = time.monotonic()
    is_done = seq_mesh.Compute()
    assert is_done
    stop = time.monotonic()
    time_seq = stop-start

    start = time.monotonic()
    is_done = par_mesh.Compute()
    assert is_done
    stop = time.monotonic()
    time_par = stop-start

    print("  Tetrahedron: ", seq_mesh.NbTetras(), par_mesh.NbTetras())
    print("  Triangle: ", seq_mesh.NbTriangles(), par_mesh.NbTriangles())
    print("  edge: ", seq_mesh.NbEdges(), par_mesh.NbEdges())

    assert  par_mesh.NbTetras() > 0
    assert  par_mesh.NbTriangles() > 0
    assert  par_mesh.NbEdges() > 0

    print("Time elapsed (seq, par): ", time_seq, time_par)

def main():
    nbox = 2
    boxsize = 100
    run_test(nbox, boxsize)

main()

