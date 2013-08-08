# Basic Properties


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook

from SMESH_mechanic import mesh as mesh1

mesh1.Compute()

# compute length for mesh1
length = mesh1.GetLength()

# compute area for mesh1
area = mesh1.GetArea()

# compute volume for mesh1
volume = mesh1.GetVolume()

# print basic properties
print
print "Basic Properties for mesh:"
print "* length =", length
print "* area   =", area
print "* volume =", volume
print
