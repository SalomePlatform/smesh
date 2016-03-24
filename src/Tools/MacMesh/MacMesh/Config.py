# Copyright (C) 2014-2016  EDF R&D
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



dz = 1
debug = 1

try : ListObj
except NameError : ListObj = []

try: Connections
except NameError: Connections = []

try : publish
except NameError : publish = 1

try : Groups
except NameError : Groups = []

try : StudyName
except NameError : StudyName = "Compound"

try : Criterion
except NameError : Criterion = 1

try : Count
except NameError : Count = 0

try : RefPts
except NameError : RefPts = []

try : DirIndex
except NameError : DirIndex = 0

try : theStudy
except NameError :
    import salome
    salome.salome_init()
    theStudy = salome.myStudy


