package com.google.code.fastwireshark;

import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.openfast.*;

import com.google.code.fastwireshark.data.DataPlan;
import com.google.code.fastwireshark.io.AsciiBinaryOutputStream;
import com.google.code.fastwireshark.io.BinaryOutputStream;
import com.google.code.fastwireshark.io.MessageTemplateRepository;
import com.google.code.fastwireshark.io.UDPLoopBackOutputStream;
import com.google.code.fastwireshark.io.XMLDataPlanLoader;
import com.google.code.fastwireshark.runner.DataPlanRunner;
import com.google.code.fastwireshark.util.Constants;



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
			public int port = -1;
			public String pcapFile;

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
							"\t[-n <port>]" +
							"\t[-P <pcap file>"
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
						port = Integer.valueOf(args[++i]);
					} else
					if(cur.equals("-P")){
						if(pcapFile != null){throw new RuntimeException("Multiple pcap file definitions");}
						pcapFile = args[++i];
					}
				}
				if(templateFiles == null){
					throw new RuntimeException("No template files specified");
				}
				if(dataPlanFile == null){
					throw new RuntimeException("No data plan file specified");
				}
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
			/*
			 * LOAD TEMPLATE 
			 */
			OutputStream out;
			if(cargs.port > 0){
				out = new UDPLoopBackOutputStream(Constants.MAX_PACKET_SIZE, cargs.port, cargs.pcapFile);
			} else
			if(cargs.binaryOutput){
				out = new BinaryOutputStream(System.out, false);
			} else {
				out = new AsciiBinaryOutputStream(System.out, true);
			}
			MessageOutputStream messageOut = new MessageOutputStream(out);
			
			for(String templateFile : cargs.templateFiles){
				MessageTemplateRepository.loadTemplates(templateFile);
			}

			for(int i = 0 ; i < MessageTemplateRepository.getTemplates().size() ; i++)
			{
				messageOut.registerTemplate(i, MessageTemplateRepository.getTemplates().get(i));
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


}