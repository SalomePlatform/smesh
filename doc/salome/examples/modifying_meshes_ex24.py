# Revolution

import math
import SMESH

import SMESH_mechanic

mesh  = SMESH_mechanic.mesh
smesh = SMESH_mechanic.smesh

# create a group of faces to be revolved
FacesRotate = [492, 493, 502, 503]
GroupRotate = mesh.CreateEmptyGroup(SMESH.FACE,"Group of faces (rotate)")
GroupRotate.Add(FacesRotate)

# define revolution angle and axis
angle45 = 45 * math.pi / 180
axisXYZ = SMESH.AxisStruct(-38.3128, -73.3658, -23.321, -13.3402, -13.3265, 6.66632)

# perform revolution of an object
mesh.RotationSweepObject(GroupRotate, axisXYZ, angle45, 4, 1e-5) 
