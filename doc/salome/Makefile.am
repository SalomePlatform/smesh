# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

# -* Makefile *- 
# Author : Patrick GOLDBRONN (CEA)
# Date : 30/11/2001
#  Modified by : Alexander BORODIN (OCN) - autotools usage
# $Header:
#
SUBDIRS = tui gui
SUBDIRSTUI = tui
SUBDIRSGUI = gui

usr_docs:
	@@SETX@; for d in $(SUBDIRSGUI); do 	\
	   (cd $$d && $(MAKE) $@) || exit 1; 	\
	done;

docs: usr_docs

dev_docs:
	@@SETX@; for d in $(SUBDIRSTUI); do 	\
	   (cd $$d && $(MAKE) $@) || exit 1; 	\
	done;
