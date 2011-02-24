package fastwireshark.data;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;


public class DataPlan implements Iterable<Message>{

	private final List<Message> dataPlan = new ArrayList<Message>();
	
	public void addMessagePlan(Message mt){
		dataPlan.add(mt);
	}
	
	public List<Message> getPlanList(){
		return Collections.unmodifiableList(dataPlan);
	}
	
	public Iterator<Message> iterator(){
		return dataPlan.iterator();
	}
	
}
