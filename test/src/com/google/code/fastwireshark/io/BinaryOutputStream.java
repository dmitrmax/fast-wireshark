package com.google.code.fastwireshark.io;

import java.io.IOException;
import java.io.OutputStream;

/**
 * An output stream that will write the bytes out as ASCII 1s and 0s representing the bits of the byte.
 * @author pmiele
 *
 */
public class BinaryOutputStream extends OutputStream{

	private final OutputStream out;
	private boolean newLineOnByte = false;
	
	
	/**
	 * The output stream itself doesn't output, but converts for another output stream.
	 * @param o OutputStream to wrap
	 */
	public BinaryOutputStream(OutputStream o){
		out = o;
	}
	
	/**
	 * The output stream itself doesn't output, but converts for another output stream.
	 * This will also cause a new line to be written every byte
	 * @param o OutputStream to wrap
	 * @param newLineOnByte
	 */
	public BinaryOutputStream(OutputStream o, boolean newLineOnByte){
		out = o;
		this.newLineOnByte = newLineOnByte;
	}
	
	
	@Override
	public void write(int i) throws IOException {
		//Take the lower byte
		byte b = (byte) i;
		out.write(b);
		if(getNewLineOnByte()){
			out.write('\n');
		}
	}
	
	public void setNewLineOnByte(boolean newLineOnByte){
		this.newLineOnByte = newLineOnByte;
	}
	
	public boolean getNewLineOnByte(){
		return this.newLineOnByte;
	}
	
	/**
	 * Flushes the buffer
	 * Flushes the wrapped buffer
	 */
	public void flush() throws IOException{
		out.flush();
		super.flush();
	}
	
}