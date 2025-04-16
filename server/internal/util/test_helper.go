package util

import "testing"

func Assert(t *testing.T, condition bool, message string) {
	if !condition {
		t.Error(message)
	}
}

func AssertEqual(t *testing.T, expected, actual interface{}) {
	if expected != actual {
		t.Errorf("expected: %v, actual: %v", expected, actual)
	}
}

func AssertErrorNil(t *testing.T, value error) {
	if value != nil {
		t.Error("expected: not nil, actual: nil")
	}
}
