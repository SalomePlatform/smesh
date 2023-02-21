#!/usr/bin/env python

import sys
import salome

import unittest

class SMESHExportOfFieldsInMemory(unittest.TestCase):

  def testMEDCouplingFieldOnCells(self):
    """
    Test focuses on ExportMEDCoupling method in the context of MED_CELL field output.
    """
    salome.standalone()
    salome.salome_init()
    ###
    ### SHAPER component
    ###

    from salome.shaper import model

    model.begin()
    partSet = model.moduleDocument()

    ### Create Part
    Part_1 = model.addPart(partSet)
    Part_1_doc = Part_1.document()

    ### Create Box
    Box_1 = model.addBox(Part_1_doc, 10, 10, 10)

    ### Create Plane
    Plane_4 = model.addPlane(Part_1_doc, model.selection("FACE", "PartSet/XOY"), 5, False)

    ### Create Plane
    Plane_5 = model.addPlane(Part_1_doc, model.selection("FACE", "PartSet/YOZ"), 5, False)

    ### Create Partition
    Partition_1_objects = [model.selection("FACE", "Plane_1"),
                          model.selection("FACE", "Plane_2"),
                          model.selection("SOLID", "Box_1_1")]
    Partition_1 = model.addPartition(Part_1_doc, Partition_1_objects, keepSubResults = True)

    ### Create Field
    Field_1_objects = [model.selection("SOLID", "Partition_1_1_2"),
                      model.selection("SOLID", "Partition_1_1_4"),
                      model.selection("SOLID", "Partition_1_1_1"),
                      model.selection("SOLID", "Partition_1_1_3")]
    Field_1 = model.addField(Part_1_doc, 1, "DOUBLE", 1, ["Comp 1"], Field_1_objects)
    Field_1.addStep(0, 0, [[0], [1], [2], [3], [4]])


    model.end()

    ###
    ### SHAPERSTUDY component
    ###

    model.publishToShaperStudy()
    import SHAPERSTUDY
    Partition_1_1, Field_1_1 = SHAPERSTUDY.shape(model.featureStringId(Partition_1))
    ###
    ### SMESH component
    ###

    import  SMESH, SALOMEDS
    from salome.smesh import smeshBuilder

    smesh = smeshBuilder.New()
    #smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                    # multiples meshes built in parallel, complex and numerous mesh edition (performance)

    Mesh_1 = smesh.Mesh(Partition_1_1)
    Regular_1D = Mesh_1.Segment()
    Local_Length_1 = Regular_1D.LocalLength(5,None,1e-07)
    Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
    Hexa_3D = Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)
    isDone = Mesh_1.Compute()
    if not isDone:
      raise Exception("Error when computing Mesh")
    smesh.SetName(Mesh_1, 'Mesh_1')

    #### Mesh_1.ExportMED( r'Mesh_with_one_field_on_cells.med', 0, 41, 1, Mesh_1.GetMesh(), 1, [ Field_1_1 ], '',-1 )
    mfd = Mesh_1.ExportMEDCoupling(0, Mesh_1.GetMesh(), 1, [ Field_1_1 ], '',-1 )#### <- important line of test is here !

    self.assertEqual(len(mfd.getMeshes()),1)
    self.assertEqual(len(mfd.getFields()),1)
    f = mfd.getFields()[0][0].field(mfd.getMeshes()[0])
    f.checkConsistencyLight()
    import medcoupling
    self.assertEqual(f.getDiscretization().getEnum(),medcoupling.ON_CELLS)
    self.assertTrue(f.getMesh().getNumberOfCells()>1)
    pass


  def testMEDCouplingFieldOnNodes(self):
    """
    Test focuses on ExportMEDCoupling method in the context of MED_NODES field output.
    """
    salome.standalone()
    salome.salome_init()

    ###
    ### SHAPER component
    ###

    from salome.shaper import model

    model.begin()
    partSet = model.moduleDocument()

    ### Create Part
    Part_1 = model.addPart(partSet)
    Part_1_doc = Part_1.document()

    ### Create Box
    Box_1 = model.addBox(Part_1_doc, 10, 10, 10)

    ### Create Plane
    Plane_4 = model.addPlane(Part_1_doc, model.selection("FACE", "PartSet/XOY"), 5, False)

    ### Create Plane
    Plane_5 = model.addPlane(Part_1_doc, model.selection("FACE", "PartSet/YOZ"), 5, False)

    ### Create Partition
    Partition_1_objects = [model.selection("FACE", "Plane_1"),
                          model.selection("FACE", "Plane_2"),
                          model.selection("SOLID", "Box_1_1")]
    Partition_1 = model.addPartition(Part_1_doc, Partition_1_objects, keepSubResults = True)

    ### Create Field
    Field_2_objects = [model.selection("VERTEX", "[Partition_1_1_2/Modified_Face&Box_1_1/Back][Partition_1_1_2/Modified_Face&Box_1_1/Left][Partition_1_1_2/Modified_Face&Box_1_1/Top]"),
                      model.selection("VERTEX", "Partition_1_1_2/Generated_Vertex&Plane_2/Plane_2&new_weak_name_1"),
                      model.selection("VERTEX", "[Partition_1_1_4/Modified_Face&Box_1_1/Front][Partition_1_1_4/Modified_Face&Box_1_1/Left][Partition_1_1_4/Modified_Face&Box_1_1/Top]"),
                      model.selection("VERTEX", "Partition_1_1_1/Generated_Vertex&Plane_1/Plane_1&new_weak_name_1"),
                      model.selection("VERTEX", "[Partition_1_1_1/Modified_Face&Box_1_1/Left][Partition_1_1_1/Modified_Face&Plane_1/Plane_1][Partition_1_1_1/Modified_Face&Plane_2/Plane_2]"),
                      model.selection("VERTEX", "Partition_1_1_3/Generated_Vertex&Plane_1/Plane_1&new_weak_name_1"),
                      model.selection("VERTEX", "[Partition_1_1_1/Modified_Face&Box_1_1/Back][Partition_1_1_1/Modified_Face&Box_1_1/Left][Partition_1_1_1/Modified_Face&Box_1_1/Bottom]"),
                      model.selection("VERTEX", "Partition_1_1_1/Generated_Vertex&Plane_2/Plane_2&new_weak_name_1"),
                      model.selection("VERTEX", "[Partition_1_1_3/Modified_Face&Box_1_1/Left][Partition_1_1_3/Modified_Face&Box_1_1/Bottom][Partition_1_1_3/Modified_Face&Box_1_1/Front]")]
    Field_2 = model.addField(Part_1_doc, 1, "DOUBLE", 1, ["Comp 1"], Field_2_objects)
    Field_2.addStep(0, 0, [[0], [1], [2], [3], [4], [5], [6], [7], [8], [9]])

    model.end()

    ###
    ### SHAPERSTUDY component
    ###

    model.publishToShaperStudy()
    import SHAPERSTUDY
    Partition_1_1, Field_2_1 = SHAPERSTUDY.shape(model.featureStringId(Partition_1))
    ###
    ### SMESH component
    ###

    import  SMESH, SALOMEDS
    from salome.smesh import smeshBuilder

    smesh = smeshBuilder.New()
    #smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                    # multiples meshes built in parallel, complex and numerous mesh edition (performance)

    Mesh_1 = smesh.Mesh(Partition_1_1)
    Regular_1D = Mesh_1.Segment()
    Local_Length_1 = Regular_1D.LocalLength(5,None,1e-07)
    Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
    Hexa_3D = Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)
    isDone = Mesh_1.Compute()
    if not isDone:
      raise Exception("Error when computing Mesh")
    smesh.SetName(Mesh_1, 'Mesh_1')

    # 23th of june 2021 : Bug both in ExportMED and in ExportMEDCoupling
    #Mesh_1.ExportMED( r'/tmp/Mesh_with_one_field_on_nodes.med', 0, 41, 1, Mesh_1.GetMesh(), 1, [ Field_2_1 ], '',-1 )
    #mfd = Mesh_1.ExportMEDCoupling(0,Mesh_1.GetMesh(), 1, [ Field_2_1 ], '',-1)

if __name__ == '__main__':
    unittest.main()
