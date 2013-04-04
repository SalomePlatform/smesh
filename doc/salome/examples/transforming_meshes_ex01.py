# Translation

import SMESH_mechanic
import SMESH

smesh = SMESH_mechanic.smesh 
mesh = SMESH_mechanic.mesh 

# define translation vector
point = SMESH.PointStruct(-150., -150., 0.)
vector =SMESH.DirStruct(point)

# translate a mesh
doCopy = 1

mesh.Translate([], vector, doCopy)
