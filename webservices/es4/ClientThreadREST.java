import java.io.*;
import java.net.*;
import java.util.*;

/**
 *  Uso:
 *      java ClientThreadREST calcola-numeri-primi <min> <max>
 *      java ClientThreadREST calcola-somma <val1> <val2>
 *
 *  Per "calcola-numeri-primi" l’intervallo viene diviso in tre blocchi contigui
 *  e spedito in parallelo ai server REST in esecuzione su 127.0.0.1/2/3 (porta 8000).
 */
class ClientThreadREST {

    public static void main(String[] args) {

        if (args.length < 3) {
            System.out.println("USAGE: java ClientThreadREST <tipofunzione> <op1> <op2>");
            return;
        }

        String service = args[0];

        if (service.equals("calcola-numeri-primi")) {
            // parsing intervallo
            int gMin = Integer.parseInt(args[1]);
            int gMax = Integer.parseInt(args[2]);
            if (gMin > gMax) {
                System.out.println("ERRORE: min deve essere ≤ max");
                return;
            }

            // suddivido in tre porzioni il più uniformi possibile
            int total  = gMax - gMin + 1;
            int step   = total / 3;
            int extra  = total % 3;           // 0,1,2

            int min1 = gMin;
            int max1 = min1 + step - 1 + (extra > 0 ? 1 : 0);
            int min2 = max1 + 1;
            int max2 = min2 + step - 1 + (extra > 1 ? 1 : 0);
            int min3 = max2 + 1;
            int max3 = gMax;

            RESTAPI s1 = new RESTAPI("127.0.0.1", service, String.valueOf(min1), String.valueOf(max1));
            RESTAPI s2 = new RESTAPI("127.0.0.2", service, String.valueOf(min2), String.valueOf(max2));
            RESTAPI s3 = new RESTAPI("127.0.0.3", service, String.valueOf(min3), String.valueOf(max3));

            s1.start();
            s2.start();
            s3.start();
        } else {
            // default: passa gli stessi parametri a tutti e 3 i server
            RESTAPI s1 = new RESTAPI("127.0.0.1", service, args[1], args[2]);
            RESTAPI s2 = new RESTAPI("127.0.0.2", service, args[1], args[2]);
            RESTAPI s3 = new RESTAPI("127.0.0.3", service, args[1], args[2]);

            s1.start();
            s2.start();
            s3.start();
        }
    }
}
class RESTAPI extends Thread {
    private final String server;
    private final String service;
    private final String param1;
    private final String param2;

    RESTAPI(String remoteServer, String srvc, String p1, String p2) {
        this.server  = remoteServer;
        this.service = srvc;
        this.param1  = p1;
        this.param2  = p2;
    }

    @Override
    public void run() {
        try {
            if (service.equals("calcola-somma")) {
                float res = calcolaSomma(Float.parseFloat(param1), Float.parseFloat(param2));
                System.out.printf("[%s] Somma: %f%n", server, res);
            } else if (service.equals("calcola-numeri-primi")) {
                int[] primes = calcolaPrimi(Integer.parseInt(param1), Integer.parseInt(param2));
                System.out.printf("[%s] Primi %s-%s: %s%n", server, param1, param2, Arrays.toString(primes));
            } else {
                System.out.println("Servizio non disponibile!");
            }
        } catch (NumberFormatException nfe) {
            System.out.println("Parametri numerici non validi: " + nfe.getMessage());
        }
    }

    private float calcolaSomma(float val1, float val2) {
        URL url;
        try {
            url = new URL("http://" + server + ":8000/calcola-somma?param1=" + val1 + "&param2=" + val2);
        } catch (MalformedURLException e) {
            System.out.println("URL errato");
            return 0;
        }

        try {
            URLConnection c = url.openConnection();
            c.connect();

            BufferedReader br = new BufferedReader(new InputStreamReader(c.getInputStream()));
            String line;
            while ((line = br.readLine()) != null) {
                int i = line.indexOf("somma");
                if (i != -1) {
                    return Float.parseFloat(line.substring(i + 7));
                }
            }
        } catch (IOException ioe) {
            System.out.println("Errore I/O: " + ioe.getMessage());
        }
        return 0;
    }
    private int[] calcolaPrimi(int min, int max) {
        URL url;
        try {
            url = new URL("http://" + server + ":8000/calcola-numeri-primi?min=" + min + "&max=" + max);
        } catch (MalformedURLException e) {
            System.out.println("URL errato");
            return new int[0];
        }

        try {
            URLConnection c = url.openConnection();
            c.connect();

            BufferedReader br = new BufferedReader(new InputStreamReader(c.getInputStream()));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) sb.append(line);

            String body = sb.toString();
            int left = body.indexOf('[');
            int right = body.indexOf(']');
            if (left == -1 || right == -1 || right <= left + 1)
                return new int[0];

            String[] toks = body.substring(left + 1, right).split(",");
            int[] out = new int[toks.length];
            for (int i = 0; i < toks.length; i++)
                out[i] = Integer.parseInt(toks[i].trim());
            return out;
        } catch (IOException ioe) {
            System.out.println("Errore I/O: " + ioe.getMessage());
            return new int[0];
        }
    }
}
