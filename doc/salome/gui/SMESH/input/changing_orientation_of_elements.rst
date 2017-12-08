.. _changing_orientation_of_elements_page:

********************************
Changing orientation of elements
********************************

Orientation of an element is changed by changing the order of its nodes.

**To change orientation of elements:**

#. Select a mesh (and display it in the 3D Viewer if you are going to pick elements by mouse).
#. In the **Modification** menu select the **Orientation** item or click **Orientation** button in the toolbar.

	.. image:: ../images/image79.png
		:align: center

	.. centered::
		**"Orientation" button**

	The following dialog box will appear:

	.. image:: ../images/orientaation1.png
		:align: center
    
	* Select type of elements to reorient: **Face** or **Volume**.
	* **The main list** shall contain the elements which will be reoriented. You can click on an element in the 3D viewer and it will be highlighted. After that click the **Add** button and the ID of this element will be added to the list. To remove a selected element or elements from the list click the **Remove** button. The **Sort** button allows to sort the list of elements IDs. The **Set filter** button allows to apply a definite :ref:`filtering_elements` "filter" to the selection of elements.
	* **Apply to all** radio button allows to modify the orientation of all elements of the selected mesh.
	* *Select from** set of fields allows to choose a sub-mesh or an existing group whose elements can be added to the list.
  
#. Click the **Apply** or **Apply and Close** button to confirm the operation.

**See Also** a sample TUI Script of a :ref:`tui_orientation` operation.

