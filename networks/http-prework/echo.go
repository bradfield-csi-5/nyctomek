package main

import (
	"log"

	"golang.org/x/sys/unix"
)

var (
	PORT               = 1983
	ADDR               = [4]byte{127, 0, 0, 1}
	LISTEN_QUEUE_DEPTH = 1000
	MSG_BUFFER_SIZE    = 64
)

func main() {
	echoServer()
}

func echoServer() {

	serverFileDescriptor, err := unix.Socket(unix.AF_INET, unix.SOCK_STREAM, unix.IPPROTO_IP)
	handleError(err)

	serverAddress := &unix.SockaddrInet4{Port: PORT, Addr: ADDR}
	err = unix.Bind(serverFileDescriptor, serverAddress)
	handleError(err)

	err = unix.Listen(serverFileDescriptor, LISTEN_QUEUE_DEPTH)
	handleError(err)

	for {
		clientFileDescriptor, clientAddress, err := unix.Accept(serverFileDescriptor)
		handleError(err)

		runEchoLoop(clientFileDescriptor, clientAddress)
	}
}

func runEchoLoop(clientFileDescriptor int, clientAddress unix.Sockaddr) {
	message := make([]byte, MSG_BUFFER_SIZE)
	for {
		receivedBytes, _, err := unix.Recvfrom(clientFileDescriptor, message, 0)
		handleError(err)

		if receivedBytes == 0 {
			unix.Close(clientFileDescriptor)
			return
		}
		err = unix.Sendto(clientFileDescriptor, message[:receivedBytes], 0, clientAddress)
		handleError(err)
	}
}

func handleError(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
