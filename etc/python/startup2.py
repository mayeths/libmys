exelines = (
    'import sys',
    'import os',
)

print("--- [Mayeths python2 startup] ---")
for line in exelines:
    try:
        exec(line)
        print(line)
    except:
        print("> Failed to execute: %s" % line)
print("---------------------------------")
