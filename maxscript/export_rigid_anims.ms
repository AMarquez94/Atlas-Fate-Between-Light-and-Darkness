clearListener()

-- ----------------------------------------------------------
-- ----------------------------------------------------------
-- ----------------------------------------------------------
struct TRigidAnimTrack (
	track_name,
	fps = 1,
	begin_time = 0,
	end_time = 1,
	keys = #()
)

struct TRigidAnimKey (
	pos, rot, scale
)

struct TRigidAnimExporter (

	project_path = "D:/DeepShadows/bin/",
	base_path = "data/",
	anims_path = base_path + "rigid_anims/",
	
	fn isValidName aname = (
		return findString aname " " == undefined 
	),
	
	-- ----------------------------------------------------------
	fn writeFixedStr fs str max_bytes = (
		-- Write the vertex type as 32 bytes string with zero paddings
		format "Writing name '%'\n" str
		WriteString fs str
		for i = str.count+1 to ( max_bytes - 1 ) do WriteByte fs 0
	),
	
	-- ----------------------------------------------------------
	fn writeKey fs k = (
		writeFloat fs k.pos.x
		writeFloat fs k.pos.y
		writeFloat fs k.pos.z
		writeFloat fs k.scale
		writeFloat fs k.rot.x
		writeFloat fs k.rot.y
		writeFloat fs k.rot.z
		writeFloat fs -k.rot.w
	),
		
	-- ----------------------------------------------------------
	fn expandLimits curr_limits keys = (
		if keys.count > 0 then (
			local new_min_t = keys[1].time
			local new_max_t = keys[keys.count].time
			if new_min_t < curr_limits.x then curr_limits.x = new_min_t
			if new_max_t > curr_limits.y then curr_limits.y = new_max_t
		)
		return curr_limits
	),
	
	fn getTracksKeys obj fps = (
		local keys = #()
		local limits = [ 100000f,-100000f ]
		limits = expandLimits limits obj.pos.controller.keys
		limits = expandLimits limits obj.scale.controller.keys
		limits = expandLimits limits obj.rotation.controller.keys
		
		local dt = 1.0 / fps * frameRate
		format "Limits of % are %. Delta is %\n" obj.name limits dt
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.position * max2mcv
		-- From mcv, we will go to max, apply the max transform and go back to mcv coord system
		
		local t = limits.x
		while t <= limits.y do (
			local k = TRigidAnimKey()
			at time t (
				-- obj.transform is evaluated at every time
				local mcv_transform = mcv2max * obj.transform * max2mcv
				
				k.pos = obj.pos * max2mcv 
				k.rot = mcv_transform.rotationPart as quat
				k.scale = obj.scale.x
				
			)
			append keys k
			t += dt
		)
		format "Track has % keys\n" keys.count
		
		struct track_info ( keys, limits )
		local tinfo = track_info()
		tinfo.keys = keys
		tinfo.limits = limits
		return tinfo
	),
	
	fn getTrack obj = (
		local t = TRigidAnimTrack()
		t.fps = 5
		local track_info = getTracksKeys obj t.fps
		t.track_name = obj.name
		t.keys = track_info.keys
		local frames_to_second = 1.0 / (float) frameRate
		format "fps % frameRate %\n" frames_to_second frameRate
		t.begin_time = (track_info.limits.x as integer )* frames_to_second
		t.end_time = (track_info.limits.y as integer) * frames_to_second
		format "Track % (%) has length %..%s\n" t.track_name track_info.limits t.begin_time t.end_time
		return t
	),
			
	-- ----------------------------------------------------------
	fn writeData ofilename tracks = (
		local fs = fopen ofilename "wb"
		if fs == undefined then throw ( "Can't create file " + ofilename )
		
		local num_keys = 0;
		for t in tracks do num_keys += t.keys.count
		
		local magicHeader = 0x11777711
		local track_name_length = 32
		local bytes_per_track = track_name_length + 5 * 4
		
		local last_end_time = 0
		for t in tracks do (
			if t.end_time > last_end_time then last_end_time = t.end_time
		)
		local total_duration = last_end_time
		
		-- Save header
		-- Total Bytes, NumIdxs, 
		local headerSize = 8
		local version = 2
		WriteLong fs magicHeader		-- version
		WriteLong fs version		-- version
		WriteLong fs tracks.count	-- NumTracks
		WriteLong fs num_keys	-- Total num keys
		WriteLong fs track_name_length
		WriteLong fs bytes_per_track
		writeFloat fs total_duration

		-- Save the names and index info
		local key0 = 0
		for t in tracks do (
			format "Track % uses % keys\n" t.track_name t.keys.count
			local num_keys = t.keys.count
			writeFixedStr fs t.track_name track_name_length
			writeLong fs key0				-- Index of first key
			writeLong fs num_keys		-- Num keys for this track
			writeFloat fs t.fps		
			writeFloat fs t.begin_time		
			writeFloat fs t.end_time
			key0 += num_keys
		)

		-- Save all the keys
		for t in tracks do (
			for k in t.keys do (
				writeKey fs k
			)
		)
		
		fclose fs
		
		format "File % saved\n" ofilename
	),
			
	-- ----------------------------------------------------------
	fn exportAll objs = (
		
		-- Decide output filename based on .max filename
		local ofilename = getFilenameFile maxFileName
		local full_path = project_path + anims_path + ofilename + ".anims"
		format "Exporting anims to %\n" full_path
		
		local tracks = #()
		for obj in objs do (
			local t = getTrack obj
			append tracks t
		)
		
		writeData full_path tracks
	)
	
)

exporter = TRigidAnimExporter()
exporter.exportAll ($* as array)



