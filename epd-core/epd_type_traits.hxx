// epd_type_traits.hxx: Type traits and utilities

#pragma once

#include <type_traits>
#include <limits>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include <iterator>

namespace epd
{
	// void_t: void type alias
	template< typename... >
	using void_t = void;
	//

	// npos_t: type that indicates an invalid position
	struct npos_t
		: ::std::integral_constant<
				::std::size_t,
				::std::numeric_limits<::std::size_t>::max()
		>
	{
	};
	//

	template< typename T >
	struct is_npos
		: ::std::integral_constant<bool, T::value == npos_t::value>
	{

	};

	template< typename T >
	constexpr bool is_npos_v = is_npos<T>::value;


	namespace internal
	{
		// Fallback case
		template<	typename D,
					typename Void,
					template< typename... > class Check,
					typename... Args 
				>
		struct detect
		{
			using value_t = std::false_type;
			using type = D;
		};


		// Check succeeded
		template<	typename D,
					template< typename... > class Check,
					typename... Args 
				>
		struct detect
			< D, void_t< Check<Args...> >, Check, Args... >
		{
			using value_t = std::true_type;
			using type = Check<Args...>;
		};
	}

	// Type representing a missing type.
	struct nonesuch
	{
		nonesuch() = delete;
		~nonesuch() = delete;
		nonesuch(nonesuch const&) = delete;
		void operator=(nonesuch const&) = delete;
	};


	template<	template< typename... > class Check,
				typename... Args
			>
	using is_detected = typename internal::detect< nonesuch, void, Check, Args... >::value_t;


	template<	template< typename... > class Check,
				typename... Args
			>
	using detected_t = typename internal::detect< nonesuch, void, Check, Args... >::type;


	template<	template< typename... > class Check,
				typename... Args
			>
	constexpr bool is_detected_v = is_detected<Check, Args...>::value;


	template<	typename Default,
				template< typename... > class Check,
				typename... Args
			>
	using detected_or = internal::detect< Default, void, Check, Args... >;


	template<	typename Default,
				template< typename... > class Check,
				typename... Args
			>
	using detected_or_t = typename internal::detect< Default, void, Check, Args... >::type;


	template<	typename Expected,
				template< typename... > class Check,
				typename... Args
			>
	using is_detected_exact = std::is_same< Expected, detected_t< Check, Args... > >;


	template<	typename Expected,
				template< typename... > class Check,
				typename... Args
			>
	constexpr bool is_detected_exact_v = std::is_same< Expected, detected_t< Check, Args... > >::value;


	template<	typename To,
				template< typename... > class Check,
				typename... Args
			>
	using is_detected_convertible = std::is_convertible< detected_t< Check, Args... >, To >;


	template<	typename To,
				template< typename... > class Check,
				typename... Args
			>
	constexpr bool is_detected_convertible_v = std::is_convertible< detected_t< Check, Args... >, To >::value;



	namespace internal
	{
		template< typename... >
		struct disjunction_impl;

		// Empty
		template< >
		struct disjunction_impl < >
		{
			using type = std::false_type;
		};

		// One element
		template< typename T >
		struct disjunction_impl <T>
		{
			using type = T;
		};

		// N elements
		template< typename T, typename... Ts >
		struct disjunction_impl <T, Ts...>
		{
			using type = std::conditional_t< T::value, T, typename disjunction_impl<Ts...>::type >;
		};
	}

	// disjunction: Fold traits with disjunction.
	// Empty => false_type
	// Any element in Ts true => that element
	// No element in Ts true => last element
	template< typename... Ts >
	using disjunction = typename internal::disjunction_impl<Ts...>::type;
	//


	namespace internal
	{
		template< typename... >
		struct conjunction_impl;

		// 0 elements
		template< >
		struct conjunction_impl <>
		{
			using type = std::true_type;
		};

		// 1 element
		template< typename T >
		struct conjunction_impl <T>
		{
			using type = T;
		};

		// N elements
		template< typename T, typename... Ts >
		struct conjunction_impl <T, Ts...>
		{
			using type = std::conditional_t< T::value, typename conjunction_impl<Ts...>::type, T >;
		};
	}


	// conjunction: Fold traits with conjunction.
	// Empty => true_type
	// Any element in Ts false => that element
	// No element in Ts false => last element
	template< typename... Ts >
	using conjunction = typename internal::conjunction_impl<Ts...>::type;
	//


	// negation: T Negated
	template< typename T >
	using negation = std::integral_constant<bool, !T::value>;
	//
	

	// contains: Checks if Ts contains T at least one time
	template< typename T, typename... Ts >
	using contains = disjunction< std::is_same<T, Ts>... >;
	
	template< typename T, typename... Ts >
	constexpr bool contains_v = contains<T, Ts...>::value;
	//


	// uncvref_t: Remove reference and cv qual. This is the correct alternative to decay_t.
	template< typename T > 
	using uncvref_t = ::std::remove_cv_t<::std::remove_reference_t<T>>;
	
	
	// first_t: First type of non empty template parameter pack
	template< typename... >
	struct first;

	template< typename T, typename... Ts >
	struct first <T, Ts...>
	{
		using type = T;
	};

	template< typename... Ts >
	using first_t = typename first<Ts...>::type;
	//

	namespace internal
	{
		template< typename T, typename... Ts, ::std::size_t... Ns >
		constexpr ::std::size_t index_of_impl(::std::index_sequence<Ns...>)
		{
			return ::std::min<::std::size_t>(
				::std::initializer_list<::std::size_t>{
						(::std::is_same<T, Ts>::value ? Ns : npos_t::value)...
					}
				);
		}
	}

	// index_of: Find index of type in type pack. Is max if not found.
	template< typename T, typename... Ts >
	constexpr ::std::size_t index_of_v =
		internal::index_of_impl<T, Ts...>(::std::index_sequence_for<Ts...>{});

	template< typename T, typename... Ts >
	using index_of = ::std::integral_constant<::std::size_t, index_of_v<T, Ts...>>;
	//

	namespace internal
	{
		template< typename, typename... >
		struct count_of_impl
			: std::integral_constant<::std::size_t, 0>
		{
		};

		template< typename U, typename T, typename... Ts >
		struct count_of_impl <U, T, Ts...>
			: std::integral_constant< ::std::size_t, ((::std::is_same<U, T>::value) ? 1 : 0) + count_of_impl<U, Ts...>::value >
		{};
	}

	template< typename T, typename... Ts >
	using count_of = internal::count_of_impl<T, Ts...>;

	template< typename T, typename... Ts >
	constexpr ::std::size_t count_of_v = count_of<T, Ts...>::value;

	namespace internal
	{
		template< std::size_t, typename... >
		struct at_impl;

		template< ::std::size_t N, typename T, typename... Ts >
		struct at_impl< N, T, Ts...>
		{
			using type = typename at_impl< N-1, Ts...>::type;
		};

		template< typename T, typename... Ts >
		struct at_impl< 0, T, Ts... >
		{
			using type = T;
		};
	}



	template< ::std::size_t N, typename... Ts >
	struct at
	{
		static_assert( N < sizeof...(Ts),
			"ut::at: Type index out of range" );

		using type = typename internal::at_impl<N, Ts...>::type;
	};

	template< ::std::size_t N, typename... Ts >
	using at_t = typename at<N, Ts...>::type;

	namespace internal
	{
		template< typename From, typename To >
		using is_static_castable_impl = decltype(static_cast<To>(std::declval<From>()));
	}

	// is_static_castable: Determine whether From is static_cast'able to To.
	template< typename From, typename To >
	using is_static_castable = is_detected< internal::is_static_castable_impl, From, To >;
	
	
	template< typename From, typename To >
	constexpr bool is_static_castable_v = is_static_castable<From, To>::value;
	
	
	namespace internal
	{
		template< typename From, typename To >
		using is_no_narrow_convertible_impl = decltype( To{ ::std::declval<From>() } );
	}
	
	// is_no_narrow_convertible: Determine whether From is convertible to To without narrowing.
	template< typename From, typename To >
	using is_no_narrow_convertible = is_detected< internal::is_no_narrow_convertible_impl, From, To >;
	
	
	template< typename From, typename To >
	constexpr bool is_no_narrow_convertible_v = is_no_narrow_convertible<From, To>::value;
	
	
	namespace internal
	{
		template< typename Tfunc, typename... Targs >
		using is_callable_test = decltype(
					::std::declval<Tfunc>()(
							::std::declval<Targs>()...
						)
				);


		template<	typename Tfunc,
					typename Tret,
					typename... Targs
		>
		using is_callable_impl = ::std::integral_constant<
			bool,
			epd::is_detected_exact<
				Tret, is_callable_test, Tfunc, Targs...
			>::value
		>;
	}
	
	template<	typename,
				typename Tret = void
	>
	struct is_callable;


	template<	typename Tfunc,
				typename... Targs,
				typename Tret
	>
	struct is_callable< Tfunc(Targs...), Tret >
		: internal::is_callable_impl<Tfunc, Tret, Targs...>
	{
	};
	
	template<	typename T,
				typename Tret = void
	>
	constexpr bool is_callable_v = is_callable<T, Tret>::value;
	
	
	template< typename T >
	using is_scoped_enum = ::std::integral_constant<
		bool,
		::std::is_enum<T>::value && !::std::is_convertible<T, int>::value
	>;
	
	template< typename T >
	constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

	
	namespace internal
	{
		template< typename T >
		using is_iterator_impl = typename ::std::iterator_traits<T>::value_type;
	}

	// TODO this is not always working. See stackoverflow.
	template< typename T >
	struct is_iterator
		: is_detected<internal::is_iterator_impl, T>
	{
	};

	template< typename T >
	constexpr bool is_iterator_v = is_iterator<T>::value;

    template< typename T >
	using always_false = std::integral_constant<bool, sizeof(T) == 0>;
	
	template< typename T >
	constexpr bool always_false_v = always_false<T>::value;
}
