# Usage of "Use Faces to be Created Manually" algorithm

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# define my 2D algorithm
def my2DMeshing(geomFace, mesh):
    import numpy as np

    # find gravity center of geomFace
    gcXYZ = geom_builder.PointCoordinates( geom_builder.MakeCDG( geomFace ))

    # define order and orientation of edges
    sortedEdges = []
    geomEdges = geom_builder.SubShapeAll( geomFace, geom_builder.ShapeType["EDGE"])
    sortedEdges.append(( geomEdges.pop(0), True ))
    while geomEdges:
        prevEdge_rev = sortedEdges[ -1 ]
        prevVV = geom_builder.SubShapeAll( prevEdge_rev[0], geom_builder.ShapeType["VERTEX"])
        prevV2 = prevVV[ prevEdge_rev[1] ]
        found = False
        for iE in range( len( geomEdges )):
            v1,v2 = geom_builder.SubShapeAll( geomEdges[ iE ], geom_builder.ShapeType["VERTEX"])
            same1,same2 = [( geom_builder.MinDistance( prevV2, v ) < 1e-7 ) for v in [v1,v2] ]
            if not same1 and not same2: continue
            sortedEdges.append(( geomEdges.pop( iE ), same1 ))
            found = True
            break
        assert found
    sortedEdges.reverse()

    # put nodes on edges in a right order
    nodes = []
    for edge, isForward in sortedEdges:
        v1,v2 = geom_builder.SubShapeAll( edge, geom_builder.ShapeType["VERTEX"])
        edgeNodes = mesh.GetSubMeshNodesId( v2,   all=False ) + \
                    mesh.GetSubMeshNodesId( edge, all=False ) + \
                    mesh.GetSubMeshNodesId( v1,   all=False )
        if not isForward: edgeNodes.reverse()
        nodes.extend( edgeNodes[:-1] )

    # create nodes inside the geomFace
    r1 = 0.6
    r2 = 1 - r1
    nodesInside = []
    for n in nodes:
        nXYZ = mesh.GetNodeXYZ( n )
        newXYZ = np.add( np.multiply( r1, gcXYZ ), np.multiply( r2, nXYZ ))
        nodesInside.append( mesh.AddNode( newXYZ[0], newXYZ[1], newXYZ[2] ))
        mesh.SetNodeOnFace( nodesInside[-1], geomFace, 0, 0 )

    # find out orientation of faces to create
    #    geomFace normal
    faceNorm = geom_builder.GetNormal( geomFace )
    v1,v2 = [ geom_builder.PointCoordinates( v ) \
              for v in geom_builder.SubShapeAll( faceNorm, geom_builder.ShapeType["VERTEX"]) ]
    faceNormXYZ = np.subtract( v2, v1 )
    outDirXYZ   = np.subtract( v1, [ 50, 50, 50 ] )
    if np.dot( faceNormXYZ, outDirXYZ ) < 0: # reversed face
        faceNormXYZ = np.multiply( -1., faceNormXYZ )
    #   mesh face normal
    e1 = np.subtract( mesh.GetNodeXYZ( nodes[0] ), mesh.GetNodeXYZ( nodes[1] ))
    e2 = np.subtract( mesh.GetNodeXYZ( nodes[0] ), mesh.GetNodeXYZ( nodesInside[0] ))
    meshNorm = np.cross( e1, e2 )
    #   faces orientation
    reverse = ( np.dot( faceNormXYZ, meshNorm ) < 0 )

    # create mesh faces
    iN = len( nodes )
    while iN:
        n1, n2, n3, n4 = nodes[iN-1], nodes[iN-2], nodesInside[iN-2], nodesInside[iN-1]
        iN -= 1
        if reverse:
            f = mesh.AddFace( [n1, n2, n3, n4] )
        else:
            f = mesh.AddFace( [n4, n3, n2, n1] )
        # new faces must be assigned to geometry to allow 3D algorithm finding them
        mesh.SetMeshElementOnShape( f, geomFace )

    if reverse:
        nodesInside.reverse()
    polygon = mesh.AddPolygonalFace( nodesInside )
    mesh.SetMeshElementOnShape( polygon, geomFace )

    return

# create geometry and get faces to mesh with my2DMeshing()
box = geom_builder.MakeBoxDXDYDZ( 100, 100, 100 )
f1 = geom_builder.SubShapeAll( box, geom_builder.ShapeType["FACE"])[0]
f2 = geom_builder.GetOppositeFace( box, f1 )
geom_builder.addToStudy( box, "box" )
geom_builder.addToStudy( f1, "f1" )
geom_builder.addToStudy( f2, "f2" )

# compute 1D mesh
mesh = smesh_builder.Mesh( box )
mesh.Segment().NumberOfSegments( 5 )
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# compute 2D mesh
mesh.Quadrangle()
mesh.UseExistingFaces(f1) # UseExistingFaces() allows using my2DMeshing();
mesh.UseExistingFaces(f2) # assign UseExistingFaces() BEFORE calling my2DMeshing()!
my2DMeshing(f1, mesh)
my2DMeshing(f2, mesh)
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# compute 3D mesh
mesh.Prism()
if not mesh.Compute(): raise Exception("Error when computing Mesh")
