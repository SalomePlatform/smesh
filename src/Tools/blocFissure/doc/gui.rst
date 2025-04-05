.. _gui:

With GUI
=====================================

To make the use of « Bloc Fissure » easier, a Graphical User Interface (GUI) was developed.
Inputs are similar to those of :ref:`script mode <script>`. GUI is accessible in the SMESH extension *Add a crack in a mesh*

.. image:: images/BF_GUI_capture.png
   :scale: 100
   :align: center
   :alt: Window of the graphical interface

Window of the graphical interface

+------------------------------------------------------------------------------------------------------------------+
|       GUI parameters                                                                                             |
+======================+===================================+=======================================================+
| **Maillage sain**    | *[string]*                        |  Relative address of the input structure 3D mesh      |
|                      | ex: mesh/maillage.med             |  in MED format (base is repertoire de travail)        |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Face fissure**     | *[string]*                        |  Relative address of the input crack geometry in      |
|                      | ex: mesh/fissure.xao              |  BREP or XAO format (base is repertoire de travail)   |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Arêtes fond        | *[list of strings]*               |  Definition of the crack front:                       |
| fissure**            | ex: ["F_1", "F_2"]                |  If XAO: list of the names of the edge group(s)       |
+----------------------+-----------------------------------+-------------------------------------------------------+
|                      | *[list of integers]*              |  If BREP (or XAO): list of edges number in GEOM       |
|                      | ex: [1,2]                         |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Distance           | *[float]*                         |  Length of influence - distance that defines the      |
| d'influence**        | ex: 20.0000                       |  size of the extracted Box around the crack           |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **min**              | *[float]*                         |  Float defining the minimum size of elements          |
|                      | ex: 1.00000                       |  to mesh the crack                                    |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **max**              | *[float]*                         |  Float defining the maximum size of elements          |
|                      | ex: 10.0000                       |  to mesh the crack                                    |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Rayon pipe**       | *[float]*                         |  Radius of the tore around the front                  |
|                      | ex: 5.00000                       |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Longueur mailles** | *[float]*                         |  Length of the segments of the tore along crack front |
|                      | ex: 5.00000                       |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Couronnes**        | *[integer]*                       |  Number of radial segment of the tore                 |
|                      | ex: 5                             |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Secteurs**         | *[integer]*                       |  Number of sectors of the tore                        |
|                      | ex: 16                            |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Triangles face     | *[float]*                         |  Mesh size of elements for the Box remeshing          |
| fissure**            | ex: 5.0000                        |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Répertoire de      | *[string]*                        |  Absolute address of the directory where files are    |
| travail**            | ex: /home/A123456                 |  saved                                                |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Nom du résultat**  | *[string]*                        |  Name of the resulting mesh                           |
|                      | ex: maillage_fissure              |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+
| **Mode**             | *[choice]*                        |  Verbose mode for Salome messages                     |
|                      | ex: verbose                       |                                                       |
+----------------------+-----------------------------------+-------------------------------------------------------+

.. image:: images/parametres_GUI.png
   :width: 350
   :align: center
   :alt: Tore parameters

Tore parameters


- **Example**: fills the panels with a test-case

- **Reset**: clears all the parameters

- **Précédent**: loads the last set of parameters used

- **Recharger**: loads a .dic file containing parameters

- **Sauver**: save a .dic file with the active parameters

- **Fermer**: exit GUI

- **OK**: Launch calculation

