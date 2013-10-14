import geompy
import smesh
import sys

# create a box
def Calcule():
   box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
   idbox = geompy.addToStudy(box, "box")
   
   # create a mesh
   tetra = smesh.Mesh(box, "MeshBox")
   
   algo1D = tetra.Segment()
   algo1D.NumberOfSegments(7)
   
   algo2D = tetra.Triangle()
   algo2D.MaxElementArea(800.)
   
   algo3D = tetra.Tetrahedron()
   algo3D.MaxElementVolume(900.)
   
   # compute the mesh
   ret = tetra.Compute()
   if ret == 0:
       print "problem when computing the mesh"
   else:
       print "mesh computed"
       pass
   tetra.ExportMED('/tmp/titi.med')
   from getStats import getStatsMaillage
   getStatsMaillage(tetra)

if __name__ == '__main__':
   print sys.argv
