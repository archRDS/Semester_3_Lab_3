package structures

import "sort"

type hashEntry struct {
	key, value string
	used       bool
}
type HashTable struct {
	cuckoo        bool
	chains        [][]hashEntry
	first, second []hashEntry
	count         int
	seed          uint64
}

func NewHashTable(cuckoo bool) *HashTable {
	h := &HashTable{cuckoo: cuckoo}
	if cuckoo {
		h.first = make([]hashEntry, 11)
		h.second = make([]hashEntry, 11)
	} else {
		h.chains = make([][]hashEntry, 11)
	}
	return h
}
func (h *HashTable) Size() int { return h.count }

func hashCode(s string, salt uint64) uint64 {
	h := uint64(1469598103934665603) ^ salt
	for i := 0; i < len(s); i++ {
		h ^= uint64(s[i])
		h *= 1099511628211
	}
	h ^= h >> 33
	h *= 0xff51afd7ed558ccd
	h ^= h >> 33
	return h
}
func (h *HashTable) bucket(key string, table int) int {
	capacity := len(h.chains)
	if h.cuckoo {
		capacity = len(h.first)
	}
	salt := h.seed
	if table == 1 {
		salt += 0x9e3779b97f4a7c15
	}
	return int(hashCode(key, salt) % uint64(capacity))
}
func (h *HashTable) slot(key string, table int) *hashEntry {
	if table == 0 {
		return &h.first[h.bucket(key, 0)]
	}
	return &h.second[h.bucket(key, 1)]
}
func (h *HashTable) Capacity() int {
	if h.cuckoo {
		return len(h.first) * 2
	}
	return len(h.chains)
}
func (h *HashTable) Get(key string) (string, bool) {
	if h.cuckoo {
		for i := 0; i < 2; i++ {
			p := h.slot(key, i)
			if p.used && p.key == key {
				return p.value, true
			}
		}
	} else {
		for _, e := range h.chains[h.bucket(key, 0)] {
			if e.key == key {
				return e.value, true
			}
		}
	}
	return "", false
}
func (h *HashTable) entries() []hashEntry {
	out := make([]hashEntry, 0, h.count)
	if h.cuckoo {
		for _, p := range h.first {
			if p.used {
				out = append(out, p)
			}
		}
		for _, p := range h.second {
			if p.used {
				out = append(out, p)
			}
		}
	} else {
		for _, b := range h.chains {
			out = append(out, b...)
		}
	}
	return out
}
func (h *HashTable) insertRaw(e hashEntry) *hashEntry {
	table := 0
	for i := 0; i < 4*len(h.first); i++ {
		p := h.slot(e.key, table)
		if !p.used {
			*p = e
			h.count++
			return nil
		}
		e, *p = *p, e
		table ^= 1
	}
	return &e
}
func (h *HashTable) rebuild(entries []hashEntry, capacity int) {
	for {
		h.first = make([]hashEntry, capacity)
		h.second = make([]hashEntry, capacity)
		h.count = 0
		h.seed++
		failed := false
		for i, e := range entries {
			if displaced := h.insertRaw(e); displaced != nil {
				retry := append([]hashEntry{}, entries[i+1:]...)
				retry = append(retry, *displaced)
				retry = append(retry, h.entries()...)
				entries = retry
				failed = true
				break
			}
		}
		if !failed {
			return
		}
		capacity = capacity*2 + 1
	}
}
func (h *HashTable) Put(key, value string) bool {
	if h.cuckoo {
		for i := 0; i < 2; i++ {
			p := h.slot(key, i)
			if p.used && p.key == key {
				p.value = value
				return false
			}
		}
		if (h.count+1)*2 > h.Capacity() {
			all := append(h.entries(), hashEntry{key, value, true})
			h.rebuild(all, len(h.first)*2+1)
			return true
		}
		if displaced := h.insertRaw(hashEntry{key, value, true}); displaced != nil {
			all := append(h.entries(), *displaced)
			h.rebuild(all, len(h.first)*2+1)
		}
		return true
	}
	index := h.bucket(key, 0)
	for i := range h.chains[index] {
		if h.chains[index][i].key == key {
			h.chains[index][i].value = value
			return false
		}
	}
	if (h.count+1)*10 > len(h.chains)*9 {
		old := h.entries()
		h.chains = make([][]hashEntry, len(h.chains)*2+1)
		h.count = 0
		for _, e := range old {
			h.Put(e.key, e.value)
		}
	}
	index = h.bucket(key, 0)
	h.chains[index] = append(h.chains[index], hashEntry{key, value, true})
	h.count++
	return true
}
func (h *HashTable) Delete(key string) bool {
	if h.cuckoo {
		for i := 0; i < 2; i++ {
			p := h.slot(key, i)
			if p.used && p.key == key {
				*p = hashEntry{}
				h.count--
				return true
			}
		}
	} else {
		index := h.bucket(key, 0)
		b := h.chains[index]
		for i, e := range b {
			if e.key == key {
				copy(b[i:], b[i+1:])
				b[len(b)-1] = hashEntry{}
				h.chains[index] = b[:len(b)-1]
				h.count--
				return true
			}
		}
	}
	return false
}
func (h *HashTable) Keys() []string {
	keys := make([]string, 0, h.count)
	for _, e := range h.entries() {
		keys = append(keys, e.key)
	}
	sort.Strings(keys)
	return keys
}
