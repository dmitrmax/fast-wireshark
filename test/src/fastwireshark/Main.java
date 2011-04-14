/*
 * $License$ 
 */ 
package fastwireshark;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.openfast.*;

import fastwireshark.data.DataPlan;
import fastwireshark.io.AsciiBinaryOutputStream;
import fastwireshark.io.BinaryOutputStream;
import fastwireshark.io.MessageTemplateRepository;
import fastwireshark.io.PcapFileWriter;
import fastwireshark.io.TCPLoopBackOutputStream;
import fastwireshark.io.UDPLoopBackOutputStream;
import fastwireshark.io.UDPReceiverInputStream;
import fastwireshark.io.UDPSenderOutputStream;
import fastwireshark.io.XMLDataPlanLoader;
import fastwireshark.runner.DataPlanRunner;
import fastwireshark.util.Constants;

public class Main {


	public static void main(String [] args){
		//Dirty code, but effective
		/**
		 * Temporary class used to make using the command line arguments easier
		 * @author pmiele
		 *
		 */
		class CommandLineArguments{
			public List<String> templateFiles;
			public String dataPlanFile;
			public boolean binaryOutput;
			public short port = -1;
			public String pcapFile;
			public boolean tcpip;
			public boolean receiver;
			public String sender;

			/**
			 * Parses out the command line arguments and sets the public fields
			 * @param args The command line arguments from main
			 */
			public CommandLineArguments(String [] args){
				if(args.length == 0){
					System.out.println("Usage:\n" +
							"\t[-t <templateFile>[ <templateFile2>...]]\n" +
							"\t[-p <planFile>]\n" +
							"\t[-b]\n" +
							"\t[-n <port>]\n" +
							"\t[-P <pcap file>]\n" +
							"\t[-T]"
							);
					System.exit(0);
				}
				for(int i = 0 ; i < args.length ; i++){
					String cur = args[i];
					if(cur.equals("-t")){
						if(templateFiles != null){ throw new RuntimeException("Multiple definitions of template files on command line");}
						templateFiles = new ArrayList<String>();
						while(++i < args.length && !args[i].startsWith("-")){
							templateFiles.add(args[i]);
						} i--;
					} else
					if(cur.equals("-p")){
						if(dataPlanFile != null){ throw new RuntimeException("Multiple Plan files specified"); }
						dataPlanFile = args[++i];
					} else
					if(cur.equals("-b")){
						if(binaryOutput){throw new RuntimeException("Multiple definitions of binary data"); }
						binaryOutput = true;
					} else
					if(cur.equals("-n")){
						if(port > 0){throw new RuntimeException("Multiple port definitions");}
						port = Short.valueOf(args[++i]);
					} else
					if(cur.equals("-P")){
						if(pcapFile != null){throw new RuntimeException("Multiple pcap file definitions");}
						pcapFile = args[++i];
					}
					if(cur.equals("-T")){
						if(tcpip){throw new RuntimeException("Multiple TCP/IP definitions");}
						tcpip = true;
					}
					if(cur.equals("-R")){
						if(receiver){throw new RuntimeException("Multiple receiver definitions");}
						receiver = true;
					}
					if(cur.equals("-S")){
						if(sender != null){throw new RuntimeException("Multiple sender definitions");}
						sender = args[++i];
					}
				}
				if(dataPlanFile == null && !receiver){
					throw new RuntimeException("No data plan file specified");
				}
			}
			
			@Override
			public String toString(){
				StringBuilder builder = new StringBuilder();
				builder.append(templateFiles).append('\n');
				builder.append(dataPlanFile).append('\n');
				builder.append(binaryOutput).append('\n');
				builder.append(port).append('\n');
				builder.append(pcapFile).append('\n');
				builder.append(tcpip).append('\n');
				return builder.toString();
			}
		}
		CommandLineArguments cargs = null;
		try{
			cargs = new CommandLineArguments(args);
		} catch (RuntimeException e){
			System.err.println(e.getMessage());
			System.exit(1);
		}
		try{
			if(cargs.receiver && cargs.port > 0){
				//This goes into an infinite loop
				networkReciever(cargs.port);
			}
			OutputStream out;
			if(cargs.sender != null && cargs.port > 0){
				out = new UDPSenderOutputStream(cargs.port, cargs.sender);
			} else
			if(cargs.pcapFile != null && cargs.port > 0){
				out = new PcapFileWriter(cargs.port, cargs.pcapFile);
			} else
			if(cargs.tcpip && cargs.port > 0){
				out = new TCPLoopBackOutputStream(Constants.MAX_PACKET_SIZE, cargs.port);
			} else
			if(cargs.port > 0){
				out = new UDPLoopBackOutputStream(Constants.MAX_PACKET_SIZE, cargs.port);
			} else
			if(cargs.binaryOutput){
				out = new BinaryOutputStream(System.out, false);
			} else {
				out = new AsciiBinaryOutputStream(System.out, true);
			}
			MessageOutputStream messageOut = new MessageOutputStream(out);
			/*
			 * LOAD TEMPLATE 
			 */
			if(cargs.templateFiles != null){
				for(String templateFile : cargs.templateFiles){
					MessageTemplateRepository.loadTemplates(templateFile);
				}
			
				for(int i = 0 ; i < MessageTemplateRepository.getTemplates().size() ; i++)
				{
					messageOut.registerTemplate(i, MessageTemplateRepository.getTemplates().get(i));
				}
			}

			/*
			 * LOAD PLAN
			 */

			XMLDataPlanLoader loader = new XMLDataPlanLoader();

			DataPlan dp = loader.loadPlan(cargs.dataPlanFile);
			DataPlanRunner runner = new DataPlanRunner();
			runner.setMessageOutputStream(messageOut);
			runner.runDataPlan(dp);
		} catch (Throwable e){
			e.printStackTrace();
		}

	}
	
	public static void networkReciever(int port){
		InputStream in = new UDPReceiverInputStream(port);
		OutputStream out = new AsciiBinaryOutputStream(System.out, true);
		while(true){
			try {
				out.write(in.read());
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}


}