#pragma once

#include <functional>

#include <vector>
#include <algorithm>

namespace jstd
{
    template<class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key> >
    class flat_set
    {
    public:
        using key_type = Key;
        using key_compare = Compare;
        using value_type = Key;
        using value_compare = Compare;
        using allocator_type = Allocator;

        using sequence_type = std::vector<value_type, allocator_type>;
        using pointer = typename sequence_type::pointer;
        using const_pointer = typename sequence_type::const_pointer;
        using reference = typename sequence_type::reference;
        using const_reference = typename sequence_type::const_reference;
        using difference_type = typename sequence_type::difference_type;
        using size_type = typename sequence_type::size_type;
        using iterator = typename sequence_type::iterator;
        using const_iterator = typename sequence_type::const_iterator;
        using reverse_iterator = typename sequence_type::reverse_iterator;
        using const_reverse_iterator = typename sequence_type::const_reverse_iterator;


        // ctor, dtor

        explicit flat_set( const Compare& comp = Compare(), const Allocator& alloc = Allocator() )
            : comp_(comp), data_(alloc) {}

        explicit flat_set(const Allocator& alloc)
            : flat_set(Compare(), alloc) {}

        template<class InputIt>
        flat_set(InputIt first, InputIt last,
                 const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : flat_set(comp, alloc)
        {
            insert(first, last);
        }

        template<class InputIt>
        flat_set(InputIt first, InputIt last, const Allocator& alloc)
            : flat_set(first, last, Compare(), alloc) {}

        flat_set(const flat_set& other) = default;

        flat_set(const flat_set& other, const Allocator& alloc)
            : comp_(other.comp_), data_(other.data_, alloc) {}

        flat_set(flat_set&& other) = default;

        flat_set(flat_set&& other, const Allocator& alloc )
            : comp_(std::move(other.comp_)), data_(std::move(other), alloc) {}

        flat_set(std::initializer_list<value_type> init,
                 const Compare& comp = Compare(), const Allocator& alloc = Allocator() )
            : flat_set(init.begin(), init.end(), alloc) {}

        flat_set(std::initializer_list<value_type> init, const Allocator& alloc)
            : flat_set(init, Compare(), alloc) {}

        ~flat_set() = default;

        // operator =

        flat_set& operator=(const flat_set& other) = default;
        flat_set& operator=(flat_set&& other) = default;
        flat_set& operator=(std::initializer_list<value_type> ilist)
        {
            data_ = sequence_type(data_.get_allocator());
            insert(ilist);

            return *this;
        }

        allocator_type get_allocator() const { return data_.get_allocator(); }


        // Iterators

        iterator begin() noexcept { return data_.begin(); }
        const_iterator begin() const noexcept { return data_.begin(); }
        const_iterator cbegin() const noexcept { return data_.cbegin(); }

        iterator end() noexcept { return data_.end(); }
        const_iterator end() const noexcept { return data_.end(); }
        const_iterator cend() const noexcept { return data_.cend(); }

        reverse_iterator rbegin() noexcept { return data_.rbegin(); }
        const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
        const_reverse_iterator crbegin() const noexcept { return data_.crbegin(); }

        reverse_iterator rend() noexcept { return data_.rend(); }
        const_reverse_iterator rend() const noexcept { return data_.rend(); }
        const_reverse_iterator crend() const noexcept { return data_.crend(); }


        // Capacity

        bool empty() const noexcept { return data_.empty(); }
        size_type size() const noexcept { return data_.size(); }
        size_type max_size() const noexcept { return data_.max_size(); }
        size_type capacity() const noexcept { return data_.capacity(); }

        void reserve(size_type cnt) { data_.reserve(cnt); }
        void shrink_to_fit() { data_.shrink_to_fit(); }

        // Modifiers

        void clear() noexcept { data_.clear(); }

        std::pair<iterator,bool> insert(const value_type& value)
        {
            auto it = lower_bound(value);

            if (it == cend() || comp_(value, *it))
                return { data_.insert(it, value), true };

            return { it, false };
        };

        std::pair<iterator,bool> insert(value_type&& value)
        {
            auto it = lower_bound(value);

            if (it == cend() || comp_(value, *it))
                return { data_.insert(it, std::forward<value_type>(value)), true };

            return { it, false };
        };

        template<class InputIt>
        void insert(InputIt first, InputIt last)
        {
            for (; first != last; ++first)
                insert(*first);
        }

        void insert(std::initializer_list<value_type> ilist) { insert(ilist.begin(), ilist.end()); }


        iterator erase(const_iterator pos) { return data_.erase(pos); }
        iterator erase(iterator pos) { return data_.erase(pos); }
        iterator erase(const_iterator first, const_iterator last) { return data_.erase(first, last); }

        size_type erase(const key_type& key)
        {
            const auto it = internal_find(key);
            if (it != cend())
            {
                data_.erase(it);
                return 1;
            }

            return 0;
        }

        template<class K>
        size_type erase(const K& x)
        {
            const auto it = internal_find(x);
            if (it != cend())
            {
                data_.erase(it);
                return 1;
            }

            return 0;
        }


        // Lookup

        size_type count(const Key& key) const
        {
            if (internal_find(key) != cend())
                return 1;
            return 0;
        }

        template<class K>
        size_type count(const K& x) const
        {
            if (internal_find(x) != cend())
                return 1;
            return 0;
        }

        iterator find(const Key& key) { return internal_find(key); }
        const_iterator find(const Key& key) const { return internal_find(key); }
        template<class K>
        iterator find(const K& x) { return internal_find(x); }
        template<class K>
        const_iterator find(const K& x) const { return internal_find(x); }

        std::pair<iterator, iterator> equal_range(const Key& key)
        {
            return { internal_lower_bound(key), internal_upper_bound(key) };
        };

        std::pair<const_iterator, const_iterator> equal_range(const Key& key) const
        {
            return { internal_lower_bound(key), internal_upper_bound(key) };
        };

        template<class K>
        std::pair<iterator, iterator> equal_range(const K& x)
        {
            return { internal_lower_bound(x), internal_upper_bound(x) };
        };

        template<class K>
        std::pair<const_iterator, const_iterator> equal_range(const K& x) const
        {
            return { internal_lower_bound(x), internal_upper_bound(x) };
        };

        iterator lower_bound( const Key& key ) { return internal_lower_bound(key); }
        const_iterator lower_bound( const Key& key ) const { return internal_lower_bound(key); }
        template<class K>
        iterator lower_bound(const K& x) { return internal_lower_bound(x); }
        template<class K>
        const_iterator lower_bound(const K& x) const { return internal_lower_bound(x); }

        iterator upper_bound(const Key& key) { return internal_upper_bound(key); }
        const_iterator upper_bound(const Key& key) const { return internal_upper_bound(key); }
        template<class K>
        iterator upper_bound(const K& x) { return internal_upper_bound(x); }
        template<class K>
        const_iterator upper_bound(const K& x) const { return internal_upper_bound(x); }

        key_compare key_comp() const { return comp_; }
        value_compare value_comp() const { return comp_; }

    private:
        template<class K> inline
        iterator internal_find(const K& x)
        {
            auto&& it = internal_lower_bound(x);

            if (it == data_.cend() || comp_(x, *it))
                return data_.end();

            return it;
        }

        template<class K> inline
        const_iterator internal_find(const K& x) const
        {
            auto&& it = internal_lower_bound(x);

            if (it == data_.cend() || comp_(x, *it))
                return data_.cend();

            return it;
        }

        template<class K> inline
        iterator internal_lower_bound(const K& x) { return std::lower_bound(begin(), end(), x, comp_); }

        template<class K> inline
        const_iterator internal_lower_bound(const K& x) const { return std::lower_bound(cbegin(), cend(), x, comp_); }

        template<class K> inline
        iterator internal_upper_bound(const K& x) { return std::upper_bound(begin(), end(), x, comp_); }

        template<class K> inline
        const_iterator internal_upper_bound(const K& x) const { return std::upper_bound(cbegin(), cend(), x, comp_); }

    private:
        key_compare comp_;
        sequence_type data_;

    };
}
