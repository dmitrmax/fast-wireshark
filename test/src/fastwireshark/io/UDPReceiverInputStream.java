package fastwireshark.io;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import fastwireshark.net.UDPReceiver;

public class UDPReceiverInputStream extends InputStream {

	private UDPReceiver receiver;
	private ByteBuffer bytes;
	private int pos = 0;
	private int cpos = 0;
	
	public UDPReceiverInputStream(int port){
		receiver = new UDPReceiver(port);
	}

	@Override
	public int read() throws IOException {
		while(bytes == null || cpos >= pos){
			bytes = receiver.read();
			if(bytes != null){
				pos = bytes.position();
				cpos = 0;
				bytes.rewind();
			}
			try {
				Thread.sleep(5);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		return bytes.get(cpos++);
	}
	

}
