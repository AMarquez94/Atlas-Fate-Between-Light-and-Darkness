#ifndef INC_ENTITY_MSGS_
#define INC_ENTITY_MSGS_

struct IMsgBaseCallback {
  // La interfaz virtual quiere un handle y una direccion al msg
  virtual void sendMsg(CHandle h_recv, const void* msg) = 0;
};

template< typename TComp, typename TMsg >
struct TMsgCallback : public IMsgBaseCallback {

  // La signature de un metodo de la class TComp que recibe
  // como argumento una referencia a una instancia de TMsg
  // y que no devuelve nada.
  typedef void(TComp::*TMethodCallback)(const TMsg&);

  // Una instancia de un puntero a uno de esos metodos
  TMethodCallback method = nullptr;

  TMsgCallback(TMethodCallback new_method) 
  : method(new_method)
  { }

  void sendMsg(CHandle h_recv, const void* generic_msg) {
    TComp* obj_recv = h_recv;
    assert(obj_recv);
    const TMsg* msg = static_cast<const TMsg*>(generic_msg);
    assert(method != nullptr);
    (obj_recv->*method)(*msg);
  }
};

struct TCallbackSlot {
  uint32_t          comp_type;
  IMsgBaseCallback* callback;
};

extern std::unordered_multimap< uint32_t, TCallbackSlot > all_registered_msgs;

template< typename TComp, typename TMsg, typename TMethod >
void registerMsg(TMethod method) {
  std::pair< uint32_t, TCallbackSlot > v;
  v.first = TMsg::getMsgID();
  v.second.comp_type = getObjectManager<TComp>()->getType();
  v.second.callback = new TMsgCallback< TComp, TMsg >(method);
  all_registered_msgs.insert(v);
}

#define DECL_MSG( acomp, amsg, amethod ) \
  registerMsg< acomp, amsg >(&acomp::amethod)

uint32_t getNextUniqueMsgID();

// Each TMsgStruct defining this will get assigned a new uint32_t
// associated and stored to this static method of each struct type
#define DECL_MSG_ID( )                                \
  static uint32_t getMsgID() {                        \
    static uint32_t msg_id = getNextUniqueMsgID();    \
    return msg_id;                                    \
  }

template< class TMsg >
void CHandle::sendMsg(const TMsg& msg) {
  CEntity* e = getObjectManager< std::remove_const<CEntity>::type >()->getAddrFromHandle(*this);
  if (e)
    e->sendMsg(msg);
}

#endif
