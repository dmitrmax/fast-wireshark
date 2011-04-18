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
package fastwireshark.data;

public class ByteMessagePlan extends Message{

	private final byte[] bytes;
	
	/**
	 * Creates a new byte message plan with the given bytes
	 * @param bytes The bytes to use in the message
	 */
	public ByteMessagePlan(byte[] bytes, String from, String to){
		super(from,to);
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
