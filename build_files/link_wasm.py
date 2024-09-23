import os, sys

emcc = sys.argv[1]

exportSymbols = [
	"createMesh",
	"freeMesh",
	"getStrData",
	"makeVertex",
	"freeAttrRef",
	"getAttr",
	"copyAttrRef",
	"memAlloc",
	"memRelease",
]

cmd = " ".join(sys.argv[1:])

cmd += " -sEXPORTED_FUNCTIONS="
first = True
for sym in exportSymbols:
	if not first:
		cmd += ","
	first = False
	cmd += "_" + sym

cmd += " -g -gsource-map -o sculptcore.js" 
print("Linking final WASM/JS files. . .", cmd)
sys.exit(os.system(cmd))
