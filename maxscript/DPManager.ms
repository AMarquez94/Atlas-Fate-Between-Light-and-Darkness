filein "DPUtils.ms"
filein "MCVExportMesh.ms"
filein "MCVExportEntity.ms"

-- To replace
struct ExportSceneManager (

	id, 
	has_collider,
	has_lod,
	base_path = "data/",
	mesh_path = base_path + "meshes/",
	
	function ExportMesh out_group out_path = (
	
		local token_path = out_path + "/" + mesh_path + "/"
		if out_group.count == 1 and id != "" then  (
			local str_path = token_path + id + ".mesh"
			exportMeshObject out_group[1] str_path
		)
		else if out_group.count == 1 then (
			local str_path = token_path + out_group[1].Name + ".mesh"
			exportMeshObject out_group[1] str_path
		)
		else (
			for i in out_group do (
				local str_path = token_path + i.Name + "_" + id + ".mesh"
				exportMeshObject i str_path
			)
		)
	)	
)

struct ExportMeshManager (

	id, 
	has_collider,
	has_lod,
	base_path = "data/",
	mesh_path = base_path + "meshes/",
	curve_path = base_path + "curves/",
	
	function ExportItem obj out_path = (

		local meshes_folder = out_path + "/" + mesh_path 
		local curves_folder = out_path + "/" + curve_path 
		doesFolderExist meshes_folder create:true
		doesFolderExist curves_folder create:true		
		--------------------------------------------------------------------------------

		-- Check if it is an spline
		if classof obj == Line then (
			local str_path = curves_folder + id + ".curve"
			exportCurve obj str_path
			messageBox ("Curve SUCCESFULLY exported at location: \n\n" + str_path)
		) 
		else (
			local str_path = meshes_folder + id + ".mesh"
			exportMeshObject obj str_path
			messageBox ("Mesh SUCCESFULLY exported at location: \n\n" + str_path)
		)	
	),
	
	function ExportMesh out_group out_path = (
	
		-- Check for path requirements 
		local req_path = findstring out_path "bin"
		
		---------------------------------------------------------------------------------
		if req_path == undefined then (
			messageBox "Path is not located at /bin.\nPlease, provide a correct path."
			return false				
		)
		
		if out_group.count == 1 then (
			ExportItem out_group[1] out_path
		)
		else (
			for i in out_group do (
				ExportItem i out_path
			)
		)
	)	
)

struct SelectionManager (
	
	layers = #(),
	selhierarchy = #(),
	categories = #("mesh", "collider", "decal"), --manually adding them by now
	strsearch = "",
	
	function triggerSelection = (
		
		local final_selection = #()
		
		for i = 1 to selhierarchy.count do (
			nodetostack = getnodebyname selhierarchy[i]
			append final_selection nodetostack
		)
		
		select final_selection
	),
	
	function updateHierarchy substr layer category = (
		
		local candidates = #()
		local layer_nodes = for i in $* where i.layer.name == layer collect i
		for x = 1 to layer_nodes.count do (
			local has_substr = findstring layer_nodes[x].Name substr
			if  has_substr != undefined or substr == "" then ( --refactor
				append candidates layer_nodes[x].Name
			)			
		)
		
		for i = 0 to categories.count do (
			-- check for mesh type on given candidates
			-- TO-DO
		)
		
		selhierarchy = candidates
		return selhierarchy
	),
	
	function updateLayers = (
		
		for i = 0 to layerManager.count-1 do(
			
			local ilayer = layerManager.getLayer i
			local layerName = ilayer.name
			append layers layerName
		)
	),
	
	function updateManager = (
	
		-- First, update the layers, search them in max
		updateLayers()
		
		-- Update the categories?
	)	
)
