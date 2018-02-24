-- Do some garbage collection first
clearListener()
gc()

-- First, we include the necessary files
filein "DPManager.ms"
filein "MCVExportMesh.ms"
filein "MCVExportScene.ms"

-- Create a rollout floater and load it.
try(cui.UnRegisterDialogBar FirstDialog)catch()
try(destroyDialog FirstDialog)catch()
FirstDialog = newRolloutFloater "Venue Constructor" 485 600

-- Create the necessary global variables 
global man_selection = SelectionManager()
man_selection.updateManager()

global man_mexport = ExportMeshManager()

-- Banner rollout
rollout FirstRollout "Version: 1.0.0"
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
	label 'lbl4' "Category:" pos:[22,121] width:50 height:14 align:#left
	
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
rollout SBExport "Export Tools" width:474 height:358
(
	edittext 'asset_dir' "Asset dir:" pos:[18,17] width:389 height:20 fieldwidth:340 labelOnTop:false align:#left
	button 'btn_test' "..." pos:[420,17] width:25 height:21 toolTip:"This is a tooltip" align:#left
	
	GroupBox 'grp1' "Export Mesh" pos:[10,54] width:450 height:101 align:#left
	GroupBox 'grp2' "Export Scene" pos:[11,162] width:450 height:153 align:#left
	progressBar 'pb6' "ProgressBar" pos:[12,326] width:449 height:21 align:#left color:green
	edittext 'edt24' "Name" pos:[193,79] width:234 height:19 align:#left
	button 'btn34' "Export Selection" pos:[194,117] width:173 height:22 align:#left
	button 'btn35' "All" pos:[376,117] width:51 height:22 align:#left
	checkbox 'chk1' "Generate Collider" pos:[29,107] width:102 height:14 align:#left
	checkbox 'chk2' "Generate LOD" pos:[29,82] width:88 height:14 align:#left
	button 'btn36' "Button" pos:[159,186] width:2 height:116 align:#left
	button 'btn39' "Export Selection" pos:[192,273] width:173 height:22 align:#left
	button 'btn40' "All" pos:[374,273] width:51 height:22 align:#left
	button 'btn41' "Button" pos:[160,75] width:2 height:70 align:#left
	edittext 'scene_name' "Name" pos:[192,192] width:234 height:19 align:#left
	checkbox 'chk5' "Generate Colliders" pos:[29,272] width:108 height:14 align:#left
	checkbox 'chk6' "Generate LODS" pos:[29,247] width:97 height:14 align:#left
	checkbox 'chk7' "Generate Textures" pos:[29,221] width:107 height:14 align:#left checked:true
	checkbox 'chk8' "Generate Materials" pos:[29,196] width:111 height:14 align:#left checked:true
	checkbox 'chk9' "Rewrite existing files" pos:[308,223] width:120 height:14 align:#left checked:true
	
	on SBExport open do (
		scene_name.text = getFilenameFile maxFileName
	)
	
	on btn_Test pressed do (
		
		dir = getSavePath caption:"Select Folder" initialDir:#images
		asset_dir.text = dir
	)
	
	on btn34 pressed do (
		
		pb6.value = 0
		local objs_selected = selection as array
		man_mexport.id = edt24.text
		man_mexport.has_lod = chk2.state 
		man_mexport.has_collider = chk1.state 
		man_mexport.ExportMesh objs_selected asset_dir.text
	)
	
	on btn35 pressed do (
		
		pb6.value = 0
		man_mexport.id = edt24.text
		man_mexport.has_lod = chk2.state 
		man_mexport.has_collider = chk1.state 
		man_mexport.ExportMesh $* asset_dir.text	
	)
	
	on btn39 pressed do (
		
		pb6.value = 0
		local objs_selected = selection as array
		local scene_exporter = TSceneExporter()
		scene_exporter.project_path = asset_dir.text +"\\"
		scene_exporter.exportAll objs_selected scene_name.text pb6
	)
	
	on btn40 pressed do (
		
		pb6.value = 0
		local scene_exporter = TSceneExporter()
		scene_exporter.project_path = asset_dir.text +"\\"
		scene_exporter.exportAll $* scene_name.text pb6
	)
)

-- Animation tools
rollout SBAnimator "Animation Tools" 
(

)

-- Utils Tools
rollout SBUtils "Utils Tools" 
(

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
--cui.RegisterDialogBar Fi