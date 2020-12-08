#pragma once
#include <utility>
#include <iterator>
#include <cassert>
#include <type_traits>

template<class T, class S, size_t N>
struct Array{
    using size_type = S;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    using iterator = T*;
    using const_iterator = const T*;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type capcity()  noexcept { return N;}

    inline size_type size()const noexcept {return size_;}


    inline reference at(size_type i){ assert(i<size_); return static_cast<reference>(data_[i]); }
    inline const_reference at(size_type i)const { assert(i<size_); return static_cast<const_reference>(data_[i]); }

    inline reference operator[](size_type i){ return at(i); }
    inline const_reference operator[](size_type i)const { return at(i); }

    inline reference front() { return at(0); }
    inline const_reference front() const { return at(0); }

    inline reference back() {return at(size_-1);}
    inline const_reference back() const { return at(size_-1);} 

    template<class... Args>
    inline T* emplace_back(Args&&... args){
        assert(size_<capcity());
        auto p = new(&this->at(size_)) value_type(std::forward<Args>(args)...);
        if( p )
            ++size_;
        return p;
    }

    inline void push_back(const_reference v){
        assert(size_<capcity());
        this->at(size_++)=v;
    }

    inline void pop_back(){
        assert(size_>0);
        --size_;
    };

    inline iterator begin() noexcept{ return static_cast<iterator>(&data_[0]); }
    inline const_iterator begin() const noexcept{ return static_cast<const_iterator>(&data_[0]); }

    inline iterator end() noexcept{ return begin()+size_; }
    inline const_iterator end() const noexcept{ return begin()+size_; }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data_[N];
    size_type size_;

};