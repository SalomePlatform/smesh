# -*- coding: utf-8 -*-
# Copyright (C) 2006-2016  EDF R&D
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

import os

def ZcracksLct(context):
    import os,subprocess
    command = ". ${ZCRACKSHOME}/salome_do_config.sh ; "
    command += 'zcracksLaunch.py &'
    if command is not "":
      try:
        subprocess.check_call(command, executable = '/bin/bash', shell = True, bufsize=-1)
      except Exception, e:
        print "Error: ",e
