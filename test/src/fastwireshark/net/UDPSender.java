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
