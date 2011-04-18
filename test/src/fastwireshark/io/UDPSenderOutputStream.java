/*
 * This file is part of FAST Wireshark.
 *
 * FAST Wireshark is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FAST Wireshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 * 
 * You should have received a copy of the Lesser GNU General Public License
 * along with FAST Wireshark.  If not, see 
 * <http://www.gnu.org/licenses/lgpl.txt>.
 */
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
