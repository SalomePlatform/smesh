.. _diagonal_inversion_of_elements_page:

***********************************
Diagonal inversion of two triangles
***********************************

In MESH you can inverse the diagonal (edge) of a pseudo-quadrangle
formed by two neighboring triangles with one common edge.

*To inverse the diagonal:*

#. From the **Modification** menu choose the **Diagonal inversion** item or click *"Diagonal Inversion"* |img| button in the toolbar.

.. |img| image:: ../images/image70.png

The following dialog box shall appear:

	.. image:: ../images/diagonalinversion.png
		  :align: center

#. Enter IDs of nodes forming the required edge in the **Edge** field (the node IDs must be separated by dashes) or select this edge in the 3D viewer.
#. Click the **Apply** or **Apply and Close** button.

	.. image:: ../images/image38.jpg 
		  :align: center

	.. centered::
		The selected edge

	.. image:: ../images/image36.jpg 
		  :align: center

	.. centered::
		The inverted edge

**See Also** a sample TUI Script of a :ref:`tui_diagonal_inversion` operation.  


