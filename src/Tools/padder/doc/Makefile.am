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

#  Author : Guillaume Boulant (EDF/R&D)

include $(top_srcdir)/adm_local/unix/make_common_starter.am

EXTRA_DIST += images input

#
# The simplest way to extend the documentation of SMESH with the
# documentation for PADDER is to add path to the padder documentation
# in the SMESH gui documentation (see the doxyfile).
#

# For test purpose, we let the user generate a local dosygen
# documentation including only the local pages  
#
test_docs: doxyfile 
	echo "===========================================" ; \
	echo "Generating PADDER documentation" ;             \
	echo "===========================================" ; \
	$(DOXYGEN) doxyfile ;
