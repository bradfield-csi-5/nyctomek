package main

import (
	"bytes"
	"fmt"
	"testing"
)

func TestEncodeDNSHeader(t *testing.T) {
	header := DNSHeader{
		ID:               1000,
		isQuery:          true,
		recursionDesired: true,
		authenticateData: true,
		questionCount:    1,
	}
	var encodedHeader []byte = encodeDNSHeader(&header)
	const expectedHeader = "\x03\xe8\x01\x20\x00\x01\x00\x00\x00\x00\x00\x00"

	if !bytes.Equal(encodedHeader, []byte(expectedHeader)) {
		fmt.Println("Actual encoding: ", encodedHeader)
		fmt.Println("Expected encoding: ", expectedHeader)
		t.Error(`encodeDNSHeader returned incorrect encoding.`)
	}
}

func TestDecodeDNSHeader(t *testing.T) {
	encodedHeader := []byte("\x03\xe8\x01\x20\x00\x01\x00\x00\x00\x00\x00\x00")
	header := decodeDNSHeader(encodedHeader)
	expectedHeader := DNSHeader{
		ID:               1000,
		isQuery:          true,
		recursionDesired: true,
		authenticateData: true,
		questionCount:    1,
	}
	if header != expectedHeader {
		t.Error(`decodeDNSHeader() returned incorrect header, `,
			`\nExpected=`, expectedHeader, `\nActual=`, header, `.`)
	}
}

func TestEncodeDNSQuestion(t *testing.T) {

	question := DNSQuestion{
		domainName:   "www.google.com",
		questionType: QUESTION_TYPE_A,
		classType:    CLASS_TYPE_IN,
	}
	const expectedQuestion = "\x03\x77\x77\x77\x06\x67\x6f\x6f\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01"

	encodedQuestion := encodeDNSQuestion(&question)

	if !bytes.Equal(encodedQuestion, []byte(expectedQuestion)) {
		t.Error("encodeDNSQuestion() returned incorrect encoding.",
			"\nExpected: ", []byte(expectedQuestion),
			"\nActual: ", encodedQuestion,
			".")
	}
}

func TestDecodeDNSQuestion(t *testing.T) {

	encodedQuestion := []byte("\x03\x77\x77\x77\x06\x67\x6f\x6f\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01")

	decodedQuestionExpected := DNSQuestion{
		domainName:   "www.google.com",
		questionType: QUESTION_TYPE_A,
		classType:    CLASS_TYPE_IN,
	}

	var bytesDecoded int
	decodedQuestionActual := decodeDNSQuestion(encodedQuestion, &bytesDecoded)

	if decodedQuestionExpected != decodedQuestionActual {
		t.Error("decodeDNSQuestion returned incorrect question.",
			"\nExpected: ", decodedQuestionExpected,
			"\nActual: ", decodedQuestionActual, ".")
	}
}

func TestDecodeResourceRecord(t *testing.T) {

	encodedResponse := []byte("\x03\xe8\x81\x80\x00\x01\x00\x01\x00\x00\x00\x01\x03\x77\x77\x77\x06\x67\x6f\x6f\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x19\x00\x04\x8e\xfa\x41\xe4\x00\x00\x29\x02\x00\x00\x00\x00\x00\x00\x00")
	encodedResourceRecord := []byte("\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x19\x00\x04\x8e\xfa\x41\xe4\xc0\x0c\x00\x01\x00\x01\x00\x00\x00\x19\x00\x04\x8e\xfa\x41\xe4")

	decodedResourceRecordExpected := DNSResourceRecord{
		domainName:          "www.google.com",
		resourceRecordType:  1,
		resourceRecordClass: 1,
		TTL:                 25,
		resourceDataLength:  4,
		resourceData:        "142.250.65.228",
	}

	var bytesDecoded int
	decodedResourceRecordActual := decodeResourceRecord(encodedResponse, encodedResourceRecord, 32, &bytesDecoded)

	if decodedResourceRecordExpected != decodedResourceRecordActual {
		t.Error("decodeResourceRecord() returned incorrect resource record.",
			"\nExpected: ", decodedResourceRecordExpected,
			"\nActual: ", decodedResourceRecordActual, ".")
	}
}

func TestDecodeDomainFacebook(t *testing.T) {
	encodedResponse := []byte("\x03\xe8\x81\x80\x00\x01\x00\x02\x00\x00\x00\x00\x03\x77\x77\x77\x08\x66\x61\x63\x65\x62\x6f\x6f\x6b\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00\x05\x00\x01\x00\x00\x04\x47\x00\x11\x09\x73\x74\x61\x72\x2d\x6d\x69\x6e\x69\x04\x63\x31\x30\x72\xc0\x10\xc0\x2e\x00\x01\x00\x01\x00\x00\x00\x0e\x00\x04\x9d\xf0\xf1\x23")
	startingPosition := 46
	domainName, bytesDecoded := decodeDomain(encodedResponse, startingPosition)

	domainNameExpected := "star-mini.c10r.facebook.com"
	bytesDecodedExpected := 17

	if domainName != domainNameExpected || bytesDecoded != bytesDecodedExpected {
		t.Error("decodeDomain() returned unexpected domain or bytes decoded.",
			"\nExpected: domainName:", domainNameExpected,
			"bytesDecoded:", bytesDecodedExpected,
			"\nActual: domainName:", domainName,
			"bytesDecoded:", bytesDecoded)
	}

}
