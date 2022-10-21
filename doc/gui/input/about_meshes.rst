.. _about_meshes_page:

************
About meshes
************

**MESH** represents a discrete approximation of a subset of the three-dimensional space by `elementary geometrical elements`_.

A SALOME study can contain multiple meshes, but they do not implicitly compose one super-mesh, and finally each of them can be used (e.g. exported) only individually.

Mesh module provides several ways to create the mesh:

* The main way is to :ref:`construct the mesh <constructing_meshes_page>` on the basis of the geometrical shape produced in the Geometry module. This way implies selection of

  * a geometrical object (*main shape*) and
  * *meshing parameters* (:ref:`meshing algorithms <basic_meshing_algos_page>` and characteristics (e.g. element size) of a required mesh encapsulated in :ref:`hypothesis <about_hypo_page>` objects).

  Construction of :ref:`sub-meshes <constructing_submeshes_page>` allows to discretize some sub-shapes of the main shape, for example a face, using the meshing parameters that differ from those used for other sub-shapes.
  Meshing parameters of meshes and sub-meshes can be :ref:`edited <editing_meshes_page>`. (Upon edition only mesh entities generated using changed meshing parameters are removed and will be re-computed).

  .. note::
	Algorithms and hypotheses used at mesh level are referred to as *global* ones and those used at sub-mesh level are referred to as *local* ones.

* Bottom-up way, using :ref:`mesh modification <modifying_meshes_page>` operations, especially :ref:`extrusion <extrusion_page>` and :ref:`revolution <revolution_page>`. To create an empty mesh not based on geometry, use the same dialog as to :ref:`construct the mesh on geometry <constructing_meshes_page>` but specify neither the geometry nor meshing algorithms.

* The mesh can be :ref:`imported <importing_exporting_meshes_page>` from (and exported to) the file in MED, UNV, STL, CGNS, DAT and GMF formats.

* The 3D mesh can be generated from the 2D mesh not based on geometry, which was either :ref:`imported <importing_exporting_meshes_page>` or created in other way. To setup the meshing parameters of a mesh not based on geometry, just invoke :ref:`Edit mesh / sub-mesh <editing_meshes_page>` command on your 2D mesh.

* Several meshes can be :ref:`combined <building_compounds_page>` into a new mesh.

* The whole mesh or its part (sub-mesh or group) can be :ref:`copied <copy_mesh_page>` into a new mesh.

* A new mesh can be created from a transformed, e.g. :ref:`translated <translation_page>`, part of the mesh.

* A new mesh can be created from the gernation of the :ref:`dual <create_dual_mesh_page>` of a Tetrahedron Mesh.


Meshes can be edited using the MESH functions destined for :ref:`modification <modifying_meshes_page>` of meshes.

Attractive meshing capabilities include:

* 3D and 2D :ref:`Viscous Layers <viscous_layers_anchor>` (boundary layers of highly stretched elements beneficial for high quality viscous computations);
* automatic conformal transition between tetrahedral and hexahedral sub-meshes.

The **structure** of a SALOME mesh is described by nodes and elements based on these nodes. The geometry of an element is defined by the sequence of nodes constituting it and the :ref:`connectivity convention <connectivity_page>` (adopted from MED library). Definition of the element basing on the elements of a lower dimension is NOT supported.

.. _elementary geometrical elements:

The mesh can include the following entities:

* **Node** - a mesh entity defining a position in 3D space with coordinates (x, y, z).
* **Edge** (or segment) - 1D mesh element linking two nodes.
* **Face** - 2D mesh element representing a part of surface bound by links between face nodes. A face can be a triangle, quadrangle or polygon.
* **Volume** - 3D mesh element representing a part of 3D space bound by volume facets. Nodes of a volume describing each facet are defined by the :ref:`connectivity convention <connectivity_page>`. A volume can be a tetrahedron, hexahedron, pentahedron, pyramid, hexagonal  or polyhedron.
* **0D** element - mesh element defined by one node.
* **Ball** element - discrete mesh element defined by a node and a diameter.


Every mesh entity has an attribute associating it to a sub-shape it is generated on (if any). The node generated on the geometrical edge or surface in addition stores its position in parametric space of the associated geometrical entity. This attribute is set up by meshing algorithms generating elements and nodes.

Mesh entities are identified by integer IDs starting from 1.
Nodes and elements are counted separately, i.e. there can be a node and element with the same ID.

SALOME supports elements of second order, without a central node (quadratic triangle, quadrangle, polygon, tetrahedron, hexahedron,
pentahedron and pyramid) and with central nodes (bi-quadratic triangle and quadrangle and tri-quadratic hexahedron).

Quadratic mesh can be obtained in three ways:

* Using a global :ref:`quadratic_mesh_anchor` hypothesis. (Elements with the central node are not generated in this way).
* Using :ref:`convert_to_from_quadratic_mesh_page` operation.
* Using an appropriate option of some meshing algorithms, which generate elements of several dimensions starting from mesh segments.


**Table of Contents**

.. toctree::
   :titlesonly:
   :maxdepth: 2

   constructing_meshes.rst
   constructing_submeshes.rst
   editing_meshes.rst
   importing_exporting_meshes.rst
   building_compounds.rst
   copy_mesh.rst
   create_dual_mesh.rst
   connectivity.rst
