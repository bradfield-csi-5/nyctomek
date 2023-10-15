package main

import (
	"bytes"
	"flag"
	"fmt"
	"log"

	"golang.org/x/sys/unix"
)

var (
	PORT               = 1983
	FORWARD_DEST_PORT  = 2025
	ADDR               = [4]byte{127, 0, 0, 1}
	LISTEN_QUEUE_DEPTH = 1000
	MSG_BUFFER_SIZE    = 4096
	CRLF_DELIMETER     = "\r\n"
)

func main() {

	mode := flag.String("mode", "echo", "Mode to run: valid options are 'echo' and 'forward'.")
	flag.Parse()

	switch *mode {
	case "echo":
		echoServer()
	case "forward":
		forwardingServer()
	case "proxy":
		httpProxyServer()
	default:
		log.Fatal("Invalid mode:", *mode, "specified, valid modes are 'echo' and 'forward'.")
	}
}

func initializeServer() int {
	serverFileDescriptor, err := unix.Socket(unix.AF_INET, unix.SOCK_STREAM, unix.IPPROTO_IP)
	handleError(err)
	unix.SetsockoptInt(serverFileDescriptor, unix.SOL_SOCKET, unix.SO_REUSEADDR, 1)
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
		err = unix.Sendto(clientFileDescriptor, []byte("runEchoLoop"), 0, clientAddress)

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

func httpProxyServer() {

	proxyFileDescriptor := initializeServer()

	destinationAddress := &unix.SockaddrInet4{Port: FORWARD_DEST_PORT, Addr: ADDR}

	for {
		clientFileDescriptor, _, err := unix.Accept(proxyFileDescriptor)
		handleError(err)

		destinationFileDescriptor, err := unix.Socket(unix.AF_INET, unix.SOCK_STREAM, unix.IPPROTO_IP)
		handleError(err)

		err = unix.Connect(destinationFileDescriptor, destinationAddress)
		handleError(err)

		runHTTPProxyLoop(clientFileDescriptor, destinationFileDescriptor)
	}
}

func parseContentLength(header string) int {
	var contentLength int
	fmt.Sscanf(header, "Content-Length: %d \r\n", &contentLength)
	return contentLength
}

func processCompleteHTTPLine(httpLine []byte, contentLength int) (int, bool) {
	var receivedNullLine bool
	if string(httpLine) == CRLF_DELIMETER {
		receivedNullLine = true
	}
	if contentLength == 0 {
		// Check if the current line contains a Content-Length header.
		contentLength = parseContentLength(string(httpLine))
	}
	return contentLength, receivedNullLine
}

func streamInHTTPContent(packet string, contentLength int, receivedContentLength int) ([]byte, int, int, bool, bool) {
	receivedContentLength += len(packet)
	done := contentLength == receivedContentLength
	return []byte(packet), contentLength, receivedContentLength, true, done
}

func streamInHTTPMessage(
	packet string,
	currentHttpLine []byte,
	receivedNullLine bool,
	contentLength int,
	receivedContentLength int) ([]byte, int, int, bool, bool) {

	if receivedNullLine && contentLength != 0 && receivedContentLength < contentLength {
		return streamInHTTPContent(packet, contentLength, receivedContentLength)
	}

	// Special Case: CRLF is split among two adjacent packets.
	currentHttpLineLastPos := len(currentHttpLine) - 1
	if currentHttpLineLastPos >= 0 &&
		currentHttpLine[currentHttpLineLastPos] == CRLF_DELIMETER[0] &&
		packet[0] == CRLF_DELIMETER[1] {
		currentHttpLine = append(currentHttpLine, packet[0])
		contentLength, receivedNullLine := processCompleteHTTPLine(currentHttpLine, contentLength)
		currentHttpLine = currentHttpLine[:0]
		packet = packet[1:]
		if receivedNullLine && contentLength != 0 && receivedContentLength < contentLength {
			return streamInHTTPContent(packet, contentLength, receivedContentLength)
		}

	}

	for len(packet) > 0 {
		copyEndPos := bytes.Index([]byte(packet), []byte(CRLF_DELIMETER))
		if copyEndPos < 0 {
			copyEndPos = len(packet)
		} else {
			copyEndPos += len(CRLF_DELIMETER)
		}
		currentHttpLine = append(currentHttpLine, []byte(packet)[:copyEndPos]...)

		if bytes.HasSuffix(currentHttpLine, []byte(CRLF_DELIMETER)) {
			contentLength, receivedNullLine = processCompleteHTTPLine(currentHttpLine, contentLength)
			currentHttpLine = currentHttpLine[:0]

		}
		packet = packet[copyEndPos:]
		if receivedNullLine && contentLength != 0 && receivedContentLength < contentLength {
			return streamInHTTPContent(packet, contentLength, receivedContentLength)
		}

	}
	done := receivedNullLine && (receivedContentLength == contentLength)
	return currentHttpLine, contentLength, receivedContentLength, receivedNullLine, done
}

func runHTTPProxyLoop(
	clientFileDescriptor int,
	destinationFileDescriptor int) {

	message := make([]byte, MSG_BUFFER_SIZE)
	//receivedBytes, _, err := unix.Recvfrom(clientFileDescriptor, message, 0)
	//handleError(err)
	//fmt.Printf("Received %d bytes from client.\n", receivedBytes)

	httpLine := make([]byte, 0)
	receivedNullLine := false
	contentLength := 0
	receivedContentLength := 0

	for done := false; done == false; {
		receivedBytes, _, err := unix.Recvfrom(clientFileDescriptor, message, 0)
		handleError(err)
		if receivedBytes == 0 {
			break
		}

		err = unix.Send(destinationFileDescriptor, message[:receivedBytes], 0)
		handleError(err)

		httpLine, contentLength, receivedContentLength, receivedNullLine, done = streamInHTTPMessage(
			string(message[:receivedBytes]),
			httpLine,
			receivedNullLine,
			contentLength,
			receivedContentLength)
	}

	for done := false; done == false; {
		receivedBytes, _, err := unix.Recvfrom(destinationFileDescriptor, message, 0)
		handleError(err)
		fmt.Printf("Received %d bytes from server.\n", receivedBytes)

		err = unix.Send(clientFileDescriptor, message[:receivedBytes], 0)
		handleError(err)

		httpLine, contentLength, receivedContentLength, receivedNullLine, done = streamInHTTPMessage(
			string(message[:receivedBytes]),
			httpLine,
			receivedNullLine,
			contentLength,
			receivedContentLength)
	}
	unix.Close(destinationFileDescriptor)
	unix.Close(clientFileDescriptor)

}

func handleError(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
