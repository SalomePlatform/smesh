.. _taper_page:

*****
Taper
*****

**Taper** mesh quality criterion represents the ratio of the areas
of two triangles separated by a diagonal within a quadrilateral face.

::

	**JA = 0.25 * (A1 + A2 + A3 + A4)**
	**TAPER = MAX(|A1/JA - 1|, |A2/JA - 1|, |A3/JA - 1|, |A4/JA - 1|)**

*To apply the Taper quality criterion to your mesh:*

.. |img| image:: ../images/image36.png

#. Display your mesh in the viewer.
#. Choose **Controls > Face Controls > Taper** or click *"Taper"* button |img| in the toolbar.


   Your mesh will be displayed in the viewer with its elements colored according to the applied mesh quality control criterion:

	.. image:: ../images/image90.jpg
		:align: center

**See Also** a sample TUI Script of a :ref:`tui_taper` filter.

