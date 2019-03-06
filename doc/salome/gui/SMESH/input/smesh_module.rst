SMESH module
============

.. contents:: 

.. py:module:: SMESH

DriverMED_ReadStatus
--------------------

.. py:class:: DriverMED_ReadStatus

   Enumeration for mesh read status

   .. py:attribute:: DRS_OK 

      Ok

   .. py:attribute:: DRS_EMPTY
                     
      a file contains no mesh with the given name

   .. py:attribute:: DRS_WARN_RENUMBER

      a MED file has overlapped ranges of element numbers,
      so the numbers from the file are ignored

   .. py:attribute:: DRS_WARN_SKIP_ELEM
      
      some elements were skipped due to incorrect file data

   .. py:attribute:: DRS_WARN_DESCENDING

      some elements were skipped due to descending connectivity

   .. py:attribute:: DRS_FAIL

      general failure (exception etc.)

ComputeErrorName
----------------

.. py:class:: ComputeErrorName

   Enumeration of computation errors

   .. py:attribute:: COMPERR_OK

      Ok
      
   .. py:attribute:: COMPERR_BAD_INPUT_MESH

      wrong mesh of lower sub-mesh

   .. py:attribute:: COMPERR_STD_EXCEPTION

      some std exception raised

   .. py:attribute:: COMPERR_OCC_EXCEPTION

      OCC exception raised

   .. py:attribute:: COMPERR_SLM_EXCEPTION

      SALOME exception raised

   .. py:attribute:: COMPERR_EXCEPTION

      other exception raised

   .. py:attribute:: COMPERR_MEMORY_PB

      memory allocation problem

   .. py:attribute:: COMPERR_ALGO_FAILED

      computation failed

   .. py:attribute:: COMPERR_BAD_SHAPE

      bad geometry

   .. py:attribute:: COMPERR_WARNING

      algo reports error but sub-mesh is computed anyway

   .. py:attribute:: COMPERR_CANCELED

      compute canceled

   .. py:attribute:: COMPERR_NO_MESH_ON_SHAPE

      no mesh elements assigned to sub-mesh

   .. py:attribute:: COMPERR_BAD_PARMETERS

      incorrect hypotheses parameters


ComputeError
------------

.. py:class:: ComputeError

   Error details

   .. py:attribute:: code

      ``int`` - :class:`ComputeErrorName` or, if negative, algo specific code

   .. py:attribute:: comment
                     
      ``str`` - textual problem description

   .. py:attribute:: algoName

      ``str``

   .. py:attribute:: subShapeID
      
      ``int`` - id of sub-shape of a shape to mesh

   .. py:attribute:: hasBadMesh

      ``boolean`` - there are elements preventing computation available for visualization

Measure
-------

.. py:class:: Measure

   Data returned by measure operations

   .. py:attribute:: minX, minY, minZ

      ``double`` - coordinates of one point

   .. py:attribute:: maxX, maxY, maxZ

      ``double`` - coordinates of another point

   .. py:attribute:: node1, node2

      ``long`` - IDs of two nodes

   .. py:attribute:: elem1, elem2

      ``long`` - IDs of two elements

   .. py:attribute:: value

      ``double`` - distance

NodePosition
------------

.. py:class:: NodePosition

   Node location on a shape

   .. py:attribute:: shapeID             

      ``long`` - ID of a shape

   .. py:attribute:: shapeType 

      ``GEOM.shape_type`` - type of shape

   .. py:attribute:: params

      ``list of float`` - 

        * [U] on EDGE, 
        * [U,V] on FACE,
        * [] on the rest shapes

ElementPosition
---------------

.. py:class:: ElementPosition

   Element location on a shape

   .. py:attribute:: shapeID

      ``long`` - ID of a shape

   .. py:attribute:: shapeType

      ``GEOM.shape_type`` - type of shape

PolySegment
-----------

.. py:class:: PolySegment

   Define a cutting plane passing through two points. 
   Used in :meth:`~smeshBuilder.Mesh.MakePolyLine`

   .. py:attribute:: node1ID1, node1ID2

      ``int,int`` - *point 1*: if *node1ID2* > 0, then the point is in the middle of a face edge defined
                by two nodes, else it is at *node1ID1*

   .. py:attribute:: node2ID1, node2ID2

      ``int,int`` - *point 2*: if *node2ID2* > 0, then the point is in the middle of a face edge defined
                by two nodes, else it is at *node2ID1*

   .. py:attribute:: vector

      ``SMESH.DirStruct`` - vector on the plane; to use a default plane set vector = (0,0,0)


ElementType
-----------

.. py:class:: ElementType

   Enumeration for element type, like in SMDS

   .. py:attribute:: 
    ALL
    NODE
    EDGE
    FACE
    VOLUME
    ELEM0D
    BALL
    NB_ELEMENT_TYPES

EntityType
----------

.. py:class:: EntityType

   Enumeration of entity type

   .. py:attribute:: 
    Entity_Node
    Entity_0D
    Entity_Edge
    Entity_Quad_Edge
    Entity_Triangle
    Entity_Quad_Triangle
    Entity_BiQuad_Triangle
    Entity_Quadrangle
    Entity_Quad_Quadrangle
    Entity_BiQuad_Quadrangle
    Entity_Polygon
    Entity_Quad_Polygon
    Entity_Tetra
    Entity_Quad_Tetra
    Entity_Pyramid
    Entity_Quad_Pyramid
    Entity_Hexa
    Entity_Quad_Hexa
    Entity_TriQuad_Hexa
    Entity_Penta
    Entity_Quad_Penta
    Entity_BiQuad_Penta
    Entity_Hexagonal_Prism
    Entity_Polyhedra
    Entity_Quad_Polyhedra
    Entity_Ball
    Entity_Last

GeometryType
------------

.. py:class:: GeometryType

   Enumeration of element geometry type

   .. py:attribute::
    Geom_POINT
    Geom_EDGE
    Geom_TRIANGLE
    Geom_QUADRANGLE
    Geom_POLYGON
    Geom_TETRA
    Geom_PYRAMID
    Geom_HEXA
    Geom_PENTA
    Geom_HEXAGONAL_PRISM
    Geom_POLYHEDRA
    Geom_BALL
    Geom_LAST

Hypothesis_Status
-----------------

.. py:class:: Hypothesis_Status

   Enumeration of result of hypothesis addition/removal

   .. py:attribute::  HYP_OK

      Ok

   .. py:attribute::  HYP_MISSING

      algo misses a hypothesis

   .. py:attribute::  HYP_CONCURRENT

      several applicable hypotheses

   .. py:attribute::  HYP_BAD_PARAMETER

      hypothesis has a bad parameter value

   .. py:attribute::  HYP_HIDDEN_ALGO

      an algo is hidden by an upper dim algo generating all-dim elements

   .. py:attribute::  HYP_HIDING_ALGO

      an algo hides lower dim algos by generating all-dim elements

   .. py:attribute::  HYP_UNKNOWN_FATAL

       all statuses below should be considered as fatal for Add/RemoveHypothesis operations

   .. py:attribute::  HYP_INCOMPATIBLE

      hypothesis does not fit algorithm

   .. py:attribute::  HYP_NOTCONFORM

      not conform mesh is produced applying a hypothesis

   .. py:attribute::  HYP_ALREADY_EXIST

      such hypothesis already exist

   .. py:attribute::  HYP_BAD_DIM

      bad dimension

   .. py:attribute::  HYP_BAD_SUBSHAPE

      shape is neither the main one, nor its sub-shape, nor a group

   .. py:attribute::  HYP_BAD_GEOMETRY

      geometry mismatches algorithm's expectation

   .. py:attribute::  HYP_NEED_SHAPE

      algorithm can work on shape only

   .. py:attribute::  HYP_INCOMPAT_HYPS

      several additional hypotheses are incompatible one with other


FunctorType
-----------

.. py:class:: FunctorType

   Enumeration of functor types

   .. py:attribute:: 
    FT_AspectRatio
    FT_AspectRatio3D
    FT_Warping   
    FT_MinimumAngle
    FT_Taper       
    FT_Skew         
    FT_Area          
    FT_Volume3D          
    FT_MaxElementLength2D
    FT_MaxElementLength3D
    FT_FreeBorders
    FT_FreeEdges
    FT_FreeNodes
    FT_FreeFaces
    FT_EqualNodes
    FT_EqualEdges
    FT_EqualFaces
    FT_EqualVolumes
    FT_MultiConnection
    FT_MultiConnection2D
    FT_Length
    FT_Length2D
    FT_Deflection2D
    FT_NodeConnectivityNumber
    FT_BelongToMeshGroup
    FT_BelongToGeom
    FT_BelongToPlane
    FT_BelongToCylinder
    FT_BelongToGenSurface
    FT_LyingOnGeom
    FT_RangeOfIds
    FT_BadOrientedVolume
    FT_BareBorderVolume
    FT_BareBorderFace
    FT_OverConstrainedVolume
    FT_OverConstrainedFace
    FT_LinearOrQuadratic
    FT_GroupColor
    FT_ElemGeomType
    FT_EntityType
    FT_CoplanarFaces
    FT_BallDiameter
    FT_ConnectedElements
    FT_LessThan
    FT_MoreThan
    FT_EqualTo
    FT_LogicalNOT
    FT_LogicalAND
    FT_LogicalOR
    FT_Undefined

.. py:module:: SMESH.Filter
   :noindex:

Filter.Criterion
----------------

.. py:class:: Criterion

   Structure containing information of a criterion

   .. py:attribute:: Type

      ``long`` - value of item of :class:`SMESH.FunctorType`

   .. py:attribute:: Compare

      ``long`` - value of item of :class:`SMESH.FunctorType` in ( FT_LessThan, FT_MoreThan, FT_EqualTo )

   .. py:attribute:: Threshold

      ``double`` - threshold value

   .. py:attribute:: ThresholdStr

      ``string`` - Threshold value defined as string. Used for:
                       1. Diapason of identifiers. Example: "1,2,3,5-10,12,27-29".
                       2. Storing name of shape.
                       3. Storing group color "0.2;0;0.5".
                       4. Storing point coordinates.

   .. py:attribute:: ThresholdID

      ``string`` - One more threshold value defined as string. Used for storing id of shape

   .. py:attribute:: UnaryOp

      ``long`` - unary logical operation: FT_LogicalNOT or FT_Undefined

   .. py:attribute:: BinaryOp

      ``long`` - binary logical operation FT_LogicalAND, FT_LogicalOR etc.

   .. py:attribute:: Tolerance

      ``double`` - Tolerance is used for 
                       1. Comparison of real values.
                       2. Detection of geometrical coincidence.

   .. py:attribute:: TypeOfElement

      ``ElementType`` - type of element :class:`SMESH.ElementType` (SMESH.NODE, SMESH.FACE etc.)

   .. py:attribute:: Precision

      ``long`` - Precision of numerical functors

.. py:currentmodule:: SMESH

FreeEdges.Border
----------------

.. py:class:: FreeEdges.Border

   Free edge: edge connected to one face only

   .. py:attribute:: myElemId

      ``long`` - ID of a face

   .. py:attribute:: myPnt1,myPnt2

      ``long`` - IDs of two nodes

PointStruct
-----------

.. py:class:: PointStruct

   3D point. 

   Use :meth:`GetPointStruct() <smeshBuilder.smeshBuilder.GetPointStruct>` 
   to convert a vertex (GEOM.GEOM_Object) to PointStruct

   .. py:attribute:: x,y,z

      ``double`` - point coordinates

DirStruct
---------

.. py:class:: DirStruct

   3D vector.

   Use :meth:`GetDirStruct() <smeshBuilder.smeshBuilder.GetDirStruct>` 
   to convert a vector (GEOM.GEOM_Object) to DirStruct

   .. py:attribute:: PS

      :class:`PointStruct` - vector components
      
AxisStruct
----------

.. py:class:: AxisStruct

   Axis defined by its origin and its vector.

   Use :meth:`GetAxisStruct() <smeshBuilder.smeshBuilder.GetAxisStruct>` 
   to convert a line or plane (GEOM.GEOM_Object) to AxisStruct

   .. py:attribute:: x,y,z

      ``double`` - coordinates of the origin

   .. py:attribute:: vx,vy,vz

      ``double`` - components of the vector

Filter
------

.. py:class:: Filter

   Filter of mesh entities

   .. py:function:: GetElementsId( mesh )

      Return satisfying elements

      :param SMESH.SMESH_Mesh mesh: the mesh; 
                                    it can be obtained via :meth:`~smeshBuilder.Mesh.GetMesh`

      :return: list of IDs
                                    
   .. py:function:: GetIDs()

      Return satisfying elements. 
      A mesh to filter must be already set, either via :meth:`SetMesh` method 
      or via ``mesh`` argument of :meth:`~smeshBuilder.smeshBuilder.GetFilter`

      :return: list of IDs
                                    
   .. py:function:: SetMesh( mesh )

      Set mesh to filter

      :param SMESH.SMESH_Mesh mesh: the mesh;
                                    it can be obtained via :meth:`~smeshBuilder.Mesh.GetMesh`

   .. py:function:: SetCriteria( criteria )

      Define filtering criteria

      :param criteria:  list of :class:`SMESH.Filter.Criterion`

NumericalFunctor
----------------

.. py:class:: NumericalFunctor

   Calculate value by ID of mesh entity. Base class of various functors

   .. py:function:: GetValue( elementID )

      Compute a value

      :param elementID: ID of element or node
      :return: floating value

SMESH_Mesh
----------

.. py:class:: SMESH_Mesh

   Mesh. It is a Python wrap over a CORBA interface of mesh.

   All its methods are exposed via :class:`smeshBuilder.Mesh` class that you can obtain by calling::

     smeshBuilder_mesh = smesh.Mesh( smesh_mesh )

SMESH_MeshEditor
----------------

.. py:class:: SMESH_MeshEditor

   Mesh editor. It is a Python wrap over a CORBA SMESH_MeshEditor interface.
   All its methods are exposed via :class:`smeshBuilder.Mesh` class.

   .. py:class:: Extrusion_Error

      Enumeration of errors of :meth:`~smeshBuilder.Mesh.ExtrusionAlongPathObjects`

      .. py:attribute::
         EXTR_OK
         EXTR_NO_ELEMENTS
         EXTR_PATH_NOT_EDGE
         EXTR_BAD_PATH_SHAPE
         EXTR_BAD_STARTING_NODE
         EXTR_BAD_ANGLES_NUMBER
         EXTR_CANT_GET_TANGENT

.. py:class:: SMESH_MeshEditor.Sew_Error

      Enumeration of errors of SMESH_MeshEditor.Sewing... methods

      .. py:attribute::
         SEW_OK
         SEW_BORDER1_NOT_FOUND
         SEW_BORDER2_NOT_FOUND
         SEW_BOTH_BORDERS_NOT_FOUND
         SEW_BAD_SIDE_NODES
         SEW_VOLUMES_TO_SPLIT
         SEW_DIFF_NB_OF_ELEMENTS
         SEW_TOPO_DIFF_SETS_OF_ELEMENTS
         SEW_BAD_SIDE1_NODES
         SEW_BAD_SIDE2_NODES
         SEW_INTERNAL_ERROR

SMESH_Pattern
-------------

.. py:class:: SMESH_Pattern

   Pattern mapper. Use a pattern defined by user for

      * creating mesh elements on geometry, faces or blocks
      * refining existing mesh elements, faces or hexahedra

   The pattern is defined by a string as explained :doc:`here <pattern_mapping>`.

   Usage work-flow is:

      * Define a pattern via Load... method
      * Compute future positions of nodes via Apply... method
      * Create nodes and elements in a mesh via :meth:`MakeMesh` method

   .. py:function:: LoadFromFile( patternFileContents )

      Load a pattern from the string *patternFileContents*

      :param str patternFileContents: string defining a pattern
      :return: True if succeeded

   .. py:function:: LoadFromFace( mesh, geomFace, toProject )

      Create a 2D pattern from the mesh built on *geomFace*.

      :param SMESH.SMESH_Mesh     mesh: source mesh
      :param GEOM.GEOM_Object geomFace: geometrical face whose mesh forms a pattern
      :param boolean         toProject: if True makes override nodes positions
                                          on *geomFace* computed by mesher
      :return: True if succeeded

   .. py:function:: LoadFrom3DBlock( mesh, geomBlock )

      Create a 3D pattern from the mesh built on *geomBlock*

      :param SMESH.SMESH_Mesh      mesh: source mesh
      :param GEOM.GEOM_Object geomBlock: geometrical block whose mesh forms a pattern
      :return: True if succeeded

   .. py:function:: ApplyToFace( geomFace, vertexOnKeyPoint1, toReverse )

      Compute nodes coordinates by applying
      the loaded pattern to *geomFace*. The first key-point
      will be mapped into *vertexOnKeyPoint1*, which must
      be in the outer wire of *geomFace*

      :param GEOM.GEOM_Object geomFace: the geometrical face to generate faces on
      :param GEOM.GEOM_Object vertexOnKeyPoint1: the vertex to be at the 1st key-point
      :param boolean          toReverse: to reverse order of key-points
      :return: list of :class:`SMESH.PointStruct` - computed coordinates of points of the pattern

   .. py:function:: ApplyTo3DBlock( geomBlock, vertex000, vertex001 )

      Compute nodes coordinates by applying
      the loaded pattern to *geomBlock*. The (0,0,0) key-point
      will be mapped into *vertex000*. The (0,0,1) 
      key-point will be mapped into *vertex001*.

      :param GEOM.GEOM_Object geomBlock: the geometrical block to generate volume elements on
      :param GEOM.GEOM_Object vertex000: the vertex to superpose (0,0,0) key-point of pattern
      :param GEOM.GEOM_Object vertex001: the vertex to superpose (0,0,1) key-point of pattern
      :return: list of :class:`SMESH.PointStruct` - computed coordinates of points of the pattern

   .. py:function:: ApplyToMeshFaces( mesh, facesIDs, nodeIndexOnKeyPoint1, toReverse )

      Compute nodes coordinates by applying
      the loaded pattern to mesh faces. The first key-point
      will be mapped into *nodeIndexOnKeyPoint1* -th node of each mesh face

      :param SMESH.SMESH_Mesh    mesh: the mesh where to refine faces
      :param list_of_ids     facesIDs: IDs of faces to refine
      :param int nodeIndexOnKeyPoint1: index of a face node to be at 1-st key-point of pattern
      :param boolean        toReverse: to reverse order of key-points
      :return: list of :class:`SMESH.PointStruct` - computed coordinates of points of the pattern

   .. py:function:: ApplyToHexahedrons( mesh, volumesIDs, node000Index, node001Index )

      Compute nodes coordinates by applying
      the loaded pattern to hexahedra. The (0,0,0) key-point
      will be mapped into *Node000Index* -th node of each volume.
      The (0,0,1) key-point will be mapped into *node001Index* -th
      node of each volume.

      :param SMESH.SMESH_Mesh   mesh: the mesh where to refine hexahedra
      :param list_of_ids  volumesIDs: IDs of volumes to refine
      :param long       node000Index: index of a volume node to be at (0,0,0) key-point of pattern
      :param long       node001Index: index of a volume node to be at (0,0,1) key-point of pattern
      :return: list of :class:`SMESH.PointStruct` - computed coordinates of points of the pattern

   .. py:function:: MakeMesh( mesh, createPolygons, createPolyedrs )

      Create nodes and elements in *mesh* using nodes
      coordinates computed by either of Apply...() methods.
      If *createPolygons* is True, replace adjacent faces by polygons
      to keep mesh conformity.
      If *createPolyedrs* is True, replace adjacent volumes by polyedrs
      to keep mesh conformity.

      :param SMESH.SMESH_Mesh     mesh: the mesh to create nodes and elements in
      :param boolean    createPolygons: to create polygons to to keep mesh conformity
      :param boolean    createPolyedrs: to create polyherda to to keep mesh conformity
      :return: True if succeeded


SMESH_subMesh
-------------

.. py:class:: SMESH_subMesh

   :doc:`Sub-mesh <constructing_submeshes>`

   .. py:function:: GetNumberOfElements()

      Return number of elements in the sub-mesh

   .. py:function:: GetNumberOfNodes( all )

      Return number of nodes in the sub-mesh

      :param boolean all: if True, also return nodes assigned to boundary sub-meshes 

   .. py:function:: GetElementsId()

      Return IDs of elements in the sub-mesh

   .. py:function:: GetNodesId()

      Return IDs of nodes in the sub-mesh

   .. py:function:: GetSubShape()

      Return :class:`geom shape <GEOM.GEOM_Object>` the sub-mesh is dedicated to

   .. py:function:: GetId()

      Return ID of the :class:`geom shape <GEOM.GEOM_Object>` the sub-mesh is dedicated to

   .. py:function:: GetMeshInfo()

      Return number of mesh elements of each :class:`SMESH.EntityType`.
      Use :meth:`~smeshBuilder.smeshBuilder.EnumToLong` to get an integer from 
      an item of :class:`SMESH.EntityType`.

      :return: array of number of elements per :class:`SMESH.EntityType`

   .. py:function:: GetMesh()

      Return the :class:`SMESH.SMESH_Mesh`

SMESH_GroupBase
---------------

.. py:class:: SMESH_GroupBase

   :doc:`Mesh group <grouping_elements>`. 
   Base class of :class:`standalone group <SMESH_Group>`, 
   :class:`group on geometry <SMESH_GroupOnGeom>` and 
   :class:`group on filter <SMESH_GroupOnFilter>`.
   Inherit all methods from :class:`SMESH_IDSource`.

   .. py:function:: SetName( name )

      Set group name

   .. py:function:: GetName()

      Return group name

   .. py:function:: GetType()

      Return :class:`group type <SMESH.ElementType>` (type of elements in the group)

   .. py:function:: Size()

      Return the number of elements in the group

   .. py:function:: IsEmpty()

      Return True if the group does not contain any elements

   .. py:function:: Contains( elem_id )

      Return True if the group contains an element with ID == *elem_id*

   .. py:function:: GetID( elem_index )

      Return ID of an element at position *elem_index* counted from 1

   .. py:function:: GetNumberOfNodes()

      Return the number of nodes of cells included to the group.
      For a nodal group return the same value as Size() function

   .. py:function:: GetNodeIDs()

      Return IDs of nodes of cells included to the group.
      For a nodal group return result of GetListOfID() function

   .. py:function:: SetColor( color )

      Set group color

      :param SALOMEDS.Color color: color

   .. py:function:: GetColor()

      Return group color

      :return: SALOMEDS.Color

SMESH_Group
-----------

.. py:class:: SMESH_Group

   :doc:`Standalone mesh group <grouping_elements>`. Inherits all methods of :class:`SMESH.SMESH_GroupBase`

   .. py:function:: Clear()

      Clears the group's contents

   .. py:function:: Add( elem_ids )

      Adds elements or nodes with specified identifiers to the group

      :param list_of_ids elem_ids: IDs to add

   .. py:function:: AddFrom( idSource )

      Add all elements or nodes from the specified source to the group

      :param SMESH.SMESH_IDSource idSource: an object to retrieve IDs from

   .. py:function:: Remove( elem_ids )

      Removes elements or nodes with specified identifiers from the group

      :param list_of_ids elem_ids: IDs to remove

SMESH_GroupOnGeom
-----------------

.. py:class:: SMESH_GroupOnGeom

   Group linked to geometry. Inherits all methods of :class:`SMESH.SMESH_GroupBase`

   .. py:function:: GetShape()

      Return an associated geometry

      :return: GEOM.GEOM_Object

SMESH_GroupOnFilter
-------------------

.. py:class:: SMESH_GroupOnFilter

   Group defined by filter. Inherits all methods of :class:`SMESH.SMESH_GroupBase`

   .. py:function:: SetFilter( filter )

      Set the :class:`filter <SMESH.Filter>`

   .. py:function:: GetFilter()

      Return the :class:`filter <SMESH.Filter>`


SMESH_IDSource
--------------

.. py:class:: SMESH_IDSource

   Base class for classes able to return IDs of mesh entities. These classes are:

   * :class:`SMESH.SMESH_Mesh`
   * :class:`SMESH.SMESH_subMesh`
   * :class:`SMESH.SMESH_GroupBase`
   * :class:`SMESH.Filter`
   * temporal ID source created by :meth:`~smeshBuilder.Mesh.GetIDSource`

   .. py:function:: GetIDs()

      Return a sequence of all element IDs

   .. py:function:: GetMeshInfo()

      Return number of mesh elements of each :class:`SMESH.EntityType`.
      Use :meth:`~smeshBuilder.smeshBuilder.EnumToLong` to get an integer from 
      an item of :class:`SMESH.EntityType`.

   .. py:function:: GetNbElementsByType()

      Return number of mesh elements of each :class:`SMESH.ElementType`.
      Use :meth:`~smeshBuilder.smeshBuilder.EnumToLong` to get an integer from 
      an item of :class:`SMESH.ElementType`.


   .. py:function:: GetTypes()

      Return types of elements it contains.
      It's empty if the object contains no IDs

      :return: list of :class:`SMESH.ElementType`

   .. py:function:: GetMesh()

      Return the :class:`SMESH.SMESH_Mesh`

SMESH_Hypothesis
----------------

.. py:class:: SMESH_Hypothesis

   Base class of all :doc:`hypotheses <about_hypo>`

   .. py:function:: GetName()

      Return string of hypothesis type name, something like "Regular_1D"

   .. py:function:: GetLibName()
      
      Return string of plugin library name
