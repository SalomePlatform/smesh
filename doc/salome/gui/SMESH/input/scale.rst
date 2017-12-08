.. _scale_page:

*****
Scale
*****

This geometrical operation allows to scale in space your mesh or some of its elements.

**To scale a mesh:**

#. From the **Modification** menu choose **Transformation** -> **Scale Transform** item.
	One of the following dialogs will appear:

	With one scale factor:

	.. image:: ../images/scale01.png
		:align: center

	Or with different scale factors for axes:

	.. image:: ../images/scale02.png
		:align: center

#. In the dialog:
	* specify the IDs of the translated elements:
		* **Select the whole mesh, submesh or group** activating this checkbox; or
		* choose mesh elements with the mouse in the 3D Viewer. It is possible to select a whole area with a mouse frame; or 
		* input the element IDs directly in **ID Elements** field. The selected elements will be highlighted in the viewer; or
		* apply Filters. **Set filter** button allows to apply a filter to the selection of elements. See more about filters in the :ref:`selection_filter_library_page` page.

	* specify the base point for scale

	* specify the scale factor

	* specify the conditions of scale:

		* activate **Move elements** radio button to scale the selected mesh (or elements) without creating a copy;
		* activate **Copy elements** radio button to duplicate the selected mesh (or elements) and to apply scaling to the copy within the same mesh;
		* activate **Create as new mesh** radio button to leave the selected mesh (or elements) at its previous location and create a new mesh of the scaled copy of the selected elements; the new mesh appears in the Object Browser with the default name MeshName_scaled (it is possible to change this name in the adjacent box);
		* activate **Copy groups** checkbox to copy the groups of elements existing in the source mesh to the newly created mesh.

	* activate **Preview** checkbox to show the result of transformation in the viewer 
	* click **Apply** or **Apply and Close** button to confirm the operation.


**Example of using:**

#. Create quandrangle mesh 3x3 on a simple planar face (200x200)

	.. image:: ../images/scaleinit01.png
		:align: center

	and union 3 faces (along axis Z) to group "gr_faces"

	.. image:: ../images/scaleinit02.png
		:align: center



#. Perform scale operation for the whole mesh and create a new mesh:

	.. image:: ../images/scale03.png
		:align: center

	result after operation:

	.. image:: ../images/scaleres03.png
		:align: center

#. Perform scale operation for the whole mesh and copy elements:

	.. image:: ../images/scale04.png
		:align: center

	result after operation:

	.. image:: ../images/scaleres04.png
		:align: center

#. Perform scale operation for a group of faces and copy elements:

	.. image:: ../images/scale06.png
		:align: center

	result after operation:

	.. image:: ../images/scaleres06.png
		:align: center



#. Perform scale operation for two edges and move elements:

	.. image:: ../images/scale07.png
		:align: center

	result after operation:

	.. image:: ../images/scaleres07.png
		:align: center



#. Perform scale operation for one face and move elements:

	.. image:: ../images/scale09.png
		:align: center

	result after operation:

	.. image:: ../images/scaleres09.png
		:align: center


**See Also** a sample TUI Script of a :ref:`tui_scale` operation.  



