package com.google.code.fastwireshark.net;

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
		try {
			socket.receive(new DatagramPacket(new byte[bytes.length],bytes.length));
		} catch (IOException e) {
			System.err.println("Error reading packet in");
			throw new RuntimeException(e);
		}
		
	}
	
	
}
