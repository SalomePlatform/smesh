.. _about_hypo_page: 

****************
About Hypotheses
****************

**Hypotheses** represent boundary conditions which will be taken into account by meshing algorithms. The hypotheses allow you to manage the level of detail of the resulting mesh: when applying different hypotheses with different parameters you can preset the quantity or size of elements which will compose your mesh. So, it will be possible to generate a coarse or a more refined mesh.

The choice of a hypothesis depends on the selected algorithm.

Hypotheses are created during creation and edition of 
:ref:`meshes <constructing_meshes_page>` and :ref:`sub-meshes <constructing_submeshes_page>`. 
Once created a hypotheses can be reused during creation and edition of other meshes and sub-meshes. All created hypotheses and algorithms are present in the Object Browser in *Hypotheses* and *Algorithms* folders correspondingly. It is possible to open a dialog to modify the parameters of a hypothesis from its context menu. This menu also provides  **Unassign** command that will unassign the hypothesis from all meshes and sub-meshes using it. Modification of any parameter of a hypothesis and its unassignment leads to automatic removal of elements generated using it.

In **MESH** there are:

* :ref:`a1d_meshing_hypo_page` for meshing of **edges**
* :ref:`a2d_meshing_hypo_page` for meshing of **faces**
* :ref:`a3d_meshing_hypo_page` for meshing of **volumes**
* :ref:`additional_hypo_page`

**Table of Contents**

.. toctree::
   :maxdepth: 2

   1d_meshing_hypo.rst
   2d_meshing_hypo.rst
   3d_meshing_hypo.rst
   additional_hypo.rst	

