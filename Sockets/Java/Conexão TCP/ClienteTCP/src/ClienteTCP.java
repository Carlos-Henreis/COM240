import java.io.*;
import java.net.*;
import javax.swing.JOptionPane;
class ClienteTCP {
    public static void main(String[] args) throws IOException {
        String frase = "234";
        String fraseModificada= "";
        do{
            frase = JOptionPane.showInputDialog("Entre com a frase");
            BufferedReader doUsuario = new BufferedReader(new InputStreamReader(System.in));
            Socket socketCliente = new Socket("200.235.93.93", 1995);

            DataOutputStream paraServidor = new DataOutputStream(socketCliente.getOutputStream());
            BufferedReader doServidor = new BufferedReader(new InputStreamReader(socketCliente.getInputStream()));
            //frase = doUsuario.readLine();
            paraServidor.writeBytes(frase + '\n');
            fraseModificada = doServidor.readLine();
            JOptionPane.showMessageDialog(null, "resposta do servidor " + fraseModificada);
            System.out.println ( "Do Servidor: " + fraseModificada ) ;
        }while (frase != null);
    }
}