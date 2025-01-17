// Copyright 2022 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_COMMON_CALL_TESTER_H_
#define V8_COMMON_CALL_TESTER_H_

#include "src/execution/simulator.h"
#include "src/handles/handles.h"
#include "src/objects/code.h"
#include "test/common/c-signature.h"

namespace v8 {
namespace internal {
namespace compiler {

template <typename R>
class CallHelper {
 public:
  explicit CallHelper(Isolate* isolate, MachineSignature* csig)
      : csig_(csig), isolate_(isolate) {
    USE(isolate_);
  }
  virtual ~CallHelper() = default;

  template <typename... Params>
  R Call(Params... args) {
    CSignature::VerifyParams<Params...>(csig_);
    Address entry = Generate();
    auto fn = GeneratedCode<R, Params...>::FromAddress(isolate_, entry);
    return fn.Call(args...);
  }

 protected:
  MachineSignature* csig_;

  virtual Address Generate() = 0;

 private:
  Isolate* isolate_;
};

template <>
template <typename... Params>
Object CallHelper<Object>::Call(Params... args) {
  CSignature::VerifyParams<Params...>(csig_);
  Address entry = Generate();
  auto fn = GeneratedCode<Address, Params...>::FromAddress(isolate_, entry);
  return Object(fn.Call(args...));
}

// A call helper that calls the given code object assuming C calling convention.
template <typename T>
class CodeRunner : public CallHelper<T> {
 public:
  CodeRunner(Isolate* isolate, Handle<Code> code, MachineSignature* csig)
      : CallHelper<T>(isolate, csig), code_(code) {}
  CodeRunner(Isolate* isolate, Handle<CodeT> code, MachineSignature* csig)
      : CallHelper<T>(isolate, csig), code_(FromCodeT(*code), isolate) {}
  ~CodeRunner() override = default;

  Address Generate() override { return code_->entry(); }

 private:
  Handle<Code> code_;
};

}  // namespace compiler
}  // namespace internal
}  // namespace v8

#endif  // V8_COMMON_CALL_TESTER_H_
