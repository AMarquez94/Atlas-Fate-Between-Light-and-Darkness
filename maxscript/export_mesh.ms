clearListener()

function writeChunk fs id nbytes = (
	WriteLong fs id #unsigned
	WriteLong fs nbytes #unsigned
)

function saveMeshToFile obj vtxs idxs vtxType = (
	
	local magicHeader = 0x44444444
	local magicVtxs = 0x55554433
	local magicIdxs = 0x55556677
	local magicEoF = 0x55558888
	
	local export_path = "C:\\code\\engine\\bin\\data\\meshes\\"
	
	-- Chunk = MagicID(4bytes) + Size(4bytes)
	-- Chunk HeaderData
	-- Chunk Vtxs
	-- Chunk Idxs
	-- .. Chunk CollisionData
	
	local outfilename = export_path + obj.name + ".mesh"
	
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

function findIndexOrRegister vtxs new_vtx = (

	-- Do a linear search...
	local id = 1
	for id = 1 to vtxs.count do (
		if areEqualVertexs vtxs[id] new_vtx then return id
	)
	
	-- At this point, it's new, so append it and return the index
	-- which is the size of the container
	append vtxs new_vtx
	return vtxs.count
)

function exportMeshObject given_obj = (
	
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
	--local mapB = 3
	
	local maxLocalToWorld = obj.transform
	local maxToMCV = rotateXMatrix  -90
	local maxWorldToLocal = inverse obj.transform
	local maxWorldToLocalMCV = maxWorldToLocal * maxToMCV
	
	local num_faces = getNumFaces obj
	
	-- For each face in the model
	local face_id
	for face_id = 1 to num_faces do (
		
		-- Retrieve the 3 indexes in the list of vertex positions
		local face = getFace obj face_id
		local faceA = meshop.getMapFace obj mapA face_id
		local normals = meshop.getFaceRNormals obj face_id
		
		-- For each vertex of each face
		for vtx_id in [1,3,2] do (
			
			-- Get the real vertex id: 1...num_vertexs
			local posW = getVert obj face[ vtx_id ]
			local pos = posW * maxWorldToLocalMCV
			
			-- Get normals
			local N = normals[ vtx_id ]
			
			-- Get UV's values
			local uvA = meshop.getMapVert obj mapA faceA[vtx_id]
		 
			local new_vtx = #( pos.x, pos.y, pos.z, N.x, N.y, N.z, uvA.x, uvA.y ) 
			
			-- Check if the vtx is unique
			local new_idx = findIndexOrRegister vtxs new_vtx
			
			-- Register the index
			append idxs new_idx
		)
	)

	-- Dump
	/*
	local idx = 1
	for v in vtxs do (
		format "Vtx % is %\n" idx v
		idx = idx + 1
	)
	idx = 1
	for f in idxs do (
		format "Idx % is %\n" idx f
		idx = idx + 1
	)
	*/
	
	saveMeshToFile obj vtxs idxs "PosNUv"
	
	-- If we created a copy of the mesh, it's time to remove it
	if obj != given_obj then delete obj
	
)


--exportMeshObject $Box001
exportMeshObject $Teapot001