#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  File   : Makefile.in
#  Author : Patrick GOLDBRONN (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am


bin_PROGRAMS = MeshCut

MeshCut_SOURCES = \
  MeshCut_Globals.hxx \
  MeshCut_Carre.hxx \
  MeshCut_Carre.cxx \
  MeshCut_Cube.hxx \
  MeshCut_Cube.cxx \
  MeshCut_Maillage.hxx \
  MeshCut_Maillage.cxx \
  MeshCut_Fonctions.hxx \
  MeshCut_Fonctions.cxx \
  MeshCut_Utils.hxx \
  MeshCut_Utils.cxx \
  MeshCut_Cas.hxx \
  MeshCut_Cas.cxx \
  MeshCut_DC.cxx

MeshCut_CPPFLAGS =  $(MED2_INCLUDES)

MeshCut_LDFLAGS =  $(MED2_LIBS) $(HDF5_LIBS)

