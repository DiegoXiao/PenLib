#ifndef PEN_LIB_SHARED_POINTER_H_
#define PEN_LIB_SHARED_POINTER_H_
template <class T>
class SharedPointer {
 public:
  SharedPointer() {
    pointer_ = 0;
  }
  SharedPointer(int null) {
    (void)null;
    pointer_ = 0;
  }
  SharedPointer(T* pointer) {
    pointer_ = pointer;
    if (pointer_ != 0) {
      pointer_->AddRef();
    }  
  }
  SharedPointer(const SharedPointer<T>& pointer) {
    pointer_ = pointer.pointer_;
    if (pointer_ != 0) {
      pointer_->AddRef();
    }  
  }
  ~SharedPointer() {
    if (pointer_) {
      pointer_->Release();
    }
  }
  operator T*() const {
    return pointer_;
  }
  operator size_t() const {
    return (size_t)pointer_;
  }
  T& operator*() const {
    return *pointer_;
  }
#ifndef STD_LIST
  T** operator&() throw() {
    return &pointer_;
  }
#endif
  T* operator->() const {
    return pointer_;
  }
  bool operator!() const {
    return (pointer_ == 0);
  }
  bool operator<(T* pointer) const {
    return pointer_ < pointer;
  }
  bool operator!=(T* pointer) const {
    return !operator==(pointer);
  }
  bool operator==(T* pointer) const {
    return pointer_ == pointer;
  }
  T* operator=(T* pointer) {
    if (*this!=pointer) {
      if (pointer) {
        pointer->AddRef();
      }
      if (pointer_) {
        (pointer_)->Release();
      } 
      pointer_ = pointer;
      return pointer_;
    }
    return *this;
  }
  T* operator=(const SharedPointer<T>& pointer) {
    if (*this != pointer) {
      if (pointer) {
        pointer->AddRef();
      } 
      if (pointer_) {
        (pointer_)->Release();
      }
      pointer_ = pointer.pointer_;
      return pointer_;
    }
    return *this;
  }
  template <typename Q>
  T* operator=(const SharedPointer<Q>& pointer) {
    T* temp = dynamic_cast<T*> (pointer.pointer_);
    if ( *this != temp ) {
      if (pointer) {
        pointer->AddRef();
      }
      if (pointer_) {
        (pointer_)->Release();
      } 
      pointer_ = temp;
      return pointer_;
    }
    return *this;
  }
  void Attach(T* pointer) {
    if (pointer_) {
      pointer_->Release();
    }
    pointer_ = pointer;
  }
  T* Detach() {
    T* pointer = pointer_;
    pointer_ = 0;
    return pointer;
  }
  T* pointer_;
};
#endif //PEN_LIB_SHARED_POINTER_H_