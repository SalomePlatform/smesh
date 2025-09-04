
"""
Most basic test of SHAPE/SMESH usecase, but it can be tested without any session launched.
"""

import sys
from salome.kernel import salome
salome.standalone() # <- key point of test is here
salome.salome_init()

#from salome.shaper import initConfig

import os
print(os.getpid())
#input("AAA")
###
### SHAPER component
###

from salome.shaper import model
model.begin()
partSet = model.moduleDocument()

### Create Part
Part_1 = model.addPart(partSet)
Part_1_doc = Part_1.document()

### Create Box
Box_1 = model.addBox(Part_1_doc, 10, 10, 10)

model.end()

###
### SHAPERSTUDY component
###
#import pdb; pdb.set_trace()
model.publishToShaperStudy()
import SHAPERSTUDY
#import pdb; pdb.set_trace()
Box_1_1, = SHAPERSTUDY.shape(model.featureStringId(Box_1))
###
### SMESH component
###

from salome.kernel import SMESH
from salome.kernel import  SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
#smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                 # multiples meshes built in parallel, complex and numerous mesh edition (performance)

Mesh_1 = smesh.Mesh(Box_1_1)
NETGEN_1D_2D_3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
NETGEN_3D_Parameters_1 = NETGEN_1D_2D_3D.Parameters()
NETGEN_3D_Parameters_1.SetMaxSize( 5 )
NETGEN_3D_Parameters_1.SetMinSize( 1 )
NETGEN_3D_Parameters_1.SetSecondOrder( 0 )
NETGEN_3D_Parameters_1.SetOptimize( 1 )
NETGEN_3D_Parameters_1.SetFineness( 2 )
NETGEN_3D_Parameters_1.SetChordalError( -1 )
NETGEN_3D_Parameters_1.SetChordalErrorEnabled( 0 )
NETGEN_3D_Parameters_1.SetUseSurfaceCurvature( 1 )
NETGEN_3D_Parameters_1.SetFuseEdges( 1 )
NETGEN_3D_Parameters_1.SetQuadAllowed( 0 )
NETGEN_3D_Parameters_1.SetCheckChartBoundary( 152 )
isDone = Mesh_1.Compute()
if not isDone:
    raise Exception("Error when computing Mesh")

## Set names of Mesh objects
smesh.SetName(NETGEN_1D_2D_3D.GetAlgorithm(), 'NETGEN 1D-2D-3D')
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')
smesh.SetName(NETGEN_3D_Parameters_1, 'NETGEN 3D Parameters_1')

assert(Mesh_1.GetMesh().NbTetras()>=5)
