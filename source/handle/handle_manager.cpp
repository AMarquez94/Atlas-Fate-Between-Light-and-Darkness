#include "mcv_platform.h"
#include "handle_manager.h"

// Zero is predefined as invalid component
uint32_t CHandleManager::next_type_of_handle_manager = 1;
CHandleManager* CHandleManager::all_managers[CHandle::max_types];
std::map<std::string, CHandleManager*> CHandleManager::all_manager_by_name;

CHandleManager* CHandleManager::predefined_managers[CHandle::max_types];
uint32_t        CHandleManager::npredefined_managers = 0;

void CHandleManager::destroyAllPendingObjects() {
  for (uint32_t i = 1; i < getNumDefinedTypes(); ++i)
    all_managers[i]->destroyPendingObjects();
}

uint32_t CHandleManager::getNumDefinedTypes() {
  return next_type_of_handle_manager;
}

CHandleManager* CHandleManager::getByType(uint32_t type) {
  return all_managers[type];
}

CHandleManager* CHandleManager::getByName(const char* name) {
  auto it = all_manager_by_name.find(name);
  if (it == all_manager_by_name.end())
    return nullptr;
  return it->second;
}

// ---------------------------------------
CHandle CHandleManager::createHandle() {
  // Yo tengo que ser valido..
  assert(type != 0);

  const uint32_t num_objs_capacity = capacity();

  // Tengo que tener libres
  assert(next_free_handle_ext_index != invalid_index);
  assert(num_objs_used < num_objs_capacity);
  uint32_t external_idx = next_free_handle_ext_index;
  auto& ed = external_to_internal[ external_idx ];

  // Actualizar mi tabla internal para poder referenciar un objeto
  // de los de verdad asociado a este handle. El nuevo objeto esta
  // al final de los objetos 'usandose'
  ed.internal_index = num_objs_used;

  // Para recuperar la external info dada una internal_index
  internal_to_external[ed.internal_index] = external_idx;

  //// Llamar al ctor del objeto usando la zona de memoria que le he 
  //// asignado
  createObj(ed.internal_index);

  // One more object in use
  ++num_objs_used;

  // Actualizar donde esta el siguiente libre, para la proxima vez 
  // que cree un objeto
  next_free_handle_ext_index = ed.next_external_index;
  assert(next_free_handle_ext_index != invalid_index || fatal("We run out of objects of type %s. Currently set to %d\n", getName(), capacity()));

  // Sacar de la cadena de handles a borrar
  ed.next_external_index = invalid_index;

  // Borrar el antiguo handle que pudiese haber
  //ed.current_owner = CHandle();

  return CHandle(type, external_idx, ed.current_age);
}

// ---------------------------------------------
void CHandleManager::destroyHandle(CHandle h) {

  // Confirmar que el handle es valido
  if (!isValid(h))
    return;

  // Guardarlo en la lista de objetos pendientes
  // de destruir
  objs_to_destroy.push_back(h);

  // Acceder a la tabla y actualiza la current
  // age para invalidar el handle
  auto external_index = h.getExternalIndex();
  auto& ed = external_to_internal[external_index];
  ed.current_age++;
}

// Now in a safe moment destroy all handles.
void CHandleManager::destroyPendingObjects() {

  for (auto h : objs_to_destroy) {
    auto external_index = h.getExternalIndex();
    auto& ed = external_to_internal[ external_index ];

    auto internal_index = ed.internal_index;

    assert(num_objs_used > 0);

    // Restore the previous current age
    // to be able to recover the CHandle from 
    // and object addr during the scope of the
    // dtor. For example, the RenderStaticMesh
    // uses it's own handle as id in the render
    // manager
    ed.current_age--;

    // Llamar al dtor del objecto real #1
    destroyObj(internal_index);

    // Invalidate the age again to the correct
    // value
    ed.current_age++;

    // Esta entrada ya no apunta a un objeto valido de la tabla
    ed.internal_index = invalid_index;

    assert(last_free_handle_ext_index != invalid_index);
    auto& last_free_ed = external_to_internal[ last_free_handle_ext_index ];
    assert(last_free_ed.next_external_index == invalid_index);

    // El antiguo 'mas viejo' pasa a ser el penultimo 'mas viejo'
    last_free_ed.next_external_index = external_index;

    // El handle borrado pasa a ser el ultimo handle a borrar
    last_free_handle_ext_index = external_index;

    // Ya deberia de ser así, hecho en el createHandle
    assert(ed.next_external_index == invalid_index);

    // --------------------------
    // Compactar los objetos para que sigan siendo 'continuos' 
    // en memoria
    // Estoy borrando el último objeto compactado?, pq si es asi no tengo nada que compactar...
    uint32_t internal_idx_of_last_valid_object = num_objs_used - 1;
    if (internal_index < internal_idx_of_last_valid_object) {   // No!!!
      moveObj(internal_idx_of_last_valid_object, internal_index);

      // Actualizar la tabla que me dice que external_idx tiene
      // cada internal idx
      // Basicamente me traigo el valor del objeto q estoy moviendo
      auto moved_object_external_index = internal_to_external[internal_idx_of_last_valid_object];
      internal_to_external[internal_index] = moved_object_external_index;

      // Actualizar tambien la tabla external para decir donde esta
      // ahora el last object que acabamos de mover
      auto& moved_object_ed = external_to_internal[ moved_object_external_index ];
      moved_object_ed.internal_index = internal_index;
    }

    // Tenemos un objecto valido menos
    num_objs_used--;
  }

  // All pending objects have been destroyed
  objs_to_destroy.clear();
}

// ---------------------------------------------
void    CHandleManager::setOwner(CHandle who, CHandle new_owner) {
  assert(who.isValid());
  auto& ed = external_to_internal[ who.getExternalIndex() ];
  ed.current_owner = new_owner;
}

CHandle CHandleManager::getOwner(CHandle who) {
  if (!who.isValid())
    return CHandle();
  auto& ed = external_to_internal[who.getExternalIndex()];
  return ed.current_owner;
}

void CHandleManager::debugInMenu(CHandle who) {
  if (!who.isValid())
    return;
  auto& ed = external_to_internal[who.getExternalIndex()];
  debugInMenuObj(ed.internal_index);
}

void CHandleManager::load(CHandle who, const json& j, TEntityParseContext& ctx) {
  if (!who.isValid())
    return;
  auto& ed = external_to_internal[who.getExternalIndex()];
  loadObj(ed.internal_index, j,ctx);
}

// -----------------------------------------
void CHandleManager::dumpInternals() const {
  dbg(" IDX       Int      Next      Age      External.  NextFree:%d LastFree:%d Used %d/%d\n"
    , next_free_handle_ext_index, last_free_handle_ext_index, size(), capacity());
  for (uint32_t i = 0; i < external_to_internal.size(); ++i) {
    const auto& ed = external_to_internal[ i ];
    uint32_t internal_idx = 0; // internal_to_external[i];
    dbg("%4d  %08x  %08x %08x      %08x\n"
      , i
      , ed.internal_index
      , ed.next_external_index
      , ed.current_age
      , internal_idx);
  }
}



