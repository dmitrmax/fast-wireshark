package com.google.code.fastwireshark.net;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

import com.google.code.fastwireshark.io.AsciiBinaryOutputStream;
import com.google.code.fastwireshark.util.Constants;



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
		OutputStream out = new AsciiBinaryOutputStream(System.out, true);
		try {
			serverclient.write(buf);
			client.read(read_buf);
			for(int i = 0 ; i < read_buf.position() ; i++){
				out.write(read_buf.array()[i]);
			}
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
