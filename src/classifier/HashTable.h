#ifndef HASHTABLE_H
#define HASHTABLE_H

////////////////////////////////////////////////////////////
// Entry in a hash table. Defined as a single-linked list.
////////////////////////////////////////////////////////////

template<class Key, class Value>
class HashTableEntry {
 public:
  const Key &key() { return key_; }
  const Value &value() { return value_; }
  HashTableEntry *next() { return next_; }

 private:
  HashTableEntry *next_;
  Key key_;
  Value value_;
};



////////////////////////////////////////////////////////////
// Iterator in a hash table.
////////////////////////////////////////////////////////////

template<class Entry>
class HashTableIterator {

 public:
  HashTableIterator(Entry *begin, Entry *end) {
    entry_ = begin;
    bucket_ = begin;
    end_ = end;
    // Find the first valid bucket.
    while (bucket_ < end_ && bucket_->next() == bucket_) ++bucket_;
    entry_ = bucket_;
  }

  HashTableIterator(Entry *entry, Entry *bucket, Entry *end) {
    entry_ = entry;
    bucket_ = bucket;
    end_ = end;
  }

  // Iterator operators.
  HashTableIterator<Entry> &operator=(const HashTableIterator<Entry> &iter) {
    entry_ = iter.entry_;
    bucket_ = iter.bucket_;
    end_ = iter.end_;
    return *this;
  }
  Entry &operator*() { return *entry_; }
  Entry *operator->() { return entry_; }
  void operator++() {
    // Three possibilities:
    // (1) there is a next element in the current bucket;
    // (2) there is a next element in a following bucket;
    // (3) there is no next element before the end.
    if (entry_->next()) {
      entry_ = entry_->next();
    } else {
      ++bucket_;
      while (bucket_ < end_ && bucket_->next() == bucket_) ++bucket_;
      entry_ = bucket_;
    }
  }

  // Equality testing.
  bool operator==(const HashTableIterator<Entry> &iter) const {
    return entry_ == iter.entry_;
  }
  bool operator!=(const HashTableIterator<Entry> &iter) const {
    return entry_ != iter.entry_;
  }

 private:
  // Pointer to the current entry.
  Entry *entry_;
  // Head of current bucket list in the hash table.
  Entry *bucket_;
  // One past the end of the current table.
  Entry *end_;
};


////////////////////////////////////////////////////////////
// Hash function.
////////////////////////////////////////////////////////////

template<class Key>
struct HashFunctionUINT32 {
  // Default implementation throws an error.
  static uint32_t hash(const Key &k) { CHECK(false); return 0; }
  static bool eq(const Key &k1, const Key &k2) {
    CHECK(false);
    return false;
  }
};

// Useful macros for the Jenkins hash.
#define u32rot(x, k) ((x << k) | (x >> (32 - k)))
#define u32mix(a, b, c) {                  \
      a -= c; a ^= u32rot(c, 4);  c += b;  \
      b -= a; b ^= u32rot(a, 6);  a += c;  \
      c -= b; c ^= u32rot(b, 8);  b += a;  \
      a -= c; a ^= u32rot(c, 16); c += b;  \
      b -= a; b ^= u32rot(a, 19); a += c;  \
      c -= b; c ^= u32rot(b, 4);  b += a;  \
    }
#define u32final(a,b,c) {          \
      c ^= b; c -= u32rot(b, 14);  \
      a ^= c; a -= u32rot(c, 11);  \
      b ^= a; b -= u32rot(a, 25);  \
      c ^= b; c -= u32rot(b, 16);  \
      a ^= c; a -= u32rot(c, 4);   \
      b ^= a; b -= u32rot(a, 14);  \
      c ^= b; c -= u32rot(b, 24);  \
    }

////////////////////////////////////////////////////////////
// Template specializations for various important key types.
////////////////////////////////////////////////////////////

// Hash function for 64-bit integers.
template<>
struct HashFunctionUINT32<uint64_t> {
  // The "dreaded" Jenkins hash.
  static uint32_t hash(const uint64_t &k) {
    // Grab high- and low-order bits; add salt.
    uint32_t aa = 0x31415926 + ((uint32_t)(k & 0xffffffff));
    uint32_t bb = 0x27182818 + ((uint32_t)(k >> 32));
    uint32_t cc = 0xdeadbeef; // Nothing to grab.
    u32final(aa, bb, cc); // There are only two words, final mixing only.
    return cc;
  }
  // Simple equality.
  static bool eq(const uint64_t &k1,
                 const uint64_t &k2) {
    return (k1 == k2);
  }
};

#undef u32rot
#undef u32mix
#undef u32final


////////////////////////////////////////////////////////////
// Hash table with templated key, value and hash function.
////////////////////////////////////////////////////////////

template<class Key, class Value, class HashFunction=HashFunctionUINT32<Key> >
class HashTable {
 public:
  HashTable() {};
  ~HashTable() {};

  typedef HashTableIterator<HashTableEntry<Key, Value> > iterator;
  typedef HashTableIterator<const HashTableEntry<Key, Value> > const_iterator;

  int size() const { return num_entries_; }
  int bucket_count() const { return buckets_.size(); }

  iterator begin() { return iterator(buckets_, buckets_ + buckets_.size()); }
  iterator end() {
    return iterator(buckets_ + buckets_.size(), buckets_ + buckets_.size());
  }

  const_iterator cbegin() const {
    return const_iterator(buckets_, buckets_ + buckets_.size());
  }
  const_iterator cend() const {
    return const_iterator(buckets_ + buckets_.size(), buckets_ + buckets_.size());
  }   

  const_iterator find(const Key &key) const {
    uint32_t index = HashFunction::hash(key);

    HashTableEntry<Key, Value> *entry = buckets_ + index;

    if (entry->next() == entry) return cend();
    while (entry && !HashFunction::equal(entry->key(), key)) {
      entry = entry->next();
    }

    if (!entry) return cend();
    return const_iterator(entry, buckets_ + index,
                          buckets_ + buckets_.size());
  }

 /*
  HashTableEntry<Key, Value> *find(const Key &key) const {
    uint32_t index = HashFunction::hash(key);

    HashTableEntry<Key, Value> *entry = buckets_ + index;

    if (entry->next() == entry) return NULL;
    while (entry && !HashFunction::equal(entry->key(), key)) {
      entry = entry->next();
    }
    return entry;
  }
 */
 
 private:
  int num_entries_; // Number of entries.
  vector<HashTableEntry<Key, Value> > buckets_; // Table of bucket entries.
  
};

#endif // HASHTABLE_H
