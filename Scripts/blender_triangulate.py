# Script to use with Blender for triangulization of all mesh objects in the scene.
# Assumes we are in object mode.
#
# HOW TO USE:
#   Open in Blender Text Editor, switch to Object Mode, click "Run Script"

import bpy
import bmesh

def triangulate(obj):
    m = obj.data
    bm = bmesh.new()
    bm.from_mesh(m)

    bmesh.ops.triangulate(bm, faces=bm.faces[:], quad_method=0, ngon_method=0)

    bm.to_mesh(m)
    bm.free()


for obj in bpy.context.scene.objects:
    if obj.type in [ 'MESH' ]:
        print("Triangulating " + obj.name)
        triangulate(obj)
