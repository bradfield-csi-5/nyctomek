package main

import (
	"log"

	"golang.org/x/sys/unix"
)

var (
	PORT               = 1983
	ADDR               = [4]byte{127, 0, 0, 1}
	LISTEN_QUEUE_DEPTH = 1000
)

func main() {
	echoServer()
}

func echoServer() {

	// Create a socket.
	serverFileDescriptor, err := unix.Socket(unix.AF_INET, unix.SOCK_STREAM, unix.IPPROTO_IP)
	handleError(err)

	// Bind to port 1983.
	serverAddress := &unix.SockaddrInet4{Port: PORT, Addr: ADDR}
	err = unix.Bind(serverFileDescriptor, serverAddress)
	handleError(err)

	// Start listening for connections.
	err = unix.Listen(serverFileDescriptor, LISTEN_QUEUE_DEPTH)
	handleError(err)

	for {

		// Accept a connection.
		connectedFileDescriptor, connectedAddress, err := unix.Accept(serverFileDescriptor)
		handleError(err)

		message := make([]byte, 1024)
		_, _, err = unix.Recvfrom(connectedFileDescriptor, message, 0)
		handleError(err)

		// Echo back the received message.
		err = unix.Sendto(connectedFileDescriptor, message, 0, connectedAddress)
		handleError(err)

		// Close the connection.
		unix.Close(connectedFileDescriptor)
	}
}

func handleError(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
