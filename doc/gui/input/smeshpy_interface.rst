.. _smeshpy_interface_page:

****************
Python interface
****************

Python API of SALOME Mesh module defines several classes that can
be used for easy mesh creation and edition.

Documentation of SALOME Mesh module Python API is available in two forms:

- :doc:`Structured documentation <modules>`, where all methods and classes are grouped by their functionality.

- :ref:`Linear documentation <modindex>` grouped only by classes, declared in the :mod:`smeshBuilder` Python module.

With SALOME 7.2, the Python interface for Mesh has been slightly modified to offer new functionality.
You may have to modify your scripts generated with SALOME 6 or older versions.
Please see :ref:`smesh_migration_page`.

Class :class:`smeshBuilder.smeshBuilder` provides an interface to create and handle
meshes. It can be used to create an empty mesh or to import mesh from the data file.

As soon as a mesh is created, it is possible to manage it via its own
methods, described in class :class:`smeshBuilder.Mesh` documentation.

Class :class:`smeshstudytools.SMeshStudyTools` provides several methods to manipulate mesh objects in Salome study.

A usual workflow to generate a mesh on geometry is following:

#. Create an instance of :class:`smeshBuilder.smeshBuilder`:
	.. code-block:: python

		from salome.smesh import smeshBuilder
		smesh = smeshBuilder.New()

#. Create a :class:`smeshBuilder.Mesh` object:

	.. code-block:: python

		mesh = smesh.Mesh( geometry )

#. Create and assign :ref:`algorithms <basic_meshing_algos_page>` by calling corresponding methods of the mesh. If a sub-shape is provided as an argument, a :ref:`sub-mesh <constructing_submeshes_page>` is implicitly created on this sub-shape:
	.. code-block:: python

		regular1D  = mesh.Segment()
		quadrangle = mesh.Quadrangle()
		# use other 2D algorithm on a face -- a sub-mesh appears in the mesh
		triangle   = mesh.Triangle( face )

#. Create and assign :ref:`hypotheses <about_hypo_page>` by calling corresponding methods of algorithms:
	.. code-block:: python

		regular1D.LocalLength( 10. )
		quadrangle.Reduced()
		triangle.SetMaxSize( 20. )
		triangle.SetFineness( smeshBuilder.VeryCoarse )

#. :ref:`Compute the mesh <compute_anchor>` (generate mesh nodes and elements):
	.. code-block:: python

		mesh.Compute()

An easiest way to start with Python scripting is to do something in
GUI and then to get a corresponding Python script via
**File > Dump Study** menu item. Don't forget that you can get
all methods of any object in hand (e.g. a mesh group or a hypothesis)
by calling *dir()* Python built-in function.

All methods of the :class:`Mesh Group <SMESH.SMESH_GroupBase>` can be found in :ref:`tui_create_standalone_group` sample script.

An example below demonstrates usage of the Python API for 3D mesh
generation and for retrieving basic information on mesh nodes, elements and groups.

.. _example_3d_mesh:

Example of 3d mesh generation:
##############################

.. literalinclude:: ../../examples/mesh_3d.py
    :language: python

:download:`Download this script <../../examples/mesh_3d.py>`

Examples of Python scripts for Mesh operations are available by
the following links:

.. toctree::
   :titlesonly:

   tui_creating_meshes
   tui_defining_hypotheses
   tui_grouping_elements
   tui_filters
   tui_modifying_meshes
   tui_transforming_meshes
   tui_viewing_meshes
   tui_quality_controls
   tui_adaptation
   tui_measurements
   tui_work_on_objects_from_gui
   tui_notebook_smesh

.. toctree::
   :hidden:

   smesh_migration.rst
   smeshBuilder.rst
   StdMeshersBuilder.rst
   smeshstudytools.rst
   modules.rst
   smesh_module.rst
