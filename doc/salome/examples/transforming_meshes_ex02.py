# Rotation

import math

import SMESH_mechanic
import SMESH

smesh = SMESH_mechanic.smesh
mesh = SMESH_mechanic.mesh 

# define rotation axis and angle
axisXYZ = SMESH.AxisStruct(0., 0., 0., 5., 5., 20.)
angle270 = 1.5 * math.pi

# rotate a mesh
mesh.Rotate([], axisXYZ, angle270, 1)  
