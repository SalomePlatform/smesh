.. _test_cases:

#######################
Test cases
#######################

A base of 28 test cases guarantees the correct functioning of « Bloc Fissure ». 12 cases are dedicated to :ref:`cracked bended pipes <pipeTC>` and the others 16 are for other :ref:`generic geometries <genericTC>`. They are shown here in order to illustrate cases on which « Bloc Fissure » works.

.. _genericTC:

Generic cases
===============================

All 16 cases are shown Table 2. Two views show the whole cracked structure and a zoom onto the crack with its tore.

+--------------------------------------------+-----------------------------------------------+
| CubeAngle                                                                                  |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/01_CubeAngle.png          | .. image:: images/01_CubeAngle_a.png          |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| CubeAngle_2                                                                                |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/02_CubeAngle_2.png        | .. image:: images/02_CubeAngle_2_a.png        |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| cubeCoin                                                                                   |
+--------------------------------------------+-----------------------------------------------+
| cubeMilieu                                                                                 |
+--------------------------------------------+-----------------------------------------------+
| cubeTransverse                                                                             |
+--------------------------------------------+-----------------------------------------------+
| cylindre                                                                                   |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/03_cylindre.png           | .. image:: images/03_cylindre_a.png           |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| cylindre_2                                                                                 |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/04_cylindre_2.png         | .. image:: images/04_cylindre_2_a.png         |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| disquePerce                                                                                |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/05_disque.png             | .. image:: images/05_disque_a.png             |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| ellipse_1                                                                                  |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/06_ellipse_1.png          | .. image:: images/06_ellipse_1_a.png          |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| ellipse_2 (tilted crack)                                                                   |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/07_ellipse_2.png          | .. image:: images/07_ellipse_2_a.png          |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| eprouvetteCourbe                                                                           |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/08_eprouvetteCourbe.png   | .. image:: images/08_eprouvetteCourbe_a.png   |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| eprouvetteDroite                                                                           |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/09_eprouvetteDroite.png   | .. image:: images/09_eprouvetteDroite_a.png   |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| eprouvetteDroite_2                                                                         |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/10_eprouvetteDroite_2.png | .. image:: images/10_eprouvetteDroite_2_a.png |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| faceGauche                                                                                 |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/11_faceGauche.png         | .. image:: images/11_faceGauche_2.png         |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| faceGauche_2                                                                               |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/12_faceGauche_2.png       | .. image:: images/12_faceGauche_2_a.png       |
|   :height: 300                             |    :height: 300                               |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| vis (Crack is in the radius filet between the screw head and the screw shank)              |
+--------------------------------------------+-----------------------------------------------+
|.. image:: images/13_vis_1.png              | .. image:: images/13_vis_1_2.png              |
|   :width: 400                              |    :width: 400                                |
|   :align: center                           |    :align: center                             |
+--------------------------------------------+-----------------------------------------------+
| tube                                                                                       |
+--------------------------------------------+-----------------------------------------------+

.. _pipeTC:

Cracked bended pipes
=====================================

All 12 bended pipe test cases are not shown here because they are relatively similars with each others. Only test case cas coude_0 et coude_3 are presented, those two tests being based on the same geometry and having a crack on the internal and on the external surface.

+--------------------------------+----------------------------------------------+
| .. image:: images/coude_0.png  | .. image:: images/coude_0_2.png              |
|    :height: 500                |    :width: 500                               |
|    :align: center              |    :align: center                            |
|                                +----------------------------------------------+
|                                | View of the external crack of test_coude_0   |
|                                +----------------------------------------------+
|                                | .. image:: images/coude_3.png                |
|                                |    :width: 500                               |
|                                |    :align: center                            |
+--------------------------------+----------------------------------------------+
| Bended pipe test case coude_0  | View of the internal crack of test_coude_3   |
| with an external crack         |                                              |
+--------------------------------+----------------------------------------------+

coude_0 can be computed using this file for the data:

  * :download:`data for a bended puipe <../ihm/dialogFissureCoude.dic>`

« Bloc Fissure » is very efficient for the case of bended pipes. The generation of the geometry and the crack insertion takes only around 15s.

Data for the test cases
=====================================

All the files for these test cases are stored in the directory of the installation of SALOME:

``Salome-VXXXX_package-YY/modules/SMESH_VXXXX/lib/python3.6/site-packages/salome/blocFissure``

The set of the med, xao and brep files can be created by the following importation:

    from blocFissure.materielCasTests import genereMateriel

Once the files are generated, they are saved into the directory of the installation of SALOME:

``Salome-VXXXX_package-YY/modules/SMESH_VXXXX/lib/python3.6/site-packages/salome/blocFissure/materielCasTests``

There is no need to generate them again.

To check the installation of salome, the test cases can be computed by the salome test process::

    salome -test blocFissure

Running test cases
=====================================

The test cases can be runned by the following instructions::

    from blocFissure.CasTests.blocFissureTest import blocFissureTest
    BLOCFISSURE_TEST = blocFissureTest(list_of_cases)
    MESSAGE_ERREUR = BLOCFISSURE_TEST.lancement()
    sys.stdout.write(MESSAGE_ERREUR)
    del BLOCFISSURE_TEST

If ``list_of_cases`` is an empty list, all the cases are computed.

If not, ``list_of_cases`` is made of the list of the numbers and/or the name of the requested case. The index of each test is the position of the test in the following table, from 0 for cubeAngle, 1 for cubeAngle_2, 2 for cubeCoin... from top to bottom, then from left to right.

+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeAngle          | cylindre_2         | eprouvetteDroite_2 | fissureCoude_3  | fissureCoude_9  |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeAngle_2        | disquePerce        | faceGauche         | fissureCoude_4  | fissure_Coude   |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeCoin           | ellipse_1          | faceGauche_2       | fissureCoude_5  | fissure_Coude_4 |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeMilieu         | ellipse_2          | fissureCoude_1     | fissureCoude_6  | vis_1           |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeTransverse     | eprouvetteCourbe   | fissureCoude_10    | fissureCoude_7  |                 |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cylindre           | eprouvetteDroite   | fissureCoude_2     | fissureCoude_8  |                 |
+--------------------+--------------------+--------------------+-----------------+-----------------+

