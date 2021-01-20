.. _test_cases:

#######################
Test cases
#######################

A base of 30 test cases guarantees the correct functioning of « Bloc Fissure ». 12 cases are dedicated to :ref:`cracked bended pipes <pipeTC>` and the others 13 are for other :ref:`generic geometries <genericTC>`. They are shown here in order to illustrate cases on which « Bloc Fissure » works.

.. _genericTC:

Generic cases
===============================

All 13 cases are shown Table 2. Two views show the whole cracked structure and a zoom onto the crack with its tore.

+-------------------------------------------+----------------------------------------------+
| CubeAngle                                                                                |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/01_CubeAngle.png         | .. image:: images/01_CubeAngle_2.png         |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| CubeAngle2                                                                               |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/02_CubeAngle2.png        | .. image:: images/02_CubeAngle2_2.png        |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| cylindre                                                                                 |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/03_cylindre.png          | .. image:: images/03_cylindre_2.png          |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| cylindre2                                                                                |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/04_cylindre2.png         | .. image:: images/04_cylindre2_2.png         |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| disque                                                                                   |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/05_disque.png            | .. image:: images/05_disque_2.png            |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| ellipse1                                                                                 |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/06_ellipse1.png          | .. image:: images/06_ellipse1_2.png          |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| ellipse2 (tilted crack)                                                                  |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/07_ellipse2.png          | .. image:: images/07_ellipse2_2.png          |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| eprouvetteCourbe                                                                         |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/08_eprouvetteCourbe.png  | .. image:: images/08_eprouvetteCourbe_2.png  |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| eprouvetteDroite                                                                         |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/09_eprouvetteDroite.png  | .. image:: images/09_eprouvetteDroite_2.png  |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| eprouvetteDroite2                                                                        |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/10_eprouvetteDroite2.png | .. image:: images/10_eprouvetteDroite2_2.png |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| faceGauche                                                                               |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/11_faceGauche.png        | .. image:: images/11_faceGauche_2.png        |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| faceGauche2                                                                              |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/12_faceGauche2.png       | .. image:: images/12_faceGauche2_2.png       |
|   :height: 300                            |    :height: 300                              |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+
| vis (Crack is in the radius filet between the screw head and the screw shank)            |
+-------------------------------------------+----------------------------------------------+
|.. image:: images/13_vis_1.png             | .. image:: images/13_vis_1_2.png             |
|   :width: 400                             |    :width: 400                               |
|   :align: center                          |    :align: center                            |
+-------------------------------------------+----------------------------------------------+

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

Running test cases
=====================================

All the files for these test cases are stored in the directory of the installation of SALOME:

``Salome-VXXXX_package-YY/modules/SMESH_VXXXX/lib/python3.6/site-packages/salome/blocFissure``

The test cases can be runned either through the python window of the SALOME GUI or with a python script.

0) **Generate med and breps files.**::

    from blocFissure.materielCasTests import genereMateriel

Once the files are generated, they are saved into the directory of the installation of SALOME:

``Salome-VXXXX_package-YY/modules/SMESH_VXXXX/lib/python3.6/site-packages/salome/blocFissure/materielCasTests``

There is no need to generate them again.

1) **To execute all test cases**::

    from blocFissure.CasTests import execution_Cas

2) **To execute only selected test cases**:

modify the file ``CasTests/execution_Cas.py`` and change::

    runall = False. #old : True

And change from 0 to 1 of the index of the test you want to launch::

    torun = [ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

then launch the test cases::

    from blocFissure.CasTests import execution_Cas

The index of each test is the position of the test in the following table, from 0 for cubeAngle, 1 for cubeAngle2,
2 for cubeCoin... from top to bottom, then from left to right.

+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeAngle          | cylindre_2         | eprouvetteDroite_2 | fissureCoude_3  | fissureCoude_8  |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeAngle2         | disquePerce        | faceGauche         | fissureCoude_4  | fissureCoude_10 |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeCoin           | ellipse_1          | faceGauche_2       | fissure_Coude_4 | fissureCoude_10 |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeMilieu         | ellipse_2          | fissure_Coude      | fissureCoude_5  | vis_1           |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cubeTransverse     | eprouvetteCourbe   | fissureCoude_1     | fissureCoude_6  |                 |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cylindre           | eprouvetteDroite   | fissureCoude_2     | fissureCoude_7  |                 |
+--------------------+--------------------+--------------------+-----------------+-----------------+

3) **To execute only one test case**::

    from blocFissure.CasTests.[TEST_CASE_NAME] import [TEST_CASE_NAME]

    [TEST_CASE_NAME](0).executeProbleme()

[TEST_CASE_NAME] is the name of the test case in the previous list. Note that the test cases fissureCoude_4 and fissure_Coude_4 are very similar.



