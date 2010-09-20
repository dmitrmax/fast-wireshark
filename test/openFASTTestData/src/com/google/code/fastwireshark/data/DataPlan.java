package com.google.code.fastwireshark.data;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public class DataPlan {

	private final List<MessagePlan> dataPlan = new ArrayList<MessagePlan>();
	
	public void addMessagePlan(MessagePlan mt){
		dataPlan.add(mt);
	}
	
	public List<MessagePlan> getPlanList(){
		return Collections.unmodifiableList(dataPlan);
	}
	
}
