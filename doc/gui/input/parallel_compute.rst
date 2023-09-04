.. _parallel_compute_page:

******************
Parallel Computing
******************


.. warning::
  This functionality is a work in progress.

  It is only available for NETGEN.

  It is only available in TUI.


The goal here is to speed up computation by running sub-meshes in parallel
(multi-threading).

*******
Concept
*******

.. image:: ../images/diagram_parallel_mesh.png

In order to parallelise the computation of the mesh we split the geometry into:

  * A 1D+2D compound
  * A list of 3D solids

Then create a sub-mesh for each of those geometry.
And associate Hypothesis to the mesh using a hypothesis on the whole geometry

We will first compute sequentially the 1D+2D compound with NETGEN_1D2D.

Then we will compute all the solids in parallel. Having done the 1D+2D first
ensure that all the solids can be computed without any concurrency.


******
How to
******

You follow the same principle as the creation of a sequential Mesh.


#. First you create the mesh:
	.. code-block:: python

		par_mesh = smesh.ParallelMesh(geom, name="par_mesh")

#. Define the Global Hypothesis that will be split into an hypothesis for the
   1D+2D compound and one for each of the 3D solids:
	.. code-block:: python

		NETGEN_3D_Parameters_1 = smesh.CreateHypothesisByAverageLength( 'NETGEN_Parameters',
                                                 'NETGENEngine', 34.641, 0 )
		par_mesh.AddGlobalHypothesis(netgen_parameters)

#. Set the parameters for the parallelisation:
	.. code-block:: python

		param = par_mesh.GetParallelismSettings()
		param.SetNbThreads(6)

#. Compute the mesh:
	.. code-block:: python

		mesh.Compute()

**See Also** a sample script of :ref:`tui_create_parallel_mesh`.
