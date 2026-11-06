with open("processreader.cpp", "r") as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if "continue;" in line and i == 47:
        lines[i] = "		throw e;\n"

with open("processreader.cpp", "w") as f:
    f.writelines(lines)
