-- Do some garbage collection first
clearListener()
gc()

-- First, we include the necessary files
filein "DPComponents.ms"
filein "DPManager.ms"
filein "MCVExportMesh.ms"
filein "MCVExportScene.ms"
filein "DPGenerator.ms"

-- Create a rollout floater and load it.
try(cui.UnRegisterDialogBar FirstDialog)catch()
try(destroyDialog FirstDialog)catch()
FirstDialog = newRolloutFloater "Venue Constructor" 485 600

-- Create the necessary global variables 
global man_selection = SelectionManager()
man_selection.updateManager()

global man_mexport = ExportMeshManager()

-- Banner rollout
rollout FirstRollout "Version: 1.2.0"
(
	bitmap theBitmap "Bitmap" width:450 height:112 fileName:"aragami_banner2.bmp"
)

-- Instructions rollouts
rollout SBInstructions "Instructions" width:351 height:99
(
	label 'lbl1' "- Selection tools: Select nodes by name, layer and category." pos:[17,9] width:302 height:24 align:#left
	label 'lbl3' "- Export tools: Export a mesh or the whole scene into the engine." pos:[17,28] width:321 height:30 align:#left
	label 'lbl4' "- Animation tools: Export and tweak your animations. WIP" pos:[17,46] width:321 height:30 align:#left
	label 'lbl5' "- Asset library: Visor to watch your assets on live." pos:[17,65] width:321 height:30 align:#left
)

-- Selection rollout
rollout SBSelection "Selection Tools" width:477 height:252
(
	GroupBox 'grp1' "Selection" pos:[11,12] width:450 height:222 align:#left
	edittext 'mesh_filter' "Name:" pos:[37,47] width:184 height:21 fieldwidth:150 labelOnTop:false align:#left
	button 'btn_select' "Select" pos:[34,175] width:188 height:33 toolTip:"This is a tooltip" align:#left

	dropdownList 'sel_layers' "" pos:[73,85] width:149 height:21 align:#left 
 
	listbox 'lbx1' "Selection Hierarchy" pos:[274,31] width:161 height:12 align:#left
	label 'lbl1' "Layers:" pos:[35,87] width:39 height:14 align:#left
	button 'btn13' "Button" pos:[248,36] width:2 height:181 align:#left
	dropdownList 'sel_cat' "" pos:[73,118] width:149 height:21 align:#left
	label 'lbl4' "Tag:" pos:[35,121] width:39 height:14 align:#left
	
	local var1 = ""
	local var2 = sel_layers.selected 
	local var3 = sel_cat.selected
	
	-- On window open
	on SBSelection open do (
		sel_layers.items = man_selection.layers
		sel_cat.items = man_selection.categories
		var2 = sel_layers.selected 
		var3 = sel_cat.selected
		lbx1.items = man_selection.updateHierarchy var1 var2 var3 
	)
	
	on mesh_filter changed var1 do ( 
		lbx1.items = man_selection.updateHierarchy var1 var2 var3 
	)
	
	on sel_layers selected  var2 do (
		var2 = sel_layers.items[var2]
		man_selection.updateManager() 
		lbx1.items = man_selection.updateHierarchy var1 var2 var3 
	)
	
	on sel_cat selected  var3 do ( 
		 var3 = sel_cat.items[var3]
		man_selection.updateManager() 
		lbx1.items = man_selection.updateHierarchy var1 var2 var3 
	)
	
	-- Here we catch button actions
	on btn_select pressed do (
		man_selection.triggerSelection();
	)
)

-- Export tools, to export entire scenes or meshes.
rollout SBExport "Export Tools" width:474 height:469
(
	edittext 'asset_dir' "Asset dir:" pos:[18,17] width:389 height:20 fieldwidth:340 labelOnTop:false align:#left
	button 'btn_test' "..." pos:[420,17] width:25 height:21 toolTip:"This is a tooltip" align:#left
	
	GroupBox 'grp1' "Export Item" pos:[10,54] width:450 height:101 align:#left
	GroupBox 'grp2' "Export Scene" pos:[10,270] width:450 height:153 align:#left
	progressBar 'pb6' "ProgressBar" pos:[11,434] width:449 height:21 color:(color 0 255 0) align:#left
	edittext 'edt24' "Name" pos:[193,79] width:234 height:19 align:#left
	button 'btn34' "Export Selection" pos:[194,117] width:173 height:22 align:#left
	button 'btn35' "All" pos:[376,117] width:51 height:22 align:#left
	checkbox 'chk1' "Generate Collider" pos:[29,107] width:102 height:14 align:#left
	checkbox 'chk2' "Generate LOD" pos:[29,82] width:88 height:14 align:#left
	button 'btn36' "Button" pos:[158,294] width:2 height:116 align:#left
	button 'btn39' "Export Selection" pos:[191,381] width:173 height:22 align:#left
	button 'btn40' "All" pos:[373,381] width:51 height:22 align:#left
	button 'btn41' "Button" pos:[160,75] width:2 height:70 align:#left
	edittext 'scene_name' "Name" pos:[191,300] width:234 height:19 align:#left
	checkbox 'chk5' "Generate Colliders" pos:[28,380] width:108 height:14 align:#left
	checkbox 'chk6' "Generate LODS" pos:[28,355] width:97 height:14 align:#left
	checkbox 'chk7' "Generate Textures" pos:[28,329] width:107 height:14 checked:true align:#left
	checkbox 'chk8' "Generate Materials" pos:[28,304] width:111 height:14 checked:true align:#left
	checkbox 'chk9' "Rewrite existing files" pos:[307,331] width:120 height:14 checked:true align:#left

	GroupBox 'grp10' "Export Prefab" pos:[10,161] width:450 height:101 align:#left
	edittext 'edt12' "Name" pos:[193,186] width:234 height:19 align:#left
	button 'btn27' "Export Selection" pos:[194,224] width:173 height:22 align:#left
	button 'btn28' "All" pos:[376,224] width:51 height:22 align:#left
	checkbox 'chk24' "Use Instancing" pos:[29,214] width:102 height:14 align:#left
	checkbox 'chk25' "Generate LOD" pos:[29,189] width:88 height:14 align:#left
	button 'btn29' "Button" pos:[160,182] width:2 height:70 align:#left
	
	on SBExport open do
	(
		scene_name.text = getFilenameFile maxFileName
	)
	
	on btn_test pressed do
	(			
		dir = getSavePath caption:"Select Folder" initialDir:#images
		asset_dir.text = dir
	)
		
	on btn34 pressed do
	(
		pb6.value = 0
		local objs_selected = selection as array
		man_mexport.id = edt24.text
		man_mexport.has_lod = chk2.state 
		man_mexport.has_collider = chk1.state 
		man_mexport.ExportMesh objs_selected asset_dir.text
	)
	
	on btn35 pressed do
	(			
		pb6.value = 0
		man_mexport.id = edt24.text
		man_mexport.has_lod = chk2.state 
		man_mexport.has_collider = chk1.state 
		man_mexport.ExportMesh $* asset_dir.text	
	)
		
	on btn39 pressed do
	(
		pb6.value = 0
		local objs_selected = selection as array
		local scene_exporter = TSceneExporter()
		scene_exporter.project_path = asset_dir.text +"\\"
		scene_exporter.exportAll objs_selected scene_name.text pb6 true
	)
		
	on btn40 pressed do
	(
		pb6.value = 0
		local scene_exporter = TSceneExporter()
		scene_exporter.project_path = asset_dir.text +"\\"
		scene_exporter.exportAll $* scene_name.text pb6
	)
		
	on btn27 pressed do
	(
		pb6.value = 0
		local objs_selected = selection as array
		local scene_exporter = TSceneExporter()
		scene_exporter.use_instancing = chk24.state
		scene_exporter.project_path = asset_dir.text +"\\"
		scene_exporter.exportAll objs_selected edt12.text pb6 false
	)
	
	on btn28 pressed do
	(			
		pb6.value = 0
		man_mexport.id = edt24.text
		man_mexport.has_lod = chk2.state 
		man_mexport.has_collider = chk1.state 
		man_mexport.ExportMesh $* asset_dir.text	
	)
)

-- Animation tools
rollout SBAnimator "Animation Tools" 
(

)

-- Utils Tools
rollout SBUtils "Utils Tools" width:469 height:263
(
	dropdownList 'ddl3' "" pos:[91,39] width:93 height:21 items:#("basic", "convex", "mesh") align:#left
	label 'lbl3' "Type" pos:[63,42] width:29 height:17 align:#left
	GroupBox 'grp11' "Collider Generator" pos:[9,9] width:453 height:108 align:#left
	button 'btn30' "Generate Selection" pos:[249,40] width:155 height:20 align:#left
	button 'btn31' "Generate All" pos:[249,73] width:155 height:20 align:#left
	checkbox 'chk1' "As Entity" pos:[105,74] width:72 height:16 align:#left
	
	GroupBox 'grp5' "Component Generator" pos:[8,130] width:453 height:107 align:#left
	dropdownList 'ddl2' "" pos:[93,162] width:93 height:21 items:#("tag", "collider", "prefab") align:#left
	label 'lbl4' "Type" pos:[63,164] width:29 height:17 align:#left
	button 'btn3' "Generate Selection" pos:[247,163] width:156 height:20 align:#left	
	button 'btn7' "Remove Selection" pos:[247,197] width:156 height:20 align:#left
	
	on btn30 pressed do
	(
		local col_gen = ColliderGenerator type:ddl3.selected
		col_gen.generateCollider selection (not chk1.state) 
	)
	
	on btn31 pressed do
	(
		local col_gen = ColliderGenerator type:ddl3.selected
		col_gen.generateCollider rootScene.world.children (not chk1.state) 
	)
	
	on btn3 pressed do
	(
		addComponent selection ddl2.selected
	)
	
	on btn7 pressed do
	(
		removeComponent selection ddl2.selected
	)
	
)

-- Asset Library
rollout SBLibrary "Asset Library" 
(
	
)

addrollout FirstRollout FirstDialog rolledUp:off
addrollout SBInstructions FirstDialog rolledUp:on
addrollout SBSelection FirstDialog rolledUp:on
addrollout SBExport FirstDialog rolledUp:on
addrollout SBAnimator FirstDialog rolledUp:on
addrollout SBUtils FirstDialog rolledUp:on
addrollout SBLibrary FirstDialog rolledUp:on