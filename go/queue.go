package structures

// Очередь хранит начало и конец; первым удаляется первый добавленный элемент.
type queueNode struct {
	value string
	next  *queueNode
}

type Queue struct {
	head *queueNode
	tail *queueNode
}

func (queue *Queue) Push(value string) {
	node := &queueNode{value: value}
	if queue.tail == nil {
		queue.head = node
	} else {
		queue.tail.next = node
	}
	queue.tail = node
}

func (queue *Queue) Get() string {
	if queue.head == nil {
		panic("queue is empty")
	}
	return queue.head.value
}

func (queue *Queue) Pop() string {
	value := queue.Get()
	queue.head = queue.head.next
	if queue.head == nil {
		queue.tail = nil
	}
	return value
}

func (queue *Queue) Values() []string {
	result := []string{}
	for node := queue.head; node != nil; node = node.next {
		result = append(result, node.value)
	}
	return result
}
