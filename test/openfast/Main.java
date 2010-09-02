
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
            ostrm = new MessageOutputStream (new FileOutputStream("data1.fast"));
        /* ostrm.setTemplateRegistry (loader.getTemplateRegistry()); */
        ostrm.registerTemplate (1, template);
        Message
            msg;
        // end variables

        // First message.
        msg = new Message (template);

        msg.setInteger ("a", 1);
        msg.setInteger ("b", -2);
        msg.setInteger ("c", 3);

        msg.setLong ("x", 4294967296L);
        msg.setLong ("y", -4294967296L);
        msg.setLong ("z", 55);

        msg.setString ("name", "Your name here");

        msg.setDecimal ("decval1", 3.14);
        msg.setDecimal ("decval2", 3000000000.0);

        msg.setByteVector ("byte1", new byte[] { 1, 2, 3, 4, 5, 10, 11 });

        ostrm.writeMessage (msg);
        ostrm.close ();


        // Second message, use different file (for no particular reason).
        ostrm = new MessageOutputStream (new FileOutputStream("data2.fast"));
        ostrm.registerTemplate (1, template);
        msg = new Message (template);

        msg.setInteger ("a", 1);
        msg.setInteger ("b", -2);
        msg.setInteger ("c", 3);

        msg.setLong ("x", 200L);
        msg.setLong ("y", -4294967296L);
        msg.setLong ("z", 55);

        msg.setString ("name", "Your name here");

        msg.setDecimal ("decval1", 3.14);
        msg.setDecimal ("decval2", 3000000000.0);

        msg.setByteVector ("byte1", new byte[] { 1, 2, 3, 4, 5, 10, 11 });

        ostrm.writeMessage (msg);
        ostrm.close ();
    }
}

