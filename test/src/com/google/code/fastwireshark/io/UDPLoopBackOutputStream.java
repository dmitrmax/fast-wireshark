package com.google.code.fastwireshark.io;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import com.google.code.fastwireshark.net.UDPLoopBackServer;


public class UDPLoopBackOutputStream extends OutputStream {

	private ByteBuffer buffer;
	private UDPLoopBackServer server;
	
	/**
	 * Creates a loopback output stream and server with the max packet size specified and port
	 * @param maxPacketSize Max size of the packet
	 * @param port Port to use
	 */
	public UDPLoopBackOutputStream(int maxPacketSize, int port, String pcapFile){
		if(maxPacketSize <= 0){
			throw new IllegalArgumentException("Invalid max packet size: " + maxPacketSize);
		}
		buffer = ByteBuffer.wrap(new byte[maxPacketSize]);
		server = new UDPLoopBackServer(port,pcapFile);
		
	}
	
	/**
	 * Buffers the byte for sending on the flush
	 * @param i Integer that only the lowest order byte will be used
	 */
	@Override
	public void write(int i) throws IOException {
		if(buffer.remaining() == 0){
			throw new RuntimeException("Buffer Overflow");
		}
		byte b = (byte) i;
		buffer.put(b);
	}
	
	/**
	 * Flushes the stream and sends a packet
	 */
	@Override
	public void flush() throws IOException {
		byte[] data = new byte[buffer.position()];
		buffer.rewind();
		buffer.get(data);
		server.write(data);
		buffer.rewind();
	}


	


}
