sudo ./ptf/ptf --test-dir tests/ --pypath sai_thrift_src/gen-py --test-params "port_map_file='port_map.ini'" --interface 0@host_port0 --interface 1@host_port1 --interface 2@host_port2 \
    	--interface 3@host_port3 --interface 4@host_port4 --interface 5@host_port5 --interface 6@host_port6 --interface 7@host_port7  \
    	sail2_new.L21DBridgeBasicTest
    	# sail2_new.L21DLagTest
    	# sail2_new.L2AcceptedFrameType sail2_new.L21DBridgeBasicTest sail2_new.L21QBridgeAccess2AccessTest sail2_new.L21QBridgeAccess2TrunkTest sail2_new.L21QBridgeTrunk2AccessTest  sail2_new.L21QBridgeTrunk2TrunkTest sail2_new.L21DLagTest sail2_new.L21QLagTest
    	# sail2_new.L21QBridgeAccess2AccessTest
    	 