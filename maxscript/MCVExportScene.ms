clearListener()
gc()

filein "DPUtils.ms"

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
		local json_filename = "data\\textures\\" + base_name + ".dds"
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
			fs.writeKeyValue "technique" "textured.tech"
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
			fs.writeKeyValue "halfExtent" [abs (obj.width * .5f), abs (obj.height * .5f), abs (obj.length * .5f)]
		) else (
			
			if obj.parent != undefined then (
				local col_path = project_path + cols_path + obj.name + ".collider"
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
		
		-- Add it in the user properties panel of max:     density = 10
		local density = getUserProp obj "density"
		if density != undefined then (
			fs.writeComma()
			fs.writeKeyValue "density" density
		)
		
		if obj.layer.name == "triggers" then (
			fs.writeComma()
			fs.writeKeyValue "trigger" true
		)
	
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
		else if classof obj == targetSpot  then ( 
			
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
		fs.writeComma()
		fs.writeKey "tags"
		fs.arrayOfStrings #("light")
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
	
	
	fn exportEntityLight obj = (
		
		-- It will be exported as part of the parent. Ejemplo del cofre y su caja de colision.
		if obj.parent != undefined then return false
		exportName obj 
		fs.writeComma()
		exportTransform obj
		fs.writeComma()
		exportCompLight obj
		fs.writeComma()
		exportCompCollider obj
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
			/*else (
				format "Warning. Don't kwow how to export obj % of class %\n" obj.name ((classof obj) as string)
			)*/
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
		
		fs.endArray()
		fs.end()		
	)
)




