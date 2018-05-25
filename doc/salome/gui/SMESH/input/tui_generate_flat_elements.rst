.. _tui_generate_flat_elements_page: 

**********************
Generate flat elements
**********************


.. _tui_double_nodes_on_group_boundaries:

Double nodes on groups boundaries
#################################

Double nodes on shared faces between groups of volumes and create flat elements on demand.

The list of groups must contain at least two groups. The groups have to be disjoint: no common element into two different groups.

The nodes of the internal faces at the boundaries of the groups are doubled. Optionally, the internal faces are replaced by flat elements.

Triangles are transformed into prisms, and quadrangles into hexahedrons.

The flat elements are stored in groups of volumes.

These groups are named according to the position of the group in the list:
the group j_n_p is the group of the flat elements that are built between the group \#n and the group \#p in the list.
If there is no shared faces between the group \#n and the group \#p in the list, the group j_n_p is not created.
All the flat elements are gathered into the group named "joints3D" (or "joints2D" in 2D situation).
The flat element of the multiple junctions between the simple junction are stored in a group named "jointsMultiples".

This example represents an iron cable (a thin cylinder) in a concrete bloc (a big cylinder).
The big cylinder is defined by two geometric volumes.

.. literalinclude:: ../../../examples/generate_flat_elements.py
    :linenos:
    :language: python

:download:`Download this script <../../../examples/generate_flat_elements.py>`

Here, the 4 groups of volumes [Solid_1_1, Solid_2_1, Solid_3_1, Solid_4_1] constitute a partition of the mesh.
The flat elements on group boundaries and on faces are built with the
2 last lines of the code above.

If the last argument (Boolean) in DoubleNodesOnGroupBoundaries is set to 1,
the flat elements are built, otherwise, there is only a duplication of the nodes.

To observe flat element groups, save the resulting mesh on a MED file and reload it.


