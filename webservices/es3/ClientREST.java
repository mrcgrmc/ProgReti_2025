import java.io.*; 
import java.net.*;
import java.util.Arrays;

class ClientREST 
{    
    public static void main(String args[])
    {
        RESTAPI service1=new RESTAPI("127.0.0.1");

        if(args.length < 3)    {
            System.out.println("USAGE: java ClientREST tipofunzione op1 op2");
        }   
        else if(args[0].equals("calcola-somma")) {
            System.out.println("Risultato: " + service1.calcolaSomma(Float.parseFloat(args[1]), Float.parseFloat(args[2])));
        }
        else if(args[0].equals("calcola-numeri-primi")){
            int[] primi = service1.calcolaPrimi(Integer.parseInt(args[1]),Integer.parseInt(args[2]));
            System.out.println("Risultato: " + Arrays.toString(primi));        
        }
    }
}

class RESTAPI
{
    String server;

    RESTAPI(String remoteServer)  {
        server = new String(remoteServer);
    }

    float calcolaSomma(float val1, float val2)  {

        URL u = null;
        float risultato=0;
        int i;

        try 
        { 
            u = new URL("http://"+server+":8000/calcola-somma?param1="+val1+"&param2="+val2);
            System.out.println("URL aperto: " + u);
        }
        catch (MalformedURLException e) 
        {
            System.out.println("URL errato: " + u);
        }

        try 
        {
            URLConnection c = u.openConnection();
            c.connect();
            BufferedReader b = new BufferedReader(new InputStreamReader(c.getInputStream()));
            System.out.println("Lettura dei dati...");
            String s;
            while( (s = b.readLine()) != null ) {
                System.out.println(s);
                if((i = s.indexOf("somma"))!=-1)
                    risultato = Float.parseFloat(s.substring(i+7));
            }
        }
        catch (IOException e) 
        {
            System.out.println(e.getMessage());
        }
    
        return (float)risultato;
    }


    /**
     * Richiede al server la lista dei primi compresi fra min e max.
     * @return array di numeri primi (lunghezza 0 se errore o nessun primo)
     */
    int[] calcolaPrimi(int min, int max) {

        URL u = null;
        int[] risultato = new int[0];
        try {
            u = new URL("http://" + server + ":8000/calcola-numeri-primi?min=" + min + "&max=" + max);
            System.out.println("URL aperto: " + u);
        } catch (MalformedURLException e) {
            System.out.println("URL errato: " + u);
            return risultato;
        }

        try {
            URLConnection c = u.openConnection();
            c.connect();

            BufferedReader b = new BufferedReader(new InputStreamReader(c.getInputStream()));
            System.out.println("Lettura dei dati…");

            // Leggo tutto il corpo in un’unica stringa
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = b.readLine()) != null)
                sb.append(line);

            String json = sb.toString();

            int left  = json.indexOf('[');
            int right = json.indexOf(']');

            if (left != -1 && right != -1 && right > left + 1) {
                String content = json.substring(left + 1, right).trim();
                String[] tokens = content.split(",");

                risultato = new int[tokens.length];
                for (int i = 0; i < tokens.length; i++)
                    risultato[i] = Integer.parseInt(tokens[i].trim());
            }

        } catch (IOException e) {
            System.out.println("Errore I/O: " + e.getMessage());
        }

        return risultato;
    }

}
