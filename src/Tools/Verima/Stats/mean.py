#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import numpy, scipy
import scipy.stats


def getMean(fichierStatMaillage):
  """
  """
  try :
     monTableau=numpy.loadtxt(fichierStatMaillage)
  except :
     print "impossible de charger le fichier : ", fichierStatMaillage

  mesIntervalles=monTableau[ :, 0:2 ]
  mesPoids=monTableau[ : ,2]
  moyIntervalles=numpy.average(mesIntervalles, axis=1)
  moyenne=numpy.average(moyIntervalles,weights=mesPoids)
  freqCum=numpy.cumsum(mesPoids)
  nbVal=freqCum[-1 ]
  mesFrequences=mesPoids/nbVal
  mesValeurs=scipy.stats.rv_discrete(values=(moyIntervalles,mesFrequences))
  Q1=mesValeurs.ppf(0.25)
  M=mesValeurs.median()
  Q3=mesValeurs.ppf(0.75)

  i=mesPoids.shape[0]  -1
  while i > 0:
    if mesPoids[i] > 0: 
       max=mesIntervalles[i][1]
       break
    i=i-1
  i=0
  while i < mesPoids.shape[0]  -1:
    if mesPoids[i] > 0: 
       min=mesIntervalles[i][0]
       break
    i=i+1
  i=0
  
  return [max,min,Q1,M,Q3,moyenne]


if __name__ == "__main__":
      from optparse import OptionParser
      p=OptionParser()
      p.add_option('-f',dest='fichier',default='tetra.taille',help='fichier a traiter')
      #p.add_option('-f',dest='fichier',default='Mesh_1_aspect_ratio_3d.txt',help='fichier a traiter')
      options, args = p.parse_args()

      getMean(options.fichier)

