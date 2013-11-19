# -*- coding: iso-8859-1 -*-

###
### Copyright EDF R&D 2012
###
### Gérald NICOLAS
### (33/0)1.47.65.56.94
###

import sys
import salome

import os
#==================== Personnalisation - Debut =========================
# Les fichiers MED des deux maillages seront exportes dans ce repertoire.
# Comportement par defaut : l'export est inactive (ligne 1099)
# The MED files for both two meshes are exported in this directory.
# Default behaviour : no export (line 1099)
HOME = os.environ["HOME"]
rep_GEOM_MAIL = os.path.join(HOME, "ASTER_USER", "TEST", "Excavation", "GEOM_MAIL")
#==================== Personnalisation - Fin ===========================

salome.salome_init()
theStudy = salome.myStudy
#
verbose = 1
verbose_max = 0
#
erreur = 0
#
l_cas = [ "tout", "syme" ]

###
### GEOM component
###
import GEOM
import math
import SALOMEDS
from salome.geom import geomBuilder
geompy = geomBuilder.New(theStudy) 
#
# 0. Les conventions d'orientation des axes sont les suivantes :
#    . l'axe Z s'enfonce dans le plan du dessin, son origine étant dans le plan
#    . l'axe Y est vers le haut
#    . l'axe X est vers la gauche.
#    . l'origine du repere est au centre du tunnel
#    Remarque : Les suffixes G (gauche), D (droit), B(bas) et H (haut) doivent
#    etre compris quand on regarde depuis l'entree du tunnel, sa partie
#    plane etant en bas.
#
# 1. Dimensions globales
# 1.1. Dimensions caracteristiques du domaine
#      Taille generale du bloc contenant le tunnel
LG_ARETE_BLOC = 4.6
#      Taille generale du massif
LG_ARETE_MASSIF = LG_ARETE_BLOC*15.0
#      Longueur du tunnel
LGTUNNEL = 20.0
#      Abscisse du centre du cylindre de percage 1 (+ ou -)
X1 = 2.0
#      Abscisse du centre du cylindre de percage 2 (+ ou -)
X2 = 1.0
#      Nombre de couches dans le tunnel
NC = 20
#
# 1.2. Dimensions deduites
#
# 1.2.1. Rayons des cylindres de percement
#      . Rayon du cylindre superieur
RAYON_H = 0.5*LG_ARETE_BLOC
#      . Rayon du premier cylindre inferieur
RAYON_1 = RAYON_H + X1
#      . Rayon du premier cylindre inferieur
#      sinus = sin(angle), angle entre le plan horizontal et
#      le plan d'intersection des deux cylindres inférieurs
sinus = ( (X1+X2)**2 - X1**2 ) / ( (X1+X2)**2 + X1**2 )
if verbose_max :
  print "sinus =", sinus
tangente = math.tan(math.asin(sinus))
if verbose_max :
  print "tangente =", tangente
Y2 = - (X1+X2)*tangente
if verbose_max :
  print "Y2 =", Y2
RAYON_2 = RAYON_H + Y2
#
if verbose_max :
  print "RAYON_H =", RAYON_H
  print "RAYON_1 =", RAYON_1
  print "RAYON_2 =", RAYON_2
#
# 1.2.2. Longueur de la boite qui servira a la construction des cavites
LG_OUTIL = 2.0 * LG_ARETE_MASSIF
#
# 1.2.3. Decalage pour que tout le massif soit traverse
DELTA = 0.01*LG_ARETE_MASSIF
#
# 2. Les cylindres du tunnelier
#
d_cyl = {}
#
LG_OUTIL = LG_OUTIL + 2.*DELTA
#
OUTIL_H = geompy.MakeCylinderRH(RAYON_H, LG_OUTIL)
TRX = 0.0
TRY = 0.0
TRZ = - DELTA
geompy.TranslateDXDYDZ(OUTIL_H, TRX, TRY, TRZ)
d_cyl["TUN_H"] = (RAYON_H, TRX, TRY)
#
OUTIL_G_1 = geompy.MakeCylinderRH(RAYON_1, LG_OUTIL)
TRX = -X1
geompy.TranslateDXDYDZ(OUTIL_G_1, TRX, TRY, TRZ)
d_cyl["TUN_G1"] = (RAYON_1, TRX, TRY)
#
OUTIL_D_1 = geompy.MakeCylinderRH(RAYON_1, LG_OUTIL)
TRX = X1
geompy.TranslateDXDYDZ(OUTIL_D_1, TRX, TRY, TRZ)
d_cyl["TUN_D1"] = (RAYON_1, TRX, TRY)
#
OUTIL_G_2 = geompy.MakeCylinderRH(RAYON_2, LG_OUTIL)
TRX = X2
TRY = Y2
geompy.TranslateDXDYDZ(OUTIL_G_2, TRX, TRY, TRZ)
d_cyl["TUN_G2"] = (RAYON_2, TRX, TRY)
#
OUTIL_D_2 = geompy.MakeCylinderRH(RAYON_2, LG_OUTIL)
TRX = -X2
geompy.TranslateDXDYDZ(OUTIL_D_2, TRX, TRY, TRZ)
d_cyl["TUN_D2"] = (RAYON_2, TRX, TRY)
#
if verbose :
  print "Cylindre"
  for cle in d_cyl.keys() :
    t_aux = d_cyl[cle]
    print cle, ": rayon =", t_aux[0], ", centre (", t_aux[1], ",", t_aux[2], ")"
#
# 3. L'empreinte de decoupe
# 3.1. Les boites de base
#
DX = 2.0*LG_ARETE_BLOC
DY = 2.0*LG_ARETE_BLOC
DZ = LG_OUTIL
B_B = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
#    En haut
TRX = -LG_ARETE_BLOC
TRY = 0.0
TRZ = - DELTA
B_H = geompy.MakeTranslation(B_B, TRX, TRY, TRZ)
#    A gauche
TRX = X2
TRY = -2.0*LG_ARETE_BLOC
B_G = geompy.MakeTranslation(B_B, TRX, TRY, TRZ)
#    A droite
TRX = -2.0*LG_ARETE_BLOC - X2
TRY = -2.0*LG_ARETE_BLOC
B_D = geompy.MakeTranslation(B_B, TRX, TRY, TRZ)
#    En bas
TRX = -LG_ARETE_BLOC
TRY = -2.5*LG_ARETE_BLOC
geompy.TranslateDXDYDZ(B_B, TRX, TRY, TRZ)
#
# 3.2. Plans d'intersection entre les cylindres inférieurs
#      Centre des premiers cylindres inférieurs
DX = -X1
DY = 0.0
DZ = 0.0
CG1 = geompy.MakeVertex(DX, DY, DZ)
DX = X1
CD1 = geompy.MakeVertex(DX, DY, DZ)
#      Vecteurs normaux aux plans des intersections des cylindres inférieurs
DX = -Y2
DY = X1+X2
DZ = 0.0
VG = geompy.MakeVectorDXDYDZ(DX, DY, DZ)
DX = Y2
VD = geompy.MakeVectorDXDYDZ(DX, DY, DZ)
#      Plans des intersections des cylindres inférieurs
DZ = 2.0*LG_OUTIL
PG = geompy.MakePlane(CG1, VG, DZ)
PD = geompy.MakePlane(CD1, VD, DZ)
#
# 3.3. Decoupes
# 3.3.1. Partie superieure
B_H1 = geompy.MakeCut(B_H, OUTIL_H)
#
# 3.3.2. Partie gauche
# . Séparation par le plan de l'intersection
B_G1 = geompy.MakeHalfPartition(B_G, PG)
# . Création des deux volumes internes
L_AUX = geompy.MakeBlockExplode(B_G1, 6, 6)
# . Repérage du volume interne supérieur
DX = 2.0*LG_ARETE_BLOC + 2.0*DELTA
DY = 2.0*LG_ARETE_BLOC + 2.0*DELTA
DZ = LG_OUTIL + 2.0*DELTA
boite_aux_G = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
TRX = 0.0
TRY = -2.0*LG_ARETE_BLOC + DELTA
TRZ = - 2.0*DELTA
geompy.TranslateDXDYDZ(boite_aux_G, TRX, TRY, TRZ)
l_aux = geompy.GetShapesOnBox (boite_aux_G, B_G1, geompy.ShapeType["SOLID"], GEOM.ST_IN )
#print "l_aux =", l_aux
B_G2 = geompy.MakeCut(l_aux[0], OUTIL_G_1)
# . Repérage du volume interne inférieur
TRX = 0.0
TRY = -RAYON_H - DELTA
TRZ = 0.0
geompy.TranslateDXDYDZ(boite_aux_G, TRX, TRY, TRZ)
l_aux = geompy.GetShapesOnBox (boite_aux_G, B_G1, geompy.ShapeType["SOLID"], GEOM.ST_IN )
B_G3 = geompy.MakeCut(l_aux[0], OUTIL_G_2)
#
# 3.3.3. Partie droite
# . Séparation par le plan de l'intersection
B_D1 = geompy.MakeHalfPartition(B_D, PD)
# . Création des deux volumes internes
L_AUX = geompy.MakeBlockExplode(B_D1, 6, 6)
# . Repérage du volume interne supérieur
DX = 2.0*LG_ARETE_BLOC + 2.0*DELTA
DY = 2.0*LG_ARETE_BLOC + 2.0*DELTA
DZ = LG_OUTIL + 2.0*DELTA
boite_aux_D = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
TRX = -DX
TRY = -2.0*LG_ARETE_BLOC + DELTA
TRZ = - 2.0*DELTA
geompy.TranslateDXDYDZ(boite_aux_D, TRX, TRY, TRZ)
l_aux = geompy.GetShapesOnBox (boite_aux_D, B_D1, geompy.ShapeType["SOLID"], GEOM.ST_IN )
if ( verbose_max ) :
  print "3.3.3. supérieur l_aux =", l_aux
B_D2 = geompy.MakeCut(l_aux[0], OUTIL_D_1)
# . Repérage du volume interne inférieur
TRX = 0.0
TRY = -RAYON_H - DELTA
TRZ = 0.0
geompy.TranslateDXDYDZ(boite_aux_D, TRX, TRY, TRZ)
l_aux = geompy.GetShapesOnBox (boite_aux_D, B_D1, geompy.ShapeType["SOLID"], GEOM.ST_IN )
if ( verbose_max ) :
  print "3.3.3. inférieur l_aux =", l_aux
B_D3 = geompy.MakeCut(l_aux[0], OUTIL_D_2)
#
# 3.3. Fusion
#
Union_1 = geompy.MakeFuse(B_B    , B_G3)
Union_2 = geompy.MakeFuse(Union_1, B_D3)
Union_3 = geompy.MakeFuse(Union_2, B_G2)
Union_4 = geompy.MakeFuse(Union_3, B_D2)
EMPREINTE = geompy.MakeFuse(Union_4, B_H1)
#
# 4. Le tunnel
# 4.1. La base
#
DX = LG_ARETE_BLOC + 2.0*DELTA
DY = LG_ARETE_BLOC + 2.0*DELTA
DZ = LGTUNNEL
BOITE_TUNNEL = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
TRX = -RAYON_H - DELTA
TRY = -RAYON_H - DELTA
TRZ = 0.0
geompy.TranslateDXDYDZ(BOITE_TUNNEL, TRX, TRY, TRZ)
#
TUNNEL_PLEIN = geompy.MakeCut(BOITE_TUNNEL, EMPREINTE)
#
# 4.2. Partitionnement par les cavites
#
DX = LG_ARETE_BLOC + 2.0*DELTA
DY = LG_ARETE_BLOC + 2.0*DELTA
DZ = LGTUNNEL/NC
boite_cav = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
TRX = -0.5*LG_ARETE_BLOC - DELTA
TRY = -0.5*LG_ARETE_BLOC - DELTA
TRZ = 0.0
geompy.TranslateDXDYDZ(boite_cav, TRX, TRY, TRZ)
TRX = 0.0
TRY = 0.0
TRZ = LGTUNNEL/NC
l_aux = [TUNNEL_PLEIN]
for iaux in range(NC) :
  TUNNEL = geompy.MakePartition(l_aux, [boite_cav], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  l_aux = [TUNNEL]
  if iaux<NC-1 :
    geompy.TranslateDXDYDZ(boite_cav, TRX, TRY, TRZ)
#
# 5. La structure générale
#
dico_struct = {}
groupe_g = {}
for cas in l_cas :
#
  if verbose :
    print ". Geometrie du cas", cas
#
  DX = LG_ARETE_MASSIF
  if cas == "syme" :
    DX = 0.5*DX
  DY = LG_ARETE_MASSIF
  DZ = LG_ARETE_MASSIF
  MASSIF_00 = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  if cas == "tout" :
    TRX = -0.5*LG_ARETE_MASSIF
  else :
    TRX = 0.0
  TRY = -0.5*LG_ARETE_MASSIF
  TRZ = 0.0
  geompy.TranslateDXDYDZ(MASSIF_00, TRX, TRY, TRZ)
  MASSIF_G = geompy.MakePartition([MASSIF_00], [TUNNEL], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  #print MASSIF_G
#
  dico_struct[cas] = MASSIF_G
#
# 6. Les groupes
#    * Volumes :
#    ===========
#      ROCHE : le massif complet
#      . Pour nn valant 01, 02, 03, ..., 19, 20 :
#        ROCHE_nn : la partie du massif qui reste quand on etudie l'excavation de la couche nn
#        CAV_nn : la partie du massif qui est extraite pour l'excavation de la couche nn
#                 ces morceaux sont disjoints ; on ne s'interesse qu'a ce qui est retire
#                 au moment de l'attaque de la couche n
#      Vu autrement : ROCHE = ROCHE_01 + CAV_01
#                     ROCHE = ROCHE_02 + CAV_01 + CAV_02
#                     ROCHE = ROCHE_03 + CAV_01 + CAV_02 + CAV_03
#                     ... ... ... ... ... ...
#                     ROCHE = ROCHE_20 + CAV_01 + CAV_02 + CAV_03 + ... + CAV_20
#
#      Toutes les mailles de ces groupes seront dupliquees (memes noeuds) et rassemblees
#      dans des groupes miroirs :
#        R_00_b est l'analogue de ROCHE
#        R_nn_b est l'analogue de ROCHE_nn
#        CAV_nn_b est l'analogue de CAV_nn
#
#    * Faces :
#    =========
#      . Les bords exterieurs du domaine
#      ---------------------------------
#        BAS  : la face inferieure du massif
#        HAUT : la face superieure du massif
#        DEVANT   : la face avant du massif
#        DERRIERE : la face arriere du massif
#        GAUCHE : la face gauche du massif
#        Pour la geometrie complete :
#          DROITE : la face droite du massif
#        sinon (pour la geometrie avec symetrie) :
#          SYME_nn : la face a droite bordant la roche au cours de l'excavation de la couche nn,
#                    nn valant 00, 01, 02, 03, ..., 19, 20
#          Remarque : SYME_00 equivaut a DROITE
#
#      . Les bords des cavites
#      -----------------------
#        Pour nn valant 01, 02, 03, ..., 19, 20 :
#        FOND_nn : la paroi qui est le fond de la cavite n, jouxtant la cavite (n+1)
#        BORD_nn : la paroi peripherique de la cavite n
#        PAROI_nn : les parois peripheriques des cavites 1, 2, ..., n-1
#          Vu autrement : PAROI_02 = BORD_01
#                         PAROI_03 = BORD_01 + BORD_02
#                         PAROI_04 = BORD_01 + BORD_02 + BORD_03
#                         ... ... ... ... ... ...
#                         PAROI_20 = BORD_01 + BORD_02 + BORD_03 + ... + BORD_19
#        FRONT_nn : la frontiere complete de la cavite n
#          Vu autrement : FRONT_01 = FOND_01 + BORD_01
#                         FRONT_02 = FOND_02 + BORD_01 + BORD_02
#                         ... ... ... ... ... ...
#                         FRONT_20 = FOND_20 + BORD_01 + BORD_02 + BORD_03 + ... + BORD_20
#          Vu autrement : FRONT_nn = PAROI_nn + FOND_nn + BORD_nn (nn>1)
#
#      . Les bords du tunnel
#      ---------------------
#        TUN_H  : la face superieure du tunnel
#        TUN_G1 : la face gauche et en haut du tunnel
#        TUN_G2 : la face gauche et en bas du tunnel
#        TUN_BASE : la face inferieure du tunnel
#        Pour la geometrie complete :
#          TUN_D1 : la face droite et en haut du tunnel
#          TUN_D2 : la face droite et en bas du tunnel
#        Remarque : les groupes TUN_xx recouvrent les bords des cavites sur toute la longueur de l'excavation
#
  l_cav_toutes_v_id = []
  l_cav_toutes_p_id = []
  l_groupe_cav_g = []
  l_groupe_roche_g = []
  l_groupe_cav_f_g = []
  l_groupe_cav_p_g = []
  l_groupe_cav_e_g = []
  l_face_avant_cav = []
  l_groupe_tunnel_g = []
#
# 6.1. La roche
#
  DX = LG_ARETE_MASSIF + 2.0*DELTA
  DY = LG_ARETE_MASSIF + 2.0*DELTA
  DZ = LG_ARETE_MASSIF + 2.0*DELTA
  boite_aux = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  TRX = -0.5*LG_ARETE_MASSIF - DELTA
  TRY = -0.5*LG_ARETE_MASSIF - DELTA
  TRZ = - DELTA
  geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
  l_solid_id = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["SOLID"], GEOM.ST_IN )
  if ( verbose_max ) :
    print "6.1. l_solid_id =", l_solid_id
  ROCHE_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["SOLID"])
  geompy.UnionIDs ( ROCHE_G, l_solid_id )
#
# 6.2. Les faces externes
# 6.2.1. Paralleles a Oyz
# 6.2.1.1. Parallele a Oyz : face gauche
#
  DX = 2.0*DELTA
  DY = LG_ARETE_MASSIF + 2.0*DELTA
  DZ = LG_ARETE_MASSIF + 2.0*DELTA
  boite_aux = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  TRX =  0.5*LG_ARETE_MASSIF - DELTA
  TRY = -0.5*LG_ARETE_MASSIF - DELTA
  TRZ = - DELTA
  geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
  l_aux  = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
  if ( verbose_max ) :
    print "6.2.1.1. Gauche l_aux =", l_aux
  GAUCHE_G = geompy.CreateGroup ( MASSIF_G, geompy.ShapeType["FACE"] )
  geompy.UnionIDs ( GAUCHE_G, l_aux )
#
# 6.2.1.2. Parallele a Oyz : face droite dans le cas complet, plan de symetrie sinon
#
  l_groupe_cav_s_g = []
  l_face_id_syme = []
  if cas == "tout" :
    TRX = -LG_ARETE_MASSIF
    TRY = 0.0
    TRZ = 0.0
    geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
    l_aux  = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
    if ( verbose_max ) :
      print "6.2.1.2. Droite l_aux =", l_aux
    DROITE_G = geompy.CreateGroup ( MASSIF_G, geompy.ShapeType["FACE"] )
    geompy.UnionIDs ( DROITE_G, l_aux )
#
  else :
#   L'ensemble des faces
    TRX = -0.5*LG_ARETE_MASSIF
    TRY = 0.0
    TRZ = 0.0
    geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
    l_face_id_syme  = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
    if ( verbose_max ) :
      print "6.2.1.2. Symetrie - tout l_face_id_syme =", l_face_id_syme
    l_aux = []
    for face_id in l_face_id_syme :
      l_aux.append(face_id)
    GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
    geompy.UnionIDs ( GR_CAV_G, l_aux )
    l_groupe_cav_s_g.append((GR_CAV_G, "SYME_00"))
#   Les faces bordant le tunnel
    DX = 2.0*DELTA
    DY = LG_ARETE_BLOC + 2.0*DELTA
    DZ = LGTUNNEL/NC + 2.0*DELTA
    boite_2 = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
    TRX = - DELTA
    TRY = -0.5*LG_ARETE_BLOC - DELTA
    TRZ = - DELTA
    geompy.TranslateDXDYDZ(boite_2, TRX, TRY, TRZ)
    TRX = 0.0
    TRY = 0.0
    TRZ = LGTUNNEL/NC
    for iaux in range(NC) :
      if ( verbose_max ) :
        print "6.2.1.2. Cavite %02d" % (iaux+1)
      # Les id des faces bordant la cavite courante
      l_aux_2 = geompy.GetShapesOnBoxIDs (boite_2, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
      if ( verbose_max ) :
        print ".. l_aux_2 =", l_aux_2
      for face_id in l_aux_2 :
        l_aux.remove(face_id)
      GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
      geompy.UnionIDs ( GR_CAV_G, l_aux )
      l_groupe_cav_s_g.append((GR_CAV_G, "SYME_%02d" % (iaux+1)))
# On translate les boites selon Z, d'une distance égale à l'épaisseur d'une cavité
      if iaux<NC-1 :
        geompy.TranslateDXDYDZ(boite_2, TRX, TRY, TRZ)
#
# 6.2.2. Paralleles a Oxz : faces haut & bas
#
  DX = LG_ARETE_MASSIF + 2.0*DELTA
  DY = 2.0*DELTA
  DZ = LG_ARETE_MASSIF + 2.0*DELTA
  boite_aux = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  TRX = -0.5*LG_ARETE_MASSIF - DELTA
  TRY =  0.5*LG_ARETE_MASSIF - DELTA
  TRZ = - DELTA
  geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
  l_aux  = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
  if ( verbose_max ) :
    print "6.2.2. Haut l_aux =", l_aux
  HAUT_G = geompy.CreateGroup ( MASSIF_G, geompy.ShapeType["FACE"] )
  geompy.UnionIDs ( HAUT_G, l_aux )
#
  TRX = 0.0
  TRY = -LG_ARETE_MASSIF
  TRZ = 0.0
  geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
  l_aux  = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
  if ( verbose_max ) :
    print "6.2.2. Bas l_aux =", l_aux
  BAS_G = geompy.CreateGroup ( MASSIF_G, geompy.ShapeType["FACE"] )
  geompy.UnionIDs ( BAS_G, l_aux )
#
# 6.2.3. Paralleles a Oxy
# 6.2.3.1. Faces debouchant du tunnel : toutes les faces contenues dans la boite
#         d'epaisseur quasi-nulle en dZ, de tailles debordant l'empreinte du tunnel en X/Y et centree en (0.,0.,0.)
  DX = LG_ARETE_BLOC + 2.0*DELTA
  DY = LG_ARETE_BLOC + 2.0*DELTA
  DZ = 2.0*DELTA
  boite_aux_1 = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  TRX = -0.5*LG_ARETE_BLOC - DELTA
  TRY = -0.5*LG_ARETE_BLOC - DELTA
  TRZ = - DELTA
  geompy.TranslateDXDYDZ(boite_aux_1, TRX, TRY, TRZ)
  l_face_avant_tunnel  = geompy.GetShapesOnBoxIDs (boite_aux_1, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
  if ( verbose_max ) :
    print "6.2.3.1. Devant l_face_avant_tunnel =", l_face_avant_tunnel
  # Création du groupe de la face avant
  GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
  geompy.UnionIDs ( GR_CAV_G, l_face_avant_tunnel )
  l_groupe_cav_f_g.append((GR_CAV_G, "FOND_00"))
#
# 6.2.3.2. Faces avant du massif : toutes les faces contenues dans la boite
#          d'epaisseur quasi-nulle en dZ, de tailles extremes en X/Y et centree en (0.,0.,0.)
  DX = LG_ARETE_MASSIF + 2.0*DELTA
  DY = LG_ARETE_MASSIF + 2.0*DELTA
  DZ = 2.0*DELTA
  boite_aux = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  TRX = -0.5*LG_ARETE_MASSIF - DELTA
  TRY = -0.5*LG_ARETE_MASSIF - DELTA
  TRZ = - DELTA
  geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
  l_aux_2  = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
  if ( verbose_max ) :
    print "6.2.3.2. Devant l_aux_2  =", l_aux_2
# Pour le groupe, on retire les faces correspondant a l'empreinte du tunnel
  l_aux = []
  for face_id in l_aux_2 :
    if face_id not in l_face_avant_tunnel :
      l_aux.append(face_id)
  if ( verbose_max ) :
    print "6.2.3.2. Devant l_aux  =", l_aux
  DEVANT_G = geompy.CreateGroup ( MASSIF_G, geompy.ShapeType["FACE"] )
  geompy.UnionIDs ( DEVANT_G, l_aux )
#
# 6.2.3.3. Faces avant du massif : toutes les faces contenues dans la boite
#          d'epaisseur quasi-nulle en dZ, de tailles extremes en X/Y et centree en (0.,0.,extremite)
  TRX = 0.0
  TRY = 0.0
  TRZ = LG_ARETE_MASSIF
  geompy.TranslateDXDYDZ(boite_aux, TRX, TRY, TRZ)
  l_aux  = geompy.GetShapesOnBoxIDs (boite_aux, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
  if ( verbose_max ) :
    print "6.2.3.3. Derriere l_aux =", l_aux
  DERRIERE_G = geompy.CreateGroup ( MASSIF_G, geompy.ShapeType["FACE"] )
  geompy.UnionIDs ( DERRIERE_G, l_aux )
#
# 6.3. Les faces et les solides internes au tunnel
  l_cyl_supp = []
#
# 6.3.1. Les faces de la base
#        Le plan support
  DX = 0.0
  DY = LG_ARETE_BLOC
  DZ = 0.0
  normale = geompy.MakeVectorDXDYDZ(DX, DY, DZ)
  DY = -0.5*LG_ARETE_BLOC
  point = geompy.MakeVertex(DX, DY, DZ)
  #        Les faces posees sur ce plan
  liste_face_tunnel_base = geompy.GetShapesOnPlaneWithLocationIDs (MASSIF_G, geompy.ShapeType["FACE"], normale, point, GEOM.ST_ON )
  if ( verbose_max ) :
    print "6.3.1. liste_face_tunnel_base =", liste_face_tunnel_base
#        Création du groupe associe
  GR_TUNNEL_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
  geompy.UnionIDs ( GR_TUNNEL_G, liste_face_tunnel_base )
  l_groupe_tunnel_g.append((GR_TUNNEL_G, "TUN_BASE"))
#
# 6.3.2. Les faces sur les portions cylindriques
#        On distingue chaque cylindre pour les futurs suivis de frontiere dans HOMARD
#        Remarque : il serait plus logique de proceder avec GetShapesOnCylinderWithLocationIDs mais
#                   je n'arrive pas donc je repere les faces dans un tube englobant la surface de chaque cylindre. Bug ? Mauvaise utilisation ?
#                   Du coup, il faut s'assurer que l'on ne capte pas des faces planes precedentes (6.3.1).
#                   Cela peut arriver si les dimensions sont peu serrees
  TRZ = - DELTA
  for cle in d_cyl.keys() :
    t_aux = d_cyl[cle]
  # Creation du tube encadrant le cylindre support
    if ( verbose_max ) :
      print "6.3.2.", cle, ": rayon =", t_aux[0], ", centre (", t_aux[1], ",", t_aux[2], ")"
    cyl_1 = geompy.MakeCylinderRH(1.1*t_aux[0], LG_OUTIL)
    cyl_2 = geompy.MakeCylinderRH(0.9*t_aux[0], LG_OUTIL)
    boite = geompy.MakeCut(cyl_1, cyl_2)
    TRX = t_aux[1]
    TRY = t_aux[2]
    geompy.TranslateDXDYDZ(boite, TRX, TRY, TRZ)
    l_cyl_supp.append((boite, cle))
  # Reperage des faces
    l_aux_1 = geompy.GetShapesOnShapeIDs (boite, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
    if ( verbose_max ) :
      print ".. l_aux_1 =", l_aux_1
    l_aux = []
    for face_id in l_aux_1 :
      if face_id not in liste_face_tunnel_base :
        l_aux.append(face_id)
    if ( verbose_max ) :
      print ".. l_aux =", l_aux
    # Création du groupe associe
    if len(l_aux) > 0 :
      GR_TUNNEL_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
      geompy.UnionIDs ( GR_TUNNEL_G, l_aux )
      l_groupe_tunnel_g.append((GR_TUNNEL_G, cle))
#
# 6.3.3. Creation de boites :
#        . une qui englobe le volume d'une cavite
#
  DX = LG_ARETE_BLOC + 2.0*DELTA
  DY = LG_ARETE_BLOC + 2.0*DELTA
  DZ = LGTUNNEL/NC + 2.0*DELTA
  boite_cav_v = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  TRX = -0.5*LG_ARETE_BLOC - DELTA
  TRY = -0.5*LG_ARETE_BLOC - DELTA
  TRZ = - DELTA
  geompy.TranslateDXDYDZ(boite_cav_v, TRX, TRY, TRZ)
#
#        . une qui englobe une le fond d'une cavite
  DZ = 2.0*DELTA
  boite_cav_f = geompy.MakeBoxDXDYDZ(DX, DY, DZ)
  TRX = -0.5*LG_ARETE_BLOC - DELTA
  TRY = -0.5*LG_ARETE_BLOC - DELTA
  TRZ = LGTUNNEL/NC - DELTA
  geompy.TranslateDXDYDZ(boite_cav_f, TRX, TRY, TRZ)
#
# 6.3.4. On deplace ces boites au fur et a mesure des couches
#        On memorise les faces et volumes contenus dans les boites et
#        on cree les groupes a la volee
#
  TRX = 0.0
  TRY = 0.0
  TRZ = LGTUNNEL/NC
  for face_id in l_face_avant_tunnel :
    l_face_avant_cav.append(face_id)
  for iaux in range(NC) :
    if ( verbose_max ) :
      print "6.3.4. Cavite %02d" % (iaux+1)
    # Création du groupe de l'exterieur des cavites
    if iaux > 0 :
      GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
      geompy.UnionIDs ( GR_CAV_G, l_cav_toutes_p_id )
      l_groupe_cav_e_g.append((GR_CAV_G, "PAROI_%02d" % (iaux+1)))
    # Les id des solides dans la cavite courante
    l_aux_1 = geompy.GetShapesOnBoxIDs (boite_cav_v, MASSIF_G, geompy.ShapeType["SOLID"], GEOM.ST_IN )
    if ( verbose_max ) :
      print ".. l_aux_1 =", l_aux_1
    # Création du groupe solide de la cavite courante
    CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["SOLID"])
    geompy.UnionIDs ( CAV_G, l_aux_1 )
    l_groupe_cav_g.append((CAV_G, "CAV_%02d" % (iaux+1)))
    # On ajoute ces id a la liste de tous ceux depuis le debut des cavites (i.e. le percement du tunnel)
    for solid_id in l_aux_1 :
      l_cav_toutes_v_id.append(solid_id)
    if ( verbose_max ) :
      print ".. l_cav_toutes_v_id =", l_cav_toutes_v_id
    # On repere les id des solides du massif mais qui ne sont pas dans les cavites precedemment traitées
    l_solid_id_1 = []
    for solid_id in l_solid_id :
      if solid_id not in l_cav_toutes_v_id :
        l_solid_id_1.append(solid_id)
    if ( verbose_max ) :
      print ".. l_solid_id_1 =", l_solid_id_1
    # Création du groupe solide de la roche de laquelle on a retiré tous les creusements effectués
    GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["SOLID"])
    geompy.UnionIDs ( GR_CAV_G, l_solid_id_1 )
    l_groupe_roche_g.append((GR_CAV_G, "ROCHE_%02d" % (iaux+1)))
    # Les id des faces du fond de la cavite courante
    l_aux_1 = geompy.GetShapesOnBoxIDs (boite_cav_f, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
    if ( verbose_max ) :
      print ".. l_aux_1 =", l_aux_1
    # Création du groupe du fond de la cavite
    GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
    geompy.UnionIDs ( GR_CAV_G, l_aux_1 )
    l_groupe_cav_f_g.append((GR_CAV_G, "FOND_%02d" % (iaux+1)))
    # Les id des faces dans la cavite courante
    l_aux_2 = geompy.GetShapesOnBoxIDs (boite_cav_v, MASSIF_G, geompy.ShapeType["FACE"], GEOM.ST_IN )
    if ( verbose_max ) :
      print ".. l_aux_2 =", l_aux_2
    # Création du groupe du pourtour de la cavite : penser a retirer les fonds et l'eventuel plan de symetrie !
    # On cumule tous les pourtours depuis le debut
    l_aux = []
    for face_id in l_aux_2 :
      if face_id not in l_aux_1 + l_face_avant_cav + l_face_id_syme :
        l_aux.append(face_id)
        l_cav_toutes_p_id.append(face_id)
    if ( verbose_max ) :
      print ".. l_aux =", l_aux
    GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
    geompy.UnionIDs ( GR_CAV_G, l_aux )
    l_groupe_cav_p_g.append((GR_CAV_G, "BORD_%02d" % (iaux+1)))
    # Création du groupe de l'exterieur des cavites
    GR_CAV_G = geompy.CreateGroup(MASSIF_G, geompy.ShapeType["FACE"])
    geompy.UnionIDs ( GR_CAV_G, l_cav_toutes_p_id+l_aux_1 )
    l_groupe_cav_e_g.append((GR_CAV_G, "FRONT_%02d" % (iaux+1)))
    # On translate les boites selon Z, d'une distance égale à l'épaisseur d'une cavité
    if iaux<NC-1 :
      geompy.TranslateDXDYDZ(boite_cav_v, TRX, TRY, TRZ)
      geompy.TranslateDXDYDZ(boite_cav_f, TRX, TRY, TRZ)
      l_face_avant_cav = []
      for face_id in l_aux_1 :
        l_face_avant_cav.append(face_id)
#
# 6.4. Archivage
#
  groupe_g[("ROCHE_G", cas)] = ROCHE_G
  groupe_g[("GAUCHE_G", cas)] = GAUCHE_G
  if cas == "tout" :
    groupe_g[("DROITE_G", cas)] = DROITE_G
  groupe_g[("HAUT_G", cas)] = HAUT_G
  groupe_g[("BAS_G", cas)] = BAS_G
  groupe_g[("DEVANT_G", cas)] = DEVANT_G
  groupe_g[("DERRIERE_G", cas)] = DERRIERE_G
  groupe_g[("cav", cas)] = l_groupe_cav_g
  groupe_g[("roche", cas)] = l_groupe_roche_g
  groupe_g[("tunnel", cas)] = l_groupe_tunnel_g
  groupe_g[("cav_f", cas)] = l_groupe_cav_f_g
  groupe_g[("cav_p", cas)] = l_groupe_cav_p_g
  groupe_g[("cav_e", cas)] = l_groupe_cav_e_g
  groupe_g[("cav_s", cas)] = l_groupe_cav_s_g
#
# 7. Affichage
# 7.1. Les objets de construction
#
geompy.addToStudy( OUTIL_H, 'OUTIL_H' )
geompy.addToStudy( OUTIL_G_1, 'OUTIL_G_1' )
geompy.addToStudy( OUTIL_D_1, 'OUTIL_D_1' )
geompy.addToStudy( OUTIL_G_2, 'OUTIL_G_2' )
geompy.addToStudy( OUTIL_D_2, 'OUTIL_D_2' )
geompy.addToStudy( B_B, 'B_B' )
geompy.addToStudy( B_H, 'B_H' )
geompy.addToStudy( B_G, 'B_G' )
geompy.addToStudy( B_D, 'B_D' )
geompy.addToStudy( CG1, 'CG1' )
geompy.addToStudy( VG, 'VG' )
geompy.addToStudy( PG, 'PG' )
geompy.addToStudy( CD1, 'CD1' )
geompy.addToStudy( VD, 'VD' )
geompy.addToStudy( PD, 'PD' )
geompy.addToStudy( B_H1, 'B_H1' )
geompy.addToStudy( B_G1, 'B_G1' )
geompy.addToStudy( boite_aux_G, 'boite_aux_G' )
geompy.addToStudy( B_G2, 'B_G2' )
geompy.addToStudy( B_G3, 'B_G3' )
geompy.addToStudy( B_D1, 'B_D1' )
geompy.addToStudy( boite_aux_D, 'boite_aux_D' )
geompy.addToStudy( B_D2, 'B_D2' )
geompy.addToStudy( B_D3, 'B_D3' )
geompy.addToStudy( EMPREINTE, 'EMPREINTE' )
geompy.addToStudy( BOITE_TUNNEL, 'BOITE_TUNNEL' )
geompy.addToStudy( TUNNEL_PLEIN, 'TUNNEL_PLEIN' )
geompy.addToStudy( boite_cav, 'boite_cav' )
geompy.addToStudy( TUNNEL, 'TUNNEL' )
geompy.addToStudy( MASSIF_00, 'MASSIF_00' )
geompy.addToStudy( boite_aux, 'boite_aux' )
geompy.addToStudy( boite_aux_1, 'boite_aux_1' )
geompy.addToStudy( normale, 'normale' )
geompy.addToStudy( point, 'point' )
geompy.addToStudy( boite_cav_v, 'boite_cav_v' )
geompy.addToStudy( boite_cav_f, 'boite_cav_f' )
for objet in l_cyl_supp :
  geompy.addToStudy( objet[0], objet[1] )
#
# 7.2. La structure a etudier et ses groupes
#
for cas in l_cas :
#
  MASSIF_G = dico_struct[cas]
  #print MASSIF_G
  geompy.addToStudy( MASSIF_G, "MASSIF"+cas )
  geompy.addToStudyInFather( MASSIF_G, groupe_g[("ROCHE_G", cas)], "ROCHE" )
  l_groupe_cav_g = groupe_g[("cav", cas)]
  for groupe in l_groupe_cav_g :
    geompy.addToStudyInFather( MASSIF_G, groupe[0], groupe[1] )
  l_groupe_roche_g = groupe_g[("roche", cas)]
  for groupe in l_groupe_roche_g :
    geompy.addToStudyInFather( MASSIF_G, groupe[0], groupe[1] )
  geompy.addToStudyInFather( MASSIF_G, groupe_g[("GAUCHE_G", cas)], "GAUCHE" )
  if cas == "tout" :
    geompy.addToStudyInFather( MASSIF_G, groupe_g[("DROITE_G", cas)], "DROITE" )
  geompy.addToStudyInFather( MASSIF_G, groupe_g[("HAUT_G", cas)], "HAUT" )
  geompy.addToStudyInFather( MASSIF_G, groupe_g[("BAS_G", cas)], "BAS" )
  geompy.addToStudyInFather( MASSIF_G, groupe_g[("DEVANT_G", cas)], "DEVANT" )
  geompy.addToStudyInFather( MASSIF_G, groupe_g[("DERRIERE_G", cas)], "DERRIERE" )
  l_groupe_tunnel_g = groupe_g[("tunnel", cas)]
  for groupe in l_groupe_tunnel_g :
    geompy.addToStudyInFather( MASSIF_G, groupe[0], groupe[1] )
  l_groupe_cav_f_g = groupe_g[("cav_f", cas)]
  l_groupe_cav_p_g = groupe_g[("cav_p", cas)]
  l_groupe_cav_e_g = groupe_g[("cav_e", cas)]
  l_groupe_cav_s_g = groupe_g[("cav_s", cas)]
  for groupe in l_groupe_cav_f_g + l_groupe_cav_p_g + l_groupe_cav_e_g + l_groupe_cav_s_g :
    geompy.addToStudyInFather( MASSIF_G, groupe[0], groupe[1] )
#

###=======================================================================
### SMESH component
###=======================================================================

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(theStudy)

from salome.BLSURFPlugin import BLSURFPluginBuilder
from salome.GHS3DPlugin import GHS3DPluginBuilder
#
for cas in l_cas :
#
  if verbose :
    print ". Maillage du cas", cas
#
  MASSIF_G = dico_struct[cas]
#
# 1. Maillage
#
  MASSIF_M = smesh.Mesh(MASSIF_G)
#
# 2. Parametres du maillage volumique
#
  if cas == "tout" :
    GHS3D_3D = MASSIF_M.Tetrahedron(algo=smeshBuilder.GHS3D)
    GHS3D_Parameters = smesh.CreateHypothesis('GHS3D_Parameters', 'GHS3DEngine')
    # Niveau d'optimisation : 3 ==> standard +
    GHS3D_Parameters.SetOptimizationLevel( 3 )
  else :
    isdone = MASSIF_M.AddHypothesis(GHS3D_Parameters)
    GHS3D_3D_1 = MASSIF_M.Tetrahedron(algo=smeshBuilder.GHS3D)
#
# 3. Parametres du maillage surfacique
#
  if cas == "tout" :
    #
    BLSURF = MASSIF_M.Triangle(algo=smeshBuilder.BLSURF)
    BLSURF_Parameters = BLSURF.Parameters()
    # Geometrical mesh - if set to "Custom", allows user input in Angle Mesh S, Angle Mesh C and Gradation fields.
    # These fields control computation of the element size, so called geometrical size, conform to the
    # surface geometry considering local curvatures.
    # If both the User size and the geometrical size are defined, the eventual element size correspond to the least of the two.
    BLSURF_Parameters.SetGeometricMesh( 1 )
    # Gradation - maximum ratio between the lengths of two adjacent edges.
    BLSURF_Parameters.SetGradation( 2.5 )
    # Angle Mesh S - maximum angle between the mesh face and the tangent to the geometrical surface at each mesh node, in degrees.
    BLSURF_Parameters.SetAngleMeshS( 16. )
    # Angle Mesh C - maximum angle between the mesh edge and the tangent to the geometrical curve at each mesh node, in degrees.
    BLSURF_Parameters.SetAngleMeshC( 16. )
    # Taille de maille globale
    BLSURF_Parameters.SetPhySize( 25. )
  else :
    isdone = MASSIF_M.AddHypothesis(BLSURF_Parameters)
    BLSURF_1 = MASSIF_M.Triangle(algo=smeshBuilder.BLSURF)
#
# 4. Calcul
#
  isDone = MASSIF_M.Compute()
#
  MASSIF_M.ConvertToQuadratic( 1 )
#
# 5. Groupes
# 5.1. Groupes issus de la géométrie : volume et limites externes
#
  ROCHE_M = MASSIF_M.Group(groupe_g[("ROCHE_G", cas)])
#
  GAUCHE_M = MASSIF_M.Group(groupe_g[("GAUCHE_G", cas)])
  if cas == "tout" :
    DROITE_M = MASSIF_M.Group(groupe_g[("DROITE_G", cas)])
  HAUT_M = MASSIF_M.Group(groupe_g[("HAUT_G", cas)])
  BAS_M = MASSIF_M.Group(groupe_g[("BAS_G", cas)])
  DEVANT_M = MASSIF_M.Group(groupe_g[("DEVANT_G", cas)])
  DERRIERE_M = MASSIF_M.Group(groupe_g[("DERRIERE_G", cas)])
#
# 5.2. Groupes issus de la géométrie : gestion des cavités
#      On mémorise dans l_groupe_b les groupes qui correspondent à des mailles
#      qu'il faudra dédoubler pour le calcul de second gradient : la roche finale et les cavités
#
  l_groupe_roche_g = groupe_g[("roche", cas)]
  l_groupe_cav_g = groupe_g[("cav", cas)]
  l_groupe_cav_f_g = groupe_g[("cav_f", cas)]
  l_groupe_cav_p_g = groupe_g[("cav_p", cas)]
  l_groupe_cav_e_g = groupe_g[("cav_e", cas)]
  l_groupe_cav_s_g = groupe_g[("cav_s", cas)]
  l_groupe_tunnel_g = groupe_g[("tunnel", cas)]
  l_groupe_m = []
  l_groupe_b = []
  for groupe in l_groupe_roche_g + l_groupe_cav_g + l_groupe_cav_f_g + l_groupe_cav_p_g + l_groupe_cav_e_g + l_groupe_cav_s_g + l_groupe_tunnel_g :
    GR_M = MASSIF_M.Group(groupe[0])
    l_groupe_m.append((GR_M, groupe[1]))
    if groupe in l_groupe_cav_g :
      l_groupe_b.append(GR_M)
    elif groupe[1] == "ROCHE_20" :
      l_groupe_b.append(GR_M)
#
# 6. Duplication des mailles dans la roche finale et les cavités
#
  d_aux = {}
  for groupe in l_groupe_b :
#
#   6.1. Nom du groupe et nombre de mailles qu'il contient
#
    name = groupe.GetName()
    size_groupe = groupe.Size()
    if verbose_max :
      print "Longueur du groupe",name, ":", size_groupe
#
#   6.2. Pour chaque maille du groupe, on repere la liste des noeuds
#        et on cree une maille avec ces memes noeuds
#
    l_aux = []
    for jaux in range (size_groupe) :
      id_elem = groupe.GetID(jaux+1)
      #if name == "CAV_01" :
        #print ".. element",jaux,":", id_elem
      l_nodes = MASSIF_M.GetElemNodes ( id_elem )
      ##print ".. l_nodes :", l_nodes
      id_elem_new = MASSIF_M.AddVolume(l_nodes)
      ##print ".. nouvel element :", id_elem_new
      l_aux.append(id_elem_new)
#
#   6.3. Creation d'un groupe contenant ces mailles doubles creees
#
    name = name + "_b"
    if name[0:1] == "R" :
      name = "R_20_b"
    #print "name :", name
    ElementType = groupe.GetType();
    groupe_new = MASSIF_M.MakeGroupByIds( name, ElementType, l_aux )
    smesh.SetName(groupe_new, name)
    d_aux[name] = groupe_new
    if verbose_max :
      size_groupe_new = groupe_new.Size()
      print "Longueur du groupe",name, ":", size_groupe
#
# 6.4. Creation des groupes contenant les mailles doubles creees pour chacune
#      des situations d'excavation :
#      Couche 20 : R_20_b
#      Couche 19 : R_19_b = R_20_b + CAV_20_b
#      Couche 18 : R_18_b = R_20_b + CAV_20_b + CAV_19_b
#      ... etc ...
#      Couche 01 : R_01_b = R_20_b + CAV_20_b + CAV_19_b + ... + CAV_02_b
#      Roche     : R_00_b = R_20_b + CAV_20_b + CAV_19_b + ... + CAV_02_b + CAV_01_b
#
  groupe_d = d_aux["R_20_b"]
  for nro_cav in range(NC-1, -1, -1 ) :
    name = "R_%02d_b" % nro_cav
    groupe_new = MASSIF_M.UnionGroups ( groupe_d, d_aux["CAV_%02d_b" % (nro_cav+1)], name)
    smesh.SetName(groupe_new, name)
    groupe_d = groupe_new
#
# 7. Ecriture
#
  fichierMedResult = '/tmp/MASSIF.new.med' 
  MASSIF_M.ExportMED( fichierMedResult, 0, SMESH.MED_V2_2, 1 )
#
#___________________________________________________________
# Ajout PN : statistiques sur les Mailles
#___________________________________________________________
fichierStatResult=fichierMedResult.replace('.med','.res')
from Stats.getStats import getStatsMaillage, getStatsGroupes
getStatsMaillage(MASSIF_M,fichierStatResult)
fichierGroupes=fichierMedResult.replace('.med','_groups.res')
getStatsGroupes(MASSIF_M,fichierGroupes)

