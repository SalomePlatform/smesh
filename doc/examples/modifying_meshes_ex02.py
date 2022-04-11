# Add 0D Element

import salome
salome.salome_init_without_session()

from salome.smesh import smeshBuilder
smesh_builder = smeshBuilder.New()

mesh = smesh_builder.Mesh()

# add node
node_id = mesh.AddNode(50, 10, 0)

# add 0D Element
new_id = mesh.Add0DElement(node_id)

print("")
if new_id == 0: print("KO node addition.")
else:           print("New 0D Element has been added with ID ", new_id)
