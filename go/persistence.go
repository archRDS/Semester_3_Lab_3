package structures

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"strings"
)

const (
	binaryHeader   = "LR3G"
	maxItems       = 1_000_000
	maxStringBytes = 16 * 1024 * 1024
)

// Lab1Data groups the structures from Lab 1 into one binary snapshot.
type Lab1Data struct {
	Array  Array
	Single SingleList
	Double DoubleList
	Queue  Queue
	Stack  Stack
}

func (data *Lab1Data) MarshalBinary() ([]byte, error) {
	out := []byte(binaryHeader)
	groups := [][]string{
		data.Array.Values(), data.Single.Values(), data.Double.Values(false),
		data.Queue.Values(), data.Stack.Values(),
	}
	for _, group := range groups {
		if len(group) > maxItems {
			return nil, errors.New("too many items")
		}
		out = binary.LittleEndian.AppendUint32(out, uint32(len(group)))
		for _, value := range group {
			if len(value) > maxStringBytes {
				return nil, errors.New("string too large")
			}
			out = binary.LittleEndian.AppendUint32(out, uint32(len(value)))
			out = append(out, value...)
		}
	}
	return out, nil
}

func readCount(in *bytes.Reader, limit uint32) (uint32, error) {
	var value uint32
	if err := binary.Read(in, binary.LittleEndian, &value); err != nil {
		return 0, err
	}
	if value > limit {
		return 0, errors.New("invalid length")
	}
	return value, nil
}

func readStrings(in *bytes.Reader) ([]string, error) {
	count, err := readCount(in, maxItems)
	if err != nil {
		return nil, err
	}
	values := []string{}
	for i := uint32(0); i < count; i++ {
		size, err := readCount(in, maxStringBytes)
		if err != nil {
			return nil, err
		}
		if uint64(size) > uint64(in.Len()) {
			return nil, io.ErrUnexpectedEOF
		}
		value := make([]byte, size)
		if _, err := io.ReadFull(in, value); err != nil {
			return nil, err
		}
		values = append(values, string(value))
	}
	return values, nil
}

func (data *Lab1Data) UnmarshalBinary(raw []byte) error {
	if !bytes.HasPrefix(raw, []byte(binaryHeader)) {
		return errors.New("invalid binary header")
	}
	in := bytes.NewReader(raw[len(binaryHeader):])
	var groups [5][]string
	for i := range groups {
		values, err := readStrings(in)
		if err != nil {
			return fmt.Errorf("group %d: %w", i, err)
		}
		groups[i] = values
	}
	if in.Len() != 0 {
		return errors.New("unexpected data after stack")
	}

	var next Lab1Data
	for _, value := range groups[0] {
		next.Array.Push(value)
	}
	for _, value := range groups[1] {
		next.Single.Insert(next.Single.Size(), value)
	}
	for _, value := range groups[2] {
		next.Double.Insert(next.Double.Size(), value)
	}
	for _, value := range groups[3] {
		next.Queue.Push(value)
	}
	for i := len(groups[4]) - 1; i >= 0; i-- {
		next.Stack.Push(groups[4][i])
	}
	*data = next
	return nil
}

func (h *HashTable) MarshalText() ([]byte, error) {
	var out strings.Builder
	method := "chain"
	if h.cuckoo {
		method = "cuckoo"
	}
	out.WriteString("LR3T " + method + "\n")
	for _, key := range h.Keys() {
		value, _ := h.Get(key)
		out.WriteString(hex.EncodeToString([]byte(key)))
		out.WriteByte('\t')
		out.WriteString(hex.EncodeToString([]byte(value)))
		out.WriteByte('\n')
	}
	return []byte(out.String()), nil
}

func (h *HashTable) UnmarshalText(raw []byte) error {
	lines := strings.Split(strings.TrimSuffix(string(raw), "\n"), "\n")
	header := strings.TrimSuffix(lines[0], "\r")
	if header != "LR3T chain" && header != "LR3T cuckoo" {
		return errors.New("invalid text header")
	}
	next := NewHashTable(header == "LR3T cuckoo")
	for _, line := range lines[1:] {
		keyHex, valueHex, found := strings.Cut(strings.TrimSuffix(line, "\r"), "\t")
		if !found {
			return errors.New("missing key/value separator")
		}
		key, err := hex.DecodeString(keyHex)
		if err != nil {
			return err
		}
		value, err := hex.DecodeString(valueHex)
		if err != nil {
			return err
		}
		if !next.Put(string(key), string(value)) {
			return errors.New("duplicate hash key")
		}
	}
	*h = *next
	return nil
}
