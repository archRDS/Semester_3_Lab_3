package structures

// Array хранит элементы, их количество и размер выделенной памяти.
type Array struct {
	data     []string
	size     int
	capacity int
}

func (array *Array) Size() int {
	return array.size
}

func (array *Array) Push(value string) {
	array.Insert(array.size, value)
}

// При нехватке места увеличивает массив и вставляет значение по индексу.
func (array *Array) Insert(index int, value string) {
	if index < 0 || index > array.size {
		panic("index out of range")
	}
	if array.size == array.capacity {
		newCapacity := 4
		if array.capacity > 0 {
			newCapacity = array.capacity * 2
		}
		newData := make([]string, newCapacity)
		copy(newData, array.data)
		array.data = newData
		array.capacity = newCapacity
	}
	for i := array.size; i > index; i-- {
		array.data[i] = array.data[i-1]
	}
	array.data[index] = value
	array.size++
}

func (array *Array) Get(index int) string {
	if index < 0 || index >= array.size {
		panic("index out of range")
	}
	return array.data[index]
}

func (array *Array) Set(index int, value string) {
	if index < 0 || index >= array.size {
		panic("index out of range")
	}
	array.data[index] = value
}

// Удаляет элемент и сдвигает оставшиеся значения влево.
func (array *Array) Delete(index int) string {
	value := array.Get(index)
	for i := index; i+1 < array.size; i++ {
		array.data[i] = array.data[i+1]
	}
	array.size--
	array.data[array.size] = ""
	return value
}

func (array *Array) Values() []string {
	result := make([]string, array.size)
	copy(result, array.data[:array.size])
	return result
}
