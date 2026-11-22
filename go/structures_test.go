package structures

import (
	"encoding/binary"
	"fmt"
	"strconv"
	"testing"

	"github.com/stretchr/testify/require"
)

func TestArray(t *testing.T) {
	var array Array
	require.Zero(t, array.Size())
	require.Empty(t, array.Values())
	require.Panics(t, func() { array.Get(0) })
	require.Panics(t, func() { array.Insert(-1, "x") })
	require.Panics(t, func() { array.Insert(1, "x") })
	for i := 0; i < 9; i++ {
		array.Push(strconv.Itoa(i))
	}
	array.Insert(0, "first")
	array.Insert(3, "middle")
	array.Set(2, "changed")
	require.Equal(t, "changed", array.Get(2))
	require.Equal(t, "middle", array.Delete(3))
	require.Equal(t, "8", array.Delete(array.Size()-1))
	require.Equal(t, []string{"first", "0", "changed", "2", "3", "4", "5", "6", "7"}, array.Values())
	require.Equal(t, 9, array.Size())
	require.Panics(t, func() { array.Set(-1, "x") })
	require.Panics(t, func() { array.Set(array.Size(), "x") })
	require.Panics(t, func() { array.Delete(array.Size()) })
}

func TestLists(t *testing.T) {
	type list interface {
		Size() int
		Insert(int, string)
		Get(int) string
		Delete(int) string
		Find(string) int
	}
	single, doubly := &SingleList{}, &DoubleList{}
	for _, tc := range []struct {
		name    string
		list    list
		values  func() []string
		reverse func() []string
	}{
		{"single", single, single.Values, nil},
		{"double", doubly, func() []string { return doubly.Values(false) }, func() []string { return doubly.Values(true) }},
	} {
		t.Run(tc.name, func(t *testing.T) {
			l := tc.list
			require.Zero(t, l.Size())
			require.Empty(t, tc.values())
			require.Panics(t, func() { l.Get(0) })
			require.Panics(t, func() { l.Delete(0) })
			require.Panics(t, func() { l.Insert(-1, "x") })
			require.Panics(t, func() { l.Insert(1, "x") })

			l.Insert(0, "a")
			l.Insert(1, "c")
			l.Insert(1, "b")
			l.Insert(0, "first")
			l.Insert(4, "b")
			require.Equal(t, 5, l.Size())
			require.Equal(t, "c", l.Get(3))
			require.Equal(t, 2, l.Find("b"))
			require.Equal(t, -1, l.Find("missing"))
			require.Equal(t, []string{"first", "a", "b", "c", "b"}, tc.values())
			if tc.reverse != nil {
				require.Equal(t, []string{"b", "c", "b", "a", "first"}, tc.reverse())
			}
			require.Equal(t, "b", l.Delete(2))
			require.Equal(t, "b", l.Delete(l.Size()-1))
			require.Equal(t, "first", l.Delete(0))
			require.Equal(t, []string{"a", "c"}, tc.values())
			if tc.reverse != nil {
				require.Equal(t, []string{"c", "a"}, tc.reverse())
			}
			require.Equal(t, "a", l.Delete(0))
			require.Equal(t, "c", l.Delete(0))
			require.Zero(t, l.Size())
			l.Insert(0, "new")
			l.Insert(1, "last")
			require.Equal(t, []string{"new", "last"}, tc.values())
		})
	}
}

func TestQueueFIFO(t *testing.T) {
	var queue Queue
	require.Empty(t, queue.Values())
	require.Panics(t, func() { queue.Get() })
	require.Panics(t, func() { queue.Pop() })
	queue.Push("a")
	queue.Push("b")
	require.Equal(t, "a", queue.Get())
	require.Equal(t, []string{"a", "b"}, queue.Values())
	require.Equal(t, "a", queue.Pop())
	queue.Push("c")
	require.Equal(t, "b", queue.Pop())
	require.Equal(t, "c", queue.Pop())
	queue.Push("new")
	require.Equal(t, []string{"new"}, queue.Values())
}

func TestStackLIFO(t *testing.T) {
	var stack Stack
	require.Empty(t, stack.Values())
	require.Panics(t, func() { stack.Get() })
	require.Panics(t, func() { stack.Pop() })
	stack.Push("a")
	stack.Push("b")
	require.Equal(t, "b", stack.Get())
	require.Equal(t, []string{"b", "a"}, stack.Values())
	require.Equal(t, "b", stack.Pop())
	require.Equal(t, "a", stack.Pop())
	stack.Push("new")
	require.Equal(t, []string{"new"}, stack.Values())
}

func TestHashTables(t *testing.T) {
	for _, cuckoo := range []bool{false, true} {
		t.Run(fmt.Sprintf("cuckoo=%t", cuckoo), func(t *testing.T) {
			h := NewHashTable(cuckoo)
			initialCapacity := h.Capacity()
			_, found := h.Get("missing")
			require.False(t, found)
			require.False(t, h.Delete("missing"))
			// These keys initially map to the same two buckets.
			require.True(t, h.Put("key25", "one"))
			require.True(t, h.Put("key44", "two"))
			require.True(t, h.Put("key52", "three"))
			require.False(t, h.Put("key44", "updated"))
			require.Equal(t, 3, h.Size())
			require.Equal(t, []string{"key25", "key44", "key52"}, h.Keys())
			for k, want := range map[string]string{"key25": "one", "key44": "updated", "key52": "three"} {
				got, ok := h.Get(k)
				require.True(t, ok)
				require.Equal(t, want, got)
			}
			require.True(t, h.Delete("key25"))
			require.False(t, h.Delete("key25"))
			for i := 0; i < 500; i++ {
				require.True(t, h.Put("item"+strconv.Itoa(i), strconv.Itoa(i)))
			}
			require.Greater(t, h.Capacity(), initialCapacity)
			require.Equal(t, 502, h.Size())
			for i := 0; i < 500; i++ {
				key := "item" + strconv.Itoa(i)
				value, found := h.Get(key)
				require.True(t, found, key)
				require.Equal(t, strconv.Itoa(i), value)
				require.True(t, h.Delete(key))
			}
			require.Equal(t, []string{"key44", "key52"}, h.Keys())
		})
	}
}

func TestBinarySnapshot(t *testing.T) {
	var data Lab1Data
	data.Array.Push("строка\x00")
	data.Array.Push("")
	data.Single.Insert(0, "one")
	data.Single.Insert(1, "two")
	data.Double.Insert(0, "left")
	data.Double.Insert(1, "right")
	data.Queue.Push("first")
	data.Queue.Push("second")
	data.Stack.Push("bottom")
	data.Stack.Push("top")
	raw, err := data.MarshalBinary()
	require.NoError(t, err)
	var decoded Lab1Data
	require.NoError(t, decoded.UnmarshalBinary(raw))
	require.Equal(t, []string{"строка\x00", ""}, decoded.Array.Values())
	require.Equal(t, []string{"one", "two"}, decoded.Single.Values())
	require.Equal(t, []string{"right", "left"}, decoded.Double.Values(true))
	require.Equal(t, "first", decoded.Queue.Pop())
	require.Equal(t, "top", decoded.Stack.Pop())

	require.Error(t, decoded.UnmarshalBinary([]byte("bad")))
	for n := len(binaryHeader); n < len(raw); n++ {
		require.Error(t, decoded.UnmarshalBinary(raw[:n]), "accepted prefix of %d bytes", n)
	}
	require.Error(t, decoded.UnmarshalBinary(append(raw, 0)))
	invalid := binary.LittleEndian.AppendUint32([]byte(binaryHeader), maxItems+1)
	require.Error(t, decoded.UnmarshalBinary(invalid))
	require.Equal(t, []string{"строка\x00", ""}, decoded.Array.Values())
	require.Equal(t, []string{"second"}, decoded.Queue.Values())
}

func TestHashTableText(t *testing.T) {
	for _, cuckoo := range []bool{false, true} {
		h := NewHashTable(cuckoo)
		h.Put("", "")
		h.Put("a", "b")
		h.Put("ключ\t", "value\n\x00")
		raw, err := h.MarshalText()
		require.NoError(t, err)
		var decoded HashTable
		require.NoError(t, decoded.UnmarshalText(raw))
		require.Equal(t, cuckoo, decoded.cuckoo)
		require.Equal(t, h.Keys(), decoded.Keys())
		value, found := decoded.Get("ключ\t")
		require.True(t, found)
		require.Equal(t, "value\n\x00", value)
		restored, err := decoded.MarshalText()
		require.NoError(t, err)
		require.Equal(t, raw, restored)
	}

	var h HashTable
	require.NoError(t, h.UnmarshalText([]byte("LR3T chain\r\n61\t62\r\n")))
	for _, bad := range []string{
		"invalid", "LR3T chain\n61", "LR3T chain\nzz\t62",
		"LR3T chain\n61\tz", "LR3T chain\n61\t62\n61\t63",
	} {
		require.Error(t, h.UnmarshalText([]byte(bad)))
		require.Equal(t, 1, h.Size())
	}
}
