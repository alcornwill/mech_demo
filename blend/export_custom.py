
bl_info = {
    "name": "Custom Export",
    "description": "Export mesh as C header file.",
    "author": "Will Alcorn",
    "version": (1, 0),
    "blender": (2, 79, 0),
    "location": "File > Export > Custom Export",
    #"wiki_url": "http://wiki.blender.org/index.php/Extensions:2.6/Py/",
    #"support": "COMMUNITY",
    "category": "Import-Export"
}


import struct
from os import mkdir, exists
from os.path import abspath, dirname, join

import bpy
from bpy.props import (
    StringProperty,
    BoolProperty,
    EnumProperty
)
from bpy_extras.io_utils import ExportHelper, orientation_helper_factory, path_reference_mode  


filepath = None


def mesh_triangulate(me):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()

def normalize_name(name):
    return name.replace('.', '').lower()

def get_data_info(data):
    num = len(data)
    stride = len(data[0])
    f = data[0][0]
    if type(f) == float:
        fmt = 'f'
    if type(f) == int:
        fmt = 'I'
    size = struct.calcsize(fmt)
    datasize = size * stride * num
    return num, stride, fmt, size, datasize
    
def hwrite(path, size, fmt, items):
    # hex write (silly)
    c = '{{:0{}x}}'.format(size*2)
    binfile = open(path, 'w')
    count = 0
    for row in items:
        for val in row:
            b = struct.pack(fmt, val)
            z = struct.unpack('I', b)[0]
            s = c.format(z).upper()
            binfile.write(s)
            
            count += size
            count %= 16
            if not count:
                binfile.write('\n')
    binfile.close()
    
def bwrite(path, size, fmt, items):
    binfile = open(path, 'wb')
    for row in items:
        for val in row:
            b = struct.pack(fmt, val)
            binfile.write(b)
    binfile.close()
    
def write_extern(method, out, objName, keyword, items):
    arrayName = objName.lower() + keyword + "Data"
    binaryFileName = (objName + '_' + keyword).lower()
    defineBinary = (binaryFileName + "_FILE").upper()
    defineName = (objName + '_NUM_' + keyword).upper()
    defineSizeName = (objName + '_' + keyword + '_DATA_SIZE').upper()
    
    datadir = join(dirname(abspath(fp)), "data")
    if not exists(datadir):
        mkdir(datadir)
    binpath = join(datadir, binaryFileName)
    
    num, stride, fmt, size, datasize = get_data_info(items)
    method(binpath, size, fmt, items)
    
    # todo could also write stride and data type
    out.write('#define {} {}\n'.format(defineBinary, binaryFileName))
    out.write('#define {} {}\n'.format(defineName, num))
    out.write('#define {} {}\n\n'.format(defineSizeName, datasize))
    
def write_binary(*args):
    write_extern(bwrite, *args)
    
def write_hex(*args):
    write_extern(hwrite, *args)
    
def write_C_array(out, name, keyword, items):
    arrayName = name.lower() + keyword.title() + "Data"
    defineName = (name + '_NUM_' + keyword).upper()
    defineSizeName = (name + '_' + keyword + '_DATA_SIZE').upper()
    
    num, stride, fmt, size, datasize = get_data_info(items)
    gltype = "GLfloat" if fmt == 'f' else "GLuint" # todo more?
    
    out.write('{} {}[] = {{\n    '.format(gltype, arrayName))
    for i in range(num):
        row = items[i]
        lastrow = i == num - 1
        for j in range(stride):
            item = row[j]
            
            if isinstance(item, float):
                out.write(' {0:.3f}'.format(item))
            else:
                out.write(' {}'.format(item))
            
            islast = j == len(row) - 1
            if lastrow and islast:
                out.write('\n')
            elif islast:
                out.write(',\n    ')
            else:
                out.write(',')
    out.write('};\n\n')
    
    out.write('#define {} {}\n'.format(defineName, len(items)))
    out.write('#define {} {}\n\n'.format(defineSizeName, datasize))

def export_obj(context, out, writer, obj, use_indices, use_edges, use_normals, use_colors, use_uvs):
    data = obj.data
    name = normalize_name(obj.name)
    
    verts = [list(vert.co) for vert in data.vertices]
    count = writer(out, name, 'vertex', verts)
    
    if use_indices:
        mesh_triangulate(data) # doesn't export quads/ngons
        indices = [list(poly.vertices) for poly in data.polygons]
        count = writer(out, name, 'index', indices)
    
    if use_edges:
        edges = [list(edge.vertices) for edge in data.edges]
        count = writer(out, name, 'edge', edges)
    
    if use_normals:
        normals = [list(vert.normal) for vert in data.vertices]
        count = writer(out, name, 'normal', normals)

    if use_colors:
        colors = [list(color.color) for color in data.vertex_colors[0].data]
        count = writer(out, name, 'color', colors)
    
    if use_uvs:    
        uvs = [list(uv.uv) for uv in data.uv_layers[0].data]
        count = writer(out, name, 'uv', uvs)
            

def custom_export(context, writer, *args):
    out = open(filepath, 'w')

    # HMM you could create a struct
    # and store data about objects with it
    # in a list
    # then can iterate list for easy batching/rendering
    
    out.write('\n')
    out.write('#include <SDL_opengl.h>\n\n')

    selected = context.selected_objects
    for obj in selected:
        export_obj(context, out, writer, obj, *args)
        
    out.close()
    
IOOBJOrientationHelper = orientation_helper_factory("IOOBJOrientationHelper", axis_forward='-Z', axis_up='Y')

class SimpleOperator(bpy.types.Operator, ExportHelper, IOOBJOrientationHelper):
    """Tooltip"""
    bl_idname = "export_scene.custom"
    bl_label = "Custom Export"
    bl_options = {'REGISTER', 'PRESET'}
    
    filename_ext = ".h"
    filter_glob = StringProperty(
        default="*.h",
        options={'HIDDEN'},
    )
    
    path_mode = path_reference_mode
    check_extension = True
    
    writer = EnumProperty(
        "Writer",
        items=[
            ('0', 'Header Only', ''),
            ('1', 'External Binary', ''),
            ('2', 'External Hex', '')
        ]
    )
    indices = BoolProperty("Indices", default=True)
    edges = BoolProperty("Edges")
    normals = BoolProperty("Normals")
    colors = BoolProperty("Colors")
    uvs = BoolProperty("UVs")

    @classmethod
    def poll(cls, context):
        return context.active_object is not None

    def execute(self, context):
        global filepath
        filepath = self.filepath
        writers = [write_C_array, write_binary, write_hex]
        writer = writers[int(self.writer)]
        custom_export(context, writer, self.indices, self.edges, self.normals, self.colors, self.uvs)
        return {'FINISHED'}
    
def menu_func_export(self, context):
    self.layout.operator(SimpleOperator.bl_idname, text="Custom Export (.c)")


def register():
    bpy.utils.register_class(SimpleOperator)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(SimpleOperator)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)
    
if __name__ == "__main__":
    register()
