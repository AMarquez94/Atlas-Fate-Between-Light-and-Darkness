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



#ifndef __SLB_EXPORT__
#define __SLB_EXPORT__

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
  #define SLB_WINDOWS 1
  #  if defined( SLB_STATIC_LIBRARY )
  #    define SLB_EXPORT
  #  elif defined( SLB_LIBRARY )
  #    define SLB_EXPORT   __declspec(dllexport)

  #  else
  #    define SLB_EXPORT   __declspec(dllimport)
  #  endif /* SLB_LIBRARY */

#else
  #  define SLB_EXPORT
#endif  

// Specific MSC pragma's
#if defined(_MSC_VER)
#pragma warning( disable: 4251 )
#pragma warning( disable: 4290 )
#pragma warning( disable: 4127 ) // constant expressions
#endif

#endif
