import java.util.Map;

import org.openfast.template.MessageTemplate;


public class MessageTest {

	MessageTemplate template;
	Map<String, Object> values;
	
	public MessageTest(MessageTemplate t, Map<String, Object> v){
		template = t;
		values = v;
	}
	
	public MessageTemplate getTemplate(){
		return template;
	}
	
	public Map<String,Object> getValues(){
		return values;
	}
	
}
