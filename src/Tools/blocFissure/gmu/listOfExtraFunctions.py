# -*- coding: utf-8 -*-
"""
Created on Mon Jun 23 14:49:36 2014

@author: I48174 (Olivier HOAREAU)
"""

import logging
import SMESH
from geomsmesh import smesh

def lookForCorner(maillageAScanner):
    
    """ Cette fonction permet de scanner la liste de noeuds qui composent le
        maillage passé en paramètre. On recherche un ou plusieurs coins, ce
        qui implique les caractéristiques suivantes:
            - le noeud doit appartenir au moins à trois éléments distincts
            - chaque élément doit appartenir à un ensemble distinct
        La fonction renvoie une liste de coins par l'intermédiaire de l'IDs
        chaque noeud. La liste contient en général au maximum deux coins.
    """
    
    logging.info("start")
    
    allNodeIds = maillageAScanner.GetNodesId()  # On stocke tout les noeuds
    listOfCorners = []
    for ND in allNodeIds:
        # On parcours la liste de noeuds
        listOfElements = maillageAScanner.GetNodeInverseElements(ND)
        if len(listOfElements) >=3:
            # On teste le nombre d'éléments qui partagent le même noeud
            # --- Filtre selon le critère 'coplanar' --- #
            listOfCriterion = [smesh.GetCriterion(SMESH.FACE, SMESH.FT_CoplanarFaces, \
                               SMESH.FT_Undefined, elem, SMESH.FT_Undefined, SMESH.FT_Undefined, 30) \
                               for elem in listOfElements]
            listOfFilters = [smesh.GetFilterFromCriteria([criteria]) for criteria in listOfCriterion]
            listOfSets = [maillageAScanner.GetIdsFromFilter(filter) for filter in listOfFilters]
            if listOfSets.count(listOfSets[0]) == len(listOfSets):
                # Si toutes les listes d'éléments sont similaires, on retourne
                # au début pour éviter de travailler sur des éléments inutiles.
                # Exemple : un noeud appartenant à 4 éléments sur la même face.
                continue
            for s in listOfSets:
                while listOfSets.count(s) > 1:
                    # On supprime tant que la liste d'éléments n'est pas unique.
                    listOfSets.remove(s)
            if len(listOfSets) >= 3:
                # Si on a au moins 3 listes d'élements différentes, on considère
                # qu'il y a présence d'un coin.
                listOfCorners.append(ND)
    return listOfCorners

def createLinesFromMesh(maillageSupport):
    
    """ Cette fonction permet de générer une liste de lignes à partir du 
        maillage support passé en paramètre. On démarre à partir d'un coin
        simple et on parcourt tout les noeuds pour former une ligne. Soit la
        figure ci-dessous :
            
            1_____4_____7    On part du coin N1, et on cherche les noeuds
            |     |     |    successifs tels que [1, 2, 3]. Lorsqu'on arrive
            |  1  |  3  |    arrive sur le noeud de fin de ligne N3, on repart
            |     |     |    du noeud précédent du premier élément (E1), à
            2_____5_____8    savoir le noeud N4. On suit les noeuds succesifs
            |     |     |    [4, 5, 6] comme précédemment et ainsi de suite.
            |  2  |  4  |    Lorsqu'on arrive sur le dernier noeud de la
            |     |     |    dernière ligne, à savoir le noeud N9, on considère
            3_____6_____9    que toutes les lignes sont créées.
            
        La fonction retourne une liste de lignes utilisées par la suite.
    """
    
    logging.info("start")
    
    allNodeIds = maillageSupport.GetNodesId()
    while len(allNodeIds):
        nodeIds = allNodeIds
        for idNode in nodeIds: # rechercher un coin
          elems = maillageSupport.GetNodeInverseElements(idNode)
          if len(elems) == 1:
            # un coin: un noeud, un element quadrangle
            elem = elems[0]
            break;
        idStart = idNode # le noeud de coin
        elemStart = elem # l'élément quadrangle au coin
        xyz = maillageSupport.GetNodeXYZ(idStart)
        logging.debug("idStart %s, coords %s", idStart, str(xyz))
    
        nodelines =[] # on va constituer une liste de lignes de points
        nextLine = True
        ligneFinale = False
        while nextLine:
            logging.debug("--- une ligne")
            idNode = idStart
            elem = elemStart
            if ligneFinale:
                agauche = False  # sens de parcours des 4 noeuds d'un quadrangle
                nextLine = False
            else:
                agauche = True
            ligneIncomplete = True  # on commence une ligne de points
            debutLigne = True
            nodeline = []
            elemline = []
            while ligneIncomplete:  # compléter la ligne de points
                nodeline.append(idNode)
                allNodeIds.remove(idNode)
                elemline.append(elem)
                nodes = maillageSupport.GetElemNodes(elem)
                i = nodes.index(idNode)  # repérer l'index du noeud courant (i) dans l'élément quadrangle (0 a 3)
                if agauche:              # déterminer le noeud suivant (j) et celui opposÃ© (k) dans le quadrangle
                    if i < 3:
                        j = i+1
                    else:
                        j = 0
                    if j < 3:
                        k = j+1
                    else:
                        k = 0
                else:
                    if i > 0:
                        j = i -1
                    else:
                        j = 3
                    if j > 0:
                        k = j -1
                    else:
                        k = 3
                isuiv = nodes[j]   # noeud suivant
                iapres = nodes[k]  # noeud opposé
                if debutLigne:
                    debutLigne = False
                    # précédent a trouver, dernière ligne : précédent au lieu de suivant
                    if agauche:
                        if i > 0:
                            iprec = nodes[i -1]
                        else:
                            iprec = nodes[3]
                        idStart = iprec
                        elems3 = maillageSupport.GetNodeInverseElements(iprec)
                        if len(elems3) == 1: # autre coin
                            ligneFinale = True
                        else:
                            for elem3 in elems3:
                                if elem3 != elem:
                                    elemStart = elem3
                                    break
                #print nodes, idNode, isuiv, iapres
                elems1 = maillageSupport.GetNodeInverseElements(isuiv)
                elems2 = maillageSupport.GetNodeInverseElements(iapres)
                ligneIncomplete = False
                for elem2 in elems2:
                    if elems1.count(elem2) and elem2 != elem:
                        ligneIncomplete = True
                        idNode = isuiv
                        elem = elem2
                        break
                if not  ligneIncomplete:
                    nodeline.append(isuiv)
                    allNodeIds.remove(isuiv)
            logging.debug("nodeline %s", nodeline)
            logging.debug("elemline %s", elemline)
            nodelines.append(nodeline)
             
        # on a constitué une liste de lignes de points connexes
        logging.debug("dimensions [%s, %s]", len(nodelines),  len(nodeline))
    
    return nodelines

def createNewMeshesFromCorner(maillageSupport, listOfCorners):
    
    """ Cette fonction permet de générer un nouveau maillage plus facile à
        utiliser. On démarre d'un coin et on récupère les trois éléments
        auquel le noeud appartient. Grâce à un filtre 'coplanar' sur les trois
        éléments, on peut générer des faces distinctes.
    """
    
    logging.info("start")
    
    tmp = []
    listOfNewMeshes = []
    for corner in listOfCorners:
        elems = maillageSupport.GetNodeInverseElements(corner)
        for i, elem in enumerate(elems):
            # --- Filtre selon le critère 'coplanar' --- #
            critere = smesh.GetCriterion(SMESH.FACE, SMESH.FT_CoplanarFaces, \
                                         SMESH.FT_Undefined, elem, SMESH.FT_Undefined, SMESH.FT_Undefined, 30)
            filtre = smesh.GetFilterFromCriteria([critere])
            grp = maillageSupport.GroupOnFilter(SMESH.FACE, 'grp', filtre)
            # On copie le maillage en fonction du filtre
            msh = smesh.CopyMesh(grp, 'new_{0}'.format(i + 1), False, True)
            # On stocke l'ensemble des noeuds du maillage dans tmp
            # On ajoute le maillage à la liste des nouveaux maillages
            # seulement s'il n'y est pas déjà
            tmp.append(msh.GetNodesId())
            if tmp.count(msh.GetNodesId()) <= 1:
                listOfNewMeshes.append(msh)
    return listOfNewMeshes