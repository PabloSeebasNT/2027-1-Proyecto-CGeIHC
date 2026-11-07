import bpy, json, os
from mathutils import Vector
base=os.path.dirname(os.path.abspath(__file__))
report={}
for name in ['Bmo','Jake']:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=os.path.join(base,name+'.fbx'))
    objects=[]
    for o in bpy.context.scene.objects:
        d={'name':o.name,'type':o.type,'parent':o.parent.name if o.parent else None}
        if o.type=='MESH':
            o.data.calc_loop_triangles()
            pts=[o.matrix_world@Vector(v) for v in o.bound_box]
            d.update(vertices=len(o.data.vertices),triangles=len(o.data.loop_triangles),
                bounds=[[min(p[i] for p in pts) for i in range(3)],[max(p[i] for p in pts) for i in range(3)]],
                materials=[m.name for m in o.data.materials if m],groups=[g.name for g in o.vertex_groups])
        if o.type=='ARMATURE':
            d['bones']=[{'name':b.name,'parent':b.parent.name if b.parent else None,'head':list(b.head_local),'tail':list(b.tail_local)} for b in o.data.bones]
        objects.append(d)
    images=[]
    for im in bpy.data.images:
        images.append({'name':im.name,'path':im.filepath,'size':list(im.size),'packed':bool(im.packed_file)})
    report[name]={'objects':objects,'images':images,'materials':[{'name':m.name,'color':list(m.diffuse_color)} for m in bpy.data.materials]}
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(base,name+'_original.blend'))
with open(os.path.join(base,'inspeccion.json'),'w',encoding='utf8') as f:json.dump(report,f,indent=2)
