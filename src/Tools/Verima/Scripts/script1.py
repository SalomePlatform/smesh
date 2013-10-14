import sys
import salome

salome.salome_init()
theStudy = salome.myStudy

import salome_notebook
notebook = salome_notebook.notebook
sys.path.insert( 0, r'/local00/home/A96028/Salome/V7_main/appli_V7_main')

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS
geompy = geomBuilder.New(theStudy)

##
### SMESH component
###
import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(theStudy)
from salome.NETGENPlugin import NETGENPluginBuilder

# create a box
O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Box_1, 'Box_1' )

Mesh_1 = smesh.Mesh(Box_1)
NETGEN_2D3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
NETGEN_3D_Simple_Parameters_1 = NETGEN_2D3D.Parameters(smeshBuilder.SIMPLE)
NETGEN_3D_Simple_Parameters_1.SetNumberOfSegments( 15 )
NETGEN_3D_Simple_Parameters_1.LengthFromEdges()
NETGEN_3D_Simple_Parameters_1.LengthFromFaces()
isDone = Mesh_1.Compute()

smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')
smesh.SetName(NETGEN_2D3D.GetAlgorithm(), 'NETGEN_2D3D')
smesh.SetName(NETGEN_3D_Simple_Parameters_1, 'NETGEN 3D Simple Parameters_1')


fichierMedResult='/tmp/tetra.med'
#Mesh_1.ExportMED(fichierMedResult)

#____________________________________________________________
from Stats.getStats import getStatsMaillage, getStatsGroupes, genHistogram

fichierStatMailles=fichierMedResult.replace('.med','.res')
fichierStatTailles=fichierMedResult.replace('.med','.taille')
fichierStatRatio=fichierMedResult.replace('.med','.ratio')

getStatsMaillage(Mesh_1,fichierStatMailles)
getStatsGroupes(Mesh_1,fichierMedResult)
genHistogram(Mesh_1, SMESH.FT_MaxElementLength3D, 20, False, fichierStatTailles,theStudy)
genHistogram(Mesh_1, SMESH.FT_AspectRatio3D, 20, False, fichierStatRatio,theStudy)



