import java.io.*;
import java.net.*;
import javax.swing.JOptionPane;

class servidorTCP{
    public static void main(String argv[]) throws Exception {
        String fraseCliente="";
        String frase;
        ServerSocket socketRecepcao = new ServerSocket(1995);
        while(true) {
            Socket socketConexao = socketRecepcao.accept();
            BufferedReader doCliente = new BufferedReader(new InputStreamReader(socketConexao.getInputStream()));
            DataOutputStream paraCliente = new DataOutputStream(socketConexao.getOutputStream()) ;
            fraseCliente= doCliente.readLine();
            JOptionPane.showMessageDialog(null, "Saida do cliente " + fraseCliente);
            frase = JOptionPane.showInputDialog("Entre servidor");
            frase += "\n";
            paraCliente.writeBytes(frase);
        }
    }
}
    