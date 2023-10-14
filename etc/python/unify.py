import sys
import os

dir = os.path.dirname(os.path.realpath(__file__))

if sys.version_info[0] == 2:
    startup = os.path.join(dir, "startup2.py")
else:
    startup = os.path.join(dir, "startup3.py")

exec(open(startup).read())
