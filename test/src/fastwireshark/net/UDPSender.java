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
package fastwireshark.net;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;

public class UDPSender {
	
	private DatagramChannel channel;
	
	public UDPSender(int port, InetAddress addr){
		
		try {
			channel = DatagramChannel.open();
			channel.connect(new InetSocketAddress(addr,port));
			if(!channel.isConnected()){
				System.err.println("ERROR CONNECTING SENDER");
			}
		} catch (IOException e) {
			System.err.println("Unable to open socket");
			throw new RuntimeException(e);
		}
		config();
	}
	
	public UDPSender(int port, String addr){
		try {
			channel = DatagramChannel.open();
			channel.connect(new InetSocketAddress(addr,port));
		} catch (IOException e) {
			System.err.println("Unable to open socket");
			throw new RuntimeException(e);
		}
		config();
	}
	
	private void config(){
		try {
			channel.configureBlocking(false);
		} catch (IOException e) {
			System.err.println("Unable to specify non-blocking");
			throw new RuntimeException(e);
		}
	}
	
	public void write(ByteBuffer buf){
		try {
			channel.write(buf);
		} catch (IOException e) {
			System.err.println("Error writing data");
			throw new RuntimeException(e);
		}
	}
	
	public void write(byte b){
		ByteBuffer buf = ByteBuffer.wrap(new byte[1]);
		buf.put(b);
		buf.rewind();
		System.out.println(buf);
		write(buf);
	}
}
