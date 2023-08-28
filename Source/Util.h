#pragma once

#include <cmath>
#include <string.h>
#include <type_traits>
#include <vector>


template <typename T>
void ZeroType(T *t, size_t count = 1)
{
  memset(t, 0, sizeof(T) * count);
}


template <typename T>
void CopyType(T *dest, const T *src, size_t count = 1)
{
  memcpy(dest, src, sizeof(T) * count);
}


template <typename T>
constexpr auto EnumValue(T v)
{
  return static_cast<std::underlying_type_t<T>>(v);
}


template <typename T>
bool AreBitwiseEqual(const T &a, const T &b)
{
  return memcmp(&a, &b, sizeof(T)) == 0;
}


// What follows is a bunch of template garbage to help deal with enum classes / flag enum classes


// helper to test if a type is an enum class (vs. just an enum)
template <typename Type, typename = Type>
struct is_enum_class : public std::is_enum<Type>
{
};

template <typename Type>
struct is_enum_class<Type, decltype(Type(+std::declval<Type>()))> : public std::false_type
{
};

// Helper to test if a type is a flag enum class (that is, an enum class that has None and All members)
template <typename Type, typename = Type, typename = Type>
struct is_flag_enum_class : public std::false_type
{
};

template <typename Type>
struct is_flag_enum_class<Type, decltype(Type::None), decltype(Type::All)> : public is_enum_class<Type>
{
};


// Unary flag enum class operators
template <typename Type, typename std::enable_if<is_flag_enum_class<Type>::value, int>::type = 0>
constexpr Type operator ~ (Type a)
{
  return static_cast<Type>(~static_cast<std::underlying_type_t<Type>>(a));
}


// Binary flag enum class operators
template <typename Type, typename std::enable_if<is_flag_enum_class<Type>::value, int>::type = 0>
constexpr Type operator | (Type a, Type b)
{
  return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(a) | static_cast<std::underlying_type_t<Type>>(b));
}


template <typename Type, typename std::enable_if<is_flag_enum_class<Type>::value, int>::type = 0>
constexpr Type operator & (Type a, Type b)
{
  return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(a) & static_cast<std::underlying_type_t<Type>>(b));
}


template <typename Type, typename std::enable_if<is_flag_enum_class<Type>::value, int>::type = 0>
constexpr Type operator ^ (Type a, Type b)
{
  return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(a) ^ static_cast<std::underlying_type_t<Type>>(b));
}


// Assigning flag enum class operators
template <typename Type, typename std::enable_if<is_flag_enum_class<Type>::value, int>::type = 0>
constexpr Type &operator |= (Type &a, Type b)
{
  a = a | b;
  return a;
}


template <typename Type, typename std::enable_if<is_flag_enum_class<Type>::value, int>::type = 0>
constexpr Type &operator &= (Type &a, Type b)
{
  a = a & b;
  return a;
}


template <typename Type, typename std::enable_if<is_flag_enum_class<Type>::value, int>::type = 0>
constexpr Type &operator ^= (Type &a, Type b)
{
  a = a ^ b;
  return a;
}


// Get the length of an array from its type

template <typename T> size_t k_arrayLength = 0;
template <typename T> size_t k_arrayLength<T &> = k_arrayLength<T>;
template <typename T, size_t N> size_t k_arrayLength<T[N]> = N;
