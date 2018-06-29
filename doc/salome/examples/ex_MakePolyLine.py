import salome
salome.salome_init()

### create geometry

from salome.geom import geomBuilder
geompy = geomBuilder.New()

Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( Box_1, 'Box_1' )

### create a mesh

import SMESH
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh( Box_1 )
Mesh_1.Segment().NumberOfSegments(15)
Mesh_1.Triangle()
Mesh_1.Compute()

# define arguments for MakePolyLine

segments = []
# between nodes 20 and 1, default plane
segments.append( SMESH.PolySegment( 20, 0, 1, 0, smesh.MakeDirStruct(0,0,0) ))
# between nodes 1 and 100, default plane
segments.append( SMESH.PolySegment( 1, 0, 200, 0, smesh.MakeDirStruct(0,0,0) ))
# between nodes 200 and edge (578, 577), plane includes vector (1,1,1)
segments.append( SMESH.PolySegment( 200, 0, 578, 577, smesh.MakeDirStruct(1,1,1) ))

Mesh_1.MakePolyLine( segments, "1D group")


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
