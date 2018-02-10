clearListener()
gc()

-- 
struct TJsonFormatter (
	
	ofilename,		-- Name of output file
	fs, 				-- File stream
	depth_level = 0,	-- To format json tabs
	
	-- --------------------
	fn begin new_ofilename = (
		ofilename = new_ofilename
		fs = createFile  ofilename
	),
	
	-- --------------------
	fn end = (
		if fs != undefined then close fs
		fs = undefined
	),
	
	-- --------------------
	fn indent = (
		for i = 1 to depth_level do format "  " to:fs
	),
	
	-- --------------------
	fn beginArray = (
		indent()
		format "[\n" to:fs
		depth_level = depth_level + 1
	),
	
	fn endArray = (
		depth_level = depth_level - 1
		indent()
		format "]\n" to:fs
	),
	
	-- --------------------
	fn beginObj = (
		indent()
		format "{\n" to:fs
		depth_level = depth_level + 1
	),
	
	fn endObj = (
		depth_level = depth_level - 1
		indent()
		format "}\n" to:fs
	),
	
	fn writeComma = (
		indent()
		format ",\n" to:fs
	),
	
		-- --------------------
	fn writeKey key_str = (
		indent()
		format "\"%\":\n" key_str to:fs
	),
		-- --------------------
	fn writeKeyValue key_str val = (
		indent()
		if classof val == string then (
			format "\"%\": \"%\"\n" key_str val to:fs
		) else if classof val == Point3 then (
			format "\"%\": \"% % %\"\n" key_str val.x val.y val.z to:fs
		) else if classof val == quat then (
			-- Quats rotate in the opposite direction
			format "\"%\": \"% % % %\"\n" key_str val.x val.y val.z -val.w to:fs
		) else (
			format "\"%\": %\n" key_str val to:fs
		)
	),
	
	-- --------------------
	fn writeName name = (
		indent()
		format "\"%\"\n" name to:fs
	),
	
	fn arrayOfStrings arr = (
		beginArray()
		for idx = 1 to arr.count do (
			if idx != 1 then writeComma()
			writeName arr[idx]
		)
		endArray()
	)
	
	
)

	-- ----------------------------------------------------------
	-- ----------------------------------------------------------
	-- ----------------------------------------------------------
struct TMaterialExporter (
	
	fs = TJsonFormatter(), 
	project_path,                 -- Asigned in the ctor 
	base_path,                 -- Asigned in the ctor 	
	
	
	fn exportMap map alias = (
		-- "C:\mcv\code\bin\data\textures\bricks.DDS"
		
		-- "C:\users\pep\desktop\download\bricks.dds"
		local map_filename = map.filename
		
		-- bricks
		local base_name = getFilenameFile map_filename
		
		-- data\textures\bricks.dds"
		local json_filename = base_path + "textures/" + base_name + ".dds"
		
		-- 
		local ofull_path = project_path + json_filename
		
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

fn isCollider obj = (
	if obj.layer.name != "collisions" and obj.layer.name != "triggers" then return false
	if classof obj != Box and classof obj != Sphere then throw "Collisions must be Spheres or Boxes."
	-- if name starts with col_...
	return true
)	

	-- ----------------------------------------------------------
	-- ----------------------------------------------------------
	-- ----------------------------------------------------------
struct TSceneExporter (

	fs = TJsonFormatter(), 
	project_path = "c:/code/engine/bin/",
	base_path = "data/",
	scenes_path = base_path + "scenes/",
	mesh_path = base_path + "meshes/",
	mats_path = base_path + "materials/",
	
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
		
		-- From mcv, we will go to max, apply the max transform and go back to mcv coord system
		local mcv_transform = mcv2max * obj.transform * max2mcv

		-- Take just the rotation as quaterion
		local mcv_quat = mcv_transform.rotationPart as quat
		
		fs.beginObj()
			fs.writeKeyValue "pos" mcv_position
			fs.writeComma()
			fs.writeKeyValue "rotation" mcv_quat
		fs.endObj()
	),
	
	
	-- ----------------------------------------------------------
	fn exportRenderMesh obj = (
		local mat = obj.material
		if mat == undefined then (
			throw ("Obj " + obj.name + " does NOT have a material")
		)
		if not (isValidName mat.name) then (
			throw ("Obj " + obj.name + " has a material with an invalid name " + mat.name )
		)
		
		fs.beginObj()
		
			-- Export Mesh
			local mesh_name = mesh_path + obj.name + ".mesh"
			fs.writeKeyValue "mesh" mesh_name
			exportMeshObject obj (project_path + mesh_name )

			fs.writeComma()
		
			-- Export material
			--local mat_name = mats_path + mat.name + ".material"
			--fs.writeKeyValue "material" mat_name
			local me = TMaterialExporter project_path:project_path base_path:base_path
			local exported_materials = me.exportMaterial mat mats_path obj
		
			fs.writeKey "materials"
			fs.arrayOfStrings exported_materials
		
		fs.endObj()
	),
	
	-- ----------------------------------------------------------
	fn exportCompCollider obj = (
		fs.writeKey "physics" 
		fs.beginObj()
		
		if classof obj == Sphere then (
			fs.writeKeyValue "shape" "sphere"
			fs.writeComma()
			fs.writeKeyValue "radius" obj.radius
		) else if classof obj == Box then (
			fs.writeKeyValue "shape" "box"
			fs.writeComma()
			-- Warning, order might be incorrect!!!
			fs.writeKeyValue "half_size" [abs obj.width, abs obj.height, abs obj.length]
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
	fn exportEditableMesh obj = (
		
		exportName obj 
		fs.writeComma()
		exportTransform obj
		fs.writeComma()
		fs.writeKey "render" 
		fs.beginArray()
			exportRenderMesh obj
		fs.endArray()
		
		for child in obj.children do (
			
			if isCollider child then (
				fs.writeComma()
				exportCompCollider child
			)
			
		)
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
			--if classof obj == Camera then exportCamera obj
			if classof obj == Editable_Mesh then exportEditableMesh obj
			else if isCollider obj then exportEntityCollider obj
			else (
				format "Warning. Don't kwow how to export obj % of class %\n" obj.name ((classof obj) as string)
			)
			fs.endObj()
		fs.endObj()
	),
	
	fn exportAll = (
		
		-- Decide output filename based on .max filename
		local ofilename = getFilenameFile maxFileName
		local full_path = project_path + scenes_path + ofilename + ".scene"
		format "Exporting to %\n" full_path
		fs.begin full_path
		fs.beginArray()
		
		local nitems = 0
		for obj in $* do (
			if nitems > 0 then  fs.writeComma()
			exportEntity obj
			nitems = nitems + 1
		)
		
		fs.endArray()
		fs.end()
		
	)
	
	
)

exporter = TSceneExporter()
exporter.exportAll()



