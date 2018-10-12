.. _tui_filters_page:

*************
Filters usage
*************

Filters allow picking only the mesh elements satisfying to a
specific condition or a set of conditions. Filters can be used to create
or edit mesh groups, remove elements from the mesh, control
mesh quality by different parameters, etc.

Several filtering criteria can be combined together by using logical
operators *AND* and *OR*. In addition, a filtering criterion can
be reverted using logical operator *NOT*.

Mesh filters can use the functionality of mesh quality controls to filter
mesh nodes / elements by a specific characteristic (Area, Length, etc).

This page provides a short description of the existing mesh filters,
describes required parameters and gives simple examples of usage in
Python scripts.

**See also:** :ref:`tui_quality_controls_page`

.. contents:: `Table of contents`


.. _filter_aspect_ratio:

Aspect ratio
============

filters 2D mesh elements (faces) according to the aspect ratio value:

* element type should be *SMESH.FACE*
* functor type should be *SMESH.FT_AspectRatio*
* threshold is floating point value (aspect ratio)

.. literalinclude:: ../../../examples/filters_ex01.py
    :language: python

:download:`Download this script <../../../examples/filters_ex01.py>`

**See also:** :ref:`tui_aspect_ratio`

.. _filter_aspect_ratio_3d:

Aspect ratio 3D
===============

filters 3D mesh elements (volumes) according to the aspect ratio value:

* element type is *SMESH.VOLUME*
* functor type is *SMESH.FT_AspectRatio3D*
* threshold is floating point value (aspect ratio)

.. literalinclude:: ../../../examples/filters_ex02.py
    :language: python

:download:`Download this script <../../../examples/filters_ex02.py>`

**See also:** :ref:`tui_aspect_ratio_3d`

.. _filter_warping_angle:

Warping angle
=============

filters 2D mesh elements (faces) according to the warping angle value:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_Warping*
* threshold is floating point value (warping angle)

.. literalinclude:: ../../../examples/filters_ex03.py
    :language: python

:download:`Download this script <../../../examples/filters_ex03.py>`

**See also:** :ref:`tui_warping`

.. _filter_minimum_angle:

Minimum angle
=============

filters 2D mesh elements (faces) according to the minimum angle value:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_MinimumAngle*
* threshold is floating point value (minimum angle)

.. literalinclude:: ../../../examples/filters_ex04.py
    :language: python

:download:`Download this script <../../../examples/filters_ex04.py>`

**See also:** :ref:`tui_minimum_angle`

.. _filter_taper: 

Taper
=====

filters 2D mesh elements (faces) according to the taper value:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_Taper*
* threshold is floating point value (taper)

.. literalinclude:: ../../../examples/filters_ex05.py
    :language: python

:download:`Download this script <../../../examples/filters_ex05.py>`

**See also:** :ref:`tui_taper`

.. _filter_skew: 

Skew
====

filters 2D mesh elements (faces) according to the skew value:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_Skew*
* threshold is floating point value (skew)

.. literalinclude:: ../../../examples/filters_ex06.py
    :language: python

:download:`Download this script <../../../examples/filters_ex06.py>`

**See also:** :ref:`tui_skew`

.. _filter_area: 

Area
====

filters 2D mesh elements (faces) according to the area value:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_Area*
* threshold is floating point value (area)

.. literalinclude:: ../../../examples/filters_ex07.py
    :language: python

:download:`Download this script <../../../examples/filters_ex07.py>`

**See also:** :ref:`tui_area`

.. _filter_volume: 

Volume
======

filters 3D mesh elements (volumes) according to the volume value:

* element type is *SMESH.VOLUME*
* functor type is *SMESH.FT_Volume3D*
* threshold is floating point value (volume)

.. literalinclude:: ../../../examples/filters_ex08.py
    :language: python

:download:`Download this script <../../../examples/filters_ex08.py>`

**See also:** :ref:`tui_volume`

.. _filter_free_borders:

Free borders
============

filters 1D mesh elements (edges) which represent free borders of a mesh:

* element type is *SMESH.EDGE*
* functor type is *SMESH.FT_FreeBorders*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex09.py
    :language: python

:download:`Download this script <../../../examples/filters_ex09.py>`

**See also:** :ref:`tui_free_borders`

.. _filter_free_edges:

Free edges
==========

filters 2D mesh elements (faces) having edges (i.e. links between
nodes, not mesh segments) belonging to one face of mesh only:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_FreeEdges*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex10.py
    :language: python

:download:`Download this script <../../../examples/filters_ex10.py>`

**See also:** :ref:`tui_free_edges`

.. _filter_free_nodes:

Free nodes
==========

filters free nodes:

* element type is *SMESH.NODE*
* functor type is *SMESH.FT_FreeNodes*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex11.py
    :language: python

:download:`Download this script <../../../examples/filters_ex11.py>`

**See also:** :ref:`tui_free_nodes`

.. _filter_free_faces: 

Free faces
==========

filters free faces:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_FreeFaces*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex12.py
    :language: python

:download:`Download this script <../../../examples/filters_ex12.py>`

**See also:** :ref:`tui_free_faces`

.. _filter_bare_border_faces: 

Bare border faces
=================

filters faces with bare borders:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_BareBorderFace*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex13.py
    :language: python

:download:`Download this script <../../../examples/filters_ex13.py>`

**See also:** :ref:`tui_bare_border_faces`

.. _filter_coplanar_faces:

Coplanar faces
==============

filters coplanar faces:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_CoplanarFaces*
* threshold value is the face ID
* tolerance is in degrees

.. literalinclude:: ../../../examples/filters_ex14.py
    :language: python

:download:`Download this script <../../../examples/filters_ex14.py>`

.. _filter_over_constrained_faces: 

Over-constrained faces
======================

filters over-constrained faces:

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_OverConstrainedFace*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex15.py
    :language: python

:download:`Download this script <../../../examples/filters_ex15.py>`

**See also:** :ref:`tui_over_constrained_faces`

.. _filter_double_elements: 

Double edges, Double faces, Double volumes
##########################################

filters mesh elements basing on the same set of nodes:

* element type is either *SMESH.EDGE*, *SMESH.FACE* or *SMESH.VOLUME*
* functor type is either *SMESH.FT_EqualEdges*, *SMESH.FT_EqualFaces* or *SMESH.FT_EqualVolumes*,
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex16.py
    :language: python

:download:`Download this script <../../../examples/filters_ex16.py>`

.. _tui_double_nodes_control: 

Double nodes
============

filters mesh nodes which are coincident with other nodes (within a given tolerance):

* element type is *SMESH.NODE*
* functor type is *SMESH.FT_EqualNodes*
* threshold value is not required
* default tolerance is 1.0e-7

.. literalinclude:: ../../../examples/filters_ex17.py
    :language: python

:download:`Download this script <../../../examples/filters_ex17.py>`

.. _filter_node_nb_conn:

Node connectivity number
========================

filters nodes according to a number of elements of highest dimension connected to a node:

* element type should be *SMESH.NODE*
* functor type should be *SMESH.FT_NodeConnectivityNumber*
* threshold is an integer value (number of elements)

.. literalinclude:: ../../../examples/filters_node_nb_conn.py
    :language: python

:download:`Download this script <../../../examples/filters_node_nb_conn.py>`

.. _filter_borders_multiconnection: 

Borders at multi-connection
===========================

filters 1D mesh elements (segments) according to the specified number of
connections (faces and volumes on whose border the segment lies):

* element type is *SMESH.EDGE*
* functor type is *SMESH.FT_MultiConnection*
* threshold is integer value (number of connections)

.. literalinclude:: ../../../examples/filters_ex18.py
    :language: python

:download:`Download this script <../../../examples/filters_ex18.py>`

**See also:** :ref:`tui_borders_at_multiconnection`

.. _filter_borders_multiconnection_2d: 

Borders at multi-connection 2D
==============================

filters 2D mesh elements (faces) with the specified maximal number of
faces connected to a border (link between nodes, not mesh segment):

* element type is *SMESH.FACE*
* functor type is *SMESH.FT_MultiConnection2D*
* threshold is integer value (number of connections)

.. literalinclude:: ../../../examples/filters_ex19.py
    :language: python

:download:`Download this script <../../../examples/filters_ex19.py>`

**See also:** :ref:`tui_borders_at_multiconnection_2d`

.. _filter_length: 

Length
======

filters 1D mesh elements (edges) according to the edge length value:

* element type should be *SMESH.EDGE*
* functor type should be *SMESH.FT_Length*
* threshold is floating point value (length)

.. literalinclude:: ../../../examples/filters_ex20.py
    :language: python

:download:`Download this script <../../../examples/filters_ex20.py>`

**See also:** :ref:`tui_length_1d`

.. _filter_length_2d: 

Length 2D
=========

filters 2D mesh elements (faces) according to the maximum length of its
edges (links between nodes):

* element type should be *SMESH.FACE*
* functor type should be *SMESH.FT_Length2D*
* threshold is floating point value (edge length)

.. literalinclude:: ../../../examples/filters_ex21.py
    :language: python

:download:`Download this script <../../../examples/filters_ex21.py>`

**See also:** :ref:`tui_length_2d`

.. _filter_max_element_length_2d: 

Element Diameter 2D
===================

filters 2D mesh elements (faces) according to the maximum length
of its edges and diagonals:

* element type should be *SMESH.FACE*
* functor type should be *SMESH.FT_MaxElementLength2D*
* threshold is floating point value (length)

.. literalinclude:: ../../../examples/filters_ex22.py
    :language: python

:download:`Download this script <../../../examples/filters_ex22.py>`

**See also:** :ref:`tui_max_element_length_2d`

.. _filter_max_element_length_3d: 

Element Diameter 3D
===================

filters 3D mesh elements (volumes) according to the maximum length
of its edges and diagonals:

* element type should be *SMESH.VOLUME*
* functor type should be *SMESH.FT_MaxElementLength3D*
* threshold is floating point value (edge/diagonal length)

.. literalinclude:: ../../../examples/filters_ex23.py
    :language: python

:download:`Download this script <../../../examples/filters_ex23.py>`

**See also:** :ref:`tui_max_element_length_3d`

.. _filter_bare_border_volumes: 

Bare border volumes
===================

filters 3D mesh elements with bare borders, i.e. having a facet not
shared with other volumes and without a face on it:

* element type is *SMESH.VOLUME*
* functor type is *SMESH.FT_BareBorderVolume*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex24.py
    :language: python

:download:`Download this script <../../../examples/filters_ex24.py>`

**See also:** :ref:`tui_bare_border_volumes`

.. _filter_over_constrained_volumes: 

Over-constrained volumes
========================

filters over-constrained volumes, whose all nodes are on the mesh boundary:

* element type is *SMESH.VOLUME*
* functor type is *SMESH.FT_OverConstrainedVolume*
* threshold value is not required

.. literalinclude:: ../../../examples/filters_ex25.py
    :language: python

:download:`Download this script <../../../examples/filters_ex25.py>`

**See also:** :ref:`tui_over_constrained_faces`

.. _filter_belong_to_group: 

Belong to Mesh Group
====================

filters mesh entities (nodes or elements) included in a mesh group
defined by threshold value:

* element type can be any, from *SMESH.NODE* to *SMESH.BALL*
* functor type should be *SMESH.FT_BelongToMeshGroup*
* threshold is mesh group object

.. literalinclude:: ../../../examples/filters_belong2group.py
    :language: python

:download:`Download this script <../../../examples/filters_belong2group.py>`

.. _filter_belong_to_geom: 

Belong to Geom
==============

filters mesh entities (nodes or elements) which all nodes lie on the
shape defined by threshold value:

* element type can be any, from *SMESH.NODE* to *SMESH.BALL*
* functor type should be *SMESH.FT_BelongToGeom*
* threshold is geometrical object
* tolerance is a distance between a node and the geometrical object; it is used if an node is not associated to any geometry.

.. literalinclude:: ../../../examples/filters_ex26.py
    :language: python

:download:`Download this script <../../../examples/filters_ex26.py>`

.. _filter_lying_on_geom:

Lying on Geom
=============

filters mesh entities (nodes or elements) at least one node of which lies on the
shape defined by threshold value:

* element type can be any, from *SMESH.NODE* to *SMESH.BALL*
* functor type should be *SMESH.FT_LyingOnGeom*
* threshold is geometrical object
* tolerance is a distance between a node and the geometrical object;

it is used if an node is not associated to any geometry.

.. literalinclude:: ../../../examples/filters_ex27.py
    :language: python

:download:`Download this script <../../../examples/filters_ex27.py>`

.. _filter_belong_to_plane: 

Belong to Plane
===============

filters mesh entities (nodes or elements) which all nodes belong to the
plane defined by threshold value with the given tolerance:

* element type can be any except *SMESH.VOLUME*
* functor type should be *SMESH.FT_BelongToPlane*
* threshold is geometrical object (plane)
* default tolerance is 1.0e-7

.. literalinclude:: ../../../examples/filters_ex28.py
    :language: python

:download:`Download this script <../../../examples/filters_ex28.py>`

.. _filter_belong_to_cylinder: 

Belong to Cylinder
==================

filters mesh entities (nodes or elements) which all nodes belong to the
cylindrical face defined by threshold value with the given tolerance:

* element type can be any except *SMESH.VOLUME*
* functor type should be *SMESH.FT_BelongToCylinder*
* threshold is geometrical object (cylindrical face)
* default tolerance is 1.0e-7

.. literalinclude:: ../../../examples/filters_ex29.py
    :language: python

:download:`Download this script <../../../examples/filters_ex29.py>`

.. _filter_belong_to_surface: 

Belong to Surface
=================

filters mesh entities (nodes or elements) which all nodes belong to the
arbitrary surface defined by threshold value with the given tolerance:

* element type can be any except *SMESH.VOLUME*
* functor type should be *SMESH.FT_BelongToGenSurface*
* threshold is geometrical object (arbitrary surface)
* default tolerance is 1.0e-7

.. literalinclude:: ../../../examples/filters_ex30.py
    :language: python

:download:`Download this script <../../../examples/filters_ex30.py>`

.. _filter_range_of_ids: 

Range of IDs
============

filters mesh entities elements (nodes or elements) according to the
specified identifiers range:

* element type can be any, from *SMESH.NODE* to *SMESH.BALL*
* functor type is *SMESH.FT_RangeOfIds*
* threshold is string listing required IDs and/or ranges of IDs, e.g."1,2,3,50-60,63,67,70-78"

.. literalinclude:: ../../../examples/filters_ex31.py
    :language: python

:download:`Download this script <../../../examples/filters_ex31.py>`

.. _filter_bad_oriented_volume:

Badly oriented volume
=====================

filters 3D mesh elements (volumes), which are incorrectly oriented from
the point of view of MED convention. 

* element type should be *SMESH.VOLUME*
* functor type is *SMESH.FT_BadOrientedVolume*
* threshold is not required

.. literalinclude:: ../../../examples/filters_ex32.py
    :language: python

:download:`Download this script <../../../examples/filters_ex32.py>`

.. _filter_linear_or_quadratic: 

Linear / quadratic
==================

filters linear / quadratic mesh elements:

* element type should be either *SMESH.EDGE*, *SMESH.FACE* or *SMESH.VOLUME*
* functor type is *SMESH.FT_LinearOrQuadratic*
* threshold is not required
* if unary operator is set to SMESH.FT_LogicalNOT, the quadratic elements are selected, otherwise (by default) linear elements are selected

.. literalinclude:: ../../../examples/filters_ex33.py
    :language: python

:download:`Download this script <../../../examples/filters_ex33.py>`

.. _filter_group_color: 

Group color
===========

filters mesh entities, belonging to the group with the color defined by the threshold value.

* element type can be any, from *SMESH.NODE* to *SMESH.BALL*
* functor type is *SMESH.FT_GroupColor*
* threshold should be of SALOMEDS.Color type

.. literalinclude:: ../../../examples/filters_ex34.py
    :language: python

:download:`Download this script <../../../examples/filters_ex34.py>`

.. _filter_geom_type: 

Geometry type
=============

filters mesh elements by the geometric type defined with the threshold
value. The list of available geometric types depends on the element
entity type.

* element type can be any, e.g.: *SMESH.EDGE*, *SMESH.FACE*, *SMESH.VOLUME*, etc.
* functor type should be *SMESH.FT_ElemGeomType*
* threshold is either of smesh.GeometryType values. Type *SMESH.GeometryType._items* in the Python Console to see all geometric types.

.. literalinclude:: ../../../examples/filters_ex35.py
    :language: python

:download:`Download this script <../../../examples/filters_ex35.py>`

.. _filter_entity_type: 

Entity type
===========

filters mesh elements by the geometric type and number of nodes. 

* element type can be any, e.g.: *SMESH.EDGE*, *SMESH.FACE*, *SMESH.VOLUME*, etc.
* functor type should be *SMESH.FT_EntityType*
* threshold is either of SMESH.EntityType values. Type *SMESH.EntityType._items* in the Python Console to see all entity types.

.. literalinclude:: ../../../examples/filters_ex37.py
    :language: python

:download:`Download this script <../../../examples/filters_ex37.py>`

.. _filter_ball_diam: 

Ball diameter
=============

filters ball elements by diameter. 

* element type should be *SMESH.BALL*
* functor type should be *SMESH.FT_BallDiameter*
* threshold is floating point value (ball diameter)

.. literalinclude:: ../../../examples/filters_ex38.py
    :language: python

:download:`Download this script <../../../examples/filters_ex38.py>`

.. _filter_domain: 

Elements of a domain
====================

filters elements of a specified domain. 

* element type can be any, e.g.: *SMESH.EDGE*, *SMESH.FACE*, *SMESH.VOLUME*, etc.
* functor type should be *SMESH.FT_ConnectedElements*
* threshold is either (1) node ID or (2)  geometrical vertex or (3) 3 coordinates of a point.

.. literalinclude:: ../../../examples/filters_ex39.py
    :language: python

:download:`Download this script <../../../examples/filters_ex39.py>`

.. _combining_filters: 

How to combine several criteria into a filter?
==============================================

Several criteria can be combined into a filter.

.. literalinclude:: ../../../examples/filters_ex36.py
    :language: python

:download:`Download this script <../../../examples/filters_ex36.py>`
