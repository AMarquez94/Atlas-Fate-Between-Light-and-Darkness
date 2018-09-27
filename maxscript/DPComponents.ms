component_collider = attributes component_collider
(
	parameters cmp_col rollout:cmp_col
	(
		col_group type: #string ui:col_group default:"All"
		col_mask type: #string ui:col_mask default:""
		is_trigger type: #boolean ui:trigger default:false
		is_dynamic type: #boolean ui:dynamic default:false
		is_controller type: #boolean ui:controller default:false
		is_gravity type: #boolean ui:isgravity default:false
	)
	
	rollout cmp_col "Component Collider" width:162 height:174
	(
		edittext 'col_group' "Group" width:130 height:20 align:#center
		edittext 'col_mask' "Mask" width:130 height:20 align:#center
		checkbox 'trigger' "Is Trigger" width:70 height:15 type:#BOOLEAN align:#center
		checkbox 'dynamic' "Is Dynamic" width:76 height:15 type:#BOOLEAN align:#center
		checkbox 'controller' "Is Controller" width:83 height:15 type:#BOOLEAN align:#center
		checkbox 'isgravity' "Is Gravity" width:71 height:15 type:#BOOLEAN align:#center
		
	)
)

component_tag = attributes component_tag
(
	parameters cmp_tag rollout:cmp_tag
	(
		tag1 type: #string ui:tag1 default:"All"
		tag2 type: #string ui:tag2 default:""
		tag3 type: #string ui:tag3 default:""
		tag4 type: #string ui:tag4 default:""
	)
	
	rollout cmp_tag "Component Tag" width:162 height:174
	(
		edittext 'tag1' "Tag 1" width:130 height:20 align:#center
		edittext 'tag2' "Tag 2" width:130 height:20 align:#center
		edittext 'tag3' "Tag 3" width:130 height:20 align:#center
		edittext 'tag4' "Tag 4" width:130 height:20 align:#center
		
	)
)

component_technique = attributes component_technique
(
	parameters cmp_tech rollout:cmp_tech
	(
		techtype type: #string ui:techtype default:"pbr.tech"
	)
	
	rollout cmp_tech "Component Technique" width:162 height:174
	(
		edittext 'techtype' "Technique" width:130 height:20 align:#center
		
	)
)

component_rotator = attributes component_rotator
(
	parameters cmp_rotator rollout:cmp_rotator
	(
		r_speed type: #string ui:r_speed default:"1"
	)
	
	rollout cmp_rotator "Component Rotator" width:162 height:174
	(
		edittext 'r_speed' "Speed" width:130 height:20 align:#center
		
	)
)

component_neon = attributes component_neon
(
	parameters cmp_neon rollout:cmp_neon
	(
		flicker type: #boolean ui:flicker default:false
		fluctuate type: #boolean ui:fluctuate default:false
	)
	
	rollout cmp_neon "Component Rotator" width:162 height:174
	(
		checkBox 'flicker' "Flicker" width:130 height:20 align:#center
		checkBox 'fluctuate' "Fluctuate" width:130 height:20 align:#center
	)
)

component_prefab = attributes component_prefab
(
	parameters cmp_prefab rollout:cmp_prefab
	(
		prefab type: #string ui:prefab default:""
	)
	
	rollout cmp_prefab "Component Prefab" width:162 height:174
	(
		edittext 'prefab' "Source" width:130 height:20 align:#center
	)
)

component_audio = attributes component_audio
(
	parameters cmp_audio rollout:cmp_audio
	(
		audio_file type: #string default:""
	)
	
	rollout cmp_audio "Component Audio" width:162 height:174
	(
		button 'btn1' "Add" pos:[15,52] width:135 height:22 align:#center
		listBox 'lbx1' "Audio Files" pos:[14,91] width:135 height:12 align:#center
		editText 'edt1' "" pos:[14,20] width:133 height:23 align:#center
		
		on btn1 pressed do
		(
			audio_file += edt1.text + "\n" 
			local h_items = filterString audio_file "\n"
			lbx1.items = h_items
		)
	)
)

component_particles = attributes component_particles
(
	parameters cmp_particles rollout:cmp_particles
	(
		particles_files type: #string default:""
	)
	
	rollout cmp_particles "Component Particles" width:162 height:174
	(
		button 'btn1' "Add" pos:[15,52] width:135 height:22 align:#center
		listBox 'lbx1' "Particle Files" pos:[14,91] width:135 height:12 align:#center
		editText 'edt1' "" pos:[14,20] width:133 height:23 align:#center
		
		on btn1 pressed do
		(
			particles_files += edt1.text + "\n"
			local h_items = filterString particles_files "\n"
			lbx1.items = h_items
		)
	)
)

-- Methods to create/destroy components

function getByType type = (

	if type == "collider" then ( return component_collider )
	if type == "tag" then ( return component_tag )
	if type == "prefab" then ( return component_prefab )
	if type == "technique" then ( return component_technique )
	if type == "audio" then ( return component_audio )
	if type == "particles" then ( return component_particles )
	if type == "rotator" then ( return component_rotator )
	if type == "neon" then ( return component_neon )
)

function addComponent c_selection type = (

	local c_type = getByType type
	
	for obj in c_selection do (
		custAttributes.add obj c_type
	)
)

function removeComponent c_selection type = (
	
	local c_type = getByType type
	
	for obj in c_selection do (
		
		local comp = retrieveDefComponent obj c_type
		if comp != undefined then ( custAttributes.delete obj comp)
	
	)
)

function retrieveDefComponent obj type = (
	
	local totalIndex = custAttributes.count obj
	
	for att_index = 1 to totalIndex do (
		local attr_iter = custAttributes.getdef obj att_index
		if attr_iter.name == type.name then ( return attr_iter )
	)
	
	return undefined
)

function retrieveComponent obj type = (
	
	local totalIndex = custAttributes.count obj
	
	for att_index = 1 to totalIndex do (
		local attr_iter = custAttributes.getdef obj att_index
		if attr_iter.name == type.name then ( 
			local comp_final = custAttributes.get obj attr_iter
			return comp_final
		)
	)
	
	return undefined
)