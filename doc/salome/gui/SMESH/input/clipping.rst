.. _clipping_page:

********
Clipping
********

**Clipping** allows creating cross-section views (clipping planes) of your mesh.
It is available as a sub-item in the context menu of an active mesh in 3D Viewer.
To create a clipping plane, click on the **New** button in the dialog and choose how it is defined: by **Absolute** or **Relative** coordinates.

* **Absolute Coordinates**

  .. image:: ../images/Clipping_Absolute.png
	:align: center

  * **Base point** - allows defining the coordinates of the base point for the clipping plane.
     * **Reset** - returns the base point to the coordinate origin.
  * **Direction** - allows defining the orientation of the clipping plane.
     * **Invert** - allows selecting, which part of the object will be removed and which will remain after clipping.
    
* **Relative mode**  

  .. image:: ../images/Clipping_Relative.png
	:align: center

  * **Orientation** ( ||X-Y, ||X-Z or ||Y-Z).
  * **Distance** between the opposite extremities of the boundary box of selected objects, if it is set to 0.5 the boundary box is split in two halves.
  * **Rotation** (in angle degrees) **around X** (Y to Z) and **around Y** (X to Z) (depending on the chosen Orientation)

	.. image:: ../images/before_clipping_preview.png 
		:align: center

        .. centered:: 
           The preview plane and the cut object

The other parameters are available in both modes : 

* **OpenGL clipping** check-box allows choosing OpenGL native clipping, which clips the whole presentation. If it is unchecked, the clipping is done on the dataset i.e. only the visibility of separate mesh cells is changed (see the examples).
* The List contains **Meshes, sub-meshes and groups** to which the cross-section will be applied.
* **Select all** check-box allows to selecting and deselecting all available objects at once.
* **Show preview** check-box shows the clipping plane in the **3D Viewer**.
* **Auto Apply** check-box shows button is on, you can preview the cross-section in the **3D Viewer**.

It is also possible to interact with the clipping plane directly in 3D view using the mouse. 

To get an object clipped, click **Apply**.

**Examples:**

	.. image:: ../images/dataset_clipping.png
		:align: center

        .. centered:: 
           The cross-section using dataset

	.. image:: ../images/opengl_clipping.png
		:align: center

        .. centered:: 
           The OpenGL cross-section
