#!/usr/bin/env python3

import salome
import medcoupling as mc

import GEOM
from salome.geom import geomBuilder

geompy = geomBuilder.New()

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

from salome.kernel.logger import Logger
logger = Logger("salome.smesh.smesh_tools")
logger.setLevel("WARNING")

# prefix for groups with internal usage
# i.e. used to transfer the faces and edges sub-shapes ids to the mesh
__prefix = "____"

def __getIdsGrpDualFromOrig(mc_mesh_file, grp_name, mesh2d, grp_level):
    """ Identify the polygonal cells ids matching the original group on the
        original mesh (before dual mesh)

    Args:
        mc_mesh_file (MEDFileMesh): mesh on which to read the group
        grp_name (string): name of the group to read
        mesh2d (MEDCouplingUMesh): mesh at lower level (-1 or -2) containing
                                   faces or segments cells
        grp_level (int): level on which to load the group (-1 or -2)

    Returns:
        id_grp_poly (DataArrayInt64): ids of cells mathcing the group. None if
                                      the group has not been found.
        nodes_added (DataArrayInt64): new nodes added on the dual mesh
    """
    try:
        grp_tria = mc_mesh_file.getGroup(grp_level, grp_name)
    except:
        logger.debug("""No group found for %s at level %i.
                     It is normal behaviour for degenerated geom edge."""\
                     %(grp_name, grp_level))
        return None, None
    # Retrieve the nodes in group
    orig_grp_nodes = grp_tria.computeFetchedNodeIds()
    # Find all the cells lying on one of the nodes
    id_grp_poly = mesh2d.getCellIdsLyingOnNodes(orig_grp_nodes, False)

    grp_poly = mesh2d[id_grp_poly]
    if grp_poly.getNumberOfCells() == 0:
        logger.debug("""No poly cell found for %s at level %i."""\
                     %(grp_name, grp_level))
        return None, None

    # find the extra face cells, on the border of the group (lying on nodes,
    # but outside the group)
    id_poly_border = grp_poly.findCellIdsOnBoundary()

    # ids of the cells in grp_poly
    id_poly = mc.DataArrayInt64.New(grp_poly.getNumberOfCells(), 1)
    id_poly.iota()

    # cells that are really in the group
    id_to_keep = id_poly.buildSubstraction(id_poly_border)

    id_grp_poly = id_grp_poly[id_to_keep]

    id_grp_poly.setName(grp_name.strip())

    # get nodes added on this group
    grp_poly = mesh2d[id_grp_poly]
    grp_nodes_poly = grp_poly.computeFetchedNodeIds()
    nodes_added = grp_nodes_poly.buildSubstraction(orig_grp_nodes)

    return id_grp_poly, nodes_added

def __projectNodeOnSubshape(nodes, subshape, coords):
    """ Project nodes on a sub-shape (face or edge) and update the mesh
        coordinates

    Args:
        nodes (DataArrayInt): nodes ids to project
        subshape (GEOM object): face or edge on which to project the nodes
        coords (DataArrayDouble): coordinates of the mesh to update. These
                                  coordinates are modified inside this function.
    """
    for i in nodes:
        x, y, z = coords[i].getValues()
        vertex = geompy.MakeVertex(x, y, z)
        try:
            prj = geompy.MakeProjection(vertex, subshape)
        except:
            logger.warning("Projection failed for %.5f %.5f %.5f but we continue with next node"%(x, y, z))
            continue
        new_coor = geompy.PointCoordinates(prj)
        # update its coordinates in the mesh
        coords[i] = new_coor
    pass

def __deleteObj(theObj):
    """ Delete object from a Study

    Args:
        theObj (GEOM or SMESH object): object to remove from the study
    """
    aStudy = salome.myStudy
    aStudyBuilder = aStudy.NewBuilder()
    SO = aStudy.FindObjectIOR(aStudy.ConvertObjectToIOR(theObj))
    if SO is not None:
        aStudyBuilder.RemoveObjectWithChildren(SO)
    pass

def smesh_create_dual_mesh(mesh_ior, output_file, adapt_to_shape=True,
                           mesh_name="MESH"):
    """ Create a dual of the mesh in input_file into output_file

    Args:
        mesh_ior (string): corba Id of the Tetrahedron mesh
        output_file (string): dual mesh file
        adapt_to_shape (bool): If True will project boundary points on shape
        mesh_name (string): Name of the dual Mesh
    """
    # Import mesh from file
    mesh = salome.orb.string_to_object(mesh_ior)
    if not mesh:
        raise Exception("Could not find mesh using id: ", mesh_ior)

    shape = mesh.GetShapeToMesh()

    if adapt_to_shape:
        faces = geompy.SubShapeAll(shape, geompy.ShapeType["FACE"])
        faces_ids = geompy.GetSubShapesIDs(shape, faces)

        # Create group with each face
        # so that we don't need GetFaceNearPoint to get the face to project the
        # point to
        id2face = {}
        id2face_edges_ids = {}
        mesh_groups = []
        for face, face_id in zip(faces, faces_ids):
            gr_mesh = mesh.CreateGroupFromGEOM(SMESH.FACE,
                                               '%sface_%i'%(__prefix, face_id),
                                               face)
            id2face[face_id] = face
            mesh_groups.append(gr_mesh)
            # get the edges bounding this face
            # so that we can project the nodes on edges before nodes on faces
            face_edges = geompy.SubShapeAll(face, geompy.ShapeType["EDGE"])
            face_edges_ids = geompy.GetSubShapesIDs(shape, face_edges)
            id2face_edges_ids[face_id] = face_edges_ids

        edges = geompy.SubShapeAll(shape, geompy.ShapeType["EDGE"])
        edges_ids = geompy.GetSubShapesIDs(shape, edges)

        id2edge = {}
        for edge, edge_id in zip(edges, edges_ids):
            gr_mesh = mesh.CreateGroupFromGEOM(SMESH.EDGE,
                                               '%sedge_%i'%(__prefix, edge_id),
                                               edge)
            id2edge[edge_id] = edge
            mesh_groups.append(gr_mesh)

    # We got a meshProxy so we need to convert pointer to MEDCoupling
    int_ptr = mesh.ExportMEDCoupling(True, True)
    dab = mc.FromPyIntPtrToDataArrayByte(int_ptr)
    mc_mesh_file = mc.MEDFileMesh.New(dab)
    tetras = mc_mesh_file[0]
    # End of SMESH -> MEDCoupling part for dualmesh

    tetras = mc.MEDCoupling1SGTUMesh(tetras)

    # Create the polyhedra from the tetrahedra (main goal of this function)
    polyh = tetras.computeDualMesh()

    ## Adding skin + transfering groups on faces from tetras mesh
    mesh2d = polyh.buildUnstructured().computeSkin()
    mesh2d.setName(mesh_name)

    polyh_coords = polyh.getCoords()

    treated_edges = []

    mc_groups = []
    for grp_name in mc_mesh_file.getGroupsOnSpecifiedLev(-1):
        # This group is created by the export
        if grp_name == "Group_Of_All_Faces":
            logger.debug("Skipping group: "+ grp_name)
            continue
        logger.debug("Transferring group: "+ grp_name)

        # get the polygons ids on the dual mesh from the triangles group
        id_grp_poly, nodes_added_on_tri = \
            __getIdsGrpDualFromOrig(mc_mesh_file, grp_name, mesh2d, -1)

        if id_grp_poly is not None and grp_name[:4] == __prefix:
            # This group is on a specific geom face
            face_id = grp_name.split("_")[-1]
            face_id = int(face_id)
            face = id2face[face_id]

            # for each face, get the edges bounding it
            grp_poly = mesh2d[id_grp_poly]
            mesh1d = grp_poly.computeSkin()

            face_edges_id = id2face_edges_ids[face_id]
            for edge_id in face_edges_id:
                grp_seg_name = "%sedge_%i"%(__prefix, edge_id)

                # get the segs on the dual mesh from the segs group
                id_grp_seg, nodes_added_on_segs = __getIdsGrpDualFromOrig(\
                                mc_mesh_file, grp_seg_name, mesh1d, -2)

                # project new nodes on its geom edge
                # (if the group exists on this edge and it has not already been
                # treated)
                if id_grp_seg is not None:
                    if edge_id not in treated_edges:
                        edge = id2edge[edge_id]
                        __projectNodeOnSubshape(nodes_added_on_segs,
                                                edge, polyh_coords)
                    else:
                        treated_edges.append(edge_id)

                    # remove these nodes from the nodes to project on face
                    nodes_added_on_tri = \
                       nodes_added_on_tri.buildSubstraction(nodes_added_on_segs)

            # project new nodes on its geom face
            __projectNodeOnSubshape(nodes_added_on_tri, face, polyh_coords)
        else:
            # add the group to write it
            mc_groups.append(id_grp_poly)

    # Creating output file
    logger.debug("Creating file with mesh: "+mesh_name)
    myfile = mc.MEDFileUMesh()
    myfile.setName(mesh_name)
    polyh.setName(mesh_name)
    myfile.setMeshAtLevel(0, polyh)
    myfile.setMeshAtLevel(-1, mesh2d)

    for group in mc_groups:
        myfile.addGroup(-1, group)

    logger.debug("Writing dual mesh in: "+output_file)
    myfile.write(output_file, 2)

    if adapt_to_shape:
        # delete temporary groups
        for grp in mesh_groups:
            __deleteObj(grp)
