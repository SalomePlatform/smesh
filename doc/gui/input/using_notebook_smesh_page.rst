.. _using_notebook_mesh_page: 

*********************
Using SALOME NoteBook
*********************

**SALOME NoteBook** allows defining variables to be used for
creation and modification of objects.

.. image:: ../images/using_notebook_smesh.png 
	:align: center

.. centered::
	Setting of variables in SALOME NoteBook

.. image:: ../images/addnode_notebook.png 
	:align: center

.. centered::
	Use of variables to add a node in MESH module

The following limitations on the use of variables still exist:

* :ref:`Distribution of Layers <radial_prism_algo_page>` hypothesis - parametrical values are correctly applied, but they are not restored after "Update study" operation.
* :ref:`Number of Segments <a1d_meshing_hypo_page>` hypothesis, Distribution with Table Density and Distribution with Analytic Density - parametrical values are not applicable.
* :ref:`Translation <translation_page>` dialog box, default mode (translation by two points) - parametrical values are correctly applied, but they are not restored after "Update study" operation.
* :ref:`Merging nodes <merging_nodes_page>` dialog box - parametrical value (tolerance of coincident nodes detection) is correctly applied, but it is not restored after "Update study" operation.
* :ref:`Revolution <revolution_page>` dialog box - it is impossible to use the angle of revolution as "total angle" if it is defined as variable.
* :ref:`Extrusion along a path <extrusion_along_path_page>` dialog box - it is impossible to use "Linear variation of the angles" mode if at least one of those angles is defined as variable.
* :ref:`Pattern mapping <pattern_mapping_page>` dialog box - parametrical values (indices of nodes) are correctly applied, but they are not restored after "Update study" operation.
* :ref:`Clipping <clipping_page>` dialog box.
* :ref:`Properties <colors_size_page>` dialog box.


Our **TUI Scripts** provide you with useful examples of :ref:`Using SALOME NoteBook <tui_notebook_smesh>`.


