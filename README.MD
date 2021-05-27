# Serial interposer for Endurosat UHF/Si446x Transceiver
Transports data in 64 byte frame chunks, with 56 bytes available for payload. Frame splitting has to be taken care of by caller.
To build, execute `make`. You can specify `CROSS_COMPILE=arm-linux-gnueabi-` in command line if you have the cross compiler enabled
to compile it for the target on x86. Two simple loopback test programs are provided.