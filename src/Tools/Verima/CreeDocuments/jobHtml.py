#!/usr/bin/env python3
# Copyright (C) 2013-2021  EDF R&D
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

import string
import os

adir=os.path.dirname(os.path.abspath(__file__))
FichierEntete=os.path.join(adir,'templatesHtml/entete.html')
FichierMaillageEntete=os.path.join(adir,'templatesHtml/maillageEntete.html')
FichierJob=os.path.join(adir,'templatesHtml/job.html')
FichierTableau=os.path.join(adir,'templatesHtml/tableau.html')
FichierLigne=os.path.join(adir,'templatesHtml/ligne.html')
FichierFinTableau=os.path.join(adir,'templatesHtml/tableauFin.html')
FichierSansGroupe=os.path.join(adir,'templatesHtml/sansGroupe.html')
FichierGroupeRatio=os.path.join(adir,'templatesHtml/groupeRatio.html')
FichierGroupeTaille=os.path.join(adir,'templatesHtml/groupeTaille.html')


def compte_all(texte, subString):
    start = 0
    compte=0
    while True:
        trouve = texte.find(subString, start)
        if trouve == -1: return compte
        compte+=1
        start = trouve + len(subString)

def FormateTexte(texte,dico):
    for clef in dico:
        texteARemplacer="%"+str(clef)+"%"
        remplacement=dico[clef]
        if texte.find(texteARemplacer) < 0 :
           print("impossible de remplacer ",texteARemplacer, "Pas d'occurence")
           print(remplacement)
           continue
        if compte_all(texte,texteARemplacer) != 1 :
           print("impossible de remplacer ",texteARemplacer, "trop d'occurences")
           continue
        remplacement=str(remplacement)
        texte=texte.replace(texteARemplacer,remplacement)
    return texte

def FormateLigne(texte,nb):
    texteARemplacer="2%"
    remplacement=str(nb)+"%"
    texte=texte.replace(texteARemplacer,remplacement)
    return texte

def CreeEntete(dico):
    texteIni=open(FichierEntete, 'r', encoding='utf8').read()
    texteRetour=FormateTexte(texteIni,dico)
    return texteRetour

def CreeMaillage(dico):
    texteIni=open(FichierMaillageEntete, 'r', encoding='utf8').read()
    texteRetour=FormateTexte(texteIni,dico)
    return texteRetour

def CreeJob(dico):
    texte=open(FichierJob, 'r', encoding='utf8').read()
    texteRetour=FormateTexte(texte,dico)
    return texteRetour

def CreeGroupeTaille(dico):
    texte=open(FichierGroupeTaille, 'r', encoding='utf8').read()
    texteRetour=FormateTexte(texte,dico)
    return texteRetour

def CreeGroupeRatio(dico):
    texte=open(FichierGroupeRatio, 'r', encoding='utf8').read()
    texteRetour=FormateTexte(texte,dico)
    return texteRetour


def CreeMailleOuGroupe(dico,nb):
    texteIni=open(FichierTableau, 'r', encoding='utf8').read()
    texteLigneIni=open(FichierLigne).read()
    for i in range (2,nb+1) :
        texteLigne=FormateLigne(texteLigneIni,i)
        texteIni=texteIni+texteLigne

    texteRetour=FormateTexte(texteIni,dico)

    finTexte=open(FichierFinTableau, 'r', encoding='utf8').read()
    texteFinal=texteRetour+finTexte

    return texteFinal


def EcritFichier(texte,filename):
    with open(filename,'w',encoding='utf8') as fp:
        fp.write(texte)
    

class Document:

    def __init__(self):
      self.monTexte=""
      self.dicoEntete={}

    def creeDocument(self,monFichier):
      EcritFichier(self.monTexte,monFichier)

    def initEntete(self,version, machine):
      self.dicoEntete["VERSIONTEST"]=version
      self.dicoEntete["MACHINE"]=machine
      self.monTexte=CreeEntete(self.dicoEntete)

    def initMaillage(self,maillageName,mailleurName,versionRefName,dicoMaillage):
      dicoMaillage["MAILLEUR"]=mailleurName
      dicoMaillage["MAILLAGE"]=maillageName
      dicoMaillage["VERSIONREF"]=versionRefName
      texteMaillageEntete=CreeMaillage(dicoMaillage)
      self.monTexte+=texteMaillageEntete

    def initJob(self,dicoJob):
      texteMailleur=CreeJob(dicoJob)
      self.monTexte+=texteMailleur

    def initMaille(self,dicoMaille,nbEntity):
      texteMaille=CreeMailleOuGroupe(dicoMaille,nbEntity)
      self.monTexte+=texteMaille

    def addNoGroup(self):
      texteNoGroupe=open(FichierSansGroupe).read()
      self.monTexte+=texteNoGroupe

    def CreeGroupeTaille(self,dico):
      texteGroupeTaille=CreeGroupeTaille(dico)
      self.monTexte+=texteGroupeTaille

    def CreeGroupeRatios(self,dico):
      texteGroupeRatio=CreeGroupeRatio(dico)
      self.monTexte+=texteGroupeRatio

#
