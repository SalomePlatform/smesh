.. _building_compounds_page:

************************
Building Compound Meshes
************************

Compound Mesh is a combination of several meshes. All elements and groups present in input meshes are present in the compound mesh. However, it does not use geometry or hypotheses of the initial meshes. 
The links between the input meshes and the compound mesh are not supported, consequently the modification of an input mesh does not lead to the update of the compound mesh.

**To Build a compound mesh:**

From the **Mesh** menu select **Build Compound** or click **"Build Compound Mesh"** button in the toolbar.

	.. image:: ../images/image161.png
		:align: center

**"Build Compound Mesh" button**


The following dialog box will appear:

	.. image:: ../images/buildcompound.png
		:align: center

	* **Name** - allows selecting the name of the resulting **Compound** mesh.
	* **Meshes, sub-meshes, groups** - allows selecting the meshes, sub-meshes and groups to be concatenated. They can be chosen in the Object Browser while holding **Ctrl** button.
	* **Processing identical groups** - allows selecting the method of processing the namesake groups existing in the input meshes. They can be either 

		* **United** - all elements of **Group1** of **Mesh_1** and **Group1** of **Mesh_2** become the elements of **Group1** of the **Compound_Mesh**, or
		* **Renamed** - **Group1** of **Mesh_1** becomes **Group1_1** and **Group1** of **Mesh_2** becomes **Group1_2**.
    
    		See :ref:`grouping_elements_page` for more information about groups.
	* **Create groups from input objects** check-box permits to automatically create groups corresponding to every initial mesh.

	.. image:: ../images/buildcompound_groups.png
		:align: center

	.. centered:: 
		"Groups created from input meshes 'Box_large' and 'Box_small'"

	* You can choose to additionally :ref:`merging_nodes_page`, :ref:`merging_elements_page` in the compound mesh, in which case it is possible to define the **Tolerance** for this operation.

	.. image:: ../images/image160.gif
		:align: center

	.. centered::
		"Example of a compound of two meshed cubes"

**See Also** a sample :ref:`tui_building_compound`.
