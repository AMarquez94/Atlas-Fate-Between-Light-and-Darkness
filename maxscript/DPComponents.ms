component_collider = attributes component_collider
(
	parameters main rollout:params
	(
		col_group type: #string ui:col_group default:"All"
		col_mask type: #string ui:col_mask default:""
		is_trigger type: #boolean ui:trigger default:false
		is_dynamic type: #boolean ui:dynamic default:false
		is_controller type: #boolean ui:controller default:false
		is_gravity type: #boolean ui:isgravity default:false
	)
	
	rollout params "Component Collider" width:162 height:174
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
	parameters main rollout:params
	(
		tag1 type: #string ui:tag1 default:"All"
		tag2 type: #string ui:tag2 default:""
		tag3 type: #string ui:tag3 default:""
		tag4 type: #string ui:tag4 default:""
	)
	
	rollout params "Component Tag" width:162 height:174
	(
		edittext 'tag1' "Tag 1" width:130 height:20 align:#center
		edittext 'tag2' "Tag 2" width:130 height:20 align:#center
		edittext 'tag3' "Tag 3" width:130 height:20 align:#center
		edittext 'tag4' "Tag 4" width:130 height:20 align:#center
		
	)
)

-- Methods to create/destroy components

function getByType type = (

	if type == "collider" then ( return component_collider )
	if type == "tag" then ( return component_tag )
)

function addComponent c_selection type = (

	local c_type = getByType type
	
	for obj in c_selection do (
		custAttributes.add obj c_type
	)
)

function removeComponent c_selection type = (
	
	local c_type = getByType type
	print "entered"
	
	for obj in c_selection do (
		z = 1
		while z !=undefined do 
		(  
			z = custattributes.getdef i 1  
			--print z
			/*if z == c_type then (
				custAttributes.delete i z 
			)*/
		) 
	)
)