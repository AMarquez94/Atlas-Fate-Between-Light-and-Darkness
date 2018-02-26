filein "DPUtils.ms"
global collider_types = #("basic","convex","mesh")

struct ColliderGenerator (
	
	col_layer,
	type = "basic",
	colliders = #(),
	as_child = false,
	
	function generateBasicShape obj = (
		local cloned_mesh = copy obj
		if as_child == true then ( cloned_mesh.parent = obj )
		cloned_mesh.name = "collider_" + obj.name
		col_layer.addnode cloned_mesh
		return cloned_mesh
	),
	
	function generateBasicComplex obj = (
		local bbox = nodeGetBoundingBox obj obj.transform
		local bb_max = bbox[1]
		local bb_min = bbox[2]
	
		local bb_width = abs(bb_max.x - bb_min.x)
		local bb_length = abs(bb_max.y - bb_min.y)
		local bb_height = abs(bb_max.z - bb_min.z)
	
		local mid_pos = (bb_max + bb_min) / 2
		local new_pos = mid_pos * obj.transform
		
		local cloned_mesh = box width:bb_width length:bb_length height:bb_height
		cloned_mesh.rotation = obj.rotation
		cloned_mesh.pos = [new_pos.x, new_pos.y, new_pos.z  - (bb_height * .5f)] 
		if as_child == true then ( cloned_mesh.parent = obj )
		cloned_mesh.name = "collider_" + obj.name
		col_layer.addnode cloned_mesh
		return cloned_mesh
	),

	-- Method used to create normal bounding boxes
	function generateBounding obj = (
		
		local class_type = classof obj
		local cloned_mesh = undefined
		
		if class_type == Sphere or class_type == Box then (
			generateBasicShape obj
		) else (
			generateBasicComplex obj
		)
	),
	
	function generateConvex obj = (
		local class_type  = classof obj
		
		if class_type== Sphere or class_type == Box then (
			generateBasicShape obj
		) else (
			print "done convex"
			local cvx_hull = nvpx.CreateConvexHull obj.mesh 100 0
			local cloned_mesh = box name:"foo"
			local conv_obj = convertToMesh cloned_mesh
			cloned_mesh.mesh = cvx_hull
			cloned_mesh.rotation = obj.rotation
			cloned_mesh.pos = obj.pos
			if as_child == true then ( cloned_mesh.parent = obj )
			cloned_mesh.name = "collider_" + obj.name
			col_layer.addnode cloned_mesh
		)
	),
	
	function generateMesh obj = (
		generateBasicShape obj		
	),
	
	function generateCollider col_obj as_children = (
		
		as_child = as_children
		col_layer = createLayer "colliders"
		
		for ent in col_obj do (
			if ent.layer == "colliders" then (
				delete ent
			) else (
				if type == "convex" then (generateConvex ent ) 
				else if type == "mesh" then ( generateMesh ent)
				else ( generateBounding ent )
			)
		)
		
		--col_layer.on = false
	)
)