#ifndef __OID_H__
#define __OID_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct object_id {
    uint8_t id[12];
  } object_id;

  int oid_construct(object_id* oid);
  int oid_construct_with_time(object_id* oid, uint32_t time);
  int oid_construct_with_buf(object_id* oid, const uint8_t* buf, uint32_t len);
  int oid_construct_with_oid(object_id* oid, const object_id* other);

  int oid_generate(uint32_t time, uint8_t* id);
  int oid_create_from_hex_string(const char* hex_string, object_id* oid);
  int oid_to_hex_string(const object_id* oid, char* res);
  int oid_is_valid(const char* res);

  int oid_equals_buf(const object_id* oid, const uint8_t* buf, uint32_t len);
  int oid_equals_oid(const object_id* oid, const object_id* other);
  int oid_create_from_time(uint32_t time, object_id* oid);
  uint32_t oid_get_timestamp(const object_id* oid);

#ifdef __cplusplus
}
#endif

#endif // !__OID_H__
