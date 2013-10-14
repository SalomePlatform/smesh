#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-

import string

FichierEntete='templatesHtml/entete.html'
FichierMailleur='templatesHtml/mailleur.html'
FichierJob='templatesHtml/job.html'
FichierTableau='templatesHtml/tableau.html'
FichierLigne='templatesHtml/ligne.html'
FichierFinTableau='templatesHtml/tableauFin.html'

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
    

if __name__=='__main__':
     dicoEntete={}
     dicoEntete["VERSIONTEST"]="V7.2"
     dicoEntete["VERSIONREF"]="V6.6"
     dicoEntete["MACHINE"]="dsp111002"
     monTexte=CreeEntete(dicoEntete)

     dicoMailleur={}
     dicoMailleur["MAILLEUR"]="GHS3D"
     texteMailleur=CreeMailleur(dicoMailleur)
     monTexte+=texteMailleur
     

     dicoJob={}
     dicoJob["NOM"]="MASSIF"
     dicoJob["NBCPU"]="10"
     dicoJob["REFCPU"]="11"
     dicoJob["DIFCPU"]="1"
     dicoJob["DIFREL"]="10%"
     dicoJob["WARNING"]="WARNING"
     dicoJob["RMAX"]=6.1
     dicoJob["RMAXREF"]=5.95
     dicoJob["WRMAX"]="WARNING"
     dicoJob["RMOY"]=3.8	
     dicoJob["RMOYREF"]=3.88
     dicoJob["WRMOY"]=""
     dicoJob["R1Q"]=2.84
     dicoJob["R1QREF"]=2.84
     dicoJob["RMED"]=3.6
     dicoJob["RMEDREF"]=3.5
     dicoJob["R2Q"]=4.91
     dicoJob["R2QREF"]=4.92
     dicoJob["TMAX"]=34.5
     dicoJob["TMAXREF"]=35.5
     dicoJob["WTMAX"]=""
     dicoJob["TMOY"]=17.4
     dicoJob["TMOYREF"]=17.5
     dicoJob["WTMOY"]=""
     dicoJob["T1Q"]=8.9
     dicoJob["T1QREF"]=8.9
     dicoJob["TMED"]=17.3
     dicoJob["TMEDREF"]=17.8
     dicoJob["T2Q"]=26.4
     dicoJob["T2QREF"]=26.5
     texteMailleur=CreeJob(dicoJob)
     monTexte+=texteMailleur

     dico={}
     dico["TITRE"]="Nombre de Mailles dans le maillage entier"
     dico["MAIL1"]="Quad_Edge"
     dico["REF1"]=421
     dico["NB1"]=422
     dico["DIF1"]=1
     dico["REL1"]=0.2
     dico["WARN1"]=""
     dico["MAIL2"]="Quad_Triangle"
     dico["REF2"]=1068
     dico["NB2"]=1078
     dico["DIF2"]=8
     dico["REL2"]=0.9
     dico["WARN2"]=""
     dico["MAIL3"]="Quad_Pyramid"
     dico["NB3"]=5520
     dico["REF3"]=5484
     dico["DIF3"]=36
     dico["REL3"]="0.6"
     dico["WARN3"]="WARNING"
     texteMaille=CreeMailleOuGroupe(dico,3)
     monTexte+=texteMaille

     fichierEssai="letter.html"
     EcritFichier(monTexte,fichierEssai)
     exit()

     dico={}
     dico["TITRE"]="Nombre de Mailles dans le groupe FOND_07"
     dico["MAIL1"]="Quad_Triangle"
     dico["REF1"]=12
     dico["NB1"]=12
     dico["DIF1"]=0
     dico["REL1"]=0
     dico["WARN1"]=""
     texteMaille=CreeMailleOuGroupe(dico,1)
     monTexte+=texteMaille

     dico={}
     dico["TITRE"]="Nombre de Mailles dans le groupe PAROI_07"
     dico["MAIL1"]="Quad_Triangle"
     dico["REF1"]=132
     dico["NB1"]=132
     dico["DIF1"]=0
     dico["REL1"]=0
     dico["WARN1"]=""
     texteMaille=CreeMailleOuGroupe(dico,1)
     monTexte+=texteMaille

     dico={}
     dico["TITRE"]="Nombre de Mailles dans le groupe ROCHE_07"
     dico["MAIL1"]="Pyramid"
     dico["REF1"]=2326
     dico["NB1"]=2330
     dico["DIF1"]=4
     dico["REL1"]=0
     dico["WARN1"]=""
     texteMaille=CreeMailleOuGroupe(dico,1)
     monTexte+=texteMaille

     dicoJob={}

     dicoJob={}
     dicoJob["NOM"]="CUBE"
     dicoJob["NBCPU"]="1"
     dicoJob["REFCPU"]="21"
     dicoJob["DIFCPU"]="1"
     dicoJob["DIFREL"]="100%"
     dicoJob["WARNING"]="WARNING"
     dicoJob["RMAX"]=7
     dicoJob["RMAXREF"]=6
     dicoJob["WRMAX"]="WARNING"
     dicoJob["RMOY"]=2.7	
     dicoJob["RMOYREF"]=1.8
     dicoJob["WRMOY"]="WARNING"
     dicoJob["R1Q"]=1.3
     dicoJob["R1QREF"]=1.4
     dicoJob["RMED"]=2.6
     dicoJob["RMEDREF"]=2.6
     dicoJob["R2Q"]=3.9
     dicoJob["R2QREF"]=3.8
     dicoJob["TMAX"]=4
     dicoJob["TMAXREF"]=3.5
     dicoJob["WTMAX"]="WARNING"
     dicoJob["TMOY"]=1.8
     dicoJob["TMOYREF"]=1.5
     dicoJob["WTMOY"]="WARNING"
     dicoJob["T1Q"]=0.7
     dicoJob["T1QREF"]=0.7
     dicoJob["TMED"]=1.9
     dicoJob["TMEDREF"]=2
     dicoJob["T2Q"]=3.1
     dicoJob["T2QREF"]=3.2
     texteMailleur=CreeJob(dicoJob)
     monTexte+=texteMailleur

     dico={}
     dico["TITRE"]="Nombre de Mailles dans le maillage entier"
     dico["MAIL1"]="Edge"
     dico["REF1"]=12
     dico["NB1"]=13
     dico["DIF1"]=1
     dico["REL1"]="8.3%"
     dico["WARN1"]="WARNING"
     dico["MAIL2"]="Triangle"
     dico["REF2"]=24
     dico["NB2"]=24
     dico["DIF2"]=0
     dico["REL2"]="0%"
     dico["WARN2"]=""
     dico["MAIL3"]="Pyramid"
     dico["NB3"]=4
     dico["REF3"]=5
     dico["DIF3"]=1
     dico["REL3"]="25%"
     dico["WARN3"]="WARNING"
     texteMaille=CreeMailleOuGroupe(dico,3)
     monTexte+=texteMaille

     dico={}
     dico["TITRE"]="Nombre de Mailles dans le groupe SURFACE"
     dico["MAIL1"]="Edge"
     dico["REF1"]=12
     dico["NB1"]=13
     dico["DIF1"]=1
     dico["REL1"]="8.3%"
     dico["WARN1"]="WARNING"
     dico["MAIL2"]="Triangle"
     dico["REF2"]=24
     dico["NB2"]=24
     dico["DIF2"]=0
     dico["REL2"]="0%"
     dico["WARN2"]=""
     texteMaille=CreeMailleOuGroupe(dico,2)
     monTexte+=texteMaille

     fichierEssai="/tmp/letter.html"
     EcritFichier(monTexte,fichierEssai)

