package com.google.code.fastwireshark.data;

public class ByteMessagePlan implements Message{

	private final byte[] bytes;
	
	/**
	 * Creates a new byte message plan with the given bytes
	 * @param bytes The bytes to use in the message
	 */
	public ByteMessagePlan(byte[] bytes){
		if(bytes == null){
			throw new IllegalArgumentException("bytes is null");
		}
		this.bytes = bytes;
	}
	
	public byte[] getBytes(){
		byte[] t = new byte[bytes.length];
		System.arraycopy(bytes, 0, t, 0, bytes.length);
		return t;
	}
}
