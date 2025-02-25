#ifndef _LRU_CACHE_HPP_
#define _LRU_CACHE_HPP_

#include <cstddef>
#include <unordered_map>
#include <list>
#include <optional>
#include <utility>

namespace waybuilder {

namespace __detail {

template<typename KeyType, typename ValueType, size_t CacheSize,
    typename CacheContType = std::unordered_map<KeyType, std::pair<ValueType, typename std::list<KeyType>::iterator>>>
class LruCache {
 private:
    using ListType = std::list<KeyType>;
    using iterator = CacheContType::iterator;
    using const_iterator = CacheContType::const_iterator;
 public:
    size_t size() const { return size_; };

    bool contains(const KeyType& key) const { return cont_.contains(key); };

    bool empty() const { return size_; };

    void clear() { list_.clear(); cont_.clear(); };

 public:
    bool insert(const KeyType& key, const ValueType& value);
    void erase(const KeyType& key);
    std::optional<ValueType> get(const KeyType& key);

 public:
    iterator begin() { return cont_.begin(); };
    const_iterator cbegin() const { return cont_.cbegin(); };
    iterator end() { return cont_.end(); };
    const_iterator cend() const { return cont_.cend(); };

 private:
    void Kick();

 private:
    ListType list_;
    CacheContType cont_;
    size_t size_ = 0;
};


template<typename KeyType, typename ValueType, size_t CacheSize, typename CacheContType>
bool LruCache<KeyType, ValueType, CacheSize, CacheContType>::insert(const KeyType& key, const ValueType& value) {
    if (cont_.find(key) != cont_.end()) {
        return false;
    }

    if (size_ >= CacheSize) {
        Kick();
    }

    list_.push_front(key);
    cont_.insert({key, std::make_pair(value, list_.begin())});
    
    ++size_;
    return true;
}


template<typename KeyType, typename ValueType, size_t CacheSize, typename CacheContType>
void LruCache<KeyType, ValueType, CacheSize, CacheContType>::Kick() {
    auto kicked_itr = --list_.end();

    cont_.erase(*kicked_itr);
    list_.pop_back();
    --size_;
}


template<typename KeyType, typename ValueType, size_t CacheSize, typename CacheContType>
void LruCache<KeyType, ValueType, CacheSize, CacheContType>::erase(const KeyType& key) {
    if (cont_.contains(key)) {
        auto& list_itr = cont_.at(key).second;
        list_.erase(list_itr);
        cont_.erase(key);
        --size_;
    }
}


template<typename KeyType, typename ValueType, size_t CacheSize, typename CacheContType>
std::optional<ValueType> LruCache<KeyType, ValueType, CacheSize, CacheContType>::get(const KeyType& key) {
    auto cont_itr = cont_.find(key);

    if (cont_itr == cont_.end()) {
        return {};
    }

    auto value_itr = cont_itr->second.second;

    if(value_itr != list_.begin()) {
        list_.erase(value_itr);
        list_.push_front(key);

        const ValueType& value = cont_itr->second.first;

        cont_.insert({key, std::make_pair(value, list_.begin())});

        return value;
    } else {
        return cont_itr->second.first;
    }
}


} // namespace __detail

} // namespace waybuilder

#endif // _LRU_CACHE_HPP_