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

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream.GetField;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import fastwireshark.util.Constants;

/**
 * Writes bytes to a pcap-style file
 * @author pmiele
 *
 */
public class PcapFileWriter extends OutputStream implements Constants{

	private OutputStream out = null;
	private int packetCounter = 0;
	private int checkSum = 0;
	private final short port;
	private ByteBuffer buffer;
	private String from;
	private String to;
	
	public PcapFileWriter(short port, String fileName){
		if(port <= 0){
			throw new IllegalArgumentException("Invalid port: " + port);
		}
		this.port = port;
		try {
			 out = new FileOutputStream(fileName);
		} catch (FileNotFoundException e) {
			throw new RuntimeException(e);
		}
		if (out == null){
			throw new RuntimeException("File not opened: " + fileName);
		}
		buffer = ByteBuffer.wrap(new byte[MAX_PACKET_SIZE]);
		
		final byte[] magicNumber = intToByteArray(PCAP_MAGIC_NUMBER);
		final byte[] majorVersionNumber = shortToByteArray(PCAP_MAJOR_VERSION_NUMBER);
		final byte[] minorVersionNumber = shortToByteArray(PCAP_MINOR_VERSION_NUMBER);
		final byte[] timeZoneOffset = intToByteArray(0);
		final byte[] sigFlags = intToByteArray(0);
		final byte[] snapLen = intToByteArray(MAX_PACKET_SIZE);
		final byte[] network = intToByteArray(PCAP_ETHERNET);
		
		try {
			//write out the global header
			out.write(magicNumber);
			out.write(majorVersionNumber);
			out.write(minorVersionNumber);
			out.write(timeZoneOffset);
			out.write(sigFlags);
			out.write(snapLen);
			out.write(network);
			
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}
	
	public void writePacket(byte[] packet){
		try {
			//udp checksum
			short c = (short)0;
			c+=port;
			c+=port;
			c+=(short)(packet.length + 8);
			for(byte b : packet){
				c+=b;
			}
			//Dont ask what most of this means
			//http://wiki.wireshark.org/Development/LibpcapFileFormat
			//Most of these values were just stolen from wireshark and moved around
			//Also I don't know why, but the endianness changes on certain fields
			out.write(intToByteArray(packetCounter++));
			out.write(intToByteArray(0));
			out.write(intToByteArray(packet.length+8+34));
			out.write(intToByteArray(packet.length+8+34));
			//start of ipv6 header
			out.write(intToByteArray(0));
			out.write(intToByteArray(0));
			out.write(intToByteArray(0));
			out.write(shortToByteArray((short)0x0008));
			//start of ipv4 header
			checkSum = 0;
			checkSum(new byte[]{(byte)0x45,(byte)0});
			out.write((byte)0x45);
			out.write((byte)0);
			checkSum(shortToByteArrayNoSwap((short)(packet.length+28)));
			out.write(shortToByteArrayNoSwap((short)(packet.length+28)));
			checkSum(shortToByteArray((short)0));
			out.write(shortToByteArray((short)0));
			checkSum(new byte[]{(byte)0x40,(byte)0});
			out.write((byte)0x40);
			out.write((byte)0);
			checkSum(new byte[]{(byte)0x40,(byte)0x11});
			out.write((byte)0x40);
			out.write((byte)0x11);
			checkSum(stringIPv4AddressToByteArray(from));
			checkSum(stringIPv4AddressToByteArray(to));
			//checksum
			while((checkSum >> 16) > 0){
				checkSum = (checkSum & 0xFFFF) + (checkSum>>16);
			}
			checkSum = ~checkSum;
			out.write(shortToByteArrayNoSwap((short)checkSum));
			//127.0.0.1 = 0x7f000001
			out.write(stringIPv4AddressToByteArray(from));
			out.write(stringIPv4AddressToByteArray(to));
			out.write(shortToByteArrayNoSwap((short)port));
			out.write(shortToByteArrayNoSwap((short)port));
			out.write(shortToByteArrayNoSwap((short)(packet.length + 8)));
			
			out.write(shortToByteArray((short)(c^0xffff)));
			out.write(packet);
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}
	
	
	//Helper methods
	private byte[] intToByteArray(int i){
		return new byte[] {(byte)(i>>0),(byte)(i>>8),(byte)(i>>16),(byte)(i>>24)};
	}
	private byte[] intToByteArrayNoSwap(int i){
		return new byte[] {(byte)(i>>24),(byte)(i>>16),(byte)(i>>8),(byte)(i>>0)};
	}
	private byte[] shortToByteArray(short s){
		return new byte[] {(byte)(s>>0),(byte)(s>>8)};
	}
	private byte[] shortToByteArrayNoSwap(short s){
		return new byte[] {(byte)(s>>8),(byte)(s>>0)};
	}
	
	/**
	 * Converts and IPv4 string to bytes
	 * @param addr
	 * @return
	 */
	private byte[] stringIPv4AddressToByteArray(String addr){
		String[] addrs = addr.split("\\.");
		byte[] b = new byte[4];
		for(int i = 0 ; i < addrs.length ; i++){
			b[i] = Byte.valueOf(addrs[i]);
		}
		return b;
	}
	
	public void setFrom(String from){
		if(from == null){throw new NullPointerException("From is null");}
		this.from = from;
	}
	public void setTo(String to){
		if(to == null){throw new NullPointerException("To is null");}
		this.to = to;
	}
	
	private void checkSum(byte[] ba){
		
		for(int i = 0 ; i < ba.length ; i+=2){
			if(ba.length %2 == 1 && i+1 == ba.length){
				checkSum+=((ba[i]<<8));
			} else {
				checkSum+=((ba[i]<<8) | (ba[i+1]));
			}
			if((checkSum & 0x80000000)>0){
				checkSum = (checkSum & 0xffff) + (checkSum>>16);
			}
		}
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
	 * Flushes the stream and writes the packet to the file
	 */
	@Override
	public void flush() throws IOException {
		byte[] data = new byte[buffer.position()];
		buffer.rewind();
		buffer.get(data);
		writePacket(data);
		buffer.rewind();
	}

	
}

