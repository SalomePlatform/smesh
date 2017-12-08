.. _split_to_tetra_page:

*****************
Splitting volumes
*****************

This operation allows to split either any volumic elements into
tetrahedra or hexahedra into prisms. 2D mesh is modified accordingly.

**To split volumes:**

#. Select a mesh, a sub-mesh or a group.
#. In the **Modification** menu select the **Split Volumes** item or click **"Split Volumes"** button in the toolbar.

	.. image:: ../images/split_into_tetra_icon.png
		:align: center

	.. centered:: 
		**"Split Volumes" button**

	The following dialog box will appear:

	.. image:: ../images/split_into_tetra.png
		:align: center                                                 


	First it is possible to select the type of operation:

	* If **Tetrahedron** button is checked, the operation will split volumes of any type into tetrahedra.
	* If **Prism** button is checked, the operation will split hexahedra into prisms.
	* The main list contains the list of volumes to split. You can click on a volume in the 3D viewer and it will be highlighted (lock Shiftkeyboard button to select several volumes). Click **Add** button and the ID of this volume will be added to the list. To remove the selected element or elements from the list click **Remove** button. **Sort list** button allows to sort the list of IDs. **Filter** button allows applying a filter to the selection of volumes.
		**Note:** If you split not all adjacent non-tetrahedral volumes, your mesh becomes non-conform. 
  
	* **Apply to all** radio button allows splitting all volumes of the currently selected mesh.
	* If **Tetrahedron** element type is selected, **Split hexahedron** group allows specifying the number of tetrahedra  a hexahedron will be split into. If the chosen method does not allow to get a conform mesh, a generic solution is applied: an additional node is created at the gravity center of a hexahedron, serving an apex of tetrahedra, all quadrangle sides of the hexahedron are split into two triangles each serving a base of a new tetrahedron. 

	* If **Prism** element type is selected, the **Split hexahedron** group looks as follows:

		.. image:: ../images/split_into_prisms.png
			:align: center                                                 

		* **Into 2 (or 4) prisms** allows to specify the number of prisms a hexahedron will be split into. 
		* **Facet to split** group allows to specify the side (facet) of the hexahedron, which is split into triangles.  This facet is defined by a point and a direction. The algorithm finds a hexahedron closest to the specified point and splits a facet whose normal is closest to the specified direction. Then the splitting is propagated from that hexahedron to all adjacent hexahedra. The point and the direction by which the first split hexahedron is found can be specified:

			* by input of coordinates in **Hexa location** and **Facet normal** fields, or 
			* by clicking **Selection** button and selecting in the viewer the element whose barycenter will be used as the start point and whose direction will be used as a normal to facet to split into triangles. Switch this button off to return to selection of volumes to split.

    
		* If **All domains** option is off, the operation stops when all hehexedra adjacent to the start hexahedron are split into prisms. Else the operation tries to continue splitting starting from  another hexahedron closest to the **Hexa location**. 

	* **Select from** set of fields allows choosing a sub-mesh or an existing group whose elements will be added to the list as you click **Add** button.


#. Click **Apply** or **Apply and Close** button to confirm the operation.


