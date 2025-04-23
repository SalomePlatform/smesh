#!/usr/bin/env python

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Sphere_1 = geompy.MakeSphereR(100)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Sphere_1, 'Sphere_1' )

Mesh_1 = smesh.Mesh(Sphere_1,'Mesh_1')
Cartesian_3D = Mesh_1.BodyFitted()
Body_Fitting_Parameters_1 = Cartesian_3D.SetGrid([ [ '34.641' ], [ 0, 1 ]],[ [ '34.641' ], [ 0, 1 ]],[ [ '34.641' ], [ 0, 1 ]],4,0)
Body_Fitting_Parameters_1.SetToUseQuanta( 1 )
Body_Fitting_Parameters_1.SetQuanta( 0.8 )
isDone = Mesh_1.Compute()

Polys   = Mesh_1.NbPolyhedrons()
Hexas1  = Mesh_1.NbHexas()

#No polyhedrons in the mesh
assert(Polys==0)

Body_Fitting_Parameters_1.SetQuanta( 0.2 )
isDone = Mesh_1.Compute()

Polys   = Mesh_1.NbPolyhedrons()
Hexas2  = Mesh_1.NbHexas()

#Still no polyhedrons in the mesh
assert(Polys==0)

#Number of hexahedrons is bigger for hexas2 because quanta value is smaller
assert( Hexas1 < Hexas2 )

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
