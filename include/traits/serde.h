#pragma once

#include "basic_types.h"
#include "stb_ds.h"

typedef u8* Stream;
static inline void stream_close(Stream stream) {
    arrfree(stream);
}

#define DECLARE_INTERFACE_Serde(type, prefix) \
Stream prefix##_serialize_fn(struct type* obj); \
bool prefix##_deserialize_fn(struct type* obj, Stream stream);

#define IMPL_INTERFACE_Serde_Serialize_BEGIN(type, prefix) \
Stream prefix##_serialize_fn(struct type* obj) { \
    Stream stream = nullptr;
#define IMPL_INTERFACE_Serde_Serialize_END(type, prefix) \
    return stream; \
}
#define IMPL_INTERFACE_Serde_Deserialize_BEGIN(type, prefix) \
bool prefix##_deserialize_fn(struct type* obj, Stream stream) { \
    usize cursor = 0;
#define IMPL_INTERFACE_Serde_Deserialize_END(type, prefix) \
    stream_close(stream); \
    return true; \
Fail: \
    stream_close(stream); \
    return false; \
}

#define SERIALIZE_FIELD_VALUE(field) do { \
	for (i32 i = 0; i < sizeof(obj->field); i++) { \
		arrput(stream, ((u8*)&(obj->field))[i]); \
	} \
} while(0)
#define SERIALIZE_FIELD_STRING(charptr) do { \
	for (i32 i = 0; obj->charptr[i] != '\0'; i++) { \
		arrput(stream, obj->charptr[i]); \
	} \
	arrput(stream, '\0'); \
} while(0)
#define SERIALIZE_FIELD_ARRAY(array_ptr, array_len) do { \
	for (i32 i = 0; i < array_len; i++) { \
		for (i32 j = 0; j < sizeof(obj->array_ptr[i]); j++) { \
			arrput(stream, ((u8*)&(obj->array_ptr[i]))[j]); \
		} \
	} \
} while(0)

#define DESERIALIZE_FIELD_VALUE(field) do { \
	if (cursor <= arrlen(stream) - sizeof(obj->field)) { \
		for (i32 i = 0; i < sizeof(obj->field); i++) { \
			((u8*)&(obj->field))[i] = stream[i + cursor]; \
		} \
		cursor += sizeof(obj->field); \
	} else { \
		goto Fail; \
	} \
} while(0)
#define DESERIALIZE_FIELD_STRING(charptr) do { \
	usize len = strlen(stream + cursor); \
	if (cursor <= arrlen(stream) - len - 1) { \
		for (i32 i = 0; i < len; i++) { \
			arrput(obj->charptr, (stream + cursor)[i]); \
		} \
		cursor += len + 1; \
	} else { \
		goto Fail; \
	} \
} while(0)
#define DESERIALIZE_FIELD_ARRAY(array_ptr, array_len) do { \
	if (cursor <= arrlen(stream) - array_len) { \
		arrfree(obj->array_ptr); \
		obj->array_ptr = nullptr; \
		arrsetcap(obj->array_ptr, array_len); \
		for (i32 i = 0; i < array_len; i++) { \
			for (i32 j = 0; j < sizeof(obj->array_ptr[i]); j++) { \
				((u8*)&(obj->array_ptr[i]))[j] = stream[i * sizeof(obj->array_ptr[i]) + j]; \
			} \
		} \
	} else { \
		goto Fail; \
	} \
} while(0)
