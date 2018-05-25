.. _use_existing_page:

**************************************
Use Edges/Faces to be Created Manually
**************************************

The algorithms **Use Edges to be Created Manually** and **Use Faces to be Created Manually** allow  creating a 1D or a 2D mesh in a python script (using *AddNode, AddEdge* and *AddFace* commands) and then using such sub-meshes in the construction of a 2D or a 3D mesh. 

For example, you want to use standard algorithms to generate 1D and 3D
meshes and to create 2D mesh by your python code. For this, you

#. create a mesh object, assign a 1D algorithm,
#. invoke **Compute** command, which computes a 1D mesh,
#. assign **Use Faces to be Created Manually** and a 3D algorithm,
#. run your python code, which creates a 2D mesh,
#. invoke **Compute** command, which computes a 3D mesh.

.. warning:: **Use Edges to be Created Manually** and **Use Faces to be Created Manually** algorithms should be assigned *before* mesh generation by the Python code.

Consider trying a sample script demonstrating the usage of :ref:`Use Faces to be Created Manually <tui_use_existing_faces>` algorithm for construction of a 2D mesh using Python commands.

.. figure:: ../images/use_existing_face_sample_mesh.png
   :align: center

**See also** :ref:`the sample script <tui_use_existing_faces>` creating the mesh shown in the image in a Shrink mode.
  
