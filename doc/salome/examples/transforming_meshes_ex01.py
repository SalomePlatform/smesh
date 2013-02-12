# Translation

import SMESH_mechanic

smesh = SMESH_mechanic.smesh 
mesh = SMESH_mechanic.mesh 

# define translation vector
point = smesh.PointStruct(-150., -150., 0.)
vector =smesh.DirStruct(point) 

# translate a mesh
doCopy = 1

mesh.Translate([], vector, doCopy)
