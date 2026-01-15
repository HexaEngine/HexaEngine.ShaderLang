#ifndef NODE_MACROS_HPP
#define NODE_MACROS_HPP

#include "utils/macros.hpp"

#define DEFINE_GET_SET_MOVE_CHILD(type, name, field)    \
    type Get##name() const noexcept { return field; } \
    void Set##name(type value) noexcept { UnregisterChild(field); field = value; RegisterChild(field); } \
    type& Get##name##Mut() noexcept { return field; }

#define DEFINE_GET_SET_MOVE_CHILDREN(type, name, field)    \
    type Get##name() const noexcept { return field; } \
    void Set##name(type value) noexcept { UnregisterChildren(field); field = value; RegisterChildren(field); } \
    type& Get##name##Mut() noexcept { return field; }

#define REGISTER_CHILD(name) \
    RegisterChild(this->name);

#define REGISTER_CHILDREN(name) \
    RegisterChildren(this->name);

#define REGISTER_CHILD_PTR(ptr, name) \
    ptr->RegisterChild(ptr->name);

#define REGISTER_CHILDREN_PTR(ptr, name) \
    ptr->RegisterChildren(ptr->name);

#define REGISTER_EXPR(name) \
    RegisterExpression(this, this->name);

#define UNIQUE_PTR_CAST_AST(ptr, type) \
    std::move(ast_ptr<type>(static_cast<type*>(std::move(ptr).release())))

#define UNIQUE_PTR_CAST_AST_DYN(ptr, type) \
    std::move(ast_ptr<type>(dynamic_cast<type*>(std::move(ptr).release())))

#define AST_ITERATE_CHILDREN(getter) for (auto c : getter()) cb(c, userdata);
#define AST_ITERATE_CHILD(getter) cb(getter, userdata);

#define AST_ITERATE_CHILDREN_MUT(getter) for (auto& c : getter()) cb(*reinterpret_cast<ASTNode**>(&c), userdata);
#define AST_ITERATE_CHILD_MUT(getter) cb(*reinterpret_cast<ASTNode**>(&getter), userdata);

#endif