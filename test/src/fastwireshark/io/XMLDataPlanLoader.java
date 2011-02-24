package fastwireshark.io;


import java.io.IOException;
import java.math.BigDecimal;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.openfast.template.MessageTemplate;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import fastwireshark.data.ByteMessagePlan;
import fastwireshark.data.DataPlan;
import fastwireshark.data.MessagePlan;
import fastwireshark.util.Constants;

public class XMLDataPlanLoader extends DefaultHandler implements Constants{

	private DataPlan plan;
	private MessageTemplate curMessageTemplate;
	private List<Object> curValues;
	private Stack<List<Object>> valueStack;
	private ByteBuffer byteMessage;
	
	/**
	 * Attempts to load the data plan from the specified XML file.
	 * Assumes that all required templates have already been loaded
	 * @param xmlFile The XML file containing the data plan to load
	 * @return The DataPlan
	 */
	public DataPlan loadPlan(String xmlFile){
		SAXParserFactory spf = SAXParserFactory.newInstance();
		try {
			SAXParser sp = spf.newSAXParser();
			sp.parse(xmlFile, this);
		} catch (ParserConfigurationException e) {
			plan = null;
			e.printStackTrace();
		} catch (SAXException e) {
			plan = null;
			e.printStackTrace();
		} catch (IOException e) {
			plan = null;
			e.printStackTrace();
		}
		return plan;
	}
	
	@Override
	public void characters(char[] ch, int start, int length)
			throws SAXException {
		/*
		 * This will be called for every 2 or so lines of data, so we will need to keep state between reads
		 */
		//The array given may not be the correct size and will contain extra info, just grab what we want
		char[] t = new char[length];
		System.arraycopy(ch, start, t, 0, length);
		ch = t;
		//If we are handling a byte message, handle it, otherwise we are done here
		if(byteMessage != null){
			//Use String library to remove extra characters and whitespace
			String value = new String(ch);
			value = value.replaceAll("[\\s]", "");
			value = value.trim();
			ch = value.toCharArray();
			//Make sure we have the correct number of 'bit' characters
			if(ch.length % BITS_IN_BYTE != 0){
				char[] error = new char[length];
				System.arraycopy(ch, start, error, 0, length);
				throw new RuntimeException("Binary is not in a multiple of 8: " + new String(error));
			}
			//Take 8 characters from the data and work with them
			byte[] bytes = new byte[ch.length/BITS_IN_BYTE];
			char[] bite = new char[BITS_IN_BYTE];
			for(int i = 0 ; i < bytes.length ; i++){
				System.arraycopy(ch, i*BITS_IN_BYTE, bite, 0, BITS_IN_BYTE);
				bytes[i] = charArrayToByte(bite);
			}
			//Store the bytes
			for(byte b : bytes){
				byteMessage.put(b);
			}
		}
	}
	
	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException{
		if(!(qName.equalsIgnoreCase(MESSAGE) || qName.equalsIgnoreCase(PLAN) || qName.equalsIgnoreCase(BYTE_MESSAGE)) && curValues == null){
			throw new RuntimeException("Optional Group contains fields");
		}
		if(qName.equalsIgnoreCase(PLAN)){
			if(plan != null){
				throw new RuntimeException("Multiple Plans in one file");
			}
			plan = new DataPlan();
		} else 
		if(qName.equalsIgnoreCase(MESSAGE)){
			if(curValues != null){
				throw new RuntimeException("Starting another message without finishing the old one");
			}
			if(attributes.getValue(VALUE) != null){
				curMessageTemplate = MessageTemplateRepository.getTemplateByID(Integer.valueOf(attributes.getValue(VALUE)));
			} else
			if(attributes.getValue(TEMPLATE_NAME) != null){
				curMessageTemplate = MessageTemplateRepository.getTemplateByName(attributes.getValue(TEMPLATE_NAME));
			} else {
				if(curMessageTemplate == null){
					throw new RuntimeException("First Message lacks a template");
				}
			}
			curValues = new ArrayList<Object>();
			valueStack = new Stack<List<Object>>();
		} else
		if(qName.equalsIgnoreCase(BYTE_MESSAGE)){
			if(curValues != null){
				throw new RuntimeException("Starting another message without finishing the old one");
			}
			byteMessage = ByteBuffer.allocate(MAX_PACKET_SIZE);
		} else
		if(qName.equalsIgnoreCase(INT32) ||
		   qName.equalsIgnoreCase(UINT32)){
			if(attributes.getValue(VALUE) != null){
				curValues.add(Integer.valueOf(attributes.getValue(VALUE)));
			} else {
				curValues.add(null);
			}
		} else
		if(qName.equalsIgnoreCase(INT64) ||
		   qName.equalsIgnoreCase(UINT64)){
			if(attributes.getValue(VALUE) != null){
				curValues.add(Long.valueOf(attributes.getValue(VALUE)));
			} else {
				curValues.add(null);
			}
		} else
		if(qName.equalsIgnoreCase(DECIMAL)) {
			if(attributes.getValue(VALUE) != null){
				curValues.add(new BigDecimal(attributes.getValue(VALUE)));
			} else {
				curValues.add(null);
			}
		} else
		if(qName.equalsIgnoreCase(ASCII) ||
		   qName.equalsIgnoreCase(UNICODE)) {
			if(attributes.getValue(VALUE) != null){
				curValues.add(attributes.getValue(VALUE));
			} else {
				curValues.add(null);
			}
		} else
		if(qName.equalsIgnoreCase(BYTEVECTOR)){
			if(attributes.getValue(VALUE) != null){
				String val = attributes.getValue(VALUE);
				if(val.length() % 2 > 0){
					throw new RuntimeException("Hex string is odd in length: " + val);
				}
				byte[] bytes = new byte[val.length()/2];
				for(int i = 0 ; i < val.length() / 2 ; i++){
					bytes[i] = hexToByte(val.charAt(i*2),val.charAt(i*2+1));
				}
				curValues.add(bytes);
			} else {
				curValues.add(null);
			}
		} else
		if(qName.equalsIgnoreCase(GROUP) ||
		   qName.equalsIgnoreCase(SEQUENCE)) {
			if(attributes.getValue(VALUE) != null){
				valueStack.push(curValues);
				curValues = new ArrayList<Object>();
			} else {
				valueStack.push(curValues);
				curValues = null;
			}
			
		}
	}
	
	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		if(qName.equalsIgnoreCase(MESSAGE)){
			plan.addMessagePlan(new MessagePlan(curMessageTemplate, curValues));
			curValues = null;
		} else 
		if(qName.equalsIgnoreCase(BYTE_MESSAGE)){
			byte[] data = new byte[byteMessage.position()];
			byteMessage.rewind();
			byteMessage.get(data);
			plan.addMessagePlan(new ByteMessagePlan(data));
			byteMessage = null;
		} else
		if(qName.equalsIgnoreCase(GROUP) ||
		   qName.equalsIgnoreCase(SEQUENCE)){
			List<Object> temp = curValues;
			curValues = valueStack.pop();
			curValues.add(temp);
		}
	}
	
	/**
	 * Converts two characters to a byte containing the hex conversion of one character
	 * in the upper nibble and the other character's hex conversion in the lower nibble
	 * @param upper Character to place in the upper nibble
	 * @param lower Character to place in the lower nibble
	 * @return Byte with both upper and lower nibbles set
	 */
	private byte hexToByte(char upper, char lower){
		byte up = (byte)(charToNibble(upper)<<4);
		byte low = charToNibble(lower) ;
		return (byte)(up | low);
	}
	
	/**
	 * Converts a single character to a byte with the lower nibble to to the hex value
	 * represented by the character
	 * @param cha The character to convert
	 * @return A byte containing the hex represented by the character
	 * @throws IllegalArgumentException If cha is not in the range of hex: 0-9 | a-f
	 */
	private byte charToNibble(char cha){
		switch (cha){
		case '0':
			return 0x0;
		case '1':
			return 0x1;
		case '2':
			return 0x2;
		case '3':
			return 0x3;
		case '4':
			return 0x4;
		case '5':
			return 0x5;
		case '6':
			return 0x6;
		case '7':
			return 0x7;
		case '8':
			return 0x8;
		case '9':
			return 0x9;
		case 'a':
		case 'A':
			return 0xA;
		case 'b':
		case 'B':
			return 0xB;
		case 'c':
		case 'C':
			return 0xC;
		case 'd':
		case 'D':
			return 0xD;
		case 'e':
		case 'E':
			return 0xE;
		case 'f':
		case 'F':
			return 0xF;
		default:
			throw new IllegalArgumentException("Invalid hex character: " + cha);
		}
	}
	
	/**
	 * Converts a char array to a byte
	 * @param ch Char array to convert, must me '0' or '1' and have a length of 8
	 * @return The byte represented by ch
	 */
	private byte charArrayToByte(char[] ch){
		if(ch.length != BITS_IN_BYTE){
			throw new IllegalArgumentException("char array is not length 8: " + new String(ch));
		}
		byte b = 0;
		for(int i = 0 ; i < ch.length ; i++){
			switch (ch[i]){
			case '0':
			break;
			case '1':
				//Shift over so that the bit is in the correct slot
				//-1 due to to account for initial position of '1'
				b |= 1<<(BITS_IN_BYTE - 1 - i);
			break;
			default:
				throw new IllegalArgumentException("Invalid binary character: " + ch[i]);
			}
		}
		return b;
	}
}
