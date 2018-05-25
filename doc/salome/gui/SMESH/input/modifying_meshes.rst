.. _modifying_meshes_page:

****************
Modifying meshes
****************

Salome provides a vast specter of mesh modification and transformation operations, giving the possibility to:

* :ref:`Add <adding_nodes_and_elements_page>` mesh elements from nodes to polyhedrons at an arbitrary place in the mesh.
* :ref:`Add quadratic <adding_quadratic_elements_page>` mesh elements from quadratic segments to quadratic hexahedrons at an arbitrary place in the mesh.
* :ref:`Remove <removing_nodes_and_elements_page>` any existin" mesh elements and nodes.
* :ref:`Translate <translation_page>` in the indicated direction the mesh or some of its elements.
* :ref:`Rotate <rotation_page>` by the indicated axis and angle the mesh or some of its elements.
* :ref:`Scale <scale_page>` the mesh or some of its elements.
* :ref:`Mirror <symmetry_page>` the mesh through a point, a vector or a plane of symmetry.
* :ref:`double_nodes_page`. Duplication of nodes can be useful to emulate a crack in the model.
* Unite meshes by :ref:`sewing <sewing_meshes_page>` free borders, border to side or side elements.
* :ref:`Merge Nodes<merging_nodes_page>` coincident within the indicated tolerance.
* :ref:`Merge Elements <merging_elements_page>` based on the same nodes.
* :ref:`Move Nodes <mesh_through_point_page>` to an arbitrary location with consequent transformation of all adjacent elements.
* :ref:`Invert an edge <diagonal_inversion_of_elements_page>` between neighboring triangles.
* :ref:`Unite two triangles <uniting_two_triangles_page>`.
* :ref:`Unite several adjacent triangles <uniting_set_of_triangles_page>`.
* :ref:`Change orientation <changing_orientation_of_elements_page>` of the selected elements.
* :ref:`Orient faces <reorient_faces_page>` by several means.
* :ref:`Cut a quadrangle <cutting_quadrangles_page>` into two triangles.
* :ref:`Split <split_to_tetra_page>` volumic elements into tetrahedra or prisms.
* :ref:`Split bi-quadratic <split_biquad_to_linear_page>` elements into linear ones without creation of additional nodes.
* :ref:`Smooth <smoothing_page>` elements, reducung distortions in them by adjusting the locations of nodes.
* Create an :ref:`extrusion <extrusion_page>` along a vector or by normal to a discretized surface.
* Create an :ref:`extrusion along a path <extrusion_along_path_page>`.
* Create elements by :ref:`revolution <revolution_page>` of the selected nodes and elements.
* Apply :ref:`pattern mapping <pattern_mapping_page>`.
* :ref:`Convert linear mesh to quadratic <convert_to_from_quadratic_mesh_page>`, or vice versa.
* :ref:`Generate boundary elements <make_2dmesh_from_3d_page>`.
* :ref:`generate_flat_elements_page`.
* :ref:`cut_mesh_by_plane_page`.


.. note::
	It is possible to :ref:`modify the mesh <edit_anchor>` of a lower dimension before generation of the mesh of a higher dimension.

.. note::
	It is possible to use the variables defined in the SALOME **NoteBook** to specify the numerical parameters used for modification of any object.


**Table of Contents**

.. toctree::
   :titlesonly:

   adding_nodes_and_elements.rst
   adding_quadratic_elements.rst 
   removing_nodes_and_elements.rst 
   translation.rst
   rotation.rst
   scale.rst
   symmetry.rst
   double_nodes_page.rst 
   sewing_meshes.rst
   merging_nodes.rst
   merging_elements.rst
   mesh_through_point.rst 
   diagonal_inversion_of_elements.rst 
   uniting_two_triangles.rst 
   uniting_set_of_triangles.rst
   changing_orientation_of_elements.rst
   reorient_faces.rst
   cutting_quadrangles.rst
   split_to_tetra.rst
   split_biquad_to_linear.rst
   smoothing.rst
   extrusion.rst
   extrusion_along_path.rst
   revolution.rst
   pattern_mapping.rst
   convert_to_from_quadratic_mesh.rst
   make_2dmesh_from_3d.rst
   generate_flat_elements.rst
   cut_mesh_by_plane.rst

