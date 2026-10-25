#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <new>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <cstddef>
#include <cassert>

namespace phreak {

template <class T,std::size_t Capacity>
class fixed_vector {
    static_assert( Capacity > 0, "Needs capacity!" );
public:
    using value_type             = T;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = T&;
    using const_reference        = const T&;
    using pointer                = T*;
    using const_pointer          = const T*;
    using iterator               = T*;
    using const_iterator         = const T*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr fixed_vector() noexcept {}
    ~fixed_vector() noexcept;
    fixed_vector(const fixed_vector& s) noexcept(std::is_nothrow_copy_constructible_v<T>);
    fixed_vector(fixed_vector&& s) noexcept(std::is_nothrow_move_constructible_v<T>);

    template <class... Args>
    explicit fixed_vector(const size_type count,Args&&... args);

    template <class InputIt,class = typename std::iterator_traits<InputIt>::iterator_category>
    fixed_vector(InputIt first,const InputIt last);
    
    fixed_vector(std::initializer_list<T> init);
    template <class U,class = std::enable_if_t<std::is_convertible_v<const U&,T>>>
    fixed_vector(std::initializer_list<U> init);

    fixed_vector& operator = (const fixed_vector& s) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>);
    fixed_vector& operator = (fixed_vector&& s) noexcept(std::is_nothrow_move_assignable_v<value_type> && std::is_nothrow_move_constructible_v<T>);
    
    fixed_vector& operator = (std::initializer_list<T> init);
    template <class U,class = std::enable_if_t<std::is_convertible_v<const U&,T>>>
    fixed_vector& operator = (std::initializer_list<U> init);

    static constexpr size_type max_size() noexcept { return Capacity; }
    static constexpr size_type capacity() noexcept { return Capacity; }

    constexpr bool empty() const noexcept { return !mSize; }
    constexpr bool full() const noexcept { return mSize == Capacity; }
    constexpr size_type size() const noexcept { return mSize; }

    constexpr pointer       data() noexcept { return mData; }
    constexpr const_pointer data() const noexcept { return mData; }

    constexpr iterator       begin () noexcept { return mData; }
    constexpr iterator       end   () noexcept { return mData + mSize; }
    constexpr const_iterator begin () const noexcept { return mData; }
    constexpr const_iterator end   () const noexcept { return mData + mSize; }
    constexpr const_iterator cbegin() const noexcept { return mData; }
    constexpr const_iterator cend  () const noexcept { return mData + mSize; }

    constexpr reverse_iterator       rbegin () noexcept { return reverse_iterator(end  ()); }
    constexpr reverse_iterator       rend   () noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rbegin () const noexcept { return const_reverse_iterator(end  ()); }
    constexpr const_reverse_iterator rend   () const noexcept { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_reverse_iterator crend  () const noexcept { return rend  (); }

    constexpr reference       front() noexcept;
    constexpr const_reference front() const noexcept;
    constexpr reference       back() noexcept;
    constexpr const_reference back() const noexcept;
    
    constexpr reference       operator [] (const size_type pos) noexcept;
    constexpr const_reference operator [] (const size_type pos) const noexcept;

    reference       at(const size_type pos);
    const_reference at(const size_type pos) const;
    
    static constexpr void reserve(size_type new_cap);
    static constexpr void shrink_to_fit() noexcept {}

    template <class... Args>
    void resize(const size_type count,Args&&... args);
    void clear() noexcept;

    template <class... Args>
    void assign(const size_type count,Args&&... args);
    template <class InputIt>
    void assign_n(InputIt first,const size_type count);
    template <class InputIt,class = typename std::iterator_traits<InputIt>::iterator_category>
    void assign(InputIt first,const InputIt last);
    void assign(std::initializer_list<T> init);
    template <class U,class = std::enable_if_t<std::is_convertible_v<const U&,T>>>
    void assign(std::initializer_list<U> init);

    template <class... Args>
    iterator emplace(const const_iterator pos,Args&&... args);
    iterator insert(const const_iterator pos,const T& value);
    iterator insert(const const_iterator pos,T&& value);

    template <class... Args>
    reference unchecked_emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<T,Args...>);
    template <class... Args>
    reference emplace_back(Args&&... args);
    template <class... Args>
    pointer try_emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<T,Args...>);
    
    reference unchecked_push_back(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T>);
    reference unchecked_push_back(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>);
    reference push_back(const T& v);
    reference push_back(T&& v);
    pointer try_push_back(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T>);
    pointer try_push_back(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>);

    void pop_back() noexcept;

    iterator erase(const const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>);
    iterator erase(const const_iterator first,const const_iterator last) noexcept(std::is_nothrow_move_assignable_v<T>);

    void swap(fixed_vector& s) noexcept(std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>);
private:
    void shrink(const size_type count);

    friend void swap(fixed_vector& a, fixed_vector& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }

    size_type mSize{};
    union { T mData[Capacity]; };
};

template <class T,std::size_t Capacity>
fixed_vector<T,Capacity>::~fixed_vector() noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        clear();
    }
}

template <class T,std::size_t Capacity>
fixed_vector<T,Capacity>::fixed_vector(const fixed_vector& s) noexcept(std::is_nothrow_copy_constructible_v<T>)
: fixed_vector{}
{
    std::uninitialized_copy_n(s.begin(),s.size(),begin());
    mSize = s.mSize;
}

template <class T,std::size_t Capacity>
fixed_vector<T,Capacity>::fixed_vector(fixed_vector&& s) noexcept(std::is_nothrow_move_constructible_v<T>)
: fixed_vector{}
{
    std::uninitialized_move_n(s.begin(),s.size(),begin());
    mSize = s.mSize;
    s.clear();
}

template <class T,std::size_t Capacity>
template <class... Args>
fixed_vector<T,Capacity>::fixed_vector(const size_type count,Args&&... args)
: fixed_vector{}
{
    resize(count,std::forward<Args>(args)...);
}

template <class T,std::size_t Capacity>
template <class InputIt,class>
fixed_vector<T,Capacity>::fixed_vector(InputIt first,const InputIt last)
: fixed_vector{}
{
    assign(std::move(first),std::move(last));
}

template <class T,std::size_t Capacity>
fixed_vector<T,Capacity>::fixed_vector(std::initializer_list<T> init)
: fixed_vector{}
{
    assign_n(init.begin(),init.size());
}

template <class T,std::size_t Capacity>
template <class U,class>
fixed_vector<T,Capacity>::fixed_vector(std::initializer_list<U> init)
: fixed_vector{}
{
    assign_n(init.begin(),init.size());
}

template <class T,std::size_t Capacity>
fixed_vector<T,Capacity>& fixed_vector<T,Capacity>::operator = (const fixed_vector& s) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>)
{
    if (&s != this) {
        assign_n(s.begin(),s.size());
    }
    return *this;
}

template <class T,std::size_t Capacity>
fixed_vector<T,Capacity>& fixed_vector<T,Capacity>::operator = (fixed_vector&& s) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>)
{
    assign_n(std::make_move_iterator(s.begin()),s.size());
    s.clear();
    return *this;
}

template <class T,std::size_t Capacity>
fixed_vector<T,Capacity>& fixed_vector<T,Capacity>::operator = (std::initializer_list<T> init)
{
    assign_n(init.begin(),init.size());
    return *this;
}

template <class T,std::size_t Capacity>
template <class U,class>
fixed_vector<T,Capacity>& fixed_vector<T,Capacity>::operator = (std::initializer_list<U> init)
{
    assign_n(init.begin(),init.size());
    return *this;
}

template <class T,std::size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::front() noexcept
{
    assert(mSize);
    return *begin();
}

template <class T,std::size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::front() const noexcept
{
    assert(mSize);
    return *begin();
}

template <class T,std::size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::back() noexcept
{
    assert(mSize);
    return *(end()-1);
}

template <class T,std::size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::back() const noexcept
{
    assert(mSize);
    return *(end()-1);
}

template <class T,std::size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::operator [] (const size_type pos) noexcept
{
    assert(pos < mSize);
    return *(begin() + pos);
}

template <class T,std::size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::operator [] (const size_type pos) const noexcept
{
    assert(pos < mSize);
    return *(begin() + pos);
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::at(const size_type pos)
{
    if (pos >= mSize) {
        throw std::out_of_range("subscript out of range");
    }
    return *(begin() + pos);
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::at(const size_type pos) const
{
    if (pos >= mSize) {
        throw std::out_of_range("subscript out of range");
    }
    return *(begin() + pos);
}

template <class T,std::size_t Capacity>
constexpr void fixed_vector<T,Capacity>::reserve(size_type new_cap)
{
    if (new_cap > Capacity) {
        throw std::bad_alloc{};
    }
}

template <class T,std::size_t Capacity>
template <class... Args>
void fixed_vector<T,Capacity>::resize(const size_type count,Args&&... args)
{
    if (count > Capacity) {
        throw std::bad_alloc{};
    }
    if (count > mSize) {
        assert(count <= Capacity);
        const auto beg{begin()};
        while (mSize < count) {
            if (mSize + 1 == count) {
                ::new (static_cast<void*>(beg + mSize)) T(std::forward<Args>(args)...);
            } else {
                ::new (static_cast<void*>(beg + mSize)) T(args...);
            }
            ++mSize;
        }
    } else {
        shrink(count);
    }
}

template <class T,std::size_t Capacity>
void fixed_vector<T,Capacity>::clear() noexcept
{
    if (mSize) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (auto it{begin()},e{it+mSize}; it != e; ++it ) {
                std::destroy_at(it);
            }
        }
        mSize = {};
    }
}

template <class T,std::size_t Capacity>
template <class... Args>
void fixed_vector<T,Capacity>::assign(const size_type count,Args&&... args)
{
    if (count > Capacity) {
        throw std::bad_alloc{};
    }
    auto it{begin()};
    const size_type osize{mSize};

    for (size_type i{}; i < count; ++i,++it) {
        if (i + 1 == count) {
            if (i < osize) {
                *it = T(std::forward<Args>(args)...);
            } else {
                unchecked_emplace_back(std::forward<Args>(args)...);
            }
        } else {
            if (i < osize) {
                *it = T(args...);
            } else {
                unchecked_emplace_back(args...);
            }
        }
    }
    shrink(count);
}

template <class T,std::size_t Capacity>
template <class InputIt>
void fixed_vector<T,Capacity>::assign_n(InputIt first,const size_type count)
{
    if (count >  Capacity) {
        throw std::bad_alloc{};
    }
    auto it{begin()};
    const size_type osize{mSize};

    for (size_type i{}; i < count; ++i,++it,++first) {
        if (i < osize) {
            if constexpr (std::is_assignable_v<T&,decltype(*first)>) {
                *it = *first;
            } else {
                *it = T(*first);
            }
        } else {
            unchecked_emplace_back(*first);
        }
    }
    shrink(count);
}

template <class T,std::size_t Capacity>
template <class InputIt,class>
void fixed_vector<T,Capacity>::assign(InputIt first,const InputIt last)
{
    using category = typename std::iterator_traits<InputIt>::iterator_category;

    if constexpr (std::is_base_of_v<std::forward_iterator_tag,category>) {
        const auto count{std::distance(first,last)};
        assert(count >= 0);
        assign_n(std::move(first),static_cast<size_type>(count));
    } else {
        auto it{begin()};
        size_type ns{};

        for (;first != last;++ns,++first,++it) {
            if (ns < mSize) {
                if constexpr (std::is_assignable_v<T&,decltype(*first)>) {
                    *it = *first;
                } else {
                    *it = T(*first);
                }
            } else {
                emplace_back(*first);
            }
        }
        shrink(ns);
    }
}

template <class T,std::size_t Capacity>
void fixed_vector<T,Capacity>::assign(std::initializer_list<T> init)
{
    assign_n(init.begin(),init.size());
}

template <class T,std::size_t Capacity>
template <class U,class>
void fixed_vector<T,Capacity>::assign(std::initializer_list<U> init)
{
    assign_n(init.begin(),init.size());
}

template <class T,std::size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::emplace(const const_iterator pos,Args&&... args)
{
    const auto beg{begin()};
    const auto id{static_cast<size_type>(pos - beg)};

    assert(id <= mSize);
    emplace_back(std::forward<Args>(args)...);
    const auto e{end()};
    std::rotate(beg + id,e - 1,e);
    return beg + id;
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::insert(const const_iterator pos,const T& value)
{
    return emplace(pos,value);
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::insert(const const_iterator pos,T&& value)
{
    return emplace(pos,std::move(value));
}

template <class T,std::size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::unchecked_emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<T,Args...>)
{
    assert(!full());
    const auto res{begin() + mSize};
    ::new (static_cast<void*>(res)) T(std::forward<Args>(args)...);
    ++mSize;
    return *res;
}

template <class T,std::size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::emplace_back(Args&&... args)
{
    if (full()) {
        throw std::bad_alloc{};
    }
    return unchecked_emplace_back(std::forward<Args>(args)...);
}

template <class T,std::size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::pointer fixed_vector<T,Capacity>::try_emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<T,Args...>)
{
    if (full()) {
        return {};
    }
    return std::addressof(unchecked_emplace_back(std::forward<Args>(args)...));
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::unchecked_push_back(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T>)
{
     return unchecked_emplace_back(v);
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::unchecked_push_back(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>)
{
     return unchecked_emplace_back(std::move(v));
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::push_back(const T& v)
{
     return emplace_back(v);
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::push_back(T&& v)
{
     return emplace_back(std::move(v));
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::pointer fixed_vector<T,Capacity>::try_push_back(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T>)
{
     return try_emplace_back(v);
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::pointer fixed_vector<T,Capacity>::try_push_back(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>)
{
     return try_emplace_back(std::move(v));
}

template <class T,std::size_t Capacity>
void fixed_vector<T,Capacity>::pop_back() noexcept
{
    assert(mSize);
    if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy_at(begin() + --mSize);
    } else {
        --mSize;
    }
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::erase(const const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>)
{
    return erase(pos,pos + 1) ;
}

template <class T,std::size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::erase(const const_iterator first,const const_iterator last) noexcept(std::is_nothrow_move_assignable_v<T>)
{
    assert(first <= last);
    assert(first >= begin());
    assert(last <= end());
    const iterator itFirst{const_cast<iterator> (first)};
    if (first == last) {
        return itFirst;
    }
    std::move(
        const_cast<iterator> (last),
        end(),
        itFirst);
    if constexpr (!std::is_trivially_destructible_v<T>) {
        for (const_iterator it{first },its{end() - 1}; it != last; ++it,--its) {
            std::destroy_at(its);
            --mSize;
        }
    } else {
        mSize -= last - first;
    }
    return itFirst;
}

template <class T,std::size_t Capacity>
void fixed_vector<T,Capacity>::swap(fixed_vector& s) noexcept(std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>)
{
    if (this == &s) {
        return;
    }
    fixed_vector* longer{size() >= s.size() ? this : &s};
    fixed_vector* shorter{longer == this ? &s : this};
    const auto ls{longer->mSize};
    const auto ss{shorter->mSize};
    
    std::swap_ranges(begin(),begin() + ss,s.begin());
    if (ss == ls) {
        return;
    }
    std::uninitialized_move(
        longer->begin() + ss,
        longer->begin() + ls,
        shorter->begin() + ss
    );
    shorter->mSize = ls;
    std::destroy(longer->begin() + ss,longer->begin() + ls);
    longer->mSize = ss;
}

template <class T,std::size_t Capacity>
void fixed_vector<T,Capacity>::shrink(const size_type count)
{
    assert(count <= mSize);
    if (count < mSize) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            const auto beg{begin()};
            for (auto it{beg+count},e{beg+mSize}; it != e; ++it ) {
                std::destroy_at(it);
            }
        }
        mSize = count;
    }
}

template <class T,std::size_t Capacity>
[[nodiscard]] constexpr bool operator==(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return std::equal(a.begin(),a.end(),b.begin(),b.end());
}

template <class T,std::size_t Capacity>
[[nodiscard]] constexpr bool operator!=(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return !(a == b);
}

template <class T,std::size_t Capacity>
[[nodiscard]] constexpr bool operator<(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return std::lexicographical_compare(a.begin(),a.end(),b.begin(),b.end(),std::less<>{});
}

template <class T,std::size_t Capacity>
[[nodiscard]] constexpr bool operator>(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return b < a;
}

template <class T,std::size_t Capacity>
[[nodiscard]] constexpr bool operator<=(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return !(b < a);
}

template <class T,std::size_t Capacity>
[[nodiscard]] constexpr bool operator>=(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return !(a < b);
}

} // namespace phreak

namespace std {

template <class T,std::size_t Capacity>
void swap (phreak::fixed_vector<T,Capacity>& a,phreak::fixed_vector<T,Capacity>& b) noexcept(noexcept(a.swap(b)))
{
    a.swap(b);
}

} // namespace std

