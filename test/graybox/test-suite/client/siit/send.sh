#!/bin/bash


# Arguments:
# $1: List of the names of the test groups you want to run, separated by any
#     character.
#     Example: "udp64, tcp46, icmpe64"
#     If this argument is unspecified, the script will run all the tests.
#     The current groups are:
#     - udp64: IPv6->IPv4 UDP tests
#     - udp46: IPv4->IPv6 UDP tests
#     - tcp64: IPv6->IPv4 TCP tests
#     - icmpi64: IPv6->IPv4 ICMP ping tests
#     - icmpi46: IPv4->IPv6 ICMP ping tests
#     - icmpe64: IPv6->IPv4 ICMP error tests
#     - icmpe46: IPv4->IPv6 ICMP error tests
#     - misc: random tests we've designed later.
#     (Feel free to add new groups if you want.)


GRAYBOX=`dirname $0`/../../../usr/graybox
# When there's no fragmentation, Jool is supposed to randomize the
# fragment ID (bytes 4 and 5) so we can't validate it.
# The ID's randomization cascades to the checksum. (Bytes 10 and
# 11.)
NOFRAG_IGNORE=4,5,10,11

function test-single {
	$GRAYBOX expect add `dirname $0`/pktgen/receiver/$2.pkt $3
	$GRAYBOX send `dirname $0`/pktgen/sender/$1.pkt
	sleep 0.1
	$GRAYBOX expect flush
}

function test-frag {
	$GRAYBOX expect add `dirname $0`/pktgen/receiver/$2.pkt
	$GRAYBOX expect add `dirname $0`/pktgen/receiver/$3.pkt
	$GRAYBOX send `dirname $0`/pktgen/sender/$1.pkt
	sleep 0.1
	$GRAYBOX expect flush
}

function test-manual {
	$GRAYBOX expect add `dirname $0`/manual/$1-expected.pkt $2
	$GRAYBOX send `dirname $0`/manual/$1-test.pkt
	sleep 0.1
	$GRAYBOX expect flush
}


`dirname $0`/../wait.sh 2001:db8:1c6:3364:2::
if [ $? -ne 0 ]; then
	exit 1
fi

echo "Testing! Please wait..."


# UDP, 6 -> 4
if [[ -z $1 || $1 = *udp64* ]]; then
	test-single 6-udp-csumok-df-nofrag 4-udp-csumok-df-nofrag $NOFRAG_IGNORE
	test-single 6-udp-csumok-nodf-nofrag 4-udp-csumok-nodf-nofrag
	test-single 6-udp-csumok-nodf-frag0 4-udp-csumok-nodf-frag0
	test-single 6-udp-csumok-nodf-frag1 4-udp-csumok-nodf-frag1
	test-single 6-udp-csumok-nodf-frag2 4-udp-csumok-nodf-frag2

	test-single 6-udp-csumfail-df-nofrag 4-udp-csumfail-df-nofrag $NOFRAG_IGNORE
	test-single 6-udp-csumfail-nodf-nofrag 4-udp-csumfail-nodf-nofrag
	test-single 6-udp-csumfail-nodf-frag0 4-udp-csumfail-nodf-frag0
	test-single 6-udp-csumfail-nodf-frag1 4-udp-csumfail-nodf-frag1
	test-single 6-udp-csumfail-nodf-frag2 4-udp-csumfail-nodf-frag2
fi

# UDP, 4 -> 6
if [[ -z $1 || $1 = *udp46* ]]; then
	test-single 4-udp-csumok-df-nofrag 6-udp-csumok-df-nofrag
	test-single 4-udp-csumok-nodf-nofrag 6-udp-csumok-nodf-nofrag
	test-single 4-udp-csumok-nodf-frag0 6-udp-csumok-nodf-frag0
	test-single 4-udp-csumok-nodf-frag1 6-udp-csumok-nodf-frag1
	test-single 4-udp-csumok-nodf-frag2 6-udp-csumok-nodf-frag2

	test-single 4-udp-csumfail-df-nofrag 6-udp-csumfail-df-nofrag
	test-single 4-udp-csumfail-nodf-nofrag 6-udp-csumfail-nodf-nofrag
	test-single 4-udp-csumfail-nodf-frag0 6-udp-csumfail-nodf-frag0
	test-single 4-udp-csumfail-nodf-frag1 6-udp-csumfail-nodf-frag1
	test-single 4-udp-csumfail-nodf-frag2 6-udp-csumfail-nodf-frag2
fi

# TCP, 6 -> 4
if [[ -z $1 || $1 = *tcp64* ]]; then
	test-single 6-tcp-csumok-df-nofrag 4-tcp-csumok-df-nofrag $NOFRAG_IGNORE
	test-single 6-tcp-csumok-nodf-nofrag 4-tcp-csumok-nodf-nofrag
	test-single 6-tcp-csumok-nodf-frag0 4-tcp-csumok-nodf-frag0
	test-single 6-tcp-csumok-nodf-frag1 4-tcp-csumok-nodf-frag1
	test-single 6-tcp-csumok-nodf-frag2 4-tcp-csumok-nodf-frag2

	test-single 6-tcp-csumfail-df-nofrag 4-tcp-csumfail-df-nofrag $NOFRAG_IGNORE
	test-single 6-tcp-csumfail-nodf-nofrag 4-tcp-csumfail-nodf-nofrag
	test-single 6-tcp-csumfail-nodf-frag0 4-tcp-csumfail-nodf-frag0
	test-single 6-tcp-csumfail-nodf-frag1 4-tcp-csumfail-nodf-frag1
	test-single 6-tcp-csumfail-nodf-frag2 4-tcp-csumfail-nodf-frag2
fi

# ICMP info, 6 -> 4
if [[ -z $1 || $1 = *icmpi64* ]]; then
	test-single 6-icmp6info-csumok-df-nofrag 4-icmp4info-csumok-df-nofrag $NOFRAG_IGNORE
	test-single 6-icmp6info-csumok-nodf-nofrag 4-icmp4info-csumok-nodf-nofrag

	test-single 6-icmp6info-csumfail-df-nofrag 4-icmp4info-csumfail-df-nofrag $NOFRAG_IGNORE
	test-single 6-icmp6info-csumfail-nodf-nofrag 4-icmp4info-csumfail-nodf-nofrag
fi

# ICMP info, 4 -> 6
if [[ -z $1 || $1 = *icmpi46* ]]; then
	test-single 4-icmp4info-csumok-df-nofrag 6-icmp6info-csumok-df-nofrag
	test-single 4-icmp4info-csumok-nodf-nofrag 6-icmp6info-csumok-nodf-nofrag

	test-single 4-icmp4info-csumfail-df-nofrag 6-icmp6info-csumfail-df-nofrag
	test-single 4-icmp4info-csumfail-nodf-nofrag 6-icmp6info-csumfail-nodf-nofrag
fi

# ICMP error, 6 -> 4
if [[ -z $1 || $1 = *icmpe64* ]]; then
	# 4,5 = frag id. Jool has to assign something random, hence we don't expect anything.
	# 10,11 = IPv4 csum. Inherits frag id's randomness.
	# 22,23 = ICMP csum. Inherits the followind fields' randomness.
	# 32,33 = inner frag id. Same as above.
	# 34 = inner DF. An atomic fragments free Jool has no way to know the DF of the original packet.
	# 38,39 = inner IPv4 csum. Inherits other field's randomness.
	test-single 6-icmp6err-csumok-df-nofrag 4-icmp4err-csumok-df-nofrag 4,5,10,11,22,23,32,33,34,38,39
	# This one doesn't have ignored bytes because DF and IDs have to be inferred from the fragment headers.
	test-single 6-icmp6err-csumok-nodf-nofrag 4-icmp4err-csumok-nodf-nofrag
fi

# ICMP error, 4 -> 6
if [[ -z $1 || $1 = *icmpe46* ]]; then
	test-single 4-icmp4err-csumok-df-nofrag 6-icmp6err-csumok-df-nofrag
	test-single 4-icmp4err-csumok-nodf-nofrag 6-icmp6err-csumok-nodf-nofrag
fi

# Miscellaneous tests
if [[ -z $1 || $1 = *misc* ]]; then
	test-manual igmp64 4,5,10,11
	test-manual igmp46
	test-manual 6791 4,5,10,11,32,33,38,39
	# TODO mangle the packet size so this doesn't have so many exceptions.
	test-single frag-icmp6 frag-icmp6 4,5,6,10,11,22,23,32,33,34,38,39,54,55
	test-single frag-icmp4 frag-icmp4
	#test-single frag-minmtu6-big frag-minmtu6-big0 frag-minmtu6-big1
fi

function test() {
	$GRAYBOX expect add `dirname $0`/$1/$2.pkt $4
	$GRAYBOX send `dirname $0`/$1/$3.pkt
	sleep 0.1
	$GRAYBOX expect flush
}

function test12() {
	$GRAYBOX expect add `dirname $0`/$1/$2.pkt $5
	$GRAYBOX expect add `dirname $0`/$1/$3.pkt $5
	$GRAYBOX send `dirname $0`/$1/$4.pkt
	sleep 0.1
	$GRAYBOX expect flush
}

# When Linux creates an ICMPv4 error on behalf of Jool, it writes 'c0' on the
# outer TOS field for me. This seems to mean "Network Control" messages
# according to DSCP, which is probably fair. Since TOS 0 would also be correct,
# we'll just accept whatever.
TOS=1
# The translated IPv4 identification is always random, so it should be always
# ignored during validation. Unfortunately, of course, the header checksum is
# also affected.
IDENTIFICATION=4,5,10,11
INNER_IDENTIFICATION=32,33,38,39

if [[ -z $1 || $1 = *rfc7915* ]]; then
	# a
	test 7915 aae1 aat1
	test 7915 aae1 aat2
	test 7915 aae1 aat3
	test 7915 abe1 abt1 $IDENTIFICATION
	test 7915 abe1 abt2 $IDENTIFICATION
	test 7915 abe2 abt3
	test 7915 ace1 act1
	test 7915 ace2 act2
	test 7915 ace3 act3
	test 7915 ace4 act4
	test 7915 ace5 act5
	test 7915 ace6 act6

	# b
	test 7915 bae1 bat1
	test 7915 bbe1 bbt1
	test 7915 bce1 bct1
	test 7915 bde1 bdt1 $IDENTIFICATION,$INNER_IDENTIFICATION
	test 7915 bee1 bet1 $IDENTIFICATION,$INNER_IDENTIFICATION
	
	# c
	ip netns exec joolns ip link set dev to_world_v6 mtu 1280
	test 7915 cae1 cat1 $TOS,$IDENTIFICATION
	test 7915 cbe1 cbt1
	# Implementation quirk: The RFC wants us to copy the IPv4 identification
	# value (16 bits) to the IPv6 identification field (32 bits).
	# However, fragmentation is done by the kernel after the translation,
	# which means Jool does not get to decide the identification value.
	#
	# I think this is fine because identification preservation is only
	# relevant when the packet is already fragmented. As a matter of fact,
	# it's better if the kernel decides the identification because it will
	# generate a 32 bit number, and not be constrained to 16 bits like Jool.
	#
	# Identification preservation for already fragmented packets is tested
	# in cf.
	test12 7915 cce1 cce2 cct1 44,45,46,47
	test 7915 cde1 cdt1
	test 7915 cee1 cet1
	test 7915 cee1 cet2
	# TODO Nontrivial bug detected here.
	#test12 7915 cfe1 cfe2 cft1
	#test12 7915 cfe1 cfe2 cft2
	ip netns exec joolns ip link set dev to_world_v6 mtu 1500

	ip netns exec joolns ip link set dev to_world_v4 mtu 1400
	test 7915 cge1 cgt1
	ip netns exec joolns ip link set dev to_world_v4 mtu 1500

	test 7915 che1 cht1 $IDENTIFICATION
	test 7915 cie1 cit1 $IDENTIFICATION

	# d
	test 7915 dae1 dat1 $IDENTIFICATION,$INNER_IDENTIFICATION
	test 7915 dbe1 dbt1
	test 7915 dbe2 dbt2
	test 7915 dbe2 dbt3

	# e
	test 7915 eae1 eat1 $TOS,$IDENTIFICATION
	ip netns exec joolns jool_siit global update amend-udp-checksum-zero 1
	test 7915 ebe1 eat1
	test 7915 ece1 ect1 $TOS,$IDENTIFICATION
	ip netns exec joolns jool_siit global update amend-udp-checksum-zero 0
	test 7915 ece1 ect1 $TOS,$IDENTIFICATION

	# f
	test 7915 fae1 fat1
	test 7915 fbe1 fbt1 $IDENTIFICATION

	# g
	test 7915 gae1 gat1
fi

$GRAYBOX stats display
result=$?
$GRAYBOX stats flush

exit $result
