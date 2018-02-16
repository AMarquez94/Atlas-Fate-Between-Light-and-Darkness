clearListener()
gc()

function writeChunk fs id nbytes = (
	WriteLong fs id #unsigned
	WriteLong fs nbytes #unsigned
)

function saveMeshToFile outfilename obj vtxs idxs vtxType = (
	
	local magicHeader = 0x44444444
	local magicVtxs = 0x55554433
	local magicIdxs = 0x55556677
	local magicEoF = 0x55558888
	
	-- Chunk = MagicID(4bytes) + Size(4bytes)
	-- Chunk HeaderData
	-- Chunk Vtxs
	-- Chunk Idxs
	-- .. Chunk CollisionData
	
	local triangle_list = 4
	
	-- Create file
	local fs = fopen outfilename "wb"
	if fs == undefined then throw ( "Can't create file " + outfilename )
	
	local bytes_per_vtx = vtxs[1].count * 4	-- each float is 4 bytes
	local bytes_per_idx = 2
	if vtxs.count >= 65535 then bytes_per_idx = 4
	
	-- Save header
	-- Total Bytes, NumIdxs, 
	local headerSize = 4*(4+4)+32
	writeChunk fs magicHeader headerSize	-- 8 bytes
	
	WriteLong fs vtxs.count		-- NumVertexs
	WriteLong fs idxs.count		-- NumIndices
	WriteLong fs triangle_list  -- PrimitiveType
	WriteLong fs bytes_per_vtx  -- bytes_per_vtx
	
	WriteLong fs bytes_per_idx  -- bytes_per_idx
	WriteLong fs 0              -- dummy1
	WriteLong fs 0              -- dummy2
	WriteLong fs 0              -- dummy3
	
	-- Write the vertex type as 32 bytes string with zero paddings
	WriteString fs vtxType
	for i = vtxType.count+1 to 31 do WriteByte fs 0
	
	-- WriteLong fs 		-- AABB
	
	-- Save vtxs
	local bytes_vtxs = vtxs.count * bytes_per_vtx
	writeChunk fs magicVtxs bytes_vtxs
	for v in vtxs do (	-- Each vtx...
		for vd in v do (	-- Each component in the vtxs
			writeFloat fs vd
		)
	)
	
	-- Save idxs
	local bytes_idxs = idxs.count * bytes_per_idx
	writeChunk fs magicIdxs bytes_idxs
	for v in idxs do (	-- Each idxs...
		local v0 = v - 1  -- Because max starts at 1, but us at 0
		if bytes_per_idx == 2 then (
			writeShort fs v0 #unsigned
		) else (
			writeLong fs v0 #unsigned
		)
	)
	
	writeChunk fs magicEoF 0
	
	-- Close file
	fclose fs
)

function areEqualVertexs v1 v2 = (
	for id = 1 to v1.count do (
		if v1[id] != v2[id] then return false
	)
	return true
)

function findIndexOrRegister ht vtxs new_vtx = (

	-- Key is a string representation of all floats in new_vtx
	local key = with printAllElements off new_vtx as string

	-- Find the string in the hash map
	local idx = ht.Item key
	if idx == undefined then (
		-- Register it
		idx = ht.count + 1
		ht.add key idx			-- Bind "vertex_as_string" to 1
		append vtxs new_vtx	
	)
	return idx
)


function exportMeshObject given_obj outfilename = (
	
	-- Convert the given object to triangular mesh
	local obj = given_obj
	if classof obj != TriMesh then (
		obj = copy given_obj
		obj.name = given_obj.name
		convertToMesh obj
	)
	
	-- The final list of vtxs and indexes to be exported
	local vtxs = #()
	local idxs = #()
	
	local mapA = 1
	local mapB = 3
	
	local maxLocalToWorld = obj.transform
	local maxToMCV = rotateXMatrix  -90
	local maxWorldToLocal = inverse obj.transform
	local maxWorldToLocalMCV = maxWorldToLocal * maxToMCV
	
	-- To quickly find if a vertex is new
	local HT = dotNetObject "System.Collections.Hashtable"
	local num_faces = getNumFaces obj
	
	-- For each face in the model
	local face_id
	for face_id = 1 to num_faces do (
		
		-- Retrieve the 3 indexes in the list of vertex positions
		local face = getFace obj face_id
		local faceA = meshop.getMapFace obj mapA face_id
		local faceB = meshop.getMapFace obj mapB face_id
		local normals = meshop.getFaceRNormals obj face_id
		
		-- For each vertex of each face
		local vtx_id
		for vtx_id in #(1,3,2) do (
			
			-- Get the real vertex id: 1...num_vertexs
			local posW = getVert obj face[ vtx_id ]
			local pos = posW * maxWorldToLocalMCV
			
			-- Get normals
			local NW = normals[ vtx_id ]
			local N = NW * maxWorldToLocalMCV.rotationpart
			
			-- Get UV's values
			local uvA = meshop.getMapVert obj mapA faceA[vtx_id]
		 	local uvB = meshop.getMapVert obj mapB faceB[vtx_id]
					
			local new_vtx = #( pos.x, pos.y, pos.z, N.x, N.y, N.z, uvA.x, 1-uvA.y, uvB.x, 1-uvB.y ) 
			
			-- Check if the vtx is unique
			local new_idx = findIndexOrRegister ht vtxs new_vtx
			
			-- Register the index
			append idxs new_idx
		)
	)
	saveMeshToFile outfilename obj vtxs idxs "PosNUvUv"
	
	-- If we created a copy of the mesh, it's time to remove it
	if obj != given_obj then delete obj
	
)

struct TJsonFormatter (
	
	ofilename,		-- Name of output file
	fs, 				-- File stream
	depth_level = 0,	-- To format json tabs
	
	-- --------------------
	fn begin new_ofilename = (
		ofilename = new_ofilename
		print new_ofilename
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
	project_path = "C:\\Users\\mcv16_alumne\\Documents\\EngineDP\\Bin\\",
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



