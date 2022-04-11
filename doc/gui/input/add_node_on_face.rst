.. _add_node_on_face_page:

*****************************
Cutting a face into triangles
*****************************

This operation cuts a face into triangles by adding a node on the face and connecting the new node with face nodes.

   .. image:: ../images/add_node_on_face.png
      :align: center

*To cut a face:*

.. |img| image:: ../images/mesh_split_face_interactive.png

#. Select a mesh and display it in the 3D Viewer.
#. In the **Modification** menu select the **Add node to triangle** item or click *"Add node to triangle"* button |img| in the toolbar.

   The following dialog box will appear:

      .. image:: ../images/add_node_on_face-dlg.png
         :align: center
                  
                  
#. Enter an **ID** of the face to split either by picking it in the 3D viewer or by typing its ID.
   
#. Press *Selection* button in **Node location by mouse click** group to activate specifying location of a new node. You can specify it

       * by clicking with your mouse on the face in the Viewer,
       * by typing coordinates in **X, Y, Z** fields,
       * by setting coordinates using arrows of spin boxes.

#. Activate **Preview** to see a result of the operation.

#. Click the **Apply** or **Apply and Close** button to confirm the operation.

**See Also** a sample TUI Script of a :ref:`tui_cutting_triangles` operation.  
