import java.math.BigDecimal;
import java.util.Iterator;
import java.util.List;

import org.openfast.*;
import org.openfast.template.*;

import com.google.code.fastwireshark.data.DataPlan;
import com.google.code.fastwireshark.data.MessagePlan;
import com.google.code.fastwireshark.io.BinaryOutputStream;
import com.google.code.fastwireshark.io.MessageTemplateRepository;
import com.google.code.fastwireshark.io.XMLDataPlanLoader;

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
	private static final MessageOutputStream messageOut = new MessageOutputStream(new BinaryOutputStream(out, true));
	
	public static void main(String [] args){
		
		out.println("Hello World");
		try{
			/*
			 * LOAD TEMPLATE 
			 */
			
			MessageTemplateRepository.loadTemplates("templates.xml");
			
			for(int i = 0 ; i < MessageTemplateRepository.getTemplates().size() ; i++)
			{
				messageOut.registerTemplate(i, MessageTemplateRepository.getTemplates().get(i));
			}
			
			
			/*
			 * LOAD PLAN
			 */
			
			XMLDataPlanLoader loader = new XMLDataPlanLoader();
			
			DataPlan dp = loader.loadPlan("testPlan.xml");
			
			/*
			 * WRITE OUT
			 */
			for(MessagePlan mt : dp.getPlanList()){
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
			out.println("DEBUG LINE");
		} catch (Throwable e){
			e.printStackTrace();
		}
		out.println("Goodbye World");
		
	}
	
	
	
	public static void runMessageTest(MessagePlan mt){
		Message m = new Message(mt.getTemplate());
		populateFields(m,mt.getValues());
		messageOut.writeMessage(m);
	}
	
	/**
	 * Populates the fields in a message with those in the map
	 * @param message Message to populate
	 * @param values Values to put into message
	 */
	public static void populateFields(Message message,List<Object> values )
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
	
	/*
	 * TODO move this
	 */
	

}
