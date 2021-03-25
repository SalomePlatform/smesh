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

from subprocess import Popen
from os import remove, getpid, path

def init(tmpdir):
  global log
  log=output(tmpdir)
  log.initialise()

def message(typ, message, goOn=False):
  global log
  log.message(typ,message,goOn)

class output():
  def __init__(self, tmpDir, tmpFile='Messages.txt'):
    self.tmpFile=path.join(tmpDir,tmpFile)

  def initialise(self):
    try:
      remove(self.tmpFile)
    except:
      pass
    f = open(self.tmpFile,'w')
    f.write('\n      ------------------------------\n')
    f.write('     |  BIENVENUE DANS L\'INTERFACE  |\n')
    f.write('     |      ZCRACKS DE SALOME       |\n')
    f.write('     |        VERSION BETA          |\n')
    f.write('       ------------------------------\n\n')
    f.close()

    pid=getpid()
    fenName='Zcracks message log'
    proc = Popen(['xterm -T "%s" -e "tail -s 0.05 -f %s --pid=%d"' %(fenName,self.tmpFile,pid)], shell=True)
    return()

  def message(self, typ, message='', goOn=False):
    fileName=self.tmpFile
    f = open(fileName,'a')
    if typ=='E':
      f.write('ERROR: '+message+'\n')
      #print 'ERROR: '+message
      if not goOn:
        exit()

    elif typ in ['A','W']:
      #print ARNING: '+message
      f.write('WARNING: '+message+'\n')

    elif typ in ['M','I']:
      #print 'INFO: '+message
      f.write(message+'\n')

    f.close()
