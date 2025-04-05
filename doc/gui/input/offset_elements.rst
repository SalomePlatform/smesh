.. _offset_elements_page:

********************************
Offset mesh 
********************************

Offset 2D elements in mesh, submesh, group or elements from filter.

*To offset all elements from mesh, sub-mesh or group:*

.. |img| image:: ../images/image81.png

#. In the **Modification** menu, select the **Transformations** then click *Offset* button |img| or click directly in the same button in the toolbar.

   The following dialog box will appear:

	.. image:: ../images/offset_mesh.png
		:align: center
    
	* Check the option **Select whole mesh, sub-mesh or group** if it is not set.
	* Set the name of the mesh, sub-mesh or created group.
	* Set the **Offset** value (negative offsets are allowed).
	* Select **Move Elements** to offset the current mesh, **Copy Elements** to offset and duplicate them, or **Create a new mesh** to create a new mesh from the offset elements.

#. Click the **Apply** or **Apply and Close** button to confirm the operation.


*To offset all elements ids from a filter:*

#. In the **Modification** menu, select the **Transformations** then click *Offset* button |img| or click directly in the same button from the toolbar.
	
	* Uncheck the option **Select whole mesh, sub-mesh or group**.

	.. image:: ../images/offset_mesh_from_filter.png
		:align: center
	
	* Then click on **Set_Filter** button
	
	The dialog for filter for faces will appear:

	.. image:: ../images/offset_filter_for_faces_gui.png
		:align: center
	
	* Define the desired criteria and then click the **Apply and Close** button to confirm the selection of elements.
	* Set **Offset** value and mesh creation **options** as done for the first case.

#. Click the **Apply** or **Apply and Close** button to confirm the operation.

**See also:** A sample TUI Script of a :ref:`tui_offset_elements` transformation.