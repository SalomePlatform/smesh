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

#-----------------------------#
def writeFile( fn, txt = None):
#-----------------------------#

   if txt == None : return
   if fn  == None : return
   fn = str(fn)
   try:
      f = open(fn, 'w')
      f.write(txt)
      f.close()
      return 1
   except IOError:
      return 0


#---------------------#
def readFile( fn, txt):
#---------------------#

   if fn  == None : return
   try:
      f = open(fn, 'w')
      txt=f.read()
      f.close()
   except IOError:
      txt=""
   return txt

