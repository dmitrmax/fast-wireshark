package fastwireshark.io;

import java.io.IOException;
import java.io.OutputStream;

/**
 * An output stream that will write the bytes out as ASCII 1s and 0s representing the bits of the byte.
 * @author pmiele
 *
 */
public class AsciiBinaryOutputStream extends OutputStream{

	private final OutputStream out;
	private boolean newLineOnByte = false;
	
	
	/**
	 * The output stream itself doesn't output, but converts for another output stream.
	 * @param o OutputStream to wrap
	 */
	public AsciiBinaryOutputStream(OutputStream o){
		out = o;
	}
	
	/**
	 * The output stream itself doesn't output, but converts for another output stream.
	 * This will also cause a new line to be written every byte
	 * @param o OutputStream to wrap
	 * @param newLineOnByte
	 */
	public AsciiBinaryOutputStream(OutputStream o, boolean newLineOnByte){
		out = o;
		this.newLineOnByte = newLineOnByte;
	}
	
	
	@Override
	public void write(int i) throws IOException {
		//Take the lower byte
		byte b = (byte) i;
		String s = Integer.toBinaryString(b);
		//If the byte is negative there will be 1's in the front, just grab the last 8 bits
		if(s.length() == 32){
			s = s.substring(24);
		}
		//Positive numbers drop leading zeros, Pad
		while(s.length() < 8) { s = "0" + s; }
		out.write(s.getBytes());
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
