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
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import fastwireshark.util.Constants;

public class UDPReceiver {

	private DatagramChannel channel;
	private DatagramSocket socket;
	
	public UDPReceiver(int port){
		try {
			channel = DatagramChannel.open();
			socket = channel.socket();
			socket.bind(new InetSocketAddress(port));
			System.out.println("Bound to: " + socket.getLocalPort() + " should be: " + port);
			/* TODO: Figure out why this was assigning a random port
			channel.connect(new InetSocketAddress(addr,port));
			//*/
			if(!channel.isConnected()){
				System.err.println("ERROR CONNECTING CHANNEL");
			}
			
		} catch (IOException e) {
			System.err.println("Unable to open socket");
			throw new RuntimeException(e);
		}
		config();
	}
	
	private void config(){
		try {
			channel.configureBlocking(true);
		} catch (IOException e) {
			System.err.println("Unable to specify non-blocking");
			throw new RuntimeException(e);
		}
	}
	
	public ByteBuffer read(){
		ByteBuffer bytes = ByteBuffer.wrap(new byte[Constants.MAX_PACKET_SIZE]);
		Object o = null;
		try {
			System.out.println("Read Bytes");
			o = channel.receive(bytes);
			System.out.println("Readed Bytes");
		} catch (IOException e) {
			System.err.println("Unable to read packet");
			throw new RuntimeException(e);
		}
		if(o == null){
			bytes = null;
		}
		return bytes;
	}
	
}
