package fastwireshark.io;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import fastwireshark.net.UDPSender;
import fastwireshark.util.Constants;

public class UDPSenderOutputStream extends OutputStream {

	private UDPSender sender;
	private ByteBuffer buffer;
	
	public UDPSenderOutputStream(int port, String addr){
		sender = new UDPSender(port,addr);
		buffer = ByteBuffer.wrap(new byte[Constants.MAX_PACKET_SIZE]);
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
		sender.write(ByteBuffer.wrap(data));
		buffer.rewind();
	}

}
