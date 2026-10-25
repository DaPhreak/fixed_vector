#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <new>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <assert.h>

namespace phreak {

template <class T,size_t Capacity>
class fixed_vector {
public:
    using value_type             = T;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using reference              = T&;
    using const_reference        = const T&;
    using pointer                = T*;
    using const_pointer          = const T*;
    using iterator               = T*;
    using const_iterator         = const T*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr fixed_vector() noexcept {};
    ~fixed_vector() noexcept;
    fixed_vector(const fixed_vector& s);
    fixed_vector(fixed_vector&& s) noexcept;

    template <class... Args>
    explicit fixed_vector(const size_type count,Args&&... args);
    
    template <class U = T,std::enable_if_t<std::is_constructible_v<T,U>>* = nullptr>
    fixed_vector(std::initializer_list<U> init);

    fixed_vector& operator = (const fixed_vector& s);
    fixed_vector& operator = (fixed_vector&& s) noexcept;
    
    template <class U = T,std::enable_if_t<std::is_constructible_v<T,U>>* = nullptr>
    fixed_vector& operator = (std::initializer_list<U> init);

    static constexpr size_type max_size() noexcept { return Capacity; }
    static constexpr size_type capacity() noexcept { return Capacity; }

    constexpr bool empty() const noexcept { return !mSize; }
    constexpr bool full() const noexcept { return mSize == Capacity; }
    constexpr size_type size() const noexcept { return mSize; }

    constexpr pointer       data() noexcept;
    constexpr const_pointer data() const noexcept;

    constexpr iterator       begin () noexcept { return data(); }
    constexpr iterator       end   () noexcept { return data() + mSize; }
    constexpr const_iterator begin () const noexcept { return data(); }
    constexpr const_iterator end   () const noexcept { return data() + mSize; }
    constexpr const_iterator cbegin() const noexcept { return data(); }
    constexpr const_iterator cend  () const noexcept { return data() + mSize; }

    constexpr reverse_iterator       rbegin () noexcept { return reverse_iterator(end ()); }
    constexpr reverse_iterator       rend   () noexcept { return reverse_iterator(data()); }
    constexpr const_reverse_iterator rbegin () const noexcept { return const_reverse_iterator(end ()); }
    constexpr const_reverse_iterator rend   () const noexcept { return const_reverse_iterator(data()); }
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
    void unchecked_resize(const size_type count,Args&&... args);
    template <class... Args>
    void resize(const size_type count,Args&&... args);
    void clear() noexcept;

    template <class... Args>
    void assign(const size_type count,Args&&... args);
    template <class InputIt>
    void assign_n(InputIt first,const size_type count);
    template <class InputIt>
    void assign(InputIt first,const InputIt last);
    template <class U = T,std::enable_if_t<std::is_constructible_v<T,U>>* = nullptr>
    void assign(std::initializer_list<U> init);

    template <class... Args>
    iterator emplace(const const_iterator pos,Args&&... args);
    iterator insert(const const_iterator pos,const T& value);
    iterator insert(const const_iterator pos,T&& value);

    template <class... Args>
    reference unchecked_emplace_back(Args&&... args);
    template <class... Args>
    reference emplace_back(Args&&... args);
    template <class... Args>
    pointer try_emplace_back(Args&&... args);
    
    reference unchecked_push_back(const T& v);
    reference unchecked_push_back(T&& v);
    reference push_back(const T& v);
    reference push_back(T&& v);
    pointer try_push_back(const T& v);
    pointer try_push_back(T&& v);

    void pop_back() noexcept;

    iterator erase(const const_iterator pos) noexcept;
    iterator erase(const const_iterator first,const const_iterator last) noexcept;

    void swap(fixed_vector& s) noexcept;
private:
    size_type mSize{};
    std::byte mStorage[ sizeof(T) * Capacity ];
};

template <class T,size_t Capacity>
fixed_vector<T,Capacity>::~fixed_vector() noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        clear();
    }
}

template <class T,size_t Capacity>
fixed_vector<T,Capacity>::fixed_vector(const fixed_vector& s)
{
    std::uninitialized_copy_n(s.data(),s.size(),data());
    mSize = s.mSize;
}

template <class T,size_t Capacity>
fixed_vector<T,Capacity>::fixed_vector(fixed_vector&& s) noexcept
{
    std::uninitialized_move_n(s.data(),s.size(),data());
    mSize = s.mSize;
    s.clear();
}

template <class T,size_t Capacity>
template <class... Args>
fixed_vector<T,Capacity>::fixed_vector(const size_type count,Args&&... args)
{
    resize(count,std::forward<Args>(args)...);
}

template <class T,size_t Capacity>
template <class U,std::enable_if_t<std::is_constructible_v<T,U>>*>
fixed_vector<T,Capacity>::fixed_vector(std::initializer_list<U> init)
{
    assign_n(init.begin(),init.size());
}

template <class T,size_t Capacity>
fixed_vector<T,Capacity>& fixed_vector<T,Capacity>::operator = (const fixed_vector& s)
{
    if (&s != this) {
        assign_n(s.data(),s.size());
    }
    return *this;
}

template <class T,size_t Capacity>
fixed_vector<T,Capacity>& fixed_vector<T,Capacity>::operator = (fixed_vector&& s) noexcept
{
    assign_n(std::make_move_iterator(s.data()),s.size());
    s.clear();
    return *this;
}

template <class T,size_t Capacity>
template <class U,std::enable_if_t<std::is_constructible_v<T,U>>*>
fixed_vector<T,Capacity>& fixed_vector<T,Capacity>::operator = (std::initializer_list<U> init)
{
    assign_n(init.begin(),init.size());
    return *this;
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::pointer fixed_vector<T,Capacity>::data() noexcept
{
    return std::launder(reinterpret_cast<pointer>(mStorage));
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::const_pointer fixed_vector<T,Capacity>::data() const noexcept
{
    return std::launder(reinterpret_cast<const_pointer>(mStorage));
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::front() noexcept
{
    assert(mSize);
    return *data();
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::front() const noexcept
{
    assert(mSize);
    return *data();
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::back() noexcept
{
    assert(mSize);
    return *(data() + (mSize - 1));
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::back() const noexcept
{
    assert(mSize);
    return *(data() + (mSize - 1));
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::operator [] (const size_type pos) noexcept
{
    assert(pos < mSize);
    return *(data() + pos);
}

template <class T,size_t Capacity>
constexpr typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::operator [] (const size_type pos) const noexcept
{
    assert(pos < mSize);
    return *(data() + pos);
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::at(const size_type pos)
{
    if (pos >= mSize) {
        throw std::out_of_range("subscript out of range");
    }
    return *(data() + pos);
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::const_reference fixed_vector<T,Capacity>::at(const size_type pos) const
{
    if (pos >= mSize) {
        throw std::out_of_range("subscript out of range");
    }
    return *(data() + pos);
}

template <class T,size_t Capacity>
constexpr void fixed_vector<T,Capacity>::reserve(size_type new_cap)
{
    if (new_cap > Capacity) {
        throw std::bad_alloc{};
    }
}

template <class T,size_t Capacity>
template <class... Args>
void fixed_vector<T,Capacity>::unchecked_resize(const size_type count,Args&&... args)
{
    if (count < mSize) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            const auto beg{data()};
            for (auto it{beg+count},e{beg+mSize}; it != e; ++it ) {
                std::destroy_at(it);
            }
        }
        mSize = count;
    } else if (count > mSize) {
        assert(count <= Capacity);
        auto beg{data()};
        while (mSize < count) {
            if (mSize + 1 == count) {
                new(beg + mSize) T(std::forward<Args>(args)...);
            } else {
                new(beg + mSize) T(args...);
            }
            ++mSize;
        }
    }
}

template <class T,size_t Capacity>
template <class... Args>
void fixed_vector<T,Capacity>::resize(const size_type count,Args&&... args)
{
    if (count > Capacity) {
        throw std::bad_alloc{};
    }
    unchecked_resize(count,std::forward<Args>(args)...);
}

template <class T,size_t Capacity>
void fixed_vector<T,Capacity>::clear() noexcept
{
    if (mSize) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (auto it{data()},e{it+mSize}; it != e; ++it ) {
                std::destroy_at(it);
            }
        }
        mSize = {};
    }
}

template <class T,size_t Capacity>
template <class... Args>
void fixed_vector<T,Capacity>::assign(const size_type count,Args&&... args)
{
    const size_t osize{mSize};
    if (count > Capacity) {
        throw std::bad_alloc{};
    } else  if (count < osize) {
        resize(count);
    }
    auto itD{data()};

    for (size_t i{}; i < count; ++i,++itD) {
        if (i + 1 == count) {
            if (i < osize) {
                *itD = T(std::forward<Args>(args)...);
            } else {
                new(itD) T(std::forward<Args>(args)...);
                ++mSize;
            }
        } else {
            if (i < osize) {
                *itD = T(args...);
            } else {
                new(itD) T(args...);
                ++mSize;
            }
        }
    }
}

template <class T,size_t Capacity>
template <class InputIt>
void fixed_vector<T,Capacity>::assign_n(InputIt first,const size_type count)
{
    const size_t osize{mSize};
    if (count >  Capacity) {
        throw std::bad_alloc{};
    } else if (count < osize) {
        resize(count);
    }
    auto itD{data()};

    for (size_t i{}; i < count; ++i,++itD,++first) {
        if (i < osize) {
            if constexpr (std::is_assignable_v<T,decltype(*first)>) {
                *itD = *first;
            } else {
                *itD = T(*first);
            }
        } else {
            new(itD) T(*first);
            ++mSize;
        }
    }
}

template <class T,size_t Capacity>
template <class InputIt>
void fixed_vector<T,Capacity>::assign(InputIt first,const InputIt last)
{
    const auto count{std::distance(first,last)};
    if (count < 0) {
        throw std::out_of_range("out of range");
    }
    assign_n(std::move(first),count);
}

template <class T,size_t Capacity>
template <class U,std::enable_if_t<std::is_constructible_v<T,U>>*>
void fixed_vector<T,Capacity>::assign(std::initializer_list<U> init)
{
    assign_n(init.begin(),init.size());
}

template <class T,size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::emplace(const const_iterator pos,Args&&... args)
{
    if (pos == end()) {
        return &emplace_back(std::forward<Args>(args)...);
    }
    assert(pos >= begin());
    assert(pos <= end());
    emplace_back();
    auto res{const_cast<iterator> (pos)};

    std::rotate(
        res,
        end() - 1,
        end()
   );
    *res = T(std::forward<Args>(args)...);
    return res;
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::insert(const const_iterator pos,const T& value)
{
    return emplace(pos,value);
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::insert(const const_iterator pos,T&& value)
{
    return emplace(pos,std::move(value));
}

template <class T,size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::unchecked_emplace_back(Args&&... args)
{
    assert(!full());
    reference res{*(data() + mSize)};
    new(std::addressof(res)) T(std::forward<Args>(args)...);
    ++mSize;
    return res;
}

template <class T,size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::emplace_back(Args&&... args)
{
    if (full()) {
        throw std::bad_alloc{};
    }
    return unchecked_emplace_back(std::forward<Args>(args)...);
}

template <class T,size_t Capacity>
template <class... Args>
typename fixed_vector<T,Capacity>::pointer fixed_vector<T,Capacity>::try_emplace_back(Args&&... args)
{
    if (full()) {
        return {};
    }
    return &unchecked_emplace_back(std::forward<Args>(args)...);
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::unchecked_push_back(const T& v)
{
     return unchecked_emplace_back(v);
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::unchecked_push_back(T&& v)
{
     return unchecked_emplace_back(std::move(v));
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::push_back(const T& v)
{
     return emplace_back(v);
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::reference fixed_vector<T,Capacity>::push_back(T&& v)
{
     return emplace_back(std::move(v));
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::pointer fixed_vector<T,Capacity>::try_push_back(const T& v)
{
     return try_emplace_back(v);
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::pointer fixed_vector<T,Capacity>::try_push_back(T&& v)
{
     return try_emplace_back(std::move(v));
}

template <class T,size_t Capacity>
void fixed_vector<T,Capacity>::pop_back() noexcept
{
    assert(mSize);
    if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy_at(data() + --mSize);
    } else {
        --mSize;
    }
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::erase(const const_iterator pos) noexcept
{
    return erase(pos,pos + 1) ;
}

template <class T,size_t Capacity>
typename fixed_vector<T,Capacity>::iterator fixed_vector<T,Capacity>::erase(const const_iterator first,const const_iterator last) noexcept
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

template <class T,size_t Capacity>
void fixed_vector<T,Capacity>::swap(fixed_vector& s) noexcept
{
    using std::swap;
    const auto ls{size()};
    const auto rs{s.size()};
    const auto max{std::max(ls,rs)};
    if (!max) {
        return;
    }
    const auto min{std::min(ls,rs)};
    const auto ld{data()};
    const auto rd{s.data()};

    for (size_t i{}; i < max; ++i) {
        if (i < min) {
            swap(*(ld+i),*(rd+i));
        } else if (i < rs) {
            unchecked_emplace_back(std::move(*(rd+i)));
            std::destroy_at(rd+i);
            --s.mSize;
        } else {
            s.unchecked_emplace_back(std::move(*(ld+i)));
            std::destroy_at(ld+i);
            --mSize;
        }
    }
}

template <class T,size_t Capacity>
[[nodiscard]] constexpr bool operator==(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return std::equal(a.begin(),a.end(),b.begin(),b.end());
}

template <class T,size_t Capacity>
[[nodiscard]] constexpr bool operator!=(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return !(a == b);
}

template <class T,size_t Capacity>
[[nodiscard]] constexpr bool operator<(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return std::lexicographical_compare(a.begin(),a.end(),b.begin(),b.end(),std::less<>{});
}

template <class T,size_t Capacity>
[[nodiscard]] constexpr bool operator>(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return b < a;
}

template <class T,size_t Capacity>
[[nodiscard]] constexpr bool operator<=(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return !(b < a);
}

template <class T,size_t Capacity>
[[nodiscard]] constexpr bool operator>=(const fixed_vector<T,Capacity>& a,const fixed_vector<T,Capacity>& b) noexcept
{
    return !(a < b);
}

} // namespace phreak

namespace std {

template <class T,size_t Capacity>
void swap (phreak::fixed_vector<T,Capacity>& lhs,phreak::fixed_vector<T,Capacity>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace std

