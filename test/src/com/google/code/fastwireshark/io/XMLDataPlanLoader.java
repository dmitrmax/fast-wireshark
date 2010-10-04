package com.google.code.fastwireshark.io;


import java.io.IOException;
import java.math.BigDecimal;
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

import com.google.code.fastwireshark.data.DataPlan;
import com.google.code.fastwireshark.data.MessagePlan;
import com.google.code.fastwireshark.util.Constants;

public class XMLDataPlanLoader extends DefaultHandler implements Constants{

	private DataPlan plan;
	private MessageTemplate curMessageTemplate;
	private List<Object> curValues;
	private Stack<List<Object>> valueStack;
	
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
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException{
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
			if(attributes.getValue(TEMPLATE_ID) != null){
				curMessageTemplate = MessageTemplateRepository.getTemplateByID(Integer.valueOf(attributes.getValue("templateID")));
			} else
			if(attributes.getValue(TEMPLATE_NAME) != null){
				curMessageTemplate = MessageTemplateRepository.getTemplateByName(attributes.getValue("templateName"));
			} else {
				if(curMessageTemplate == null){
					throw new RuntimeException("First Message lacks a template");
				}
			}
			curValues = new ArrayList<Object>();
			valueStack = new Stack<List<Object>>();
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
		if(qName.equalsIgnoreCase(STRING)||
		   qName.equalsIgnoreCase(ASCII) ||
		   qName.equalsIgnoreCase(UNICODE)) {
			if(attributes.getValue(VALUE) != null){
				curValues.add(attributes.getValue(VALUE));
			} else {
				curValues.add(null);
			}
		} else
		if(qName.equalsIgnoreCase(GROUP) ||
		   qName.equalsIgnoreCase(SEQUENCE)) {
			valueStack.push(curValues);
			curValues = new ArrayList<Object>();
		}
	}
	
	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		if(qName.equalsIgnoreCase(MESSAGE)){
			plan.addMessagePlan(new MessagePlan(curMessageTemplate, curValues));
			curValues = null;
		} else 
		if(qName.equalsIgnoreCase(GROUP) ||
		   qName.equalsIgnoreCase(SEQUENCE)){
			List<Object> temp = curValues;
			curValues = valueStack.pop();
			curValues.add(temp);
		}
	}
	
	
}
