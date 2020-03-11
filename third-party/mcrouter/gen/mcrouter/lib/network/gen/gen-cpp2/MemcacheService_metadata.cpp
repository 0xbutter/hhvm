/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include <thrift/lib/cpp2/gen/module_metadata_cpp.h>
#include "mcrouter/lib/network/gen/gen-cpp2/MemcacheService_metadata.h"

namespace apache {
namespace thrift {
namespace detail {
namespace md {
using ThriftMetadata = ::apache::thrift::metadata::ThriftMetadata;
using ThriftPrimitiveType = ::apache::thrift::metadata::ThriftPrimitiveType;
using ThriftType = ::apache::thrift::metadata::ThriftType;
using ThriftService = ::apache::thrift::metadata::ThriftService;
using ThriftServiceContext = ::apache::thrift::metadata::ThriftServiceContext;
using ThriftFunctionGenerator = void (*)(ThriftMetadata&, ThriftService&);



void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGet(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcGet";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McGetReply", std::make_unique<Struct< ::facebook::memcache::thrift::McGetReply>>("Memcache.McGetReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcGet_request_1;
  MemcacheService_Memcache_mcGet_request_1.id = 1;
  MemcacheService_Memcache_mcGet_request_1.name = "request";
  MemcacheService_Memcache_mcGet_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcGet_request_1_type = std::make_unique<Typedef>("Memcache.McGetRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McGetRequest>>("Memcache.McGetRequest"));
  MemcacheService_Memcache_mcGet_request_1_type->writeAndGenType(MemcacheService_Memcache_mcGet_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcGet_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcSet(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcSet";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McSetReply", std::make_unique<Struct< ::facebook::memcache::thrift::McSetReply>>("Memcache.McSetReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcSet_request_1;
  MemcacheService_Memcache_mcSet_request_1.id = 1;
  MemcacheService_Memcache_mcSet_request_1.name = "request";
  MemcacheService_Memcache_mcSet_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcSet_request_1_type = std::make_unique<Typedef>("Memcache.McSetRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McSetRequest>>("Memcache.McSetRequest"));
  MemcacheService_Memcache_mcSet_request_1_type->writeAndGenType(MemcacheService_Memcache_mcSet_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcSet_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcDelete(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcDelete";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McDeleteReply", std::make_unique<Struct< ::facebook::memcache::thrift::McDeleteReply>>("Memcache.McDeleteReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcDelete_request_1;
  MemcacheService_Memcache_mcDelete_request_1.id = 1;
  MemcacheService_Memcache_mcDelete_request_1.name = "request";
  MemcacheService_Memcache_mcDelete_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcDelete_request_1_type = std::make_unique<Typedef>("Memcache.McDeleteRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McDeleteRequest>>("Memcache.McDeleteRequest"));
  MemcacheService_Memcache_mcDelete_request_1_type->writeAndGenType(MemcacheService_Memcache_mcDelete_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcDelete_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcLeaseGet(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcLeaseGet";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McLeaseGetReply", std::make_unique<Struct< ::facebook::memcache::thrift::McLeaseGetReply>>("Memcache.McLeaseGetReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcLeaseGet_request_1;
  MemcacheService_Memcache_mcLeaseGet_request_1.id = 1;
  MemcacheService_Memcache_mcLeaseGet_request_1.name = "request";
  MemcacheService_Memcache_mcLeaseGet_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcLeaseGet_request_1_type = std::make_unique<Typedef>("Memcache.McLeaseGetRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McLeaseGetRequest>>("Memcache.McLeaseGetRequest"));
  MemcacheService_Memcache_mcLeaseGet_request_1_type->writeAndGenType(MemcacheService_Memcache_mcLeaseGet_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcLeaseGet_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcLeaseSet(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcLeaseSet";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McLeaseSetReply", std::make_unique<Struct< ::facebook::memcache::thrift::McLeaseSetReply>>("Memcache.McLeaseSetReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcLeaseSet_request_1;
  MemcacheService_Memcache_mcLeaseSet_request_1.id = 1;
  MemcacheService_Memcache_mcLeaseSet_request_1.name = "request";
  MemcacheService_Memcache_mcLeaseSet_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcLeaseSet_request_1_type = std::make_unique<Typedef>("Memcache.McLeaseSetRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McLeaseSetRequest>>("Memcache.McLeaseSetRequest"));
  MemcacheService_Memcache_mcLeaseSet_request_1_type->writeAndGenType(MemcacheService_Memcache_mcLeaseSet_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcLeaseSet_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcAdd(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcAdd";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McAddReply", std::make_unique<Struct< ::facebook::memcache::thrift::McAddReply>>("Memcache.McAddReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcAdd_request_1;
  MemcacheService_Memcache_mcAdd_request_1.id = 1;
  MemcacheService_Memcache_mcAdd_request_1.name = "request";
  MemcacheService_Memcache_mcAdd_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcAdd_request_1_type = std::make_unique<Typedef>("Memcache.McAddRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McAddRequest>>("Memcache.McAddRequest"));
  MemcacheService_Memcache_mcAdd_request_1_type->writeAndGenType(MemcacheService_Memcache_mcAdd_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcAdd_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcReplace(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcReplace";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McReplaceReply", std::make_unique<Struct< ::facebook::memcache::thrift::McReplaceReply>>("Memcache.McReplaceReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcReplace_request_1;
  MemcacheService_Memcache_mcReplace_request_1.id = 1;
  MemcacheService_Memcache_mcReplace_request_1.name = "request";
  MemcacheService_Memcache_mcReplace_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcReplace_request_1_type = std::make_unique<Typedef>("Memcache.McReplaceRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McReplaceRequest>>("Memcache.McReplaceRequest"));
  MemcacheService_Memcache_mcReplace_request_1_type->writeAndGenType(MemcacheService_Memcache_mcReplace_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcReplace_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGets(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcGets";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McGetsReply", std::make_unique<Struct< ::facebook::memcache::thrift::McGetsReply>>("Memcache.McGetsReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcGets_request_1;
  MemcacheService_Memcache_mcGets_request_1.id = 1;
  MemcacheService_Memcache_mcGets_request_1.name = "request";
  MemcacheService_Memcache_mcGets_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcGets_request_1_type = std::make_unique<Typedef>("Memcache.McGetsRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McGetsRequest>>("Memcache.McGetsRequest"));
  MemcacheService_Memcache_mcGets_request_1_type->writeAndGenType(MemcacheService_Memcache_mcGets_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcGets_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcCas(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcCas";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McCasReply", std::make_unique<Struct< ::facebook::memcache::thrift::McCasReply>>("Memcache.McCasReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcCas_request_1;
  MemcacheService_Memcache_mcCas_request_1.id = 1;
  MemcacheService_Memcache_mcCas_request_1.name = "request";
  MemcacheService_Memcache_mcCas_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcCas_request_1_type = std::make_unique<Typedef>("Memcache.McCasRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McCasRequest>>("Memcache.McCasRequest"));
  MemcacheService_Memcache_mcCas_request_1_type->writeAndGenType(MemcacheService_Memcache_mcCas_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcCas_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcIncr(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcIncr";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McIncrReply", std::make_unique<Struct< ::facebook::memcache::thrift::McIncrReply>>("Memcache.McIncrReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcIncr_request_1;
  MemcacheService_Memcache_mcIncr_request_1.id = 1;
  MemcacheService_Memcache_mcIncr_request_1.name = "request";
  MemcacheService_Memcache_mcIncr_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcIncr_request_1_type = std::make_unique<Typedef>("Memcache.McIncrRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McIncrRequest>>("Memcache.McIncrRequest"));
  MemcacheService_Memcache_mcIncr_request_1_type->writeAndGenType(MemcacheService_Memcache_mcIncr_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcIncr_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcDecr(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcDecr";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McDecrReply", std::make_unique<Struct< ::facebook::memcache::thrift::McDecrReply>>("Memcache.McDecrReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcDecr_request_1;
  MemcacheService_Memcache_mcDecr_request_1.id = 1;
  MemcacheService_Memcache_mcDecr_request_1.name = "request";
  MemcacheService_Memcache_mcDecr_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcDecr_request_1_type = std::make_unique<Typedef>("Memcache.McDecrRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McDecrRequest>>("Memcache.McDecrRequest"));
  MemcacheService_Memcache_mcDecr_request_1_type->writeAndGenType(MemcacheService_Memcache_mcDecr_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcDecr_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcMetaget(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcMetaget";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McMetagetReply", std::make_unique<Struct< ::facebook::memcache::thrift::McMetagetReply>>("Memcache.McMetagetReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcMetaget_request_1;
  MemcacheService_Memcache_mcMetaget_request_1.id = 1;
  MemcacheService_Memcache_mcMetaget_request_1.name = "request";
  MemcacheService_Memcache_mcMetaget_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcMetaget_request_1_type = std::make_unique<Typedef>("Memcache.McMetagetRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McMetagetRequest>>("Memcache.McMetagetRequest"));
  MemcacheService_Memcache_mcMetaget_request_1_type->writeAndGenType(MemcacheService_Memcache_mcMetaget_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcMetaget_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcAppend(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcAppend";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McAppendReply", std::make_unique<Struct< ::facebook::memcache::thrift::McAppendReply>>("Memcache.McAppendReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcAppend_request_1;
  MemcacheService_Memcache_mcAppend_request_1.id = 1;
  MemcacheService_Memcache_mcAppend_request_1.name = "request";
  MemcacheService_Memcache_mcAppend_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcAppend_request_1_type = std::make_unique<Typedef>("Memcache.McAppendRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McAppendRequest>>("Memcache.McAppendRequest"));
  MemcacheService_Memcache_mcAppend_request_1_type->writeAndGenType(MemcacheService_Memcache_mcAppend_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcAppend_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcPrepend(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcPrepend";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McPrependReply", std::make_unique<Struct< ::facebook::memcache::thrift::McPrependReply>>("Memcache.McPrependReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcPrepend_request_1;
  MemcacheService_Memcache_mcPrepend_request_1.id = 1;
  MemcacheService_Memcache_mcPrepend_request_1.name = "request";
  MemcacheService_Memcache_mcPrepend_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcPrepend_request_1_type = std::make_unique<Typedef>("Memcache.McPrependRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McPrependRequest>>("Memcache.McPrependRequest"));
  MemcacheService_Memcache_mcPrepend_request_1_type->writeAndGenType(MemcacheService_Memcache_mcPrepend_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcPrepend_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcTouch(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcTouch";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McTouchReply", std::make_unique<Struct< ::facebook::memcache::thrift::McTouchReply>>("Memcache.McTouchReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcTouch_request_1;
  MemcacheService_Memcache_mcTouch_request_1.id = 1;
  MemcacheService_Memcache_mcTouch_request_1.name = "request";
  MemcacheService_Memcache_mcTouch_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcTouch_request_1_type = std::make_unique<Typedef>("Memcache.McTouchRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McTouchRequest>>("Memcache.McTouchRequest"));
  MemcacheService_Memcache_mcTouch_request_1_type->writeAndGenType(MemcacheService_Memcache_mcTouch_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcTouch_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcFlushRe(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcFlushRe";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McFlushReReply", std::make_unique<Struct< ::facebook::memcache::thrift::McFlushReReply>>("Memcache.McFlushReReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcFlushRe_request_1;
  MemcacheService_Memcache_mcFlushRe_request_1.id = 1;
  MemcacheService_Memcache_mcFlushRe_request_1.name = "request";
  MemcacheService_Memcache_mcFlushRe_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcFlushRe_request_1_type = std::make_unique<Typedef>("Memcache.McFlushReRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McFlushReRequest>>("Memcache.McFlushReRequest"));
  MemcacheService_Memcache_mcFlushRe_request_1_type->writeAndGenType(MemcacheService_Memcache_mcFlushRe_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcFlushRe_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcFlushAll(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcFlushAll";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McFlushAllReply", std::make_unique<Struct< ::facebook::memcache::thrift::McFlushAllReply>>("Memcache.McFlushAllReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcFlushAll_request_1;
  MemcacheService_Memcache_mcFlushAll_request_1.id = 1;
  MemcacheService_Memcache_mcFlushAll_request_1.name = "request";
  MemcacheService_Memcache_mcFlushAll_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcFlushAll_request_1_type = std::make_unique<Typedef>("Memcache.McFlushAllRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McFlushAllRequest>>("Memcache.McFlushAllRequest"));
  MemcacheService_Memcache_mcFlushAll_request_1_type->writeAndGenType(MemcacheService_Memcache_mcFlushAll_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcFlushAll_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGat(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcGat";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McGatReply", std::make_unique<Struct< ::facebook::memcache::thrift::McGatReply>>("Memcache.McGatReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcGat_request_1;
  MemcacheService_Memcache_mcGat_request_1.id = 1;
  MemcacheService_Memcache_mcGat_request_1.name = "request";
  MemcacheService_Memcache_mcGat_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcGat_request_1_type = std::make_unique<Typedef>("Memcache.McGatRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McGatRequest>>("Memcache.McGatRequest"));
  MemcacheService_Memcache_mcGat_request_1_type->writeAndGenType(MemcacheService_Memcache_mcGat_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcGat_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGats(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcGats";
  auto func_ret_type = std::make_unique<Typedef>("Memcache.McGatsReply", std::make_unique<Struct< ::facebook::memcache::thrift::McGatsReply>>("Memcache.McGatsReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcGats_request_1;
  MemcacheService_Memcache_mcGats_request_1.id = 1;
  MemcacheService_Memcache_mcGats_request_1.name = "request";
  MemcacheService_Memcache_mcGats_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcGats_request_1_type = std::make_unique<Typedef>("Memcache.McGatsRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McGatsRequest>>("Memcache.McGatsRequest"));
  MemcacheService_Memcache_mcGats_request_1_type->writeAndGenType(MemcacheService_Memcache_mcGats_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcGats_request_1));
  service.functions.push_back(std::move(func));
}
void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcVersion(ThriftMetadata& metadata, ThriftService& service) {
  ::apache::thrift::metadata::ThriftFunction func;
  (void)metadata;
  func.name = "mcVersion";
  auto func_ret_type = std::make_unique<Typedef>("Common.McVersionReply", std::make_unique<Struct< ::facebook::memcache::thrift::McVersionReply>>("Common.McVersionReply"));
  func_ret_type->writeAndGenType(func.returnType, metadata);
  ::apache::thrift::metadata::ThriftField MemcacheService_Memcache_mcVersion_request_1;
  MemcacheService_Memcache_mcVersion_request_1.id = 1;
  MemcacheService_Memcache_mcVersion_request_1.name = "request";
  MemcacheService_Memcache_mcVersion_request_1.is_optional = false;
  auto MemcacheService_Memcache_mcVersion_request_1_type = std::make_unique<Typedef>("Common.McVersionRequest", std::make_unique<Struct< ::facebook::memcache::thrift::McVersionRequest>>("Common.McVersionRequest"));
  MemcacheService_Memcache_mcVersion_request_1_type->writeAndGenType(MemcacheService_Memcache_mcVersion_request_1.type, metadata);
  func.arguments.push_back(std::move(MemcacheService_Memcache_mcVersion_request_1));
  service.functions.push_back(std::move(func));
}

void ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen(ThriftMetadata& metadata, ThriftServiceContext& context) {
  (void) metadata;
  ::apache::thrift::metadata::ThriftService MemcacheService_Memcache;
  MemcacheService_Memcache.name = "MemcacheService.Memcache";
  static const ThriftFunctionGenerator functions[] = {
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGet,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcSet,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcDelete,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcLeaseGet,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcLeaseSet,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcAdd,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcReplace,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGets,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcCas,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcIncr,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcDecr,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcMetaget,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcAppend,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcPrepend,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcTouch,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcFlushRe,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcFlushAll,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGat,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcGats,
    ServiceMetadata<::facebook::memcache::thrift::MemcacheSvIf>::gen_mcVersion,
  };
  for (auto& function_gen : functions) {
    function_gen(metadata, MemcacheService_Memcache);
  }
  context.set_service_info(std::move(MemcacheService_Memcache));
  ::apache::thrift::metadata::ThriftModuleContext module;
  module.set_name("MemcacheService");
  context.set_module(std::move(module));
}
} // namespace md
} // namespace detail
} // namespace thrift
} // namespace apache