# -*- coding: utf-8 -*-

import sys
import salome

salome.salome_init_without_session()

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
unPentaBiQuad = smesh.Mesh()
nodeID = unPentaBiQuad.AddNode( 0, 0, 0 )
nodeID = unPentaBiQuad.AddNode( 10, 0, 0 )
nodeID = unPentaBiQuad.AddNode( 0, 10, 0 )
nodeID = unPentaBiQuad.AddNode( 0, 0, 10 )
nodeID = unPentaBiQuad.AddNode( 10, 0, 10 )
nodeID = unPentaBiQuad.AddNode( 0, 10, 10 )
nodeID = unPentaBiQuad.AddNode( 5, 0, 0 )
nodeID = unPentaBiQuad.AddNode( 7, 7, 0 )
nodeID = unPentaBiQuad.AddNode( 0, 5, 0 )
nodeID = unPentaBiQuad.AddNode( 5, 0, 10 )
nodeID = unPentaBiQuad.AddNode( 7, 7, 10 )
nodeID = unPentaBiQuad.AddNode( 0, 5, 10 )
nodeID = unPentaBiQuad.AddNode( 0, 0, 5 )
nodeID = unPentaBiQuad.AddNode( 10, 0, 5 )
nodeID = unPentaBiQuad.AddNode( 0, 10, 5 )
nodeID = unPentaBiQuad.AddNode( 5, -1, 5 )
nodeID = unPentaBiQuad.AddNode( 8, 8, 5 )
nodeID = unPentaBiQuad.AddNode( -1, 5, 5 )
volID = unPentaBiQuad.AddVolume( [ 4, 5, 6, 1, 2, 3, 10, 11, 12, 7, 8, 9, 13, 14, 15, 16, 17, 18 ] )

infos = unPentaBiQuad.GetMeshInfo()
print("Number of biquadratic pentahedrons:", infos[SMESH.Entity_BiQuad_Penta])
if (infos[SMESH.Entity_BiQuad_Penta] != 1):
  raise RuntimeError("Bad number of biquadratic pentahedrons: should be 1")

## Set names of Mesh objects
smesh.SetName(unPentaBiQuad.GetMesh(), 'unPentaBiQuad')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
