// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: msg_client_login.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "msg_client_login.pb.h"

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

namespace node {

namespace {

const ::google::protobuf::Descriptor* LoginRequest_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  LoginRequest_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_msg_5fclient_5flogin_2eproto() {
  protobuf_AddDesc_msg_5fclient_5flogin_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "msg_client_login.proto");
  GOOGLE_CHECK(file != NULL);
  LoginRequest_descriptor_ = file->message_type(0);
  static const int LoginRequest_offsets_[5] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, originip_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, routecount_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, account_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, version_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(LoginRequest, remoteip_),
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
                 &protobuf_AssignDesc_msg_5fclient_5flogin_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    LoginRequest_descriptor_, &LoginRequest::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_msg_5fclient_5flogin_2eproto() {
  delete LoginRequest::default_instance_;
  delete LoginRequest_reflection_;
}

void protobuf_AddDesc_msg_5fclient_5flogin_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\026msg_client_login.proto\022\004node\"h\n\014LoginR"
    "equest\022\020\n\010originip\030\001 \002(\t\022\022\n\nroutecount\030\002"
    " \002(\r\022\017\n\007account\030\003 \002(\004\022\017\n\007version\030\004 \001(\r\022\020"
    "\n\010remoteip\030\005 \001(\t", 136);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "msg_client_login.proto", &protobuf_RegisterTypes);
  LoginRequest::default_instance_ = new LoginRequest();
  LoginRequest::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_msg_5fclient_5flogin_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_msg_5fclient_5flogin_2eproto {
  StaticDescriptorInitializer_msg_5fclient_5flogin_2eproto() {
    protobuf_AddDesc_msg_5fclient_5flogin_2eproto();
  }
} static_descriptor_initializer_msg_5fclient_5flogin_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int LoginRequest::kOriginipFieldNumber;
const int LoginRequest::kRoutecountFieldNumber;
const int LoginRequest::kAccountFieldNumber;
const int LoginRequest::kVersionFieldNumber;
const int LoginRequest::kRemoteipFieldNumber;
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
  originip_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  routecount_ = 0u;
  account_ = GOOGLE_ULONGLONG(0);
  version_ = 0u;
  remoteip_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

LoginRequest::~LoginRequest() {
  SharedDtor();
}

void LoginRequest::SharedDtor() {
  if (originip_ != &::google::protobuf::internal::kEmptyString) {
    delete originip_;
  }
  if (remoteip_ != &::google::protobuf::internal::kEmptyString) {
    delete remoteip_;
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
  if (default_instance_ == NULL) protobuf_AddDesc_msg_5fclient_5flogin_2eproto();
  return *default_instance_;
}

LoginRequest* LoginRequest::default_instance_ = NULL;

LoginRequest* LoginRequest::New() const {
  return new LoginRequest;
}

void LoginRequest::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_originip()) {
      if (originip_ != &::google::protobuf::internal::kEmptyString) {
        originip_->clear();
      }
    }
    routecount_ = 0u;
    account_ = GOOGLE_ULONGLONG(0);
    version_ = 0u;
    if (has_remoteip()) {
      if (remoteip_ != &::google::protobuf::internal::kEmptyString) {
        remoteip_->clear();
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
      // required string originip = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_originip()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->originip().data(), this->originip().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_routecount;
        break;
      }

      // required uint32 routecount = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_routecount:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &routecount_)));
          set_has_routecount();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_account;
        break;
      }

      // required uint64 account = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_account:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &account_)));
          set_has_account();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(32)) goto parse_version;
        break;
      }

      // optional uint32 version = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_version:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &version_)));
          set_has_version();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(42)) goto parse_remoteip;
        break;
      }

      // optional string remoteip = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_remoteip:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_remoteip()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->remoteip().data(), this->remoteip().length(),
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
  // required string originip = 1;
  if (has_originip()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->originip().data(), this->originip().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      1, this->originip(), output);
  }

  // required uint32 routecount = 2;
  if (has_routecount()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->routecount(), output);
  }

  // required uint64 account = 3;
  if (has_account()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(3, this->account(), output);
  }

  // optional uint32 version = 4;
  if (has_version()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(4, this->version(), output);
  }

  // optional string remoteip = 5;
  if (has_remoteip()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->remoteip().data(), this->remoteip().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      5, this->remoteip(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* LoginRequest::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required string originip = 1;
  if (has_originip()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->originip().data(), this->originip().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->originip(), target);
  }

  // required uint32 routecount = 2;
  if (has_routecount()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->routecount(), target);
  }

  // required uint64 account = 3;
  if (has_account()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(3, this->account(), target);
  }

  // optional uint32 version = 4;
  if (has_version()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(4, this->version(), target);
  }

  // optional string remoteip = 5;
  if (has_remoteip()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->remoteip().data(), this->remoteip().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        5, this->remoteip(), target);
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
    // required string originip = 1;
    if (has_originip()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->originip());
    }

    // required uint32 routecount = 2;
    if (has_routecount()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->routecount());
    }

    // required uint64 account = 3;
    if (has_account()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt64Size(
          this->account());
    }

    // optional uint32 version = 4;
    if (has_version()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->version());
    }

    // optional string remoteip = 5;
    if (has_remoteip()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->remoteip());
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
    if (from.has_originip()) {
      set_originip(from.originip());
    }
    if (from.has_routecount()) {
      set_routecount(from.routecount());
    }
    if (from.has_account()) {
      set_account(from.account());
    }
    if (from.has_version()) {
      set_version(from.version());
    }
    if (from.has_remoteip()) {
      set_remoteip(from.remoteip());
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
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;

  return true;
}

void LoginRequest::Swap(LoginRequest* other) {
  if (other != this) {
    std::swap(originip_, other->originip_);
    std::swap(routecount_, other->routecount_);
    std::swap(account_, other->account_);
    std::swap(version_, other->version_);
    std::swap(remoteip_, other->remoteip_);
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

}  // namespace node

// @@protoc_insertion_point(global_scope)
