package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"slices"
	"strconv"
	"strings"

	"golang.org/x/sys/unix"
)

const HEADER_MASK_QUERY_OR_RESPONSE = 1 << 31
const HEADER_MASK_AUTHORITATIVE_ANSWER = 1 << 10
const HEADER_MASK_TRUNCATED = 1 << 9
const HEADER_MASK_RECURSION_DESIRED = 1 << 8
const HEADER_MASK_RECURSION_AVAILABLE = 1 << 7
const HEADER_MASK_AUTHENTICATE_DATA = 1 << 5

type DNSHeader struct {
	ID                     uint16
	isQuery                bool
	opCode                 uint8
	authoritativeAnswer    bool
	truncated              bool
	recursionDesired       bool
	recursionAvailable     bool
	authenticateData       bool
	rcode                  uint8
	questionCount          uint16
	answerCount            uint16
	nameServerCount        uint16
	additionalRecordsCount uint16
}

type QuestionType uint16

const (
	QUESTION_TYPE_A     QuestionType = 1
	QUESTION_TYPE_S     QuestionType = 2
	QUESTION_TYPE_CNAME QuestionType = 5
	QUESTION_TYPE_MX    QuestionType = 15
	QUESTION_TYPE_ANY   QuestionType = 255
)

type ClassType uint16

const (
	CLASS_TYPE_IN  ClassType = 1
	CLASS_TYPE_CH  ClassType = 3
	CLASS_TYPE_HS  ClassType = 4
	CLASS_TYPE_ANY ClassType = 255
)

type DNSQuestion struct {
	domainName   string
	questionType QuestionType
	classType    ClassType
}

type ResourceRecordType uint16

const (
	RESOURCE_RECORD_TYPE_A     ResourceRecordType = 1
	RESOURCE_RECORD_TYPE_S     ResourceRecordType = 2
	RESOURCE_RECORD_TYPE_CNAME ResourceRecordType = 5
	RESOURCE_RECORD_TYPE_MX    ResourceRecordType = 15
)

type ResourceRecordClass uint16

const (
	RESOURCE_RECORD_CLASS_IN ResourceRecordClass = 1
	RESOURCE_RECORD_CLASS_CH ResourceRecordClass = 3
	RESOURCE_RECORD_CLASS_HS ResourceRecordClass = 4
)

type DNSResourceRecord struct {
	domainName          string
	resourceRecordType  ResourceRecordType
	resourceRecordClass ResourceRecordClass
	TTL                 uint32
	resourceDataLength  uint16
	resourceData        string
}

func encodeDNSHeader(header *DNSHeader) []byte {

	var headerBuffer []byte

	flags := uint16(header.opCode) << 11
	if header.recursionDesired {
		flags |= HEADER_MASK_RECURSION_DESIRED
	}
	if header.authenticateData {
		flags |= HEADER_MASK_AUTHENTICATE_DATA
	}

	headerBuffer = binary.BigEndian.AppendUint16(headerBuffer, header.ID)
	headerBuffer = binary.BigEndian.AppendUint16(headerBuffer, flags)
	headerBuffer = binary.BigEndian.AppendUint16(headerBuffer, header.questionCount)
	headerBuffer = binary.BigEndian.AppendUint16(headerBuffer, header.answerCount)
	headerBuffer = binary.BigEndian.AppendUint16(headerBuffer, header.nameServerCount)
	headerBuffer = binary.BigEndian.AppendUint16(headerBuffer, header.additionalRecordsCount)

	return headerBuffer
}

func decodeDNSHeader(headerBuffer []byte) DNSHeader {

	var header DNSHeader

	header.ID = binary.BigEndian.Uint16(headerBuffer[0:])
	flags := binary.BigEndian.Uint16(headerBuffer[2:])
	header.isQuery = ((flags & 0x8000) == 0)
	header.opCode = uint8((flags << 1) >> 11)
	header.authoritativeAnswer = (flags & HEADER_MASK_AUTHORITATIVE_ANSWER) != 0
	header.truncated = (flags & HEADER_MASK_TRUNCATED) != 0
	header.recursionDesired = (flags & HEADER_MASK_RECURSION_DESIRED) != 0
	header.recursionAvailable = (flags & HEADER_MASK_RECURSION_AVAILABLE) != 0
	header.authenticateData = (flags & HEADER_MASK_AUTHENTICATE_DATA) != 0
	header.questionCount = binary.BigEndian.Uint16(headerBuffer[4:])
	header.answerCount = binary.BigEndian.Uint16(headerBuffer[6:])
	header.nameServerCount = binary.BigEndian.Uint16(headerBuffer[8:])

	return header
}

func encodeDNSQuestion(question *DNSQuestion) []byte {

	var questionBuffer []byte

	domainLabels := strings.Split(question.domainName, ".")
	for _, v := range domainLabels {
		questionBuffer = append(questionBuffer, byte(len(v)))
		questionBuffer = append(questionBuffer, v...)
	}
	questionBuffer = append(questionBuffer, byte(0))
	questionBuffer = binary.BigEndian.AppendUint16(questionBuffer, uint16(question.questionType))
	questionBuffer = binary.BigEndian.AppendUint16(questionBuffer, uint16(question.classType))

	return questionBuffer
}

func decompressDomainName(buffer []byte, responseBuffer []byte, bytesDecoded *int) string {
	domainNameBuffer := make([]byte, len(buffer))
	var domainNameSuffix string
	src := 0
	dest := 0
	for ; src < len(buffer); src++ {
		if buffer[src] == 0 {
			src++
			break
		}
		if buffer[src] < 64 {
			if src > 0 || *bytesDecoded > 0 {
				domainNameBuffer[dest] = '.'
				dest++
			}
		} else if (buffer[src] & 0x80) != 0 {
			domainNameLocation := buffer[src+1]

			bytesDecodedTemp := src // HACK: Hint to the recursive call to *not* skip the dot.
			domainNameSuffix = decompressDomainName(responseBuffer[domainNameLocation:], responseBuffer, &bytesDecodedTemp)
			src += 2 // Account for the `c0` & offset byte.
			break
		} else {
			domainNameBuffer[dest] = buffer[src]
			dest++
		}
	}
	*bytesDecoded = src
	return string(domainNameBuffer[:dest]) + domainNameSuffix
}

func decodeDomainName(buffer []byte, skipDot bool) string {
	index := slices.Index(buffer, 0)
	if index > 0 {
		start := 0
		sliceLen := index
		if skipDot {
			start++
			sliceLen--
		}
		sliceWithDomainName := make([]byte, sliceLen)
		copy(sliceWithDomainName, buffer[start:index])
		for i, v := range sliceWithDomainName {
			if v < 64 {
				sliceWithDomainName[i] = '.'
			}
		}
		return string(sliceWithDomainName[:])
	}
	return ""
}

func max(a int, b int) int {
	if a > b {
		return a
	} else {
		return b
	}
}

func decodeDomain(responseBuffer []byte, startPos int) (string, int) {
	var domainName string
	bytesDecoded := 0

	domainBuffer := make([]byte, len(responseBuffer))
	destPos := 0
	currentPos := startPos
	maxPos := startPos

	for responseBuffer[currentPos]&80 != 0 {
		currentPos++
		maxPos = max(maxPos, currentPos)
		currentPos = int(responseBuffer[currentPos])
	}

	numOctets := responseBuffer[currentPos]
	currentPos++

	for responseBuffer[currentPos] != 0 && currentPos < len(responseBuffer) {

		if numOctets > 0 {
			domainBuffer[destPos] = responseBuffer[currentPos]
			numOctets--
		} else if numOctets == 0 {
			if responseBuffer[currentPos]&0x80 != 0 {
				currentPos++
				maxPos = max(maxPos, currentPos)
				currentPos = int(responseBuffer[currentPos])
				continue
			}
			numOctets = responseBuffer[currentPos]
			domainBuffer[destPos] = '.'
		}
		destPos++
		currentPos++
		maxPos = max(maxPos, currentPos)
	}

	domainName = string(domainBuffer[:destPos])
	bytesDecoded = maxPos - startPos + 1

	return domainName, bytesDecoded
}

func decodeDNSQuestion(questionBuffer []byte, bytesDecoded *int) DNSQuestion {
	var question DNSQuestion
	question.domainName = decodeDomainName(questionBuffer, true)
	typeIndex := len(question.domainName) + 2
	question.questionType = QuestionType(binary.BigEndian.Uint16(questionBuffer[typeIndex:]))
	question.classType = ClassType(binary.BigEndian.Uint16(questionBuffer[typeIndex+2:]))
	*bytesDecoded = typeIndex + 4
	return question
}

func decodeResourceRecord(responseBuffer []byte, resourceRecordBuffer []byte, positionInReply int, bytesDecoded *int) DNSResourceRecord {
	var resourceRecord DNSResourceRecord
	typeIndex := 0
	resourceRecord.domainName, typeIndex = decodeDomain(responseBuffer, positionInReply)

	resourceRecord.resourceRecordType = ResourceRecordType(
		binary.BigEndian.Uint16(resourceRecordBuffer[typeIndex:]))
	resourceRecord.resourceRecordClass = ResourceRecordClass(
		binary.BigEndian.Uint16(resourceRecordBuffer[typeIndex+2:]))
	resourceRecord.TTL =
		binary.BigEndian.Uint32(resourceRecordBuffer[typeIndex+4:])
	resourceRecord.resourceDataLength =
		binary.BigEndian.Uint16(resourceRecordBuffer[typeIndex+8:])

	*bytesDecoded = typeIndex + 10

	if resourceRecord.resourceRecordType == RESOURCE_RECORD_TYPE_A &&
		resourceRecord.resourceRecordClass == RESOURCE_RECORD_CLASS_IN {
		var rdData []byte
		rdData = append(rdData, strconv.Itoa(int(resourceRecordBuffer[typeIndex+10]))...)
		rdData = append(rdData, byte('.'))
		rdData = append(rdData, strconv.Itoa(int(resourceRecordBuffer[typeIndex+11]))...)
		rdData = append(rdData, byte('.'))
		rdData = append(rdData, strconv.Itoa(int(resourceRecordBuffer[typeIndex+12]))...)
		rdData = append(rdData, byte('.'))
		rdData = append(rdData, strconv.Itoa(int(resourceRecordBuffer[typeIndex+13]))...)
		resourceRecord.resourceData = string(rdData)
		*bytesDecoded += 4
	} else if resourceRecord.resourceRecordType == RESOURCE_RECORD_TYPE_CNAME {
		resourceData, domainBytesDecoded := decodeDomain(responseBuffer, positionInReply+typeIndex+10)
		resourceRecord.resourceData = resourceData
		*bytesDecoded += domainBytesDecoded

	}
	return resourceRecord
}

func queryDomainNameService(domainName string, dnsServer string) int32 {

	socketFD, err := unix.Socket(unix.AF_INET, unix.SOCK_DGRAM, unix.IPPROTO_IP)
	checkErr(err)

	ipAddress := [4]byte{8, 8, 8, 8}
	serverAddress := &unix.SockaddrInet4{
		Port: 53,
		Addr: ipAddress,
	}

	localIPAddress := [4]byte{192, 168, 0, 135}
	localAddress := &unix.SockaddrInet4{
		Port: 1983,
		Addr: localIPAddress,
	}
	err = unix.Bind(socketFD, localAddress)
	checkErr(err)

	err = unix.Connect(socketFD, serverAddress)
	checkErr(err)

	header := DNSHeader{
		ID:               1000,
		isQuery:          true,
		recursionDesired: true,
		authenticateData: true,
		questionCount:    1,
	}
	question := DNSQuestion{
		domainName:   domainName,
		questionType: QUESTION_TYPE_A,
		classType:    CLASS_TYPE_IN,
	}

	requestBuffer := append(encodeDNSHeader(&header), encodeDNSQuestion(&question)...)

	err = unix.Sendmsg(socketFD, []byte(requestBuffer), nil, serverAddress, unix.MSG_DONTWAIT)

	checkErr(err)

	reply := make([]byte, 1024)

	_, _, err = unix.Recvfrom(socketFD, reply, 0)

	checkErr(err)

	responseHeader := decodeDNSHeader(reply)

	fmt.Printf("\nHEADER\n")
	printHeader(&responseHeader)

	positionInReply := 12
	bytesDecoded := 0
	for i := 0; i < int(responseHeader.questionCount); i++ {
		responseQuestion := decodeDNSQuestion(reply[positionInReply:], &bytesDecoded)
		positionInReply += bytesDecoded
		fmt.Printf("\nQUESTION %d\n", i+1)
		printQuestionRecord(&responseQuestion)
	}

	for i := 0; i < int(responseHeader.answerCount); i++ {
		resourceRecord := decodeResourceRecord(reply, reply[positionInReply:], positionInReply, &bytesDecoded)
		positionInReply += bytesDecoded
		fmt.Printf("\nANSWER %d\n", i+1)
		printResourceRecord(&resourceRecord)
	}

	unix.Close(socketFD)
	return 0
}

func printHeader(header *DNSHeader) {

	fmt.Printf("ID: %d, isQuery: %t, isResponse: %t, opCode: %d, rcode: %d\n",
		header.ID, header.isQuery, !header.isQuery, header.opCode, header.rcode)

	fmt.Printf("Truncated: %t, Recursion Desired: %t, "+
		"Recursion Available: %t, Authenticate Data: %t\n",
		header.truncated, header.recursionDesired,
		header.recursionAvailable, header.authenticateData)

	fmt.Printf("Authoritative: %t, Questions: %d, Answers: %d, "+
		"Name Server Records: %d, Additional Records: %d.\n",
		header.authoritativeAnswer, header.questionCount,
		header.answerCount, header.nameServerCount,
		header.additionalRecordsCount)
}

func printQuestionRecord(question *DNSQuestion) {
	fmt.Printf("Domain Name: %s\nType: %s\nClass: %d\n",
		question.domainName, DNSTypeToString(uint16(question.questionType)), question.classType)
}

func printResourceRecord(resourceRecord *DNSResourceRecord) {
	fmt.Printf("Domain Name: %s\nType: %s\nClass: %d\nTTL: %d\nData Length: %d\nData: %s\n",
		resourceRecord.domainName, DNSTypeToString(uint16(resourceRecord.resourceRecordType)),
		resourceRecord.resourceRecordClass, resourceRecord.TTL,
		resourceRecord.resourceDataLength, resourceRecord.resourceData)
}

func DNSTypeToString(dnsType uint16) string {
	switch dnsType {
	case 1:
		return "A"
	case 2:
		return "NS"
	case 5:
		return "CNAME"
	case 6:
		return "SOA"
	default:
		return "UNKNOWN"
	}
}

func checkErr(err error) {

	if err != nil {
		log.Fatal(err)
	}
}
