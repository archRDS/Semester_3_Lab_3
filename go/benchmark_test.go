package structures

import "testing"

func BenchmarkStructures(b *testing.B) {
	var array Array
	var single SingleList
	var doubly DoubleList
	var queue Queue
	var stack Stack
	chain, cuckoo := NewHashTable(false), NewHashTable(true)
	for _, benchmark := range []struct {
		name string
		run  func()
	}{
		{"array", func() { array.Push("value"); array.Delete(0) }},
		{"single_list", func() { single.Insert(0, "value"); single.Delete(0) }},
		{"double_list", func() { doubly.Insert(0, "value"); doubly.Delete(0) }},
		{"queue", func() { queue.Push("value"); queue.Pop() }},
		{"stack", func() { stack.Push("value"); stack.Pop() }},
		{"hash_chain", func() { chain.Put("key", "value"); chain.Get("key"); chain.Delete("key") }},
		{"hash_cuckoo", func() { cuckoo.Put("key", "value"); cuckoo.Get("key"); cuckoo.Delete("key") }},
	} {
		b.Run(benchmark.name, func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				benchmark.run()
			}
		})
	}
}
