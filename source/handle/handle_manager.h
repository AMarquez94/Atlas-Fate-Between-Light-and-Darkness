#pragma once

#include "handle_def.h"
#include <new>

// -----------------------------------
class CHandleManager {

  static const uint32_t max_total_objects_allowed = 1 << CHandle::num_bits_index;
  static const uint32_t invalid_index = ~0;   // all ones

 // Se accede por external_id via []
  struct TExternalData {
    uint32_t internal_index;      // Donde esta de verdad mi objeto en el array lineal de objetos.
    uint32_t current_age : CHandle::num_bits_age;
    uint32_t next_external_index;
    CHandle  current_owner;
    TExternalData()
      : internal_index(0)
      , current_age(0)
      , next_external_index(0)
    { }
  };

protected:

  // El tipo de objetos que este handle maneja
  uint32_t        type;

  // Accessed using the external_index, gives info in the internal_table
  std::vector<TExternalData>  external_to_internal;

  // Accessed using the internal_index, returns the external index, used
  // when moving the real objects
  std::vector<uint32_t>       internal_to_external;

  // Size of those vector
  uint32_t        num_objs_used;

  uint32_t        next_free_handle_ext_index;
  uint32_t        last_free_handle_ext_index;

  // Handles to be destroyed in a safe moment.
  std::vector< CHandle > objs_to_destroy;

  const char*     name;

  // ---------------------------------------------------------
  // Shared by all managers
  static uint32_t        next_type_of_handle_manager;
  // Array de handle managers para acceder rapidamente por typo de handle
  static CHandleManager* all_managers[CHandle::max_types];
  // Por nombre
  static std::map<std::string, CHandleManager*> all_manager_by_name;


  // ---------------------------------------------
  virtual void createObj(uint32_t internal_idx) = 0;
  virtual void destroyObj(uint32_t internal_idx) = 0;
  virtual void moveObj(uint32_t src_internal_idx, uint32_t dst_internal_idx) = 0;
  virtual void loadObj(uint32_t src_internal_idx, const json& j, TEntityParseContext& ctx) = 0;
  virtual void debugInMenuObj(uint32_t internal_idx) = 0;
  virtual void renderDebugObj(uint32_t internal_idx) = 0;

public:
  // ---------------------------------------------------------
  // Filled during ctor of the ObjectManagers
  static CHandleManager* predefined_managers[CHandle::max_types];
  static uint32_t        npredefined_managers;

public:

  // ---------------------------------------
  CHandleManager()
    : type(0)
    , num_objs_used(0)
    , next_free_handle_ext_index(0)
    , last_free_handle_ext_index(0)
    , name(nullptr)
  { }

  CHandleManager(const CHandleManager&) = delete;

  // ---------------------------------------
  virtual void init(uint32_t max_objects, bool is_multithreaded = false) {
    // Comprobar los limites de handle por el numero de bits que hemos asignado
    assert(max_objects <= max_total_objects_allowed);
    assert(max_objects > 0);

    // Register this as the manager that will handle
    // the type 'type' and has name 'getName()'
    type = next_type_of_handle_manager;
    next_type_of_handle_manager++;
    all_managers[type] = this;
    // Registrarme por nombre
    all_manager_by_name[getName()] = this;

    num_objs_used = 0;

    uint32_t num_objs_capacity = max_objects;

    external_to_internal.resize( num_objs_capacity );
    internal_to_external.resize( num_objs_capacity );

    // Init the external_to_internal table
    int i = 0;
    for( auto& ed : external_to_internal ) {
      ed.current_age = 1;
      ed.internal_index = invalid_index;
      if (i != num_objs_capacity - 1)
        ed.next_external_index = i + 1;
      else
        ed.next_external_index = invalid_index;
      internal_to_external[i] = invalid_index;
      ++i;
    }

    // Primer y ultimo objetos libres al arrancar
    next_free_handle_ext_index = 0;
    last_free_handle_ext_index = num_objs_capacity - 1;
  }

  // ---------------------------------------
  bool isValid(CHandle h) const {
    assert(h.getType() == type);
    assert(h.getExternalIndex() < capacity());
    auto& ed = external_to_internal[ h.getExternalIndex() ];
    return ed.current_age == h.getAge();
  }

  const char* getName() const { return name; }
  uint32_t getType() const { return type; }
  uint32_t size() const { return num_objs_used; }
  uint32_t capacity() const { return (uint32_t) external_to_internal.size(); }

  void destroyPendingObjects();

  // ---------------------------------------
  CHandle createHandle();
  void    destroyHandle(CHandle h);
  void    debugInMenu(CHandle h);
  void    renderDebug(CHandle h);
  void    load(CHandle h, const json& j, TEntityParseContext& ctx);

  // ---------------------------------------------
  // Virtual methods applying to all objects
  virtual void updateAll(float dt) = 0;
  virtual void renderDebugAll() = 0;
  //virtual void renderInDebugAll() = 0;
  virtual void debugInMenuAll() = 0;

  // ---------------------------------------------
  void    setOwner(CHandle who, CHandle new_owner);
  CHandle getOwner(CHandle who);

  // ---------------------------------------
  static CHandleManager* getByType(uint32_t type);
  static CHandleManager* getByName(const char* obj_name);
  static uint32_t getNumDefinedTypes();
  static void destroyAllPendingObjects();
  void dumpInternals() const;

};

