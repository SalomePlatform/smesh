# Split volumic elements into tetrahedrons

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)

# mesh a hexahedral mesh
box = geompy.MakeBoxDXDYDZ (1, 1, 1 )
mesh = smesh.Mesh( box )
mesh.AutomaticHexahedralization(0)
print("Nb volumes mesh: %s" % mesh.NbHexas())

# split each hexahedron into 6 tetrahedra
mesh.SplitVolumesIntoTetra( mesh, smesh.Hex_6Tet )
print("Nb volumes mesh: %s" % mesh.NbTetras())
