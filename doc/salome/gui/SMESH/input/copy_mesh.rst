.. _copy_mesh_page:

*********
Copy Mesh
*********

A mesh can be created by copying a part of or the whole other mesh.

*To make a copy of a mesh:*

.. |img| image:: ../images/copy_mesh_icon.png

From the contextual menu in the Object Browser of from the **Mesh** menu select **Copy Mesh** or click *"Copy Mesh"* button |img| in the toolbar.

The following dialog box will appear:

.. image:: ../images/copy_mesh_dlg.png
	:align: center


In the dialog:

* specify the part of mesh to copy:

  * **Select whole mesh, sub-mesh or group** by mouse activating this checkbox; or
  * choose mesh elements with the mouse in the 3D Viewer. It is possible to select a whole area with a mouse frame; or 
  * input the **Source Element IDs** directly in this field. The selected elements will be highlighted in the viewer; or
  * apply Filters. **Set filter** button allows to apply a filter to the selection of elements. See more about filters in the :ref:`selection_filter_library_page` page.

* specify the **New Mesh Name**;
* specify the conditions of copying:

  * activate **Generate groups** checkbox to copy the groups of the source mesh to the newly created mesh.

* Click **Apply** or **Apply and Close** button to confirm the operation.


**See Also** a sample script of :ref:`tui_copy_mesh`.

