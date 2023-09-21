package main

import (
	"flag"
)

func main() {

	domainName := flag.String("domain", "www.google.com", "Domain name to look up.")
	dnsServer := flag.String("server", "8.8.8.8", "DNS server to query.")

	flag.Parse()
	queryDomainNameService(*domainName, *dnsServer)

}
