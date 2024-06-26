# Smoothing

from mechanic import *

# select the top face
faces = geom_builder.SubShapeAllSorted(mechanic, geom_builder.ShapeType["FACE"])

# create a group of faces to be smoothed
group_smooth = mesh.GroupOnGeom(faces[3], "Group of faces (smooth)", SMESH.FACE)

# perform smoothing

# boolean SmoothObject(Object, IDsOfFixedNodes, MaxNbOfIterations, MaxAspectRatio, Method)
res0 = mesh.SmoothObject(group_smooth, [], 20, 2., smesh_builder.CENTROIDAL_SMOOTH)
res1 = mesh.SmoothObject(group_smooth, [], 20, 2., smesh_builder.LAPLACIAN_SMOOTH)
print("\nSmoothing ... ", end=' ')
if not (res0 and res1): print("failed!")
else:       print("done.")
