# http://seclist.us/simple-network-packet-sniffer-written-in-python.html

import socket, sys, time, platform, struct
from PyQt4 import QtGui, QtCore

class SniffThread(QtCore.QThread):
	def eth(self, packet, extractedAttIndex, printKey):
		# Header lengths.
		ethHeaderLength = 14

		# Get Ethernet header using begin and end.
		# No need for windows calibration, Ethernet support under Linux only.
		begin = 0
		end = begin + ethHeaderLength
		ethHeader = packet[begin:end]

		# Unpack the header because it originally in hex.
		# The regular expression helps unpack the header.
		# ! signifies we are unpacking a network endian.
		# 6s signifies we are unpacking a string of size 6 bytes.
		# H signifies we are unpacking an integer of size 2 bytes.
		ethHeaderUnpacked = struct.unpack('!6s6sH', ethHeader)
			
		# The first 6s is 6 bytes and contains the destination address.
		ethDestAddress = ethHeaderUnpacked[0]
		
		# The second 6s is 6 bytes and contains the source address.
		ethSourceAddress = ethHeaderUnpacked[1]
		
		# The first H is 2 bytes and contains the packet length.
		ethType = socket.ntohs(ethHeaderUnpacked[2])
		
		# Properly unpack and format the destination address.
		ethDestAddress = '%.2x:%.2x:%.2x:%.2x:%.2x:%.2x' % (ord(ethDestAddress[0]), ord(ethDestAddress[1]), ord(ethDestAddress[2]), ord(ethDestAddress[3]), ord(ethDestAddress[4]), ord(ethDestAddress[5]))
		
		# Properly unpack and format the source address.
		ethSourceAddress = '%.2x:%.2x:%.2x:%.2x:%.2x:%.2x' % (ord(ethSourceAddress[0]), ord(ethSourceAddress[1]), ord(ethSourceAddress[2]), ord(ethSourceAddress[3]), ord(ethSourceAddress[4]), ord(ethSourceAddress[5]))
		
		# Check if the print key is 0.
		# If true, header information will be printed.
		# 	Check if the user selected extracted attribute index is 0.
		#	If true, all attributes will be printed.
		#	If false, the attribute the user selected extracted attribute index corresponds to will be printed.
		# If false, the attribute the user selected attribute index corresponds to will be returned.
		if printKey == 0:
			# Print Ethernet Header
			self.unpackedInfo.append('\n********************\n** Ethernet (MAC) **\n********************')
			
			if (extractedAttIndex == 1) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Destination Address: ' + str(ethDestAddress))
			if (extractedAttIndex == 2) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Source Address: ' + str(ethSourceAddress))
			if (extractedAttIndex == 3) or (extractedAttIndex == 0):
					self.unpackedInfo.append('EtherType: ' + str(ethType))
		else:
			if (extractedAttIndex == 1):
				return str(ethDestAddress)
			if (extractedAttIndex == 2):
				return str(ethSourceAddress)
			if (extractedAttIndex == 3):
				return str(ethType)
		
	def arp(self, packet, extractedAttIndex, printKey):
		# Header lengths.
		ethHeaderLength = 14
		arpHeaderLength = 28
		
		# Get ARP header using begin and end.
		begin = ethHeaderLength
		end = begin + arpHeaderLength
		arpHeader = packet[begin:end]
		
		# Unpack the header because it originally in hex.
		# The regular expression helps unpack the header.
		# ! signifies we are unpacking a network endian.
		# H signifies we are unpacking an integer of size 2 bytes.
		# B signifies we are unpacking an integer of size 1 byte.
		# 6s signifies we are unpacking a string of size 6 bytes.
		# 4s signifies we are unpacking a string of size 4 bytes.
		arpHeaderUnpacked = struct.unpack('!HHBBH6s4s6s4s', arpHeader)
		
		# The first H is 2 bytes and contains the hardware type.
		arpHardwareType = socket.ntohs(arpHeaderUnpacked[0])
		
		# The second H is 2 bytes and contains the protocol type.
		arpProtocolType = socket.ntohs(arpHeaderUnpacked[1])
		
		# The first B is 1 byte and contains the hardware address length.
		arpHardAddressLength = arpHeaderUnpacked[2]
		
		# The second B is 1 byte and contains the protocol address length.
		arpProtAddressLength = arpHeaderUnpacked[3]
		
		# The third H is 2 bytes and contains the operation.
		arpOperation = arpHeaderUnpacked[4]

		# The first 6s is 6 bytes and contains the sender hardware address.
		arpSenderHardAddress = arpHeaderUnpacked[5]
		
		# The first 4s is 4 bytes and contains the sender protocol address.
		arpSenderProtAddress = socket.inet_ntoa(arpHeaderUnpacked[6])
		
		# The second 6s is 6 bytes and contains the target hardware address.
		arpTargetHardAddress = arpHeaderUnpacked[7]
		
		# The second 4s is 4 bytes and contains the target protocol address.
		arpTargetProtAddress = socket.inet_ntoa(arpHeaderUnpacked[8])
		
		# Properly unpack and format the source MAC address.
		arpSenderHardAddress = '%.2x:%.2x:%.2x:%.2x:%.2x:%.2x' % (ord(arpSenderHardAddress[0]), ord(arpSenderHardAddress[1]), ord(arpSenderHardAddress[2]), ord(arpSenderHardAddress[3]), ord(arpSenderHardAddress[4]), ord(arpSenderHardAddress[5]))
		
		# Properly unpack and format the destination MAC address.
		arpTargetHardAddress = '%.2x:%.2x:%.2x:%.2x:%.2x:%.2x' % (ord(arpTargetHardAddress[0]), ord(arpTargetHardAddress[1]), ord(arpTargetHardAddress[2]), ord(arpTargetHardAddress[3]), ord(arpTargetHardAddress[4]), ord(arpTargetHardAddress[5]))
		
		# Check if the print key is 0.
		# If true, header information will be printed.
		# 	Check if the user selected extracted attribute index is 0.
		#	If true, all attributes will be printed.
		#	If false, the attribute the user selected extracted attribute index corresponds to will be printed.
		# If false, the attribute the user selected attribute index corresponds to will be returned.
		if printKey == 0:
			# Print ARP Header
			self.unpackedInfo.append('\n*******************\n******* ARP *******\n*******************')
			
			if (extractedAttIndex == 1) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Hardware Type: ' + str(arpHardwareType))
			if (extractedAttIndex == 2) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Protocol Type: ' + str(arpProtocolType))
			if (extractedAttIndex == 3) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Hardware Address Length: ' + str(arpHardAddressLength))
			if (extractedAttIndex == 4) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Protocol Address Length: ' + str(arpProtAddressLength))
			if (extractedAttIndex == 5) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Operation: ' + str(arpOperation))
			if (extractedAttIndex == 6) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Sender Hardware Address: ' + str(arpSenderHardAddress))
			if (extractedAttIndex == 7) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Sender Protocol Address: ' + str(arpSenderProtAddress))
			if (extractedAttIndex == 8) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Target Hardware Address: ' + str(arpTargetHardAddress))
			if (extractedAttIndex == 9) or (extractedAttIndex == 0):	
				self.unpackedInfo.append('Target Protocol Address: ' + str(arpTargetProtAddress))
				
			# Separator	
			self.unpackedInfo.append('\n----------------------------------------')	
		else:
			if (extractedAttIndex == 1):
				return str(arpHardwareType)
			if (extractedAttIndex == 2):	
				return str(arpProtocolType)
			if (extractedAttIndex == 3):	
				return str(arpHardAddressLength)
			if (extractedAttIndex == 4):	
				return str(arpProtAddressLength)
			if (extractedAttIndex == 5):	
				return str(arpOperation)
			if (extractedAttIndex == 6):	
				return str(arpSenderHardAddress)
			if (extractedAttIndex == 7):	
				return str(arpSenderProtAddress)
			if (extractedAttIndex == 8):	
				return str(arpTargetHardAddress)
			if (extractedAttIndex == 9):	
				return str(arpTargetProtAddress)

	def ip(self, packet, extractedAttIndex, printKey):
		# Header lengths.
		ethHeaderLength = 14
		ipHeaderLength = 20
		
		# Get IP header using begin and end.
		# Specific Linux and Windows calibration is needed.
		if self.os == self.linux:	
			begin = ethHeaderLength
			end = begin + ipHeaderLength
		elif self.os == self.windows:
			begin = 0
			end = begin + ipHeaderLength
		ipHeader = packet[begin:end]

		# Unpack the header because it originally in hex.
		# The regular expression helps unpack the header.
		# ! signifies we are unpacking a network endian.
		# B signifies we are unpacking an integer of size 1 byte.
		# H signifies we are unpacking an integer of size 2 bytes.
		# 4s signifies we are unpacking a string of size 4 bytes.
		ipHeaderUnpacked = struct.unpack('!BBHHHBBH4s4s' , ipHeader)
		
		# The first B is 1 byte and contains the version and header length.
		# Both are 4 bits each, split ipHeaderUnpacked[0] in "half".
		ipVersionAndHeaderLength = ipHeaderUnpacked[0]
		ipVersion = ipVersionAndHeaderLength >> 4
		ipHeaderLength = ipVersionAndHeaderLength & 0xF

		# The second B is 1 byte and contains the service type and ECN.
		ipDSCPAndECN = ipHeaderUnpacked[1]
		ipDSCP = ipDSCPAndECN >> 2
		ipECN = ipDSCPAndECN & 0x3

		# The first H is 2 bytes and contains the total length.
		ipTotalLength = ipHeaderUnpacked[2]
		
		# The second H is 2 bytes and contains the total length.
		ipIdentification = ipHeaderUnpacked[3]

		# The third H is 2 bytes and contains the flags and fragment offset.
		# Flags is 3 bits and fragment offset is 13 bits.
		# Split ipHeaderUnpacked[4].
		ipFlagsAndFragmentOffset = ipHeaderUnpacked[4]
		ipFlags = ipFlagsAndFragmentOffset >> 13
		ipFragmentOffset = ipFlagsAndFragmentOffset & 0x1FFF

		# The third B is 1 byte and contains the time to live.
		ipTimeToLive = ipHeaderUnpacked[5]
			
		# Our fourth B is 1 byte and contains the protocol.
		ipProtocol = ipHeaderUnpacked[6]
		
		# The fourth H is 2 bytes and contains the header checksum.
		ipHeaderChecksum = ipHeaderUnpacked[7]

		# The first 4s is 4 bytes and contains the source address.
		ipSourceAddress = socket.inet_ntoa(ipHeaderUnpacked[8]);

		# The second 4s is 4 bytes and contains the dest address.
		ipDestAddress = socket.inet_ntoa(ipHeaderUnpacked[9]);

		# Check if the print key is 0.
		# If true, header information will be printed.
		# 	Check if the user selected extracted attribute index is 0.
		#	If true, all attributes will be printed.
		#	If false, the attribute the user selected extracted attribute index corresponds to will be printed.
		# If false, the attribute the user selected attribute index corresponds to will be returned.
		if printKey == 0:
			# Print IP Header
			# Some segments of the header are switched back to hex form because that
			# 	is the format wireshark has it.
			self.unpackedInfo.append('\n********************\n******** IP ********\n********************')
			
			if (extractedAttIndex == 1) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Version: ' + str(ipVersion))
			if (extractedAttIndex == 2) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Header Length: ' + str(ipHeaderLength) + ' 32-bit words')
			if (extractedAttIndex == 3) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Differentiated Services Code Point: ' + format(ipDSCP, '#04X') + ' , ' + str(ipDSCP))
			if (extractedAttIndex == 4) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Explicit Congestion Notification: ' + format(ipECN, '#04X') + ' , ' + str(ipECN))
			if (extractedAttIndex == 5) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Total Length: ' + str(ipTotalLength) + ' bytes')
			if (extractedAttIndex == 6) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Identification: ' + format(ipIdentification, '#04X') + ' , ' + str(ipIdentification))
			if (extractedAttIndex == 7) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Flags: ' + format(ipFlags, '#04X') + ' , ' + str(ipFlags))
			if (extractedAttIndex == 8) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Fragment Offset: ' + str(ipFragmentOffset) + ' eight-byte blocks')
			if (extractedAttIndex == 9) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Time to Live: ' + str(ipTimeToLive) + ' hops')
			if (extractedAttIndex == 10) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Protocol: ' + str(ipProtocol))
			if (extractedAttIndex == 11) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Header Checksum: ' + format(ipHeaderChecksum, '#04X'))
			if (extractedAttIndex == 12) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Source Address: ' + str(ipSourceAddress))
			if (extractedAttIndex == 13) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Destination Address: ' + str(ipDestAddress))
		else:
			if (extractedAttIndex == 1):
				return str(ipVersion)
			if (extractedAttIndex == 2):
				return str(ipHeaderLength)
			if (extractedAttIndex == 3):
				return format(ipDSCP, '#04X')
			if (extractedAttIndex == 4):
				return format(ipECN, '#04X')
			if (extractedAttIndex == 5):
				return str(ipTotalLength)
			if (extractedAttIndex == 6):
				return format(ipIdentification, '#04X')
			if (extractedAttIndex == 7):
				return format(ipFlags, '#04X')
			if (extractedAttIndex == 8):
				return str(ipFragmentOffset)
			if (extractedAttIndex == 9):
				return str(ipTimeToLive)
			if (extractedAttIndex == 10):
				return str(ipProtocol)
			if (extractedAttIndex == 11):
				return format(ipHeaderChecksum, '#04X')
			if (extractedAttIndex == 12):
				return str(ipSourceAddress)
			if (extractedAttIndex == 13):
				return str(ipDestAddress)
				
	def icmp(self, packet, extractedAttIndex, printKey):
		# Header lengths.
		ethHeaderLength = 14
		ipHeaderLength = 20
		icmpHeaderLength = 8
		
		# Get ICMP header using begin and end.
		# Specific Linux and Windows calibration is needed.
		if self.os == self.linux:
			begin = ethHeaderLength + ipHeaderLength
			end = begin + icmpHeaderLength
		elif self.os == self.windows:
			begin = ipHeaderLength
			end = begin + icmpHeaderLength
		icmpHeader = packet[begin:end]

		# Unpack the header because it originally in hex.
		# The regular expression helps unpack the header.
		# ! signifies we are unpacking a network endian.
		# B signifies we are unpacking an integer of size 1 byte.
		# H signifies we are unpacking an integer of size 2 bytes.
		# L signifies we are unpacking a long of size 4 bytes.
		icmpHeaderUnpacked = struct.unpack('!BBHL', icmpHeader)

		# The first B is 1 byte and contains the type.
		icmpType = icmpHeaderUnpacked[0]

		# The second B is 1 byte and contains the code.
		icmpCode = icmpHeaderUnpacked[1]

		# The first H is 2 bytes and contains the checksum.
		icmpChecksum = icmpHeaderUnpacked[2]

		# Check if the type is 1 or 8, if so, unpack the identifier and sequence number.
		if (icmpType == 0) or (icmpType == 8):
			# The first L is 4 bytes and contains the rest of the header.
			icmpIdentifier = icmpHeaderUnpacked[3] >> 16
			icmpSeqNumber = icmpHeaderUnpacked[3] & 0xFFFF
		
		# Check if the print key is 0.
		# If true, header information will be printed.
		# 	Check if the user selected extracted attribute index is 0.
		#	If true, all attributes will be printed.
		#	If false, the attribute the user selected extracted attribute index corresponds to will be printed.
		# If false, the attribute the user selected attribute index corresponds to will be returned.
		if printKey == 0:
			if (icmpType == 0) or (icmpType == 8):
				# Print ICMP Header
				# Some segments of the header are switched back to hex form because that
				# 	is the format wireshark has it.
				self.unpackedInfo.append('\n********************\n******* ICMP *******\n********************')
				
				if (extractedAttIndex == 1) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Type: ' + str(icmpType))
				if (extractedAttIndex == 2) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Code: ' + str(icmpCode))
				if (extractedAttIndex == 3) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Checksum: ' + format(icmpChecksum, '#04X'))
				if (extractedAttIndex == 4) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Identifier: ' + str(icmpIdentifier))
				if (extractedAttIndex == 5) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Sequence Number: ' + str(icmpSeqNumber))
			else:
				self.unpackedInfo.append('\n********************\n******* ICMP *******\n********************')
				
				if (extractedAttIndex == 1) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Type: ' + str(icmpType))
				if (extractedAttIndex == 2) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Code: ' + str(icmpCode))
				if (extractedAttIndex == 3) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Checksum: ' + format(icmpChecksum, '#04X'))
				if (extractedAttIndex == 4) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Attribute not available.')
				if (extractedAttIndex == 5) or (extractedAttIndex == 0):
					self.unpackedInfo.append('Attribute not available.')
					
			# Separator	
			self.unpackedInfo.append('\n----------------------------------------')
		else:
			if (icmpType == 0) or (icmpType == 8):
				if (extractedAttIndex == 1):
					return str(icmpType)
				if (extractedAttIndex == 2):
					return str(icmpCode)
				if (extractedAttIndex == 3):
					return format(icmpChecksum, '#04X')
				if (extractedAttIndex == 4):
					return str(icmpIdentifier)
				if (extractedAttIndex == 5):
					return str(icmpSeqNumber)
			else:			
				if (extractedAttIndex == 1):
					return str(icmpType)
				if (extractedAttIndex == 2):
					return str(icmpCode)
				if (extractedAttIndex == 3):
					return format(icmpChecksum, '#04X')
				if (extractedAttIndex == 4):
					return 'Attribute not available.'
				if (extractedAttIndex == 5):
					return 'Attribute not available.'
		
	def tcp(self, packet, extractedAttIndex, printKey):
		# Header lengths.
		ethHeaderLength = 14
		ipHeaderLength = 20
		tcpHeaderLength = 20

		# Get TCP header using begin and end.
		# Specific Linux and Windows calibration is needed.
		if self.os == self.linux:
			begin = ethHeaderLength + ipHeaderLength
			end = begin + tcpHeaderLength
		elif self.os == self.windows:
			begin = ipHeaderLength
			end = begin + tcpHeaderLength
		tcpHeader = packet[begin:end]

		# Unpack the header because it originally in hex.
		# The regular expression helps unpack the header.
		# ! signifies we are unpacking a network endian.
		# H signifies we are unpacking an integer of size 2 bytes.
		# L signifies we are unpacking a long of size 4 bytes.
		# B signifies we are unpacking an integer of size 1 byte.
		tcpHeaderUnpacked = struct.unpack('!HHLLBBHHH', tcpHeader)
		
		# The first H is 2 bytes and contains the source port.
		tcpSourcePort = tcpHeaderUnpacked[0]
		
		# The second H is 2 bytes and contains the destination port.
		tcpDestPort = tcpHeaderUnpacked[1]

		# The first L is 2 bytes and contains the sequence number.
		tcpSeqNumber = tcpHeaderUnpacked[2]
		
		# The second L is 4 bytes and contains the acknowledgement number.
		tcpAckNumber = tcpHeaderUnpacked[3]
		
		# The first B is 1 byte and contains the data offset, reserved bits, and NS flag.
		# Split tcpHeaderUnpacked[4]
		tcpDataOffsetAndReserved = tcpHeaderUnpacked[4]
		tcpDataOffset = tcpDataOffsetAndReserved >> 4
		tcpReserved = (tcpDataOffsetAndReserved >> 1) & 0x7
		tcpNSFlag = tcpDataOffsetAndReserved & 0x1
		
		# The second B is 1 byte and contains the rest of the flags.
		# Split tcpHeaderUnpacked[5].
		tcpRestOfFLags = tcpHeaderUnpacked[5]
		tcpCWRFlag = tcpRestOfFLags >> 7
		tcpECEFlag = (tcpRestOfFLags >> 6) & 0x1
		tcpURGFlag = (tcpRestOfFLags >> 5) & 0x1
		tcpACKFlag = (tcpRestOfFLags >> 4) & 0x1
		tcpPSHFlag = (tcpRestOfFLags >> 3) & 0x1
		tcpRSTFlag = (tcpRestOfFLags >> 2) & 0x1
		tcpSYNFlag = (tcpRestOfFLags >> 1) & 0x1
		tcpFINFlag = tcpRestOfFLags & 0x1
		
		# The third H is 2 bytes and contains the window size.
		tcpWindowSize = tcpHeaderUnpacked[6]
		
		# The fourth H is 2 byte and conntains the checksum.
		tcpChecksum = tcpHeaderUnpacked[7]
		
		# The fifth H is 2 bytes and constains the urgent pointer.
		tcpUrgentPointer = tcpHeaderUnpacked[8]
		
		# Check if the print key is 0.
		# If true, header information will be printed.
		# 	Check if the user selected extracted attribute index is 0.
		#	If true, all attributes will be printed.
		#	If false, the attribute the user selected extracted attribute index corresponds to will be printed.
		# If false, the attribute the user selected attribute index corresponds to will be returned.
		if printKey == 0:
			# Print TCP Header
			# Some segments of the header are switched back to hex form because that
			# 	is the format wireshark has it.
			self.unpackedInfo.append('\n*******************\n******* TCP *******\n*******************')
		
			if (extractedAttIndex == 1) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Source Port: ' + str(tcpSourcePort))
			if (extractedAttIndex == 2) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Destination Port: ' + str(tcpDestPort))
			if (extractedAttIndex == 3) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Sequence Number: ' + str(tcpSeqNumber))
			if (extractedAttIndex == 4) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Acknowledgment Number: ' + str(tcpAckNumber))
			if (extractedAttIndex == 5) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Data Offset: ' + str(tcpDataOffset) + ' 32-bit words')
			if (extractedAttIndex == 6) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Reserved: ' + format(tcpReserved, '03b') + '. .... ....')
			if (extractedAttIndex == 7) or (extractedAttIndex == 0):
				self.unpackedInfo.append('NS Flag:  ' + '...' + format(tcpNSFlag, '01b') + ' .... ....')
			if (extractedAttIndex == 8) or (extractedAttIndex == 0):
				self.unpackedInfo.append('CWR Flag: ' + '.... ' + format(tcpCWRFlag, '01b') + '... ....')
			if (extractedAttIndex == 9) or (extractedAttIndex == 0):
				self.unpackedInfo.append('ECE Flag: ' + '.... .' + format(tcpECEFlag, '01b') + '.. ....')
			if (extractedAttIndex == 10) or (extractedAttIndex == 0):
				self.unpackedInfo.append('URG Flag: ' + '.... ..' + format(tcpURGFlag, '01b') + '. ....')
			if (extractedAttIndex == 11) or (extractedAttIndex == 0):
				self.unpackedInfo.append('ACK Flag: ' + '.... ...' + format(tcpACKFlag, '01b') + ' ....')
			if (extractedAttIndex == 12) or (extractedAttIndex == 0):
				self.unpackedInfo.append('PSH Flag: ' + '.... .... ' + format(tcpPSHFlag, '01b') + '...')
			if (extractedAttIndex == 13) or (extractedAttIndex == 0):
				self.unpackedInfo.append('RST Flag: ' + '.... .... .' + format(tcpRSTFlag, '01b') + '..')
			if (extractedAttIndex == 14) or (extractedAttIndex == 0):
				self.unpackedInfo.append('SYN Flag: ' + '.... .... ..' + format(tcpSYNFlag, '01b') + '.')
			if (extractedAttIndex == 15) or (extractedAttIndex == 0):
				self.unpackedInfo.append('FIN Flag: ' + '.... .... ...' + format(tcpFINFlag, '01b'))
			if (extractedAttIndex == 16) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Window Size: ' + str(tcpWindowSize) + ' bytes')
			if (extractedAttIndex == 17) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Urgent Pointer: ' + str(tcpUrgentPointer))
			if (extractedAttIndex == 18) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Checksum: ' + format(tcpChecksum, '#04X'))
		
			# Separator	
			self.unpackedInfo.append('\n----------------------------------------')	
		else:
			if (extractedAttIndex == 1):
				return str(tcpSourcePort)
			if (extractedAttIndex == 2):
				return str(tcpDestPort)
			if (extractedAttIndex == 3):
				return str(tcpSeqNumber)
			if (extractedAttIndex == 4):
				return str(tcpAckNumber)
			if (extractedAttIndex == 5):
				return str(tcpDataOffset)
			if (extractedAttIndex == 6):
				return format(tcpReserved, '03b')
			if (extractedAttIndex == 7):
				return format(tcpNSFlag, '01b')
			if (extractedAttIndex == 8):
				return format(tcpCWRFlag, '01b')
			if (extractedAttIndex == 9):
				return format(tcpECEFlag, '01b')
			if (extractedAttIndex == 10):
				return format(tcpURGFlag, '01b')
			if (extractedAttIndex == 11):
				return format(tcpACKFlag, '01b')
			if (extractedAttIndex == 12):
				return format(tcpPSHFlag, '01b')
			if (extractedAttIndex == 13):
				return format(tcpRSTFlag, '01b')
			if (extractedAttIndex == 14):
				return format(tcpSYNFlag, '01b')
			if (extractedAttIndex == 15):
				return format(tcpFINFlag, '01b')
			if (extractedAttIndex == 16):
				return str(tcpWindowSize)
			if (extractedAttIndex == 17):
				return str(tcpUrgentPointer)
			if (extractedAttIndex == 18):
				return format(tcpChecksum, '#04X')

	def udp(self, packet, extractedAttIndex, printKey):
		# Header lengths.
		ethHeaderLength = 14
		ipHeaderLength = 20
		udpHeaderLength = 8
		
		# Get UDP header using begin and end.
		# Specific Linux and Windows calibration is needed.
		if self.os == self.linux:
			begin = ethHeaderLength + ipHeaderLength
			end = begin + udpHeaderLength
		elif self.os == self.windows:
			begin = ipHeaderLength
			end = begin + udpHeaderLength
		udpHeader = packet[begin:end]
	
		# Unpack the header because it originally in hex.
		# The regular expression helps unpack the header.
		# ! signifies we are unpacking a network endian.
		# H signifies we are unpacking an integer of size 2 bytes.
		udpHeaderUnpacked = struct.unpack('!HHHH', udpHeader)
		 
		# The first H is 2 bytes and contains the source port.
		udpSourcePort = udpHeaderUnpacked[0]
		
		# The second H is 2 bytes and contains the destination port.
		udpDestPort = udpHeaderUnpacked[1]
		
		# The third H is 2 bytes and contains the packet length.
		udpLength = udpHeaderUnpacked[2]
		
		# The fourth H is 2 bytes and contains the header checksum.
		udpChecksum = udpHeaderUnpacked[3]
		
		# Check if the print key is 0.
		# If true, header information will be printed.
		# 	Check if the user selected extracted attribute index is 0.
		#	If true, all attributes will be printed.
		#	If false, the attribute the user selected extracted attribute index corresponds to will be printed.
		# If false, the attribute the user selected attribute index corresponds to will be returned.
		if printKey == 0:
			# Print UDP Header
			self.unpackedInfo.append('\n*******************\n******* UDP *******\n*******************')
			
			if (extractedAttIndex == 1) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Source Port: ' + str(udpSourcePort))
			if (extractedAttIndex == 2) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Destination Port: ' + str(udpDestPort))
			if (extractedAttIndex == 3) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Length: ' + str(udpLength) + ' bytes')
			if (extractedAttIndex == 4) or (extractedAttIndex == 0):
				self.unpackedInfo.append('Checksum: ' + format(udpChecksum, '#04X'))

			# Separator	
			self.unpackedInfo.append('\n----------------------------------------')	
		else:
			if (extractedAttIndex == 1):
				return str(udpSourcePort)
			if (extractedAttIndex == 2):
				return str(udpDestPort)
			if (extractedAttIndex == 3):
				return str(udpLength)
			if (extractedAttIndex == 4):
				return format(udpChecksum, '#04X')
				
	def findProtocol(self, packet):
		# Will hold the packet protocol.
		packetProtocol = ''
		
		# If the OS is Linux, unpack Ethernet's protocol.
		# If the OS is Windows, mimic unpacking Ethernet's protocol.
		if self.os == self.linux:
			ethProtocol = self.eth(packet, 3, 1)
			ethProtocol = int(ethProtocol)
		elif self.os == self.windows:
			ethProtocol = 8

		# Find if the Ethernet protocol is ARP or IP.
		# If the protocol is 1544, meaning ARP, then set packetProtocol to 0.
		# If the protocol is 8, meaning IP, find the protocol within IP.
		if ethProtocol == 1544:
			packetProtocol = 1
		elif ethProtocol == 8:
			# Unpack IP's protocol.
			ipProtocol = self.ip(packet, 10, 1)
			ipProtocol = int(ipProtocol)
			
			# If the protocol is 1, meaning ICMP, then set packetProtocol to 2 (Linux) or 1 (Windows).
			# If the protocol is 6, meaning TCP, then set packetProtocol to 3 (Linux) or 2 (Windows).
			# If the protocol is 17, meaning UDP, then set packetProtocol to 4 (Linux) or 3 (Windows).
			if self.os == self.linux:
				if ipProtocol == 1:
					packetProtocol = 2
				elif ipProtocol == 6:
					packetProtocol = 3
				elif ipProtocol == 17:
					packetProtocol = 4
			elif self.os == self.windows:
				if ipProtocol == 1:
					packetProtocol = 1
				elif ipProtocol == 6:
					packetProtocol = 2
				elif ipProtocol == 17:
					packetProtocol = 3
				
		# Return the packet protocol.
		return packetProtocol		
		
	def extractAllAtt(self, packet):
		# All attributes for each protocol will be displayed.
		extractedAttIndex = 0
		
		# Attributes will be printed.
		printKey = 0
		
		# If the OS is Linux, unpack Ethernet's protocol.
		# If the OS is Windows, mimic unpacking Ethernet's protocol.
		if self.os == self.linux:
			# Unpack the Ethernet (MAC) information.
			self.eth(packet, extractedAttIndex, printKey)
		
			# Find the packet's Ethernet protocol.
			ethProtocol = self.eth(packet, 3, 1)
			ethProtocol = int(ethProtocol)
		elif self.os == self.windows:
			ethProtocol = 8

		# Find if the Ethernet protocol is ARP or IP.
		if ethProtocol == 1544:
			# Unpack the ARP information.
			self.arp(packet, extractedAttIndex, printKey)
		elif ethProtocol == 8:
			# Unpack IP's information.
			self.ip(packet, extractedAttIndex, printKey)
			
			# Find the packet's IP protocol.
			ipProtocol = self.ip(packet, 10, 1)
			ipProtocol = int(ipProtocol)
			
			# If the protocol is 1, meaning ICMP, then unpack the ICMP information.
			# If the protocol is 6, meaning TCP, then unpack the TCP information.
			# If the protocol is 17, meaning UDP, then unpack the UDP information.
			if ipProtocol == 1:
				self.icmp(packet, extractedAttIndex, printKey)
			elif ipProtocol == 6:
				self.tcp(packet, extractedAttIndex, printKey)
			elif ipProtocol == 17:
				self.udp(packet, extractedAttIndex, printKey)

	def filterAndExtract(self, packet, filteredProtocolIndex, extractedAttIndex):
		# Get the protocol index of the packet.
		protocolIndex = self.findProtocol(packet)
		
		if self.os == self.linux:
			if (filteredProtocolIndex == protocolIndex) or (filteredProtocolIndex == 0):
				# Attributes will be printed.
				printKey = 0

				# Find the user selected filtered protocol index.
				if filteredProtocolIndex == 0:
					if extractedAttIndex >= 1:
						self.eth(packet, extractedAttIndex, printKey)

						# Separator
						self.unpackedInfo.append('\n----------------------------------------')
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
				elif filteredProtocolIndex == 1:
					# The user selected extracted attribute index will be calibrated (if needed) to specify which attribute to extract.
					if extractedAttIndex >= 4:
						self.arp(packet, extractedAttIndex - 3, printKey)
					elif extractedAttIndex >= 1:
						self.eth(packet, extractedAttIndex, printKey)

						# Separator
						self.unpackedInfo.append('\n----------------------------------------')
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
				elif filteredProtocolIndex == 2:
					if extractedAttIndex >= 17:
						self.icmp(packet, extractedAttIndex - 16, printKey)
					elif extractedAttIndex >= 4:
						self.ip(packet, extractedAttIndex - 3, printKey)

						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex >= 1:	
						self.eth(packet, extractedAttIndex, printKey)

						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
				elif filteredProtocolIndex == 3:
					if extractedAttIndex >= 17:	
						self.tcp(packet, extractedAttIndex - 16, printKey)
					elif extractedAttIndex >= 4:
						self.ip(packet, extractedAttIndex - 3, printKey)

						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex >= 1:	
						self.eth(packet, extractedAttIndex, printKey)

						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
				elif filteredProtocolIndex == 4:
					if extractedAttIndex >= 17:	
						self.udp(packet, extractedAttIndex - 16, printKey)
					elif extractedAttIndex >= 4:
						self.ip(packet, extractedAttIndex - 3, printKey)

						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex >= 1:	
						self.eth(packet, extractedAttIndex, printKey)

						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
						
				return 0
			else:
				return 1
		elif self.os == self.windows:
			if (filteredProtocolIndex == protocolIndex) or (filteredProtocolIndex == 0):
				# Attributes will be printed.
				printKey = 0
				
				# Find the user selected filtered protocol index.
				if filteredProtocolIndex == 0:
					if extractedAttIndex >= 1:
						self.ip(packet, extractedAttIndex, printKey)

						# Separator
						self.unpackedInfo.append('\n----------------------------------------')
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
				elif filteredProtocolIndex == 1:
					# The user selected extracted attribute index will be calibrated (if needed) to specify which attribute to extract.
					if extractedAttIndex >= 14:	
						self.icmp(packet, extractedAttIndex - 13, printKey)
					elif extractedAttIndex >= 1:
						self.ip(packet, extractedAttIndex, printKey)
							
						# Separator	
						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
				elif filteredProtocolIndex == 2:
					if extractedAttIndex >= 14:	
						self.tcp(packet, extractedAttIndex - 13, printKey)
					elif extractedAttIndex >= 1:
						self.ip(packet, extractedAttIndex, printKey)
							
						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
				elif filteredProtocolIndex == 3:
					if extractedAttIndex >= 14:	
						self.udp(packet, extractedAttIndex - 13, printKey)
					elif extractedAttIndex >= 1:
						self.ip(packet, extractedAttIndex, printKey)
							
						self.unpackedInfo.append('\n----------------------------------------')	
					elif extractedAttIndex == 0:
						self.extractAllAtt(packet)
									
				return 0
			else:
				return 1

	def findMaxDiameter(self):
		# Find the maximum diameter of all the packets.
		maxDiameter = max(self.diameterList)
		
		# Print the maximum diameter.
		self.calculationList.append('Max Diameter: ' + str(maxDiameter) + ' hops')
		
	def findMaxPacketLength(self):
		# Find the maximum packet length of all the packets.
		maxLength = max(self.lengthList)

		# Print the maximum packet length.
		self.calculationList.append('Max Packet Length: ' + str(maxLength) + ' bytes')

	def findAvgDiameter(self):
		# Hold the sum and the count of the diameters.
		diameterSum = 0
		count = 0
		avgDiameter = 0
		
		# Add all of the diameters together.
		for diameter in self.diameterList:
			diameterSum = diameterSum + diameter
			count = count + 1
			
		# Divide diameterSum by count to give average.
		avgDiameter = diameterSum / count
		self.calculationList.append('Avg Diameter: ' + str(avgDiameter) + ' hops')

	def findAvgPacketLength(self):
		# Hold the sum and the count of the packet lengths.
		lengthSum = 0
		count = 0
		avgLength = 0
		
		# Add all of the lengths together.
		for length in self.lengthList:
			lengthSum = lengthSum + length
			count = count + 1

		# Divide lengthSum by count to give average.
		avgLength = lengthSum / count
		self.calculationList.append('Avg Packet Length: ' + str(avgLength) + ' bytes')

	def prepareCalculationData(self, packet):
		# If the OS is Linux, unpack Ethernet's protocol.
		# If the OS is Windows, mimic unpacking Ethernet's protocol.
		if self.os == self.linux:
			# Find the packet's Ethernet protocol.
			ethProtocol = self.eth(packet, 3, 1)
			ethProtocol = int(ethProtocol)
		elif self.os == self.windows:
			ethProtocol = 8

		# Find if the Ethernet protocol is IP.
		# If the protocol is 8, meaning IP, find the diameter and find the protocol within IP.
		if ethProtocol == 8:
			# Append the IP total length to the length list.
			# Append the diameters to the diameter list using TTL.
			# These will be used for calculations.
			ipTotalLength = self.ip(packet, 5, 1)
			ipTotalLength = int(ipTotalLength)
			self.lengthList.append(ipTotalLength)
				
			# Find the diameter of the network.
			# Different servers have different operating systems that have different TTLs.
			# Cisco is 255, Windows is 128, Linux is 64.
			ipTimeToLive = self.ip(packet, 9, 1)
			ipTimeToLive = int(ipTimeToLive)
			
			if ipTimeToLive > 128:
				self.diameterList.append(255 - ipTimeToLive)
			elif ipTimeToLive > 64:
				self.diameterList.append(128 - ipTimeToLive)
			else:
				self.diameterList.append(64 - ipTimeToLive)
				
			return 0
		else:
			return 1
				
	def calculateData(self):				
		# Check if the diameter list and the length list are not empty.
		#	If true, perform various  calculations.
		self.findMaxDiameter()
		self.findMaxPacketLength()
		self.findAvgDiameter()
		self.findAvgPacketLength()
			
	def stop(self):
		# Disable promiscuous mode under Windows.
		if self.os == self.windows:
			self.sock.ioctl(socket.SIO_RCVALL, socket.RCVALL_OFF)
  
		# Close the socket.
		self.sock.close()
			
		# Terminate the thread.
		self.terminate()
		
	def close():
		try:
			# Exit the application.
			print('Goodbye.')
			time.sleep(1)
			sys.exit()
		except KeyboardInterrupt:
			sys.exit()

	def sniff(self, filteredProtocolIndex, extractedAttIndex):
		# Sniff packets. Will loop until user presses Ctrl+c.
		while True:
			# Recieve the packets in the network.
			# Packet will be a tuple, use the first element in the tuple.
			packet = self.sock.recvfrom(65565)
			packet = packet[0]				
			
			# Delete the data inside the lists containing unpacked info and calculations.
			# This is needed as these lists can contain data about a current packet being sniffed.
			del self.unpackedInfo[:]
			del self.calculationList[:]
			
			# Filter and extract packet info using packet, filteredProtocolIndex and extractedAttIndex.
			# Save the packet for other operations.
			filterAndExtract = self.filterAndExtract(packet, filteredProtocolIndex, extractedAttIndex)

			# Check if the user selected filtered protocol index is supported.
			# If true, send the main thread the unpacked info.
			if filterAndExtract == 0:
				for i in range(len(self.unpackedInfo)):
					self.emit(QtCore.SIGNAL('updatePackets(QString)'), self.unpackedInfo[i])
					
				prepareCalculationData = self.prepareCalculationData(packet)

				if prepareCalculationData == 0:
					self.calculateData()
					self.emit(QtCore.SIGNAL('updateMaxDiameter(QString)'), self.calculationList[0])
					self.emit(QtCore.SIGNAL('updateMaxPacketLength(QString)'), self.calculationList[1])
					self.emit(QtCore.SIGNAL('updateAvgDiameter(QString)'), self.calculationList[2])
					self.emit(QtCore.SIGNAL('updateAvgPacketLength(QString)'), self.calculationList[3])
						
	def __init__(self, filteredProtocolIndex, extractedAttIndex):
		QtCore.QThread.__init__(self)
		
		# Set the recieved user selected filtered protocol index and user selected extracted att index to local variables.
		self.filteredProtocolIndex = filteredProtocolIndex
		self.extractedAttIndex = extractedAttIndex
		
		# Special lists used for calculations.
		self.lengthList = []
		self.diameterList = []
		
		# Special lists used to hold unpacked info and calculation results.
		self.unpackedInfo = []
		self.calculationList = []

		# Check the OS the application is running on.
		self.os = platform.system()
		self.linux = 'Linux'
		self.windows = 'Windows'
		
		try:
			# If Linux, set up the raw socket the Linux way.
			# If Windows, set up the raw socket the Windows way.
			if self.os == self.linux:
				# Create the raw socket.
				self.sock = socket.socket(socket.AF_PACKET , socket.SOCK_RAW , socket.ntohs(0x0003))
			elif self.os == self.windows:
				# The public network interface.
				HOST = socket.gethostbyname(socket.gethostname())

				# Create a raw socket and bind it to the public interface.
				self.sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
				self.sock.bind((HOST, 0))

				# Include IP headers
				self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)

				# Receive all packages.
				self.sock.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)
		except socket.error, msg:
			print('Socket could not be created. \nError code: ' + str(msg[0]) + '\nMessage: ' + msg[1])
			close()

	def __del__(self):
		self.wait()

	def run(self):
		self.sniff(self.filteredProtocolIndex, self.extractedAttIndex)

class Gui(QtGui.QWidget):	
	def startSniff(self):
		# Check to see if sniffing is 1 (currently not sniffing).
		# If true, create a thread to perform the sniffing.
		if self.sniffKey == 1:
			# Set sniffing to 0 (currently sniffing).
			# Set the sniffing label to sniffing in order to notify user.
			self.sniffKey = 0
			self.sniffingLabel.setText('Sniffing...')
			
			# Save the user selected filtered protocol index.
			# Save the user selected extracted attribute index.
			filteredProtocolIndex = self.protocolComboBox.currentIndex()
			extractedAttIndex = self.attComboBox.currentIndex()
			
			# Create a sniff thread to perform sniffing and create different signals for all the data sent back and forth.
			# Data sent back and forth:
			#	Unpack packet info
			#	max diameter and max packet length
			#	avg diameter and avg packet length
			self.sniffThread = SniffThread(filteredProtocolIndex, extractedAttIndex)
			self.connect(self.sniffThread, QtCore.SIGNAL('updatePackets(QString)'), self.updatePackets)
			self.connect(self.sniffThread, QtCore.SIGNAL('updateMaxDiameter(QString)'), self.updateMaxDiameter)
			self.connect(self.sniffThread, QtCore.SIGNAL('updateMaxPacketLength(QString)'), self.updateMaxPacketLength)
			self.connect(self.sniffThread, QtCore.SIGNAL('updateAvgDiameter(QString)'), self.updateAvgDiameter)
			self.connect(self.sniffThread, QtCore.SIGNAL('updateAvgPacketLength(QString)'), self.updateAvgPacketLength)

			# Start the sniff thread.
			self.sniffThread.start()
		
	def stopSniff(self):
		# Check to see if sniffing is 1 (currently sniffing).
		# If true, stop the sniffing.
		if self.sniffKey == 0:
			# Set sniffing to 0 (currently sniffing).
			# Set the sniffing label to sniffing in order to notify user.
			self.sniffKey = 1
			self.sniffingLabel.setText('Not sniffing.')
			
			# Call the stop function from the sniff thread.
			self.sniffThread.stop()
		
	def updatePackets(self, unpackedInfo):
		# Append the packet edit text with the unpacked info.
		self.packetEditText.append(unpackedInfo)
		
	def updateMaxDiameter(self, maxDiameter):
		# Append the max diameter label with the max diameter.
		self.maxDiameterLabel.setText(maxDiameter)
	
	def updateMaxPacketLength(self, maxLength):
		# Append the max length label with the max length.
		self.maxLengthLabel.setText(maxLength)
		
	def updateAvgDiameter(self, avgDiameter):
		# Append the avg diameter label with the avg diameter.
		self.avgDiameterLabel.setText(avgDiameter)
		
	def updateAvgPacketLength(self, avgLength):
		# Append the avg length label with the avg length.
		self.avgLengthLabel.setText(avgLength)
			
	def updateAtts(self, protocol):	
		# Establish the attribute combo box content for each protocol's attributes.
		allAttributes = ['All']
		ethAttributes = ['Destination Address', 'Source Address', 'EtherType']
		arpAttributes = ['Hardware Type', 'Protocol Type', 'Hardware Address Length', 'Protocol Address Length', 'Operation', 'Sender Hardware Address', 'Sender Protocol Address', 'Target Hardware Address', 'Target Protocol Address']
		ipAttributes = ['Version', 'Header Length', 'Differentiated Services Code Point', 'Explicit Congestion Notification', 'Total Length', 'Identification', 'Flags', 'Fragment Offset', 'Time to Live', 'Protocol', 'Header Checksum', 'Source Address', 'Destination Address']
		icmpAttributes = ['Type', 'Code', 'Checksum', 'Identifier (If available)', 'Sequence Number (If available)']
		tcpAttributes = ['Source Port', 'Destination Port', 'Sequence Number', 'Acknowledgment Number', 'Data Offset', 'Reserved', 'NS Flag', 'CWR Flag', 'ECE Flag', 'URG Flag', 'ACK Flag', 'PSH Flag', 'RST Flag', 'SYN Flag', 'FIN Flag', 'Window Size', 'Urgent Pointer', 'Checksum']
		udpAttributes = ['Source Port', 'Destination Port', 'Length', 'Checksum']
		
		# Clear the previous attribute combo box content.
		self.attComboBox.clear()
		
		if self.os == self.linux:
			# All protocols will have All and Etherenet attributes.
			attributes = allAttributes + ethAttributes

			# Find the user selected filtered protocol index.
			# Concatenate the corresponding attributes of the user selected filtered protocol to attributes.
			if protocol == 'ARP':
				attributes += arpAttributes
			elif protocol == 'ICMP':
				attributes += ipAttributes + icmpAttributes
			elif protocol == 'TCP':
				attributes += ipAttributes + tcpAttributes
			elif protocol == 'UDP':
				attributes += ipAttributes + udpAttributes
				
			# Insert the new attributes to attribute combo box.
			self.attComboBox.insertItems(len(attributes), attributes)
		elif self.os == self.windows:
			attributes = allAttributes + ipAttributes
			
			if protocol == 'ICMP':
				attributes += icmpAttributes
			elif protocol == 'TCP':
				attributes += tcpAttributes
			elif protocol == 'UDP':
				attributes += udpAttributes
				
			self.attComboBox.insertItems(len(attributes), attributes)
			
		# Check if the application is sniffing.
		# If true, stop the sniffing.
		if self.sniffKey == 0:
			self.stopSniff()

	def initGUI(self):
		# Establish the attribute combo box content for each protocol's attributes.
		if self.os == self.linux:
			protocols = ['All', 'ARP', 'ICMP', 'TCP', 'UDP']
		elif self.os == self.windows:
			protocols = ['All', 'ICMP', 'TCP', 'UDP']
		
		# Set the window title and size.
		self.setWindowTitle('Packet Sniffer')
		self.resize(500, 425)

		# Prepare the grid layout.
		grid = QtGui.QGridLayout()
		grid.setSpacing(10)

		# Create the start and stop button.
		# These start and stop the sniffing.
		startButton = QtGui.QPushButton('Start')
		stopButton = QtGui.QPushButton('Stop')
		
		# Create the protocol label, protocol combobox, and insert the protocols in the protocol combobox.
		# These let the user select a protocol.
		protocolLabel = QtGui.QLabel('Protocols:')
		self.protocolComboBox = QtGui.QComboBox(self)
		self.protocolComboBox.insertItems(len(protocols), protocols)

		# Create the attribute label, attribute combobox, and run updateAtts.
		# updateAtts inserts the selected protocol's corresponding attributes to the attribute combobox.
		# These let the user select an attribute.
		attLabel = QtGui.QLabel('Attributes:')
		self.attComboBox = QtGui.QComboBox(self)
		self.updateAtts('All')

		# Create the packet label and the packet edit text label.
		# These display the packet information to the user.
		packetLabel = QtGui.QLabel('Packets:')
		self.packetEditText = QtGui.QTextEdit()
		self.packetEditText.setReadOnly(True)
		
		# Create the calculation label, the max diameter, max length label, avg diameter, and avg length label.
		# These let the user know important calculations about the network.
		calculationLabel = QtGui.QLabel('Calculations:')
		self.maxDiameterLabel = QtGui.QLabel('Max Diameter: ---- hops')
		self.maxLengthLabel = QtGui.QLabel('Max Packet Length: ---- bytes')
		self.avgDiameterLabel = QtGui.QLabel('Avg Diameter: ---- hops')
		self.avgLengthLabel = QtGui.QLabel('Avg Packet Length: ---- bytes')
		
		# Create the sniffing label.
		# This lets the user know what the application is doing.
		self.sniffingLabel = QtGui.QLabel('Not sniffing.')

		# Add the various buttons, labels, comboboxes, and edit texts to the grid.
		grid.addWidget(startButton, 1, 0, 1, 2)
		grid.addWidget(stopButton, 2, 0, 1, 2)

		grid.addWidget(protocolLabel, 3, 0, 1, 2)
		grid.addWidget(self.protocolComboBox, 4, 0, 1, 2)
		
		grid.addWidget(attLabel, 5, 0, 1, 2)
		grid.addWidget(self.attComboBox, 6, 0, 1, 2)

		grid.addWidget(packetLabel, 7, 0)
		grid.addWidget(self.packetEditText, 8, 0, 15, 1)
		
		grid.addWidget(calculationLabel, 7, 1)
		
		grid.addWidget(self.maxDiameterLabel, 8, 1)
		grid.addWidget(self.maxLengthLabel, 9, 1)
		
		grid.addWidget(self.avgDiameterLabel, 10, 1)
		grid.addWidget(self.avgLengthLabel, 11, 1)
		
		grid.addWidget(self.sniffingLabel, 24, 0, 1, 2)
		
		# Prepare the start button, stop button, and protocol combo box signals.
		# The protocol combo box signal calls updateAtts, which updates the attribute combobox with corresponding attributes. 
		startButton.clicked.connect(self.startSniff)
		stopButton.clicked.connect(self.stopSniff)
		self.connect(self.protocolComboBox, QtCore.SIGNAL('activated(QString)'), self.updateAtts)

		# Set the layout and show the window.
		self.setLayout(grid) 
		self.show()

	def __init__(self):
		super(Gui, self).__init__()
				
		# Check the OS the application is running on.
		self.os = platform.system()
		self.linux = 'Linux'
		self.windows = 'Windows'	
		
		# Simple key that lets the application know whether it is sniffing.
		# 1 = not sniffing.
		# 0 = sniffing.
		self.sniffKey = 1
		
		# Initilize the GUI.
		self.initGUI()

def main():
	# Find the OS the application is running on.
	os = platform.system()
	linux = 'Linux'
	windows = 'Windows'	

	# Check if the application is running under a supported OS.
	# If true, run the rest of the application.
	# If no true, notify the user their OS is not supported.
	if (os == linux) or (os == windows):
		app = QtGui.QApplication(sys.argv)
		gui = Gui()
		sys.exit(app.exec_())
	else:
		print('The OS you are running is not supported.')

if __name__ == '__main__':
    main()