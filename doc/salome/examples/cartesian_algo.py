# Usage of Body Fitting algorithm


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook


# create a sphere
sphere = geompy.MakeSphereR( 50 )
geompy.addToStudy( sphere, "sphere" )

# create a mesh and assign a "Body Fitting" algo
mesh = smesh.Mesh( sphere )
cartAlgo = mesh.BodyFitted()

# define a cartesian grid using Coordinates
coords = range(-100,100,10)
cartHyp = cartAlgo.SetGrid( coords,coords,coords, 1000000)

# compute the mesh
mesh.Compute()
print "nb hexahedra",mesh.NbHexas()
print "nb tetrahedra",mesh.NbTetras()
print "nb polyhedra",mesh.NbPolyhedrons()
print

# define the grid by setting constant spacing
cartHyp = cartAlgo.SetGrid( "10","10","10", 1000000)

mesh.Compute()
print "nb hexahedra",mesh.NbHexas()
print "nb tetrahedra",mesh.NbTetras()
print "nb polyhedra",mesh.NbPolyhedrons()


# define the grid by setting different spacing in 2 sub-ranges of geometry
spaceFuns = ["5","10+10*t"]
cartAlgo.SetGrid( [spaceFuns, [0.5]], [spaceFuns, [0.5]], [spaceFuns, [0.25]], 10 )

mesh.Compute()
print "nb hexahedra",mesh.NbHexas()
print "nb tetrahedra",mesh.NbTetras()
print "nb polyhedra",mesh.NbPolyhedrons()
print
