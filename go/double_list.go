package structures

// doubleNode хранит ссылки на предыдущий и следующий узлы.
type doubleNode struct {
	value string
	prev  *doubleNode
	next  *doubleNode
}

type DoubleList struct {
	head *doubleNode
	tail *doubleNode
	size int
}

func (list *DoubleList) Size() int {
	return list.size
}

func (list *DoubleList) at(index int) *doubleNode {
	if index < 0 || index >= list.size {
		panic("index out of range")
	}
	node := list.head
	for i := 0; i < index; i++ {
		node = node.next
	}
	return node
}

// Вставляет узел и обновляет связи с соседними узлами.
func (list *DoubleList) Insert(index int, value string) {
	if index < 0 || index > list.size {
		panic("index out of range")
	}
	node := &doubleNode{value: value}
	if index == list.size {
		node.prev = list.tail
		if list.tail == nil {
			list.head = node
		} else {
			list.tail.next = node
		}
		list.tail = node
	} else {
		after := list.at(index)
		node.next = after
		node.prev = after.prev
		if after.prev == nil {
			list.head = node
		} else {
			after.prev.next = node
		}
		after.prev = node
	}
	list.size++
}

func (list *DoubleList) Get(index int) string {
	return list.at(index).value
}

// Удаляет узел, сохраняя прямые и обратные связи списка.
func (list *DoubleList) Delete(index int) string {
	node := list.at(index)
	if node.prev == nil {
		list.head = node.next
	} else {
		node.prev.next = node.next
	}
	if node.next == nil {
		list.tail = node.prev
	} else {
		node.next.prev = node.prev
	}
	list.size--
	return node.value
}

func (list *DoubleList) Find(value string) int {
	index := 0
	for node := list.head; node != nil; node = node.next {
		if node.value == value {
			return index
		}
		index++
	}
	return -1
}

// Читает список от начала или от конца.
func (list *DoubleList) Values(reverse bool) []string {
	result := []string{}
	if reverse {
		for node := list.tail; node != nil; node = node.prev {
			result = append(result, node.value)
		}
	} else {
		for node := list.head; node != nil; node = node.next {
			result = append(result, node.value)
		}
	}
	return result
}
