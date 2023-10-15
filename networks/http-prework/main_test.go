package main

import (
	"bytes"
	"fmt"
	"testing"
)

const (
	delimeterLength = 4
	CRLF            = "\r\n"
	headerDelimeter = "\r\n\r\n"
)

func readHttpMessage(packets []string) []byte {
	output := make([]byte, 0)
	tail := make([]byte, 0)
	i := 0
	for {
		tailLen := min(delimeterLength, len(packets[i]))
		tailStartPos := len(packets[i]) - tailLen
		currentTail := packets[i][tailStartPos:]
		tail = append(tail, currentTail...)
		if len(tail) > delimeterLength {
			tailStartPos = len(tail) - delimeterLength
			tail = tail[tailStartPos:]
		}
		output = append(output, packets[i]...)
		if bytes.Equal(tail, []byte(headerDelimeter)) {
			break
		}
		i++
	}

	contentLengthPos := bytes.Index(output, []byte("Content-Length: "))
	if contentLengthPos >= 0 {
		newlinePos := bytes.Index(output[contentLengthPos:], []byte("\r\n"))
		var contentLength int
		fmt.Sscanf(string(output[contentLengthPos:newlinePos]), "Content-Length: %d", contentLength)
		for {
			output = append(output, packets[i]...)
			contentLength -= len(packets[i])
			if contentLength <= 0 {
				break
			}
			i++
		}
	}

	return output
}

func TestReadHTTPMessage(t *testing.T) {

	input := []string{"GET / HTTP/1.0", "\r\n\r\n"}
	expectedOutput := "GET / HTTP/1.0\r\n\r\n"

	output := readHttpMessage(input)

	if !bytes.Equal(output, []byte(expectedOutput)) {
		t.Error("Expected output: ", expectedOutput,
			"Actual output: ", output, ".")
	}

	input = []string{"GET / HTTP/1.1\r\nHost: localhost:1983\r\n",
		"Accept-Language: pl,en-US;q=0.7,en;q=0.3\r\n",
		"Accept-Encoding: gzip, deflate, br\r\n",
		"DNT: 1\r\nConnection: keep-alive\r",
		"\n\r\n"}
	expectedOutput = "GET / HTTP/1.1\r\nHost: localhost:1983\r\nAccept-Language: pl,en-US;q=0.7,en;q=0.3\r\nAccept-Encoding: gzip, deflate, br\r\nDNT: 1\r\nConnection: keep-alive\r\n\r\n"

	output = readHttpMessage(input)

	if !bytes.Equal(output, []byte(expectedOutput)) {
		t.Error("Expected output: ", expectedOutput,
			"Actual output: ", output, ".")
	}

	input = []string{"\x48\x54\x54\x50\x2f\x31\x2e\x30\x20\x32\x30\x30\x20\x4f\x4b\x0d",
		"\x0a\x53\x65\x72\x76\x65\x72\x3a\x20\x42\x61\x73\x65\x48\x54\x54",
		"\x50\x2f\x30\x2e\x36\x20\x50\x79\x74\x68\x6f\x6e\x2f\x33\x2e\x31",
		"\x30\x2e\x31\x32\x0d\x0a\x44\x61\x74\x65\x3a\x20\x53\x75\x6e\x2c",
		"\x20\x30\x38\x20\x4f\x63\x74\x20\x32\x30\x32\x33\x20\x30\x33\x3a",
		"\x31\x30\x3a\x34\x39\x20\x47\x4d\x54\x0d\x0a\x43\x6f\x6e\x74\x65",
		"\x6e\x74\x2d\x4c\x65\x6e\x67\x74\x68\x3a\x20\x34\x35\x39\x0d\x0a",
		"\x0d\x0a\x7b\x0a\x20\x20\x20\x20\x22\x48\x6f\x73\x74\x22\x3a\x20",
		"\x22\x6c\x6f\x63\x61\x6c\x68\x6f\x73\x74\x3a\x32\x30\x32\x35\x22",
		"\x2c\x0a\x20\x20\x20\x20\x22\x55\x73\x65\x72\x2d\x41\x67\x65\x6e",
		"\x74\x22\x3a\x20\x22\x4d\x6f\x7a\x69\x6c\x6c\x61\x2f\x35\x2e\x30",
		"\x20\x28\x58\x31\x31\x3b\x20\x55\x62\x75\x6e\x74\x75\x3b\x20\x4c",
		"\x69\x6e\x75\x78\x20\x78\x38\x36\x5f\x36\x34\x3b\x20\x72\x76\x3a",
		"\x31\x30\x39\x2e\x30\x29\x20\x47\x65\x63\x6b\x6f\x2f\x32\x30\x31",
		"\x30\x30\x31\x30\x31\x20\x46\x69\x72\x65\x66\x6f\x78\x2f\x31\x31",
		"\x38\x2e\x30\x22\x2c\x0a\x20\x20\x20\x20\x22\x41\x63\x63\x65\x70",
		"\x74\x22\x3a\x20\x22\x69\x6d\x61\x67\x65\x2f\x61\x76\x69\x66\x2c",
		"\x69\x6d\x61\x67\x65\x2f\x77\x65\x62\x70\x2c\x2a\x2f\x2a\x22\x2c",
		"\x0a\x20\x20\x20\x20\x22\x41\x63\x63\x65\x70\x74\x2d\x4c\x61\x6e",
		"\x67\x75\x61\x67\x65\x22\x3a\x20\x22\x70\x6c\x2c\x65\x6e\x2d\x55",
		"\x53\x3b\x71\x3d\x30\x2e\x37\x2c\x65\x6e\x3b\x71\x3d\x30\x2e\x33",
		"\x22\x2c\x0a\x20\x20\x20\x20\x22\x41\x63\x63\x65\x70\x74\x2d\x45",
		"\x6e\x63\x6f\x64\x69\x6e\x67\x22\x3a\x20\x22\x67\x7a\x69\x70\x2c",
		"\x20\x64\x65\x66\x6c\x61\x74\x65\x2c\x20\x62\x72\x22\x2c\x0a\x20",
		"\x20\x20\x20\x22\x44\x4e\x54\x22\x3a\x20\x22\x31\x22\x2c\x0a\x20",
		"\x20\x20\x20\x22\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x22\x3a",
		"\x20\x22\x6b\x65\x65\x70\x2d\x61\x6c\x69\x76\x65\x22\x2c\x0a\x20",
		"\x20\x20\x20\x22\x52\x65\x66\x65\x72\x65\x72\x22\x3a\x20\x22\x68",
		"\x74\x74\x70\x3a\x2f\x2f\x6c\x6f\x63\x61\x6c\x68\x6f\x73\x74\x3a",
		"\x32\x30\x32\x35\x2f\x22\x2c\x0a\x20\x20\x20\x20\x22\x53\x65\x63",
		"\x2d\x46\x65\x74\x63\x68\x2d\x44\x65\x73\x74\x22\x3a\x20\x22\x69",
		"\x6d\x61\x67\x65\x22\x2c\x0a\x20\x20\x20\x20\x22\x53\x65\x63\x2d",
		"\x46\x65\x74\x63\x68\x2d\x4d\x6f\x64\x65\x22\x3a\x20\x22\x6e\x6f",
		"\x2d\x63\x6f\x72\x73\x22\x2c\x0a\x20\x20\x20\x20\x22\x53\x65\x63",
		"\x2d\x46\x65\x74\x63\x68\x2d\x53\x69\x74\x65\x22\x3a\x20\x22\x73",
		"\x61\x6d\x65\x2d\x6f\x72\x69\x67\x69\x6e\x22\x0a\x7d"}
	expectedOutput =
		"\x48\x54\x54\x50\x2f\x31\x2e\x30\x20\x32\x30\x30\x20\x4f\x4b\x0d" +
			"\x0a\x53\x65\x72\x76\x65\x72\x3a\x20\x42\x61\x73\x65\x48\x54\x54" +
			"\x50\x2f\x30\x2e\x36\x20\x50\x79\x74\x68\x6f\x6e\x2f\x33\x2e\x31" +
			"\x30\x2e\x31\x32\x0d\x0a\x44\x61\x74\x65\x3a\x20\x53\x75\x6e\x2c" +
			"\x20\x30\x38\x20\x4f\x63\x74\x20\x32\x30\x32\x33\x20\x30\x33\x3a" +
			"\x31\x30\x3a\x34\x39\x20\x47\x4d\x54\x0d\x0a\x43\x6f\x6e\x74\x65" +
			"\x6e\x74\x2d\x4c\x65\x6e\x67\x74\x68\x3a\x20\x34\x35\x39\x0d\x0a" +
			"\x0d\x0a\x7b\x0a\x20\x20\x20\x20\x22\x48\x6f\x73\x74\x22\x3a\x20" +
			"\x22\x6c\x6f\x63\x61\x6c\x68\x6f\x73\x74\x3a\x32\x30\x32\x35\x22" +
			"\x2c\x0a\x20\x20\x20\x20\x22\x55\x73\x65\x72\x2d\x41\x67\x65\x6e" +
			"\x74\x22\x3a\x20\x22\x4d\x6f\x7a\x69\x6c\x6c\x61\x2f\x35\x2e\x30" +
			"\x20\x28\x58\x31\x31\x3b\x20\x55\x62\x75\x6e\x74\x75\x3b\x20\x4c" +
			"\x69\x6e\x75\x78\x20\x78\x38\x36\x5f\x36\x34\x3b\x20\x72\x76\x3a" +
			"\x31\x30\x39\x2e\x30\x29\x20\x47\x65\x63\x6b\x6f\x2f\x32\x30\x31" +
			"\x30\x30\x31\x30\x31\x20\x46\x69\x72\x65\x66\x6f\x78\x2f\x31\x31" +
			"\x38\x2e\x30\x22\x2c\x0a\x20\x20\x20\x20\x22\x41\x63\x63\x65\x70" +
			"\x74\x22\x3a\x20\x22\x69\x6d\x61\x67\x65\x2f\x61\x76\x69\x66\x2c" +
			"\x69\x6d\x61\x67\x65\x2f\x77\x65\x62\x70\x2c\x2a\x2f\x2a\x22\x2c" +
			"\x0a\x20\x20\x20\x20\x22\x41\x63\x63\x65\x70\x74\x2d\x4c\x61\x6e" +
			"\x67\x75\x61\x67\x65\x22\x3a\x20\x22\x70\x6c\x2c\x65\x6e\x2d\x55" +
			"\x53\x3b\x71\x3d\x30\x2e\x37\x2c\x65\x6e\x3b\x71\x3d\x30\x2e\x33" +
			"\x22\x2c\x0a\x20\x20\x20\x20\x22\x41\x63\x63\x65\x70\x74\x2d\x45" +
			"\x6e\x63\x6f\x64\x69\x6e\x67\x22\x3a\x20\x22\x67\x7a\x69\x70\x2c" +
			"\x20\x64\x65\x66\x6c\x61\x74\x65\x2c\x20\x62\x72\x22\x2c\x0a\x20" +
			"\x20\x20\x20\x22\x44\x4e\x54\x22\x3a\x20\x22\x31\x22\x2c\x0a\x20" +
			"\x20\x20\x20\x22\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x22\x3a" +
			"\x20\x22\x6b\x65\x65\x70\x2d\x61\x6c\x69\x76\x65\x22\x2c\x0a\x20" +
			"\x20\x20\x20\x22\x52\x65\x66\x65\x72\x65\x72\x22\x3a\x20\x22\x68" +
			"\x74\x74\x70\x3a\x2f\x2f\x6c\x6f\x63\x61\x6c\x68\x6f\x73\x74\x3a" +
			"\x32\x30\x32\x35\x2f\x22\x2c\x0a\x20\x20\x20\x20\x22\x53\x65\x63" +
			"\x2d\x46\x65\x74\x63\x68\x2d\x44\x65\x73\x74\x22\x3a\x20\x22\x69" +
			"\x6d\x61\x67\x65\x22\x2c\x0a\x20\x20\x20\x20\x22\x53\x65\x63\x2d" +
			"\x46\x65\x74\x63\x68\x2d\x4d\x6f\x64\x65\x22\x3a\x20\x22\x6e\x6f" +
			"\x2d\x63\x6f\x72\x73\x22\x2c\x0a\x20\x20\x20\x20\x22\x53\x65\x63" +
			"\x2d\x46\x65\x74\x63\x68\x2d\x53\x69\x74\x65\x22\x3a\x20\x22\x73" +
			"\x61\x6d\x65\x2d\x6f\x72\x69\x67\x69\x6e\x22\x0a\x7d"
	output = readHttpMessage(input)

	if !bytes.Equal(output, []byte(expectedOutput)) {
		t.Error("Expected output: ", expectedOutput,
			"Actual output: ", output, ".")
	}
}

func parseHttpLines(packets []string) []string {
	var httpLines []string
	currentLine := make([]byte, 0)
	for i := 0; i < len(packets); i++ {
		packet := packets[i]
		// Sanity Check.
		if len(packet) == 0 {
			continue
		}
		// Special Case: CRLF is split among two adjacent packets.
		currentLineLastPos := len(currentLine) - 1
		if currentLineLastPos >= 0 &&
			currentLine[currentLineLastPos] == CRLF[0] {
			if len(packet) > 0 && packet[0] == CRLF[1] {
				currentLine = append(currentLine, packet[0])
				httpLines = append(httpLines, string(currentLine))
				if string(currentLine) == CRLF {
					return httpLines
				}
				currentLine = currentLine[:0]
				packet = packet[1:]
			}
		}

		for len(packet) > 0 {
			copyEndPos := bytes.Index([]byte(packet), []byte(CRLF))
			if copyEndPos < 0 {
				copyEndPos = len(packet)
			} else {
				copyEndPos += len(CRLF)
			}
			currentLine = append(currentLine, []byte(packet)[:copyEndPos]...)

			if bytes.HasSuffix(currentLine, []byte(CRLF)) {
				httpLines = append(httpLines, string(currentLine))
				currentLine = currentLine[:0]
				if string(currentLine) == CRLF {
					return httpLines
				}
			}
			packet = packet[copyEndPos:]
		}
	}

	return httpLines
}

func compareParseHttpLineResults(output []string, expectedOutput []string, t *testing.T) {

	if len(output) != len(expectedOutput) {
		t.Error("Length mismatch, output length:", len(output),
			" expectedOutput length:", len(expectedOutput))
	} else {
		for i := 0; i < len(output); i++ {
			if output[i] != expectedOutput[i] {
				t.Error("Content mismatch at element:", i,
					" output contains:", output[i],
					" expectedOutput contains:", expectedOutput[i])
			}
		}
	}
}

func TestParseHttpLine(t *testing.T) {

	var input, output, expectedOutput []string

	input = []string{
		"GET / HTTP/1.1\r\n",
		"Host: localhost:1983\r\n",
		"User-Agent: curl/7.81.0\r\n",
		"Accept: */*\r\n",
		"\r\n",
	}
	expectedOutput = []string{
		"GET / HTTP/1.1\r\n",
		"Host: localhost:1983\r\n",
		"User-Agent: curl/7.81.0\r\n",
		"Accept: */*\r\n",
		"\r\n",
	}
	output = parseHttpLines(input)
	compareParseHttpLineResults(output, expectedOutput, t)

	input = []string{
		"GET / HTTP/1.1\r\n",
		"Host: ",
		"localhost:1983\r\n",
		"User-Agent: curl",
		"/",
		"7",
		".81.0\r",
		"\n",
		"Accept: */*",
		"\r\n",
		"\r",
		"\n",
	}
	expectedOutput = []string{
		"GET / HTTP/1.1\r\n",
		"Host: localhost:1983\r\n",
		"User-Agent: curl/7.81.0\r\n",
		"Accept: */*\r\n",
		"\r\n",
	}
	output = parseHttpLines(input)
	compareParseHttpLineResults(output, expectedOutput, t)
}

func parseContentLengthHeader(header string) int {
	var contentLength int
	fmt.Sscanf(header, "Content-Length: %d \r\n", &contentLength)
	return contentLength
}

func compareParseContentLengthResults(
	header string,
	contentLength int,
	contentLengthExpected int,
	t *testing.T) {
	if contentLength != contentLengthExpected {
		t.Error("Incorrect Content-Length for header:", header,
			"got:", contentLength, "expected:", contentLengthExpected)
	}
}

func TestParseContentLengthHeader(t *testing.T) {
	var header string
	var contentLength, contentLengthExpected int

	header = "Content-Length: 5\r\n"
	contentLength = parseContentLengthHeader(header)
	contentLengthExpected = 5
	compareParseContentLengthResults(header, contentLength, contentLengthExpected, t)

	header = "Content-Length: 525\r\n"
	contentLength = parseContentLengthHeader(header)
	contentLengthExpected = 525
	compareParseContentLengthResults(header, contentLength, contentLengthExpected, t)

	header = "User-Agent: curl/7.81.0\r\n"
	contentLength = parseContentLengthHeader(header)
	contentLengthExpected = 0
	compareParseContentLengthResults(header, contentLength, contentLengthExpected, t)

	header = "GET / HTTP/1.1"
	contentLength = parseContentLengthHeader(header)
	contentLengthExpected = 0
	compareParseContentLengthResults(header, contentLength, contentLengthExpected, t)
}

func parseNullLine(header string) bool {
	return header == "\r\n"
}

func TestParseNullLine(t *testing.T) {
	if parseNullLine("\r\n") == false {
		t.Error("parseNullLine returned false for '\\r\\n'")
	}
	if parseNullLine("GET / HTTP/1.1\r\n") {
		t.Error("parseNullLine returned true for 'GET / HTTP/1.1\\r\\n'")
	}
}
