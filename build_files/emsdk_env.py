#!python
import sys, os, subprocess

emsdkPath = sys.argv[1]
sys.path.append(emsdkPath)

import emsdk
emsdk.main(["activate latest"])
tools_to_activate = emsdk.currently_active_tools()
tools_to_activate = emsdk.process_tool_list(tools_to_activate)
env_string = emsdk.construct_env(tools_to_activate, False, False)

for line in env_string.replace("\r", "").split("\n"):
	if env_string.startswith("SET "):
		line = line[4:]
	elif env_string.startswith("export "):
		line = line[7:]
	elif env_string.startswith("$env:"):
		line = line[5:]
	elif env_string.startswith("setenv"):
		line = line[6:]
	elif env_string.startswith("export"):
		line = line[6:]
	
	i = line.find("=")
	key = line[:i].strip()
	val = line[i+1:]

	if len(key) > 0:
		os.environ[key] = val

	#print("KEY", key, "VAL", val)

print("EMSDK::ENV", env_string)

CMD = sys.argv[2]

splitchar = ";" if sys.platform.lower().startswith("win") else ":"

for path in os.environ["PATH"].split(splitchar):
	print("  ", path)
	path2 = os.path.join(path, CMD)
	if os.path.exists(path2):
		print("found!")
		CMD = os.path.abspath(path2)

		if os.path.exists(CMD + ".py"):
			CMD = "python " + CMD + ".py"
		break

cmd = ""
for arg in sys.argv[3:]:
	if arg.strip().startswith("/std:") or arg.startswith("--std"):
		arg = "-std=c++20"
	if arg.strip() == "/nologo":
		continue
	cmd += arg + " "

print(sys.argv)
print("COMMAND:", CMD, cmd)

sys.stdout.flush()

sys.exit(os.system(CMD + " " + cmd))

