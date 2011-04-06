package fastwireshark.data;
import java.util.List;

import org.openfast.template.MessageTemplate;


public class MessagePlan extends Message{

	private MessageTemplate template;
	private List<Object> values;
	
	public MessagePlan(MessageTemplate t, List<Object> v, String from, String to){
		super(from,to);
		template = t;
		values = v;
	}
	
	public MessageTemplate getTemplate(){
		return template;
	}
	
	public List<Object> getValues(){
		return values;
	}
	
}
