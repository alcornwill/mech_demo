
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
# YOU COULD also limit 
# all floats to 1 char
# then has to be in range -1.28 > 1.28
# fixed precision 2
# ALSO you could
# limit names to 4 chars

# NOTE
# fixed is a short int
# representing a fixed point decimal
# with 3 signigicant digits
# (range: -32 to 32)

# format:
# bytes     type    content
################################
# 2         ushort  num meshes
############ MESHES ############ 
# 1         uchar   name length
# ~         char    name
# 1         uchar   num geoms
# ~         Geom    geoms
############ GEOMS #############
# 1         uchar   mat name length
# ~         char    mat name
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
# 2         ushort  num materials
########### MATERIALS ##########
# 1         uchar   name length
# ~         char    name
# 2*3       ushort  color
################################
# 2         ushort  num objects
############ OBJECTS ###########
# 2         uchar   name length
# ~         char    name
# 2         uchar   mesh name length
# ~         char    mesh name
# 2         uchar   parent name length
# ~         char    parent name
# 2         uchar   anim name length
# ~         char    anim name
# 2*16      fixed   transform
################################
# 2         ushort  num animations
########### ANIMATIONS #########
# 1         uchar   name length
# ~         char    name
# 2         ushort  num keys
# ~         AKey    animation keys
######### ANIMATION KEYS ######
# 2         ushort  AKey time
# 2*16      fixed   AKey transform
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
import bmesh
from bmesh.types import BMFace

def write_uchar(val):
    l = pack('B', val)
    out.write(l)
    log.write("wrote: uchar: {}\n".format(str(val)))

def write_ushort(val):
    l = pack('H', val)
    out.write(l)
    log.write("wrote: ushort: {}\n".format(str(val)))

def write_short(val):
    l = pack('h', val)
    out.write(l)
    log.write("wrote: short: {}\n".format(str(val)))
    
def write_fixed(val):
    write_short(int(val * 1000))
    
def normalize_name(name):
    return name.replace('.', '').lower()
    
def write_length(data):
    # write the length of an iterable
    write_ushort(len(data))
    
def write_array(data):
    fixed = type(data[0]) == float
    # write data
    for item in data:
        if fixed:
            write_fixed(item)
        else:
            write_ushort(item)

def write_name(name):
    # writes number of characters, followed by char array
    name = normalize_name(name)
    name = name.encode()
    
    # write name length (unsigned char)
    write_uchar(len(name))
    
    l = len(name)
    fmt = str(l) + 's'
    b = pack(fmt, name)
    out.write(b)
    log.write("wrote: char[{}]: {}\n".format(l, name.decode()))
    
def write_matrix(mat):
    m = [i for vec in mat for i in vec]
    for f in m:
        write_fixed(f)
    log.write("wrote: matrix\n")
    
def write_color(color):
    for val in list(color):
        write_fixed(val)
    log.write("wrote: color\n")
    
def bmesh_split(bm, geom, dest):
    dest.faces.ensure_lookup_table()
    indices = [elem.index for elem in geom]
    if type(geom[0]) == BMFace:
        for face in list(dest.faces):
            if face.index not in indices:
                dest.faces.remove(dest.faces[face.index])
                dest.faces.ensure_lookup_table()
        assert(len(dest.faces) == len(geom))
        
def bmesh_split_by_material(bm, mat):
    dest = bm.copy()
    dest.faces.ensure_lookup_table()

    for face in list(dest.faces):
        if face.material_index != mat:
            dest.faces.remove(face)     
            
    for v in list(dest.verts):
        # remove loose verts
        if v.is_valid and v.is_wire:
            dest.verts.remove(v)
    
    dest.faces.index_update()
    dest.edges.index_update()
    dest.verts.index_update()
    
    expected = len([face for face in bm.faces if face.material_index == mat]) 
    actual = len(dest.faces)
    assert expected == actual, "expected: {}\nactual: {}".format(expected, actual)
    
    return dest
                
def write_mesh(obj, use_indices, use_edges, use_normals, use_colors, use_uvs):
    data = obj.data
    
    # write name
    write_name(data.name)
    mats = data.materials
    
    bm = bmesh.new()
    bm.from_mesh(data)
    
    bmesh.ops.triangulate(bm, faces=bm.faces) # doesn't export quads/ngons
    
    bm.faces.ensure_lookup_table()
    
    bms = []
    for mat in range(len(mats)):
        faces = [face for face in bm.faces if face.material_index == mat]
        if faces:
            ##bm2 = bmesh.new()
            #bmesh.ops.split(bm, geom=faces, dest=bm2)
            #bm2 = bm.copy()
            #bmesh_split(bm, geom=faces, dest=bm2)
            bm2 = bmesh_split_by_material(bm, mat)
            bms.append((bm2, mats[mat]))
            
    # write number of geoms
    write_uchar(len(bms))
    
    for bm1, mat in bms:
        # for each geom
        bm1.faces.ensure_lookup_table()
        bm1.edges.ensure_lookup_table()
        
        # write material name
        write_name(mat.name)
    
        # write verts
        numVerts = len(bm1.verts)
        verts = [vec for vert in bm1.verts for vec in list(vert.co)]
        write_ushort(numVerts)
        write_array(verts)
            
        # write indices
        if use_indices:
            numIndices = len(bm1.faces)
            indices = [loop.vert.index for face in bm1.faces for loop in face.loops]
            write_ushort(numIndices)
            write_array(indices)
        else:
            write_ushort(0)
        
        # write edges
        if use_edges:
            numEdges = len(bm1.edges)
            edges = [vert.index for edge in bm1.edges for vert in edge.verts]
            write_ushort(numEdges)
            write_array(edges)
        else:
            write_ushort(0)
        
        # write normals
        if use_normals:
            numNormals = len(bm1.verts)
            normals = [vec for vert in bm1.verts for vec in list(vert.normal)]
            write_ushort(numNormals)
            write_array(normals)
        else:
            write_ushort(0)
        
        # todo colors and uvs
        write_ushort(0)
        write_ushort(0)
        
        bm1.free()
        
    bm.free()

            
def write_obj(obj):

    write_name(obj.name)
    write_name(obj.data.name)
    
    # write parent name
    if obj.parent:
        write_name(obj.parent.name)
    else:
        write_uchar(0)
        
    write_name(obj.name) # animation name...
        
    # use OrientationHelper?
    write_matrix(obj.matrix_local)

    
def write_anim(obj):

    write_name(obj.name) # hmm, the object name is the animation name
    # todo we could mangle with action.name
    # but only one animation for now
    
    # write animation keys
    # we have to iterate over frame range
    # and just write out the transform
    anim = obj.animation_data.action
    start, end = anim.frame_range
    start = int(start)
    end = int(end+1)
    write_ushort(end - start)
    for t in range(start, end):
        bpy.context.scene.frame_set(t)
        write_ushort(t)
        write_matrix(obj.matrix_local)
    
def write_material(mat):
    write_name(mat.name)
    write_color(mat.diffuse_color)
 
def custom_export(filepath, *args):
    global out
    out = open(filepath, 'wb')

    objs = bpy.context.selected_objects
    
    # write meshes
    log.write("#### MESHES ####\n")
    write_length(objs)
    for obj in objs:
        write_mesh(obj, *args)
        
    # write materials
    log.write("#### MATERIALS ####\n")
    mats = bpy.data.materials
    write_length(mats)
    for mat in mats:
        write_material(mat)
        
    # write objects
    log.write("#### OBJECTS ####\n")
    write_length(objs)
    for obj in objs:
        write_obj(obj)
    
    # write animations
    log.write("#### ANIMATIONS ####\n")
    write_length(objs)
    for obj in objs:
        write_anim(obj)
        
    out.close()
    log.close()
    
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
        
        global log
        log = open(self.filepath + '.log', 'w')
    
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
