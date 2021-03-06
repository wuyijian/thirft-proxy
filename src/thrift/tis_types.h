/**
 * Autogenerated by Thrift Compiler (0.10.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef tis_TYPES_H
#define tis_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>




class CheckResultStruct;

class RequestCheckStruct;

typedef struct _CheckResultStruct__isset {
  _CheckResultStruct__isset() : code(false), action(false), label(false), level(false), detail(false) {}
  bool code :1;
  bool action :1;
  bool label :1;
  bool level :1;
  bool detail :1;
} _CheckResultStruct__isset;

class CheckResultStruct : public virtual ::apache::thrift::TBase {
 public:

  CheckResultStruct(const CheckResultStruct&);
  CheckResultStruct& operator=(const CheckResultStruct&);
  CheckResultStruct() : code(0), action(0), label(0), level(0), detail() {
  }

  virtual ~CheckResultStruct() throw();
  int32_t code;
  int32_t action;
  int32_t label;
  int32_t level;
  std::string detail;

  _CheckResultStruct__isset __isset;

  void __set_code(const int32_t val);

  void __set_action(const int32_t val);

  void __set_label(const int32_t val);

  void __set_level(const int32_t val);

  void __set_detail(const std::string& val);

  bool operator == (const CheckResultStruct & rhs) const
  {
    if (!(code == rhs.code))
      return false;
    if (!(action == rhs.action))
      return false;
    if (!(label == rhs.label))
      return false;
    if (!(level == rhs.level))
      return false;
    if (!(detail == rhs.detail))
      return false;
    return true;
  }
  bool operator != (const CheckResultStruct &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const CheckResultStruct & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(CheckResultStruct &a, CheckResultStruct &b);

inline std::ostream& operator<<(std::ostream& out, const CheckResultStruct& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _RequestCheckStruct__isset {
  _RequestCheckStruct__isset() : appid(false), gameid(false), content(false), dataid(false), userid(false), ip(false) {}
  bool appid :1;
  bool gameid :1;
  bool content :1;
  bool dataid :1;
  bool userid :1;
  bool ip :1;
} _RequestCheckStruct__isset;

class RequestCheckStruct : public virtual ::apache::thrift::TBase {
 public:

  RequestCheckStruct(const RequestCheckStruct&);
  RequestCheckStruct& operator=(const RequestCheckStruct&);
  RequestCheckStruct() : appid(0), gameid(0), content(), dataid(), userid(), ip() {
  }

  virtual ~RequestCheckStruct() throw();
  int32_t appid;
  int32_t gameid;
  std::string content;
  std::string dataid;
  std::string userid;
  std::string ip;

  _RequestCheckStruct__isset __isset;

  void __set_appid(const int32_t val);

  void __set_gameid(const int32_t val);

  void __set_content(const std::string& val);

  void __set_dataid(const std::string& val);

  void __set_userid(const std::string& val);

  void __set_ip(const std::string& val);

  bool operator == (const RequestCheckStruct & rhs) const
  {
    if (!(appid == rhs.appid))
      return false;
    if (!(gameid == rhs.gameid))
      return false;
    if (!(content == rhs.content))
      return false;
    if (!(dataid == rhs.dataid))
      return false;
    if (!(userid == rhs.userid))
      return false;
    if (!(ip == rhs.ip))
      return false;
    return true;
  }
  bool operator != (const RequestCheckStruct &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RequestCheckStruct & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(RequestCheckStruct &a, RequestCheckStruct &b);

inline std::ostream& operator<<(std::ostream& out, const RequestCheckStruct& obj)
{
  obj.printTo(out);
  return out;
}



#endif
