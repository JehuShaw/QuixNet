// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: msg_send_mail.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "msg_send_mail.pb.h"

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

namespace game {

namespace {

const ::google::protobuf::Descriptor* SendMailPacket_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  SendMailPacket_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_msg_5fsend_5fmail_2eproto() {
  protobuf_AddDesc_msg_5fsend_5fmail_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "msg_send_mail.proto");
  GOOGLE_CHECK(file != NULL);
  SendMailPacket_descriptor_ = file->message_type(0);
  static const int SendMailPacket_offsets_[9] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, sender_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, sender_name_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, sender_type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, receivers_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, receiver_type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, title_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, brief_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, content_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, mail_type_),
  };
  SendMailPacket_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      SendMailPacket_descriptor_,
      SendMailPacket::default_instance_,
      SendMailPacket_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SendMailPacket, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(SendMailPacket));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_msg_5fsend_5fmail_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    SendMailPacket_descriptor_, &SendMailPacket::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_msg_5fsend_5fmail_2eproto() {
  delete SendMailPacket::default_instance_;
  delete SendMailPacket_reflection_;
}

void protobuf_AddDesc_msg_5fsend_5fmail_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\023msg_send_mail.proto\022\004game\"\266\001\n\016SendMail"
    "Packet\022\016\n\006sender\030\001 \001(\004\022\023\n\013sender_name\030\002 "
    "\001(\t\022\023\n\013sender_type\030\003 \001(\005\022\021\n\treceivers\030\004 "
    "\003(\004\022\025\n\rreceiver_type\030\005 \001(\005\022\r\n\005title\030\006 \002("
    "\t\022\r\n\005brief\030\007 \001(\t\022\017\n\007content\030\010 \001(\t\022\021\n\tmai"
    "l_type\030\t \001(\005", 212);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "msg_send_mail.proto", &protobuf_RegisterTypes);
  SendMailPacket::default_instance_ = new SendMailPacket();
  SendMailPacket::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_msg_5fsend_5fmail_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_msg_5fsend_5fmail_2eproto {
  StaticDescriptorInitializer_msg_5fsend_5fmail_2eproto() {
    protobuf_AddDesc_msg_5fsend_5fmail_2eproto();
  }
} static_descriptor_initializer_msg_5fsend_5fmail_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int SendMailPacket::kSenderFieldNumber;
const int SendMailPacket::kSenderNameFieldNumber;
const int SendMailPacket::kSenderTypeFieldNumber;
const int SendMailPacket::kReceiversFieldNumber;
const int SendMailPacket::kReceiverTypeFieldNumber;
const int SendMailPacket::kTitleFieldNumber;
const int SendMailPacket::kBriefFieldNumber;
const int SendMailPacket::kContentFieldNumber;
const int SendMailPacket::kMailTypeFieldNumber;
#endif  // !_MSC_VER

SendMailPacket::SendMailPacket()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void SendMailPacket::InitAsDefaultInstance() {
}

SendMailPacket::SendMailPacket(const SendMailPacket& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void SendMailPacket::SharedCtor() {
  _cached_size_ = 0;
  sender_ = GOOGLE_ULONGLONG(0);
  sender_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  sender_type_ = 0;
  receiver_type_ = 0;
  title_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  brief_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  content_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  mail_type_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

SendMailPacket::~SendMailPacket() {
  SharedDtor();
}

void SendMailPacket::SharedDtor() {
  if (sender_name_ != &::google::protobuf::internal::kEmptyString) {
    delete sender_name_;
  }
  if (title_ != &::google::protobuf::internal::kEmptyString) {
    delete title_;
  }
  if (brief_ != &::google::protobuf::internal::kEmptyString) {
    delete brief_;
  }
  if (content_ != &::google::protobuf::internal::kEmptyString) {
    delete content_;
  }
  if (this != default_instance_) {
  }
}

void SendMailPacket::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* SendMailPacket::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return SendMailPacket_descriptor_;
}

const SendMailPacket& SendMailPacket::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_msg_5fsend_5fmail_2eproto();
  return *default_instance_;
}

SendMailPacket* SendMailPacket::default_instance_ = NULL;

SendMailPacket* SendMailPacket::New() const {
  return new SendMailPacket;
}

void SendMailPacket::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    sender_ = GOOGLE_ULONGLONG(0);
    if (has_sender_name()) {
      if (sender_name_ != &::google::protobuf::internal::kEmptyString) {
        sender_name_->clear();
      }
    }
    sender_type_ = 0;
    receiver_type_ = 0;
    if (has_title()) {
      if (title_ != &::google::protobuf::internal::kEmptyString) {
        title_->clear();
      }
    }
    if (has_brief()) {
      if (brief_ != &::google::protobuf::internal::kEmptyString) {
        brief_->clear();
      }
    }
    if (has_content()) {
      if (content_ != &::google::protobuf::internal::kEmptyString) {
        content_->clear();
      }
    }
  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    mail_type_ = 0;
  }
  receivers_.Clear();
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool SendMailPacket::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional uint64 sender = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &sender_)));
          set_has_sender();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_sender_name;
        break;
      }

      // optional string sender_name = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_sender_name:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_sender_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->sender_name().data(), this->sender_name().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_sender_type;
        break;
      }

      // optional int32 sender_type = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_sender_type:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &sender_type_)));
          set_has_sender_type();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(32)) goto parse_receivers;
        break;
      }

      // repeated uint64 receivers = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_receivers:
          DO_((::google::protobuf::internal::WireFormatLite::ReadRepeatedPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 1, 32, input, this->mutable_receivers())));
        } else if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag)
                   == ::google::protobuf::internal::WireFormatLite::
                      WIRETYPE_LENGTH_DELIMITED) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPackedPrimitiveNoInline<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, this->mutable_receivers())));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(32)) goto parse_receivers;
        if (input->ExpectTag(40)) goto parse_receiver_type;
        break;
      }

      // optional int32 receiver_type = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_receiver_type:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &receiver_type_)));
          set_has_receiver_type();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(50)) goto parse_title;
        break;
      }

      // required string title = 6;
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_title:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_title()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->title().data(), this->title().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(58)) goto parse_brief;
        break;
      }

      // optional string brief = 7;
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_brief:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_brief()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->brief().data(), this->brief().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(66)) goto parse_content;
        break;
      }

      // optional string content = 8;
      case 8: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_content:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_content()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->content().data(), this->content().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(72)) goto parse_mail_type;
        break;
      }

      // optional int32 mail_type = 9;
      case 9: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_mail_type:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &mail_type_)));
          set_has_mail_type();
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

void SendMailPacket::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional uint64 sender = 1;
  if (has_sender()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(1, this->sender(), output);
  }

  // optional string sender_name = 2;
  if (has_sender_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->sender_name().data(), this->sender_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->sender_name(), output);
  }

  // optional int32 sender_type = 3;
  if (has_sender_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->sender_type(), output);
  }

  // repeated uint64 receivers = 4;
  for (int i = 0; i < this->receivers_size(); i++) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(
      4, this->receivers(i), output);
  }

  // optional int32 receiver_type = 5;
  if (has_receiver_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(5, this->receiver_type(), output);
  }

  // required string title = 6;
  if (has_title()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->title().data(), this->title().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      6, this->title(), output);
  }

  // optional string brief = 7;
  if (has_brief()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->brief().data(), this->brief().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      7, this->brief(), output);
  }

  // optional string content = 8;
  if (has_content()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->content().data(), this->content().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      8, this->content(), output);
  }

  // optional int32 mail_type = 9;
  if (has_mail_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(9, this->mail_type(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* SendMailPacket::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // optional uint64 sender = 1;
  if (has_sender()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(1, this->sender(), target);
  }

  // optional string sender_name = 2;
  if (has_sender_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->sender_name().data(), this->sender_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->sender_name(), target);
  }

  // optional int32 sender_type = 3;
  if (has_sender_type()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->sender_type(), target);
  }

  // repeated uint64 receivers = 4;
  for (int i = 0; i < this->receivers_size(); i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteUInt64ToArray(4, this->receivers(i), target);
  }

  // optional int32 receiver_type = 5;
  if (has_receiver_type()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(5, this->receiver_type(), target);
  }

  // required string title = 6;
  if (has_title()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->title().data(), this->title().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        6, this->title(), target);
  }

  // optional string brief = 7;
  if (has_brief()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->brief().data(), this->brief().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        7, this->brief(), target);
  }

  // optional string content = 8;
  if (has_content()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->content().data(), this->content().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        8, this->content(), target);
  }

  // optional int32 mail_type = 9;
  if (has_mail_type()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(9, this->mail_type(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int SendMailPacket::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional uint64 sender = 1;
    if (has_sender()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt64Size(
          this->sender());
    }

    // optional string sender_name = 2;
    if (has_sender_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->sender_name());
    }

    // optional int32 sender_type = 3;
    if (has_sender_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->sender_type());
    }

    // optional int32 receiver_type = 5;
    if (has_receiver_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->receiver_type());
    }

    // required string title = 6;
    if (has_title()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->title());
    }

    // optional string brief = 7;
    if (has_brief()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->brief());
    }

    // optional string content = 8;
    if (has_content()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->content());
    }

  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    // optional int32 mail_type = 9;
    if (has_mail_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->mail_type());
    }

  }
  // repeated uint64 receivers = 4;
  {
    int data_size = 0;
    for (int i = 0; i < this->receivers_size(); i++) {
      data_size += ::google::protobuf::internal::WireFormatLite::
        UInt64Size(this->receivers(i));
    }
    total_size += 1 * this->receivers_size() + data_size;
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

void SendMailPacket::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const SendMailPacket* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const SendMailPacket*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void SendMailPacket::MergeFrom(const SendMailPacket& from) {
  GOOGLE_CHECK_NE(&from, this);
  receivers_.MergeFrom(from.receivers_);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_sender()) {
      set_sender(from.sender());
    }
    if (from.has_sender_name()) {
      set_sender_name(from.sender_name());
    }
    if (from.has_sender_type()) {
      set_sender_type(from.sender_type());
    }
    if (from.has_receiver_type()) {
      set_receiver_type(from.receiver_type());
    }
    if (from.has_title()) {
      set_title(from.title());
    }
    if (from.has_brief()) {
      set_brief(from.brief());
    }
    if (from.has_content()) {
      set_content(from.content());
    }
  }
  if (from._has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    if (from.has_mail_type()) {
      set_mail_type(from.mail_type());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void SendMailPacket::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void SendMailPacket::CopyFrom(const SendMailPacket& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool SendMailPacket::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000020) != 0x00000020) return false;

  return true;
}

void SendMailPacket::Swap(SendMailPacket* other) {
  if (other != this) {
    std::swap(sender_, other->sender_);
    std::swap(sender_name_, other->sender_name_);
    std::swap(sender_type_, other->sender_type_);
    receivers_.Swap(&other->receivers_);
    std::swap(receiver_type_, other->receiver_type_);
    std::swap(title_, other->title_);
    std::swap(brief_, other->brief_);
    std::swap(content_, other->content_);
    std::swap(mail_type_, other->mail_type_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata SendMailPacket::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = SendMailPacket_descriptor_;
  metadata.reflection = SendMailPacket_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace game

// @@protoc_insertion_point(global_scope)
