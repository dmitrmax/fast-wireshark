
import org.openfast.*;
import org.openfast.template.MessageTemplate;
import org.openfast.template.loader.*;

import java.io.FileInputStream;
import java.io.FileOutputStream;

public class Main
{
    public static void main (String args[]) throws Exception
    {
        MessageTemplateLoader
            loader = new XMLMessageTemplateLoader();
        FileInputStream
            istrm = new FileInputStream ("templates.xml");
        loader.load (istrm);
        istrm.close ();
        MessageTemplate
            template = loader.getTemplateRegistry().get("TestTemplate1");
        MessageOutputStream
            ostrm = new MessageOutputStream (new FileOutputStream("data.fast"));
        /* ostrm.setTemplateRegistry (loader.getTemplateRegistry()); */
        ostrm.registerTemplate (1, template);
        Message
            msg;
        // end variables

        msg = new Message (template);

        msg.setInteger ("a", 1);
        msg.setInteger ("b", -2);
        msg.setInteger ("c", 3);

        msg.setLong ("x", 4294967296L);
        msg.setLong ("y", -4294967296L);
        msg.setLong ("z", 55);

        msg.setString ("name", "Your name here");

        ostrm.writeMessage (msg);
        ostrm.close ();
    }
}

