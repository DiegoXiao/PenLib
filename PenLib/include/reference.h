#ifndef PEN_LIB_REFERENCE_H_
#define PEN_LIB_REFERENCE_H_
#include <Windows.h>
#include "shared_pointer.h"
// 引用计数接口
struct ReferenceInterface {
  // 增加引用计数
  virtual long __stdcall AddRef() = 0;
  // 减少引用计数
  virtual long __stdcall Release() = 0;
};
// 聚合接口
struct AggregationInterface {
  virtual long __stdcall SetRef(ReferenceInterface* ref) = 0;
  virtual long __stdcall Dispose() = 0;
};
typedef SharedPointer<ReferenceInterface> ReferencePtr;
// 引用计数的实现
template<class T>
class Reference : public T {
 public:
  // 构造函数 
  Reference() : reference_(0) {}
  // 析构函数
  virtual ~Reference() {}
  // 增加引用计数
  virtual long __stdcall AddRef() {
    long reference =  InterlockedIncrement(&reference_);
    return reference;
  }
  // 减少引用计数
  virtual long __stdcall Release() {
    long reference = InterlockedDecrement(&reference_);
    if (reference == 0)
      delete this;
    return reference;
  }
  // 引用计数
  volatile long reference_;
};
template<class T>
class Reference2 : public T {
public:
  // 构造函数 
  Reference2() : reference_(0) {}
  // 析构函数
  virtual ~Reference2() {}
  // 增加引用计数
  virtual unsigned long __stdcall AddRef() {
    unsigned long reference =  InterlockedIncrement(&reference_);
    return reference;
  }
  // 减少引用计数
  virtual unsigned long __stdcall Release() {
    unsigned long reference = InterlockedDecrement(&reference_);
    if (reference == 0) {
      delete this;
    }
    return reference;
  }
  // 引用计数
  volatile long reference_;
};
template<class T>
class Reference3 : public T {
public:
  // 构造函数 
  Reference3() : reference_(0) {}
  // 析构函数
  virtual ~Reference3() {}
  // 增加引用计数
  virtual long __stdcall AddRef() {
    return ++reference_;
  }
  // 减少引用计数
  virtual long __stdcall Release() {
    long reference = --reference_;
    if (reference == 0) {
      delete this;
    }
    return reference;
  }
  // 引用计数
  long reference_;
};
template<class T>
class Aggregation : public T {
public:
  Aggregation() : reference_(0), outer_(nullptr) {}
  virtual ~Aggregation() {}
  virtual long __stdcall AddRef() {
    if (outer_ == nullptr) {
      return InterlockedIncrement(&reference_);
    } 
    else {
      return outer_->AddRef();
    }
  }
  long __stdcall Release() {
    if (outer_ == nullptr) {
      long reference = InterlockedDecrement(&reference_);
      if (reference == 0) {
        Dispose();
      }
      return reference;
    } 
    else {
      return outer_->Release();
    }
  }
  long __stdcall SetRef(ReferenceInterface* ref) {
    outer_ = ref;
    return 0;
  }
  long __stdcall Dispose() {
    delete this;
    return 0;
  }
  // 引用计数
  long reference_;
  // 外部计数
  ReferenceInterface* outer_;
};
#ifndef NOCOMREF
template<class T>
class ComRef : public T {
public:
  ComRef() : reference_(0) {}
  virtual ~ComRef() {}
  STDMETHODIMP QueryInterface(REFIID riid, void ** ppv) {
    *ppv = nullptr;
    if (riid == __uuidof(IUnknown) || 
      riid == __uuidof(T)) *ppv = this;
    if (*ppv == nullptr) {
      return ResultFromScode(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
  }
  STDMETHODIMP_(ULONG) AddRef(void) {
    ULONG reference =  InterlockedIncrement(&reference_);
    return reference;
  }
  STDMETHODIMP_(ULONG) Release(void) {
    ULONG reference = InterlockedDecrement(&reference_);
    if (reference == 0) {
      delete this;
    }
    return reference;
  }
  // 引用计数
  volatile long reference_;
};
#endif
#ifdef _DEBUG
#ifndef TBB_MALLOC
#define new DEBUG_NEW
#endif
#endif
template<typename InterfaceType, typename CoClassType> 
long CreateInterface(InterfaceType** face) {
  if (face == nullptr) {
    return -1;
  }
  try {
    *face = new CoClassType;
    (*face)->AddRef();
  } catch (...) {    
    return -1;
  }
  return 0;
}
template<typename InterfaceType, typename CoClassType> 
long CreateInterface2(InterfaceType** face) {
  if (face == nullptr) {
    return -1;
  }
  try {
    *face = new CoClassType;
  } catch (...) {    
    return -1;
  }
  return 0;
}
#ifdef _DEBUG
#ifndef TBB_MALLOC
#undef new
#endif
#endif
#endif // PEN_LIB_REFERENCE_H_