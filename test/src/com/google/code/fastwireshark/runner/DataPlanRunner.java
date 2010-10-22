package com.google.code.fastwireshark.runner;

import java.io.IOException;
import java.math.BigDecimal;
import java.util.Iterator;
import java.util.List;

import org.openfast.DecimalValue;
import org.openfast.GroupValue;
import org.openfast.Message;
import org.openfast.MessageOutputStream;
import org.openfast.SequenceValue;
import org.openfast.template.Field;
import org.openfast.template.Group;
import org.openfast.template.Scalar;
import org.openfast.template.Sequence;

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
			if(((Scalar)f).getType().getName().equals(INT32) ||
			   ((Scalar)f).getType().getName().equals(UINT32)){
				gv.setInteger(i, (Integer)o);
			} else
			if(((Scalar)f).getType().getName().equals(INT64) ||
			   ((Scalar)f).getType().getName().equals(UINT64)){
				gv.setLong(i, (Long)o);
			} else
			if(((Scalar)f).getType().getName().equals(DECIMAL)){
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
//					gv.setDecimal(i, (BigDecimal)o);
					gv.setFieldValue(i, dv);
				}
			} else
			if(((Scalar)f).getType().getName().equals(UNICODE) ||
			   ((Scalar)f).getType().getName().equals(ASCII)){
					gv.setString(i, (String)o);
			} else
			if(((Scalar)f).getType().getName().equals(BYTEVECTOR)){
					gv.setByteVector(i, (byte[])o);
			}
		}
	}
}
