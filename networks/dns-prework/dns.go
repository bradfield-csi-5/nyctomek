package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"net"
	"slices"
	"strconv"
	"strings"
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

type ResourceRecord struct {
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

func decodeDomainName(buffer []byte) string {
	index := slices.Index(buffer, 0)
	if index > 0 {
		sliceWithDomainName := make([]byte, index-1)
		copy(sliceWithDomainName, buffer[1:index])
		for i, v := range sliceWithDomainName {
			if v < 64 {
				sliceWithDomainName[i] = '.'
			}
		}
		return string(sliceWithDomainName[:])
	}
	return ""
}

func decodeDNSQuestion(questionBuffer []byte, bytesDecoded *int) DNSQuestion {
	var question DNSQuestion
	question.domainName = decodeDomainName(questionBuffer)
	typeIndex := len(question.domainName) + 2
	question.questionType = QuestionType(binary.BigEndian.Uint16(questionBuffer[typeIndex:]))
	question.classType = ClassType(binary.BigEndian.Uint16(questionBuffer[typeIndex+2:]))
	*bytesDecoded = typeIndex + 4
	return question
}

func decodeResourceRecord(responseBuffer []byte, resourceRecordBuffer []byte, bytesDecoded *int) ResourceRecord {
	var resourceRecord ResourceRecord
	typeIndex := 0
	if (resourceRecordBuffer[0] & 0x80) != 0 {
		domainNameLocation := resourceRecordBuffer[1]
		resourceRecord.domainName = decodeDomainName(responseBuffer[domainNameLocation:])
		typeIndex = 2
	} else {
		resourceRecord.domainName = decodeDomainName(resourceRecordBuffer)
		typeIndex = len(resourceRecord.domainName) + 2
	}
	resourceRecord.resourceRecordType = ResourceRecordType(
		binary.BigEndian.Uint16(resourceRecordBuffer[typeIndex:]))
	resourceRecord.resourceRecordClass = ResourceRecordClass(
		binary.BigEndian.Uint16(resourceRecordBuffer[typeIndex+2:]))
	resourceRecord.TTL =
		binary.BigEndian.Uint32(resourceRecordBuffer[typeIndex+4:])
	resourceRecord.resourceDataLength =
		binary.BigEndian.Uint16(resourceRecordBuffer[typeIndex+8:])

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
	}
	*bytesDecoded = typeIndex + 14
	return resourceRecord
}

func queryDomainNameService(domainName string, dnsServer string) int32 {

	con, err := net.Dial("udp", dnsServer+":53")

	checkErr(err)

	defer con.Close()

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

	_, err = con.Write([]byte(requestBuffer))

	checkErr(err)

	reply := make([]byte, 1024)

	_, err = con.Read(reply)

	responseHeader := decodeDNSHeader(reply)
	var bytesDecoded int
	responseQuestion := decodeDNSQuestion(reply[12:], &bytesDecoded)
	resourceRecord := decodeResourceRecord(reply, reply[12+bytesDecoded:], &bytesDecoded)
	fmt.Println("Header=", responseHeader)
	fmt.Println("Question=", responseQuestion)
	fmt.Println("Resource Record=", resourceRecord)

	checkErr(err)

	return 0
}

func checkErr(err error) {

	if err != nil {
		log.Fatal(err)
	}
}
