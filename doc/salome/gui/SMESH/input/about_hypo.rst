.. _about_hypo_page: 

****************
About Hypotheses
****************

**Hypotheses** represent boundary conditions which will be taken into account by meshing algorithms. The hypotheses allow you to manage the level of detail of the resulting mesh: when applying different hypotheses with different parameters you can preset the quantity or size of elements which will compose your mesh. So, it will be possible to generate a coarse or a more refined mesh.

The choice of a hypothesis depends on the selected algorithm.

Hypotheses are created during creation and edition of 
:ref:`constructing_meshes_page`:"meshes" and
:ref:`constructing_submeshes_page`:"sub-meshes". 
Once created a hypotheses can be reused during creation and edition of other meshes and sub-meshes. All created hypotheses and algorithms are present in the Object Browser in *Hypotheses* and *Algorithms* folders correspondingly. It is possible to open a dialog to modify the parameters of a hypothesis from its context menu. This menu also provides  **Unassign** command that will unassign the hypothesis from all meshes and sub-meshes using it. Modification of any parameter of a hypothesis and its unassignment leads to automatic removal of elements generated using it.

In  **MESH** there are the following Basic Hypotheses:

* :ref:`a1d_meshing_hypo_page` (for meshing of **edges**):
	* :ref:`number_of_segments_anchor`
	* :ref:`average_length_anchor`
	* :ref:`max_length_anchor`
	* :ref:`adaptive_1d_anchor`
	* :ref:`arithmetic_1d_anchor`
	* :ref:`geometric_1d_anchor`
	* :ref:`start_and_end_length_anchor`
	* :ref:`deflection_1d_anchor`
	* :ref:`automatic_length_anchor`
	* :ref:`fixed_points_1d_anchor`

* :ref:`a2d_meshing_hypo_page` (for meshing of **faces**):

	* :ref:`max_element_area_anchor`
	* :ref:`length_from_edges_anchor`
	* :ref:`hypo_quad_params_anchor`

* 3D Hypothesis (for meshing of **volumes**):

	* :ref:`max_element_volume_hypo_page`



There also exist :ref:`additional_hypo_page`:

	* :ref:`propagation_anchor`
	* :ref:`propagofdistribution_anchor`
	* :ref:`viscous_layers_anchor`
	* :ref:`quadratic_mesh_anchor`
	* :ref:`quadrangle_preference_anchor`


.. toctree::
	:maxdepth: 2

	1d_meshing_hypo.rst
	2d_meshing_hypo.rst
	max_element_volume_hypo.rst
	additional_hypo.rst	

