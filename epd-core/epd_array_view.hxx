// epd_array_view.hxx: Lightweight, non-owning reference to sequential data

#pragma once

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include <utility>

#include "epd_type_traits.hxx"

namespace epd
{
	namespace internal
	{
		template< typename C >
		using is_container_check = decltype(std::declval<C>().size(), std::declval<C>().data());

		template< typename C >
		using is_container = is_detected<is_container_check, C>;
	}


	// Iterator for array_view
	template< typename TView, bool IsConst = true >
	class array_view_iter
	{
		public:
			template< typename TView2, bool IsConst2 >
			friend class array_view_iter;

		private:
			using TThis = array_view_iter<TView, IsConst>;

		public:
			using iterator_category	= std::random_access_iterator_tag;
			using difference_type	= typename TView::difference_type;
			using size_type			= typename TView::size_type;
			using bare_pointer		= typename TView::bare_pointer;

			// TView::value_type might not have a const qualification, e.g. when
			// this iterator is created by cbegin.
			using value_type		= std::conditional_t<
										IsConst,
										std::add_const_t< typename TView::value_type >,
										typename TView::value_type
									  >;

			using pointer			= std::add_pointer_t<value_type>;
			using reference			= std::add_lvalue_reference_t<value_type>;

		public:
			constexpr array_view_iter(bare_pointer p_data, size_type p_stride)
				: m_Data(p_data), m_Stride(p_stride)
			{
			}

			constexpr array_view_iter(const TThis&) = default;			
			constexpr TThis& operator=(const TThis&) = default;

			// Only allow conversion non-const -> const
			template< 	bool IsConst2 = IsConst,
						typename = ::std::enable_if_t<IsConst2>
			>
			constexpr array_view_iter(const array_view_iter<TView, false>& p_iter)
				: m_Data(p_iter.data()), m_Stride(p_iter.stride())
			{
				
			}

			~array_view_iter() = default;

		public:
			constexpr auto swap(TThis& p_rhs)
				-> void
			{
				using std::swap;

				swap(m_Data, p_rhs.m_Data);
				swap(m_Stride, p_rhs.m_Stride);
			}

			constexpr auto operator++()
				-> TThis&
			{
				m_Data += (m_Stride);
				return *this;
			}

			constexpr auto operator*() const
				-> reference
			{
				return *reinterpret_cast<value_type*>(m_Data);
			}

			constexpr auto operator++(int)
				-> TThis
			{
				TThis t_tmp{ *this };
				++(*this);
				return t_tmp;
			}

			constexpr auto operator->() const
				-> pointer
			{
				return reinterpret_cast<value_type*>(m_Data);
			}

			constexpr auto operator--()
				-> TThis&
			{
				m_Data -= (m_Stride);
				return *this;
			}

			constexpr auto operator--(int)
				-> TThis
			{
				TThis t_tmp{ *this };
				--(*this);
				return t_tmp;
			}

			constexpr auto operator+=(size_type p_sz)
				-> TThis&
			{
				m_Data += (m_Stride*p_sz);
				return *this;
			}
			
			constexpr auto operator+(size_type p_sz) const
				-> TThis
			{
				TThis t_tmp{ *this };
				t_tmp += p_sz;
				return t_tmp;
			}

			constexpr auto operator-(size_type p_sz) const
				-> TThis
			{
				TThis t_tmp{ *this };
				t_tmp -= p_sz;
				return t_tmp;
			}

			constexpr auto operator-=(size_type p_sz)
				-> TThis&
			{
				m_Data -= (m_Stride*p_sz);
				return *this;
			}

			constexpr auto operator[](size_type p_sz) const
				-> reference
			{
				return *((*this) + p_sz);
			}

			constexpr auto data() const
				-> bare_pointer
			{
				return m_Data;
			}
			
			constexpr auto stride() const
				-> size_type
			{
				return m_Stride;
			}

			/*template< bool B >
			constexpr auto swap(array_view_iter<TView, B>& p_rhs)
				-> void
			{
				using std::swap;

				swap(m_Data, p_rhs.m_Data);
				swap(m_Stride, p_rhs.m_Stride);
			}*/

		private:
			bare_pointer m_Data;
			size_type m_Stride;
	};

	template< typename TView, bool B >
	constexpr auto operator+(typename TView::size_type p_sz, const array_view_iter<TView, B>& p_iter)
		-> array_view_iter<TView, B>
	{
		return p_iter + p_sz;
	}

	template< typename TView, bool B1, bool B2 >
	constexpr auto operator-(const array_view_iter<TView, B1>& p_lhs, const array_view_iter<TView, B2>& p_rhs)
		-> typename array_view_iter<TView>::difference_type
	{
		return (p_lhs.data() - p_rhs.data()) / p_lhs.stride();
	}

	template< typename TView, bool B1, bool B2 >
	constexpr auto operator==(const array_view_iter<TView, B1>& p_lhs, const array_view_iter<TView, B2>& p_rhs)
		-> bool
	{
		return p_lhs.data() == p_rhs.data();
	}

	template< typename TView, bool B1, bool B2 >
	constexpr auto operator!=(const array_view_iter<TView, B1>& p_lhs, const array_view_iter<TView, B2>& p_rhs)
		-> bool
	{
		return !(p_lhs == p_rhs);
	}

	template< typename TView, bool B1, bool B2 >
	constexpr auto operator<(const array_view_iter<TView, B1>& p_lhs, const array_view_iter<TView, B2>& p_rhs)
		-> bool
	{
		return p_lhs.data() < p_rhs.data();
	}

	template< typename TView, bool B1, bool B2 >
	constexpr auto operator>=(const array_view_iter<TView, B1>& p_lhs, const array_view_iter<TView, B2>& p_rhs)
		-> bool
	{
		return !(p_lhs < p_rhs);
	}

	template< typename TView, bool B1, bool B2 >
	constexpr auto operator>(const array_view_iter<TView, B1>& p_lhs, const array_view_iter<TView, B2>& p_rhs)
		-> bool
	{
		return p_rhs < p_lhs;
	}

	template< typename TView, bool B1, bool B2 >
	constexpr auto operator<=(const array_view_iter<TView, B1>& p_lhs, const array_view_iter<TView, B2>& p_rhs)
		-> bool
	{
		return !(p_lhs > p_rhs);
	}



	template< typename T >
	class array_view
	{
		private:
			using TThis = array_view<T>;

			// If T has the form "const U" only immutable access is granted
			constexpr static bool IsConst = std::is_const<T>::value;

		public:
			using size_type			= std::int32_t;
			using difference_type	= std::int32_t;
			using value_type		= T;
			using const_pointer		= std::add_const_t<std::add_pointer_t<T>>;
			using const_reference	= std::add_const_t<std::add_lvalue_reference_t<T>>;
			using pointer			= std::add_pointer_t<T>;
			using reference			= std::add_lvalue_reference_t<T>;
			using bare_pointer		= std::conditional_t<IsConst, const char*, char*>;


			using const_iterator			= array_view_iter<TThis, true>;
			using iterator					= array_view_iter<TThis, IsConst>;
			using const_reverse_iterator	= std::reverse_iterator<const_iterator>;
			using reverse_iterator			= std::reverse_iterator<iterator>;
			
		private:
			// Private use only
			constexpr array_view(bare_pointer p_ptr, size_type p_len, size_type p_stride)
				: m_Data(p_ptr), m_Length(p_len), m_Stride(p_stride)
			{
			}

		public:
			// Construct empty view
			constexpr array_view()
				: m_Stride(sizeof(T)), m_Length(0), m_Data(nullptr)
			{			
			}
		
			// Construct from static array
			template<	typename U,
						size_t N,
						typename = std::enable_if_t<std::is_convertible<U*, T*>::value>
			>
			constexpr array_view(U(&p_array)[N])
				: m_Stride(sizeof(U)), m_Length(N), m_Data((bare_pointer)p_array)
			{
			}

			// Construct from pointer and length
			template<	typename U,
						typename = std::enable_if_t<std::is_convertible<U*, T*>::value>
			>
			constexpr array_view(U* p_ptr, size_type p_len)
				: m_Stride(sizeof(U)), m_Length(p_len), m_Data((bare_pointer)p_ptr)
			{
			}

			// Construct from iterator pair
			template< typename It >
			constexpr array_view(const It& p_begin, const It& p_end)
				:	m_Stride(sizeof(typename ::std::iterator_traits<It>::value_type)),
					m_Length(::std::distance(p_begin, p_end)),
					m_Data(nullptr)
			{
				if(m_Length > 0)
					m_Data = (bare_pointer)::std::addressof(*p_begin);
			}

			// Construct from container with contigous memory layout.
			// C needs to support size() and data().
			template<	typename C,
						typename = std::enable_if_t<internal::is_container<C>::value>,
						typename = std::enable_if_t<std::is_convertible<decltype(std::declval<C>().data()), T*>::value>
			>
			constexpr array_view(C& p_Container)
				:	m_Stride(sizeof(typename C::value_type)),
					m_Length(p_Container.size()),
					m_Data(nullptr)		
			{
				if(m_Length > 0)
					m_Data = (bare_pointer)p_Container.data();
			}
			
			// Construct from initializer_list
			template<	typename U,
						typename = std::enable_if_t<std::is_convertible<const U*, T*>::value>
			>
			constexpr array_view(::std::initializer_list<U> p_lst)
				: array_view(p_lst.begin(), p_lst.end())
			{
			}		
			
			// Copy is trivial
			array_view(const TThis&) = default;
            TThis& operator=(const TThis& other) = default;

			// Move is trivial
			array_view(TThis&&) = default;
            TThis& operator=(TThis&& other) = default;

		public:
			constexpr auto operator[](size_type p_sz) const
				-> reference
			{
				return *reinterpret_cast<T*>(m_Data + (p_sz*m_Stride));
			}

			auto at(size_type p_sz) const
				-> reference
			{
				if (p_sz < m_Length)
					return (*this)[p_sz];
				else throw std::out_of_range("array_view access out of bounds");
			}
		
		public:
			constexpr auto length() const
				-> size_type
			{
				return m_Length;
			}

			constexpr auto size() const
				-> size_type
			{
				return length();
			}

			constexpr auto data() const
				-> pointer
			{
				return reinterpret_cast<pointer>(m_Data);
			}

		public:
			constexpr auto first(size_t p_sz) const
				-> TThis
			{
				return TThis(m_Data, std::min(length(), p_sz), m_Stride);
			}

			constexpr auto last(size_t p_sz) const
				-> TThis
			{
				size_type t_count = std::min(p_sz, length());

				return TThis(m_Data + (m_Stride*(length()-t_count)), t_count, m_Stride);
			}

			// Creates new range [begin, end)
			constexpr auto span(size_t p_begin, size_t p_end) const
				-> TThis
			{
				if (p_end < p_begin)
					throw std::out_of_range("array_view: p_begin > p_end");
				else if (p_end == p_begin)
					return TThis((T*)nullptr, 0);
				else
				{
					size_type t_len = (p_end - p_begin);

					return TThis(m_Data + (m_Stride*p_begin), t_len, m_Stride);
				}
			}

		public:
			// mutable iterator methods
			constexpr auto begin() const
				-> iterator
			{
				return iterator(m_Data, m_Stride);
			}

			constexpr auto end() const
				-> iterator
			{
				return iterator(m_Data+(m_Length*m_Stride), m_Stride);
			}

			constexpr auto rbegin() const
				-> reverse_iterator
			{
				return reverse_iterator(end());
			}

			constexpr auto rend() const
				-> reverse_iterator
			{
				return reverse_iterator(begin());
			}
			//

			// immutable iterator methods
			constexpr auto cbegin() const
				-> const_iterator
			{
				return begin();
			}

			constexpr auto cend() const
				-> const_iterator
			{
				return end();
			}

			constexpr auto crbegin() const
				-> const_reverse_iterator
			{
				return rbegin();
			}

			constexpr auto crend() const
				-> const_reverse_iterator
			{
				return rend();
			}
			//

		private:
			// Size of objects. This is needed to not slice elements when refering
			// to their base class.
			size_type m_Stride;
			size_type m_Length;
			bare_pointer m_Data;
	};


	template<	typename T,
				typename U,
				typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>
	>
	auto operator== (const array_view<T>& p_lhs, const array_view<U>& p_rhs)
		-> bool
	{
		return std::equal(	p_lhs.cbegin(), p_lhs.cend(),
							p_rhs.cbegin(), p_rhs.cend()
		);
	}


	template<	typename T,
				typename U,
				typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>
	>
	auto operator!= (const array_view<T>& p_lhs, const array_view<U>& p_rhs)
		-> bool
	{
		return !(p_lhs == p_rhs);
	}


	template<	typename T,
				typename U,
				typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>
	>
	auto operator< (const array_view<T>& p_lhs, const array_view<U>& p_rhs)
		-> bool
	{
		return std::lexicographical_compare(	p_lhs.cbegin(), p_lhs.cend(),
												p_rhs.cbegin(), p_rhs.cend()
		);
	}

	template<	typename T,
				typename U,
				typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>
	>
	auto operator>= (const array_view<T>& p_lhs, const array_view<U>& p_rhs)
		-> bool
	{
		return !(p_lhs < p_rhs);
	}


	template<	typename T,
				typename U,
				typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>
	>
	auto operator> (const array_view<T>& p_lhs, const array_view<U>& p_rhs)
		-> bool
	{
		return p_rhs < p_lhs;
	}


	template<	typename T,
				typename U,
				typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>
	>
	auto operator<= (const array_view<T>& p_lhs, const array_view<U>& p_rhs)
		-> bool
	{
		return !(p_lhs > p_rhs);
	}

	
	// Convenience aliases
	using byte_view = array_view<std::uint8_t>;
	using const_byte_view = array_view<const std::uint8_t>;
	
	
	template< typename C >
	auto make_view(C& p_container)
		-> epd::array_view< typename C::value_type >
	{
		return { p_container.begin(), p_container.end() };
	}

	template< typename C >
	auto make_const_view(const C& p_container)
		-> array_view< const typename C::value_type >
	{
		return { p_container.cbegin(), p_container.cend() };
	}
	
	template< typename C >
	auto make_const_view(C&&)
		-> array_view< const typename C::value_type >
	{
		static_assert(always_false_v<C>,
			"Cannot create view of rvalue container!");
		
		return { };
	}
	
	template< typename C >
	auto make_view(const C& p_container)
		-> array_view< const typename C::value_type >
	{
		return make_const_view(p_container);
	}
	
	template< typename C >
	auto make_view(C&&)
		-> array_view< const typename C::value_type >
	{
		static_assert(always_false_v<C>,
			"Cannot create view of rvalue container!");
		
		return { };
	}
	
	template< typename T >
	auto make_view(const T* p_ptr, ::std::size_t p_len)
		-> array_view<const T>
	{
		return { p_ptr, p_len };
	}
	
	template< typename T >
	auto make_view(T* p_ptr, ::std::size_t p_len)
		-> array_view<T>
	{
		return { p_ptr, p_len };
	}
	
	template< typename T >
	auto make_const_view(const T* p_ptr, ::std::size_t p_len)
		-> array_view<const T>
	{
		return { p_ptr, p_len };
	}
	
	template< typename T, ::std::size_t N >
	auto make_view(const T(&p_array)[N])
		-> array_view<const T>
	{
		return { p_array };
	}
	
	template< typename T, ::std::size_t N >
	auto make_view(T(&p_array)[N])
		-> array_view<T>
	{
		return { p_array };
	}
	
	template< typename T, ::std::size_t N >
	auto make_view(T(&&p_array)[N])
		-> array_view<T>
	{
		static_assert(always_false_v<T>,
			"Cannot create view of rvalue array!");
		
		return { };
	}
	
	template< typename It >
	auto make_view(It p_begin, It p_end)
		-> array_view<	::std::remove_reference_t<
							typename It::reference
						>
					>
	{
		return { p_begin, p_end };
	}
	
	template< typename It >
	auto make_const_view(It p_begin, It p_end)
		-> array_view<	::std::add_const_t<
							::std::remove_reference_t<
								typename It::reference
							>
						> 
					>
	{
		return { p_begin, p_end };
	}
}