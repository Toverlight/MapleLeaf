#include "builtin/components.h"

IMPL_COMP(Transform)

IMPL_INTERFACE_BEGIN(Transform, Serde, transform, Serialize)
    SERIALIZE_FIELD_VALUE(px);
    SERIALIZE_FIELD_VALUE(py);
    SERIALIZE_FIELD_VALUE(r);
    SERIALIZE_FIELD_VALUE(sx);
    SERIALIZE_FIELD_VALUE(sy);
IMPL_INTERFACE_END(Transform, Serde, transform, Serialize)

IMPL_INTERFACE_BEGIN(Transform, Serde, transform, Deserialize)
    DESERIALIZE_FIELD_VALUE(px);
    DESERIALIZE_FIELD_VALUE(py);
    DESERIALIZE_FIELD_VALUE(r);
    DESERIALIZE_FIELD_VALUE(sx);
    DESERIALIZE_FIELD_VALUE(sy);
IMPL_INTERFACE_END(Transform, Serde, transform, Deserialize)


IMPL_COMP(Node)

// TODO Node Serde接口
