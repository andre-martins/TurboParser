#ifndef HASHTABLE_H
#define HASHTABLE_H

////////////////////////////////////////////////////////////
// Entry in a hash table. Defined as a single-linked list.
////////////////////////////////////////////////////////////

template<class Value>
class HashTableEntry {
public:
  HashTableEntry(const Value &value) {
    value_ = value;
  }

  Value &value() { return value_; }
  HashTableEntry *next() { return next_; }
  void insert(HashTableEntry *next) { next_ = next; }

private:
  HashTableEntry *next_;
  Value value_;
};

////////////////////////////////////////////////////////////
// Iterator in a hash table.
////////////////////////////////////////////////////////////

template<class Entry, class Value>
class HashTableIterator {
public:
  // Construct an iterator pointing to a particular entry.
  // Note: entry should be NULL iff buckets_.size() == position_.
  HashTableIterator(Entry *entry, const std::vector<Entry*> &buckets,
                    uint32_t position) : buckets_(buckets) {
    entry_ = entry;
    position_ = position;
  }

  // Construct an iterator by passing a bucket position, which may or not be
  // valid. The iterator will point to the head entry in the first valid
  // bucket.
  HashTableIterator(const std::vector<Entry*> &buckets,
                    uint32_t position) : buckets_(buckets) {
    position_ = position;
    MoveToValidBucket();
  }

  // Retrieve pointer to entry.
  Entry *entry() { return entry_; }

  // Iterator operators.
  HashTableIterator<Entry, Value>
    &operator=(const HashTableIterator<Entry, Value> &it) {
    entry_ = it.entry_;
    buckets_ = it.buckets_;
    position_ = it.position_;
    return *this;
  }
  Value &operator*() const { return entry_->value(); }
  Value *operator->() const { return &entry_->value(); }
  void operator++() {
    // Assume entry_ != NULL, otherwise this would be the end iterator.
    // Three possibilities:
    // (1) there is a next element in the current bucket;
    // (2) there is a next element in a following bucket;
    // (3) there is no next element before the end.
    if (entry_->next()) {
      entry_ = entry_->next();
    } else {
      ++position_;
      MoveToValidBucket();
    }
  }

  // Equality testing.
  bool operator==(const HashTableIterator<Entry, Value> &iter) const {
    return entry_ == iter.entry_;
  }
  bool operator!=(const HashTableIterator<Entry, Value> &iter) const {
    return entry_ != iter.entry_;
  }

private:
  // Increment position until a non-null bucket is find.
  void MoveToValidBucket() {
    while (position_ < buckets_.size() && !(buckets_[position_])) ++position_;
    if (position_ != buckets_.size()) {
      entry_ = buckets_[position_];
    } else {
      entry_ = NULL;
    }
  }

  // Pointer to the current entry.
  Entry *entry_;
  // Bucket vector in the hash table.
  const vector<Entry*> &buckets_;
  // Current bucket position.
  uint32_t position_;
};

////////////////////////////////////////////////////////////
// General hash function.
////////////////////////////////////////////////////////////

template<class Key>
struct HashFunction {
  // Default implementation throws an error.
  static uint32_t hash(const Key &k) { CHECK(false); return 0; }
  static bool eq(const Key &k1, const Key &k2) {
    CHECK(false);
    return false;
  }
};

////////////////////////////////////////////////////////////
// Template specializations for various important key types.
////////////////////////////////////////////////////////////

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

// Hash function for 64-bit integers.
template<>
struct HashFunction<uint64_t> {
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
  static bool equal(const uint64_t &k1,
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

#define HANDLE_COLLISIONS

template<class Key, class Value, class Hash = HashFunction<Key> >
class HashTable {
private:
  typedef std::pair<Key, Value> key_value;
  typedef HashTableEntry<key_value> entry;

public:
  typedef HashTableIterator<entry, key_value> iterator;
  typedef HashTableIterator<entry, const key_value> const_iterator;

public:
  HashTable() {
    num_entries_ = 0;
#ifdef HANDLE_COLLISIONS
    buckets_.resize(32);
#else
    buckets_.resize(0x1 << 24);
#endif
    max_num_buckets_ = (0x1 << 28); // Never more than 0x1 << 30.
    max_load_factor_ = 1.0;
  }

  ~HashTable() {
    clear();
    buckets_.clear();
  }

  // Clear the hash table.
  void clear() {
    iterator iter = begin();
    while (iter != end()) {
      entry *e = iter.entry();
      ++iter;
      delete e;
    }
    buckets_.assign(buckets_.size(), NULL);
  }

  // Return number of entries.
  uint32_t size() const { return num_entries_; }
  // Return number of buckets.
  uint32_t bucket_count() const { return buckets_.size(); }

  // Begin/end iterators and constant iterators.
  iterator begin() { return iterator(buckets_, 0); }
  iterator end() {
    return iterator(buckets_, buckets_.size());
  }
  const_iterator begin() const {
    return const_iterator(buckets_, 0);
  }
  const_iterator end() const {
    return const_iterator(buckets_, buckets_.size());
  }

  // Find key and return a constant iterator. Return end() if key
  // does not exist.
  const_iterator find(const Key &key) const {
    uint32_t index = (Hash::hash(key) % bucket_count());
    entry *e = buckets_[index];
#ifdef HANDLE_COLLISIONS
    while (e && !Hash::equal(e->value().first, key)) e = e->next();
#endif
    if (!e) return end();
    return const_iterator(e, buckets_, index);
  }

  // Find key and return an iterator. Return end() if key
  // does not exist.
  iterator find(const Key &key) {
    uint32_t index = (Hash::hash(key) % bucket_count());
    entry *e = buckets_[index];
#ifdef HANDLE_COLLISIONS
    while (e && !Hash::equal(e->value().first, key)) e = e->next();
#endif
    if (!e) return end();
    return iterator(e, buckets_, index);
  }

  // Insert a new element (key-value pair).
  // Note: this does not check for duplicates. It assumes the key
  // does not exist yet.
  std::pair<iterator, bool> insert(const key_value &item) {
#ifdef HANDLE_COLLISIONS
    RehashIfNecessary();
#endif
    const Key &key = item.first;
    uint32_t index = (Hash::hash(key) % bucket_count());
    // Create a new entry and insert it as the bucket head.
    entry *e = new entry(item);
    e->insert(buckets_[index]);
#ifndef HANDLE_COLLISIONS
    CHECK(!buckets_[index]);
#endif
    buckets_[index] = e;
    ++num_entries_;
    iterator it = iterator(e, buckets_, index);
    return std::make_pair(it, true);
  }

  // Get/set maximum load factor.
  float max_load_factor() const { return max_load_factor_; }
  void max_load_factor(float z) { max_load_factor_ = z; }

  // Compute current load factor.
  float load_factor() const {
    return static_cast<float>(size()) /
      static_cast<float>(bucket_count());
  }

  // Rehash the table. Note: this function is not thread-safe.
  // Others cannot read while the table is being rehashed.
  void rehash(uint32_t n) {
    //LOG(INFO) << "Rehashing..." << endl;
    vector<entry*> new_buckets(n);
    iterator it = begin();
    while (it != end()) {
      entry* e = it.entry();
      ++it;
      const Key &key = e->value().first;
      uint32_t index = (Hash::hash(key) % n);
      // Create a new entry and insert it as the bucket head.
      e->insert(new_buckets[index]);
      new_buckets[index] = e;
    }
    buckets_.swap(new_buckets);
  }

  // Print some statistics.
  void print_statistics() {
    LOG(INFO) << "Num buckets = " << bucket_count();
    LOG(INFO) << "Num entries = " << size();
    int empty = 0;
    for (int i = 0; i < bucket_count(); ++i) {
      if (!buckets_[i]) ++empty;
    }
    LOG(INFO) << "Empty buckets = " << empty;
    LOG(INFO) << "Load factor = " << load_factor();
  }

private:
  // Trigger a call to rehash if the load factor is above the limit
  // and the number of buckets is affordable, in which the case the
  // size of the table is doubled.
  void RehashIfNecessary() {
    int num_buckets = bucket_count() * 2;
    if (num_buckets < max_num_buckets_ &&
        load_factor() > max_load_factor_) {
      rehash(num_buckets);
    }
  }

private:
  int num_entries_; // Number of entries.
  std::vector<entry*> buckets_; // Table of bucket entries.
  int max_num_buckets_; // Maximum number of buckets.
  float max_load_factor_; // Maximum load factor.
};

#endif // HASHTABLE_H
