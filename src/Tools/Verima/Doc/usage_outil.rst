Utilisation de l'outil
======================
Création de la base
--------------------

La base de données ne doit, à priori, n'être créée qu'une unique fois. Cependant, relancer le script de création ne changera pas les informations contenues dans la base et n'a pas d'incidence sur la base ni sur son contenu. Le script va seulement éventuellement ajouter dans la base, les références de la machine sur laquelle il est exécuté.

  * createDatabase.py

	- Crée la structure de la base et le fichier myMeshDB.db
        - Si le fichier myMeshDB.db (c'est à dire si la base) existe déjà, le script ne modifie pas la structure des tables et n'affecte pas les enregistrements qu'elles contiennent déjà.
        - Le script crée un enregistrement correspondant à la machine et à sa version d'OS (résultats de la commande uname). Si l'enregistrement existe déjà, la requête affiche l'erreur False et la database n'est pas touchée.  
        - Le script a un unique paramètre optionnel : le nom de la database. 

     .. code-block:: python

          python createDatabase -d maBaseAMoi.db

Le jeu de test créé par défaut 
-------------------------------

    * Pour les machines 
        - la machine sur laquelle la base est créée est ajoutée.

    * Pour les mailleurs
        - 1, Blsurf+Ghs3D
        - 2, Tetra


    * Pour les scripts
        -1,  script1 utilise le mailleur 2 dimension 2
        -2,  excavation utilise le mailleur 1 dimension 3

    * Pour les versions
        - Salome 7.3 comme version de référence
        - lorsqu'on utilise une version de Salome, cette version sera créée dans la database. Il n'est pas nécessaire de créer une autre version que la version de référence.

    * Pour les groupes de référence
        - Seul le script excavation a des groupes de référence : il s'agit de FRONT_07 et FOND_07, PAROI_07 et ROCHE_07, FOND_16, PAROI_16 et ROCHE_16.

    * Pour changer le jeu de test par défaut, il faut éditer dans la directory Base, les fichiers tableMailleurs, tableMaillages, tableGroupesRef et tableVersions  et modifier les méthodes "remplit". Par exemple, pour ajouter le mailleur "MonMailleur" :


         .. code-block:: python

          self.insereLigneAutoId(('MonMailleur'))




Ajouter un enregistrement à une table 
-------------------------------------

Lors d'une nouvelle version de Salome, de l'arrivée d'un nouveau cas test ou d'un nouveau mailleur, il sera nécessaire d'enrichir la base de données.  Aucun contrôle sur la cohérence des valeurs n'est effectué. 

  * l'autoincrement

   Les identifiants ("id") des tables Maillages, Mailleurs et Versions sont crées de façon automatique lors de l'insertion d'un enregistrement dans la table. Il n'est ni possible ni nécessaire de les préciser lors de la création d'un enregistrement.
   Ainsi, pour ajouter un mailleur, il suffit de spécifier son nom.

 
  * ajoutEnreg.py

        - Le script a deux paramètres : le nom de la databse (optionnel) et le nom de la table qu'il faut enrichir.
          les valeurs des colonnes doivent être fournies dans l'ordre.

         .. code-block:: python

            python ajoutEnreg -d maBaseAMoi.db -t TableMailleurs  "monMailleur"


        - Les contrôles effectués sont minimaux : nombre de valeurs de l'enregistrement et identifiant. En revanche, la cohérence entre tables n'est pas vérifiée. Ainsi, par exemple, pour entrer un nouveau cas test, il faut ajouter un enregistrement à la table des maillages. Si l'identifiant du mailleur n'existe pas, aucune erreur ne sera détectée.

         .. code-block:: python

          python ajoutEnreg -d maBaseAMoi.db -t TableMailleurs  "monMailleur"
          python ajoutEnreg -d maBaseAMoi.db -t TableMaillages  "monMaillage" "mesScripts/lanceMonMaillage" "/tmp/monFichierMed" 4 3 5 5 5 5 "essai pour mon Mailleur"

       


Changement de la version de référence
-------------------------------------
 
A priori, cette fonction ne devrait pas être utilisée. mais ... Elle permet de changer la version de référence.

  * changeRef.py

         .. code-block:: python

            python changeVersion.py Salome7.3 ou
            python changeVersion.py 3

Consultation des tables
-----------------------

  * le script Gui/visualiseDatabase.py (qui peut également être lancé de l'outil générique lance.py) permet de visualiser les tables contenues dans la base. (Attention au path de la database)

         .. code-block:: python

            python visualiseDatabase.py -d ../myMesh.db

.. image:: images/visualisation.png


Lancer un job de maillage particulier ou  l'ensemble des tests
----------------------------------------------------------------

   * le script passejob.py permet de passer l'ensemble des tests ou un cas particulier. il admet les options suivantes :

       - '-a' pour passer l ensemble des Tests (non activée par defaut)
       - '-s' pour preciser le path du runAppli (par exemple ~/Appli). permet au job de trouver le runAppli
       - '-d' pour preciser le fichier dataBase
       - '-v' pour spécifier la version de Salome
       - si l'option -a n'est pas activée, il faut préciser l'identifiant du job à passer 


         .. code-block:: python

            python passeJobs.py -d ../myMesh.db 1

Modifier les scripts pour les intégrer dans le mécanisme de test 
-------------------------------------------------------------------

    * ajout des lignes suivantes à la fin du script :

         .. code-block:: python

            from Stats.getStats import getStatsMaillage, getStatsGroupes, genHistogram
            # 
            fichierStatMailles=fichierMedResult.replace('.med','.res')
            fichierStatRatio=fichierMedResult.replace('.med','.ratio')
            fichierStatTailles=fichierMedResult.replace('.med','.taille')
            # 
            getStatsMaillage(monMaillage,fichierStatMailles)
            getStatsGroupes(monMaillage,fichierMedResult)
            genHistogram(monMaillage, SMESH.FT_AspectRatio3D, 20, False, fichierStatRatio,theStudy)
            genHistogram(monMaillage, SMESH.FT_MaxElementLength3D, 20, False, fichierStatTailles,theStudy)

    * si le maillage est du 2D, on remplace SMESH.FT_MaxElementLength3D par SMESH.FT_MaxElementLength2D et on ne calcule pas l'aspect Ratio

Lancement du script de comparaison
-----------------------------------
  * compareVersions.py
       - '-s' pour preciser le path du runAppli (par exemple ~/Appli). permet au job de trouver le runAppli
       - '-v' pour spécifier la version de Salome a comparer
       - '-r' pour spécifier une version de comparaison différente de la version de référence
       - '-d' pour preciser le fichier dataBase
       - '-f' pour spécifier le nom du fichier html produit (/tmp/toto.html par défaut -) )

   produit le rapport Html à ajouter à la newsletter


export/import de la base
-------------------------
  * exportDatabaseToCSV.py 

    - admet l option  -p (pour partiel) qui ne sauvegarde pas les tables a priori communes a tous : 
            * la table des mailleurs
            * la table des maillages
            * la table des versions
            * la table des machines
            * la table des groupes references

    - les fichiers sont ranges dans la directory ExportDB+date. la premiere ligne de chaque fichier contient le nom des colonnes, puis les valeurs par ligne

   - pour faire une vraie sauvegarde de la base (structure et donnees) il faut lancer sqlite3 et executer .dump


  * importDatabaseFromCSV.py

    - parametre d 'entree obligatoire : la directory qui contient les fichiers a importer 
    - admet l option  -p (pour partiel) qui n importe pas les tables a priori communes a tous  
    - admet l option  -f (pour force) qui pour les enregistrements qui existent déjà dans la base remplace 
      par les valeurs donnees dans le fichier
  
