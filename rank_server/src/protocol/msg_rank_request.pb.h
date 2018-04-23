// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: msg_rank_request.proto

#ifndef PROTOBUF_msg_5frank_5frequest_2eproto__INCLUDED
#define PROTOBUF_msg_5frank_5frequest_2eproto__INCLUDED

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
#include "msg_rank_item.pb.h"
// @@protoc_insertion_point(includes)

namespace rank {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_msg_5frank_5frequest_2eproto();
void protobuf_AssignDesc_msg_5frank_5frequest_2eproto();
void protobuf_ShutdownFile_msg_5frank_5frequest_2eproto();

class RequestRankPacket;
class RequestRankResultPacket;

// ===================================================================

class RequestRankPacket : public ::google::protobuf::Message {
 public:
  RequestRankPacket();
  virtual ~RequestRankPacket();

  RequestRankPacket(const RequestRankPacket& from);

  inline RequestRankPacket& operator=(const RequestRankPacket& from) {
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
  static const RequestRankPacket& default_instance();

  void Swap(RequestRankPacket* other);

  // implements Message ----------------------------------------------

  RequestRankPacket* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RequestRankPacket& from);
  void MergeFrom(const RequestRankPacket& from);
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

  // required int32 rank_type = 1;
  inline bool has_rank_type() const;
  inline void clear_rank_type();
  static const int kRankTypeFieldNumber = 1;
  inline ::google::protobuf::int32 rank_type() const;
  inline void set_rank_type(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:rank.RequestRankPacket)
 private:
  inline void set_has_rank_type();
  inline void clear_has_rank_type();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int32 rank_type_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_msg_5frank_5frequest_2eproto();
  friend void protobuf_AssignDesc_msg_5frank_5frequest_2eproto();
  friend void protobuf_ShutdownFile_msg_5frank_5frequest_2eproto();

  void InitAsDefaultInstance();
  static RequestRankPacket* default_instance_;
};
// -------------------------------------------------------------------

class RequestRankResultPacket : public ::google::protobuf::Message {
 public:
  RequestRankResultPacket();
  virtual ~RequestRankResultPacket();

  RequestRankResultPacket(const RequestRankResultPacket& from);

  inline RequestRankResultPacket& operator=(const RequestRankResultPacket& from) {
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
  static const RequestRankResultPacket& default_instance();

  void Swap(RequestRankResultPacket* other);

  // implements Message ----------------------------------------------

  RequestRankResultPacket* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RequestRankResultPacket& from);
  void MergeFrom(const RequestRankResultPacket& from);
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

  // required int32 rank_type = 1;
  inline bool has_rank_type() const;
  inline void clear_rank_type();
  static const int kRankTypeFieldNumber = 1;
  inline ::google::protobuf::int32 rank_type() const;
  inline void set_rank_type(::google::protobuf::int32 value);

  // repeated .rank.RankItemPacket items = 2;
  inline int items_size() const;
  inline void clear_items();
  static const int kItemsFieldNumber = 2;
  inline const ::rank::RankItemPacket& items(int index) const;
  inline ::rank::RankItemPacket* mutable_items(int index);
  inline ::rank::RankItemPacket* add_items();
  inline const ::google::protobuf::RepeatedPtrField< ::rank::RankItemPacket >&
      items() const;
  inline ::google::protobuf::RepeatedPtrField< ::rank::RankItemPacket >*
      mutable_items();

  // @@protoc_insertion_point(class_scope:rank.RequestRankResultPacket)
 private:
  inline void set_has_rank_type();
  inline void clear_has_rank_type();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedPtrField< ::rank::RankItemPacket > items_;
  ::google::protobuf::int32 rank_type_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  friend void  protobuf_AddDesc_msg_5frank_5frequest_2eproto();
  friend void protobuf_AssignDesc_msg_5frank_5frequest_2eproto();
  friend void protobuf_ShutdownFile_msg_5frank_5frequest_2eproto();

  void InitAsDefaultInstance();
  static RequestRankResultPacket* default_instance_;
};
// ===================================================================


// ===================================================================

// RequestRankPacket

// required int32 rank_type = 1;
inline bool RequestRankPacket::has_rank_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void RequestRankPacket::set_has_rank_type() {
  _has_bits_[0] |= 0x00000001u;
}
inline void RequestRankPacket::clear_has_rank_type() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void RequestRankPacket::clear_rank_type() {
  rank_type_ = 0;
  clear_has_rank_type();
}
inline ::google::protobuf::int32 RequestRankPacket::rank_type() const {
  return rank_type_;
}
inline void RequestRankPacket::set_rank_type(::google::protobuf::int32 value) {
  set_has_rank_type();
  rank_type_ = value;
}

// -------------------------------------------------------------------

// RequestRankResultPacket

// required int32 rank_type = 1;
inline bool RequestRankResultPacket::has_rank_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void RequestRankResultPacket::set_has_rank_type() {
  _has_bits_[0] |= 0x00000001u;
}
inline void RequestRankResultPacket::clear_has_rank_type() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void RequestRankResultPacket::clear_rank_type() {
  rank_type_ = 0;
  clear_has_rank_type();
}
inline ::google::protobuf::int32 RequestRankResultPacket::rank_type() const {
  return rank_type_;
}
inline void RequestRankResultPacket::set_rank_type(::google::protobuf::int32 value) {
  set_has_rank_type();
  rank_type_ = value;
}

// repeated .rank.RankItemPacket items = 2;
inline int RequestRankResultPacket::items_size() const {
  return items_.size();
}
inline void RequestRankResultPacket::clear_items() {
  items_.Clear();
}
inline const ::rank::RankItemPacket& RequestRankResultPacket::items(int index) const {
  return items_.Get(index);
}
inline ::rank::RankItemPacket* RequestRankResultPacket::mutable_items(int index) {
  return items_.Mutable(index);
}
inline ::rank::RankItemPacket* RequestRankResultPacket::add_items() {
  return items_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::rank::RankItemPacket >&
RequestRankResultPacket::items() const {
  return items_;
}
inline ::google::protobuf::RepeatedPtrField< ::rank::RankItemPacket >*
RequestRankResultPacket::mutable_items() {
  return &items_;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace rank

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_msg_5frank_5frequest_2eproto__INCLUDED
