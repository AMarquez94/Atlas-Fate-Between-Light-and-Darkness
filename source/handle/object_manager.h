#ifndef INC_OBJECT_MANAGER_H_
#define INC_OBJECT_MANAGER_H_

#include "handle_manager.h"
//#include "tbb/tbb.h"

// Declared in module_multithreading
extern bool is_multithreaded_enabled;

// This is a Handle manager which stores objects of
// type TObj
template< class TObj >
class CObjectManager : public CHandleManager
{
  // Linear memory to store all my objects
  std::vector< uint8_t > allocated_memory;

  TObj* objs = nullptr;
  bool multithreaded = false;

  void createObj(uint32_t internal_idx) override {
    TObj* addr_to_use = objs + internal_idx;
    new (addr_to_use) TObj;
  }

  void destroyObj(uint32_t internal_idx) override {
    TObj* addr_to_use = objs + internal_idx;
    // Llamar al dtor de la clase directamente, no llamar
    // al delete pq la memoria no es del sistema
    addr_to_use->~TObj();
  }

  void moveObj(uint32_t src_internal_idx, uint32_t dst_internal_idx) override {
    TObj* src = objs + src_internal_idx;
    TObj* dst = objs + dst_internal_idx;
    // Build a TObj in dst addr using the move semantics
    new (dst) TObj(std::move(*src));
  }

  void debugInMenuObj(uint32_t internal_idx) override {
    TObj* addr_to_use = objs + internal_idx;
    addr_to_use->debugInMenu();
  }

  void renderDebugObj(uint32_t internal_idx) override {
	  TObj * addr_to_use = objs + internal_idx;
	  addr_to_use->renderDebug();
  }

  void loadObj(uint32_t internal_idx, const json& j, TEntityParseContext& ctx) override {
    TObj* addr_to_use = objs + internal_idx;
    addr_to_use->load( j, ctx );
  }

public:

  CObjectManager(const char* new_name) : objs(nullptr) {
    name = new_name;

    // Self registering in the ctor in the array of predefined managers
    CHandleManager::predefined_managers[CHandleManager::npredefined_managers] = this;
    CHandleManager::npredefined_managers++;
  }

  void init(uint32_t max_objects, bool is_multithreaded = false) override {
    CHandleManager::init(max_objects);

    multithreaded = is_multithreaded;
    // Uso un std::vector como facility para pedir
    // memoria lineal de N bytes y que se libere
    // automaticamente en el dtor del manager
    allocated_memory.resize(max_objects * sizeof(TObj));

    // Paso por void para 'borrar' el tipo unsigned char
    objs = static_cast< TObj *>((void*)allocated_memory.data());

    TObj::registerMsgs();
  }

  // --------------------------------------------
  CHandle getHandleFromAddr(TObj* obj_addr) {
    // Obj addr and objs are of the same type
    // We use pointer aritmetic to know how many 'objects' 
    // we are separated 
    auto internal_index = obj_addr - objs;
    if (internal_index >= num_objs_used || internal_index < 0)
      return CHandle();
    auto external_index = internal_to_external[internal_index];
    auto& ed = external_to_internal[ external_index ];
    return CHandle(type, external_index, ed.current_age);
  }

  // --------------------------------------------
  TObj* getAddrFromHandle(CHandle h) {

    if (!h.getType())
      return nullptr;

    // Si no es mi tipo, no eres valido
    if (h.getType() != getType()) {
      fatal("You have request to convert a handle of type %s to "
        "a class of type %s"
        , CHandleManager::getByType(h.getType())->getName()
        , getName()
      );
      return nullptr;
    }

    assert(h.getType() == getType());
    const auto& ed = external_to_internal[ h.getExternalIndex() ];

    // El handle es 'viejo', en esa entrada hay otro objeto
    // ahora
    if (ed.current_age != h.getAge())
      return nullptr;

    return objs + ed.internal_index;
  }

  // ----------------------------------------
  void updateAll(float dt) override {
    assert(objs);

    if (!num_objs_used)
        return;
    /*
    if ( multithreaded && is_multithreaded_enabled ) {
      int nDefThreads = tbb::task_scheduler_init::default_num_threads();
      size_t step = num_objs_used / nDefThreads;
      tbb::parallel_for(size_t(0), size_t(num_objs_used), step, [&, dt](size_t i) {
        objs[i].update(dt); }
      );
    }
    else {
      */
      for (uint32_t i = 0; i < num_objs_used; ++i) 
        objs[i].update(dt);
    //}
  }

  // ---------------------------------------- 
  void renderDebugAll() override {
	  assert(objs);
	  for (uint32_t i = 0; i < num_objs_used; ++i)
		  objs[i].renderDebug();
  }
  
  // -------------------------------
  template< typename TFn >
  void forEach(TFn fn) {
    assert(objs);
    for (uint32_t i = 0; i < num_objs_used; ++i)
      fn(objs + i);
  }

  // -------------------------------
  template< typename TFn >
  void forEachWithExternalIdx(TFn fn) {
    assert(objs);
    for (uint32_t i = 0; i < num_objs_used; ++i)
      fn(objs + i, internal_to_external[i]);
  }

  void debugInMenuAll() {
    assert(objs);

    // Report some usage information about this type of object
    // The ### tells the ImGUI to use the OM%d as Identifier of the tree, not the title text
    // which avoids closing the tree node automatically when the title changes
    char buf[80];
    sprintf(buf, "%s [%d/%d (%dKb)]###OM%d", getName(), (int)size(), (int)capacity(), (int)( allocated_memory.size() >> 10 ), getType());
    if (ImGui::TreeNode(buf)) {
      for (uint32_t i = 0; i < num_objs_used; ++i) {
        ImGui::PushID(i);
        objs[i].debugInMenu();
        ImGui::PopID();
		ImGui::Separator();
      }
      ImGui::TreePop();
    }
  }

  // --------------------------------------------
  //void sendMsg(CHandle h_recv, uint32_t msg_id, const void* msg_data) {
  //  TObj* obj = getAddrFromHandle(h_recv);
  //  obj->sendMsg(msg_id, msg_data);
  //}

};

#define DECL_OBJ_MANAGER( obj_name, obj_class_name ) \
  CObjectManager< obj_class_name > om_ ## obj_class_name(obj_name); \
  template<> \
  CObjectManager< obj_class_name >* getObjectManager<obj_class_name>() { \
    return &om_ ## obj_class_name; \
  }

#endif
