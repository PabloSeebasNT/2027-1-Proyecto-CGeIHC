import bpy,bmesh,os,json,math
from mathutils import Vector,Matrix
base=os.path.dirname(os.path.abspath(__file__))
out=os.path.join(base,'export');os.makedirs(out,exist_ok=True)
report={}

def active(o):
 bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
def components(o):
 active(o);bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.separate(type='LOOSE');bpy.ops.object.mode_set(mode='OBJECT')
 return list(bpy.context.selected_objects)
def bounds(o):
 return [Vector([min(v.co[i] for v in o.data.vertices) for i in range(3)]),Vector([max(v.co[i] for v in o.data.vertices) for i in range(3)])]
def center_at(o,center):
 lo,hi=bounds(o);shift=Vector(center)-(lo+hi)*.5
 for v in o.data.vertices:v.co+=shift
def material(name,color):
 m=bpy.data.materials.new(name);m.diffuse_color=(*color,1);m.use_nodes=True
 bs=m.node_tree.nodes.get('Principled BSDF');bs.inputs['Base Color'].default_value=(*color,1);bs.inputs['Roughness'].default_value=.8
 return m
def assign(o,m):
 o.data.materials.clear();o.data.materials.append(m)
 for p in o.data.polygons:p.material_index=0
def count(obs):
 total=0
 for o in obs:o.data.calc_loop_triangles();total+=len(o.data.loop_triangles)
 return total
def join(obs,name):
 if len(obs)==1:obs[0].name=name;return obs[0]
 bpy.ops.object.select_all(action='DESELECT')
 for o in obs:o.select_set(True)
 bpy.context.view_layer.objects.active=obs[0];bpy.ops.object.join();obs[0].name=name
 return obs[0]
def finish(name,parts,height):
 low=min(v.co.z for o in parts for v in o.data.vertices);high=max(v.co.z for o in parts for v in o.data.vertices)
 factor=height/(high-low)
 for o in parts:
  for v in o.data.vertices:v.co=(v.co-Vector((0,0,low)))*factor
  active(o)
  bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.00001);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(o.data);bm.free()
  o.data.update()
 bpy.context.scene.render.engine='BLENDER_WORKBENCH';scene=bpy.context.scene
 scene.display.shading.light='STUDIO';scene.display.shading.color_type='TEXTURE';scene.display.shading.show_cavity=True
 scene.display.shading.background_type='VIEWPORT';scene.display.shading.background_color=(.12,.12,.12)
 bpy.ops.object.camera_add(location=(height*.7,-height*2,height*.75));cam=bpy.context.object
 cam.rotation_euler=(Vector((0,0,height*.5))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=height*1.35;scene.camera=cam
 scene.render.resolution_x=700;scene.render.resolution_y=700;scene.render.resolution_percentage=100
 scene.render.filepath=os.path.join(base,name+'_preparado.png');bpy.ops.render.render(write_still=True)
 report[name]['after']=count(parts);report[name]['parts']=[o.name for o in parts]
 for o in parts:
  active(o);bpy.ops.wm.obj_export(filepath=os.path.join(out,o.name+'.obj'),export_selected_objects=True,forward_axis='NEGATIVE_Z',up_axis='Y',export_materials=True,export_triangulated_mesh=True,path_mode='STRIP')
 # Mantener las piezas editables y organizadas bajo un padre real en Blender.
 root=bpy.data.objects.new(name+'_raiz',None);scene.collection.objects.link(root)
 pivots={}
 for i,o in enumerate(parts):
  lo,hi=bounds(o);pivot=(lo+hi)*.5
  if i==0:pivot=Vector((0,0,0))
  elif 'pierna' in o.name or 'brazo' in o.name:pivot.z=hi.z-.025
  pivots[o.name]=[round(pivot.x,5),round(pivot.z,5),round(-pivot.y,5)]
  for v in o.data.vertices:v.co-=pivot
  o.location=pivot;o.parent=root if i==0 else parts[0]
 report[name]['pivots']=pivots
 bpy.ops.wm.save_as_mainfile(filepath=os.path.join(base,name+'_Ooo.blend'))

bpy.ops.wm.open_mainfile(filepath=os.path.join(base,'Jake_original.blend'))
o=next(o for o in bpy.context.scene.objects if o.type=='MESH');report['Jake']={'before':count([o])}
for v in o.data.vertices:v.co=Matrix.Rotation(-math.pi/2,4,'Z')@o.matrix_world@v.co
o.matrix_world=Matrix.Identity(4)
im=bpy.data.images.load(os.path.join(base,'Jake_BaseColor.png'));im.pack()
m=material('Jake_color',(1,1,1));tex=m.node_tree.nodes.new('ShaderNodeTexImage');tex.image=im;m.node_tree.links.new(tex.outputs['Color'],m.node_tree.nodes.get('Principled BSDF').inputs['Base Color']);assign(o,m)
pieces=components(o);body=[];face=[];legs=[]
for p in pieces:
 lo,hi=bounds(p)
 if hi.z<1.4:legs.append(p)
 elif len(p.data.vertices)>2000:body.append(p)
 else:face.append(p)
body=join(body,'Jake_cuerpo');face=join(face,'Jake_cara')
legs.sort(key=lambda p:(sum(v.co.x for v in p.data.vertices)/len(p.data.vertices)))
legs[0].name='Jake_pierna_izq';legs[1].name='Jake_pierna_der'
parts=[body,face]+legs
for p in parts:
 active(p);mod=p.modifiers.new('Reducir_sin_perder_silueta','DECIMATE');mod.ratio=.68 if p==body else .78;bpy.ops.object.modifier_apply(modifier=mod.name)
finish('Jake',parts,2.5)

bpy.ops.wm.open_mainfile(filepath=os.path.join(base,'Bmo_original.blend'))
objects={o.name:o for o in bpy.context.scene.objects if o.type=='MESH'};report['Bmo']={'before':count(objects.values())}
for o in objects.values():
 o.parent=None;o.matrix_parent_inverse=Matrix.Identity(4)
 o.animation_data_clear();o.delta_location=(0,0,0);o.delta_rotation_euler=(0,0,0);o.delta_scale=(1,1,1)
 o.matrix_world=Matrix.Identity(4)
palette={'plastico':(.12,.55,.42),'metal':(.10,.43,.34),'pantalla':(.60,.88,.69),'negro':(.015,.025,.025),'Plastic Red 9':(.12,.55,.42)}
for m in bpy.data.materials:
 color=palette.get(m.name,(.12,.55,.42));m.diffuse_color=(*color,1);m.use_nodes=True;m.node_tree.nodes.clear()
 bs=m.node_tree.nodes.new('ShaderNodeBsdfPrincipled');bs.inputs['Base Color'].default_value=(*color,1);bs.inputs['Roughness'].default_value=.8
 output=m.node_tree.nodes.new('ShaderNodeOutputMaterial');m.node_tree.links.new(bs.outputs['BSDF'],output.inputs['Surface'])
body=objects['cuerpo'];body.name='Bmo_cuerpo';center_at(body,(0,0,4.04))
arms=components(objects['brazos']);legs=components(objects['piernas'])
arms.sort(key=lambda p:bounds(p)[0].x);legs.sort(key=lambda p:bounds(p)[0].x)
green=material('Bmo_verde',(.12,.55,.42));black=material('Bmo_negro',(.015,.025,.025))
for i,p in enumerate(arms):p.name='Bmo_brazo_'+('izq' if i==0 else 'der');center_at(p,((-1 if i==0 else 1)*1.61,0,3.65));assign(p,green)
for i,p in enumerate(legs):p.name='Bmo_pierna_'+('izq' if i==0 else 'der');center_at(p,((-1 if i==0 else 1)*.79,0,1.10));assign(p,green)
for key,x,z,col in [('boton',.76,3.35,(.05,.22,.85)),('boton2',.64,2.65,(.9,.08,.06)),('boton3',.15,3.12,(.03,.67,.83)),('cruceta',-.75,2.85,(.96,.75,.05))]:
 p=objects[key]
 # Botones originales planos en XY; orientarlos hacia el frente (-Y).
 for v in p.data.vertices:v.co=Matrix.Rotation(math.pi/2,4,'X')@v.co
 center_at(p,(x,-1.04,z));assign(p,material('Bmo_'+key,col))
# Las letras venian fuera de sitio: conservar sus mallas para editar, no tapar la cara.
bpy.data.objects.remove(objects['letras'],do_unlink=True)
buttons=join([objects[k] for k in ['boton','boton2','boton3','cruceta']],'Bmo_botones')
# La pantalla del FBX no contiene rostro. Dos ojos y una sonrisa de geometria ligera.
face=[]
for x in [-.48,.48]:
 bpy.ops.mesh.primitive_uv_sphere_add(segments=12,ring_count=6,location=(x,-1.04,5.25))
 p=bpy.context.object;p.scale=(.085,.025,.11);active(p);bpy.ops.object.transform_apply(location=True,rotation=True,scale=True);assign(p,black);face.append(p)
curve=bpy.data.curves.new('sonrisa','CURVE');curve.dimensions='3D';curve.bevel_depth=.035;curve.bevel_resolution=1
s=curve.splines.new('POLY');s.points.add(12)
for i,pt in enumerate(s.points):
 t=i/12;pt.co=(-.35+.7*t,-1.045,4.92-.18*math.sin(math.pi*t),1)
smile=bpy.data.objects.new('sonrisa',curve);bpy.context.collection.objects.link(smile);active(smile);bpy.ops.object.convert(target='MESH');assign(smile,black);face.append(smile)
face=join(face,'Bmo_cara')
parts=[body,buttons,face]+arms+legs
finish('Bmo',parts,1.7)
json.dump(report,open(os.path.join(base,'optimizacion.json'),'w'),indent=2)
