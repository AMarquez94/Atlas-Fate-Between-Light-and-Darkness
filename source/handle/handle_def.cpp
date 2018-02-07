#include "mcv_platform.h"
#include "handle.h"

// ------------------------------------
bool CHandle::isValid() const {
  auto hm = CHandleManager::getByType(type);
  return hm && hm->isValid(*this);
}

void CHandle::destroy() {
  auto hm = CHandleManager::getByType(type);
  if (hm)
    hm->destroyHandle(*this);
}

const char* CHandle::getTypeName() const {
  auto hm = CHandleManager::getByType(type);
  if (hm)
    return hm->getName();
  return "<invalid>";
}

void CHandle::debugInMenu() {
  auto hm = CHandleManager::getByType(type);
  if (hm)
    hm->debugInMenu( *this );
}

void CHandle::load(const json& j, TEntityParseContext& ctx) {
  auto hm = CHandleManager::getByType(type);
  if (hm)
    hm->load(*this, j, ctx);
}

// ------------------------------------
void CHandle::setOwner(CHandle new_owner) {
  auto hm = CHandleManager::getByType(type);
  if (hm)
    hm->setOwner(*this, new_owner);
}

CHandle CHandle::getOwner() const {
  auto hm = CHandleManager::getByType(type);
  if (hm)
    return hm->getOwner(*this);
  return CHandle();
}