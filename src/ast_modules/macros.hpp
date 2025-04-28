#ifndef NODE_MACROS_HPP
#define NODE_MACROS_HPP

#define DEFINE_GETTER_SETTER(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(const type &value) noexcept { field = value; }

#define DEFINE_GETTER_SETTER_PTR(type, name, field)    \
    type Get##name() const noexcept { return field; } \
    void Set##name(type value) noexcept { field = value; }

#define DEFINE_GET_SET_MOVE(type, name, field)    \
    const type& Get##name() const noexcept { return field; } \
    void Set##name(type&& value) noexcept { field = std::move(value); } \
    type& Get##name##Mut() noexcept { return field; }

#define UNIQUE_PTR_CAST(ptr, type) \
    std::move(std::unique_ptr<type>(static_cast<type*>(std::move(ptr).release())))

#define DEFINE_FLAGS_OPERATORS(EnumName, BackingType) \
	inline static EnumName operator~(EnumName value) { return (EnumName)~(BackingType)value; } \
	inline static EnumName operator|(EnumName lhs, EnumName rhs) { return (EnumName)((BackingType)lhs | (BackingType)rhs); } \
	inline static EnumName operator&(EnumName lhs, EnumName rhs) { return (EnumName)((BackingType)lhs & (BackingType)rhs); } \
	inline static EnumName operator^(EnumName lhs, EnumName rhs) { return (EnumName)((BackingType)lhs ^ (BackingType)rhs); } \
	inline static EnumName& operator|=(EnumName& lhs, EnumName rhs) { return (EnumName&)((BackingType&)lhs |= (BackingType)rhs); } \
	inline static EnumName& operator&=(EnumName& lhs, EnumName rhs) { return (EnumName&)((BackingType&)lhs &= (BackingType)rhs); } \
	inline static EnumName& operator^=(EnumName& lhs, EnumName rhs) { return (EnumName&)((BackingType&)lhs ^= (BackingType)rhs); }

#endif