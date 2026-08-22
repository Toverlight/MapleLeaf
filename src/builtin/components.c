#include <maple/builtin/components.h>
#include <stb_ds.h>

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

bool node_add_child(Node* node, Node* child) {
    if (!node || !child) return false;
    if (child->parent == node) return false;
    arrput(node->children, child);
    child->parent = node;
    return true;
}
bool node_remove_child(Node* node, Node* child) {
    if (!node || !child) return false;
    for (isize i = 0; i < arrlen(node->children); i++) {
        if (node->children[i] == child) {
            arrdel(node->children, i);
            break;
        }
    }
    return true;
}
bool node_set_parent(Node* node, Node* parent) {
    if (!node) return false;
    if (node->parent == parent) return false;
    if (node->parent) {
        for (isize i = 0; i < arrlen(node->parent->children); i++) {
            if (node->parent->children[i] == node) {
                arrdel(node->parent->children, i);
                break;
            }
        }
    }
    node->parent = parent;
    if (parent) {
        arrput(parent->children, node);
    }
    return true;
}
