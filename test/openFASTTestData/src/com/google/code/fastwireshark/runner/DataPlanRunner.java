package com.google.code.fastwireshark.runner;

import java.io.IOException;
import java.math.BigDecimal;
import java.util.Iterator;
import java.util.List;

import org.openfast.Message;
import org.openfast.MessageOutputStream;
import org.openfast.template.Field;
import org.openfast.template.Scalar;

import com.google.code.fastwireshark.data.DataPlan;
import com.google.code.fastwireshark.data.MessagePlan;
import com.google.code.fastwireshark.util.Constants;

public class DataPlanRunner implements Constants{

	/**
	 * The output stream that will be used to write out the messages
	 */
	private MessageOutputStream messageOut;
	
	/**
	 * Retrieves the output stream used by this runner to write out run results
	 * @return The MessageOutputStream used by the runner
	 */
	public MessageOutputStream getMessageOutputStream(){
		return messageOut;
	}
	
	/**
	 * Sets the output stream to be used by this runner to write out run results
	 * @param messageOut The MessageOutputStream to use to write out run results
	 */
	public void setMessageOutputStream(MessageOutputStream messageOut){
		this.messageOut = messageOut;
	}
	
	/**
	 * Runs a DataPlan
	 * Will iterate over all the MessagePlans in the DataPlan and write out each message in order.
	 * @param dp The DataPlan to run
	 * @throws RuntimeException If {@link #messageOut} is null
	 */
	public void runDataPlan(DataPlan dp){
		if(messageOut == null){
			throw new RuntimeException("messageOut is null");
		}
		for(MessagePlan mp : dp){
			runMessagePlan(mp);
		}
	}
	
	
	/**
	 * Runs a MessagePlan
	 * Creates a message based on the plan template, then populates the fields, then writes it using the Runner's MessageOutputStream 
	 * The MessageOutputStream is then flushed to force the message out.
	 * @param mp The Message Plan to run
	 */
	private void runMessagePlan(MessagePlan mp){
		Message m = new Message(mp.getTemplate());
		populateFields(m,mp.getValues());
		messageOut.writeMessage(m);
		try {
			messageOut.getUnderlyingStream().flush();
		} catch (IOException e) {
			System.err.println("Error writing message: " + m);
			e.printStackTrace();
		}
	}
	
	/**
	 * Populates the fields in a message with those in the map
	 * @param message Message to populate
	 * @param values Values to put into message
	 */
	private void populateFields(Message message,List<Object> values )
	{
		Iterator<Object> iter = values.iterator();
		for(int i = 1 ; i <= values.size() ; i++){
			Object o = iter.next();
			Field f = message.getTemplate().getField(i);
			
			if(((Scalar)f).getType().getName().equals(INT32) ||
			   ((Scalar)f).getType().getName().equals(UINT32)){
				message.setInteger(i, (Integer)o);
			} else
			if(((Scalar)f).getType().getName().equals(INT64) ||
			   ((Scalar)f).getType().getName().equals(UINT64)){
				message.setLong(i, (Long)o);
			} else
			if(((Scalar)f).getType().getName().equals(DECIMAL)){
				//Decimal can be of differing arguments, further determine correct type
				if(o instanceof Double){
					message.setDecimal(i, (Double)o);
				} else 
				if(o instanceof Float){
					message.setDecimal(i, (Float)o);
				} else 
				if(o instanceof BigDecimal){
					message.setDecimal(i, (BigDecimal)o);
				}
			} else
			if(((Scalar)f).getType().getName().equals(STRING) ||
			   ((Scalar)f).getType().getName().equals(UNICODE) ||
			   ((Scalar)f).getType().getName().equals(ASCII)){
				message.setString(i, (String)o);
			}
		}
	}
}
