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
package fastwireshark.util;

/**
 * Defines constants that are used between multiple classes
 * @author pmiele
 *
 */
public interface Constants {

	public static final String INT32 = "int32";
	public static final String UINT32 = "uInt32";
	public static final String INT64 = "int64";
	public static final String UINT64 = "uInt64";
	public static final String DECIMAL = "decimal";
	public static final String UNICODE = "unicode";
	public static final String ASCII = "ascii";
	public static final String BYTEVECTOR = "byteVector";
	public static final String GROUP = "group";
	public static final String SEQUENCE = "sequence";
	public static final String MESSAGE = "message";
	public static final String BYTE_MESSAGE = "bytemessage";
	public static final String PLAN = "plan";
	public static final String TEMPLATE_ID = "templateID";
	public static final String TEMPLATE_NAME = "templateName";
	public static final String VALUE = "value";
	public static final String FROM = "from";
	public static final String TO = "to";
	
	public static final int MAX_PACKET_SIZE = 1024*1024*10;
	public static final int BITS_IN_BYTE = 8;
	
	/*
	 * Pcap File Constants
	 */
	public static final int PCAP_MAGIC_NUMBER = 0xa1b2c3d4;
	public static final short PCAP_MAJOR_VERSION_NUMBER = (short)2;
	public static final short PCAP_MINOR_VERSION_NUMBER = (short)4;
	public static final int PCAP_ETHERNET = 1;
	
	public static final String DEFAULT_ADDRESS = "127.0.0.1";
}
