# Using SALOME NoteBook

from salome.kernel import salome
salome.salome_init_without_session()
from salome.kernel.salome_notebook import notebook

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# set variables
notebook.set("Length", 100)
notebook.set("Width", 200)
notebook.set("Offset", 50)

notebook.set("NbSegments", 7)
notebook.set("MaxElementArea", 800)
notebook.set("MaxElementVolume", 900)

# create a box
box = geom_builder.MakeBoxDXDYDZ("Length", "Width", 300)
idbox = geom_builder.addToStudy(box, "Box")

# create a mesh
tetra = smesh_builder.Mesh(box, "MeshBox")

algo1D = tetra.Segment()
algo1D.NumberOfSegments("NbSegments")

algo2D = tetra.Triangle()
algo2D.MaxElementArea("MaxElementArea")

algo3D = tetra.Tetrahedron()
algo3D.MaxElementVolume("MaxElementVolume")

# compute the mesh
if not tetra.Compute(): raise Exception("Error when computing Mesh")

# translate the mesh
point = SMESH.PointStruct("Offset", 0., 0.)
vector = SMESH.DirStruct(point)
tetra.TranslateObject(tetra, vector, 0)
