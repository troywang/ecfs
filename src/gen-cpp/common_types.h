/**
 * Autogenerated by Thrift Compiler (0.8.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef common_TYPES_H
#define common_TYPES_H

#include <Thrift.h>
#include <TApplicationException.h>
#include <protocol/TProtocol.h>
#include <transport/TTransport.h>






class DatanodeEndpoint {
 public:

  static const char* ascii_fingerprint; // = "20CDC9D979DE694980EA2466496D8E68";
  static const uint8_t binary_fingerprint[16]; // = {0x20,0xCD,0xC9,0xD9,0x79,0xDE,0x69,0x49,0x80,0xEA,0x24,0x66,0x49,0x6D,0x8E,0x68};

  DatanodeEndpoint() : ipv4(0), port(0), nodeid(0) {
  }

  virtual ~DatanodeEndpoint() throw() {}

  int32_t ipv4;
  int16_t port;
  int16_t nodeid;

  void __set_ipv4(const int32_t val) {
    ipv4 = val;
  }

  void __set_port(const int16_t val) {
    port = val;
  }

  void __set_nodeid(const int16_t val) {
    nodeid = val;
  }

  bool operator == (const DatanodeEndpoint & rhs) const
  {
    if (!(ipv4 == rhs.ipv4))
      return false;
    if (!(port == rhs.port))
      return false;
    if (!(nodeid == rhs.nodeid))
      return false;
    return true;
  }
  bool operator != (const DatanodeEndpoint &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DatanodeEndpoint & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _ThriftResult__isset {
  _ThriftResult__isset() : verbose(false) {}
  bool verbose;
} _ThriftResult__isset;

class ThriftResult {
 public:

  static const char* ascii_fingerprint; // = "96705E9A3FD7B072319C71653E0DBB90";
  static const uint8_t binary_fingerprint[16]; // = {0x96,0x70,0x5E,0x9A,0x3F,0xD7,0xB0,0x72,0x31,0x9C,0x71,0x65,0x3E,0x0D,0xBB,0x90};

  ThriftResult() : code(0), verbose("") {
  }

  virtual ~ThriftResult() throw() {}

  int32_t code;
  std::string verbose;

  _ThriftResult__isset __isset;

  void __set_code(const int32_t val) {
    code = val;
  }

  void __set_verbose(const std::string& val) {
    verbose = val;
    __isset.verbose = true;
  }

  bool operator == (const ThriftResult & rhs) const
  {
    if (!(code == rhs.code))
      return false;
    if (__isset.verbose != rhs.__isset.verbose)
      return false;
    else if (__isset.verbose && !(verbose == rhs.verbose))
      return false;
    return true;
  }
  bool operator != (const ThriftResult &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ThriftResult & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileWriteToken {
 public:

  static const char* ascii_fingerprint; // = "417159561947AFE9D6FAB9AED700BE08";
  static const uint8_t binary_fingerprint[16]; // = {0x41,0x71,0x59,0x56,0x19,0x47,0xAF,0xE9,0xD6,0xFA,0xB9,0xAE,0xD7,0x00,0xBE,0x08};

  FileWriteToken() : blockid(0) {
  }

  virtual ~FileWriteToken() throw() {}

  int32_t blockid;
  std::vector<int32_t>  chunkids;
  std::vector<DatanodeEndpoint>  endpoints;

  void __set_blockid(const int32_t val) {
    blockid = val;
  }

  void __set_chunkids(const std::vector<int32_t> & val) {
    chunkids = val;
  }

  void __set_endpoints(const std::vector<DatanodeEndpoint> & val) {
    endpoints = val;
  }

  bool operator == (const FileWriteToken & rhs) const
  {
    if (!(blockid == rhs.blockid))
      return false;
    if (!(chunkids == rhs.chunkids))
      return false;
    if (!(endpoints == rhs.endpoints))
      return false;
    return true;
  }
  bool operator != (const FileWriteToken &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileWriteToken & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileReadToken {
 public:

  static const char* ascii_fingerprint; // = "BFDAF9305445EF65B35E937060EE9443";
  static const uint8_t binary_fingerprint[16]; // = {0xBF,0xDA,0xF9,0x30,0x54,0x45,0xEF,0x65,0xB3,0x5E,0x93,0x70,0x60,0xEE,0x94,0x43};

  FileReadToken() {
  }

  virtual ~FileReadToken() throw() {}

  std::vector<int32_t>  chunkids;
  std::vector<DatanodeEndpoint>  endpoints;

  void __set_chunkids(const std::vector<int32_t> & val) {
    chunkids = val;
  }

  void __set_endpoints(const std::vector<DatanodeEndpoint> & val) {
    endpoints = val;
  }

  bool operator == (const FileReadToken & rhs) const
  {
    if (!(chunkids == rhs.chunkids))
      return false;
    if (!(endpoints == rhs.endpoints))
      return false;
    return true;
  }
  bool operator != (const FileReadToken &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileReadToken & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileRecoverToken {
 public:

  static const char* ascii_fingerprint; // = "00D98B57963D0B9CE31C9F0F53403AFB";
  static const uint8_t binary_fingerprint[16]; // = {0x00,0xD9,0x8B,0x57,0x96,0x3D,0x0B,0x9C,0xE3,0x1C,0x9F,0x0F,0x53,0x40,0x3A,0xFB};

  FileRecoverToken() : blockid(0), oldchunk(0), newchunk(0) {
  }

  virtual ~FileRecoverToken() throw() {}

  std::vector<DatanodeEndpoint>  endpoints;
  std::vector<int32_t>  chunkids;
  int32_t blockid;
  int32_t oldchunk;
  int32_t newchunk;

  void __set_endpoints(const std::vector<DatanodeEndpoint> & val) {
    endpoints = val;
  }

  void __set_chunkids(const std::vector<int32_t> & val) {
    chunkids = val;
  }

  void __set_blockid(const int32_t val) {
    blockid = val;
  }

  void __set_oldchunk(const int32_t val) {
    oldchunk = val;
  }

  void __set_newchunk(const int32_t val) {
    newchunk = val;
  }

  bool operator == (const FileRecoverToken & rhs) const
  {
    if (!(endpoints == rhs.endpoints))
      return false;
    if (!(chunkids == rhs.chunkids))
      return false;
    if (!(blockid == rhs.blockid))
      return false;
    if (!(oldchunk == rhs.oldchunk))
      return false;
    if (!(newchunk == rhs.newchunk))
      return false;
    return true;
  }
  bool operator != (const FileRecoverToken &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileRecoverToken & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultGetWriteToken {
 public:

  static const char* ascii_fingerprint; // = "F3C754971DB6A8E0F6C1A655C6F485F0";
  static const uint8_t binary_fingerprint[16]; // = {0xF3,0xC7,0x54,0x97,0x1D,0xB6,0xA8,0xE0,0xF6,0xC1,0xA6,0x55,0xC6,0xF4,0x85,0xF0};

  ResultGetWriteToken() {
  }

  virtual ~ResultGetWriteToken() throw() {}

  ThriftResult result;
  FileWriteToken token;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_token(const FileWriteToken& val) {
    token = val;
  }

  bool operator == (const ResultGetWriteToken & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(token == rhs.token))
      return false;
    return true;
  }
  bool operator != (const ResultGetWriteToken &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultGetWriteToken & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultFixWriteToken {
 public:

  static const char* ascii_fingerprint; // = "F3C754971DB6A8E0F6C1A655C6F485F0";
  static const uint8_t binary_fingerprint[16]; // = {0xF3,0xC7,0x54,0x97,0x1D,0xB6,0xA8,0xE0,0xF6,0xC1,0xA6,0x55,0xC6,0xF4,0x85,0xF0};

  ResultFixWriteToken() {
  }

  virtual ~ResultFixWriteToken() throw() {}

  ThriftResult result;
  FileWriteToken token;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_token(const FileWriteToken& val) {
    token = val;
  }

  bool operator == (const ResultFixWriteToken & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(token == rhs.token))
      return false;
    return true;
  }
  bool operator != (const ResultFixWriteToken &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultFixWriteToken & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultGetReadToken {
 public:

  static const char* ascii_fingerprint; // = "9DD198F1076354724B23934EDD179145";
  static const uint8_t binary_fingerprint[16]; // = {0x9D,0xD1,0x98,0xF1,0x07,0x63,0x54,0x72,0x4B,0x23,0x93,0x4E,0xDD,0x17,0x91,0x45};

  ResultGetReadToken() {
  }

  virtual ~ResultGetReadToken() throw() {}

  ThriftResult result;
  FileReadToken token;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_token(const FileReadToken& val) {
    token = val;
  }

  bool operator == (const ResultGetReadToken & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(token == rhs.token))
      return false;
    return true;
  }
  bool operator != (const ResultGetReadToken &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultGetReadToken & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultGetNodeInfo {
 public:

  static const char* ascii_fingerprint; // = "683A2F7B1A60A645E8D67BC5769C877F";
  static const uint8_t binary_fingerprint[16]; // = {0x68,0x3A,0x2F,0x7B,0x1A,0x60,0xA6,0x45,0xE8,0xD6,0x7B,0xC5,0x76,0x9C,0x87,0x7F};

  ResultGetNodeInfo() {
  }

  virtual ~ResultGetNodeInfo() throw() {}

  ThriftResult result;
  DatanodeEndpoint endpoint;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_endpoint(const DatanodeEndpoint& val) {
    endpoint = val;
  }

  bool operator == (const ResultGetNodeInfo & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(endpoint == rhs.endpoint))
      return false;
    return true;
  }
  bool operator != (const ResultGetNodeInfo &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultGetNodeInfo & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultFixRecoverToken {
 public:

  static const char* ascii_fingerprint; // = "E63F4E49238490BACD4B847C980C819A";
  static const uint8_t binary_fingerprint[16]; // = {0xE6,0x3F,0x4E,0x49,0x23,0x84,0x90,0xBA,0xCD,0x4B,0x84,0x7C,0x98,0x0C,0x81,0x9A};

  ResultFixRecoverToken() {
  }

  virtual ~ResultFixRecoverToken() throw() {}

  ThriftResult result;
  FileRecoverToken token;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_token(const FileRecoverToken& val) {
    token = val;
  }

  bool operator == (const ResultFixRecoverToken & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(token == rhs.token))
      return false;
    return true;
  }
  bool operator != (const ResultFixRecoverToken &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultFixRecoverToken & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class DatanodeHeartbeat {
 public:

  static const char* ascii_fingerprint; // = "761DA4D6DC65FA18F7AFCC175BC54E81";
  static const uint8_t binary_fingerprint[16]; // = {0x76,0x1D,0xA4,0xD6,0xDC,0x65,0xFA,0x18,0xF7,0xAF,0xCC,0x17,0x5B,0xC5,0x4E,0x81};

  DatanodeHeartbeat() {
  }

  virtual ~DatanodeHeartbeat() throw() {}

  DatanodeEndpoint endpoint;

  void __set_endpoint(const DatanodeEndpoint& val) {
    endpoint = val;
  }

  bool operator == (const DatanodeHeartbeat & rhs) const
  {
    if (!(endpoint == rhs.endpoint))
      return false;
    return true;
  }
  bool operator != (const DatanodeHeartbeat &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DatanodeHeartbeat & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class RecoveryHeartbeat {
 public:

  static const char* ascii_fingerprint; // = "01B77A1E864D2179E0D9471EEB3DA6F4";
  static const uint8_t binary_fingerprint[16]; // = {0x01,0xB7,0x7A,0x1E,0x86,0x4D,0x21,0x79,0xE0,0xD9,0x47,0x1E,0xEB,0x3D,0xA6,0xF4};

  RecoveryHeartbeat() : load(0) {
  }

  virtual ~RecoveryHeartbeat() throw() {}

  DatanodeEndpoint endpoint;
  int32_t load;

  void __set_endpoint(const DatanodeEndpoint& val) {
    endpoint = val;
  }

  void __set_load(const int32_t val) {
    load = val;
  }

  bool operator == (const RecoveryHeartbeat & rhs) const
  {
    if (!(endpoint == rhs.endpoint))
      return false;
    if (!(load == rhs.load))
      return false;
    return true;
  }
  bool operator != (const RecoveryHeartbeat &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RecoveryHeartbeat & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultGetChunk {
 public:

  static const char* ascii_fingerprint; // = "6ECF9D78DCC8499EA07DE1A8B0F26B3E";
  static const uint8_t binary_fingerprint[16]; // = {0x6E,0xCF,0x9D,0x78,0xDC,0xC8,0x49,0x9E,0xA0,0x7D,0xE1,0xA8,0xB0,0xF2,0x6B,0x3E};

  ResultGetChunk() : data("") {
  }

  virtual ~ResultGetChunk() throw() {}

  ThriftResult result;
  std::string data;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_data(const std::string& val) {
    data = val;
  }

  bool operator == (const ResultGetChunk & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(data == rhs.data))
      return false;
    return true;
  }
  bool operator != (const ResultGetChunk &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultGetChunk & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultGetCode {
 public:

  static const char* ascii_fingerprint; // = "6ECF9D78DCC8499EA07DE1A8B0F26B3E";
  static const uint8_t binary_fingerprint[16]; // = {0x6E,0xCF,0x9D,0x78,0xDC,0xC8,0x49,0x9E,0xA0,0x7D,0xE1,0xA8,0xB0,0xF2,0x6B,0x3E};

  ResultGetCode() : data("") {
  }

  virtual ~ResultGetCode() throw() {}

  ThriftResult result;
  std::string data;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_data(const std::string& val) {
    data = val;
  }

  bool operator == (const ResultGetCode & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(data == rhs.data))
      return false;
    return true;
  }
  bool operator != (const ResultGetCode &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultGetCode & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultReadData {
 public:

  static const char* ascii_fingerprint; // = "6ECF9D78DCC8499EA07DE1A8B0F26B3E";
  static const uint8_t binary_fingerprint[16]; // = {0x6E,0xCF,0x9D,0x78,0xDC,0xC8,0x49,0x9E,0xA0,0x7D,0xE1,0xA8,0xB0,0xF2,0x6B,0x3E};

  ResultReadData() : data("") {
  }

  virtual ~ResultReadData() throw() {}

  ThriftResult result;
  std::string data;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_data(const std::string& val) {
    data = val;
  }

  bool operator == (const ResultReadData & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(data == rhs.data))
      return false;
    return true;
  }
  bool operator != (const ResultReadData &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultReadData & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultReportChunks {
 public:

  static const char* ascii_fingerprint; // = "2349343F5FB03F96FA2E204A54D8CBE1";
  static const uint8_t binary_fingerprint[16]; // = {0x23,0x49,0x34,0x3F,0x5F,0xB0,0x3F,0x96,0xFA,0x2E,0x20,0x4A,0x54,0xD8,0xCB,0xE1};

  ResultReportChunks() {
  }

  virtual ~ResultReportChunks() throw() {}

  ThriftResult result;
  std::set<int32_t>  chunkInUse;
  std::set<int32_t>  chunkUnUse;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_chunkInUse(const std::set<int32_t> & val) {
    chunkInUse = val;
  }

  void __set_chunkUnUse(const std::set<int32_t> & val) {
    chunkUnUse = val;
  }

  bool operator == (const ResultReportChunks & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(chunkInUse == rhs.chunkInUse))
      return false;
    if (!(chunkUnUse == rhs.chunkUnUse))
      return false;
    return true;
  }
  bool operator != (const ResultReportChunks &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultReportChunks & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class ResultGetBlocks {
 public:

  static const char* ascii_fingerprint; // = "43EF4D6B62529D59114D31A8C9141EA4";
  static const uint8_t binary_fingerprint[16]; // = {0x43,0xEF,0x4D,0x6B,0x62,0x52,0x9D,0x59,0x11,0x4D,0x31,0xA8,0xC9,0x14,0x1E,0xA4};

  ResultGetBlocks() {
  }

  virtual ~ResultGetBlocks() throw() {}

  ThriftResult result;
  std::set<int32_t>  blockids;

  void __set_result(const ThriftResult& val) {
    result = val;
  }

  void __set_blockids(const std::set<int32_t> & val) {
    blockids = val;
  }

  bool operator == (const ResultGetBlocks & rhs) const
  {
    if (!(result == rhs.result))
      return false;
    if (!(blockids == rhs.blockids))
      return false;
    return true;
  }
  bool operator != (const ResultGetBlocks &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ResultGetBlocks & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};



#endif
