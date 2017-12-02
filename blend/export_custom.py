
bl_info = {
    "name": "Custom Export",
    "description": "Export custom binary format.",
    "author": "Will Alcorn",
    "version": (1, 0),
    "blender": (2, 79, 0),
    "location": "File > Export > Custom Export",
    #"wiki_url": "http://wiki.blender.org/index.php/Extensions:2.6/Py/",
    #"support": "COMMUNITY",
    "category": "Import-Export"
}

# stupid idea:
# it would be a funny limitation
# to only allow 255 vertices
# then can use char everywhere
# (i guess you could automatically split up meshes...)

# NOTE
# fixed is a short int
# representing a fixed point decimal
# with 3 signigicant digits
# (range: -32 to 32)

# format:
# bytes     type    content
################################
# 2         ushort  num meshes
################################
# 1         uchar   name length
# ~         char    name
# 2         ushort  num vertices
# ~         fixed   vertex data
# 2         ushort  num indices
# ~         ushort  index data
# 2         ushort  num edges
# ~         ushort  edge data
# 2         ushort  num normals
# ~         fixed   normal data
# 2         ushort  num colors
# ~         fixed   color data
# 2         ushort  num uvs
# ~         fixed   uv data
################################
# 2         ushort  num objects
################################
# 2         uchar   name length
# ~         char    name
# 2         uchar   mesh name length
# ~         char    mesh name
# 2         uchar   parent name length
# ~         char    parent name
# 2*16      fixed   transform
################################
# 2         ushort  num animations
################################
# 1         uchar   object name length
# ~         char    object name
# 2         ushort  num keys
# ~         AKey    animation keys
#   2       ushort  AKey time
#   2*16    fixed   AKey transform
################################



from struct import pack, calcsize

import bpy
from bpy.props import (
    StringProperty,
    BoolProperty,
    EnumProperty
)
from bpy_extras.io_utils import (
    ExportHelper, 
    orientation_helper_factory, 
    path_reference_mode,
    axis_conversion
)


def mesh_triangulate(me):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()

def normalize_name(name):
    return name.replace('.', '').lower()
    
def write_length(data):
    # write the length of an iterable
    l = pack('H', len(data))
    out.write(l)
            
def write_array(data):
    # write number of elements
    write_length(data)
    
    fixed = type(data[0][0]) == float
    fmt = 'h' if fixed else 'H'
    
    # write data
    for row in data:
        for val in row:
            if fixed:
                val = int(val * 1000) # fixed point precision 3
            b = pack(fmt, val)
            out.write(b)

def write_name(name):
    # writes number of characters, followed by char array
    name = normalize_name(name)
    name = name.encode()
    
    # write name length (unsigned char)
    l = pack('B', len(name))
    out.write(l)
    
    l = len(name)
    fmt = str(l) + 's'
    b = pack(fmt, name)
    out.write(b)
    
def write_matrix(mat):
    m = [i for vec in mat for i in vec]
    for f in m:
        b = pack('f', f)
        out.write(b)
    
def write_mesh(obj, use_indices, use_edges, use_normals, use_colors, use_uvs):
    data = obj.data
    
    # write name
    write_name(data.name)
    
    # write mesh data
    verts = [list(vert.co) for vert in data.vertices]
    write_array(verts)
    
    if use_indices:
        mesh_triangulate(data) # doesn't export quads/ngons
        indices = [list(poly.vertices) for poly in data.polygons]
        write_array(indices)
    
    if use_edges:
        edges = [list(edge.vertices) for edge in data.edges]
        write_array(edges)
    
    if use_normals:
        normals = [list(vert.normal) for vert in data.vertices]
        write_array(normals)

    if use_colors:
        colors = [list(color.color) for color in data.vertex_colors[0].data]
        write_array(colors)
    
    if use_uvs:    
        uvs = [list(uv.uv) for uv in data.uv_layers[0].data]
        write_array(uvs)
            
def write_obj(obj):

    write_name(obj.name)
    write_name(obj.data.name)
    
    # write parent name
    if obj.parent:
        write_name(obj.parent.name)
    else:
        out.write(pack('B', 0))
        
    # use OrientationHelper?
    write_matrix(obj.matrix_local)

    
def write_anim(obj):

    write_name(obj.name)
    
    # todo we could write action.name
    # but only one animation for now
    
    # write animation keys
    # we have to iterate over frame range
    # and just write out the transform
    anim = obj.animation_data.action
    start, end = anim.frame_range
    start = int(start)
    end = int(end)
    out.write(pack('I', end - start))
    for t in range(start, end+1):
        bpy.context.scene.frame_set(t)

        out.write(pack('f', t))
        write_matrix(obj.matrix_local)
    
 
def custom_export(filepath, *args):
    global out
    out = open(filepath, 'wb')

    objs = bpy.context.selected_objects
    
    # write meshes
    write_length(objs)
    for obj in objs:
        write_mesh(obj, *args)
        
    # write objects
    write_length(objs)
    for obj in objs:
        write_obj(obj)
    
    # write animations
    write_length(objs)
    for obj in objs:
        write_anim(obj)
        
    out.close()
    
IOOBJOrientationHelper = orientation_helper_factory("IOOBJOrientationHelper", axis_forward='-Z', axis_up='Y')

class SimpleOperator(bpy.types.Operator, ExportHelper, IOOBJOrientationHelper):
    """Tooltip"""
    bl_idname = "export_scene.custom"
    bl_label = "Custom Export"
    bl_options = {'REGISTER', 'PRESET'}
    
    filename_ext = ".3d"
    filter_glob = StringProperty(
        default="*.3d",
        options={'HIDDEN'},
    )
    
    path_mode = path_reference_mode
    check_extension = True
    
    indices = BoolProperty("Indices", default=True)
    edges = BoolProperty("Edges")
    normals = BoolProperty("Normals")
    colors = BoolProperty("Colors")
    uvs = BoolProperty("UVs")

    @classmethod
    def poll(cls, context):
        return context.active_object is not None

    def execute(self, context):
        global global_matrix
        global_matrix = axis_conversion(from_forward=self.axis_forward,
                                        from_up=self.axis_up,
                                        ).to_4x4()
        # i guess, to use global_matrix
        # you have to multiply by all
        # matrices, vertices, normals?
    
        custom_export(self.filepath, self.indices, self.edges, self.normals, self.colors, self.uvs)
        context.scene.frame_set(1)
        return {'FINISHED'}
    
def menu_func_export(self, context):
    self.layout.operator(SimpleOperator.bl_idname, text="Custom Export (.3d)")


def register():
    bpy.utils.register_class(SimpleOperator)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(SimpleOperator)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)
    
if __name__ == "__main__":
    register()
