clearListener()

struct  TSkeletonsExporter (
	project_path = "D:/Games/Deep_Shadows/Bin/data/skeletons/player/",
	skel_name = "player",			-- smurf
	skel_root_path,					-- "C:/../bin/data/skeletons/smurf/,
	skel_file, 						-- "C:/../bin/data/skeletons/smurf/smurf.csf"
	
	fn isBone obj = ( 
		-- Discard black bones
		return classof obj == Biped_Object and obj.wireColor != Black
	),
	-- Search the root bones in the scene to define the .csf
	fn findSkelCandidates = (
		local obj_bones = #()
		for obj in $* do (
			if isBone obj and obj.parent == undefined then (
				format "% is a root bone\n" obj.name
				append obj_bones obj
			)
		)
		return obj_bones
	),
	
	-- Find in the scene the bones which will be exported as part of the animation
	-- Change the color to black to discard the bones
	fn findBoneCandidates = (
		local objs = #()		
		for obj in $* do (
			
			if isBone obj then (
				format "% is a bone for the animation\n" obj.name
				append objs obj
			)
		)
		return objs
	),
	
	-- Returns an array of meshes which have the skin modifier and will be exported as .cmf
	fn findMeshCandidates = (
		local objs = #()
		for obj in $* do (
			if (classof obj == PolyMeshObject) or (classof obj == Editable_Mesh) then (
				if obj.modifiers[#Skin] != undefined then (
					format "% is a skinned mesh \n" obj.name
					append objs obj
				)
			)
		)
		return objs
	),
	
	
	fn setSkelName new_name = (
		skel_name = new_name
		skel_root_path = project_path + "skeletons/" + skel_name + "/"
		makedir skel_root_path 
		skel_file = skel_root_path + skel_name + ".csf"
	),
	
	-- Find a bone candidate to be the root, and choose his name as name of the output file
	fn findRootBone = (
		local root_bones = findSkelCandidates()
		local root = root_bones[1]
		setSkelName root.name
		return root_bones
	),
	
	fn exportSkelAndMeshes = (
		if ExportCalSkel == undefined then throw "Please, install the cal3d tools"
		
		local root_bones = findRootBone()
		local root = root_bones[1]
		
		-- Pasar a modo figura si esta en modo pose
		local was_in_figure_mode = root.controller.figureMode
		root.controller.figureMode = true
		local bones_to_export = findBoneCandidates()
		
		for b in bones_to_export do (
			format "Bone Skel is %\n" b.name
		)
		
		format "Exporting cal csf to %\n" skel_file
		local rc = ExportCalSkel skel_file bones_to_export off
	
		-- Export la mesh...
		local obj_meshes = findMeshCandidates()
		for obj in obj_meshes do (
			local mesh_file = skel_root_path + obj.name + ".cmf"
			-- 0.01 threshold, no LOD, no springs
			local rc = ExportCalMesh mesh_file skel_file obj 4 0.01 false false
			if rc != 0 then throw ("Failed to export cal3d mesh from object " + obj.name )
		)
		
		-- Restaurar el modo pose si es q estaba en modo pose inicialmente
		if was_in_figure_mode == false then root.controller.figureMode = false
    ),
	
	fn exportAnim = (
		local root_bones = findRootBone()
		
		-- Export anims. 
		-- Take the name of the output file from the .max filename
		local anim_name = getFilenameFile  maxfilename
		local animation_file = skel_root_path + anim_name + ".caf"
		-- Get all the bones to export
		local bones_to_export = findBoneCandidates()
		-- frames to integer-frames
		-- From 60f we need to get the 60 as integer
		local begin_time =  animationRange.start as integer * frameRate / 4800  
		local end_time = animationRange.end as integer  * frameRate / 4800
		local rc = ExportCalAnim animation_file skel_file bones_to_export begin_time end_time 0 frameRate
		if rc != 0 then throw ("Failed to export cal3d anim: " + (rc as integer) )
	)
)

gc()
se = TSkeletonsExporter()
--se.exportSkelAndMeshes()
se.exportAnim()