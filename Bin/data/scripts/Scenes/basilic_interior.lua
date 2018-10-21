function onSceneStart_scene_basilic_interior()
	startCinematicMode(178.5);
	deleteAllCacheResources();
	setOutBlackScreen(0.25);
	gameManager:changeToEndScene();
	--finish_job_end_cinematic_scene()
	--shutdown_end_cinematic_scene();
	pre_end_cinematic_scene();
end

function finish_job_end_cinematic_scene()
	execScriptDelayed("setCinematicPlayerState(true,\"final_shutdown_scene_cinematic\")",0.75);
	setInBlackScreen(0.25);
	execScriptDelayed("move(\"The Player\", VEC3(-7.881,0.044,-0.818),VEC3(-7.881,0.044,0.0));",0.75);
	execScriptDelayed("setOutBlackScreen(0.25);",1);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_14_Rot_2\", 10.0, \"cinematic\", \"\")", 0.75);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_14_Rot_1\", 0.0, \"cinematic\", \"\")", 0.75);


	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_15_Rot_2\", 10.0, \"cinematic\", \"\")", 6);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_15_Rot_1\", 0.0, \"cinematic\", \"\")", 6);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_14_Rot_2\", 0.0)",6);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_14_Rot_1\", 0.0)",6);
	execScriptDelayed("speedUpRuedasFinalScene()",7.5); 

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_16_Rot_2\", 10.0, \"cinematic\", \"\")", 12);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_16_Rot_1\", 0.0, \"cinematic\", \"\")", 12);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_15_Rot_2\", 0.0)",12);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_15_Rot_1\", 0.0)",12);

	
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_17_Rot_2\", 20.0, \"cinematic\", \"\")", 17);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_17_Rot_1\", 0.0, \"cinematic\", \"\")", 17);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_16_Rot_2\", 0.0)",17);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_16_Rot_1\", 0.0)",17);

	execScriptDelayed("ambientAdjustmentForFinalScene(10)",37);
	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -4.275),VEC3(-8.850, 0.044, -8));",28);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_18_Rot_2\", 25.0, \"cinematic\", \"\")", 28.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_18_Rot_1\", 0.0, \"cinematic\", \"\")", 28.5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_17_Rot_2\", 0.0)",28.5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_17_Rot_1\", 0.0)",28.5);
	--execScriptDelayed("startTransmission(7)", 38);
	execScriptDelayed("activateSubtitles(31);",38);
	execScriptDelayed("deactivateSubtitles();",45);
	execScriptDelayed("execLastAtlasScreen()",53);
	--execScriptDelayed("execLastAtlasScreen()",3);
	execScriptDelayed("lightUpForFinalScene(true,3);",37);
end

function shutdown_end_cinematic_scene()
	execScriptDelayed("setCinematicPlayerState(true,\"final_shutdown_scene_cinematic\")",0.75);
	setInBlackScreen(0.25);
	execScriptDelayed("move(\"The Player\", VEC3(-7.881,0.044,-0.818),VEC3(-7.881,0.044,0.0));",0.75);
	execScriptDelayed("setOutBlackScreen(0.25);",1);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_14_Rot_2\", 10.0, \"cinematic\", \"\")", 0.75);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_14_Rot_1\", 0.0, \"cinematic\", \"\")", 0.75);


	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_15_Rot_2\", 10.0, \"cinematic\", \"\")", 6);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_15_Rot_1\", 0.0, \"cinematic\", \"\")", 6);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_14_Rot_2\", 0.0)",6);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_14_Rot_1\", 0.0)",6);
	execScriptDelayed("speedUpRuedasFinalScene()",7.5); 

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_16_Rot_2\", 10.0, \"cinematic\", \"\")", 12);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_16_Rot_1\", 0.0, \"cinematic\", \"\")", 12);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_15_Rot_2\", 0.0)",12);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_15_Rot_1\", 0.0)",12);
	
	--execScriptDelayed("startTransmission(19)", 17);
	execScriptDelayed("activateSubtitles(32);",17);
	execScriptDelayed("activateSubtitles(33);",23);
	execScriptDelayed("activateSubtitles(34);",28.5);
	execScriptDelayed("deactivateSubtitles();",36);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_17_Rot_2\", 20.0, \"cinematic\", \"\")", 17);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_17_Rot_1\", 0.0, \"cinematic\", \"\")", 17);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_16_Rot_2\", 0.0)",17);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_16_Rot_1\", 0.0)",17);
	execScriptDelayed("stopRuedasFinalScene()",30); 

	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -4.275),VEC3(-8.850, 0.044, -8));",28);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_18_Rot_2\", 25.0, \"cinematic\", \"\")", 28.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_18_Rot_1\", 0.0, \"cinematic\", \"\")", 28.5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_17_Rot_2\", 0.0)",28.5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_17_Rot_1\", 0.0)",28.5);
	--
	execScriptDelayed("pasarelaLightsFadeOut()", 44);
	

	execScriptDelayed("execLastAtlasScreen()",53);
	execScriptDelayed("lightUpForFinalScene(false, 13.0);",22);
	execScriptDelayed("lightDownForFinalScene(true, 0.5);",37);


end

function pre_end_cinematic_scene()

	--First plane
	move("The Player", VEC3(-8.850, 0.044, -33),VEC3(-8.850, 0.044, -30));
	setCinematicPlayerState(true,"final_scene_cinematic");
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_1_Rot_2\", 30.0, \"cinematic\", \"\")", 0);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_1_Rot_1\", 0.0, \"cinematic\", \"\")", 0);

	--Second plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_2_Rot_2\", 10.0, \"cinematic\", \"\")", 8);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_2_Rot_1\", 0.0, \"cinematic\", \"\")", 8);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_1_Rot_2\", 0.0)",8);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_1_Rot_1\", 0.0)",8);

	--Third plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_3_Rot_2\", 10.0, \"cinematic\", \"\")", 14);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_3_Rot_1\", 0.0, \"cinematic\", \"\")", 14);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_2_Rot_2\", 0.0)",14);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_2_Rot_1\", 0.0)",14);

	--Fourth plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_4_Rot_2\", 10.0, \"cinematic\", \"\")", 19);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_4_Rot_1\", 0.0, \"cinematic\", \"\")", 19);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_3_Rot_2\", 0.0)",19);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_3_Rot_1\", 0.0)",19);
	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -24),VEC3(-8.850, 0.044, -22));",19);

	--Fifth plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_5_Rot_2\", 10.0, \"cinematic\", \"\")", 25);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_5_Rot_1\", 0.0, \"cinematic\", \"\")", 25);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_4_Rot_2\", 0.0)",25);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_4_Rot_1\", 0.0)",25);
	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -15),VEC3(-8.850, 0.044, -14));",25);

	--Sixth plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_6_Rot_2\", 8.0, \"cinematic\", \"\")", 31);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_6_Rot_1\", 0.0, \"cinematic\", \"\")", 31);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_5_Rot_2\", 0.0)",31);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_5_Rot_1\", 0.0)",31);
	
	--Seventh plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_7_Rot_2\", 15.0, \"cinematic\", \"\")", 39);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_7_Rot_1\", 0.0, \"cinematic\", \"\")", 39);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_6_Rot_2\", 0.0)",39);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_6_Rot_1\", 0.0)",39);

	--Eighth plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_8_2\", 8.0, \"cinematic\", \"\")", 47);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_8\", 0.0, \"cinematic\", \"\")", 47);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_7_Rot_2\", 0.0)",47);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_7_Rot_1\", 0.0)",47);
	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -6.1),VEC3(-8.850, 0.044, -4));",47);

	--Ninth plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_9_Rot_2\", 15.0, \"cinematic\", \"\")", 51);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_9_Rot_1\", 0.0, \"cinematic\", \"\")", 51);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_8\", 0.0)",51);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_8_2\", 0.0)",51);

	--Tenth plane--Starts talking
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_10_Rot_2\", 15.0, \"cinematic\", \"\")", 58);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_10_Rot_1\", 0.0, \"cinematic\", \"\")", 58);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_9_Rot_2\", 0.0)",58);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_9_Rot_1\", 0.0)",58);
	execScriptDelayed("startTransmission(26)", 60);
	execScriptDelayed("activateSubtitles(25);",60);
	execScriptDelayed("activateSubtitles(26);",64);

	--Eleventh plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_11_Rot_2\", 10.0, \"cinematic\", \"\")", 69);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_11_Rot_1\", 0.0, \"cinematic\", \"\")", 69);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_10_Rot_2\", 0.0)",69);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_10_Rot_1\", 0.0)",69);
	execScriptDelayed("activateSubtitles(27);",69);

	--Twelfth plane
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_12_Rot_2\", 10.0, \"cinematic\", \"\")", 79);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_12_Rot_1\", 0.0, \"cinematic\", \"\")", 79);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_11_Rot_2\", 0.0)",79);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_11_Rot_1\", 0.0)",79);
	execScriptDelayed("move(\"The Player\", VEC3(-8.850, 0.044, -4.275),VEC3(-8.850, 0.044, -1));",79);
	execScriptDelayed("activateSubtitles(28);",79);
	execScriptDelayed("deactivateSubtitles();",86);

	--Thirteenth plane
	execScriptDelayed("setInBlackScreen(0.25);",87);
	execScriptDelayed("setOutBlackScreen(0.25);",87.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_13\", 0.0, \"cinematic\", \"\")", 87.3);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_12_Rot_2\", 0.0)",87.3);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_12_Rot_1\", 0.0)",87.3);
	
	--Final Decision
	execScriptDelayed("setCinematicPlayerState(true,\"final_decision_cinematic\")", 87.3);
	execScriptDelayed("startTransmission(8.5)", 89);
	execScriptDelayed("activateSubtitles(29);",89);
	execScriptDelayed("activateSubtitles(30);",94);
	execScriptDelayed("deactivateSubtitles();",97.5);
end

function temp_final_decision()
	setInBlackScreen(0.25);
	move("The Player", VEC3(-8.850, 0.044, -4.275),VEC3(-8.850, 0.044, -1));
	setCinematicPlayerState(true,"remove_inhibitor_tutorial");

	execScriptDelayed("setOutBlackScreen(0.25);",0.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Plane_13\", 0.0, \"cinematic\", \"\")", 0.3);
	execScriptDelayed("setCinematicPlayerState(true,\"final_decision_cinematic\")", 0.3);

	--execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_12_Rot_2\", 0.0)",0.3);
	--execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Plane_12_Rot_1\", 0.0)",0.3);
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