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
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

import fastwireshark.util.Constants;



public class TCPLoopBackServer {

	private ServerSocketChannel server;
	private SocketChannel serverclient;
	private SocketChannel client;
	
	
	public TCPLoopBackServer(int port){
		try {
			server = ServerSocketChannel.open();
			
			server.socket().bind(new InetSocketAddress(InetAddress.getLocalHost(),port));
			client = SocketChannel.open();
			server.configureBlocking(false);
			client.configureBlocking(false);
			client.connect(new InetSocketAddress(InetAddress.getLocalHost(),port));
			serverclient = server.accept();
			while(!client.isConnected()){
				client.finishConnect();
			}
			server.configureBlocking(true);
			client.configureBlocking(true);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void write(ByteBuffer buf){
		ByteBuffer read_buf = ByteBuffer.allocate(Constants.MAX_PACKET_SIZE);
		try {
			serverclient.write(buf);
			client.read(read_buf);
			read_buf.clear();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void close(){
		try {
			server.close();
			client.close();
			serverclient.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
}
