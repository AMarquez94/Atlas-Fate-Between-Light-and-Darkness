clearListener()
filein "DPUtils.ms"

function exportCurve obj str_path = (

	local maxLocalToWorld = obj.transform
	local maxToMCV = rotateXMatrix  -90
	local maxWorldToLocal = inverse obj.transform
	local maxWorldToLocalMCV = maxWorldToLocal * maxToMCV
	total_points = #()
	
	fs = TJsonFormatter()
	fs.begin str_path
	fs.beginObj()
	fs.writeKeyValue "type" "catmull-rom"
	fs.writeComma()
	
	-- Retrieve the knots
	for s = 1 to (numsplines obj) do
	(
		for k = 1 to (numknots obj s) do 
		(
			local p_point = getKnotPoint obj s k
			p_point = p_point * maxToMCV
			append total_points (p_point[1] as string + " " + p_point[2] as string + " " + p_point[3] as string)
		)
	)
	
	-- Export the knots
	fs.writeKey "knots"
	fs.arrayOfStrings total_points
	
	fs.endObj()
	fs.end()	
)

