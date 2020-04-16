/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-present
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_RANGE_CONVERSION_HPP
#define RANGES_V3_RANGE_CONVERSION_HPP

#include <vector>

#include <meta/meta.hpp>

#include <range/v3/range_fwd.hpp>

#include <range/v3/action/concepts.hpp>
#include <range/v3/functional/pipeable.hpp>
#include <range/v3/iterator/common_iterator.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/traits.hpp>
#include <range/v3/utility/static_const.hpp>

#include <range/v3/detail/disable_warnings.hpp>

namespace ranges
{
    /// \cond
    namespace detail
    {
        struct to_container
        {
            template<typename MetaFn>
            struct fn;

            template<typename MetaFn, typename Fn>
            struct closure;

            template<typename MetaFn, typename Rng>
            using container_t = meta::invoke<MetaFn, Rng>;

            template<typename Rng, typename MetaFn>
            friend auto operator|(Rng && rng,
                                  closure<MetaFn, fn<MetaFn>> (*)(to_container))
                -> CPP_broken_friend_ret(container_t<MetaFn, Rng>)( //
                    requires invocable<fn<MetaFn>, Rng>)
            {
                return fn<MetaFn>{}(static_cast<Rng &&>(rng));
            }

            template<typename MetaFn, typename Pipeable>
            friend auto operator|(closure<MetaFn, fn<MetaFn>> (*)(to_container),
                                  Pipeable pipe)
                -> CPP_broken_friend_ret(
                    closure<MetaFn, composed<Pipeable, fn<MetaFn>>>)( //
                    requires(is_pipeable_v<Pipeable>))
            {
                return closure<MetaFn, composed<Pipeable, fn<MetaFn>>>{
                    compose(static_cast<Pipeable &&>(pipe), fn<MetaFn>{})};
            }
        };

        // A simple, light-weight transform iterator that applies ranges::to
        // to each element in the range. Used by ranges::to to convert a range
        // of ranges into a container of containers.
        template<typename Rng, typename Cont>
        struct to_container_iterator
        {
        private:
            using I = range_cpp17_iterator_t<Rng>;
            using ValueType = range_value_t<Cont>;
            I it_;

        public:
            using difference_type = typename std::iterator_traits<I>::difference_type;
            using value_type = ValueType;
            using reference = ValueType;
            using pointer = typename std::iterator_traits<I>::pointer;
            using iterator_category = typename std::iterator_traits<I>::iterator_category;

            to_container_iterator() = default;
            template<typename OtherIt>
            to_container_iterator(OtherIt it)
              : it_(std::move(it))
            {}
            friend bool operator==(to_container_iterator const & a,
                                   to_container_iterator const & b)
            {
                return a.it_ == b.it_;
            }
            friend bool operator!=(to_container_iterator const & a,
                                   to_container_iterator const & b)
            {
                return !(a == b);
            }
            reference operator*() const
            {
                return to_container::fn<meta::id<ValueType>>{}(*it_);
            }
            to_container_iterator & operator++()
            {
                ++it_;
                return *this;
            }
            to_container_iterator operator++(int)
            {
                auto tmp = *this;
                ++it_;
                return tmp;
            }
            CPP_member
            auto operator--() -> CPP_ret(to_container_iterator &)(
                requires derived_from<iterator_category, std::bidirectional_iterator_tag>)
            {
                --it_;
                return *this;
            }
            CPP_member
            auto operator--(int) -> CPP_ret(to_container_iterator &)(
                requires derived_from<iterator_category, std::bidirectional_iterator_tag>)
            {
                auto tmp = *this;
                ++it_;
                return tmp;
            }
            CPP_member
            auto operator+=(difference_type n) -> CPP_ret(to_container_iterator &)(
                requires derived_from<iterator_category, std::random_access_iterator_tag>)
            {
                it_ += n;
                return *this;
            }
            CPP_member
            auto operator-=(difference_type n) -> CPP_ret(to_container_iterator &)(
                requires derived_from<iterator_category, std::random_access_iterator_tag>)
            {
                it_ -= n;
                return *this;
            }
            CPP_broken_friend_member
            friend auto operator+(to_container_iterator i, difference_type n) //
                -> CPP_broken_friend_ret(to_container_iterator)(
                    requires derived_from<iterator_category,
                                          std::random_access_iterator_tag>)
            {
                return i += n;
            }
            CPP_broken_friend_member
            friend auto operator-(to_container_iterator i, difference_type n) //
                -> CPP_broken_friend_ret(to_container_iterator)(
                    requires derived_from<iterator_category,
                                          std::random_access_iterator_tag>)
            {
                return i -= n;
            }
            CPP_broken_friend_member
            friend auto operator-(difference_type n, to_container_iterator i) //
                -> CPP_broken_friend_ret(to_container_iterator)(
                    requires derived_from<iterator_category,
                                          std::random_access_iterator_tag>)
            {
                return i -= n;
            }
            CPP_broken_friend_member
            friend auto operator-(to_container_iterator const & i,
                                  to_container_iterator const & j) //
                -> CPP_broken_friend_ret(difference_type)(
                    requires derived_from<iterator_category,
                                          std::random_access_iterator_tag>)
            {
                return i.it_ - j.it_;
            }
            CPP_member
            auto operator[](difference_type n) const -> CPP_ret(reference)(
                requires derived_from<iterator_category, std::random_access_iterator_tag>)
            {
                return *(*this + n);
            }
        };

        template<typename Rng, typename Cont>
        using to_container_iterator_t =
            enable_if_t<(bool)range<Rng>, to_container_iterator<Rng, Cont>>;

        // clang-format off
        template<typename Cont>
        CPP_concept_fragment(has_allocator_type_, requires()(0) &&
            ranges::type<typename Cont::allocator_type>
        );
        template<typename Cont>
        CPP_concept_bool has_allocator_type =
            CPP_fragment(detail::has_allocator_type_, Cont);

        template<typename Rng>
        CPP_concept_bool range_and_not_view =
            ranges::defer::range<Rng> && !ranges::defer::view_<Rng>;

        template<typename Rng, typename Cont>
        CPP_concept_fragment(convertible_to_cont_impl_frag_, requires()(0) &&
            constructible_from<range_value_t<Cont>, range_reference_t<Rng>> &&
            constructible_from<
                Cont,
                range_cpp17_iterator_t<Rng>,
                range_cpp17_iterator_t<Rng>>
        );
        template<typename Rng, typename Cont>
        CPP_concept_bool convertible_to_cont_impl_ =
            range_and_not_view<Cont> && move_constructible<Cont> &&
            CPP_fragment(detail::convertible_to_cont_impl_frag_, Rng, Cont);

        template<typename Rng, typename Cont>
        CPP_concept_fragment(convertible_to_cont_cont_impl_frag_, requires()(0) &&
                range_and_not_view<range_value_t<Cont>> &&
                // Test that each element of the input range can be ranges::to<>
                // to the output container.
                invocable<
                    to_container::fn<meta::id<range_value_t<Cont>>>,
                    range_reference_t<Rng>> &&
                constructible_from<
                    Cont,
                    to_container_iterator_t<Rng, Cont>,
                    to_container_iterator_t<Rng, Cont>>
        );
        template<typename Rng, typename Cont>
        CPP_concept_bool convertible_to_cont_cont_impl_ =
            range<Cont> && (!view_<Cont>) && move_constructible<Cont> &&
            CPP_fragment(detail::convertible_to_cont_cont_impl_frag_, Rng, Cont);

        namespace defer
        {
            template<typename Cont>
            CPP_concept has_allocator_type =
                CPP_defer(detail::has_allocator_type, Cont);

            template<typename Rng, typename Cont>
            CPP_concept convertible_to_cont_impl_ =
                CPP_defer(detail::convertible_to_cont_impl_, Rng, Cont);

            template<typename Rng, typename Cont>
            CPP_concept convertible_to_cont_cont_impl_ =
                CPP_defer(detail::convertible_to_cont_cont_impl_, Rng, Cont);
        }

        template<typename Rng, typename Cont>
        CPP_concept_bool convertible_to_cont =
            defer::has_allocator_type<Cont> && // HACKHACK
            defer::convertible_to_cont_impl_<Rng, Cont>;

        template<typename Rng, typename Cont>
        CPP_concept_bool convertible_to_cont_cont =
            defer::has_allocator_type<Cont> && // HACKHACK
            defer::convertible_to_cont_cont_impl_<Rng, Cont>;

        namespace defer
        {
            template<typename Rng, typename Cont>
            CPP_concept convertible_to_cont =
                CPP_defer(detail::convertible_to_cont, Rng, Cont);

            template<typename Rng, typename Cont>
            CPP_concept convertible_to_cont_cont =
                CPP_defer(detail::convertible_to_cont_cont, Rng, Cont);
        }

        template<typename C, typename I, typename R>
        CPP_concept_bool to_container_reserve =
            reservable_with_assign<C, I> &&
            sized_range<R>;

        template<typename MetaFn, typename Rng>
        using container_t = meta::invoke<MetaFn, Rng>;

        template<typename Rng, typename MetaFn>
        CPP_concept_bool convertible_to_cont_cont_or_cont =
            defer::convertible_to_cont_cont<Rng, container_t<MetaFn, Rng>> ||
            defer::convertible_to_cont<Rng, container_t<MetaFn, Rng>>;

        namespace defer
        {
            template<typename Rng, typename MetaFn>
            CPP_concept convertible_to_cont_cont_or_cont =
                CPP_defer(detail::convertible_to_cont_cont_or_cont, Rng, MetaFn);
        }
        // clang-format on

        struct RANGES_STRUCT_WITH_ADL_BARRIER(to_container_closure_base)
        {
            CPP_template(typename Rng, typename MetaFn, typename Fn)(     //
                requires ranges::defer::input_range<Rng> &&               //
                    defer::convertible_to_cont_cont_or_cont<Rng, MetaFn>) //
                friend constexpr auto
                operator|(Rng && rng, to_container::closure<MetaFn, Fn> fn)
            {
                return static_cast<Fn &&>(fn)(static_cast<Rng &&>(rng));
            }

            template<typename MetaFn, typename Fn, typename Pipeable>
            friend constexpr auto operator|(to_container::closure<MetaFn, Fn> sh,
                                            Pipeable pipe)
                -> CPP_broken_friend_ret(
                    to_container::closure<MetaFn, composed<Pipeable, Fn>>)(
                    requires(is_pipeable_v<Pipeable>))
            {
                return to_container::closure<MetaFn, composed<Pipeable, Fn>>{
                    compose(static_cast<Pipeable &&>(pipe), static_cast<Fn &&>(sh))};
            }
        };

        template<typename MetaFn, typename Fn>
        struct to_container::closure
          : to_container_closure_base
          , Fn
        {
            closure() = default;
            constexpr explicit closure(Fn fn)
              : Fn(static_cast<Fn &&>(fn))
            {}
        };

        template<typename MetaFn>
        struct to_container::fn
        {
        private:
            template<typename Cont, typename I, typename Rng>
            static Cont impl(Rng && rng, std::false_type)
            {
                return Cont(I{ranges::begin(rng)}, I{ranges::end(rng)});
            }
            template<typename Cont, typename I, typename Rng>
            static auto impl(Rng && rng, std::true_type)
            {
                Cont c;
                auto const rng_size = ranges::size(rng);
                using size_type = decltype(c.max_size());
                using C = common_type_t<range_size_t<Rng>, size_type>;
                RANGES_EXPECT(static_cast<C>(rng_size) <= static_cast<C>(c.max_size()));
                c.reserve(static_cast<size_type>(rng_size));
                c.assign(I{ranges::begin(rng)}, I{ranges::end(rng)});
                return c;
            }

        public:
            template<typename Rng>
            auto operator()(Rng && rng) const -> CPP_ret(container_t<MetaFn, Rng>)( //
                requires input_range<Rng> &&                                        //
                    convertible_to_cont<Rng, container_t<MetaFn, Rng>>)
            {
                static_assert(!is_infinite<Rng>::value,
                              "Attempt to convert an infinite range to a container.");
                using cont_t = container_t<MetaFn, Rng>;
                using iter_t = range_cpp17_iterator_t<Rng>;
                using use_reserve_t =
                    meta::bool_<(bool)to_container_reserve<cont_t, iter_t, Rng>>;
                return impl<cont_t, iter_t>(static_cast<Rng &&>(rng), use_reserve_t{});
            }
            template<typename Rng>
            auto operator()(Rng && rng) const volatile -> CPP_ret(container_t<MetaFn, Rng>)( //
                requires input_range<Rng> &&                                        //
                    convertible_to_cont_cont<Rng, container_t<MetaFn, Rng>>)
            {
                static_assert(!is_infinite<Rng>::value,
                              "Attempt to convert an infinite range to a container.");
                using cont_t = container_t<MetaFn, Rng>;
                using iter_t = to_container_iterator<Rng, cont_t>;
                using use_reserve_t =
                    meta::bool_<(bool)to_container_reserve<cont_t, iter_t, Rng>>;
                return impl<cont_t, iter_t>(static_cast<Rng &&>(rng), use_reserve_t{});
            }
        };

        template<typename MetaFn, typename Fn>
        using to_container_closure = to_container::closure<MetaFn, Fn>;

        template<typename MetaFn>
        using to_container_fn = to_container_closure<MetaFn, to_container::fn<MetaFn>>;

        template<template<typename...> class ContT>
        struct from_range
        {
#if RANGES_CXX_DEDUCTION_GUIDES >= RANGES_CXX_DEDUCTION_GUIDES_17
            // Attempt to use a deduction guide first...
            template<typename Rng>
            static auto from_rng_(int) -> decltype(ContT(range_cpp17_iterator_t<Rng>{},
                                                         range_cpp17_iterator_t<Rng>{}));
            // No deduction guide. Fallback to instantiating with the
            // iterator's value type.
            template<typename Rng>
            static auto from_rng_(long)
                -> meta::invoke<meta::quote<ContT>, range_value_t<Rng>>;

            template<typename Rng>
            using invoke = decltype(from_range::from_rng_<Rng>(0));
#else
            template<typename Rng>
            using invoke = meta::invoke<meta::quote<ContT>, range_value_t<Rng>>;
#endif
        };
    } // namespace detail
    /// \endcond

    /// \addtogroup group-range
    /// @{

    /// \ingroup group-range
    RANGES_INLINE_VARIABLE(detail::to_container_fn<detail::from_range<std::vector>>,
                           to_vector)

    /// \cond
    namespace _to_
    {
        /// \endcond

        /// \brief For initializing a container of the specified type with the elements of
        /// an Range
        template<template<typename...> class ContT>
        auto to(RANGES_HIDDEN_DETAIL(detail::to_container = {}))
            -> detail::to_container_fn<detail::from_range<ContT>>
        {
            return {};
        }

        /// \overload
        template<template<typename...> class ContT, typename Rng>
        auto to(Rng && rng) -> CPP_ret(ContT<range_value_t<Rng>>)( //
            requires range<Rng> &&
                detail::convertible_to_cont<Rng, ContT<range_value_t<Rng>>>)
        {
            return detail::to_container_fn<detail::from_range<ContT>>{}(
                static_cast<Rng &&>(rng));
        }

        /// \overload
        template<typename Cont>
        auto to(RANGES_HIDDEN_DETAIL(detail::to_container = {}))
            -> detail::to_container_fn<meta::id<Cont>>
        {
            return {};
        }

        /// \overload
        template<typename Cont, typename Rng>
        auto to(Rng && rng) -> CPP_ret(Cont)( //
            requires range<Rng> && detail::convertible_to_cont<Rng, Cont>)
        {
            return detail::to_container_fn<meta::id<Cont>>{}(static_cast<Rng &&>(rng));
        }

        /// \cond
        // Slightly odd initializer_list overloads, undocumented for now.
        template<template<typename...> class ContT, typename T>
        auto to(std::initializer_list<T> il) -> CPP_ret(ContT<T>)( //
            requires detail::convertible_to_cont<std::initializer_list<T>, ContT<T>>)
        {
            return detail::to_container_fn<detail::from_range<ContT>>{}(il);
        }
        template<typename Cont, typename T>
        auto to(std::initializer_list<T> il) -> CPP_ret(Cont)( //
            requires detail::convertible_to_cont<std::initializer_list<T>, Cont>)
        {
            return detail::to_container_fn<meta::id<Cont>>{}(il);
        }
        /// \endcond

        /// \cond
    } // namespace _to_
    using namespace _to_;
    /// \endcond
    /// @}

    ////////////////////////////////////////////////////////////////////////////
    /// \cond
    namespace _to_
    {
        // The old name "ranges::to_" is now deprecated:
        template<template<typename...> class ContT>
        RANGES_DEPRECATED("Please use ranges::to (no underscore) instead.")
        auto to_(detail::to_container = {})
            -> detail::to_container_fn<detail::from_range<ContT>>
        {
            return {};
        }
        template<template<typename...> class ContT, typename Rng>
        RANGES_DEPRECATED("Please use ranges::to (no underscore) instead.")
        auto to_(Rng && rng) -> CPP_ret(ContT<range_value_t<Rng>>)( //
            requires range<Rng> &&
                detail::convertible_to_cont<Rng, ContT<range_value_t<Rng>>>)
        {
            return static_cast<Rng &&>(rng) | ranges::to_<ContT>();
        }
        template<template<typename...> class ContT, typename T>
        RANGES_DEPRECATED("Please use ranges::to (no underscore) instead.")
        auto to_(std::initializer_list<T> il) -> CPP_ret(ContT<T>)( //
            requires detail::convertible_to_cont<std::initializer_list<T>, ContT<T>>)
        {
            return il | ranges::to_<ContT>();
        }
        template<typename Cont>
        RANGES_DEPRECATED("Please use ranges::to (no underscore) instead.")
        auto to_(detail::to_container = {}) -> detail::to_container_fn<meta::id<Cont>>
        {
            return {};
        }
        template<typename Cont, typename Rng>
        RANGES_DEPRECATED("Please use ranges::to (no underscore) instead.")
        auto to_(Rng && rng) -> CPP_ret(Cont)( //
            requires range<Rng> && detail::convertible_to_cont<Rng, Cont>)
        {
            return static_cast<Rng &&>(rng) | ranges::to_<Cont>();
        }
        template<typename Cont, typename T>
        RANGES_DEPRECATED("Please use ranges::to (no underscore) instead.")
        auto to_(std::initializer_list<T> list) -> CPP_ret(Cont)( //
            requires detail::convertible_to_cont<std::initializer_list<T>, Cont>)
        {
            return list | ranges::to_<Cont>();
        }
    } // namespace _to_
    /// \endcond

    template<typename MetaFn, typename Fn>
    RANGES_INLINE_VAR constexpr bool
        is_pipeable_v<detail::to_container_closure<MetaFn, Fn>> = true;
} // namespace ranges

#include <range/v3/detail/reenable_warnings.hpp>

#endif