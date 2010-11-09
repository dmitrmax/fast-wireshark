package com.google.code.fastwireshark.io;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;

import com.google.code.fastwireshark.util.Constants;

/**
 * Writes datagram packets to a pcap-style file
 * @author pmiele
 *
 */
public class PcapFileWriter implements Constants{

	OutputStream out = null;
	int packetCounter = 0;
	int checkSum = 0;
	
	public PcapFileWriter(String fileName){
		try {
			 out = new FileOutputStream(fileName);
		} catch (FileNotFoundException e) {
			throw new RuntimeException(e);
		}
		if (out == null){
			throw new RuntimeException("File not opened: " + fileName);
		}
		
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
	
	public void writePacket(DatagramPacket packet){
		try {
			//udp checksum
			short c = (short)0;
			c+=packet.getPort();
			c+=packet.getPort();
			c+=(short)(packet.getData().length + 8);
			for(byte b : packet.getData()){
				c+=b;
			}
			//Dont ask what most of this means
			//http://wiki.wireshark.org/Development/LibpcapFileFormat
			//Most of these values were just stolen from wireshark and moved around
			//Also I don't know why, but the endianness changes on certain fields
			out.write(intToByteArray(packetCounter++));
			out.write(intToByteArray(0));
			out.write(intToByteArray(packet.getData().length+8+34));
			out.write(intToByteArray(packet.getData().length+8+34));
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
			checkSum(shortToByteArrayNoSwap((short)(packet.getData().length+28)));
			out.write(shortToByteArrayNoSwap((short)(packet.getData().length+28)));
			checkSum(shortToByteArray((short)0));
			out.write(shortToByteArray((short)0));
			checkSum(new byte[]{(byte)0x40,(byte)0});
			out.write((byte)0x40);
			out.write((byte)0);
			checkSum(new byte[]{(byte)0x40,(byte)0x11});
			out.write((byte)0x40);
			out.write((byte)0x11);
			checkSum(intToByteArrayNoSwap(0x7f000001));
			checkSum(intToByteArrayNoSwap(0x7f000001));
			//checksum
			while((checkSum >> 16) > 0){
				checkSum = (checkSum & 0xFFFF) + (checkSum>>16);
			}
			checkSum = ~checkSum;
			out.write(shortToByteArrayNoSwap((short)checkSum));
			//127.0.0.1 = 0x7f000001
			out.write(intToByteArrayNoSwap(0x7f000001));
			out.write(intToByteArrayNoSwap(0x7f000001));
			out.write(shortToByteArrayNoSwap((short)packet.getPort()));
			out.write(shortToByteArrayNoSwap((short)packet.getPort()));
			out.write(shortToByteArrayNoSwap((short)(packet.getData().length + 8)));
			
			out.write(shortToByteArray((short)(c^0xffff)));
			out.write(packet.getData());
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
			if(checkSum == 0xbc3c){
				System.out.println("EQUALS");
			}
		}
	}

	
}

