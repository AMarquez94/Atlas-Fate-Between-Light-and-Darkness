#include "mcv_platform.h"
#include "game_core_skeleton.h"
#include "render/mesh/mesh_loader.h"
#include "utils/data_saver.h"
#include "cal3d2engine.h"
#include "ctes.h"
#include "utils/dirent.h"
#include "cal3d/cal3d.h"

#pragma comment(lib, "cal3d.lib" )

// ----------------------------------------------
class CGameCoreSkeletonResourceClass : public CResourceClass {
public:
  CGameCoreSkeletonResourceClass() {
    class_name = "Skeletons";
    extensions = { ".skeleton" };
  }
  IResource* create(const std::string& name) const override {
    dbg("Creating skelton resource %s\n", name.c_str());
    CGameCoreSkeleton* res = new CGameCoreSkeleton(name);
    bool is_ok = res->create(name);
    assert(is_ok);
    return res;
  }
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CGameCoreSkeletonResourceClass>, use this function:
template<>
const CResourceClass* getResourceClassOf<CGameCoreSkeleton>() {
  static CGameCoreSkeletonResourceClass the_resource_class;
  return &the_resource_class;
}

struct TSkinVertex {
  VEC3 pos;
  VEC3 normal;
  VEC2 uv;
  VEC4 tangent;

  uint8_t bone_ids[4];
  uint8_t bone_weights[4];    // 0.255   -> 0..1
};

// ------------------------------------------------------------
void showCoreBoneRecursive(CalCoreSkeleton* core_skel, int bone_id ) {
  assert(core_skel);
  if (bone_id == -1)
    return;
  CalCoreBone* cb = core_skel->getCoreBone(bone_id);
  char buf[128];
  sprintf(buf, "%s [Id:%d]", cb->getName().c_str(), bone_id);
  ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Appearing);
  if (ImGui::TreeNode(buf)) {
    auto children = cb->getListChildId();
    for (auto it : children) {
      showCoreBoneRecursive(core_skel, it);
    }
    ImGui::TreePop();
  }
}

// ------------------------------------------------------------
void CGameCoreSkeleton::debugInMenu() {
  auto* core_skel = getCoreSkeleton();
  
  if (ImGui::TreeNode("Hierarchy Bones")) {
    auto& core_bones = core_skel->getVectorRootCoreBoneId();
    for (auto& bone_id : core_bones)
      showCoreBoneRecursive(getCoreSkeleton(), bone_id);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("All Bones")) {
    auto& core_bones = core_skel->getVectorCoreBone();
    for (int bone_id = 0; bone_id < core_bones.size(); ++bone_id) {
      CalCoreBone* cb = core_skel->getCoreBone(bone_id);
      if (ImGui::TreeNode(cb->getName().c_str())) {
        ImGui::LabelText( "ID", "%d", bone_id );
        if (ImGui::SmallButton("Show Axis"))
          bone_ids_to_debug.push_back(bone_id);

		CalVector pos = cb->getTranslationAbsolute();
		ImGui::LabelText("Pos", "%f %f %f", pos.x, pos.y, pos.z);

        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }

  ImGui::DragFloat("Debug Bones Scale", &bone_ids_debug_scale, 0.01f, 0.1f, 5.0f);
}

// ------------------------------------------------------------
bool CGameCoreSkeleton::convertCalCoreMesh2RenderMesh(CalCoreMesh* cal_mesh, const std::string& ofilename ) {

  CMemoryDataSaver mds_vtxs;
  CMemoryDataSaver mds_idxs;

  auto nbones = this->getCoreSkeleton()->getVectorCoreBone().size();
  auto nsubmeshes = cal_mesh->getCoreSubmeshCount();
 
  // Compute total vertexs counting all materials
  int  total_vtxs = 0;
  int  total_faces = 0;
  for (int idx_sm = 0; idx_sm < nsubmeshes; ++idx_sm) {
    CalCoreSubmesh* cal_sm = cal_mesh->getCoreSubmesh(idx_sm);
    total_vtxs += cal_sm->getVertexCount();
    total_faces += cal_sm->getFaceCount();
	cal_sm->enableTangents(idx_sm, true);
  }

  // For each submesh ( each material...);
  int  acc_vtxs = 0;
  for (int idx_sm = 0; idx_sm < nsubmeshes; ++idx_sm) {

    CalCoreSubmesh* cal_sm = cal_mesh->getCoreSubmesh(idx_sm);

    // Copy The vertexs
    auto& cal_vtxs = cal_sm->getVectorVertex();
    auto num_vtxs = cal_sm->getVertexCount();

    // Access to the first texture coordinate set
    auto& cal_all_uvs = cal_sm->getVectorVectorTextureCoordinate();
    if (cal_all_uvs.empty()) {
      //fatal("The skin mesh does not have texture coords!\n");
      //return false;
      // Create some fake/empty tex coords
      cal_all_uvs.resize(1);
      cal_all_uvs[0].resize(num_vtxs);
    }
    auto& cal_uvs0 = cal_all_uvs[0];

	bool bal = cal_sm->isTangentsEnabled(0);
	auto& call_all_tng = cal_sm->getVectorVectorTangentSpace();

	if (call_all_tng[0].empty() && call_all_tng[1].empty()) {

		call_all_tng.resize(1);
		call_all_tng[0].resize(num_vtxs);
	}

	auto& cal_tan0 = call_all_tng[0];
	if (cal_tan0.empty()) cal_tan0 = call_all_tng[1];
	
   // std::vector<VEC4> tangents = computeTangent(cal_sm);
    // Process the vtxs
    for (int vid = 0; vid < num_vtxs; ++vid) {
      CalCoreSubmesh::Vertex& cal_vtx = cal_vtxs[vid];

      // Prepare a clean vertexs
      TSkinVertex skin_vtx;
      memset(&skin_vtx, 0x00, sizeof(TSkinVertex));

      // Pos & Normal
      skin_vtx.pos = Cal2DX(cal_vtx.position);
      skin_vtx.normal = Cal2DX(cal_vtx.normal);
      skin_vtx.tangent.x = cal_tan0[vid].tangent.x;
      skin_vtx.tangent.y = cal_tan0[vid].tangent.y;
      skin_vtx.tangent.z = cal_tan0[vid].tangent.z;
      skin_vtx.tangent.w = cal_tan0[vid].crossFactor;

      // Texture coords
      skin_vtx.uv.x = cal_uvs0[vid].u;
      skin_vtx.uv.y = cal_uvs0[vid].v;

      // Weights...
      int total_weight = 0;
      for (size_t ninfluence = 0; ninfluence < cal_vtx.vectorInfluence.size() && ninfluence < 4; ++ninfluence) {
        auto cal_influence = cal_vtx.vectorInfluence[ninfluence];
        assert(cal_influence.boneId < MAX_SUPPORTED_BONES);
        skin_vtx.bone_ids[ninfluence] = (uint8_t)(cal_influence.boneId);
        assert(skin_vtx.bone_ids[ninfluence] < nbones);

        // Convert cal3d influence from 0..1 to a char from 0..255
        skin_vtx.bone_weights[ninfluence] = (uint8_t)(255.f * cal_influence.weight);
         
        total_weight += skin_vtx.bone_weights[ninfluence];
      }

      // Confirm the sum of rounded weights are still 1  = > 255
      int err = 255 - total_weight;
      if (err != 0) {
        assert(err > 0);
        skin_vtx.bone_weights[0] += err;
      }

      mds_vtxs.write(skin_vtx);
    }

    // Process the faces
    auto num_faces = cal_sm->getFaceCount();
    const auto& cal_faces = cal_sm->getVectorFace();
    for (auto& face : cal_faces) {
      static int lut[3] = { 0,2,1 };
      for (int i = 0; i < 3; ++i) {
        int my_i = lut[i];      // swap face culling

        uint32_t vertex_id = face.vertexId[my_i];

        // The second ground of vertexs, have the face_id not starting in the zero
        // it starts in the previous count of vertex we have defined
        // VertexSubMesh0... | VertexSubMesh1 ... |...
        vertex_id += acc_vtxs;

        // Cal is defined to be 32 bits per index
        if (total_vtxs > 65535) {
          mds_idxs.write((uint32_t)vertex_id);   // 4 bytes per index
        }
        else {
          mds_idxs.write((uint16_t)vertex_id);   // 2 bytes per index
        }
      }
    }

    // Keep the total accumulated vertexs
    acc_vtxs += num_vtxs;
  }

  TMeshLoader mesh_io;
  auto& header = mesh_io.header;
  header.bytes_per_idx = total_vtxs > 65535 ? sizeof(uint32_t) : sizeof(uint16_t);
  header.bytes_per_vtx = sizeof(TSkinVertex);
  header.num_indices = total_faces * 3;
  header.num_vertexs = total_vtxs;
  header.primitive_type = CRenderMesh::TRIANGLE_LIST;
  strcpy(header.vertex_type_name, "PosNUvTanSkin");

  mesh_io.vtxs = mds_vtxs.buffer;
  mesh_io.idxs = mds_idxs.buffer;
  
  // Map each subgroup from cal3d to one render from us
  // mesh_io.subgroups.resize(nsubmeshes);
  // ...

  CFileDataSaver fds(ofilename.c_str());
  mesh_io.save(fds);

  return true;
}

std::vector<VEC4> CGameCoreSkeleton::computeTangent(CalCoreSubmesh* cal_sm) {

    auto num_vtxs = cal_sm->getVertexCount();
    auto& cal_vtxs = cal_sm->getVectorVertex();

    const auto& cal_faces = cal_sm->getVectorFace();
    auto& cal_all_uvs = cal_sm->getVectorVectorTextureCoordinate();

    std::vector<VEC3> tan1;
    tan1.resize(num_vtxs);

    std::vector<VEC3> tan2;
    tan2.resize(num_vtxs);

    std::vector<VEC4> tangent;
    tangent.resize(num_vtxs);

    for (auto& face : cal_faces) {

        uint32_t i1 = face.vertexId[0];
        uint32_t i2 = face.vertexId[1];
        uint32_t i3 = face.vertexId[2];

        CalCoreSubmesh::Vertex& v1 = cal_vtxs[i1];
        CalCoreSubmesh::Vertex& v2 = cal_vtxs[i2];
        CalCoreSubmesh::Vertex& v3 = cal_vtxs[i3];

        CalCoreSubmesh::TextureCoordinate& w1 = cal_all_uvs[0][i1];
        CalCoreSubmesh::TextureCoordinate& w2 = cal_all_uvs[0][i2];
        CalCoreSubmesh::TextureCoordinate& w3 = cal_all_uvs[0][i3];

        float x1 = v2.position.x - v1.position.x;
        float x2 = v3.position.x - v1.position.x;
        float y1 = v2.position.y - v1.position.y;
        float y2 = v3.position.y - v1.position.y;
        float z1 = v2.position.z - v1.position.z;
        float z2 = v3.position.z - v1.position.z;

        float s1 = w2.u - w1.u;
        float s2 = w3.u - w1.u;
        float t1 = w2.v - w1.v;
        float t2 = w3.v - w1.v;

        float r = 1.0F / (s1 * t2 - s2 * t1);
        VEC3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
            (t2 * z1 - t1 * z2) * r);
        VEC3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
            (s1 * z2 - s2 * z1) * r);

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    for (long a = 0; a < num_vtxs; a++)
    {
        CalCoreSubmesh::Vertex& v1 = cal_vtxs[a];
        const VEC3 n = VEC3(v1.normal.x, v1.normal.y, v1.normal.z);
        const VEC3 t = tan1[a];

        // Gram-Schmidt orthogonalize
        VEC3 xyz = t - n * n.Dot(t);
        xyz.Normalize();

        // Calculate handedness
        VEC3 t_comp = n.Cross(t);
        float w = (t_comp.Dot(tan2[a]) < 0.0F) ? -1.0F : 1.0F;
        tangent[a] = VEC4(xyz.x, xyz.y, xyz.z, w);
    }

    return tangent;
}

// ------------------------------------------------------------
bool CGameCoreSkeleton::create(const std::string& res_name) {

  // Load an initial scene
  json json = loadJson(res_name);

  std::string name = json["name"];
  root_path = "data/skeletons/" + name + "/";

  CalLoader::setLoadingMode(LOADER_ROTATE_X_AXIS);

  // Read the core skeleton
  std::string csf = root_path + name + ".csf";
  bool is_ok = loadCoreSkeleton(csf);
  if (!is_ok)
    return false;

  DIR *dr;
  std::string cmfpath = "data/skeletons/" + name;
  dr = opendir(cmfpath.c_str());

  struct dirent *drnt;
  drnt = readdir(dr);
  std::size_t found;
  
  do {
	  std::string actual_file = drnt->d_name;
	  found = actual_file.find(".cmf");
	  if (found != std::string::npos) {
		  int mesh_id = loadCoreMesh(cmfpath + "/" + actual_file);
		  if (mesh_id < 0)
			  return false;
		  std::string skin_mesh_file = root_path + actual_file.replace(actual_file.find(".cmf"), actual_file.length(),"") + ".mesh";
		  convertCalCoreMesh2RenderMesh(getCoreMesh(mesh_id), skin_mesh_file);
		  std::remove(skin_mesh_file.replace(skin_mesh_file.find(".mesh"), skin_mesh_file.length(), ".cmf").c_str());
	  }
	  drnt = readdir(dr);
  } while (drnt != NULL);
  
  // Read all anims
  auto& anims = json["anims"];
  for (auto it = anims.begin(); it != anims.end(); ++it) {
    assert(it->is_object());

    auto& anim = *it;
    std::string anim_name = anim["name"];
    std::string caf = root_path + anim_name + ".caf";
    int anim_id = loadCoreAnimation(caf, anim_name);
    if (anim_id < 0)
      continue;

	// read other metadata associated to the anim
	if (anim.count("callbacks")) {
		auto& j_callbacks = anim["callbacks"];
		for (auto it = j_callbacks.begin(); it != j_callbacks.end(); ++it) {
			AnimationCallback *new_callback = new AnimationCallback();
			float timeToCall = it.value().value("time_to_call", 0.0f);
			std::string function_to_call = it.value().value("function_to_call", "");
			new_callback->luaFunction = function_to_call;
			new_callback->animationName = anim_name;
			CalCoreAnimation *core_anim = getCoreAnimation(anim_id);
			core_anim->registerCallback(new_callback, timeToCall, false);
		}
	}

	if (anim.count("animation_completed_callback")) {

		AnimationCallback *new_callback = new AnimationCallback();
		new_callback->luaFunction = "";
		new_callback->animationName = anim_name;
		CalCoreAnimation *core_anim = getCoreAnimation(anim_id);
		core_anim->registerCallback(new_callback, 9999999.99f, true);		
	}

    if (anim.count("audio_callbacks")) {
        auto& j_audio_callbacks = anim["audio_callbacks"];
        for (auto it = j_audio_callbacks.begin(); it != j_audio_callbacks.end(); it++) {
            AnimationAudioCallback *new_audio_callback = new AnimationAudioCallback();
            float timeToCall = it.value().value("time_to_call", 0.0f);
            std::string audio_name = it.value().value("audio_name", "");
            bool relative_to_player = it.value().value("relative_to_player", true);
            new_audio_callback->audioName = audio_name;
            CalCoreAnimation *core_anim = getCoreAnimation(anim_id);
            core_anim->registerCallback(new_audio_callback, timeToCall, false);
        }
    }
  }

  // Array of bone ids to debug (auto conversion from array of json to array of ints)
  if(json["bone_ids_to_debug"].is_array())
    bone_ids_to_debug = json["bone_ids_to_debug"].get< std::vector< int > >();

  if (json.count("lookat_corrections")) {

	  auto& jcorrs = json["lookat_corrections"];
	  assert(jcorrs.is_array());
	  for (int i = 0; i < jcorrs.size();i++) {
		  TBoneCorrection c;
		  c.load(jcorrs[i]);

		  c.bone_id = getCoreSkeleton()->getCoreBoneId(c.bone_name);
		  lookat_corrections.push_back(c);
	  }

  }

  return true;
}

void CalculateTangentArray(long vertexCount, const VEC3 *vertex, const VEC3 *normal, 
							const VEC2 *texcoord, long triangleCount, const CalCoreSubmesh::Face *triangle, VEC4 *tangent)
{
	VEC3 *tan1 = new VEC3[vertexCount * 2];
	VEC3 *tan2 = tan1 + vertexCount;
	ZeroMemory(tan1, vertexCount * sizeof(VEC3) * 2);

	for (long a = 0; a < triangleCount; a++)
	{
		long i1 = triangle->vertexId[0];
		long i2 = triangle->vertexId[1];
		long i3 = triangle->vertexId[2];

		const VEC3& v1 = vertex[i1];
		const VEC3& v2 = vertex[i2];
		const VEC3& v3 = vertex[i3];

		const VEC2& w1 = texcoord[i1];
		const VEC2& w2 = texcoord[i2];
		const VEC2& w3 = texcoord[i3];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		VEC3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		VEC3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;

		triangle++;
	}

	for (long a = 0; a < vertexCount; a++)
	{
		const VEC3& n = normal[a];
		const VEC3& t = tan1[a];

		// Gram-Schmidt orthogonalize
		VEC3 temp_vec = (t - n * n.Dot(t));
		temp_vec.Normalize();
		tangent[a] = VEC4(temp_vec.x, temp_vec.y, temp_vec.z, 0);
		// Calculate handedness
		tangent[a].w = (n.Cross(t).Dot(tan2[a]) < 0.0F) ? -1.0F : 1.0F;
	}

	delete[] tan1;
}
