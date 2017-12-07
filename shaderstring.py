
file = open("shaders/default.vert", 'r')
vs = file.read()
file.close()
vs = vs.replace('\n', '\\n')

file = open("shaders/default.frag", 'r')
fs = file.read()
file.close()
fs = fs.replace('\n', '\\n')

s = """
const GLchar * vsSource = "{}";

const GLchar * fsSource = "{}";
""".format(vs, fs)

header = open("shaders.h", 'w')
header.write(s)
header.close()