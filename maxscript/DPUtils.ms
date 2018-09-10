-- Utility functions

-- Json struct writter
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
		) else if classof val == Point4 then (
			format "\"%\": \"% % % %\"\n" key_str val.x val.y val.z val.w to:fs
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

fn isCollider obj = (
	
	if obj.layer.name != "colliders" and obj.layer.name != "triggers" then return false
	return true
)

fn isLight obj = (
	
	if obj.layer.name != "lights" and superclassof obj != light then return false
	return true
)	

fn isValidName aname = (
	return findString aname " " == undefined 
)
	

fn doesFolderExist f create:false = (
	
	local val = if (doesFileExist f) and (getfileattribute f #directory) then true else false
	if not val and create then
	(
		val = makeDir f
	)
	return val
) 

function checkPoint p_point = (
	
	if abs(p_point.x) < .00001 do ( p_point.x = 0 )
	if abs(p_point.y) < .00001 do ( p_point.y = 0 )
	if abs(p_point.z) < .00001 do ( p_point.z = 0 )
	
	return p_point
)

fn isConvex obj =
(
	local convex = true
	
	-- set the selection level to vertex
	subObjectLevel = 1
	
	-- selects the concave vertex
	PolyToolsSelect.ConvexConcave 0.0 1
	
	if (obj.selectedVerts.count > 0) then
		convex = false
	
	-- back to object selection level
	subobjectLevel = 0
	
	return convex
)

function createLayer layerName = (
	
	if LayerManager.getLayerFromName layerName == undefined then (
		LayerManager.newLayerFromName layerName
	)		
	
	return LayerManager.getLayerFromName layerName
)

fn clamp minVal maxVal val =
(
	if val < minVal then val = minVal
	else if val > maxVal then val = maxVal
	return val
)