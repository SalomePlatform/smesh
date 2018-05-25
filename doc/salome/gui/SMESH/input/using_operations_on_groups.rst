.. _using_operations_on_groups_page: 

****************************
Boolean operations on groups
****************************

In MESH you can perform some Boolean operations on groups, which belong to one and the same mesh.

* :ref:`union_anchor`
* :ref:`intersection_anchor`
* :ref:`cut_anchor` 




.. _union_anchor:

Union of groups
###############

This operation allows to create a new group in such a way that all
mesh elements that are present in the initial groups will be added to
the new one.

*To union groups:*

#. In the **Mesh** menu select the **Union Groups** item. The following dialog box will appear:

	.. image:: ../images/uniongroups.png
		:align: center                                                 

   In this dialog box you should specify the name of the resulting group and set of groups which will be united.

   For example, we have two groups Group1 and Group2. 
   The result of their **Union** will be Group12:

	.. image:: ../images/image133.gif
		:align: center

	.. centered::
		Group1

	.. image:: ../images/image134.gif
		:align: center

	.. centered::
		Group2

	.. image:: ../images/image135.gif
		:align: center

	.. centered::
		Group12

#. Click the **Apply** or **Apply and Close** button to confirm creation of the group.


**See Also** a sample TUI Script of a :ref:`tui_union_of_groups` operation.


.. _intersection_anchor:

Intersection of groups
######################

This operation allows to create a new group in such a way that all
mesh elements that are present in all initial groups together are added to the
new one.

*To intersect groups:*

#. In the **Mesh** menu select the **Intersect Groups** item. The following dialog box will appear:

	.. image:: ../images/intersectgroups.png
		:align: center                                                 

   In this dialog box you should specify the name of the resulting group and set of groups which will be intersected.

   For example, we have two groups Group1 and Group2. 
   The result of their **Intersection** will be Group12a:

	.. image:: ../images/image133.gif
		:align: center

	.. centered::
		Group1

	.. image:: ../images/image134.gif
		:align: center

	.. centered::
		Group2

	.. image:: ../images/image136.gif
		:align: center

	.. centered::
		Group12a

#. Click the **Apply** or **Apply and Close** button to confirm creation of the group.


**See Also** a sample TUI Script of an :ref:`tui_intersection_of_groups` operation.  


.. _cut_anchor:

Cut of groups
#############

This operation allows to create a new group in such a way that all
mesh elements that are present in the main groups but are absent in the
tool groups are added to the new one.

*To cut groups:*

#. In the **Mesh** menu select the **Cut Groups** item. The following dialog box will appear:

	.. image:: ../images/cutgroups.png
		:align: center                                                 

   In this dialog box you should specify the name of the resulting group and groups which will be cut.

   For example, we have two groups Group1 and Group2. 
   The result of their **Cut** will be Group12b:

	.. image:: ../images/image133.gif
		:align: center

	.. centered::
		Group1

	.. image:: ../images/image134.gif
		:align: center

	.. centered::
		Group2

	.. image:: ../images/image137.gif
		:align: center

	.. centered::
		Group12b

#. Click the **Apply** or **Apply and Close** button to confirm creation of the group.


**See Also** a sample TUI Script of a :ref:`tui_cut_of_groups` operation.  


