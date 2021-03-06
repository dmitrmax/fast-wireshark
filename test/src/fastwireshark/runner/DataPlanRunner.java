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
package fastwireshark.runner;

import java.io.IOException;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.util.Iterator;
import java.util.List;

import org.openfast.DecimalValue;
import org.openfast.GroupValue;
import org.openfast.Message;
import org.openfast.MessageOutputStream;
import org.openfast.SequenceValue;
import org.openfast.template.ComposedScalar;
import org.openfast.template.Field;
import org.openfast.template.Group;
import org.openfast.template.Scalar;
import org.openfast.template.Sequence;

import fastwireshark.data.ByteMessagePlan;
import fastwireshark.data.DataPlan;
import fastwireshark.data.MessagePlan;
import fastwireshark.io.PcapFileWriter;
import fastwireshark.util.Constants;

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
		for(fastwireshark.data.Message mp : dp){
			if(messageOut.getUnderlyingStream() instanceof PcapFileWriter){
				PcapFileWriter writer = (PcapFileWriter) messageOut.getUnderlyingStream();
				writer.setFrom(mp.getFrom());
				writer.setTo(mp.getTo());
			}
			if(mp instanceof MessagePlan){
				runMessagePlan((MessagePlan)mp);
			} else if (mp instanceof ByteMessagePlan){
				runByteMessagePlan((ByteMessagePlan)mp);
			}
		}
		messageOut.close();
		try {
			messageOut.getUnderlyingStream().close();
		} catch (IOException e) {
			e.printStackTrace();
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
		populateGroup(m,mp.getValues());
		messageOut.writeMessage(m);
		try {
			messageOut.getUnderlyingStream().flush();
		} catch (IOException e) {
			System.err.println("Error writing message: " + m);
			e.printStackTrace();
		}
	}
	
	/**
	 * Runs a ByteMessagePlan
	 * Reads the bytes of the message plan and writes it out
	 * @param mp The Message Plan to run
	 */
	private void runByteMessagePlan(ByteMessagePlan mp){
		OutputStream out = messageOut.getUnderlyingStream();
		try {
			out.write(mp.getBytes());
			out.flush();
		} catch (IOException e) {
			System.err.println("Error writing message: " + mp);
			e.printStackTrace();
		}
	}
	
	/**
	 * Populate the fields in a group with those in a list
	 * @param gv The group to populate
	 * @param values The list of values to use to populate
	 */
	@SuppressWarnings("unchecked")
	private void populateGroup(GroupValue gv, List<Object> values){
		Iterator<Object> iter = values.iterator();
		//The message is special and needs to have its first value skipped, this causes all other instances to require one less in the loop
		for(int i = gv instanceof Message ? 1 : 0 ; i <= values.size() + (gv instanceof Message ? 0 : -1) ; i++){
			Object o = iter.next();
			Field f = gv.getGroup().getField(i);
      if (o == null) {
        // DO NOTHING!
      } else
			if(f instanceof Sequence){
				Sequence s = (Sequence)f;
				SequenceValue sv = new SequenceValue(s);
				List<List<Object>> seqValues = (List<List<Object>>) o;
				for(List<Object> l : seqValues){
					GroupValue sgv = new GroupValue(s.getGroup());
					populateGroup(sgv,l);
					sv.add(sgv);
				}
				gv.setFieldValue(i, sv);
			} else
			if(f instanceof Group){
				Group g = (Group)f;
				GroupValue ggv = new GroupValue(g);
				populateGroup(ggv,(List<Object>)o);
				gv.setFieldValue(i, ggv);
			} else
			if(f instanceof ComposedScalar){
				setValue(gv,(ComposedScalar)f,i,o);
			} else
			if(f instanceof Scalar){
				setValue(gv,(Scalar)f,i,o);
			}
      		
			
		}
	}
	
	public void setValue(GroupValue gv, Scalar f, int i, Object o){
		if(f.getType().getName().equals(INT32) ||
				   f.getType().getName().equals(UINT32)){
					gv.setInteger(i, (Integer)o);
				} else
				if(f.getType().getName().equals(INT64) ||
				   f.getType().getName().equals(UINT64)){
					gv.setLong(i, (Long)o);
				} else
				if(f.getType().getName().equals(DECIMAL)){
				//Decimal can be of differing arguments, further determine correct type
					if(o instanceof Double){
						gv.setDecimal(i, (Double)o);
					} else 
					if(o instanceof Float){
						gv.setDecimal(i, (Float)o);
					} else 
					if(o instanceof BigDecimal){
						BigDecimal b = (BigDecimal)o;
						DecimalValue dv = new DecimalValue(b.unscaledValue().longValue(),-b.scale());
						gv.setFieldValue(i, dv);
					}
				} else
				if(f.getType().getName().equals(UNICODE) ||
				   f.getType().getName().equals(ASCII)){
						gv.setString(i, (String)o);
				} else
				if(f.getType().getName().equals(BYTEVECTOR)){
						gv.setByteVector(i, (byte[])o);
				}
	}
	
	public void setValue(GroupValue gv, ComposedScalar f, int i, Object o){
		if(f.getType().getName().equals(INT32) ||
				   f.getType().getName().equals(UINT32)){
					gv.setInteger(i, (Integer)o);
				} else
				if(f.getType().getName().equals(INT64) ||
				   f.getType().getName().equals(UINT64)){
					gv.setLong(i, (Long)o);
				} else
				if(f.getType().getName().equals(DECIMAL)){
				//Decimal can be of differing arguments, further determine correct type
					if(o instanceof Double){
						gv.setDecimal(i, (Double)o);
					} else 
					if(o instanceof Float){
						gv.setDecimal(i, (Float)o);
					} else 
					if(o instanceof BigDecimal){
						BigDecimal b = (BigDecimal)o;
						DecimalValue dv = new DecimalValue(b.unscaledValue().longValue(),-b.scale());
//						gv.setDecimal(i, (BigDecimal)o);
						gv.setFieldValue(i, dv);
					}
				} else
				if(f.getType().getName().equals(UNICODE) ||
				   f.getType().getName().equals(ASCII)){
						gv.setString(i, (String)o);
				} else
				if(f.getType().getName().equals(BYTEVECTOR)){
						gv.setByteVector(i, (byte[])o);
				}
	}
}
