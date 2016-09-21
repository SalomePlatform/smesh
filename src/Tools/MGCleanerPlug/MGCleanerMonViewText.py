# -*- coding: utf-8 -*-
# Copyright (C) 2013-2016  EDF R&D
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

# Modules Python
import os
import sys
import string
import types
import tempfile
import traceback
import pprint as PP #pretty print

from qtsalome import *

# Import des panels

from MGCleanerViewText_ui import Ui_ViewExe

verbose = True

force = os.getenv("FORCE_DISTENE_LICENSE_FILE")
if force != None:
  os.environ["DISTENE_LICENSE_FILE"] = force
  os.environ["DLIM8VAR"] = "NOTHING"

class MGCleanerMonViewText(Ui_ViewExe, QDialog):
    """
    Classe permettant la visualisation de texte
    """
    def __init__(self, parent, txt, ):
        QDialog.__init__(self,parent)
        self.setupUi(self)
        self.resize( QSize(1000,600).expandedTo(self.minimumSizeHint()) )
        #self.connect( self.PB_Ok,SIGNAL("clicked()"), self, SLOT("close()") )
        self.PB_Ok.clicked.connect( self.theClose )
        self.PB_Save.clicked.connect( self.saveFile )
        self.PB_Save.setToolTip("Save trace in log file")
        self.PB_Ok.setToolTip("Close view")
        self.monExe=QProcess(self)

        self.monExe.readyReadStandardOutput.connect( self.readFromStdOut )
        self.monExe.readyReadStandardError.connect( self.readFromStdErr )
        self.monExe.finished.connect( self.finished )

        """ for test set environment
        env = QProcessEnvironment().systemEnvironment()
        env.insert("HELLO", "bonjour") #Add an environment variable for debug
        self.monExe.setProcessEnvironment(env)
        if verbose: 
          PP.pprint([str(i) for i in sorted(self.monExe.processEnvironment().toStringList()) if 'DISTENE' in i])
        """
        
        cmds = ''
        ext = ''
        if sys.platform == "win32":
            if os.path.exists(self.parent().fichierOut):
                cmds += 'del %s\n' % self.parent().fichierOut
            ext = '.bat'
        else:
            cmds += '#!/bin/bash\n'
            cmds += 'pwd\n'
            #cmds += 'which mg-cleaner.exe\n'
            cmds += 'echo "DISTENE_LICENSE_FILE="$DISTENE_LICENSE_FILE\n'
            cmds += 'echo "DLIM8VAR="$DLIM8VAR\n'
            cmds += 'rm -f %s\n' % self.parent().fichierOut
            ext = '.bash'

        cmds += 'echo %s\n' % txt #to see what is compute command
        cmds += txt+'\n'
        cmds += 'echo "END_OF_MGCleaner"\n'
        
        nomFichier = os.path.splitext(self.parent().fichierOut)[0] + ext
        with open(nomFichier, 'w') as f:
          f.write(cmds)
        self.make_executable(nomFichier)
        
        if verbose: print("INFO: MGCleaner launch script file: %s" % nomFichier)
        
        self.monExe.start(nomFichier)
        self.monExe.closeWriteChannel()
        self.enregistreResultatsDone=False
        self.show()

    def make_executable(self, path):
        mode = os.stat(path).st_mode
        mode |= (mode & 0o444) >> 2    # copy R bits to X
        os.chmod(path, mode)

    def saveFile(self):
        #recuperation du nom du fichier
        savedir=os.environ['HOME']
        fn = QFileDialog.getSaveFileName(None, self.trUtf8("Save File"),savedir)
        if fn.isNull() : return
        ulfile = os.path.abspath(unicode(fn))
        try:
           f = open(fn, 'wb')
           f.write(str(self.TB_Exe.toPlainText()))
           f.close()
        except IOError, why:
           QMessageBox.critical(self, self.trUtf8('Save File'),
                self.trUtf8('The file <b>%1</b> could not be saved.<br>Reason: %2')
                    .arg(unicode(fn)).arg(str(why)))

    def readFromStdErr(self):
        a=self.monExe.readAllStandardError()
        self.TB_Exe.append(unicode(a.data()))

    def readFromStdOut(self) :
        a=self.monExe.readAllStandardOutput()
        aa=unicode(a.data())
        self.TB_Exe.append(aa)
    
    def finished(self):
        self.parent().enregistreResultat()
        self.enregistreResultatsDone=True
    
    def theClose(self):
      if not self.enregistreResultatsDone:
        self.parent().enregistreResultat()
        self.enregistreResultatsDone=True
      self.close()
