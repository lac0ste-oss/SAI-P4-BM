sudo ./ptf/ptf --test-dir shabi_test/ --pypath sai_thrift_src/gen-py --interface 0@sw_port0 --interface 1@sw_port1 --interface 2@sw_port2 \
    	 --interface 3@sw_port3 --interface 4@sw_port4 --interface 5@sw_port5 --interface 6@sw_port6 --interface 7@sw_port7  \
    	 sai_shabi.ShabiTest
    	 # sail2_new.L21DBridgeBasicTest sail2_new.L21QBridgeAccess2AccessTest  sail2_new.L21QBridgeAccess2TrunkTest sail2_new.L21QBridgeTrunk2TrunkTest sail2_new.L21DLagTest 
    	 