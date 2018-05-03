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

-- Methods to create/destroy components

function getByType type = (

	if type == "collider" then ( return component_collider )
	if type == "tag" then ( return component_tag )
	if type == "prefab" then ( return component_prefab )
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