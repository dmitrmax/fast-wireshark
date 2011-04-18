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
import java.net.SocketException;
import java.net.UnknownHostException;

public class UDPLoopBackServer{

	private DatagramSocket socket;
	
	/**
	 * Sets up a Loop Back on the specified port
	 * @param port
	 */
	public UDPLoopBackServer(int port){
		if(port <= 0){
			throw new IllegalArgumentException("Invalid Port Specified: " + port);
		}
		try {
			socket = new DatagramSocket(port, InetAddress.getLocalHost());
		} catch (SocketException e) {
			System.err.println("Error creating socket on port: " + port);
			throw new RuntimeException(e);
		} catch (UnknownHostException e) {
			System.err.println("Unable to resolve local host");
			throw new RuntimeException(e);
		}
	}
	
	/**
	 * Sends a UDP packet containing the given bytes over the loopback
	 * @param bytes The bytes to send
	 */
	public void write(byte[] bytes){
		//Send the bytes
		try {
			 socket.send(new DatagramPacket(bytes,bytes.length,socket.getLocalSocketAddress()));
		} catch (SocketException e) {
			System.err.println("Error writing out message. Invalid Socket.");
			throw new RuntimeException(e);
		} catch (IOException e) {
			System.err.println("Error writing out message.");
			throw new RuntimeException(e);
		}
		//Read the bytes in to clear the buffer
		DatagramPacket p = null;
		try {
			p = new DatagramPacket(new byte[bytes.length],bytes.length);
			socket.receive(p);
		} catch (IOException e) {
			System.err.println("Error reading packet in");
			throw new RuntimeException(e);
		}
	}
	
	
}
