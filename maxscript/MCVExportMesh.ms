clearListener()

struct TMeshSubGroup (
	first_idx,
	num_indices,
	material_index,
	user_material_id
)

function getMaterialsUsedByMesh obj = (
	-- Get faces by material id
	local num_faces = getNumFaces obj
	local faces_by_mat = #()
	for face_id = 1 to num_faces do (
		local mat_id = getFaceMatID obj face_id
		if faces_by_mat[ mat_id ] == undefined then (
			faces_by_mat[ mat_id ] = #()
		)			
		append faces_by_mat[ mat_id ] face_id
	)
	return faces_by_mat
)

function writeChunk fs id nbytes = (
	
	WriteLong fs id #unsigned
	WriteLong fs nbytes #unsigned
)

function saveMeshToFile outfilename obj vtxs idxs vtxType subgroups = (
	
	print outfilename
	-- Chunk = MagicID(4bytes) + Size(4bytes)
	local magicHeader = 0x44444444	-- Chunk HeaderData
	local magicVtxs = 0x55554433		-- Chunk Vtxs
	local magicIdxs = 0x55556677		-- Chunk Idxs
	local magicSubGroups = 0x55556688
	local magicEoF = 0x55558888		-- .. Chunk CollisionData
	local triangle_list = 4
	
	-- Create file
	local fs = fopen outfilename "wb"
	if fs == undefined then throw ( "Can't create file " + outfilename )
	
	local bytes_per_vtx = vtxs[1].count * 4	-- each float is 4 bytes
	local bytes_per_idx = 2
	if vtxs.count >= 65535 then bytes_per_idx = 4
	
	-- Total Bytes, NumIdxs, 
	local headerSize = 4*(4+4)+32
	writeChunk fs magicHeader headerSize	-- 8 bytes
	
	WriteLong fs vtxs.count		-- NumVertexs
	WriteLong fs idxs.count		-- NumIndices
	WriteLong fs triangle_list  -- PrimitiveType
	WriteLong fs bytes_per_vtx  -- bytes_per_vtx
	
	WriteLong fs bytes_per_idx  -- bytes_per_idx
	WriteLong fs subgroups.count   -- Num SubGroups
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
	
	-- Save subgroups
	local bytes_per_subgroup = 4 * 4			-- 4 ints
	local bytes_subgroups = subgroups.count * bytes_per_subgroup
	writeChunk fs magicSubGroups bytes_subgroups
	for sg in subgroups do (	-- Each subgroup
		writeLong fs sg.first_idx #unsigned
		writeLong fs sg.num_indices #unsigned
		writeLong fs sg.material_index #unsigned
		writeLong fs sg.user_material_id #unsigned
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
	local subgroups = #()
		
	local mapA = 1
	local mapB = 3
	
	local maxLocalToWorld = obj.transform
	local maxToMCV = rotateXMatrix  -90
	local maxWorldToLocal = inverse obj.transform
	local maxWorldToLocalMCV = maxWorldToLocal * maxToMCV
	
	-- To quickly find if a vertex is new
	local HT = dotNetObject "System.Collections.Hashtable"
	local num_faces = getNumFaces obj
	
	local mapB_status = meshop.getMapSupport obj mapB
	mapB = mapA
	
	if mapB_status == true then ( mapB = 3 )
	
-- For each material used by the mesh
	local mat_idx 
	for mat_idx = 1 to faces_by_material_used.count do (
		local faces = faces_by_material_used[mat_idx]
		if faces == undefined then continue
		
		-- We are starting a new group. 
		local new_subgroup = TMeshSubGroup()
		new_subgroup.first_idx = idxs.count
		-- Each face uses 3 indices..
		new_subgroup.num_indices = faces.count * 3
		new_subgroup.material_index = subgroups.count
		new_subgroup.user_material_id = mat_idx
		
		-- For each face using that material
		for face_id in faces do (
		
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
				
				-- if mapA != mapB then join new_vtx #(uvB.x, 1-uvB.y)
				
				-- Check if the vtx is unique
				local new_idx = findIndexOrRegister ht vtxs new_vtx
				
				-- Register the index
				append idxs new_idx
			)
		)
		
		append subgroups new_subgroup
	)

	saveMeshToFile outfilename obj vtxs idxs "PosNUvUv" subgroups
	
	-- If we created a copy of the mesh, it's time to remove it
	if obj != given_obj then delete obj
	
)


function exportColliderObject given_obj outfilename = (

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
	
	local maxLocalToWorld = obj.transform
	local maxToMCV = rotateXMatrix  -90
	local maxWorldToLocal = inverse obj.transform
	local maxWorldToLocalMCV = maxWorldToLocal * maxToMCV
	
	-- To quickly find if a vertex is new
	local HT = dotNetObject "System.Collections.Hashtable"
	local num_faces = getNumFaces obj
	
	-- For each face in the model
	for face_id = 1 to num_faces do (
		
		-- Retrieve the 3 indexes in the list of vertex positions
		local face = getFace obj face_id

		-- For each vertex of each face
		for vtx_id in #(1,3,2) do (
			
			-- Get the real vertex id: 1...num_vertexs
			local posW = getVert obj face[ vtx_id ]
			local pos = posW * maxWorldToLocalMCV

			local new_vtx = #( pos.x, pos.y, pos.z) 
			local new_idx = findIndexOrRegister ht vtxs new_vtx
			
			-- Register the index
			append idxs new_idx
		)
	)

	saveMeshToFile outfilename obj vtxs idxs "PosN" subgroups
	
	-- If we created a copy of the mesh, it's time to remove it
	if obj != given_obj then delete obj
	
)

