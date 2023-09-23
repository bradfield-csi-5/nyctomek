package main

import (
	"flag"
	"log"

	"golang.org/x/sys/unix"
)

var (
	PORT               = 1983
	FORWARD_DEST_PORT  = 2023
	ADDR               = [4]byte{127, 0, 0, 1}
	LISTEN_QUEUE_DEPTH = 1000
	MSG_BUFFER_SIZE    = 64
)

func main() {

	mode := flag.String("mode", "echo", "Mode to run: valid options are 'echo' and 'forward'.")
	flag.Parse()

	switch *mode {
	case "echo":
		echoServer()
	case "forward":
		forwardingServer()
	default:
		log.Fatal("Invalid mode:", *mode, "specified, valid modes are 'echo' and 'forward'.")
	}
}

func initializeServer() int {
	serverFileDescriptor, err := unix.Socket(unix.AF_INET, unix.SOCK_STREAM, unix.IPPROTO_IP)
	handleError(err)

	serverAddress := &unix.SockaddrInet4{Port: PORT, Addr: ADDR}
	err = unix.Bind(serverFileDescriptor, serverAddress)
	handleError(err)

	err = unix.Listen(serverFileDescriptor, LISTEN_QUEUE_DEPTH)
	handleError(err)

	return serverFileDescriptor
}

func echoServer() {

	serverFileDescriptor := initializeServer()

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

func forwardingServer() {

	serverFileDescriptor := initializeServer()

	destinationFileDescriptor, err := unix.Socket(unix.AF_INET, unix.SOCK_STREAM, unix.IPPROTO_IP)
	handleError(err)

	destinationAddress := &unix.SockaddrInet4{Port: FORWARD_DEST_PORT, Addr: ADDR}
	err = unix.Connect(destinationFileDescriptor, destinationAddress)
	handleError(err)

	for {
		clientFileDescriptor, _, err := unix.Accept(serverFileDescriptor)
		handleError(err)

		runForwardingLoop(clientFileDescriptor, destinationFileDescriptor, destinationAddress)
	}
}

func runForwardingLoop(
	clientFileDescriptor int,
	destinationFileDescriptor int,
	destinationAddress unix.Sockaddr) {

	message := make([]byte, MSG_BUFFER_SIZE)

	for {
		receivedBytes, _, err := unix.Recvfrom(clientFileDescriptor, message, 0)
		handleError(err)

		if receivedBytes == 0 {
			unix.Close(clientFileDescriptor)
			return
		}
		err = unix.Sendto(destinationFileDescriptor, message[:receivedBytes], 0, destinationAddress)
		handleError(err)
	}
}

func handleError(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
