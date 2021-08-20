

# Find if SALOME_ACTOR_DELEGATE_TO_VTK is activated

import os
import sys

if 'SALOME_ACTOR_DELEGATE_TO_VTK' not in os.environ:
    raise RuntimeError('SALOME_ACTOR_DELEGATE_TO_VTK is not set!')
