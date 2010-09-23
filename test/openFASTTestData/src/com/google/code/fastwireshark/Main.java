package com.google.code.fastwireshark;

import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.List;

import org.openfast.*;

import com.google.code.fastwireshark.data.DataPlan;
import com.google.code.fastwireshark.io.AsciiBinaryOutputStream;
import com.google.code.fastwireshark.io.BinaryOutputStream;
import com.google.code.fastwireshark.io.MessageTemplateRepository;
import com.google.code.fastwireshark.io.XMLDataPlanLoader;
import com.google.code.fastwireshark.runner.DataPlanRunner;

import static java.lang.System.out;


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

			/**
			 * Parses out the command line arguments and sets the public fields
			 * @param args The command line arguments from main
			 */
			public CommandLineArguments(String [] args){

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
		CommandLineArguments cargs = new CommandLineArguments(args);
		try{
			/*
			 * LOAD TEMPLATE 
			 */
			MessageOutputStream messageOut;
			if(cargs.binaryOutput){
				messageOut = new MessageOutputStream(new BinaryOutputStream(new FileOutputStream("output.txt"), false));
			} else {
				messageOut = new MessageOutputStream(new AsciiBinaryOutputStream(out, true));
			}
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