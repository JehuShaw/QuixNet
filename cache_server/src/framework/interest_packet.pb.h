// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: interest_packet.proto

#ifndef PROTOBUF_interest_5fpacket_2eproto__INCLUDED
#define PROTOBUF_interest_5fpacket_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace node {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_interest_5fpacket_2eproto();
void protobuf_AssignDesc_interest_5fpacket_2eproto();
void protobuf_ShutdownFile_interest_5fpacket_2eproto();

class InterestPacket;

// ===================================================================

class InterestPacket : public ::google::protobuf::Message {
 public:
  InterestPacket();
  virtual ~InterestPacket();

  InterestPacket(const InterestPacket& from);

  inline InterestPacket& operator=(const InterestPacket& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const InterestPacket& default_instance();

  void Swap(InterestPacket* other);

  // implements Message ----------------------------------------------

  InterestPacket* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const InterestPacket& from);
  void MergeFrom(const InterestPacket& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated int32 interests = 1;
  inline int interests_size() const;
  inline void clear_interests();
  static const int kInterestsFieldNumber = 1;
  inline ::google::protobuf::int32 interests(int index) const;
  inline void set_interests(int index, ::google::protobuf::int32 value);
  inline void add_interests(::google::protobuf::int32 value);
  inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      interests() const;
  inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_interests();

  // @@protoc_insertion_point(class_scope:node.InterestPacket)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > interests_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_interest_5fpacket_2eproto();
  friend void protobuf_AssignDesc_interest_5fpacket_2eproto();
  friend void protobuf_ShutdownFile_interest_5fpacket_2eproto();

  void InitAsDefaultInstance();
  static InterestPacket* default_instance_;
};
// ===================================================================


// ===================================================================

// InterestPacket

// repeated int32 interests = 1;
inline int InterestPacket::interests_size() const {
  return interests_.size();
}
inline void InterestPacket::clear_interests() {
  interests_.Clear();
}
inline ::google::protobuf::int32 InterestPacket::interests(int index) const {
  return interests_.Get(index);
}
inline void InterestPacket::set_interests(int index, ::google::protobuf::int32 value) {
  interests_.Set(index, value);
}
inline void InterestPacket::add_interests(::google::protobuf::int32 value) {
  interests_.Add(value);
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
InterestPacket::interests() const {
  return interests_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
InterestPacket::mutable_interests() {
  return &interests_;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace node

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_interest_5fpacket_2eproto__INCLUDED
