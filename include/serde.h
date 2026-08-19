#pragma once

#include "basic_types.h"

typedef u8* Stream;
#define IMPL_INTERFACE_Serde(type, prefix) \
Stream prefix##_serialize_fn(struct type* obj); \
bool prefix##_deserialize_fn(struct type* obj, Stream stream);

#define SERIALIZE_FIELD_VALUE(stream, obj, field) do { \
	for (i32 i = 0; i < sizeof((obj)->(field)); i++) { \
		arrput(stream, ((u8*)&((obj)->(field)))[i]); \
	} \
} while(0)
#define SERIALIZE_FIELD_STRING(stream, obj, charptr) do { \
	for (i32 i = 0; (obj)->(charptr)[i] != '\0'; i++) { \
		arrput(stream, (obj)->(charptr)[i]); \
	} \
	arrput(stream, '\0'); \
} while(0)
#define SERIALIZE_FIELD_ARRAY(stream, obj, array_ptr, array_len) do { \
	for (i32 i = 0; i < array_len; i++) { \
		for (i32 j = 0; j < sizeof((obj)->(array_ptr)[i]); j++) { \
			arrput(stream, ((u8*)&((obj)->(array_ptr)[i]))[j]); \
		} \
	} \
} while(0)

#define DESERIALIZE_FIELD_VALUE(obj, field, stream, cursor, fail_label) do { \
	if (cursor <= arrlen(stream) - sizeof((obj)->(field))) { \
		for (i32 i = 0; i < sizeof((obj)->(field)); i++) { \
			((u8*)&((obj)->(field)))[i] = stream[i + cursor]; \
		} \
		cursor += sizeof((obj)->(field)); \
	} else { \
		goto fail_label; \
	} \
} while(0)
#define DESERIALIZE_FIELD_STRING(obj, charptr, stream, cursor, fail_label) do { \
	usize len = strlen(stream + cursor); \
	if (cursor <= arrlen(stream) - len - 1) { \
		for (i32 i = 0; i < len; i++) { \
			arrput((obj)->(charptr), (stream + cursor)[i]); \
		} \
		cursor += len + 1; \
	} else { \
		goto fail_label; \
	} \
} while(0)
#define DESERIALIZE_FIELD_ARRAY(obj, array_ptr, array_len, stream, cursor, fail_label) do { \
	if (cursor <= arrlen(stream) - array_len) { \
		arrfree((obj)->(array_ptr)); \
		(obj)->(array_ptr) = nullptr; \
		arrsetcap((obj)->(array_ptr), array_len); \
		for (i32 i = 0; i < array_len; i++) { \
			for (i32 j = 0; j < sizeof((obj)->(array_ptr)[i]); j++) { \
				((u8*)&((obj)->(array_ptr)[i]))[j] = stream[i * sizeof((obj)->(array_ptr)[i]) + j]; \
			} \
		} \
	} else { \
		goto fail_label; \
	} \
} while(0)
