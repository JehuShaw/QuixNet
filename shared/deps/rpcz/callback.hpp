// This file was GENERATED by command:
//     pump.py callback.hpp.pump
// DO NOT EDIT BY HAND!!!

// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: nadavs@google.com <Nadav Samet>

#ifndef RPCZ_CALLBACK_H
#define RPCZ_CALLBACK_H

#include <boost/noncopyable.hpp>

namespace rpcz {

class closure : boost::noncopyable {
 public:
  closure() {}
  virtual ~closure() {}
  virtual void run() = 0;
};

// For args = 0
namespace internal {
class function_closure_0 : public closure {
 public:
  typedef void (*FunctionType)();

  function_closure_0(FunctionType function, bool self_deleting)
    : function_(function), self_deleting_(self_deleting) {}
  virtual ~function_closure_0() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_();
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
};

template <typename Class>
class method_closure_0 : public closure {
 public:
  typedef void (Class::*MethodType)();

  method_closure_0(Class* object, MethodType method,
      bool self_deleting)    : object_(object), method_(method),
      self_deleting_(self_deleting) {}
  virtual ~method_closure_0() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)();
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;
};
}  // namespace internal

inline closure* new_callback(void (*function)()) {
  return new internal::function_closure_0(function, true);
}
inline closure* new_permanent_callback(void (*function)()) {
  return new internal::function_closure_0(function, false);
}

template <typename Class>
inline closure* new_callback(Class* object, void (Class::*method)()) {
  return new internal::method_closure_0<Class>(object, method, true);
}

template <typename Class>
inline closure* new_permanent_callback(Class* object, void (Class::*method)()) {
  return new internal::method_closure_0<Class>(object, method, false);
}

// For args = 1
namespace internal {
template <typename Arg1>
class function_closure_1 : public closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1);

  function_closure_1(FunctionType function, bool self_deleting, Arg1 arg1)
    : function_(function), self_deleting_(self_deleting), arg1_(arg1) {}
  virtual ~function_closure_1() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_);
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
};

template <typename Class, typename Arg1>
class method_closure_1 : public closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1);

  method_closure_1(Class* object, MethodType method, bool self_deleting,
      Arg1 arg1)    : object_(object), method_(method),
      self_deleting_(self_deleting), arg1_(arg1) {}
  virtual ~method_closure_1() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;  Arg1 arg1_;

};
}  // namespace internal

template <typename Arg1>
inline closure* new_callback(void (*function)(Arg1), Arg1 arg1) {
  return new internal::function_closure_1<Arg1>(function, true, arg1);
}
template <typename Arg1>
inline closure* new_permanent_callback(void (*function)(Arg1), Arg1 arg1) {
  return new internal::function_closure_1<Arg1>(function, false, arg1);
}

template <typename Class, typename Arg1>
inline closure* new_callback(Class* object, void (Class::*method)(Arg1),
    Arg1 arg1) {
  return new internal::method_closure_1<Class, Arg1>(object, method, true,
      arg1);
}

template <typename Class, typename Arg1>
inline closure* new_permanent_callback(Class* object,
    void (Class::*method)(Arg1), Arg1 arg1) {
  return new internal::method_closure_1<Class, Arg1>(object, method, false,
      arg1);
}

// For args = 2
namespace internal {
template <typename Arg1, typename Arg2>
class function_closure_2 : public closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2);

  function_closure_2(FunctionType function, bool self_deleting, Arg1 arg1,
      Arg2 arg2)
    : function_(function), self_deleting_(self_deleting), arg1_(arg1),
        arg2_(arg2) {}
  virtual ~function_closure_2() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_, arg2_);
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
};

template <typename Class, typename Arg1, typename Arg2>
class method_closure_2 : public closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2);

  method_closure_2(Class* object, MethodType method, bool self_deleting,
      Arg1 arg1, Arg2 arg2)    : object_(object), method_(method),
      self_deleting_(self_deleting), arg1_(arg1), arg2_(arg2) {}
  virtual ~method_closure_2() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_, arg2_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;  Arg1 arg1_;
  Arg2 arg2_;

};
}  // namespace internal

template <typename Arg1, typename Arg2>
inline closure* new_callback(void (*function)(Arg1, Arg2), Arg1 arg1,
    Arg2 arg2) {
  return new internal::function_closure_2<Arg1, Arg2>(function, true, arg1,
      arg2);
}
template <typename Arg1, typename Arg2>
inline closure* new_permanent_callback(void (*function)(Arg1, Arg2), Arg1 arg1,
    Arg2 arg2) {
  return new internal::function_closure_2<Arg1, Arg2>(function, false, arg1,
      arg2);
}

template <typename Class, typename Arg1, typename Arg2>
inline closure* new_callback(Class* object, void (Class::*method)(Arg1, Arg2),
    Arg1 arg1, Arg2 arg2) {
  return new internal::method_closure_2<Class, Arg1, Arg2>(object, method,
      true, arg1, arg2);
}

template <typename Class, typename Arg1, typename Arg2>
inline closure* new_permanent_callback(Class* object,
    void (Class::*method)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
  return new internal::method_closure_2<Class, Arg1, Arg2>(object, method,
      false, arg1, arg2);
}

// For args = 3
namespace internal {
template <typename Arg1, typename Arg2, typename Arg3>
class function_closure_3 : public closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3);

  function_closure_3(FunctionType function, bool self_deleting, Arg1 arg1,
      Arg2 arg2, Arg3 arg3)
    : function_(function), self_deleting_(self_deleting), arg1_(arg1),
        arg2_(arg2), arg3_(arg3) {}
  virtual ~function_closure_3() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_, arg2_, arg3_);
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3>
class method_closure_3 : public closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3);

  method_closure_3(Class* object, MethodType method, bool self_deleting,
      Arg1 arg1, Arg2 arg2, Arg3 arg3)    : object_(object), method_(method),
      self_deleting_(self_deleting), arg1_(arg1), arg2_(arg2), arg3_(arg3) {}
  virtual ~method_closure_3() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_, arg2_, arg3_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;

};
}  // namespace internal

template <typename Arg1, typename Arg2, typename Arg3>
inline closure* new_callback(void (*function)(Arg1, Arg2, Arg3), Arg1 arg1,
    Arg2 arg2, Arg3 arg3) {
  return new internal::function_closure_3<Arg1, Arg2, Arg3>(function, true,
      arg1, arg2, arg3);
}
template <typename Arg1, typename Arg2, typename Arg3>
inline closure* new_permanent_callback(void (*function)(Arg1, Arg2, Arg3),
    Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  return new internal::function_closure_3<Arg1, Arg2, Arg3>(function, false,
      arg1, arg2, arg3);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3>
inline closure* new_callback(Class* object, void (Class::*method)(Arg1, Arg2,
    Arg3), Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  return new internal::method_closure_3<Class, Arg1, Arg2, Arg3>(object,
      method, true, arg1, arg2, arg3);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3>
inline closure* new_permanent_callback(Class* object,
    void (Class::*method)(Arg1, Arg2, Arg3), Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  return new internal::method_closure_3<Class, Arg1, Arg2, Arg3>(object,
      method, false, arg1, arg2, arg3);
}

// For args = 4
namespace internal {
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
class function_closure_4 : public closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4);

  function_closure_4(FunctionType function, bool self_deleting, Arg1 arg1,
      Arg2 arg2, Arg3 arg3, Arg4 arg4)
    : function_(function), self_deleting_(self_deleting), arg1_(arg1),
        arg2_(arg2), arg3_(arg3), arg4_(arg4) {}
  virtual ~function_closure_4() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_, arg2_, arg3_, arg4_);
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4>
class method_closure_4 : public closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4);

  method_closure_4(Class* object, MethodType method, bool self_deleting,
      Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)    : object_(object),
      method_(method), self_deleting_(self_deleting), arg1_(arg1), arg2_(arg2),
      arg3_(arg3), arg4_(arg4) {}
  virtual ~method_closure_4() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_, arg2_, arg3_, arg4_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;

};
}  // namespace internal

template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline closure* new_callback(void (*function)(Arg1, Arg2, Arg3, Arg4),
    Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
  return new internal::function_closure_4<Arg1, Arg2, Arg3, Arg4>(function,
      true, arg1, arg2, arg3, arg4);
}
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline closure* new_permanent_callback(void (*function)(Arg1, Arg2, Arg3,
    Arg4), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
  return new internal::function_closure_4<Arg1, Arg2, Arg3, Arg4>(function,
      false, arg1, arg2, arg3, arg4);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4>
inline closure* new_callback(Class* object, void (Class::*method)(Arg1, Arg2,
    Arg3, Arg4), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
  return new internal::method_closure_4<Class, Arg1, Arg2, Arg3, Arg4>(object,
      method, true, arg1, arg2, arg3, arg4);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4>
inline closure* new_permanent_callback(Class* object,
    void (Class::*method)(Arg1, Arg2, Arg3, Arg4), Arg1 arg1, Arg2 arg2,
    Arg3 arg3, Arg4 arg4) {
  return new internal::method_closure_4<Class, Arg1, Arg2, Arg3, Arg4>(object,
      method, false, arg1, arg2, arg3, arg4);
}

// For args = 5
namespace internal {
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5>
class function_closure_5 : public closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
      Arg5 arg5);

  function_closure_5(FunctionType function, bool self_deleting, Arg1 arg1,
      Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
    : function_(function), self_deleting_(self_deleting), arg1_(arg1),
        arg2_(arg2), arg3_(arg3), arg4_(arg4), arg5_(arg5) {}
  virtual ~function_closure_5() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_, arg2_, arg3_, arg4_, arg5_);
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5>
class method_closure_5 : public closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
      Arg5 arg5);

  method_closure_5(Class* object, MethodType method, bool self_deleting,
      Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
      Arg5 arg5)    : object_(object), method_(method),
      self_deleting_(self_deleting), arg1_(arg1), arg2_(arg2), arg3_(arg3),
      arg4_(arg4), arg5_(arg5) {}
  virtual ~method_closure_5() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_, arg2_, arg3_, arg4_, arg5_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;

};
}  // namespace internal

template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5>
inline closure* new_callback(void (*function)(Arg1, Arg2, Arg3, Arg4, Arg5),
    Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5) {
  return new internal::function_closure_5<Arg1, Arg2, Arg3, Arg4,
      Arg5>(function, true, arg1, arg2, arg3, arg4, arg5);
}
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5>
inline closure* new_permanent_callback(void (*function)(Arg1, Arg2, Arg3, Arg4,
    Arg5), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5) {
  return new internal::function_closure_5<Arg1, Arg2, Arg3, Arg4,
      Arg5>(function, false, arg1, arg2, arg3, arg4, arg5);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5>
inline closure* new_callback(Class* object, void (Class::*method)(Arg1, Arg2,
    Arg3, Arg4, Arg5), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5) {
  return new internal::method_closure_5<Class, Arg1, Arg2, Arg3, Arg4,
      Arg5>(object, method, true, arg1, arg2, arg3, arg4, arg5);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5>
inline closure* new_permanent_callback(Class* object,
    void (Class::*method)(Arg1, Arg2, Arg3, Arg4, Arg5), Arg1 arg1, Arg2 arg2,
    Arg3 arg3, Arg4 arg4, Arg5 arg5) {
  return new internal::method_closure_5<Class, Arg1, Arg2, Arg3, Arg4,
      Arg5>(object, method, false, arg1, arg2, arg3, arg4, arg5);
}

// For args = 6
namespace internal {
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5, typename Arg6>
class function_closure_6 : public closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
      Arg5 arg5, Arg6 arg6);

  function_closure_6(FunctionType function, bool self_deleting, Arg1 arg1,
      Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
    : function_(function), self_deleting_(self_deleting), arg1_(arg1),
        arg2_(arg2), arg3_(arg3), arg4_(arg4), arg5_(arg5), arg6_(arg6) {}
  virtual ~function_closure_6() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_);
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;
  Arg6 arg6_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5, typename Arg6>
class method_closure_6 : public closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
      Arg5 arg5, Arg6 arg6);

  method_closure_6(Class* object, MethodType method, bool self_deleting,
      Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5,
      Arg6 arg6)    : object_(object), method_(method),
      self_deleting_(self_deleting), arg1_(arg1), arg2_(arg2), arg3_(arg3),
      arg4_(arg4), arg5_(arg5), arg6_(arg6) {}
  virtual ~method_closure_6() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;
  Arg6 arg6_;

};
}  // namespace internal

template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5, typename Arg6>
inline closure* new_callback(void (*function)(Arg1, Arg2, Arg3, Arg4, Arg5,
    Arg6), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) {
  return new internal::function_closure_6<Arg1, Arg2, Arg3, Arg4, Arg5,
      Arg6>(function, true, arg1, arg2, arg3, arg4, arg5, arg6);
}
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5, typename Arg6>
inline closure* new_permanent_callback(void (*function)(Arg1, Arg2, Arg3, Arg4,
    Arg5, Arg6), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5,
    Arg6 arg6) {
  return new internal::function_closure_6<Arg1, Arg2, Arg3, Arg4, Arg5,
      Arg6>(function, false, arg1, arg2, arg3, arg4, arg5, arg6);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5, typename Arg6>
inline closure* new_callback(Class* object, void (Class::*method)(Arg1, Arg2,
    Arg3, Arg4, Arg5, Arg6), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
    Arg5 arg5, Arg6 arg6) {
  return new internal::method_closure_6<Class, Arg1, Arg2, Arg3, Arg4, Arg5,
      Arg6>(object, method, true, arg1, arg2, arg3, arg4, arg5, arg6);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5, typename Arg6>
inline closure* new_permanent_callback(Class* object,
    void (Class::*method)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6), Arg1 arg1,
    Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) {
  return new internal::method_closure_6<Class, Arg1, Arg2, Arg3, Arg4, Arg5,
      Arg6>(object, method, false, arg1, arg2, arg3, arg4, arg5, arg6);
}

// For args = 7
namespace internal {
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5, typename Arg6, typename Arg7>
class function_closure_7 : public closure {
 public:
  typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
      Arg5 arg5, Arg6 arg6, Arg7 arg7);

  function_closure_7(FunctionType function, bool self_deleting, Arg1 arg1,
      Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
    : function_(function), self_deleting_(self_deleting), arg1_(arg1),
        arg2_(arg2), arg3_(arg3), arg4_(arg4), arg5_(arg5), arg6_(arg6),
        arg7_(arg7) {}
  virtual ~function_closure_7() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_, arg7_);
    if (needs_delete) delete this;
  }

 private:
  FunctionType function_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;
  Arg6 arg6_;
  Arg7 arg7_;
};

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5, typename Arg6, typename Arg7>
class method_closure_7 : public closure {
 public:
  typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
      Arg5 arg5, Arg6 arg6, Arg7 arg7);

  method_closure_7(Class* object, MethodType method, bool self_deleting,
      Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6,
      Arg7 arg7)    : object_(object), method_(method),
      self_deleting_(self_deleting), arg1_(arg1), arg2_(arg2), arg3_(arg3),
      arg4_(arg4), arg5_(arg5), arg6_(arg6), arg7_(arg7) {}
  virtual ~method_closure_7() {}

  void run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_, arg2_, arg3_, arg4_, arg5_, arg6_, arg7_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
  Arg4 arg4_;
  Arg5 arg5_;
  Arg6 arg6_;
  Arg7 arg7_;

};
}  // namespace internal

template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5, typename Arg6, typename Arg7>
inline closure* new_callback(void (*function)(Arg1, Arg2, Arg3, Arg4, Arg5,
    Arg6, Arg7), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5,
    Arg6 arg6, Arg7 arg7) {
  return new internal::function_closure_7<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6,
      Arg7>(function, true, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
    typename Arg5, typename Arg6, typename Arg7>
inline closure* new_permanent_callback(void (*function)(Arg1, Arg2, Arg3, Arg4,
    Arg5, Arg6, Arg7), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5,
    Arg6 arg6, Arg7 arg7) {
  return new internal::function_closure_7<Arg1, Arg2, Arg3, Arg4, Arg5, Arg6,
      Arg7>(function, false, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5, typename Arg6, typename Arg7>
inline closure* new_callback(Class* object, void (Class::*method)(Arg1, Arg2,
    Arg3, Arg4, Arg5, Arg6, Arg7), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4,
    Arg5 arg5, Arg6 arg6, Arg7 arg7) {
  return new internal::method_closure_7<Class, Arg1, Arg2, Arg3, Arg4, Arg5,
      Arg6, Arg7>(object, method, true, arg1, arg2, arg3, arg4, arg5, arg6,
      arg7);
}

template <typename Class, typename Arg1, typename Arg2, typename Arg3,
    typename Arg4, typename Arg5, typename Arg6, typename Arg7>
inline closure* new_permanent_callback(Class* object,
    void (Class::*method)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7), Arg1 arg1,
    Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7) {
  return new internal::method_closure_7<Class, Arg1, Arg2, Arg3, Arg4, Arg5,
      Arg6, Arg7>(object, method, false, arg1, arg2, arg3, arg4, arg5, arg6,
      arg7);
}

} //namespace rpcz
#endif
