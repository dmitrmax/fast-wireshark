import java.util.ArrayList;
import java.util.List;


public class TestCase {

	private final List<MessageTest> testList = new ArrayList<MessageTest>();
	
	public void addMessageTest(MessageTest mt){
		testList.add(mt);
	}
	
	public List<MessageTest> getTestList(){
		return testList;
	}
	
}
