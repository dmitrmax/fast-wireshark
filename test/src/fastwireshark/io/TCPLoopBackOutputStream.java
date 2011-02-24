package fastwireshark.io;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import fastwireshark.net.TCPLoopBackServer;


public class TCPLoopBackOutputStream extends OutputStream {

	private ByteBuffer buffer;
	private TCPLoopBackServer server;
	
	/**
	 * Creates a loopback output stream and server with the max packet size specified and port
	 * @param maxPacketSize Max size of the packet
	 * @param port Port to use
	 */
	public TCPLoopBackOutputStream(int maxPacketSize, int port){
		if(maxPacketSize <= 0){
			throw new IllegalArgumentException("Invalid max packet size: " + maxPacketSize);
		}
		buffer = ByteBuffer.wrap(new byte[maxPacketSize]);
		server = new TCPLoopBackServer(port);
		
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
	
	
	@Override
	public void close() throws IOException {
		super.close();
		server.close();
	}
	
	/**
	 * Flushes the stream and sends a packet
	 */
	@Override
	public void flush() throws IOException {
		int pos = buffer.position();
		byte[] b = new byte[pos];
		buffer.rewind();
		buffer.get(b, 0, pos);
		server.write(ByteBuffer.wrap(b));
		buffer.clear();
	}


	


}
