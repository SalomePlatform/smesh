#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-

import string
import os

dir=os.path.dirname(os.path.abspath(__file__))
FichierEntete=os.path.join(dir,'templatesHtml/entete.html')
FichierMailleur=os.path.join(dir,'templatesHtml/mailleur.html')
FichierJob=os.path.join(dir,'templatesHtml/job.html')
#FichierJob=os.path.join(dir,'templatesHtml/jobIncomplet.html')
FichierTableau=os.path.join(dir,'templatesHtml/tableau.html')
FichierLigne=os.path.join(dir,'templatesHtml/ligne.html')
FichierFinTableau=os.path.join(dir,'templatesHtml/tableauFin.html')
FichierSansGroupe=os.path.join(dir,'templatesHtml/sansGroupe.html')


def compte_all(texte, subString):
    start = 0
    compte=0
    while True:
        trouve = texte.find(subString, start)
        if trouve == -1: return compte
        compte+=1
        start = trouve + len(subString)

def FormateTexte(texte,dico):
    for clef in dico.keys():
        texteARemplacer="%"+str(clef)+"%"
        remplacement=dico[clef]
        if texte.find(texteARemplacer) < 0 :
           print "impossible de remplacer ",texteARemplacer, "Pas d'occurence"
           print remplacement
           continue
        if compte_all(texte,texteARemplacer) != 1 :
           print "impossible de remplacer ",texteARemplacer, "trop d'occurences"
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
    texteIni=open(FichierEntete).read()
    texteRetour=FormateTexte(texteIni,dico)
    return texteRetour

def CreeMailleur(dico):
    texteIni=open(FichierMailleur).read()
    texteRetour=FormateTexte(texteIni,dico)
    return texteRetour

def CreeJob(dico):
    texte=open(FichierJob).read()
    texteRetour=FormateTexte(texte,dico)
    return texteRetour

def CreeMailleOuGroupe(dico,nb):
    texteIni=open(FichierTableau).read()

    texteLigneIni=open(FichierLigne).read()
    for i in range (2,nb+1) :
        texteLigne=FormateLigne(texteLigneIni,i)
        texteIni=texteIni+texteLigne

    texteRetour=FormateTexte(texteIni,dico)

    finTexte=open(FichierFinTableau).read()
    texteFinal=texteRetour+finTexte

    return texteFinal


def EcritFichier(texte,filename):
    fp=open(filename,'w')
    fp.write(texte)
    fp.close()
    

class Document:

    def __init__(self):
      self.monTexte=""
      self.dicoEntete={}

    def creeDocument(self,monFichier):
      EcritFichier(self.monTexte,monFichier)

    def initEntete(self,version, versionRef,machine):
      self.dicoEntete["VERSIONTEST"]=version
      self.dicoEntete["VERSIONREF"]=versionRef
      self.dicoEntete["MACHINE"]=machine
      self.monTexte=CreeEntete(self.dicoEntete)

    def initMailleur(self,mailleurName):
      dicoMailleur={}
      dicoMailleur["MAILLEUR"]=mailleurName
      texteMailleur=CreeMailleur(dicoMailleur)
      self.monTexte+=texteMailleur

    def initJob(self,dicoJob):
      texteMailleur=CreeJob(dicoJob)
      self.monTexte+=texteMailleur

    def initMaille(self,dicoMaille,nbEntity):
      texteMaille=CreeMailleOuGroupe(dicoMaille,nbEntity)
      self.monTexte+=texteMaille

    def addNoGroup(self):
      texteNoGroupe=open(FichierSansGroupe).read()
      self.monTexte+=texteNoGroupe


#
