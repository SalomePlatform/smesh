.. _building_compounds_page:

************************
Building Compound Meshes
************************

Compound Mesh is a combination of several mesh objects (meshes, groups, submeshes). All elements and groups present in input meshes are present in the compound mesh. However, it does not use geometry or hypotheses of the initial mesh objects.
The links between the input meshes and the compound mesh are not supported, consequently the modification of an input mesh does not lead to the update of the compound mesh.

There are two modes of building a compound:

* joining selected mesh objects into a new mesh.
* appending selected mesh objects to an existing mesh.

*To Build a compound mesh:*

.. |img| image:: ../images/image161.png

From the **Mesh** menu select **Build Compound** or click *"Build Compound Mesh"* button |img| in the toolbar. The following dialog box will appear:

	.. image:: ../images/buildcompound.png
		:align: center

* **Result** group allows selecting a mode of operation

  * Activating **Create new mesh named** enables typing the name of the resulting compound mesh.
  * Activating **Append to mesh** enables selection of a mesh to append other selected objects to.
   
* **Meshes, sub-meshes, groups** allows selecting the meshes, sub-meshes and groups to be concatenated. They can be chosen in the Object Browser while holding *Ctrl* button.
* **Processing identical groups** allows selecting the method of processing the namesake groups existing in the input meshes. They can be either 

  * **United** - all elements of *Group1* of *Mesh_1* and *Group1* of *Mesh_2* become the elements of *Group1* of the *Compound_Mesh*, or
  * **Renamed** - *Group1* of *Mesh_1* becomes *Group1_1* and *Group1* of *Mesh_2* becomes *Group1_2*.
    
  See :ref:`grouping_elements_page` for more information about groups.
* **Create groups from input objects** check-box permits to automatically create groups corresponding to every initial mesh.

	.. image:: ../images/buildcompound_groups.png
		:align: center

	.. centered:: 
		Groups created from input meshes 'Box_large' and 'Box_small'

* You can choose to additionally :ref:`Merge coincident nodes <merging_nodes_page>` :ref:`and elements <merging_elements_page>` in the compound mesh, in which case it is possible to define the **Tolerance** for this operation.

	.. image:: ../images/image160.gif
		:align: center

	.. centered::
		Example of a compound of two meshed cubes

**See Also** a sample script of :ref:`tui_building_compound`.
