// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: msg_master_login.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "msg_master_login.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace master {

namespace {

const ::google::protobuf::Descriptor* LoginRequest_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  LoginRequest_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_msg_5fmaster_5flogin_2eproto() {
  protobuf_AddDesc_msg_5fmaster_5flogin_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "msg_master_login.proto");
  GOOGLE_CHECK(file != NULL);
  LoginRequest_descriptor_ = file->message_type(0);
  static const int LoginRequest_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, account_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, sessionkey_),
  };
  LoginRequest_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      LoginRequest_descriptor_,
      LoginRequest::default_instance_,
      LoginRequest_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(LoginRequest));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_msg_5fmaster_5flogin_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    LoginRequest_descriptor_, &LoginRequest::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_msg_5fmaster_5flogin_2eproto() {
  delete LoginRequest::default_instance_;
  delete LoginRequest_reflection_;
}

void protobuf_AddDesc_msg_5fmaster_5flogin_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\026msg_master_login.proto\022\006master\"3\n\014Logi"
    "nRequest\022\017\n\007account\030\001 \002(\r\022\022\n\nsessionkey\030"
    "\002 \002(\t", 85);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "msg_master_login.proto", &protobuf_RegisterTypes);
  LoginRequest::default_instance_ = new LoginRequest();
  LoginRequest::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_msg_5fmaster_5flogin_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_msg_5fmaster_5flogin_2eproto {
  StaticDescriptorInitializer_msg_5fmaster_5flogin_2eproto() {
    protobuf_AddDesc_msg_5fmaster_5flogin_2eproto();
  }
} static_descriptor_initializer_msg_5fmaster_5flogin_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int LoginRequest::kAccountFieldNumber;
const int LoginRequest::kSessionkeyFieldNumber;
#endif  // !_MSC_VER

LoginRequest::LoginRequest()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void LoginRequest::InitAsDefaultInstance() {
}

LoginRequest::LoginRequest(const LoginRequest& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void LoginRequest::SharedCtor() {
  _cached_size_ = 0;
  account_ = 0u;
  sessionkey_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

LoginRequest::~LoginRequest() {
  SharedDtor();
}

void LoginRequest::SharedDtor() {
  if (sessionkey_ != &::google::protobuf::internal::kEmptyString) {
    delete sessionkey_;
  }
  if (this != default_instance_) {
  }
}

void LoginRequest::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* LoginRequest::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return LoginRequest_descriptor_;
}

const LoginRequest& LoginRequest::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_msg_5fmaster_5flogin_2eproto();
  return *default_instance_;
}

LoginRequest* LoginRequest::default_instance_ = NULL;

LoginRequest* LoginRequest::New() const {
  return new LoginRequest;
}

void LoginRequest::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    account_ = 0u;
    if (has_sessionkey()) {
      if (sessionkey_ != &::google::protobuf::internal::kEmptyString) {
        sessionkey_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool LoginRequest::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required uint32 account = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &account_)));
          set_has_account();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_sessionkey;
        break;
      }

      // required string sessionkey = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_sessionkey:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_sessionkey()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->sessionkey().data(), this->sessionkey().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void LoginRequest::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required uint32 account = 1;
  if (has_account()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->account(), output);
  }

  // required string sessionkey = 2;
  if (has_sessionkey()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->sessionkey().data(), this->sessionkey().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->sessionkey(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* LoginRequest::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required uint32 account = 1;
  if (has_account()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(1, this->account(), target);
  }

  // required string sessionkey = 2;
  if (has_sessionkey()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->sessionkey().data(), this->sessionkey().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->sessionkey(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int LoginRequest::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required uint32 account = 1;
    if (has_account()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->account());
    }

    // required string sessionkey = 2;
    if (has_sessionkey()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->sessionkey());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void LoginRequest::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const LoginRequest* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const LoginRequest*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void LoginRequest::MergeFrom(const LoginRequest& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_account()) {
      set_account(from.account());
    }
    if (from.has_sessionkey()) {
      set_sessionkey(from.sessionkey());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void LoginRequest::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void LoginRequest::CopyFrom(const LoginRequest& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool LoginRequest::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;

  return true;
}

void LoginRequest::Swap(LoginRequest* other) {
  if (other != this) {
    std::swap(account_, other->account_);
    std::swap(sessionkey_, other->sessionkey_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata LoginRequest::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = LoginRequest_descriptor_;
  metadata.reflection = LoginRequest_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace master

// @@protoc_insertion_point(global_scope)
