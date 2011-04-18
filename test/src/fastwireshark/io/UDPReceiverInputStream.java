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
