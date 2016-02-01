# Translation

import SMESH_mechanic

mesh = SMESH_mechanic.mesh 

# define translation vector
vector = [-150., -150., 0.]

# make a translated copy of all elements of the mesh
mesh.TranslateObject(mesh, vector, Copy=True)
