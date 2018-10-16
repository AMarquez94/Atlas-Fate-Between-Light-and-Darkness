function onSceneStart_scene_basilic_interior()
	end_cinematic_scene();
end

function end_cinematic_scene()

	--execScriptDelayed("activateSubtitles(9);",53);
	--execScriptDelayed("deactivateSubtitles();", 56);


	move("The Player", VEC3(-8.850, 0.044, -33),VEC3(-8.850, 0.044, -30));
	setCinematicPlayerState(true,"final_scene_cinematic");
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_1_Rot_2\", 30.0, \"cinematic\", \"\")", 0);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_1_Rot_1\", 0.0, \"cinematic\", \"\")", 0);


	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_2_Rot_2\", 10.0, \"cinematic\", \"\")", 8);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_2_Rot_1\", 0.0, \"cinematic\", \"\")", 8);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_1_Rot_2\", 0.0)",8);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_1_Rot_1\", 0.0)",8);


	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_3_Rot_2\", 10.0, \"cinematic\", \"\")", 14);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_3_Rot_1\", 0.0, \"cinematic\", \"\")", 14);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_2_Rot_2\", 0.0)",14);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_2_Rot_1\", 0.0)",14);


	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_4_Rot_2\", 10.0, \"cinematic\", \"\")", 19);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_4_Rot_1\", 0.0, \"cinematic\", \"\")", 19);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_3_Rot_2\", 0.0)",19);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_3_Rot_1\", 0.0)",19);
	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -24),VEC3(-8.850, 0.044, -22));",19);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_5_Rot_2\", 10.0, \"cinematic\", \"\")", 25);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_5_Rot_1\", 0.0, \"cinematic\", \"\")", 25);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_4_Rot_2\", 0.0)",25);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_4_Rot_1\", 0.0)",25);
	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -15),VEC3(-8.850, 0.044, -14));",25);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_6_Rot_2\", 8.0, \"cinematic\", \"\")", 31);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_6_Rot_1\", 0.0, \"cinematic\", \"\")", 31);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_5_Rot_2\", 0.0)",31);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_5_Rot_1\", 0.0)",31);
	
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_7_Rot_2\", 10.0, \"cinematic\", \"\")", 39);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_7_Rot_1\", 0.0, \"cinematic\", \"\")", 39);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_6_Rot_2\", 0.0)",39);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_6_Rot_1\", 0.0)",39);
end


function onTriggerEnter_BI_TriggerKill_player()
	playerController:die();
end

function turn_off_and_on_light_interior()
	toggleButtonCanBePressed("Button Turn Off Light", false);
	toggle_spotlight("Spot002");
	execScriptDelayed("toggle_spotlight(\"Spot002\")", 20);
	execScriptDelayed("toggleButtonCanBePressed(\"Button Turn Off Light\", true)", 20);
end