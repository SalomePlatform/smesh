
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
    f.write('     |   BIENVENU DANS L\'INTERFACE  |\n')
    f.write('     |      ZCRACKS DE SALOME       |\n')
    f.write('     |        VERSION ALPHA         |\n')
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
