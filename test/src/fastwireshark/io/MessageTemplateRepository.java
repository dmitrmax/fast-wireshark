/*
 * This file is part of FAST Wireshark.
 *
 * FAST Wireshark is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FAST Wireshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 * 
 * You should have received a copy of the Lesser GNU General Public License
 * along with FAST Wireshark.  If not, see 
 * <http://www.gnu.org/licenses/lgpl.txt>.
 */
package fastwireshark.io;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.openfast.template.MessageTemplate;
import org.openfast.template.loader.MessageTemplateLoader;
import org.openfast.template.loader.XMLMessageTemplateLoader;

/**
 * A repository for central access for MessageTemplates
 * @author pmiele
 *
 */
public class MessageTemplateRepository {

	private static final List<MessageTemplate> templates = new ArrayList<MessageTemplate>();
	
	/**
	 * Loads and adds the templates from the specified file to the repository
	 * @param file The template file to load
	 * @throws FileNotFoundException If the file is not found
	 */
	public static void loadTemplates(String file) throws FileNotFoundException{
		InputStream templateSource = new FileInputStream(file);
		MessageTemplateLoader templateLoader = new XMLMessageTemplateLoader();
		Collections.addAll(templates,templateLoader.load(templateSource));
	}
	
	/**
	 * Retrieves the list of the loaded MessageTemplates
	 * @return Unmodifiable view of the loaded templates
	 */
	public static List<MessageTemplate> getTemplates(){
		return Collections.unmodifiableList(templates);
	}
	
	/**
	 * Retrieves a MessageTemplate from the repository based on the name of the template
	 * Case Sensitive
	 * @param name The name of the template to retrieve
	 * @return The template if it exists, null otherwise
	 */
	public static MessageTemplate getTemplateByName(String name){
		MessageTemplate retur = null;
		for(MessageTemplate t : templates){
			if(t.getName().equals(name)){
				retur = t;
				break;
			}
		}
		return retur;
	}
	
	/**
	 * Retrieves a MessageTemplate from the repository based on the id of the template
	 * @param id The ID of the template to retrieve
	 * @return The template if it exists, null otherwise
	 */
	public static MessageTemplate getTemplateByID(Integer id){
		MessageTemplate retur = null;
		for(MessageTemplate t : templates){
			if(t.getId().equals(id.toString())){
				retur = t;
				break;
			}
		}
		return retur;
	}
	
}
