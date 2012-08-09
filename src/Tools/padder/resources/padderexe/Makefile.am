# Copyright (C) 2011-2012  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

include $(top_srcdir)/adm_local/unix/make_common_starter.am

mysalomeresdir=$(salomeresdir)/padderexe

version=med3
notinstall_version=med2

# We install the padder.exe program and the files required for testing
# the execution from within the installation directory
dist_mysalomeres_SCRIPTS =    \
	$(version)/padder.exe \
	padder.sh

nodist_mysalomeres_SCRIPTS =         \
	envPadder.sh

dist_mysalomeres_DATA =         \
	$(version)/data.txt     \
	$(version)/concrete.med \
	$(version)/ferraill.med

NOT_USED_FILES = buildparticules.py particules.png
EXTRA_DIST += $(notinstall_version) $(NOT_USED_FILES)
