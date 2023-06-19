#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2018-2023  CEA, EDF, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

import unittest

class TestMesh(unittest.TestCase):

    def setUp(self):
        import salome
        salome.salome_init()

    def processGuiEvents(self):
        import salome
        if salome.sg.hasDesktop():
            salome.sg.updateObjBrowser();
            import SalomePyQt
            SalomePyQt.SalomePyQt().processEvents()

    def test_mesh(self):
        """Quick test for Mesh module"""

        print()
        print('Testing Mesh module')

        # ==== Geometry part ====

        from salome.geom import geomBuilder
        geompy = geomBuilder.New()

        # ---- create box
        print('... Create box')
        box = geompy.MakeBox(0., 0., 0., 100., 200., 300)
        self.assertIsNotNone(box)
        self.processGuiEvents()

        # ==== Mesh part ====

        import SMESH
        from salome.smesh import smeshBuilder
        smesh = smeshBuilder.New()
        self.processGuiEvents()

        # ---- create hypotheses 
        print('... Create hypotheses')

        # **** create local length 
        print('...... Local Length')
        local_length = smesh.CreateHypothesis('LocalLength')
        self.assertIsNotNone(local_length)
        local_length.SetLength(100)
        self.assertEqual(local_length.GetName(), 'LocalLength')
        self.assertEqual(local_length.GetLength(), 100)
        self.processGuiEvents()

        # **** create number of segments
        print('...... Number Of Segments')
        nb_segments = smesh.CreateHypothesis('NumberOfSegments')
        self.assertIsNotNone(nb_segments)
        nb_segments.SetNumberOfSegments(7)
        self.assertEqual(nb_segments.GetName(), 'NumberOfSegments')
        self.assertEqual(nb_segments.GetNumberOfSegments(), 7)
        self.processGuiEvents()

        # **** create max element area
        print('...... Max Element Area')
        max_area = smesh.CreateHypothesis('MaxElementArea')
        max_area.SetMaxElementArea(2500)
        self.assertEqual(max_area.GetName(), 'MaxElementArea')
        self.assertEqual(max_area.GetMaxElementArea(), 2500)
        self.processGuiEvents()

        # ---- create algorithms
        print('... Create algorithms')

        # **** create regular 1d
        print('...... Regular 1D')
        regular = smesh.CreateHypothesis('Regular_1D')
        self.assertTrue(len(regular.GetCompatibleHypothesis()) > 0)
        self.assertEqual(regular.GetName(), 'Regular_1D')
        self.processGuiEvents()

        # **** create quadrangle mapping
        print('...... Quadrangle_2D')
        quad = smesh.CreateHypothesis('Quadrangle_2D')
        self.assertTrue(len(quad.GetCompatibleHypothesis()) > 0)
        self.assertEqual(quad.GetName(), 'Quadrangle_2D')
        self.processGuiEvents()

        # ---- create mesh on box
        print('... Create mesh on box')
        mesh = smesh.CreateMesh(box)
        self.assertEqual(mesh.AddHypothesis(box, regular)[0], SMESH.HYP_OK)
        self.assertEqual(mesh.AddHypothesis(box, quad)[0], SMESH.HYP_OK)
        self.assertEqual(mesh.AddHypothesis(box, nb_segments)[0], SMESH.HYP_OK)
        self.assertEqual(mesh.AddHypothesis(box, max_area)[0], SMESH.HYP_OK)
        self.processGuiEvents()

        # ---- compute mesh
        print('... Compute mesh')
        self.assertEqual(smesh.Compute(mesh, box), True)
        self.processGuiEvents()

if __name__ == '__main__':
    unittest.main()
