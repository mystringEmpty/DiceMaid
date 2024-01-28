#pragma once
#include <map>
#include <unordered_map>
#include <iostream>
//#include <mutex>
//using exlock = std::lock_guard<std::mutex>;

template<class T, bool(T::* U)()const = &T::is_void>
bool is_void(const T& obj) {
    return obj.is_void();
}
template<typename T>
std::enable_if_t<std::is_fundamental_v<T> || std::is_same_v<T, std::string>, bool> is_void(const T& obj) {
    return false;
}
template<typename _Kty, typename _Ty>
using umap = std::unordered_map<_Kty, _Ty>;
template<typename _Kty, typename _Ty>
class fifo_base {
protected:
    size_t inc_end{ 0 };
    umap<_Kty, size_t> index;
    std::map<size_t, _Kty> orders;
    umap<_Kty, _Ty> values;
public:
    using value_type = std::pair<const _Kty, _Ty>;
    fifo_base() = default;
    _Ty& operator[](const _Kty& key) {
        //exlock guard{ ex_key };
        if (!index.count(key)) {
            orders.emplace(inc_end, key);
            index.emplace(key, inc_end++);
        }
        return values[key];
    }
    size_t size()const { return index.size(); }
    bool empty()const { return index.empty(); }
    _Ty& at(const _Kty& key) {
        return values.at(key);
    }
    const _Ty& at(const _Kty& key)const {
        return values.at(key);
    }
    value_type& in(size_t idx) {
        return *values.find(orders.at(idx));
    }
    const value_type& in(size_t idx)const {
        return *values.find(orders.at(idx));
    }
    size_t next(size_t idx)const {
        auto fwd{ orders.upper_bound(idx) };
        return fwd == orders.end() ? inc_end : fwd->first;
    }
    size_t next(size_t idx, size_t n)const {
        auto fwd{ orders.lower_bound(idx)};
        std::advance(fwd, n);
        return fwd == orders.end() ? inc_end : fwd->first;
    }
    size_t prev(size_t idx, size_t n = 1)const {
        auto it{ orders.upper_bound(idx) };
        std::advance(it, -n);
        return it->first;
    }
};
template<typename _Kty, typename _Ty>
class fifo_const_iter {
    using const_iterator = fifo_const_iter<_Kty, _Ty>;
    using _Base = fifo_base<_Kty, _Ty>;
    const _Base* const con;
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair<const _Kty, _Ty>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;
    size_t idx;
    explicit fifo_const_iter(const _Base* ref, size_t i = 0) :con(ref), idx(i) {}
    const value_type& operator*()const {
        return con->in(idx);
    }
    const value_type* operator->() {
        return &con->in(idx);
    }
    bool operator==(const const_iterator& other)const {
        return con == other.con &&
            (idx == other.idx || con->empty());
    }
    bool operator!=(const const_iterator& other)const {
        return con != other.con ||
            (idx != other.idx && !con->empty());
    }
    const_iterator& operator++() {
        idx = con->next(idx);
        return *this;
    }
    const_iterator& operator--() {
        idx = con->prev(idx);
        return *this;
    }
};
template<typename _Kty, typename _Ty>
class fifo_iter {
    using _Base = fifo_base<_Kty, _Ty>;
    _Base* con;
public:
    size_t idx;
    using iterator = fifo_iter<_Kty, _Ty>;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair<const _Kty, _Ty>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
    explicit fifo_iter(_Base* ref = nullptr, size_t i = 0) :con(ref), idx(i) {}
    reference operator*() {
        return con->in(idx);
    }
    pointer operator->() {
        return &(const_cast<reference>(con->in(idx)));
    }
    pointer operator->()const {
        return &(const_cast<reference>(con->in(idx)));
    }
    bool operator==(const iterator& other)const {
        return con == other.con &&
            (idx == other.idx || con->empty());
    }
    bool operator!=(const iterator& other)const {
        return con != other.con ||
            (idx != other.idx && !con->empty());
    }
    iterator& operator=(const iterator& other) {
        con = other.con;
        idx = other.idx;
        return *this;
    }
    iterator& operator++() {
        idx = con->next(idx);
        return *this;
    }
    iterator& operator+=(size_t n) {
        idx = con->next(idx, n);
        return *this;
    }
    iterator& operator--() {
        idx = con->prev(idx);
        return *this;
    }
    iterator& operator-=(size_t n) {
        idx = con->prev(idx, n);
        return *this;
    }
}; 
//template<typename _Kty>
//struct std::iterator_traits<fifo_iter<_Kty>> {
//    using iterator_category = std::forward_iterator_tag;
//    using value_type = int;
//};
template<typename _Kty>
class fifo_compare {
    umap<_Kty, size_t>* idx;
public:
    fifo_compare(umap<_Kty, size_t>* p):idx(p){}
    bool operator()(const _Kty& left, const _Kty& right) const{
        if (auto l{ idx->find(left) };l != idx->end()) {
            if (auto r{ idx->find(right) }; r != idx->end()) {
                return l->second < r->second;
            }
            else return true;
        }
        else return false;
    }
};
template<typename _Kty, typename _Ty, typename Compare = void,
    typename _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>
class fifo_map :public fifo_base<_Kty, _Ty> {
    //std::mutex ex_key;
    using _Base = fifo_base<_Kty, _Ty>;
    using const_iterator = fifo_const_iter<_Kty, _Ty>;
public:
    using key_type = _Kty;
    using mapped_type = _Ty;
    using value_type = std::pair<const _Kty, _Ty>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = fifo_iter<_Kty, _Ty>;
    using key_compare = fifo_compare<_Kty>;
    key_compare cmp;
    fifo_map(): cmp(&index) {};
    fifo_map(std::initializer_list<std::pair<_Kty, _Ty>> list): cmp(&index) {
        for (auto& x : list) {
            insert(x);
        }
    }
    bool operator==(const fifo_map& other)const {
        return values == other.values;
    }
    size_t max_size()const noexcept { return values.max_size(); }
    bool count(const _Kty& key)const { return index.count(key); }
    iterator begin() {
        return iterator(this, orders.empty() ? 0 : orders.begin()->first);
    }
    const_iterator begin()const {
        return const_iterator(this, orders.empty() ? 0 : orders.begin()->first);
    }
    const_iterator cbegin()const { return begin(); }
    iterator end() {
        return iterator(this, inc_end);
    }
    const_iterator end()const {
        return const_iterator(this, inc_end);
    }
    const_iterator cend()const { return end(); }
    iterator find(const _Kty& key) { 
        if (auto it{ index.find(key) };it!= index.end()) {
            return iterator(this, it->second);
        }
        return end();
    }
    const_iterator find(const _Kty& key)const {
        if (auto it{ index.find(key) }; it != index.end()) {
            return const_iterator(this, it->second);
        }
        return end();
    }
    std::pair<iterator, bool> insert(const _Kty& k, const _Ty& v) {
        return insert({k,v});
    }
    std::pair<iterator, bool> insert(const value_type& p) {
        if (auto it{ index.find(p.first) }; it != index.end()) {
            values.at(it->first) = p.second;
            return { iterator(this, it->second), false };
        }
        values.emplace(p);
        orders.emplace(inc_end, p.first);
        index.emplace(p.first, inc_end);
        return { iterator(this, ++inc_end), true };
    }
    std::pair<iterator, bool> emplace(const _Kty& key, const _Ty& val) {
        if (auto it{ index.find(key) }; it != index.end()) {
            return { iterator(this, it->second), false };
        }
        values.emplace(key, val );
        orders.emplace(inc_end, key);
        index.emplace(key, inc_end);
        return { iterator(this, ++inc_end) , true};
    }
    iterator erase(const _Kty& key) {
        //exlock guard{ ex_key };
        if (auto it(index.find(key)); it != index.end()) {
            auto idx{ it->second };
            auto next{ ++orders.find(idx) };
            index.erase(it);
            orders.erase(idx);
            values.erase(key);
            if (next != orders.end())return iterator(this, next->first);
        }
        return end();
    }
    iterator erase(const iterator& iter) {
        //exlock guard{ ex_key };
        if (auto it(orders.find(iter.idx)); it != orders.end()) {
            index.erase(it->second);
            values.erase(it->second);
            orders.erase(it);
            return iterator(this, next(iter.idx));
        }
        return end();
    }
    void swap(fifo_map& other) noexcept {
        std::swap(inc_end, other.inc_end);
        values.swap(other.values);
        index.swap(other.index);
        orders.swap(other.orders);
    }
    void clear() noexcept {
        if (inc_end) {
            inc_end = 0;
            values.clear();
            index.clear();
            orders.clear();
        }
    }
};