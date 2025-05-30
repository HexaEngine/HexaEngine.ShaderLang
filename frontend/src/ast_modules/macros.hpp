#ifndef NODE_MACROS_HPP
#define NODE_MACROS_HPP

#include "utils/macros.hpp"

#define DEFINE_GET_SET_MOVE_CHILD(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { UnregisterChild(field.get()); field = std::move(value); RegisterChild(field.get()); } \
    type& Get##name##Mut() noexcept { return field; }

#define DEFINE_GET_SET_MOVE_CHILDREN(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { UnregisterChildren(field); field = std::move(value); RegisterChildren(field); } \
    type& Get##name##Mut() noexcept { return field; }

#define REGISTER_CHILD(name) \
    RegisterChild(this->name);

#define REGISTER_CHILDREN(name) \
    RegisterChildren(this->name);

#define REGISTER_EXPR(name) \
    RegisterExpression(this, this->name.get());

#define UNIQUE_PTR_CAST_AST(ptr, type) \
    std::move(ast_ptr<type>(static_cast<type*>(std::move(ptr).release())))

#define UNIQUE_PTR_CAST_AST_DYN(ptr, type) \
    std::move(ast_ptr<type>(dynamic_cast<type*>(std::move(ptr).release())))

#endif