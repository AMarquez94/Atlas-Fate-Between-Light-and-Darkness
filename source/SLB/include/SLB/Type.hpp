/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2011 Jose L. Hidalgo Valiño (PpluX)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  
  Jose L. Hidalgo (www.pplux.com)
  pplux@pplux.com
*/



#ifndef __SLB_TYPE__
#define __SLB_TYPE__

#include "lua.hpp"
#include "Debug.hpp"
#include "SPP.hpp"
#include "Manager.hpp"
#include "ClassInfo.hpp"


namespace SLB {
namespace Private {

  // Default implementation
  template<class T>
  struct Type
  {
    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      if (c == 0) luaL_error(L, "Unknown class %s", _TIW(T).name());
      return c;
    }

    static void push(lua_State *L,const T &obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(8,"Push<T=%s>(L=%p, obj =%p)", _TIW(T).name(), L, &obj);
      getClass(L)->push_copy(L, (void*) &obj);
    }

    static T get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(8,"Get<T=%s>(L=%p, pos = %i)", _TIW(T).name(), L, pos);
      T* obj = reinterpret_cast<T*>( getClass(L)->get_ptr(L, pos) );  
      SLB_DEBUG(9,"obj = %p", obj);
      return *obj;
    }

  };

  template<class T>
  struct Type<T*>
  {
    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      if (c == 0) luaL_error(L, "Unknown class %s", _TIW(T).name());
      return c;
    }

    static void push(lua_State *L, T *obj, bool fromConstructor = false)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p (from constructor=%s)",
          _TIW(T).name(),
          obj,
          fromConstructor? "true" : "false" );

      if (obj == 0)
      {
        lua_pushnil(L);
        return;
      }

      /*TODO Change this for TypeInfoWrapper?
      const std::type_info &t_T = _TIW(T);
      const std::type_info &t_obj = _TIW(*obj);
      
      assert("Invalid typeinfo!!! (type)" && (&t_T) );
      assert("Invalid typeinfo!!! (object)" && (&t_obj) );

      if (t_obj != t_T)
      {
        //Create TIW
        TypeInfoWrapper wt_T = _TIW(T);
        TypeInfoWrapper wt_obj = _TIW(*obj);
        // check if the internal class exists...
        ClassInfo *c = SLB::Manager::getInstance(L)->getClass(wt_obj);
        if ( c ) 
        {
          SLB_DEBUG(8,"Push<T*=%s> with conversion from "
            "T(%p)->T(%p) (L=%p, obj =%p)",
            c->getName().c_str(), t_obj.name(), t_T.name(),L, obj);
          // covert the object to the internal class...
          void *real_obj = SLB::Manager::getInstance(L)->convert( wt_T, wt_obj, obj );
          c->push_ptr(L, real_obj, fromConstructor);
          return;
        }
      }*/
      // use this class...  
      getClass(L)->push_ptr(L, (void*) obj, fromConstructor);


    }

    static T* get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      return reinterpret_cast<T*>( getClass(L)->get_ptr(L, pos) );
    }

  };
  
  template<class T>
  struct Type<const T*>
  {
    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      if (c == 0) luaL_error(L, "Unknown class %s", _TIW(T).name());
      return c;
    }

    static void push(lua_State *L,const T *obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p", _TIW(T).name(), obj);
      if (obj == 0)
      {
        lua_pushnil(L);
        return;
      }
      /*
      if (_TIW(*obj) != _TIW(T))
      {
        //Create TIW
        TypeInfoWrapper wt_T = _TIW(T);
        TypeInfoWrapper wt_obj = _TIW(*obj);
        // check if the internal class exists...
        ClassInfo *c = SLB::Manager::getInstance(L)->getClass(wt_obj);
        if ( c ) 
        {
          SLB_DEBUG(8,"Push<const T*=%s> with conversion from "
            "T(%p)->T(%p) (L=%p, obj =%p)",
            c->getName().c_str(), typeid(*obj).name(), _TIW(T).name(),L, obj);
          // covert the object to the internal class...
          const void *real_obj = SLB::Manager::getInstance(L)->convert( wt_T, wt_obj, obj );
          c->push_const_ptr(L, real_obj);
          return;
        }
      }
      */
      getClass(L)->push_const_ptr(L, (const void*) obj);
    }

    static const T* get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      return reinterpret_cast<const T*>( getClass(L)->get_const_ptr(L, pos) );
    }

  };

  template<class T>
  struct Type<const T&>
  {
    static void push(lua_State *L,const T &obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p(const ref)", _TIW(T).name(), &obj);
      Type<const T*>::push(L, &obj);
    }

    static const T& get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      const T* obj = Type<const T*>::get(L,pos);
      //TODO: remove the _TIW(T).getName() and use classInfo :)
      if (obj == 0L) luaL_error(L, "Can not get a reference of class %s", _TIW(T).name());
      return *(obj);
    }

  };
  
  template<class T>
  struct Type<T&>
  {
    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      if (c == 0) luaL_error(L, "Unknown class %s", _TIW(T).name());
      return c;
    }

    static void push(lua_State *L,T &obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p (reference)", _TIW(T).name(), &obj);
      getClass(L)->push_ref(L, (void*) &obj);
    }

    static T& get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      return *(Type<T*>::get(L,pos));
    }

  };

  //--- Specializations ---------------------------------------------------

  template<>
  struct Type<void*>
  {
    static void push(lua_State *L,void* obj)
    {
      SLB_DEBUG_CALL; 
      if (obj == 0) lua_pushnil(L);
      else
      {
        SLB_DEBUG(8,"Push<void*> (L=%p, obj =%p)",L, obj);
        lua_pushlightuserdata(L, obj);
      }
    }

    static void *get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(8,"Get<void*> (L=%p, pos=%i ) =%p)",L, pos, lua_touserdata(L,pos));
      if (lua_islightuserdata(L,pos)) return lua_touserdata(L,pos);
      //TODO: Check here if is an userdata and convert it to void
      return 0;
    }

  };

  // Type specialization for <int>
  template<>
  struct Type<int>
  {
    static void push(lua_State *L, int v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push integer = %d",v);
      lua_pushinteger(L,v);
    }
    static int get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      int v = (int) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get integer (pos %d) = %d",p,v);
      return v;
    }
  };

  template<> struct Type<int&> : public Type<int> {};
  template<> struct Type<const int&> : public Type<int> {};

  // Type specialization for <unsigned int>
  template<>
  struct Type<unsigned int>
  {
    static void push(lua_State *L, unsigned int v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned integer = %d",v);
      lua_pushinteger(L,v);
    }
    static unsigned int get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned int v = static_cast<unsigned int>(lua_tointeger(L,p));
      SLB_DEBUG(6,"Get unsigned integer (pos %d) = %d",p,v);
      return v;
    }

  };

  template<> struct Type<unsigned int&> : public Type<unsigned int> {};
  template<> struct Type<const unsigned int&> : public Type<unsigned int> {};
  

  template<>
  struct Type<long>
  {
    static void push(lua_State *L, long v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push long = %ld",v);
      lua_pushinteger(L,v);
    }
    static long get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      long v = (long) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get long (pos %d) = %ld",p,v);
      return v;
    }

  };

  template<> struct Type<long&> : public Type<long> {};
  template<> struct Type<const long&> : public Type<long> {};
  

  /* unsigned long == unsigned int */
  template<>
  struct Type<unsigned long>
  {
    static void push(lua_State *L, unsigned long v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned long = %lu",v);
      lua_pushnumber(L,v);
    }

    static unsigned long get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned long v = (unsigned long) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get unsigned long (pos %d) = %lu",p,v);
      return v;
    }

  };

  template<> struct Type<unsigned long&> : public Type<unsigned long> {};
  template<> struct Type<const unsigned long&> : public Type<unsigned long> {};
  

  template<>
  struct Type<unsigned long long>
  {
    static void push(lua_State *L, unsigned long long v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned long long = %llu",v);
      lua_pushnumber(L,(lua_Number) v);
    }

    static unsigned long long get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned long long v = (unsigned long long) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get unsigned long long (pos %d) = %llu",p,v);
      return v;
    }
  };

  template<> struct Type<unsigned long long&> : public Type<unsigned long long> {};
  template<> struct Type<const unsigned long long&> : public Type<unsigned long long> {};
  
  // Type specialization for <double>
  template<>
  struct Type<double>
  {
    static void push(lua_State *L, double v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push double = %f",v);
      lua_pushnumber(L,v);
    }
    static double get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      double v = (double) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get double (pos %d) = %f",p,v);
      return v;
    }

  };

  template<> struct Type<double&> : public Type<double> {};
  template<> struct Type<const double&> : public Type<double> {};
  
  // Type specialization for <float>
  template<>
  struct Type<float>
  {
    static void push(lua_State *L, float v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push float = %f",v);
      lua_pushnumber(L,v);
    }

    static float get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      float v = (float) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get float (pos %d) = %f",p,v);
      return v;
    }

  };

  template<> struct Type<float&> : public Type<float> {};
  template<> struct Type<const float&> : public Type<float> {};
  
  
  // Type specialization for <bool>
  template<>
  struct Type<bool>
  {
    static void push(lua_State *L, bool v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push bool = %d",(int)v);
      lua_pushboolean(L,v);
    }
    static bool get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      bool v = (lua_toboolean(L,p) != 0);
      SLB_DEBUG(6,"Get bool (pos %d) = %d",p,v);
      return v;
    }
  };


  template<> struct Type<bool&> : public Type<bool> {};
  template<> struct Type<const bool&> : public Type<bool> {};

  template<>
  struct Type<std::string>
  {
    static void push(lua_State *L, const std::string &v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push const std::string& = %s",v.c_str());
      lua_pushstring(L, v.c_str());
    }

    static std::string get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      const char* v = (const char*) lua_tostring(L,p);
      SLB_DEBUG(6,"Get std::string (pos %d) = %s",p,v);
      return v;
    }

  };

  template<>
  struct Type<const std::string &>
  {
    static void push(lua_State *L, const std::string &v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push const std::string& = %s",v.c_str());
      lua_pushstring(L, v.c_str());
    }

    // let the compiler do the conversion...
    static const std::string get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      const char* v = (const char*) lua_tostring(L,p);
      SLB_DEBUG(6,"Get std::string (pos %d) = %s",p,v);
      return std::string(v);
    }

  };


  // Type specialization for <const char*>
  template<>
  struct Type<const char*>
  {
    static void push(lua_State *L, const char* v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push const char* = %s",v);
      lua_pushstring(L,v);
    }

    static const char* get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      const char* v = (const char*) lua_tostring(L,p);
      SLB_DEBUG(6,"Get const char* (pos %d) = %s",p,v);
      return v;
    }

  };

  template<>
  struct Type<const unsigned char*>
  {
    static void push(lua_State *L, const unsigned char* v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push const unsigned char* = %s",v);
      lua_pushstring(L,(const char*)v);
    }

    static const unsigned char* get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      const unsigned char* v = (const unsigned char*) lua_tostring(L,p);
      SLB_DEBUG(6,"Get const unsigned char* (pos %d) = %s",p,v);
      return v;
    }

  };

}} // end of SLB::Private

#endif
