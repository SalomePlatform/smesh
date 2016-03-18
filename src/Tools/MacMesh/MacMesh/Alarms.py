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

def Message (code) :
	import sys
	MessageString = { 1 : lambda x: "Successfully created \n",
                          2 : lambda x: "Fatal: Incorrect input \n",
                          3 : lambda x: "Fatal: Overlapping objects detected \n",
                          4 : lambda x: "Fatal: Incompatible object type with neighbouring objects" }[code](str(code))
	print MessageString
	#if code > 1 : sys.exit()
	return 1

