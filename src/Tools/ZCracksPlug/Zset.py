# Copyright (C) 2016-2021  EDF R&D
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

import os, tempfile, shutil
from . import utilityFunctions as uF
from .output import message

def medToGeo(medFile, geoFile, tmpdir, opt=[], verbose=0):
  medLoc=os.path.dirname(medFile)
  medName=os.path.basename(medFile)
  inpFile=os.path.join(tmpdir,'import.inp')
  zfile = open(inpFile,'w')
  zfile.write('****mesher\n')
  zfile.write(' ***mesh %s\n' %(geoFile.replace('.geo','')+'.geo'))
  zfile.write('  **import med %s\n' %medName)
  for x in opt:
    zfile.write(x+'\n')
  zfile.write('****return\n')
  zfile.close()
  commande='cd %s; Zrun -m %s' %(medLoc,inpFile)
  if verbose!=0: commande+=' >> %s' %os.path.join(tmpdir,'log.msg')
  res=os.system(commande)

  return(res)

  #os.remove(inpFile)


def geoToMed(medFile, geoFile, tmpdir, opt=[], verbose=0):
  medLoc=os.path.dirname(medFile)
  medName=os.path.basename(medFile)
  inpFile=os.path.join(tmpdir,'export.inp')
  zfile = open(inpFile,'w')
  zfile.write('****mesher\n')
  zfile.write(' ***mesh\n')
  zfile.write('  **open %s\n' %geoFile)
  for x in opt:
    zfile.write(x+'\n')
  zfile.write('  **export med %s\n' %medName)
  zfile.write('****return\n')
  zfile.close()
  commande='cd %s; Zrun -m %s' %(medLoc,inpFile)
  if verbose!=0: commande+=' >> %s' %os.path.join(tmpdir,'log.msg')
  res=os.system(commande)
  #print ' -------- '
  #print res
  #print ' -------- '
  return(res)


def launchZcrack(minS, maxS,
                saneN, crackN, crackedN,
                grad, quad, extrL,
                nbLay, nbIter,
                Gvol, Gfac, Gedg, Gnod,
                surfOpt, tmpdir, cas2D, refine, verbose=0, ):

  zfile = open(os.path.join(tmpdir,'insert.z7p'),'w')
  zfile.write('   #include <Zcracks_base.z7p> \n')
  zfile.write(' int main()\n{\n')
  zfile.write('   init_var();\n')

  if cas2D==True:
    zfile.write('   if_2D=1;\n')
    zfile.write('   thickness.resize(1);\n')
    zfile.write('   thickness[0]=-1.;\n')

  zfile.write('   format="geo";\n')
  zfile.write('   gradation=%e;\n' %grad)
  zfile.write('   min_size= %e;\n' %minS)
  zfile.write('   max_size=%e;\n' %maxS)
  zfile.write('   nb_velem=%d;\n' %(nbLay*2))
  zfile.write('   nb_iter=%d;\n' %nbIter)
  zfile.write('   sane_name="%s";\n' %saneN.replace('.geo',''))
  #zfile.write('   crack_name="%s";\n' %crackN.replace('.geo',''))
  zfile.write('   convert_surface("%s");\n' %crackN.replace('.geo',''))
  zfile.write('   cracked_name="%s";\n' %crackedN.replace('.geo',''))

  if Gfac!='': zfile.write('   faset_names="%s";\n' %(Gfac[0] if isinstance(Gfac, list) else Gfac))
  if Gnod!='': zfile.write('   nset_names="%s";\n' %(Gnod[0] if isinstance(Gnod, list) else Gnod))
  if Gvol!='': zfile.write('   elset_names="%s";\n' %(Gvol[0] if isinstance(Gvol, list) else Gvol))
  if Gedg!='': zfile.write('   liset_names="%s";\n' %(Gedg[0] if isinstance(Gedg, list) else Gedg))
  if surfOpt!='':
    zfile.write('   yams_options="%s";\n' %surfOpt)

  if refine: zfile.write('   if_must_refine=1;\n')

  if extrL<=1.E-12:
    zfile.write('   if_must_define_elset=0;\n')
  else:
    zfile.write('   if_must_define_elset=1;\n')
    if extrL==[]:
      zfile.write('   elset_radius=%e;\n' %maxS)
    else:
      zfile.write('   elset_radius=%e;\n' %extrL)

  zfile.write('   nice_cut(20.0);\n}\n\n')
  zfile.close()
  commande='Zrun -zp %s' %(os.path.join(tmpdir,'insert.z7p'))
  if verbose!=0: commande+=' >> %s' %os.path.join(tmpdir,'log.msg')
  res=os.system(commande)
  #print ' -------- '
  #print res
  #print ' -------- '
  return(res)



def insertCrack(data, names, tmpdir='./zcracks_temp', verbose=0):

  saneN=names['saneGeoName']
  crackN=names['crackGeoName']
  crackedN=names['crackedGeoName']

  minS=data['minSize'][0]
  maxS=data['maxSize'][0]
  extrL=data['extractLength'][0]

  grad = data['gradation'][0] if 'gradation' in list(data.keys()) else 1.3
  quad = data['quad'] if 'quad' in list(data.keys()) else False
  cas2D = data['is2D'] if 'is2D' in list(data.keys()) else False
  refine = data['refine'] if 'refine' in list(data.keys()) else False
  nbLay = data['layers'][0] if 'layers' in list(data.keys()) else 5
  nbIter = data['iterations'][0] if 'iterations' in list(data.keys()) else 2

  Gvol = data['grVol'] if 'grVol' in list(data.keys()) else ''
  Gfac = data['grFace'] if 'grFace' in list(data.keys()) else ''
  Gedg = data['grEdge'] if 'grEdge' in list(data.keys()) else ''
  Gnod = data['grNodes'] if 'grNodes' in list(data.keys()) else ''
  surfOpt = data['surfopt'] if 'surfopt' in list(data.keys()) else ''


  if not os.path.isdir(tmpdir): os.mkdir(tmpdir)
  curDir=os.getcwd()
  os.chdir(tmpdir)
  res=launchZcrack(minS, maxS, saneN, crackN, crackedN,grad, quad, extrL,
                nbLay, nbIter,Gvol, Gfac, Gedg, Gnod,surfOpt, tmpdir, cas2D, refine, verbose)
  os.chdir(curDir)
  return(res)


#def TUI(data, names, tmpdir='./zcracks_temp', verbose=0)
