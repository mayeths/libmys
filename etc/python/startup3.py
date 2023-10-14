exelines = (
    'import numpy as np',
    'import sys',
    'import os',
)

print("--- [Mayeths python3 startup] ---")
for line in exelines:
    try:
        exec(line)
        print(line)
    except:
        print("> Failed to execute: %s" % line)
print("---------------------------------")
