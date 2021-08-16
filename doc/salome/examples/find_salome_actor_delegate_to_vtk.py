

# Find if SALOME_ACTOR_DELEGATE_TO_VTK is activated

import os
import sys

result = 'SALOME_ACTOR_DELEGATE_TO_VTK' in os.environ
sys.exit(int(not result))
