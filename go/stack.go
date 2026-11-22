package structures

// Стек хранит вершину; первым удаляется последний добавленный элемент.
type stackNode struct {
	value string
	next  *stackNode
}

type Stack struct {
	top *stackNode
}

func (stack *Stack) Push(value string) {
	stack.top = &stackNode{value: value, next: stack.top}
}

func (stack *Stack) Get() string {
	if stack.top == nil {
		panic("stack is empty")
	}
	return stack.top.value
}

func (stack *Stack) Pop() string {
	value := stack.Get()
	stack.top = stack.top.next
	return value
}

func (stack *Stack) Values() []string {
	result := []string{}
	for node := stack.top; node != nil; node = node.next {
		result = append(result, node.value)
	}
	return result
}
