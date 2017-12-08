.. _translation_page:

***********
Translation
***********

This geometrical operation allows to translate in space your mesh
or some of its elements.

**To translate a mesh:**


#. From the **Modification** menu choose **Transformation** -> **Translation** item  or click **"Translation"** button in the toolbar.

	.. image:: ../images/translation.png 
		:align: center
	.. centered::
		"Translation button"

	One of the following dialogs will appear:

	It is possible to define the vector of thanslation:

	.. image:: ../images/translation1.png 
		:align: center

	.. centered::
		"by two points"

|

	.. image:: ../images/translation2.png 
		:align: center

	.. centered::
		"by the vector from the origin of coordinates" 



#. In the dialog:

	* specify the IDs of the translated elements:

		* **Select the whole mesh, submesh or group** activating this checkbox; or
		* choose mesh elements with the mouse in the 3D Viewer. It is possible to select a whole area with a mouse frame; or 
		* input the element IDs directly in **ID Elements** field. The selected elements will be highlighted in the viewer; or
		* apply Filters. **Set filter** button allows to apply a filter to the selection of elements. See more about filters in the :ref:`selection_filter_library_page` page.

	* specify the vector of translation:

		* specify the cooordinates of the start and end **Points** of the vector of translation; or
		* specify the end point of the **Vector** of rotation starting at the origin of coordinates.

	* specify the conditions of translation:

		* activate **Move elements** radio button to create  the source mesh (or elements) at the new location and erase it from the previous location;
		* activate **Copy elements** radio button to create the source mesh (or elements) at the new location, but leave it at the previous location, the source mesh will be considered one and single mesh with the result of the rotation;
		* activate **Create as new mesh** radio button to leave the source mesh (or elements) at its previous location and create a new mesh at the new location, the new mesh appears in the Object Browser with the default name MeshName_rotated (it is possible to change this name in the adjacent box);
		* activate **Copy groups** checkbox to copy the groups of elements of the source mesh to the newly created mesh.

	* activate **Preview** checkbox to show the result of transformation in the viewer 
	* click **Apply** or **Apply and Close** button to confirm the operation.

**See Also** a sample TUI Script of a :ref:`tui_translation` operation.  


