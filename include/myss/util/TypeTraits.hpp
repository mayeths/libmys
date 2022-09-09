#pragma once


template <typename T>
struct template_parameter;

// template <template <typename ...> class C, typename T1>
// struct template_parameter<C<T1>>
// {
//     using type1 = T1;
// };

template <template <typename ...> class C, typename T1, typename T2>
struct template_parameter<C<T1, T2>>
{
    using type1 = T1;
    using type2 = T2;
};

// template <template <typename ...> class C, typename T1, typename T2, typename T3>
// struct template_parameter<C<T1, T2, T3>>
// {
//     using type1 = T1;
//     using type2 = T2;
//     using type3 = T3;
// };

// template <typename C>
// using template_parameter1_t = typename template_parameter<C>::type1;
// template <typename C>
// using template_parameter2_t = typename template_parameter<C>::type2;
// // template <typename T>
// // using template_parameter3_t = typename template_parameter<T>::type3;


