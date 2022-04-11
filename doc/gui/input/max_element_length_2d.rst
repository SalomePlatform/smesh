.. _max_element_length_2d_page:

*******************
Element Diameter 2D
*******************

This quality control criterion consists in calculation of the maximal length of edges and diagonals of 2D mesh elements (triangles and quadrangles). For polygons the value is always zero.

*To apply the Element Diameter 2D quality criterion to your mesh:*

.. |img| image:: ../images/image42.png

#. Display your mesh in the viewer. 
#. Choose **Controls > Face Controls > Element Diameter 2D** or click *"Element Diameter 2D"* button |img| in the toolbar. 

   Your mesh will be displayed in the viewer with its elements colored according to the applied mesh quality control criterion:

	.. image:: ../images/max_element_length_2d.png
		:align: center

**See Also** a sample TUI Script of a :ref:`tui_max_element_length_2d` filter.
