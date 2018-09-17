#pragma once

// To de defined in the future
struct TEntityParseContext;

// CHandleManager is a class
class CHandleManager;

// Aqui estamos diciendo que CObjectManager no es una clase
// sino un template que dado un tipo TObj genera clases
template< class TObj >
class CObjectManager;

// There is exists somewhere a function which returns
// the handle manager of type TObj
template<typename TObj>
CObjectManager<TObj>* getObjectManager();

// 32 bits -> 
class CHandle {

public:

    static const uint32_t num_bits_type = 7;      // 128 different component types 
    static const uint32_t num_bits_index = 12;
    static const uint32_t num_bits_age = 32 - num_bits_type - num_bits_index;
    static const uint32_t max_types = 1 << num_bits_type;   // 128

    // Ctor vacio. Todo ceros es un handle invalido
    CHandle() : type(0), external_index(0), age(0) {}

    // Ctor dando los 3 valores.
    CHandle(uint32_t new_type
        , uint32_t new_external_index
        , uint32_t new_age)
        : type(new_type)
        , external_index(new_external_index)
        , age(new_age) {}

    // Contruir un handle a partir de una direccion de un obj
    // Solo va a devolver un handle valido si la direccion
    // pertenece al manager de ese tipo de objetos
    template< class TObj >
    CHandle(TObj* obj_addr) {
        auto hm = getObjectManager<std::remove_const<TObj>::type>();
        *this = hm->getHandleFromAddr(obj_addr);
    }

    // Acceso read-only a los datos.
    uint32_t getType()          const { return type; }
    uint32_t getExternalIndex() const { return external_index; }
    uint32_t getAge()           const { return age; }
    const char* getTypeName()   const;

    bool isValid() const;

    // Operador de 'iguales' -> Tiene que coincidir todo
    bool operator==(CHandle h) const {
        return type == h.type
            && external_index == h.external_index
            && age == h.age;
    }

    bool operator!=(CHandle h) const {
        return !(*this == h);
    }

    uint32_t asUnsigned() const { return *(uint32_t*)this; }
    std::string asString() const { return std::to_string(asUnsigned()); };
    void fromUnsigned(uint32_t data) { *(uint32_t*)this = data; }
    //TODO: fromString?
    // When we want to store/retrieve the handle in a void* 
    // void* is not the addr of the real object, just the handle 
    void* asVoidPtr() const { return (uintptr_t*)(size_t)asUnsigned(); }
    void fromVoidPtr(void* user_data) { fromUnsigned((uint32_t)(size_t)(user_data)); }

    // Create & Destroy
    template<class TObj>
    void create() {
        auto hm = getObjectManager<TObj>();
        *this = hm->createHandle();
    }
    void destroy();


    // Automatic cast.
    template<class TObj>
    operator TObj*() const {
        // std::remove_const<T>::type returns the TObj without const
        // Used when TObj is const*. We want the manager of <TLife> objs
        // not the manager of <const TLife>, so we use the remove_constw
        auto hm = getObjectManager< std::remove_const<TObj>::type >();
        return hm->getAddrFromHandle(*this); 
    }

    void    setOwner(CHandle new_owner);
    CHandle getOwner() const;

    // 
    void    debugInMenu();
    void    renderDebug();
    void    load(const json& j, TEntityParseContext& ctx);

    template< class TMsg >
    void sendMsg(const TMsg& msg);

private:

    // Guardar N bits para cada members, con la intencion de que objeto 
    // CHandle ocupe 32 bits
    uint32_t type : num_bits_type;  // Que tipo de objeto representa
    uint32_t external_index : num_bits_index; // Sirve para encontrar el objeto de verdad
    uint32_t age : num_bits_age;   // Para descartar versiones antiguas de los objetos

};

