import os
from sys import argv
argv = argv[argv.index("--") + 1:]

# Add "--no-window-focus" to the argv list
argv.append("--no-window-focus")

# Prevent splash screen from loading
os.environ["PYOPENGL_PLATFORM"] = "osmesa"

# format specific options... change as you like
args_fbx = dict(
    # global_scale=1.0,
    use_image_search=True
)

args_obj = dict(

)

args_3ds = dict(
    # constrain_size=0.0,
)

args_gltf = dict(

)

args_glb = dict(

)

import bpy
#bpy.ops.wm.read_homefile(use_empty=True) # Open Blender without the startup modal window
for f in argv:
    ext = os.path.splitext(f)[1].lower()

    if ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=f, **args_fbx)
        #bpy.ops.import_scene.x3d(filepath=f, axis_forward='-Z', axis_up='Y', filter_glob='*.fbx', use_selection=False, **args_fbx)
    elif ext == ".obj":
        bpy.ops.import_scene.obj(filepath=f, **args_obj)
    elif ext == ".3ds":
        bpy.ops.import_scene.autodesk_3ds(filepath=f, **args_3ds)
    elif ext == ".gltf":
        bpy.ops.import_scene.gltf(filepath=f, **args_gltf)
    elif ext == ".glb":
        bpy.ops.import_scene.gltf(filepath=f, **args_glb)
    else:
        print("Extension %r is not known!" % ext)
if not argv:
    print("No files passed")