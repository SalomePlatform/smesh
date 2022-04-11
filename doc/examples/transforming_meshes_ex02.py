# Rotation

import math

from mechanic import *

# define rotation axis and angle
axisXYZ = SMESH.AxisStruct(0., 0., 0., 5., 5., 20.)
angle270 = 1.5 * math.pi

# rotate a mesh
mesh.Rotate([], axisXYZ, angle270, 1)  
