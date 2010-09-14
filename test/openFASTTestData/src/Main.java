import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.openfast.*;
import org.openfast.template.*;
import org.openfast.template.loader.*;

import static java.lang.System.out;


public class Main {
	
	private static final String INT32 = "int32";
	private static final String UINT32 = "uInt32";
	private static final String INT64 = "int64";
	private static final String UINT64 = "uInt64";
	private static final String DECIMAL = "decimal";
	private static final String STRING = "string";
	private static final Object UNICODE = "unicode";
	private static final Object ASCII = "ascii";
	private static final MessageOutputStream messageOut = new MessageOutputStream(new BinaryOutputStream());
	//TODO move somewhere else
	private static MessageTemplate[] templates;
	
	@SuppressWarnings("serial")
	public static void main(String [] args){
		
		out.println("Hello World");
		try{
			/*
			 * LOAD TEMPLATE 
			 * TODO move this
			 */
			InputStream templateSource = new FileInputStream("templates.xml");
			MessageTemplateLoader templateLoader = new XMLMessageTemplateLoader();
			templates = templateLoader.load(templateSource);
			for(MessageTemplate t : templates){
				out.println(t);
				for(Field f : t.getFields()){
					out.println("\t" + f.getName());
				}
			}
			
			/*
			 * WRITE OUT
			 */
			//FileOutputStream("output.txt")
			for(int i = 0 ; i < templates.length ; i++)
			{
				messageOut.registerTemplate(i, templates[i]);
			}
			TestCase tc = new TestCase();
			
			
			tc.addMessageTest(new MessageTest(getTemplateByName("t_ascii_unicode_string"), new HashMap<String,Object>(){
				{
					put("a","abc");
					put("u","abc");
					put("s","abc");
				}
			}));
			
			for(MessageTest mt : tc.getTestList()){
				out.println("***");
				runMessageTest(mt);
			}
			
			
			
			/*
			 * READ IN
			 */
/*
			InputStream fastEncodedStream = new FileInputStream("output.txt");
			MessageInputStream messageIn = new MessageInputStream(fastEncodedStream);
			for(int i = 0; i < NUM_TEMPLATES; i++)
			{
				messageIn.registerTemplate(i, templates[i]);
			}
			for(int i = 0if(f.getQName().getName().equals(INT64) ||
			   f.getQName().getName().equals(UINT64)){
				message.setLong(e.getKey(), (Long)e.getValue());
			} ; i < 2 ; i++){
				message = messageIn.readMessage();
				int number = message.getInt("int32");
				int templateId = message.getInt(0);
				out.println(number + " : " + templateId);
			}
			//*/
		} catch (Throwable e){
			out.println(e);
		}
		out.println("Goodbye World");
		
	}
	
	
	
	public static class BinaryOutputStream extends OutputStream {

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
			out.println(s);
		}
		
	}
	
	public static void runMessageTest(MessageTest mt){
		Message m = new Message(mt.getTemplate());
		populateFields(m,mt.getValues());
		messageOut.writeMessage(m);
	}
	
	/**
	 * Populates the fields in a message with those in the map
	 * @param message Message to populate
	 * @param values Values to put into message
	 */
	public static void populateFields(Message message,Map<String, Object> values )
	{
		for(Entry<String, Object> e : values.entrySet()){
			Field f = message.getTemplate().getField(e.getKey());
			
			if(((Scalar)f).getType().getName().equals(INT32) ||
			   ((Scalar)f).getType().getName().equals(UINT32)){
				message.setInteger(e.getKey(), (Integer)e.getValue());
			} else
			if(((Scalar)f).getType().getName().equals(INT64) ||
			   ((Scalar)f).getType().getName().equals(UINT64)){
				message.setLong(e.getKey(), (Long)e.getValue());
			} else
			if(((Scalar)f).getType().getName().equals(DECIMAL)){
				//Decimal can be of differing arguments, further determine correct type
				if(e.getValue() instanceof Double){
					message.setDecimal(e.getKey(), (Double)e.getValue());
				} else 
				if(e.getValue() instanceof Float){
					message.setDecimal(e.getKey(), (Float)e.getValue());
				} else 
				if(e.getValue() instanceof BigDecimal){
					message.setDecimal(e.getKey(), (BigDecimal)e.getValue());
				}
			} else
			if(((Scalar)f).getType().getName().equals(STRING) ||
			   ((Scalar)f).getType().getName().equals(UNICODE) ||
			   ((Scalar)f).getType().getName().equals(ASCII)){
				message.setString(e.getKey(), (String)e.getValue());
			}
		}
	}
	
	/*
	 * TODO move this
	 */
	public static MessageTemplate getTemplateByName(String name){
		MessageTemplate retur = null;
		for(MessageTemplate t : templates){
			if(t.getName().equals(name)){
				retur = t;
				break;
			}
		}
		return retur;
	}

}
