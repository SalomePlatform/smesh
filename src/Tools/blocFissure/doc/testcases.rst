.. _test_cases:

#######################
Test cases
#######################

A base of 25 test cases ensures the proper functioning of « Bloc Fissure ». 12 cases are dedicated to :ref:`cracked bended pipes <pipeTC>` and the others 13 are for other :ref:`generic geometries <genericTC>`. They are shown here in order to illustrate cases on which « Bloc Fissure » works.

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

« Bloc Fissure » is very efficient for the case of bended pipes. The generation of the geometry and the crack insertion takes only around 15s.

Running test cases
=====================================

In SALOME interface:

0) **Generate med and breps files.** Once the files are generated, they are saved in SALOME source files. There is no need to generate them again::

    from blocFissure.materielCasTests import genereMateriel

Then the user can either launch all test cases, several at once or just one :

1) **To execute all test cases**::

    from blocFissure.CasTests import execution_Cas

2) **To execute only selected test cases**:

modify execution_Cas.py file and change::

    runall = False. #old : True

And change from 0 to 1 of the number of the test you want to launch::

    torun = [ 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,]

then launch the test cases::

    from blocFissure.CasTests import execution_Cas

3) **To execute only one test case**::

    from blocFissure.CasTests.[TEST_CASE_NAME] import [TEST_CASE_NAME]

    [TEST_CASE_NAME](0).executeProbleme()

[TEST_CASE_NAME] is the name of the test case in the following list. Note that the test cases fissureCoude_4 and fissure_Coude_4 are very similar.

+--------------------+--------------------+--------------------+-----------------+-----------------+
| faceGauche         | cylindre_2         | disquePerce        | fissureCoude_3  | fissureCoude_6  |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| faceGauche_2       | eprouvetteCourbe   | vis_1              | fissureCoude_4  | fissureCoude_7  |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| ellipse_1          | eprouvetteDroite   | fissureCoude       | fissure_Coude_4 | fissureCoude_8  |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| ellipse_2          | cubeAngle          | fissureCoude_1     | fissureCoude_5  | fissureCoude_9  |
+--------------------+--------------------+--------------------+-----------------+-----------------+
| cylindre           | cubeAngle2         | fissureCoude_2     | fissureCoude_5  | fissureCoude_10 |
+--------------------+--------------------+--------------------+-----------------+-----------------+



