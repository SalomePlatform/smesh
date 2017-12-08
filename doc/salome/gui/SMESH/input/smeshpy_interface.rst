.. _smeshpy_interface_page:

****************
Python interface
****************

Python API of SALOME Mesh module defines several classes that can
be used for easy mesh creation and edition.

Documentation of SALOME %Mesh module Python API is available in two forms:

- :ref:`Structured documentation <modules_page>`, where all methods and classes are grouped by their functionality.

- :ref:`Linear documentation <genindex>` grouped only by classes, declared in the :mod:`smeshBuilder` and :mod:`StdMeshersBuilder` Python packages.

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
		:linenos:

		from salome.smesh import smeshBuilder
		smesh = smeshBuilder.New( salome.myStudy )

#. Create a :class:`smeshBuilder.Mesh` object:

	.. code-block:: python
		:linenos:

		mesh = smesh.Mesh( geometry )

#. Create and assign :ref:`basic_meshing_algos_page` by calling corresponding methods of the mesh. If a sub-shape is provided as an argument, a :ref:`constructing_submeshes_page` is implicitly created on this sub-shape:
	.. code-block:: python
		:linenos:

		regular1D = smeshBuilder.Mesh.Segment()
		mefisto   = smeshBuilder.Mesh.Triangle( smeshBuilder.MEFISTO )
		# use other triangle algorithm on a face -- a sub-mesh appears in the mesh
		netgen    = smeshBuilder.Mesh.Triangle( smeshBuilder.NETGEN_1D2D, face )

#. Create and assign :ref:`about_hypo_page` by calling corresponding methods of algorithms:
	.. code-block:: python
		:linenos:

		segLen10 = StdMeshersBuilder.StdMeshersBuilder_Segment.LocalLength( 10. )
		maxArea  = StdMeshersBuilder.StdMeshersBuilder_Segment.LocalLength( 100. )
		netgen.SetMaxSize( 20. )
		netgen.SetFineness( smeshBuilder.VeryCoarse )
  
#. :ref:`compute_anchor` the mesh (generate mesh nodes and elements):
	.. code-block:: python
		:linenos:

		Mesh.Compute()

An easiest way to start with Python scripting is to do something in
GUI and then to get a corresponding Python script via 
**File > Dump Study** menu item. Don't forget that you can get
all methods of any object in hand (e.g. a mesh group or a hypothesis)
by calling *dir()* Python built-in function.

All methods of the Mesh Group can be found in :ref:`tui_create_standalone_group` sample script.

An example below demonstrates usage of the Python API for 3d mesh
generation and for retrieving information on mesh nodes and elements.

.. _example_3d_mesh:

Example of 3d mesh generation:
##############################

.. _3dmesh.py:

``3dmesh.py`` 


.. literalinclude:: ../../../examples/3dmesh.py
    :linenos:
    :language: python

:download:`../../../examples/3dmesh.py`

Examples of Python scripts for Mesh operations are available by
the following links:

- :ref:`tui_creating_meshes_page`
- :ref:`tui_defining_hypotheses_page`
- :ref:`tui_grouping_elements_page`
- :ref:`tui_filters_page`
- :ref:`tui_modifying_meshes_page`
- :ref:`tui_transforming_meshes_page`
- :ref:`tui_viewing_meshes_page`
- :ref:`tui_quality_controls_page`
- :ref:`tui_measurements_page`
- :ref:`tui_work_on_objects_from_gui`
- :ref:`tui_notebook_smesh_page`
- :ref:`tui_cartesian_algo`
- :ref:`tui_use_existing_faces`
- :ref:`tui_prism_3d_algo`
- :ref:`tui_generate_flat_elements_page`



.. toctree::
	:maxdepth: 2

	smesh_migration.rst
