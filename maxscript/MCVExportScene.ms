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
		print ofull_path
		-- Check if ofull_path exists
		if not doesFileExist ofull_path then (
			format "We should copy from % to %\n" map_filename ofull_path
			copyFile map_filename ofull_path
		)
		
		fs.writeKeyValue alias json_filename
	),
	
	-- Exports a single std material to a json format
	fn exportStdMaterial mat mat_name = (
		
		format "Exporting material % % %\n" mat_name mat (classof mat as string)
		fs.begin (project_path + mat_name )
		fs.beginObj()
			fs.writeKeyValue "technique" "textured_bk.tech"
			fs.writeComma()
			fs.writeKeyValue "shadows" true
			fs.writeComma()
			fs.writeKey "textures" 
			fs.beginObj()
				
			if classof mat == Standardmaterial then (
				exportMap mat.diffusemap "albedo"
				fs.writeComma()
				exportMap mat.selfIllumMap "lightmap"
				-- ... other maps..
			)
		
			fs.endObj()
		fs.endObj()
		fs.end()
	),
	
	-- Will return an array of all the materials names used by obj and exported by us
	fn exportMaterial mat base_name obj = (
		
		local exported_materials = #()
		
		if classof mat == StandardMaterial then (
			local mat_name = base_name + mat.name + ".material"
			append exported_materials mat_name
			exportStdMaterial mat mat_name
			
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
				exportStdMaterial mat_of_mesh mat_name
			)
		)
		
		return exported_materials
	)
)

struct TSceneExporter (

	fs = TJsonFormatter(), 
	project_path = "",
	base_path = "data/",
	scenes_path = base_path + "scenes/",
	mesh_path = base_path + "meshes/",
	mats_path = base_path + "materials/",
	cols_path = base_path + "colliders/",
	tex_path = base_path + "textures/",
	
	fn exportName obj = (
		fs.writeKeyValue "name" obj.name
	),
	
	
	fn isValidName aname = (
		return findString aname " " == undefined 
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
			fs.endObj()
		)
	),
	
	function writeGroupComponent obj = (
		
		local is_group = isgrouphead obj.parent
		if is_group then (
			fs.writeKey "group" 
			fs.beginObj()
		
		)
	),
	
	-- ----------------------------------------------------------
	fn exportTransform obj = (
		fs.writeKey "transform" 
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.position * max2mcv
		
		local mcv_transform = mcv2max * obj.transform * max2mcv
		local mcv_quat = mcv_transform.rotationPart as quat
		local mcv_final_position = checkPoint mcv_position
		fs.beginObj()
			fs.writeKeyValue "pos" mcv_final_position
			fs.writeComma()
			fs.writeKeyValue "rotation" mcv_quat
		fs.endObj()
	),
	
	
	-- ----------------------------------------------------------
	fn exportRenderMesh obj = (
		
		local exported_materials = #(mats_path + "mtl_default.material")
		local mat = obj.material
		if mat == undefined then (
			print ("Obj " + obj.name + " does NOT have a material")
		)
		if mat != undefined and not (isValidName mat.name) then (
			throw ("Obj " + obj.name + " has a material with an invalid name " + mat.name )
		)
		
		fs.beginObj()
		
		-- Export Mesh
		local mesh_name = mesh_path + obj.name + ".mesh"
		fs.writeKeyValue "mesh" mesh_name
		exportMeshObject obj (project_path + mesh_name )
		fs.writeComma()
	
		if mat != undefined then (
			local me = TMaterialExporter project_path:project_path base_path:base_path
			exported_materials = me.exportMaterial mat mats_path obj
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
			local col_path = project_path + cols_path + obj.name + ".collider"
			if obj.parent != undefined then (
				if obj.mesh.verts.count == obj.parent.mesh.verts.count then (
				-- We are facing, wether convex or mesh collider.
				fs.writeKeyValue "shape" "mesh" 
				fs.writeComma()
				fs.writeKeyValue "name" (cols_path + obj.name + ".collider")
				) else (
					fs.writeKeyValue "shape" "convex" 
					fs.writeComma()
					fs.writeKeyValue "name" (cols_path + obj.name + ".collider")
				)
			) else  (
				fs.writeKeyValue "shape" "mesh" 
				fs.writeComma()
				fs.writeKeyValue "name" (cols_path + obj.name + ".collider")
			)
			
			exportColliderObject obj col_path
		)
		
		writeColliderComponent obj
		fs.endObj()
	),
	
	-- ----------------------------------------------------------	
	fn exportCompLight obj = (
		
		fs.writeKey "light" 
		fs.beginObj()
		
		if classof obj == Directionallight then (
		fs.writeKeyValue "type" "directional"
		fs.writeComma()
		)
		else if classof obj == targetSpot or classof obj == freeSpot then ( 
			
		local target = getnodebyname (obj.name + ".Target")
		fs.writeKeyValue "type" "spotlight"
		fs.writeComma()
		fs.writeKeyValue "range" (distance target.pos obj.pos)
		fs.writeComma()
		fs.writeKeyValue "angle" obj.hotspot
		fs.writeComma()
		)
		else if classof obj == Omnilight   then (
		fs.writeKeyValue "type" "pointlight"
		fs.writeComma()
		fs.writeKeyValue "range" obj.farAttenEnd 
		fs.writeComma()
		)
		else if classof obj == skylight   then (
		fs.writeKeyValue "type" "ambient"
		fs.writeComma()
		)
		
		fs.writeKeyValue "color" [obj.rgb.r, obj.rgb.g, obj.rgb.b]
		fs.writeComma()
		fs.writeKeyValue "intensity" obj.multiplier 
		fs.writeComma()
		fs.writeKeyValue "shadows" obj.castShadows 
		fs.endObj()
	),

	-- ----------------------------------------------------------
	fn exportEditableMesh obj = (
		
		exportName obj 
		fs.writeComma()
		exportTransform obj
		fs.writeComma()
		fs.writeKey "render" 
		fs.beginArray()
		exportRenderMesh obj
		fs.endArray()
		
		-- Do some work here to support hierarchy and groups
		for child in obj.children do (
			if isCollider child then (
				fs.writeComma()
				exportCompCollider child
			)
		)
	),
	
	fn createLightCollider obj = (
		
		local col_path = project_path + cols_path + obj.name + ".collider"
		if classof obj == targetSpot or classof obj == freeSpot  then ( 
			local dist = obj.targetDistance
			if dist == undefined then ( dist = obj.baseObject.targetDistance )
			local int_radius = tan(obj.falloff * .5f) * dist
			local totalsides = clamp 12 32 ((int_radius * .25f) as integer)
			local collider = cone radius1:int_radius radius2:0 height:dist heightsegs:1 sides:totalsides
			collider.transform = obj.transform
			collider.position = (obj.position - obj.dir * dist)
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
			
			exportColliderObject collider col_path
			delete collider
		)
		else if classof obj == omnilight then ( 
			fs.writeKey "collider" 
			fs.beginObj()
			fs.writeKeyValue "shape" "sphere" 
			fs.writeComma()
			fs.writeKeyValue "radius" obj.farAttenEnd 
			fs.writeComma()
			fs.writeKeyValue "is_trigger" true
			fs.endObj()
			fs.writeComma()
		)
		
	),
	
	fn exportEntityLight obj = (
		
		-- It will be exported as part of the parent. Ejemplo del cofre y su caja de colision.
		if obj.parent != undefined then return false
		exportName obj 
		fs.writeComma()
		exportTransform obj
		fs.writeComma()
		exportCompLight obj
		fs.writeComma()
		createLightCollider obj
		fs.writeKey "tags"
		fs.arrayOfStrings #("light")
		
	),
	
	
	-- ----------------------------------------------------------
	fn exportEntityCollider obj = (
		
		-- It will be exported as part of the parent. Ejemplo del cofre y su caja de colision.
		if obj.parent != undefined then return false
		exportName obj 
		fs.writeComma()
		exportTransform obj
		fs.writeComma()
		exportCompCollider obj
	),
	
	
	-- ----------------------------------------------------------
	fn exportEntity obj = (
		
		fs.beginObj()
		fs.writeKey "entity"
			fs.beginObj()
			if isCollider obj then (exportEntityCollider obj )
			else if isLight obj then ( exportEntityLight obj )
			else exportEditableMesh obj
			writeTagComponent obj
			writeHierComponent obj
			fs.endObj()
		fs.endObj()
	),
	
	-- ----------------------------------------------------------
	-- ----------------------------------------------------------
	
	fn exportAll nodes ofilename prg = (
		
		-------------------------------------------------------------------------
		local req_path = findstring project_path "\bin"
		if req_path == undefined then (
			messageBox "Path is not located at /bin.\nPlease, provide a correct path."
			return false				
		)
		-------------------------------------------------------------------------
		
		-- Create the folders if they doesn't exist
		doesFolderExist (project_path + base_path) create:true
		doesFolderExist (project_path + scenes_path) create:true
		doesFolderExist (project_path + mesh_path) create:true
		doesFolderExist (project_path + mats_path) create:true
		doesFolderExist (project_path + cols_path) create:true
		doesFolderExist (project_path + tex_path) create:true
		
		-- Decide output filename based on .max filename
		local full_path = project_path + scenes_path + ofilename + ".scene"
		format "Exporting to %\n" full_path
		fs.begin full_path
		fs.beginArray()
		
		local nitems = 0
		local prg_step = 100 / (nodes.count + 1)
		
		for obj in nodes do (
			
			if nitems > 0 then  fs.writeComma()
			exportEntity obj
			nitems = nitems + 1
			
			prg.value = (nitems + 1)* prg_step
		)
		
		prg.value = 100
		fs.endArray()
		fs.end()	
		
		messageBox ("Scene SUCCESFULLY exported at location: \n\n" + full_path)		
	)
)




