#include <iostream>
#include <boost/asio.hpp>
#include <boost/property_tree/ini_parser.hpp>

int main() {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service);

    boost::property_tree::ptree config;
    
    try {
        // Carregar configurações do arquivo de configuração
        boost::property_tree::ini_parser::read_ini("config.ini", config);

        // Obter a porta do servidor do arquivo de configuração
        int serverPort = config.get<int>("PORT", 12345);

        // Endereço e porta do servidor
        boost::asio::ip::tcp::endpoint server_endpoint(
            boost::asio::ip::address::from_string("127.0.0.1"), serverPort);

        // Conectar ao servidor
        socket.connect(server_endpoint);
        std::cout << "Conectado ao servidor." << std::endl;

        // Enviar dados para o servidor
        std::string message = "Dados que serão enviados ao servidor.";
        boost::asio::write(socket, boost::asio::buffer(message));

        std::cout << "Dados enviados ao servidor." << std::endl;

        // Fechar a conexão
        socket.close();
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
    }

    return 0;
}
