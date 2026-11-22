package structures

// singleNode хранит значение и ссылку на следующий узел.
type singleNode struct {
	value string
	next  *singleNode
}

type SingleList struct {
	head *singleNode
	tail *singleNode
	size int
}

func (list *SingleList) Size() int {
	return list.size
}

// Находит узел по индексу, последовательно проходя список.
func (list *SingleList) at(index int) *singleNode {
	if index < 0 || index >= list.size {
		panic("index out of range")
	}
	node := list.head
	for i := 0; i < index; i++ {
		node = node.next
	}
	return node
}

// Вставляет новый узел в начало или после предыдущего узла.
func (list *SingleList) Insert(index int, value string) {
	if index < 0 || index > list.size {
		panic("index out of range")
	}
	node := &singleNode{value: value}
	if index == 0 {
		node.next = list.head
		list.head = node
		if list.tail == nil {
			list.tail = node
		}
	} else {
		before := list.tail
		if index < list.size {
			before = list.at(index - 1)
		}
		node.next = before.next
		before.next = node
		if node.next == nil {
			list.tail = node
		}
	}
	list.size++
}

func (list *SingleList) Get(index int) string {
	return list.at(index).value
}

// Удаляет узел и соединяет соседние элементы списка.
func (list *SingleList) Delete(index int) string {
	if index < 0 || index >= list.size {
		panic("index out of range")
	}
	var node *singleNode
	if index == 0 {
		node = list.head
		list.head = node.next
		if list.head == nil {
			list.tail = nil
		}
	} else {
		before := list.at(index - 1)
		node = before.next
		before.next = node.next
		if before.next == nil {
			list.tail = before
		}
	}
	list.size--
	return node.value
}

func (list *SingleList) Find(value string) int {
	index := 0
	for node := list.head; node != nil; node = node.next {
		if node.value == value {
			return index
		}
		index++
	}
	return -1
}

func (list *SingleList) Values() []string {
	result := []string{}
	for node := list.head; node != nil; node = node.next {
		result = append(result, node.value)
	}
	return result
}
