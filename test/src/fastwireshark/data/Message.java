package fastwireshark.data;

import static fastwireshark.util.Constants.DEFAULT_ADDRESS;

public abstract class Message {

	private String to;
	private String from;
	
	public Message(String from, String to){
		if(from == null){from = DEFAULT_ADDRESS;}
		if(to == null){to = DEFAULT_ADDRESS;}
		this.from = from;
		this.to = to;
	}
	
	public String getFrom(){
		return from;
	}
	
	public String getTo(){
		return to;
	}
	
}
