clearListener()
gc()

filein "DPUtils.ms"
filein "DPComponents.ms"

struct TMaterialExporter (
	
	fs = TJsonFormatter(), 
	project_path,                 -- Asigned in the ctor 
	base_path,                 -- Asigned in the ctor 	
	
	fn exportMap map alias = (
		
		local map_filename = "default_texture"
		if map != undefined then (
			map_filename = map.filename
		)
		local base_name = getFilenameFile map_filename
		local json_filename = "data/textures/" + base_name + ".dds"
		local ofull_path = project_path + json_filename

		-- Check if ofull_path exists
		if not doesFileExist ofull_path then (
			format "We should copy from % to %\n" map_filename ofull_path
			copyFile map_filename ofull_path
		)
		
		fs.writeKeyValue alias json_filename
	),
	
		-- Exports a single std material to a json format
	fn exportMixMaterial mat mat_name base_name tech sdw = (
		
		exported_materials = #()
		local mat_name1 = base_name + mat.map1.name + ".material"
		local mat_name2 = base_name + mat.map2.name + ".material"
		append exported_materials mat_name1
		append exported_materials mat_name2
		
		format "Exporting mixed material % % %\n" mat_name mat (classof mat as string)
		fs.begin (project_path + mat_name )
		fs.beginObj()
			if tech != undefined then ( 
				fs.writeKeyValue "technique" tech 
				fs.writeComma()
			)
			
			fs.writeKeyValue "shadows" sdw
			fs.writeComma()
			fs.writeKey "textures" 
			fs.beginObj()
			fs.endObj()
			fs.writeComma()
			fs.writeKey "mats"
			fs.arrayOfStrings exported_materials
			fs.writeComma()
			exportMap mat.mask "mix_blend_weights"
			
		fs.endObj()
		fs.end()
	),
	
	-- Exports a single std material to a json format
	fn exportStdMaterial mat mat_name tech sdw = (
		
		format "Exporting mixing material % % %\n" mat_name mat (classof mat as string)
		fs.begin (project_path + mat_name )
		fs.beginObj()
			if tech != undefined then ( 
				fs.writeKeyValue "technique" tech 
				fs.writeComma()
			)
			fs.writeKeyValue "shadows" sdw
			fs.writeComma()
			fs.writeKey "textures" 
			fs.beginObj()
				
			if classof mat == Standardmaterial then (
				exportMap mat.diffusemap "albedo"		
				if mat.ambientMap != undefined then (
					fs.writeComma()
					exportMap mat.ambientMap "lightmap"
				)			
				if mat.specularMap != undefined then (
					fs.writeComma()
					exportMap mat.specularMap "metallic"
				)			
				if mat.glossinessMap != undefined then (
					fs.writeComma()
					exportMap mat.glossinessMap "roughness"
				)	
				
				if mat.bumpMap != undefined then (
					fs.writeComma()
					exportMap mat.bumpMap "normal"
				)
				else (
					fs.writeComma()
					fs.writeKeyValue "normal" "data/textures/null_normal.dds"
				)
				
				if mat.displacementMap != undefined then (
					fs.writeComma()
					exportMap mat.displacementMap "height"
				)				
				if mat.selfIllumMap != undefined then (
					fs.writeComma()
					exportMap mat.selfIllumMap "emissive"
				)
				if mat.refractionMap != undefined then (
					fs.writeComma()
					exportMap mat.refractionMap "aocclusion"
				)
			)
		
			fs.endObj()
		fs.endObj()
		fs.end()
	),
	
	-- Will return an array of all the materials names used by obj and exported by us
	fn exportMaterial mat base_name obj tech sdw = (
		
		local exported_materials = #()
		
		if classof mat == StandardMaterial then (
			local mat_name = base_name + mat.name + ".material"
			append exported_materials mat_name
			exportStdMaterial mat mat_name tech sdw
			
		) else if classof mat == MultiMaterial then (
			local multi_mat = mat
			
			local materials_of_mesh = getMaterialsUsedByMesh obj
			for mat_idx = 1 to materials_of_mesh.count do (
				if materials_of_mesh[ mat_idx ] == undefined then continue
				local mat_of_mesh = multi_mat[ mat_idx ]
				
				if mat_of_mesh == undefined then throw ("Mesh " + obj.name + " is using a multimaterial in slot " + (mat_idx as string)+ " but the multimat does not have this submat")
				if classof mat_of_mesh != StandardMaterial then throw ("Mesh " + obj.name + " is using a multimaterial in slot " + (mat_idx as string) + " but the multimat in this slot is not a stdMaterial")
				
				local mat_name = base_name + mat_of_mesh.name + ".material"
				append exported_materials mat_name
				exportStdMaterial mat_of_mesh mat_name tech sdw
			)
		) else if classof mat == Blend then (
			local mat_name = base_name + mat.name + ".material"
			local mat_name1 = base_name + mat.map1.name + ".material"
			local mat_name2 = base_name + mat.map2.name + ".material"
			append exported_materials mat_name
			exportStdMaterial mat.map1 mat_name1 tech sdw
			exportStdMaterial mat.map2 mat_name2 tech sdw
			exportMixMaterial mat mat_name base_name "pbr_mix.tech"
		)
		
		return exported_materials
	)
)

struct TSceneExporter (

	fs = TJsonFormatter(), 
	project_path = "",
	base_path = "data/",
	scenes_path = base_path + "scenes/",
	prefabs_path = base_path + "prefabs/",
	mesh_path = base_path + "meshes/",
	mats_path = base_path + "materials/",
	cols_path = base_path + "colliders/",
	tex_path = base_path + "textures/",
	curve_path = base_path + "curves/",
	local_path = "",
	use_instancing = false,
	is_prefab = false,
		is_scene = true,
	nitems = 0,
	
	fn exportName obj = (
		fs.writeKeyValue "name" obj.name
	),
	
	fn exportAABB obj = (
		fs.writeComma()
		fs.writeKeyValue "abs_aabb" true
		fs.writeComma()
		fs.writeKeyValue "local_aabb" true
	),
	
	function writeComponents obj = (
		
		-- Prepare comps to be serialized.
		writeTagComponent obj
		writeColliderComponent obj
	),

	function writeTagComponent obj = (
		
		c_values = retrieveComponent obj component_tag

		if  c_values != undefined then (
			
			local final_tags = #()
			local total_tags = #(c_values.tag1, c_values.tag2, c_values.tag3, c_values.tag4)
			for tag in total_tags do (
				if tag != "" then ( append final_Tags tag )
			)
			
			total_tags
			fs.writeComma()
			fs.writeKey "tags"
			fs.arrayOfStrings final_tags
		)

	),

	function writeColliderComponent obj = (
		
		c_values = retrieveComponent obj component_collider
		
		if  c_values != undefined then (
			
			fs.writeComma()
			fs.writeKeyValue "group" c_values.col_group
			fs.writeComma()
			fs.writeKeyValue "mask" c_values.col_mask
			fs.writeComma()
			fs.writeKeyValue "is_trigger" c_values.is_trigger
			fs.writeComma()
			fs.writeKeyValue "is_dynamic" c_values.is_dynamic
			fs.writeComma()
			fs.writeKeyValue "is_controller" c_values.is_controller			
			fs.writeComma()
			fs.writeKeyValue "is_gravity" c_values.is_gravity	
			
		)
	),
	
	function writeHierComponent obj = (
				
		if obj.parent != undefined then (
			fs.writeKey "hierarchy" 
			fs.beginObj()
			fs.writeKeyValue "parent" obj.parent.name
			if obj.parent != undefined then (
				tm = obj.transform * (inverse obj.parent.transform)
				
				local max2mcv = rotateXMatrix -90
				local mcv2max = rotateXMatrix 90
				local mcv_position = obj.pos * max2mcv

				local mcv_transform = mcv2max * obj.transform * max2mcv
				local mcv_quat = mcv_transform.rotationPart as quat
				local mcv_final_position = checkPoint mcv_position
				fs.writeComma()
				fs.writeKeyValue "pos" mcv_final_position
				fs.writeComma()
				fs.writeKeyValue "rotation" mcv_quat
			)
			fs.endObj()
			fs.writeComma()
		)
	),
	
	function writePrefabComponent obj = (
		
		if classof obj == xrefobject then (
			local f_name = getFilenameFile obj.filename 
			local f_path = (prefabs_path + local_path + f_name + ".prefab")--(filterString (prefabs_path + obj.filename) ".max")
			fs.writeKeyValue "prefab" f_path
			fs.writeComma()
		)
	),
	
	function writeGroupComponent obj = (
		
		if obj.parent != undefined then (
			local is_group = isgrouphead obj.parent
			if is_group then (
				fs.writeKey "group" 
				fs.beginObj()
				fs.writeComma()
			)
		)
		
	),

	-- ----------------------------------------------------------
	fn exportTransform obj = (
		fs.writeKey "transform" 
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.pos * max2mcv
		print obj.pos
		local mcv_transform = mcv2max * obj.transform * max2mcv
		local mcv_quat = mcv_transform.rotationPart as quat
		local mcv_final_position = checkPoint mcv_position
		fs.beginObj()
			fs.writeKeyValue "pos" mcv_final_position
			fs.writeComma()
			fs.writeKeyValue "rotation" mcv_quat
		fs.endObj()
	),
	
	fn exportTransformDecal obj = (
		fs.writeKey "transform" 
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.pos * max2mcv
		print obj.pos
		local mcv_transform = mcv2max * obj.transform * max2mcv
		local mcv_quat = mcv_transform.rotationPart as quat
		local mcv_final_position = checkPoint mcv_position
		fs.beginObj()
			fs.writeKeyValue "pos" mcv_final_position
			fs.writeComma()
			fs.writeKeyValue "rotation" mcv_quat
			fs.writeComma()
			fs.writeKeyValue "scale" [obj.width, 0.1, obj.length]
		fs.endObj()
	),
	
	fn exportTransformNormal obj = (
		fs.writeKey "transform" 
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.pos * max2mcv
		print obj.pos
		local mcv_transform = obj.transform * max2mcv
		local mcv_quat = mcv_transform.rotationPart as quat
		local mcv_final_position = checkPoint mcv_position
		fs.beginObj()
			fs.writeKeyValue "pos" mcv_final_position
			fs.writeComma()
			fs.writeKeyValue "rotation" mcv_quat
			fs.writeComma()
			fs.writeKeyValue "scale" [obj.width, obj.length, 1]
		fs.endObj()
	),
	
		-- ----------------------------------------------------------
	fn exportTransformPrefab obj = (
		fs.writeKey "transform" 
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.pos * max2mcv
		local mcv_transform = mcv2max * obj.transform * max2mcv
		local mcv_quat = mcv_transform.rotationPart as quat
		local mcv_final_position = checkPoint mcv_position
		fs.beginObj()
			fs.writeKeyValue "pos" mcv_final_position
		fs.endObj()
	),
	
	fn exportTransformLookAt obj = (
		local target = getnodebyname (obj.name + ".Target")
		if target == null then (return exportTransform obj)
		
		fs.writeKey "transform" 
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.position * max2mcv
		local mcv_target = target.position * max2mcv
		
		local mcv_final_position = checkPoint mcv_position
		local mcv_final_target = checkPoint mcv_target
		
		fs.beginObj()
			fs.writeKeyValue "pos" mcv_final_position
			fs.writeComma()
			fs.writeKeyValue "lookat" mcv_final_target
		fs.endObj()
	),
	
	-- ----------------------------------------------------------
	fn exportRenderMesh obj = (
		
		local f_mats_path = mats_path + local_path
		local exported_materials = #(f_mats_path + "mtl_default.material")
		local mat = obj.material
		
		-- Do some material safe checks.
		if mat == undefined then (
			print ("Obj " + obj.name + " does NOT have a material")
		)
		if mat != undefined and not (isValidName mat.name) then (
			messageBox ("Wrong material name: " + mat.name) title:"Warning Message"
		)
		
		fs.beginObj()
		(
			-- Export Mesh
			local mesh_name = mesh_path + local_path + obj.name + ".mesh"
			fs.writeKeyValue "mesh" mesh_name
			exportMeshObject obj (project_path + mesh_name )
			fs.writeComma()
		
			if mat != undefined then (
				local me = TMaterialExporter project_path:project_path base_path:base_path
				exported_materials = me.exportMaterial mat f_mats_path obj undefined true
			)
			
			fs.writeKey "materials"
			fs.arrayOfStrings exported_materials
			
		)
		fs.endObj()	
	),
	
	function exportInstancedMesh mesh_path mesh_name = (
			
		local ifs = TJsonFormatter()
		ifs.begin (project_path + mesh_path)
		ifs.beginObj()
			ifs.writeKeyValue "instanced_mesh" mesh_name
			ifs.writeComma()
			ifs.writeKeyValue "instances_type" "Instance"
			ifs.writeComma()
			ifs.writeKeyValue "num_instances_reserved" 8
		ifs.endObj()
		ifs.end()
	),

		-- ----------------------------------------------------------
	fn exportRenderInstance obj = (
		
		local f_mats_path = mats_path + local_path
		local exported_materials = #(f_mats_path + "mtl_instanced_default.material")
		local mat = obj.material
		
		-- Do some material safe checks.
		if mat == undefined then (
			print ("Obj " + obj.name + " does NOT have a material")
		)
		if mat != undefined and not (isValidName mat.name) then (
			messageBox ("Wrong material name: " + mat.name) title:"Warning Message"
		)
		
		-- First, export the instanced mesh file
		local mesh_name = mesh_path + local_path + obj.name + ".mesh"
		local imesh_name = mesh_path + local_path + obj.name + ".instanced_mesh"
		exportInstancedMesh imesh_name mesh_name
		
		-- Export instanced Mesh
		fs.writeKeyValue "mesh" imesh_name
		exportMeshObject obj (project_path + mesh_name )
		fs.writeComma()
				
		if mat != undefined then (
			local me = TMaterialExporter project_path:project_path base_path:base_path
			exported_materials = me.exportMaterial mat f_mats_path obj "pbr_instanced.tech" true
		)
		
		fs.writeKey "materials"
		fs.arrayOfStrings exported_materials
		fs.endObj()
	),
	
	fn exportDecalInstance obj = (
		
		local decal_name = "instanced_default"
		local f_mats_path = mats_path + local_path
		local exported_materials = #(f_mats_path + "mtl_instanced_default.material")
		local mat = obj.material
		
		-- Do some material safe checks.
		if mat != undefined then (
			decal_name = substituteString mat.name "mtl_" ""
		)
		
		-- First, export the instanced mesh file
		local mesh_name = "data/meshes/UnitBox.mesh"
		local imesh_name = mesh_path + local_path + decal_name + ".instanced_mesh"
		exportInstancedMesh imesh_name mesh_name
		
		-- Export instanced Mesh
		fs.writeKey "instance" 
		fs.beginObj()
		fs.writeKeyValue "mesh" imesh_name
		fs.writeComma()
				
		if mat != undefined then (
			local me = TMaterialExporter project_path:project_path base_path:base_path
			exported_materials = me.exportMaterial mat f_mats_path obj "pbr_decals.tech" false
		)
		
		fs.writeKey "materials"
		fs.arrayOfStrings exported_materials
		fs.endObj()
	),
	
	-- ----------------------------------------------------------
	fn exportCompCollider obj = (
		
		fs.writeKey "collider" 
		fs.beginObj()

		if superclassof obj == light then ( -- to temp bypass light triggers
			fs.writeKeyValue "shape" "sphere" 
			fs.writeComma()
			fs.writeKeyValue "radius" 5
			fs.writeComma()
			fs.writeKeyValue "trigger" true
		)
		else if classof obj == Sphere  then (
			fs.writeKeyValue "shape" "sphere" 
			fs.writeComma()
			fs.writeKeyValue "radius" obj.radius
		) else if classof obj == Box then (
			fs.writeKeyValue "shape" "box" 
			fs.writeComma()
			fs.writeKeyValue "size" [abs (obj.width * .5f), abs (obj.height * .5f), abs (obj.length * .5f)]
		) else (
			local col_path = project_path + local_path + cols_path + obj.name + ".collider"
			
			if obj.parent != undefined then (
				if obj.mesh.verts.count == obj.parent.mesh.verts.count then (
					-- We are facing, wether convex or mesh collider.
					fs.writeKeyValue "shape" "mesh" 
					fs.writeComma()
					fs.writeKeyValue "name" (cols_path + obj.name + ".collider")
				) else (
					fs.writeKeyValue "shape" "mesh" 
					fs.writeComma()
					fs.writeKeyValue "name" (cols_path + obj.name + ".collider")
				)
			) else  (
				fs.writeKeyValue "shape" "mesh" 
				fs.writeComma()
				fs.writeKeyValue "name" (cols_path + obj.name + ".collider")
			)
			
			exportColliderObject obj col_path false
		)
		
		writeColliderComponent obj
		fs.endObj()
	),
	
	-- ----------------------------------------------------------	
	fn exportCompLight obj = (

		if classof obj == TargetDirectionalLight then (
			local target = getnodebyname (obj.name + ".Target")
			fs.writeKey "light_dir" 
			fs.beginObj()
			fs.writeKeyValue "type" "directional"
			fs.writeComma()
			fs.writeKeyValue "color" [obj.rgb.r/255, obj.rgb.g/255, obj.rgb.b/255, obj.rgb.a/255]
			fs.writeComma()
			fs.writeKeyValue "intensity" obj.multiplier 
			fs.writeComma()
			fs.writeKeyValue "proj_type" "ortographic"
			fs.writeComma()
			fs.writeKeyValue "z_near" 0.01
			fs.writeComma()	
			fs.writeKeyValue "z_far" (distance target.pos obj.pos)
			fs.writeComma()	
			fs.writeKeyValue "ortosize" obj.falloff
			fs.writeComma()
			fs.writeKeyValue "casts_shadows" obj.castShadows 
			fs.writeComma()
			fs.writeKeyValue "shadows_step" 1.5
			fs.writeComma()
			fs.writeKeyValue "shadows_resolution" 4096
			fs.writeComma()
			fs.writeKeyValue "shadows_fmt" "R32_TYPELESS"
			fs.writeComma()
			fs.writeKeyValue "volume" false
			fs.endObj()			
			return true
		)
		else if classof obj == targetSpot or classof obj == freeSpot then ( 
			local target = getnodebyname (obj.name + ".Target")
			fs.writeKey "light_spot" 
			fs.beginObj()
			fs.writeKeyValue "type" "spotlight"
			fs.writeComma()
			fs.writeKeyValue "proj_type" "perspective"
			fs.writeComma()
			fs.writeKeyValue "range" (distance target.pos obj.pos)
			fs.writeComma()
			fs.writeKeyValue "angle" obj.falloff
			fs.writeComma()
			fs.writeKeyValue "inner_cut" obj.hotspot
			fs.writeComma()
			fs.writeKeyValue "z_far" (distance target.pos obj.pos)
						fs.writeComma()
			fs.writeKeyValue "volume" false
			fs.writeComma()	
		)
		else if classof obj == Omnilight   then (
			fs.writeKey "light_point" 
			fs.beginObj()
			fs.writeKeyValue "type" "pointlight"
			fs.writeComma()
			fs.writeKeyValue "inner_cut" obj.farAttenStart
			fs.writeComma()
			fs.writeKeyValue "outer_cut" obj.farAttenEnd 
			fs.writeComma()
		)
		else if classof obj == skylight   then (
			fs.writeKey "light_ambient" 
			fs.beginObj()
			fs.writeKeyValue "type" "ambient"
			fs.writeComma()
		)

		fs.writeKeyValue "color" [obj.rgb.r/255, obj.rgb.g/255, obj.rgb.b/255, obj.rgb.a/255]
		fs.writeComma()
		fs.writeKeyValue "intensity" obj.multiplier 
		fs.writeComma()
		fs.writeKeyValue "casts_shadows" obj.castShadows 
		fs.writeComma()
		fs.writeKeyValue "shadows_step" 1.0
		fs.writeComma()
		fs.writeKeyValue "shadows_resolution" 512
		fs.writeComma()
		fs.writeKeyValue "shadows_fmt" "R32_TYPELESS"
		fs.endObj()
	),

	-- ----------------------------------------------------------
	fn exportEditableMesh obj = (
		
		exportName obj 
		fs.writeComma()
		writeHierComponent obj
		writeGroupComponent obj
		
		if is_prefab == true then (
			exportTransformPrefab obj
		)
		else (
			exportTransform obj
		)
		
	
		if use_instancing == true then (
			fs.writeComma()
			fs.writeKey "instance" 
			fs.beginObj()
			exportRenderInstance obj
			fs.endObj()			
		) else (
			fs.writeComma()
			fs.writeKey "render" 
			fs.beginArray()
			exportRenderMesh obj
			fs.endArray()
		)
	
		
		for child in obj.children do (
			if isCollider child then (
				fs.writeComma()
				exportCompCollider child
			)
		)
		
		writeTagComponent obj
		exportAABB obj
	),
	
	-- Automatic light collider generation depending on light type
	fn createLightCollider obj = (
		
		local col_path = project_path + cols_path + local_path + obj.name + ".collider"
		if classof obj == targetSpot or classof obj == freeSpot  then ( 
			local dist = obj.targetDistance
			if dist == undefined then ( dist = obj.baseObject.targetDistance )
			local int_radius = tan(obj.falloff * .5f) * dist
			local totalsides = clamp 12 32 ((int_radius * .25f) as integer)
			local collider = cone radius1:int_radius radius2:0 height:obj.nearAttenEnd heightsegs:1 sides:totalsides
			collider.transform = obj.transform
			collider.position = (obj.position - obj.dir * obj.nearAttenEnd)
			collider = convertToMesh collider
			collider.pivot = obj.position
			
			fs.writeKey "collider" 
			fs.beginObj()
			fs.writeKeyValue "shape" "convex" 
			fs.writeComma()
			fs.writeKeyValue "name" (cols_path + obj.name + ".collider")
			fs.writeComma()
			fs.writeKeyValue "is_trigger" true
			fs.endObj()
			fs.writeComma()
			
			exportColliderObject collider col_path true
			delete collider
		)
		else if classof obj == omnilight then ( 
			fs.writeKey "collider" 
			fs.beginObj()
			fs.writeKeyValue "shape" "sphere"
			fs.writeComma()
			fs.writeComma()
			fs.writeKeyValue "radius" obj.nearAttenEnd
			fs.writeComma()
			fs.writeKeyValue "is_trigger" true
			fs.endObj()
			fs.writeComma()
		)
		
	),
	
	-- Export a light entity
	fn exportEntityLight obj = (
		
		-- It will be exported as part of the parent. Ejemplo del cofre y su caja de colision.
		--if obj.parent != undefined then return false
		exportName obj 
		fs.writeComma()
		exportTransformLookAt obj
		fs.writeComma()
		exportCompLight obj
		exportAABB obj
		fs.writeComma()
		createLightCollider obj
		fs.writeKey "tags"
		fs.arrayOfStrings #("light")
		
	),
	
	-- Export an entity as a single individual collider
	fn exportEntityCollider obj = (
		
		-- It will be exported as part of the parent. Ejemplo del cofre y su caja de colision.
		if obj.parent != undefined then return false
		exportName obj 
		fs.writeComma()
		exportTransform obj
		fs.writeComma()
		exportCompCollider obj
	),
	
	-- Export an entity which is an xref object as a prefab
	fn exportEntityPrefab obj = (
		
		-- We must ignore those which are 
		if obj.parent != undefined then return false
		obj.transform.controller = prs()
		
		if (nitems > 0) then fs.writeComma()
				
		fs.beginObj()
		fs.writeKey "entity"
			fs.beginObj()
			exportName obj 
			fs.writeComma()
			writePrefabComponent obj
			exportTransform obj
			fs.endObj()
		fs.endObj()
	),
	
	-- Export an entity which is an xref object as a prefab
	fn exportEntityDecal obj = (
		
		-- We must ignore those which are 
		if (nitems > 0) then fs.writeComma()
				
		fs.beginObj()
		fs.writeKey "entity"
			fs.beginObj()
			exportName obj 
			fs.writeComma()
			exportTransformDecal obj
			fs.writeComma()
			exportDecalInstance obj
			exportAABB obj
			fs.endObj()
		fs.endObj()
	),
	
		-- Export an entity which is an xref object as a prefab
	fn exportEntityGrass obj = (
		
		-- We must ignore those which are 
		if (nitems > 0) then fs.writeComma()
		
		local exported_mesh_path = mesh_path + "grass.instanced_mesh"
		local exported_materials = #(mats_path + "grass_instanced.material")
				
		fs.beginObj()
		fs.writeKey "entity"
			fs.beginObj()
			exportName obj 
			fs.writeComma()
			exportTransformNormal obj
			fs.writeComma()
			fs.writeKey "instance" 
			fs.beginObj()
			fs.writeKeyValue "mesh" exported_mesh_path
			fs.writeComma()
			fs.writeKey "materials"
			fs.arrayOfStrings exported_materials
			fs.endObj()
			fs.endObj()
		fs.endObj()
	),
	
	-- ----------------------------------------------------------
	fn exportEntity obj = (
		
		-- Adding general discarding here
		if classof obj == Targetobject then (return false )
		if isCollider obj and  obj.parent != undefined then (return false )
		if classof obj == QuadPatch then (return exportEntityDecal obj)
		if classof obj == TriPatch then (return exportEntityGrass obj)
		if classof obj == xrefobject then (return exportEntityPrefab obj)
				
		if (nitems > 0) then fs.writeComma()
		
		fs.beginObj()
		fs.writeKey "entity"
			fs.beginObj()
			if isCollider obj then ( exportEntityCollider obj)
			else if isLight obj then ( exportEntityLight obj)
			else exportEditableMesh obj
			fs.endObj()
		fs.endObj()
		
	),
	
	fn exportRoot filename = (
		
		fs.beginObj()
		fs.writeKey "entity"
			fs.beginObj()
			fs.writeKeyValue "name" filename
			fs.writeComma()
			fs.writeKey "transform" 
			fs.beginObj()
			fs.writeKeyValue "pos" "0.0 0.0 0.0"
			fs.endObj()
		fs.endObj()
		fs.endObj()
		fs.writeComma()
	),
	
	-- ----------------------------------------------------------
	-- ----------------------------------------------------------
	
	fn exportAll nodes ofilename prg l_is_scene = (
		
		is_scene = l_is_scene;
		is_prefab =  not is_scene;
		
		-------------------------------------------------------------------------
		local req_path = findstring project_path "\bin"
		if req_path == undefined then (
			messageBox "Path is not located at /bin.\nPlease, provide a correct path."
			return false				
		)
		-------------------------------------------------------------------------
		local_path = getFilenamePath ofilename
		-- Create the folders if they doesn't exist
		doesFolderExist (project_path + base_path) create:true
		doesFolderExist (project_path + scenes_path + local_path) create:true
		doesFolderExist (project_path + prefabs_path + local_path) create:true
		doesFolderExist (project_path + mesh_path + local_path) create:true
		doesFolderExist (project_path + mats_path + local_path ) create:true
		doesFolderExist (project_path + cols_path + local_path) create:true
		doesFolderExist (project_path + tex_path + local_path) create:true
		doesFolderExist (project_path + curve_path + local_path) create:true
		
		-- Decide output filename based on .max filename
		local full_path = project_path + prefabs_path + ofilename + ".prefab"
		if is_scene == true then ( 
			local full_path = project_path + scenes_path + ofilename + ".scene"
		)
		
		-- Begin to write
		fs.begin full_path
		fs.beginArray()
		
		-- Write the root node if it is a prefab
		--if is_scene == true then (
			exportRoot ofilename -- Export root
			format "Exporting to %\n" full_path
		--)
		
		nitems = 0
		local prg_step = 100 / (nodes.count + 1)
		
		for obj in nodes do (
			
			exportEntity obj
			nitems = nitems + 1
			
			prg.value = (nitems + 1) * prg_step
		)
		
		prg.value = 100
		fs.endArray()
		fs.end()	
		
		messageBox ("Scene SUCCESFULLY exported at location: \n\n" + full_path)		
	)
)




